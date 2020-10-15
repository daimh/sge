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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_prog.h"
#include "uti/sge_unistd.h"
#include "uti/sge_profiling.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi2.h"
#include "gdi/sge_gdi_ctx.h"

#include "comm/commlib.h"

#include "sig_handlers.h"
#include "usage.h"
#include "sge_options.h"
#include "msg_common.h"
#include "msg_clients_common.h"
#include "msg_qdel.h"

static bool sge_parse_cmdline_qdel(char **argv, char **envp, lList **ppcmdline, lList **alpp);
static bool sge_parse_qdel(lList **ppcmdline, lList **ppreflist, u_long32 *pforce, lList **ppuserlist, lList **alpp);

extern char **environ;

/************************************************************************/
int main(int argc, char **argv) {
   /* lListElem *rep, *nxt_rep, *jep, *aep, *jrep, *idep; */
   int ret = 0;
   lListElem *aep, *idep;
   lList *jlp = NULL, *alp = NULL, *pcmdline = NULL, *ref_list = NULL, *user_list=NULL;
   u_long32 force = 0;
   int wait;
   unsigned long status = 0;
   bool have_master_privileges;
   cl_com_handle_t* handle = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;

   DENTER_MAIN(TOP_LAYER, "qdel");

   log_state_set_log_gui(1);

   if (sge_gdi2_setup(&ctx, QDEL, MAIN_THREAD, &alp) != AE_OK) {
      answer_list_output(&alp);
      goto error_exit;
   }

   if (!sge_parse_cmdline_qdel(++argv, environ, &pcmdline, &alp)) {
      /*
      ** high level parsing error! show answer list
      */
      answer_list_output(&alp);
      lFreeList(&pcmdline);
      goto error_exit;
   }

   if (!sge_parse_qdel(&pcmdline, &ref_list, &force, &user_list, &alp)) {
      /*
      ** low level parsing error! show answer list
      */
      answer_list_output(&alp);
      lFreeList(&pcmdline);
      goto error_exit;
   }

   DPRINTF(("force     = "sge_u32"\n", force));
   
   if (user_list) {
      lListElem *id;

      if (lGetNumberOfElem(ref_list) == 0){
         id = lAddElemStr(&ref_list, ID_str, "0", ID_Type);
         lSetList(id, ID_user_list, user_list);
      } else {
         for_each(id, ref_list){
            lSetList(id, ID_user_list, user_list);
         }
      }
   }

   /* TODO: remove this code from client, should be hidden in gdi layer 
   **       timeout value should be set in gdi_setup
   */
   handle=cl_com_get_handle(prognames[QDEL], 0);
   cl_com_set_synchron_receive_timeout(handle, 10*60);

   /* Are there jobs which should be deleted? */
   if (!ref_list) {
      sge_usage(QDEL, stderr);
      printf("%s\n", MSG_PARSE_NOOPTIONARGUMENT);
      goto error_exit;
   }

   /* Has the user the permission to use the the '-f' (forced) flag */
   have_master_privileges = false;
   if (force == 1) {
      have_master_privileges = sge_gdi2_check_permission(ctx, &alp, MANAGER_CHECK);
      lFreeList(&alp);
   }
   /* delete the job */
   {
      int delete_mode;

      /* 
       * delete_mode:
       *    1 => admin user used '-f'     
       *         -> forced deletion
       *    7 => non admin user used '-f' 
       *         -> first try normal deletion
       *         -> wait a minute
       *         -> forced deletion (delete_mode==5)
       *    3 => normal qdel
       *         -> normal deletion
       */
      if (force == 1) {
         if (have_master_privileges == true) {
            delete_mode = 1;
         } else {
            delete_mode = 7;
         }
      } else {
         delete_mode = 3;
      }
      while (delete_mode) {
         int no_forced_deletion = delete_mode & 2;
         bool do_again;
         bool first_try = true;
         const int MAX_DELETE_JOBS = 500;
         lList *part_ref_list = NULL;
         lList *cp_ref_list = lCopyList("", ref_list);

         for_each(idep, cp_ref_list) {
            lSetUlong(idep, ID_force, !no_forced_deletion);
         } 

         /*
          * Send delete request to master. If the master is not able to
          * execute the whole request when the 'all' or '-uall' flag was
          * specified, then the master may discontinue the 
          * transaction (STATUS_OK_DOAGAIN). In this case the client has 
          * to redo the transaction.
          */ 
         do {
            do_again = false;

            if (part_ref_list == NULL) {
               int i;
               int max = MIN(lGetNumberOfElem(cp_ref_list), MAX_DELETE_JOBS); 
               lListElem *temp_ref = NULL;

               first_try = true;

               part_ref_list = lCreateList("part_del_jobs", ID_Type);
               for (i = 0; i < max; i++){
                  const char* job = NULL;
                  temp_ref = lFirst(cp_ref_list);
                  job = lGetString(temp_ref, ID_str);
                  /* break if we got job ids first and hit a job name now */
                  if (!isdigit(job[0]) && (i > 0)) {
                     break;
                  }   
                  temp_ref = lDechainElem(cp_ref_list, temp_ref);
                  lAppendElem(part_ref_list, temp_ref);
                  /* break, if we got a job name by it self */
                  if (!isdigit(job[0])){
                     break;
                  }   
               }
            }
            alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_DEL, &part_ref_list, NULL, NULL, false);

            for_each(aep, alp) {
               status = lGetUlong(aep, AN_status);

               if (lGetUlong(aep, AN_quality) == ANSWER_QUALITY_ERROR) {
                  ret = 1;
               }

               if (delete_mode != 5 && 
                   ((first_try  && status != STATUS_OK_DOAGAIN) ||
                    (!first_try && status == STATUS_OK))) {
                  
                  printf("%s\n", lGetString(aep, AN_text));
                  
               }
               /* but a job name might have extended to more than MAX_DELETE_JOBS */
               if (status == STATUS_OK_DOAGAIN) {
                  do_again = true;
               }
            }

            if (!do_again){
               lFreeList(&part_ref_list);
            }
            lFreeList(&alp);

            first_try = false;
         } while (do_again || (lGetNumberOfElem(cp_ref_list) > 0));

         lFreeList(&cp_ref_list);

         if (delete_mode == 7) {
            /* 
             * loop for one minute
             * this should prevent non-admin-users from using the '-f'
             * option regularly
             */
            for(wait = 12; wait > 0; wait--) {
               printf(".");
               fflush(stdout);
               sleep(5);
            } 
            printf("\n");

            delete_mode = 5;
         } else {
            delete_mode = 0;
         } 
      }
   }

   lFreeList(&alp);
   lFreeList(&jlp);
   lFreeList(&ref_list);
   sge_gdi2_shutdown((void**)&ctx);
   sge_prof_cleanup();
   SGE_EXIT((void**)&ctx, ret);
   return ret;

error_exit:
   lFreeList(&alp);
   lFreeList(&jlp);
   lFreeList(&ref_list);
   sge_gdi2_shutdown((void**)&ctx);
   sge_prof_cleanup();
   SGE_EXIT((void**)&ctx, 1);
   DEXIT;
   return 1;
}

/****
 **** sge_parse_cmdline_qdel (static)
 ****
 **** 'stage 1' parsing of qdel-options. parses options
 **** with their arguments and stores them in ppcmdline.
 ****/
static bool sge_parse_cmdline_qdel(
char **argv,
char **envp,
lList **ppcmdline,
lList **alpp
) {
   char **sp;
   char **rp;

   DENTER(TOP_LAYER, "sge_parse_cmdline_qdel");

   rp = argv;
   while (*(sp=rp)) {
      /* -help */
      if ((rp = parse_noopt(sp, "-help", NULL, ppcmdline, alpp)) != sp)
         continue;
      
      /* -f option */
      if ((rp = parse_noopt(sp, "-f", "--force", ppcmdline, alpp)) != sp)
         continue;

      /* -u */
      if (!strcmp("-u", *sp)) {
         lList *user_list = NULL;
         lListElem *ep_opt;

         sp++;
         if (*sp) {
            str_list_parse_from_string(&user_list, *sp, ",");

            ep_opt = sge_add_arg(ppcmdline, 0, lListT, *(sp - 1), *sp);
            lSetList(ep_opt, SPA_argval_lListT, user_list);
            sp++;
         }
         rp = sp;
         continue;
      }

      if (!strcmp("-t", *sp)) {
         lList *task_id_range_list = NULL;
         lListElem *ep_opt;

         /* next field is path_name */
         sp++;
         if (*sp == NULL) {
             answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                                     "%s", MSG_PARSE_TOPTIONMUSTHAVEALISTOFTASKIDRANGES);
             goto error;
         }

         DPRINTF(("\"-t %s\"\n", *sp));

         range_list_parse_from_string(&task_id_range_list, alpp, *sp,
                                      false, true, INF_NOT_ALLOWED);
         if (!task_id_range_list) {
            goto error; 
         }

         range_list_sort_uniq_compress(task_id_range_list, alpp, true);
         if (lGetNumberOfElem(task_id_range_list) > 1) {
            answer_list_add(alpp, MSG_QCONF_ONLYONERANGE, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            goto error;
         }

         ep_opt = sge_add_arg(ppcmdline, t_OPT, lListT, *(sp - 1), *sp);
         lSetList(ep_opt, SPA_argval_lListT, task_id_range_list);

         sp++;
         rp = sp;
         continue;
      }

      /* job id's */
      if (*sp) {
         lList *del_list = NULL;
         lListElem *job;         
         lListElem *ep = NULL;
         str_list_parse_from_string(&del_list, *sp, ",");
        
         for_each(job, del_list) {
            const char *job_name;
            job_name = lGetString(job, ST_name);
            if(ep == NULL) {
               ep = sge_add_arg(ppcmdline, 0, lListT, "jobs", NULL);
            }
            lAddElemStr(lGetListRef(ep, SPA_argval_lListT), ST_name, job_name, ST_Type);
         }  
         sp ++;
         rp = sp;
         lFreeList(&del_list);
         continue;        
      }

      /* oops */
      answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                              MSG_PARSE_INVALIDOPTIONARGUMENTX_S, *sp);
error:      
      sge_usage(QDEL, stderr);
      DRETURN(false);
   }

   DRETURN(true);

}

/****
 **** sge_parse_qdel (static)
 ****
 **** 'stage 2' parsing of qdel-options. Gets the options
 **** from ppcmdline, sets the force-flag and puts the
 **** job-numbers (strings) in ppreflist.
 ****/
static bool sge_parse_qdel(
lList **ppcmdline,
lList **ppreflist,
u_long32 *pforce,
lList **ppuserlist,
lList **alpp
) {
   u_long32 helpflag;
   lListElem *ep;
   bool ret = true;

   DENTER(TOP_LAYER, "sge_parse_qdel");

   /* Loop over all options. Only valid options can be in the
      ppcmdline list. 
   */

   while(lGetNumberOfElem(*ppcmdline))
   {
      if(parse_flag(ppcmdline, "-help",  alpp, &helpflag)) {
         sge_usage(QDEL, stdout);
         DEXIT;
         SGE_EXIT(NULL, 0);
         break;
      }
      if(parse_flag(ppcmdline, "-f", alpp, pforce)) 
         continue;

      while ((ep = lGetElemStr(*ppcmdline, SPA_switch, "-u"))) {
         lXchgList(ep, SPA_argval_lListT, ppuserlist);
         lRemoveElem(*ppcmdline, &ep);
      } 

      if(parse_multi_stringlist(ppcmdline, "-u", alpp, ppuserlist, ST_Type, ST_name)) {
         continue;  
      }

      if(parse_multi_jobtaskslist(ppcmdline, "jobs", alpp, ppreflist, true, 0)) {
         continue;
      }

       /* we get to this point, than there are -t options without job names. We have to write an error message */
      if ((ep = lGetElemStr(*ppcmdline, SPA_switch, "-t")) != NULL) {
         answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR, MSG_JOB_LONELY_TOPTION_S, lGetString(ep, SPA_switch_arg));
         break;
      }
   }

   if (answer_list_has_error(alpp)) {
      ret = false;
   }

   DRETURN(ret);
}
