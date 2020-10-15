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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_mtutil.h"

#include "spool/berkeleydb/msg_spoollib_berkeleydb.h"
#include "spool/berkeleydb/sge_bdb_types.h"

#include "basis_types.h"

struct _bdb_info {
   pthread_mutex_t   mtx;                 /* lock access to this object */
   pthread_key_t     key;                 /* for thread specific data */
  
   const char *      server;              /* server, in case of RPC mechanism */
   const char *      path;                /* the database path */

   DB_ENV *          env;                 /* global database environment */
   DB **             db;                  /* global database object */

   time_t            next_clear;          /* time of next logfile clear */
   time_t            next_checkpoint;     /* time of next checkpoint */
   bool              recover;             /* shall we recover on open? */
   bool              private;             /* open with DB_PRIVATE? */
};

typedef struct bdb_connection {
   DB_ENV *    env;                 /* thread specific database environment */
   DB **        db;                 /* thread specific database object */
   DB_TXN *    txn;                 /* transaction handle, always per thread */
} bdb_connection;

static void
bdb_init_connection(bdb_connection *con);

static void 
bdb_destroy_connection(void *connection);

/****** spool/berkeleydb/bdb_create() *****************************************
*  NAME
*     bdb_create() -- create Berkeley DB specific data structures
*
*  SYNOPSIS
*     bdb_info 
*     bdb_create(const char *server, const char *path) 
*
*  FUNCTION
*     Creates and initializes an object describing the connection to a 
*     Berkeley DB database and holding database and transaction handles.
*
*     Transaction handles are thread specific, in case of spooling using the
*     RPC client/server mechanism, the database environment and the database
*     handle are also thread specific.
*     
*     The parameter server is optional. If it is given, the Berkeley DB RPC
*     mechanism will be used. It then defines the name of the host, where the
*     Berkeley DB RPC server (berkeley_db_svc_) is running.
*
*     Path is either
*     - the absolute path to a Database in case of local spooling
*     - the database name (last component of path) in case of RPC mechanism
*
*  INPUTS
*     const char *server - hostname of Berkeley DB rpc server, or NULL
*     const char *path   - path to the database
*     const char *options - options list for opening database; currently
*                          may only be "private" to specify using DB_PRIVATE
*
*  RESULT
*     bdb_info - pointer to a newly created and initialized structure
*
*  NOTES
*     MT-NOTE: bdb_create() is MT safe 
*******************************************************************************/
bdb_info
bdb_create(const char *server, const char *path, const char *options)
{
   int ret, i;
   bdb_info info = (bdb_info) malloc(sizeof(struct _bdb_info));

   pthread_mutex_init(&(info->mtx), NULL);
   ret = pthread_key_create(&(info->key), bdb_destroy_connection);
   if (ret != 0) {
      fprintf(stderr, "can't initialize key for thread local storage: %s\n", strerror(ret));
   }
   info->server = server;
   info->path   = path;
   info->env    = NULL;

   info->db     = (DB **)malloc(BDB_ALL_DBS * sizeof(DB *));
   for (i = 0; i < BDB_ALL_DBS; i++) {
      info->db[i] = NULL;
   }

   info->next_clear = 0;
   info->next_checkpoint = 0;
   info->recover = false;
   if (options != NULL && strcmp(options, "private") == 0)
      info->private = true;
   else
      /* fixme: check validity -- how to report eror here?  */
      info->private = false;

   return info;
}

/*
* initialize thread local storage for a connection.
*  NOTES
*     MT-NOTE: bdb_init_connection() is MT safe 
*/
static void
bdb_init_connection(bdb_connection *con)
{
   int i;

   con->env = NULL;

   con->db     = (DB **)malloc(BDB_ALL_DBS * sizeof(DB *));
   for (i = 0; i < BDB_ALL_DBS; i++) {
      con->db[i] = NULL;
   }

   con->txn = NULL;
}

/*
* destroy the thread local storage for a connection
*  NOTES
*     MT-NOTE: bdb_destroy_connection() is MT safe 
*/
static void
bdb_destroy_connection(void *connection)
{
   /* Nothing to do here in principle.
    * Transactions and database connections shall be closed by calling the 
    * shutdown function.
    * But we can generate an error, if there is still something open.
    */
   bdb_connection *con = (bdb_connection *)connection;

   DENTER(TOP_LAYER, "bdb_destroy_connection");

   if (con->txn != NULL) {
      /* error */
   }

   if (con->db != NULL) {
      sge_free(&(con->db));
      /* error */
   }

   if (con->env != NULL) {
      /* error */
   }
   
   DEXIT;
}

/****** spool/berkeleydb/bdb_get_server() **************************************
*  NAME
*     bdb_get_server() -- get server name for a Berkeley DB
*
*  SYNOPSIS
*     const char * 
*     bdb_get_server(bdb_info info) 
*
*  FUNCTION
*     Returns the hostname of a Berkeley DB RPC server, if the RPC mechanism
*     is used, else NULL.
*
*  INPUTS
*     bdb_info info - the database object
*
*  RESULT
*     const char * - hostname or NULL
*
*  NOTES
*     MT-NOTE: bdb_get_server() is MT safe 
*******************************************************************************/
const char *
bdb_get_server(bdb_info info)
{
#ifdef DB_RPCCLIENT
   return info->server;
#else
   return NULL;
#endif
}

/****** spool/berkeleydb/bdb_get_path() ****************************************
*  NAME
*     bdb_get_path() -- get the database path
*
*  SYNOPSIS
*     const char * 
*     bdb_get_path(bdb_info info) 
*
*  FUNCTION
*     Returns the path to a Berkeley DB database.
*     If the RPC mechanism is used, this is the last component of the path.
*
*  INPUTS
*     bdb_info info - the database object
*
*  RESULT
*     const char * - path to the database.
*
*  NOTES
*     MT-NOTE: bdb_get_path() is MT safe 
*******************************************************************************/
const char *
bdb_get_path(bdb_info info)
{
   return info->path;
}

/****** spool/berkeleydb/bdb_get_env() *****************************************
*  NAME
*     bdb_get_env() -- get Berkeley DB database environment
*
*  SYNOPSIS
*     DB_ENV * bdb_get_env(bdb_info info) 
*
*  FUNCTION
*     Returns the Berkeley DB database environment set earlier using 
*     bdb_set_env().
*     If the RPC mechanism is used, the environment is stored per thread.
*
*  INPUTS
*     bdb_info info - the database object
*
*  RESULT
*     DB_ENV * - the database environment
*
*  NOTES
*     MT-NOTE: bdb_get_env() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/bdb_set_env()
*******************************************************************************/
DB_ENV *
bdb_get_env(bdb_info info)
{
   DB_ENV *env = NULL;

   if (info->server == NULL) {
      env = info->env;
   } else {
      GET_SPECIFIC(bdb_connection, con, bdb_init_connection, info->key, 
                   "bdb_get_env");
      env = con->env;
   }

   return env;
}

/****** spool/berkeleydb/bdb_get_db() ******************************************
*  NAME
*     bdb_get_db() -- get Berkeley DB database handle
*
*  SYNOPSIS
*     DB * 
*     bdb_get_db(bdb_info info) 
*
*  FUNCTION
*     Return the Berkeleyd BD database handle set earlier using bdb_set_db().
*     If the RPC mechanism is used, the database handle is stored per thread.
*
*  INPUTS
*     bdb_info info - the database object
*
*  RESULT
*     DB * - the database handle
*
*  NOTES
*     MT-NOTE: bdb_get_db() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/bdb_set_db()
*******************************************************************************/
DB *
bdb_get_db(bdb_info info, const bdb_database database)
{
   DB *db = NULL;

   if (info->server == NULL) {
      db = info->db[database];
   } else {
      GET_SPECIFIC(bdb_connection, con, bdb_init_connection, info->key, 
                   "bdb_get_db");
      db = con->db[database];
   }

   return db;
}

/****** spool/berkeleydb/bdb_get_txn() *****************************************
*  NAME
*     bdb_get_txn() -- get a transaction handle
*
*  SYNOPSIS
*     DB_TXN * 
*     bdb_get_txn(bdb_info info) 
*
*  FUNCTION
*     Returns a transaction handle set earlier with bdb_set_txn().
*     Each thread can have one transaction open.
*
*  INPUTS
*     bdb_info info - the database object
*
*  RESULT
*     DB_TXN * - a transaction handle
*
*  NOTES
*     MT-NOTE: bdb_get_txn() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/bdb_set_txn()
*******************************************************************************/
DB_TXN *
bdb_get_txn(bdb_info info)
{
   GET_SPECIFIC(bdb_connection, con, bdb_init_connection, info->key, 
                "bdb_get_txn");
   return con->txn;
}

time_t
bdb_get_next_clear(bdb_info info)
{
   return info->next_clear;
}

time_t
bdb_get_next_checkpoint(bdb_info info)
{
   return info->next_checkpoint;
}

bool
bdb_get_recover(bdb_info info) 
{
   return info->recover;
}

bool
bdb_get_private(bdb_info info)
{
   return info->private;
}

/****** spool/berkeleydb/bdb_set_env() *****************************************
*  NAME
*     bdb_set_env() -- set the Berkeley DB environment
*
*  SYNOPSIS
*     void 
*     bdb_set_env(bdb_info info, DB_ENV *env) 
*
*  FUNCTION
*     Sets the Berkeley DB environment.
*     If the RPC mechanism is used, an environment has to be created and opened
*     per thread.
*
*  INPUTS
*     bdb_info info - the database object
*     DB_ENV *env           - the environment handle to set
*
*  NOTES
*     MT-NOTE: bdb_set_env() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/bdb_get_env()
*******************************************************************************/
void
bdb_set_env(bdb_info info, DB_ENV *env)
{
   if (info->server == NULL) {
      info->env  = env;
   } else {
      GET_SPECIFIC(bdb_connection, con, bdb_init_connection, info->key, 
                   "bdb_set_env");
      con->env = env;
   }
}

/****** spool/berkeleydb/bdb_set_db() ******************************************
*  NAME
*     bdb_set_db() -- set a Berkeley DB database handle
*
*  SYNOPSIS
*     void 
*     bdb_set_db(bdb_info info, DB *db) 
*
*  FUNCTION
*     Sets the Berkeley DB database handle.
*     If the RPC mechanism is used, a database handle has to be created and 
*     opened per thread.
*
*  INPUTS
*     bdb_info info - the database object
*     DB *db                - the database handle to store
*
*  NOTES
*     MT-NOTE: bdb_set_db() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/bdb_get_db()
*******************************************************************************/
void
bdb_set_db(bdb_info info, DB *db, const bdb_database database)
{
   if (info->server == NULL) {
      info->db[database]  = db;
   } else {
      GET_SPECIFIC(bdb_connection, con, bdb_init_connection, info->key, 
                   "bdb_set_db");
      con->db[database] = db;
   }
}

/****** spool/berkeleydb/bdb_set_txn() *****************************************
*  NAME
*     bdb_set_txn() -- store a transaction handle
*
*  SYNOPSIS
*     void 
*     bdb_set_txn(bdb_info info, DB_TXN *txn) 
*
*  FUNCTION
*     Stores a Berkeley DB transaction handle.
*     It is always stored in thread local storage.
*
*  INPUTS
*     bdb_info info - the database object
*     DB_TXN *txn           - the transaction handle to store
*
*  NOTES
*     MT-NOTE: bdb_set_txn() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/bdb_get_txn()
*******************************************************************************/
void
bdb_set_txn(bdb_info info, DB_TXN *txn)
{
   GET_SPECIFIC(bdb_connection, con, bdb_init_connection, info->key, 
                "bdb_set_txn");
   con->txn = txn;
}

void
bdb_set_next_clear(bdb_info info, const time_t next)
{
   info->next_clear = next;
}

void
bdb_set_next_checkpoint(bdb_info info, const time_t next)
{
   info->next_checkpoint = next;
}

void
bdb_set_recover(bdb_info info, bool recover)
{
   info->recover = recover;
}

/****** spool/berkeleydb/bdb_get_dbname() **************************************
*  NAME
*     bdb_get_dbname() -- get a meaningfull database name
*
*  SYNOPSIS
*     const char * 
*     bdb_get_dbname(bdb_info info, dstring *buffer) 
*
*  FUNCTION
*     Return a meaningfull name for a database connection.
*     It contains the server name (in case of RPC mechanism) and the database
*     path.
*     A dstring buffer has to be provided by the caller.
*
*  INPUTS
*     bdb_info info - the database object
*     dstring *buffer       - buffer to hold the database name
*
*  RESULT
*     const char * - the database name
*
*  NOTES
*     MT-NOTE: bdb_get_dbname() is MT safe 
*******************************************************************************/
const char *
bdb_get_dbname(bdb_info info, dstring *buffer)
{
   const char *ret;
   const char *server = bdb_get_server(info);
   const char *path   = bdb_get_path(info);

   if (path == NULL) {
      ret = sge_dstring_copy_string(buffer, MSG_BERKELEY_DBNOTINITIALIZED);
   } else if (server == NULL) {
      ret = sge_dstring_copy_string(buffer, path);
   } else {
      ret = sge_dstring_sprintf(buffer, "%s:%s", server, path);
   }

   return ret;
}

void
bdb_lock_info(bdb_info info)
{
   sge_mutex_lock("bdb mutex", "bdb_lock_info", __LINE__, &(info->mtx));
}

void
bdb_unlock_info(bdb_info info)
{
   sge_mutex_unlock("bdb mutex", "bdb_unlock_info", __LINE__, &(info->mtx));
}

const char *
bdb_get_database_name(const bdb_database database)
{
   const char *ret;

   switch (database) {
      case BDB_CONFIG_DB:
         ret = "sge";
         break;
      case BDB_JOB_DB:
         ret = "sge_job";
         break;
      default:
         ret = NULL;
         break;
   };

   return ret;
}

