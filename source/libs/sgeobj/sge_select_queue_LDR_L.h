#ifndef __SGE_SELECT_QUEUE_LDR_L_H
#define __SGE_SELECT_QUEUE_LDR_L_H

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

/****** spool/--LDR_Type***************************************
*
*  NAME
*    LDR_Type -- is a high level structure to monitor changes
*                in consumables, which are used as load threasholds.
*
*  ELEMENTS
*   SGE_REF(LDR_global,CULL_ANY_SUBTYPE, CULL_DEFAULT)
*     Reference to a global cunsumable object
*
*   SGE_REF(LDR_host,CULL_ANY_SUBTYPE, CULL_DEFAULT)
*     Reference to a host consumable object
*
*   SGE_REF(LDR_queue,CULL_ANY_SUBTYPE, CULL_DEFAULT)
*     Reference to a queue consumable object
*
*   SGE_LIST(LDR_queue_ref_list, QR_Type, CULL_DEFAULT)
*     A list of queues instances, which are using all these 
*     consumables as load thresholds.
*
*  FUNCTION
*     All three consumables form a kind of queue_consumable_category.
*     All queues which are listed in here are using the same consumables
*     to compute the load alarm. The whole list is using references because
*     it is easier to monitor changes in one of the consumable objects and
*     the function calculation the load alarm for each queue knows exactly
*     which queues need to be recalculated.
*
*
*  IMPORTANT
*     This list a fixed position list. Do not use what filtering on the
*     elements or the code will break!
*
****************************************************************************
*/
enum {
   LDR_queue_ref_list_pos = 0,
   LDR_limit_pos,
   LDR_global_pos, 
   LDR_host_pos,
   LDR_queue_pos
}; 

enum {
   LDR_queue_ref_list = LDR_LOWERBOUND,
   LDR_limit,
   LDR_global,
   LDR_host,
   LDR_queue
};

LISTDEF(LDR_Type)
   SGE_LIST(LDR_queue_ref_list, QR_Type, CULL_DEFAULT)
   SGE_STRING(LDR_limit, CULL_DEFAULT)
   SGE_REF(LDR_global,CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(LDR_host,CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_REF(LDR_queue,CULL_ANY_SUBTYPE, CULL_DEFAULT)
LISTEND 

NAMEDEF(LDRN)
   NAME("LDR_queue_ref_list")
   NAME("LDR_limit")
   NAME("LDR_global")
   NAME("LDR_host")
   NAME("LDR_queue")
NAMEEND

#define LDRS sizeof(LDRN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif
