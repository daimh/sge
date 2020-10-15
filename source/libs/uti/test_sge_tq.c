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
 *   Copyright: 2009 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdlib.h>
#include <stdio.h>
#include <fnmatch.h>

#include "uti/sge_rmon.h"
#include "uti/sge_tq.h"
#include "uti/sge_err.h"
#include "uti/sge_thread_ctrl.h"
#include "uti/sge_mtutil.h"

/*
 * Producer and consumer maximum should be a multiple of 2 and 3 
 */
#define TEST_SL_MAX_CONSUMER 24 
#define TEST_SL_MAX_PRODUCER 24 

#define TEST_SL_MAX_ELEMENTS 10000

struct _test_sl_thread_cp_t {
   pthread_mutex_t mutex;
   sge_tq_queue_t *queue;
   u_long32 counter;
   dstring sequence;
   volatile bool do_terminate;
};

typedef struct _test_sl_thread_cp_t test_sl_thread_cp_t;

void *
test_thread_consumer_template(void *arg, sge_tq_type_t type, const char *type_string) {
   void *ret = NULL;
   test_sl_thread_cp_t *global = (test_sl_thread_cp_t *)arg;

   DENTER(TOP_LAYER, "test_thread_consumer_main");
   while (global->do_terminate != true) { 
      const char *string;
     
      /* consume: first element */
      sge_tq_wait_for_task(global->queue, 1, type, (void **)&string);

      if (string != NULL) {
         if (fnmatch(type_string, string, 0) != 0) {
            fprintf(stderr, "got %s from queue and not %s\n", string, type_string);
            break;
         }
      } else {
         if (sge_thread_has_shutdown_started() == false) {  
            fprintf(stderr, "got NULL from queue although thread was not terminated\n");
            break;
         }
      }

      pthread_mutex_lock(&global->mutex);
      sge_dstring_append_char(&global->sequence, 'c');
      pthread_mutex_unlock(&global->mutex);
   }
   DRETURN(ret);
}

void *
test_thread_producer_template(void *arg, sge_tq_type_t type, const char *type_string) {
   void *ret = NULL;
   test_sl_thread_cp_t *global = (test_sl_thread_cp_t *)arg;

   DENTER(TOP_LAYER, "test_thread_producer_main");
   while (global->do_terminate != true) { 
      /* produce: new element */
      sge_tq_store_notify(global->queue, type, (void*)type_string);

      /* trigger termination */
      pthread_mutex_lock(&global->mutex);
      sge_dstring_append_char(&global->sequence, 'p');
      global->counter++;
      if (global->counter > TEST_SL_MAX_ELEMENTS) {
         global->do_terminate = true;
         sge_thread_notify_all_waiting();
      }
      pthread_mutex_unlock(&global->mutex);
   }
   DRETURN(ret);
}

void *
test_thread_consumer_type1(void *arg) {
   return test_thread_consumer_template(arg, SGE_TQ_TYPE1, "type_1");
}

void *
test_thread_consumer_type2(void *arg) {
   return test_thread_consumer_template(arg, SGE_TQ_TYPE2, "type_2");
}

void *
test_thread_consumer_unknown(void *arg) {
   return test_thread_consumer_template(arg, SGE_TQ_UNKNOWN, "type_?");
}

void *
test_thread_producer_type1(void *arg) {
   return test_thread_producer_template(arg, SGE_TQ_TYPE1, "type_1");
}

void *
test_thread_producer_type2(void *arg) {
   return test_thread_producer_template(arg, SGE_TQ_TYPE2, "type_2");
}

/*
 * Scenario: Producer - Consumer
 * - TEST_SL_MAX_CONSUMER consumer threads will be created
 * - TEST_SL_MAX_PRODUCER producer threads will be created
 * - consumer threads wait for an element in a global list
 * - producer threads put an element into the list
 * - consumer und producer append a c or p letter into a global string
 * - the producer creating the 1000th element triggers termination of threads
 * - global string contains execution sequence of p and c threads
 */
bool
test_mt_consumer_producer(void) {
   bool ret = true;
   test_sl_thread_cp_t global;
   const char *string;
   int i;
   char last = '\0';
   int switches = 0;

   DENTER(TOP_LAYER, "test_mt_consumer_producer");

   /* create a list */
   memset(&global, 0, sizeof(test_sl_thread_cp_t));
   global.do_terminate = false;
   global.counter = 0;
   pthread_mutex_init(&global.mutex, NULL);
   ret = sge_tq_create(&global.queue);

   /* spawn threads */
   if (ret) {
      pthread_t consumer[TEST_SL_MAX_CONSUMER];
      pthread_t producer[TEST_SL_MAX_PRODUCER];
      int i;

      for (i = 0; i < TEST_SL_MAX_CONSUMER; i++) {
         if (i < TEST_SL_MAX_CONSUMER / 3) {
            pthread_create(&(consumer[i]), NULL, test_thread_consumer_type1, &global);
         } else if (i < TEST_SL_MAX_CONSUMER * 2 / 3) {
            pthread_create(&(consumer[i]), NULL, test_thread_consumer_type2, &global);
         } else {
            pthread_create(&(consumer[i]), NULL, test_thread_consumer_unknown, &global);
         }
      }
      for (i = 0; i < TEST_SL_MAX_PRODUCER; i++) {
         if (i < TEST_SL_MAX_PRODUCER / 2) {
            pthread_create(&(producer[i]), NULL, test_thread_producer_type1, &global);
         } else {
            pthread_create(&(producer[i]), NULL, test_thread_producer_type2, &global);
         }
      }

      for (i = 0; i < TEST_SL_MAX_CONSUMER; i++) {
         pthread_join(consumer[i], NULL);
      }
      for (i = 0; i < TEST_SL_MAX_PRODUCER; i++) {
         pthread_join(producer[i], NULL);
      }
   }

   string = sge_dstring_get_string(&global.sequence);
   for (i = 0; i < strlen(string); i++) {
      if (last != string[i]) {
         switches++;
      }
      last = string[i];
   }


   /* following does not work in slow or virtual hosts */
#if 0
   {
      int max_possible_switches = TEST_SL_MAX_PRODUCER * TEST_SL_MAX_ELEMENTS / 2;
      int min = max_possible_switches / 33;
      int max = max_possible_switches - max_possible_switches / 33;

      /* 
       * check thread type execution sequence:
       * for this test we are happy if it is in min/max range 
       * plus/minus ~3%
       */
      if (switches < min || switches > max) {
         fprintf(stderr, "switch of thread type is out of range in %s(). "
                 "got %d but expected something in range [%d;%d]", 
                 SGE_FUNC, switches, min, max);
      }
   }
#endif

   /* cleanup */
   pthread_mutex_destroy(&global.mutex);
   ret &= sge_tq_destroy(&global.queue);

   DRETURN(ret);
}

int main(int argc, char *argv[]) {
   bool ret = true;

   DENTER_MAIN(TOP_LAYER, "test_sl");

   sge_err_init();

   ret &= test_mt_consumer_producer();

   DRETURN(ret == true ? 0 : 1);
}

