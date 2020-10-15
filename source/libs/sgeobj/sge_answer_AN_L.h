#ifndef __SGE_ANSWERL_H
#define __SGE_ANSWERL_H

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

#include "sgeobj/sge_boundaries.h"
#include "cull/cull.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */

/****** sgeobj/answer/--AN_Type ***********************************************
*  NAME
*     AN_Type - CULL answer element
*
*  ELEMENTS
*     SGE_ULONG(AN_status)
*        status of an answer (e.g STATUS_NOCOMMD) 
*
*     SGE_STRING(AN_text)
*        printable error text
*
*     SGE_ULONG(AN_quality)
*        answer quality (e.g ANSWER_QUALITY_ERROR)
*
*  FUNCTION
*     CULL element holding information for an answer of a request. 
******************************************************************************/
enum {
   AN_status = AN_LOWERBOUND,
   AN_text,
   AN_quality
};

LISTDEF(AN_Type)
   JGDI_OBJ(JGDIAnswer)
   SGE_ULONG(AN_status, CULL_DEFAULT)
   SGE_STRING(AN_text, CULL_DEFAULT)
   SGE_ULONG(AN_quality, CULL_DEFAULT)
LISTEND 

NAMEDEF(ANN)
   NAME("AN_status")
   NAME("AN_text")
   NAME("AN_quality")
NAMEEND

/* *INDENT-ON* */

#define ANS sizeof(ANN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif

#endif  /* __SGE_ANSWERL_H */

