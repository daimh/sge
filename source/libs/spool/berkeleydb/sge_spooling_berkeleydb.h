#ifndef __SGE_SPOOLING_BERKELEYDB_H 
#define __SGE_SPOOLING_BERKELEYDB_H 
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

#include <time.h>

#include "cull/cull.h"

#include "spool/sge_spooling.h"
#include "spool/sge_spooling_utilities.h"

/****** spool/berkeleydb/--Spooling-Berkeley-DB ********************************
*
*  NAME
*     berkeleydb spooling - spooling of data in BerkeleyDB database
*
*  FUNCTION
*     The module provides functions and a spooling framework instantiation
*     for data input/output into a BerkeleyDB database.
*
*  SEE ALSO
****************************************************************************
*/

#ifdef SPOOLING_berkeleydb
const char *get_spooling_method(void);
#else
const char *get_berkeleydb_spooling_method(void);
#endif

lListElem *
spool_berkeleydb_create_context(lList **answer_list, const char *args);

bool 
spool_berkeleydb_default_startup_func(lList **answer_list, 
                                    const lListElem *rule, bool check);

bool 
spool_berkeleydb_common_startup_func(lList **answer_list, 
                                   const lListElem *rule);

bool 
spool_berkeleydb_default_shutdown_func(lList **answer_list, 
                                     const lListElem *rule);
bool 
spool_berkeleydb_default_maintenance_func(lList **answer_list, 
                                        const lListElem *rule,
                                        const spooling_maintenance_command cmd,
                                        const char *args);

bool
spool_berkeleydb_trigger_func(lList **answer_list, const lListElem *rule,
                              time_t trigger, time_t *next_trigger);

bool
spool_berkeleydb_transaction_func(lList **answer_list, const lListElem *rule, 
                                  spooling_transaction_command cmd);
bool 
spool_berkeleydb_default_list_func(lList **answer_list, 
                                 const lListElem *type, 
                                 const lListElem *rule, lList **list, 
                                 const sge_object_type object_type);
lListElem *
spool_berkeleydb_default_read_func(lList **answer_list, 
                                 const lListElem *type, 
                                 const lListElem *rule, const char *key, 
                                 const sge_object_type object_type);
bool 
spool_berkeleydb_default_write_func(lList **answer_list, 
                                  const lListElem *type, 
                                  const lListElem *rule, 
                                  const lListElem *object, const char *key, 
                                  const sge_object_type object_type);
bool 
spool_berkeleydb_default_delete_func(lList **answer_list, 
                                   const lListElem *type, 
                                   const lListElem *rule, 
                                   const char *key, 
                                   const sge_object_type object_type);
#endif /* __SGE_SPOOLING_BERKELEYDB_H */    
