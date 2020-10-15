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
 *  Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

#include "uti/sge_rmon.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_stdio.h"
#include "uti/sge_time.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_lock.h"
#include "uti/sge_mtutil.h"

#include "spool/sge_spooling.h"

#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/msg_sgeobjlib.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_resource_utilization_RUE_L.h"
#include "sgeobj/sge_resource_utilization_RDE_L.h"

#include "sched/sge_resource_utilization.h"
#include "sched/sge_select_queue.h"
#include "sched/schedd_monitor.h"
#include "sched/sge_job_schedd.h"
#include "sched/sge_serf.h"
#include "sched/valid_queue_user.h"
#include "sched/sge_complex_schedd.h"

#include "evm/sge_event_master.h"
#include "evm/sge_queue_event_master.h"
#include "evm/sge_event_master.h"

#include "sge_utility_qmaster.h"
#include "sge_host_qmaster.h"
#include "sge_cqueue_qmaster.h"
#include "sge_job_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "sge_give_jobs.h"
#include "sge_qinstance_qmaster.h"
#include "mail.h"
#include "symbols.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"
#include "msg_daemons_common.h"

#define ARSEQ_NUM_FILE "arseqnum"
#define SGE_ATTR_MAIL_LIST "mail_list"

typedef struct {
   u_long32 ar_id;
   bool changed;
   pthread_mutex_t ar_id_mutex;
} ar_id_t;

ar_id_t ar_id_control = {0, false, PTHREAD_MUTEX_INITIALIZER};

static bool ar_reserve_queues(lList **alpp, lListElem *ar);
static u_long32 sge_get_ar_id(sge_gdi_ctx_class_t *ctx, monitoring_t *monitor);
static u_long32 guess_highest_ar_id(void);

static void sge_ar_send_mail(lListElem *ar, int type);

void
ar_initialize_timer(sge_gdi_ctx_class_t *ctx, lList **answer_list, monitoring_t *monitor) 
{
   object_description *object_base = NULL;
   lListElem *ar, *next_ar;
   u_long32 now = sge_get_gmt();
   lList *ar_master_list = NULL;

   DENTER(TOP_LAYER, "ar_initialize_timer");

   object_base = object_type_get_object_description();
   ar_master_list = *object_base[SGE_TYPE_AR].list;

   next_ar = lFirst(ar_master_list);

   while((ar = next_ar)) {
      te_event_t ev = NULL;

      next_ar = lNext(ar);

      if (now < lGetUlong(ar, AR_start_time)) {
         sge_ar_state_set_waiting(ar);

         ev = te_new_event((time_t)lGetUlong(ar, AR_start_time), TYPE_AR_EVENT,
                     ONE_TIME_EVENT, lGetUlong(ar, AR_id), AR_RUNNING, NULL);
         te_add_event(ev);
         te_add_event(ev);
         te_free_event(&ev);

      } else if (now < lGetUlong(ar, AR_end_time)) {
         sge_ar_state_set_running(ar);

         ev = te_new_event((time_t)lGetUlong(ar, AR_end_time), TYPE_AR_EVENT,
                     ONE_TIME_EVENT, lGetUlong(ar, AR_id), AR_EXITED, NULL);
         te_add_event(ev);
         te_free_event(&ev);
      } else {
         dstring buffer = DSTRING_INIT;
         u_long32 ar_id = lGetUlong(ar, AR_id);

         sge_ar_state_set_running(ar);

         sge_ar_remove_all_jobs(ctx, ar_id, 1, monitor);

         ar_do_reservation(ar, false);

         reporting_create_ar_log_record(NULL, ar, ARL_TERMINATED,
                                  "end time of AR reached",
                                  now);
         reporting_create_ar_acct_records(NULL, ar, now);

         sge_dstring_sprintf(&buffer, sge_U32CFormat, ar_id);

         lRemoveElem(ar_master_list, &ar);

         spool_delete_object(answer_list, spool_get_default_context(),
                             SGE_TYPE_AR,
                             sge_dstring_get_string(&buffer),
                             ctx->get_job_spooling(ctx));

         sge_dstring_free(&buffer);
      }
   }
   DRETURN_VOID;
}

/****** sge_advance_reservation_qmaster/ar_mod() *******************************
*  NAME
*     ar_mod() -- gdi callback function for adding modifing advance reservations
*
*  SYNOPSIS
*     int ar_mod(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *new_ar, 
*     lListElem *ar, int add, const char *ruser, const char *rhost, 
*     gdi_object_t *object, int sub_command, monitoring_t *monitor) 
*
*  FUNCTION
*     This function is called from the framework that
*     add/modify/delete generic gdi objects.
*     The purpose of this function is it to add new advance reservation
*     objects.
*     Modifing is currently not supported.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - gdi context pointer
*     lList **alpp             - the answer_list
*     lListElem *new_ar        - if a new ar object will be created by this
*                                function, then new_ar is a newly initialized
*                                CULL object.
*     lListElem *ar            - a reduced ar object that contains all of the
*                                requested values
*     int add                  - 1 for add requests
*                                0 for mod requests
*     const char *ruser        - username who invoked this GDI request
*     const char *rhost        - hostname of where the GDI request was invoked
*     gdi_object_t *object     - structure of the GDI framework that contains
*                                additional informations to perform the request
*     int sub_command          - GDI sub command
*     monitoring_t *monitor    - monitoring structure
*
*  RESULT
*     int - 0 on success
*           STATUS_EUNKNOWN if an error occured
*           STATUS_NOTOK_DOAGAIN if a temporary error
*
*  NOTES
*     MT-NOTE: ar_mod() is not MT safe 
*******************************************************************************/
int ar_mod(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *new_ar,
           lListElem *ar, int add, const char *ruser, 
           const char *rhost, gdi_object_t *object, int sub_command,
           monitoring_t *monitor)
{
   object_description *object_base = object_type_get_object_description();
   u_long32 ar_id;
   u_long32 max_advance_reservations =  mconf_get_max_advance_reservations();

   DENTER(TOP_LAYER, "ar_mod");

   if (!ar_validate(ar, alpp, true, false)) {
      goto ERROR;
   }

   if (add) {
      /* get new ar ids until we find one that is not yet used */
      do {
         ar_id = sge_get_ar_id(ctx, monitor);
      } while (ar_list_locate(*object_base[SGE_TYPE_AR].list, ar_id));
      lSetUlong(new_ar, AR_id, ar_id);
      /*
      ** set the owner of new_ar, don't overwrite it with
      ** attr_mod_str(alpp, ar, new_ar, AR_owner, object->object_name);
      */
      lSetString(new_ar, AR_owner, ruser);
      lSetString(new_ar, AR_group, ctx->get_groupname(ctx));
   } else {
      ERROR((SGE_EVENT, MSG_NOTYETIMPLEMENTED_S, "advance reservation modification"));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      goto ERROR; 
   }

   if (max_advance_reservations > 0 &&
       max_advance_reservations <= lGetNumberOfElem(*object_base[SGE_TYPE_AR].list)) {
      ERROR((SGE_EVENT, MSG_AR_MAXARSPERCLUSTER_U, sge_u32c(max_advance_reservations)));
      answer_list_add(alpp, SGE_EVENT, STATUS_NOTOK_DOAGAIN, ANSWER_QUALITY_ERROR);
      goto DOITAGAIN; 
   }

   /*    AR_name, SGE_STRING */
   attr_mod_zerostr(ar, new_ar, AR_name, object->object_name);
   /*   AR_account, SGE_STRING */
   attr_mod_zerostr(ar, new_ar, AR_account, object->object_name);
   /*   AR_submission_time, SGE_ULONG */
   lSetUlong(new_ar, AR_submission_time, sge_get_gmt());   
   /*   AR_start_time, SGE_ULONG          required */
   attr_mod_ulong(ar, new_ar, AR_start_time, object->object_name);
   /*   AR_end_time, SGE_ULONG            required */
   attr_mod_ulong(ar, new_ar, AR_end_time, object->object_name);
   /*   AR_duration, SGE_ULONG */
   attr_mod_ulong(ar, new_ar, AR_duration, object->object_name);
   /*   AR_verify, SGE_ULONG              just verify the reservation or final case */
   attr_mod_ulong(ar, new_ar, AR_verify, object->object_name);
   /*   AR_error_handling, SGE_ULONG      how to deal with soft and hard exceptions */
   attr_mod_ulong(ar, new_ar, AR_error_handling, object->object_name);
   /*   AR_state, SGE_ULONG               state of the AR */
   lSetUlong(new_ar, AR_state, AR_WAITING);
   /*   AR_checkpoint_name, SGE_STRING    Named checkpoint */
   attr_mod_zerostr(ar, new_ar, AR_checkpoint_name, object->object_name);
   /*   AR_resource_list, SGE_LIST */
   attr_mod_sub_list(alpp, new_ar, AR_resource_list, AR_name, ar, sub_command, SGE_ATTR_COMPLEX_VALUES, SGE_OBJ_AR, 0, NULL);
   /*   AR_queue_list, SGE_LIST */
   attr_mod_sub_list(alpp, new_ar, AR_queue_list, AR_name, ar, sub_command, SGE_ATTR_QUEUE_LIST, SGE_OBJ_AR, 0, NULL);
   /*   AR_mail_options, SGE_ULONG   */
   attr_mod_ulong(ar, new_ar, AR_mail_options, object->object_name);
   /*   AR_mail_list, SGE_LIST */
   attr_mod_sub_list(alpp, new_ar, AR_mail_list, AR_name, ar, sub_command, SGE_ATTR_MAIL_LIST, SGE_OBJ_AR, 0, NULL);
   /*   AR_pe, SGE_STRING */
   attr_mod_zerostr(ar, new_ar, AR_pe, object->object_name);
   /*   AR_master_queue_list, SGE_LIST */
   attr_mod_sub_list(alpp, new_ar ,AR_master_queue_list, AR_name, ar, sub_command, SGE_ATTR_QUEUE_LIST, SGE_OBJ_AR, 0, NULL);
   /*   AR_pe_range, SGE_LIST */
   attr_mod_sub_list(alpp, new_ar, AR_pe_range, AR_name, ar, sub_command, SGE_ATTR_PE_LIST, SGE_OBJ_AR, 0, NULL);
   /*   AR_acl_list, SGE_LIST */
   attr_mod_sub_list(alpp, new_ar, AR_acl_list, AR_name, ar, sub_command, SGE_ATTR_USER_LISTS, SGE_OBJ_AR, 0, NULL);
   /*   AR_xacl_list, SGE_LIST */
   attr_mod_sub_list(alpp, new_ar, AR_xacl_list, AR_name, ar, sub_command, SGE_ATTR_XUSER_LISTS, SGE_OBJ_AR, 0, NULL);
   /*   AR_type, SGE_ULONG     */
   attr_mod_ulong(ar, new_ar, AR_type, object->object_name);

   /* try to reserve the queues */
   if (!ar_reserve_queues(alpp, new_ar)) {
      goto ERROR;
   }

   INFO((SGE_EVENT, MSG_AR_GRANTED_U, sge_u32c(ar_id)));
   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
   DRETURN(0);

ERROR:
   DRETURN(STATUS_EUNKNOWN);
DOITAGAIN:
   DRETURN(STATUS_NOTOK_DOAGAIN);
}

/****** sge_advance_reservation_qmaster/ar_spool() *****************************
*  NAME
*     ar_spool() -- gdi callback funktion to spool an advance reservation
*
*  SYNOPSIS
*     int ar_spool(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *ep, 
*     gdi_object_t *object) 
*
*  FUNCTION
*     This function is called from the framework that
*     add/modify/delete generic gdi objects.
*     After an object was modified/added successfully it
*     is necessary to spool the current state to the filesystem.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - GDI context
*     lList **alpp             - answer_list
*     lListElem *ep            - element to spool
*     gdi_object_t *object     - structure from the GDI framework
*
*  RESULT
*     [alpp] - error messages will be added to this list
*     0 - success
*     STATUS_EEXIST - an error occured
*
*  NOTES
*     MT-NOTE: ar_spool() is MT safe 
*******************************************************************************/
int ar_spool(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *ep, gdi_object_t *object)
{
   lList *answer_list = NULL;
   bool dbret;
   bool job_spooling = ctx->get_job_spooling(ctx);
   dstring buffer = DSTRING_INIT;

   DENTER(TOP_LAYER, "ar_spool");

   sge_dstring_sprintf(&buffer, sge_U32CFormat, lGetUlong(ep, AR_id));
   dbret = spool_write_object(&answer_list, spool_get_default_context(), ep, 
                              sge_dstring_get_string(&buffer), SGE_TYPE_AR,
                              job_spooling);
   answer_list_output(&answer_list);

   if (!dbret) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_PERSISTENCE_WRITE_FAILED_S,
                              sge_dstring_get_string(&buffer));
   } 
   sge_dstring_free(&buffer);

   DRETURN(dbret ? 0 : 1);
}

/****** sge_advance_reservation_qmaster/ar_success() ***************************
*  NAME
*     ar_success() -- does something after a successfully add or modify request
*
*  SYNOPSIS
*     int ar_success(sge_gdi_ctx_class_t *ctx, lListElem *ep, lListElem 
*     *old_ep, gdi_object_t *object, lList **ppList, monitoring_t *monitor) 
*
*  FUNCTION
*     This function is called from the framework that
*     add/modify/delete generic gdi objects.
*     After an object was modified/added and spooled successfully 
*     it is possibly necessary to perform additional tasks.
*     For example it is necessary to send some events to
*     other deamon.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - GDI context
*     lListElem *ep            - new added object
*     lListElem *old_ep        - old object before modifications or NULL
*                                for add requests
*     gdi_object_t *object     - structure from the GDI framework
*     lList **ppList           - ???
*     monitoring_t *monitor    - monitoring structure
*
*  RESULT
*     int - 0
*
*  NOTES
*     MT-NOTE: ar_success() is not MT safe 
*******************************************************************************/
int ar_success(sge_gdi_ctx_class_t *ctx, lListElem *ep, lListElem *old_ep,
               gdi_object_t *object, lList **ppList, monitoring_t *monitor)
{
   te_event_t ev;
   dstring buffer = DSTRING_INIT;
   u_long32 timestamp = 0; 

   DENTER(TOP_LAYER, "ar_success");

   /* with old_ep it is possible to identify if it is an add or modify request */
   timestamp = sge_get_gmt();
   if (old_ep == NULL) {
      reporting_create_new_ar_record(NULL, ep, timestamp);
      reporting_create_ar_attribute_record(NULL, ep, timestamp);
   } else {
      reporting_create_ar_attribute_record(NULL, ep, timestamp);
   }

   /*
   ** return element with correct id
   */
   if (ppList != NULL) {
      if (*ppList == NULL) {
         *ppList = lCreateList("", AR_Type);
      }   
      lAppendElem(*ppList, lCopyElem(ep)); 
   }

   sge_ar_state_set_waiting(ep);

   /*
   ** send sgeE_AR_MOD/sgeE_AR_ADD event
   */
   sge_dstring_sprintf(&buffer, sge_U32CFormat, lGetUlong(ep, AR_id));
   sge_add_event(0, old_ep?sgeE_AR_MOD:sgeE_AR_ADD, lGetUlong(ep, AR_id), 0, 
                 sge_dstring_get_string(&buffer), NULL, NULL, ep);
   lListElem_clear_changed_info(ep);
   sge_dstring_free(&buffer);

   /*
   ** add the timer to trigger the state change
    */
   ev = te_new_event((time_t)lGetUlong(ep, AR_start_time), TYPE_AR_EVENT, ONE_TIME_EVENT, lGetUlong(ep, AR_id), AR_RUNNING, NULL);
   te_add_event(ev);
   te_free_event(&ev);

   DRETURN(0);
}

/****** sge_advance_reservation_qmaster/ar_del() *******************************
*  NAME
*     ar_del() -- removes advance reservation from master list
*
*  SYNOPSIS
*     int ar_del(sge_gdi_ctx_class_t *ctx, lListElem *ep, lList **alpp, lList 
*     **ar_list, char *ruser, char *rhost) 
*
*  FUNCTION
*     This function removes a advance reservation from the master list and
*     performs the necessary cleanup.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - GDI context
*     lListElem *ep            - element that should be removed (ID_Type)
*     lList **alpp             - answer list
*     lList **ar_list          - list from where the element should be removed
*                                (normally a reference to the master ar list)
*     char *ruser              - user who invoked this GDI request
*     char *rhost              - host where the request was invoked
*
*  RESULT
*     int - 0 on success
*           STATUS_EUNKNOWN on failure
*
*  NOTES
*     MT-NOTE: ar_del() is not MT safe 
*******************************************************************************/
int ar_del(sge_gdi_ctx_class_t *ctx, lListElem *ep, lList **alpp, lList **master_ar_list, 
           const char *ruser, const char *rhost, monitoring_t *monitor)
{
   const char *id_str = NULL;
   lList *user_list = NULL;
   lListElem *ar, *nxt;
   bool removed_one = false;
   bool has_manager_privileges = false;
   dstring buffer = DSTRING_INIT;
   lCondition *ar_where = NULL;
   u_long32 now;

   DENTER(TOP_LAYER, "ar_del");

   if (!ep || !ruser || !rhost) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      sge_dstring_free(&buffer);
      DRETURN(STATUS_EUNKNOWN);
   }

   /* ep is no ar_del element, if ep has no ID_str */
   if (lGetPosViaElem(ep, ID_str, SGE_NO_ABORT) < 0) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS,
            lNm2Str(ID_str), SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      sge_dstring_free(&buffer);
      DRETURN(STATUS_EUNKNOWN);
   }

   id_str = lGetString(ep, ID_str);
   
   if ((user_list = lGetList(ep, ID_user_list)) != NULL) {
      lCondition *new_where = NULL;
      lListElem *user;

      for_each(user, user_list) {
         if (sge_is_pattern(lGetString(user, ST_name)) && !manop_is_manager(ruser)) {
            ERROR((SGE_EVENT, MSG_SGETEXT_MUST_BE_MGR_TO_SS,
                  ruser, "modify all advance reservations"));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            sge_dstring_free(&buffer);
            lFreeWhere(&ar_where);
            DRETURN(STATUS_EUNKNOWN);
         }

         new_where = lWhere("%T(%I p= %s)", AR_Type, AR_owner, lGetString(user, ST_name));
         if (ar_where == NULL) {
            ar_where = new_where;
         } else {
            ar_where = lOrWhere(ar_where, new_where);
         }   
      }
   } else if (sge_is_pattern(id_str)) {
      /* if no userlist and wildcard jobs was requested only delete the own ars */
      lCondition *new_where = NULL;
      new_where = lWhere("%T(%I p= %s)", AR_Type, AR_owner, ruser);
      if (ar_where == NULL) {
         ar_where = new_where;
      } else {
         ar_where = lOrWhere(ar_where, new_where);
      }   
   }

   if (id_str != NULL && (strcmp(id_str, "0") != 0)) {
      char *dptr;
      lCondition *new_where = NULL;

      u_long32 value = strtoul(id_str, &dptr, 0);
      if (dptr[0] == '\0') {
         /* is numeric value */
         new_where = lWhere("%T(%I==%u)", AR_Type, AR_id, value); 
      } else {
         bool error = false;
         if (isdigit(id_str[0])) {
            ERROR((SGE_EVENT, MSG_OBJECT_INVALID_NAME_S, id_str));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            error = true;
         } else if (verify_str_key(alpp, id_str, MAX_VERIFY_STRING,
                    lNm2Str(AR_name), KEY_TABLE) != STATUS_OK) {
            error = true;
         } else {
            new_where = lWhere("%T(%I p= %s)", AR_Type, AR_name, id_str);
         }

         if (error) {
            sge_dstring_free(&buffer);
            lFreeWhere(&new_where);
            lFreeWhere(&ar_where);
            DRETURN(STATUS_EUNKNOWN);
         }
      }

      if (!ar_where) {
         ar_where = new_where;
      } else {
         ar_where = lAndWhere(ar_where, new_where);
      }
   } else {
      id_str = NULL;
   }

   if (id_str == NULL && user_list == NULL) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_SPECIFYUSERORID_S, SGE_OBJ_AR));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      sge_dstring_free(&buffer);
      lFreeWhere(&ar_where);
      DRETURN(STATUS_EUNKNOWN);
   }

   if (manop_is_manager(ruser)) {
      has_manager_privileges = true;
   }

   now = sge_get_gmt();
   nxt = lFirst(*master_ar_list);
   while ((ar=nxt)) {
      u_long32 ar_id = lGetUlong(ar, AR_id);
      sge_dstring_sprintf(&buffer, sge_U32CFormat, sge_u32c(ar_id));

      nxt = lNext(ar);

      if ((ar_where != NULL) && !lCompare(ar, ar_where)) {
         continue;
      }

      removed_one = true;

      if (!has_manager_privileges && strcmp(ruser, lGetString(ar, AR_owner))) {
         ERROR((SGE_EVENT, MSG_DELETEPERMS_SSU,
                  ruser, SGE_OBJ_AR, sge_u32c(ar_id)));
         answer_list_add(alpp, SGE_EVENT, STATUS_ENOTOWNER, ANSWER_QUALITY_ERROR);
         continue;
      }

      sge_ar_state_set_deleted(ar);

      /* remove timer for this advance reservation */
      te_delete_one_time_event(TYPE_AR_EVENT, ar_id, AR_RUNNING, NULL);
      te_delete_one_time_event(TYPE_AR_EVENT, ar_id, AR_EXITED, NULL);

      sge_ar_send_mail(ar, MAIL_AT_EXIT);

      /* remove all jobs refering to the AR */
      if (sge_ar_remove_all_jobs(ctx, ar_id, lGetUlong(ep, ID_force), monitor)) {
         /* either all jobs were successfull removed or we had no jobs */

         /* unblock reserved queues */
         ar_do_reservation(ar, false);

         reporting_create_ar_log_record(NULL, ar, ARL_DELETED, 
                                        "AR deleted",
                                        now);  
         reporting_create_ar_acct_records(NULL, ar, now); 

         gdil_del_all_orphaned(ctx, lGetList(ar, AR_granted_slots), alpp);

         lRemoveElem(*master_ar_list, &ar);

         INFO((SGE_EVENT, MSG_JOB_DELETEX_SSU,
                  ruser, SGE_OBJ_AR, sge_u32c(ar_id)));
         answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
         
         sge_event_spool(ctx, alpp, 0, sgeE_AR_DEL, 
                         ar_id, 0, sge_dstring_get_string(&buffer), NULL, NULL,
                         NULL, NULL, NULL, true, true);
      } else {
         INFO((SGE_EVENT, MSG_JOB_REGDELX_SSU,
                  ruser, SGE_OBJ_AR, sge_u32c(ar_id)));
         answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
         sge_event_spool(ctx, alpp, 0, sgeE_AR_MOD, 
                         ar_id, 0, sge_dstring_get_string(&buffer), NULL, NULL,
                         ar, NULL, NULL, true, true);
      }

    }

   if (!removed_one) {
      if (id_str != NULL) {
         sge_dstring_sprintf(&buffer, "%s", id_str);
         ERROR((SGE_EVENT, MSG_SGETEXT_DOESNOTEXIST_SS, SGE_OBJ_AR, sge_dstring_get_string(&buffer)));
      } else {
         lListElem *user;
         bool first = true;
         int umax = 5;

         sge_dstring_sprintf(&buffer, "%s", "");
         for_each(user, user_list) {
            if (!first) {
               sge_dstring_append(&buffer, ",");
            } else {
               first = false;
            }
            if (umax == 0) {
               sge_dstring_append(&buffer, "...");
               break;
            }
            sge_dstring_append(&buffer, lGetString(user, ST_name));
            umax--;
         }
         ERROR((SGE_EVENT, MSG_SGETEXT_THEREARENOXFORUSERS_SS, SGE_OBJ_AR, sge_dstring_get_string(&buffer)));
      } 
  
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      sge_dstring_free(&buffer);
      lFreeWhere(&ar_where);
      DRETURN(STATUS_EEXIST);
   }

   sge_dstring_free(&buffer);
   lFreeWhere(&ar_where);
   DRETURN(0);
}

/****** sge_advance_reservation_qmaster/sge_get_ar_id() ************************
*  NAME
*     sge_get_ar_id() -- returns the next possible unused id
*
*  SYNOPSIS
*     static u_long32 sge_get_ar_id(sge_gdi_ctx_class_t *ctx, monitoring_t 
*     *monitor) 
*
*  FUNCTION
*     returns the next possible unused advance reservation id.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - gdi context
*     monitoring_t *monitor    - monitoring structure
*
*  RESULT
*     static u_long32 - ar id
*
*  NOTES
*     MT-NOTE: sge_get_ar_id() is MT safe 
*******************************************************************************/
static u_long32 sge_get_ar_id(sge_gdi_ctx_class_t *ctx, monitoring_t *monitor)
{
   u_long32 ar_id;
   bool is_store_ar = false;

   DENTER(TOP_LAYER, "sge_get_ar_id");

   sge_mutex_lock("ar_id_mutex", "sge_get_ar_id", __LINE__, 
                  &ar_id_control.ar_id_mutex);
 
   ar_id_control.ar_id++;
   ar_id_control.changed = true;
   if (ar_id_control.ar_id > MAX_SEQNUM) {
      DPRINTF(("highest ar number MAX_SEQNUM %d exceeded, starting over with 1\n", MAX_SEQNUM));
      ar_id_control.ar_id = 1;
      is_store_ar = true;
   }
   ar_id = ar_id_control.ar_id;

   sge_mutex_unlock("ar_id_mutex", "sge_get_ar_id", __LINE__, 
                  &ar_id_control.ar_id_mutex);
  
   if (is_store_ar) {
      sge_store_ar_id(ctx, NULL, monitor);
   }
  
   DRETURN(ar_id);
}

/****** sge_advance_reservation_qmaster/sge_store_ar_id() **********************
*  NAME
*     sge_store_ar_id() -- store ar id
*
*  SYNOPSIS
*     void sge_store_ar_id(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, 
*     monitoring_t *monitor) 
*
*  FUNCTION
*     At qmaster shutdown it's necessary to store the latest highest ar id to
*     reinitialize the counter at the next qmaster start. This is done by a event
*     timer in specific intervall.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - GDI context
*     te_event_t anEvent       - event that triggered this function
*     monitoring_t *monitor    - pointer to monitor (not used here)
*
*  NOTES
*     MT-NOTE: sge_store_ar_id() is not MT safe 
*******************************************************************************/
void sge_store_ar_id(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor) {
   u_long32 ar_id = 0;
   bool changed = false;

   DENTER(TOP_LAYER, "sge_store_ar_id");
   
   sge_mutex_lock("ar_id_mutex", "sge_store_ar_id", __LINE__, 
                  &ar_id_control.ar_id_mutex);
   if (ar_id_control.changed) {
      ar_id = ar_id_control.ar_id;
      ar_id_control.changed = false;
      changed = true;
   }   
   sge_mutex_unlock("ar_id_mutex", "sge_store_ar_id", __LINE__, 
                  &ar_id_control.ar_id_mutex);     

   /* here we got a race condition that can (very unlikely)
      cause concurrent writing of the sequence number file  */ 
   if (changed) {
      FILE *fp = fopen(ARSEQ_NUM_FILE, "w");

      if (fp == NULL) {
         ERROR((SGE_EVENT, MSG_NOSEQFILECREATE_SSS, "ar", ARSEQ_NUM_FILE, strerror(errno)));
      } else {
         FPRINTF((fp, sge_u32"\n", ar_id));
         FCLOSE(fp);
      }   
   }
   DRETURN_VOID;

FPRINTF_ERROR:
FCLOSE_ERROR:
   ERROR((SGE_EVENT, MSG_NOSEQFILECLOSE_SSS, "ar", ARSEQ_NUM_FILE, strerror(errno)));
   DRETURN_VOID;
}

/****** sge_advance_reservation_qmaster/sge_init_ar_id() ***********************
*  NAME
*     sge_init_ar_id() -- init ar id counter
*
*  SYNOPSIS
*     void sge_init_ar_id(void) 
*
*  FUNCTION
*     Called during startup and sets the advance reservation id counter. 
*
*  NOTES
*     MT-NOTE: sge_init_ar_id() is MT safe 
*******************************************************************************/
void sge_init_ar_id(void) 
{
   FILE *fp = NULL;
   u_long32 ar_id = 0;
   u_long32 guess_ar_id = 0;
  
   DENTER(TOP_LAYER, "sge_init_ar_id");
   
   if ((fp = fopen(ARSEQ_NUM_FILE, "r"))) {
      if (fscanf(fp, sge_u32, &ar_id) != 1) {
         ERROR((SGE_EVENT, MSG_NOSEQNRREAD_SSS, "ar", ARSEQ_NUM_FILE, strerror(errno)));
      }
      FCLOSE(fp);
FCLOSE_ERROR:
      fp = NULL;
   } else {
      WARNING((SGE_EVENT, MSG_NOSEQFILEOPEN_SSS, "ar", ARSEQ_NUM_FILE, strerror(errno)));
   }  
   
   guess_ar_id = guess_highest_ar_id();
   ar_id = MAX(ar_id, guess_ar_id);
   
   sge_mutex_lock("ar_id_mutex", "sge_init_ar_id", __LINE__, 
                  &ar_id_control.ar_id_mutex);
   ar_id_control.ar_id = ar_id;
   ar_id_control.changed = true;
   sge_mutex_unlock("ar_id_mutex", "sge_init_ar_id", __LINE__, 
                  &ar_id_control.ar_id_mutex);   
                  
   DRETURN_VOID;
}

/****** sge_advance_reservation_qmaster/guess_highest_ar_id() ******************
*  NAME
*     guess_highest_ar_id() -- guesses the histest ar id
*
*  SYNOPSIS
*     static u_long32 guess_highest_ar_id(void) 
*
*  FUNCTION
*     Iterates over all granted advance reservations in the cluster and determines
*     the highest id
*
*  RESULT
*     static u_long32 - determined id
*
*  NOTES
*     MT-NOTE: guess_highest_ar_id() is MT safe 
*******************************************************************************/
static u_long32 guess_highest_ar_id(void)
{
   lListElem *ar;
   u_long32 maxid = 0;
   lList *master_ar_list = *(object_type_get_master_list(SGE_TYPE_AR)); 

   DENTER(TOP_LAYER, "guess_highest_ar_id");   

   /* this function is called during qmaster startup and not while it is running,
      we do not need to monitor this lock */
   SGE_LOCK(LOCK_GLOBAL, LOCK_READ);
   
   ar = lFirst(master_ar_list);
   if (ar) { 
      int pos;
      pos = lGetPosViaElem(ar, AR_id, SGE_NO_ABORT); 
      
      for_each(ar, master_ar_list) {
         maxid = MAX(maxid, lGetPosUlong(ar, pos));
      }   
   }

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_READ);

   DRETURN(maxid);
}   

/****** sge_advance_reservation_qmaster/sge_ar_event_handler() *****************
*  NAME
*     sge_ar_event_handler() -- advance reservation event handler
*
*  SYNOPSIS
*     void sge_ar_event_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, 
*     monitoring_t *monitor) 
*
*  FUNCTION
*     Registered function in the times event framework. For every granted a trigger
*     for the start time of the advance reservation is registered. When the function is
*     executed at start time it regististers a additional timer for the end time of
*     the advance reservation.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - GDI context
*     te_event_t anEvent       - triggered timed event
*     monitoring_t *monitor    - monitoring structure
*
*  NOTES
*     MT-NOTE: sge_ar_event_handler() is MT safe 
*******************************************************************************/
void sge_ar_event_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   lListElem *ar;
   u_long32 ar_id = te_get_first_numeric_key(anEvent);
   u_long32 state = te_get_second_numeric_key(anEvent);
   te_event_t ev;
   dstring buffer = DSTRING_INIT;

   DENTER(TOP_LAYER, "sge_ar_event_handler");
   
   /*
    To guarantee all jobs are removed from the cluster when AR end time is
    reached it is necessary to consider the DURATION_OFFSET for Advance Reservation also.
    This means all jobs submitted to a AR will have a resulting runtime limit of AR duration - DURATION_OFFSET.
    Jobs requesting a longer runtime will not be scheduled.
    The AR requester needs to keep this in mind when he creates a new AR.
    */
   MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitor);

   if (!(ar = ar_list_locate(*(object_type_get_master_list(SGE_TYPE_AR)), ar_id))) {
      ERROR((SGE_EVENT, MSG_EVE_TE4AR_U, sge_u32c(ar_id)));   
      SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);      
      DRETURN_VOID;
   }

   sge_dstring_sprintf(&buffer, sge_U32CFormat, ar_id);
   
   if (state == AR_EXITED) {
      time_t timestamp = (time_t) sge_get_gmt();

      sge_ar_state_set_exited(ar);

      /* remove all jobs running in this AR */
      sge_ar_remove_all_jobs(ctx, ar_id, 1, monitor);

      /* unblock reserved queues */
      ar_do_reservation(ar, false);

      reporting_create_ar_log_record(NULL, ar, ARL_TERMINATED, 
                                     "end time of AR reached",
                                     timestamp);  
      reporting_create_ar_acct_records(NULL, ar, timestamp); 

      sge_ar_send_mail(ar, MAIL_AT_EXIT);

      /* remove all orphaned queue intances, which are empty. */
      gdil_del_all_orphaned(ctx, lGetList(ar, AR_granted_slots), NULL);

      /* remove the AR itself */
      DPRINTF(("AR: exited, removing AR %s\n", sge_dstring_get_string(&buffer)));
      lRemoveElem(*(object_type_get_master_list(SGE_TYPE_AR)), &ar);
      sge_event_spool(ctx, NULL, 0, sgeE_AR_DEL, 
                      ar_id, 0, sge_dstring_get_string(&buffer), NULL, NULL,
                      NULL, NULL, NULL, true, true);

   } else {
      /* AR_RUNNING */
      DPRINTF(("AR: started, changing state of AR "sge_u32"\n", ar_id));

      sge_ar_state_set_running(ar);

      ev = te_new_event((time_t)lGetUlong(ar, AR_end_time), TYPE_AR_EVENT, ONE_TIME_EVENT, ar_id, AR_EXITED, NULL);
      te_add_event(ev);
      te_free_event(&ev);

      /* this info is not spooled */
      sge_add_event(0, sgeE_AR_MOD, ar_id, 0, 
                    sge_dstring_get_string(&buffer), NULL, NULL, ar);
      lListElem_clear_changed_info(ar);

      reporting_create_ar_log_record(NULL, ar, ARL_STARTTIME_REACHED, 
                                     "start time of AR reached",
                                     sge_get_gmt());

      sge_ar_send_mail(ar, MAIL_AT_BEGINNING);
   }

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
   sge_dstring_free(&buffer);

   DRETURN_VOID;
}

/****** sge_advance_reservation_qmaster/ar_reserve_queues() ********************
*  NAME
*     ar_reserve_queues() -- selects the queues for reserving 
*
*  SYNOPSIS
*     static bool ar_reserve_queues(lList **alpp, lListElem *ar) 
*
*  FUNCTION
*     The function executes the scheduler code to select queues matching the
*     advance reservation request for reserving. The function works on temporary
*     lists and creates the AR_granted_slots list
*
*  INPUTS
*     lList **alpp  - answer list pointer pointer
*     lListElem *ar - ar object
*
*  RESULT
*     static bool - true on success, enough resources reservable
*                   false in verify mode or not enough resources available
*
*  NOTES
*     MT-NOTE: ar_reserve_queues() is not MT safe, needs GLOBAL_LOCK
*******************************************************************************/
static bool ar_reserve_queues(lList **alpp, lListElem *ar)
{
   lList **splitted_job_lists[SPLIT_LAST];
   lList *suspended_list = NULL;                   /* JB_Type */
   lList *running_list = NULL;                     /* JB_Type */

   int verify_mode = lGetUlong(ar, AR_verify);
   lList *talp = NULL;
   lList *ar_queue_request = lGetList(ar, AR_queue_list);
   const char *ar_pe_request = lGetString(ar, AR_pe);
   const char *ar_ckpt_request = lGetString(ar, AR_checkpoint_name);

   lListElem *cqueue = NULL;
   bool ret = true;
   int i = 0;
   lListElem *dummy_job = lCreateElem(JB_Type);
   sge_assignment_t a = SGE_ASSIGNMENT_INIT;
   object_description *object_base = object_type_get_object_description();
   lList *master_cqueue_list = *object_base[SGE_TYPE_CQUEUE].list;
   lList *master_userset_list = *object_base[SGE_TYPE_USERSET].list;
   lList *master_job_list = *object_base[SGE_TYPE_JOB].list;
   lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;
   lList *master_hgroup_list = *object_base[SGE_TYPE_HGROUP].list;
   lList *master_cal_list = *object_base[SGE_TYPE_CALENDAR].list;

   /* These lists must be copied */
   lList *master_pe_list = lCopyList("", *object_base[SGE_TYPE_PE].list);
   lList *master_exechost_list = lCopyList("", *object_base[SGE_TYPE_EXECHOST].list);

   dispatch_t result = DISPATCH_NEVER_CAT;

   DENTER(TOP_LAYER, "ar_reserve_queues");

   if (lGetList(ar, AR_acl_list) != NULL) {
      lSetString(dummy_job, JB_owner, "*");
      lSetString(dummy_job, JB_group, "*");
   } else {
      lSetString(dummy_job, JB_owner, lGetString(ar, AR_owner));
      lSetString(dummy_job, JB_group, lGetString(ar, AR_group));
   }

   assignment_init(&a, dummy_job, NULL, false);
   a.host_list        = master_exechost_list;
   a.centry_list      = master_centry_list;
   a.acl_list         = master_userset_list;
   a.hgrp_list        = master_hgroup_list;
   a.gep              = host_list_locate(master_exechost_list, SGE_GLOBAL_NAME);
   a.start            = lGetUlong(ar, AR_start_time);
   a.duration         = lGetUlong(ar, AR_duration);
   a.is_reservation   = true;
   a.is_advance_reservation = true;
   a.now              = sge_get_gmt();

   /* 
    * Current scheduler code expects all queue instances in a plain list. We use 
    * a copy of all queue instances that needs to be free'd explicitely after 
    * deciding about assignment. This is because assignment_release() sees 
    * queue_list only as a list pointer.
    */
   a.queue_list = lCreateList("", QU_Type);

    /* imagine qs is empty */
    sconf_set_qs_state(QS_STATE_EMPTY);

   /* redirect scheduler monitoring into answer list */
   if (verify_mode == AR_JUST_VERIFY) {
      DPRINTF(("AR Verify Mode\n"));
      a.monitor_alpp = &talp;
   }

   for_each(cqueue, master_cqueue_list) {
      const char *cqname = lGetString(cqueue, CQ_name);
      lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
      lListElem *qinstance;

      if (cqueue_match_static(cqname, &a) != DISPATCH_OK) {
         continue;
      }

      for_each(qinstance, qinstance_list) {
         const char *cal_name;

         /* skip orphaned queues */
         if (qinstance_state_is_orphaned(qinstance)) {
            continue;
         }
      
         /* we only have to consider requested queues */
         if (ar_queue_request != NULL) {
            if (qref_list_cq_rejected(ar_queue_request, cqname,
                     lGetHost(qinstance, QU_qhostname), master_hgroup_list)) {
               continue; 
            } 
         }

         /* we only have to consider queues containing the requested pe */
         if (ar_pe_request != NULL) {
            bool found = false;
            lListElem *pe_ref;

            for_each(pe_ref, lGetList(qinstance, QU_pe_list)) {
               if (pe_name_is_matching(lGetString(pe_ref, ST_name), ar_pe_request)) {
                  found = true;
                  break;
               }
            }
            if (!found) {
               continue;
            }

         }

         /* we only have to consider queues containing the requested checkpoint object */
         if (ar_ckpt_request != NULL) {
            if (lGetSubStr(qinstance, ST_name, ar_ckpt_request, QU_ckpt_list) == NULL) {
               continue;
            }
         }

         /* sort out queue that are calendar disabled in requested time frame */
         if ((cal_name = lGetString(qinstance, QU_calendar)) != NULL) {
            lListElem *cal_ep = calendar_list_locate(master_cal_list, cal_name); 

            if (!calendar_open_in_time_frame(cal_ep, lGetUlong(ar, AR_start_time), lGetUlong(ar, AR_duration))) {
               /* skip queue */
               answer_list_add_sprintf(alpp, STATUS_OK, ANSWER_QUALITY_INFO, MSG_AR_QUEUEDISABLEDINTIMEFRAME,
                                       lGetString(qinstance, QU_full_name)); 
               continue;
            }
         } 
         /* sort out queues where not all users have access */
         if (lGetList(ar, AR_acl_list) != NULL) {
            if (!sge_ar_have_users_access(alpp, ar, lGetString(qinstance, QU_full_name), 
                                                lGetList(qinstance, QU_acl),
                                                lGetList(qinstance, QU_xacl),
                                                master_userset_list)) {
               continue;
            }
         }

         lAppendElem(a.queue_list, lCopyElem(qinstance));
      }
   }

   /*
    * split jobs
    */
   {
      /* initialize all job lists */
      for (i = SPLIT_FIRST; i < SPLIT_LAST; i++) {
         splitted_job_lists[i] = NULL;
      }
      splitted_job_lists[SPLIT_SUSPENDED] = &suspended_list;
      splitted_job_lists[SPLIT_RUNNING] = &running_list;

      /* splitted job lists must be freed */
      split_jobs(&master_job_list, mconf_get_max_aj_instances(), splitted_job_lists, true);
   }

   /*
    * prepare resource schedule
    */
   prepare_resource_schedules(*(splitted_job_lists[SPLIT_RUNNING]),
                              *(splitted_job_lists[SPLIT_SUSPENDED]),
                              master_pe_list, a.host_list, a.queue_list, 
                              NULL, a.centry_list, a.acl_list,
                              a.hgrp_list, NULL, false, a.now);

   /* free generated job lists */
   lFreeList(splitted_job_lists[SPLIT_RUNNING]);
   lFreeList(splitted_job_lists[SPLIT_SUSPENDED]);

   lSetUlong(dummy_job, JB_execution_time, lGetUlong(ar, AR_start_time));
   lSetUlong(dummy_job, JB_deadline, lGetUlong(ar, AR_end_time));
   lSetList(dummy_job, JB_hard_resource_list, lCopyList("", lGetList(ar, AR_resource_list)));
   lSetList(dummy_job, JB_hard_queue_list, lCopyList("", lGetList(ar, AR_queue_list)));
   lSetList(dummy_job, JB_master_hard_queue_list, lCopyList("", lGetList(ar, AR_master_queue_list)));
   lSetUlong(dummy_job, JB_type, lGetUlong(ar, AR_type));
   lSetString(dummy_job, JB_checkpoint_name, lGetString(ar, AR_checkpoint_name));


   if (lGetString(ar, AR_pe)) {
      lSetString(dummy_job, JB_pe, lGetString(ar, AR_pe));
      lSetList(dummy_job, JB_pe_range, lCopyList("", lGetList(ar, AR_pe_range)));

      result = sge_select_parallel_environment(&a, master_pe_list);
      if (result == DISPATCH_OK) {
         lSetString(ar, AR_granted_pe, lGetString(a.pe, PE_name));
      }
   } else {
      result = sge_sequential_assignment(&a);
   }

   /* stop redirection of scheduler monitoring messages */
   if (verify_mode == AR_JUST_VERIFY) {
      /* copy error msgs from talp into alpp */
      answer_list_append_list(alpp, &talp);
      a.monitor_alpp = NULL;

      if (result == DISPATCH_OK) {
         if (!a.pe) {
            answer_list_add_sprintf(alpp, STATUS_OK, ANSWER_QUALITY_INFO,
                                    "%s", MSG_JOB_VERIFYFOUNDQ);
         } else {
            answer_list_add_sprintf(alpp, STATUS_OK, ANSWER_QUALITY_INFO, MSG_JOB_VERIFYFOUNDSLOTS_I, a.slots);
         }
      } else {
         answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_INFO, MSG_JOB_NOSUITABLEQ_S, MSG_JOB_VERIFYVERIFY);
      }
      /* ret has to be false in verify mode, otherwise the framework adds the object to the master list */
      ret = false;
   } else {
      if (result != DISPATCH_OK) {
         answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR, MSG_JOB_NOSUITABLEQ_S, SGE_OBJ_AR);
         ret = false;
      } else {
         lSetList(ar, AR_granted_slots, a.gdil);
         ar_initialize_reserved_queue_list(ar);
         a.gdil = NULL;

         ar_do_reservation(ar, true);
      }
   }

   /* stop dreaming */
   sconf_set_qs_state(QS_STATE_FULL);

   lFreeList(&(a.queue_list));
   lFreeList(&master_pe_list);
   lFreeList(&master_exechost_list);
   lFreeElem(&dummy_job);

   assignment_release(&a);

   DRETURN(ret);
}

/****** sge_advance_reservation_qmaster/ar_do_reservation() ********************
*  NAME
*     ar_do_reservation() -- do the reservation in the selected queue instances
*
*  SYNOPSIS
*     int ar_do_reservation(lListElem *ar, bool incslots) 
*
*  FUNCTION
*     This function does the (un)reserveration in the selected parallel environment
*     and the selected queue instances
*
*  INPUTS
*     lListElem *ar - ar object (AR_Type)
*     bool incslots - increase or decrease usage
*
*  RESULT
*     int - 0
*
*  NOTES
*     MT-NOTE: ar_do_reservation() is not MT safe 
*
*  SEE ALSO
*     sge_resource_utilization/rqs_add_job_utilization()
*******************************************************************************/
int ar_do_reservation(lListElem *ar, bool incslots)
{
   lListElem *dummy_job = lCreateElem(JB_Type);
   lListElem *qep;
   lListElem *global_host_ep = NULL;
   int pe_slots = 0;
   int tmp_slots;
   const char *granted_pe = lGetString(ar, AR_granted_pe);
   u_long32 start_time = lGetUlong(ar, AR_start_time);
   u_long32 duration = lGetUlong(ar, AR_duration);
   object_description *object_base = object_type_get_object_description();
   lList *master_cqueue_list = *object_base[SGE_TYPE_CQUEUE].list;
   lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;
   lList *master_exechost_list = *object_base[SGE_TYPE_EXECHOST].list;
   lList *master_pe_list = *object_base[SGE_TYPE_PE].list;
   bool is_master_task = true;

   DENTER(TOP_LAYER, "ar_do_reservation");

   lSetList(dummy_job, JB_hard_resource_list, lCopyList("", lGetList(ar, AR_resource_list)));
   lSetList(dummy_job, JB_hard_queue_list, lCopyList("", lGetList(ar, AR_queue_list)));

   global_host_ep = host_list_locate(master_exechost_list, SGE_GLOBAL_NAME);

   for_each(qep, lGetList(ar, AR_granted_slots)) {
      lListElem *host_ep = NULL;
      const char *queue_hostname = NULL;
      const char *queue_name = lGetString(qep, JG_qname);
      lListElem *queue = cqueue_list_locate_qinstance(master_cqueue_list, queue_name);

      if (!queue) {
         ERROR((SGE_EVENT, MSG_JOB_UNABLE2FINDQOFJOB_S, queue_name));
         is_master_task = false;
         continue;
      }

      queue_hostname = lGetHost(queue, QU_qhostname);
      
      if (!incslots) {
         tmp_slots = -lGetUlong(qep, JG_slots);
      } else {
         tmp_slots = lGetUlong(qep, JG_slots);
      }

      pe_slots += tmp_slots;

      /* reserve global host */
      if (rc_add_job_utilization(dummy_job, 0, SCHEDULING_RECORD_ENTRY_TYPE_RESERVING,
                                 global_host_ep, master_centry_list, tmp_slots,
                                 EH_consumable_config_list, EH_resource_utilization,
                                 SGE_GLOBAL_NAME, start_time, duration, GLOBAL_TAG,
                                 false, is_master_task) != 0) {
         /* this info is not spooled */
         sge_add_event(0, sgeE_EXECHOST_MOD, 0, 0, 
                       SGE_GLOBAL_NAME, NULL, NULL, global_host_ep);
         lListElem_clear_changed_info(global_host_ep);
      }

      /* reserve exec host */
      host_ep = host_list_locate(master_exechost_list, queue_hostname);
      if (rc_add_job_utilization(dummy_job, 0, SCHEDULING_RECORD_ENTRY_TYPE_RESERVING,
                                 host_ep, master_centry_list, tmp_slots, EH_consumable_config_list,
                                 EH_resource_utilization, queue_hostname, start_time,
                                 duration, HOST_TAG, false, is_master_task) != 0) {
         /* this info is not spooled */
         sge_add_event(0, sgeE_EXECHOST_MOD, 0, 0, 
                       queue_hostname, NULL, NULL, host_ep);
         lListElem_clear_changed_info(host_ep);
      }

      /* reserve queue instance */
      rc_add_job_utilization(dummy_job, 0, SCHEDULING_RECORD_ENTRY_TYPE_RESERVING,
                             queue, master_centry_list, tmp_slots, QU_consumable_config_list,
                             QU_resource_utilization, queue_name, start_time, duration,
                             QUEUE_TAG, false, is_master_task);

      qinstance_increase_qversion(queue);
      /* this info is not spooled */
      qinstance_add_event(queue, sgeE_QINSTANCE_MOD);
      is_master_task = false;
   }

   if (granted_pe != NULL) {
      lListElem *pe = pe_list_locate(master_pe_list, granted_pe);

      if (!pe) {
         ERROR((SGE_EVENT, MSG_OBJ_UNABLE2FINDPE_S, granted_pe));
      } else {
         utilization_add(lFirst(lGetList(pe, PE_resource_utilization)), start_time,
                                duration, pe_slots, 0, 0, PE_TAG, granted_pe,
                                SCHEDULING_RECORD_ENTRY_TYPE_RESERVING, false, false);
         sge_add_event(0, sgeE_PE_MOD, 0, 0, granted_pe, NULL, NULL, pe);
         lListElem_clear_changed_info(pe);
      }
   }

   lFreeElem(&dummy_job);

   DRETURN(0);
}

/****** libs/sgeobj/ar_list_has_reservation_due_to_ckpt() **********************
*  NAME
*     ar_list_has_reservation_due_to_ckpt() -- does ckpt change breake an ar 
*
*  SYNOPSIS
*     bool ar_list_has_reservation_due_to_ckpt(lList *ar_master_list, 
*                                              lList **answer_list,
*                                              const char *qinstance_name, 
*                                              lList *ckpt_string_list) 
*
*  FUNCTION
*     This function tests if a modification of a ckpt list in a qinstance is
*     allowed according to the advance reservations. 
*
*     Input paramters are: the advance reservation master list, the name of the
*     qinstance which sould be modified and the ST_Type string list of ckpt
*     names which represents the new setting for the qinstance.
*
*     If there is no reservation for this qinstance-ckpt combination or if 
*     the reservation would be still valid after the modification then 
*     the function returnes 'false". Otherwise 'true' 
*
*  INPUTS
*     lList *ar_master_list      - advance reservation master list
*     lList **answer_list        - answer list which will contain the reason why a 
*                                  modification is not valid
*     const char *qinstance_name - name of a qinstance <cqname@hostname>
*     lList *ckpt_string_list    - ST_Type list containing ckpt names 
*
*  RESULT
*     boolean
*        true - modification would breake at least one ar
*        false - no ar will be broken if the ckpt list is modified 
*
*  NOTES
*     MT-NOTE: ar_get_string_from_event() is MT safe 
*******************************************************************************/
bool 
ar_list_has_reservation_due_to_ckpt(lList *ar_master_list, lList **answer_list, 
                                    const char *qinstance_name, lList *ckpt_string_list) 
{
   lListElem *ar;

   DENTER(TOP_LAYER, "ar_has_reservation_due_to_ckpt");

   for_each(ar, ar_master_list) {
      const char *ckpt_string = lGetString(ar, AR_checkpoint_name);

      if (ckpt_string != NULL && lGetElemStr(lGetList(ar, AR_granted_slots), JG_qname, qinstance_name)) {
         if (lGetElemStr(ckpt_string_list, ST_name, ckpt_string) == NULL) {
            ERROR((SGE_EVENT, MSG_PARSE_MOD_REJECTED_DUE_TO_AR_SSU, ckpt_string, 
                   SGE_ATTR_CKPT_LIST, sge_u32c(lGetUlong(ar, AR_id))));
            answer_list_add(answer_list, SGE_EVENT, 
                            STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            DRETURN(true);
         }
      }
   }
   DRETURN(false);
}

/****** libs/sgeobj/ar_list_has_reservation_due_to_pe() **********************
*  NAME
*     ar_list_has_reservation_due_to_pe() -- does pe change breake an ar 
*
*  SYNOPSIS
*     bool ar_list_has_reservation_due_to_pe(lList *ar_master_list, 
*                                            lList **answer_list,
*                                            const char *qinstance_name, 
*                                            lList *pe_string_list) 
*
*  FUNCTION
*     This function tests if a modification of a pe list in a qinstance is
*     allowed according to the advance reservations. 
*
*     Input paramters are: the advance reservation master list, the name of the
*     qinstance which should be modified and the ST_Type string list of pe 
*     names which represents the new setting for the qinstance.
*
*     If there is no reservation for this qinstance-ckpt combination or if 
*     the reservation would be still valid after the modification then 
*     the function returnes 'false". Otherwise 'true' 
*
*  INPUTS
*     lList *ar_master_list      - advance reservation master list
*     lList **answer_list        - answer list which will contain the reason why a 
*                                  modification is not valid
*     const char *qinstance_name - name of a qinstance <cqname@hostname>
*     lList *pe_string_list    - ST_Type list containing pe names 
*
*  RESULT
*     boolean
*        true - modification would breake at least one ar
*        false - no ar will be broken if the ckpt list is modified 
*
*  NOTES
*     MT-NOTE: ar_get_string_from_event() is MT safe 
*******************************************************************************/
bool 
ar_list_has_reservation_due_to_pe(lList *ar_master_list, lList **answer_list, 
                                  const char *qinstance_name, lList *pe_string_list) 
{
   lListElem *ar;

   DENTER(TOP_LAYER, "ar_list_has_reservation_due_to_pe");

   for_each(ar, ar_master_list) {
      const char *pe_string = lGetString(ar, AR_pe);

      if (pe_string != NULL && lGetElemStr(lGetList(ar, AR_granted_slots), JG_qname, qinstance_name)) {
         if (lGetElemStr(pe_string_list, ST_name, pe_string) == NULL) {
            ERROR((SGE_EVENT, MSG_PARSE_MOD_REJECTED_DUE_TO_AR_SSU, pe_string, 
                   SGE_ATTR_PE_LIST, sge_u32c(lGetUlong(ar, AR_id))));
            answer_list_add(answer_list, SGE_EVENT, 
                            STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            DRETURN(true);
         }
      }
   }
   DRETURN(false);
}

/****** sgeobj/ar_list_has_reservation_for_pe_with_slots() ********************
*  NAME
*     ar_list_has_reservation_for_pe_with_slots() -- Does PE change violate AR 
*
*  SYNOPSIS
*     bool 
*     ar_list_has_reservation_for_pe_with_slots(lList *ar_master_list, 
*                                               lList **answer_list, 
*                                               const char *pe_name, 
*                                               u_long32 new_slots) 
*
*  FUNCTION
*     This function tests if a modification of slots entry in a pe is
*     allowed according to the advance reservations. 
*
*     Input paramters are: the advance reservation master list, the name of the
*     pe which should be modified and the new slots value which should
*     be set in the pe which might vialote the advance reservations in
*     the system
*
*     If there is no reservation for this pe or if the new slots setting
*     does not violate the advance reservations in the system then this
*     function returns 'false'. Otherwise 'true'
*
*  INPUTS
*     lList *ar_master_list - master advance reservation list 
*     lList **answer_list   - answer list 
*     const char *pe_name   - pe name 
*     u_long32 new_slots    - new slots setting for pe with 'pe_name' 
*
*  RESULT
*     bool 
*        true - modification would break the ar's currently known
*        false - modification is valid
*
*  NOTES
*     MT-NOTE: ar_list_has_reservation_for_pe_with_slots() is MT safe 
*******************************************************************************/
bool
ar_list_has_reservation_for_pe_with_slots(lList *ar_master_list, 
                                          lList **answer_list,
                                          const char *pe_name, 
                                          u_long32 new_slots) 
{
   bool ret = false;
   lListElem *ar;
   lListElem *gs;
   u_long32 max_res_slots = 0;

   DENTER(TOP_LAYER, "ar_list_has_reservation_for_pe_with_slots");

   for_each(ar, ar_master_list) {
      const char *pe_string = lGetString(ar, AR_pe);

      if (pe_name != NULL && pe_string != NULL && strcmp(pe_string, pe_name) == 0) {
         for_each(gs, lGetList(ar, AR_granted_slots)) {
            u_long32 slots = lGetUlong(gs, JG_slots);
         
            max_res_slots += slots;
         }
      }
   }
   if (max_res_slots > new_slots) {
      ERROR((SGE_EVENT, MSG_PARSE_MOD_REJECTED_DUE_TO_AR_PE_SLOTS_U,
             sge_u32c(max_res_slots)));
      answer_list_add(answer_list, SGE_EVENT,
                      STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      ret = true;
   }
   DRETURN(ret);
}

/****** sge_advance_reservation_qmaster/ar_initialize_reserved_queue_list() ******
*  NAME
*     ar_initialize_reserved_queue_list() -- initialize reserved queue structure
*
*  SYNOPSIS
*     void ar_initialize_reserved_queue_list(lListElem *ar) 
*
*  FUNCTION
*     The function creates the list AR_reserved_queues that stores the necessary
*     data to debit jobs in a AR. The Elements in the queue are a reduced
*     element of type QI_Type
*
*  INPUTS
*     lListElem *ar - advance reservation that should be initialized
*
*  NOTES
*     MT-NOTE: ar_initialize_reserved_queue_list() is not MT safe 
*******************************************************************************/
void ar_initialize_reserved_queue_list(lListElem *ar)
{
   lListElem *gep;
   lList *gdil = lGetList(ar, AR_granted_slots);
   object_description *object_base = object_type_get_object_description();
   lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;
   lList *master_cqueue_list = *object_base[SGE_TYPE_CQUEUE].list;
   dstring buffer = DSTRING_INIT;
   bool is_master_queue = true;

   static int queue_field[] = { QU_qhostname,
                                QU_qname,
                                QU_full_name,
                                QU_job_slots,
                                QU_consumable_config_list,
                                QU_tagged4schedule,
                                QU_resource_utilization,
                                QU_message_list,
                                QU_state,
                                QU_s_rt,
                                QU_h_rt,
                                QU_s_cpu,
                                QU_h_cpu,
                                QU_s_fsize,
                                QU_h_fsize,
                                QU_s_data,
                                QU_h_data,
                                QU_s_stack,
                                QU_h_stack,
                                QU_s_core,
                                QU_h_core,
                                QU_s_rss,
                                QU_h_rss,
                                QU_s_vmem,
                                QU_h_vmem,
                                NoName };
    static char *value = "INFINITY";
    static int attr[] = {
      QU_s_cpu, QU_h_cpu, QU_s_fsize, QU_h_fsize, QU_s_data,
      QU_h_data, QU_s_stack, QU_h_stack, QU_s_core, QU_h_core,
      QU_s_rss, QU_h_rss, QU_s_vmem, QU_h_vmem, NoName
    };

   lDescr *rdp = NULL;
   lEnumeration *what; 
   lList *queue_list;

   DENTER(TOP_LAYER, "ar_initialize_reserved_queue_list");

   what = lIntVector2What(QU_Type, queue_field);
   lReduceDescr(&rdp, QU_Type, what);
   lFreeWhat(&what);

   queue_list = lCreateList("", rdp);

   for_each(gep, gdil) {
      int index = 0;
      u_long32 slots = lGetUlong(gep, JG_slots);
      lListElem *queue = lCreateElem(rdp);
      lList *crl = NULL;
      lListElem *cr;
      lListElem *new_cr;
      
      const char *queue_name = lGetString(gep, JG_qname);
      char *cqueue_name = cqueue_get_name_from_qinstance(queue_name);
       
      lSetHost(queue, QU_qhostname, lGetHost(gep, JG_qhostname));
      lSetString(queue, QU_full_name, queue_name);
      lSetString(queue, QU_qname, cqueue_name);

      sge_dstring_clear(&buffer);
      double_print_time_to_dstring(lGetUlong(ar, AR_duration), &buffer);
      lSetString(queue, QU_h_rt, sge_dstring_get_string(&buffer));
      lSetString(queue, QU_s_rt, sge_dstring_get_string(&buffer));

      /*
       * initialize values
       */
      while (attr[index] != NoName) {
         lSetString(queue, attr[index], value);
         index++;
      }

      lSetUlong(queue, QU_job_slots, slots);

      for_each(cr, lGetList(ar, AR_resource_list)) {
         u_long32 consumable = lGetUlong(cr, CE_consumable);
         if (consumable != CONSUMABLE_NO) {
            double newval;
           
            if (lGetUlong(cr, CE_consumable) == CONSUMABLE_YES) {
               newval = lGetDouble(cr, CE_doubleval) * slots;
            } else {
               if (!is_master_queue) {
                  /* job consumables are only attached to the selected masterq */
                  continue;
               }
               newval = lGetDouble(cr, CE_doubleval);
            }

            sge_dstring_sprintf(&buffer, "%f", newval);
            new_cr = lCopyElem(cr);
            lSetString(new_cr, CE_stringval, sge_dstring_get_string(&buffer));
            lSetDouble(new_cr, CE_doubleval, newval);

            if (crl == NULL) {
               crl = lCreateList("", CE_Type);
            }
            lAppendElem(crl, new_cr);
         }
      }

      lSetList(queue, QU_consumable_config_list, crl);
      lAppendElem(queue_list, queue);

      /* ensure availability of implicit slot request */
      qinstance_set_conf_slots_used(queue);

      /* initialize QU_resource_utilization */
      qinstance_debit_consumable(queue, NULL, master_centry_list, 0, true, NULL);

      /* initialize QU_state */
      {
         lListElem *master_cqueue;
         lListElem *master_queue;

         master_cqueue = lGetElemStr(master_cqueue_list, CQ_name, cqueue_name);
         if (master_cqueue != NULL) {
            if ((master_queue = lGetSubStr(master_cqueue, QU_full_name,
                  queue_name, CQ_qinstances)) != NULL){
               if (qinstance_state_is_ambiguous(master_queue)) {
                  lAddUlong(ar, AR_qi_errors, 1);
                  sge_dstring_sprintf(&buffer, "reserved queue %s is %s", queue_name,
                                qinstance_state_as_string(QI_AMBIGUOUS));
                  qinstance_set_error(queue, QI_AMBIGUOUS, sge_dstring_get_string(&buffer), true);
               }
               if (qinstance_state_is_alarm(master_queue)) {
                  lAddUlong(ar, AR_qi_errors, 1);
                  sge_dstring_sprintf(&buffer, "reserved queue %s is %s", queue_name,
                                qinstance_state_as_string(QI_ALARM));
                  qinstance_set_error(queue, QI_ALARM, sge_dstring_get_string(&buffer), true);
               }
               if (qinstance_state_is_suspend_alarm(master_queue)) {
                  lAddUlong(ar, AR_qi_errors, 1);
                  sge_dstring_sprintf(&buffer, "reserved queue %s is %s", queue_name,
                                qinstance_state_as_string(QI_SUSPEND_ALARM));
                  qinstance_set_error(queue, QI_SUSPEND_ALARM, sge_dstring_get_string(&buffer), true);
               }
               if (qinstance_state_is_manual_disabled(master_queue)) {
                  lAddUlong(ar, AR_qi_errors, 1);
                  sge_dstring_sprintf(&buffer, "reserved queue %s is %s", queue_name,
                                qinstance_state_as_string(QI_DISABLED));
                  qinstance_set_error(queue, QI_DISABLED, sge_dstring_get_string(&buffer), true);
               }
               if (qinstance_state_is_unknown(master_queue)) {
                  lAddUlong(ar, AR_qi_errors, 1);
                  sge_dstring_sprintf(&buffer, "reserved queue %s is %s", queue_name,
                                qinstance_state_as_string(QI_UNKNOWN));
                  qinstance_set_error(queue,QI_UNKNOWN, sge_dstring_get_string(&buffer), true);
               }
               if (qinstance_state_is_error(master_queue)) {
                  lAddUlong(ar, AR_qi_errors, 1);
                  sge_dstring_sprintf(&buffer, "reserved queue %s is %s", queue_name,
                                qinstance_state_as_string(QI_ERROR));
                  qinstance_set_error(queue, QI_ERROR, sge_dstring_get_string(&buffer), true);
               }
            }
         }
      }

      sge_free(&cqueue_name);
      is_master_queue = false;
   }
   lSetList(ar, AR_reserved_queues, queue_list);

   sge_free(&rdp);
   sge_dstring_free(&buffer);

   DRETURN_VOID; 
}

/****** sge_advance_reservation_qmaster/sge_ar_remove_all_jobs() ***************
*  NAME
*     sge_ar_remove_all_jobs() -- removes all jobs of an AR
*
*  SYNOPSIS
*     void sge_ar_remove_all_jobs(sge_gdi_ctx_class_t *ctx, u_long32 
*     ar_id, monitoring_t *monitor) 
*
*  FUNCTION
*     The function deletes all jobs (and tasks) requested the advance
*     reservation
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - context handler
*     u_long32 ar_id           - advance reservation id
*     monitoring_t *monitor    - monitoring structure
*
*  NOTES
*     MT-NOTE: sge_ar_remove_all_jobs() is not MT safe 
*******************************************************************************/
bool sge_ar_remove_all_jobs(sge_gdi_ctx_class_t *ctx, u_long32 ar_id, int forced, monitoring_t *monitor)
{
   lListElem *nextjep, *jep;
   lListElem *tmp_task;
   bool ret = true;

   DENTER(TOP_LAYER, "sge_ar_remove_all_jobs");

   nextjep = lFirst(*(object_type_get_master_list(SGE_TYPE_JOB)));
   while ((jep=nextjep)) {
      u_long32 task_number;
      u_long32 start = MIN(job_get_smallest_unenrolled_task_id(jep),
                           job_get_smallest_enrolled_task_id(jep));
      u_long32 end = MAX(job_get_biggest_unenrolled_task_id(jep),
                         job_get_biggest_enrolled_task_id(jep));

      nextjep = lNext(jep);

      if (lGetUlong(jep, JB_ar) != ar_id) {
         continue;
      }

      DPRINTF(("removing job "sge_u32"\n", lGetUlong(jep, JB_job_number)));
      DPRINTF((" ----> task_start = "sge_u32", task_end = "sge_u32"\n", start, end));

      for (task_number = start; 
           task_number <= end; 
           task_number++) {

         if (job_is_ja_task_defined(jep, task_number)) {

            if (job_is_enrolled(jep, task_number)) {
               /* delete all enrolled pending tasks */
               DPRINTF(("removing enrolled task "sge_u32"."sge_u32"\n", lGetUlong(jep, JB_job_number), task_number));
               tmp_task = lGetSubUlong(jep, JAT_task_number, task_number, JB_ja_tasks); 

               /* 
                * if task is already in status deleted and was signaled
                * only recently and deletion is not forced, do nothing
                */
               if (ISSET(lGetUlong(tmp_task, JAT_status), JFINISHED)) {
                  continue;
               }

               if (forced) {
                  sge_commit_job(ctx, jep, tmp_task, NULL, COMMIT_ST_FINISHED_FAILED_EE,
                                 COMMIT_DEFAULT | COMMIT_NEVER_RAN, monitor);
               } else {
                  if (!ISSET(lGetUlong(tmp_task, JAT_state), JDELETED)) {
                     job_mark_job_as_deleted(ctx, jep, tmp_task);
                  }
                  ret = false;
               }
            } else {
               /* delete all unenrolled running tasks */
               DPRINTF(("removing unenrolled task "sge_u32"."sge_u32"\n", lGetUlong(jep, JB_job_number), task_number));
               tmp_task = job_get_ja_task_template_pending(jep, task_number);

               sge_commit_job(ctx, jep, tmp_task, NULL, COMMIT_ST_FINISHED_FAILED,
                              COMMIT_NO_SPOOLING | COMMIT_UNENROLLED_TASK | COMMIT_NEVER_RAN,
                              monitor);
            }
         }
      }
   }

   DRETURN(ret);
}

/****** sge_advance_reservation_qmaster/sge_ar_list_conflicts_with_calendar() ******
*  NAME
*     sge_ar_list_conflicts_with_calendar() -- checks if the given calendar
*                                              conflicts with AR open time frame
*
*  SYNOPSIS
*     bool sge_ar_list_conflicts_with_calendar(lList **answer_list, const char 
*     *qinstance_name, lListElem *cal_ep, lList *master_ar_list) 
*
*  FUNCTION
*     Iteraters over all existing Advance Reservations reserved queues and verifies
*     that the new calender does not invalidate the AR if the queue was reserved
*
*  INPUTS
*     lList **answer_list        - answer list
*     const char *qinstance_name - qinstance name the calendar was configured
*     lListElem *cal_ep          - the calendar object (CAL_Type)
*     lList *master_ar_list      - master AR list
*
*  RESULT
*     bool - true if conflicts
*            false if OK
*
*  NOTES
*     MT-NOTE: sge_ar_list_conflicts_with_calendar() is MT safe 
*******************************************************************************/
bool
sge_ar_list_conflicts_with_calendar(lList **answer_list, const char *qinstance_name,
                                    lListElem *cal_ep, lList *master_ar_list)
{
   lListElem *ar;

   DENTER(TOP_LAYER, "ar_list_conflicts_with_calendar");

   for_each(ar, master_ar_list) {
      if (lGetElemStr(lGetList(ar, AR_granted_slots), JG_qname, qinstance_name)) {
         u_long32 start_time = lGetUlong(ar, AR_start_time);
         u_long32 duration = lGetUlong(ar, AR_duration);

         if (!calendar_open_in_time_frame(cal_ep, start_time, duration)) {
            ERROR((SGE_EVENT, MSG_PARSE_MOD2_REJECTED_DUE_TO_AR_SSU, lGetString(cal_ep, CAL_name), 
                   SGE_ATTR_CALENDAR, sge_u32c(lGetUlong(ar, AR_id))));
            answer_list_add(answer_list, SGE_EVENT, 
                            STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            DRETURN(true);
         }
      }
   }
   DRETURN(false);
}

/****** sge_advance_reservation_qmaster/sge_ar_state_set_running() *************
*  NAME
*     sge_ar_state_set_running() -- set ar in running state
*
*  SYNOPSIS
*     void sge_ar_state_set_running(lListElem *ar) 
*
*  FUNCTION
*     Sets the AR state to running. A running state can result in error state
*     if one of the reserved queues is unable to run a job. This is covered by the
*     function
*
*  INPUTS
*     lListElem *ar - advance reservation object (AR_Type)
*
*  NOTES
*     MT-NOTE: sge_ar_state_set_running() is MT safe 
*
*  SEE ALSO
*     sge_advance_reservation_qmaster/sge_ar_state_set_exited()
*     sge_advance_reservation_qmaster/sge_ar_state_set_deleted()
*     sge_advance_reservation_qmaster/sge_ar_state_set_waiting()
*******************************************************************************/
void sge_ar_state_set_running(lListElem *ar)
{
   u_long32 old_state = lGetUlong(ar, AR_state);

   if (old_state == AR_DELETED || old_state == AR_EXITED) {
      return;
   }

   if (sge_ar_has_errors(ar)) {
      lSetUlong(ar, AR_state, AR_ERROR);
      if (old_state != AR_WARNING && old_state != lGetUlong(ar, AR_state)) {
         /* state change from "running" to "error" */
         reporting_create_ar_log_record(NULL, ar, ARL_UNSATISFIED, "AR resources unsatisfied", sge_get_gmt());
         sge_ar_send_mail(ar, MAIL_AT_ABORT);
      } else if (old_state != lGetUlong(ar, AR_state)) {
         /* state change from "warning" to "error" */
         sge_ar_send_mail(ar, MAIL_AT_ABORT);
      }
   } else {
      lSetUlong(ar, AR_state, AR_RUNNING);
      if (old_state != AR_WAITING && old_state != lGetUlong(ar, AR_state)) {
         /* state change from "error" to "running" */
         reporting_create_ar_log_record(NULL, ar, ARL_OK, "AR resources satisfied", sge_get_gmt());
         sge_ar_send_mail(ar, MAIL_AT_ABORT);
      }
   }
}

/****** sge_advance_reservation_qmaster/sge_ar_state_set_waiting() *************
*  NAME
*     sge_ar_state_set_waiting() -- set ar in running state
*
*  SYNOPSIS
*     void sge_ar_state_set_waiting(lListElem *ar) 
*
*  FUNCTION
*     Sets the AR state to waiting. A waiting state can result in warning state
*     if one of the reserved queues is unable to run a job. This is covered by the
*     function
*
*  INPUTS
*     lListElem *ar - advance reservation object (AR_Type)
*
*  NOTES
*     MT-NOTE: sge_ar_state_set_waiting() is MT safe 
*
*  SEE ALSO
*     sge_advance_reservation_qmaster/sge_ar_state_set_exited()
*     sge_advance_reservation_qmaster/sge_ar_state_set_deleted()
*     sge_advance_reservation_qmaster/sge_ar_state_set_running()
*******************************************************************************/
void sge_ar_state_set_waiting(lListElem *ar)
{
   u_long32 old_state = lGetUlong(ar, AR_state);

   if (old_state == AR_DELETED || old_state == AR_EXITED) {
      return;
   }

   if (sge_ar_has_errors(ar)) {
      lSetUlong(ar, AR_state, AR_WARNING);
      if (old_state != lGetUlong(ar, AR_state)) {
         reporting_create_ar_log_record(NULL, ar, ARL_UNSATISFIED, "AR resources unsatisfied", sge_get_gmt());
      }
   } else {
      lSetUlong(ar, AR_state, AR_WAITING);
      if (old_state != lGetUlong(ar, AR_state)) {
         reporting_create_ar_log_record(NULL, ar, ARL_OK, "AR resources satisfied", sge_get_gmt());
      }
   }
}

/****** sge_advance_reservation_qmaster/sge_ar_state_set_deleted() *************
*  NAME
*     sge_ar_state_set_deleted() -- sets AR into deleted state
*
*  SYNOPSIS
*     void sge_ar_state_set_deleted(lListElem *ar) 
*
*  FUNCTION
*     Sets the AR state to deleted
*
*  INPUTS
*     lListElem *ar - advance reservation object (AR_Type)
*
*  NOTES
*     MT-NOTE: sge_ar_state_set_deleted() is MT safe 
*
*  SEE ALSO
*     sge_advance_reservation_qmaster/sge_ar_state_set_exited()
*     sge_advance_reservation_qmaster/sge_ar_state_set_waiting()
*     sge_advance_reservation_qmaster/sge_ar_state_set_running()
*******************************************************************************/
void sge_ar_state_set_deleted(lListElem *ar) {
   lSetUlong(ar, AR_state, AR_DELETED);
}

/****** sge_advance_reservation_qmaster/sge_ar_state_set_exited() **************
*  NAME
*     sge_ar_state_set_exited() -- sets AR into exited state
*
*  SYNOPSIS
*     void sge_ar_state_set_exited(lListElem *ar) 
*
*  FUNCTION
*     Sets the AR state to deleted
*
*  INPUTS
*     lListElem *ar - advance reservation object (AR_Type)
*
*  NOTES
*     MT-NOTE: sge_ar_state_set_exited() is MT safe 
*
*  SEE ALSO
*     sge_advance_reservation_qmaster/sge_ar_state_set_deleted()
*     sge_advance_reservation_qmaster/sge_ar_state_set_waiting()
*     sge_advance_reservation_qmaster/sge_ar_state_set_running()
*******************************************************************************/
void sge_ar_state_set_exited(lListElem *ar) {
   lSetUlong(ar, AR_state, AR_EXITED);
}

/****** sge_advance_reservation_qmaster/sge_ar_list_set_error_state() **********
*  NAME
*     sge_ar_list_set_error_state() -- Set/unset all ARs reserved in a specific queue
*                                      into error state
*
*  SYNOPSIS
*     void sge_ar_list_set_error_state(lList *ar_list, const char *qname, 
*     u_long32 error_type, bool send_events, bool set_error) 
*
*  FUNCTION
*     The function sets/unsets all ARs that reserved in a queue in the error state and
*     generates the error messages for qrstat -explain
*     
*
*  INPUTS
*     lList *ar_list      - master advance reservation list
*     const char *qname   - queue name
*     u_long32 error_type - error type
*     bool send_events    - send events?
*     bool set_error      - set or unset
*
*  NOTES
*     MT-NOTE: sge_ar_list_set_error_state() is MT safe 
*******************************************************************************/
void sge_ar_list_set_error_state(lList *ar_list, const char *qname, u_long32 error_type, bool set_error)
{
   lListElem *ar;
   dstring buffer = DSTRING_INIT;

   DENTER(TOP_LAYER, "sge_ar_list_set_error_state");

   for_each(ar, ar_list) {
      lListElem *qinstance;
      lList *granted_slots = lGetList(ar, AR_reserved_queues);

      if ((qinstance =lGetElemStr(granted_slots, QU_full_name, qname)) != NULL) {
         u_long32 old_errors = lGetUlong(ar, AR_qi_errors);
         u_long32 new_errors;

         if (set_error) {
            new_errors = old_errors + 1;
            sge_dstring_sprintf(&buffer, MSG_AR_RESERVEDQUEUEHASERROR_SS, qname,
                                qinstance_state_as_string(error_type));
         } else {
            new_errors = old_errors - 1;
         }
         lSetUlong(ar, AR_qi_errors, new_errors);

         qinstance_set_error(qinstance, error_type, sge_dstring_get_string(&buffer), set_error);

         /* update states */
         if (old_errors == 0 || new_errors == 0) {
            if ((lGetUlong(ar, AR_state) == AR_RUNNING || lGetUlong(ar, AR_state) == AR_ERROR)) {
               sge_ar_state_set_running(ar);
            } else {
               sge_ar_state_set_waiting(ar);
            }
            /* this info is not spooled */
            sge_dstring_sprintf(&buffer, sge_U32CFormat, lGetUlong(ar, AR_id));
            sge_add_event(0, sgeE_AR_MOD, 0, 0, 
                          sge_dstring_get_string(&buffer), NULL, NULL, ar);
            lListElem_clear_changed_info(ar);
         }
      }
   }

   sge_dstring_free(&buffer);
   DRETURN_VOID;
}

/****** sge_advance_reservation_qmaster/sge_ar_send_mail() *********************
*  NAME
*     sge_ar_send_mail() -- send mail for advance reservation state change
*
*  SYNOPSIS
*     static void sge_ar_send_mail(lListElem *ar, int type) 
*
*  FUNCTION
*     Create and send mail for a specific event
*
*  INPUTS
*     lListElem *ar - advance reservation object (AR_Type)
*     int type      - event type
*
*  NOTES
*     MT-NOTE: sge_ar_send_mail() is MT safe 
*******************************************************************************/
static void sge_ar_send_mail(lListElem *ar, int type)
{
   dstring buffer = DSTRING_INIT;
   dstring subject = DSTRING_INIT;
   dstring body = DSTRING_INIT;
   u_long32 ar_id;
   const char *ar_name;
   const char *mail_type = NULL;

   DENTER(TOP_LAYER, "sge_ar_send_mail");

   if (!VALID(type, lGetUlong(ar, AR_mail_options))) {
      sge_dstring_append_mailopt(&buffer, type);
      DPRINTF(("mailopt %s was not requested\n", sge_dstring_get_string(&buffer)));
      sge_dstring_free(&subject);
      sge_dstring_free(&body);
      sge_dstring_free(&buffer);
      DRETURN_VOID;
   } 

   ar_id = lGetUlong(ar, AR_id);
   ar_name = lGetString(ar, AR_name);

   switch(type) {
      case MAIL_AT_BEGINNING:
         sge_ctime((time_t)lGetUlong(ar, AR_start_time), &buffer);
         sge_dstring_sprintf(&subject, MSG_MAIL_ARSTARTEDSUBJ_US,
                 sge_u32c(ar_id), ar_name?ar_name:"none");
         sge_dstring_sprintf(&body, MSG_MAIL_ARSTARTBODY_USSS,
                 sge_u32c(ar_id), ar_name?ar_name:"none", lGetString(ar, AR_owner), sge_dstring_get_string(&buffer));
         mail_type = MSG_MAIL_TYPE_ARSTART;
         break;
      case MAIL_AT_EXIT:
         if (lGetUlong(ar, AR_state) == AR_DELETED) {
            sge_ctime((time_t)sge_get_gmt(), &buffer);
            sge_dstring_sprintf(&subject, MSG_MAIL_ARDELETEDSUBJ_US,
                    sge_u32c(ar_id), ar_name?ar_name:"none");
            sge_dstring_sprintf(&body, MSG_MAIL_ARDELETETBODY_USSS,
                    sge_u32c(ar_id), ar_name?ar_name:"none", lGetString(ar, AR_owner), sge_dstring_get_string(&buffer));
            mail_type = MSG_MAIL_TYPE_ARDELETE;
         } else {
            sge_ctime((time_t)lGetUlong(ar, AR_end_time), &buffer);
            sge_dstring_sprintf(&subject, MSG_MAIL_AREXITEDSUBJ_US,
                    sge_u32c(ar_id), ar_name?ar_name:"none");
            sge_dstring_sprintf(&body, MSG_MAIL_AREXITBODY_USSS,
                    sge_u32c(ar_id), ar_name?ar_name:"none", lGetString(ar, AR_owner), sge_dstring_get_string(&buffer));
            mail_type = MSG_MAIL_TYPE_AREND;
         }
         break;
      case MAIL_AT_ABORT:
         if (lGetUlong(ar, AR_state) == AR_ERROR) {
            sge_ctime((time_t)sge_get_gmt(), &buffer);
            sge_dstring_sprintf(&subject, MSG_MAIL_ARERRORSUBJ_US,
                    sge_u32c(ar_id), ar_name?ar_name:"none");
            sge_dstring_sprintf(&body, MSG_MAIL_ARERRORBODY_USSS,
                    sge_u32c(ar_id), ar_name?ar_name:"none", lGetString(ar, AR_owner), sge_dstring_get_string(&buffer));
            mail_type = MSG_MAIL_TYPE_ARERROR;
         } else {
            sge_ctime((time_t)sge_get_gmt(), &buffer);
            sge_dstring_sprintf(&subject, MSG_MAIL_AROKSUBJ_US,
                    sge_u32c(ar_id), ar_name?ar_name:"none");
            sge_dstring_sprintf(&body, MSG_MAIL_AROKBODY_USSS,
                    sge_u32c(ar_id), ar_name?ar_name:"none", lGetString(ar, AR_owner), sge_dstring_get_string(&buffer));
            mail_type = MSG_MAIL_TYPE_AROK;
         }
         break;
      default:
         /* should never happen */
         break;
   }

   cull_mail(QMASTER, lGetList(ar, AR_mail_list), sge_dstring_get_string(&subject), sge_dstring_get_string(&body), mail_type);

   sge_dstring_free(&buffer);
   sge_dstring_free(&subject);
   sge_dstring_free(&body);

   DRETURN_VOID;
}

/****** sge_advance_reservation_qmaster/ar_list_has_reservation_due_to_qinstance_complex_attr() ******
*  NAME
*     ar_list_has_reservation_due_to_qinstance_complex_attr() -- check
*        if change of complex values is valid concerning ar 
*
*  SYNOPSIS
*     bool ar_list_has_reservation_due_to_qinstance_complex_attr(
*        lList *ar_master_list, 
*        lList **answer_list, 
*        lListElem *qinstance, 
*        lList *ce_master_list) 
*
*  FUNCTION
*     Check if the modification of the complex_values of a qinstance
*     whould break existing advance reservations 
*
*  INPUTS
*     lList *ar_master_list - master AR list 
*     lList **answer_list   - answer list 
*     lListElem *qinstance  - qinstance 
*     lList *ce_master_list - master centry list 
*
*  RESULT
*     bool 
*        true - modification is not allowed
*        false - modification is allowed
*
*  NOTES
*     MT-NOTE: ar_list_has_reservation_due_to_qinstance_complex_attr() is  
*     MT safe 
*******************************************************************************/
bool 
ar_list_has_reservation_due_to_qinstance_complex_attr(lList *ar_master_list, 
                                                      lList **answer_list,
                                                      lListElem *qinstance, 
                                                      lList *ce_master_list)
{  
   lListElem *ar = NULL;
   lListElem *gs;

   DENTER(TOP_LAYER, "ar_list_has_reservation_due_to_qinstance_complex_attr");

   for_each(ar, ar_master_list) {
      const char *qinstance_name = lGetString(qinstance, QU_full_name);

      if ((gs =lGetElemStr(lGetList(ar, AR_granted_slots), JG_qname, qinstance_name))) {

         lListElem *rue = NULL;
         lListElem *request = NULL;
         lList *rue_list;

         for_each(request, lGetList(ar, AR_resource_list)) {
            const char *ce_name = lGetString(request, CE_name);
            lListElem *ce = lGetElemStr(ce_master_list, CE_name, ce_name);
            bool is_consumable = (lGetUlong(ce, CE_consumable) > 0) ? true : false;

            if (!is_consumable) {
               char text[2048];
               u_long32 slots = lGetUlong(gs, JG_slots);
               lListElem *current = lGetSubStr(qinstance, CE_name, 
                                               ce_name, QU_consumable_config_list);
               if (current != NULL) {                                  
                  current = lCopyElem(current);
                  lSetUlong(current, CE_relop, lGetUlong(ce, CE_relop));
                  lSetDouble(current, CE_pj_doubleval, lGetDouble(current, CE_doubleval));
                  lSetString(current, CE_pj_stringval, lGetString(current, CE_stringval));

                  if (compare_complexes(slots, request, current, text, false, true) == 0) {
                     ERROR((SGE_EVENT, MSG_QUEUE_MODCMPLXDENYDUETOAR_SS, ce_name,
                            SGE_ATTR_COMPLEX_VALUES));
                     answer_list_add(answer_list, SGE_EVENT, 
                                     STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                     lFreeElem(&current);
                     DRETURN(true);
                  }
                  lFreeElem(&current);
               }
            }
         }

         /* now it gets expensive. Before we can start the check at first we have to build the
            consumable config list. */
         qinstance_reinit_consumable_actual_list(qinstance, answer_list);
         rue_list = lGetList(qinstance, QU_resource_utilization);

         for_each(rue, rue_list) {
            const char *ce_name = lGetString(rue, RUE_name);
            lListElem *ce = lGetElemStr(ce_master_list, CE_name, ce_name);
            bool is_consumable = (lGetUlong(ce, CE_consumable) > 0) ? true : false;

            if (is_consumable) {
               lListElem *rde = NULL;
               lList * rde_list = lGetList(rue, RUE_utilized);
               lListElem *cv = lGetSubStr(qinstance, CE_name, ce_name, QU_consumable_config_list);

               if (cv == NULL) {
                  ERROR((SGE_EVENT, MSG_QUEUE_MODNOCMPLXDENYDUETOAR_SS, 
                         ce_name, SGE_ATTR_COMPLEX_VALUES));
                  answer_list_add(answer_list, SGE_EVENT, 
                                  STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                  DRETURN(true);
               } else {
                  double configured = lGetDouble(cv, CE_doubleval);

                  for_each(rde, rde_list) {
                     double amount = lGetDouble(rde, RDE_amount);

                     if (amount > configured) {
                        ERROR((SGE_EVENT, MSG_QUEUE_MODCMPLXDENYDUETOAR_SS, ce_name,
                               SGE_ATTR_COMPLEX_VALUES));
                        answer_list_add(answer_list, SGE_EVENT, 
                                        STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                        DRETURN(true);
                     }
                  }
               }
            } 
         } 
      }
   }
   DRETURN(false);
}  

/****** sge_advance_reservation_qmaster/ar_list_has_reservation_due_to_host_complex_attr() ******
*  NAME
*     ar_list_has_reservation_due_to_host_complex_attr() -- check
*        if change of complex values is valid concerning ar 
*
*  SYNOPSIS
*     bool ar_list_has_reservation_due_to_host_complex_attr(
*        lList  *ar_master_list, 
*        lList **answer_list, 
*        lListElem *host, 
*        lList *ce_master_list) 
*
*  FUNCTION
*      Check if the modification of the complex_values of a host
*      whould break existing advance reservations.
*
*  INPUTS
*     lList *ar_master_list - master AR list 
*     lList **answer_list   - AN_Type list 
*     lListElem *host       - host 
*     lList *ce_master_list - master centry list 
*
*  RESULT
*     bool 
*        true - modification is not allowed
*        false - modification is allowed
*
*  NOTES
*     MT-NOTE: ar_list_has_reservation_due_to_host_complex_attr() is MT 
*     safe 
*******************************************************************************/
bool 
ar_list_has_reservation_due_to_host_complex_attr(lList *ar_master_list, lList **answer_list,
                                                 lListElem *host, lList *ce_master_list)
{  
   lListElem *ar = NULL;
   const char *hostname = lGetHost(host, EH_name);

   DENTER(TOP_LAYER, "ar_list_has_reservation_due_to_host_complex_attr");

   for_each(ar, ar_master_list) {
      lListElem *gs = NULL;

      for_each(gs, lGetList(ar, AR_granted_slots)) {
         const char *gh = lGetHost(gs, JG_qhostname);

         if (!sge_hostcmp(gh, hostname)) {
            lListElem *rue = NULL;
            lListElem *request = NULL;
            lList *rue_list = lGetList(host, EH_resource_utilization);

            for_each(request, lGetList(ar, AR_resource_list)) {
               const char *ce_name = lGetString(request, CE_name);
               lListElem *ce = lGetElemStr(ce_master_list, CE_name, ce_name);
               bool is_consumable = (lGetUlong(ce, CE_consumable) > 0) ? true : false;
  
               if (!is_consumable) {
                  char text[2048];
                  u_long32 slots = lGetUlong(gs, JG_slots);
                  lListElem *current = lGetSubStr(host, CE_name, 
                                                  ce_name, EH_consumable_config_list);
                  if (current != NULL) {
                     current = lCopyElem(current);
                     lSetUlong(current, CE_relop, lGetUlong(ce, CE_relop));
                     lSetDouble(current, CE_pj_doubleval, lGetDouble(current, CE_doubleval));
                     lSetString(current, CE_pj_stringval, lGetString(current, CE_stringval));
                  
                     if (compare_complexes(slots, request, current, text, false, true) == 0) {
                        ERROR((SGE_EVENT, MSG_QUEUE_MODCMPLXDENYDUETOAR_SS, ce_name,
                               SGE_ATTR_COMPLEX_VALUES));
                        answer_list_add(answer_list, SGE_EVENT, 
                                        STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                        lFreeElem(&current);
                        DRETURN(true);
                     }
                     lFreeElem(&current);
                  }
               }
            }
            for_each(rue, rue_list) {
               const char *ce_name = lGetString(rue, RUE_name);
               lListElem *ce = lGetElemStr(ce_master_list, CE_name, ce_name);
               bool is_consumable = (lGetUlong(ce, CE_consumable) > 0) ? true : false;

               if (is_consumable) {
                  lListElem *rde = NULL;
                  lList *rde_list = lGetList(rue, RUE_utilized);
                  lListElem *cv = lGetSubStr(host, CE_name, ce_name, EH_consumable_config_list);

                  if (cv == NULL) {
                     ERROR((SGE_EVENT, MSG_QUEUE_MODNOCMPLXDENYDUETOAR_SS, 
                            ce_name, SGE_ATTR_COMPLEX_VALUES));
                     answer_list_add(answer_list, SGE_EVENT, 
                                     STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                     DRETURN(true);
                  } else {
                     double configured = lGetDouble(cv, CE_doubleval);

                     for_each(rde, rde_list) {
                        double amount = lGetDouble(rde, RDE_amount);

                        if (amount > configured) {
                           ERROR((SGE_EVENT, MSG_QUEUE_MODCMPLXDENYDUETOAR_SS, 
                                  ce_name, SGE_ATTR_COMPLEX_VALUES));
                           answer_list_add(answer_list, SGE_EVENT, 
                                           STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                           DRETURN(true);
                        }
                     }
                  }
               } 
            } 
         } 
      } 
   }
   DRETURN(false);
}  
