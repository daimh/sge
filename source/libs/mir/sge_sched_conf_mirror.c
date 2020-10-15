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

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_answer.h"

#include "mir/msg_mirlib.h"
#include "mir/sge_mirror.h"
#include "mir/sge_sched_conf_mirror.h"

#include "sge.h"

/****** Eventmirror/schedd_conf/schedd_conf_update_master_list() ***************
*  NAME
*     schedd_conf_update_master_list() -- update the scheduler configuration
*
*  SYNOPSIS
*     bool 
*     schedd_conf_update_master_list(sge_object_type type, 
*                                    sge_event_action action,
*                                    lListElem *event, void *clientdata)
*
*  FUNCTION
*     Update the global master list of scheduler configurations
*     based on an event.
*     The function is called from the event mirroring interface.
*     The list only contains one element that is replaced when a
*     modify event arrives.
*
*  INPUTS
*     sge_object_type type     - event type
*     sge_event_action action - action to perform
*     lListElem *event        - the raw event
*     void *clientdata        - client data
*
*  RESULT
*     bool - true, if update is successfull, else false
*
*  NOTES
*     The function should only be called from the event mirror interface.
*
*  SEE ALSO
*     Eventmirror/--Eventmirror
*******************************************************************************/
sge_callback_result
schedd_conf_update_master_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
                               sge_event_action action, lListElem *event, void *clientdata)
{
   lList *list = NULL;
   lList *answer_list = NULL;
   lDescr *list_descr;

   lList *data_list;
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "schedd_conf_update_master_list");

   list_descr = SC_Type;

   if ((data_list = lGetList(event, ET_new_version)) != NULL) {
      if ((ep = lFirst(data_list)) != NULL) {
         ep = lDechainElem(data_list, ep);
      }
   }

   /* if neccessary, create list and copy schedd info */
   if (ep != NULL) {
      list = lCreateList("schedd config", list_descr);
      lAppendElem(list, ep);
   }

   if (!sconf_set_config(&list, &answer_list)) {
      lFreeList(&list);
      answer_list_output(&answer_list);
   }

   DEXIT;
   return SGE_EMA_OK;
}
