#ifndef _SGE_THREAD_JVM_H_
#define _SGE_THREAD_JVM_H_
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

#ifndef NO_JNI

#include <pthread.h>

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi_packet.h"

typedef struct _master_jvm_class_t master_jvm_class_t;

struct _master_jvm_class_t {
   /* 
    * mutex to gard all members of this structure 
    */
   pthread_mutex_t mutex;

   /* 
    * If the JVM thread can't start the JVM in will run into
    * code which waits for this condition to be signalled. This
    * prevents the JVM thread trying to start the JVM again and again.
    */
   pthread_cond_t  cond_var;

   /* 
    * has the JVM thread been started  
    */
   bool is_running;

   /*
    * next thread id to be used when jvm is restarted
    */
   int thread_id;

   /*
    * use bootstrap info to identify if scheduler should be started (true)
    * or ignore that information (false) 
    */                  
   bool use_bootstrap;

   /*
    * true if shutdown should continue
    */
   bool shutdown_started;
}; 

void
sge_jvm_initialize(sge_gdi_ctx_class_t *ctx, lList **answer_list);

void
sge_jvm_terminate(sge_gdi_ctx_class_t *ctx, lList **answer_list);

#endif

#endif

