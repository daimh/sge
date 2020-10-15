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
#include <signal.h>
#include <stdlib.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"

#include "comm/commlib.h"

#include "sig_handlers.h"

#if defined(SOLARIS)
#   include "sge_smf.h"
#   include "sge_string.h"
#endif

static void sge_terminate(int);
static void sge_sigpipe_handler(int);
static void sge_alarmclock(int dummy);

#ifdef WIN32
#   define SIGURG 16
#   define SIGIO  23
#   define SIGVTALRM 26
#   define SIGPROF 27
#endif

sigset_t default_mask;
sigset_t omask;
sigset_t io_mask;
struct sigaction sigterm_vec, sigterm_ovec;
struct sigaction sigalrm_vec, sigalrm_ovec;
struct sigaction sigcld_pipe_vec, sigcld_pipe_ovec;
volatile int shut_me_down                     = 0;
volatile int sge_sig_handler_dead_children    = 0;
volatile int sge_sig_handler_in_main_loop     = 1;
volatile int sge_sig_handler_sigpipe_received = 0;

/********************************************************/
void sge_setup_sig_handlers(
int me_who 
) {
   DENTER(TOP_LAYER, "sge_setup_sig_handlers");

   /******* set default signal mask *******/
   sigfillset(&default_mask);   /* default mask */
   sigdelset(&default_mask, SIGINT);
   sigdelset(&default_mask, SIGQUIT);
   sigdelset(&default_mask, SIGALRM);
   sigdelset(&default_mask, SIGTERM);
   sigdelset(&default_mask, SIGURG);
   sigdelset(&default_mask, SIGIO);
   sigdelset(&default_mask, SIGABRT);
   sigdelset(&default_mask, SIGILL);
#ifdef SIGBUS
   sigdelset(&default_mask, SIGBUS);
#endif
   sigdelset(&default_mask, SIGSEGV);
   sigdelset(&default_mask, SIGTTIN);
   sigdelset(&default_mask, SIGTTOU);
   sigdelset(&default_mask, SIGFPE);
/* Allow SIGTRAP for debuggin purpose */
   sigdelset(&default_mask, SIGTRAP); 
   sigdelset(&default_mask, SIGVTALRM);
   sigdelset(&default_mask, SIGPROF);

   if ((me_who == QCONF) || 
       (me_who == EXECD) || 
       (me_who == QMASTER) || 
       (me_who == SCHEDD)) {
      sigdelset(&default_mask, SIGCHLD);
#ifdef SIGCLD
      sigdelset(&default_mask, SIGCLD);
#endif
      sigdelset(&default_mask, SIGPIPE);
   }

   sigprocmask(SIG_SETMASK, &default_mask, &omask);
   
   
   /******* define signal mask for io operations *******/
   sigfillset(&io_mask);        
   sigdelset(&io_mask, SIGINT);
   sigdelset(&io_mask, SIGQUIT);
   sigdelset(&io_mask, SIGALRM);
   sigdelset(&io_mask, SIGURG);
   sigdelset(&io_mask, SIGIO);
   sigdelset(&io_mask, SIGABRT);
   sigdelset(&io_mask, SIGILL);
#ifdef SIGBUS
   sigdelset(&io_mask, SIGBUS);
#endif
   sigdelset(&io_mask, SIGSEGV);
   sigdelset(&io_mask, SIGTTIN);
   sigdelset(&io_mask, SIGTTOU);
   sigdelset(&io_mask, SIGFPE);
/* Allow SIGTRAP for debuggin purpose */
   sigdelset(&io_mask, SIGTRAP);
   sigdelset(&io_mask, SIGVTALRM);
   sigdelset(&io_mask, SIGPROF);

   /******* setup signal handler for SIGALRM *******/
   sigalrm_vec.sa_handler = sge_alarmclock;
   sigfillset(&sigalrm_vec.sa_mask);
   sigdelset(&sigalrm_vec.sa_mask, SIGQUIT);
   sigdelset(&sigalrm_vec.sa_mask, SIGABRT);
   sigdelset(&sigalrm_vec.sa_mask, SIGILL);
   sigalrm_vec.sa_flags = 0;
   sigaction(SIGALRM, &sigalrm_vec, &sigalrm_ovec);


   /******** signal handler for SIGTERM and SIGINT *******/
   sigterm_vec.sa_handler = sge_terminate;

   /* block virtually all signals during execution of handler */
   sigfillset(&sigterm_vec.sa_mask);
   sigdelset(&sigterm_vec.sa_mask, SIGABRT);
   sigdelset(&sigterm_vec.sa_mask, SIGILL);

   /* restart interupted system call */
   sigterm_vec.sa_flags = 0;
#ifdef SA_RESTART
   sigcld_pipe_vec.sa_flags = SA_RESTART;
#endif

   sigaction(SIGTERM, &sigterm_vec, &sigterm_ovec);
   sigaction(SIGINT, &sigterm_vec, NULL);


   /******** signal handler for SIGCHLD AND SIGPIPE *******/
   if ((me_who == QCONF) ||
       (me_who == EXECD) || 
       (me_who == QMASTER) ||
       (me_who == SCHEDD)) {
      sigcld_pipe_vec.sa_handler = sge_reap;
      sigfillset(&sigcld_pipe_vec.sa_mask);
      sigdelset(&sigcld_pipe_vec.sa_mask, SIGQUIT);
      sigdelset(&sigcld_pipe_vec.sa_mask, SIGALRM);
      sigdelset(&sigcld_pipe_vec.sa_mask, SIGURG);
      sigdelset(&sigcld_pipe_vec.sa_mask, SIGIO);
      sigdelset(&sigcld_pipe_vec.sa_mask, SIGABRT);
      sigdelset(&sigcld_pipe_vec.sa_mask, SIGILL);

      /* restart interupted system call */
      sigcld_pipe_vec.sa_flags = 0;
#ifdef SA_RESTART
   sigcld_pipe_vec.sa_flags = SA_RESTART;
#endif
      sigaction(SIGCHLD, &sigcld_pipe_vec, &sigcld_pipe_ovec);
      
      sigcld_pipe_vec.sa_handler = sge_sigpipe_handler;
      sigaction(SIGPIPE, &sigcld_pipe_vec, &sigcld_pipe_ovec);
   }

   DEXIT;
   return;
}

/********************************************************/
static void sge_alarmclock(int dummy)
{
   return;
}

/***************************************************************************/
static void sge_terminate(int dummy)
{
   /* set shut-me-down variable */
   shut_me_down = 1;
    
   /* inform commlib to ignore all timeouts */
   cl_com_ignore_timeouts(true);
   
   /* This is not the best way to shut down a process. 
      TODO: remove the exit call, applications should check shut_me_down */
   if (!sge_sig_handler_in_main_loop) {
#if defined(SOLARIS)
      if (sge_smf_used() == 1) {
         /* We don't do disable on svcadm restart */
         if (sge_strnullcmp(sge_smf_get_instance_state(), SCF_STATE_STRING_ONLINE) == 0 &&
             sge_strnullcmp(sge_smf_get_instance_next_state(), SCF_STATE_STRING_NONE) == 0) {      
            sge_smf_temporary_disable_instance();
         }
   }
#endif
      exit(1);
   }
}

/***************************************************************************/
void sge_reap(int dummy)
{
   sge_sig_handler_dead_children = 1;
}

/***************************************************************************/
static void sge_sigpipe_handler(int dummy)
{
   sge_sig_handler_sigpipe_received = 1;
}
