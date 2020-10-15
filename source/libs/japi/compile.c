#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include "drmaa.h"

#define JOB_MAX 100

typedef struct {
   char *name;
   char *ns;
   char *jid;
   char *output_file;
} job_info_t;

job_info_t job[JOB_MAX];

static int terminate_session = 0;
static int terminate_program = 0;

/*
static void usage(FILE* fp) {
	fprintf(fp, "usage: raimk <aimk_options>");
}
*/

/* clear bookkeeping for a job */
static void clear_job_info(int n)
{
   free((job[n].jid)); 
   free((job[n].name)); 
   free((job[n].ns)); 
   free((job[n].output_file)); 
}

/* initialize bookkeeping */
static void clear_all_job_info(void)
{
   int i;

   for (i = 0; i < JOB_MAX; i++) {
      clear_job_info(i);
   }
}

/* return number of jobs still registered */
static int number_of_jobs(void) {
   int i;
   int ret = 0;

   for (i = 0; i < JOB_MAX; i++) {
      if (job[i].jid != NULL) {
         ret++;
      }
   }

   return ret;
}

/* search a certain job */
static int search_job(const char *jobid) 
{
   int i;

   for (i=0; i < JOB_MAX; i++) {
      if (job[i].jid != NULL) {
         if (strcmp(jobid, job[i].jid) == 0) {
            return i;
         }
      }
   }

   return -1;
}

/* delete all our jobs through drmaa */
static void delete_all_jobs(void)
{
   int i;
   char err_buf[1024];

   terminate_session = 0;

   for (i = 0; i < JOB_MAX; i++) {
      if (job[i].jid != NULL) {
         int drmaa_ret;
         printf("    deleting job %s\n", job[i].jid);
         drmaa_ret = drmaa_control(job[i].jid, DRMAA_CONTROL_TERMINATE, err_buf, sizeof(err_buf));
         if (drmaa_ret != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "deleting job %s failed: %s\n", job[i].jid, err_buf);
         }
      }
   }
}

/* signal handler for CTRL-C */
static void my_compile_signal_handler(int sig)
{
   static int terminated = 0;

   if (sig == SIGINT || sig == SIGTERM) {
      if (terminated) {
         terminate_program = 1;
      } else {
         terminate_session = 1;
         terminated = 1;
      }
   }
}

int main(int argc, char *argv[])
{
   const char *filename;
   int ret = 0;
   char diagnosis[DRMAA_ERROR_STRING_BUFFER];
   char jobwd[1024*4];
   char jobid[1024];
   char line[2*1024];
   int drmaa_errno;
   drmaa_job_template_t *jt = NULL;
   int stat;
   int aborted, exited, exit_status, signaled;
   int j, njobs = 0;
   FILE *fp;
   struct sigaction sa;

   /* clear job info structure */
   clear_all_job_info();

   /* setup a signal handler for shutdown */
   memset(&sa, 0, sizeof(sa));
   sa.sa_handler = my_compile_signal_handler;  /* one handler for all signals */
   sigemptyset(&sa.sa_mask);
   sigaction(SIGINT,  &sa, NULL);
   sigaction(SIGTERM, &sa, NULL);
   sigaction(SIGHUP,  &sa, NULL);
   sigaction(SIGPIPE, &sa, NULL);

   /* we can override use of a compile.conf in cwd by environment */
   filename = getenv("RAIMK_COMPILE_CONF");
   if (filename == NULL) {
      filename = "compile.conf";
   }

   /* we'll start the job in the cwd */
   if (!getcwd(jobwd, sizeof(jobwd)-1)) {
      fprintf(stderr, "getcwd() failed: %s\n", strerror(errno));
      ret = 2;
      goto Finish;
   }

   /* try to open config file */
   if (!(fp = fopen(filename, "r"))) {
      fprintf(stderr, "fopen(\"compile.conf\") failed: %s\n", strerror(errno));
      ret = 2;
      goto Finish;
   }

   /* initialize a drmaa session */
   if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
      fclose(fp);
      return 2;
   }

   printf("--- start cluster session --------------------------------\n");

   /* parse the config file and start a job for every architecture */
   while (fscanf(fp, "%[^\n]\n", line) == 1) {
      char nat_spec[1024];
      char arch[1024];
      char name[1024];
      char ns[1024];
      char output_file[1024];
      char dummy[1024];

      /* skip comment lines */
      if (line[0] == '#')
         continue;

      if (sscanf(line, "%[^\t ]%[\t ]%[^\n]\n", arch, dummy, ns) != 3) {
         fprintf(stderr, "parsing error in compile.conf\n");
         continue;
      }

      sprintf(name, "build %s", arch);

      /* build job template */
      if (drmaa_allocate_job_template(&jt, NULL, 0)!=DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
         ret = 2;
         goto Finish;
      }

      drmaa_set_attribute(jt, DRMAA_WD, jobwd, NULL, 0);
      drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, argv[1], NULL, 0);
      drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);
      drmaa_set_attribute(jt, DRMAA_JOB_NAME, name, NULL, 0);

      sprintf(nat_spec, "-b no -S /bin/sh %s", ns);
      drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, nat_spec, NULL, 0);

      sprintf(output_file, ":%s/build_%s.log", jobwd, arch);
      drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, output_file, NULL, 0);

      drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, (const char **)&argv[2], NULL, 0);

      /* submit job */
      if ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
               sizeof(diagnosis)-1)) != DRMAA_ERRNO_SUCCESS) {
         drmaa_delete_job_template(jt, NULL, 0);

         if (drmaa_errno == DRMAA_ERRNO_DENIED_BY_DRM) {
            printf("--- job \"%s\" using \"%s\" wasn't accepted: %s\n", name, ns, diagnosis);
            continue;
         }

         fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
         ret = 2;
         goto Finish;
      }

      /* remember job information */
      job[njobs].jid = strdup(jobid);
      job[njobs].name = strdup(name);
      job[njobs].ns = strdup(ns);
      job[njobs].output_file = strdup(output_file);
      njobs++;

      drmaa_delete_job_template(jt, NULL, 0);
      
      printf("    submitted job \"%s\" as job %s\n", name, jobid);
   }
   fclose(fp);

   /* monitor jobs, until all have finished */
   while (number_of_jobs() > 0) {
      /* We wait with timeout to be able to react on events like CTRL-C */
      drmaa_errno = drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY, jobid, sizeof(jobid)-1, 
                               &stat, 1, NULL, diagnosis, sizeof(diagnosis)-1);

      /* error */
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS && drmaa_errno != DRMAA_ERRNO_EXIT_TIMEOUT) {
         fprintf(stderr, "drmaa_wait() failed: %s\n", diagnosis);
         ret = 2;
         goto Finish;
      }

      /* user pressed CTRL-C: delete all jobs */
      if (terminate_session) {
         printf("--- shutdown requested --------------------------------\n");
         delete_all_jobs();
      }
 
      /* if user pressed CTRL-C multiple times, exit */
      if (terminate_program) {
         printf("--- forced shutdown -----------------------------------\n");
         goto Finish;
      }
  
      /* 
       * a job terminated - evaluate return codes and deregister job from
       * our internal bookkeeping
       */
      if (drmaa_errno == DRMAA_ERRNO_SUCCESS) {
         j = search_job(jobid);
         if (j < 0) {
            fprintf(stderr, "drmaa_wait() returns unknown job ... ?\n");
         }

         /* report how job finished */
         drmaa_wifaborted(&aborted, stat, NULL, 0);
         if (aborted) {
            printf("--- run \"%s\" stopped or never started\n", job[j].name);
         } else {
            int failed = 1;
            char *path = job[j].output_file + 1;

            drmaa_wifexited(&exited, stat, NULL, 0);
            if (exited) {
               drmaa_wexitstatus(&exit_status, stat, NULL, 0);
               if (exit_status == 0) {
                  printf("+++ run \"%s\" was successful\n", job[j].name);
                  failed = 0;
               } else {
                  printf("### run \"%s\" broken ##################################\n", job[j].name);
                  ret = 1;
               }
            } else {
               drmaa_wifsignaled(&signaled, stat, NULL, 0);
               if (signaled) {
                  char termsig[DRMAA_SIGNAL_BUFFER+1];
                  drmaa_wtermsig(termsig, DRMAA_SIGNAL_BUFFER, stat, NULL, 0);
                  printf("job \"%s\" finished due to signal %s\n", job[j].name, termsig);
               } else {
                  printf("job \"%s\" finished with unclear conditions\n", job[j].name);
               }
            }

            /* 
             * If a job succeeded, we delete its output file.
             * If it failed, we show the end of the output file.
             */
            if (failed) {
               char tail_cmd[1024];
               sprintf(tail_cmd, "tail -15 %s", path);
               ret = system(tail_cmd);
            } else {
               if (unlink(path) != 0) {
                  fprintf(stderr, "couldn't unlink \"%s\" job output file %s: %s\n",
                          job[j].name, path, strerror(errno));
               }
            }
         }

         /* clean the job struct */
         clear_job_info(j);
      }
   }

   printf("--- end cluster session --------------------------------\n");

Finish:
   if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
      return 1;
   }
   return ret;
}
