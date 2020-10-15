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

#include "sge_qmaster_process_message.h"

#include <string.h>
#include <unistd.h>

#include "uti/sge_mtutil.h"
#include "uti/sge_lock.h"
#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/sge_prog.h"
#include "uti/sge_bootstrap.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_ack.h"

#include "gdi/sge_gdi2.h"
#include "gdi/sge_security.h"
#include "gdi/sge_gdi_packet_pb_cull.h"
#include "gdi/sge_gdi_packet_internal.h"

#include "comm/commlib.h"

#include "spool/sge_spooling.h"

#include "evm/sge_event_master.h"

#include "sig_handlers.h"
#include "sge_c_gdi.h"
#include "sge_c_report.h"
#include "sge_thread_worker.h"
#include "msg_qmaster.h"
#include "msg_common.h"

static void 
do_gdi_packet(sge_gdi_ctx_class_t *ctx, lList **answer_list, 
              struct_msg_t *aMsg, monitoring_t *monitor);

static void do_c_ack(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg, monitoring_t *monitor);

static void 
do_report_request(sge_gdi_ctx_class_t *ctx, struct_msg_t*, monitoring_t *monitor);

static void 
do_event_client_exit(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg, monitoring_t *monitor);

static void sge_c_job_ack(sge_gdi_ctx_class_t *ctx,
                          const char *host,
                          const char *commproc,
                          u_long32 ack_tag, 
                          u_long32 ack_ulong,
                          u_long32 ack_ulong2, 
                          const char *ack_str,
                          monitoring_t *monitor);

/*
 * Prevent these functions made inline by compiler. This is 
 * necessary for Solaris 10 dtrace pid provider to work.
 */
#ifdef SOLARIS
#pragma no_inline(do_gdi_packet, do_c_ack, do_report_request)
#endif

/****** qmaster/sge_qmaster_process_message/sge_qmaster_process_message() ******
*  NAME
*     sge_qmaster_process_message() -- Entry point for qmaster message handling
*
*  SYNOPSIS
*     void* sge_qmaster_process_message(void *anArg) 
*
*  FUNCTION
*     Get a pending message. Handle message based on message tag.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx
*     monitoring_t *monitor
*
*  RESULT
*     void* - none
*
*  NOTES
*     MT-NOTE: thread safety needs to be verified!
*     MT-NOTE:
*     MT-NOTE: This function should only be used as a 'thread function'
*
*******************************************************************************/
void sge_qmaster_process_message(sge_gdi_ctx_class_t *ctx, monitoring_t *monitor)
{
   int res;
   struct_msg_t msg;

   DENTER(TOP_LAYER, "sge_qmaster_process_message");
   
   memset((void*)&msg, 0, sizeof(struct_msg_t));

   /*
    * INFO (CR)  
    *
    * The not syncron sge_get_any_request() call will not raise cpu usage to 100%
    * because sge_get_any_request() is doing a cl_commlib_trigger() which will
    * return after the timeout specified at cl_com_create_handle() call in prepare_enroll()
    * which is set to 1 second. A syncron receive would result in a unnecessary qmaster shutdown
    * timeout (syncron receive timeout) when no messages are there to read.
    *
    */

   MONITOR_IDLE_TIME((

   res = sge_gdi2_get_any_request(ctx, msg.snd_host, msg.snd_name, 
                                  &msg.snd_id, &msg.buf, &msg.tag, 1, 0, &msg.request_mid)

   ), monitor, mconf_get_monitor_time(), mconf_is_monitor_message());

   MONITOR_MESSAGES(monitor);      
   
   if (res == CL_RETVAL_OK) {
      switch (msg.tag) {
         case TAG_GDI_REQUEST: 
            MONITOR_INC_GDI(monitor);
            do_gdi_packet(ctx, NULL, &msg, monitor);
            break;
         case TAG_ACK_REQUEST:
            MONITOR_INC_ACK(monitor);
            do_c_ack(ctx, &msg, monitor);
            break;
         case TAG_EVENT_CLIENT_EXIT:
            MONITOR_INC_ECE(monitor);
            do_event_client_exit(ctx, &msg, monitor);
            break;
         case TAG_REPORT_REQUEST: 
            MONITOR_INC_REP(monitor);
            do_report_request(ctx, &msg, monitor);
            break;
         default: 
            DPRINTF(("***** UNKNOWN TAG TYPE %d\n", msg.tag));
      }
      clear_packbuffer(&(msg.buf));
   }
 
   DRETURN_VOID; 
} /* sge_qmaster_process_message */

static void 
do_gdi_packet(sge_gdi_ctx_class_t *ctx, lList **answer_list, 
              struct_msg_t *aMsg, monitoring_t *monitor)
{
   sge_pack_buffer *pb_in = &(aMsg->buf);
   sge_gdi_packet_class_t *packet = NULL;
   bool local_ret;

   DENTER(TOP_LAYER, "do_gdi_packet");

   /*
    * unpack the packet and set values 
    */
   local_ret = sge_gdi_packet_unpack(&packet, answer_list, pb_in);
   packet->host = sge_strdup(NULL, aMsg->snd_host);
   packet->commproc = sge_strdup(NULL, aMsg->snd_name);
   packet->commproc_id = aMsg->snd_id;
   packet->response_id = aMsg->request_mid;
   packet->is_intern_request = false;
   packet->is_gdi_request = true;

   /* 
    * Security checks:
    *    check version 
    *    check auth_info (user)
    *    check host, commproc 
    */
   local_ret =
     sge_gdi_packet_verify_version(packet, &(packet->first_task->answer_list))
     && local_ret;
   if (local_ret) {
      local_ret =
         sge_gdi_packet_parse_auth_info(packet,
                                        &(packet->first_task->answer_list));
   }
   if (local_ret) {
      const char *admin_user = ctx->get_admin_user(ctx);
      const char *progname = ctx->get_progname(ctx);

      if (!sge_security_verify_user(packet->host, packet->commproc, 
                                    packet->commproc_id, admin_user, packet->user, progname)) {
         CRITICAL((SGE_EVENT, MSG_SEC_CRED_SSSI, packet->user, packet->host, 
                   packet->commproc, (int)packet->commproc_id));
         answer_list_add(&(packet->first_task->answer_list), SGE_EVENT, 
                         STATUS_ENOSUCHUSER, ANSWER_QUALITY_ERROR); 
      }
   }

   /*
    * EB: TODO: CLEANUP: Handle permission checks in listener not in worker thread.
    *
    * This would be the correct place to handle all permissions   
    * checks which are currently done inside of sge_c_gdi.
    * This can only be done if it is not neccessary anymore to pass a 
    * packbuffer to a worker thread.
    */

   /* 
    * handle GDI packet and send response 
    */
  
#ifdef SEND_ANSWER_IN_LISTENER
   /*
    * Due to the GDI-GET optimization it is neccessary to initialize a pb
    * that is passed to and filled by the worker thread
    *
    * EB: TODO: CLEANUP: Don't pass packbuffer to worker thread.
    * 
    * Better solution would be that the packbuffer is only used here
    * by the listener thread. This would be possible if GDI get
    * requests would be handled by read-only threads.
    */
   init_packbuffer(&(packet->pb), 0, 0);
#endif

   /*
    * Put the packet into the queue so that a worker can handle it
    */
   sge_tq_store_notify(Master_Task_Queue, SGE_TQ_GDI_PACKET, packet);

#ifdef SEND_ANSWER_IN_LISTENER
   sge_gdi_packet_wait_till_handled(packet);

   /*
    * Send the answer to the client
    */
   MONITOR_MESSAGES_OUT(monitor);
   sge_gdi2_send_any_request(ctx, 0, NULL,
                             aMsg->snd_host, aMsg->snd_name, aMsg->snd_id, &(packet->pb),
                             TAG_GDI_REQUEST, aMsg->request_mid, answer_list);

   /*
    * Cleanup
    */
   clear_packbuffer(&(packet->pb));
   sge_gdi_packet_free(&packet);
#else
#  ifdef BLOCK_LISTENER
   sge_gdi_packet_wait_till_handled(packet);
   sge_gdi_packet_free(&packet);
#  endif
#endif

   DRETURN_VOID;
}

/****** sge_qmaster_process_message/do_report_request() ************************
*  NAME
*     do_report_request() -- Process execd load report 
*
*  SYNOPSIS
*     static void do_report_request(struct_msg_t *aMsg) 
*
*  FUNCTION
*     Process execd load reports (TAG_REPORT_REQUEST). Unpack a CULL list with
*     the load report from the pack buffer, which is part of 'aMsg'. Process
*     execd load report.
*
*  INPUTS
*     struct_msg_t *aMsg - execd load report message
*
*  RESULT
*     void - none 
*
*******************************************************************************/
static void 
do_report_request(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg, monitoring_t *monitor)
{
   lList *rep = NULL;
   const char *admin_user = ctx->get_admin_user(ctx);
   const char *myprogname = ctx->get_progname(ctx);
   sge_gdi_packet_class_t *packet = NULL;

   DENTER(TOP_LAYER, "do_report_request");

   /* Load reports are only accepted from admin/root user */
   /* fixme: no actual authentication without CSP? */
   if (!sge_security_verify_unique_identifier(true, admin_user, myprogname, 0,
                                              aMsg->snd_host, aMsg->snd_name, aMsg->snd_id)) {
      DRETURN_VOID;
    }

   if (cull_unpack_list(&(aMsg->buf), &rep)) {
      ERROR((SGE_EVENT, SFNMAX, MSG_CULL_FAILEDINCULLUNPACKLISTREPORT));
      DRETURN_VOID;
   }

   /*
    * create a GDI packet to transport the list to the worker where
    * it will be handled
    */   
   packet = sge_gdi_packet_create_base(NULL);
   packet->host = sge_strdup(NULL, aMsg->snd_host);
   packet->commproc = sge_strdup(NULL, aMsg->snd_name);
   packet->commproc_id = aMsg->snd_id;
   packet->response_id = aMsg->request_mid;
   packet->is_intern_request = false;
   packet->is_gdi_request = false;

   /* 
    * Append a pseudo GDI task
    */ 
   sge_gdi_packet_append_task(packet, NULL, 0, 0, &rep, NULL, NULL, NULL, false);

   /*
    * Put the packet into the task queue so that workers can handle it
    */
   sge_tq_store_notify(Master_Task_Queue, SGE_TQ_GDI_PACKET, packet);

   DRETURN_VOID;
} /* do_report_request */

/****** qmaster/sge_qmaster_process_message/do_event_client_exit() *************
*  NAME
*     do_event_client_exit() -- handle event client exit message 
*
*  SYNOPSIS
*     static void do_event_client_exit(const char *aHost, const char *aSender, 
*     sge_pack_buffer *aBuffer) 
*
*  FUNCTION
*     Handle event client exit message. Extract event client id from pack
*     buffer. Remove event client. 
*
*  INPUTS
*     const char *aHost        - sender 
*     const char *aSender      - communication endpoint 
*     sge_pack_buffer *aBuffer - buffer 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: do_event_client_exit() is NOT MT safe. 
*
*******************************************************************************/
static void do_event_client_exit(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg, monitoring_t *monitor)
{
   u_long32 client_id = 0;

   DENTER(TOP_LAYER, "do_event_client_exit");

   if (unpackint(&(aMsg->buf), &client_id) != PACK_SUCCESS)
   {
      ERROR((SGE_EVENT, MSG_COM_UNPACKINT_I, 1));
      DPRINTF(("%s: client id unpack failed - host %s - sender %s\n", SGE_FUNC, aMsg->snd_host, aMsg->snd_name));
      DEXIT;
      return;
   }

   DPRINTF(("%s: remove client " sge_u32 " - host %s - sender %s\n", SGE_FUNC, client_id, aMsg->snd_host, aMsg->snd_name));

   /* 
   ** check for scheduler shutdown if the request comes from admin or root 
   ** TODO: further enhancement could be matching the owner of the event client 
   **       with the request owner
   */
   if (client_id == 1) {
      const char *admin_user = ctx->get_admin_user(ctx);
      const char *myprogname = ctx->get_progname(ctx);
      if (false == sge_security_verify_unique_identifier(true, admin_user, myprogname, 0,
                                               aMsg->snd_host, aMsg->snd_name, aMsg->snd_id)) {
         DEXIT;
         return;
       }
   }
   
   sge_remove_event_client(client_id);

   DEXIT;
   return;
} /* do_event_client_exit() */


/****************************************************
 Master code.
 Handle ack requests
 Ack requests can be packed together in one packet.

 These are:
 - an execd sends an ack for a received job
 - an execd sends an ack for a signal delivery
 - the schedd sends an ack for received events

 if the counterpart uses the dispatcher ack_tag is the
 TAG we sent to the counterpart.
 ****************************************************/
static void do_c_ack(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg, monitoring_t *monitor) 
{
   u_long32 ack_tag, ack_ulong, ack_ulong2;
   const char *admin_user = ctx->get_admin_user(ctx);
   const char *myprogname = ctx->get_progname(ctx);
   lListElem *ack = NULL;

   DENTER(TOP_LAYER, "do_c_ack");

   MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitor);

   /* Do some validity tests */
   while (pb_unused(&(aMsg->buf)) > 0) {
      if (cull_unpack_elem(&(aMsg->buf), &ack, NULL)) {
         ERROR((SGE_EVENT, "failed unpacking ACK"));
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE)
         DRETURN_VOID;
      }
      ack_tag = lGetUlong(ack, ACK_type);
      ack_ulong = lGetUlong(ack, ACK_id);
      ack_ulong2 = lGetUlong(ack, ACK_id2);

      DPRINTF(("ack_ulong = %ld, ack_ulong2 = %ld\n", (long) ack_ulong,
               (long) ack_ulong2));
      switch (ack_tag) { /* send by dispatcher */
      case ACK_SIGJOB:
      case ACK_SIGQUEUE:
         MONITOR_EACK(monitor);
         /* 
         ** accept only ack requests from admin or root
         */
         if (false == sge_security_verify_unique_identifier(true, admin_user, myprogname, 0,
                                                  aMsg->snd_host, aMsg->snd_name, aMsg->snd_id)) {
            SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE)
            DRETURN_VOID;
         }
         /* an execd sends a job specific acknowledge ack_ulong == jobid of received job */
         sge_c_job_ack(ctx, aMsg->snd_host, aMsg->snd_name, ack_tag, ack_ulong, ack_ulong2, lGetString(ack, ACK_str), monitor);
         break;

      case ACK_EVENT_DELIVERY:
         /* 
         ** TODO: in this case we should check if the event belongs to the user
         **       who send the request
         */
         sge_handle_event_ack(ack_ulong2, (ev_event)ack_ulong);
         break;

      default:
         WARNING((SGE_EVENT, MSG_COM_UNKNOWN_TAG, sge_u32c(ack_tag)));
         break;
      }
      lFreeElem(&ack);
   }

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE)
   DRETURN_VOID;
}

/***************************************************************/
static void sge_c_job_ack(sge_gdi_ctx_class_t *ctx, const char *host, const char *commproc, u_long32 ack_tag, 
                          u_long32 ack_ulong, u_long32 ack_ulong2, const char *ack_str,
                          monitoring_t *monitor) 
{
   lList *answer_list = NULL;
   bool job_spooling = ctx->get_job_spooling(ctx);

   DENTER(TOP_LAYER, "sge_c_job_ack");

   if (strcmp(prognames[EXECD], commproc)) {
      ERROR((SGE_EVENT, MSG_COM_ACK_S, commproc));
      DRETURN_VOID;
   }

   switch (ack_tag) {
   case ACK_SIGJOB:
      {
         lListElem *jep = NULL;
         lListElem *jatep = NULL;

         DPRINTF(("TAG_SIGJOB\n"));
         /* ack_ulong is the jobid */
         if (!(jep = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), ack_ulong))) {
            ERROR((SGE_EVENT, MSG_COM_ACKEVENTFORUNKOWNJOB_U, sge_u32c(ack_ulong) ));
            DRETURN_VOID;
         }
         jatep = job_search_task(jep, NULL, ack_ulong2);
         if (jatep == NULL) {
            ERROR((SGE_EVENT, MSG_COM_ACKEVENTFORUNKNOWNTASKOFJOB_UU, sge_u32c(ack_ulong2), sge_u32c(ack_ulong)));
            DRETURN_VOID;
         }

         DPRINTF(("JOB "sge_u32": SIGNAL ACK\n", lGetUlong(jep, JB_job_number)));
         lSetUlong(jatep, JAT_pending_signal, 0);
         te_delete_one_time_event(TYPE_SIGNAL_RESEND_EVENT, ack_ulong, ack_ulong2, NULL);
         {
            dstring buffer = DSTRING_INIT;
            spool_write_object(&answer_list, spool_get_default_context(), jep, 
                               job_get_key(lGetUlong(jep, JB_job_number), 
                                           ack_ulong2, NULL, &buffer), 
                               SGE_TYPE_JOB, job_spooling);
            sge_dstring_free(&buffer);
         }
         answer_list_output(&answer_list);
         
         break;
      }

   case ACK_SIGQUEUE:
      {
         lListElem *qinstance = NULL;
         lListElem *cqueue = NULL;
         dstring cqueue_name = DSTRING_INIT;
         dstring host_domain = DSTRING_INIT;

         cqueue_name_split(ack_str, &cqueue_name, &host_domain, NULL,  
                          NULL);
            
         cqueue = lGetElemStr(*(object_type_get_master_list(SGE_TYPE_CQUEUE)),
                              CQ_name, sge_dstring_get_string(&cqueue_name));

         sge_dstring_free(&cqueue_name);

         if (cqueue != NULL) {
            lList *qinstance_list = lGetList(cqueue, CQ_qinstances);

            qinstance = lGetElemHost(qinstance_list, QU_qhostname, 
               sge_dstring_get_string(&host_domain));
         }
         sge_dstring_free(&host_domain);

         if (qinstance == NULL) {
            ERROR((SGE_EVENT, MSG_COM_ACK_QUEUE_S, ack_str));
            DRETURN_VOID;
         }
      
         DPRINTF(("QUEUE %s: SIGNAL ACK\n", lGetString(qinstance, QU_full_name)));

         lSetUlong(qinstance, QU_pending_signal, 0);
         te_delete_one_time_event(TYPE_SIGNAL_RESEND_EVENT, 0, 0, lGetString(qinstance, QU_full_name));
         spool_write_object(&answer_list, spool_get_default_context(), qinstance, 
                            lGetString(qinstance, QU_full_name), SGE_TYPE_QINSTANCE, job_spooling);
         answer_list_output(&answer_list);
         break;
      }

   default:
      ERROR((SGE_EVENT, SFNMAX, MSG_COM_ACK_UNKNOWN));
   }
   DRETURN_VOID;
}
