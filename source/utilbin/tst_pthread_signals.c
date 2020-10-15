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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "basis_types.h"

#undef FALSE
#undef TRUE

enum { FALSE = 0, TRUE = 1, NUM_THRDS = 3 };

typedef struct {
   pthread_mutex_t  mtx;
   pthread_cond_t   cndvar;
   int              quit;  /* should we quit */
   int              cntr;  /* number of threads (except main thread) */
} control_block_t;

static control_block_t cb = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0 };


static void  incr_thrd_cnt(void);
static void* signal_emitter(void*);
static void* signal_waiter(void*);
static void  ignore_signals(void);
static void  reap_thrds(void);
static int   should_quit(void);


/****** /utilbin/tst_pthread_signals/main() ************************************
*  NAME
*     main() -- test pthread signal handling
*
*  SYNOPSIS
*     int main(int argc, char* argv[]) 
*
*  FUNCTION
*     Test pthread signal handling. Create a signal handling thread and multiple
*     threads which do emit signals. Shutdown if a 'SIGINT' is received.
*
*     Please note that all signals are send to the process, not a particular
*     thread.
*
*  INPUTS
*     int argc     - not used
*     char* argv[] - not used 
*
*  RESULT
*     int - 0
*
*******************************************************************************/
int main(int argc, char* argv[])
{
   sigset_t sig_set;
   pthread_t id[NUM_THRDS];
   int i;


   sigfillset(&sig_set);
   pthread_sigmask(SIG_SETMASK, &sig_set, NULL);
   
   printf("main: creating threads\n");

   pthread_create(&(id[0]), NULL, signal_waiter, NULL);
   incr_thrd_cnt();

   for (i = 1; i < NUM_THRDS; i++) {
      pthread_create(&(id[i]), NULL, signal_emitter, NULL);
      incr_thrd_cnt();
   }

   printf("main: startig to join threads\n");

   for (i = 0; i < NUM_THRDS; i++) {
      pthread_join(id[i], NULL);
   }

   printf("main: all threads joined - quit\n");
   return 0;
} /* main() */

/****** utilbin/tst_pthread_signals/incr_thrd_cnt() ***************************
*  NAME
*     incr_thrd_cnt() -- increment thread count 
*
*  SYNOPSIS
*     static void incr_thrd_cnt(void) 
*
*  FUNCTION
*     Increment number of active threads. The so called 'main thread' is NOT
*     counted. The unmber of active threads is used to coordinate the shutdown
*     process.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: incr_thrd_cnt() is MT safe. 
*
*  SEE ALSO
*     utilbin/tst_pthread_signals/should_quit()
*     utilbin/tst_pthread_signals/reap_thrds()
*
*******************************************************************************/
static void incr_thrd_cnt(void)
{
   pthread_mutex_lock(&cb.mtx);
   cb.cntr++;
   pthread_mutex_unlock(&cb.mtx);

   return;
} /* incr_thrd_cnt() */

/****** utilbin/tst_pthread_signals/signal_emitter() ***************************
*  NAME
*     signal_emitter() -- emit signals 
*
*  SYNOPSIS
*     static void* signal_emitter(void* anArg) 
*
*  FUNCTION
*     Emit signals, randomly selected from a fixed set of signals. Enter
*     infinite loop. Select signal. Send signal to the process. After
*     each iteration check for termination.
*
*  INPUTS
*     void* anArg - not used 
*
*  RESULT
*     void* - NULL 
*
*  NOTES
*     MT-NOTE: signal_emitter() is a thread function.
*
*******************************************************************************/
static void* signal_emitter(void* anArg)
{
   int sig[3] = {SIGPIPE, SIGUSR1, SIGUSR2};
   unsigned int i = (unsigned int)pthread_self(); /* seed */
   bool done = false;

   while (!done) {
      int j = (rand_r(&i) % 3);

      if (should_quit() == TRUE) {
         printf("signal_emitter: will terminate\n");
         done = true;
         break;
      }

      printf("signal_emitter %d will raise: %d\n", (int)pthread_self(), sig[j]);
      kill(getpid(), sig[j]);
      sleep(4);
   }

   return NULL;
} /* signal_emitter() */

/****** utilbin/tst_pthread_signals/should_quit() ******************************
*  NAME
*     should_quit() -- should thread quit? 
*
*  SYNOPSIS
*     static int should_quit(void) 
*
*  FUNCTION
*     Determine if thread should quit. Lock control block mutex. Inspect quit
*     flag. Decrement thread counter. Signal control block condition variable
*     waiters. Unlock control block mutex.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     FALSE - continue 
*     TRUE  - quit
*
*  NOTES
*     MT-NOTE: should_quit() is MT safe. 
*
*******************************************************************************/
static int should_quit(void)
{
   int res = FALSE;

   printf("should_quit: check termination\n");
   pthread_mutex_lock(&cb.mtx);

   if (cb.quit == TRUE) {
      printf("should_quit: do quit\n");
      cb.cntr--;
      pthread_cond_signal(&cb.cndvar);
      res = TRUE;
   }

   pthread_mutex_unlock(&cb.mtx);
   return res;
} /* should_quit() */

/****** utilbin/tst_pthread_signals/signal_waiter() ****************************
*  NAME
*     signal_waiter() -- wait for signals
*
*  SYNOPSIS
*     static void* signal_waiter(void* anArg) 
*
*  FUNCTION
*     Wait for signals. Establish recognized signal set. Enter infinite loop.
*     Wait for signal. Announce signal received. If signal is 'SIGINT', wait
*     for all other threads (except main thread) to terminate. Terminate.
*
*  INPUTS
*     void* anArg - not used 
*
*  RESULT
*     void* - NULL 
*
*  NOTES
*     MT-NOTE: signal_waiter() is a thread function.
*
*******************************************************************************/
static void* signal_waiter(void* anArg)
{
   sigset_t set;
   int num;
   bool exit = false;


   printf("signal_waiter started\n");

   ignore_signals();

   sigemptyset(&set);
   sigaddset(&set, SIGINT);
   sigaddset(&set, SIGALRM);
   sigaddset(&set, SIGPIPE);
   sigaddset(&set, SIGUSR1);
   sigaddset(&set, SIGUSR2);

   while (!exit)
   {
      printf("signal_waiter is waiting for signal\n");

      sigwait(&set, &num);

      switch (num) {
         case SIGINT:
            printf("signal_waiter: got signal SIGINT\n");
            reap_thrds();
            exit = true;
            break;
         case SIGALRM:
            printf("signal_waiter: got signal SIGALRM\n");
            break;
         case SIGPIPE:
            printf("signal_waiter: got signal SIGPIPE\n");
            break;
         case SIGUSR1:
            printf("signal_waiter: got signal SIGUSR1\n");
            break;
         case SIGUSR2:
            printf("signal_waiter: got signal SIGUSR2\n");
            break;
         default:
            printf("signal_waiter: got signal %d\n", num);
            break;
      }
   }

   return NULL;
} /* signal_waiter() */

/****** utilbin/tst_pthread_signals/ignore_signals() ***************************
*  NAME
*     ignore_signals() -- ignore signals 
*
*  SYNOPSIS
*     static void ignore_signals(void) 
*
*  FUNCTION
*     Ignore all signals, which 'signal_waiter()' should not recognize.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: ignore_signals() is NOT MT safe. 
*
*******************************************************************************/
static void ignore_signals(void)
{
   struct sigaction act;


   act.sa_handler = SIG_IGN;
   sigaction(SIGABRT, &act, NULL);
   sigaction(SIGCHLD, &act, NULL);
   sigaction(SIGCONT, &act, NULL);
   sigaction(SIGHUP, &act, NULL);
   sigaction(SIGQUIT, &act, NULL);
   sigaction(SIGTERM, &act, NULL);
   sigaction(SIGTSTP, &act, NULL);
   sigaction(SIGTTIN, &act, NULL);
   sigaction(SIGTTOU, &act, NULL);
   sigaction(SIGURG, &act, NULL);

#if !defined(DARWIN) && !defined(FREEBSD) && !defined(NETBSD)
   sigaction(SIGPOLL, &act, NULL);
#endif

   return;
} /* ignore_signals() */

/****** utilbin/tst_pthread_signals/reap_thrds() *******************************
*  NAME
*     reap_thrds() -- reap threads 
*
*  SYNOPSIS
*     static void reap_thrds(void) 
*
*  FUNCTION
*     Reap threads. Lock control block mutex. Set quit flag to true. If there
*     is more than one remaining, wait on control block condition variable.
*     Unlock control block mutex. 
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: reap_thrds() must be called from within a single thread only.
*
*******************************************************************************/
static void reap_thrds(void)
{
   printf("reap_thrds: start to reap threads\n");

   pthread_mutex_lock(&cb.mtx);
   cb.quit = TRUE;

   while(cb.cntr > 1) {
      pthread_cond_wait(&cb.cndvar, &cb.mtx);
   }
   
   printf("reap_thrds: all threads harvested\n");
   pthread_mutex_unlock(&cb.mtx);
   return;
} /* reap_thrds() */

