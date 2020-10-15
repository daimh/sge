#ifndef _SGE_CKPTL_H_
#define _SGE_CKPTL_H_

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
 * This is the list type to hold the checkpointing
 * object for the interfaces to the various
 * supported checkpointing mechanisms.
 */

enum {
   CK_name = CK_LOWERBOUND,
   CK_interface,
   CK_ckpt_command,
   CK_migr_command,
   CK_rest_command,
   CK_ckpt_dir,
   CK_when,
   CK_signal,
   CK_job_pid,
   CK_clean_command
};

LISTDEF(CK_Type)
   JGDI_ROOT_OBJ(Checkpoint, SGE_CK_LIST, ADD | MODIFY | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_CKPT_ADD) | MODIFY(sgeE_CKPT_MOD) | DELETE(sgeE_CKPT_DEL) | GET_LIST(sgeE_CKPT_LIST))
   SGE_STRING_D(CK_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_JGDI_CONF, "template")
   SGE_STRING_D(CK_interface, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "userdefined")
   SGE_STRING_D(CK_ckpt_command, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "none")
   SGE_STRING_D(CK_migr_command, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "none")
   SGE_STRING_D(CK_rest_command, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "none")
   SGE_STRING_D(CK_ckpt_dir, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "/tmp")
   SGE_STRING_D(CK_when, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "sx")
   SGE_STRING_D(CK_signal, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "none")
   SGE_ULONG_D(CK_job_pid, CULL_DEFAULT | CULL_JGDI_HIDDEN, 0)
   SGE_STRING_D(CK_clean_command, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "none")
LISTEND 

NAMEDEF(CKN)
   NAME("CK_name")
   NAME("CK_interface")
   NAME("CK_ckpt_command")
   NAME("CK_migr_command")
   NAME("CK_rest_command")
   NAME("CK_ckpt_dir")
   NAME("CK_when")
   NAME("CK_signal")   
   NAME("CK_job_pid")
   NAME("CK_clean_command")
NAMEEND

/* *INDENT-ON* */  

#define CKS sizeof(CKN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* _SGE_CKPTL_H_ */
