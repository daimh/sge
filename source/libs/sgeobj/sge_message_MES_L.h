#ifndef __SGE_MESSAGE_MES_L_H
#define __SGE_MESSAGE_MES_L_H

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

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */ 

/*
 * Message element
 */
enum {
   MES_job_number_list = MES_LOWERBOUND,
   MES_message_number,
   MES_message
};

LISTDEF(MES_Type)
   JGDI_OBJ(JobSchedulingMessage)
   SGE_LIST(MES_job_number_list, ULNG_Type, CULL_DEFAULT)
   SGE_ULONG(MES_message_number, CULL_DEFAULT)
   SGE_STRING(MES_message, CULL_DEFAULT)
LISTEND 

NAMEDEF(MESN)
   NAME("MES_job_number_list")
   NAME("MES_message_number")
   NAME("MES_message")
NAMEEND

/* *INDENT-ON* */   

#define MESS sizeof(MESN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif
