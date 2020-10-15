#ifndef __SGE_LOADSENSORL_H
#define __SGE_LOADSENSORL_H

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

/****** sgeobj/loadsensor/--LS_Type *******************************************
*  NAME
*     LS_Type - CULL load sensor element
*     
*  ELEMENTS
*     SGE_STRING(LS_name)        
*        name of this load sensor, no hashing, 
*        we only have few loadsensors/host 
*
*     SGE_STRING(LS_command)     
*        absolute path of the ls script 
*
*     SGE_STRING(LS_pid)         
*        pid of the ls process 
*
*     SGE_REF(LS_in)             
*        stdin to the ls process (type: FILE*) 
*
*     SGE_REF(LS_out)            
*        stdout of the ls process (type: FILE*) 
*
*     SGE_REF(LS_err)            
*        stderr of the ls process (type: FILE*) 
*
*     SGE_BOOL(LS_has_to_restart)        
*        should we restart the ls script?
*
*     SGE_ULONG(LS_tag)          
*        tag for internal use
*
*     SGE_LIST(LS_incomplete)    
*        current values we got from the ls script 
*
*     SGE_LIST(LS_complete)      
*        last complete set of ls values 
*
*     SGE_ULONG(LS_last_mod)     
*        last modification time of ls script 
*
*  FUNCTION
*     The attributes of this element show the state of a load sensor.
*     A list of these elements is used in the execd.
******************************************************************************/
enum {
   LS_name = LS_LOWERBOUND,
   LS_command,
   LS_pid,
   LS_in,
   LS_out,
   LS_err,
   LS_has_to_restart,
   LS_tag,
   LS_incomplete,
   LS_complete,
   LS_last_mod
};

LISTDEF(LS_Type)
   SGE_STRING(LS_name, CULL_DEFAULT)       
   SGE_STRING(LS_command, CULL_DEFAULT)   
   SGE_STRING(LS_pid, CULL_DEFAULT)      
   SGE_REF(LS_in, SGE_ANY_SUBTYPE, CULL_DEFAULT)         /* type is FILE * */
   SGE_REF(LS_out, SGE_ANY_SUBTYPE, CULL_DEFAULT)        /* type is FILE * */
   SGE_REF(LS_err, SGE_ANY_SUBTYPE, CULL_DEFAULT)        /* type is FILE * */
   SGE_BOOL(LS_has_to_restart, CULL_DEFAULT) 
   SGE_ULONG(LS_tag, CULL_DEFAULT)         
   SGE_LIST(LS_incomplete, LR_Type, CULL_DEFAULT)  
   SGE_LIST(LS_complete, LR_Type, CULL_DEFAULT)   
   SGE_ULONG(LS_last_mod, CULL_DEFAULT) 
LISTEND 

NAMEDEF(LSN)
   NAME("LS_name")
   NAME("LS_command")
   NAME("LS_pid")
   NAME("LS_in")
   NAME("LS_out")
   NAME("LS_err")
   NAME("LS_has_to_restart")
   NAME("LS_tag")
   NAME("LS_incomplete")
   NAME("LS_complete")
   NAME("LS_last_mod")
NAMEEND

/* *INDENT-ON* */ 

#define LSS sizeof(LSN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif /* __SGE_LOADSENSORL_H */
