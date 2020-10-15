/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_language.h"
#include "uti/sge_stdlib.h"

#include "gdi/sge_gdi.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_ja_task.h"

#include "sched/sort_hosts.h"
#include "sched/sgeee.h"

#include "sge_usageL.h"

#define HOST_USAGE_ATTR_CPU     "cpu"
#define HOST_USAGE_ATTR_MEM     "mem"
#define HOST_USAGE_ATTR_IO      "io"

#define ERROR_LOAD_VAL 9999


#ifdef notdef
#define running_status(x) ((x)!=IDLE && (x)!=JFINISHED)
#endif

int sge_split_job_finished(lList **jobs, lList **finished, char *finished_name);
int sge_split_job_running(lList **jobs, lList **running, char *running_name);

typedef struct {
   int name_format;
   int format_times;
   char *delim;
   char *line_delim;
   char *rec_delim;
   char *str_format;
   char *field_names;
} format_t;

static int
setup_lists(lList **jobs, lList **hosts, lList **config, lList **centry)
{
   lList *alp;
   lListElem *aep;
   lEnumeration *what;

   /*
    * get job_list
    */

   what = lWhat("%T(ALL)", JB_Type);
   alp=sge_gdi(SGE_JB_LIST, SGE_GDI_GET, jobs, NULL, what);
   lFreeWhat(what);

   aep = lFirst(alp);
   answer_exit_if_not_recoverable(aep);
   if (answer_get_status(aep) != STATUS_OK) {
      fprintf(stderr, "%s", lGetString(aep, AN_text));
      exit(1);
   }
   lFreeList(alp);
#ifdef notdef
   if (!*jobs || (root = lFirst(*sharetree)) == NULL) {
      fprintf(stderr, MSG_JOB_NOJOBLIST );
      exit(2);
   }
#endif
   /*
    * get host list
    */

   what = lWhat("%T(ALL)", EH_Type);
   alp=sge_gdi(SGE_EH_LIST, SGE_GDI_GET, hosts, NULL, what);
   lFreeWhat(what);

   aep = lFirst(alp);
   answer_exit_if_not_recoverable(aep);
   if (answer_get_status(aep) != STATUS_OK) {
      fprintf(stderr, "%s", lGetString(aep, AN_text));
      exit(1);
   }
   lFreeList(alp);

   /*
    * get centry list
    */

   what = lWhat("%T(ALL)", CE_Type);
   alp=sge_gdi(SGE_CE_LIST, SGE_GDI_GET, centry, NULL, what);
   lFreeWhat(what);

   aep = lFirst(alp);
   answer_exit_if_not_recoverable(aep);
   if (answer_get_status(aep) != STATUS_OK) {
      fprintf(stderr, "%s", lGetString(aep, AN_text));
      exit(1);
   }
   lFreeList(alp);


   /*
    * get config list
    */

   what = lWhat("%T(ALL)", SC_Type);
   alp=sge_gdi(SGE_SC_LIST, SGE_GDI_GET, config, NULL, what);
   lFreeWhat(what);

   aep = lFirst(alp);
   answer_exit_if_not_recoverable(aep);
   if (answer_get_status(aep) != STATUS_OK) {
      fprintf(stderr, "%s", lGetString(aep, AN_text));
      exit(1);
   }
   lFreeList(alp);
   alp = NULL;
      if (!sconf_validate_config(&alp, config))
        fprintf(stderr, "%s\n", lGetString(lFirst(alp), AN_text));


   return 0;


}
/*--------------------------------------------------------------------
 * build_usage_list - create a new usage list from an existing list
 *--------------------------------------------------------------------*/
static lList *build_usage_list( char *name, lList *old_usage_list )
{
   lList *usage_list = NULL;
   lListElem *usage;

   if (old_usage_list) {

      /*-------------------------------------------------------------
       * Copy the old list and zero out the usage values
       *-------------------------------------------------------------*/

      usage_list = lCopyList(name, old_usage_list);
      for_each(usage, usage_list)
         lSetDouble(usage, UA_value, 0);

   } else {

      /*
       * the UA_value fields are implicitly set to 0 at creation
       * time of a new element with lCreateElem or lAddElemStr
       */

      lAddElemStr(&usage_list, UA_name, HOST_USAGE_ATTR_CPU, UA_Type);
      lAddElemStr(&usage_list, UA_name, HOST_USAGE_ATTR_MEM, UA_Type);
      lAddElemStr(&usage_list, UA_name, HOST_USAGE_ATTR_IO, UA_Type);
   }

   return usage_list;
}


static int
free_lists(lList *jobs, lList *hosts, lList *config)
{
   if (jobs)
      lFreeList(jobs);
   if (hosts)
      lFreeList(&hosts);
   if (config)
      lFreeList(&config);
   return 0;
}

/* ----------------------------------------

   calculate_host_usage()

   calculates the usage for a host from the
   JB_scaled_usage_list field for jobs associated with the host


   returns:
      0 successful
     -1 errors in functions called by calculate_host_usage

*/

int calculate_host_usage(lList **jobs, lList *hosts)
{
lList *running_jobs = NULL;    /* JB_Type */
lList *finished_jobs = NULL;   /* JB_Type */
char *host_name;
double host_cpu_usage, host_io_usage, host_mem_usage;
double host_cpu_usage_pct, host_io_usage_pct, host_mem_usage_pct;
double total_host_cpu_usage=0.0, total_host_io_usage=0.0, total_host_mem_usage=0.0;
lListElem *hep, *running_job_elem, *rjq, *job_usage_elem, *host_usage_elem, *host_usage_pct_elem, *finished_job_elem, *fjq;
lList *job_usage_list = NULL;

/* 'ulong' is not known on some architectures */
#ifndef ulong 
#define ulong lUlong
#endif
ulong host_sge_tickets, total_host_tickets=0, num_running_jobs_on_host;
#ifdef TEST_CALC_HOST_USAGE
FILE *fpdchu;
#endif
   DENTER(TOP_LAYER, "calculate_host_usage");
#ifdef TEST_CALC_HOST_USAGE
   fpdchu = fopen("/tmp/sge_debug_calc_host_usage.out", "a");
#endif
   if (!(hosts)) {
      DEXIT;
      return -1;
   }
      /* split job list in finished jobs and others */
      if (sge_split_job_finished( 
            jobs, /* source list                        */ 
            &finished_jobs,     /* put the finished jobs at this location */
            "finished jobs")) {
         DPRINTF(("couldn't split job list concerning finished state\n"));
#ifdef TEST_CALC_HOST_USAGE
         fprintf(fpdchu, "%s\n", MSG_JOB_SPLITJOBLISTFINISHSTATEFAILED );
         fclose(fpdchu);
#endif
         DEXIT;
         return -1;
      }

   /* split job list in running jobs and others */
   if (sge_split_job_running(
         jobs,   /* source list
   */
         &running_jobs,         /* put the running jobs at this location
   */
         "running jobs")) {
      DPRINTF(("couldn't split job list in running and non running ones\n"));
#ifdef TEST_CALC_HOST_USAGE

         fprintf(fpdchu, "%s\n", MSG_JOB_SPLITJOBLISTRUNNINGSTATEFAILED );
         fclose(fpdchu);
#endif
      DEXIT;
      return -1;
   }

#ifdef notdef
   fprintf(fpdchu, "host_list \n");
   lDumpList(fpdchu, hosts, 5);
   fprintf(fpdchu, "running_jobs \n");
   lDumpList(fpdchu, running_jobs, 5);
   fprintf(fpdchu, "finished_jobs \n");
   lDumpList(fpdchu, finished_jobs, 5);
#endif
   for_each (hep, hosts) {    
      host_name = lGetHost(hep, EH_name);
      if (strcmp(host_name,"global")) { /* don't treat global */

#ifdef TEST_CALC_HOST_USAGE
      fprintf(fpdchu, "For host_name %s \n", host_name);
#endif
      host_cpu_usage = 0.0;
      host_mem_usage = 0.0;
      host_io_usage = 0.0;
      num_running_jobs_on_host = 0;


/* Calculate host usages for running jobs */

       for_each (running_job_elem, running_jobs) {

        for_each (rjq, lGetList(running_job_elem, JB_granted_destin_identifier_list)) {

#ifdef TEST_CALC_HOST_USAGE
          fprintf(fpdchu, "Job %u is on host %s\n", lGetUlong(running_job_elem, JB_job_number), lGetHost(rjq, JG_qhostname));
#endif
          job_usage_list = NULL;
          if (sge_hostcmp(lGetHost(rjq, JG_qhostname), host_name) == 0) {

#ifdef TEST_CALC_HOST_USAGE
            fprintf(fpdchu, "found running job on host %s \n", host_name);
#endif
            num_running_jobs_on_host += 1; 
            job_usage_list = lGetList(running_job_elem, JB_scaled_usage_list);

#ifdef TEST_CALC_HOST_USAGE
            fprintf(fpdchu, "job_usage_list \n");
            lDumpList(fpdchu, job_usage_list, 5);
#endif
            for_each(job_usage_elem, job_usage_list)  {
               if (strcmp(lGetString(job_usage_elem, UA_name), "cpu") == 0)  {
                  host_cpu_usage += lGetDouble(job_usage_elem, UA_value);
                  total_host_cpu_usage += lGetDouble(job_usage_elem, UA_value);
               }
               else  {
                  if (strcmp(lGetString(job_usage_elem, UA_name), "mem") == 0)  {
                     host_mem_usage += lGetDouble(job_usage_elem, UA_value);
                     total_host_mem_usage += lGetDouble(job_usage_elem, UA_value);
                  }
                  else  {
                    if (strcmp(lGetString(job_usage_elem, UA_name), "io") == 0)  {
                      host_io_usage += lGetDouble(job_usage_elem, UA_value);
                      total_host_io_usage += lGetDouble(job_usage_elem, UA_value);

                    } /* if (lGetString(job_usage_elem, UA_name)=="io")*/
                  } /* if (lGetString(job_usage_elem, UA_name)=="mem")*/                }  /* if (lGetString(job_usage_elem, UA_name)=="cpu")*/
             }  /*for_each(job_usage_elem, job_usage_list)*/
          }  /* if (sge_hostcmp(lGetHost(rjq, JG_qhostname), host_name) ==
0) */
        } /* for_each (rjq, lGetList(running_job_elem, JB_granted_destin_identifier_list))*/
      }  /* for_each (running_job_elem, running_jobs) */

      /* Calculate the number of running jobs on the host -- This code
was added here because it was a convenient place to gather the information */

      lSetUlong(hep, EH_num_running_jobs, num_running_jobs_on_host);
#ifdef TEST_CALC_HOST_USAGE
/*
          fprintf(fpdchu, "Num of jobs on host %u\n", lGetUlong(hep, EH_num_running_jobs));
*/
#endif
 

/* Calculate host usages for finished jobs */

      for_each (finished_job_elem, finished_jobs) {

        for_each (fjq, lGetList(finished_job_elem, JB_granted_destin_identifier_list)) {
#ifdef TEST_CALC_HOST_USAGE
          fprintf(fpdchu, "Job %u is on host %s\n", lGetUlong(finished_job_elem, JB_job_number), lGetHost(fjq, JG_qhostname));
#endif
          job_usage_list = NULL;
          if (sge_hostcmp(lGetHost(fjq, JG_qhostname), host_name) == 0) {
#ifdef TEST_CALC_HOST_USAGE
            fprintf(fpdchu, "found finished job on host %s \n", host_name);
#endif
            job_usage_list = lGetList(finished_job_elem, JB_scaled_usage_list);
#ifdef TEST_CALC_HOST_USAGE
            fprintf(fpdchu, "job_usage_list \n");
            lDumpList(fpdchu, job_usage_list, 5);
#endif
            for_each(job_usage_elem, job_usage_list)  {
               if (strcmp(lGetString(job_usage_elem, UA_name), "cpu") == 0)  {
                  host_cpu_usage += lGetDouble(job_usage_elem, UA_value);
                  total_host_cpu_usage += lGetDouble(job_usage_elem, UA_value);
               }
               else  {
                  if (strcmp(lGetString(job_usage_elem, UA_name), "mem") == 0)  {
                     host_mem_usage += lGetDouble(job_usage_elem, UA_value);
                     total_host_mem_usage += lGetDouble(job_usage_elem, UA_value);
                  }
                  else  {
                    if (strcmp(lGetString(job_usage_elem, UA_name), "io") == 0)  {
                      host_io_usage += lGetDouble(job_usage_elem, UA_value);
                      total_host_io_usage += lGetDouble(job_usage_elem, UA_value);
                    } /* if (lGetString(job_usage_elem, UA_name)=="io")*/
                  } /* if (lGetString(job_usage_elem, UA_name)=="mem")*/                }  /* if (lGetString(job_usage_elem, UA_name)=="cpu")*/
            }  /*for_each(job_usage_elem, job_usage_list)*/
          }  /* if (sge_hostcmp(lGetHost(fjq, JG_qhostname), host_name) ==
0) */
        } /* for_each (fjq, lGetList(finished_job_elem, JB_granted_destin_identifier_list))*/
      }  /* for_each (finished_job_elem, finished_jobs) */



/* build the EH_scaled_usage_list */

   lSetList(hep, EH_scaled_usage_list, build_usage_list("hostusagelist", NULL));

   for_each(host_usage_elem, lGetList(hep, EH_scaled_usage_list))  {
      if (strcmp(lGetString(host_usage_elem, UA_name), "cpu") == 0)  {
         lSetDouble(host_usage_elem, UA_value, host_cpu_usage);
      }
      else  {
         if (strcmp(lGetString(host_usage_elem, UA_name), "mem") == 0)
{
            lSetDouble(host_usage_elem, UA_value, host_mem_usage);
         }
         else  {
            if (strcmp(lGetString(host_usage_elem, UA_name), "io") == 0)  {
               lSetDouble(host_usage_elem, UA_value, host_io_usage);
            } /* if (lGetString(host_usage_elem, UA_name)=="io")*/
         } /* if (lGetString(host_usage_elem, UA_name)=="mem")*/
      }  /* if (lGetString(host_usage_elem, UA_name)=="cpu")*/
   }  /*for_each(host_usage_elem, lGetList(hep, EH_scaled_usage_list)*/
   }  /* if (strcmp(host,"global")) */
}  /* for_each (hep, hosts) */


/*****************************************************************/

   /* build the EH_scaled_usage_pct_list */
   /*  EH_scaled_usage_pct_list is comprised of the individual usage 
       percentages for cpu, mem, and io on a host basis */

   for_each (hep, hosts)  {   
      host_name = lGetHost(hep, EH_name);
      if (strcmp(host_name,"global")) { /* don't treat global */
         lList *hep_EH_scaled_usage_list = NULL;

         lSetList(hep, EH_scaled_usage_pct_list, build_usage_list("hostusagelist", NULL));

         hep_EH_scaled_usage_list = lGetList(hep, EH_scaled_usage_list);        
 
         for_each(host_usage_pct_elem, lGetList(hep, EH_scaled_usage_pct_list)) {
            if (strcmp(lGetString(host_usage_pct_elem, UA_name), "cpu") == 0)  {
               if (total_host_cpu_usage != 0)  {
                  host_cpu_usage = lGetDouble(lGetElemStr(hep_EH_scaled_usage_list, UA_name, "cpu"), UA_value);
                  host_cpu_usage_pct = 100.0 * (host_cpu_usage/total_host_cpu_usage);
               } else {
                  host_cpu_usage_pct = 0.0;
               }        
               lSetDouble(host_usage_pct_elem, UA_value, host_cpu_usage_pct);
            } else {
               if (strcmp(lGetString(host_usage_pct_elem, UA_name), "mem") == 0) {
                  if (total_host_mem_usage != 0)  {
                     host_mem_usage = lGetDouble(lGetElemStr(hep_EH_scaled_usage_list, UA_name, "mem"), UA_value);
                     host_mem_usage_pct = 100.0 * (host_mem_usage/total_host_mem_usage);
                  } else { 
                     host_mem_usage_pct = 0.0;
                  }
                  lSetDouble(host_usage_pct_elem, UA_value, host_mem_usage_pct);
               } else {
                  if (strcmp(lGetString(host_usage_pct_elem, UA_name), "io") == 0)  {
                     if (total_host_io_usage != 0)  {
                        host_io_usage = lGetDouble(lGetElemStr(hep_EH_scaled_usage_list, UA_name, "io"), UA_value);
                        host_io_usage_pct = 100 * (host_io_usage/total_host_io_usage);
                     } else {
                        host_io_usage_pct = 0.0;
                     }
                     lSetDouble(host_usage_pct_elem, UA_value, host_io_usage_pct);
                  } /* if (lGetString(host_usage_elem, UA_name)=="io")*/
               } /* if (lGetString(host_usage_elem, UA_name)=="mem")  (else) */
            }  /* if (lGetString(host_usage_elem, UA_name)=="cpu")*/
         }  /*for_each(host_usage_elem, lGetList(hep, EH_scaled_usage_pct_list)*/
      } /* if (strcmp(host_name,"global")) */
   }  /* for_each (hep, hosts) */




/*****************************************************************/
/* Calculate host tickets    */
   for_each (hep, hosts) { 
      host_name = lGetHost(hep, EH_name);
      if (strcmp(host_name,"global"))  {
         lSetUlong(hep, EH_sge_tickets, 0);
         host_sge_tickets = 0;
         for_each (running_job_elem, running_jobs) {
            for_each (rjq, lGetList(running_job_elem, JB_granted_destin_identifier_list)) {
               if (strcmp(lGetHost(rjq, JG_qhostname), host_name) == 0) {
                  host_sge_tickets += lGetUlong(running_job_elem, JB_ticket);
               }
            }
         }
         lSetUlong(hep, EH_sge_tickets, host_sge_tickets);
         total_host_tickets += host_sge_tickets;
      } /* if (strcmp(host_name,"global")) */
   }  /* for_each (hep, hosts) */

/* Calculate percentage of tickets for each host */

   for_each (hep, hosts) {  
      host_name = lGetHost(hep, EH_name);
      if (strcmp(host_name,"global"))  {
         if (total_host_tickets != 0)  {
            lSetDouble(hep, EH_sge_ticket_pct, (double)lGetUlong(hep, EH_sge_tickets)/(double)total_host_tickets);
         } else {
            lSetDouble(hep, EH_sge_ticket_pct, 0.0);
         }  /* (total_host_tickets != 0) */
      }   /* if (strcmp(host_name,"global"))  */
   }    /* for_each (hep, hosts) */




#ifdef TEST_CALC_HOST_USAGE
   fprintf(fpdchu, "host_list \n");
   lDumpList(fpdchu, hosts, 5);
   fclose(fpdchu);
#endif
   DEXIT;
   return 0;
}


#ifdef notdef
/* ----------------------------------------

   calculate_host_tickets()

   calculates the total number of tickets on a host from the
   JB_ticket field for jobs associated with the host


   returns:
      0 successful
     -1 errors in functions called by calculate_host_tickets

*/

int calculate_host_tickets(lList *jobs, lList *hosts)
{
char *host_name;
u_long host_sge_tickets;
lList *running_jobs;
lListElem *hep, *running_job_elem, *rjq;
   DENTER(TOP_LAYER, "calculate_host_tickets");

   if (!hosts) {
      DEXIT;
      return -1;
   }

  if (!running) {
      for_each (hep, *hosts) {
         lSetUlong(hep, EH_sge_tickets, 0);
      }
      DEXIT;
      return 0;
   }

   for_each (hep, hosts) {         
      host_name = lGetHost(hep, EH_name);
      lSetUlong(hep, EH_sge_tickets, 0);
      host_sge_tickets = 0;
      for_each (running_job_elem, *running) {
         for_each (rjq, lGetList(running_job_elem,
                                 JB_granted_destin_identifier_list)) {
            if (strcmp(lGetHost(rjq, JG_qhostname), host_name) == 0)
               host_sge_tickets += lGetUlong(running_job_elem, JB_ticket);
         }
      }
      lSetUlong(hep, EH_sge_tickets, host_sge_tickets);
      total_host_tickets += host_sge_tickets;

   }  /* for_each (hep, hosts) */
   for_each (hep, hosts) {       
     lSetDouble(hep, EH_sge_ticket_pct, double(lGetUlong(hep, EH_sge_tickets))/total_host_tickets);
   }   /* for_each (hep, hosts) */
   DEXIT;
   return 0;
}

#endif

/*******************************************************************/


/*
   calculate_host_pcts()

   calculates the resource_capability_factor_pct and the sge_load_pct
   on a host basis

   returns:
      0 successful
     -1 errors in functions called by calculate_host_pcts

*/

int calculate_host_pcts(lList *hosts, lList *centry)
{
   double total_resource_capability_factor=0, total_sge_load=0;
   lListElem *hep;
   char *host_name;

   DENTER(TOP_LAYER, "calculate_host_pcts");

   if (!hosts) {
      DEXIT;
      return -1;
   }


 
/*  Calculate the total_resource_capability_factor and the total_sge_load */

   for_each (hep, hosts)  {     
      host_name = lGetHost(hep, EH_name);
      if (strcmp(host_name,"global"))  {

      total_resource_capability_factor += lGetDouble(hep, EH_resource_capability_factor);


      total_sge_load += lGetDouble(hep, EH_sort_value);
      }  /*  if (strcmp(host_name,"global")) */
   }  /* for_each (hep, hosts) */

/*  Calculate the percentages of resource_capability_factor and sge load for each host */

   for_each (hep, hosts)  {     
      host_name = lGetHost(hep, EH_name);
      if (strcmp(host_name,"global"))  {
         if (total_resource_capability_factor != 0.0)  {
            lSetDouble(hep, EH_resource_capability_factor_pct, (lGetDouble(hep, EH_resource_capability_factor)/total_resource_capability_factor));
         }
         else  {
            lSetDouble(hep, EH_resource_capability_factor_pct, 0.0);
         }
         if (total_sge_load != 0.0)  {
            lSetDouble(hep, EH_sge_load_pct, (lGetDouble(hep, EH_sort_value)/total_sge_load));
         }
         else  {
            lSetDouble(hep, EH_sge_load_pct, 0.0);
         }
      } /* if (strcmp(host_name,"global")) */
   }  /* for_each (hep, hosts) */

   DEXIT;
   return 0;
}




double get_host_usage_value(lList *host_usage, char *name)
{
   lListElem *ue;
   double value = 0;
   if ((ue = lGetElemStr(host_usage, UA_name, name)))
      value = lGetDouble(ue, UA_value);
   return value;
}


typedef enum {
   ULONG_T=0,
   DATE_T,
   STRING_T,
   DOUBLE_T
} item_type_t;

typedef struct {
   char *name;
   item_type_t type;
   void *val;
} item_t;

static double host_mem, host_cpu, host_io, host_mem_pct, host_cpu_pct, host_io_pct, host_rcf, host_rcf_pct, host_tickets_pct, host_load, host_load_pct;
static lUlong current_time, host_running_jobs, host_tickets, host_share_load;
static char *host_name;

static item_t item[] = {
    { "curr_time", DATE_T, &current_time },
    { "host_name", STRING_T, &host_name },
    { "host_running_jobs", ULONG_T, &host_running_jobs },
    { "host_rcf", DOUBLE_T, &host_rcf },
    { "host_rcf_pct", DOUBLE_T, &host_rcf_pct },
    { "host_tickets", ULONG_T, &host_tickets },
    { "host_tickets_pct", DOUBLE_T, &host_tickets_pct },
    { "host_load", DOUBLE_T, &host_load },
    { "host_load_pct", DOUBLE_T, &host_load_pct },
    { "host_share_load", ULONG_T, &host_share_load },

    { "host_cpu", DOUBLE_T, &host_cpu },
    { "host_mem", DOUBLE_T, &host_mem },
    { "host_io", DOUBLE_T, &host_io },
    { "host_cpu_pct", DOUBLE_T, &host_cpu_pct },
    { "host_mem_pct", DOUBLE_T, &host_mem_pct },
    { "host_io_pct", DOUBLE_T, &host_io_pct },
};

static int items = sizeof(item) / sizeof(item_t);

int
print_host_field(FILE *out, item_t *item, format_t *format)
{
   if (format->name_format)
      fprintf(out, "%s=", item->name);

   switch(item->type) {
      case ULONG_T:
        fprintf(out, sge_u32, *(lUlong *)item->val);
        break;
      case DATE_T:
        {
           time_t t = *(lUlong *)item->val;
           if (t && format->format_times) {
              char *tc = strdup(ctime(&t));
              if (tc && *tc) {
                  tc[strlen(tc)-1] = 0;
                  fprintf(out, format->str_format, tc);
                  sge_free(&tc);
               }
           } else {
              fprintf(out, sge_u32, (u_long32) t);
           }
        }
        break;
      case DOUBLE_T:
        fprintf(out, "%f", *(double *)item->val);
        break;
      case STRING_T:
        fprintf(out, format->str_format, *(char **)item->val);
        break;
   }

   fprintf(out, "%s", format->delim);
   return 0;
}


int
print_host_hdr(FILE *out, format_t *format)
{
   int i;
   if (format->field_names) {
      char *field;
      char *fields = strdup(format->field_names);
      field = strtok(fields, ",");
      while (field) {
         for (i=0; i<items; i++) {
            if (strcmp(field, item[i].name)==0) {
               fprintf(out, "%s%s", item[i].name, format->delim);
               break;
            }
         }
         field = strtok(NULL, ",");
      }
      sge_free(&fields);
   } else {
      for (i=0; i<items; i++)
         fprintf(out, "%s%s", item[i].name, format->delim);
   }
   fprintf(out, "%s", format->line_delim);
   fprintf(out, "%s", format->rec_delim);

   return 0;
}

int
print_host(FILE *out, lListElem *host, char **names, format_t *format)
 {
   lList *host_usage, *host_usage_pct;
   int i, fields_printed=0;
   current_time = sge_get_gmt();
   host_name = lGetHost(host, EH_name);
   host_rcf = lGetDouble(host, EH_resource_capability_factor);
   host_rcf_pct = lGetDouble(host, EH_resource_capability_factor_pct);
   host_tickets = lGetUlong(host, EH_sge_tickets);
   host_tickets_pct = lGetDouble(host, EH_sge_ticket_pct);
   host_load = lGetDouble(host, EH_sort_value);
   host_load_pct = lGetDouble(host, EH_sge_load_pct);
   host_running_jobs = lGetUlong(host, EH_num_running_jobs);
   host_share_load = lGetUlong(host, EH_sge_load);

   if (host && (host_usage = lGetList(host, EH_scaled_usage_list))) {
      host_cpu = get_host_usage_value(host_usage, HOST_USAGE_ATTR_CPU);
      host_mem = get_host_usage_value(host_usage, HOST_USAGE_ATTR_MEM);
      host_io  = get_host_usage_value(host_usage, HOST_USAGE_ATTR_IO);
   } else {
      host_cpu = host_mem = host_io = 0;
   }

   if (host && (host_usage_pct = lGetList(host, EH_scaled_usage_pct_list))) {
      host_cpu_pct = get_host_usage_value(host_usage_pct, HOST_USAGE_ATTR_CPU);
      host_mem_pct = get_host_usage_value(host_usage_pct, HOST_USAGE_ATTR_MEM);
      host_io_pct  = get_host_usage_value(host_usage_pct, HOST_USAGE_ATTR_IO);
   } else {
      host_cpu_pct = host_mem_pct = host_io_pct = 0;
   }
#ifdef notdef 
/*            -- look at keeping host long term usage later if
                 time allows - will need to spool long term usage in the                 host_list_
*/
   if (user && (ltusage = lGetList(user, UP_long_term_usage))) {
      ltcpu = get_usage_value(ltusage, USAGE_ATTR_CPU);
      ltmem = get_usage_value(ltusage, USAGE_ATTR_MEM);
      ltio  = get_usage_value(ltusage, USAGE_ATTR_IO);
   } else {
      ltcpu = ltmem = ltio = 0;
   }
#endif

   if (names) {
      int found=0;
      char **name = names;
      while (*name) {
         if (strcmp(*name, host_name)==0)
            found = 1;
         name++;
      }
      if (!found)
         return 1;
   }
   
   if (format->field_names) {
      char *field;
      char *fields = strdup(format->field_names);
      field = strtok(fields, ",");
      while (field) {
         for (i=0; i<items; i++) {
            if (strcmp(field, item[i].name)==0) {
               print_host_field(out, &item[i], format);
               fields_printed++;
               break;
            }
         }
         field = strtok(NULL, ",");
      }
      sge_free(&fields);
   } else {
      for (i=0; i<items; i++)
         print_host_field(out, &item[i], format);
         fields_printed++;
   }

   if (fields_printed)
      fprintf(out, "%s", format->line_delim);

   return 0;
}


int
print_hosts(FILE *out, lList *hosts,  char **names, format_t *format)
{
lListElem *hep;
   for_each(hep, hosts) {
       print_host(out, hep, names, format);
   }

   return 0;
}


void host_usage(void)
{
   fprintf(stderr, "%s sge_host_mon [-cdfhilmnorsux] [host_names ...]\n\n" ,MSG_SGEHOSTMON_USAGE ); 
   fprintf(stderr, " -c count          %s\n", MSG_SGEHOSTMON_c_OPT_USAGE);
   fprintf(stderr, " -d delimiter      %s\n", MSG_SGEHOSTMON_d_OPT_USAGE);
   fprintf(stderr, " -f field[,field]  %s\n", MSG_SGEHOSTMON_f_OPT_USAGE);
   fprintf(stderr, " -h                %s\n", MSG_SGEHOSTMON_h_OPT_USAGE);
   fprintf(stderr, " -i interval       %s\n", MSG_SGEHOSTMON_i_OPT_USAGE);
   fprintf(stderr, " -l delimiter      %s\n", MSG_SGEHOSTMON_l_OPT_USAGE);
   fprintf(stderr, " -m output_mode    %s\n", MSG_SGEHOSTMON_m_OPT_USAGE);
   fprintf(stderr, " -n                %s\n", MSG_SGEHOSTMON_n_OPT_USAGE);
   fprintf(stderr, " -o output_file    %s\n", MSG_SGEHOSTMON_o_OPT_USAGE);
   fprintf(stderr, " -r delimiter      %s\n", MSG_SGEHOSTMON_r_OPT_USAGE);
   fprintf(stderr, " -s string_format  %s\n", MSG_SGEHOSTMON_s_OPT_USAGE);
   fprintf(stderr, " -t                %s\n", MSG_SGEHOSTMON_t_OPT_USAGE);
   fprintf(stderr, " -u                %s\n", MSG_SGEHOSTMON_u_OPT_USAGE);
   fprintf(stderr, " -x                %s\n", MSG_SGEHOSTMON_x_OPT_USAGE);
   fprintf(stderr,"\n");
}


int main(int argc, char **argv)
{
   lList *jobs, *hosts, *config, *centry;
   int interval=15;
   int err=0;
   int count=-1;
   int header=0;
   format_t format;
   char *ofile=NULL;
   char **names=NULL;
   int name_count=0;
   int decay_usage=0;
   int group_nodes=1;
   int c;
   extern char *optarg;
   extern int optind;
   FILE *outfile = stdout;
   char *output_mode = "w";
   
   extern sge_schedd_conf_type scheddconf;

   
   format.name_format=0;
   format.delim="\t";
   format.line_delim="\n";
   format.rec_delim="\n";
   format.str_format="%s";
   format.field_names=NULL;
   format.format_times=0;


#ifdef __SGE_COMPILE_WITH_GETTEXT__  
   /* init language output for gettext() , it will use the right language */
   sge_init_language_func((gettext_func_type)        gettext,
                         (setlocale_func_type)      setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type)     textdomain);
   sge_init_language(NULL,NULL);   
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */


   while ((c = getopt(argc, argv, "i:c:f:d:hl:m:no:r:s:tux")) != -1) {
      switch(c) {
         case 'i':
            if (sscanf(optarg, "%d", &interval) != 1) {
        	fprintf(stderr, MSG_ERROR_XISNOTAVALIDINTERVAL_S , optarg);
         fprintf(stderr, "\n");
        	err++;
            }
            break;
         case 'c':
            if (sscanf(optarg, "%d", &count) != 1) {
        	fprintf(stderr, MSG_ERROR_XISNOTAVALIDCOUNT_S , optarg);
         fprintf(stderr, "\n");
        	err++;
            }
            break;
         case 'f':
            format.field_names = strdup(optarg);
            break;
         case 'o':
            ofile = strdup(optarg);
            break;
         case 'd':
            format.delim = strdup(optarg);
            break;
         case 'h':
            header = 1;
            break;
         case 'm':
            output_mode = strdup(optarg);
            break;
         case 'n':
            format.name_format = 1;
            break;
         case 'l':
            format.line_delim = strdup(optarg);
            break;
         case 'r':
            format.rec_delim = strdup(optarg);
            break;
         case 's':
            format.str_format = strdup(optarg);
            break;
         case 't':
            format.format_times = 1;
            break;
         case 'u':
            decay_usage = 1;
            break;
         case 'x':
            group_nodes = 0;
            break;
         case '?':
            err++;
      }
   }

   if (err) {
      host_usage();
      exit(1);
   }

   if (ofile) {
      if ((outfile = fopen(ofile, output_mode)) == NULL) {
         fprintf(stderr, MSG_FILE_COULDNOTOPENXFORY_SS , ofile, output_mode);
         fprintf(stderr, "\n");
         exit(1);
      }
   }

   if ((name_count = argc - optind) > 0)
       names = &argv[optind];

   sge_gdi_setup("sge_host_mon", NULL);

   if (header)
      print_host_hdr(outfile, &format);

   while(count == -1 || count-- > 0) {

      setup_lists(&jobs, &hosts, &config, &centry);


/* Calculate usage and usage percentages on a host basis for cpu, mem,  and io.  Calculate host tickets and ticket percentages on a host basis.  Also calculate EH_num_running_jobs. */

      calculate_host_usage(&jobs, hosts); 

#ifdef notdef
/* Calculate tickets and ticket percentages on a host basis */

      calculate_host_tickets(jobs, hosts);
#endif
/* Calculate EH_sge_load_pct and EH_resource_capability_factor_pct on a host basis.  */ 
#ifdef notdef
      calculate_host_pcts(hosts, centry);
#endif

   switch (scheddconf.queue_sort_method) {
   case QSM_SEQNUM:
      sort_host_list(hosts, centry);
      break;
   case QSM_LOAD:
      sort_host_list(hosts, centry);
      break;
   case QSM_SHARE:
      sort_host_list(hosts, centry);
      sort_host_list_by_share_load(hosts, centry);
      break;
}
#ifdef notdef 
      if (decay_usage)
         curr_time = sge_get_gmt();

      _sge_calc_share_tree_proportions(sharetree,
        	lGetUlong(root, STN_type)==STT_PROJECT?NULL:users, 
        	lGetUlong(root, STN_type)==STT_PROJECT?users:NULL, 
        	config, NULL, curr_time);
#endif
      print_hosts(outfile, hosts, names, &format);


      free_lists(jobs, hosts, config);
      if (count) {
         fprintf(outfile, "%s", format.rec_delim);
         sleep(interval);
      }

      fflush(outfile);
   }
#ifdef notdef
      free_lists(jobs, hosts, config);
#endif
   sge_gdi_shutdown();

   return 0;
}
