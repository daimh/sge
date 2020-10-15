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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "uti/sge_signal.h"
#include "uti/sge_string.h"

#include "uti/msg_utilib.h"

#if defined WIN32 || defined __CYGWIN__
#   define SIGIOT 6
#   define SIGURG 16
#   define SIGIO 23
#   define SIGVTALRM 26
#   define SIGPROF 27
#   define SIGWINCH 28
#endif

#if defined(ALPHA)
#  undef NSIG
#  define NSIG (SIGUSR2+1)
#endif

const sig_mapT sig_map[] = 
{
   {SGE_SIGHUP, SIGHUP, "HUP"},
   {SGE_SIGINT, SIGINT, "INT"},
   {SGE_SIGQUIT, SIGQUIT, "QUIT"},
   {SGE_SIGILL, SIGILL, "ILL"},
   {SGE_SIGTRAP, SIGTRAP, "TRAP"},
   {SGE_SIGABRT, SIGABRT, "ABRT"},
   {SGE_SIGIOT, SIGIOT, "IOT"},
#ifdef SIGEMT
   {SGE_SIGEMT, SIGEMT, "EMT"},
#else
   {SGE_SIGEMT, SIGUNKNOWN, "EMT"},
#endif
   {SGE_SIGFPE, SIGFPE, "FPE"},
   {SGE_SIGKILL, SIGKILL, "KILL"},
   {SGE_SIGSEGV, SIGSEGV, "SEGV"},
   {SGE_SIGPIPE, SIGPIPE, "PIPE"},
   {SGE_SIGALRM, SIGALRM, "ALRM"},
   {SGE_SIGTERM, SIGTERM, "TERM"},
   {SGE_SIGURG, SIGURG, "URG"},
   {SGE_SIGSTOP, SIGSTOP, "STOP"},
   {SGE_SIGTSTP, SIGTSTP, "TSTP"},
   {SGE_SIGCONT, SIGCONT, "CONT"},
   {SGE_SIGCHLD, SIGCHLD, "CHLD"},
   {SGE_SIGTTIN, SIGTTIN, "TTIN"},
   {SGE_SIGTTOU, SIGTTOU, "TTOU"},
   {SGE_SIGIO, SIGIO, "IO"},
   {SGE_SIGXCPU, SIGXCPU, "XCPU"},
   {SGE_SIGXFSZ, SIGXFSZ, "XFSZ"},
   {SGE_SIGWINCH, SIGWINCH, "WINCH"},
   {SGE_SIGUSR1, SIGUSR1, "USR1"},
   {SGE_SIGUSR2, SIGUSR2, "USR2"},
   {SGE_SIGBUS, SIGBUS, "BUS"},
   {SGE_MIGRATE, SIGTTOU, "MIGRATE"},
   {0, 0, ""}
};

/****** uti/signal/sge_unmap_signal() *****************************************
*  NAME
*     sge_unmap_signal() -- Unmap 32bit SGE/EE signal to system signal 
*
*  SYNOPSIS
*     int sge_unmap_signal(u_long32 sge_sig) 
*
*  FUNCTION
*     Unmap the 32bit SGE/EEsignal to the system specific signal 
*
*  INPUTS
*     u_long32 sge_sig - SGE/EE signal 
*
*  RESULT
*     int - system signal
*
*  NOTES
*     MT-NOTE: sge_unmap_signal() is MT safe
*
******************************************************************************/
int sge_unmap_signal(u_long32 sge_sig) 
{
   const sig_mapT *mapptr=sig_map;

   while (mapptr->sge_sig) {
      if (mapptr->sge_sig == sge_sig) {
         return mapptr->sig;
      }
      mapptr++;
   }
   return -1;
}

/****** uti/signal/sge_map_signal() *******************************************
*  NAME
*     sge_map_signal() -- Map system signal to 32bit SGE/EE signal 
*
*  SYNOPSIS
*     u_long32 sge_map_signal(int sys_sig) 
*
*  FUNCTION
*     Map the system specific signal to the 32bit sge signal 
*
*  INPUTS
*     int sys_sig - system signal 
*
*  RESULT
*     u_long32 - SGE/EE Signal
*
*  NOTES
*     MT-NOTE: sge_map_signal() is MT safe
*
******************************************************************************/
u_long32 sge_map_signal(int sys_sig) 
{
   const sig_mapT *mapptr=sig_map;

   while (mapptr->sge_sig) {
      if (mapptr->sig == sys_sig) {
         return mapptr->sge_sig;
      }
      mapptr++;
   }
   return -1;
}

/****** uti/signal/sge_str2signal() ********************************************
*  NAME
*     str2signal() -- Make an SGE signal out of a string 
*
*  SYNOPSIS
*     u_long32 sge_str2signal(const char *str) 
*
*  FUNCTION
*     Make an sge signal out of a string. 'str' can be the signal name
*     (caseinsensitive) without sig or the signal number.  (Take care
*     numbers are system dependent.)
*
*  INPUTS
*     const char *str - signal string 
*
*  RESULT
*     u_long32 - SGE/EE signal 
*
*  NOTES
*     MT-NOTE: sge_str2signal() is MT safe
*
******************************************************************************/
u_long32 sge_str2signal(const char *str) 
{
   const sig_mapT *mapptr=sig_map;
   u_long32 signum;

   /* look for signal names in mapping table */
   while (mapptr->sge_sig) {
      if (!strcasecmp(str, mapptr->signame)) {
         return mapptr->sge_sig;
      }
      mapptr++;
   }

   /* could not find per name -> look for signal numbers */
   if (sge_strisint(str)) {
      signum = strtol(str, NULL, 10);
      mapptr = sig_map;
      while (mapptr->sge_sig) {
         if ((int) signum ==  mapptr->sig) {
            return mapptr->sge_sig;
         }
         mapptr++;
      }
   }

   return -1;
}

/****** uti/signal/sge_sys_str2signal() ***************************************
*  NAME
*     sge_sys_str2signal() -- Make an SGE signal out of a string 
*
*  SYNOPSIS
*     u_long32 sge_sys_str2signal(const char *str) 
*
*  FUNCTION
*     Make an SGE signal out of a string 
*
*  INPUTS
*     const char *str - signal name 
*
*  RESULT
*     u_long32 - SGE/EE signal
*
*  NOTES
*     MT-NOTE: sge_sys_str2signal() is MT safe
*
******************************************************************************/
u_long32 sge_sys_str2signal(const char *str) 
{
   const sig_mapT *mapptr=sig_map;
   u_long32 signum;

   /* look for signal names in mapping table */
   while (mapptr->sge_sig) {
      if (!strcasecmp(str, mapptr->signame)) {
         return mapptr->sig;
      }
      mapptr++;
   }

   /* could not find per name -> look for signal numbers */
   if (sge_strisint(str)) {
      signum = strtol(str, NULL, 10);
      return signum;
   }

   return -1;
}

/****** uti/signal/sge_sig2str() **********************************************
*  NAME
*     sge_sig2str() -- Make a string out of an SGE signal
*
*  SYNOPSIS
*     const char* sge_sig2str(u_long32 sge_sig) 
*
*  FUNCTION
*     Make a string out of an SGE signal
*
*  INPUTS
*     u_long32 sge_sig - SGE signal
*
*  RESULT
*     const char* - signal string
*
*  NOTES
*     MT-NOTE: sge_sig2str() is MT safe
*
******************************************************************************/
const char *sge_sig2str(u_long32 sge_sig) 
{
   const sig_mapT *mapptr;

   /* look for signal names in mapping table */
   for (mapptr=sig_map; mapptr->sge_sig; mapptr++) {
      if (sge_sig == mapptr->sge_sig) {
         return mapptr->signame;
      }
   }

   return MSG_PROC_UNKNOWNSIGNAL;
}

/****** uti/signal/sge_sys_sig2str() ******************************************
*  NAME
*     sge_sys_sig2str() -- Make a string out of a system signal 
*
*  SYNOPSIS
*     const char* sge_sys_sig2str(u_long32 sys_sig) 
*
*  FUNCTION
*     Make a string out of a system signal.  This isn't the same as strsignal,
*     e.g. giving "ILL" v. "Illegal instruction".
*
*  INPUTS
*     u_long32 sys_sig - system signal 
*
*  RESULT
*     const char* - signal string
*
*  NOTES
*     MT-NOTE: sge_sys_sig2str() is MT safe
*
******************************************************************************/
const char *sge_sys_sig2str(u_long32 sys_sig) 
{
   const sig_mapT *mapptr;

   /* look for signal names in mapping table */
   for (mapptr=sig_map; mapptr->sge_sig; mapptr++) {
      if ((int) sys_sig == mapptr->sig) {
         return mapptr->signame;
      }
   }

   return MSG_PROC_UNKNOWNSIGNAL;
}

/****** uti/signal/sge_set_def_sig_mask() *************************************
*  NAME
*     sge_set_def_sig_mask() -- Set signal mask to default
*
*  SYNOPSIS
*     void sge_set_def_sig_mask(int sig_num, err_func_t err_func)
*
*  FUNCTION
*     Set signal mask to default for all signals except given signal
*
*  INPUTS
*     sigset_t sig_num    - signals which should be ignored
*                           (use sigemptyset and sigaddset to set signals,
*                           if NULL, no signals are ignored)
*     err_func_t err_func - callback function to report errors
*
*  NOTES
*     MT-NOTE: sge_set_def_sig_mask() is MT safe
*
******************************************************************************/
void sge_set_def_sig_mask(sigset_t* sig_num, err_func_t err_func)
{
   int i = 1;
   struct sigaction sig_vec;
   
   while (i < NSIG) {
      /*
       * never set default handler for 
       * SIGKILL and SIGSTOP
       */
      if ((i == SIGKILL) || (i == SIGSTOP)) {
         i++;
         continue;
      }

      /*
       * on HPUX don't set default handler for
       * _SIGRESERVE and SIGDIL
       */
#if defined(HPUX)
      if ((i == _SIGRESERVE) || (i == SIGDIL)) {
         i++;
         continue;
      }
#endif     

      /*
       * never set default handler for signals set
       * in sig_num if not NULL 
       */
      if (sig_num != NULL && sigismember(sig_num, i)) {
         i++;
         continue;
      }

      errno = 0;
      sigemptyset(&sig_vec.sa_mask);
      sig_vec.sa_flags = 0;
      sig_vec.sa_handler = 0;
      sig_vec.sa_handler = SIG_DFL;
      if (sigaction(i, &sig_vec, NULL)) {
         if (err_func) {
            char err_str[256];
            snprintf(err_str, 256, MSG_PROC_SIGACTIONFAILED_IS, i, strerror(errno));
            err_func(err_str);
         }
      }
      i++;
   }
}   

/****** uti/signal/sge_unblock_all_signals() **********************************
*  NAME
*     sge_unblock_all_signals()
*
*  SYNOPSIS
*     void sge_unblock_all_signals(void)
*
*  FUNCTION
*     Allow for all signals.
*
*  NOTES
*     MT-NOTE: sge_unblock_all_signals() is MT safe
*
*******************************************************************************/
void sge_unblock_all_signals(void)
{
   sigset_t sigmask;
   /* unblock all signals */
   /* without this we depend on shell to unblock the signals */
   /* result is that SIGXCPU was not delivered with several shells */
   sigemptyset(&sigmask);
   sigprocmask(SIG_SETMASK, &sigmask, NULL);
}

/****** uti/signal/sge_thread_block_all_signals() *****************************
*  NAME
*     sge_thread_block_all_signals()
*
*  SYNOPSIS
*     int sge_thread_block_all_signals(void)
*
*  FUNCTION
*     Blocks all signals the OS knows for the calling thread.
*
*  OUTPUTS
*    sigset_t *oldsigmask - the sigmask of this thread that was set before
*                           this function was called.
*
*  RETURN VALUES
*     int - 0 if ok,
*           errno if pthread_sigmask failed,
*           1000 if oldsigmask == NULL.
*
*  NOTES
*     MT-NOTE: sge_thread_block_signals() is MT safe
*******************************************************************************/
int sge_thread_block_all_signals(sigset_t *oldsigmask)
{
   sigset_t new_mask;
   int      ret = 1000;

   if (oldsigmask != NULL) {
      sigfillset(&new_mask);
      ret = pthread_sigmask(SIG_BLOCK, &new_mask, oldsigmask);
   }
   return ret;
}
