#ifndef _SGE_THREAD_CONTROL_H_
#define _SGE_THREAD_CONTROL_H_
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

#include <pthread.h>

typedef struct _thread_control_t {
   /* Used to guard 'cond_var' and 'shutdown_started' variable of this structure */
   pthread_mutex_t mutex;

   /* 
    * Used for thread waiting. If a thread wants to wait for shutdown of qmaster it can
    * wait for this condition.
    */
   pthread_cond_t  cond_var;

   /* flag thats indicates that the shutdown process has already started */
   bool shutdown_started;
} thread_control_t;

extern thread_control_t Thread_Control;

bool
sge_thread_has_shutdown_started(void);

void
sge_thread_notify_all_waiting(void);

void
sge_thread_wait_for_signal(void);

#endif

