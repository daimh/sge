#ifndef __SGE_SELECT_QUEUE_QRL_L_H
#define __SGE_SELECT_QUEUE_QRL_L_H

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

/****** spool/--QR_Type***************************************
*
*  NAME
*     QR_Type -- sub list of queue references
*
*  ELEMENTS
*     SGE_REF(QR_Queue,CULL_ANY_SUBTYPE, CULL_DEFAULT
*        References a queue instance object
*
*  FUNCTION
*     Lists all queue instance objects in the same 
*     queue_consumable_category
*
****************************************************************************
*/
enum {
   QRL_queue_pos = 0
};

enum {
   QRL_queue = QRL_LOWERBOUND
};

LISTDEF(QRL_Type)
   SGE_REF(QRL_queue,CULL_ANY_SUBTYPE, CULL_DEFAULT)
LISTEND 

NAMEDEF(QRL_N)
   NAME("QRL_queue")
NAMEEND

#define QRL_S sizeof(QRL_N)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif
