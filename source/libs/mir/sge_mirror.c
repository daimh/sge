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
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_profiling.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"

#include "cull/cull_list.h"

#include "sgeobj/sge_event.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_cuser.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_host.h"

#include "evc/msg_evclib.h"
#include "mir/msg_mirlib.h"
#include "mir/sge_host_mirror.h"
#include "mir/sge_queue_mirror.h"
#include "mir/sge_job_mirror.h"
#include "mir/sge_ja_task_mirror.h"
#include "mir/sge_pe_task_mirror.h"
#include "mir/sge_sharetree_mirror.h"
#include "mir/sge_sched_conf_mirror.h"
#include "mir/sge_mirror.h"

#include "gdi/sge_gdi_ctx.h"

#include "sig_handlers.h"

/* Datastructure for internal storage of subscription information
 * and callbacks.
 */
typedef struct {
   sge_mirror_callback callback_before;   /* callback before mirroring      */
   sge_mirror_callback callback_default;  /* default mirroring function     */
   sge_mirror_callback callback_after;    /* callback after mirroring       */
   void *clientdata;                      /* client data passed to callback */
} mirror_description;

static sge_mirror_error 
sge_mirror_process_event_list_(sge_evc_class_t *evc, lList *event_list);

#ifdef SOLARIS
#pragma no_inline(sge_mirror_process_event_list,sge_mirror_process_event_list_)
#endif

static sge_mirror_error
_sge_mirror_subscribe(sge_evc_class_t *evc,
                      sge_object_type type, sge_mirror_callback callback_before,
                      sge_mirror_callback callback_after, void *clientdata,
                      const lCondition *where, const lEnumeration *what);

static sge_mirror_error
_sge_mirror_unsubscribe(sge_evc_class_t *evc, sge_object_type type);

static void
sge_mirror_free_list(sge_object_type type);

static sge_mirror_error
sge_mirror_process_event(sge_evc_class_t *evc,
                         mirror_description *mirror_base,
                         object_description *object_base,
                         sge_object_type type,
                         sge_event_action action,
                         lListElem *event);

static sge_callback_result
sge_mirror_process_shutdown(sge_evc_class_t *evc,
                            object_description *object_base,
                            sge_object_type type,
                            sge_event_action action,
                            lListElem *event,
                            void *clientdata);
static sge_callback_result
sge_mirror_process_mark4registration(sge_evc_class_t *evc,
                                     object_description *object_base,
                                     sge_object_type type,
                                     sge_event_action action,
                                     lListElem *event,
                                     void *clientdata);

static sge_callback_result
generic_update_master_list(sge_evc_class_t *evc,
                           object_description *object_base,
                           sge_object_type type,
                           sge_event_action action,
                           lListElem *event,
                           void *clientdata);

static sge_callback_result
ar_update_master_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                       sge_event_action action, lListElem *event, void *clientdata);

static
sge_mirror_error sge_mirror_update_master_list_ar_key(lList **list, const lDescr *list_descr,
                                                      int key_nm, const char *key,
                                                      sge_event_action action, lListElem *event);

/*
 * One entry per event type, this is the basic definition.
 * Each thread will have its own table based on this one. 
 */
static const mirror_description dev_mirror_base[SGE_TYPE_ALL] = {
   /*cbb   cbd                                     cba   cd   */
   { NULL, host_update_master_list,                NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, NULL,                                   NULL, NULL },
   { NULL, host_update_master_list,                NULL, NULL },
   { NULL, ja_task_update_master_list,             NULL, NULL },
   { NULL, pe_task_update_master_list,             NULL, NULL },
   { NULL, job_update_master_list,                 NULL, NULL },
   { NULL, job_schedd_info_update_master_list,     NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, sharetree_update_master_list,           NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, cqueue_update_master_list,              NULL, NULL },
   { NULL, qinstance_update_cqueue_list,           NULL, NULL },
   { NULL, schedd_conf_update_master_list,         NULL, NULL },
   { NULL, NULL,                                   NULL, NULL },
   { NULL, sge_mirror_process_shutdown,            NULL, NULL },
   { NULL, sge_mirror_process_mark4registration,   NULL, NULL },
   { NULL, host_update_master_list,                NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, host_update_master_list,                NULL, NULL }, /*hgroup*/
   { NULL, generic_update_master_list,             NULL, NULL },
   { NULL, generic_update_master_list,             NULL, NULL }, /*zombie*/
   { NULL, generic_update_master_list,             NULL, NULL }, /*suser*/
   { NULL, generic_update_master_list,             NULL, NULL }, /*rqs*/
   { NULL, ar_update_master_list,                  NULL, NULL }, /*advance reservation*/
   { NULL, NULL,                                   NULL, NULL }, /*jobscripts*/
#ifndef __SGE_NO_USERMAPPING__
   { NULL, NULL,                                   NULL, NULL },
#endif
};

/*-------------------------*/
/* multithreading support. */
/*-------------------------*/

typedef struct {
   bool produce_qmaster_alive_timeout; /* used to produce qmaster alive timeout when
                                        * SGE_PRODUCE_ALIVE_TIMEOUT_ERROR environment
                                        * variable is set. */
   mirror_description mirror_base[SGE_TYPE_ALL]; /* subscription handlers */
} mir_state_t;

static pthread_key_t   mir_state_key;
static pthread_once_t  mir_once_control = PTHREAD_ONCE_INIT;

static void mir_state_init(mir_state_t* state)
{
   int i;
   /*
    * if environment varialbe SGE_PRODUCE_ALIVE_TIMEOUT_ERROR
    * is defined, the mirror event client will produce qmaster
    * alive timeout errors and reconnect to qmaster
    */
   if (getenv("SGE_PRODUCE_ALIVE_TIMEOUT_ERROR")) {
      state->produce_qmaster_alive_timeout = true;
   } else {
      state->produce_qmaster_alive_timeout = false;
   }

   /* initialize mirroring data structures - only changeable fields */
   for (i = 0; i < SGE_TYPE_ALL; i++) {
      state->mirror_base[i].callback_before  = NULL;
      state->mirror_base[i].callback_after   = NULL;
      state->mirror_base[i].callback_default = dev_mirror_base[i].callback_default;
      state->mirror_base[i].clientdata       = NULL;
   }
}

static void mir_state_destroy(void* state)
{
   sge_free(&state);
}

static void mir_mt_init(void)
{
   pthread_key_create(&mir_state_key, &mir_state_destroy);
}

static bool mir_get_produce_qmaster_alive_timeout(void)
{
   GET_SPECIFIC(mir_state_t, mir_state, mir_state_init, mir_state_key, "mir_get_produce_qmaster_alive_timeout");
   return mir_state->produce_qmaster_alive_timeout;
}

static mirror_description *mir_get_mirror_base(void)
{
   GET_SPECIFIC(mir_state_t, mir_state, mir_state_init, mir_state_key, "mir_get_mirror_base");
   return mir_state->mirror_base;
}

/*----------------------------*/
/* End multithreading support */
/*----------------------------*/

/****** Eventmirror/sge_mirror_initialize() ********************************************
*  NAME
*     sge_mirror_initialize() -- initialize a process local event mirror interface
*
*  SYNOPSIS
*     sge_mirror_error sge_mirror_initialize(ev_registration_id id,
*                                            const char *name)
*
*  FUNCTION
*     Initializes internal data structures and registers with qmaster
*     using the event client mechanisms.
*
*     Events covering shutdown requests and qmaster shutdown notification
*     are subscribed.
*
*  INPUTS
*     ev_registration_id id - id used to register with qmaster
*     const char *name      - name used to register with qmaster a
*     bool use_global_date  - if that to true, the implemenation is not thread
*                             save anymore. This setting is to ensure, that
*                             old code is still working, without beeing rewritten.
*                             if false is put inhere, the mirror interface is
*                             thread save. The current implementation has a limit,
*                             which is, that only one thread can work on the mirror
*                             data at a time, since it is stored as thread global.
*     event_client_update_func_t - a function which knows on how to handle new events.
*                                  The events are stored by this function and not send
*                                  out.
*
*  RESULT
*     sge_mirror_error - SGE_EM_OK or an error code
*
*  SEE ALSO
*     Eventmirror/sge_mirror_shutdown()
*     Eventclient/-ID-numbers
*******************************************************************************/
sge_mirror_error
sge_mirror_initialize(sge_evc_class_t *evc, ev_registration_id id, const char *name,
                      bool use_global_data, event_client_update_func_t update_func,
                      evm_mod_func_t mod_func, evm_add_func_t add_func,
                      evm_remove_func_t remove_func, evm_ack_func_t ack_func)
{
   DENTER(TOP_LAYER, "sge_mirror_initialize");

   evc->ec_local.update_func = update_func;
   evc->ec_local.mod_func = mod_func;
   evc->ec_local.add_func = add_func;
   evc->ec_local.remove_func = remove_func;
   evc->ec_local.ack_func = ack_func;
   evc->ec_local.init = true;

   pthread_once(&mir_once_control, mir_mt_init);
   obj_init(use_global_data);

   /* subscribe some events with default handling */
   sge_mirror_subscribe(evc, SGE_TYPE_SHUTDOWN, NULL, NULL, NULL, NULL, NULL);
   sge_mirror_subscribe(evc, SGE_TYPE_MARK_4_REGISTRATION, NULL, NULL, NULL, NULL, NULL);

   /* register with qmaster */
   evc->ec_commit(evc, NULL);

   DRETURN(SGE_EM_OK);
}

/****** Eventmirror/sge_mirror_shutdown() ***************************************
*  NAME
*     sge_mirror_shutdown() -- shutdown mirroring
*
*  SYNOPSIS
*     sge_mirror_error sge_mirror_shutdown(void)
*
*  FUNCTION
*     Shuts down the event mirroring mechanism:
*     Unsubscribes all events, deletes contents of the corresponding
*     object lists and deregisteres from qmaster.
*
*  RESULT
*     sge_mirror_error - SGE_EM_OK or error code
*
*  SEE ALSO
*     Eventmirror/sge_mirror_initialize()
*******************************************************************************/
sge_mirror_error sge_mirror_shutdown(sge_evc_class_t *evc)
{
   DENTER(TOP_LAYER, "sge_mirror_shutdown");

   if (evc && evc->ec_is_initialized(evc)) {
      sge_mirror_unsubscribe(evc, SGE_TYPE_ALL);
      evc->ec_deregister(evc);
   }

   DRETURN(SGE_EM_OK);
}

/****** Eventmirror/sge_mirror_subscribe() **************************************
*  NAME
*     sge_mirror_subscribe() -- subscribe certain event types
*
*  SYNOPSIS
*     sge_mirror_error sge_mirror_subscribe(sge_object_type type,
*                                           sge_mirror_callback callback_before,
*                                           sge_mirror_callback callback_after,
*                                           void *clientdata)
*
*  FUNCTION
*     Subscribe a certain event type.
*     Callback functions can be specified, that can be executed before the
*     mirroring action and/or after the mirroring action.
*
*     The corresponding data structures are initialized,
*     the events associated with the event type are subscribed with the
*     event client interface.
*
*  INPUTS
*     lListElem *event client              - event client to work with
*     sge_object_type type                 - event type to subscribe or
*                                           SGE_TYPE_ALL
*     sge_mirror_callback callback_before - callback to be executed before
*                                           mirroring
*     sge_mirror_callback callback_after  - callback to be executed after
*                                           mirroring
*     void *clientdata                    - clientdata to be passed to the
*                                           callback functions
*
*  RESULT
*     sge_mirror_error - SGE_EM_OK or an error code
*
*  SEE ALSO
*     Eventmirror/-Eventmirror-Typedefs
*     Eventclient/-Subscription
*     Eventclient/-Events
*     Eventmirror/sge_mirror_unsubscribe()
*******************************************************************************/
sge_mirror_error sge_mirror_subscribe(sge_evc_class_t *evc,
                                      sge_object_type type,
                                      sge_mirror_callback callback_before,
                                      sge_mirror_callback callback_after,
                                      void *clientdata,
                                      const lCondition *where,
                                      const lEnumeration *what)
{
   sge_mirror_error ret = SGE_EM_OK;

   DENTER(TOP_LAYER, "sge_mirror_subscribe");

   if (type < 0 || type > SGE_TYPE_ALL) {
      ERROR((SGE_EVENT, MSG_MIRROR_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
      DRETURN(SGE_EM_BAD_ARG);
   }

   if (type == SGE_TYPE_ALL) {
      sge_object_type i;
      
      for (i = SGE_TYPE_ADMINHOST; i < SGE_TYPE_ALL; i++) {
         _sge_mirror_subscribe(evc, i, callback_before, callback_after, clientdata, NULL, NULL);
      }
   } else {
      ret = _sge_mirror_subscribe(evc, type, callback_before, callback_after, clientdata, where, what);
   }

   DRETURN(ret);
}

static sge_mirror_error
_sge_mirror_subscribe(sge_evc_class_t *evc,
                      sge_object_type type,
                      sge_mirror_callback callback_before,
                      sge_mirror_callback callback_after,
                      void *clientdata,
                      const lCondition *where, const lEnumeration *what)
{
   lListElem *what_el = lWhatToElem(what);
   lListElem *where_el = lWhereToElem(where);

   /* type already has been checked before */
   switch (type) {
      case SGE_TYPE_ADMINHOST:
         evc->ec_subscribe(evc, sgeE_ADMINHOST_LIST);
         evc->ec_subscribe(evc, sgeE_ADMINHOST_ADD);
         evc->ec_subscribe(evc, sgeE_ADMINHOST_DEL);
         evc->ec_subscribe(evc, sgeE_ADMINHOST_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_ADMINHOST_MOD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_ADMINHOST_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_ADMINHOST_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_ADMINHOST_LIST, what_el, where_el);
         }
         break;
      case SGE_TYPE_CALENDAR:
         evc->ec_subscribe(evc, sgeE_CALENDAR_LIST);
         evc->ec_subscribe(evc, sgeE_CALENDAR_ADD);
         evc->ec_subscribe(evc, sgeE_CALENDAR_DEL);
         evc->ec_subscribe(evc, sgeE_CALENDAR_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_CALENDAR_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CALENDAR_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CALENDAR_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CALENDAR_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_CKPT:
         evc->ec_subscribe(evc, sgeE_CKPT_LIST);
         evc->ec_subscribe(evc, sgeE_CKPT_ADD);
         evc->ec_subscribe(evc, sgeE_CKPT_DEL);
         evc->ec_subscribe(evc, sgeE_CKPT_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_CKPT_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CKPT_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CKPT_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CKPT_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_CENTRY:
         evc->ec_subscribe(evc, sgeE_CENTRY_LIST);
         evc->ec_subscribe(evc, sgeE_CENTRY_ADD);
         evc->ec_subscribe(evc, sgeE_CENTRY_DEL);
         evc->ec_subscribe(evc, sgeE_CENTRY_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_CENTRY_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CENTRY_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CENTRY_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CENTRY_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_CONFIG:
         evc->ec_subscribe(evc, sgeE_CONFIG_LIST);
         evc->ec_subscribe(evc, sgeE_CONFIG_ADD);
         evc->ec_subscribe(evc, sgeE_CONFIG_DEL);
         evc->ec_subscribe(evc, sgeE_CONFIG_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_CONFIG_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CONFIG_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CONFIG_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CONFIG_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_GLOBAL_CONFIG:
         evc->ec_subscribe(evc, sgeE_GLOBAL_CONFIG);
                                         /* regard it as a sort of trigger
                                          * in fact this event is not needed!
                                          * it doesn't even contain the config data!
                                          * it is sent before the CONFIG_MOD event, so
                                          * the receiver cannot even properly react to it!
                                          */
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_GLOBAL_CONFIG, what_el, where_el);
         }
         break;
      case SGE_TYPE_EXECHOST:
         evc->ec_subscribe(evc, sgeE_EXECHOST_LIST);
         evc->ec_subscribe(evc, sgeE_EXECHOST_ADD);
         evc->ec_subscribe(evc, sgeE_EXECHOST_DEL);
         evc->ec_subscribe(evc, sgeE_EXECHOST_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_EXECHOST_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_EXECHOST_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_EXECHOST_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_EXECHOST_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_JATASK:
         evc->ec_subscribe(evc, sgeE_JATASK_ADD);
         evc->ec_subscribe(evc, sgeE_JATASK_DEL);
         evc->ec_subscribe(evc, sgeE_JATASK_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_JATASK_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JATASK_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JATASK_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_PETASK:
         evc->ec_subscribe(evc, sgeE_PETASK_ADD);
         evc->ec_subscribe(evc, sgeE_PETASK_DEL);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_PETASK_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_PETASK_DEL, what_el, where_el);
         }
         break;
      case SGE_TYPE_JOB:
         evc->ec_subscribe(evc, sgeE_JOB_LIST);
         evc->ec_subscribe(evc, sgeE_JOB_ADD);
         evc->ec_subscribe(evc, sgeE_JOB_DEL);
         evc->ec_subscribe(evc, sgeE_JOB_MOD);
         evc->ec_subscribe(evc, sgeE_JOB_MOD_SCHED_PRIORITY);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_JOB_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JOB_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JOB_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JOB_MOD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JOB_MOD_SCHED_PRIORITY, what_el, where_el);
         }
         /* TODO: SG: what and where for usage */
         evc->ec_subscribe(evc, sgeE_JOB_USAGE);
         evc->ec_subscribe(evc, sgeE_JOB_FINAL_USAGE);
         break;
      case SGE_TYPE_JOB_SCHEDD_INFO:
         evc->ec_subscribe(evc, sgeE_JOB_SCHEDD_INFO_LIST);
         evc->ec_subscribe(evc, sgeE_JOB_SCHEDD_INFO_ADD);
         evc->ec_subscribe(evc, sgeE_JOB_SCHEDD_INFO_DEL);
         evc->ec_subscribe(evc, sgeE_JOB_SCHEDD_INFO_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_JOB_SCHEDD_INFO_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JOB_SCHEDD_INFO_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JOB_SCHEDD_INFO_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_JOB_SCHEDD_INFO_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_MANAGER:
         evc->ec_subscribe(evc, sgeE_MANAGER_LIST);
         evc->ec_subscribe(evc, sgeE_MANAGER_ADD);
         evc->ec_subscribe(evc, sgeE_MANAGER_DEL);
         evc->ec_subscribe(evc, sgeE_MANAGER_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_MANAGER_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_MANAGER_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_MANAGER_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_MANAGER_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_OPERATOR:
         evc->ec_subscribe(evc, sgeE_OPERATOR_LIST);
         evc->ec_subscribe(evc, sgeE_OPERATOR_ADD);
         evc->ec_subscribe(evc, sgeE_OPERATOR_DEL);
         evc->ec_subscribe(evc, sgeE_OPERATOR_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_OPERATOR_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_OPERATOR_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_OPERATOR_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_OPERATOR_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_SHARETREE:
         evc->ec_subscribe(evc, sgeE_NEW_SHARETREE);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_NEW_SHARETREE, what_el, where_el);
         }
         break;
      case SGE_TYPE_PE:
         evc->ec_subscribe(evc, sgeE_PE_LIST);
         evc->ec_subscribe(evc, sgeE_PE_ADD);
         evc->ec_subscribe(evc, sgeE_PE_DEL);
         evc->ec_subscribe(evc, sgeE_PE_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_PE_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_PE_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_PE_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_PE_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_PROJECT:
         evc->ec_subscribe(evc, sgeE_PROJECT_LIST);
         evc->ec_subscribe(evc, sgeE_PROJECT_ADD);
         evc->ec_subscribe(evc, sgeE_PROJECT_DEL);
         evc->ec_subscribe(evc, sgeE_PROJECT_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_PROJECT_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_PROJECT_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_PROJECT_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_PROJECT_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_CQUEUE:
         evc->ec_subscribe(evc, sgeE_CQUEUE_LIST);
         evc->ec_subscribe(evc, sgeE_CQUEUE_ADD);
         evc->ec_subscribe(evc, sgeE_CQUEUE_DEL);
         evc->ec_subscribe(evc, sgeE_CQUEUE_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_CQUEUE_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CQUEUE_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CQUEUE_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CQUEUE_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_QINSTANCE:
         evc->ec_subscribe(evc, sgeE_QINSTANCE_ADD);
         evc->ec_subscribe(evc, sgeE_QINSTANCE_DEL);
         evc->ec_subscribe(evc, sgeE_QINSTANCE_MOD);
         evc->ec_subscribe(evc, sgeE_QINSTANCE_SOS);
         evc->ec_subscribe(evc, sgeE_QINSTANCE_USOS);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_QINSTANCE_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_QINSTANCE_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_QINSTANCE_MOD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_QINSTANCE_SOS, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_QINSTANCE_USOS, what_el, where_el);
         }
         break;
      case SGE_TYPE_SCHEDD_CONF:
         evc->ec_subscribe(evc, sgeE_SCHED_CONF);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_SCHED_CONF, what_el, where_el);
         }
         break;
      case SGE_TYPE_SCHEDD_MONITOR:
         evc->ec_subscribe(evc, sgeE_SCHEDDMONITOR);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_SCHEDDMONITOR, what_el, where_el);
         }
         break;
      case SGE_TYPE_SHUTDOWN:
         evc->ec_subscribe_flush(evc, sgeE_SHUTDOWN, 0);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_SHUTDOWN, what_el, where_el);
         }
         break;
      case SGE_TYPE_MARK_4_REGISTRATION:
         evc->ec_subscribe_flush(evc, sgeE_QMASTER_GOES_DOWN, 0);
         evc->ec_subscribe_flush(evc, sgeE_ACK_TIMEOUT, 0);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_QMASTER_GOES_DOWN, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_ACK_TIMEOUT, what_el, where_el);
         }
         break;
      case SGE_TYPE_SUBMITHOST:
         evc->ec_subscribe(evc, sgeE_SUBMITHOST_LIST);
         evc->ec_subscribe(evc, sgeE_SUBMITHOST_ADD);
         evc->ec_subscribe(evc, sgeE_SUBMITHOST_DEL);
         evc->ec_subscribe(evc, sgeE_SUBMITHOST_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_SUBMITHOST_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_SUBMITHOST_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_SUBMITHOST_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_SUBMITHOST_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_USER:
         evc->ec_subscribe(evc, sgeE_USER_LIST);
         evc->ec_subscribe(evc, sgeE_USER_ADD);
         evc->ec_subscribe(evc, sgeE_USER_DEL);
         evc->ec_subscribe(evc, sgeE_USER_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_USER_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_USER_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_USER_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_USER_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_USERSET:
         evc->ec_subscribe(evc, sgeE_USERSET_LIST);
         evc->ec_subscribe(evc, sgeE_USERSET_ADD);
         evc->ec_subscribe(evc, sgeE_USERSET_DEL);
         evc->ec_subscribe(evc, sgeE_USERSET_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_USERSET_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_USERSET_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_USERSET_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_USERSET_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_HGROUP:
         evc->ec_subscribe(evc, sgeE_HGROUP_LIST);
         evc->ec_subscribe(evc, sgeE_HGROUP_ADD);
         evc->ec_subscribe(evc, sgeE_HGROUP_DEL);
         evc->ec_subscribe(evc, sgeE_HGROUP_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_HGROUP_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_HGROUP_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_HGROUP_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_HGROUP_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_RQS:
         evc->ec_subscribe(evc, sgeE_RQS_LIST);
         evc->ec_subscribe(evc, sgeE_RQS_ADD);
         evc->ec_subscribe(evc, sgeE_RQS_DEL);
         evc->ec_subscribe(evc, sgeE_RQS_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_RQS_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_RQS_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_RQS_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_RQS_MOD, what_el, where_el);
         }
         break;
#ifndef __SGE_NO_USERMAPPING__
      case SGE_TYPE_CUSER:
         evc->ec_subscribe(evc, sgeE_CUSER_LIST);
         evc->ec_subscribe(evc, sgeE_CUSER_ADD);
         evc->ec_subscribe(evc, sgeE_CUSER_DEL);
         evc->ec_subscribe(evc, sgeE_CUSER_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_CUSER_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CUSER_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CUSER_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_CUSER_MOD, what_el, where_el);
         }
         break;
#endif
      case SGE_TYPE_ZOMBIE:
            return SGE_EM_NOT_INITIALIZED;

      case SGE_TYPE_SUSER:
            return SGE_EM_NOT_INITIALIZED;
      case SGE_TYPE_AR:
         evc->ec_subscribe(evc, sgeE_AR_LIST);
         evc->ec_subscribe(evc, sgeE_AR_ADD);
         evc->ec_subscribe(evc, sgeE_AR_DEL);
         evc->ec_subscribe(evc, sgeE_AR_MOD);
         if (what_el && where_el) {
            evc->ec_mod_subscription_where(evc, sgeE_AR_LIST, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_AR_ADD, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_AR_DEL, what_el, where_el);
            evc->ec_mod_subscription_where(evc, sgeE_AR_MOD, what_el, where_el);
         }
         break;
      case SGE_TYPE_JOBSCRIPT:
            return SGE_EM_NOT_INITIALIZED;
      default:
         return SGE_EM_BAD_ARG;
   }

   { /* install callback function */
      mirror_description *mirror_base = mir_get_mirror_base();

      mirror_base[type].callback_before = callback_before;
      mirror_base[type].callback_after  = callback_after;
      mirror_base[type].clientdata      = clientdata;
   }

   lFreeElem(&where_el);
   lFreeElem(&what_el);

   return SGE_EM_OK;
}

/****** Eventmirror/sge_mirror_unsubscribe() ************************************
*  NAME
*     sge_mirror_unsubscribe() -- unsubscribe event types
*
*  SYNOPSIS
*     sge_mirror_error sge_mirror_unsubscribe(sge_object_type type)
*
*  FUNCTION
*     Unsubscribes a certain event type or all if SGE_TYPE_ALL is given as type.
*
*     Unsubscribes the corresponding events in the underlying event client
*     interface and frees data stored in the corresponding mirrored list(s).
*  INPUTS
*     sge_object_type type - the event type to unsubscribe or SGE_TYPE_ALL
*
*  RESULT
*     sge_mirror_error - SGE_EM_OK or an error code
*
*  SEE ALSO
*     Eventmirror/-Eventmirror-Typedefs
*     Eventclient/-Subscription
*     Eventclient/-Events
*     Eventmirror/sge_mirror_subscribe()
*******************************************************************************/
sge_mirror_error sge_mirror_unsubscribe(sge_evc_class_t *evc, sge_object_type type)
{
   sge_mirror_error ret = SGE_EM_OK;
   DENTER(TOP_LAYER, "sge_mirror_unsubscribe");

   if (type < 0 || type > SGE_TYPE_ALL) {
      ERROR((SGE_EVENT, MSG_MIRROR_INVALID_OBJECT_TYPE_SI, SGE_FUNC, type));
      DRETURN(SGE_EM_BAD_ARG);
   }

   if (type == SGE_TYPE_ALL) {
      sge_object_type i;

      for (i = SGE_TYPE_ADMINHOST; i < SGE_TYPE_ALL; i++) {
         if (i != SGE_TYPE_SHUTDOWN && i != SGE_TYPE_MARK_4_REGISTRATION) {
            _sge_mirror_unsubscribe(evc, i);
         }
      }
   } else {
      ret = _sge_mirror_unsubscribe(evc, type);
   }

   DRETURN(ret);
}

static sge_mirror_error _sge_mirror_unsubscribe(sge_evc_class_t *evc, sge_object_type type)
{
   mirror_description *mirror_base = mir_get_mirror_base();
 
   DENTER(TOP_LAYER, "_sge_mirror_unsubscribe");
 
   /* type has been checked in calling function - clear callback information */
   mirror_base[type].callback_before  = NULL;
   mirror_base[type].callback_after   = NULL;
   mirror_base[type].clientdata       = NULL;

   switch (type) {
      case SGE_TYPE_ADMINHOST:
         evc->ec_unsubscribe(evc, sgeE_ADMINHOST_LIST);
         evc->ec_unsubscribe(evc, sgeE_ADMINHOST_ADD);
         evc->ec_unsubscribe(evc, sgeE_ADMINHOST_DEL);
         evc->ec_unsubscribe(evc, sgeE_ADMINHOST_MOD);
         break;
      case SGE_TYPE_CALENDAR:
         evc->ec_unsubscribe(evc, sgeE_CALENDAR_LIST);
         evc->ec_unsubscribe(evc, sgeE_CALENDAR_ADD);
         evc->ec_unsubscribe(evc, sgeE_CALENDAR_DEL);
         evc->ec_unsubscribe(evc, sgeE_CALENDAR_MOD);
         break;
      case SGE_TYPE_CKPT:
         evc->ec_unsubscribe(evc, sgeE_CKPT_LIST);
         evc->ec_unsubscribe(evc, sgeE_CKPT_ADD);
         evc->ec_unsubscribe(evc, sgeE_CKPT_DEL);
         evc->ec_unsubscribe(evc, sgeE_CKPT_MOD);
         break;
      case SGE_TYPE_CENTRY:
         evc->ec_unsubscribe(evc, sgeE_CENTRY_LIST);
         evc->ec_unsubscribe(evc, sgeE_CENTRY_ADD);
         evc->ec_unsubscribe(evc, sgeE_CENTRY_DEL);
         evc->ec_unsubscribe(evc, sgeE_CENTRY_MOD);
         break;
      case SGE_TYPE_CONFIG:
         evc->ec_unsubscribe(evc, sgeE_CONFIG_LIST);
         evc->ec_unsubscribe(evc, sgeE_CONFIG_ADD);
         evc->ec_unsubscribe(evc, sgeE_CONFIG_DEL);
         evc->ec_unsubscribe(evc, sgeE_CONFIG_MOD);
         break;
      case SGE_TYPE_GLOBAL_CONFIG:
         evc->ec_unsubscribe(evc, sgeE_GLOBAL_CONFIG);
         break;
      case SGE_TYPE_EXECHOST:
         evc->ec_unsubscribe(evc, sgeE_EXECHOST_LIST);
         evc->ec_unsubscribe(evc, sgeE_EXECHOST_ADD);
         evc->ec_unsubscribe(evc, sgeE_EXECHOST_DEL);
         evc->ec_unsubscribe(evc, sgeE_EXECHOST_MOD);
         break;
      case SGE_TYPE_JATASK:
         evc->ec_unsubscribe(evc, sgeE_JATASK_ADD);
         evc->ec_unsubscribe(evc, sgeE_JATASK_DEL);
         evc->ec_unsubscribe(evc, sgeE_JATASK_MOD);
         break;
      case SGE_TYPE_PETASK:
         evc->ec_unsubscribe(evc, sgeE_PETASK_ADD);
         evc->ec_unsubscribe(evc, sgeE_PETASK_DEL);
         break;
      case SGE_TYPE_JOB:
         evc->ec_unsubscribe(evc, sgeE_JOB_LIST);
         evc->ec_unsubscribe(evc, sgeE_JOB_ADD);
         evc->ec_unsubscribe(evc, sgeE_JOB_DEL);
         evc->ec_unsubscribe(evc, sgeE_JOB_MOD);
         evc->ec_unsubscribe(evc, sgeE_JOB_MOD_SCHED_PRIORITY);
         evc->ec_unsubscribe(evc, sgeE_JOB_USAGE);
         evc->ec_unsubscribe(evc, sgeE_JOB_FINAL_USAGE);
/*          evc->ec_unsubscribe(evc, sgeE_JOB_FINISH); */
         break;
      case SGE_TYPE_JOB_SCHEDD_INFO:
         evc->ec_unsubscribe(evc, sgeE_JOB_SCHEDD_INFO_LIST);
         evc->ec_unsubscribe(evc, sgeE_JOB_SCHEDD_INFO_ADD);
         evc->ec_unsubscribe(evc, sgeE_JOB_SCHEDD_INFO_DEL);
         evc->ec_unsubscribe(evc, sgeE_JOB_SCHEDD_INFO_MOD);
         break;
      case SGE_TYPE_MANAGER:
         evc->ec_unsubscribe(evc, sgeE_MANAGER_LIST);
         evc->ec_unsubscribe(evc, sgeE_MANAGER_ADD);
         evc->ec_unsubscribe(evc, sgeE_MANAGER_DEL);
         evc->ec_unsubscribe(evc, sgeE_MANAGER_MOD);
         break;
      case SGE_TYPE_OPERATOR:
         evc->ec_unsubscribe(evc, sgeE_OPERATOR_LIST);
         evc->ec_unsubscribe(evc, sgeE_OPERATOR_ADD);
         evc->ec_unsubscribe(evc, sgeE_OPERATOR_DEL);
         evc->ec_unsubscribe(evc, sgeE_OPERATOR_MOD);
         break;
      case SGE_TYPE_SHARETREE:
         evc->ec_unsubscribe(evc, sgeE_NEW_SHARETREE);
         break;
      case SGE_TYPE_PE:
         evc->ec_unsubscribe(evc, sgeE_PE_LIST);
         evc->ec_unsubscribe(evc, sgeE_PE_ADD);
         evc->ec_unsubscribe(evc, sgeE_PE_DEL);
         evc->ec_unsubscribe(evc, sgeE_PE_MOD);
         break;
      case SGE_TYPE_PROJECT:
         evc->ec_unsubscribe(evc, sgeE_PROJECT_LIST);
         evc->ec_unsubscribe(evc, sgeE_PROJECT_ADD);
         evc->ec_unsubscribe(evc, sgeE_PROJECT_DEL);
         evc->ec_unsubscribe(evc, sgeE_PROJECT_MOD);
         break;
      case SGE_TYPE_CQUEUE:
         evc->ec_unsubscribe(evc, sgeE_CQUEUE_LIST);
         evc->ec_unsubscribe(evc, sgeE_CQUEUE_ADD);
         evc->ec_unsubscribe(evc, sgeE_CQUEUE_DEL);
         evc->ec_unsubscribe(evc, sgeE_CQUEUE_MOD);
         break;
      case SGE_TYPE_QINSTANCE:
         evc->ec_unsubscribe(evc, sgeE_QINSTANCE_ADD);
         evc->ec_unsubscribe(evc, sgeE_QINSTANCE_DEL);
         evc->ec_unsubscribe(evc, sgeE_QINSTANCE_MOD);
         evc->ec_unsubscribe(evc, sgeE_QINSTANCE_SOS);
         evc->ec_unsubscribe(evc, sgeE_QINSTANCE_USOS);
         break;
      case SGE_TYPE_SCHEDD_CONF:
         evc->ec_unsubscribe(evc, sgeE_SCHED_CONF);
         break;
      case SGE_TYPE_SCHEDD_MONITOR:
         evc->ec_unsubscribe(evc, sgeE_SCHEDDMONITOR);
         break;
      case SGE_TYPE_SHUTDOWN:
         ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_HAVETOHANDLEEVENTS));
         break;
      case SGE_TYPE_MARK_4_REGISTRATION:
         ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_HAVETOHANDLEEVENTS));
         break;
      case SGE_TYPE_SUBMITHOST:
         evc->ec_unsubscribe(evc, sgeE_SUBMITHOST_LIST);
         evc->ec_unsubscribe(evc, sgeE_SUBMITHOST_ADD);
         evc->ec_unsubscribe(evc, sgeE_SUBMITHOST_DEL);
         evc->ec_unsubscribe(evc, sgeE_SUBMITHOST_MOD);
         break;
      case SGE_TYPE_USER:
         evc->ec_unsubscribe(evc, sgeE_USER_LIST);
         evc->ec_unsubscribe(evc, sgeE_USER_ADD);
         evc->ec_unsubscribe(evc, sgeE_USER_DEL);
         evc->ec_unsubscribe(evc, sgeE_USER_MOD);
         break;
      case SGE_TYPE_USERSET:
         evc->ec_unsubscribe(evc, sgeE_USERSET_LIST);
         evc->ec_unsubscribe(evc, sgeE_USERSET_ADD);
         evc->ec_unsubscribe(evc, sgeE_USERSET_DEL);
         evc->ec_unsubscribe(evc, sgeE_USERSET_MOD);
         break;
      case SGE_TYPE_HGROUP:
         evc->ec_unsubscribe(evc, sgeE_HGROUP_LIST);
         evc->ec_unsubscribe(evc, sgeE_HGROUP_ADD);
         evc->ec_unsubscribe(evc, sgeE_HGROUP_DEL);
         evc->ec_unsubscribe(evc, sgeE_HGROUP_MOD);
         break;
      case SGE_TYPE_RQS:
         evc->ec_unsubscribe(evc, sgeE_RQS_LIST);
         evc->ec_unsubscribe(evc, sgeE_RQS_ADD);
         evc->ec_unsubscribe(evc, sgeE_RQS_DEL);
         evc->ec_unsubscribe(evc, sgeE_RQS_MOD);
         break;
#ifndef __SGE_NO_USERMAPPING__
      case SGE_TYPE_CUSER:
         evc->ec_unsubscribe(evc, sgeE_CUSER_LIST);
         evc->ec_unsubscribe(evc, sgeE_CUSER_ADD);
         evc->ec_unsubscribe(evc, sgeE_CUSER_DEL);
         evc->ec_unsubscribe(evc, sgeE_CUSER_MOD);
         break;
#endif
      case SGE_TYPE_ZOMBIE:
            DRETURN(SGE_EM_NOT_INITIALIZED);
      case SGE_TYPE_SUSER:
            DRETURN(SGE_EM_NOT_INITIALIZED);
      case SGE_TYPE_AR:
         evc->ec_unsubscribe(evc, sgeE_AR_LIST);
         evc->ec_unsubscribe(evc, sgeE_AR_ADD);
         evc->ec_unsubscribe(evc, sgeE_AR_DEL);
         evc->ec_unsubscribe(evc, sgeE_AR_MOD);
         break;
      case SGE_TYPE_JOBSCRIPT:
            DRETURN(SGE_EM_NOT_INITIALIZED);
     default:
         ERROR((SGE_EVENT, "received invalid event group %d", type));
         DRETURN(SGE_EM_BAD_ARG);
   }

   /* clear data */
   sge_mirror_free_list(type);

   DRETURN(SGE_EM_OK);
}

/****** Eventmirror/sge_mirror_process_events() *********************************
*  NAME
*     sge_mirror_process_events() -- retrieve and process events
*
*  SYNOPSIS
*     sge_mirror_error sge_mirror_process_events(void)
*
*  FUNCTION
*     Retrieves new events from qmaster.
*     If new events have arrived from qmaster, they are processed,
*     that means, for each event
*     - if installed, a "before mirroring" callback is called
*     - the event is mirrored into the corresponding master list
*     - if installed, a "after mirroring" callback is called
*
*     If retrieving new events from qmaster fails over a time period
*     of 10 times the configured event delivery interval (see event
*     client interface, function ec_get_edtime), a timeout warning
*     is generated and a new registration of the event client is
*     prepared.
*
*  RESULT
*     sge_mirror_error - SGE_EM_OK or an error code
*
*  SEE ALSO
*     Eventmirror/--Eventmirror
*     Eventclient/Client/ec_get_edtime()
*     Eventclient/Client/ec_set_edtime()
*******************************************************************************/
sge_mirror_error sge_mirror_process_events(sge_evc_class_t *evc)
{
   lList *event_list = NULL;
   sge_mirror_error ret = SGE_EM_OK;
   static int test_debug = 0;

   DENTER(TOP_LAYER, "sge_mirror_process_events");

   if (evc && evc->ec_get(evc, &event_list, false)) {
      if (event_list != NULL) {
         ret = sge_mirror_process_event_list(evc, event_list);
         lFreeList(&event_list);
      }
   } else {
      WARNING((SGE_EVENT, SFNMAX, MSG_MIRROR_QMASTERALIVETIMEOUTEXPIRED));
      evc->ec_mark4registration(evc);
      ret = SGE_EM_TIMEOUT;
   }

   if (mir_get_produce_qmaster_alive_timeout()) {
      test_debug++;
      if (test_debug > 3) {
         test_debug = 0;
         WARNING((SGE_EVENT, SFNMAX, MSG_MIRROR_QMASTERALIVETIMEOUTEXPIRED));
         evc->ec_mark4registration(evc);
         ret = SGE_EM_TIMEOUT;
      }
   }

   DRETURN(ret);
}

/****** Eventmirror/sge_mirror_strerror() ***************************************
*  NAME
*     sge_mirror_strerror() -- map errorcode to error message 
*
*  SYNOPSIS
*     const char* sge_mirror_strerror(sge_mirror_error num) 
*
*  FUNCTION
*     Returns a string describing a given error number.
*     This function can be used to output error messages
*     if a function of the event mirror interface fails.
*
*  INPUTS
*     sge_mirror_error num - error number
*
*  RESULT
*     const char* - corresponding error message
*
*  SEE ALSO
*     Eventmirror/-Eventmirror-Typedefs
*******************************************************************************/
const char *sge_mirror_strerror(sge_mirror_error num)
{
   switch (num) {
      case SGE_EM_OK:
         return MSG_MIRROR_OK;
      case SGE_EM_NOT_INITIALIZED:
         return MSG_MIRROR_NOTINITIALIZED;
      case SGE_EM_BAD_ARG:
         return MSG_MIRROR_BADARG;
      case SGE_EM_TIMEOUT:
         return MSG_MIRROR_TIMEOUT;
      case SGE_EM_DUPLICATE_KEY:
         return MSG_MIRROR_DUPLICATEKEY;
      case SGE_EM_KEY_NOT_FOUND:
         return MSG_MIRROR_KEYNOTFOUND;
      case SGE_EM_CALLBACK_FAILED:
         return MSG_MIRROR_CALLBACKFAILED;
      case SGE_EM_PROCESS_ERRORS:
         return MSG_MIRROR_PROCESSERRORS;
      default:
         return "";
   }
}

static void sge_mirror_free_list(sge_object_type type)
{
   object_type_free_master_list(type);
}

static sge_mirror_error
sge_mirror_process_event_list_(sge_evc_class_t *evc, lList *event_list)
{
   lListElem *event = NULL;
   sge_mirror_error function_ret;
   bool no_more_events=false;
   int num_events = 0;
   mirror_description *mirror_base = mir_get_mirror_base();
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "sge_mirror_process_event_list");

   PROF_START_MEASUREMENT(SGE_PROF_MIRROR);

   function_ret = SGE_EM_OK;

   for_each(event, event_list) {
      sge_mirror_error ret = SGE_EM_OK;
      if (no_more_events) {
         break;
      }
      num_events++;

      switch (lGetUlong(event, ET_type)) {
         case sgeE_ADMINHOST_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_ADMINHOST, SGE_EMA_LIST, event);
            break;
         case sgeE_ADMINHOST_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_ADMINHOST, SGE_EMA_ADD, event);
            break;
         case sgeE_ADMINHOST_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_ADMINHOST, SGE_EMA_DEL, event);
            break;
         case sgeE_ADMINHOST_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_ADMINHOST, SGE_EMA_MOD, event);
            break;

         case sgeE_CALENDAR_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CALENDAR, SGE_EMA_LIST, event);
            break;
         case sgeE_CALENDAR_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CALENDAR, SGE_EMA_ADD, event);
            break;
         case sgeE_CALENDAR_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CALENDAR, SGE_EMA_DEL, event);
            break;
         case sgeE_CALENDAR_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CALENDAR, SGE_EMA_MOD, event);
            break;

         case sgeE_CKPT_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CKPT, SGE_EMA_LIST, event);
            break;
         case sgeE_CKPT_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CKPT, SGE_EMA_ADD, event);
            break;
         case sgeE_CKPT_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CKPT, SGE_EMA_DEL, event);
            break;
         case sgeE_CKPT_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CKPT, SGE_EMA_MOD, event);
            break;

         case sgeE_CENTRY_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CENTRY, SGE_EMA_LIST, event);
            break;
         case sgeE_CENTRY_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CENTRY, SGE_EMA_ADD, event);
            break;
         case sgeE_CENTRY_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CENTRY, SGE_EMA_DEL, event);
            break;
         case sgeE_CENTRY_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CENTRY, SGE_EMA_MOD, event);
            break;

         case sgeE_CONFIG_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CONFIG, SGE_EMA_LIST, event);
            break;
         case sgeE_CONFIG_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CONFIG, SGE_EMA_ADD, event);
            break;
         case sgeE_CONFIG_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CONFIG, SGE_EMA_DEL, event);
            break;
         case sgeE_CONFIG_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CONFIG, SGE_EMA_MOD, event);
            break;

         case sgeE_EXECHOST_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_EXECHOST, SGE_EMA_LIST, event);
            break;
         case sgeE_EXECHOST_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_EXECHOST, SGE_EMA_ADD, event);
            break;
         case sgeE_EXECHOST_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_EXECHOST, SGE_EMA_DEL, event);
            break;
         case sgeE_EXECHOST_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_EXECHOST, SGE_EMA_MOD, event);
            break;

         case sgeE_GLOBAL_CONFIG:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_GLOBAL_CONFIG, SGE_EMA_MOD, event);
            break;

         case sgeE_JATASK_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JATASK, SGE_EMA_ADD, event);
            break;
         case sgeE_JATASK_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JATASK, SGE_EMA_DEL, event);
            break;
         case sgeE_JATASK_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JATASK, SGE_EMA_MOD, event);
            break;

         case sgeE_PETASK_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PETASK, SGE_EMA_ADD, event);
            break;
         case sgeE_PETASK_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PETASK, SGE_EMA_DEL, event);
            break;

         case sgeE_JOB_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB, SGE_EMA_LIST, event);
            break;
         case sgeE_JOB_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB, SGE_EMA_ADD, event);
            break;
         case sgeE_JOB_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB, SGE_EMA_DEL, event);
            break;
         case sgeE_JOB_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB, SGE_EMA_MOD, event);
            break;
         case sgeE_JOB_MOD_SCHED_PRIORITY:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB, SGE_EMA_MOD, event);
            break;
         case sgeE_JOB_USAGE:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB, SGE_EMA_MOD, event);
            break;
         case sgeE_JOB_FINAL_USAGE:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB, SGE_EMA_MOD, event);
            break;
#if 0
         case sgeE_JOB_FINISH:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB, SGE_EMA_MOD, event);
            break;
#endif

         case sgeE_JOB_SCHEDD_INFO_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB_SCHEDD_INFO, SGE_EMA_LIST, event);
            break;
         case sgeE_JOB_SCHEDD_INFO_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB_SCHEDD_INFO, SGE_EMA_ADD, event);
            break;
         case sgeE_JOB_SCHEDD_INFO_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB_SCHEDD_INFO, SGE_EMA_DEL, event);
            break;
         case sgeE_JOB_SCHEDD_INFO_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_JOB_SCHEDD_INFO, SGE_EMA_MOD, event);
            break;

         case sgeE_MANAGER_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_MANAGER, SGE_EMA_LIST, event);
            break;
         case sgeE_MANAGER_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_MANAGER, SGE_EMA_ADD, event);
            break;
         case sgeE_MANAGER_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_MANAGER, SGE_EMA_DEL, event);
            break;
         case sgeE_MANAGER_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_MANAGER, SGE_EMA_MOD, event);
            break;

         case sgeE_OPERATOR_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_OPERATOR, SGE_EMA_LIST, event);
            break;
         case sgeE_OPERATOR_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_OPERATOR, SGE_EMA_ADD, event);
            break;
         case sgeE_OPERATOR_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_OPERATOR, SGE_EMA_DEL, event);
            break;
         case sgeE_OPERATOR_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_OPERATOR, SGE_EMA_MOD, event);
            break;

         case sgeE_NEW_SHARETREE:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_SHARETREE, SGE_EMA_LIST, event);
            break;

         case sgeE_PE_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PE, SGE_EMA_LIST, event);
            break;
         case sgeE_PE_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PE, SGE_EMA_ADD, event);
            break;
         case sgeE_PE_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PE, SGE_EMA_DEL, event);
            break;
         case sgeE_PE_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PE, SGE_EMA_MOD, event);
            break;

         case sgeE_PROJECT_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PROJECT, SGE_EMA_LIST, event);
            break;
         case sgeE_PROJECT_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PROJECT, SGE_EMA_ADD, event);
            break;
         case sgeE_PROJECT_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PROJECT, SGE_EMA_DEL, event);
            break;
         case sgeE_PROJECT_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_PROJECT, SGE_EMA_MOD, event);
            break;

         case sgeE_QMASTER_GOES_DOWN:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_MARK_4_REGISTRATION, SGE_EMA_TRIGGER, event);
            break;

         case sgeE_ACK_TIMEOUT:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_MARK_4_REGISTRATION, SGE_EMA_TRIGGER, event);
            break;
         
         case sgeE_CQUEUE_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CQUEUE, SGE_EMA_LIST, event);
            break;
         case sgeE_CQUEUE_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CQUEUE, SGE_EMA_ADD, event);
            break;
         case sgeE_CQUEUE_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CQUEUE, SGE_EMA_DEL, event);
            break;
         case sgeE_CQUEUE_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CQUEUE, SGE_EMA_MOD, event);
            break;
         
         case sgeE_QINSTANCE_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_QINSTANCE, SGE_EMA_ADD, event);
            break;
         case sgeE_QINSTANCE_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_QINSTANCE, SGE_EMA_DEL, event);
            break;
         case sgeE_QINSTANCE_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_QINSTANCE, SGE_EMA_MOD, event);
            break;
         case sgeE_QINSTANCE_SOS:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_QINSTANCE, SGE_EMA_MOD, event);
            break;
         case sgeE_QINSTANCE_USOS:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_QINSTANCE, SGE_EMA_MOD, event);
            break;


         case sgeE_SCHED_CONF:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_SCHEDD_CONF, SGE_EMA_MOD, event);
            break;

         case sgeE_SCHEDDMONITOR:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_SCHEDD_MONITOR, SGE_EMA_TRIGGER, event);
            break;

         case sgeE_SHUTDOWN:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_SHUTDOWN, SGE_EMA_TRIGGER, event);
            no_more_events = true;
            break;

         case sgeE_SUBMITHOST_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_SUBMITHOST, SGE_EMA_LIST, event);
            break;
         case sgeE_SUBMITHOST_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_SUBMITHOST, SGE_EMA_ADD, event);
            break;
         case sgeE_SUBMITHOST_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_SUBMITHOST, SGE_EMA_DEL, event);
            break;
         case sgeE_SUBMITHOST_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_SUBMITHOST, SGE_EMA_MOD, event);
            break;

         case sgeE_USER_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_USER, SGE_EMA_LIST, event);
            break;
         case sgeE_USER_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_USER, SGE_EMA_ADD, event);
            break;
         case sgeE_USER_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_USER, SGE_EMA_DEL, event);
            break;
         case sgeE_USER_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_USER, SGE_EMA_MOD, event);
            break;

         case sgeE_USERSET_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_USERSET, SGE_EMA_LIST, event);
            break;
         case sgeE_USERSET_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_USERSET, SGE_EMA_ADD, event);
            break;
         case sgeE_USERSET_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_USERSET, SGE_EMA_DEL, event);
            break;
         case sgeE_USERSET_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_USERSET, SGE_EMA_MOD, event);
            break;

         case sgeE_RQS_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_RQS, SGE_EMA_LIST, event);
            break;
         case sgeE_RQS_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_RQS, SGE_EMA_ADD, event);
            break;
         case sgeE_RQS_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_RQS, SGE_EMA_DEL, event);
            break;
         case sgeE_RQS_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_RQS, SGE_EMA_MOD, event);
            break;
   
#ifndef __SGE_NO_USERMAPPING__
         case sgeE_CUSER_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CUSER, SGE_EMA_LIST, event);
            break;
         case sgeE_CUSER_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CUSER, SGE_EMA_ADD, event);
            break;
         case sgeE_CUSER_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CUSER, SGE_EMA_DEL, event);
            break;
         case sgeE_CUSER_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_CUSER, SGE_EMA_MOD, event);
            break;
#endif

         case sgeE_HGROUP_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_HGROUP, SGE_EMA_LIST, event);
            break;
         case sgeE_HGROUP_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_HGROUP, SGE_EMA_ADD, event);
            break;
         case sgeE_HGROUP_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_HGROUP, SGE_EMA_DEL, event);
            break;
         case sgeE_HGROUP_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_HGROUP, SGE_EMA_MOD, event);
            break;

         case sgeE_AR_LIST:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_AR, SGE_EMA_LIST, event);
            break;
         case sgeE_AR_ADD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_AR, SGE_EMA_ADD, event);
            break;
         case sgeE_AR_DEL:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_AR, SGE_EMA_DEL, event);
            break;
         case sgeE_AR_MOD:
            ret = sge_mirror_process_event(evc, mirror_base, object_base, SGE_TYPE_AR, SGE_EMA_MOD, event);
            break;

         default:
            break;
      }

      /* 
       * if processing of an event failed, return an error to the caller of
       * this function, but continue processing.
       */
      if (ret != SGE_EM_OK) {
         function_ret = SGE_EM_PROCESS_ERRORS;
      }
   }

   if (prof_is_active(SGE_PROF_MIRROR)) {
      prof_stop_measurement(SGE_PROF_MIRROR, NULL);
      PROFILING((SGE_EVENT, "PROF: sge_mirror processed %d events in %.3f s",
                 num_events, prof_get_measurement_wallclock(SGE_PROF_MIRROR,
                 false, NULL)));
   }

   DRETURN(function_ret);
}

/*
 * need this wrapping as to workaround dtrace problems with 
 * pid providers when function pointers are used
 */
sge_mirror_error
sge_mirror_process_event_list(sge_evc_class_t *evc, lList *event_list)
{
   return sge_mirror_process_event_list_(evc, event_list);
}

static sge_mirror_error 
sge_mirror_process_event(sge_evc_class_t *evc, mirror_description *mirror_base, object_description *object_base,
                        sge_object_type type, sge_event_action action, lListElem *event)
{
   sge_callback_result ret;
   char buffer[1024];
   dstring buffer_wrapper;

   DENTER(TOP_LAYER, "sge_mirror_process_event");

   sge_dstring_init(&buffer_wrapper, buffer, sizeof(buffer));

/* DEBUG */
#if 1
   DPRINTF(("%s\n", event_text(event, &buffer_wrapper)));
#else
   {
      u_long32 number, type, intkey, intkey2;
      const char *strkey, *strkey2;
      number = lGetUlong(event, ET_number);
      type = lGetUlong(event, ET_type);
      intkey = lGetUlong(event, ET_intkey);
      intkey2 = lGetUlong(event, ET_intkey2);
      strkey = lGetString(event, ET_strkey);
      strkey2 = lGetString(event, ET_strkey2);
      DPRINTF(("\tEvent: %s intkey %d intkey2 %d strkey \"%s\" strkey2 \"%s\"\n",
               event_text(event, &buffer_wrapper), intkey, intkey2,
               strkey?strkey:"NULL", strkey2?strkey2:"NULL"));
   }
#endif

   if (mirror_base[type].callback_before != NULL) {
      ret = mirror_base[type].callback_before(evc, object_base, type, action, event, mirror_base[type].clientdata);
      if (ret == SGE_EMA_FAILURE) {
         ERROR((SGE_EVENT, MSG_MIRROR_CALLBACKFAILED_S, event_text(event, &buffer_wrapper)));
         DRETURN(SGE_EM_CALLBACK_FAILED);
      } else if (ret == SGE_EMA_IGNORE) {
         DRETURN(SGE_EM_OK);
      }
   }

   if (mirror_base[type].callback_default != NULL) {
      ret = mirror_base[type].callback_default(evc, object_base, type, action, event, NULL);
      if (ret == SGE_EMA_FAILURE) {
         ERROR((SGE_EVENT, MSG_MIRROR_CALLBACKFAILED_S, event_text(event, &buffer_wrapper)));
         DRETURN(SGE_EM_CALLBACK_FAILED);
      } else if (ret == SGE_EMA_IGNORE) {
         DRETURN(SGE_EM_OK);
      }
   }

   if (mirror_base[type].callback_after != NULL) {
      ret = mirror_base[type].callback_after(evc, object_base, type, action, event, mirror_base[type].clientdata);
      if (ret == SGE_EMA_FAILURE) {
         ERROR((SGE_EVENT, MSG_MIRROR_CALLBACKFAILED_S, event_text(event, &buffer_wrapper)));
         DRETURN(SGE_EM_CALLBACK_FAILED);
      }
   }

   DRETURN(SGE_EM_OK);
}

static sge_callback_result
sge_mirror_process_shutdown(sge_evc_class_t *evc, object_description *object_base,sge_object_type type,
                            sge_event_action action, lListElem *event, void *clientdata)
{
   DENTER(TOP_LAYER, "sge_mirror_process_shutdown");

   DPRINTF(("shutting down sge mirror\n"));
   sge_mirror_shutdown(evc);

   shut_me_down = 1;
   DRETURN(SGE_EMA_OK);
}

static sge_callback_result
sge_mirror_process_mark4registration(sge_evc_class_t *evc, object_description *object_base,
                                     sge_object_type type,
                                     sge_event_action action,
                                     lListElem *event,
                                     void *clientdata)
{
   DENTER(TOP_LAYER, "sge_mirror_process_mark4registration");

   DPRINTF(("mark4registration - this happens for ACK TIMEOUT or QMASTER GOES DOWN event\n"));

   evc->ec_mark4registration(evc);

   DRETURN(SGE_EMA_OK);
}

static sge_callback_result
generic_update_master_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                           sge_event_action action, lListElem *event, void *clientdata)
{
   lList **list = NULL;
   const lDescr *list_descr;
   int key_nm;
   const char *key;

   DENTER(TOP_LAYER, "generic_update_master_list");

   list       = sge_master_list(object_base, type);
   list_descr = lGetListDescr(lGetList(event, ET_new_version));
   key_nm     = object_type_get_key_nm(type);

   key = lGetString(event, ET_strkey);

   if (sge_mirror_update_master_list_str_key(list, list_descr, key_nm, key, action, event) != SGE_EM_OK) {
      DRETURN(SGE_EMA_FAILURE);
   }

   if (!object_type_commit_master_list(type, NULL)) {
      DRETURN(SGE_EMA_FAILURE);
   }

   DRETURN(SGE_EMA_OK);
}


/****** Eventmirror/sge_mirror_update_master_list_str_key() *********************
*  NAME
*     sge_mirror_update_master_list_str_key() -- update a master list
*
*  SYNOPSIS
*     sge_mirror_error sge_mirror_update_master_list_str_key(lList **list,
*                                                            const lDescr *list_descr,
*                                                            int key_nm,
*                                                            const char *key,
*                                                            sge_event_action action,
*                                                            lListElem *event)
*
*  FUNCTION
*     Updates a certain element in a certain mirrored list.
*     Which element to update is specified by a given string key.
*
*  INPUTS
*     lList **list            - the master list to update
*     lDescr *list_descr      - descriptor of the master list
*     int key_nm              - field identifier of the key
*     const char *key         - value of the key
*     sge_event_action action - action to perform on list
*     lListElem *event        - raw event element
*
*  RESULT
*     sge_mirror_error - SGE_EM_OK or an error code
*
*  SEE ALSO
*     Eventmirror/sge_mirror_update_master_list()
*******************************************************************************/
sge_mirror_error
sge_mirror_update_master_list_str_key(lList **list, const lDescr *list_descr,
                                      int key_nm, const char *key,
                                      sge_event_action action, lListElem *event)
{
   lListElem *ep;
   sge_mirror_error ret;

   DENTER(TOP_LAYER, "sge_mirror_update_master_list_str_key");

#if 0
   ep = lGetElemStr(*list, key_nm, key);

   ret = sge_mirror_update_master_list(list, list_descr, ep, key, action, event);
#else
   /* TODO: is the code above correct, do we always have list != NULL ??? */
   if (list != NULL) {
      ep = lGetElemStr(*list, key_nm, key);
      ret = sge_mirror_update_master_list(list, list_descr, ep, key, action, event);
   } else {
      ret = SGE_EM_NOT_INITIALIZED;
   }
#endif

   DRETURN(ret);
}

/****** Eventmirror/sge_mirror_update_master_list_host_key() *********************
*  NAME
*     sge_mirror_update_master_list_host_key() -- update a master list
*
*  SYNOPSIS
*     sge_mirror_error sge_mirror_update_master_list_host_key(lList **list,
*                                                            const lDescr *list_descr,
*                                                            int key_nm,
*                                                            const char *key,
*                                                            sge_event_action action,
*                                                            lListElem *event)
*
*  FUNCTION
*     Updates a certain element in a certain mirrored list.
*     Which element to update is specified by a given hostname.
*
*  INPUTS
*     lList **list            - the master list to update
*     lDescr *list_descr      - descriptor of the master list
*     int key_nm              - field identifier of the key
*     const char *key         - value of the key (a hostname)
*     sge_event_action action - action to perform on list
*     lListElem *event        - raw event element
*
*  RESULT
*     sge_mirror_error - SGE_EM_OK or an error code
*
*  SEE ALSO
*     Eventmirror/sge_mirror_update_master_list()
*******************************************************************************/
sge_mirror_error sge_mirror_update_master_list_host_key(lList **list, const lDescr *list_descr,
                                                        int key_nm, const char *key,
                                                        sge_event_action action, lListElem *event)
{
   lListElem *ep;
   sge_mirror_error ret;

   DENTER(TOP_LAYER, "sge_mirror_update_master_list_host_key");

   ep = lGetElemHost(*list, key_nm, key);
   ret = sge_mirror_update_master_list(list, list_descr, ep, key, action, event);

   DRETURN(ret);
}

/****** Eventmirror/sge_mirror_update_master_list() *****************************
*  NAME
*     sge_mirror_update_master_list() -- update a master list
*
*  SYNOPSIS
*     sge_mirror_error sge_mirror_update_master_list(lList **list,
*                                                    const lDescr *list_descr,
*                                                    lListElem *ep,
*                                                    const char *key,
*                                                    sge_event_action action,
*                                                    lListElem *event)
*
*  FUNCTION
*     Updates a given master list according to the given event information.
*     The following actions are performed (depending on parameter action):
*     - SGE_EMA_LIST: an existing mirrored list is completely replaced
*     - SGE_EMA_ADD:  a new element is added to the mirrored list
*     - SGE_EMA_MOD:  a given element is modified
*     - SGE_EMA_DEL:  a given element is deleted
*
*  INPUTS
*     lList **list            - mirrored list to manipulate
*     lDescr *list_descr      - descriptor of mirrored list
*     lListElem *ep           - element to manipulate or NULL
*     const char *key         - key of an element to manipulate
*     sge_event_action action - action to perform
*     lListElem *event        - raw event
*
*  RESULT
*     sge_mirror_error - SGE_EM_OK, or an error code
*
*  SEE ALSO
*     Eventmirror/sge_mirror_update_master_list_str_key()
*     Eventmirror/sge_mirror_update_master_list_host_key()
*******************************************************************************/
sge_mirror_error
sge_mirror_update_master_list(lList **list, const lDescr *list_descr,
                              lListElem *ep, const char *key,
                              sge_event_action action, lListElem *event)
{
   lList *data_list = NULL;

   DENTER(TOP_LAYER, "sge_mirror_update_master_list");

   switch (action) {
      case SGE_EMA_LIST:
         /* switch to new list */
         lXchgList(event, ET_new_version, list);
         break;

      case SGE_EMA_ADD:
         /* check for duplicate */
         if (ep != NULL) {
            ERROR((SGE_EVENT, "duplicate list element "SFQ"\n", (key != NULL) ?key:"NULL"));
            DRETURN(SGE_EM_DUPLICATE_KEY);
         }
   
         /* if neccessary, create list */
         if (*list == NULL) {
            *list = lCreateList("", list_descr);
         }

         /* insert element */
         data_list = lGetList(event, ET_new_version);
         lAppendElem(*list, lDechainElem(data_list, lFirst(data_list)));
         break;

      case SGE_EMA_DEL:
         /* check for existence */
         if (ep == NULL) {
            ERROR((SGE_EVENT, "element "SFQ" does not exist\n", (key != NULL) ?key:"NULL"));
            DRETURN(SGE_EM_KEY_NOT_FOUND);
         }

         /* remove element */
         lRemoveElem(*list, &ep);
         break;

      case SGE_EMA_MOD:
         /* check for existence */
         if (ep == NULL) {
            ERROR((SGE_EVENT, "element "SFQ" does not exist\n", (key != NULL) ?key:"NULL"));
            DRETURN(SGE_EM_KEY_NOT_FOUND);
         }
         lRemoveElem(*list, &ep);
         data_list = lGetList(event, ET_new_version);
         lAppendElem(*list, lDechainElem(data_list, lFirst(data_list)));
         break;
      default:
         DRETURN(SGE_EM_BAD_ARG);
   }

   DRETURN(SGE_EM_OK);
}

static sge_callback_result
/****** sge_mirror/ar_update_master_list() *************************************
*  NAME
*     ar_update_master_list() -- update the master advance reservation list
*
*  SYNOPSIS
*     static sge_callback_result ar_update_master_list(sge_evc_class_t *evc,
*     object_description *object_base, sge_object_type type, sge_event_action
*     action, lListElem *event, void *clientdata)
*
*  FUNCTION
*     Updates the global master list of advance reservations based on an event.
*     The function is called from the event mirroring interface.
*
*  INPUTS
*     sge_evc_class_t *evc            - event client class
*     object_description *object_base - object base of master lists
*     sge_object_type type            - event type
*     sge_event_action action         - action to perform
*     lListElem *event                - the raw event
*     void *clientdata                - client data
*
*  RESULT
*     static sge_callback_result - true, if update is successfull
*                                  false if an error occured
*
*  NOTES
*     MT-NOTE: ar_update_master_list() is not MT safe
*******************************************************************************/
ar_update_master_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                        sge_event_action action, lListElem *event, void *clientdata)
{
   lList **list = NULL;
   const lDescr *list_descr;
   int key_nm;
   const char *key;

   DENTER(TOP_LAYER, "ar_update_master_list");

   list = sge_master_list(object_base, type);
   list_descr = lGetListDescr(lGetList(event, ET_new_version));
   key_nm = object_type_get_key_nm(type);

   key = lGetString(event, ET_strkey);

   if (sge_mirror_update_master_list_ar_key(list, list_descr, key_nm, key, action, event) != SGE_EM_OK) {
      DRETURN(SGE_EMA_FAILURE);
   }

   DRETURN(SGE_EMA_OK);
}

/****** sge_mirror/sge_mirror_update_master_list_ar_key() **********************
*  NAME
*     sge_mirror_update_master_list_ar_key() -- updates the advance reservation
*                                               master list
*
*  SYNOPSIS
*     static sge_mirror_error sge_mirror_update_master_list_ar_key(lList
*     **list, const lDescr *list_descr, int key_nm, const char *key,
*     sge_event_action action, lListElem *event)
*
*  FUNCTION
*     Updates a certain element in the advance reservation mirrored list. Which
*     element to update is specified by the given AR_id as string value
*
*  INPUTS
*     lList **list             - the master list to update
*     const lDescr *list_descr - descriptor of the master list (AR_Type)
*     int key_nm               - field identifies of the key (AR_name)
*     const char *key          - AR_id as string value
*     sge_event_action action  - action to perform on this list
*     lListElem *event         - raw event element
*
*  RESULT
*     static sge_mirror_error - SGE_EM_OK or an error code
*
*  NOTES
*     MT-NOTE: sge_mirror_update_master_list_ar_key() is MT safe, needs GLOBAL_LOCK 
*******************************************************************************/
static sge_mirror_error sge_mirror_update_master_list_ar_key(lList **list, const lDescr *list_descr,
                                                        int key_nm, const char *key,
                                                        sge_event_action action, lListElem *event)
{
   lListElem *ep = NULL;
   sge_mirror_error ret;

   DENTER(TOP_LAYER, "sge_mirror_update_master_list_ar_key");

   if (list != NULL) {
      if (key != NULL) {
         ep = lGetElemUlong(*list, key_nm, atoi(key));
      }
      ret = sge_mirror_update_master_list(list, list_descr, ep, key, action, event);
   } else {
      ret = SGE_EM_NOT_INITIALIZED;
   }

   DRETURN(ret);
}
