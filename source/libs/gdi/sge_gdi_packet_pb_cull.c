
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

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include <pwd.h>

#ifndef WIN32NATIVE
#	include <unistd.h>
#endif
#include <stdlib.h>

#include "sge.h"

#include "basis_types.h"

#include "cull/pack.h"

#include "uti/sge_mtutil.h"
#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "gdi/sge_gdi2.h"
#include "gdi/sge_gdi_packet_pb_cull.h"
#include "gdi/sge_gdi_packet.h"
#include "gdi/msg_gdilib.h"
#include "gdi/sge_security.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_event.h"
#include "sgeobj/sge_object.h"

#include "msg_common.h"
#include "msg_qmaster.h"

static bool
sge_pack_gdi_info(u_long32 command)
{
   bool ret = true;

   DENTER(GDI_LAYER, "sge_pack_gdi_info");
   switch (command) {
   case SGE_GDI_GET:
      DPRINTF(("packing SGE_GDI_GET request\n"));
      break;
   case SGE_GDI_ADD:
   case SGE_GDI_ADD | SGE_GDI_RETURN_NEW_VERSION:
   case SGE_GDI_ADD | SGE_GDI_SET_ALL:
   case SGE_GDI_ADD | SGE_GDI_EXECD_RESTART:
      DPRINTF(("packing SGE_GDI_ADD request\n"));
      break;
   case SGE_GDI_DEL:
   case SGE_GDI_DEL | SGE_GDI_ALL_JOBS:
   case SGE_GDI_DEL | SGE_GDI_ALL_USERS:
   case SGE_GDI_DEL | SGE_GDI_ALL_JOBS | SGE_GDI_ALL_USERS:
      DPRINTF(("packing SGE_GDI_DEL request\n"));
      break;
   case SGE_GDI_MOD:
   case SGE_GDI_MOD | SGE_GDI_ALL_JOBS:
   case SGE_GDI_MOD | SGE_GDI_ALL_USERS:
   case SGE_GDI_MOD | SGE_GDI_ALL_JOBS | SGE_GDI_ALL_USERS:
   case SGE_GDI_MOD | SGE_GDI_APPEND:
   case SGE_GDI_MOD | SGE_GDI_REMOVE:
   case SGE_GDI_MOD | SGE_GDI_CHANGE:
   case SGE_GDI_MOD | SGE_GDI_SET_ALL:
      DPRINTF(("packing SGE_GDI_MOD request\n"));
      break;
   case SGE_GDI_TRIGGER:
      DPRINTF(("packing SGE_GDI_TRIGGER request\n"));
      break;
   case SGE_GDI_PERMCHECK:
      DPRINTF(("packing SGE_GDI_PERMCHECK request\n"));
      break;
   case SGE_GDI_SPECIAL:
      DPRINTF(("packing special things\n"));
      break;
   case SGE_GDI_COPY:
      DPRINTF(("request denied\n"));
      break;
   case SGE_GDI_REPLACE:
   case SGE_GDI_REPLACE | SGE_GDI_SET_ALL:
      DPRINTF(("packing SGE_GDI_REPLACE request\n"));
      break;
   default:
      ERROR((SGE_EVENT, MSG_GDI_ERROR_INVALIDVALUEXFORARTOOP_D,
             sge_u32c(command)));
      ret = false;
   }

   DRETURN(ret);
}

static bool
sge_gdi_map_pack_errors(int pack_ret, lList **answer_list)
{
   bool ret = true;

   DENTER(GDI_LAYER, "sge_gdi_map_pack_errors");
   switch (pack_ret) {
   case PACK_SUCCESS:
      break;
   case PACK_ENOMEM:
      answer_list_add_sprintf(answer_list, STATUS_ERROR2,
                              ANSWER_QUALITY_ERROR, "%s",
                              MSG_GDI_MEMORY_NOTENOUGHMEMORYFORPACKINGGDIREQUEST);
      break;
   case PACK_FORMAT:
      answer_list_add_sprintf(answer_list, STATUS_ERROR3,
                              ANSWER_QUALITY_ERROR,
                              "%s", MSG_GDI_REQUESTFORMATERROR);
      break;
   default:
      answer_list_add_sprintf(answer_list, STATUS_ERROR1,
                              ANSWER_QUALITY_ERROR, "%s",
                              MSG_GDI_UNEXPECTEDERRORWHILEPACKINGGDIREQUEST);
      break;
   }
   ret = (pack_ret == PACK_SUCCESS) ? true : false;

   DRETURN(ret);
}

/****** gdi/request_internal/sge_gdi_packet_get_pb_size() ********************
*  NAME
*     sge_gdi_packet_get_pb_size() -- returns the needed puckbuffer size 
*
*  SYNOPSIS
*     u_long32 sge_gdi_packet_get_pb_size(sge_gdi_packet_class_t *packet) 
*
*  FUNCTION
*     This function checks how lage a packbuffer needs to be so that
*     the given "packet" can be packed completely. 
*
*     This operation is needed to avoid continuous reallocation
*     when the real buffer should be allocated. 
*
*  INPUTS
*     sge_gdi_packet_class_t *packet - packet pointer 
*
*  RESULT
*     u_long32 - size in byte 
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_get_pb_size() is MT safe 
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_pack_task() 
*******************************************************************************/
u_long32
sge_gdi_packet_get_pb_size(sge_gdi_packet_class_t *packet)
{
   u_long32 ret = 0;

   DENTER(TOP_LAYER, "sge_gdi_packet_get_pb_size");
   if (packet != NULL) {
      bool local_ret;
      lList *local_answer_list = NULL;
      sge_pack_buffer pb;

      init_packbuffer(&pb, 0, 1);
      local_ret = sge_gdi_packet_pack(packet, &local_answer_list, &pb);
      if (local_ret) {
         ret = pb_used(&pb);
      }
      clear_packbuffer(&pb);
   }
   DRETURN(ret);
}

/****** gdi/request_internal/sge_gdi_packet_unpack() *************************
*  NAME
*     sge_gdi_packet_unpack() -- unpacks a GDI packet 
*
*  SYNOPSIS
*     bool 
*     sge_gdi_packet_unpack(sge_gdi_packet_class_t **packet, 
*                           lList **answer_list, sge_pack_buffer *pb) 
*
*  FUNCTION
*     This function unpacks all data representing a single or multi
*     GDI request. The information is parsed from the given packing 
*     buffer "pb" and is stored into "packet". Necessary memory will
*     be allocated.
*
*  INPUTS
*     sge_gdi_packet_class_t ** packet - new GDI packet 
*     lList **answer_list              - answer_list 
*     sge_pack_buffer *pb              - packing buffer 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_unpack() is MT safe 
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_get_pb_size() 
*     gdi/request_internal/sge_gdi_packet_pack_task() 
*     gdi/request_internal/sge_gdi_packet_pack()
*******************************************************************************/
bool
sge_gdi_packet_unpack(sge_gdi_packet_class_t **packet, lList **answer_list,
                      sge_pack_buffer *pb)
{
   bool aret = true;		/* fixme: is this the right default
				   (for when *packet == NULL)? */
   bool has_next;
   int pack_ret;

   DENTER(TOP_LAYER, "sge_gdi_packet_unpack");
   *packet = sge_gdi_packet_create_base(answer_list);
   if (*packet != NULL) {
      bool first = true;

      do {
         u_long32 target = 0;
         u_long32 command = 0;
         lList *data_list = NULL;
         u_long32 version = 0;
         lList *a_list = NULL;
         lCondition *condition = NULL;
         lEnumeration *enumeration = NULL;
         char *auth_info = NULL;
         u_long32 task_id = 0;
         u_long32 packet_id = 0;
         u_long32 has_next_int = 0;

         if ((pack_ret = unpackint(pb, &(command)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = unpackint(pb, &(target)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = unpackint(pb, &(version)))) {
            goto error_with_mapping;
         }
         /* JG: TODO (322): At this point we should check the version! 
          **                 The existent check function sge_gdi_packet_verify_version
          **                 cannot be called as neccesary data structures are 
          **                 available here (e.g. answer list).
          **                 Better do these changes at a more general place 
          **                 together with (hopefully coming) further communication
          **                 redesign.
          */
         if ((pack_ret = cull_unpack_list(pb, &(data_list)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = cull_unpack_list(pb, &(a_list)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = cull_unpack_cond(pb, &(condition)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = cull_unpack_enum(pb, &(enumeration)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = unpackstr(pb, &(auth_info)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = unpackint(pb, &(task_id)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = unpackint(pb, &(packet_id)))) {
            goto error_with_mapping;
         }
         if ((pack_ret = unpackint(pb, &has_next_int))) {
            goto error_with_mapping;
         }
         has_next = (has_next_int > 0) ? true : false;

         if (first) {
            (*packet)->id = packet_id;
            (*packet)->version = version;
            (*packet)->auth_info = auth_info;
            auth_info = NULL;
            first = false;
         } else {
            sge_free(&auth_info);
         }

         /* EB: TODO: ST: cleanup - set last parameter to true */
         aret = sge_gdi_packet_append_task(*packet, answer_list, target,
                                           command, &data_list, &a_list,
                                           &condition, &enumeration,
                                           false);
         if (aret == false) {
            goto error;
         }
      } while (has_next);
   }
   DRETURN(aret);
 error_with_mapping:
   aret = sge_gdi_map_pack_errors(pack_ret, answer_list);
 error:
   /* Don't free packet; the calls of this function access it even
      when aret is false.  */
   DRETURN(aret);
}

/****** gdi/request_internal/sge_gdi_packet_pack() ***************************
*  NAME
*     sge_gdi_packet_pack() -- pack a GDI packet 
*
*  SYNOPSIS
*     bool 
*     sge_gdi_packet_pack(sge_gdi_packet_class_t * packet, lList**answer_list, 
*                         sge_pack_buffer *pb) 
*
*  FUNCTION
*     This functions packs all data representing a multi GDI request
*     into "pb". Errors will be reported with a corresponding
*     "answer_list" message and a negative return value.
*
*     "pb" has to be initialized before this function is called.
*     init_packbuffer() or a similar function has do be used to
*     initialize this "pb". The function sge_gdi_packet_get_pb_size()
*     might be used to calculate the maximum size as if the buffer
*     would be needed to pack all tasks of a multi GDI request. 
*     Using this size as initial size for the "pb"
*     will prevent continuous reallocation of memory in this 
*     function.
*
*  INPUTS
*     sge_gdi_packet_class_t * packet - GDI packet 
*     lList **answer_list             - answer list 
*     sge_pack_buffer *pb             - backbuffer 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_pack() is MT safe 
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_get_pb_size() 
*     gdi/request_internal/sge_gdi_packet_pack_task() 
*     gdi/request_internal/sge_gdi_packet_unpack()
*******************************************************************************/
bool
sge_gdi_packet_pack(sge_gdi_packet_class_t *packet, lList **answer_list,
                    sge_pack_buffer *pb)
{
   bool ret = true;
   sge_gdi_task_class_t *task = NULL;

   DENTER(TOP_LAYER, "sge_gdi_packet_pack");

   task = packet->first_task;
   while (ret && task != NULL) {
      ret |= sge_gdi_packet_pack_task(packet, task, answer_list, pb);
      task = task->next;
   }
   ret = ((sge_put_auth_token(packet, answer_list) != auth_failed) && ret);

   DRETURN(ret);
}

/****** gdi/request_internal/sge_gdi_packet_pack_task() **********************
*  NAME
*     sge_gdi_packet_pack_task() -- pack a single GDI task 
*
*  SYNOPSIS
*     bool 
*     sge_gdi_packet_pack_task(sge_gdi_packet_class_t * packet, 
*                              sge_gdi_task_class_t * task, 
*                              lList **answer_list, 
*                              sge_pack_buffer *pb) 
*
*  FUNCTION
*     This functions packs all data representing one GDI request
*     of a mutli GDI request (represented by "packet" and "task")
*     into "pb". Errors will be reported with a corresponding
*     "answer_list" message and a negative return value.
*
*     "pb" has to be initialized before this function is called.
*     init_packbuffer() or a similar function has do be used to
*     initialize this "pb". The function sge_gdi_packet_get_pb_size()
*     might be used to calculate the maximum size as if the buffer
*     would be needed to pack all tasks of a multi GDI request. 
*     Using this size as initial size for the "pb"
*     will prevent continuous reallocation of memory in this 
*     function.
*
*  INPUTS
*     sge_gdi_packet_class_t * packet - GDI packet 
*     sge_gdi_task_class_t * task     - GDI task 
*     lList **answer_list             - answer_list 
*     sge_pack_buffer *pb             - packing buffer 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - failure 
*
*  NOTES
*     MT-NOTE: sge_gdi_packet_pack_task() is MT safe 
*
*  SEE ALSO
*     gdi/request_internal/sge_gdi_packet_get_pb_size() 
*     gdi/request_internal/sge_gdi_packet_pack() 
*******************************************************************************/
bool
sge_gdi_packet_pack_task(sge_gdi_packet_class_t *packet,
                         sge_gdi_task_class_t *task, lList **answer_list,
                         sge_pack_buffer *pb)
{
   bool ret = true;
   int pack_ret = PACK_SUCCESS;

   DENTER(TOP_LAYER, "sge_gdi_packet_pack_task");

   if ((task != NULL) && (packet != NULL)
       && (packet->is_intern_request == false)) {
      sge_pack_gdi_info(task->command);

      /* ===> pack the prefix */
      pack_ret = packint(pb, task->command);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
      pack_ret = packint(pb, task->target);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
      pack_ret = packint(pb, packet->version);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }

      /* 
       * if the lSelect call was postponed then it will be done here.
       * here we are able to pack the result list directly into the packbuffer.
       * additionally it is necessary to add an answer to the answer list.
       * (which will be packed below). 
       */
      if (task->do_select_pack_simultaneous) {
         lSelectHashPack("", task->data_list, task->condition,
                         task->enumeration, false, pb);
         lFreeWhat(&(task->enumeration));
         lFreeWhere(&(task->condition));
         task->data_list = NULL;

         /* DIRTY HACK: The "ok" message should be removed from the answer list
          * 05/21/2007 qualitiy was ANSWER_QUALITY_INFO but this results in "ok"
          * messages on qconf side */
         answer_list_add(&(task->answer_list), MSG_GDI_OKNL, STATUS_OK,
                         ANSWER_QUALITY_END);
      } else {
         /* ===> pack the list */
         pack_ret = cull_pack_list(pb, task->data_list);
         if (pack_ret != PACK_SUCCESS) {
            goto error_with_mapping;
         }
      }

      /* ===> pack the suffix */
      pack_ret = cull_pack_list(pb, task->answer_list);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
      pack_ret = cull_pack_cond(pb, task->condition);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
      pack_ret = cull_pack_enum(pb, task->enumeration);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
      pack_ret = packstr(pb, packet->auth_info);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
      pack_ret = packint(pb, task->id);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
      pack_ret = packint(pb, packet->id);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
      pack_ret = packint(pb, (task->next != NULL) ? 1 : 0);
      if (pack_ret != PACK_SUCCESS) {
         goto error_with_mapping;
      }
   }

   DRETURN(ret);
 error_with_mapping:
   ret = sge_gdi_map_pack_errors(pack_ret, answer_list);
   DRETURN(ret);
}

