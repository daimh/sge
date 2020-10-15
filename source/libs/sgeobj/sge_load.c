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

#include <string.h>

#include "uti/sge_rmon.h" 
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_str.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_load.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "basis_types.h"
#include "msg_common.h"

bool 
sge_is_static_load_value(const char *name) 
{
   bool ret = false;

   DENTER(BASIS_LAYER, "sge_is_static_load_value");

   if (name != NULL) {
      if (strcmp(name, LOAD_ATTR_ARCH) == 0 || 
          strcmp(name, LOAD_ATTR_NUM_PROC) == 0 ||
          strcmp(name, LOAD_ATTR_MEM_TOTAL) == 0 ||
          strcmp(name, LOAD_ATTR_SWAP_TOTAL) == 0 ||
          strcmp(name, LOAD_ATTR_VIRTUAL_TOTAL) == 0 ||
          strcmp(name, LOAD_ATTR_TOPOLOGY) == 0 ||
          strcmp(name, LOAD_ATTR_SOCKETS) == 0 ||
          strcmp(name, LOAD_ATTR_CORES) == 0 ||
          strcmp(name, LOAD_ATTR_THREADS) == 0 ) {
        ret = true;
      }
   }

   DRETURN(ret);
}
