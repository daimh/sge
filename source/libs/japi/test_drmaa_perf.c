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
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include "drmaa.h"

/* program defaults */
int njobs    = 100;
int nthreads = 1;
int dowait   = 1;
int quiet   = 0;
char *native_spec = "-w n";
char *job_path = NULL;
char **job_args = NULL;

char *scenario = NULL;
char *site_b = NULL;

drmaa_job_template_t *jt;


static drmaa_job_template_t *create_job_template(const char *job_path, const char *project, int i);
static void *submit_jobs(void *arg);
static void get_gmt(struct timeval *);
static int submit_by_project(const char *project);

static void usage(void) 
{
   fprintf(stderr, "usage: test_drmaa_perf [ options ] <path-to-job> [<job_args>]\n");
   fprintf(stderr, "   -jobs       <njobs>                                       number of jobs per thread (default 100)\n");
   fprintf(stderr, "   -native     <nativespec>                                  native specification passed (default \"-w n\")\n");
   fprintf(stderr, "   -threads    <nthreads>                                    number of submission thread (default 1)\n");
   fprintf(stderr, "   -wait       [yes|no]                                      wait for job completion (default yes)\n");
   fprintf(stderr, "   -quiet      [yes|no]                                      wait for job completion (default no)\n");
   fprintf(stderr, "   -scenario   [queue|type|number|pe].[none|resource|hostgroup|softresource|softhostgroup] options -jobs/-threads be ignored\n");

   /* scenario assumes
      - queue "all.q"
      - projects "project[1-4]"
      - consumables "APP1 ... APP40"
      - queue attribute type=all be used with all.q */
           
   return;
}

#define DELTA_SECONDS(t1, t2) (((double)t2.tv_sec - (double)t1.tv_sec) + ((double)t2.tv_usec - (double)t1.tv_usec)/1000000)

int main(int argc, char *argv[])
{
   char diagnosis[DRMAA_ERROR_STRING_BUFFER];
   char *s, jobid[100];
   int drmaa_errno, i;
   int ret = 0;
   struct timeval start_s, finish_s, wait_s;
  
   if (argc<2) {
      usage();
      return 1;
   }

   i = 1;
   do {
      if (!strcmp("-help", argv[i]) ||
          !strcmp("-h", argv[i])) {
         usage();
         return 0;

      } else if (!strcmp("-jobs", argv[i])) {
         i++; 
         if (argc < i+1) {
            usage();
            return 1;
         }
         njobs = atoi(argv[i]);
         i++; 

      } else if (!strcmp("-native", argv[i])) {
         i++; 
         if (argc < i+1) {
            usage();
            return 1;
         }
         native_spec = argv[i];
         i++; 

      } else if (!strcmp("-threads", argv[i])) {
         if (argc < i+1) {
            usage();
            return 1;
         }
         i++; 
         nthreads = atoi(argv[i]);
         i++; 

      } else if (!strcmp("-quiet", argv[i])) {
         i++; 
         if (argc < i+1) {
            usage();
            return 1;
         }
         if (!strcmp("yes", argv[i]) || !strcmp("y", argv[i])) 
            quiet = 1;
         else if (!strcmp("no", argv[i]) || !strcmp("n", argv[i])) 
            quiet = 0;
         else {
            usage();
            return 1;
         }
         i++; 

      } else if (!strcmp("-wait", argv[i])) {
         i++; 
         if (argc < i+1) {
            usage();
            return 1;
         }
         if (!strcmp("yes", argv[i]) || !strcmp("y", argv[i])) 
            dowait = 1;
         else if (!strcmp("no", argv[i]) || !strcmp("n", argv[i])) 
            dowait = 0;
         else {
            usage();
            return 1;
         }
         i++; 

      } else if (!strcmp("-scenario", argv[i])) {
         i++; 
         if (argc < i+1) {
            usage();
            return 1;
         }
         s = strchr(argv[i], '.');
         *s = '\0';
         if (strcmp("queue", argv[i]) && strcmp("type", argv[i]) && 
             strcmp("number", argv[i]) && strcmp("pe", argv[i])) {
            usage();
            return 1;
         }
         scenario = strdup(argv[i]);
        
         s++;
         if (strcmp("hostgroup", s) && strcmp("resource", s) && 
             strcmp("none", s) && strcmp("softresource", s) && 
             strcmp("softhostgroup", s)) {
            usage();
            return 1;
         }
         site_b = strdup(s);
         i++; 

      } else {
         job_path = argv[i];
         i++; 
         if (job_path[0]=='-') {
            usage();
            return 1;
         }

         if (argv[i]) {
            job_args = &argv[i];
         }
      }
   } while (i < argc && !job_path);

   if (!job_path) {
      usage();
      return 1;
   }

#if 0
   printf("job_path: \"%s\"\n", job_path);
   printf("njobs:    %d\n", njobs);
   printf("nthreads: %d\n", nthreads);
   printf("native:   %s\n", native_spec);
   printf("dowait:   %s\n", dowait?"yes":"no");
   printf("quiet:    %s\n", quiet?"yes":"no");
   printf("scenario: %s\n", scenario?scenario:"<no such>");
   printf("site_b:   %s\n", site_b?site_b:"<no such>");
   printf("1st arg:  %s\n", job_args?job_args[0]:"<noargs>");
#endif

   if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
      return 1;
   }

   get_gmt(&start_s);

   if (!scenario) {
      if (!getuid()) {
         fprintf(stderr, "switching to ah114088:gridware\n");
         setegid(339);
         seteuid(115088);
      }

      if (!(jt = create_job_template(job_path, NULL, 0))) {
         fprintf(stderr, "create_sleeper_job_template() failed\n");
         return 1;
      }

      if (nthreads==1) {
         if (submit_jobs(&argv[i]))
             return 1;
      } else {
         pthread_t *ids = NULL;
         ids = (pthread_t *)malloc(sizeof (pthread_t) * nthreads);

         for (i = 0; i < nthreads; i++) {
            if (pthread_create(&ids[i], NULL, submit_jobs, NULL)) {
               fprintf(stderr, "pthread_create() failed: %s\n", strerror(errno));
	       free(ids);
               return 1;
            }
         }

         for (i = 0; i < nthreads; i++) {
            pthread_join(ids[i], NULL);
         }
      }
   
      drmaa_delete_job_template(jt, NULL, 0);

      if (!getuid()) {
         fprintf(stderr, "switching to root:root\n");
         seteuid(0);
         setegid(0);
      }

   } else {
      if (submit_by_project("project1") || submit_by_project("project2") ||
          submit_by_project("project3") || submit_by_project("project4"))
            return 1;
   }

   get_gmt(&finish_s);
   printf("submission took %8.3f seconds\n", DELTA_SECONDS(start_s, finish_s)); 

   if (dowait) {
      int success = 1;

      for (i=0; i<njobs * nthreads; i++) {
         int stat;
         int aborted, exited, exit_status, signaled;

         drmaa_errno = drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY, jobid, sizeof(jobid)-1, 
            &stat, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_wait() failed: %s\n", diagnosis);
            return 1;
         }

         /*
          * report how job finished 
          */
         drmaa_wifaborted(&aborted, stat, NULL, 0);
         if (aborted) {
            printf("job \"%s\" never ran\n", jobid);
            success = 0;
         } else {
            drmaa_wifexited(&exited, stat, NULL, 0);
            if (exited) {
               drmaa_wexitstatus(&exit_status, stat, NULL, 0);
               if (exit_status != 0) {
                  success = 0;
                  printf("job \"%s\" with exit status %d\n", jobid, exit_status);
               } else {
                  if (!quiet)
                     printf("job \"%s\" finished regularly\n", jobid);
               }
            } else {
               success = 0;
               drmaa_wifsignaled(&signaled, stat, NULL, 0);
               if (signaled) {
                  char termsig[DRMAA_SIGNAL_BUFFER+1];
                  drmaa_wtermsig(termsig, DRMAA_SIGNAL_BUFFER, stat, NULL, 0);
                  printf("job \"%s\" finished due to signal %s\n", jobid, termsig);
               } else
                  printf("job \"%s\" finished with unclear conditions\n", jobid);
            }
         }
      }

      if (!success)
         ret = 1;

      get_gmt(&wait_s);
      printf("wait took %8.3f seconds\n", DELTA_SECONDS(finish_s, wait_s)); 
      printf("jobs took %8.3f seconds\n", DELTA_SECONDS(start_s, wait_s)); 
   }

   if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
      return 1;
   }
   
  return ret;
}

static int submit_by_project(const char *project)
{
   drmaa_job_template_t *jt = NULL; 
   char diagnosis[DRMAA_ERROR_STRING_BUFFER];
   char jobid[100];
   int drmaa_errno, i;

   for (i=1; i<=40; i++) {
      if (!(jt = create_job_template(job_path, project, i))) {
         fprintf(stderr, "create_sleeper_job_template() failed\n");
         return 1;
      }

      while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
               sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
         fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
         sleep(1);
      }
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
         return 1;
      }
      if (!quiet)
         printf("\t \"%s\"\n", jobid);
      drmaa_delete_job_template(jt, NULL, 0);
   }

   return 0;
}
static void *submit_jobs(void *arg)
{
   char diagnosis[DRMAA_ERROR_STRING_BUFFER];
   char jobid[100];
   int drmaa_errno, i;

   for (i=0; i<njobs; i++) {
      while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
               sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
         fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
         sleep(1);
      }
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
         return (void *)1;
      }
      if (!quiet)
         printf("\t \"%s\"\n", jobid);
   }

   return NULL;
}

static drmaa_job_template_t *create_job_template(const char *job_path, 
      const char *project, int i)
{
   drmaa_job_template_t *jt = NULL;
   char buffer[10240];

   if (drmaa_allocate_job_template(&jt, NULL, 0)!=DRMAA_ERRNO_SUCCESS)
      return NULL;

   /* run in users home directory */
   drmaa_set_attribute(jt, DRMAA_WD, DRMAA_PLACEHOLDER_HD, NULL, 0);

   if (scenario) {

      if (!strcmp(scenario, "queue")) {
         if (!strcmp(site_b, "none"))
            sprintf(buffer, "-P %s -l APP%d=1 -q all.q", project, i);
         else if (!strcmp(site_b, "hostgroup"))
            sprintf(buffer, "-P %s -l APP%d=1 -q all.q@@site_b", project, i);
         else if (!strcmp(site_b, "resource"))
            sprintf(buffer, "-P %s -l APP%d=1,site=b -q all.q", project, i);
         else if (!strcmp(site_b, "softresource"))
            sprintf(buffer, "-P %s -l APP%d=1 -q all.q -soft -l site=b", project, i);
         else /* "softhostgroup" */
            sprintf(buffer, "-P %s -l APP%d=1 -q all.q -soft -q *@@site_b", project, i);

      } else if (!strcmp(scenario, "type")) {
         if (!strcmp(site_b, "none"))
            sprintf(buffer, "-P %s -l APP%d=1,type=all", project, i);
         else if (!strcmp(site_b, "hostgroup"))
            sprintf(buffer, "-P %s -l APP%d=1,type=all -q *@@site_b", project, i);
         else if (!strcmp(site_b, "resource"))
            sprintf(buffer, "-P %s -l APP%d=1,type=all,site=b", project, i);
         else if (!strcmp(site_b, "softresource"))
            sprintf(buffer, "-P %s -l APP%d=1,type=all -soft -l site=b", project, i);
         else /* "softhostgroup" */
            sprintf(buffer, "-P %s -l APP%d=1,type=all -soft -q *@@site_b", project, i);

      } else if (!strcmp(scenario, "number")) {
         if (!strcmp(site_b, "none"))
            sprintf(buffer, "-P %s -l APP%d=1,number=24", project, i);
         else if (!strcmp(site_b, "hostgroup"))
            sprintf(buffer, "-P %s -l APP%d=1,number=24 -q *@@site_b", project, i);
         else if (!strcmp(site_b, "resource"))
            sprintf(buffer, "-P %s -l APP%d=1,number=24,site=b", project, i);
         else if (!strcmp(site_b, "softresource"))
            sprintf(buffer, "-P %s -l APP%d=1,number=24 -soft -l site=b", project, i);
         else /* "softhostgroup" */
            sprintf(buffer, "-P %s -l APP%d=1,number=24 -soft -q *@@site_b", project, i);
         
      } else if (!strcmp(scenario, "pe")) {
         if (!strcmp(site_b, "none"))
            sprintf(buffer, "-P %s -l APP%d=1 -pe pe1 1", project, i);
         else if (!strcmp(site_b, "hostgroup"))
            sprintf(buffer, "-P %s -l APP%d=1 -q *@@site_b -pe pe1 1", project, i);
         else if (!strcmp(site_b, "resource"))
            sprintf(buffer, "-P %s -l APP%d=1,site=b -pe pe1 1", project, i);
         else if (!strcmp(site_b, "softresource"))
            sprintf(buffer, "-P %s -l APP%d=1 -pe pe1 1 -soft -l site=b", project, i);
         else /* "softhostgroup" */
            sprintf(buffer, "-P %s -l APP%d=1 -pe pe1 1 -soft -q *@@site_b", project, i);
         
      } else { /* "none" */ 
         sprintf(buffer, "-l APP%d=1", i);
      }

      if (native_spec) {
         strcat(buffer, " ");
         strcat(buffer, native_spec);
      }
   } else
      strcpy(buffer, native_spec);

/* printf("### native spec \"%s\"\n", buffer); */
   drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, buffer, NULL, 0);

   /* the job to be run */
   drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, job_path, NULL, 0);

   /* the job's arguments if any */
   if (job_args)
      drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, (const char **)job_args, NULL, 0);

   /* join output/error file */
   drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);

   /* submit job using euid/egid */
   if (!getuid()) {
      drmaa_set_attribute(jt, DRMAA_SUBMIT_AS_EUID, "y", NULL, 0);
   }

   /* path for output */
   drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":/dev/null", NULL, 0);

   return jt;
}

static void get_gmt(struct timeval *now)
{
   gettimeofday(now, NULL);
   return;
}
