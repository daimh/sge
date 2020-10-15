#ifndef __SGE_QETIL_H
#define __SGE_QETIL_H

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

/****** sgeobj/QETI/--QETI_Type **************************************************
*  NAME
*     QETI_Type - Queue End Time Iterator building blocks
*
*  ELEMENTS
*
*     SGE_DOUBLE(QETI_total)
*        Keeps information about total amount of the resource instance.
*        It would be nice if that in information were also contained in
*        the RUE_Type element referenced via the resource utilization 
*        entry QETI_resource_instance. Unfortunately this is not the 
*        case so we keep it here separately.
*
*     SGE_REF(QETI_resource_instance)
*        Reference to a resource utilization entry (RUE_Type element) that keeps 
*        all information about resource utilization of a particular resource
*        instance.
*
*     SGE_REF(QETI_queue_end_next)
*        Reference to the next entry in the resource utilization diagram 
*        (RDE_Type elemnt). All next pointers related to all resource instances
*        represent the state of a sge_qeti_t iterator.
*
*  FUNCTION
*     Qeti != Yeti 
*
*  SEE ALSO
******************************************************************************/
enum {
   QETI_total = QETI_LOWERBOUND,
   QETI_resource_instance,
   QETI_queue_end_next
};

LISTDEF(QETI_Type)
   SGE_DOUBLE(QETI_total, CULL_DEFAULT)
   SGE_REF(QETI_resource_instance, RUE_Type, CULL_DEFAULT)
   SGE_REF(QETI_queue_end_next, RDE_Type, CULL_DEFAULT)
LISTEND 

NAMEDEF(QETIN)
   NAME("QETI_total")
   NAME("QETI_resource_instance")
   NAME("QETI_queue_end_next")
NAMEEND

#define QETIS sizeof(QETIN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_QETIL_H */

