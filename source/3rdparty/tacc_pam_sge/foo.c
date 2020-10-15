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
#define MAX_SLEEP_DEFAULT 0 /* Max sleep default (micro-secs) */
#define EXECD_SPOOL_DIR_DEFAULT "/share/sge6.2/execd_spool"
#define MAX_BYPASS_USERS 30
#define BYPASS_USER_STRING_LENGTH 512

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <ctype.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <unistd.h>

#include "basis_types.h"
#include "uti/sge_rmon.h"
#include "symbols.h"
#include "sge.h"
#include "sge_gdi.h"
#include "sge_time.h"
#include "sge_log.h"
#include "sge_stdlib.h"
#include "sge_all_listsL.h"
#include "commlib.h"
#include "sge_host.h"
#include "sig_handlers.h"
#include "sge_sched.h"
#include "cull_sort.h"
#include "usage.h"
#include "sge_dstring.h"
#include "sge_feature.h"
#include "parse.h"
#include "sge_prog.h"
#include "sge_parse_num_par.h"
#include "sge_string.h"
#include "show_job.h"
#include "qstat_printing.h"
#include "sge_range.h"
#include "sge_schedd_text.h"
#include "qm_name.h"
#include "load_correction.h"
#include "msg_common.h"
#include "msg_clients_common.h"
/*#include "msg_qstat.h"*/
#include "sge_conf.h" 
#include "sgeee.h" 
#include "sge_support.h"
#include "sge_unistd.h"
#include "sge_answer.h"
#include "sge_pe.h"
#include "sge_str.h"
#include "sge_ckpt.h"
#include "sge_qinstance.h"
#include "sge_qinstance_state.h"
#include "sge_centry.h"
#include "sge_schedd_conf.h"
#include "sge_cqueue.h"
#include "sge_cqueue_qstat.h"
#include "sge_qref.h"

#include "sge_mt_init.h"
#include "read_defaults.h"
#include "setup_path.h"
#include "sgeobj/sge_ulong.h"
/*#include "gdi/sge_gdi_ctx.h"
#include "sge_qstat.h"
*/
/*#include "qstat_xml.h"*/
/*#include "qstat_cmdline.h"*/
#include "cull_file.h"
#include "uti/sge_profiling.h"

#include "uti/sge_spool.h"


#define PAM_DEBUG 1

int check(int poop)
{
  int retval;
  char user[256];
  char hostname[256];
  char seed_string[256];
  int  namelength=255;
  unsigned int myseed=0;
  unsigned long local_sleep;
  unsigned long max_sleep=0;
  float normalized_rndm;
  int i;
  int ii;
#ifdef PAM_DEBUG 
  FILE *fpd;
#endif
  FILE *fp;
  DIR *dp;
  struct dirent *ep;
  char *my_task_id;
  char *host;
  const char *user_sge;
   
  int default_max_sleep=1;
  int default_execd_spool_dir=1;
  
  char execd_spool_dir[256]="";
  
/* sge specific variables */
  static lListElem *my_job = NULL;
     
  u_long32 my_job_id;
  u_long32 junk=1;
  sge_spool_flags_t my_flags=SPOOL_DEFAULT;
  char spool_path[256] = "";
   
  char *bypass_user_list[MAX_BYPASS_USERS];
  /*  char bypass_user_buffer[BYPASS_USER_STRING_LENGTH];*/
  /*char *bup; */
  int bypass_user_count=0;
  
  char my_active_jobs_directory[256];
  char my_active_job[256];
  char *strptr;
  char host_file[256]="";
   
  bootstrap_mt_init();
  /* hopefully I can get rid of this */ 
  sge_prof_set_enabled(false);
  
/*   /\* Grab the user & host  *\/ */
/*   retval = pam_get_user(pamh,&user,NULL); */
/*   if (retval != PAM_SUCCESS)  */
/*     return PAM_SUCCESS; */

/*   if (user == NULL || *user == '\0') */
/*     return PAM_SUCCESS; */

  strcpy(user,"bbarth");

  gethostname(hostname,namelength);

#ifdef PAM_DEBUG 
  fpd = fopen("/tmp/pam_sge_debug.out","a");
  fprintf(fpd,"Hostname is: %s\n",hostname);
  fprintf(fpd,"User is:     %s\n",user);
  fflush(fpd);
#endif

   /* immediately bail out if root or special people are knocking */
  if( (strcmp(user,"root") == 0) )
    {
#ifdef PAM_DEBUG 
      fprintf(fpd,"Allowing root access...\n");
      fclose(fpd);
#endif
      return PAM_SUCCESS;
    }
        
   /* read interesting parameters from the /etc/pam.d/sshd file */
   /* note: a couple of other possible params are:
      sge_qmaster_port (set in sge build, usually 536)
      sge_execd_port (set in sge build, usually 537) */
      
/*   for(i=0;i<argc;i++) { */
/*      if(strncmp(argv[i],"max_sleep=",10)==0) { */
/*        max_sleep=atoi(argv[i]+10); */
/*        default_max_sleep=0; */
/*      } */
/*      if(strncmp(argv[i],"execd_spool_dir=",16)==0) { */
/*        strcpy(execd_spool_dir,argv[i]+16); */
/*        default_execd_spool_dir=0; */
/*      } */
/*      if(strncmp(argv[i],"bypass_users=",13)==0) { */
/*        bypass_user_count=0; */
/*        strcpy(bypass_user_buffer,argv[i]+13); */
/*        bup=strtok(bypass_user_buffer,","); */
/*        if (bup) { */
/*          bypass_user_list[bypass_user_count]=bup; */
/*          bypass_user_count++; */
/*          while ((bup=strtok(NULL,","))) { */
/*              bypass_user_list[bypass_user_count]=bup; */
/* 	     bypass_user_count++; */
/*          } */
/*        } */
/*      } */
/*   } */

  strcpy(execd_spool_dir,"/share/sge6.2/execd_spool");
  
  /* scan the bypass list right away and return success if the user is in it */
  
#ifdef PAM_DEBUG 
  fprintf(fpd,"checking bypass list...\n");
  fflush(fpd);
#endif

  for (i=0;i<bypass_user_count;i++) {
       if(strcmp(bypass_user_list[i],user)==0) {
#ifdef PAM_DEBUG 
          fprintf(fpd,"Allowing access to user in bypass list...\n");
          fclose(fpd);
#endif
          return PAM_SUCCESS;
       }
  }

  
  /* handle specification/non-specification of variables in the /etc/pam.d/sshd file*/    
  if (default_max_sleep) {
     max_sleep=MAX_SLEEP_DEFAULT;
     }
     
  if (default_execd_spool_dir) {
     strcpy(execd_spool_dir,EXECD_SPOOL_DIR_DEFAULT);
     }
     

#ifdef PAM_DEBUG 
      fprintf(fpd,"SGE execd spool dir set to...\n");
      fprintf(fpd,"execd_spool_dir = %s\n",execd_spool_dir);
      fprintf(fpd,"Max sleep time set to (micro sec)...\n");
      fprintf(fpd,"max_sleep = %ld\n",max_sleep);
      fflush(fpd);
#endif
 
  /*-------------------------------------------------------------------------
   * Desynchronize the sge execd spool directory accesses - 
   * by default, this is off, but it might come in useful, so I left it in (SGJ)
   *-------------------------------------------------------------------------*/
   
   if (max_sleep>0) {
      ii=0;
      for(i=0;i< (int) strlen(hostname);i++) {
        if (isdigit(hostname[i])){
         seed_string[ii]=hostname[i];
         ii++;
        }
      }
      seed_string[ii]='\0';

      myseed = (unsigned int) atoi(seed_string);

      srand(myseed);
      normalized_rndm = (float)rand()/RAND_MAX;
      local_sleep     =  normalized_rndm*max_sleep;

      usleep(local_sleep);
   }

#ifdef PAM_DEBUG 
   fprintf(fpd,"checking the execd spool directory for active jobs and job data ...\n");
   fflush(fpd);
#endif

   /* get rid of the domain name from the pam call */
   host=strtok(hostname,".");
   
   /* build the active_dir string */
   sprintf(my_active_jobs_directory,"%s/%s/active_jobs",execd_spool_dir,host);
   
   /* if the directory exists, scan the subdirectories as job names */
   dp =opendir(my_active_jobs_directory);
   if (dp !=NULL) {
       while ((ep=readdir(dp))){
             if (strncmp(".",ep->d_name,1)!=0) {
#ifdef PAM_DEBUG 
                fprintf(fpd,"processing active job: %s\n",my_active_job);
                fflush(fpd);
#endif
                strcpy(my_active_job,ep->d_name);
                strptr=strtok(my_active_job,".");
                my_job_id=atoi(strptr);
                my_task_id=strtok(NULL,".");

#ifdef PAM_DEBUG 
                fprintf(fpd,"using SGE tools to build path for job: %d, task: %s ...\n",my_job_id,my_task_id);
                fflush(fpd);
#endif

                sge_get_file_path(spool_path, JOB_SPOOL_DIR, FORMAT_DEFAULT,my_flags, my_job_id, junk, NULL);
                sprintf(host_file,"%s/%s/%s.%s",execd_spool_dir,host,spool_path,my_task_id);

#ifdef PAM_DEBUG 
                fprintf(fpd,"sge tools returned: %s ...\n",spool_path);
                fprintf(fpd,"fixed up to: %s ...\n",host_file);
                fprintf(fpd,"checking if this file exists ...\n");
                fflush(fpd);
#endif

                if ((fp=fopen(host_file,"r"))) {
                   fclose(fp);
#ifdef PAM_DEBUG 
                   fprintf(fpd,"file exists. using SGE tools to load job data...\n");
                   fflush(fpd);
#endif
		   
                   my_job = lReadElemFromDisk(NULL, host_file, JB_Type, "job");
		   

#ifdef PAM_DEBUG 
		   if(!my_job) fprintf(fpd,"my_job: %p\n",my_job);
                   fprintf(fpd,"job data loaded, checking job owner...\n");
                   fflush(fpd);
		   retval=0;
#endif
                   if ((retval=lGetPosViaElem(my_job, JB_owner, SGE_NO_ABORT))>=0) {
                      if ((user_sge=lGetString(my_job, JB_owner))){
#ifdef PAM_DEBUG 
                         fprintf(fpd,"job owner is: %s\n",user_sge);
                         fflush(fpd);
#endif		      
                         if(strcmp(user,user_sge)==0) {
#ifdef PAM_DEBUG 
                           fprintf(fpd,"PAM_SUCCESS !\n");
                           fflush(fpd);
#endif		      
                           lFreeElem(&my_job);
		           return PAM_SUCCESS;
	                 }
                      }
                   }
		   else {
#ifdef PAM_DEBUG 
		     fprintf(fpd,"Failed to GetPosViaElem w/ val %d\n",retval);
		     fflush(fpd);
#endif		      
		   }
                   lFreeElem(&my_job);
                 }

              }
       } 
       closedir(dp);
   } else {
#ifdef PAM_DEBUG 
                fprintf(fpd,"no active jobs directory present: %s\n", my_active_jobs_directory);
                fflush(fpd);
#endif
   }
   
#ifdef PAM_DEBUG 
   fprintf(fpd,"PAM_ACCT_EXPIRED returned - no valid user matches found...\n");
   fflush(fpd);
#endif
   return PAM_ACCT_EXPIRED;
	   
}


int main(int argc, char **argv)
{
  check(0);
  return 0;
}
