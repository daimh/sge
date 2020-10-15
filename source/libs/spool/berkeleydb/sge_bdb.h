#ifndef __SGE_BDB_H 
#define __SGE_BDB_H 
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
#include "spool/berkeleydb/sge_bdb_types.h"
#include "basis_types.h"

bool
spool_berkeleydb_check_version(lList **answer_list);

bool 
spool_berkeleydb_create_environment(lList **answer_list, 
                                    bdb_info info);

bool 
spool_berkeleydb_open_database(lList **answer_list, bdb_info info, 
                               bool create);

bool 
spool_berkeleydb_check_reopen_database(lList **answer_list, 
                                       bdb_info info);

bool 
spool_berkeleydb_close_database(lList **answer_list, bdb_info info);

bool
spool_berkeleydb_start_transaction(lList **answer_list, bdb_info info);

bool
spool_berkeleydb_end_transaction(lList **answer_list, bdb_info info, 
                                 bool commit);

bool 
spool_berkeleydb_trigger(lList **answer_list, bdb_info info, 
                         time_t trigger, time_t *next_trigger);

bool 
spool_berkeleydb_read_list(lList **answer_list, bdb_info info,
                           const bdb_database database,
                           lList **list, const lDescr *descr,
                           const char *key);
bool 
spool_berkeleydb_write_object(lList **answer_list, bdb_info info,
                              const bdb_database database,
                              const lListElem *object, const char *key);

bool spool_berkeleydb_write_string(lList **answer_list, bdb_info info,
                              const bdb_database database,
                              const char *key, const char *str);
                              
bool
spool_berkeleydb_write_pe_task(lList **answer_list, bdb_info info,
                               const lListElem *object, 
                               u_long32 job_id, u_long32 ja_task_id,
                               const char *pe_task_id);
bool
spool_berkeleydb_write_ja_task(lList **answer_list, bdb_info info,
                               const lListElem *object, 
                               u_long32 job_id, u_long32 ja_task_id);
bool
spool_berkeleydb_write_job(lList **answer_list, bdb_info info,
                           const lListElem *object, 
                           u_long32 job_id, u_long32 ja_task_id, bool only_job);
bool
spool_berkeleydb_write_cqueue(lList **answer_list, bdb_info info, 
                              const lListElem *object, const char *key);

bool
spool_berkeleydb_delete_object(lList **answer_list, bdb_info info, 
                               const bdb_database database,
                               const char *key, bool sub_objects);
bool
spool_berkeleydb_delete_pe_task(lList **answer_list, bdb_info info,
                                const char *key, bool sub_objects);
bool
spool_berkeleydb_delete_ja_task(lList **answer_list, bdb_info info,
                                const char *key, bool sub_objects);
bool
spool_berkeleydb_delete_job(lList **answer_list, bdb_info info,
                            const char *key, bool sub_objects);
bool
spool_berkeleydb_delete_cqueue(lList **answer_list, bdb_info info,
                               const char *key);

bool
spool_berkeleydb_read_keys(lList **answer_list, bdb_info info,
                           const bdb_database database,
                           lList **list, const char *key);

lListElem *
spool_berkeleydb_read_object(lList **answer_list, bdb_info info,
                             const bdb_database database,
                             const char *key);

char *
spool_berkeleydb_read_string(lList **answer_list, bdb_info info,
                             const bdb_database database,
                             const char *key);
                             
#endif /* __SGE_BDB_H */    
