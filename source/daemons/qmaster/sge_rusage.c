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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdlib.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/sge_time.h"

#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_advance_reservation.h"

#include "sched/sge_job_schedd.h"

#include "sge_rusage.h"
#include "msg_qmaster.h"

#define ARCH_COLUMN ""

#define ACTFILE_FPRINTF_FORMAT \
"%s%c%s%c%s%c%s%c%s%c"sge_u32"%c%s%c"sge_u64"%c"sge_u32"%c"sge_u32"%c"sge_u32"%c"sge_u32"%c"sge_u32"%c" \
sge_u64"%c%f%c%f%c%f%c"sge_u64"%c"sge_u64"%c"sge_u64"%c"sge_u64"%c"sge_u64"%c"sge_u64"%c"sge_u64"%c%f%c" \
sge_u64"%c"sge_u64"%c"sge_u64"%c"sge_u64"%c"sge_u64"%c"sge_u64"%c%s%c%s%c%s%c%d%c"sge_u32"%c%f%c%f%c%f%c%s%c%f%c%s%c%f%c"sge_u32"%c"sge_u32"" \
ARCH_COLUMN \
"\n"

#define SET_STR_DEFAULT(jr, nm, s) if (!lGetString(jr, nm)) \
                                      lSetString(jr, nm, s);
#define SET_HOST_DEFAULT(jr, nm, s) if (!lGetHost(jr, nm)) \
                                      lSetHost(jr, nm, s);

/****** sge_rusage/reporting_get_ulong_usage() ********************************
*  NAME
*     reporting_get_ulong_usage() -- return usage of a certain attribute
*
*  SYNOPSIS
*     static u_long64
*     reporting_get_ulong_usage(const lList *usage_list, lList *reported_list, 
*                               const char *name, const char *rname, u_long32 def) 
*
*  FUNCTION
*     Return the usage information of a certain attribute (e.g. cpu, mem, ...).
*     If already usage had been reported for the same job (ja_task, pe_task),
*     do only report the newly added usage.
*     If no usage information is available for the given attribute, a default
*     value will be returned.
*
*     name and rname may differ, as already reported usage is taken from job 
*     online usage, e.g. attr USAGE_ATTR_CPU, whereas the final usage
*     is reported in the attr USAGE_ATTR_CPU_ACCT. When we report final usage,
*     we take the usage given by USAGE_ATTR_CPU_ACCT, but have to subtract 
*     already reported usage coming from online usage USAGE_ATTR_CPU.
*
*  INPUTS
*     const lList *usage_list - the usage (of a ja_task or pe_task)
*     lList *reported_list    - the already (earlier) reported usage
*     const char *name        - the name of the attribute
*     const char *rname       - the name of the attribute in the already
*                               reported usage
*     u_long32 def            - default value
*
*  RESULT
*     static u_long64 - the usage
*
*  NOTES
*     MT-NOTE: reporting_get_ulong_usage() is MT safe 
*
*  SEE ALSO
*     sgeobj/usage/usage_list_get_ulong_usage()
*******************************************************************************/
static u_long64
reporting_get_ulong_usage(const lList *usage_list, lList *reported_list,
                           const char *name, const char *rname, u_long32 def)
{
   /* total usage */
   u_long64 usage = usage_list_get_ulong_usage(usage_list, name, def);

   if (reported_list != NULL) {
      u_long64 reported;

      /* usage already reported */
      reported = usage_list_get_ulong_usage(reported_list, rname, def);

      /* after this action, we'll have reported the total usage */
      usage_list_set_ulong_usage(reported_list, rname, usage);

      /* in this intermediate accounting record, we'll report the usage 
       * consumed since the last intermediate accounting record.
       */
      usage -= reported;
   }

   return usage;
}

/****** sge_rusage/reporting_get_ulong_usage_sum() ****************************
*  NAME
*     reporting_get_ulong_usage_sum() -- return usage for a certain attribute
*
*  SYNOPSIS
*     static u_long64
*     reporting_get_ulong_usage_sum(const lList *usage_list, lList *reported_list,
*                                   bool accounting_summary, const lListElem *ja_task,
*                                   const char *name, const char *rname, u_long32 def)
*
*  FUNCTION
*     Return the usage information of a certain attribute (e.g. cpu, mem, ...).
*     If already usage had been reported for the same job (ja_task, pe_task),
*     do only report the newly added usage.
*     If no usage information is available for the given attribute, a default
*     value will be returned.
*     If accounting_summary is true, the usage of all pe_tasks in the given
*     ja_task object will be summed up as well.
*     If reported_usage is NULL, no usage will be booked as already reported,
*     e.g. for maximum values.
*
*     name and rname may differ, as already reported usage is taken from job 
*     online usage, e.g. attr USAGE_ATTR_CPU, whereas the final usage
*     is reported in the attr USAGE_ATTR_CPU_ACCT. When we report final usage,
*     we take the usage given by USAGE_ATTR_CPU_ACCT, but have to subtract 
*     already reported usage coming from online usage USAGE_ATTR_CPU.
*
*  INPUTS
*     const lList *usage_list  - the usage (of a ja_task or pe_task)
*     lList *reported_list     - the already (earlier) reported usage
*     bool accounting_summary  - shall we sum up pe_task usage?
*     const lListElem *ja_task - ja_task having pe_tasks
*     const char *name         - the name of the attribute
*     const char *rname        - the name of the attribute in the already
*                                reported usage
*     u_long32 def             - default value
*
*  RESULT
*     static u_long64 - the usage
*
*  NOTES
*     MT-NOTE: reporting_get_ulong_usage_sum() is MT safe 
*
*  SEE ALSO
*     sge_rusage/reporting_get_ulong_usage()
*******************************************************************************/
static u_long64
reporting_get_ulong_usage_sum(const lList *usage_list, lList *reported_list,
                               bool accounting_summary, const lListElem *ja_task,
                               const char *name, const char *rname, u_long32 def)
{
   u_long64 usage = reporting_get_ulong_usage(usage_list, reported_list, name, rname, def);

   /* when we do an accounting summary, we also have to sum up the pe task usage */
   if (accounting_summary) {
      lListElem *pe_task = NULL;
      lList *pe_tasks = lGetList(ja_task, JAT_task_list);

      for_each (pe_task, pe_tasks) {
         lList *pe_usage_list = lGetList(pe_task, PET_scaled_usage);
         if (pe_usage_list != NULL) {
            lList *pe_reported_list = NULL;
            if (reported_list != NULL) {
               pe_reported_list = lGetOrCreateList(pe_task, PET_reported_usage,
                                                   "reported_usage", UA_Type);
            }
            usage += reporting_get_ulong_usage(pe_usage_list, pe_reported_list, name, rname, def);
         }
      }
   }

   return usage;
}

/****** sge_rusage/reporting_get_double_usage() ********************************
*  NAME
*     reporting_get_double_usage() -- return usage of a certain attribute
*
*  SYNOPSIS
*     static double
*     reporting_get_double_usage(const lList *usage_list, lList *reported_list, 
*                                const char *name, const char *rname, double def) 
*
*  FUNCTION
*     Return the usage information of a certain attribute (e.g. cpu, mem, ...).
*     If already usage had been reported for the same job (ja_task, pe_task),
*     do only report the newly added usage.
*     If no usage information is available for the given attribute, a default
*     value will be returned.
*
*     name and rname may differ, as already reported usage is taken from job 
*     online usage, e.g. attr USAGE_ATTR_CPU, whereas the final usage
*     is reported in the attr USAGE_ATTR_CPU_ACCT. When we report final usage,
*     we take the usage given by USAGE_ATTR_CPU_ACCT, but have to subtract 
*     already reported usage coming from online usage USAGE_ATTR_CPU.
*
*  INPUTS
*     const lList *usage_list - the usage (of a ja_task or pe_task)
*     lList *reported_list    - the already (earlier) reported usage
*     const char *name        - the name of the attribute
*     const char *rname       - the name of the attribute in the already
*                               reported usage
*     double def              - default value
*
*  RESULT
*     static double - the usage
*
*  NOTES
*     MT-NOTE: reporting_get_double_usage() is MT safe 
*
*  SEE ALSO
*     sgeobj/usage/usage_list_get_double_usage()
*******************************************************************************/
static double
reporting_get_double_usage(const lList *usage_list, lList *reported_list,
                           const char *name, const char *rname, double def) 
{
   /* total usage */
   double usage = usage_list_get_double_usage(usage_list, name, def);

   if (reported_list != NULL) {
      double reported;

      /* usage already reported */
      reported = usage_list_get_double_usage(reported_list, rname, def);

      /* after this action, we'll have reported the total usage */
      usage_list_set_double_usage(reported_list, rname, usage);

      /* in this intermediate accounting record, we'll report the usage 
       * consumed since the last intermediate accounting record.
       */
      usage -= reported;
   }

   return usage;
}

/****** sge_rusage/reporting_get_double_usage_sum() ****************************
*  NAME
*     reporting_get_double_usage_sum() -- return usage for a certain attribute
*
*  SYNOPSIS
*     static double
*     reporting_get_double_usage_sum(const lList *usage_list, lList *reported_list,
*                                    bool accounting_summary, const lListElem *ja_task,
*                                    const char *name, const char *rname, double def)
*
*  FUNCTION
*     Return the usage information of a certain attribute (e.g. cpu, mem, ...).
*     If already usage had been reported for the same job (ja_task, pe_task),
*     do only report the newly added usage.
*     If no usage information is available for the given attribute, a default
*     value will be returned.
*     If accounting_summary is true, the usage of all pe_tasks in the given
*     ja_task object will be summed up as well.
*     If reported_usage is NULL, no usage will be booked as already reported,
*     e.g. for maximum values.
*
*     name and rname may differ, as already reported usage is taken from job 
*     online usage, e.g. attr USAGE_ATTR_CPU, whereas the final usage
*     is reported in the attr USAGE_ATTR_CPU_ACCT. When we report final usage,
*     we take the usage given by USAGE_ATTR_CPU_ACCT, but have to subtract 
*     already reported usage coming from online usage USAGE_ATTR_CPU.
*
*  INPUTS
*     const lList *usage_list  - the usage (of a ja_task or pe_task)
*     lList *reported_list     - the already (earlier) reported usage
*     bool accounting_summary  - shall we sum up pe_task usage?
*     const lListElem *ja_task - ja_task having pe_tasks
*     const char *name         - the name of the attribute
*     const char *rname        - the name of the attribute in the already
*                                reported usage
*     double def               - default value
*
*  RESULT
*     static double - the usage
*
*  NOTES
*     MT-NOTE: reporting_get_double_usage_sum() is MT safe 
*
*  SEE ALSO
*     sge_rusage/reporting_get_double_usage()
*******************************************************************************/
static double
reporting_get_double_usage_sum(const lList *usage_list, lList *reported_list,
                               bool accounting_summary, const lListElem *ja_task,
                               const char *name, const char *rname, double def)
{
   double usage = reporting_get_double_usage(usage_list, reported_list, name, rname, def);

   /* when we do an accounting summary, we also have to sum up the pe task usage */
   if (accounting_summary) {
      lListElem *pe_task = NULL;
      lList *pe_tasks = lGetList(ja_task, JAT_task_list);

      for_each (pe_task, pe_tasks) {
         lList *pe_usage_list = lGetList(pe_task, PET_scaled_usage);
         if (pe_usage_list != NULL) {
            lList *pe_reported_list = NULL;
            if (reported_list != NULL) {
               pe_reported_list = lGetOrCreateList(pe_task, PET_reported_usage,
                                                   "reported_usage", UA_Type);
            }
            usage += reporting_get_double_usage(pe_usage_list, pe_reported_list, name, rname, def);
         }
      }
   }

   return usage;
}

/* ------------------------------------------------------------

   write usage to a dstring buffer

   sge_write_rusage - write rusage info to a dstring buffer
   Returns: false, if it receives invalid data
            true on success

*/

static const char *
none_string(const char *str)
{
   const char *ret = str;

   if (str == NULL || strlen(str) == 0) {
      ret = NONE_STR;
   }

   return ret;
}

const char *
sge_write_rusage(dstring *buffer, 
                 lListElem *jr, lListElem *job, lListElem *ja_task, 
                 const char *category_str, const char delimiter, 
                 bool intermediate)
{
   lList *usage_list        = NULL; /* usage list of ja_task or pe_task */
   lList *reported_list     = NULL; /* already reported usage of ja_task or pe_task */

   const char *pe_task_id;
   const char *ret = NULL;
   char *qname = NULL;
   char *hostname = NULL;
   lListElem *pe_task = NULL;
   u_long32 submission_time = 0;
   u_long32 start_time      = 0;
   u_long32 end_time        = 0;
   u_long32 now             = sge_get_gmt();
   u_long32 ar_id           = 0;
   lListElem *ar = NULL;
   u_long32 exit_status     = 0;
   bool do_accounting_summary = false;

   DENTER(TOP_LAYER, "sge_write_rusage");

   /* invalid input data */
   if (buffer == NULL) {
      DRETURN(ret);
   }

   /* 
    * Figure out if it is a parallel job,
    * and if we shall write individual accounting entries or a summary.
    */
   if (lGetString(ja_task, JAT_granted_pe) != NULL) {
      object_description *object_base = object_type_get_object_description();
      const lListElem *pe = pe_list_locate(*object_base[SGE_TYPE_PE].list, 
                            lGetString(ja_task, JAT_granted_pe));
      do_accounting_summary = pe_do_accounting_summary(pe);
   }

   /*
    * Now figure out the usage lists:
    * - the pe task usage list for pe tasks
    * - the ja_task list for ja tasks
    */
   if ((pe_task_id = lGetString(jr, JR_pe_task_id_str)) != NULL) {
      /* nothing to be done for pe task, if summary is requested */
      if (do_accounting_summary) {
         DRETURN(ret);
      }

      /* try to find the pe_task */
      pe_task = lGetElemStr(lGetList(ja_task, JAT_task_list), PET_id, pe_task_id);
      if (pe_task == NULL) {
         dstring err_buffer = DSTRING_INIT;
         ERROR((SGE_EVENT, MSG_GOTUSAGEREPORTFORUNKNOWNPETASK_S,
                           job_get_id_string(lGetUlong(job, JB_job_number), 
                                             lGetUlong(ja_task, JAT_task_number), 
                                             pe_task_id, &err_buffer)));
         sge_dstring_free(&err_buffer);
         DRETURN(ret);
      }

      /* we output pe_task usage */
      usage_list = lGetList(pe_task, PET_scaled_usage);
   } else {
      /* we output ja_task usage */
      usage_list = lGetList(ja_task, JAT_scaled_usage_list);
   }

   /*
    * For intermediate records, we store the reported usage in 
    * ja_task or pe_task reported_list.
    */
   if (intermediate) {
      if (pe_task != NULL) {
         reported_list = lGetOrCreateList(pe_task, PET_reported_usage,
                                          "reported_usage", UA_Type);
      } else {
         reported_list = lGetOrCreateList(ja_task, JAT_reported_usage_list,
                                          "reported_usage", UA_Type);
      }

      /* 
       * The LAST_INTERMEDIATE timestamp of the previous intermediate
       * record is the start_time of the current interval.
       */
      start_time = usage_list_get_ulong_usage(reported_list, LAST_INTERMEDIATE, 0),

      /* now set actual time as time of last intermediate usage report */
      usage_list_set_ulong_usage(reported_list, LAST_INTERMEDIATE, now);
   }

   SET_STR_DEFAULT(jr, JR_queue_name, "UNKNOWN@UNKNOWN");

   /* job name and account get taken from local job structure */
   if (lGetString(job, JB_job_name) == NULL) {
      lSetString(job, JB_job_name, "UNKNOWN");
   }   
   if (lGetString(job, JB_account) == NULL) {
      lSetString(job, JB_account, "UNKNOWN");
   }   

   /* figure out queue name and host name */
   {
      dstring cqueue = DSTRING_INIT;
      dstring hname = DSTRING_INIT;

      cqueue_name_split(lGetString(jr, JR_queue_name), &cqueue, &hname, NULL, 
         NULL);

      qname = strdup(sge_dstring_get_string(&cqueue));
      hostname = strdup(sge_dstring_get_string(&hname));

      sge_dstring_free(&cqueue);
      sge_dstring_free(&hname);
   }

   /* get submission_time, start_time, end_time */
   end_time = usage_list_get_ulong_usage(usage_list, "end_time", 0);
   submission_time = usage_list_get_ulong_usage(usage_list, "submission_time", 0);

   if (intermediate) {
      /*
       * for the job, we don't have the submission time in the job report 
       * before job exit 
       */
      if (job != NULL && pe_task == NULL) {
         submission_time = lGetUlong(job, JB_submission_time);
      }
      /* 
       * For the first intermediate record, the start_time is the ja_task start time.
       * For consequent intermediate records, we already set the start_time to the
       * previous intermediate record's end time.
       */
      if (start_time == 0 && ja_task != NULL) {
         start_time = lGetUlong(ja_task, JAT_start_time);
      }

      /*
       * For still running jobs, the job report from execd does not yet contain
       * the end_time.
       * So this is *not* the final usage record, and we use now as end_time.
       */
      if (end_time == 0) {
         end_time = now;
      }

      /*
       * While the job is still running, we don't get an exit_status reported
       * by sge_execd.
       * In this case set exit_status to -1, meaning in ARCo: Job still running.
       * See CR 6621482.
       */
      exit_status = usage_list_get_ulong_usage(usage_list, "exit_status", -1);
   } else {
      start_time = usage_list_get_ulong_usage(usage_list, "start_time", 0);
      exit_status = usage_list_get_ulong_usage(usage_list, "exit_status", 0);
   }

   ar_id = lGetUlong(job, JB_ar);
   if (ar_id != 0) {
      object_description *object_base = object_type_get_object_description();
      ar = ar_list_locate(*object_base[SGE_TYPE_AR].list, ar_id);
   }
   
   /*
    * Output all the usage information.
    * For cpu, mem, io, iow we have to take into account,
    * that usage might already have been reported in intermediate accounting
    * records.
    * All the ru_* attributes are only reported at process end,
    * see man page getrusage(2), so nothing to be done for intermediate
    * records.
    */
   ret = sge_dstring_sprintf(buffer, ACTFILE_FPRINTF_FORMAT, 
          qname, delimiter,
          hostname, delimiter,
          lGetString(job, JB_group), delimiter,
          lGetString(job, JB_owner), delimiter,
          lGetString(job, JB_job_name), delimiter,
          lGetUlong(jr, JR_job_number), delimiter,
          lGetString(job, JB_account), delimiter,
          usage_list_get_ulong_usage(usage_list, "priority", 0),  delimiter,
          submission_time, delimiter,
          start_time, delimiter,
          end_time, delimiter,
          lGetUlong(jr, JR_failed), delimiter,
          exit_status, delimiter,
          usage_list_get_ulong_usage(usage_list, "ru_wallclock", 0), delimiter,
          reporting_get_double_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_utime", "ru_utime", 0), delimiter,
          reporting_get_double_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_stime", "ru_stime", 0), delimiter,
          reporting_get_double_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_maxrss", "ru_maxrss", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_ixrss", "ru_ixrss", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_ismrss", "ru_ismrss", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_idrss", "ru_idrss", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_isrss", "ru_isrss", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_minflt", "ru_minflt", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_majflt", "ru_majflt", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_nswap", "ru_nswap", 0), delimiter,
          reporting_get_double_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_inblock", "ru_inblock", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_oublock", "ru_oublock", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_msgsnd", "ru_msgsnd", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_msgrcv", "ru_msgrcv", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_nsignals", "ru_nsignals", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_nvcsw", "ru_nvcsw", 0), delimiter,
          reporting_get_ulong_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task, "ru_nivcsw", "ru_nivcsw", 0), delimiter,
          none_string(lGetString(job, JB_project)), delimiter,
          none_string(lGetString(job, JB_department)), delimiter,
          none_string(lGetString(ja_task, JAT_granted_pe)), delimiter,
          sge_granted_slots(lGetList(ja_task, JAT_granted_destin_identifier_list)), delimiter,
          job_is_array(job) ? lGetUlong(ja_task, JAT_task_number) : 0, delimiter,
          reporting_get_double_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task,
                                         intermediate ? USAGE_ATTR_CPU : USAGE_ATTR_CPU_ACCT, USAGE_ATTR_CPU, 0), delimiter,
          reporting_get_double_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task,
                                         intermediate ? USAGE_ATTR_MEM : USAGE_ATTR_MEM_ACCT, USAGE_ATTR_MEM, 0), delimiter,
          reporting_get_double_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task,
                                         intermediate ? USAGE_ATTR_IO : USAGE_ATTR_IO_ACCT, USAGE_ATTR_IO, 0), delimiter,
          none_string(category_str), delimiter,
          reporting_get_double_usage_sum(usage_list, reported_list, do_accounting_summary, ja_task,
                                         intermediate ? USAGE_ATTR_IOW : USAGE_ATTR_IOW_ACCT, USAGE_ATTR_IOW, 0), delimiter,
          none_string(pe_task_id), delimiter,
          reporting_get_double_usage_sum(usage_list, NULL, do_accounting_summary, ja_task,
                                         intermediate ? USAGE_ATTR_MAXVMEM : USAGE_ATTR_MAXVMEM_ACCT, USAGE_ATTR_MAXVMEM, 0), delimiter,
          lGetUlong(job, JB_ar), delimiter,
          (ar != NULL) ? lGetUlong(ar, AR_submission_time): 0
             );

   sge_free(&qname);
   sge_free(&hostname);

   DRETURN(ret);
}

