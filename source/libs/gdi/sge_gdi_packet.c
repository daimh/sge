
/* ___INFO__MARK_BEGIN__ */

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
 *  Portions of this code are Copyright 2011 Univa Inc.
 *
 ************************************************************************/

/* ___INFO__MARK_END__ */

#include <stdlib.h>
#include <string.h>

#include "basis_types.h"

#include "comm/commlib.h"

#include "uti/sge_mtutil.h"
#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"
#include "uti/sge_thread_ctrl.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_multi_MA_L.h"

#include "gdi/sge_gdi2.h"
#include "gdi/sge_gdiP.h"
#include "gdi/sge_gdi_packet.h"
#include "gdi/sge_gdi_packet_pb_cull.h"
#include "gdi/version.h"
#include "gdi/sge_security.h"
#include "gdi/msg_gdilib.h"

#include "msg_common.h"
#include "msg_qmaster.h"

/****** gdi/request_internal/--Packets_and_Taks() *****************************
*  NAME
*     Packets_and_Tasks -- Functions and structures behind GDI
*
*  FUNCTION
*     Packets and tasks are structures which are used in the methods 
*     implementing GDI. The real C structure names are 
*
*        "sge_gdi_packet_class_t" and
*        "sge_gdi_task_class_t". 
*
*     An instance of the packet structure represents a GDI request.
*     Each packet has assigned at least one task. But it may have
*     multiple tasks. A Packet with one task represents a
*     singe GDI request and that one with multiple task represents
*     a multi GDI request.
*
*     GDI requests are created by function calls sge_gdi() and
*     sge_gdi_multi(). Internally the packet and task data 
*     structures are used to represent that request information.
*
*     Within the client side code (sge_gdi(), sge_gdi_multi(),
*     sge_gdi_wait(), sge_extract_answer() ... ) there are multiple
*     functions used to create and verify packets and tasks:
*
*        sge_gdi_packet_append_task()
*        sge_gdi_packet_create()
*        sge_gdi_packet_create_base()
*        sge_gdi_packet_create_multi_answer()
*        sge_gdi_packet_free()
*        sge_gdi_packet_get_last_task_id()
*        sge_gdi_packet_initialize_auth_info()
*        sge_gdi_task_create()
*        sge_gdi_task_free()
*
*     Following functions are used to handle data transfer
*     from external GDI clients (qstat, qconf, ...) or internal
*     GDI clients (scheduler, JVM, ...)
*
*        sge_gdi_packet_execute_internal()
*        sge_gdi_packet_execute_external()
*        sge_gdi_packet_wait_for_result_internal()
*        sge_gdi_packet_wait_for_result_external()
*
*     Data transfer of packets transfered  between external GDI 
*     clients is prepared with following functions process:
*
*        sge_gdi_packet_get_pb_size()
*        sge_gdi_packet_initialize_auth_info()
*        sge_gdi_packet_pack()
*        sge_gdi_packet_pack_task()
*        sge_gdi_packet_parse_auth_info()
*        sge_gdi_packet_unpack()
*
*    Within the master process special syncronisation
*    for the thread accessing packet and task datastructures
*    is required because the data structures are not thread
*    safe per default. Only one thread is allowed to access
*    a packet or task structure at the same time. To synchronize
*    their activities between "listener" and "worker"
*    threads a queue of packet elements is used. This
*    "Master_Packet_Queue" is guarded by the function calls
*
*        sge_gdi_packet_queue_wait_for_new_packet()
*        sge_gdi_packet_queue_store_notify()
*        sge_gdi_packet_queue_wakeup_all_waiting()
*
*     Additional synchronisation functions are
*
*        sge_gdi_packet_wait_till_handled()
*        sge_gdi_packet_broadcast_that_handled()
*
*  SEE ALSO
*     gdi/request_internal/Master_Packet_Queue
*     gdi/request_internal/sge_gdi_task_create()
*     gdi/request_internal/sge_gdi_task_free()
*     gdi/request_internal/sge_gdi_task_get_operation_name()
*     gdi/request_internal/sge_gdi_packet_append_task()
*     gdi/request_internal/sge_gdi_packet_broadcast_that_handled()
*     gdi/request_internal/sge_gdi_packet_create()
*     gdi/request_internal/sge_gdi_packet_create_base()
*     gdi/request_internal/sge_gdi_packet_create_multi_answer()
*     gdi/request_internal/sge_gdi_packet_get_last_task_id()
*     gdi/request_internal/sge_gdi_packet_execute_internal()
*     gdi/request_internal/sge_gdi_packet_execute_external()
*     gdi/request_internal/sge_gdi_packet_free()
*     gdi/request_internal/sge_gdi_packet_get_pb_size()
*     gdi/request_internal/sge_gdi_packet_initialize_auth_info()
*     gdi/request_internal/sge_gdi_packet_pack()
*     gdi/request_internal/sge_gdi_packet_pack_task()
*     gdi/request_internal/sge_gdi_packet_parse_auth_info()
*     gdi/request_internal/sge_gdi_packet_queue_store_notify()
*     gdi/request_internal/sge_gdi_packet_queue_wait_for_new_packet()
*     gdi/request_internal/sge_gdi_packet_queue_wakeup_all_waiting()
*     gdi/request_internal/sge_gdi_packet_unpack()
*     gdi/request_internal/sge_gdi_packet_initialize_auth_info()
*     gdi/request_internal/sge_gdi_packet_wait_for_result_internal()
*     gdi/request_internal/sge_gdi_packet_wait_for_result_external()
*     gdi/request_internal/sge_gdi_packet_wait_till_handled()
*******************************************************************************/

#if 1
#define SGE_GDI_PACKET_DEBUG
#endif

#if defined(SGE_GDI_PACKET_DEBUG)
bool sge_gdi_task_debug_print(sge_gdi_task_class_t * task);
#endif

static bool sge_gdi_task_free(sge_gdi_task_class_t ** task);

static bool sge_gdi_task_verify(sge_gdi_task_class_t * task,
                                lList **answer_list);

static sge_gdi_task_class_t *sge_gdi_task_create(sge_gdi_packet_class_t *packet, 
                                                 lList **answer_list,
                                                 u_long32 target,
                                                 u_long32 command, lList **lp,
                                                 lList **a_list,
                                                 lCondition **condition,
                                                 lEnumeration **enumeration,
                                                 bool do_copy, bool do_verify);

static bool sge_gdi_task_free(sge_gdi_task_class_t ** task);

#if defined(SGE_GDI_PACKET_DEBUG)
bool sge_gdi_packet_debug_print(sge_gdi_packet_class_t * packet);
#endif


#if defined(SGE_GDI_PACKET_DEBUG)
bool 
sge_gdi_task_debug_print(sge_gdi_task_class_t * task) 
{
    bool ret = true;
    
    DENTER(TOP_LAYER, "sge_gdi_task_debug_print");
    if (task != NULL) {
        DPRINTF(("task->id = " sge_U32CFormat "\n", sge_u32c(task->id)));
        DPRINTF(("task->command = " sge_U32CFormat "\n",
                sge_u32c(task->command)));
        DPRINTF(("task->target = " sge_U32CFormat "\n", sge_u32c(task->target)));
        DPRINTF(("task->data_list = %p\n", task->data_list));
        DPRINTF(("task->answer_list = %p\n", task->answer_list));
        DPRINTF(("task->condition = %p\n", task->condition));
        DPRINTF(("task->enumeration = %p\n", task->enumeration));
        DPRINTF(("task->next = %p\n", task->next));
    } else {
        DPRINTF(("task is NULL\n"));
    }
    DRETURN(ret);
}

#endif

static bool
sge_gdi_task_verify(sge_gdi_task_class_t * task, lList **answer_list)
{
   bool ret = true;
#if 0
   int operation = 0;
   lList *list = NULL;
   u_long32 target = 0;
#endif

   DENTER(TOP_LAYER, "sge_gdi_task_verify");
#if 0
   operation = SGE_GDI_GET_OPERATION(task->command);
   list = task->data_list;
   target = task->target;
   /* EB: TODO: this check does not work for AR objects why? */
   if (!list
       && !(operation == SGE_GDI_PERMCHECK || operation == SGE_GDI_GET
            || operation == SGE_GDI_TRIGGER || (operation == SGE_GDI_DEL
                                                && target ==
                                                SGE_STN_LIST))
      ) {
      answer_list_add_sprintf(answer_list, STATUS_ESEMANTIC,
                 ANSWER_QUALITY_ERROR, MSG_GDI_GDI_VERIFY_REQUEST_FAILED);
      ret = false;
   }
#endif
   DRETURN(ret);
}

static sge_gdi_task_class_t *
sge_gdi_task_create(sge_gdi_packet_class_t * packet, lList **answer_list,
                    u_long32 target, u_long32 command, lList **lp,
                    lList **a_list, lCondition **condition,
                    lEnumeration **enumeration, bool do_copy, bool do_verify)
{
   sge_gdi_task_class_t *task = NULL;

   DENTER(TOP_LAYER, "sge_gdi_task_create");

   task = (sge_gdi_task_class_t *) sge_malloc(sizeof(sge_gdi_task_class_t));
   if (task != NULL) {
      task->id =
         ((packet->last_task != NULL) ? (packet->last_task->id + 1) : 1);
      task->command = command;
      task->target = target;
      task->next = NULL;
      task->do_select_pack_simultaneous = false;
      if (do_copy) {
         if (enumeration != NULL && *enumeration != NULL) {
            task->data_list = (((lp != NULL) && (*lp != NULL)) ?
                               lSelect("", *lp, NULL, *enumeration) : NULL);
         } else {
            task->data_list = (((lp != NULL) && (*lp != NULL)) ?
                               lCopyList("", *lp) : NULL);
         }
         task->answer_list = (((a_list != NULL) && (*a_list != NULL)) ?
                              lCopyList("", *a_list) : NULL);
         task->condition = (((condition != NULL) && (*condition != NULL)) ?
                            lCopyWhere(*condition) : NULL);
         task->enumeration = (((enumeration != NULL) && (*enumeration != NULL)) ?
                              lCopyWhat(*enumeration) : NULL);
      } else {
         if ((lp != NULL) && (*lp != NULL)) {
            task->data_list = *lp;
            *lp = NULL;
         } else {
            task->data_list = NULL;
         }
         if ((a_list != NULL) && (*a_list != NULL)) {
            task->answer_list = *a_list;
            *a_list = NULL;
         } else {
            task->answer_list = NULL;
         }
         if ((condition != NULL) && (*condition != NULL)) {
            task->condition = *condition;
            *condition = NULL;
         } else {
            task->condition = NULL;
         }
         if ((enumeration != NULL) && (*enumeration != NULL)) {
            task->enumeration = *enumeration;
            *enumeration = NULL;
         } else {
            task->enumeration = NULL;
         }
      }
      if (do_verify && !sge_gdi_task_verify(task, answer_list)) {
         if (do_copy == true) {
            lFreeList(&(task->data_list));
            lFreeList(&(task->answer_list));
            lFreeWhere(&(task->condition));
            lFreeWhat(&(task->enumeration));
         } else {
            task->data_list = NULL;
            task->answer_list = NULL;
            task->condition = NULL;
            task->enumeration = NULL;
         }
         sge_gdi_task_free(&task);
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EMALLOC,
                              ANSWER_QUALITY_ERROR,
                              "%s", MSG_MEMORY_MALLOCFAILED);
   }
   DRETURN(task);
}

/****** gdi/request_internal/sge_gdi_task_free() ******************************
*  NAME
*     sge_gdi_task_free() -- free a gdi task structure
*
*  SYNOPSIS
*     static bool
*     sge_gdi_task_free(sge_gdi_task_class_t **task)
*
*  FUNCTION
*     free all elements of the gdi task structure and the structure itself.
*
*  INPUTS
*     sge_gdi_task_class_t **task - pointer to the task structure pointer
*
*  RESULT
*     static bool -
*        true - success (always)
*
*  NOTES
*     MT-NOTE: sge_gdi_task_free() is MT safe as long as the structure
*              passed to this function is not accessed by more than one
*              thread simultaneously.
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_task_create()
******************************************************************************/
static bool
sge_gdi_task_free(sge_gdi_task_class_t ** task)
{
   bool ret = true;

   DENTER(TOP_LAYER, "sge_gdi_task_free");
   if (task != NULL && *task != NULL) {
      lFreeList(&((*task)->data_list));
      lFreeList(&((*task)->answer_list));
      lFreeWhat(&((*task)->enumeration));
      lFreeWhere(&((*task)->condition));
      sge_free(task);
   }
   DRETURN(ret);
}

#if defined(SGE_GDI_PACKET_DEBUG)
bool
sge_gdi_packet_debug_print(sge_gdi_packet_class_t * packet)
{
   bool ret = true;

   DENTER(TOP_LAYER, "sge_gdi_packet_debug_print");

   if (packet != NULL) {
      sge_gdi_task_class_t *task;

      DPRINTF(("packet->id = " sge_U32CFormat "\n", sge_u32c(packet->id)));
      DPRINTF(("packet->host = " SFQ "\n", packet->host ? packet->host : "<null>"));
      DPRINTF(("packet->commproc = " SFQ "\n", packet->commproc ? packet->commproc : "<null>"));
      DPRINTF(("packet->auth_info = " SFQ "\n", packet->auth_info ?  packet->auth_info : "<null>"));
      DPRINTF(("packet->version = " sge_U32CFormat "\n",
               sge_u32c(packet->version)));
      DPRINTF(("packet->first_task = %p\n", packet->first_task));
      DPRINTF(("packet->last_task = %p\n", packet->last_task));

      task = packet->first_task;
      while (task != NULL) {
         sge_gdi_task_debug_print(task);
         task = task->next;
      }
   } else {
      DPRINTF(("packet is NULL\n"));;
   }
   DRETURN(ret);
}

#endif

/****** gdi/request_internal/sge_gdi_packet_create_base() ********************
*  NAME
*     sge_gdi_packet_create_base() -- ???
*
*  SYNOPSIS
*     sge_gdi_packet_class_t *
*     sge_gdi_packet_create_base(lList **answer_list)
*
*  FUNCTION
*     Creates a new GDI packet and initializes all base structure members
*     where necessary information is available.
*
*     "uid", "gid", "user" and "group" members part of sge_gdi_packet_class_t
*     will not be initialized with this function. Instead
*     sge_gdi_packet_create() can be used or the function
*     sge_gdi_packet_initialize_auth_info() can be called afterwards.
*
*  INPUTS
*     lList **answer_list - answer list in case of error
*
*  RESULT
*     sge_gdi_packet_class_t * - new GDI packet
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_create_base() is MT safe
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_create()
*     gdi/request_internal/sge_gdi_packet_initialize_auth_info()
******************************************************************************/
sge_gdi_packet_class_t *
sge_gdi_packet_create_base(lList **answer_list)
{
   sge_gdi_packet_class_t *ret = NULL;

   DENTER(TOP_LAYER, "sge_gdi_packet_create_base");
   ret = (sge_gdi_packet_class_t *) sge_malloc(sizeof(sge_gdi_packet_class_t));
   if (ret != NULL) {
      int local_ret1;
      int local_ret2;

      local_ret1 = pthread_mutex_init(&(ret->mutex), NULL);
      local_ret2 = pthread_cond_init(&(ret->cond), NULL);
      if (local_ret1 == 0 && local_ret2 == 0) {
         ret->is_intern_request = false;
         ret->is_gdi_request = true;
         ret->is_handled = false;
         ret->id = 0;
         ret->host = NULL;
         ret->commproc = NULL;
         ret->commproc_id = 0;

         ret->version = GRM_GDI_VERSION;
         ret->first_task = NULL;
         ret->last_task = NULL;
         ret->auth_info = NULL;
         ret->next = NULL;
         memset(&(ret->pb), 0, sizeof(sge_pack_buffer));
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EMALLOC,
                                 ANSWER_QUALITY_ERROR,
                                 "%s", MSG_MEMORY_MALLOCFAILED);
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EMALLOC,
                              ANSWER_QUALITY_ERROR, "%s", MSG_SGETEXT_NOMEM);
   }
   DRETURN(ret);
}

/****** gdi/request_internal/sge_gdi_packet_create() **************************
*  NAME
*     sge_gdi_packet_create() -- create a new GDI packet and initialize it
*
*  SYNOPSIS
*     sge_gdi_packet_class_t *
*     sge_gdi_packet_create(sge_gdi_ctx_class_t *ctx, lList **answer_list)
*
*  FUNCTION
*     Creates a new GDI packet element and initializes all members of the
*     structure.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - GDI context
*     lList **answer_list      - answer list
*
*  RESULT
*     sge_gdi_packet_class_t * - new packet element or NULL in case of errors
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_create() is MT safe
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_create_base()
*     gdi/request_internal/sge_gdi_packet_initialize_auth_info()
******************************************************************************/
sge_gdi_packet_class_t *
sge_gdi_packet_create(sge_gdi_ctx_class_t * ctx, lList **answer_list, bool use_euid_egid)
{
   sge_gdi_packet_class_t *ret = NULL;

   DENTER(TOP_LAYER, "sge_gdi_packet_create");
   ret = sge_gdi_packet_create_base(answer_list);
   if (ret != NULL) {
      sge_gdi_packet_initialize_auth_info(ctx, ret, answer_list, use_euid_egid);
   }
   DRETURN(ret);
}

/****** gdi/request_internal/sge_gdi_packet_append_task() *********************
*  NAME
*     sge_gdi_packet_append_task() -- append an additional GDI task
*
*  SYNOPSIS
*     bool
*     sge_gdi_packet_append_task(sge_gdi_packet_class_t *packet,
*                                lList **answer_list, u_long32 target,
*                                u_long32 command, lList **lp,
*                                lList **a_list, lCondition **condition,
*                                lEnumeration **enumeration,
*                                bool do_copy, bool do_verify)
*
*  FUNCTION
*     This function creates a new GDI task representing one
*     request of a multi GDI request. The task will be appended to
*     the list of tasks part of "packet". It will be initialized
*     with the values given by "target" and "command". Pointer
*     parameters like "lp", "a_list", "condition", and "enumeration"
*     will either be copied ("do_copy" == true) or they will just be
*     used as they are. In that case they will direct to NULL after
*     the function returns. The memory allocated by the provided
*     pointer parameters will be released when the task is destroyed
*     they are part of.
*
*     In case of any error the function will return "false"
*     and the "answer_list" will be filled with a message.
*     Causes for errors are:
*        - malloc failure
*        - verification failure (sge_gdi_task_verify)
*
*  INPUTS
*     sge_gdi_packet_class_t *packet - packet
*     lList **answer_list            - answer list used by this function
*     u_long32 target                - GDI target value
*     u_long32 command               - GDI command
*     lList **lp                     - list pointer
*     lList **a_list                 - answer list pointer
*     lCondition **condition         - CULL condition
*     lEnumeration **enumeration     - CULL enumeration
*     bool do_copy                   - do copy all elements passed
*                                      to this structure
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error (answer_list will be filled with detailed information)
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_append_task() is MT safe
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_task_create()
*     gdi/request_internal/sge_gdi_task_verify()
******************************************************************************/
bool
sge_gdi_packet_append_task(sge_gdi_packet_class_t * packet,
                           lList **answer_list, u_long32 target,
                           u_long32 command, lList **lp, lList **a_list,
                           lCondition **condition, lEnumeration **enumeration,
                           bool do_copy)
{
   bool ret = true;
   sge_gdi_task_class_t *task = NULL;

   DENTER(TOP_LAYER, "sge_gdi_packet_append_task");

   task =
      sge_gdi_task_create(packet, answer_list, target, command, lp, a_list,
                          condition, enumeration, do_copy, true);

   if (packet->last_task != NULL) {
      packet->last_task->next = task;
      packet->last_task = task;
   } else {
      packet->first_task = task;
      packet->last_task = task;
   }
   task = NULL;

   DRETURN(ret);
}

/****** gdi/request_internal/sge_gdi_task_get_operation_name() ****************
*  NAME
*     sge_gdi_task_get_operation_name() -- get command name 
*
*  SYNOPSIS
*     const char * 
*     sge_gdi_task_get_operation_name(sge_gdi_task_class_t *task) 
*
*  FUNCTION
*     This function returns a string of representing the command type
*     of a task part of a multi GDI request (e.g the function will return
*     "GET" when (task->command == SGE_GDI_GET))
*
*  INPUTS
*     sge_gdi_task_class_t *task - gdi task 
*
*  RESULT
*     const char * - string
*
*  NOTES
*     MT-NOTE: sge_gdi_task_get_operation_name() is MT safe 
*******************************************************************************/
const char *
sge_gdi_task_get_operation_name(sge_gdi_task_class_t *task)
{
   const char *ret = NULL;
   int operation = SGE_GDI_GET_OPERATION(task->command);

   switch (operation) {
      case SGE_GDI_GET:
         ret = "GET";
         break;
      case SGE_GDI_ADD:
         ret = "ADD";
         break;
      case SGE_GDI_DEL:
         ret = "DEL";
         break;
      case SGE_GDI_MOD:
         ret = "MOD";
         break;
      case SGE_GDI_COPY:
         ret = "COPY";
         break;
      case SGE_GDI_TRIGGER:
         ret = "TRIGGER";
         break;
      case SGE_GDI_PERMCHECK:
         ret = "PERMCHECK";
         break;
      case SGE_GDI_REPLACE:
         ret = "REPLACE";
         break;
      default:
         ret = "???";
         break;
   }
   return ret;
}

/****** gdi/request_internal/sge_gdi_packet_get_last_task_id() ****************
*  NAME
*     sge_gdi_packet_get_last_task_id() -- returns the last used task id
*
*  SYNOPSIS
*     u_long32
*     sge_gdi_packet_get_last_task_id(sge_gdi_packet_class_t *packet)
*
*  FUNCTION
*     Returns the last used task id in a package or 0 if there is no task.
*     Can be used to get the id of the last task created with
*     sge_gdi_packet_append_task()
*
*  INPUTS
*     sge_gdi_packet_class_t *packet - packet pointer
*
*  RESULT
*     u_long32 - task id in the range [1, ULONG32_MAX]
*                or 0
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_get_last_task_id() is not MT safe
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_task_create()
*     gdi/request_internal/sge_gdi_packet_append_task()
******************************************************************************/
u_long32
sge_gdi_packet_get_last_task_id(sge_gdi_packet_class_t * packet)
{
   u_long32 ret = 0;

   DENTER(TOP_LAYER, "sge_gdi_packet_get_last_task_id");

   if (packet->last_task != NULL) {
      ret = packet->last_task->id;
   }
   DRETURN(ret);
}

/****** gdi/request_internal/sge_gdi_packet_free() ****************************
*  NAME
*     sge_gdi_packet_free() -- free memory allocated by packet
*
*  SYNOPSIS
*     bool sge_gdi_packet_free(sge_gdi_packet_class_t **packet)
*
*  FUNCTION
*     Releases the memory allocated by all members of the "packet"
*     structure including the list of tasks and sublists part of the
*     tasks.
*
*     The caller has to take care that the packet is not part of
*     a packet queue when this function is called!
*
*  INPUTS
*     sge_gdi_packet_class_t **packet - packet
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_free() is not MT safe
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_task_create()
*     gdi/request_internal/sge_gdi_packet_create()
******************************************************************************/
bool
sge_gdi_packet_free(sge_gdi_packet_class_t ** packet)
{
   bool ret = true;

   DENTER(TOP_LAYER, "sge_gdi_packet_free");
   if (packet != NULL && *packet != NULL) {
      sge_gdi_task_class_t *task = NULL;
      sge_gdi_task_class_t *next_task = NULL;
      int local_ret1;
      int local_ret2;

      next_task = (*packet)->first_task;
      while ((task = next_task) != NULL) {
         next_task = task->next;
         sge_gdi_task_free(&task);
      }
      local_ret1 = pthread_mutex_destroy(&((*packet)->mutex));
      local_ret2 = pthread_cond_destroy(&((*packet)->cond));
      if (local_ret1 != 0 || local_ret2 != 0) {
         ret = false;
      }
      sge_free(&((*packet)->host));
      sge_free(&((*packet)->commproc));
      sge_free(&((*packet)->auth_info));
      sge_free(packet);
   }
   DRETURN(ret);
}

/****** sge_gdi_packet/sge_gdi_packet_verify_version() ************************
*  NAME
*     sge_gdi_packet_verify_version() -- verify packet version
*
*  SYNOPSIS
*     bool sge_gdi_packet_verify_version(sge_gdi_packet_class_t *packet,
*                                        lList **alpp)
*
*  FUNCTION
*     This function is the replacement for the function
*     verify_request_version() which was part of the source code
*     before the packet structure was introduced.
*
*     It compares the version information of the provided "packet"
*     with the compiled-in version number GRM_GDI_VERSION.
*
*     If both versions are not the same then it tries to find
*     if the client which provided us with this packet structure
*     has a higher version number or the binary executing
*     this function. In both cases the answer_list will
*     be filled with an appropriate message.
*
*  INPUTS
*     sge_gdi_packet_class_t *packet - packet
*     lList **alpp                   - answer list
*
*  RESULT
*     bool - error state
*        true  - same version
*        false - different version numbers
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_verify_version() is not MT safe
******************************************************************************/
bool
sge_gdi_packet_verify_version(sge_gdi_packet_class_t * packet, lList **alpp)
{
   bool ret = true;
   char *client_version = NULL;
   dstring ds;
   char buffer[256];
   const vdict_t *vp;
   const vdict_t *vdict = GRM_GDI_VERSION_ARRAY;
   u_long32 version = packet->version;

   DENTER(TOP_LAYER, "sge_gdi_packet_verify_version");
   sge_dstring_init(&ds, buffer, sizeof(buffer));

   if (version != GRM_GDI_VERSION) {
      for (vp = &vdict[0]; vp->version; vp++) {
         if (version == vp->version) {
            client_version = vp->release;
         }
      }

      if (client_version) {
         WARNING((SGE_EVENT, MSG_GDI_WRONG_GDI_SSISS, packet->host,
                  packet->commproc, (int)(packet->id), client_version,
                  feature_get_product_name(FS_VERSION, &ds)));
      } else {
         WARNING((SGE_EVENT, MSG_GDI_WRONG_GDI_SSIUS, packet->host,
                  packet->commproc, (int)(packet->id), sge_u32c(version),
                  feature_get_product_name(FS_VERSION, &ds)));
      }
      answer_list_add(alpp, SGE_EVENT, STATUS_EVERSION, ANSWER_QUALITY_ERROR);
      ret = false;
   }
   DRETURN(ret);
}

