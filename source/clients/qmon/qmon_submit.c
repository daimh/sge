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
#include <unistd.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

/* This is here to avoid problems on Cygwin with hwloc, as something below clashes with a declaration in w32api/rpc.h.  Fixme: fix this better.  */
#include "sgeobj/sge_binding.h"

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>

#include <Xmt/Xmt.h>
#include <Xmt/Hash.h>
#include <Xmt/Icon.h>
#include <Xmt/Create.h>
#include <Xmt/Chooser.h>
#include <Xmt/Layout.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/InputField.h>
#include <Xmt/MsgLine.h>

/*----------------------------------------------------------------------------*/
#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_appres.h"
#include "qmon_util.h"
#include "qmon_cull.h"
#include "qmon_submit.h"
#include "qmon_comm.h"
#include "qmon_request.h"
#include "qmon_globals.h"
#include "qmon_widgets.h"
#include "qmon_quarks.h"
#include "qmon_timer.h"
#include "qmon_message.h"
#include "qmon_job.h"
#include "qmon_init.h"
#include "Matrix.h"

#include "uti/sge_unistd.h"
#include "uti/sge_prog.h"
#include "uti/sge_afsutil.h"
#include "uti/setup_path.h"
#include "uti/sge_io.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"
#include "uti/sge_time.h"

#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_path_alias.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_jsv.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_var.h"

#include "gdi/qm_name.h"
#include "gdi/sge_security.h" 
#include "gdi/sge_gdi_ctx.h"

#include "symbols.h"
#include "parse_qsub.h"
#include "parse_job_cull.h"
#include "unparse_job_cull.h"
#include "read_defaults.h"
#include "write_job_defaults.h"
#include "msg_common.h"
#include "msg_clients_common.h"
#include "msg_qmon.h"

extern sge_gdi_ctx_class_t *ctx;

extern char **environ;

/*-------------------------------------------------------------------------*/

typedef struct _tSMEntry {
   String   job_script;
   String   job_tasks;
   String   job_name;
   String   project;
   String   ckpt_obj;
   String   directive_prefix;
   String   cell;
   String   account_string;
   String   wd_path;
   String   jsv_url;
   String   binding;
   String   pe;
   lList    *task_range;            /* RN_Type */
   lList    *job_args;              /* ST_Type */
   lList    *shell_list;            /* PN_Type */
   lList    *mail_list;             /* MR_Type */
   lList    *stdoutput_path_list;   /* PN_Type */
   lList    *stdinput_path_list;    /* PN_Type */
   lList    *stderror_path_list;    /* PN_Type */   
   lList    *hard_resource_list;     
   lList    *soft_resource_list;
   lList    *hard_queue_list;       /* QR_Type */
   lList    *soft_queue_list;       /* QR_Type */
   lList    *master_queue_list;       /* QR_Type */
   lList    *env_list;              /* Environment */
   lList    *ctx_list;              /* Context */
   lList    *hold_jid;              /* JB_jid_request_list */
   lList    *hold_jid_ad;           /* JB_ja_ad_request_list */
   int      mail_options;
   int      merge_output;
   int      priority;
   int      jobshare;
   Cardinal execution_time;
   Cardinal deadline;
   int      hold;
   int      now;
   int      notify;
   int      reservation;
   int      restart;
   int      cwd;
   int      checkpoint_attr;
   int      checkpoint_interval;
   int      verify_mode;
   int      ar_id;
   int      task_concurrency;
} tSMEntry;

   
XtResource sm_resources[] = {
   { "job_script", "job_script", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, job_script),
      XtRImmediate, NULL },

   { "job_tasks", "job_tasks", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, job_tasks),
      XtRImmediate, NULL },

   { "job_name", "job_name", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, job_name),
      XtRImmediate, NULL },

   { "job_args", "job_args", QmonRST_Type,
      sizeof(lList *),
      XtOffsetOf(tSMEntry, job_args),
      XtRImmediate, NULL },

   { "project", "project", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, project),
      XtRImmediate, NULL },

   { "ckpt_obj", "ckpt_obj", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, ckpt_obj),
      XtRImmediate, NULL },

   { "directive_prefix", "directive_prefix", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, directive_prefix),
      XtRImmediate, NULL },

   { "cell", "cell", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, cell),
      XtRImmediate, NULL },

   { "account_string", "account_string", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, account_string),
      XtRImmediate, NULL },

   { "wd_path", "wd_path", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, wd_path),
      XtRImmediate, NULL },

   { "jsv_url", "jsv_url", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, jsv_url),
      XtRImmediate, NULL },

   { "binding", "binding", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, binding),
      XtRImmediate, NULL },

   { "priority", "priority", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, priority),
      XtRImmediate, NULL },

   { "jobshare", "jobshare", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, jobshare),
      XtRImmediate, NULL },

   { "restart", "restart", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, restart),
      XtRImmediate, NULL },

   { "notify", "notify", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, notify),
      XtRImmediate, NULL },

   { "hold", "hold", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, hold),
      XtRImmediate, (XtPointer) 0 },

   { "reservation", "reservation", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, reservation),
      XtRImmediate, (XtPointer) 0 },

   { "task_range", "task_range", QmonRTRN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, task_range),
      XtRImmediate, NULL },

   { "now", "now", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, now),
      XtRImmediate, (XtPointer) 0 },

   { "cwd", "cwd", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, cwd),
      XtRImmediate, NULL },

   { "execution_time", "execution_time", XtRCardinal,
      sizeof(Cardinal), XtOffsetOf(tSMEntry, execution_time),
      XtRImmediate, NULL },

   { "deadline", "deadline", XtRCardinal,
      sizeof(Cardinal), XtOffsetOf(tSMEntry, deadline),
      XtRImmediate, NULL },

   { "merge_output", "merge_output", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, merge_output),
      XtRImmediate, NULL },
   
   { "stdoutput_path_list", "stdoutput_path_list", QmonRPN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, stdoutput_path_list),
      XtRImmediate, NULL },
   
   { "stdinput_path_list", "stdinput_path_list", QmonRPN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, stdinput_path_list),
      XtRImmediate, NULL },

   { "stderror_path_list", "stderror_path_list", QmonRPN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, stderror_path_list),
      XtRImmediate, NULL },
   
   { "env_list", "env_list", QmonRENV_Type,
      sizeof(lList *),
      XtOffsetOf(tSMEntry, env_list),
      XtRImmediate, NULL },

   { "ctx_list", "ctx_list", QmonRCTX_Type,
      sizeof(lList *),
      XtOffsetOf(tSMEntry, ctx_list),
      XtRImmediate, NULL },

   { "shell_list", "shell_list", QmonRPN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, shell_list),
      XtRImmediate, NULL },

   { "pe", "pe", XtRString,
      sizeof(String),
      XtOffsetOf(tSMEntry, pe),
      XtRImmediate, NULL },

   { "ar_id", "ar_id", XtRInt,
      sizeof(int),
      XtOffsetOf(tSMEntry, ar_id),
      XtRImmediate, NULL },

   { "mail_options", "mail_options", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, mail_options),
      XtRImmediate, NULL }, 

   { "mail_list", "mail_list", QmonRMR_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, mail_list),
      XtRImmediate, NULL },

   { "hard_queue_list", "hard_queue_list", QmonRQR_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, hard_queue_list),
      XtRImmediate, NULL },

   { "soft_queue_list", "soft_queue_list", QmonRQR_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, soft_queue_list),
      XtRImmediate, NULL },

   { "master_queue_list", "master_queue_list", QmonRQR_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, master_queue_list),
      XtRImmediate, NULL },

   { "hold_jid", "hold_jid", QmonRJRE_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, hold_jid),
      XtRImmediate, NULL },

   { "hold_jid_ad", "hold_jid_ad", QmonRJRE_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, hold_jid_ad),
      XtRImmediate, NULL },

   { "verify_mode", "verify_mode", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, verify_mode),
      XtRImmediate, NULL },

   { "task_concurrency", "task_concurrency", XtRInt,
      sizeof(int), XtOffsetOf(tSMEntry, task_concurrency),
      XtRImmediate, NULL },

};

XtResource stdoutput_list_resources[] = {

   { "stdoutput_path_list", "stdoutput_path_list", QmonRPN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, stdoutput_path_list),
      XtRImmediate, NULL }
};

XtResource stdinput_list_resources[] = {

   { "stdinput_path_list", "stdinput_path_list", QmonRPN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, stdinput_path_list),
      XtRImmediate, NULL }
};

XtResource stderror_list_resources[] = {

   { "stderror_path_list", "stderror_path_list", QmonRPN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, stderror_path_list),
      XtRImmediate, NULL }
};

XtResource shell_list_resources[] = {

   { "shell_list", "shell_list", QmonRPN_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, shell_list),
      XtRImmediate, NULL }

};

XtResource mail_list_resources[] = {

   { "mail_list", "mail_list", QmonRMR_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, mail_list),
      XtRImmediate, NULL }

};

XtResource env_list_resources[] = {

   { "env_list", "env_list", QmonRVA_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, env_list),
      XtRImmediate, NULL }

};

XtResource ctx_list_resources[] = {

   { "ctx_list", "ctx_list", QmonRVA_Type,
      sizeof(lList*), XtOffsetOf(tSMEntry, ctx_list),
      XtRImmediate, NULL }

};


/*-------------------------------------------------------------------------*/
static Widget qmonSubmitCreate(Widget parent);
static void qmonSubmitPopdown(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitGetScript(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitGetJSVScript(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitInteractive(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitBinary(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitJobSubmit(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitCheckInput(Widget w, XtPointer cld, XtPointer cad);
/* static void qmonSubmitCommitInput(Widget w, XtPointer cld, XtPointer cad); */
static void qmonSubmitOutputMerge(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitShellList(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitMailList(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitStderrList(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitStdoutList(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitStdinList(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitEnvList(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitCtxList(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitOkay(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitEdit(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitReset(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitClear(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitReload(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitDelete(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitMailInput(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitCreateDialogs(Widget w);
static void qmonSubmitExecTime(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitDeadline(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitGetEnv(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitClearCtxEnv(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitAskForPE(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitAskForCkpt(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitAskForProject(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitAskForAR(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitCheckJobName(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitSaveDefault(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitLoadDefault(Widget w, XtPointer cld, XtPointer cad);
static void qmonSubmitGreyOut(Widget w, XtPointer cld, XtPointer cad);

static Boolean qmonCullToSM(lListElem *jep, tSMEntry *data, char *prefix);
static Boolean qmonSMToCull(tSMEntry *data, lListElem *jep, int save);
static void qmonFreeSMData(tSMEntry *data);
static void qmonInitSMData(tSMEntry *data);
static u_long32 ConvertMailOptions(int mail_options);
static int MailOptionsToDialog(u_long32 mail_options);
static String cwd_string(StringConst sge_o_home);
static void qmonSubmitChangeResourcesPixmap(void); 
static void qmonSubmitReadScript(Widget w, String file, String merges, int r_defaults);
static void qmonSubmitSetSensitive(int mode, int submode);
static void qmonSubmitToggleHoldNow(Widget w, XtPointer cld, XtPointer cad);
static lList *lCopyUniqNullNone(lList *lp, int keyfield);
/*-------------------------------------------------------------------------*/

static Widget qmon_submit = 0;
static Widget submit_layout = 0;
static Widget submit_detail_layout = 0;
static Widget submit_prefix = 0;
static Widget submit_script = 0;
static Widget submit_scriptPB = 0;
static Widget submit_jsv_urlPB = 0;
static Widget submit_jsv_url = 0;
static Widget submit_binding = 0;
static Widget submit_name = 0;
static Widget submit_job_args = 0;
static Widget submit_execution_time = 0;
static Widget submit_exec_timePB = 0;
static Widget submit_project = 0;
static Widget submit_projectPB = 0;
static Widget submit_ckpt_obj = 0;
static Widget submit_ckpt_objPB = 0;
static Widget submit_deadline_row = 0;
static Widget submit_deadline = 0;
static Widget submit_deadlinePB = 0;
static Widget submit_cwd = 0;
static Widget submit_wd = 0;
static Widget submit_shell = 0;
static Widget submit_shellPB = 0;
static Widget submit_output_merge = 0;
static Widget submit_stdoutput = 0;
static Widget submit_stdoutputPB = 0;
static Widget submit_stdinput = 0;
static Widget submit_stdinputPB = 0;
static Widget submit_stderror = 0;
static Widget submit_stderrorPB = 0;
static Widget submit_env = 0;
static Widget submit_envPB = 0;
static Widget submit_ctx = 0;
static Widget submit_ctxPB = 0;
static Widget submit_pe = 0;
static Widget submit_pePB = 0;
static Widget submit_ar = 0;
static Widget submit_arPB = 0;
static Widget submit_resources = 0;
static Widget submit_mail = 0;
static Widget submit_mail_user = 0;
static Widget submit_mail_userPB = 0;
static Widget submit_notify = 0;
static Widget submit_reservation = 0;
static Widget submit_hold = 0;
static Widget submit_task_hold = 0;
static Widget submit_now = 0;
static Widget submit_restart = 0;
static Widget submit_main_link = 0; 
static Widget submit_message = 0;
static Widget submit_interactive = 0;
static Widget submit_binary = 0;
static Widget submit_tasks = 0;

static Widget submit_edit = 0;
static Widget submit_reload = 0;
static Widget submit_load = 0;
static Widget submit_save = 0;
static Widget submit_submit = 0;
static Widget submit_done = 0;
static Widget submit_details = 0;
static Widget submit_clear = 0;

/* subdialogs */
static Widget shell_list_w = 0;
static Widget mail_list_w = 0;
static Widget stderror_list_w = 0;
static Widget stdoutput_list_w = 0;
static Widget stdinput_list_w = 0;
static Widget env_list_w = 0;
static Widget ctx_list_w = 0;

static tSubmitMode submit_mode_data = {SUBMIT_NORMAL, 
                                       SUBMIT_NORMAL|SUBMIT_SCRIPT, 0};
static tSMEntry SMData; 
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/* PUBLIC                                                                  */
/*-------------------------------------------------------------------------*/
void qmonSubmitPopup(Widget w, XtPointer cld, XtPointer cad)
{
   tSubmitMode *data = (tSubmitMode *)cld;
   XmString xtitle = NULL;
   char buf[128];
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonSubmitPopup");
   
   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   qmonMirrorMultiAnswer(JOB_T | USERSET_T | PROJECT_T | PE_T | CKPT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }
   
   if (!qmon_submit) {
      /*
      ** create the main dialog, if it doesn't exist
      */
      qmon_submit = qmonSubmitCreate(AppShell);

#ifdef FIXME
      /* 
      ** set the close button callback 
      ** set the icon and iconName
      */
      XmtCreatePixmapIcon(qmon_submit, qmonGetIcon("toolbar_submit"), None);
      XtVaSetValues(qmon_submit, XtNiconName, "qmon:Submit Control", NULL);
      XmtAddDeleteCallback(qmon_submit, XmDO_NOTHING, qmonSubmitPopdown,  NULL);
#endif

      /* 
      ** create the attached modal dialogs for shell, mail, pe, env, ctx
      **
      */
      qmonSubmitCreateDialogs(qmon_submit);

      /*
      ** initialize the SMData struct
      */
      qmonInitSMData(&SMData);
   }

   /* 
   ** start up job timer  
   */
   qmonStartTimer(JOB_T | PE_T);

   /*
   ** reset interactive mode
   */
   XmToggleButtonSetState(submit_interactive, 0, True);
   XmToggleButtonSetState(submit_binary, 0, True);

   /*
   ** reset; fill the dialog in qalter mode
   */
   qmonSubmitClear(w, NULL, NULL);
   
   /*
   ** set submit_mode_data
   */
   if (data) {
      submit_mode_data.mode = data->mode;
      submit_mode_data.job_id = data->job_id;
      /* set dialog title */
      sprintf(buf, "Alter job " sge_u32, data->job_id);
      xtitle = XmtCreateXmString(buf); 
      XtVaSetValues( qmon_submit,
                     XmNdialogTitle, xtitle,
                     NULL);
      XmStringFree(xtitle);
   } 
   else {
      submit_mode_data.mode = SUBMIT_NORMAL;
      submit_mode_data.job_id = 0;
      xtitle = XmtCreateLocalizedXmString(qmon_submit, "@{Submit Job}");
      XtVaSetValues( qmon_submit,
                     XmNdialogTitle, xtitle,
                     NULL);
      XmStringFree(xtitle);
   }

   if (submit_mode_data.mode == SUBMIT_QALTER_PENDING) {
      lListElem *job_to_set;

      qmonMirrorMultiAnswer(JOB_T, &alp);
      if (alp) {
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);
         /* set default cursor */
         XmtDisplayDefaultCursor(w);
         DEXIT;
         return;
      }
      job_to_set = job_list_locate(qmonMirrorList(SGE_JB_LIST), 
                                   submit_mode_data.job_id);
      /*
      ** is it an interactive job ?
      */
      if (lGetString(job_to_set, JB_script_file)) {
         DPRINTF(("qalter interactive mode\n"));
         XmToggleButtonSetState(submit_interactive, 0, True);
      }
      else {
         DPRINTF(("qalter batch mode\n"));
         XmToggleButtonSetState(submit_interactive, 1, True);
      }
      if (JOB_TYPE_IS_BINARY(lGetUlong(job_to_set, JB_type))) {
         DPRINTF(("binary submission\n"));
         XmToggleButtonSetState(submit_binary, 1, True);
      } else {
         DPRINTF(("Script submission\n"));
         XmToggleButtonSetState(submit_binary, 0, True);
      }

      /*
      ** for debugging
      */
      if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
         printf("___JOB_BEFORE_ALTER_____________________\n");
         lWriteElemTo(job_to_set, stdout);
         printf("________________________________________\n");
      }

      qmonCullToSM(job_to_set, &SMData, NULL);
      XmtDialogSetDialogValues(submit_layout, &SMData);

      setButtonLabel(submit_submit, "@{Qalter}");
      /*
      ** change the resources pixmap icon if necessary
      */
      qmonSubmitChangeResourcesPixmap();
   }
   else {
      setButtonLabel(submit_submit, "@{Submit}");
   }
      

   /*
   ** set sensitivity depending on submit mode
   ** if called in SUBMIT_QALTER_PENDING mode
   */

   qmonSubmitSetSensitive(submit_mode_data.mode, submit_mode_data.sub_mode);

   /*
   ** pop it up and raise to top of window stack
   */
   xmui_manage(qmon_submit);

   /* set default cursor */
   XmtDisplayDefaultCursor(w);
   
   DEXIT;
}   


/*-------------------------------------------------------------------------*/
lList *qmonSubmitHR(void)
{
   return SMData.hard_resource_list;
}
   
/*-------------------------------------------------------------------------*/
lList *qmonSubmitSR(void)
{
   return SMData.soft_resource_list;
}

/*-------------------------------------------------------------------------*/
String qmonSubmitRequestType(void)
{
   static char buf[BUFSIZ];

   DENTER(GUI_LAYER, "qmonSubmitRequestType");

   XmtDialogGetDialogValues(submit_layout, &SMData);

   if (SMData.pe) {
      sprintf(buf, 
              XmtLocalize(submit_layout, "@fBParallel Job Request - %s",
                           "@fBParallel Job Request - %s"),
              SMData.pe); 
   }
   else
      strcpy(buf, 
             XmtLocalize(submit_layout, "@fBSerial Job", "@fBSerial Job"));
                           

   DEXIT;
   return buf;
}

/*-------------------------------------------------------------------------*/
void qmonSubmitSetResources(
lList **hr,
lList **sr 
) {
   DENTER(GUI_LAYER, "qmonSubmitSetResources");

   /*
   ** free the old lists 
   */
   lFreeList(&(SMData.hard_resource_list));
   lFreeList(&(SMData.soft_resource_list));

   /*
   ** attach the new lists
   */
   if (sr && *sr) {
      SMData.soft_resource_list = lCopyList("sr", *sr);
   }
  
   if (hr && *hr) {
      SMData.hard_resource_list = lCopyList("hr", *hr);
   }

   /*
   ** change the resources pixmap icon if necessary
   */
   qmonSubmitChangeResourcesPixmap();

   DEXIT;
}
   
   
   
/*-------------------------------------------------------------------------*/
/* P R I V A T E                                                           */
/*-------------------------------------------------------------------------*/
static Widget qmonSubmitCreate(
Widget parent 
) {
   

   DENTER(GUI_LAYER, "qmonSubmitCreate");

#if 0
   /* create shell */
   qmon_submit = XmtCreateChild(parent, "qmon_submit");

   /* submit control layout */
   submit_layout = XmtCreateLayout( qmon_submit, "submit_layout", 
                                    NULL, 0); 

   /* bind resource list */
   XmtDialogBindResourceList( submit_layout, 
                              sm_resources, XtNumber(sm_resources));
#endif

   /* create children */
   submit_layout = XmtBuildQueryDialog( parent, "qmon_submit",
                          sm_resources, XtNumber(sm_resources),
                          "submit_detail_layout", &submit_detail_layout,
                          "submit_scriptPB", &submit_scriptPB,
                          "submit_script", &submit_script,
                          "submit_jsv_urlPB", &submit_jsv_urlPB,
                          "submit_jsv_url", &submit_jsv_url,
                          "submit_binding", &submit_binding,
                          "submit_shellPB", &submit_shellPB,
                          "submit_stdoutputPB", &submit_stdoutputPB,
                          "submit_stdinputPB", &submit_stdinputPB,
                          "submit_stderrorPB", &submit_stderrorPB,
                          "submit_mail_userPB", &submit_mail_userPB,
                          "submit_envPB", &submit_envPB,
                          "submit_ctxPB", &submit_ctxPB,
                          "submit_pePB", &submit_pePB,
                          "submit_arPB", &submit_arPB,
                          "submit_submit", &submit_submit,
                          "submit_done", &submit_done,
                          "submit_edit", &submit_edit,
                          "submit_details", &submit_details,
                          "submit_resources", &submit_resources,
                          "submit_shell", &submit_shell,
                          "submit_stdoutput", &submit_stdoutput,
                          "submit_stdinput", &submit_stdinput,
                          "submit_stderror", &submit_stderror,
                          "submit_output_merge", &submit_output_merge,
                          "submit_cwd", &submit_cwd,
                          "submit_wd", &submit_wd,
                          "submit_notify", &submit_notify,
                          "submit_reservation", &submit_reservation,
                          "submit_hold", &submit_hold,
                          "submit_task_hold", &submit_task_hold,
                          "submit_now", &submit_now,
                          "submit_restart", &submit_restart,
                          "submit_mail", &submit_mail,
                          "submit_mail_user", &submit_mail_user,
                          "submit_env", &submit_env,
                          "submit_ctx", &submit_ctx,
                          "submit_pe", &submit_pe,
                          "submit_ar", &submit_ar,
                          "submit_execution_time", &submit_execution_time,
                          "submit_exec_timePB", &submit_exec_timePB,
                          "submit_deadline_row", &submit_deadline_row,
                          "submit_deadline", &submit_deadline,
                          "submit_deadlinePB", &submit_deadlinePB,
                          "submit_tasks", &submit_tasks,
                          "submit_project", &submit_project,
                          "submit_projectPB", &submit_projectPB,
                          "submit_ckpt_obj", &submit_ckpt_obj,
                          "submit_ckpt_objPB", &submit_ckpt_objPB,
                          "submit_clear", &submit_clear,
                          "submit_reload", &submit_reload,
                          "submit_save", &submit_save,
                          "submit_load", &submit_load,
                          "submit_name", &submit_name,
                          "submit_prefix", &submit_prefix,
                          "submit_main_link", &submit_main_link,
                          "submit_message", &submit_message,
                          "submit_interactive", &submit_interactive,
                          "submit_binary", &submit_binary,
                          "submit_job_args", &submit_job_args,
                          NULL);
   
   /*
   ** in SGE mode the project field and the deadline time have to be
   ** displayed, otherwise they are unmanaged
   */
   XtAddCallback(submit_deadlinePB, XmNactivateCallback, 
                  qmonSubmitDeadline, NULL);
   XtManageChild(submit_layout);

   /* callbacks */
   XtAddCallback(submit_interactive, XmNvalueChangedCallback, 
                     qmonSubmitInteractive, NULL);
   XtAddCallback(submit_binary, XmNvalueChangedCallback, 
                     qmonSubmitBinary, NULL);
   XtAddCallback(submit_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
/*    XtAddCallback(submit_script, XmNactivateCallback,  */
/*                      qmonSubmitReload, NULL); */
   XtAddCallback(submit_script, XmtNinputCallback,                  
                     qmonSubmitReload, NULL);
   XtAddCallback(submit_script, XmtNverifyCallback, 
                     qmonSubmitCheckInput, NULL);
/*    XtAddCallback(submit_script, XmtNinputCallback,  */
/*                      qmonSubmitCommitInput, NULL); */
   XtAddCallback(submit_scriptPB, XmNactivateCallback, 
                     qmonSubmitGetScript, NULL);
   XtAddCallback(submit_name, XmtNverifyCallback, 
                     qmonSubmitCheckJobName, NULL);
   XtAddCallback(submit_submit, XmNactivateCallback, 
                     qmonSubmitJobSubmit, NULL);
   XtAddCallback(submit_resources, XmNactivateCallback, 
                     qmonRequestPopup, NULL);
   XtAddCallback(submit_done, XmNactivateCallback, 
                     qmonSubmitPopdown, NULL);
   XtAddCallback(submit_edit, XmNactivateCallback, 
                     qmonSubmitEdit, NULL);
   XtAddCallback(submit_clear, XmNactivateCallback, 
                     qmonSubmitClear, NULL);
   XtAddCallback(submit_reload, XmNactivateCallback, 
                     qmonSubmitReload, NULL);
   XtAddCallback(submit_save, XmNactivateCallback, 
                     qmonSubmitSaveDefault, NULL);
   XtAddCallback(submit_load, XmNactivateCallback, 
                     qmonSubmitLoadDefault, NULL);
   XtAddCallback(submit_mail_userPB, XmNactivateCallback, 
                     qmonSubmitMailList, NULL);
   XtAddCallback(submit_envPB, XmNactivateCallback, 
                     qmonSubmitEnvList, NULL);
   XtAddCallback(submit_ctxPB, XmNactivateCallback, 
                     qmonSubmitCtxList, NULL);
   XtAddCallback(submit_ckpt_objPB, XmNactivateCallback, 
                     qmonSubmitAskForCkpt, NULL);
   XtAddCallback(submit_pePB, XmNactivateCallback, 
                     qmonSubmitAskForPE, NULL);
   XtAddCallback(submit_arPB, XmNactivateCallback, 
                     qmonSubmitAskForAR, NULL);
   XtAddCallback(submit_output_merge, XmNvalueChangedCallback, 
                     qmonSubmitOutputMerge, NULL);
   XtAddCallback(submit_stdoutputPB, XmNactivateCallback, 
                     qmonSubmitStdoutList, NULL);
   XtAddCallback(submit_stdinputPB, XmNactivateCallback, 
                     qmonSubmitStdinList, NULL);
   XtAddCallback(submit_stderrorPB, XmNactivateCallback, 
                     qmonSubmitStderrList, NULL);
   XtAddCallback(submit_exec_timePB, XmNactivateCallback, 
                     qmonSubmitExecTime, NULL);
   XtAddCallback(submit_shellPB, XmNactivateCallback, 
                     qmonSubmitShellList, NULL);
   XtAddCallback(submit_projectPB, XmNactivateCallback, 
                     qmonSubmitAskForProject, NULL);
   XtAddCallback(submit_now, XmNvalueChangedCallback,
                     qmonSubmitToggleHoldNow, NULL);
   XtAddCallback(submit_hold, XmNvalueChangedCallback,
                     qmonSubmitToggleHoldNow, NULL);
   XtAddCallback(submit_cwd, XmNvalueChangedCallback,
                     qmonSubmitGreyOut, NULL);
   XtAddCallback(submit_jsv_urlPB, XmNactivateCallback, 
                     qmonSubmitGetJSVScript, NULL);

   XtAddEventHandler(XtParent(submit_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
   XtAddEventHandler(XtParent(submit_layout), StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);

   DEXIT;
   return submit_layout;

}



/*-------------------------------------------------------------------------*/
static void qmonSubmitPopdown(Widget w, XtPointer cld, XtPointer cad)
{

   DENTER(GUI_LAYER, "qmonSubmitPopdown");

   if (qmon_submit) {
      qmonStopTimer(JOB_T | PE_T);
      xmui_unmanage(qmon_submit);
   } 
   DEXIT;
}   

/*-------------------------------------------------------------------------*/
static void qmonSubmitInteractive(Widget w, XtPointer cld, XtPointer cad)
{
   XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct*) cad;
   char buf[512];
   String dsp;

   const char* qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(GUI_LAYER, "qmonSubmitInteractive");

   if (!cbs->set) {
      SETBIT(SUBMIT_NORMAL, submit_mode_data.sub_mode);
      CLEARBIT(SUBMIT_QSH, submit_mode_data.sub_mode);
   } else {
      SETBIT(SUBMIT_QSH, submit_mode_data.sub_mode);
      CLEARBIT(SUBMIT_NORMAL, submit_mode_data.sub_mode);
   }  

   XmToggleButtonSetState(submit_binary, 0, True); 
   
   /*
   ** clear the entries and set default name of job
   */
   qmonSubmitClear(w, NULL, NULL);

   /*
   ** reset the sensitivity depending on state
   */
   if (ISSET(submit_mode_data.sub_mode, SUBMIT_QSH)) {
      XmtInputFieldSetString(submit_name, "INTERACTIVE"); 
      dsp = DisplayString(XtDisplay(w));
      if (!strcmp(dsp, ":0") || !strcmp(dsp, ":0.0"))
         sprintf(buf, "DISPLAY=%s%s", qualified_hostname, dsp); 
      else
         sprintf(buf, "DISPLAY=%s", dsp); 
      XmtInputFieldSetString(submit_env, buf); 
      XmToggleButtonSetState(submit_now, 1, True);
   }

   qmonSubmitSetSensitive(submit_mode_data.mode, submit_mode_data.sub_mode);

   DEXIT;
}   

/*-------------------------------------------------------------------------*/
static void qmonSubmitBinary(Widget w, XtPointer cld, XtPointer cad)
{
   XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct*) cad;

   DENTER(GUI_LAYER, "qmonSubmitBinary");

   if (!cbs->set) {
      SETBIT(SUBMIT_SCRIPT, submit_mode_data.sub_mode);
      CLEARBIT(SUBMIT_BINARY, submit_mode_data.sub_mode);
   } else {
      SETBIT(SUBMIT_BINARY, submit_mode_data.sub_mode);
      CLEARBIT(SUBMIT_SCRIPT, submit_mode_data.sub_mode);
   }   
   
   /*
    * clear the entries and set default name of job
    */
   qmonSubmitClear(w, NULL, NULL);

   qmonSubmitSetSensitive(submit_mode_data.mode, submit_mode_data.sub_mode);

   DEXIT;
}   

/*-------------------------------------------------------------------------*/
static void qmonSubmitSetSensitive(
int mode,
int submode 
) {
   Boolean sensitive, sensitive2;

   const char *username = ctx->get_username(ctx);

   DENTER(GUI_LAYER, "qmonSubmitSetSensitive");

   if (mode == SUBMIT_NORMAL)
      sensitive = True;
   else
      sensitive = False;
   
   if (ISSET(submode, SUBMIT_QSH))
      sensitive2 = False;
   else
      sensitive2 = True;

   /*
   ** main submit dialogue section
   */
   XtSetSensitive(submit_prefix, sensitive2 & sensitive);
   XtSetSensitive(submit_script, sensitive2 & sensitive);
   XtSetSensitive(submit_scriptPB, sensitive2 & sensitive);
   XtSetSensitive(submit_hold, sensitive);
   XtSetSensitive(submit_task_hold, sensitive);

   XtSetSensitive(submit_tasks, sensitive2 & sensitive);
   XtSetSensitive(submit_job_args, sensitive);

   XtSetSensitive(submit_execution_time, sensitive2);
   XtSetSensitive(submit_exec_timePB, sensitive2);
   XtSetSensitive(submit_stdoutput, sensitive2);
   XtSetSensitive(submit_stdinput, sensitive2);
   XtSetSensitive(submit_stderror, sensitive2);
   XtSetSensitive(submit_stdoutputPB, sensitive2);
   XtSetSensitive(submit_stdinputPB, sensitive2); 
   XtSetSensitive(submit_stderrorPB, sensitive2);
   XtSetSensitive(submit_output_merge, sensitive2);
/*    XtSetSensitive(submit_now, sensitive2); */

   /*
   ** detail submit dialogue section, mail allowed only for abort
   */
   XmtChooserSetSensitive(submit_mail, 0, sensitive2);
   XmtChooserSetSensitive(submit_mail, 1, sensitive2);
   XmtChooserSetSensitive(submit_mail, 3, sensitive2);
   XtSetSensitive(submit_notify, sensitive2);

   /*
   ** set to special value, is reset by qmonSubmitClear
   */
   XtSetSensitive(submit_restart, sensitive2);

   /*
   ** set sensitivity of deadline field
   */
   if (userset_is_deadline_user(qmonMirrorList(SGE_US_LIST),
            username)) {
      if (sensitive) {      
         XtSetSensitive(submit_deadline, sensitive2);
         XtSetSensitive(submit_deadlinePB, sensitive2);
      }   
      else {
         XtSetSensitive(submit_deadline, sensitive);
         XtSetSensitive(submit_deadlinePB, sensitive);
      }   
   }
   else {
      XtSetSensitive(submit_deadline, False);
      XtSetSensitive(submit_deadlinePB, False);
   }

   /*
   ** action buttons
   */
   XtSetSensitive(submit_interactive, sensitive);
   XtSetSensitive(submit_binary, sensitive && sensitive2);
   XtSetSensitive(submit_reload, sensitive);
   XtSetSensitive(submit_edit, sensitive);
   XtSetSensitive(submit_save, sensitive);
   XtSetSensitive(submit_load, sensitive);

   DEXIT;
}   


/*-------------------------------------------------------------------------*/
/* set sensitivity of stderr/stdout input                                  */
/*-------------------------------------------------------------------------*/
static void qmonSubmitOutputMerge(Widget w, XtPointer cld, XtPointer cad)
{  
   XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct*)cad;

   DENTER(GUI_LAYER, "qmonSubmitOutputMerge");
   
   if (cbs->set) {
      XtSetSensitive(submit_stderror, False);
      XtSetSensitive(submit_stderrorPB, False);
   }
   else {
      XtSetSensitive(submit_stderror, True);
      XtSetSensitive(submit_stderrorPB, True);
   }
   
   DEXIT;
}   

/*-------------------------------------------------------------------------*/
/* set sensitivity of submit_wd                                            */
/*-------------------------------------------------------------------------*/
static void qmonSubmitGreyOut(Widget w, XtPointer cld, XtPointer cad)
{  
   XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct*)cad;

   DENTER(GUI_LAYER, "qmonSubmitGreyOut");
   
   if (cbs->set) {
      XtSetSensitive(submit_wd, False);
   }
   else {
      XtSetSensitive(submit_wd, True);
   }
   
   DEXIT;
}   

/*-------------------------------------------------------------------------*/
/* write a job configuration to file                                       */
/*-------------------------------------------------------------------------*/
static void qmonSubmitSaveDefault(Widget w, XtPointer cld, XtPointer cad)
{  

   static char filename[BUFSIZ];
   static char directory[BUFSIZ];
   Boolean status = False;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonSubmitSaveDefault");

   status = XmtAskForFilename(w, NULL,
                              "@{Please type or select a filename}",
                              NULL, NULL,
                              filename, sizeof(filename),
                              directory, sizeof(directory),
                              "*", 0,
                              NULL);

   if (status == True) {
      if (filename[strlen(filename)-1] != '/')  {
         lListElem *jep = lCreateElem(JB_Type);

         /* 
         ** get the values from the dialog fields
         */
         XmtDialogGetDialogValues(submit_layout, &SMData);
         if (!qmonSMToCull(&SMData, jep, 1)) {
            DPRINTF(("qmonSMToCull failure\n"));
            qmonMessageShow(w, True, "@{Saving of job attributes failed!}");
            lFreeElem(&jep);
            DRETURN_VOID;
         }

         alp = write_job_defaults(ctx, jep, filename, 0);

         qmonMessageBox(w, alp, 0);

         lFreeElem(&jep);
         lFreeList(&alp);
      }
      else {
         qmonMessageShow(w, True, "@{No valid filename specified !}");
      }
   }

   DRETURN_VOID;
}   

/*-------------------------------------------------------------------------*/
/* load a job configuration from file                                      */
/*-------------------------------------------------------------------------*/
static void qmonSubmitLoadDefault(Widget w, XtPointer cld, XtPointer cad)
{  

   static char filename[4*BUFSIZ];
   static char directory[4*BUFSIZ];
   Boolean status = False;
   String script = NULL;

   DENTER(GUI_LAYER, "qmonSubmitLoadDefault");

   script = XmtInputFieldGetString(submit_script);
   if (!script || script[0] == '\0') {
      qmonMessageShow(w, True, "@{Choose a script first !}");
      DEXIT;
      return;
   }

   status = XmtAskForFilename(w, NULL,
                              "@{Please type or select a filename}",
                              NULL, NULL,
                              filename, sizeof(filename),
                              directory, sizeof(directory),
                              "*", 0,
                              NULL);

   if (status == True) {
      qmonSubmitReadScript(w, script, filename, 1);
   }

   DEXIT;
}   


/*-------------------------------------------------------------------------*/
/* popup XmtAskForString Dialog, get exec time and set it in input field   */
/* if inputfield is empty or 0 there are no restrictions on exec time      */
/*-------------------------------------------------------------------------*/
static void qmonSubmitExecTime(Widget w, XtPointer cld, XtPointer cad)
{  
   Boolean status;
   char message[] = "@{submit.asksubmittime.Enter the submit time in the\nfollowing format: [[CC]]YY]MMDDhhmm[.ss]\nor leave the current time and press ok}";
   char exec_time[128];
   lList *alp = NULL;
   dstring ds;
   char buffer[128];

   DENTER(GUI_LAYER, "qmonSubmitExecTime");

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   XmtDialogGetDialogValues(submit_layout, &SMData);

   strcpy(exec_time, sge_at_time(SMData.execution_time, &ds));    

   status = XmtAskForString(w, NULL, message, 
                           exec_time, sizeof(exec_time), 
                           NULL);
   /* 
   ** validate exec_time and show warning msgbox
   */
   if (status) {
      u_long32 tmp_date_time;

      ulong_parse_date_time_from_string(&tmp_date_time, &alp, exec_time);
      SMData.execution_time = tmp_date_time;
      if (alp) {
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);
      } else {   
         XmtDialogSetDialogValues(submit_layout, &SMData);
      }   
   }

   DEXIT;
}   

/*-------------------------------------------------------------------------*/
/* popup XmtAskForString Dialog, get deadline time                         */
/* if inputfield is empty or 0 there are no restrictions on exec time      */
/*-------------------------------------------------------------------------*/
static void qmonSubmitDeadline(Widget w, XtPointer cld, XtPointer cad)
{  
   Boolean status;
   char message[] = "@{submit.askdeadlinetime.Enter the deadline time in the\nfollowing format: [[CC]]YY]MMDDhhmm.[ss]\nor leave the current time and press ok}";
   char deadline_time[128];
   char *set_deadline_time = NULL;
   lList *alp = NULL;
   dstring ds;
   char buffer[128];

   DENTER(GUI_LAYER, "qmonSubmitDeadline");

   sge_dstring_init(&ds, buffer, sizeof(buffer));
   XmtDialogGetDialogValues(submit_layout, &SMData);

   set_deadline_time = XmtInputFieldGetString(submit_deadline);
   
   if (set_deadline_time && set_deadline_time[0] != '\0')
      strcpy(deadline_time, set_deadline_time);
   else   
      strcpy(deadline_time, sge_at_time(0, &ds));    

   status = XmtAskForString(w, NULL, message, 
                           deadline_time, sizeof(deadline_time), 
                           NULL);
   /* 
   ** validate deadline_time and show warning msgbox
   */
   if (status) {
      u_long32 tmp_date_time;

      ulong_parse_date_time_from_string(&tmp_date_time, &alp, deadline_time);
      SMData.deadline = tmp_date_time;
      if (alp) {
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);
      } else {   
         XmtDialogSetDialogValues(submit_layout, &SMData);
      }   
   }

   DEXIT;
}   


/*-------------------------------------------------------------------------*/
/* get the dialog entries and send gdi request                             */
/*-------------------------------------------------------------------------*/
static void qmonSubmitJobSubmit(Widget w, XtPointer cld, XtPointer cad)
{
   lList *lp = NULL, *alp = NULL;
   lEnumeration *what = NULL;
   char buf[BUFSIZ];
   Boolean status = False;
   unsigned long job_number;
   int just_verify = 0;
   const char *username = ctx->get_username(ctx);
   const char *sge_root = ctx->get_sge_root(ctx);
   const char *mastername = ctx->get_master(ctx, false);
  
   DENTER(GUI_LAYER, "qmonSubmitJobSubmit");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   /* 
   **get the values from the dialog fields
   */
   XmtDialogGetDialogValues(submit_layout, &SMData);

   if (submit_mode_data.mode != SUBMIT_QALTER_PENDING) {
      /*
      ** validate input, return error message
      */
      if ((!SMData.job_script || SMData.job_script[0] == '\0') && 
          !ISSET(submit_mode_data.sub_mode, SUBMIT_QSH)) {
         snprintf(buf, sizeof(buf), "%s", XmtLocalize(w, "Job Script required !",
                  "Job Script required !"));
         goto error;
      }

      if (SMData.pe && SMData.pe[0] != '\0') {
         char theInput[1024];
         char *pe = NULL;
         char *pe_range = NULL;
         lList *alp = NULL;
   
         sge_strlcpy(theInput, SMData.pe, 1023);
         pe = strtok(theInput, " ");
         pe_range = strtok(NULL, "\n");
         if (!(pe_range && pe_range[0] != '\0')) {
            snprintf(buf, sizeof(buf), "%s",
               XmtLocalize(w, 
               "Parallel Environment requires valid name and valid range !", 
               "Parallel Environment requires valid name and valid range !")
            );
            goto error;
         } else {
            lList *range_list = NULL;

            range_list_parse_from_string(&range_list, &alp, pe_range,
                                         1, 0, INF_ALLOWED);
            lFreeList(&range_list);
            if (alp) {
               snprintf(buf, sizeof(buf), "%s",
                  XmtLocalize(w, 
                  "Parallel Environment requires valid name and valid range !", 
                  "Parallel Environment requires valid name and valid range !")
               );
               lFreeList(&alp);
               goto error;
            }
         }
      }

      if (!(lp = lCreateElemList("JobSubmitList", JB_Type, 1))) {
         DPRINTF(("lCreateElemList failure\n"));
         snprintf(buf, sizeof(buf), "%s",
                 XmtLocalize(w, 
                             "Job submission failed", 
                             "Job submission failed")
         );
         goto error;
      }

      if (!qmonSMToCull(&SMData, lFirst(lp), 0)) {
         DPRINTF(("qmonSMToCull failure\n"));
         snprintf(buf, sizeof(buf), "%s",
                 XmtLocalize(w, 
                             "Job submission failed", 
                             "Job submission failed")
         );
         goto error;
      }

      /*
      ** security hook
      */
      if (set_sec_cred(sge_root, mastername, lFirst(lp), &alp) != 0) {
         qmonMessageBox(w, alp, true);
         lFreeList(&alp);
         snprintf(buf, sizeof(buf), "%s\n", MSG_SEC_SETJOBCRED);
         goto error;
      }   

      if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
         printf("___SUBMITTED_JOB________________________\n");
         lWriteListTo(lp, stdout);
         printf("________________________________________\n");
      }

      just_verify = (lGetUlong(lFirst(lp), JB_verify_suitable_queues) == JUST_VERIFY || 
                      lGetUlong(lFirst(lp), JB_verify_suitable_queues) == POKE_VERIFY);

      what = lWhat("%T(ALL)", JB_Type);
      alp = qmonAddList(SGE_JB_LIST, qmonMirrorListRef(SGE_JB_LIST), 
                        JB_job_number, &lp, NULL, what);

      if (lFirst(alp) && (lGetUlong(lFirst(alp), AN_status) == STATUS_OK
            || lGetUlong(lFirst(alp), AN_quality) != ANSWER_QUALITY_ERROR))
         status = True;

      qmonMessageBox(w, alp, just_verify);

      if (status && !just_verify) {
/*          char task_str[1024]; */
         /*
         ** start a timer for immediate jobs to check if submission succeeded
         */
         if (JOB_TYPE_IS_IMMEDIATE(lGetUlong(lFirst(lp), JB_type))) {
            job_number = lGetUlong(lFirst(lp), JB_job_number);
            qmonTimerCheckInteractive(w, (XtPointer)job_number, NULL);
         } 
         updateJobList();
/*          strcpy(task_str, ""); */
/*          ja_task_list_print_to_string(lGetList(lFirst(lp), JB_ja_tasks), task_str); */
/*          XmtMsgLinePrintf(submit_message, "Job %d (%s) submitted",  */
/*                 (int)lGetUlong(lFirst(lp), JB_job_number), task_str); */
         XmtMsgLinePrintf(submit_message, 
                           XmtLocalize(w, "Job %d submitted", "Job %d submitted"), 
                           (int)lGetUlong(lFirst(lp), JB_job_number));
         XmtMsgLineClear(submit_message, DISPLAY_MESSAGE_DURATION); 
      }
      else if (!just_verify) {
         int jobid;
         if ( lFirst(lp) && (jobid = (int)lGetUlong(lFirst(lp), JB_job_number)))
            XmtMsgLinePrintf(submit_message, 
                             XmtLocalize(w, "Job %d failed", "Job %d failed"),
                             jobid); 
         else
            XmtMsgLinePrintf(submit_message, "%s",
                             XmtLocalize(w, "Job Submission failed", 
                                          "Job Submission failed")); 
         XmtMsgLineClear(submit_message, DISPLAY_MESSAGE_DURATION); 
      }
      lFreeWhat(&what);
      lFreeList(&lp);
      lFreeList(&alp);
   }
   else {
      Boolean close_dialog = True;
      /* should be the same fields like in tSMEntry in qmon_submit.h */
      static int fixed_qalter_fields[] = {
         JB_job_number,
         JB_ja_tasks,
         JB_ja_structure,
         JB_job_name,
         JB_job_args,
         JB_priority,
         JB_jobshare,
         JB_execution_time,
         JB_cwd,
         JB_hard_resource_list,
         JB_soft_resource_list,
         JB_reserve,
         JB_merge_stderr,
         JB_stdout_path_list,
         JB_stdin_path_list,
         JB_stderr_path_list,
         JB_mail_options,
         JB_mail_list,
         JB_notify,
         JB_restart,
         JB_account,
         JB_project,
         JB_checkpoint_name,
         JB_pe_range,
         JB_pe,
         JB_hard_queue_list,
         JB_soft_queue_list,
         JB_master_hard_queue_list,
         JB_jid_request_list,
/*         JB_jid_predecessor_list, */
         JB_shell_list,
         JB_env_list,
         JB_verify_suitable_queues,
         JB_type,
         JB_ja_task_concurrency,
         NoName
      };
      int qalter_fields[100];
      int i;
      lEnumeration *what;
      lDescr *rdp = NULL;

      /* initialize int array */
      qalter_fields[0] = NoName;

      /* add all standard qalter fields */
      for (i=0; fixed_qalter_fields[i]!= NoName; i++) {
         nm_set((int*)qalter_fields, (int)fixed_qalter_fields[i]);
      }

      /* 
      ** the deadline initiation time 
      ** can be modified if the user is a deadline user 
      */
      qmonMirrorMultiAnswer(USERSET_T, &alp);
      if (alp) {
         qmonMessageBox(w, alp, 0);
         /* set default cursor */
         XmtDisplayDefaultCursor(w);
         lFreeList(&alp);
         DEXIT;
         return;
      }

      if (userset_is_deadline_user(qmonMirrorList(SGE_US_LIST),
            username)) 
         nm_set((int*)qalter_fields, JB_deadline);

      if (!(what = lIntVector2What(JB_Type, (int*) qalter_fields))) {
         DPRINTF(("lIntVector2What failure\n"));
         sprintf(buf, "Job modify operation failed\n");
         goto error;
      }

      lReduceDescr(&rdp, JB_Type, what);
      if (!rdp) {
         DPRINTF(("lReduceDescr failure\n"));
         sprintf(buf, "failed to build reduced descriptor\n");
         goto error;
      }
      lFreeWhat(&what);
      
      if (!(lp = lCreateElemList("JobSubmitList", rdp, 1))) {
         sge_free(&rdp);
         DPRINTF(("lCreateElemList failure\n"));
         sprintf(buf, "Job submission failed\n");
         goto error;
      }
      sge_free(&rdp);

      lSetUlong(lFirst(lp), JB_job_number, submit_mode_data.job_id);
      
      if (!qmonSMToCull(&SMData,lFirst(lp), 0)) {
         DPRINTF(("qmonSMToCull failure\n"));
         sprintf(buf, "Job submission failed\n");
         goto error;
      }

      if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
         printf("___QALTERED_JOB________________________\n");
         lWriteListTo(lp, stdout);
         printf("________________________________________\n");
      }

      alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
      if (!qmonMessageBox(w, alp, 0)) {
         updateJobListCB(w, NULL, NULL);
         XmtMsgLinePrintf(submit_message, "Job %d altered", 
                           (int)lGetUlong(lFirst(lp), JB_job_number));
         XmtMsgLineClear(submit_message, DISPLAY_MESSAGE_DURATION); 
         close_dialog = True;
      } 
      else {
         XmtMsgLinePrintf(submit_message, "Alter Job %d failed",
                           (int)lGetUlong(lFirst(lp), JB_job_number));
         XmtMsgLineClear(submit_message, DISPLAY_MESSAGE_DURATION); 
         close_dialog = False;
      }

      lFreeList(&lp);
      lFreeList(&alp);
      
      if (close_dialog)
         qmonSubmitPopdown(w, NULL, NULL); 
   
   }
   
   /* 
   ** set normal cursor and discard click ahead
   */
   XmtDiscardButtonEvents(w); 
   XmtDisplayDefaultCursor(w);

   DEXIT;
   return;

   error:
      qmonMessageShow(w, True, "%s", buf);
      XmtDisplayDefaultCursor(w);
      DEXIT;
}

/*-------------------------------------------------------------------------*/
/* get the Job Script, extract the sge directives and show them in      */
/* the dialog                                                              */
/*-------------------------------------------------------------------------*/
static void qmonSubmitGetScript(Widget w, XtPointer cld, XtPointer cad)
{
   static char filename[4*BUFSIZ];
   static char directory[4*BUFSIZ];
   static char pattern[BUFSIZ];
   Boolean status = False;
 
   DENTER(GUI_LAYER, "qmonSubmitGetScript");

   status = XmtAskForFilename(w, NULL,
                              "@{Please type or select a filename}",
                              NULL, NULL,
                              filename, sizeof(filename),
                              directory, sizeof(directory),
                              pattern, sizeof(pattern),
                              NULL);

   if (status == True)  {
      qmonSubmitReadScript(w, filename, NULL, 1);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* get the Job Script, extract the sge directives and show them in      */
/* the dialog                                                              */
/*-------------------------------------------------------------------------*/
static void qmonSubmitGetJSVScript(Widget w, XtPointer cld, XtPointer cad)
{
   static char filename[4*BUFSIZ];
   static char directory[4*BUFSIZ];
   static char pattern[BUFSIZ];
   Boolean status = False;
 
   DENTER(GUI_LAYER, "qmonSubmitGetJSVScript");

   status = XmtAskForFilename(w, NULL,
                              "@{Please type or select a filename}",
                              NULL, NULL,
                              filename, sizeof(filename),
                              directory, sizeof(directory),
                              pattern, sizeof(pattern),
                              NULL);

   if (status == True)  {
      dstring tmp_url = DSTRING_INIT;

      sge_dstring_sprintf(&tmp_url, "%s:%s", "script", filename);
      XmtInputFieldSetString(submit_jsv_url, sge_dstring_get_string(&tmp_url));
      sge_dstring_free(&tmp_url);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitReadScript(
Widget w,
String filename,
String merge_script,
int read_defaults 
) {
   lListElem *job = NULL;
   lList *alp = NULL;
   lList *cmdline = NULL;
   char prefix[20];
   String dir_pre;
   SGE_STRUCT_STAT statb;

   u_long32 myuid = ctx->get_uid(ctx);
   u_long32 prog_number = ctx->get_who(ctx);
   const char *username = ctx->get_username(ctx);
   const char *cell_root = ctx->get_cell_root(ctx);
   const char *unqualified_hostname = ctx->get_unqualified_hostname(ctx);
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(GUI_LAYER, "qmonSubmitReadScript");

   if (filename[strlen(filename)-1] == '/' || filename[0] == '\0')  {
      qmonMessageShow(w, True, "Invalid script name '%s'", filename);
      DEXIT;
      return;
   }

   if (ISSET(submit_mode_data.sub_mode, SUBMIT_SCRIPT) &&
       (SGE_STAT(filename, &statb) == -1 || (statb.st_mode & S_IFMT) != S_IFREG)) {
      qmonMessageShow(w, True, "File '%s' does not exist or is no regular file !", filename);
      DEXIT;
      return;
   }

   /*
   ** get the directive prefix
   */

   dir_pre = XmtInputFieldGetString(submit_prefix);
   dir_pre = qmon_trim(dir_pre);
   sge_strlcpy(prefix, dir_pre, sizeof(prefix));

   if (read_defaults) {
      opt_list_append_opts_from_default_files(prog_number, cell_root, username, &cmdline, &alp, environ);
      if (alp) {
         if (qmonMessageBox(w, alp, 0) == -1) {
            lFreeList(&alp);
            DEXIT;
            return;
         }
         lFreeList(&alp);
      }
   }   

   /*
   ** stage one of script file parsing
   */ 
   if (ISSET(submit_mode_data.sub_mode, SUBMIT_SCRIPT)) {
      alp = parse_script_file(prog_number, filename, (prefix[0] ? prefix : NULL), &cmdline, environ, 
                              FLG_HIGHER_PRIOR);
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);

      if (merge_script) {
         lList *cl2 = NULL;
         /*
         ** stage one ana half of script file parsing
         ** merge an additional script in to override settings
         */ 
         alp = parse_script_file(prog_number, merge_script, "", &cl2, environ, 
                                 FLG_HIGHER_PRIOR | FLG_USE_NO_PSEUDOS);
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);

         if (cl2 != NULL) {
            lAddList(cmdline, &cl2);
         }
      }   
   } else {
      alp = parse_script_file(prog_number, filename, (prefix[0] ? prefix : NULL), &cmdline, environ,
                              FLG_HIGHER_PRIOR | FLG_IGNORE_EMBEDED_OPTS); 
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
   } 

   /*
   ** stage two of script file parsing
   */ 
   alp = cull_parse_job_parameter(myuid, username, cell_root, unqualified_hostname, qualified_hostname, cmdline, &job);

   lFreeList(&cmdline);
   qmonMessageBox(w, alp, 0);
   lFreeList(&alp);

   /*
   ** for debugging
   */
   if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
      printf("___PARSED_SCRIPT_+DEFAULTS_____________\n");
      lWriteElemTo(job, stdout);
      printf("________________________________________\n");
   }

   /*
   ** stage three fill the dialog
   */
   qmonCullToSM(job, &SMData, prefix);
   lFreeElem(&job);
   XmtDialogSetDialogValues(submit_layout, &SMData);

   /*
   ** change the resources pixmap icon if necessary
   */
   qmonSubmitChangeResourcesPixmap();

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonInitSMData(
tSMEntry *data 
) {
   DENTER(GUI_LAYER, "qmonInitSMData");
   
   memset((void*)data, 0, sizeof(tSMEntry));
   data->verify_mode = SKIP_VERIFY;
   data->directive_prefix = XtNewString("#$");

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonFreeSMData(
tSMEntry *data 
) {
   DENTER(GUI_LAYER, "qmonFreeSMData");
   
   if (data->job_script) {
      XtFree((char*)data->job_script);
      data->job_script = NULL;
   }

   if (data->job_tasks) {
      XtFree((char*)data->job_tasks);
      data->job_tasks = NULL;
   }


   if (data->job_name) {
      XtFree((char*)data->job_name);
      data->job_name = NULL;
   }

   if (data->project) {
      XtFree((char*)data->project);
      data->project = NULL;
   }

   if (data->ckpt_obj) {
      XtFree((char*)data->ckpt_obj);
      data->ckpt_obj = NULL;
   }


   if (data->directive_prefix) {
      XtFree((char*)data->directive_prefix);
      data->directive_prefix = NULL;
   }

   if (data->cell) {
      XtFree((char*)data->cell);
      data->cell = NULL;
   }

   if (data->account_string) {
      XtFree((char*)data->account_string);
      data->account_string = NULL;
   }   

   if (data->wd_path) {
      XtFree((char*)data->wd_path);
      data->wd_path = NULL;
   }   

   if (data->jsv_url) {
      XtFree((char*)data->jsv_url);
      data->jsv_url = NULL;
   }   

   if (data->binding) {
      XtFree((char*)data->binding);
      data->binding = NULL;
   }   

   if (data->pe) {
      XtFree((char*)data->pe);
      data->pe = NULL;
   }   

   lFreeList(&(data->task_range));

   lFreeList(&(data->job_args));
   
   lFreeList(&(data->shell_list));

   lFreeList(&(data->mail_list));

   lFreeList(&(data->stdoutput_path_list));
   
   lFreeList(&(data->stdinput_path_list));

   lFreeList(&(data->stderror_path_list));

   lFreeList(&(data->hard_resource_list));

   lFreeList(&(data->soft_resource_list));

   lFreeList(&(data->hard_queue_list));

   lFreeList(&(data->soft_queue_list));

   lFreeList(&(data->master_queue_list));

   lFreeList(&(data->hold_jid));

   lFreeList(&(data->hold_jid_ad));

   lFreeList(&(data->env_list));

   lFreeList(&(data->ctx_list));

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/*  the job element to dialog data conversion                              */
/*  we need a valid tSMEntry pointer                                       */
/*-------------------------------------------------------------------------*/
static Boolean qmonCullToSM(
lListElem *jep,
tSMEntry *data,
char *prefix 
) {
   StringConst job_script;
   dstring dyn_job_tasks = DSTRING_INIT;
   char pe_tasks[BUFSIZ];
   StringConst job_name;
   StringConst directive_prefix;
   StringConst account_string;
   StringConst wd_path;
   StringConst pe;
   StringConst project;
   StringConst ckpt_obj;
   const char* tmp_string;
   const char* username = ctx->get_username(ctx);
   const char* qualified_hostname = ctx->get_qualified_hostname(ctx);
   
   DENTER(GUI_LAYER, "qmonCullToSM");

   /*
   ** free any allocated memory
   */
   qmonFreeSMData(data); 
   qmonInitSMData(data);


   /* 
   ** read in default values from a job 
   */
   if ((job_script = (StringConst)lGetString(jep, JB_script_file)))
      data->job_script = XtNewString(job_script);
   else
      data->job_script = NULL;

   if (job_is_array(jep))
      ja_task_list_print_to_string(lGetList(jep, JB_ja_tasks), &dyn_job_tasks);
   tmp_string = (const char *)sge_dstring_get_string(&dyn_job_tasks);
   if (tmp_string && tmp_string[0] != '\0')
      data->job_tasks = XtNewString(tmp_string);

   if ((job_name = (StringConst)lGetString(jep, JB_job_name)))
      data->job_name = XtNewString(job_name);
   else
      data->job_name = NULL;
      
   /*
   ** do we handle the prefix right ?
   */
   if (prefix) 
      data->directive_prefix = XtNewString(prefix);
   else {
      if ((directive_prefix = (StringConst)lGetString(jep, JB_directive_prefix)))
         data->directive_prefix = XtNewString(directive_prefix);
      else
         data->directive_prefix = XtNewString("#$");
   }

#if 0 /* JG: removed JB_cell from job object */     
   if ((cell = lGetString(jep, JB_cell)))
      data->cell = XtNewString(cell);
   else {
      cell = getenv("SGE_CELL");
      if (cell)
         data->cell = XtNewString(cell);
      else
         data->cell = XtNewString("default");
   }
#endif   

   if ((project = (StringConst)lGetString(jep, JB_project)))
      data->project = XtNewString(project);
   else
      data->project = NULL;

   if ((ckpt_obj = (StringConst)lGetString(jep, JB_checkpoint_name)))
      data->ckpt_obj = XtNewString(ckpt_obj);
   else
      data->ckpt_obj = NULL;

   if ((account_string = (StringConst)lGetString(jep, JB_account)))
      data->account_string = XtNewString(account_string);
   else
      data->account_string = NULL;

   data->ar_id = lGetUlong(jep, JB_ar);

   if ((wd_path = (StringConst)lGetString(jep, JB_cwd)))
      data->wd_path = XtNewString(wd_path);
   else
      data->wd_path = NULL;

   data->jsv_url = NULL;

   data->binding = NULL;

   data->shell_list = lCopyUniqNullNone(lGetList(jep, JB_shell_list), PN_host);
   
   data->mail_list = lCopyList("JB_mail_list", lGetList(jep, JB_mail_list));
   if (!data->mail_list) {
      lListElem* entry = lAddElemStr(&(data->mail_list), MR_user, 
                                    username, MR_Type);
      if (entry)
         lSetHost(entry, MR_host, qualified_hostname);
   }

   {
      lList *env_list = lCopyList("blub", lGetList(jep, JB_env_list));
      lList *prefix_vars = NULL;

      var_list_split_prefix_vars(&env_list, &prefix_vars, VAR_PREFIX);
      lFreeList(&prefix_vars);
      data->env_list = env_list;
   }

   data->ctx_list = lCopyList("JB_ctx_list", lGetList(jep, JB_context));

   data->job_args = lCopyList("JB_job_args", lGetList(jep, JB_job_args));

   data->mail_options = MailOptionsToDialog(lGetUlong(jep, JB_mail_options));

   data->stdoutput_path_list = lCopyUniqNullNone(lGetList(jep, JB_stdout_path_list), PN_host);
   data->stdinput_path_list = lCopyUniqNullNone(lGetList(jep, JB_stdin_path_list), PN_host);
   data->stderror_path_list = lCopyUniqNullNone(lGetList(jep, JB_stderr_path_list), PN_host);
   data->merge_output = lGetBool(jep, JB_merge_stderr);
/*    data->reserve = lGetBool(jep, JB_reserve); */
   data->priority = lGetUlong(jep, JB_priority) - BASE_PRIORITY;
   data->task_concurrency = lGetUlong(jep, JB_ja_task_concurrency);
   data->jobshare = lGetUlong(jep, JB_jobshare);
   data->execution_time = lGetUlong(jep, JB_execution_time);
   data->deadline = lGetUlong(jep, JB_deadline);

   data->hard_resource_list = lCopyList("JB_hard_resource_list", 
                                    lGetList(jep, JB_hard_resource_list));
   data->soft_resource_list = lCopyList("JB_soft_resource_list", 
                                    lGetList(jep, JB_soft_resource_list));;

   data->hard_queue_list = lCopyList("JB_hard_queue_list", 
                                    lGetList(jep, JB_hard_queue_list));;

   data->soft_queue_list = lCopyList("JB_soft_queue_list", 
                                    lGetList(jep, JB_soft_queue_list));;

   data->master_queue_list = lCopyList("JB_master_hard_queue_list", 
                                    lGetList(jep, JB_master_hard_queue_list));;

/*   data->hold_jid = lCopyList("JB_jid_predecessor_list", 
                                    lGetList(jep, JB_jid_predecessor_list));; */
   data->hold_jid = lCopyList("JB_jid_request_list", 
                                    lGetList(jep, JB_jid_request_list));; 
                                    
   data->hold_jid_ad = lCopyList("JB_ja_ad_request_list", 
                                    lGetList(jep, JB_ja_ad_request_list)); 

   data->restart = lGetUlong(jep, JB_restart);

   if ((pe = (StringConst)lGetString(jep, JB_pe))) {
      dstring range_string = DSTRING_INIT;

      range_list_print_to_string(lGetList(jep, JB_pe_range), &range_string, true, false, false);
      sprintf(pe_tasks, "%s %s", pe, sge_dstring_get_string(&range_string));  
      sge_dstring_free(&range_string);
      data->pe = XtNewString(pe_tasks);
   }
   else
      data->pe = NULL;

#if FIXME
   if (job_is_array(jep)) {
      data->task_range = lCopyList("JB_ja_structure", 
                                 lGetList(jep, JB_ja_structure));
   }
#endif

   if (lGetNumberOfElem(lGetList(jep, JB_ja_tasks)) &&
       lGetUlong(lFirst(lGetList(jep, JB_ja_tasks)), JAT_hold)) {
      data->hold = 1;
   } else {
      if (range_list_is_id_within(lGetList(jep, JB_ja_n_h_ids), 1)) {
         data->hold = 0;
      } else {
         data->hold = 1;
      }
   }

   data->now = JOB_TYPE_IS_IMMEDIATE(lGetUlong(jep, JB_type));

   data->notify = lGetBool(jep, JB_notify);

   data->reservation = lGetBool(jep, JB_reserve);

   data->verify_mode = lGetUlong(jep, JB_verify_suitable_queues);

   data->checkpoint_attr = 0;
   data->checkpoint_interval = 0;

   
   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
/* we need a valid job element pointer                                     */
/*-------------------------------------------------------------------------*/
static Boolean qmonSMToCull(
tSMEntry *data,
lListElem *jep,
int save 
) {
   int len;
   char *job_script;
   char *s;
   int reduced_job;
   char pe_tasks[BUFSIZ];
   char *pe = NULL;
   char *pe_range = NULL;
   lList *perl = NULL;
   lList *alp = NULL;
   lList *path_alias = NULL;
   const char *cell_root = ctx->get_cell_root(ctx);
   const char *username = ctx->get_username(ctx);
   const char *unqualified_hostname = ctx->get_unqualified_hostname(ctx);
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);
   
   DENTER(GUI_LAYER, "qmonSMToCull");

   /*
    * first of all get job directive prefix 
    */
   reduced_job = !(submit_mode_data.mode != SUBMIT_QALTER_PENDING && submit_mode_data.mode != SUBMIT_QALTER_RUNNING ); 

   if (!reduced_job) {
      if (data->directive_prefix && data->directive_prefix[0] != '\0' && 
         !ISSET(submit_mode_data.sub_mode, SUBMIT_QSH))
         lSetString(jep, JB_directive_prefix, data->directive_prefix);
      else   
         lSetString(jep, JB_directive_prefix, NULL);

      if (!save && !ISSET(submit_mode_data.sub_mode, SUBMIT_QSH)) {
         /* Job Script/Name */
         lSetString(jep, JB_script_file, data->job_script);
         job_script = sge_file2string(data->job_script, &len);
         lSetString(jep, JB_script_ptr, job_script);
         XtFree((char*)job_script);
         lSetUlong(jep, JB_script_size, len);
      }

      if (ISSET(submit_mode_data.sub_mode, SUBMIT_BINARY)) {
         u_long32 type = lGetUlong(jep, JB_type);

         JOB_TYPE_SET_BINARY(type);
         lSetUlong(jep, JB_type, type); 
         lSetString(jep, JB_script_ptr, NULL);
         lSetUlong(jep, JB_script_size, 0);
      }
      if (ISSET(submit_mode_data.sub_mode, SUBMIT_QSH)) {
         u_long32 type = lGetUlong(jep, JB_type);
         JOB_TYPE_SET_QSH(type);
         lSetUlong(jep, JB_type, type); 
      }      
   }

   if (data->job_tasks && data->job_tasks[0] != '\0') {
      /* job with tasks */
      lList *range_list = NULL;

      range_list_parse_from_string(&range_list, &alp, data->job_tasks,
                                   0, 1, INF_NOT_ALLOWED);

      if (alp) {
         qmonMessageBox(qmon_submit, alp, 0);
         lFreeList(&alp);
         return False;
      }  
      range_list_sort_uniq_compress(range_list, &alp, false);
      if (lGetNumberOfElem(range_list) > 1) {
         answer_list_add(&alp, MSG_QCONF_ONLYONERANGE, STATUS_ESYNTAX, 0);
         qmonMessageBox(qmon_submit, alp, 0);
         lFreeList(&alp);
         return False; 
      }
 
 
        
      /* initialize JB_ja_structure */
      if (range_list == NULL) {
         job_set_submit_task_ids(jep, 1, 1, 1);
      } else {
         if (!reduced_job) {
            lSetList(jep, JB_ja_structure, range_list);
         }
         {
            u_long32 job_type = lGetUlong(jep, JB_type);
            JOB_TYPE_SET_ARRAY(job_type);
            lSetUlong(jep, JB_type, job_type);
         } 
      } 

      if (!reduced_job) {
         job_initialize_id_lists(jep, NULL);      
      } else {
          lList *jat_list = NULL;
          lAddElemUlong(&jat_list, JAT_task_number, 0, JAT_Type);
          lSetList(jep, JB_ja_tasks, jat_list);
      }
   }
   else {   
      if (reduced_job) {
         /* ordinary job */
         lList *jat_list = NULL;
         lSetList(jep, JB_ja_structure, NULL);
         lAddElemUlong(&jat_list, JAT_task_number, 0, JAT_Type);
         lSetList(jep, JB_ja_tasks, jat_list);
      } else {
         /* ordinary job */
         lList *jat_list = NULL;
         lSetList(jep, JB_ja_structure, NULL);
         lAddElemUlong(&jat_list, JAT_task_number, 1, JAT_Type);
         lSetList(jep, JB_ja_tasks, jat_list);
      }  
 
      if (!reduced_job) {
         job_set_submit_task_ids(jep, 1, 1, 1);
         job_initialize_id_lists(jep, NULL); 
      }
   }

   if (!data->job_name || data->job_name[0] == '\0') {
      if (data->job_script) {
         s = strrchr(data->job_script, '/');
         lSetString(jep, JB_job_name, s ? ++s : data->job_script); 
      }
      else
         lSetString(jep, JB_job_name, NULL);
   }
   else
      lSetString(jep, JB_job_name, data->job_name);
   
   lSetString(jep, JB_project, data->project);
   lSetString(jep, JB_checkpoint_name, data->ckpt_obj);

   /* 
    * Here would be the building of script filled job elem, these
    * entries are overruled by the dialog
    */
      
   /* environment */
   if (!reduced_job) {
      /*
      ** path aliasing
      */
      if (path_alias_list_initialize(&path_alias, &alp, (StringConst)cell_root, (StringConst)username, 
                                     (StringConst)qualified_hostname) == -1) {
         if (alp) {
            qmonMessageBox(qmon_submit, alp, 0);
            lFreeList(&alp);
            DEXIT;
            return False;
         }   
      }
      job_initialize_env(jep, &alp, path_alias, unqualified_hostname, qualified_hostname);
      if (alp) {
         qmonMessageBox(qmon_submit, alp, 0);
         lFreeList(&alp);
         DEXIT;
         return False;
      }
   }

   /* 
    * process the resources from dialog
    */ 
   lSetUlong(jep, JB_priority, data->priority + (u_long32)BASE_PRIORITY);
   lSetUlong(jep, JB_ja_task_concurrency, data->task_concurrency);
   lSetUlong(jep, JB_jobshare, data->jobshare);
   lSetUlong(jep, JB_execution_time, data->execution_time);
   lSetBool(jep, JB_merge_stderr, data->merge_output);
   lSetBool(jep, JB_notify, data->notify);
   lSetBool(jep, JB_reserve, data->reservation);
   lSetUlong(jep, JB_restart, data->restart);
   lSetUlong(jep, JB_deadline, data->deadline);
   {
      u_long32 jb_now = lGetUlong(jep, JB_type);
      if(data->now) {
         JOB_TYPE_SET_IMMEDIATE(jb_now);
      } else {
         JOB_TYPE_CLEAR_IMMEDIATE(jb_now);
      }
      lSetUlong(jep, JB_type, jb_now);
   }   
   lSetUlong(jep, JB_ar, data->ar_id);

   if (data->cwd) {
      const char *env_value = job_get_env_string(jep, VAR_PREFIX "O_HOME");
      lSetString(jep, JB_cwd, cwd_string(env_value));
      lSetList(jep, JB_path_aliases, lCopyList("PathAliases", path_alias));
   } else {
      if (data->wd_path) {
         char *wdp = qmon_trim(data->wd_path);
         if (wdp[0] != '\0') {
            lSetString(jep, JB_cwd, wdp);
            lSetList(jep, JB_path_aliases, lCopyList("PathAliases", path_alias));
         } else {
            lSetString(jep, JB_cwd, NULL);
         }   
      }
   }   
   lFreeList(&path_alias);

   /* job submition verifier */
   if (data->jsv_url) {
      char *jsv_script = qmon_trim(data->jsv_url);
      if (jsv_script[0] != '\0') {
         const char *name = "qmon_cmdline";

         jsv_list_remove(name, JSV_CONTEXT_CLIENT);
         jsv_list_add(name, JSV_CONTEXT_CLIENT, &alp, jsv_script);
      }   
   }

   /* core binding parameters */
   if (data->binding) {
      lListElem *binding_elem = lCreateElem(BN_Type);
      dstring binding_string = DSTRING_INIT;

      sge_dstring_append(&binding_string, qmon_trim(data->binding));
      if (binding_parse_from_string(binding_elem, &alp, &binding_string)) {
         lList *binding_list = lCreateList("", BN_Type);

         lAppendElem(binding_list, binding_elem);
         lSetList(jep, JB_binding, binding_list);
      }
      sge_dstring_free(&binding_string);
   }

   lSetString(jep, JB_account, data->account_string);
 
   /* default mailer address */
   if (!data->mail_list && !save) {
      data->mail_list = lCreateElemList("ML", MR_Type, 1);
      if (data->mail_list) {
         lSetString(lFirst(data->mail_list), MR_user, username);
         lSetHost(lFirst(data->mail_list), MR_host, qualified_hostname);
      }
   }
   lSetList(jep, JB_mail_list, lCopyList("ML", data->mail_list));
   DPRINTF(("JB_mail_list %p\n", data->mail_list));
   
   lSetUlong(jep, JB_mail_options, ConvertMailOptions(data->mail_options));

   if (data->hold) {
      lListElem *jap;

      if (reduced_job) {
         lList *ja_tasks = lGetList(jep, JB_ja_tasks);

         if (ja_tasks) {
            if (job_is_array(jep)) {
               if (data->task_range) {
                  lListElem *range;
                  u_long32 start, end, step;

                  for_each (range, data->task_range) {
                     start = lGetUlong(range, RN_min);
                     end = lGetUlong(range, RN_max);
                     step = lGetUlong(range, RN_step);
                     jap = lFirst(lGetList(jep, JB_ja_tasks));
                     for (;start <= end && jap; start += step) {
                        while (jap && 
                               lGetUlong(jap, JAT_task_number) != start) {
                           jap = lNext(jap);
                        }
                        if (jap) {
                           lSetUlong(jap, JAT_hold, 
                                     MINUS_H_CMD_SET|MINUS_H_TGT_USER);
                        }   
                     }
                  }
               } else {
                  lListElem *jap;
                  for_each (jap, lGetList(jep, JB_ja_tasks)) {
                     lSetUlong(jap, JAT_hold, MINUS_H_CMD_SET|MINUS_H_TGT_USER);
                  }
               }
            } else {
               lListElem *ja_task = lFirst(lGetList(jep, JB_ja_tasks));

               lSetUlong(ja_task, JAT_hold, MINUS_H_CMD_SET|MINUS_H_TGT_USER);
            }
         }
      } else {
         lList *task_ids = NULL;
         lList *n_h_ids = NULL;
         lList *u_h_ids = NULL;

         lXchgList(jep, JB_ja_n_h_ids, &task_ids);
         if (data->task_range) {
            range_list_calculate_intersection_set(&u_h_ids, NULL, task_ids,
                                                  data->task_range);
            range_list_calculate_difference_set(&n_h_ids, NULL, 
                                                task_ids, u_h_ids);
         } else {
            u_h_ids = task_ids;
         }
         lXchgList(jep, JB_ja_n_h_ids, &n_h_ids);
         lXchgList(jep, JB_ja_u_h_ids, &u_h_ids);
      }
   } else {
      lListElem *jap;
      for_each (jap, lGetList(jep, JB_ja_tasks)) {
         lSetUlong(jap, JAT_hold, 0);
      }
   }

   lSetUlong(jep, JB_verify_suitable_queues, data->verify_mode);

   DPRINTF(("JB_stdout_path_list %p\n", data->stdoutput_path_list));
   lSetList(jep, JB_stdout_path_list, lCopyUniqNullNone(data->stdoutput_path_list, PN_host));
   
   DPRINTF(("JB_stdin_path_list %p\n", data->stdinput_path_list));
   lSetList(jep, JB_stdin_path_list, lCopyUniqNullNone(data->stdinput_path_list, PN_host));
   
   DPRINTF(("JB_stderr_path_list %p\n", data->stderror_path_list));
   lSetList(jep, JB_stderr_path_list, lCopyUniqNullNone(data->stderror_path_list, PN_host));
   
   DPRINTF(("JB_shell_list %p\n", data->shell_list));
   lSetList(jep, JB_shell_list, lCopyUniqNullNone(data->shell_list, PN_host));
  
   DPRINTF(("JB_env_list %p\n", data->env_list));
   { 
      lList *env_vars = lGetList(jep, JB_env_list);
      lList *prefix_vars = NULL;
   
      var_list_split_prefix_vars(&env_vars, &prefix_vars, VAR_PREFIX);
      lSetList(jep, JB_env_list, lCopyList("env_list", data->env_list));
      lAddList(lGetList(jep, JB_env_list), &prefix_vars);
   }

   DPRINTF(("JB_ctx_list %p\n", data->ctx_list));
   lSetList(jep, JB_context, lCopyList("ctx_list", data->ctx_list));

   DPRINTF(("JB_job_args %p\n", data->job_args));
   lSetList(jep, JB_job_args, lCopyList("job_args", data->job_args));

   DPRINTF(("data->hard_resource_list is %s\n", 
            data->hard_resource_list ? "NOT NULL" : "NULL"));
   lSetList(jep, JB_hard_resource_list, 
               lCopyList("hard_resource_list", data->hard_resource_list));

   DPRINTF(("data->soft_resource_list is %s\n", 
            data->soft_resource_list ? "NOT NULL" : "NULL"));
   lSetList(jep, JB_soft_resource_list, 
               lCopyList("soft_resource_list", data->soft_resource_list));

   DPRINTF(("data->hard_queue_list is %s\n", 
            data->hard_queue_list ? "NOT NULL" : "NULL"));
   lSetList(jep, JB_hard_queue_list, 
               lCopyList("hard_queue_list", data->hard_queue_list));

   DPRINTF(("data->soft_queue_list is %s\n", 
            data->soft_queue_list ? "NOT NULL" : "NULL"));
   lSetList(jep, JB_soft_queue_list, 
               lCopyList("soft_queue_list", data->soft_queue_list));

   DPRINTF(("data->master_queue_list is %s\n", 
            data->master_queue_list ? "NOT NULL" : "NULL"));
   lSetList(jep, JB_master_hard_queue_list, 
               lCopyList("master_hard_queue_list", data->master_queue_list));

   DPRINTF(("data->hold_jid is %s\n", 
            data->hold_jid ? "NOT NULL" : "NULL"));
/*   lSetList(jep, JB_jid_predecessor_list, 
               lCopyList("JB_jid_predecessor_list", data->hold_jid));*/
   lSetList(jep, JB_jid_request_list, 
               lCopyList("JB_jid_request_list", data->hold_jid));
   
   DPRINTF(("data->hold_jid_ad is %s\n", 
            data->hold_jid_ad ? "NOT NULL" : "NULL"));

   lSetList(jep, JB_ja_ad_request_list, 
               lCopyList("JB_ja_ad_request_list", data->hold_jid_ad));

   DPRINTF(("data->pe is %s\n", data->pe ? data->pe: "NULL"));
   if (data->pe && data->pe[0] != '\0') {
      strcpy(pe_tasks, data->pe);
      pe = strtok(pe_tasks, " ");
      pe_range =  strtok(NULL, "\n");
      range_list_parse_from_string(&perl, &alp, pe_range,
                                   0, 0, INF_ALLOWED);
      if (pe && perl && !alp) { 
         lSetString(jep, JB_pe, pe);
         lSetList(jep, JB_pe_range, perl);
      }
      else {
         lFreeList(&alp);
         return False;
      }
   }

   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
static String cwd_string(
StringConst sge_o_home 
) {
   static char cwd_str[BUFSIZ];
   char cwd_str2[BUFSIZ];
   char cwd_str3[BUFSIZ];
   
   DENTER(GUI_LAYER, "cwd_string");

   /*
   ** See src/parse.c, it seems a bit complicated to me
   */
   if (!getcwd(cwd_str, sizeof(cwd_str))) {
      DPRINTF(("getcwd failed\n"));
      cwd_str[0] = '\0';
   }
   if (!sge_chdir(sge_o_home)) {
      if (!getcwd(cwd_str2, sizeof(cwd_str2))) {
         DPRINTF(("getcwd failed\n"));
         cwd_str[0] = '\0';
      }
      errno = 0;
      if (sge_chdir(cwd_str))
         /* fixme: should this do something better? */
         DPRINTF((MSG_FILE_CHDIR_SS, cwd_str, strerror(errno)));
      if (!strncmp(cwd_str2, cwd_str, strlen(cwd_str2))) {
         sprintf(cwd_str3, "%s%s", sge_o_home, 
                  (char *) cwd_str + strlen(cwd_str2));
         strcpy(cwd_str, cwd_str3);
      }
   }
   if (cwd_str[0] != '\0') {
      DEXIT;
      return cwd_str;
   }
   else {
      DEXIT;
      return NULL;
   }
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitCheckJobName(Widget w, XtPointer cld, XtPointer cad)
{
   XmtInputFieldCallbackStruct *cbs = (XmtInputFieldCallbackStruct*)cad;
   
   DENTER(GUI_LAYER, "qmonSubmitCheckJobName");
   if (!(cbs->input) || (cbs->input[0] == '\0')) {
      qmonMessageShow(w, True, "@{No Job Name specified !}");
      cbs->okay = False;
      DEXIT;
      return;
   }

   if (strchr(cbs->input, '/')) {
      qmonMessageShow(w, True, "@{Job Name must not contain / !}");
      cbs->okay = False;
   } 

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonSubmitCheckInput(Widget w, XtPointer cld, XtPointer cad)
{
   XmtInputFieldCallbackStruct *cbs = (XmtInputFieldCallbackStruct*)cad;
   
   DENTER(GUI_LAYER, "qmonSubmitCheckInput");

   if (!cbs->input || *(cbs->input) == '\0')
      qmonSubmitClear(w, NULL, NULL);

   DEXIT;
}

#if 0
/*-------------------------------------------------------------------------*/
static void qmonSubmitCommitInput(Widget w, XtPointer cld, XtPointer cad)
{
   char *cbs = (char *)cad;

   DENTER(GUI_LAYER, "qmonSubmitCommitInput");

   if (!cbs || *(cbs) == '\0') {
      qmonSubmitClear(w, NULL, NULL);
   }

   DEXIT;
}
#endif

	
/*-------------------------------------------------------------------------*/
static u_long32 ConvertMailOptions(
int mail_options 
) {
   u_long32 out = 0;
   int i;
   static u_long32 mail_at[] = { MAIL_AT_BEGINNING, MAIL_AT_EXIT, 
                                 MAIL_AT_ABORT, MAIL_AT_SUSPENSION};
   
   DENTER(GUI_LAYER, "ConvertMailOptions");

   if (mail_options) {
      for (i=0; i<4; i++) {
         if ((mail_options & (1<<i)) == (1<<i))
            out |= mail_at[i]; 
      }
   }
   else
      out = NO_MAIL;
      
   DPRINTF(("mail_options = 0x%08x\n", out));

   DEXIT;
   return out;
}

/*-------------------------------------------------------------------------*/
static int MailOptionsToDialog(
u_long32 mail_options 
) {
   int out = 0;
   int i;
   static u_long32 mail_at[] = { MAIL_AT_BEGINNING, MAIL_AT_EXIT, 
                                 MAIL_AT_ABORT, MAIL_AT_SUSPENSION};
   
   DENTER(GUI_LAYER, "ConvertMailOptions");

   if (mail_options) {
      for (i=0; i<4; i++) {
         if ((mail_options & mail_at[i]))
            out += (1<<i);
      }
   }
      
   DPRINTF(("mail_options = 0x%08x, out = %d\n", mail_options, out));

   DEXIT;
   return out;
}



/*-------------------------------------------------------------------------*/
static void qmonSubmitEdit(Widget w, XtPointer cld, XtPointer cad)
{
   char *script_name;
   char *file;
   int status;
   
   DENTER(GUI_LAYER, "qmonSubmitEdit");
   
   script_name = XmtInputFieldGetString(submit_script);

   if (script_name) {
      file = strdup(script_name);
      status = qmonForkEditor(file);
      sge_free(&file);
   }
   else
      status = qmonForkEditor(NULL);

   if (status != 0) {
      qmonMessageShow(w, True, "@{Cannot start editor !}");
   }

   DEXIT;
}


/*-------------------------------------------------------------------------*/
/* sub dialogs shell/mail/stderr/stdout list                               */
/*-------------------------------------------------------------------------*/
static void qmonSubmitCreateDialogs(
Widget w 
) {
   Widget matrix, reset, cancel, okay, delete, env_list_env, env_list_rm,
         mail_list_new, ctx_list_rm;

   DENTER(GUI_LAYER, "qmonSubmitCreateDialogs");

   /* create subdialogs */

   if (!shell_list_w) {
      shell_list_w = XmtBuildQueryDialog( w, 
                                       "submit_shell_list_shell", 
                                       shell_list_resources, 
                                       XtNumber(shell_list_resources), 
                                       "shell_list_matrix", &matrix, 
                                       "shell_list_cancel", &cancel,
                                       "shell_list_okay", &okay,
                                       "shell_list_reset", &reset,
                                       NULL
                                       );

      XtAddCallback(reset, XmNactivateCallback, 
                     qmonSubmitReset, (XtPointer) shell_list_w);
      XtAddCallback(okay, XmNactivateCallback,
                     qmonSubmitOkay, (XtPointer) shell_list_w);
      XtAddCallback(cancel, XmNactivateCallback,
                     qmonSubmitCancel, (XtPointer) shell_list_w);

   }

   if (!mail_list_w) {
      mail_list_w = XmtBuildQueryDialog( w, 
                                       "submit_mail_list_shell", 
                                       mail_list_resources, 
                                       XtNumber(mail_list_resources), 
                                       "mail_list_matrix", &matrix, 
                                       "mail_list_cancel", &cancel,
                                       "mail_list_okay", &okay,
                                       "mail_list_reset", &reset,
                                       "mail_list_delete", &delete,
                                       "mail_list_new", &mail_list_new,
                                       NULL
                                       );
      XtRealizeWidget(mail_list_w);

      XtAddCallback(reset, XmNactivateCallback, 
                     qmonSubmitReset, (XtPointer) mail_list_w);
      XtAddCallback(okay, XmNactivateCallback,
                     qmonSubmitOkay, (XtPointer) mail_list_w);
      XtAddCallback(cancel, XmNactivateCallback,
                     qmonSubmitCancel, (XtPointer) mail_list_w);
      XtAddCallback(delete, XmNactivateCallback,
                     qmonSubmitDelete, (XtPointer) matrix);
      XtAddCallback(mail_list_new, XmtNinputCallback,
                     qmonSubmitMailInput, (XtPointer) matrix);

   }

   if (!stdoutput_list_w) {
      stdoutput_list_w = XmtBuildQueryDialog( w, 
                                       "submit_stdoutput_list_shell", 
                                       stdoutput_list_resources, 
                                       XtNumber(stdoutput_list_resources), 
                                       "stdoutput_list_matrix", &matrix, 
                                       "stdoutput_list_cancel", &cancel,
                                       "stdoutput_list_okay", &okay,
                                       "stdoutput_list_reset", &reset,
                                       NULL
                                       );
      XtAddCallback(reset, XmNactivateCallback, 
                     qmonSubmitReset, (XtPointer) stdoutput_list_w);
      XtAddCallback(okay, XmNactivateCallback,
                     qmonSubmitOkay, (XtPointer) stdoutput_list_w);
      XtAddCallback(cancel, XmNactivateCallback,
                     qmonSubmitCancel, (XtPointer) stdoutput_list_w);

   }
   
   if (!stdinput_list_w) {
      stdinput_list_w = XmtBuildQueryDialog( w, 
                                       "submit_stdinput_list_shell", 
                                       stdinput_list_resources, 
                                       XtNumber(stdinput_list_resources), 
                                       "stdinput_list_matrix", &matrix, 
                                       "stdinput_list_cancel", &cancel,
                                       "stdinput_list_okay", &okay,
                                       "stdinput_list_reset", &reset,
                                       NULL
                                       );
      XtAddCallback(reset, XmNactivateCallback, 
                     qmonSubmitReset, (XtPointer) stdinput_list_w);
      XtAddCallback(okay, XmNactivateCallback,
                     qmonSubmitOkay, (XtPointer) stdinput_list_w);
      XtAddCallback(cancel, XmNactivateCallback,
                     qmonSubmitCancel, (XtPointer) stdinput_list_w);

   }

   if (!stderror_list_w) {
      stderror_list_w = XmtBuildQueryDialog( w, 
                                       "submit_stderror_list_shell", 
                                       stderror_list_resources, 
                                       XtNumber(stderror_list_resources), 
                                       "stderror_list_matrix", &matrix, 
                                       "stderror_list_cancel", &cancel,
                                       "stderror_list_okay", &okay,
                                       "stderror_list_reset", &reset,
                                       NULL
                                       );
      XtAddCallback(reset, XmNactivateCallback, 
                     qmonSubmitReset, (XtPointer) stderror_list_w);
      XtAddCallback(okay, XmNactivateCallback,
                     qmonSubmitOkay, (XtPointer) stderror_list_w);
      XtAddCallback(cancel, XmNactivateCallback,
                     qmonSubmitCancel, (XtPointer) stderror_list_w);

   }

   if (!env_list_w) {
      env_list_w = XmtBuildQueryDialog( w, 
                                       "submit_env_list_shell", 
                                       env_list_resources, 
                                       XtNumber(env_list_resources), 
                                       "env_list_matrix", &matrix,
                                       "env_list_cancel", &cancel,
                                       "env_list_okay", &okay,
                                       "env_list_env", &env_list_env,
                                       "env_list_rm", &env_list_rm,
                                       NULL
                                       );
      XtAddCallback(okay, XmNactivateCallback,
                     qmonSubmitOkay, (XtPointer) env_list_w);
      XtAddCallback(cancel, XmNactivateCallback,
                     qmonSubmitCancel, (XtPointer) env_list_w);
      XtAddCallback(env_list_env, XmNactivateCallback,
                     qmonSubmitGetEnv, (XtPointer) matrix);
      XtAddCallback(env_list_rm, XmNactivateCallback,
                     qmonSubmitClearCtxEnv, (XtPointer) matrix);

   }

   if (!ctx_list_w) {
      ctx_list_w = XmtBuildQueryDialog( w,
                                       "submit_ctx_list_shell",
                                       ctx_list_resources,
                                       XtNumber(ctx_list_resources),
                                       "ctx_list_matrix", &matrix,
                                       "ctx_list_cancel", &cancel,
                                       "ctx_list_okay", &okay,
                                       "ctx_list_rm", &ctx_list_rm,
                                       NULL
                                       );
      XtAddCallback(okay, XmNactivateCallback,
                     qmonSubmitOkay, (XtPointer) ctx_list_w);
      XtAddCallback(cancel, XmNactivateCallback,
                     qmonSubmitCancel, (XtPointer) ctx_list_w);
      XtAddCallback(ctx_list_rm, XmNactivateCallback,
                     qmonSubmitClearCtxEnv, (XtPointer) matrix);

   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitAskForPE(Widget w, XtPointer cld, XtPointer cad)
{
   Boolean status = False;
   lList *pel = NULL;
   lListElem *pep = NULL;
   int n, i;
   StringConst *strs = NULL;
   static char buf[BUFSIZ];
   
   DENTER(GUI_LAYER, "qmonSubmitAskForPE");
   
   pel = qmonMirrorList(SGE_PE_LIST);
   n = lGetNumberOfElem(pel);
   if (n>0) {
      strs = (StringConst*)XtMalloc(sizeof(String)*n); 
      for (pep=lFirst(pel), i=0; i<n; pep=lNext(pep), i++) {
        /*
        ** we get only references don't free, the strings
        */
        strs[i] = (StringConst)lGetString(pep, PE_name);
      }
    
      strcpy(buf, "");
      /* FIX_CONST_GUI */
      status = XmtAskForItem(w, NULL, "@{Select a Parallel Environment}",
                        "@{Available Parallel Environments}", 
                        (String*) strs, n, False, buf, BUFSIZ, NULL); 
      
      if (status) {
         strcat(buf, " 1");
         XmtInputFieldSetString(submit_pe, buf);
         XmProcessTraversal(submit_pe, XmTRAVERSE_CURRENT);
/*          XmTextSetHighlight(submit_pe,  */
/*                             XmTextGetLastPosition(submit_pe) - 1,  */
/*                             XmTextGetLastPosition(submit_pe), */
/*                             XmHIGHLIGHT_SELECTED); */
         XmTextSetInsertionPosition(submit_pe,
                                    XmTextGetLastPosition(submit_pe));
      }
      /*
      ** don't free referenced strings, they are in the pel list
      */
      XtFree((char*)strs);
   }
   else
      qmonMessageShow(w, True, 
            "@{Please configure a Parallel Environment first !}");
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitAskForAR(Widget w, XtPointer cld, XtPointer cad)
{
   Boolean status = False;
   lList *arl = NULL;
   lListElem *cep = NULL;
   int n, i;
   StringConst *strs = NULL;
   static char buf[BUFSIZ];
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonSubmitAskForAR");
   
   qmonMirrorMultiAnswer(AR_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   arl = qmonMirrorList(SGE_AR_LIST);
   n = lGetNumberOfElem(arl);
   if (n>0) {
      char name[BUFSIZ];
      strs = (StringConst*)XtMalloc(sizeof(String)*n); 
      for (cep=lFirst(arl), i=0; i<n; cep=lNext(cep), i++) {
        snprintf(name, BUFSIZ-1, sge_u32, lGetUlong(cep, AR_id)); 
        /*
        ** we get only references don't free, the strings
        */
        strs[i] = strdup(name);
      }
    
      strcpy(buf, "");
      /* FIX_CONST_GUI */
      status = XmtAskForItem(w, NULL, "@{Select an Advance Reservation}",
                        "@{Available Advance Reservations}", 
                        (String*) strs, n, False, buf, BUFSIZ, NULL); 
      
      if (status) {
         XmtInputFieldSetString(submit_ar, buf);
      }
      /*
      ** free referenced strings, they are strdup'ed from AR_id
      */
      for (i=0; i<n; i++) {
         XtFree((char*)strs[i]);
      }   
      XtFree((char*)strs);
   }
   else
      qmonMessageShow(w, True, 
            "@{Please submit an Advance Reservation first !}");
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitAskForCkpt(Widget w, XtPointer cld, XtPointer cad)
{
   Boolean status = False;
   lList *ckptl = NULL;
   lListElem *cep = NULL;
   int n, i;
   StringConst *strs = NULL;
   static char buf[BUFSIZ];
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonSubmitAskForCkpt");
   
   qmonMirrorMultiAnswer(CKPT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   ckptl = qmonMirrorList(SGE_CK_LIST);
   n = lGetNumberOfElem(ckptl);
   if (n>0) {
      strs = (StringConst*)XtMalloc(sizeof(String)*n); 
      for (cep=lFirst(ckptl), i=0; i<n; cep=lNext(cep), i++) {
        /*
        ** we get only references don't free, the strings
        */
        strs[i] = (StringConst)lGetString(cep, CK_name);
      }
    
      strcpy(buf, "");
      /* FIX_CONST_GUI */
      status = XmtAskForItem(w, NULL, "@{Select a checkpoint object}",
                        "@{Available checkpoint objects}", 
                        (String*) strs, n, False, buf, BUFSIZ, NULL); 
      
      if (status) {
         XmtInputFieldSetString(submit_ckpt_obj, buf);
      }
      /*
      ** don't free referenced strings, they are in the pel list
      */
      XtFree((char*)strs);
   }
   else
      qmonMessageShow(w, True, 
            "@{Please configure a checkpoint object first !}");
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitAskForProject(Widget w, XtPointer cld, XtPointer cad)
{
   Boolean status = False;
   lList *pl = NULL;
   lListElem *cep = NULL;
   int n, i;
   StringConst *strs = NULL;
   static char buf[BUFSIZ];
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonSubmitAskForProject");
   
   qmonMirrorMultiAnswer(PROJECT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   pl = qmonMirrorList(SGE_PR_LIST);
   n = lGetNumberOfElem(pl);
   if (n>0) {
      strs = (StringConst*)XtMalloc(sizeof(String)*n); 
      for (cep=lFirst(pl), i=0; i<n; cep=lNext(cep), i++) {
        /*
        ** we get only references don't free, the strings
        */
        strs[i] = (StringConst)lGetString(cep, PR_name);
      }
    
      strcpy(buf, "");
      /* FIX_CONST_GUI */
      status = XmtAskForItem(w, NULL, "@{Select a project}",
                        "@{Available projects}", (String*) strs, n,
                        False, buf, BUFSIZ, NULL); 
      
      if (status) {
         XmtInputFieldSetString(submit_project, buf);
      }
      /*
      ** don't free referenced strings, they are in the pl list
      */
      XtFree((char*)strs);
   }
   else
      qmonMessageShow(w, True, 
            "@{Please configure a project first !}");
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonSubmitShellList(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonSubmitShellList");
   /* 
   ** get the values from the dialog fields, if there have been entries
   ** in the main dialog get them to set them in the subdialog
   */
   XmtDialogGetDialogValues(submit_layout, &SMData);
   
   /*
   ** set the entries in the helper dialog and pop it up
   */
   XmtDialogSetDialogValues(shell_list_w, &SMData);
   XtManageChild(shell_list_w);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitMailList(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonSubmitMailList");
   /* 
   ** get the values from the dialog fields, if there have been entries
   ** in the main dialog get them to set them in the subdialog
   */
   XmtDialogGetDialogValues(submit_layout, &SMData);
   
   /*
   ** set the entries in the helper dialog and pop it up
   */
   XmtDialogSetDialogValues(mail_list_w, &SMData);
   XtManageChild(mail_list_w);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitMailInput(Widget w, XtPointer cld, XtPointer cad)
{
   Widget list = (Widget) cld;
   String address;
   XmString item;
   
   DENTER(GUI_LAYER, "qmonSubmitMailInput");

   address = XmtInputFieldGetString(w);
   if (address && address[0] != '\0') {
      item = XmStringCreateLtoR(address, XmFONTLIST_DEFAULT_TAG);
      XmListAddItem(list, item, 0);
      XmStringFree(item);
   }
    
   XmtInputFieldSetString(w, "");
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitStderrList(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonSubmitStderrList");
   /* 
   ** get the values from the dialog fields, if there have been entries
   ** in the main dialog get them to set them in the subdialog
   */
   XmtDialogGetDialogValues(submit_layout, &SMData);
   
   /*
   ** set the entries in the helper dialog and pop it up
   */
   
   XmtDialogSetDialogValues(stderror_list_w, &SMData);
   XtManageChild(stderror_list_w);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitStdoutList(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonSubmitStdoutList");
   /* 
   ** get the values from the dialog fields, if there have been entries
   ** in the main dialog get them to set them in the subdialog
   */
   XmtDialogGetDialogValues(submit_layout, &SMData);
   
   /*
   ** set the entries in the helper dialog and pop it up
   */
   
   XmtDialogSetDialogValues(stdoutput_list_w, &SMData);
   XtManageChild(stdoutput_list_w);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitStdinList(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonSubmitStdinList");
   /* 
   ** get the values from the dialog fields, if there have been entries
   ** in the main dialog get them to set them in the subdialog
   */
   XmtDialogGetDialogValues(submit_layout, &SMData);
   
   /*
   ** set the entries in the helper dialog and pop it up
   */
   
   XmtDialogSetDialogValues(stdinput_list_w, &SMData);
   XtManageChild(stdinput_list_w);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitEnvList(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonSubmitEnvList");
   /* 
   ** get the values from the dialog fields, if there have been entries
   ** in the main dialog get them to set them in the subdialog
   */
   XmtDialogGetDialogValues(submit_layout, &SMData);
   
   /*
   ** set the entries in the helper dialog and pop it up
   */

   XmtDialogSetDialogValues(env_list_w, &SMData);
   XtManageChild(env_list_w);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitCtxList(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonSubmitCtxList");
   /*
   ** get the values from the dialog fields, if there have been entries
   ** in the main dialog get them to set them in the subdialog
   */
   XmtDialogGetDialogValues(submit_layout, &SMData);

   /*
   ** set the entries in the helper dialog and pop it up
   */

   XmtDialogSetDialogValues(ctx_list_w, &SMData);
   XtManageChild(ctx_list_w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitClear(Widget w, XtPointer cld, XtPointer cad)
{

   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(GUI_LAYER, "qmonSubmitClear");
   
   /*
   ** free currently set lists and reset to default values
   */
   qmonFreeSMData(&SMData);
   qmonInitSMData(&SMData);

   XmtDialogSetDialogValues(submit_layout, &SMData);

   /*
   ** reset sensitivity of stderr
   */
   if (!ISSET(submit_mode_data.sub_mode, SUBMIT_QSH)) {
      XtSetSensitive(submit_stderror, True);
      XtSetSensitive(submit_stderrorPB, True);
   } else {
      char buf[512];
      String dsp = NULL;

      XmtInputFieldSetString(submit_name, "INTERACTIVE"); 
      dsp = DisplayString(XtDisplay(w));
      if (!strcmp(dsp, ":0") || !strcmp(dsp, ":0.0"))
         sprintf(buf, "DISPLAY=%s%s", qualified_hostname, dsp); 
      else
         sprintf(buf, "DISPLAY=%s", dsp); 
      XmtInputFieldSetString(submit_env, buf); 
      XmToggleButtonSetState(submit_now, 1, True);
   }


   /*
   ** change the resources pixmap icon if necessary
   */
   qmonSubmitChangeResourcesPixmap();

   /*
   ** clear message line
   */
   XmtMsgLineClear(submit_message, XmtMsgLineNow); 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitReload(Widget w, XtPointer cld, XtPointer cad)
{
   String filename;

   DENTER(GUI_LAYER, "qmonSubmitReload");

   /*
   ** if a script is specified reload default files and script
   */
   filename = XmtInputFieldGetString(submit_script);

   if (filename && filename[0] != '\0')
      qmonSubmitReadScript(w, filename, NULL, 1);
   else   
      qmonSubmitClear(w, NULL, NULL);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonSubmitReset(Widget w, XtPointer cld, XtPointer cad)
{
   Widget dialog = (Widget)cld;
   
   DENTER(GUI_LAYER, "qmonSubmitReset");

   XmtDialogSetDialogValues(dialog, &SMData);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitDelete(Widget w, XtPointer cld, XtPointer cad)
{
   Widget list = (Widget)cld;
   XmString *selectedItems;
   Cardinal selectedItemCount;
   Cardinal itemCount;
   
   DENTER(GUI_LAYER, "qmonSubmitDelete");

   XtVaGetValues( list,
                  XmNselectedItemCount, &selectedItemCount,
                  XmNselectedItems, &selectedItems,
                  XmNitemCount, &itemCount,
                  NULL);

   if (selectedItemCount == 0) {
      DEXIT;
      return;
   }

   if (selectedItemCount == itemCount) {
      XmtDisplayWarning(w, "onemailaddress", "There must be at least one mail address.");
      DEXIT;
      return;
   }

   XmListDeleteItems(list, selectedItems, selectedItemCount);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitClearCtxEnv(Widget w, XtPointer cld, XtPointer cad)
{
   Widget matrix = (Widget)cld;
   
   DENTER(GUI_LAYER, "qmonSubmitClearCtxEnv");

   XtVaSetValues( matrix,
                  XmNcells, NULL,
                  NULL);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitGetEnv(Widget w, XtPointer cld, XtPointer cad)
{
   Widget matrix = (Widget) cld;

   char **env = environ;
   int row;
   int max_rows;
   String variable, value;
   String temp;
   char *p;
   
   DENTER(GUI_LAYER, "qmonSubmitGetEnv");

   XtVaSetValues( matrix,
                  XmNcells, NULL,
                  NULL);

   XtVaGetValues(matrix, XmNrows, &max_rows, NULL);

   for(row=0;*env;env++, row++) {
      temp = strdup(*env);
      variable = temp;
      p = strchr(temp, '=');
      *p='\0';
      value = ++p;
      if (row == max_rows) {
         XbaeMatrixAddRows(matrix, 
                           max_rows, 
                           NULL,       /* empty rows  */
                           NULL,       /* no lables   */
                           NULL,       /* no different colors */
                           10);         /* we add 10 rows      */
         max_rows += 10;
      }
      XbaeMatrixSetCell(matrix, row, 0, variable ? variable : "");
      XbaeMatrixSetCell(matrix, row, 1, value ? value : "");
      DPRINTF(("[%.3d] %s\n", row, *env));
      XtFree((char*)temp);
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitOkay(Widget w, XtPointer cld, XtPointer cad)
{
   Widget dialog = (Widget)cld;
   
   DENTER(GUI_LAYER, "qmonSubmitOkay");

   /*
   ** write the values from the subdialog into SMData
   */
   XmtDialogGetDialogValues(dialog, &SMData);
    
#if FIXME
   if ((SMData.pe && !SMData.pe_range) || (!SMData.pe && SMData.pe_range)) {
      qmonMessageShow(w, True, "Range required for Parallel Environment !");
      DEXIT;
      return;
   }
#endif

   /*
   ** set the new SMData into the main submit dialog
   */
   XmtDialogSetDialogValues(submit_layout, &SMData);

   xmui_unmanage(dialog);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitCancel(Widget w, XtPointer cld, XtPointer cad)
{
   Widget dialog = (Widget)cld;
   
   DENTER(GUI_LAYER, "qmonSubmitCancel");

   xmui_unmanage(dialog);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* change the resources pixmap                                             */
/*-------------------------------------------------------------------------*/
static void qmonSubmitChangeResourcesPixmap(void) 
{
   static Pixmap pix1 = None, pix2 = None;
   static int have_resources = 0;

   DENTER(GUI_LAYER, "qmonSubmitChangeResourcesPixmap");

   if (pix1 == None || pix2 == None) {
      pix1 = qmonGetIcon("resources");
      pix2 = qmonGetIcon("resources_enabled");
      if (pix1 == None || pix2 == None)
         DPRINTF(("Pixmap can't be loaded in qmonSubmitChangeResourcesPixmap\n"));
   }
  
   if (SMData.hard_resource_list || SMData.soft_resource_list) {
      if (!have_resources) {
         have_resources = 1;
         XtVaSetValues(submit_resources, XmNlabelPixmap, pix2, NULL);
      }
   }
   else {
      if (have_resources) {
         have_resources = 0;
         XtVaSetValues(submit_resources, XmNlabelPixmap, pix1, NULL);
      }
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSubmitToggleHoldNow(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonToggleHoldNow");

   /* hold selected => deselect now) */
   if (XmToggleButtonGetState(submit_hold) && !SMData.hold) {
      XmToggleButtonSetState(submit_now, 0, False);
      XtSetSensitive(submit_now, False);
   } else {   
      XtSetSensitive(submit_now, True);
   }

   if (XmToggleButtonGetState(submit_now) && !SMData.now) {
      XmToggleButtonSetState(submit_hold, 0, False);
   }   

   if (XmToggleButtonGetState(submit_now)) {
      XtSetSensitive(submit_hold, False);
      XtSetSensitive(submit_reservation, False);
      XmToggleButtonSetState(submit_reservation, 0, False);
   } else {
      XtSetSensitive(submit_hold, True);
      XtSetSensitive(submit_reservation, True);
   }
   
   DEXIT;
}

static lList *lCopyUniqNullNone(lList *lp, int keyfield)
{

   lList *cl = NULL;
   lListElem *ep;
   lListElem *rep;

   DENTER(CULL_LAYER, "lCopyUniqNullNone");

   cl = lCopyList("", lp);
   /*
    * sort the list first to make our algorithm work
    */
   lPSortList(cl, "%I+", keyfield);

   /*
    * go over all elements and remove following elements
    */
   ep = lFirst(cl);
   while (ep) {
      rep = lNext(ep);
      while (rep &&
             ((lGetHost(rep, keyfield) == NULL && lGetHost(ep, keyfield) == NULL) ||
              (lGetHost(rep, keyfield) == NULL && !strcasecmp(lGetHost(ep, keyfield), "none")) ||
              (lGetHost(ep, keyfield) == NULL && !strcasecmp(lGetHost(rep, keyfield), "none")) || 
              (!sge_hostcmp(lGetHost(rep, keyfield), lGetHost(ep, keyfield))))) {
         lRemoveElem(cl, &rep);
         rep = lNext(ep);
      }
      ep = lNext(ep);
   }

   DRETURN(cl);
}   
