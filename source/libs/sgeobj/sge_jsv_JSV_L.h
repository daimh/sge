#ifndef __SGE_JSVL_H
#define __SGE_JSVL_H

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
 *   Copyright: 2008 by Sun Microsystems, Inc.
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

/****** sgeobj/jsv/--JSV_Type **********************************************
*  NAME
*     JSV_Type - CULL job submission verification element
*     
*  ELEMENTS
*     SGE_STRING(JSV_name)        
*        name of this element, no hashing because there are not much
*
*     SGE_STRING(JSV_command)     
*        absolute path of the script 
*
*     SGE_STRING(JSV_pid)         
*        pid of the process 
*
*     SGE_REF(JSV_in)             
*        stdin to the process (type: FILE*) 
*
*     SGE_REF(JSV_out)            
*        stdout of the process (type: FILE*) 
*
*     SGE_REF(JSV_err)            
*        stderr of the process (type: FILE*) 
*
*     SGE_BOOL(JSV_has_to_restart)        
*        should we restart the ls script?
*
*     SGE_LIST(JSV_incomplete)    
*        current values we got from the script 
*
*     SGE_LIST(JSV_complete)      
*        last complete set of send values 
*
*     SGE_ULONG(JSV_last_mod)     
*        last modification time of ls script 
*
*  FUNCTION
*     The attributes of this element show the state of a jsv script.
*     A list of these elements is used in submit clients and master process. 
******************************************************************************/
enum {
   JSV_name = JSV_LOWERBOUND,
   JSV_context,
   JSV_url,
   JSV_type,
   JSV_user,
   JSV_command,
   JSV_pid,
   JSV_in,
   JSV_out,
   JSV_err,
   JSV_has_to_restart,
   JSV_last_mod,
   JSV_send_env,
   JSV_old_job,
   JSV_new_job,
   JSV_restart,
   JSV_accept,
   JSV_done,
   JSV_soft_shutdown,
   JSV_test,
   JSV_test_pos,
   JSV_result
};

LISTDEF(JSV_Type)
   SGE_STRING(JSV_name, CULL_DEFAULT)                    /* name of a JSV */
   SGE_STRING(JSV_context, CULL_DEFAULT)                 /* value of the JSV_CONTEXT_CLIENT define
                                                          * or name of the worker thread in
                                                          * case of server JSV 
                                                          */
   SGE_STRING(JSV_url, CULL_DEFAULT)                     /* jsv_url as specified in sge_types man page */
   SGE_STRING(JSV_type, CULL_DEFAULT)                    /* in the moment only "script" is allowed here */
   SGE_STRING(JSV_user, CULL_DEFAULT)                    /* user name used in jsv_url */
   SGE_STRING(JSV_command, CULL_DEFAULT)                 /* absolute path of jsv_url */
   SGE_STRING(JSV_pid, CULL_DEFAULT)                     /* -1 or pid of running jsv instance */ 
   SGE_REF(JSV_in, SGE_ANY_SUBTYPE, CULL_DEFAULT)        /* type is FILE* */
   SGE_REF(JSV_out, SGE_ANY_SUBTYPE, CULL_DEFAULT)       /* type is FILE* */
   SGE_REF(JSV_err, SGE_ANY_SUBTYPE, CULL_DEFAULT)       /* type is FILE* */
   SGE_BOOL(JSV_has_to_restart, CULL_DEFAULT)            /* JSV has to be restarted as soon as possible */
   SGE_ULONG(JSV_last_mod, CULL_DEFAULT)                 /* timestamp when the jsv script file
                                                          * was last modified */ 
   SGE_BOOL(JSV_send_env, CULL_DEFAULT)                  /* environment information has to be sent
                                                          * to the JSV */
   SGE_REF(JSV_old_job, JB_Type, CULL_DEFAULT)           /* job template before verification step */
   SGE_REF(JSV_new_job, JB_Type, CULL_DEFAULT)           /* job after the verification step */
   SGE_BOOL(JSV_restart, CULL_DEFAULT) 
   SGE_BOOL(JSV_accept, CULL_DEFAULT) 
   SGE_BOOL(JSV_done, CULL_DEFAULT) 
   SGE_BOOL(JSV_soft_shutdown, CULL_DEFAULT)              
   SGE_BOOL(JSV_test, CULL_DEFAULT)
   SGE_ULONG(JSV_test_pos, CULL_DEFAULT)
   SGE_STRING(JSV_result, CULL_DEFAULT)
LISTEND 

NAMEDEF(JSVN)
   NAME("JSV_name")
   NAME("JSV_context")
   NAME("JSV_url")
   NAME("JSV_type")
   NAME("JSV_user")
   NAME("JSV_command")
   NAME("JSV_pid")
   NAME("JSV_in")
   NAME("JSV_out")
   NAME("JSV_err")
   NAME("JSV_has_to_restart")
   NAME("JSV_last_mod")
   NAME("JSV_send_env")
   NAME("JSV_old_job")
   NAME("JSV_new_job")
   NAME("JSV_restart")
   NAME("JSV_accept")
   NAME("JSV_done")
   NAME("JSV_soft_shutdown")
   NAME("JSV_test")
   NAME("JSV_test_pos")
   NAME("JSV_result")
NAMEEND

/* *INDENT-ON* */ 

#define JSVS sizeof(JSVN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif /* __SGE_JSVL_H */
