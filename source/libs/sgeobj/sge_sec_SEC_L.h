#ifndef __SGE_SEC_SEC_L_H
#define __SGE_SEC_SEC_L_H

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
   SEC_ConnectionID = SEC_LOWERBOUND,
   SEC_Host,
   SEC_Commproc,
   SEC_Id,
   SEC_UniqueIdentifier,
   SEC_ExpiryDate,
   SEC_SeqNoSend,
   SEC_SeqNoReceive,
   SEC_KeyPart0,
   SEC_KeyPart1,
   SEC_KeyPart2,
   SEC_KeyPart3,
   SEC_KeyPart4,
   SEC_KeyPart5,
   SEC_KeyPart6,
   SEC_KeyPart7
};

LISTDEF(SEC_Type)
   SGE_ULONG(SEC_ConnectionID, CULL_DEFAULT)
   SGE_HOST(SEC_Host, CULL_DEFAULT)
   SGE_STRING(SEC_Commproc, CULL_DEFAULT)
   SGE_INT(SEC_Id, CULL_DEFAULT)
   SGE_STRING(SEC_UniqueIdentifier, CULL_DEFAULT)
   SGE_STRING(SEC_ExpiryDate, CULL_DEFAULT)
   SGE_ULONG(SEC_SeqNoSend, CULL_DEFAULT)
   SGE_ULONG(SEC_SeqNoReceive, CULL_DEFAULT)
   SGE_ULONG(SEC_KeyPart0, CULL_DEFAULT)
   SGE_ULONG(SEC_KeyPart1, CULL_DEFAULT)
   SGE_ULONG(SEC_KeyPart2, CULL_DEFAULT)
   SGE_ULONG(SEC_KeyPart3, CULL_DEFAULT)
   SGE_ULONG(SEC_KeyPart4, CULL_DEFAULT)
   SGE_ULONG(SEC_KeyPart5, CULL_DEFAULT)
   SGE_ULONG(SEC_KeyPart6, CULL_DEFAULT)
   SGE_ULONG(SEC_KeyPart7, CULL_DEFAULT)
LISTEND 

NAMEDEF(SECN)
   NAME("SEC_ConnectionID")
   NAME("SEC_Host")
   NAME("SEC_Commproc")
   NAME("SEC_Id")
   NAME("SEC_UniqueIdentifier")
   NAME("SEC_ExpiryDate")
   NAME("SEC_SeqNoSend")
   NAME("SEC_SeqNoReceive")
   NAME("SEC_KeyPart0")
   NAME("SEC_KeyPart1")
   NAME("SEC_KeyPart2")
   NAME("SEC_KeyPart3")
   NAME("SEC_KeyPart4")
   NAME("SEC_KeyPart5")
   NAME("SEC_KeyPart6")
   NAME("SEC_KeyPart7")
NAMEEND

/* *INDENT-ON* */   

#define SECS sizeof(SECN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* SECL_H */
