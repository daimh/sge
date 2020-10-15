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


/* system */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "uti/sge_lock.h"
#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_dstring.h"
#include "uti/sge_time.h"

#include "cull/cull.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"

#include "spool/sge_spooling.h"
#include "spool/loader/sge_spooling_loader.h"

static const int loops = 2000;

static int delay = 0;

#define LOCAL_TRANSACTION 0
#define SGE_LOCKING 1

/* JG: TODO: test:
 * - do we have the locking problems, if the keys are more different?
 *   ---> still there, but less frequent
 * - error handling: if a deadlock is reported, repeate the operation, 
 *   after a short delay.
 *   ---> doesn't work within the same transaction
 */

#if SGE_LOCKING
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
#endif

static bool add_job(int job_id)
{
   bool write_ok;
   lListElem *job;
   lList *answer_list = NULL;
   lList *master_job_list = *object_type_get_master_list(SGE_TYPE_JOB);

   const char *key;
   dstring key_dstring;
   char key_buffer[100];

   sge_dstring_init(&key_dstring, key_buffer, sizeof(key_buffer));

   job = lAddElemUlong(&master_job_list, JB_job_number, job_id, JB_Type);
   key = job_get_key(job_id, 0, NULL, &key_dstring);
#if LOCAL_TRANSACTION
   spool_transaction(&answer_list, spool_get_default_context(),
                     STC_begin); 
   answer_list_output(&answer_list);
#endif
   write_ok = spool_write_object(&answer_list, spool_get_default_context(),
                                job, key, SGE_TYPE_JOB, false);
   answer_list_output(&answer_list);

   if (delay > 0) {
      sge_usleep(delay * 1000);
   }

#if LOCAL_TRANSACTION
   spool_transaction(&answer_list, spool_get_default_context(),
                     write_ok ? STC_commit : STC_rollback); 
   answer_list_output(&answer_list);
#endif

   return write_ok;
}

static bool del_job(int job_id)
{
   bool del_ok;
   lList *answer_list = NULL;
   lList *master_job_list = *object_type_get_master_list(SGE_TYPE_JOB);

   const char *key;
   dstring key_dstring;
   char key_buffer[100];

   sge_dstring_init(&key_dstring, key_buffer, sizeof(key_buffer));

   key = job_get_key(job_id, 0, NULL, &key_dstring);
#if LOCAL_TRANSACTION
   spool_transaction(&answer_list, spool_get_default_context(),
                     STC_begin); 
   answer_list_output(&answer_list);
#endif
   del_ok = spool_delete_object(&answer_list, spool_get_default_context(),
                               SGE_TYPE_JOB, key, false);
   answer_list_output(&answer_list);

   lDelElemUlong(&master_job_list, JB_job_number, job_id);

   if (delay > 0) {
      sge_usleep(delay * 1000);
   }

#if LOCAL_TRANSACTION
   spool_transaction(&answer_list, spool_get_default_context(),
                     write_ok ? STC_commit : STC_rollback); 
   answer_list_output(&answer_list);
#endif

   return del_ok;
}

static void *work(void *args)
{
   int i;
   int work_num;
   int failed = 0;

   DENTER(TOP_LAYER, "work");

   work_num = *((int *)args);
  
   WARNING((SGE_EVENT, "work %d starting %d loops", work_num, loops));
  
   for (i = 0; i < loops; i++) {
#if SGE_LOCKING
/*       SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE); */
      pthread_mutex_lock(&mtx);
#endif
      if (!add_job(work_num * loops + i)) {
         failed++;
      }
#if SGE_LOCKING
/*       SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE); */
      pthread_mutex_unlock(&mtx);
#endif
   }

   WARNING((SGE_EVENT, "work %d finished adding %d jobs, %d failed",
            work_num, loops, failed));

   failed = 0;
   for (i = 0; i < loops; i++) {
#if SGE_LOCKING
/*       SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE); */
      pthread_mutex_lock(&mtx);
#endif
      if(!del_job(work_num * loops + i)) {
         failed++;
      }
#if SGE_LOCKING
/*       SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE); */
      pthread_mutex_unlock(&mtx);
#endif
   }

   WARNING((SGE_EVENT, "work %d finished deleting %d jobs, %d failed",
            work_num, loops, failed));

   
   DEXIT;
   return (void *)NULL;
}

int main(int argc, char *argv[])
{
   const char *url;
   int i, threads;
   pthread_t *t;
   int *args;

   lList *answer_list = NULL;
   lListElem *spooling_context;

   DENTER_MAIN(TOP_LAYER, "test_berkeleydb_mt");

   /* parse commandline parameters */
   if (argc < 3) {
      ERROR((SGE_EVENT, "usage: test_berkeleydb_mt <url> <threads> [<delay>]\n"));
      ERROR((SGE_EVENT, "       <url>     = path or host:database\n"));
      ERROR((SGE_EVENT, "       <threads> = number of threads\n"));
      ERROR((SGE_EVENT, "       <delay>   = delay after writing [ms]\n"));
      SGE_EXIT(NULL, 1);
   }

   url = argv[1];
   threads = atoi(argv[2]);

   if (argc > 3) {
      delay = atoi(argv[3]);
   }

   /* allocate memory for pthreads and arguments */
   t = (pthread_t *)malloc(threads * sizeof(pthread_t));
   args = (int *)malloc(threads * sizeof(int));

   DPRINTF(("writing to database %s from %d threads\n", url, threads));

   /* initialize spooling */
   spooling_context = spool_create_dynamic_context(&answer_list, NULL, url, NULL);
   answer_list_output(&answer_list);
   if (spooling_context == NULL) {
      SGE_EXIT(NULL, EXIT_FAILURE);
   }

   spool_set_default_context(spooling_context);

   if (!spool_startup_context(&answer_list, spooling_context, true)) {
      answer_list_output(&answer_list);
      SGE_EXIT(NULL, EXIT_FAILURE);
   }
   answer_list_output(&answer_list);

   /* let n threads to parallel spooling */
   for (i = 0; i < threads; i++) {
      args[i] = i + 1;     
      pthread_create(&(t[i]), NULL, work, (void*)(&args[i]));
   }

   /* also work in current thread */
   work((void *)0);

   /* wait for termination of all threads */
   for (i = 0; i < threads; i++) {
      pthread_join(t[i], NULL);
   }

   /* shutdown spooling */
   spool_shutdown_context(&answer_list, spooling_context);
   answer_list_output(&answer_list);

   sge_free(&t);

   DEXIT;
   return EXIT_SUCCESS;
}
