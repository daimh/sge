#ifndef __SGE_SPOOLING_DYNAMIC_H 
#define __SGE_SPOOLING_DYNAMIC_H 
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

#include "spool/sge_spooling.h"
#include "spool/sge_spooling_utilities.h"

/****** spool/dynamic/--Spooling-Dynamic ********************************
*
*  NAME
*     dynamic spooling - spooling of data into method specified at runtime
*
*  FUNCTION
*     This module provides functions necessary to setup the spooling framework
*     in a way, that the spooling method to use is determined at runtime
*     by loading a certain shared library containing the spooling method.
*
*  SEE ALSO
*     spool/berkeleydb/--Spooling-Berkeley-DB
*     spool/classic/--Spooling-Classic
****************************************************************************
*/

#ifdef SPOOLING_dynamic
const char *get_spooling_method(void);
#else
const char *get_dynamic_spooling_method(void);
#endif

lListElem *
spool_dynamic_create_context(lList **answer_list, const char *method,
                             const char *shlib_name, const char *args);

#endif /* __SGE_SPOOLING_DYNAMIC_H */    

