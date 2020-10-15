#ifndef _SGE_THREAD_CONTROL_OLD_H_
#define _SGE_THREAD_CONTROL_OLD_H_
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <pthread.h>

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi_packet.h"

#include "sge_thread_jvm.h"

typedef struct {
   /* exit state: 100 = another master took over */
   int exit_state;

   /* Worker threads: handling incoming "intern GDI requests" */
   cl_raw_list_t *worker_thread_pool;

   /* Message threads: accepting and answering certain commlib requests */
   cl_raw_list_t *listener_thread_pool;

   /* Signal thread */
   cl_raw_list_t *signal_thread_pool;

   /* Timed event thread */
   cl_raw_list_t *timer_thread_pool;

   /* Event event master thread */
   cl_raw_list_t *event_master_thread_pool;

   /* Scheduler thread */
   cl_raw_list_t *scheduler_thread_pool; 

   /* JVM thread */ 
   cl_raw_list_t *jvm_thread_pool;        

   /* intern GDI test thread */ 
   cl_raw_list_t *test_thread_pool;        
} main_control_t;

extern main_control_t Main_Control;

int 
sge_qmaster_shutdown_via_signal_thread(int i);

int
sge_qmaster_get_exit_state(void); 

void
sge_qmaster_set_exit_state(int new_state);

bool
sge_qmaster_do_final_spooling(void);


#endif 

