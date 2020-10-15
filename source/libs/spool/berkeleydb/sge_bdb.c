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

/* Fixme: purge the RPC server stuff.  */

#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#define BDB_LAYER BASIS_LAYER

#include <errno.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_profiling.h"
#include "uti/sge_string.h"
#include "uti/sge_unistd.h"

#include "cull/cull.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_str.h"

#include "spool/berkeleydb/msg_spoollib_berkeleydb.h"
#include "spool/berkeleydb/sge_bdb.h"

#include "msg_common.h"

#if 1
static const int pack_part = CULL_SPOOL | CULL_SUBLIST | CULL_SPOOL_PROJECT | 
                             CULL_SPOOL_USER;
#else
static const int pack_part = 0;
#endif

static void 
spool_berkeleydb_error_close(bdb_info info);

static void
spool_berkeleydb_handle_bdb_error(lList **answer_list, bdb_info info,
                                  int bdb_errno);

static bool
spool_berkeleydb_clear_log(lList **answer_list, bdb_info info);

static bool
spool_berkeleydb_trigger_rpc(lList **answer_list, bdb_info info);

static bool
spool_berkeleydb_checkpoint(lList **answer_list, bdb_info info);

/****** spool/berkeleydb/spool_berkeleydb_check_version() **********************
*  NAME
*     spool_berkeleydb_check_version() -- check version of shared libs 
*
*  SYNOPSIS
*     bool 
*     spool_berkeleydb_check_version(lList **answer_list) 
*
*  FUNCTION
*     Checks if major and minor version number returned by the db_version()
*     library call of Berkeley DB matches the version numbers set at compile
*     time.
*
*     The major and minor number must be equal, the patch level may differ.
*
*  INPUTS
*     lList **answer_list - used to return info and error messages
*
*  RESULT
*     bool - true, on success, else false
*
*  NOTES
*     MT-NOTE: spool_berkeleydb_check_version() is MT safe 
*
*******************************************************************************/
bool
spool_berkeleydb_check_version(lList **answer_list)
{
   bool ret = true;
   const char *version;
   int major, minor;

   DENTER(BDB_LAYER, "spool_berkeleydb_check_version");
   
   version = db_version(&major, &minor, NULL);

   answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                           ANSWER_QUALITY_INFO, 
                           MSG_BERKELEY_USINGBDBVERSION_S,
                           version);

   if (major != DB_VERSION_MAJOR || minor != DB_VERSION_MINOR) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_WRONGBDBVERSIONEXPECTING_SDD,
                              version, DB_VERSION_MAJOR, DB_VERSION_MINOR);
      ret = false;
   }
  
   DEXIT;
   return ret;
}

/****** spool/berkeleydb/spool_berkeleydb_create_environment() *****************
*  NAME
*     spool_berkeleydb_create_environment() -- ??? 
*
*  SYNOPSIS
*     bool spool_berkeleydb_create_environment(lList **answer_list, bdb_info info)
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list   - ??? 
*     bdb_info info - ??? 
*
*  RESULT
*     bool - 
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     MT-NOTE: spool_berkeleydb_create_environment() is not MT safe 
*
*  BUGS
*     ??? 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
bool spool_berkeleydb_create_environment(lList **answer_list, 
                                         bdb_info info)
{ 
   bool ret = true;
   int dbret;
   const char *server, *path;

   DB_ENV *env = NULL;

   DENTER(BDB_LAYER, "spool_berkeleydb_create_environment");

   server = bdb_get_server(info);
   path   = bdb_get_path(info);

   /* check database directory (only in case of local spooling) */
   if (server == NULL && !sge_is_directory(path)) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_DATABASEDIRDOESNTEXIST_S,
                              path);
      ret = false;
   }

   if (ret) {
      /* we have to lock the info structure, as multiple threads might try
       * to open the env in parallel.
       */
      bdb_lock_info(info);

      /* check, if env has been initialized in the meantime */
      env = bdb_get_env(info);
   }

   /* continue only, if env isn't initialized yet */
   if (ret && env == NULL) {
      int flags = 0;

#ifdef DB_RPCCLIENT
      if (server != NULL) {
         flags |= DB_RPCCLIENT;
      }
#endif

      PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      dbret = db_env_create(&env, flags);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      if (dbret != 0) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_COULDNTCREATEENVIRONMENT_IS,
                                 dbret, db_strerror(dbret));
         ret = false;
      }

      /* do deadlock detection internally (only in case of local spooling) */
      if (ret && server == NULL) {
         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = env->set_lk_detect(env, DB_LOCK_DEFAULT);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         if (dbret != 0) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_COULDNTESETUPLOCKDETECTION_IS,
                                    dbret, db_strerror(dbret));
            ret = false;
         } 

         /* 
          * performance tuning 
          * Switch off flushing of transaction log for every single transaction.
          * This tuning option has huge impact on performance, but only a slight impact 
          * on database durability: In case of a server/filesystem crash, we might loose
          * the last transactions committed before the crash. Still all transactions will
          * be atomic, isolated and the database will be consistent at any time.
          */
         if (ret) {
            dbret = env->set_flags(env, DB_TXN_WRITE_NOSYNC, 1);
            if (dbret != 0) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_CANTSETENVFLAGS_IS,
                                       dbret, db_strerror(dbret));
               ret = false;
            } 
         }

         /* 
          * performance tuning 
          * increase the cache size
          */
         if (ret) {
            dbret = env->set_cachesize(env, 0, 4 * 1024 * 1024, 1);
            if (dbret != 0) {
               spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR,
                                       MSG_BERKELEY_CANTSETENVCACHE_IS,
                                       dbret, db_strerror(dbret));
               ret = false;
            }
         }
      }

#ifdef DB_RPCCLIENT
      /* if we use a RPC server, set it in the DB_ENV */
      if (ret && server != NULL) {
         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = env->set_rpc_server(env, NULL, server, 0, 0, 0);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         if (dbret != 0) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_COULDNTESETRPCSERVER_IS,
                                    dbret, db_strerror(dbret));
            ret = false;
         } 
      }
#endif

      /* the lock parameters only can be set, if we have local spooling.
       * RPC server: use DB_CONFIG file.
       */
      if (server == NULL) {
         /* worst case scenario: n lockers, all changing m objects in 
          * parallel 
          */
#if 0
         int lockers = 5;
         int objects = 5000;
         int locks = lockers * 2 * objects;

         /* set locking params: max lockers */
         if (ret) {
            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbret = env->set_lk_max_lockers(env, lockers);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            if (dbret != 0) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_COULDNTSETLOCKERS_IS,
                                       dbret, db_strerror(dbret));
               ret = false;
            } 
         }
         /* set locking params: max objects */
         if (ret) {
            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbret = env->set_lk_max_objects(env, objects);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            if (dbret != 0) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_COULDNTSETOBJECTS_IS,
                                       dbret, db_strerror(dbret));
               ret = false;
            } 
         }
         /* set locking params: max locks */
         if (ret) {
            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbret = env->set_lk_max_locks(env, locks);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            if (dbret != 0) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_COULDNTSETLOCKS_IS,
                                       dbret, db_strerror(dbret));
               ret = false;
            } 
         }
#endif
      }

      /* open the environment */
      if (ret) {
         int flags = DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | 
                     DB_INIT_TXN;

         if (server == NULL) {
            flags |= DB_THREAD;
            /* Don't do this by default as it's dnagerous with db_dump
               for instance.  See
               <http://docs.oracle.com/cd/E17276_01/html/api_reference/C/envopen.html#open_DB_PRIVATE>  */
            if (bdb_get_private(info))
               flags |= DB_PRIVATE;
         }

         if (bdb_get_recover(info)) {
            flags |= DB_RECOVER;
         }

         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = env->open(env, path, flags,
                           S_IRUSR | S_IWUSR);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         if (dbret != 0){
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_COULDNTOPENENVIRONMENT_SSIS,
                                    server == NULL ? "local spooling" : server, 
                                    path, dbret, db_strerror(dbret));
            ret = false;
            env = NULL;
         }
         
         bdb_set_env(info, env);
      }
   }

   /* now unlock the info structure */
   bdb_unlock_info(info);

   DEXIT;
   return ret;
}

bool 
spool_berkeleydb_open_database(lList **answer_list, bdb_info info, 
                               bool create)
{
   bool ret = true;
   bdb_database i;

   DENTER(BDB_LAYER, "spool_berkeleydb_open_database");

   for (i = BDB_CONFIG_DB; i < BDB_ALL_DBS && ret; i++) {
      DB_ENV *env;
      DB *db;

      int dbret = 0;

      /* we have to lock info, as multiple threads might try to (re)open
       * the database connection in parallel 
       */
      bdb_lock_info(info);
       
      env = bdb_get_env(info);

      if (env == NULL) {
         dstring dbname_dstring = DSTRING_INIT;
         const char *dbname;
         
         dbname = bdb_get_dbname(info, &dbname_dstring);
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_NOCONNECTIONOPEN_S,
                                 dbname);
         sge_dstring_free(&dbname_dstring);
         ret = false;
      }

      /* check db - another thread could have opened it in the meantime */
      if (ret) {
         db = bdb_get_db(info, i);
      }

      if (ret && db == NULL) {
         /* create a database handle */
         if (ret) {
            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbret = db_create(&db, env, 0);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            if (dbret != 0) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_COULDNTCREATEDBHANDLE_IS,
                                       dbret, db_strerror(dbret));
               ret = false;
               db = NULL;
            }
         }

         /* open database handle */
         if (ret) {
            int flags = 0;
            int mode  = 0;

            if (bdb_get_server(info) == NULL) {
               flags |= DB_THREAD;
            }

            /* the config db will only be created, if explicitly requested
             * (in spoolinit). DB already existing will be handled as error.
             * Other databases will be created as needed.
             */
            if (i == BDB_CONFIG_DB) {
               if (create) {
                  flags |= DB_CREATE | DB_EXCL;
                  mode =  S_IRUSR | S_IWUSR;
               }
            } else {
                  flags |= DB_CREATE;
                  mode =  S_IRUSR | S_IWUSR;
            }

            ret = spool_berkeleydb_start_transaction(answer_list, info);
            if (ret) {
               const char *db_name = bdb_get_database_name(i); 
               DB_TXN *txn = bdb_get_txn(info);
               PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
               dbret = db->open(db, txn, db_name, NULL, 
                                DB_BTREE, flags, mode);
               PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
               ret = spool_berkeleydb_end_transaction(answer_list, info, true);
            }
            if (dbret != 0) {
               spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR,
                                       create ? MSG_BERKELEY_COULDNTCREATEDB_SIS
                                              : MSG_BERKELEY_COULDNTOPENDB_SIS,
                                       bdb_get_database_name(i), 
                                       dbret, db_strerror(dbret));
               ret = false;
            }
         }

         /* if everything is ok - set the database handle */
         if (ret) {
            bdb_set_db(info, db, i);
            DPRINTF(("opened database connection, env = %p, db = %p\n", env, db));
         }
      }

      bdb_unlock_info(info);
   }

   DEXIT;
   return ret;
}

bool 
spool_berkeleydb_close_database(lList **answer_list, bdb_info info)
{
   bool ret = true;

   DB_ENV *env;

   /* database name for info or error output */
   char dbname_buffer[MAX_STRING_SIZE];
   dstring dbname_dstring = DSTRING_INIT;
   const char *dbname;

   DENTER(BDB_LAYER, "spool_berkeleydb_close_database");

   sge_dstring_init(&dbname_dstring, dbname_buffer, sizeof(dbname_buffer));
   dbname = bdb_get_dbname(info, &dbname_dstring);

   /* lock the database info, multiple threads might try to close it */
   bdb_lock_info(info);
   env = bdb_get_env(info);
   if (env == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              dbname);
      ret = false;
   } else {
      bdb_database i;
      int dbret;
      for (i = BDB_CONFIG_DB; i < BDB_ALL_DBS; i++) {
         DB *db;

         /* close open database */
         db = bdb_get_db(info, i);
         if (db != NULL) {
            int dbret;

            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbret = db->close(db, 0);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            if (dbret != 0) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_COULDNTCLOSEDB_SIS,
                                       bdb_get_database_name(i), 
                                       dbret, db_strerror(dbret));
               ret = false;
            }

            db = NULL;
            bdb_set_db(info, db, i);
         }
      }

      /* close env in any case, even if db->close failed */
      PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      dbret = env->close(env, 0);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      if (dbret != 0) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_COULDNTCLOSEENVIRONMENT_SIS,
                                 dbname, dbret, db_strerror(dbret));
         ret = false;
      } else {
        answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                ANSWER_QUALITY_INFO, 
                                MSG_BERKELEY_CLOSEDDB_S,
                                dbname);
      }

      env = NULL;
      bdb_set_env(info, env);
   }

   bdb_unlock_info(info);

   DEXIT;
   return ret;
}

/****** sge_bdb/spool_berkeleydb_start_transaction() ***************************
*  NAME
*     spool_berkeleydb_start_transaction() -- start a transaction
*
*  SYNOPSIS
*     bool 
*     spool_berkeleydb_start_transaction(lList **answer_list, bdb_info info) 
*
*  FUNCTION
*     Starts a transaction.
*     Transactions are bound to a certain thread, multiple threads can start
*     transactions in parallel.
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     bdb_info info - database handle
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: spool_berkeleydb_start_transaction() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/spool_berkeleydb_end_transaction()
*******************************************************************************/
bool
spool_berkeleydb_start_transaction(lList **answer_list, bdb_info info)
{
   bool ret = true;

   DB_ENV *env;
   DB_TXN *txn;

   DENTER(BDB_LAYER, "spool_berkeleydb_start_transaction");

   env = bdb_get_env(info);
   txn = bdb_get_txn(info);

   if (env == NULL) {
      dstring dbname_dstring = DSTRING_INIT;
      const char *dbname;
      
      dbname = bdb_get_dbname(info, &dbname_dstring);
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              dbname);
      sge_dstring_free(&dbname_dstring);
      ret = false;
   } else {
      if (txn != NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 "%s", MSG_BERKELEY_TXNALREADYOPEN);
         ret = false;
      } else {
         int dbret;
         int flags = 0;

         /* 
          * RPC server does no deadlock detection - if a lock cannot be 
          * obtained, exit immediately
          */
         if (bdb_get_server(info) != NULL) {
            flags |= DB_TXN_NOWAIT;
         }

         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = env->txn_begin(env, NULL, &txn, flags);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         if (dbret != 0) {
            spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_ERRORSTARTINGTRANSACTION_IS,
                                    dbret, db_strerror(dbret));
            ret = false;
            txn = NULL;
         }
      }

      bdb_set_txn(info, txn);
      DEBUG((SGE_EVENT, "BEGIN transaction"));
   }

   DEXIT;
   return ret;
}

bool
spool_berkeleydb_end_transaction(lList **answer_list, bdb_info info, 
                                 bool commit)
{
   bool ret = true;
   int dbret;

   DB_ENV *env;
   DB_TXN *txn;

   DENTER(BDB_LAYER, "spool_berkeleydb_end_transaction");

   env = bdb_get_env(info);
   txn = bdb_get_txn(info);

   if (env == NULL) {
      dstring dbname_dstring = DSTRING_INIT;
      const char *dbname;
      
      dbname = bdb_get_dbname(info, &dbname_dstring);
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              dbname);
      sge_dstring_free(&dbname_dstring);
      ret = false;
   } else {
      if (txn == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 "%s", MSG_BERKELEY_TXNNOTOPEN);
         ret = false;
      } else {
         if (commit) {
            DEBUG((SGE_EVENT, "COMMIT transaction"));
            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbret = txn->commit(txn, 0);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         } else {
            DEBUG((SGE_EVENT, "ABORT transaction"));
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_WARNING, 
                                    "%s", MSG_BERKELEY_ABORTINGTRANSACTION);
            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbret = txn->abort(txn);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         }

         if (dbret != 0) {
            spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_ERRORENDINGTRANSACTION_IS,
                                    dbret, db_strerror(dbret));
            ret = false;
         }

         txn = NULL;
         bdb_set_txn(info, txn);
      }
   }

   DEXIT;
   return ret;
}

bool 
spool_berkeleydb_trigger(lList **answer_list, bdb_info info, 
                         time_t trigger, time_t *next_trigger)
{
   bool ret = true;

   DENTER(BDB_LAYER, "spool_berkeleydb_trigger");

   if (bdb_get_next_clear(info) <= trigger) {
      /* 
       * in the clear interval, we 
       * - clear unused transaction logs for local spooling
       * - do a dummy request in case of RPC spooling to avoid timeouts
       */
      if (bdb_get_server(info) == NULL) {
         ret = spool_berkeleydb_clear_log(answer_list, info);
      } else {
         ret = spool_berkeleydb_trigger_rpc(answer_list, info);
      }
      bdb_set_next_clear(info, trigger + BERKELEYDB_CLEAR_INTERVAL);
   }

   if (bdb_get_next_checkpoint(info) <= trigger) {
      ret = spool_berkeleydb_checkpoint(answer_list, info);
      bdb_set_next_checkpoint(info, trigger + BERKELEYDB_CHECKPOINT_INTERVAL);
   }

   /* set time of next trigger */
   *next_trigger = MIN(bdb_get_next_clear(info), bdb_get_next_checkpoint(info));

   DEXIT;
   return ret;
}

bool 
spool_berkeleydb_read_list(lList **answer_list, bdb_info info,
                           const bdb_database database,
                           lList **list, const lDescr *descr,
                           const char *key)
{
   bool ret = true;
   int dbret;

   DB *db;
   DB_TXN *txn;

   DBT key_dbt, data_dbt;
   DBC *dbc;

   DENTER(BDB_LAYER, "spool_berkeleydb_read_list");

   db  = bdb_get_db(info, database);
   txn = bdb_get_txn(info);

   if (db == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              bdb_get_database_name(database));
      spool_berkeleydb_error_close(info);
      ret = false;
   } else {
      DEBUG((SGE_EVENT, "querying objects with keys %s*", key));

      PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      dbret = db->cursor(db, txn, &dbc, 0);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      if (dbret != 0) {
         spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_CANNOTCREATECURSOR_IS,
                                 dbret, db_strerror(dbret));
         ret = false;
      } else {
         bool done;
         /* initialize query to first record for this object type */
         memset(&key_dbt, 0, sizeof(key_dbt));
         memset(&data_dbt, 0, sizeof(data_dbt));
         key_dbt.data = (void *)key;
         key_dbt.size = strlen(key) + 1;
         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = dbc->c_get(dbc, &key_dbt, &data_dbt, DB_SET_RANGE);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         done = false;
         while (!done) {
            if (dbret != 0 && dbret != DB_NOTFOUND) {
               spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_QUERYERROR_SIS,
                                       key, dbret, db_strerror(dbret));
               ret = false;
               done = true;
               break;
            } else if (dbret == DB_NOTFOUND) {
               DPRINTF(("last record reached\n"));
               done = true;
               break;
            } else if (key_dbt.data != NULL && 
                       strncmp(key_dbt.data, key, strlen(key)) 
                       != 0) {
               DPRINTF(("current key is %s\n", key_dbt.data));
               DPRINTF(("last record of this object type reached\n"));
               done = true;
               break;
            } else {
               sge_pack_buffer pb;
               lListElem *object = NULL;
               int cull_ret;

               DPRINTF(("read object with key "SFQ", size %d\n", 
                        key_dbt.data, data_dbt.size));
               cull_ret = init_packbuffer_from_buffer(&pb, data_dbt.data, 
                                                      data_dbt.size);
               if (cull_ret != PACK_SUCCESS) {
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                          ANSWER_QUALITY_ERROR, 
                                          MSG_BERKELEY_UNPACKINITERROR_SS,
                                          (char *)key_dbt.data,
                                          cull_pack_strerror(cull_ret));
                  ret = false;
                  done = true;
                  break;
               }

               cull_ret = cull_unpack_elem_partial(&pb, &object, descr, pack_part);
               if (cull_ret != PACK_SUCCESS) {
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                          ANSWER_QUALITY_ERROR, 
                                          MSG_BERKELEY_UNPACKERROR_SS,
                                          (char *)key_dbt.data,
                                          cull_pack_strerror(cull_ret));
                  ret = false;
                  done = true;
                  break;
               }
               /* we may not free the packbuffer: it references the buffer
                * delivered from the database
                * clear_packbuffer(&pb);
                */
               if (object != NULL) {
                  if (*list == NULL) {
                     *list = lCreateList(key, descr);
                  }
                  lAppendElem(*list, object);
               }

               /* get next record */
               PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
               dbret = dbc->c_get(dbc, &key_dbt, &data_dbt, DB_NEXT);
               PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            }
         }
         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbc->c_close(dbc);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      }
   }

   DEXIT;
   return ret;
}

bool 
spool_berkeleydb_write_object(lList **answer_list, bdb_info info,
                              const bdb_database database,
                              const lListElem *object, const char *key)
{
   bool ret = true;
   lList *tmp_list = NULL;

   DENTER(BDB_LAYER, "spool_berkeleydb_write_object");

   /* do not spool free elems. If a free elem is passed, put a copy 
    * into a temporary list and spool this copy.
    */
   if (object->status == FREE_ELEM) {
      tmp_list = lCreateList("tmp", object->descr);
      lAppendElem(tmp_list, (lListElem *)object);
   }

   {
      sge_pack_buffer pb;
      int cull_ret;

      cull_ret = init_packbuffer(&pb, 8192, 0);
      if (cull_ret != PACK_SUCCESS) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_PACKINITERROR_SS,
                                 key,
                                 cull_pack_strerror(cull_ret));
         ret = false;
      } else {
         cull_ret = cull_pack_elem_partial(&pb, object, NULL, pack_part);
         if (cull_ret != PACK_SUCCESS) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_PACKERROR_SS,
                                    key,
                                    cull_pack_strerror(cull_ret));
            ret = false;
         } else { 
            int dbret;
            DBT key_dbt, data_dbt;

            DB *db = bdb_get_db(info, database);
            DB_TXN *txn = bdb_get_txn(info);

            if (db == NULL) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_NOCONNECTIONOPEN_S,
                                       bdb_get_database_name(database));
               spool_berkeleydb_error_close(info);
               ret = false;
            }

            if (ret) {
               memset(&key_dbt, 0, sizeof(key_dbt));
               memset(&data_dbt, 0, sizeof(data_dbt));
               key_dbt.data = (void *)key;
               key_dbt.size = strlen(key) + 1;
               data_dbt.data = pb.head_ptr;
               data_dbt.size = pb.bytes_used;

               DPRINTF(("storing object with key "SFQ", size = %d "
                        "to env = %p, db = %p, txn = %p, txn_id = %d\n",
                        key, data_dbt.size, bdb_get_env(info), db,
                        txn, (txn->id == NULL) ? 0 : txn->id(txn)));

               /* Store a key/data pair. */
               PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
               dbret = db->put(db, txn, &key_dbt, &data_dbt, 0);
               PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);

               if (dbret != 0) {
                  spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                          ANSWER_QUALITY_ERROR, 
                                          MSG_BERKELEY_PUTERROR_SIS,
                                          key, dbret, db_strerror(dbret));
                  ret = false;
               } else {
                  DEBUG((SGE_EVENT, "stored object with key "SFQ", size %d", key, data_dbt.size));
               }
            }
         }

         clear_packbuffer(&pb);
      }
   }

   if (tmp_list != NULL) {
      lDechainElem(tmp_list, (lListElem *)object);
      lFreeList(&tmp_list);
   }

   DEXIT;
   return ret;
}

bool spool_berkeleydb_write_string(lList **answer_list, bdb_info info,
                              const bdb_database database,
                              const char *key, const char *str)
{
   bool ret = true;

   DENTER(BDB_LAYER, "spool_berkeleydb_write_string");

   {
      int dbret;
      DBT key_dbt, data_dbt;

      DB *db = bdb_get_db(info, database);
      DB_TXN *txn = bdb_get_txn(info);

      if (db == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_NOCONNECTIONOPEN_S,
                                 bdb_get_database_name(database));
         spool_berkeleydb_error_close(info);
         ret = false;
      } else {
         memset(&key_dbt, 0, sizeof(key_dbt));
         memset(&data_dbt, 0, sizeof(data_dbt));
         key_dbt.data = (void *)key;
         key_dbt.size = strlen(key) + 1;
         data_dbt.data = (void *) str;
         data_dbt.size = strlen(str) + 1;

         DPRINTF(("storing string with key "SFQ", size = %d "
                  "to env = %p, db = %p, txn = %p, txn_id = %d\n", 
                  key, data_dbt.size, bdb_get_env(info), db, 
                  txn, (txn->id == NULL) ? 0 : txn->id(txn)));

         /* Store a key/data pair. */
         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = db->put(db, txn, &key_dbt, &data_dbt, 0);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);

         if (dbret != 0) {
            spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_PUTERROR_SIS,
                                    key, dbret, db_strerror(dbret));
            ret = false;
         } else {
            DEBUG((SGE_EVENT, "stored object with key "SFQ", size %d", key, data_dbt.size));
         }
      }
   }

   DEXIT;
   return ret;
}

bool
spool_berkeleydb_write_pe_task(lList **answer_list, bdb_info info,
                               const lListElem *object, 
                               u_long32 job_id, u_long32 ja_task_id,
                               const char *pe_task_id)
{
   bool ret = true;
   dstring dbkey_dstring;
   char dbkey_buffer[MAX_STRING_SIZE];
   const char *dbkey;

   sge_dstring_init(&dbkey_dstring, 
                    dbkey_buffer, sizeof(dbkey_buffer));

   dbkey = sge_dstring_sprintf(&dbkey_dstring,
			       "%s:%8"sge_U32CLetter".%8"sge_U32CLetter" %s",
                               object_type_get_name(SGE_TYPE_PETASK),
                               job_id, ja_task_id, pe_task_id);

   ret = spool_berkeleydb_write_object(answer_list, info, BDB_JOB_DB,
                                       object, dbkey);

   return ret;
}

bool
spool_berkeleydb_write_ja_task(lList **answer_list, bdb_info info,
                               const lListElem *object, 
                               u_long32 job_id, u_long32 ja_task_id)
{
   bool ret = true;
   dstring dbkey_dstring;
   char dbkey_buffer[MAX_STRING_SIZE];
   const char *dbkey;
   lList *tmp_list = NULL;

   sge_dstring_init(&dbkey_dstring,
                    dbkey_buffer, sizeof(dbkey_buffer));

   dbkey = sge_dstring_sprintf(&dbkey_dstring, "%s:%8"sge_U32CLetter".%8"sge_U32CLetter,
                               object_type_get_name(SGE_TYPE_JATASK),
                               job_id, ja_task_id);

   lXchgList((lListElem *)object, JAT_task_list, &tmp_list);
   ret = spool_berkeleydb_write_object(answer_list, info, BDB_JOB_DB,
                                       object, dbkey);
   lXchgList((lListElem *)object, JAT_task_list, &tmp_list);

   return ret;
}

bool
spool_berkeleydb_write_job(lList **answer_list, bdb_info info,
                           const lListElem *object, 
                           u_long32 job_id, u_long32 ja_task_id, bool only_job)
{
   bool ret = true;
   dstring dbkey_dstring;
   char dbkey_buffer[MAX_STRING_SIZE];
   const char *dbkey;
   lList *tmp_list = NULL;

   sge_dstring_init(&dbkey_dstring,
                    dbkey_buffer, sizeof(dbkey_buffer));

   dbkey = sge_dstring_sprintf(&dbkey_dstring, "%s:%8"sge_U32CLetter,
                               object_type_get_name(SGE_TYPE_JOB), job_id);

   lXchgList((lListElem *)object, JB_ja_tasks, &tmp_list);
   
   ret = spool_berkeleydb_write_object(answer_list, info, BDB_JOB_DB,
                                       object, dbkey);

   lXchgList((lListElem *)object, JB_ja_tasks, &tmp_list);

   if (ret && !only_job) {
      lListElem *ja_task;

      ja_task = lGetElemUlong(lGetList(object, JB_ja_tasks), JAT_task_number, ja_task_id);
      if (ja_task != NULL) {
         ret = spool_berkeleydb_write_ja_task(answer_list, info, ja_task, job_id, ja_task_id);
      }
   }

   return ret;
}

bool
spool_berkeleydb_write_cqueue(lList **answer_list, bdb_info info, 
                              const lListElem *object, const char *key)
{
   bool ret = true;
   dstring dbkey_dstring;
   char dbkey_buffer[MAX_STRING_SIZE];
   const char *dbkey;
   lList *tmp_list = NULL;

   sge_dstring_init(&dbkey_dstring,
                    dbkey_buffer, sizeof(dbkey_buffer));

   dbkey = sge_dstring_sprintf(&dbkey_dstring, "%s:%s", 
                               object_type_get_name(SGE_TYPE_CQUEUE), 
                               key);

   lXchgList((lListElem *)object, CQ_qinstances, &tmp_list);
   
   ret = spool_berkeleydb_write_object(answer_list, info, BDB_CONFIG_DB,
                                       object, dbkey);

   lXchgList((lListElem *)object, CQ_qinstances, &tmp_list);

   return ret;
}

/****** spool/berkeleydb/spool_berkeleydb_delete_object() **********************
*  NAME
*     spool_berkeleydb_delete_object() -- delete one or multiple objects
*
*  SYNOPSIS
*     bool 
*     spool_berkeleydb_delete_object(lList **answer_list, bdb_info info,
*                                    const char *key, bool sub_objects) 
*
*  FUNCTION
*     If sub_objects = false, deletes the object specified by key.
*     If sub_objects = true, key will be used as pattern to delete multiple
*     objects.
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     bdb_info info - database handle
*     const char *key       - key
*     bool sub_objects      - use key as pattern?
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: spool_berkeleydb_delete_object() is MT safe 
*******************************************************************************/
bool
spool_berkeleydb_delete_object(lList **answer_list, bdb_info info, 
                               const bdb_database database,
                               const char *key, bool sub_objects)
{
   bool ret = true;

   int dbret;

   DB *db;
   DB_TXN *txn;

   DENTER(BDB_LAYER, "spool_berkeleydb_delete_object");

   db = bdb_get_db(info, database);
   txn = bdb_get_txn(info);

   if (db == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              bdb_get_database_name(database));
      spool_berkeleydb_error_close(info);
      ret = false;
   } else {
      if (sub_objects) {
         DBC *dbc;

         DPRINTF(("querying objects with keys %s*\n", key));

         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = db->cursor(db, txn, &dbc, 0);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         if (dbret != 0) {
            spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_CANNOTCREATECURSOR_IS,
                                    dbret, db_strerror(dbret));
            ret = false;
         } else {
            bool done;
            DBT cursor_dbt, data_dbt;
            /* initialize query to first record for this object type */
            memset(&cursor_dbt, 0, sizeof(cursor_dbt));
            memset(&data_dbt, 0, sizeof(data_dbt));
            cursor_dbt.data = (void *)key;
            cursor_dbt.size = strlen(key) + 1;
            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbret = dbc->c_get(dbc, &cursor_dbt, &data_dbt, DB_SET_RANGE);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            done = false;
            while (!done) {
               if (dbret != 0 && dbret != DB_NOTFOUND) {
                  spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                          ANSWER_QUALITY_ERROR, 
                                          MSG_BERKELEY_QUERYERROR_SIS,
                                          key, dbret, db_strerror(dbret));
                  ret = false;
                  done = true;
                  break;
               } else if (dbret == DB_NOTFOUND) {
                  DPRINTF(("last record reached\n"));
                  done = true;
                  break;
               } else if (cursor_dbt.data != NULL && 
                          strncmp(cursor_dbt.data, key, strlen(key)) 
                          != 0) {
                  DPRINTF(("current key is %s\n", cursor_dbt.data));
                  DPRINTF(("last record of this object type reached\n"));
                  done = true;
                  break;
               } else {
                  int delete_ret;
                  DBT delete_dbt;

                  /* remember key of record to delete */
                  memset(&delete_dbt, 0, sizeof(delete_dbt));
                  delete_dbt.data = strdup(cursor_dbt.data);
                  delete_dbt.size = cursor_dbt.size;

                  /* switch cursor to next position */
                  memset(&cursor_dbt, 0, sizeof(cursor_dbt));
                  memset(&data_dbt, 0, sizeof(data_dbt));
                  PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
                  dbret = dbc->c_get(dbc, &cursor_dbt, &data_dbt, DB_NEXT);
                  PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);

                  /* delete record with stored key */
                  PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
                  delete_ret = db->del(db, txn, &delete_dbt, 0);
                  PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
                  if (delete_ret != 0) {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                             ANSWER_QUALITY_ERROR, 
                                             MSG_BERKELEY_DELETEERROR_SIS,
                                             (char *)delete_dbt.data,
                                             delete_ret, db_strerror(delete_ret));
                     ret = false;
                     sge_free(&(delete_dbt.data));
                     done = true;
                     break;
                  } else {
                     DEBUG((SGE_EVENT, "deleted record with key "SFQ, (char *)delete_dbt.data));
                  }
                  sge_free(&(delete_dbt.data));
               }
            }
            PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            dbc->c_close(dbc);
            PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         }
      } else {
         DBT delete_dbt;
         memset(&delete_dbt, 0, sizeof(delete_dbt));
         delete_dbt.data = (void *)key;
         delete_dbt.size = strlen(key) + 1;
         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = db->del(db, txn, &delete_dbt, 0);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         if (dbret != 0 /* && dbret != DB_NOTFOUND */) {
            spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_DELETEERROR_SIS,
                                    key, dbret, db_strerror(dbret));
            ret = false;
         } else {
            DEBUG((SGE_EVENT, "deleted record with key "SFQ, key));
         }
      }
   }

   DEXIT;
   return ret;
}

/****** spool/berkeleydb/spool_berkeleydb_delete_pe_task() *********************
*  NAME
*     spool_berkeleydb_delete_pe_task() -- delete one or multiple pe task(s)
*
*  SYNOPSIS
*     bool 
*     spool_berkeleydb_delete_pe_task(lList **answer_list, bdb_info info,
*                                     const char *key, bool sub_objects) 
*
*  FUNCTION
*     Deletes one or multiple pe_tasks specified by key.
*  
*     The key has the form "<job_id>.<ja_task_id> <pe_task_id>" formatted as 
*     "%8d.%8d %s".
*     If sub_objects = true, it can be used as pattern, typically used to 
*     delete all pe_tasks of a certain ja_task by setting key to 
*     "<job_id>.<ja_task_id>" or just "<job_id>" to delete all pe_tasks
*     dependent on a certain job.
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     bdb_info info - database handle
*     const char *key       - key
*     bool sub_objects      - interpret key as pattern?
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: spool_berkeleydb_delete_pe_task() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/spool_berkeleydb_delete_object()
*******************************************************************************/
bool
spool_berkeleydb_delete_pe_task(lList **answer_list, bdb_info info,
                                const char *key, bool sub_objects)
{
   bool ret = true;

   dstring dbkey_dstring;
   char dbkey_buffer[MAX_STRING_SIZE];
   const char *dbkey;
   const char *table_name;

   sge_dstring_init(&dbkey_dstring, dbkey_buffer, sizeof(dbkey_buffer));
   table_name = object_type_get_name(SGE_TYPE_PETASK);
   dbkey = sge_dstring_sprintf(&dbkey_dstring, "%s:%s", table_name, key);
   ret = spool_berkeleydb_delete_object(answer_list, info, BDB_JOB_DB, 
                                        dbkey, sub_objects);

   return ret;
}

/****** spool/berkeleydb/spool_berkeleydb_delete_ja_task() *********************
*  NAME
*     spool_berkeleydb_delete_ja_task() -- delete ja_task(s)
*
*  SYNOPSIS
*     bool 
*     spool_berkeleydb_delete_ja_task(lList **answer_list, bdb_info info,
*                                     const char *key, bool sub_objects) 
*
*  FUNCTION
*     Deletes one or multiple ja_tasks specified by key.
*     The ja_task(s) and all dependent pe_tasks are deleted.
*  
*     The key has the form "<job_id>.<ja_task_id>" formatted as "%8d.%8d".
*     If sub_objects = true, it can be used as pattern, typically used to 
*     delete all ja_tasks of a certain job by setting key to "<job_id>.".
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     bdb_info info - database handle
*     const char *key       - key
*     bool sub_objects      - use key as pattern?
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: spool_berkeleydb_delete_ja_task() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/spool_berkeleydb_delete_object()
*     spool/berkeleydb/spool_berkeleydb_delete_pe_task()
*******************************************************************************/
bool
spool_berkeleydb_delete_ja_task(lList **answer_list, bdb_info info,
                                const char *key, bool sub_objects)
{
   bool ret = true;

   dstring dbkey_dstring;
   char dbkey_buffer[MAX_STRING_SIZE];
   const char *dbkey;
   const char *table_name;

   sge_dstring_init(&dbkey_dstring, dbkey_buffer, sizeof(dbkey_buffer));
   table_name = object_type_get_name(SGE_TYPE_JATASK);
   dbkey = sge_dstring_sprintf(&dbkey_dstring, "%s:%s", table_name, key);
   ret = spool_berkeleydb_delete_object(answer_list, info, BDB_JOB_DB, 
                                        dbkey, sub_objects);

   if (ret) {
      ret = spool_berkeleydb_delete_pe_task(answer_list, info, key, true);
   }

   return ret;
}

/****** spool/berkeleydb/spool_berkeleydb_delete_job() *************************
*  NAME
*     spool_berkeleydb_delete_job() -- delete a job
*
*  SYNOPSIS
*     bool 
*     spool_berkeleydb_delete_job(lList **answer_list, bdb_info info, 
*                                 const char *key, bool sub_objects) 
*
*  FUNCTION
*     Deletes the given job and all its ja_tasks.
*     Key usually will be the unique job id formatted with %8d, but the function
*     allows for some sort of pattern matching by specifying only parts of the
*     jobid, e.g. the key "00001" will delete all jobs from 1000 to 1999, 
*     an empty string will mean "delete all jobs", if sub_objects = true.
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     bdb_info info - database handle
*     const char *key       - key (job_number)
*     bool sub_objects      - is the given key a pattern?
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: spool_berkeleydb_delete_job() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/spool_berkeleydb_delete_object()
*     spool/berkeleydb/spool_berkeleydb_delete_ja_task()
*******************************************************************************/
bool
spool_berkeleydb_delete_job(lList **answer_list, bdb_info info,
                            const char *key, bool sub_objects)
{
   bool ret = true;

   dstring dbkey_dstring;
   char dbkey_buffer[MAX_STRING_SIZE];
   const char *dbkey;
   const char *table_name;

   sge_dstring_init(&dbkey_dstring, dbkey_buffer, sizeof(dbkey_buffer));
   table_name = object_type_get_name(SGE_TYPE_JOB);
   dbkey = sge_dstring_sprintf(&dbkey_dstring, "%s:%s", table_name, key);
   ret = spool_berkeleydb_delete_object(answer_list, info, BDB_JOB_DB, 
                                        dbkey, sub_objects);

   if (ret) {
      ret = spool_berkeleydb_delete_ja_task(answer_list, info, key, true);
   }

   return ret;
}

/****** spool/berkeleydb/spool_berkeleydb_delete_cqueue() **********************
*  NAME
*     spool_berkeleydb_delete_cqueue() -- delete a cluster queue
*
*  SYNOPSIS
*     bool 
*     spool_berkeleydb_delete_cqueue(lList **answer_list, bdb_info info,
*                                    const char *key) 
*
*  FUNCTION
*     Deletes a cluster queue and all its queue instances.
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     bdb_info info - database handle
*     const char *key       - key (name) of cluster queue to delete
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: spool_berkeleydb_delete_cqueue() is MT safe 
*
*  SEE ALSO
*     spool/berkeleydb/spool_berkeleydb_delete_object()
*******************************************************************************/
bool
spool_berkeleydb_delete_cqueue(lList **answer_list, bdb_info info,
                               const char *key)
{
   bool ret = true;

   dstring dbkey_dstring;
   char dbkey_buffer[MAX_STRING_SIZE];
   const char *dbkey;
   const char *table_name;

   sge_dstring_init(&dbkey_dstring, dbkey_buffer, sizeof(dbkey_buffer));
   table_name = object_type_get_name(SGE_TYPE_CQUEUE);
   dbkey = sge_dstring_sprintf(&dbkey_dstring, "%s:%s", table_name, key);
   ret = spool_berkeleydb_delete_object(answer_list, info, BDB_CONFIG_DB,
                                        dbkey, false);

   if (ret) {
      table_name = object_type_get_name(SGE_TYPE_QINSTANCE);
      dbkey = sge_dstring_sprintf(&dbkey_dstring, "%s:%s@", table_name, key);
      ret = spool_berkeleydb_delete_object(answer_list, info, BDB_CONFIG_DB,
                                           dbkey, true);
   }

   return ret;
}


/* ---- static functions ---- */

static void 
spool_berkeleydb_error_close(bdb_info info)
{
   DB_ENV *env;
   DB     *db;
   DB_TXN *txn;
   bdb_database i;

   /* try to shutdown all open resources */
   txn = bdb_get_txn(info);
   if (txn != NULL) {
      txn->abort(txn);
      bdb_set_txn(info, NULL);
   }

   for (i = BDB_CONFIG_DB; i < BDB_ALL_DBS; i++) {
      db = bdb_get_db(info, i);
      if (db != NULL) {
         db->close(db, 0);
         bdb_set_db(info, NULL, i);
      }
   }

   env = bdb_get_env(info);
   if (env != NULL) {
      env->close(env, 0);
      bdb_set_env(info, NULL);
   }
}

static void
spool_berkeleydb_handle_bdb_error(lList **answer_list, bdb_info info, 
                                  int bdb_errno)
{
   /* we lost the connection to a RPC server */
   if (bdb_errno == DB_NOSERVER
#ifdef DB_NOSERVER_ID
       || bdb_errno == DB_NOSERVER_ID
#endif
       ) {
      const char *server = bdb_get_server(info);
      const char *path   = bdb_get_path(info);

      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_CONNECTION_LOST_SS,
                              server != NULL ? server : "no server defined",
                              path != NULL ? path : "no database path defined");

      spool_berkeleydb_error_close(info);
   }
#ifdef DB_NOSERVER_HOME
   else if (bdb_errno == DB_NOSERVER_HOME) {
      const char *server = bdb_get_server(info);
      const char *path   = bdb_get_path(info);

      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_RPCSERVERLOSTHOME_SS,
                              server != NULL ? server : "no server defined",
                              path != NULL ? path : "no database path defined");

      spool_berkeleydb_error_close(info);
   }
#endif
   else if (bdb_errno == DB_RUNRECOVERY) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              "%s", MSG_BERKELEY_RUNRECOVERY);

      spool_berkeleydb_error_close(info);
   }
}

bool 
spool_berkeleydb_check_reopen_database(lList **answer_list, 
                                       bdb_info info)
{
   bool ret = true;
   DB_ENV *env;

   DENTER(BDB_LAYER, "spool_berkeleydb_check_reopen_database");

   env = bdb_get_env(info);

   /*
    * if environment is not set, it was either
    * - closed due to an error condition
    * - never open for this thread
    * try to open it.
    */
   if (env == NULL) {
      ret = spool_berkeleydb_create_environment(answer_list, info);

      if (ret) {
         ret = spool_berkeleydb_open_database(answer_list, info, false);
      }
   }

   DEXIT;
   return ret;
}

bool 
spool_berkeleydb_read_keys(lList **answer_list, bdb_info info,
                           const bdb_database database,
                           lList **list, const char *key)
{
   bool ret = true;
   int dbret;

   DB *db;
   DB_TXN *txn;

   DBT key_dbt, data_dbt;
   DBC *dbc;

   DENTER(BDB_LAYER, "spool_berkeleydb_read_keys");

   db  = bdb_get_db(info, database);
   txn = bdb_get_txn(info);

   if (db == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              bdb_get_database_name(database));
      ret = false;
   } else {
      DPRINTF(("querying objects with keys %s*\n", key));

      PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      dbret = db->cursor(db, txn, &dbc, 0);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      if (dbret != 0) {
         spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_CANNOTCREATECURSOR_IS,
                                 dbret, db_strerror(dbret));
         ret = false;
      } else {
         bool done;
         /* initialize query to first record for this object type */
         memset(&key_dbt, 0, sizeof(key_dbt));
         memset(&data_dbt, 0, sizeof(data_dbt));
         key_dbt.data = (void *)key;
         key_dbt.size = strlen(key) + 1;
         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = dbc->c_get(dbc, &key_dbt, &data_dbt, DB_SET_RANGE);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         done = false;
         while (!done) {
            if (dbret != 0 && dbret != DB_NOTFOUND) {
               spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_BERKELEY_QUERYERROR_SIS,
                                       key, dbret, db_strerror(dbret));
               ret = false;
               done = true;
               break;
            } else if (dbret == DB_NOTFOUND) {
               DPRINTF(("last record reached\n"));
               done = true;
               break;
            } else if (key_dbt.data != NULL && 
                       strncmp(key_dbt.data, key, strlen(key)) 
                       != 0) {
               DPRINTF(("current key is %s\n", key_dbt.data));
               DPRINTF(("last record of this object type reached\n"));
               done = true;
               break;
            } else {
               DPRINTF(("read object with key "SFQ", size %d\n", 
                        key_dbt.data, data_dbt.size));
               lAddElemStr(list, STU_name, key_dbt.data, STU_Type);

               PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
               dbret = dbc->c_get(dbc, &key_dbt, &data_dbt, DB_NEXT);
               PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
            }
         }
         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbc->c_close(dbc);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      }
   }

   DEXIT;
   return ret;
}

lListElem *
spool_berkeleydb_read_object(lList **answer_list, bdb_info info,
                             const bdb_database database,
                             const char *key)
{
   lListElem *ret = NULL;
   int dbret;

   DB *db;
   DB_TXN *txn;

   DBT key_dbt, data_dbt;

   DENTER(BDB_LAYER, "spool_berkeleydb_read_object");

   db  = bdb_get_db(info, database);
   txn = bdb_get_txn(info);

   if (db == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              bdb_get_database_name(database));
   } else {
      DPRINTF(("querying object with key %s\n", key));

      /* initialize query to first record for this object type */
      memset(&key_dbt, 0, sizeof(key_dbt));
      key_dbt.data = (void *)key;
      key_dbt.size = strlen(key) + 1;
      memset(&data_dbt, 0, sizeof(data_dbt));
      data_dbt.flags = DB_DBT_MALLOC;
      PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      dbret = db->get(db, txn, &key_dbt, &data_dbt, 0);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      if (dbret != 0) {
         spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_QUERYERROR_SIS,
                                 key, dbret, db_strerror(dbret));
      } else {
         sge_pack_buffer pb;
         int cull_ret;
         const lDescr *descr;

         DPRINTF(("read object with key "SFQ", size %d\n", 
                  key_dbt.data, data_dbt.size));
         cull_ret = init_packbuffer_from_buffer(&pb, data_dbt.data, 
                                                data_dbt.size);
         if (cull_ret != PACK_SUCCESS) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_UNPACKINITERROR_SS,
                                    (char *)key_dbt.data,
                                    cull_pack_strerror(cull_ret));
            ret = NULL;
         }
         DPRINTF(("init_packbuffer succeeded\n"));

         descr = object_type_get_descr(object_name_get_type(key_dbt.data));
         cull_ret = cull_unpack_elem_partial(&pb, &ret, descr, pack_part);
         if (cull_ret != PACK_SUCCESS) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_UNPACKERROR_SS,
                                    (char *)key_dbt.data,
                                    cull_pack_strerror(cull_ret));
            ret = NULL;
         }

         /* We specified DB_DBT_MALLOC - BDB will malloc memory for each
          * object found and we have to free it.
          */
         if (data_dbt.data != NULL) {
            sge_free(&(data_dbt.data));
         }
      }
   }

   DRETURN(ret);
}

char *
spool_berkeleydb_read_string(lList **answer_list, bdb_info info,
                             const bdb_database database,
                             const char *key)
{
   char *ret = NULL;
   int dbret;

   DB *db;
   DB_TXN *txn;

   DBT key_dbt, data_dbt;

   DENTER(BDB_LAYER, "spool_berkeleydb_read_string");

   db  = bdb_get_db(info, database);
   txn = bdb_get_txn(info);

   if (db == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              bdb_get_database_name(database));
   } else {
      DPRINTF(("querying string with key %s\n", key));

      /* initialize query to first record for this object type */
      memset(&key_dbt, 0, sizeof(key_dbt));
      key_dbt.data = (void *)key;
      key_dbt.size = strlen(key) + 1;
      memset(&data_dbt, 0, sizeof(data_dbt));
      data_dbt.flags = DB_DBT_MALLOC;
      PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      dbret = db->get(db, txn, &key_dbt, &data_dbt, 0);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      if (dbret != 0) {
         spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_QUERYERROR_SIS,
                                 key, dbret, db_strerror(dbret));
      } else {
         ret = (char *) data_dbt.data;
      }
   }

   DRETURN(ret);
}

static bool
spool_berkeleydb_clear_log(lList **answer_list, bdb_info info)
{
   bool ret = true;
   DB_ENV *env;

   DENTER(BDB_LAYER, "spool_berkeleydb_clear_log");

   /* check connection */
   env = bdb_get_env(info);
   if (env == NULL) {
      dstring dbname_dstring = DSTRING_INIT;
      const char *dbname;
   
      dbname = bdb_get_dbname(info, &dbname_dstring);
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              dbname);
      sge_dstring_free(&dbname_dstring);
      ret = false;
   }

   if (ret) {
      int dbret;
      char **list = NULL;

      PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      dbret = env->log_archive(env, &list, DB_ARCH_ABS);
      PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
      if (dbret != 0) {
         spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_CANNOTRETRIEVELOGARCHIVE_IS,
                                 dbret, db_strerror(dbret));
         ret = false;
      }

      if (ret && list != NULL) {
         char **file;

         for (file = list; *file != NULL; file++) {
            if (remove(*file) != 0) {
               dstring error_dstring = DSTRING_INIT;

               answer_list_add_sprintf(answer_list, STATUS_EDISK, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_ERRORDELETINGFILE_SS,
                                       *file,
                                       sge_strerror(errno, &error_dstring));
               sge_dstring_free(&error_dstring);
               ret = false;
               break;
            }
         }

         sge_free(&list);
      }
   }

   DEXIT;
   return ret;
}

static bool
spool_berkeleydb_trigger_rpc(lList **answer_list, bdb_info info)
{
   bool ret = true;
   DB_ENV *env;

   DENTER(BDB_LAYER, "spool_berkeleydb_trigger_rpc");

   /* check connection */
   env = bdb_get_env(info);
   if (env == NULL) {
      dstring dbname_dstring = DSTRING_INIT;
      const char *dbname;
   
      dbname = bdb_get_dbname(info, &dbname_dstring);
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_BERKELEY_NOCONNECTIONOPEN_S,
                              dbname);
      sge_dstring_free(&dbname_dstring);
      ret = false;
   }

   if (ret) {
      lList *local_answer_list = NULL;
      lListElem *ep;

      ep = spool_berkeleydb_read_object(&local_answer_list, info, BDB_CONFIG_DB,
                                        "..trigger_bdb_rpc_server..");
      lFreeElem(&ep);
      lFreeList(&local_answer_list);
   }

   DEXIT;
   return ret;
}

static bool
spool_berkeleydb_checkpoint(lList **answer_list, bdb_info info)
{
   bool ret = true;

   DENTER(BDB_LAYER, "spool_berkeleydb_checkpoint");

   /* only necessary for local spooling */
   if (bdb_get_server(info) == NULL) {
      DB_ENV *env;

      env = bdb_get_env(info);
      if (env == NULL) {
      dstring dbname_dstring = DSTRING_INIT;
      const char *dbname;
      
         dbname = bdb_get_dbname(info, &dbname_dstring);
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_BERKELEY_NOCONNECTIONOPEN_S,
                                 dbname);
         sge_dstring_free(&dbname_dstring);
         ret = false;
      }

      if (ret) {
         int dbret;

         PROF_START_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         dbret = env->txn_checkpoint(env, 0, 0, 0);
         PROF_STOP_MEASUREMENT(SGE_PROF_SPOOLINGIO);
         if (dbret != 0) {
            spool_berkeleydb_handle_bdb_error(answer_list, info, dbret);
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_BERKELEY_CANNOTCHECKPOINT_IS,
                                    dbret, db_strerror(dbret));
            ret = false;
         } 
      }
   }

   DEXIT;
   return ret;
}

