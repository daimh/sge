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

/* fixme: drop into cgroups of (arbitrary choice of) user's job */

#define MAX_SLEEP_DEFAULT 0 /* Max sleep default (micro-secs) */
#define EXECD_SPOOL_DIR_DEFAULT "/opt/sge/default/spool"
#define MAX_BYPASS_USERS 30
#define BYPASS_USER_STRING_LENGTH 512

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#define SERVICENAME "pam_sge_authorize"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <ctype.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <syslog.h>

#include "sgeobj/sge_all_listsL.h"
#include "sge_bootstrap.h"
#include "cull_file.h"
#include "uti/sge_spool.h"

/* --- authentication management functions --- */

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh _UNUSED, int flags _UNUSED,
                                   int argc _UNUSED, const char **argv _UNUSED)
{
  return PAM_SUCCESS;
}


PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh _UNUSED, int flags _UNUSED,
                              int argc _UNUSED, const char **argv _UNUSED)
{
  return PAM_SUCCESS;
}


/* --- password management --- */


PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh _UNUSED, int flags _UNUSED,
                                int argc _UNUSED, const char **argv _UNUSED)
{
  return PAM_SUCCESS;
}


/* --- session management --- */

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh _UNUSED,
                                   int flags _UNUSED, int argc _UNUSED,
                                   const char **argv _UNUSED)
{
  return PAM_SUCCESS;
}



PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh _UNUSED, int flags _UNUSED,
                                    int argc _UNUSED, const char **argv _UNUSED)
{
  return PAM_SUCCESS;
}


/* Adapted from pam_sge-qrsh-setup */
static void pam_sge_log(int priority, const char *msg, ...)
{
  char buf[512];
  va_list plist;
  va_start(plist, msg);
  vsnprintf(buf, sizeof(buf), msg, plist);
  va_end(plist);
  openlog(SERVICENAME, LOG_PID | LOG_CONS | LOG_NOWAIT, LOG_AUTH);
  syslog(priority, "%s", buf);
  closelog();
}


PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh,int flags _UNUSED,int argc,
                                const char **argv)
{
  int retval;
  const char *user=NULL;
  char hostname[256];
  char seed_string[256];
  int  namelength=255;
  unsigned int myseed=0;
  unsigned long local_sleep;
  unsigned long max_sleep=0;
  float normalized_rndm;
  int i;
  int ii;
  int debug=0;
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
  char bypass_user_buffer[BYPASS_USER_STRING_LENGTH];
  char *bup;
  int bypass_user_count=0;
  
  char my_active_jobs_directory[256];
  char my_active_job[256];
  char *strptr;
  char host_file[256]="";

  bootstrap_mt_init();
  
  /* Grab the user & host  */
  retval = pam_get_user(pamh,&user,NULL);
  if (retval != PAM_SUCCESS) 
    return PAM_SUCCESS;

  if (user == NULL || *user == '\0')
    return PAM_SUCCESS;

  gethostname(hostname,namelength);
        
   /* read interesting parameters from the /etc/pam.d/sshd file */
   /* note: a couple of other possible params are:
      sge_qmaster_port (set in sge build, usually 536)
      sge_execd_port (set in sge build, usually 537) */
      
  for(i=0;i<argc;i++) {
     if(strncmp(argv[i],"max_sleep=",10)==0) {
       max_sleep=atoi(argv[i]+10);
       default_max_sleep=0;
     }
     if(strncmp(argv[i],"execd_spool_dir=",16)==0) {
       strcpy(execd_spool_dir,argv[i]+16);
       default_execd_spool_dir=0;
     }
     if(strncmp(argv[i],"bypass_users=",13)==0) {
       bypass_user_count=0;
       strcpy(bypass_user_buffer,argv[i]+13);
       bup=strtok(bypass_user_buffer,",");
       if (bup) {
         bypass_user_list[bypass_user_count]=bup;
         bypass_user_count++;
         while ((bup=strtok(NULL,","))) {
             bypass_user_list[bypass_user_count]=bup;
	     bypass_user_count++;
         }
       }
     }
     if (strncmp(argv[i], "debug", 5) == 0) {
       debug=1;
       pam_sge_log(LOG_DEBUG, "Hostname is: %s", hostname);
       pam_sge_log(LOG_DEBUG, "User is:     %s", user);
     }
  }

   /* immediately bail out if root or special people are knocking */
  if( (strcmp(user,"root") == 0) )
    {
      if (debug) pam_sge_log(LOG_DEBUG, "Allowing root access...");
      return PAM_SUCCESS;
    }
  
  /* scan the bypass list right away and return success if the user is in it */
  
  if (debug) pam_sge_log(LOG_DEBUG, "checking bypass list...");

  for (i=0;i<bypass_user_count;i++) {
       if(strcmp(bypass_user_list[i],user)==0) {
         if (debug)
           pam_sge_log(LOG_DEBUG, "Allowing access to user in bypass list...");
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
     

  if (debug) {
      pam_sge_log(LOG_DEBUG, "SGE execd spool dir set to...");
      pam_sge_log(LOG_DEBUG, "execd_spool_dir = %s", execd_spool_dir);
      pam_sge_log(LOG_DEBUG, "Max sleep time set to (micro sec)...");
      pam_sge_log(LOG_DEBUG, "max_sleep = %ld", max_sleep);
  }
 
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

   if (debug)
     pam_sge_log(LOG_DEBUG,
                 "checking execd spool directory for active jobs and job data...");

   /* get rid of the domain name from the pam call */
   host=strtok(hostname,".");
   
   /* build the active_dir string */
   sprintf(my_active_jobs_directory,"%s/%s/active_jobs",execd_spool_dir,host);
   
   /* if the directory exists, scan the subdirectories as job names */
   dp =opendir(my_active_jobs_directory);
   if (dp !=NULL) {
       while ((ep=readdir(dp))){
             if (strncmp(".",ep->d_name,1)!=0) {
                strcpy(my_active_job,ep->d_name);
                if (debug)
                  pam_sge_log(LOG_DEBUG, "processing active job: %s",
                              my_active_job);
                strptr=strtok(my_active_job,".");
                my_job_id=atoi(strptr);
                my_task_id=strtok(NULL,".");

                pam_sge_log(LOG_DEBUG,
                            "using SGE tools to build path for job: %d, task: %s...",
                            my_job_id,my_task_id);

                sge_get_file_path(spool_path, JOB_SPOOL_DIR, SPOOL_DEFAULT,my_flags, my_job_id, junk, NULL);
                sprintf(host_file,"%s/%s/%s.%s",execd_spool_dir,host,spool_path,my_task_id);

                if (debug) {
                  pam_sge_log(LOG_DEBUG, "sge tools returned: %s ...",
                              spool_path);
                  pam_sge_log(LOG_DEBUG, "fixed up to: %s ...", host_file);
                  pam_sge_log(LOG_DEBUG, "checking if this file exists ...");
                }

                if ((fp=fopen(host_file,"r"))) {
                   fclose(fp);
                   if (debug)
                     pam_sge_log(LOG_DEBUG,
                                 "file exists. using SGE tools to load job data...");
		   
                   my_job = lReadElemFromDisk(NULL, host_file, JB_Type, "job");
		   

                   if (debug) {
                     if(!my_job) pam_sge_log(LOG_DEBUG, "my_job: %p",
                                             my_job);
                     pam_sge_log(LOG_DEBUG,
                                 "job data loaded, checking job owner...");
                   }
                   if ((retval=lGetPosViaElem(my_job, JB_owner, SGE_NO_ABORT))>=0) {
                      if ((user_sge=lGetString(my_job, JB_owner))){
                        if (debug)
                          pam_sge_log(LOG_DEBUG, "job owner is: %s",
                                      user_sge);
                        if(strcmp(user,user_sge)==0) {
                          if (debug)
                            pam_sge_log(LOG_DEBUG, "PAM_SUCCESS !");
                          /* fixme: do the same as pam_sge-qrsh-setup
                             to attach this session to a job.  */
                          lFreeElem(&my_job);
                          return PAM_SUCCESS;
                        }
                      }
                   }
		   else {
                     if (debug)
                       pam_sge_log(LOG_DEBUG,
                                   "Failed to GetPosViaElem w/ val %d", retval);
		   }
                   lFreeElem(&my_job);
                 }

              }
       } 
       closedir(dp);
   } else {
     if (debug)
       pam_sge_log(LOG_DEBUG, "no active jobs directory present: %s",
                   my_active_jobs_directory);
   }
   
   if (debug)
   pam_sge_log(LOG_DEBUG,
               "PAM_ACCT_EXPIRED returned - no valid user matches found...");
   return PAM_ACCT_EXPIRED;
	   
}



