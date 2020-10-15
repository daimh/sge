#ifndef __SGE_MESOBJL_H
#define __SGE_MESOBJL_H

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

/*
 * Message element
 */
enum {
   QIM_type = QIM_LOWERBOUND,
   QIM_message
};

LISTDEF(QIM_Type)
   JGDI_OBJ(QueueInstanceMessage)
   SGE_ULONG(QIM_type, CULL_DEFAULT | CULL_SPOOL)
   SGE_STRING(QIM_message, CULL_DEFAULT | CULL_SPOOL)
LISTEND

NAMEDEF(QIMN)
   NAME("QIM_type")
   NAME("QIM_message")
NAMEEND

#define QIMS sizeof(QIMN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_MESOBJL_H */
