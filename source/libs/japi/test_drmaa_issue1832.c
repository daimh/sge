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
 *   The Initial Developer of the Original Code is: Rising Sun Pictures
 * 
 *   Copyright: 2001 by Rising Sun Pictures
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "drmaa.h"

/*
 * This test validates the required DRMAA functionality for the -hold_jid_ad option.
 */

#define BULK_SIZE 12

int do_ps = 0;  /* do drmaa_job_ps() for progress bar information */
int do_1st = 0; /* do first scenario only */
int do_exit = 1; /* treat it as error when exit status can not be determined in all cases */

static char errorbuf[DRMAA_ERROR_STRING_BUFFER];

static
drmaa_job_template_t *create_job_template(const char *job_path, const char *job_name, const char *pred_name, int seconds, int hold);

static void ids_count_status(char **ids, int size, int chunks, int *hold, int *pending, int *running, int *done)
{
   int j, drmaa_errno, status;

   for (j = 0; j < size; j++) {

      if (!ids[j]) {
         *done += chunks;
         continue; /* drmaa_job_ps() won't work for already reaped jobs */
      }
      while ((drmaa_errno = drmaa_job_ps(ids[j], &status, errorbuf, sizeof(errorbuf)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
         sleep(1);

      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_job_ps failed: %s\n", errorbuf);
         exit(1);
      }

      switch (status) {
      case DRMAA_PS_SYSTEM_ON_HOLD:
      case DRMAA_PS_USER_ON_HOLD:
      case DRMAA_PS_USER_SYSTEM_ON_HOLD:
          *hold += chunks;
          break;

      case DRMAA_PS_QUEUED_ACTIVE:
          *pending += chunks;
          break;

      case DRMAA_PS_RUNNING:
          *running += chunks;
          break;

      case DRMAA_PS_DONE:
      case DRMAA_PS_UNDETERMINED:
          *done += chunks;
          break;

      default:
          break;
      }
   }

   return;
}

static int ids_remove_id(char **ids, int size, const char *id)
{
   int j, found = 0;

   for (j = 0; j < size; j++) {
      if (ids[j] && !strcmp(id, ids[j])) {
         free(ids[j]);
         found = 1;
         break;
      }
   }
   return found;
}

static void state_monitor(char **ids_a, int chunks_a, char **ids_b, int chunks_b)
{
   int j, status, drmaa_errno;
   int hold = 0, pending = 0, running = 0, done = 0;

   /* wait jobs as to verify they run orderly */
   do {
      char jobid[512];

      drmaa_errno = drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY, jobid, sizeof(jobid)-1, &status, 0, NULL, errorbuf, sizeof(errorbuf)-1);

      if (drmaa_errno==DRMAA_ERRNO_INVALID_JOB) {
         break; /* nothing more to wait for */
      }
      
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS && drmaa_errno != DRMAA_ERRNO_EXIT_TIMEOUT) {
         fprintf(stderr, "drmaa_wait() failed: %s\n", errorbuf);
         exit(1);
      }
      if (drmaa_errno == DRMAA_ERRNO_SUCCESS) {
         int exited, exit_status;

         /* mark the job as done and reaped */
         if (!ids_remove_id(ids_a, BULK_SIZE/chunks_a, jobid))
            ids_remove_id(ids_b, BULK_SIZE/chunks_b, jobid);

         if (do_exit) {
            if (drmaa_wifexited(&exited, status, NULL, 0) != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wifexited(\"%s\") failed\n", jobid);
               exit(1);
            }
            if (!exited) {
               fprintf(stderr, "job \"%s\" didn't exit orderly\n", jobid);
               exit(1);
            }
            if (drmaa_wexitstatus(&exit_status, status, NULL, 0) != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wexitstatus(\"%s\") failed\n", jobid);
               exit(1);
            }
            if (exit_status != 0) {
               fprintf(stderr, "job \"%s\" exit with %d\n", jobid, exit_status);
               exit(1);
            }
         }
      }
   } while (drmaa_errno != DRMAA_ERRNO_EXIT_TIMEOUT);

   if (do_ps) {
      ids_count_status(ids_a, BULK_SIZE/chunks_a, chunks_a, &hold, &pending, &running, &done);
      ids_count_status(ids_b, BULK_SIZE/chunks_b, chunks_b, &hold, &pending, &running, &done);
   } else {
      for (j = 0; j < BULK_SIZE/chunks_a; j++) 
         if (!ids_a[j]) done += chunks_a;
      for (j = 0; j < BULK_SIZE/chunks_b; j++) 
         if (!ids_b[j]) done += chunks_b;
      pending = 2* BULK_SIZE - done;
   }

   for (j=0; j<done; j++) putchar('d');
   for (j=0; j<running; j++) putchar('r');
   for (j=0; j<pending; j++) putchar('p');
   for (j=0; j<hold; j++) putchar('h');
   putchar('\n');
}

static void
validate_jobs(drmaa_job_ids_t *jobids_a, int chunks_a, drmaa_job_ids_t *jobids_b, int chunks_b)
{
   int j, status, drmaa_errno;
   char jobid[512];
   const char *all_jobs[] = { "DRMAA_JOB_IDS_SESSION_ALL" };
   char **ids_a, **ids_b;

   printf("JobA: chunksize %d chunks %d\n", chunks_a, BULK_SIZE/chunks_a);
   printf("JobB: chunksize %d chunks %d\n", chunks_b, BULK_SIZE/chunks_b);

   ids_a = (char **)malloc(sizeof(char *)*BULK_SIZE/chunks_a);
   ids_b = (char **)malloc(sizeof(char *)*BULK_SIZE/chunks_b);
   if (!ids_a || !ids_b) {
      fprintf(stderr, "malloc() failed\n");
      exit(1);
   }

   /* dup job A ids and verify user hold due to -h */
   for (j = 0; j < BULK_SIZE/chunks_a; j++) {

      drmaa_errno = drmaa_get_next_job_id(jobids_a, jobid, sizeof(jobid)-1);
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_get_next_job_id failed: %s\n", errorbuf);
         exit(1);
      }
      /* drmaa_job_ps(3) to return either DRMAA_PS_SYSTEM_ON_HOLD or DRMAA_PS_USER_SYSTEM_ON_HOLD for 
       * array tasks that are in hold due to -hold_jid_ad wc_job_list */
      drmaa_errno = drmaa_job_ps(jobid, &status, errorbuf, sizeof(errorbuf)-1);
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_job_ps failed: %s\n", errorbuf);
         exit(1);
      }

      if (status != DRMAA_PS_USER_ON_HOLD) {
         fprintf(stderr, "drmaa_job_ps failed: didn't return DRMAA_PS_USER_ON_HOLD but %d\n", status);
         exit(1);
      }
      ids_a[j] = strdup(jobid);
   }

   /* dup job B ids and verify system hold due to -hold_jid_ad */
   for (j = 0; j < BULK_SIZE/chunks_b; j++) {

      drmaa_errno = drmaa_get_next_job_id(jobids_b, jobid, sizeof(jobid)-1);
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_get_next_job_id failed: %s\n", errorbuf);
         exit(1);
      }
      /* drmaa_job_ps(3) to return either DRMAA_PS_SYSTEM_ON_HOLD or DRMAA_PS_USER_SYSTEM_ON_HOLD for 
       * array tasks that are in hold due to -hold_jid_ad wc_job_list */
      drmaa_errno = drmaa_job_ps(jobid, &status, errorbuf, sizeof(errorbuf)-1);
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_job_ps failed: %s\n", errorbuf);
         exit(1);
      }

      if (status != DRMAA_PS_SYSTEM_ON_HOLD &&
          status != DRMAA_PS_USER_SYSTEM_ON_HOLD) {
         fprintf(stderr, "drmaa_job_ps(%s) failed: didn't return DRMAA_PS_SYSTEM_ON_HOLD or DRMAA_PS_USER_SYSTEM_ON_HOLD but %d\n", 
               jobid, status);
         exit(1);
      }

      ids_b[j] = strdup(jobid);
   }

   state_monitor(ids_a, chunks_a, ids_b, chunks_b);
   /* release job A */
   for (j = 0; j < BULK_SIZE/chunks_a; j++) {
      printf("drmaa_control(%s, DRMAA_CONTROL_RELEASE)\n", ids_a[j]);
      if (drmaa_control(ids_a[j], DRMAA_CONTROL_RELEASE, errorbuf, sizeof(errorbuf)-1) != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_job_ps failed: %s\n", errorbuf);
         exit(1);
      }
   }

   while (drmaa_synchronize(all_jobs, 1, 0, errorbuf, sizeof(errorbuf)-1)==DRMAA_ERRNO_EXIT_TIMEOUT) {
      state_monitor(ids_a, chunks_a, ids_b, chunks_b);
   }
   state_monitor(ids_a, chunks_a, ids_b, chunks_b);

   for (j = 0; j < BULK_SIZE/chunks_a; j++) {
      free(ids_a[j]);
   }   
   free(ids_a);

   for (j = 0; j < BULK_SIZE/chunks_b; j++) {
      free(ids_b[j]);
   }   
   free(ids_b);
}

static 
drmaa_job_template_t *create_job_template(const char *job_path, const char *job_name, const char *pred_name, int seconds, int hold) 
{
   drmaa_job_template_t *jt = NULL;
   int drmaa_errno;
   char buf[512];

   const char *job_argv[2];

   if (drmaa_allocate_job_template(&jt, NULL, 0) != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "error: failed to create job template %s.\n", job_name);
      exit(1);
   }

   /* the job to be run */
   drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, job_path, NULL, 0);

   /* join output/error file */
   drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);

   /* path for output */
   drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":/dev/null", NULL, 0);

   /* job name for hold_jid_ad list */
   drmaa_set_attribute(jt, DRMAA_JOB_NAME, job_name, NULL, 0);


   /* drmaa_run_bulk_job(3) must accept -hold_jid_ad wc_job_list when passed through job  
      template attribute drmaa_native_specification */
   if (hold) {
      strcpy(buf, "-h ");
   }

   if (pred_name != NULL) {
      if (strlen(pred_name) > 256) {
         fprintf(stderr, "error: predecessor name too long!\n");
         exit(1);
      }
      strcpy(buf, "-hold_jid_ad ");
      strcat(buf, pred_name);
   }

   if (hold || pred_name != NULL) {
      drmaa_errno = drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, buf, errorbuf, sizeof(errorbuf)-1);
      if(drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_set_attribute failed: %s\n", errorbuf);
         exit(1);
      }
   }

   /* control job sleep time */
   sprintf(buf, "%d", seconds);
   job_argv[0] = buf; 
   job_argv[1] = NULL;
   drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);

   return jt;
}

int main(int argc, char **argv)
{
   drmaa_job_template_t *jt_a = NULL;
   drmaa_job_template_t *jt_b = NULL;
   drmaa_job_ids_t *jobids_a = NULL;
   drmaa_job_ids_t *jobids_b = NULL;
   const char *base_name = "Job";
   char name_a[512], name_b[512];

   struct chunking_t {
      int a;
      int b;
   } chunking[] = {
      { 1, 1 },
      { 1, 3 },
      { 1, 4 },
      { 1, 12 },
      { 3, 1 },
      { 3, 3 },
      { 3, 4 },
      { 3, 12 },
      { 4, 1 },
      { 4, 3 },
      { 4, 4 },
      { 4, 12 },
      { 12, 1 },
      { 12, 3 },
      { 12, 4 },
      { 12, 12 } 
   };
   int samples, i, drmaa_errno = 0;

   while (argc > 2) {
      if (!strcmp(argv[1], "-ps")) {
         fprintf(stderr, "use drmaa_job_ps() for progress monitoring\n");
         do_ps = 1;
         argc--;
         argv++;
      }
      if (!strcmp(argv[1], "-1st")) {
         fprintf(stderr, "first sample only\n");
         do_1st = 1;
         argc--;
         argv++;
      }
      if (!strcmp(argv[1], "-exit_unknown")) {
         fprintf(stderr, "first sample only\n");
         do_exit = 0;
         argc--;
         argv++;
      }

      if (!strcmp(argv[1], "-nm")) {
         argc--;
         argv++;
         base_name = argv[1];
         argc--;
         argv++;
         fprintf(stderr, "use \"%s\" as base name\n", base_name);
      }
   }

   if (argc != 2) {
      printf("Usage: %s path_to_sleeper_script\n", argv[0]);
      exit(1);
   }

   if (strstr(argv[1], "sleeper.sh") == NULL) {
      printf("Usage: %s path_to_sleeper_script\n", argv[0]);
      exit(1);
   }

   if (do_1st)
      samples = 1;
   else
      samples = sizeof(chunking)/sizeof(void *);

   while ((drmaa_errno=drmaa_init(NULL, errorbuf, sizeof(errorbuf)-1) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE))
      sleep(1);
   if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_init failed: %s\n", errorbuf);
      exit(EXIT_FAILURE);
   }

   strcpy(name_a, base_name);
   strcat(name_a, "A");
   strcpy(name_b, base_name);
   strcat(name_b, "B");

   for (i=0; i< samples; i++) {

      jt_a = create_job_template(argv[1], name_a, NULL, chunking[i].a, 1);
      jt_b = create_job_template(argv[1], name_b, name_a, chunking[i].b, 0);
      
      while ((drmaa_errno = drmaa_run_bulk_jobs(&jobids_a, jt_a, 1, BULK_SIZE, chunking[i].a, 
            errorbuf, sizeof(errorbuf)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
         sleep(1);
      if(drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_run_bulk_jobs(%d-%d:%d) failed: %s\n", 1, BULK_SIZE, chunking[i].a, errorbuf);
         return 1;
      }

      while ((drmaa_errno = drmaa_run_bulk_jobs(&jobids_b, jt_b, 1, BULK_SIZE, chunking[i].b, 
            errorbuf, sizeof(errorbuf)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
         sleep(1);
      if(drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_run_bulk_jobs(%d-%d:%d) failed: %s\n", 1, BULK_SIZE, chunking[i].b, errorbuf);
         return 1;
      }

      /* sleeper will sleep 60s by default, so this test should be okay */
      validate_jobs(jobids_a, chunking[i].a, jobids_b, chunking[i].b);

      drmaa_release_job_ids(jobids_a);
      drmaa_release_job_ids(jobids_b);
      
      drmaa_errno = drmaa_delete_job_template(jt_a, errorbuf, sizeof(errorbuf)-1);
      if(drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_delete_job_template failed: %s\n", errorbuf);
         return 1;
      }

      drmaa_errno = drmaa_delete_job_template(jt_b, errorbuf, sizeof(errorbuf)-1);
      if(drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_delete_job_template failed: %s\n", errorbuf);
         return 1;
      }
   }

   while ((drmaa_errno = drmaa_exit(errorbuf, sizeof(errorbuf)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
      sleep(1);
      
   if(drmaa_errno != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_exit failed: %s\n", errorbuf);
      return 1;
   }
    
   printf ("OK\n");

   return 0;
}
