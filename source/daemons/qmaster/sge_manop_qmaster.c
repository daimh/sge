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
#include <stdio.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_uidgid.h"

#include "comm/commlib.h"

#include "cull/cull.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_manop.h"

#include "spool/sge_spooling.h"

#include "sge_persistence_qmaster.h"
#include "sge_manop_qmaster.h"
#include "sge_event_master.h"
#include "msg_common.h"
#include "msg_qmaster.h"

/* ------------------------------------------------------------

   sge_add_manop() - adds an manop list to the global manager/operator
                     list

   if the invoking process is the qmaster 
   the added manop list is spooled in the MANAGER_FILE/OPERATOR_FILE

*/
int sge_add_manop(
sge_gdi_ctx_class_t *ctx,
lListElem *ep,
lList **alpp,
char *ruser,
char *rhost,
u_long32 target  /* may be SGE_UM_LIST or SGE_UO_LIST */
) {
   const char *manop_name;
   const char *object_name;
   lList **lpp = NULL;
   lListElem *added;
   int pos;
   int key;
   lDescr *descr = NULL;
   ev_event eve = sgeE_EVENTSIZE; 

   DENTER(TOP_LAYER, "sge_add_manop");

   if ( !ep || !ruser || !rhost ) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   switch (target) {
   case SGE_UM_LIST:
      lpp = object_type_get_master_list(SGE_TYPE_MANAGER);
      object_name = MSG_OBJ_MANAGER;
      key = UM_name;
      descr = UM_Type;
      eve = sgeE_MANAGER_ADD;
      break;
   case SGE_UO_LIST:
      lpp = object_type_get_master_list(SGE_TYPE_OPERATOR);
      object_name = MSG_OBJ_OPERATOR;
      key = UO_name;
      descr = UO_Type;
      eve = sgeE_OPERATOR_ADD;
      break;
   default :
      DPRINTF(("unknown target passed to %s\n", SGE_FUNC));
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   /* ep is no acl element, if ep has no UM_name/UO_name */
   if ((pos = lGetPosViaElem(ep, key, SGE_NO_ABORT)) < 0) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS,
            lNm2Str(key), SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   manop_name = lGetPosString(ep, pos);
   if (!manop_name) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   if (lGetElemStr(*lpp, key, manop_name)) {
      ERROR((SGE_EVENT, MSG_SGETEXT_ALREADYEXISTS_SS, object_name, manop_name));
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EEXIST;
   }

   /* update in interal lists */
   added = lAddElemStr(lpp, key, manop_name, descr);

   /* update on file */
   if(!sge_event_spool(ctx, alpp, 0, eve, 
                       0, 0, manop_name, NULL, NULL,
                       added, NULL, NULL, true, true)) {
      ERROR((SGE_EVENT, MSG_CANTSPOOL_SS, object_name, manop_name));
      answer_list_add(alpp, SGE_EVENT, STATUS_EDISK, ANSWER_QUALITY_ERROR);
   
      /* remove element from list */
      lRemoveElem(*lpp, &added);

      DEXIT;
      return STATUS_EDISK;
   }

   INFO((SGE_EVENT, MSG_SGETEXT_ADDEDTOLIST_SSSS,
            ruser, rhost, manop_name, object_name));
   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
   DEXIT;
   return STATUS_OK;
}

/****** sge_manop_qmaster/sge_del_manop() **************************************
*  NAME
*     sge_del_manop() -- delete manager or operator
*
*  SYNOPSIS
*     int 
*     sge_del_manop(sge_gdi_ctx_class_t *ctx, lListElem *ep, lList **alpp, 
*                   char *ruser, char *rhost, u_long32 target) 
*
*  FUNCTION
*     Deletes a manager or an operator from the corresponding master list.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - gdi context
*     lListElem *ep            - the manager/operator to delete
*     lList **alpp             - answer list to return messages
*     char *ruser              - user having triggered the action
*     char *rhost              - host from which the action has been triggered
*     u_long32 target          - SGE_UM_LIST or SGE_UO_LIST
*
*  RESULT
*     int - STATUS_OK or STATUS_* error code
*
*  NOTES
*     MT-NOTE: sge_del_manop() is MT safe - if we hold the global lock.
*******************************************************************************/
int sge_del_manop(sge_gdi_ctx_class_t *ctx, lListElem *ep, lList **alpp, char *ruser, char *rhost, u_long32 target)
{
   lListElem *found;
   int pos;
   const char *manop_name;
   const char *object_name;
   lList **lpp = NULL;
   int key = NoName;
   ev_event eve = sgeE_EVENTSIZE;

   DENTER(TOP_LAYER, "sge_del_manop");

   if (ep == NULL || ruser == NULL || rhost == NULL) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EUNKNOWN);
   }

   switch (target) {
      case SGE_UM_LIST:
         lpp = object_type_get_master_list(SGE_TYPE_MANAGER);
         object_name = MSG_OBJ_MANAGER;
         key = UM_name;
         eve = sgeE_MANAGER_DEL;
         break;
      case SGE_UO_LIST:
         lpp = object_type_get_master_list(SGE_TYPE_OPERATOR);
         object_name = MSG_OBJ_OPERATOR;
         key = UO_name;
         eve = sgeE_OPERATOR_DEL;
         break;
      default :
         DPRINTF(("unknown target passed to %s\n", SGE_FUNC));
         DRETURN(STATUS_EUNKNOWN);
   }

   /* ep is no manop element, if ep has no UM_name/UO_name */
   if ((pos = lGetPosViaElem(ep, key, SGE_NO_ABORT)) < 0) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS, lNm2Str(key), SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EUNKNOWN);
   }

   manop_name = lGetPosString(ep, pos);
   if (manop_name == NULL) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EUNKNOWN);
   }

   /* prevent removing of root from man/op-list */
   if (strcmp(manop_name, "root") == 0) {
      ERROR((SGE_EVENT, MSG_SGETEXT_MAY_NOT_REMOVE_USER_FROM_LIST_SS, "root", object_name));  
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EEXIST);
   }

   /* prevent removing the admin user from man/op-list */
   if (strcmp(manop_name, ctx->get_admin_user(ctx)) == 0) {
      ERROR((SGE_EVENT, MSG_SGETEXT_MAY_NOT_REMOVE_USER_FROM_LIST_SS,
             ctx->get_admin_user(ctx), object_name));  
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EEXIST);
   }

   found = lGetElemStr(*lpp, key, manop_name);
   if (!found) {
      ERROR((SGE_EVENT, MSG_SGETEXT_DOESNOTEXIST_SS, object_name, manop_name));
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EEXIST);
   }
   
   lDechainElem(*lpp, found);

   /* update on file */
   if (!sge_event_spool(ctx, alpp, 0, eve,
                        0, 0, manop_name, NULL, NULL,
                        NULL, NULL, NULL, true, true)) {
      ERROR((SGE_EVENT, MSG_CANTSPOOL_SS, object_name, manop_name));
      answer_list_add(alpp, SGE_EVENT, STATUS_EDISK, ANSWER_QUALITY_ERROR);
   
      /* chain in again */
      lAppendElem(*lpp, found);

      DRETURN(STATUS_EDISK);
   }
   lFreeElem(&found);

   INFO((SGE_EVENT, MSG_SGETEXT_REMOVEDFROMLIST_SSSS, ruser, rhost, manop_name, object_name));
   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
   DRETURN(STATUS_OK);
}

