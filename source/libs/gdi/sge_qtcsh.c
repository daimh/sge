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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <pthread.h>

#include "uti/sge_mtutil.h"
#include "uti/sge_rmon.h"
#include "uti/setup_path.h"
#include "uti/sge_log.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_prog.h"
#include "uti/sge_stdio.h"
#include "uti/sge_string.h"
#include "uti/sge_parse_args.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_config.h"
#include "sgeobj/sge_conf.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_qtcsh.h"

#include "msg_common.h"


/* module global variables */
static pthread_mutex_t qtask_mutex = PTHREAD_MUTEX_INITIALIZER;
static lList *task_config = NULL;
static int mode_verbose = 0;
static int mode_remote = 1;
static int force_remote = 0;
static int mode_immediate = 1;

static int init_qtask_config(sge_gdi_ctx_class_t *ctx, lList **alpp, print_func_t ostream);

/****** sge_qtcsh/init_qtask_config() ******************************************
*  NAME
*     init_qtask_config() -- ??? 
*
*  SYNOPSIS
*     static int init_qtask_config(lList **alpp, print_func_t ostream) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx        - ???
*     lList **alpp         - ??? 
*     print_func_t ostream - ??? 
*
*  RESULT
*     static int - 0 - success
*                 -1 - failed
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     MT-NOTES: init_qtask_config() is not MT safe as it uses global variables
*
*  BUGS
*     ??? 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static int init_qtask_config(
sge_gdi_ctx_class_t *ctx,
lList **alpp,
print_func_t ostream 
) {
   struct passwd *pwd;
   char fname[SGE_PATH_MAX];
   char buffer[10000];
   FILE *fp;
   lList *clp_cluster = NULL, *clp_user = NULL;
   lListElem *nxt, *cep_dest, *cep, *next;
   const char *task_name;
   struct passwd pw_struct;
   char *pw_buffer;
   size_t pw_buffer_size;
   const char* user_name = ctx->get_username(ctx);
   const char* cell_root = ctx->get_cell_root(ctx);

   /* cell global settings */
   snprintf(fname, sizeof(fname), "%s/common/qtask", cell_root);

   if (!(fp = fopen(fname, "r")) && errno != ENOENT) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_FILE_ERROROPENINGXY_SS, fname, strerror(errno)));
      answer_list_add(alpp, SGE_EVENT, STATUS_EDISK, ANSWER_QUALITY_ERROR);
      (*ostream)("%s", SGE_EVENT);
      goto Error;
   }
   if (fp) {
      /* read in config file */
      if (read_config_list(fp, &clp_cluster, alpp, CF_Type, CF_name, CF_value,
                        CF_sublist, NULL, RCL_NO_VALUE, buffer, sizeof(buffer)-1)) {
         FCLOSE(fp);
         goto Error;
      }
      FCLOSE(fp);
   }

   /* skip tasknames containing '/' */
   nxt = lFirst(clp_cluster);
   while ((cep=nxt)) {
      nxt = lNext(cep);
      if (strrchr(lGetString(cep, CF_name), '/')) 
         lRemoveElem(clp_cluster, &cep);

   }

   pw_buffer_size = get_pw_buffer_size();
   pw_buffer = sge_malloc(pw_buffer_size);
   pwd = sge_getpwnam_r(user_name, &pw_struct, pw_buffer, pw_buffer_size);
   
   /* user settings */
   if (pwd == NULL) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_USER_INVALIDNAMEX_S , user_name));
      answer_list_add(alpp, SGE_EVENT, STATUS_ENOSUCHUSER, ANSWER_QUALITY_ERROR);
      (*ostream)("%s", SGE_EVENT);
      goto Error;
   }
   if (!pwd->pw_dir) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_USER_NOHOMEDIRFORUSERX_S , user_name));
      answer_list_add(alpp, SGE_EVENT, STATUS_EDISK, ANSWER_QUALITY_ERROR);
      (*ostream)("%s", SGE_EVENT);
      goto Error;
   }
   snprintf(fname, sizeof(fname), "%s/.qtask", pwd->pw_dir);
   
   if (!(fp = fopen(fname, "r")) && errno != ENOENT) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_FILE_ERROROPENINGXY_SS, fname, strerror(errno)));
      answer_list_add(alpp, SGE_EVENT, STATUS_EDISK, ANSWER_QUALITY_ERROR);
      (*ostream)("%s", SGE_EVENT);
      goto Error;
   }
   if (fp) {
      /* read in config file */
      if (read_config_list(fp, &clp_user, alpp, CF_Type, CF_name, CF_value,
                           CF_sublist, NULL, RCL_NO_VALUE, buffer, sizeof(buffer)-1)) {
         FCLOSE(fp);
         goto Error;
      }
      FCLOSE(fp);
   }

   /* skip tasknames containing '/' */
   nxt = lFirst(clp_user);
   while ((cep=nxt)) {
      nxt = lNext(cep);
      if (strrchr(lGetString(cep, CF_name), '/')) 
         lRemoveElem(clp_user, &cep);
   }

#if 0
   if (clp_user)
      for_each (cep, clp_user) {
         (*ostream) ("info: user: command %s request %s\n", lGetString(cep, CF_name), (s=lGetString(cep, CF_value))?s:"");
      }
   else 
      (*ostream) ("info: empty user task list\n");
#endif

   /* merge contents of user list into cluster list */
   next = lFirst(clp_user);
   while ((cep=next)) {
      char *ro_task_name;
      next = lNext(cep);
      task_name = lGetString(cep, CF_name);
   
      /* build task name with leading '!' for search operation */
      ro_task_name = sge_malloc(strlen(task_name) + 2);
      ro_task_name[0] = '!';
      strcpy(&ro_task_name[1], task_name); /* RATS: ignore */

      if ((cep_dest=lGetElemStr(clp_cluster, CF_name, ro_task_name))) {
         /* do not override cluster global task entry */
         lRemoveElem(clp_user, &cep);
      } else if ((cep_dest=lGetElemStr(clp_cluster, CF_name, task_name))) {
         /* override cluster global task entry */
         lSetString(cep_dest, CF_value, lGetString(cep, CF_value));
         lRemoveElem(clp_user, &cep);
      } else {
         /* no entry in cluster global task list 
            use entry from user task list */
         lDechainElem(clp_user, cep);
         if (!clp_cluster) 
            clp_cluster = lCreateList("cluster config", CF_Type);
         lAppendElem(clp_cluster, cep); 
      }

      sge_free(&ro_task_name);
   }
   lFreeList(&clp_user);

   
   lFreeList(&task_config);
   task_config = clp_cluster;

   /* remove leading '!' from command names */
   for_each (cep, clp_cluster) {
      task_name = lGetString(cep, CF_name);
      if (task_name[0] == '!') {
         char *t = sge_malloc(strlen(task_name));
         strcpy(t, &task_name[1]); /* RATS: ignore */
         lSetString(cep, CF_name, t);
         sge_free(&t);
      }
   }


#if 0
   if (task_config)
      for_each (cep, task_config) {
         (*ostream) ("info: session: command %s request %s\n", lGetString(cep, CF_name), (s=lGetString(cep, CF_value))?s:"");
      }
   else 
      (*ostream) ("info: empty task list\n");
#endif

   sge_free(&pw_buffer);
   return 0;

FCLOSE_ERROR:
Error:
   lFreeList(&clp_cluster);
   lFreeList(&clp_user);
   sge_free(&pw_buffer);
   return -1;
}

int sge_execv(
char *path,    /* this is how tcsh tries to start the command */
char *argv[],
char *expath,  /* this is how user typed in the command */
int close_stdin /* use of qrsh's -nostdin option */
) {
   const char *value; 
   char *taskname = NULL;
   lListElem *task = NULL;
   int i = 0;
   int narg_resreq = 0;
   int narg_argv = 0;
   int newargv_size = 0;
   char **argv_iter = NULL;
   char **newargv = NULL;
   /* TODO: This should be SGE_PATH_MAX. */
   char qrsh_path[SGE_PATH_MAX];

   /* remote execution only for commands without any path information */
   if (!strchr(expath, '/')) {
      taskname = expath;
   }

   if (mode_verbose) {
      fprintf(stderr, "sge_execv(path = %s, taskname = %s, expath = %s, close_stdin = %d)\n", 
         path, taskname?taskname:"<no remote execution>", expath, close_stdin);
   }

   if (!mode_remote || 
         !taskname ||
         !(task=lGetElemStr(task_config, CF_name, taskname))) {
      if (mode_verbose)
         fprintf(stderr, "local execution of "SFQ"\n", expath);
      return execv(path, argv);
   }
  
   if ((value = lGetString(task, CF_value))) {
      narg_resreq = sge_quick_count_num_args (value);
   }

   for (argv_iter=argv; argv_iter[0] != NULL; argv_iter++) {
      narg_argv++; 
   }
   
   newargv_size =
      1 +                                /* qrsh */
      (close_stdin?1:0) +                /* -nostdin */
      (mode_verbose?1:0) +               /* -verbose */
      2 +                                /* -now [y|n] */
      narg_resreq +                      /* resource requests to qrsh */
      narg_argv +                        /* argv of command to be started */
      1;                                 /* NULL */
   newargv = sge_malloc(sizeof(char *) * newargv_size);
   memset(newargv, 0, newargv_size);

   /* build argv for qrsh */
   i = 0;
   newargv[i++] = strdup("qrsh");

   if (close_stdin) 
      newargv[i++] = strdup("-nostdin");

   if (mode_verbose) 
      newargv[i++] = strdup("-verbose");

   if (mode_immediate) {
      newargv[i++] = strdup("-now");
      newargv[i++] = strdup("y");
   } else {
      newargv[i++] = strdup("-now");
      newargv[i++] = strdup("n");
   }

   /* add optional qrsh arguments from qtask file */
   if (value) {
      sge_parse_args (value, &newargv[i]);
      i += narg_resreq;
   }
	 
   /* add command's arguments */
   for (argv_iter=argv; argv_iter[0] != NULL; argv_iter++) {
      newargv[i++] = argv_iter[0];
   }

   newargv[i] = NULL;
      
   snprintf(qrsh_path, sizeof(qrsh_path), "%s/bin/%s/qrsh",
            sge_get_root_dir(1, NULL, 0, 1), sge_get_arch());

   return execvp(qrsh_path, newargv);
}

/****** QTCSH/sge_get_qtask_args() *********************************************
*  NAME
*     sge_get_qtask_args() -- get args for a qtask entry
*
*  SYNOPSIS
*     char** sge_get_qtask_args(void *ctx, char *taskname, lList **answer_list)
*
*  FUNCTION
*     This function reads the qtask files and returns an array of args for the
*     given qtask entry.  Calling this function will initialize the qtask
*     framework, if it has not already been initialized.
*
*  INPUTS
*     void *ctx           - the communication context (sge_gdi_ctx_class_t *)
*     char *taskname      - The name of the entry for which to look in the qtask
*                           files
*     lList **answer_list - For returning error information
*
*  RESULT
*     char **           A NULL-terminated array of args for the given qtask
*                       entry
*
*  NOTES
*     MT-NOTE: sge_get_qtask_args() is MT safe with respect to itself, but it is
*              not thread safe to use this function in conjuction with the
*              init_qtask_config() or sge_init() functions or accessing the
*              task_config global variable.
*
*******************************************************************************/
char **sge_get_qtask_args(void *context, char *taskname, lList **answer_list)
{
   const char *value = NULL; 
   int num_args = 0;
   lListElem *task = NULL;
   char** args = NULL;
   sge_gdi_ctx_class_t *ctx = (sge_gdi_ctx_class_t *)context;
   
   DENTER (TOP_LAYER, "sge_get_qtask_args");
   
   if (mode_verbose) {
      fprintf(stderr, "sge_get_qtask_args(taskname = %s)\n", taskname);
   }

   /* If the task_config has not been filled yet, fill it.  We call
    * init_qtask_config() instead of sge_init() because we don't need to setup
    * the GDI.  We just need the qtask arguments. */
   /* We lock this part because multi-threaded DRMAA apps can have problems
    * here.  Once we're past this part, qtask_config's read-only, so we don't
    * have any more problems. */
   sge_mutex_lock("qtask_mutex", SGE_FUNC, __LINE__, &qtask_mutex);
    
   if (task_config == NULL) {
      /* Just using printf here since we don't really have an exciting function
       * like xprintf to pass in.  This was really meant for use with qtsch. */
      if (init_qtask_config(ctx, answer_list, (print_func_t)printf) != 0) {
         sge_mutex_unlock("qtask_mutex", SGE_FUNC, __LINE__, &qtask_mutex);
         DEXIT;
         return args;
      }
   }

   sge_mutex_unlock("qtask_mutex", SGE_FUNC, __LINE__, &qtask_mutex);
   
   task = lGetElemStr(task_config, CF_name, taskname);
   
   if (task == NULL) {
      DEXIT;
      return args;
   }
  
   value = lGetString(task, CF_value);
   
   if (value != NULL) {
      num_args = sge_quick_count_num_args(value);
   }
   
   args = sge_malloc(sizeof(char *) * (num_args + 1));
   memset(args, 0, sizeof(char *) * (num_args + 1));
   sge_parse_args (value, args);
   
   DEXIT;
   return args;
}

void sge_init(
print_func_t ostream 
) {
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;

   /* TODO:
    *  sge_gdi_param(SET_EXIT_ON_ERROR, 0, NULL);
    */
   if (sge_gdi2_setup(&ctx, QTCSH, MAIN_THREAD, NULL) == AE_OK) {
      if (init_qtask_config(ctx, &alp, ostream) != 0 ) {
         mode_remote = 0;          
      } else {
         /* Remote execution is default.

            Turn off remote execution only in case we were 
            started in the context of an already running job.
            This is done to prevent recursive 

              qrsh -> qtcsh -> qrsh -> qtcsh -> ...

            submission via SGE/SGE in case qtcsh
            is the login shell at the execution server.
          */
         if ( mode_remote != 0 ) {
            mode_remote = force_remote?mode_remote:!getenv("JOB_ID");          
         }
/*          (*ostream) ("mode_remote = %d\n", mode_remote); */
      }
      lFreeList(&alp);
   } else {
      mode_remote = 0;          
/*       (*ostream) ("no $SGE_ROOT, running as normal tcsh\n"); */
   }

   return;
}

void set_sgemode(
int addr,
int value 
) {
   switch (addr) {
   case CATCH_EXEC_MODE_REMOTE:
      mode_remote = value;
      break;
   case CATCH_EXEC_MODE_VERBOSE:
      mode_verbose = value;
      break;
   case CATCH_EXEC_MODE_IMMEDIATE:
      mode_immediate = value;
      break;
   case CATCH_EXEC_MODE_FORCE_REMOTE:
      force_remote = value;
      break;
   default:
      break;
  }      
  return;
}

int get_sgemode(
int addr 
) {
   int value = -1;

   switch (addr) {
   case CATCH_EXEC_MODE_REMOTE:
      value = mode_remote;
      break;
   case CATCH_EXEC_MODE_VERBOSE:
      value = mode_verbose;
      break;
   case CATCH_EXEC_MODE_IMMEDIATE:
      value = mode_immediate;
   default:
      break;
  }      
  return value;
}
