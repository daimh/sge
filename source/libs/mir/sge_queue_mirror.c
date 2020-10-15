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

#include "sge.h"

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_object.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"

#include "mir/msg_mirlib.h"
#include "mir/sge_mirror.h"
#include "mir/sge_queue_mirror.h"

sge_callback_result
cqueue_update_master_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
                          sge_event_action action, lListElem *event, void *clientdata)
{
   sge_callback_result ret = SGE_EMA_OK;
   const char *name = NULL;
   lList *qinstance_list = NULL;
   lListElem *cqueue = NULL;
   lList **list = NULL;
   const lDescr *list_descr = NULL;

   DENTER(TOP_LAYER, "cqueue_update_master_list");
   name = lGetString(event, ET_strkey);
   list = sge_master_list(object_base, SGE_TYPE_CQUEUE); 
   list_descr = lGetListDescr(lGetList(event, ET_new_version));
   cqueue = cqueue_list_locate(*list, name);

   if ((action == SGE_EMA_MOD || action == SGE_EMA_ADD) 
       && cqueue != NULL) {
      /*
       * modify events for CQ_Type objects; we may not update
       * - CQ_qinstances - it is maintained by QINSTANCE events
       */         
      lXchgList(cqueue, CQ_qinstances, &qinstance_list);
   }
   
   ret &= (sge_mirror_update_master_list(list, list_descr, cqueue,
                         name, action, event) == SGE_EM_OK) ? SGE_EMA_OK : SGE_EMA_FAILURE;
   cqueue = cqueue_list_locate(*list, name);

   if ((action == SGE_EMA_MOD || action == SGE_EMA_ADD)
       && cqueue != NULL) {
      /*
       * Replace CQ_qinstances list
       */         
      lXchgList(cqueue, CQ_qinstances, &qinstance_list);
      lFreeList(&qinstance_list);
   }

   DEXIT;
   return ret;
}

sge_callback_result
qinstance_update_cqueue_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
                             sge_event_action action, lListElem *event, void *clientdata)
{
   sge_callback_result ret = SGE_EMA_OK;
   const char *name = NULL;
   const char *hostname = NULL;
   lListElem *cqueue = NULL;

   DENTER(TOP_LAYER, "qinstance_update_cqueue_list");
   name = lGetString(event, ET_strkey);
   hostname = lGetString(event, ET_strkey2);

   cqueue = cqueue_list_locate( *(sge_master_list(object_base, SGE_TYPE_CQUEUE)), name);
                        
   if (cqueue != NULL) {
      dstring key_buffer = DSTRING_INIT;
      lList *list = lGetList(cqueue, CQ_qinstances);
      const lDescr *list_descr = lGetListDescr(lGetList(event, ET_new_version));
      
      lListElem *qinstance = qinstance_list_locate(list, hostname, NULL);
      const char *key = NULL;
      bool is_list = list != NULL ? true : false;
      
      sge_dstring_sprintf(&key_buffer, SFN"@"SFN, name, hostname);
      key = sge_dstring_get_string(&key_buffer);

      if (action == SGE_EMA_MOD) {
         u_long32 type = lGetUlong(event, ET_type);

         if (type == sgeE_QINSTANCE_SOS || 
             type == sgeE_QINSTANCE_USOS) {
            if (qinstance != NULL) {
               if (type == sgeE_QINSTANCE_SOS) {
                  qinstance_state_set_susp_on_sub(qinstance, true);
               } else {
                  qinstance_state_set_susp_on_sub(qinstance, false);
               }
            } else {
               ERROR((SGE_EVENT, MSG_QINSTANCE_CANTFINDFORUPDATEIN_SS, key,
                      SGE_FUNC));
               ret = SGE_EMA_FAILURE;
            }
            sge_dstring_free(&key_buffer);
            DEXIT;
            return ret;
         }
      }
      ret &= (sge_mirror_update_master_list(&list, list_descr, qinstance, key,
                             action, event) == SGE_EM_OK) ? true : false;
      sge_dstring_free(&key_buffer);
      if (!is_list) {
         lSetList(cqueue, CQ_qinstances, list);
      }
   } else {
      ERROR((SGE_EVENT, MSG_CQUEUE_CANTFINDFORUPDATEIN_SS, name, SGE_FUNC));
      ret = SGE_EMA_FAILURE;
   }

   DEXIT;
   return ret;
}
