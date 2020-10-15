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

#include "uti/sge_rmon.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_profiling.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_dstring.h"
#include "uti/setup_path.h"
#include "uti/sge_prog.h"

#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_all_listsL.h"

#include "spool/sge_spooling.h"
#include "spool/loader/sge_spooling_loader.h"
#include "spool/flatfile/sge_spooling_flatfile.h"
#include "spool/flatfile/sge_flatfile_obj.h"
#include "spool/flatfile/sge_flatfile.h"
#include "spool/sge_dirent.h"

#include "gdi/sge_gdi_ctx.h"

#include "sge.h"
#include "msg_utilbin.h"

static int spool_object_list(const char *directory,
                             const spooling_field *fields, 
                             const spool_flatfile_instr *instr,
                             const lDescr *descr,
                             sge_object_type obj_type); 

static void usage(const char *argv0)
{
   fprintf(stderr, "%s\n %s command\n\n", MSG_UTILBIN_USAGE, argv0);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_COMMANDINTRO1);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_COMMANDINTRO2);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_TEST);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_ADMINHOSTS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_CALENDARS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_CKPTS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_COMPLEXES);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_CONFIGURATION);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_CQUEUES);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_EXECHOSTS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_LOCAL_CONF);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_MANAGERS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_OPERATORS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_PES);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_PROJECTS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_SHARETREE);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_SUBMITHOSTS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_USERS);
   fprintf(stderr, "%s\n", MSG_SPOOLDEFAULTS_USERSETS);
}

static int init_framework(sge_gdi_ctx_class_t *ctx)
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
   if (spooling_context == NULL) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_SPOOLDEFAULTS_CANNOTCREATECONTEXT));
   } else {
      spool_set_default_context(spooling_context);

      /* initialize spooling context */
      if (!spool_startup_context(&answer_list, spooling_context, true)) {
         CRITICAL((SGE_EVENT, SFNMAX, MSG_SPOOLDEFAULTS_CANNOTSTARTUPCONTEXT));
      } else {
         ret = EXIT_SUCCESS;
      }
      answer_list_output(&answer_list);
   }

   DRETURN(ret);
}

static int spool_manops(sge_object_type type, int argc, char *argv[])
{
   int ret = EXIT_SUCCESS;
   int i;
   lList *answer_list = NULL;
   lList **lpp = object_type_get_master_list(type);
   int key = object_type_get_key_nm(type);
   const lDescr *descr = object_type_get_descr(type);

   DENTER(TOP_LAYER, "spool_manops");

   if (*lpp == NULL) {
      *lpp = lCreateList("master list", descr);
   }

   for (i = 2; i < argc; i++) {
      const char *name = argv[i];
      lListElem *ep = lAddElemStr(lpp, key, name, descr);

      if (!spool_write_object(&answer_list, spool_get_default_context(),
                              ep, name, type, true)) {
         /* error output has been done in spooling function */
         answer_list_output(&answer_list);
         ret = EXIT_FAILURE;
      }
   }

   DRETURN(ret);
}

static int spool_configuration(int argc, char *argv[])
{
   int ret = EXIT_SUCCESS;
   lListElem *conf = NULL;
   lList *answer_list = NULL;
   spooling_field *fields = sge_build_CONF_field_list(true);

   DENTER(TOP_LAYER, "spool_configuration");

   conf = spool_flatfile_read_object(&answer_list, CONF_Type, NULL,
                                   fields, NULL, false, &qconf_sfi,
                                   SP_FORM_ASCII, NULL, argv[2]);
   sge_free(&fields);
   if (conf == NULL) {
      ERROR((SGE_EVENT, MSG_SPOOLDEFAULTS_CANTREADGLOBALCONF_S, argv[2]));
      ret = EXIT_FAILURE;
   } else {
      /* put config into a list - we can't spool free objects */
      lSetHost(conf, CONF_name, SGE_GLOBAL_NAME);
      if (!spool_write_object(&answer_list, spool_get_default_context(), conf, SGE_GLOBAL_NAME, SGE_TYPE_CONFIG, true)) {
         /* error output has been done in spooling function */
         ret = EXIT_FAILURE;
      }
      answer_list_output(&answer_list);
   }
   lFreeElem(&conf);

   DRETURN(ret);
}

static int spool_local_conf(int argc, char *argv[])
{
   int ret = EXIT_SUCCESS;

   DENTER(TOP_LAYER, "spool_local_conf");

   /* we get an additional argument: the config name */
   if (argc < 4) {
      usage(argv[0]);
      ret = EXIT_FAILURE;
   } else {
      lList *answer_list = NULL;
      spooling_field *fields = sge_build_CONF_field_list(true);
      lListElem *conf = NULL;

      conf = spool_flatfile_read_object(&answer_list, CONF_Type, NULL,
                                   fields, NULL, false, &qconf_sfi,
                                   SP_FORM_ASCII, NULL, argv[2]);
      sge_free(&fields);

      if (conf == NULL) {
         ERROR((SGE_EVENT, MSG_SPOOLDEFAULTS_CANTREADLOCALCONF_S, argv[2]));
         ret = EXIT_FAILURE;
      } else {
         /* check if a config is already there */
         lListElem *le = spool_read_object(NULL, spool_get_default_context(), SGE_TYPE_CONFIG, argv[3]); 
         if (le == NULL) { 
            /* put config into a list - we can't spool free objects */
            lSetHost(conf, CONF_name, argv[3]);
            if (!spool_write_object(&answer_list, spool_get_default_context(), 
                                 conf, argv[3], SGE_TYPE_CONFIG, true)) {
               /* error output has been done in spooling function */
               ret = EXIT_FAILURE;
            } 
         } else {
            fprintf(stderr, "config already there!\n");
         }
         answer_list_output(&answer_list);
         lFreeElem(&conf);
      }
      lFreeList(&answer_list);
   }

   DRETURN(ret);
}

static int spool_sharetree(int argc, char *argv[])
{
   const spooling_field *fields = sge_build_STN_field_list(true, false);
   int ret; 

   ret = spool_object_list(argv[2], fields, &qconf_sfi, STN_Type, SGE_TYPE_SHARETREE);
   sge_free(&fields);
   return ret;
}

static int spool_complexes(int argc, char *argv[])
{
   return spool_object_list(argv[2], CE_fields, &qconf_sfi, CE_Type, SGE_TYPE_CENTRY);
}

static int spool_adminhosts(int argc, char *argv[])
{
   return spool_object_list(argv[2], AH_fields, &qconf_sfi, AH_Type, SGE_TYPE_ADMINHOST);
}

static int spool_calendars(int argc, char *argv[])
{
   return spool_object_list(argv[2], CAL_fields, &qconf_sfi, CAL_Type, SGE_TYPE_CALENDAR);
}

static int spool_ckpts(int argc, char *argv[])
{
   return spool_object_list(argv[2], CK_fields, &qconf_sfi, CK_Type, SGE_TYPE_CKPT);
}

static int spool_cqueues(int argc, char *argv[])
{
   lList *answer_list = NULL;
   spool_read_list(&answer_list, spool_get_default_context(), 
                   object_type_get_master_list(SGE_TYPE_CENTRY), 
                   SGE_TYPE_CENTRY);
   spool_read_list(&answer_list, spool_get_default_context(), 
                   object_type_get_master_list(SGE_TYPE_EXECHOST), 
                   SGE_TYPE_EXECHOST);
   answer_list_output(&answer_list);

   return spool_object_list(argv[2], CQ_fields, &qconf_sfi, CQ_Type, SGE_TYPE_CQUEUE);
}

static int spool_exechosts(int argc, char *argv[])
{
   lList *answer_list = NULL;
   const spooling_field *fields = sge_build_EH_field_list(true, false, false);
   int ret; 

   spool_read_list(&answer_list, spool_get_default_context(), 
                   object_type_get_master_list(SGE_TYPE_CENTRY), 
                   SGE_TYPE_CENTRY);
   answer_list_output(&answer_list);

   ret = spool_object_list(argv[2], fields, &qconf_sfi, EH_Type, SGE_TYPE_EXECHOST);
   sge_free(&fields);
   return ret;
}

static int spool_projects(int argc, char *argv[])
{
   const spooling_field *fields = sge_build_PR_field_list(true);
   int ret; 

   ret = spool_object_list(argv[2], fields, &qconf_sfi, PR_Type, SGE_TYPE_PROJECT);
   sge_free(&fields);
   return ret;
}

static int spool_submithosts(int argc, char *argv[])
{
   return spool_object_list(argv[2], SH_fields, &qconf_sfi, SH_Type, SGE_TYPE_SUBMITHOST);
}

static int spool_users(int argc, char *argv[])
{
   const spooling_field *fields = sge_build_UU_field_list(true);
   int ret; 

   ret = spool_object_list(argv[2], fields, &qconf_sfi, UU_Type, SGE_TYPE_PROJECT);
   sge_free(&fields);
   return ret;
}

static int spool_pes(int argc, char *argv[])
{
   return spool_object_list(argv[2], PE_fields, &qconf_sfi, PE_Type, SGE_TYPE_PE);
}

static int spool_usersets(int argc, char *argv[])
{
   return spool_object_list(argv[2], US_fields, &qconf_sfi, US_Type, SGE_TYPE_USERSET);
}

static int spool_object_list(const char *directory,
                             const spooling_field *fields, 
                             const spool_flatfile_instr *instr,
                             const lDescr *descr,
                             sge_object_type obj_type)
{
   int ret = EXIT_SUCCESS;
   lList *answer_list = NULL;
   lListElem *ep;
   lList *direntries;
   lListElem *direntry;
   const char *name;
   dstring file = DSTRING_INIT;

   DENTER(TOP_LAYER, "spool_object_list");

   direntries = sge_get_dirents(directory);
   for_each(direntry, direntries) {
      name = lGetString(direntry, ST_name);
      if (name[0] != '.') {
         sge_dstring_sprintf(&file, "%s/%s", directory, name);
         ep = spool_flatfile_read_object(&answer_list, descr, NULL,
                                         fields, NULL, true, instr,
                                         SP_FORM_ASCII, NULL, sge_dstring_get_string(&file));
         
         if (ep != NULL && !spool_write_object(&answer_list, spool_get_default_context(), ep, 
                                 name, obj_type, true)) {
            /* error output has been done in spooling function */
            ret = EXIT_FAILURE;
            answer_list_output(&answer_list);
            break;
         }
         lFreeElem(&ep);
      }
   }
   lFreeList(&direntries);
   sge_dstring_free(&file);

   lFreeList(&answer_list);

   DRETURN(ret);
}

int main(int argc, char *argv[])
{
   int ret = EXIT_SUCCESS;
   lList *answer_list = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;

   DENTER_MAIN(TOP_LAYER, "spooldefaults");

   log_state_set_log_gui(0);
   prof_mt_init();

   if (sge_setup2(&ctx, SPOOLDEFAULTS, MAIN_THREAD, &answer_list, false) != AE_OK) {
      show_answer(answer_list);
      lFreeList(&answer_list);
      SGE_EXIT((void **)&ctx, EXIT_FAILURE);
   }

   /* parse commandline */
   if (argc < 2) {
      usage(argv[0]);
      ret = EXIT_FAILURE;
   } else {
      ret = init_framework(ctx);

      if (ret == EXIT_SUCCESS) {
         if (strcmp(argv[1], "test") == 0) {
            /* nothing to do - init_framework succeeded */
         } else {
            /* all other commands have at least one parameter */
            if (argc < 3) {
               usage(argv[0]);
               ret = EXIT_FAILURE;
            } else if (strcmp(argv[1], "adminhosts") == 0) {
               ret = spool_adminhosts(argc, argv);
            } else if (strcmp(argv[1], "calendars") == 0) {
               ret = spool_calendars(argc, argv);
            } else if (strcmp(argv[1], "ckpts") == 0) {
               ret = spool_ckpts(argc, argv);
            } else if (strcmp(argv[1], "complexes") == 0) {
               ret = spool_complexes(argc, argv);
            } else if (strcmp(argv[1], "configuration") == 0) {
               ret = spool_configuration(argc, argv);
            } else if (strcmp(argv[1], "cqueues") == 0) {
               ret = spool_cqueues(argc, argv);
            } else if (strcmp(argv[1], "exechosts") == 0) {
               ret = spool_exechosts(argc, argv);
            } else if (strcmp(argv[1], "local_conf") == 0) {
               ret = spool_local_conf(argc, argv);
            } else if (strcmp(argv[1], "managers") == 0) {
               ret = spool_manops(SGE_TYPE_MANAGER, argc, argv);
            } else if (strcmp(argv[1], "operators") == 0) {
               ret = spool_manops(SGE_TYPE_OPERATOR, argc, argv);
            } else if (strcmp(argv[1], "pes") == 0) {
               ret = spool_pes(argc, argv);
            } else if (strcmp(argv[1], "projects") == 0) {
               ret = spool_projects(argc, argv);
            } else if (strcmp(argv[1], "sharetree") == 0) {
               ret = spool_sharetree(argc, argv);
            } else if (strcmp(argv[1], "submithosts") == 0) {
               ret = spool_submithosts(argc, argv);
            } else if (strcmp(argv[1], "users") == 0) {
               ret = spool_users(argc, argv);
            } else if (strcmp(argv[1], "usersets") == 0) {
               ret = spool_usersets(argc, argv);
            } else {
               usage(argv[0]);
               ret = EXIT_FAILURE;
            }
         }
      }
   }

   if ((ret != EXIT_FAILURE) && (spool_get_default_context() != NULL)) {
      time_t next_trigger = 0;

      if (!spool_trigger_context(&answer_list, spool_get_default_context(), 
                                 0, &next_trigger)) {
         ret = EXIT_FAILURE;
      }
      if (!spool_shutdown_context(&answer_list, spool_get_default_context())) {
         ret = EXIT_FAILURE;
      }
   }

   answer_list_output(&answer_list);

   sge_prof_cleanup();

   SGE_EXIT((void **)&ctx, ret);

   DRETURN(ret);
}
