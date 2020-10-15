#ifndef __SGE_SPOOLING_H 
#define __SGE_SPOOLING_H 
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

#include "mir/sge_mirror.h"

#include "spool/sge_spooling.h"

#include "sgeobj/sge_spooling_SPC_L.h"
#include "sgeobj/sge_spooling_SPR_L.h"
#include "sgeobj/sge_spooling_SPT_L.h"
#include "sgeobj/sge_spooling_SPTR_L.h"
#include "sgeobj/sge_spooling_SPM_L.h"

/****** spool/--Spooling ***************************************
*
*  NAME
*     Spooling -- Spooling framework
*
*  FUNCTION
*     The spooling framework provides an abstraction layer between the 
*     application requiring the spooling of configuration and data, 
*     and the concrete data representation layer.
*
*  SEE ALSO
*     spool/spool_create_context()
*     spool/spool_free_context()
*
*     spool/spool_set_default_context()
*     spool/spool_get_default_context()
*
*     spool/spool_context_search_rule()
*     spool/spool_context_create_rule()
*     spool/spool_context_search_type()
*     spool/spool_context_create_type()
*
*     spool/spool_type_search_default_rule()
*     spool/spool_type_add_rule()
*
*     spool/spool_startup_context()
*     spool/spool_shutdown_context()
*
*     spool/spool_read_list()
*     spool/spool_read_object()
*
*     spool/spool_write_object()
*
*     spool/spool_delete_object()
*
*     spool/spool_compare_objects()
*
****************************************************************************
*/
/****** spool/-Spooling-Typedefs ***************************************
*
*  NAME
*     Typedefs -- type definitions for the spooling framework
*
*  SYNOPSIS
*     typedef bool (*spooling_startup_func)(lList **answer_list,
*                                           const lListElem *rule, 
*                                           bool check); 
*     typedef bool (*spooling_shutdown_func)(lList **answer_list, 
*                                            const lListElem *rule); 
*
*     typedef bool (*spooling_list_func)(lList **answer_list,
*                                        const lListElem *type, 
*                                        const lListElem *rule, 
*                                        lList **list, 
*                                        const sge_object_type object_type);
*
*     typedef bool (*spooling_write_func)(lList **answer_list,
*                                         const lListElem *type, 
*                                         const lListElem *rule, 
*                                         const lListElem *object, 
*                                         const char *key, 
*                                         const sge_object_type object_type);
*
*     typedef lListElem *(*spooling_read_func)(lList **answer_list,
*                                              const lListElem *type, 
*                                              const lListElem *rule, 
*                                              const char *key, 
*                                              const sge_object_type object_type);
*
*     typedef bool (*spooling_delete_func)(lList **answer_list,
*                                          const lListElem *type, 
*                                          const lListElem *rule, 
*                                          const char *key, 
*                                          const sge_object_type object_type);
*
*     typedef bool (*spooling_validate_func)(lList **answer_list,
*                                          const lListElem *type,
*                                          const lListElem *rule, 
*                                          lListElem *object, 
*                                          const sge_object_type object_type);
*
*  FUNCTION
*     These functions have to be provided by a target implementation for the 
*     spooling framework.
*
*     The startup and shutdown function initialize the spooling system and
*     shut it down (e.g. establish a database connection and close it again.
*
*     The list, read, write and delete functions are performing the data
*     storage and retrieval into / from the used storage system.
*
*     Instances of these function types will be used as callback in calls
*     to spool_startup_context, spool_shutdown_context, spool_read_list etc.
*
*  SEE ALSO
*     spool/spool_startup_context()
*     spool/spool_shutdown_context()
*
*     spool/spool_read_list()
*     spool/spool_read_object()
*
*     spool/spool_write_object()
*
*     spool/spool_delete_object()
*
****************************************************************************
*/

typedef enum {
   SPM_init,
   SPM_history,
   SPM_backup,
   SPM_purge,
   SPM_vacuum,
   SPM_info
} spooling_maintenance_command;

typedef enum {
   STC_begin,
   STC_commit,
   STC_rollback
} spooling_transaction_command;

typedef const char *
(*spooling_get_method_func)(void);

typedef lListElem *
(*spooling_create_context_func)(lList **answer_list, const char *args);

typedef bool
(*spooling_option_func)(lList **answer_list, lListElem *rule, 
                        const char *option); 
typedef bool
(*spooling_startup_func)(lList **answer_list, const lListElem *rule, 
                         bool check); 
typedef bool
(*spooling_shutdown_func)(lList **answer_list, const lListElem *rule); 

typedef bool 
(*spooling_maintenance_func)(lList **answer_list, const lListElem *rule, 
                             const spooling_maintenance_command cmd, 
                             const char *args);

typedef bool
(*spooling_trigger_func)(lList **answer_list, const lListElem *rule, 
                         time_t trigger, time_t *next_trigger);
                                  
typedef bool
(*spooling_transaction_func)(lList **answer_list, const lListElem *rule, 
                             spooling_transaction_command cmd);
                                  
typedef bool
(*spooling_list_func)(lList **answer_list, 
                      const lListElem *type, const lListElem *rule, 
                      lList **list, 
                      const sge_object_type object_type);
                                  
typedef bool
(*spooling_write_func)(lList **answer_list, 
                       const lListElem *type, const lListElem *rule, 
                       const lListElem *object, const char *key, 
                       const sge_object_type object_type);

typedef lListElem *
(*spooling_read_func)(lList **answer_list, 
                      const lListElem *type, const lListElem *rule, 
                      const char *key, 
                      const sge_object_type object_type);

typedef bool
(*spooling_delete_func)(lList **answer_list, 
                        const lListElem *type, const lListElem *rule, 
                        const char *key, 
                        const sge_object_type object_type);

typedef bool
(*spooling_validate_func)(lList **answer_list, 
                        const lListElem *type, const lListElem *rule, 
                        lListElem *object, 
                        const sge_object_type object_type);

typedef bool
(*spooling_validate_list_func)(lList **answer_list, 
                        const lListElem *type, const lListElem *rule, 
                        const sge_object_type object_type);

/* creation and maintenance of the spooling context */
lListElem *
spool_create_context(lList **answer_list, const char *name);

lListElem *
spool_free_context(lList **answer_list, lListElem *context);

void 
spool_set_default_context(lListElem *context);

lListElem *
spool_get_default_context(void);

lListElem *
spool_context_search_rule(const lListElem *context, const char *name);

lListElem *
spool_context_create_rule(lList **answer_list, lListElem *context, 
                          const char *name, const char *url,
                          spooling_option_func option_func, 
                          spooling_startup_func startup_func, 
                          spooling_shutdown_func shutdown_func, 
                          spooling_maintenance_func maintenance_func,
                          spooling_trigger_func trigger_func,
                          spooling_transaction_func transaction_func,
                          spooling_list_func list_func, 
                          spooling_read_func read_func, 
                          spooling_write_func write_func, 
                          spooling_delete_func delete_func,
                          spooling_validate_func validate_func,
                          spooling_validate_list_func validate_list_func);

lListElem *
spool_context_search_type(const lListElem *context, 
                          const sge_object_type object_type);

lListElem *
spool_context_create_type(lList **answer_list, lListElem *context, 
                          const sge_object_type object_type);

lListElem *
spool_type_search_default_rule(const lListElem *spool_type);

lListElem 
*spool_type_add_rule(lList **answer_list, lListElem *spool_type, 
                     const lListElem *rule, lBool is_default);

bool
spool_set_option(lList **answer_list, lListElem *context, const char *option);

/* startup and shutdown */
bool 
spool_startup_context(lList **answer_list, lListElem *context, bool check);

bool 
spool_shutdown_context(lList **answer_list, lListElem *context);

bool
spool_maintain_context(lList **answer_list, lListElem *context, 
                       const spooling_maintenance_command cmd,
                       const char *args);

bool
spool_trigger_context(lList **answer_list, lListElem *context, 
                      time_t trigger, time_t *next_trigger);

bool spool_transaction(lList **answer_list, lListElem *context, 
                       spooling_transaction_command cmd);

/* reading */
bool 
spool_read_list(lList **answer_list, const lListElem *context, 
                lList **list, const sge_object_type object_type);

lListElem *
spool_read_object(lList **answer_list, const lListElem *context, 
                  const sge_object_type object_type, const char *key);

/* writing */
bool 
spool_write_object(lList **answer_list, const lListElem *context, 
                   const lListElem *object, const char *key, 
                   const sge_object_type object_type,
                   bool do_job_spooling);

/* deleting */
bool 
spool_delete_object(lList **answer_list, const lListElem *context, 
                    const sge_object_type object_type, const char *key,
                    bool do_job_spooling);

/* compare spooled attributes of 2 objects */
bool
spool_compare_objects(lList **answer_list, const lListElem *context, 
                      const sge_object_type object_type, 
                      const lListElem *ep1, const lListElem *ep2);

const char *get_spooling_method(void);
#endif /* __SGE_SPOOLING_H */    
