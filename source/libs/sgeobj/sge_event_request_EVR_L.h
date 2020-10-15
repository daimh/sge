#ifndef __SGE_EVENTREQUESTL_H
#define __SGE_EVENTREQUESTL_H

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
#include "uti/sge_monitor.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */

/* documentation see libs/evc/sge_event_client.c */
enum {
   /* identification */
   EVR_operation = EVR_LOWERBOUND,        /* evm_request_t */
   EVR_timestamp,                         /* timestamp when operation was triggered */
   EVR_event_client_id,                   /* e.g. vor EVR_DEL_EVC request */
   EVR_event_number,                      /* for EVR_ACK_EVENT */
   EVR_session,                           /* DRMAA session */
   EVR_event_client,                      /* event client object, e.g. for EVR_ADD_EVC, EVR_MOD_EVC */
   EVR_event_list                         /* for EVR_ADD_EVENT request */
};

LISTDEF(EVR_Type)
   SGE_ULONG(EVR_operation, CULL_DEFAULT)
   SGE_ULONG(EVR_timestamp, CULL_DEFAULT)
   SGE_ULONG(EVR_event_client_id, CULL_DEFAULT)
   SGE_ULONG(EVR_event_number, CULL_DEFAULT)
   SGE_STRING(EVR_session, CULL_DEFAULT)
   SGE_OBJECT(EVR_event_client, EV_Type, CULL_DEFAULT)
   SGE_LIST(EVR_event_list, ET_Type, CULL_DEFAULT)
LISTEND 

NAMEDEF(EVRN)
   NAME("EVR_operation")
   NAME("EVR_timestamp")
   NAME("EVR_event_client_id")
   NAME("EVR_event_number")
   NAME("EVR_session")
   NAME("EVR_event_client")
   NAME("EVR_event_list")
NAMEEND

#define EVRS sizeof(EVRN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_EVENTREQUESTL_H */
