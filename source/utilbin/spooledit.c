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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_stdio.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_dstring.h"
#include "uti/sge_spool.h"
#include "uti/sge_uidgid.h"
#include "uti/setup_path.h"
#include "uti/sge_prog.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_answer.h"

#include "spool/sge_spooling.h"
#include "spool/loader/sge_spooling_loader.h"
#include "spool/berkeleydb/sge_bdb.h"

#include "gdi/sge_gdi_ctx.h"

#include "sge_mt_init.h"
#include "msg_common.h"
#include "msg_utilbin.h"



static void 
usage(const char *argv0)
{
   fprintf(stderr, "%s\n %s command\n\n", MSG_UTILBIN_USAGE, argv0);
   fprintf(stderr, "%s\n", MSG_DBSTAT_COMMANDINTRO1);
   fprintf(stderr, "%s\n", MSG_DBSTAT_COMMANDINTRO2);
   fprintf(stderr, "%s\n", MSG_DBSTAT_LIST);
   fprintf(stderr, "%s\n", MSG_DBSTAT_DUMP);
   fprintf(stderr, "%s\n", MSG_DBSTAT_LOAD);
   fprintf(stderr, "%s\n", MSG_DBSTAT_DELETE);
}

static int 
init_framework(sge_gdi_ctx_class_t *ctx, bdb_info *info)
{
   int ret = EXIT_FAILURE;

   lList *answer_list = NULL;
   lListElem *spooling_context = NULL;
   const char *spooling_method = ctx->get_spooling_method(ctx);
   const char *spooling_lib = ctx->get_spooling_lib(ctx);
   const char *spooling_params = ctx->get_spooling_params(ctx);

   DENTER(TOP_LAYER, "init_framework");

#ifdef HP1164   
   sge_set_admin_username("none", NULL, 0);
#endif

   /* create spooling context */
   spooling_context = spool_create_dynamic_context(&answer_list, 
                                                   spooling_method,
                                                   spooling_lib, 
                                                   spooling_params);
   answer_list_output(&answer_list);
   if (!strcmp(bootstrap_get_spooling_method(),"classic")) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_SPOOLDEFAULTS_CANTHANDLECLASSICSPOOLING));
   } else if (spooling_context == NULL) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_SPOOLDEFAULTS_CANNOTCREATECONTEXT));
   } else {
      spool_set_default_context(spooling_context);
      spool_set_option(&answer_list, spooling_context, "recover=false");
      answer_list_output(&answer_list);

      /* initialize spooling context */
      if (!spool_startup_context(&answer_list, spooling_context, true)) {
         CRITICAL((SGE_EVENT, SFNMAX, MSG_SPOOLDEFAULTS_CANNOTSTARTUPCONTEXT));
      } else {
         /* search the berkeley db info - take it from any object type, 
          * berkeleydb spools all objects using the same rule.
          */
         lListElem *type = spool_context_search_type(spooling_context, 
                                                     SGE_TYPE_JOB);
         lListElem *rule = spool_type_search_default_rule(type);
         *info = (bdb_info)lGetRef(rule, SPR_clientdata);
         ret = EXIT_SUCCESS;
      }
      answer_list_output(&answer_list);
   }  

   DRETURN(ret);
}

static bdb_database
get_database_from_key(const char *key)
{
   bdb_database database = BDB_CONFIG_DB;

   if (key != NULL) {
      if (strncmp(key, "J", 1) == 0 ||
          strncmp(key, "PET", 3) == 0) {
         database = BDB_JOB_DB;
      }
   }
   
   return database;
}

static const lDescr *
get_descr_from_key(const char *key) 
{
   const lDescr *descr = NULL;

   DENTER(TOP_LAYER, "get_descr_from_key");

   if (key != NULL) {
      struct saved_vars_s *context = NULL;
      const char *type_name;

      type_name = sge_strtok_r(key, ":", &context);
      if (type_name != NULL) {
         sge_object_type type = object_name_get_type(type_name);
         if (type != SGE_TYPE_ALL) {
            descr = object_type_get_descr(type);
         }
      }

      sge_free_saved_vars(context);

      if (descr == NULL) {
         ERROR((SGE_EVENT, MSG_DBSTAT_INVALIDKEY_S, key));
      }
   }

   DRETURN(descr);
}

static int 
list_objects(bdb_info info, const char *key) 
{
   int   ret = EXIT_SUCCESS;
   bool  dbret = false;
   lList *answer_list = NULL;
   lList *stu_list = NULL;
   bdb_database database;

   DENTER(TOP_LAYER, "list_objects");

   database = get_database_from_key(key);

   /* 
    * Start transaction to sync with other transaction-protected
    * cursor operations possibly launched from qmaster and friends. 
    */ 
   dbret = spool_berkeleydb_start_transaction(&answer_list, info);
   if (dbret) {
      /*
       * Transaction started; read the list
       */
      dbret = spool_berkeleydb_read_keys(&answer_list, info, database,
                                         &stu_list, key);
   } else {
      answer_list_output(&answer_list);
      ret = EXIT_FAILURE;
   }

   if (dbret) {
      if (strlen(key) == 0) {
         /* 
          * If no key was given, read the job database either.
          */
         dbret = spool_berkeleydb_read_keys(&answer_list, info, BDB_JOB_DB,
                                            &stu_list, key);
      }
   } else {
      answer_list_output(&answer_list);
      ret = EXIT_FAILURE;
   }

   if (dbret) {
      /*
       * Sucess: print out data.
       */
      const lListElem *stu_elem = NULL;
      for_each(stu_elem, stu_list) {
         fprintf(stdout, "%s\n", lGetString(stu_elem, STU_name));
      }
   } else {
      answer_list_output(&answer_list);
      ret = EXIT_FAILURE;
   }

   /*
    * Done. Close transaction.
    */
   dbret = spool_berkeleydb_end_transaction(&answer_list, info,
                                            (bool)(ret == EXIT_SUCCESS));
   if (!dbret) {
      answer_list_output(&answer_list);
      ret = EXIT_FAILURE;
   }

   lFreeList(&stu_list);

   DRETURN(ret);
}

static int 
dump_object(bdb_info info, const char *key) 
{
   int ret = EXIT_SUCCESS;
   bool dbret;
   lList *answer_list = NULL;
   bdb_database database;

   DENTER(TOP_LAYER, "dump_object");

   database = get_database_from_key(key);

   /* start a transaction */
   dbret = spool_berkeleydb_start_transaction(&answer_list, info);
   if (!dbret) {
      answer_list_output(&answer_list);
      ret = EXIT_FAILURE;
   } else {
      /* job script is spooled as string, not as cull object */
      if (strncmp(key, "JOBSCRIPT:", 10) == 0) {
         const char *job_script;
         job_script = spool_berkeleydb_read_string(&answer_list, info, database, key);
         if (job_script == NULL) {
            answer_list_output(&answer_list);
            ret = EXIT_FAILURE;
         } else {
            /* dump job script with a trailing linefeed, it might be missing in the script */
            printf("%s\n", job_script != NULL ? job_script : "no job script");
            sge_free(&job_script);
         }
      } else {
         /* read object */
         lListElem *object;
         object = spool_berkeleydb_read_object(&answer_list, info, database, key);
         if (object == NULL) {
            answer_list_output(&answer_list);
            ret = EXIT_FAILURE;
         } else {
            lDumpElemFp(stdout, object, 0);
            lFreeElem(&object);
         }
      }
   }

   /* close the transaction */
   dbret = spool_berkeleydb_end_transaction(&answer_list, info, 
                                            (ret == EXIT_SUCCESS) ? true : false);
   if (!dbret) {
      answer_list_output(&answer_list);
      ret = EXIT_FAILURE;
   }

   DRETURN(ret);
}

static int 
load_object(bdb_info info, const char *key, const char *fname) 
{
   int ret = EXIT_SUCCESS;
   bool dbret;
   lList *answer_list = NULL;
   bdb_database database;
   const lDescr *descr;
   lListElem *object = NULL;

   DENTER(TOP_LAYER, "load_object");

   database = get_database_from_key(key);
   descr    = get_descr_from_key(key);

   if (descr == NULL) {
      ret = EXIT_FAILURE;
   }

   if (ret == EXIT_SUCCESS) {
      FILE *fd;

      if((fd = fopen(fname, "r")) == NULL) {
         ERROR((SGE_EVENT, MSG_ERROROPENINGFILEFORREADING_SS, fname, strerror(errno)));
         ret = EXIT_FAILURE;
      } else {
         object = lUndumpElemFp(fd, descr);
         FCLOSE(fd);
         if (object == NULL) {
            ERROR((SGE_EVENT, MSG_DBSTAT_ERRORUNDUMPING_S, fname));
            ret = EXIT_FAILURE;
         }
      }
   }

   if (object != NULL) {
      /* start a transaction */
      dbret = spool_berkeleydb_start_transaction(&answer_list, info);
      if (!dbret) {
         answer_list_output(&answer_list);
         ret = EXIT_FAILURE;
      } else {
         /* read object */
         dbret = spool_berkeleydb_write_object(&answer_list, info, database, object, key);
         if (!dbret) {
            answer_list_output(&answer_list);
            ret = EXIT_FAILURE;
         }

         /* close the transaction */
         dbret = spool_berkeleydb_end_transaction(&answer_list, info, 
                                                  (ret == EXIT_SUCCESS) ? true : false);
         if (!dbret) {
            answer_list_output(&answer_list);
            ret = EXIT_FAILURE;
         }
      }
   }

   lFreeElem(&object);

   DRETURN(ret);

FCLOSE_ERROR:
   ERROR((SGE_EVENT, MSG_FILE_NOCLOSE_SS, fname, strerror(errno)));
   DRETURN(EXIT_FAILURE);
}

static int 
delete_object( bdb_info info, const char *key) 
{
   int ret = EXIT_SUCCESS;
   bool dbret;
   lList *answer_list = NULL;
   bdb_database database;

   DENTER(TOP_LAYER, "delete_object");

   database = get_database_from_key(key);

   /* start a transaction */
   dbret = spool_berkeleydb_start_transaction(&answer_list, info);
   if (!dbret) {
      answer_list_output(&answer_list);
      ret = EXIT_FAILURE;
   } else {
      /* delete object with given key */
      dbret = spool_berkeleydb_delete_object(&answer_list, info, database, 
                                             key, false);
      if (!dbret) {
         answer_list_output(&answer_list);
         ret = EXIT_FAILURE;
      } else {
         fprintf(stdout, "deleted object with key "SFQ"\n", key);
      }
   }

   /* close the transaction */
   dbret = spool_berkeleydb_end_transaction(&answer_list, info, 
                                            (ret == EXIT_SUCCESS) ? true : false);
   if (!dbret) {
      answer_list_output(&answer_list);
      ret = EXIT_FAILURE;
   }

   DRETURN(ret);
}


int 
main(int argc, char *argv[])
{
   int ret = EXIT_SUCCESS;
   lList *answer_list = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;

   DENTER_MAIN(TOP_LAYER, "spooledit");

   if (sge_setup2(&ctx, SPOOLDEFAULTS, MAIN_THREAD, &answer_list, false) != AE_OK) {
      answer_list_output(&answer_list);
      SGE_EXIT((void**)&ctx, 1);
   }

   if (ret == EXIT_SUCCESS) {
      /* parse commandline */
      if (argc < 2 || (strcmp(argv[1], "-help") == 0)) {
         usage(argv[0]);
         ret = EXIT_FAILURE;
      } else {
         bdb_info info = NULL;
         ret = init_framework(ctx, &info);

         if (ret == EXIT_SUCCESS) {
            if (strcmp(argv[1], "list") == 0) {
               ret = list_objects(info, argc > 2 ? argv[2] : "");
            } else if (strcmp(argv[1], "dump") == 0) {
               if (argc < 3) {
                  usage(argv[0]);
                  ret = EXIT_FAILURE;
               } else {
                  ret = dump_object(info, argv[2]);
               }
            } else if (strcmp(argv[1], "load") == 0) {
               if (argc < 4) {
                  usage(argv[0]);
                  ret = EXIT_FAILURE;
               } else {
                  ret = load_object(info, argv[2], argv[3]);
               }
            } else if (strcmp(argv[1], "delete") == 0) {
               if (argc < 3) {
                  usage(argv[0]);
                  ret = EXIT_FAILURE;
               } else {
                  ret = delete_object(info, argv[2]);
               }
            } else {
               usage(argv[0]);
               ret = EXIT_FAILURE;
            }
         }
      }
   }

   if (spool_get_default_context() != NULL) {
#if 0
      time_t next_trigger = 0;
      if (!spool_trigger_context(&answer_list, spool_get_default_context(), 
                                 0, &next_trigger)) {
         ret = EXIT_FAILURE;
      }
#endif
      if (!spool_shutdown_context(&answer_list, spool_get_default_context())) {
         ret = EXIT_FAILURE;
      }
   }

   answer_list_output(&answer_list);

   SGE_EXIT((void**)&ctx, ret);

   DRETURN(ret);
}
