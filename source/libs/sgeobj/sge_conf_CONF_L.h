#ifndef __SGE_CONF_CONF_L_H
#define __SGE_CONF_CONF_L_H

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
   CONF_name = CONF_LOWERBOUND,
   CONF_version,
   CONF_entries                 /* Points to CF_Type list */
};

LISTDEF(CONF_Type)
   JGDI_ROOT_OBJ(Configuration, SGE_CONF_LIST, ADD | MODIFY | DELETE | GET_LIST | GET)
   JGDI_EVENT_OBJ(ADD(sgeE_CONFIG_ADD) | MODIFY(sgeE_CONFIG_MOD) | DELETE(sgeE_CONFIG_DEL) | GET_LIST(sgeE_CONFIG_LIST))
   SGE_HOST_D(CONF_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_JGDI_CONF, "global")
   SGE_ULONG(CONF_version, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)
   SGE_LIST(CONF_entries, CF_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
LISTEND 

NAMEDEF(CONFN)
   NAME("CONF_name")
   NAME("CONF_version")
   NAME("CONF_entries")
NAMEEND

#define CONFS sizeof(CONFN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_CONFL_H */
