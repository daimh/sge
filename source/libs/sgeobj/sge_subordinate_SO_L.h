#ifndef __SGE_SUBORDINATE_SO_L_H
#define __SGE_SUBORDINATE_SO_L_H

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

/* *INDENT-OFF* */ 

enum {
   SO_name = SO_LOWERBOUND, 
   SO_threshold,
   SO_slots_sum,
   SO_seq_no,
   SO_action
};

/* We want to use the configuration parameter "subordinate_list"
 * for both the classic queue instance-wise suspend on subordinate
 * and the slot-wise suspend on subordinate.
 * The fields SO_name and SO_threshold are used by the queue instance-wise
 * suspend on subordinate, SO_name, SO_slots_sum, SO_seq_no and SO_action
 * are used by the slot-wise suspend on subordinate.
 * If SO_slots_sum is 0, it's queue instance-wise, otherwise slot-wise
 * suspend on subordinate that is configured.
 */

LISTDEF(SO_Type)
   JGDI_OBJ(Subordinate)
   SGE_STRING(SO_name, CULL_PRIMARY_KEY | CULL_SUBLIST) 
   SGE_ULONG(SO_threshold, CULL_SUBLIST)
   SGE_ULONG(SO_slots_sum, CULL_SUBLIST) /* for slot-wise sos */
   SGE_ULONG(SO_seq_no, CULL_SUBLIST)    /* for slot-wise sos */
   SGE_ULONG(SO_action, CULL_SUBLIST)
LISTEND

NAMEDEF(SON)
   NAME("SO_name")
   NAME("SO_threshold")
   NAME("SO_slots_sum")
   NAME("SO_seq_no")
   NAME("SO_action")
NAMEEND

#define SOS sizeof(SON)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_SUBORDINATEL_H */
