#ifndef SGE_SCHEDD_PREPARE_DATA
#define SGE_SCHEDD_PREPARE_DATA
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
#include "sgeobj/sge_object.h"
#include "mir/sge_mirror.h"
#include "cull/cull.h"

typedef struct {
   lCondition *where_queue;
   lCondition *where_queue2;
   lCondition *where_all_queue;
   lCondition *where_cqueue;
   lCondition *where_job;
   lCondition *where_host;
   lCondition *where_dept;
   lCondition *where_acl;
   lCondition *where_jat;

   lEnumeration *what_queue;
   lEnumeration *what_queue2;
   lEnumeration *what_cqueue;
   lEnumeration *what_job;
   lEnumeration *what_host;
   lEnumeration *what_acldept;
   lEnumeration *what_jat;
   lEnumeration *what_pet;
   lEnumeration *what_pe;
} sge_where_what_t;

void 
ensure_valid_what_and_where(sge_where_what_t *where_what);

sge_callback_result
sge_process_project_event_before(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                 sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_schedd_conf_event_before(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                     sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_schedd_conf_event_after(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                    sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_job_event_before(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                             sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_job_event_after(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                            sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_ja_task_event_before(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                 sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_ja_task_event_after(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_global_config_event(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_schedd_monitor_event(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                 sge_event_action action, lListElem *event, void *clientdata);

sge_callback_result
sge_process_userset_event_before(sge_evc_class_t *evc, object_description *object_base,
                                 sge_object_type type, sge_event_action action, 
                                 lListElem *event, void *clientdata);

#endif /* SGE_SCHEDD_PREPARE_DATA */



