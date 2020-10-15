#ifndef __SGE_SCHED_THREAD_H
#define __SGE_SCHED_THREAD_H
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

typedef struct {
   lList *host_list;        /* EH_Type */
   lList *queue_list;       /* QU_Type */
   lList *dis_queue_list;   /* QU_Type  contains the queues only for the reservation*/
   lList *all_queue_list;   /* QU_Type */
   lList *job_list;         /* JB_Type */
   lList *centry_list;      /* CE_Type */
   lList *acl_list;         /* US_Type */
   lList *pe_list;          /* PE_Type */
   lList *user_list;        /* UP_Type */
   lList *dept_list;        /* US_Type */
   lList *project_list;     /* UP_Type */
   lList *share_tree;       /* STN_Type */
   lList *ckpt_list;        /* CK_Type */
   lList *running_per_user; /* JC_Type */
   lList *hgrp_list;        /* HGRP_Type */
   lList *rqs_list;         /* RQS_Type */
   lList *ar_list;          /* AR_Type */
   bool monitor_next_run;
} scheduler_all_data_t;

typedef struct {
   pthread_mutex_t mutex;      /* used for mutual exclusion                         */
   pthread_cond_t  cond_var;   /* used for waiting                                  */
   bool            exit;       /* true -> exit event delivery                       */
   bool            triggered;  /* new events addded, a scheduling run is triggered  */
   lList           *new_events; /* the storage for new events                       */
   bool            rebuild_categories;
   bool            new_global_conf;
} scheduler_control_t;

extern scheduler_control_t Scheduler_Control;

#if 0
void st_start_scheduler_thread(pthread_t *thread, const char* name); 
void st_shutdown(pthread_t schedd_thread);
#endif

void st_set_flag_new_global_conf(bool new_value);
bool st_get_flag_new_global_conf(void);

int subscribe_scheduler(sge_evc_class_t *evc, sge_where_what_t *where_what);

void *scheduler_thread(void* anArg);

int scheduler_method(sge_evc_class_t *evc, lList **answer_list, scheduler_all_data_t *lists, lList **order);

#endif /* __SGE_SCHED_THREAD_H */
