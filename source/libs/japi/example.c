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
#include <unistd.h>
#include <string.h>

#include "drmaa.h"

#define JOB_CHUNK 8
#define NBULKS 3

static drmaa_job_template_t *create_job_template(const char *job_path, int seconds, int as_bulk_job);

int main(int argc, char *argv[])
{
   char diagnosis[DRMAA_ERROR_STRING_BUFFER];
   const char *all_jobids[NBULKS*JOB_CHUNK + JOB_CHUNK+1];
   char jobid[100];
   int drmaa_errno, i, pos = 0;
   const char *job_path;
   drmaa_job_template_t *jt;
  
   if (argc<2) {
      fprintf(stderr, "usage: example <path-to-job>\n");
      return 1;
   }
   job_path = argv[1];

   if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
      return 1;
   }

   /*
    *   submit some bulk jobs
    */
   if (!(jt = create_job_template(job_path, 5, 1))) {
      fprintf(stderr, "create_job_template() failed\n");
      return 1;
   }
   for (i=0; i<NBULKS; i++) {
      drmaa_job_ids_t *jobids;
      int j;

      while ((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, JOB_CHUNK, 1, diagnosis,
                 sizeof(diagnosis)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
         fprintf(stderr, "drmaa_run_bulk_jobs() failed - retry: %s %s\n", diagnosis, drmaa_strerror(drmaa_errno));
         sleep(1);
      } 
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_run_bulk_jobs() failed: %s %s\n", diagnosis, drmaa_strerror(drmaa_errno));
         return 1;
      }

      printf("submitted bulk job with jobids:\n");
      for (j=0; j<JOB_CHUNK; j++) {
         drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
         all_jobids[pos++] = strdup(jobid);
         printf("\t \"%s\"\n", jobid);
      } 
      drmaa_release_job_ids(jobids);
   }
   drmaa_delete_job_template(jt, NULL, 0);

   /*
    *   submit some sequential jobs
    */
   if (!(jt = create_job_template(job_path, 5, 0))) {
      fprintf(stderr, "create_sleeper_job_template() failed\n");
      return 1;
   }
   for (i=0; i<JOB_CHUNK; i++) {
      while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
               sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
         fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
         sleep(1);
      }
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
         return 1;
      }
      printf("\t \"%s\"\n", jobid);
      all_jobids[pos++] = strdup(jobid);
   }

   /* set string array end mark */
   all_jobids[pos] = NULL;

   drmaa_delete_job_template(jt, NULL, 0);

   /*
    *   synchronize with all jobs
    */
   drmaa_errno = drmaa_synchronize(all_jobids, DRMAA_TIMEOUT_WAIT_FOREVER, 0, diagnosis, sizeof(diagnosis)-1);
   if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
      return 1;
   }
   printf("synchronized with all jobs\n");

   /*
    *   wait all those jobs
    */
   for (pos=0; pos<NBULKS*JOB_CHUNK + JOB_CHUNK; pos++) {
      int stat;
      int aborted, exited, exit_status, signaled;

      drmaa_errno = drmaa_wait(all_jobids[pos], jobid, sizeof(jobid)-1, 
         &stat, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);

      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_wait(%s) failed: %s\n", all_jobids[pos], diagnosis);
         return 1;
      }

      /*
       * report how job finished 
       */
      drmaa_wifaborted(&aborted, stat, NULL, 0);
      if (aborted)
         printf("job \"%s\" never ran\n", all_jobids[pos]);
      else {
         drmaa_wifexited(&exited, stat, NULL, 0);
         if (exited) {
            drmaa_wexitstatus(&exit_status, stat, NULL, 0);
            printf("job \"%s\" finished regularly with exit status %d\n", 
                  all_jobids[pos], exit_status);
         } else {
            drmaa_wifsignaled(&signaled, stat, NULL, 0);
            if (signaled) {
               char termsig[DRMAA_SIGNAL_BUFFER+1];
               drmaa_wtermsig(termsig, DRMAA_SIGNAL_BUFFER, stat, NULL, 0);
               printf("job \"%s\" finished due to signal %s\n", 
                  all_jobids[pos], termsig);
            } else
               printf("job \"%s\" finished with unclear conditions\n", 
                  all_jobids[pos]);
         }
      }
   }

   if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
      return 1;
   }
   
  return 0;
}


static drmaa_job_template_t *create_job_template(const char *job_path, int seconds, int as_bulk_job)
{
   const char *job_argv[2];
   drmaa_job_template_t *jt = NULL;
   char buffer[100];

   if (drmaa_allocate_job_template(&jt, NULL, 0)!=DRMAA_ERRNO_SUCCESS)
      return NULL;

   /* run in users home directory */
   drmaa_set_attribute(jt, DRMAA_WD, DRMAA_PLACEHOLDER_HD, NULL, 0);

   /* the job to be run */
   drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, job_path, NULL, 0);

   /* the job's arguments */
   sprintf(buffer, "%d", seconds);
   job_argv[0] = buffer; 
   job_argv[1] = NULL;
   drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);

   /* join output/error file */
   drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);

   /* path for output */
   if (!as_bulk_job)
      drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":"DRMAA_PLACEHOLDER_HD"/DRMAA_JOB", NULL, 0);
   else
      drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":"DRMAA_PLACEHOLDER_HD"/DRMAA_JOB."DRMAA_PLACEHOLDER_INCR, NULL, 0);

   return jt;
}
