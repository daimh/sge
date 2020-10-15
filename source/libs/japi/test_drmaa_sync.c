/** test_drmaa_sync.c
 *
 * Created on September 15, 2004, 11:12 AM
 *
 * @author  dan.templeton@sun.com
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "japi/drmaa.h"

static void *submit_thread(void *arg);
static void *sync_thread(void *arg);

static int count = -1;
static pthread_mutex_t japi_session_mutex = PTHREAD_MUTEX_INITIALIZER;

#define LOCK_COUNT()   pthread_mutex_lock(&japi_session_mutex)                                 
#define UNLOCK_COUNT() pthread_mutex_unlock(&japi_session_mutex)

int main(int argc, char *argv[])
{
   pthread_t tid1, tid2;
   
   drmaa_init (NULL, NULL, 0);
   
   pthread_create (&tid1, NULL, submit_thread, argv[1]);
   pthread_create (&tid2, NULL, sync_thread, NULL);
   
   sleep (60);
   drmaa_exit (NULL, 0);
   
   pthread_join (tid1, NULL);
   pthread_join (tid2, NULL);
   
   return count;
}

static void *submit_thread(void *arg)
{
   drmaa_job_template_t *jt = NULL;
   char jobid[DRMAA_JOBNAME_BUFFER];
   const char *job_argv[2] = {"5", NULL};
   int check_count = 1;
   
   drmaa_allocate_job_template (&jt, NULL, 0);
   drmaa_set_attribute (jt, DRMAA_REMOTE_COMMAND, (char *)arg, NULL, 0);
   drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
   drmaa_set_attribute (jt, DRMAA_OUTPUT_PATH, ":"DRMAA_PLACEHOLDER_HD"/DRMAA_JOB", NULL, 0);
   drmaa_set_attribute (jt, DRMAA_JOIN_FILES, "y", NULL, 0);
   
   while (drmaa_run_job (jobid, DRMAA_JOBNAME_BUFFER, jt, NULL, 0) == DRMAA_ERRNO_SUCCESS) {
      if (check_count) {
         LOCK_COUNT();
      
         if (count < 0) {
            count = 0;
         }
         
         UNLOCK_COUNT();
         
         check_count = 0;
      }
   }
   
   drmaa_delete_job_template (jt, NULL, 0);
   
   return (void *)NULL;
}

static void *sync_thread(void *arg)
{
   const char *jobids[2] = {DRMAA_JOB_IDS_SESSION_ALL, NULL};
   int dispose = 0;
   
   while (drmaa_synchronize (jobids, DRMAA_TIMEOUT_WAIT_FOREVER, dispose, NULL, 0) == DRMAA_ERRNO_SUCCESS) {
      LOCK_COUNT();
      
      if (count >= 0) {
         count++;
      }
      
      UNLOCK_COUNT();
      
      dispose = !dispose;
   }
   
   return (void *)NULL;
}
