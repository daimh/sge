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

#include "sge_thread_jvm.h"

#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "uti/sge_mtutil.h"
#include "uti/sge_rmon.h"
#include "uti/sge_thread_ctrl.h"

#define THREAD_CONTROL_MUTEX "thread_control_mutex"

#define THREAD_LAYER BASIS_LAYER

thread_control_t Thread_Control = {
   PTHREAD_MUTEX_INITIALIZER,
   PTHREAD_COND_INITIALIZER,
   false
};

/****** uti/thread_ctrl/sge_thread_has_shutdown_started() **********************
*  NAME
*     sge_thread_has_shutdown_started() -- shutdown in progress? 
*
*  SYNOPSIS
*     bool sge_thread_has_shutdown_started(void) 
*
*  FUNCTION
*     Service function which can be used to check if the executing
*     component is already shutting down. 
*
*  INPUTS
*     void - NONE
*
*  RESULT
*     bool - is in progress?
*        true  - yes
*        false - no
*
*  NOTES
*     MT-NOTE: sge_thread_has_shutdown_started() is MT safe 
*******************************************************************************/
bool
sge_thread_has_shutdown_started(void)
{
   bool res = false;

   DENTER(THREAD_LAYER, "sge_thread_has_shutdown_started");
   sge_mutex_lock(THREAD_CONTROL_MUTEX, SGE_FUNC, __LINE__, &Thread_Control.mutex);
   res = Thread_Control.shutdown_started;
   sge_mutex_unlock(THREAD_CONTROL_MUTEX, SGE_FUNC, __LINE__, &Thread_Control.mutex);
   DEXIT;
   return res;
}

/****** uti/thread_ctrl/sge_thread_notify_all_waiting() ************************
*  NAME
*     sge_thread_notify_all_waiting() -- notify waiting thread 
*
*  SYNOPSIS
*     void sge_thread_notify_all_waiting(void) 
*
*  FUNCTION
*     After the main thread has initialized all needed components and 
*     threads it waits for a certain condition to be signaled 
*     (sge_thread_wait_for_signal).
*     This signal will start the shutdown process of the master.
*     This function triggers this signal.
*
*  INPUTS
*     void - NONE 
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: sge_thread_notify_all_waiting() is MT safe 
*
*  SEE ALSO
*     uti/thread_ctrl/sge_thread_wait_for_signal()
*******************************************************************************/
void
sge_thread_notify_all_waiting(void)
{
   DENTER(THREAD_LAYER, "sge_thread_notify_all_waiting");

   sge_mutex_lock(THREAD_CONTROL_MUTEX, SGE_FUNC, __LINE__, &Thread_Control.mutex);

   Thread_Control.shutdown_started = true;
   pthread_cond_broadcast(&Thread_Control.cond_var);

   sge_mutex_unlock(THREAD_CONTROL_MUTEX, SGE_FUNC, __LINE__, &Thread_Control.mutex);

   DRETURN_VOID;
}

/****** uti/thread_ctrl/sge_thread_wait_for_signal() ***************************
*  NAME
*     sge_thread_wait_for_signal() -- block current thread till shutdown 
*
*  SYNOPSIS
*     void sge_thread_wait_for_signal(void) 
*
*  FUNCTION
*     A call of this function will block the executing thread until the 
*     shutdown of the process is triggered via sge_thread_notify_all_waiting()
*
*  INPUTS
*     void - NONE
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: sge_thread_wait_for_signal() is not MT safe 
*
*  SEE ALSO
*     uti/thread_ctrl/sge_thread_notify_all_waiting()
*******************************************************************************/
void 
sge_thread_wait_for_signal(void)
{
   DENTER(THREAD_LAYER, "sge_thread_wait_for_signal");

   sge_mutex_lock(THREAD_CONTROL_MUTEX, SGE_FUNC, __LINE__, &Thread_Control.mutex);

   while (Thread_Control.shutdown_started == false) {
      pthread_cond_wait(&Thread_Control.cond_var, &Thread_Control.mutex);
   }

   sge_mutex_unlock(THREAD_CONTROL_MUTEX, SGE_FUNC, __LINE__, &Thread_Control.mutex);

   DEXIT;
   return;
}

