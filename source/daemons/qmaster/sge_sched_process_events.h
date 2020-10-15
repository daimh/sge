#ifndef __SGE_SCHED_PROCESS_EVENTS_H
#define __SGE_SCHED_PROCESS_EVENTS_H
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "evc/sge_event_client.h"
#include "mir/sge_mirror.h"
#include "sge_sched_prepare_data.h"
#include "sge_sched_thread.h"

void st_start_scheduler_thread(pthread_t *thread, const char* name); 
void st_shutdown(pthread_t schedd_thread);

void st_set_flag_new_global_conf(bool new_value);
bool st_get_flag_new_global_conf(void);

int subscribe_scheduler(sge_evc_class_t *evc, sge_where_what_t *where_what);

int sge_before_dispatch(sge_evc_class_t *evc);

void *scheduler_thread(void* anArg);

void event_update_func(u_long32 ec_id, lList **alpp, lList *event_list);

void set_job_flushing(sge_evc_class_t *evc);

#endif /* __SGE_SCHED_PROCESS_EVENTS_H */
