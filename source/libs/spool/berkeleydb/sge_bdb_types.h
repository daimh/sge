#ifndef __SGE_BDB_TYPES_H 
#define __SGE_BDB_TYPES_H 
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
#include <pthread.h>

#include <db.h>

#include "uti/sge_dstring.h"

/* JG: TODO: the following defines should better be parameters to
 *           the berkeley db spooling
 */

/* how often will the transaction log be cleared */
#define BERKELEYDB_CLEAR_INTERVAL 300

/* how often will the database be checkpointed (cache written to disk) */
#define BERKELEYDB_CHECKPOINT_INTERVAL 60

/* to please IRIX compiler: this is the minimum of clear and checkpoint interval */
#define BERKELEYDB_MIN_INTERVAL BERKELEYDB_CHECKPOINT_INTERVAL

/* Berkeley DB data structures:
 * We have a bdb_info object per spooling rule.
 * It holds all data that is required to use the spooling rule.
 * In case of local spooling, the DB_ENV and the DB are accessable for multiple threads.
 * If we use the RPC server, we need per thread DB_ENV and DB.
 * The transaction handle is always thread specific.
 * Thread specific data is initialized in the spooling startup function.
 */

typedef enum {
   BDB_CONFIG_DB = 0,
   BDB_JOB_DB,
   
   BDB_ALL_DBS
} bdb_database;

typedef struct _bdb_info *bdb_info;

bdb_info
bdb_create(const char *server, const char *path, const char * options);

const char *
bdb_get_server(bdb_info info);

const char *
bdb_get_path(bdb_info info);

DB_ENV *
bdb_get_env(bdb_info info);

DB *
bdb_get_db(bdb_info info, const bdb_database database);

DB_TXN *
bdb_get_txn(bdb_info info);

time_t
bdb_get_next_clear(bdb_info info);

time_t
bdb_get_next_checkpoint(bdb_info info);

bool 
bdb_get_recover(bdb_info info);

bool
bdb_get_private(bdb_info info);

void
bdb_set_env(bdb_info info, DB_ENV *env);

void
bdb_set_db(bdb_info info, DB *db, const bdb_database database);

void
bdb_set_txn(bdb_info info, DB_TXN *txn);

void
bdb_set_next_clear(bdb_info info, const time_t next);

void
bdb_set_next_checkpoint(bdb_info info, const time_t next);

void 
bdb_set_recover(bdb_info info, bool recover);

const char *
bdb_get_dbname(bdb_info info, dstring *buffer);

void
bdb_lock_info(bdb_info info);

void
bdb_unlock_info(bdb_info info);

const char *
bdb_get_database_name(const bdb_database database);

#endif /* __SGE_BDB_TYPES_H */    
