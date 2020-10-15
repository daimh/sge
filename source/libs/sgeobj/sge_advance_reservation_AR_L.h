#ifndef __SGE_ADVANCE_RESERVATION_AR_L_H
#define __SGE_ADVANCE_RESERVATION_AR_L_H

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

#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* Advance Reservation Object */
enum {
   AR_id = AR_LOWERBOUND,
   AR_name,
   AR_account,
   AR_owner,
   AR_group,

   AR_submission_time,
   AR_start_time,               /* required */
   AR_end_time,                 /* required */
   AR_duration,
   AR_verify,                   /* just verify the reservation or final case */
   AR_error_handling,           /* how to deal with soft and hard exceptions */

   AR_state,                    /* state of the AR */

   AR_checkpoint_name,          /* Named checkpoint */

   AR_resource_list,
   AR_resource_utilization,
   AR_queue_list,

   AR_granted_slots,            /* spooled value */
   AR_reserved_queues,          /* runtime value */

   AR_mail_options,     
   AR_mail_list,

   AR_pe,
   AR_pe_range,
   AR_granted_pe,
   AR_master_queue_list,

   AR_acl_list,
   AR_xacl_list,
   AR_type,                     /* -now switch */
   AR_qi_errors                 /* reserved queue instances in error state */
};

LISTDEF(AR_Type)
   JGDI_ROOT_OBJ(AdvanceReservation, SGE_AR_LIST, ADD | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_AR_ADD) | MODIFY(sgeE_AR_MOD) | DELETE(sgeE_AR_DEL) | GET_LIST(sgeE_AR_LIST))
   SGE_ULONG(AR_id, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL)
   SGE_STRING(AR_name, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_STRING(AR_account, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_STRING(AR_owner, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_STRING(AR_group, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)

   SGE_ULONG(AR_submission_time, CULL_DEFAULT | CULL_SPOOL)
   SGE_ULONG(AR_start_time, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_ULONG(AR_end_time, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_ULONG(AR_duration, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   SGE_ULONG(AR_verify, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_ULONG(AR_error_handling, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   SGE_ULONG(AR_state, CULL_DEFAULT | CULL_JGDI_RO | CULL_SPOOL)

   SGE_STRING(AR_checkpoint_name, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)   

   SGE_LIST(AR_resource_list, CE_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_LIST(AR_resource_utilization, RUE_Type, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_LIST(AR_queue_list, QR_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   SGE_LIST(AR_granted_slots, JG_Type, CULL_SPOOL | CULL_JGDI_RO)
   SGE_LIST(AR_reserved_queues, QU_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)

   SGE_ULONG(AR_mail_options, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF) 
   SGE_LIST(AR_mail_list, MR_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)  

   SGE_STRING(AR_pe, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)              
   SGE_LIST(AR_pe_range, RN_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)     
   SGE_STRING(AR_granted_pe, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)              
   SGE_LIST(AR_master_queue_list, QR_Type, CULL_DEFAULT | CULL_SPOOL)

   SGE_LIST(AR_acl_list, ARA_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_LIST(AR_xacl_list, ARA_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_ULONG(AR_type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_ULONG(AR_qi_errors, CULL_DEFAULT | CULL_JGDI_HIDDEN)
LISTEND

NAMEDEF(ARN)
   NAME("AR_id")
   NAME("AR_name")
   NAME("AR_account")
   NAME("AR_owner")
   NAME("AR_group")

   NAME("AR_submission_time")
   NAME("AR_start_time")
   NAME("AR_end_time")
   NAME("AR_duration")
   NAME("AR_verify")
   NAME("AR_error_handling")

   NAME("AR_state")

   NAME("AR_checkpoint_name")

   NAME("AR_resource_list")
   NAME("AR_resource_utilization")
   NAME("AR_queue_list")

   NAME("AR_granted_slots")
   NAME("AR_reserved_queues")

   NAME("AR_mail_options")
   NAME("AR_mail_list")

   NAME("AR_pe")
   NAME("AR_pe_range")
   NAME("AR_granted_pe")
   NAME("AR_master_queue_list")

   NAME("AR_acl_list")  /* user + group or userset */
   NAME("AR_xacl_list")
   NAME("AR_type")
   NAME("AR_qi_errors")
NAMEEND

#define ARS sizeof(ARN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif /* __SGE_ADVANCE_RESERVATIONL_H */
