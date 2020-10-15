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

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi2.h"

#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_answer.h"

#include "read_defaults.h"
#include "parse_qsub.h"
#include "parse_job_cull.h"
#include "usage.h"
#include "sig_handlers.h"
#include "sge_qrsub.h"
#include "msg_clients_common.h"

extern char **environ;


/************************************************************************/
int main(int argc, char **argv) {
   lList *pcmdline = NULL;
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   lList *ar_lp = NULL;

   lListElem *ar = NULL;

   DENTER_MAIN(TOP_LAYER, "qrsub");

   /* Set up the program information name */
   sge_setup_sig_handlers(QRSUB);

   log_state_set_log_gui(1);

   if (sge_gdi2_setup(&ctx, QRSUB, MAIN_THREAD, &alp) != AE_OK) {
      answer_list_output(&alp);
      goto error_exit;
   }

   /*
   ** stage 1 of commandline parsing
   */
   {
      dstring file = DSTRING_INIT;
      const char *user = ctx->get_username(ctx);
      const char *cell_root = ctx->get_cell_root(ctx);

      /* arguments from SGE_ROOT/common/sge_ar_request file */
      get_root_file_path(&file, cell_root, SGE_COMMON_DEF_AR_REQ_FILE);
      if ((alp = parse_script_file(QRSUB, sge_dstring_get_string(&file), "", &pcmdline, environ, 
         FLG_HIGHER_PRIOR | FLG_IGN_NO_FILE)) == NULL) {
         /* arguments from $HOME/.sge_ar_request file */
         if (get_user_home_file_path(&file, SGE_HOME_DEF_AR_REQ_FILE, user, &alp)) {
            lFreeList(&alp);
            alp = parse_script_file(QRSUB, sge_dstring_get_string(&file), "", &pcmdline, environ, 
            FLG_HIGHER_PRIOR | FLG_IGN_NO_FILE);
         }
      }
      sge_dstring_free(&file); 

      if (alp) {
         answer_list_output(&alp);
         lFreeList(&pcmdline);
         goto error_exit;
      }
   }
   
   alp = cull_parse_cmdline(QRSUB, argv+1, environ, &pcmdline, FLG_USE_PSEUDOS);

   if (answer_list_print_err_warn(&alp, NULL, "qrsub: ", MSG_WARNING) > 0) {
      lFreeList(&pcmdline);
      goto error_exit;
   }
   
   if (!pcmdline) {
      /* no command line option is present: print help to stderr */
      sge_usage(QRSUB, stderr);
      fprintf(stderr, "%s\n", MSG_PARSE_NOOPTIONARGUMENT);
      goto error_exit;
   }

   /*
   ** stage 2 of command line parsing
   */
   ar = lCreateElem(AR_Type);

   if (!sge_parse_qrsub(ctx, pcmdline, &alp, &ar)) {
      answer_list_output(&alp);
      lFreeList(&pcmdline);
      goto error_exit;
   }

   ar_lp = lCreateList(NULL, AR_Type);
   lAppendElem(ar_lp, ar);

   alp = ctx->gdi(ctx, SGE_AR_LIST, SGE_GDI_ADD | SGE_GDI_RETURN_NEW_VERSION, &ar_lp, NULL, NULL, false);
   lFreeList(&ar_lp);
   answer_list_on_error_print_or_exit(&alp, stdout);
   if (answer_list_has_error(&alp)) {
      sge_gdi2_shutdown((void**)&ctx);
      sge_prof_cleanup();
      if (answer_list_has_status(&alp, STATUS_NOTOK_DOAGAIN)) {
         DRETURN(25);
      } else {
         DRETURN(1);
      }
   }

   sge_gdi2_shutdown((void**)&ctx);
   sge_prof_cleanup();
   DRETURN(0);

error_exit:
   sge_gdi2_shutdown((void**)&ctx);
   sge_prof_cleanup();
   SGE_EXIT((void**)&ctx, 1);
   DRETURN(1);
}

