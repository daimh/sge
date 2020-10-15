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

#include <Xm/Xm.h>
#include <Xm/Text.h>

#include <Xmt/Xmt.h>
#include <Xmt/MsgLine.h>
#include <Xmt/Dialogs.h>
#include <Xmt/Create.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_message.h"
#include "qmon_file.h"
#include "qmon_appres.h"
#include "qmon_comm.h"
#include "qmon_timer.h"
#include "qmon_util.h"

#include "uti/sge_io.h"
#include "uti/msg_utilib.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_resource_quota.h"

#include "spool/flatfile/sge_flatfile.h"
#include "spool/flatfile/sge_flatfile_obj.h"

#include "gdi/sge_gdi_ctx.h"

#include "sge_resource_quota_qconf.h"
#include "msg_common.h"

extern sge_gdi_ctx_class_t *ctx;

static Widget qmon_rqs = 0;
static Widget rqs_text = 0;

static void qmonRQSOkay(Widget w, XtPointer cld, XtPointer cad);
static void qmonRQSCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonRQSSetText(Widget tw, lList *rqs_list, lList **alpp);
static void qmonRQSGetText(Widget tw, lList *rqs_list, lList **alpp);

/*-------------------------------------------------------------------------*/
void qmonRQSPopup(Widget w, XtPointer cld, XtPointer cad) 
{
   Widget shell, rqs_okay, rqs_cancel;
   lList *alp = NULL;
   lList *rqs_list = NULL;

   DENTER(GUI_LAYER, "qmonRQSPopup");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_rqs) {
      shell = XmtGetTopLevelShell(w);
      qmon_rqs = XmtBuildQueryDialog(shell, "rqs_shell", 
                              NULL, 0,
                              "rqs_text", &rqs_text,
                              "rqs_okay", &rqs_okay,
                              "rqs_cancel", &rqs_cancel,
                              NULL);
      XtAddCallback(rqs_okay, XmNactivateCallback,
                     qmonRQSOkay, NULL); 
      XtAddCallback(rqs_cancel, XmNactivateCallback, 
                     qmonRQSCancel, NULL);
   }


   qmonMirrorMultiAnswer(RQS_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set normal cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }

   rqs_list = qmonMirrorList(SGE_RQS_LIST);

   xmui_manage(qmon_rqs);

   qmonRQSSetText(rqs_text, rqs_list, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
   }

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonRQSOkay(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;
   lList *rqs_list = NULL;

   DENTER(GUI_LAYER, "qmonRQSOkay");
   
   rqs_list = qmonMirrorList(SGE_RQS_LIST);
   qmonRQSGetText(rqs_text, rqs_list, &alp);
   if (answer_list_has_error(&alp)) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }

   xmui_unmanage(qmon_rqs);
   lFreeList(&alp);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonRQSCancel(Widget w, XtPointer cld, XtPointer cad)
{

   DENTER(GUI_LAYER, "qmonRQSCancel");
   
   xmui_unmanage(qmon_rqs);

   DEXIT;
}

static void qmonRQSSetText(Widget tw, lList *rqs_list, lList **alpp)
{
   char *text = NULL;

   DENTER(GUI_LAYER, "qmonRQSSetText");

   if (rqs_list != NULL) {
      const char *filename = NULL;
      
      filename = spool_flatfile_write_list(alpp, rqs_list, RQS_fields,
                                           &qconf_rqs_sfi, 
                                           SP_DEST_TMP, SP_FORM_ASCII,
                                           filename, false);
      text = qmonReadText(filename, alpp);
      unlink(filename);
      sge_free(&filename);
      if (text != NULL) {
         XmTextSetString(tw, text);
         XtFree(text);
      } else {
         XmTextSetString(tw, "");
      }   

   } 
   DEXIT;
}


static void qmonRQSGetText(Widget tw, lList *rqs_list, lList **alpp)
{
   char *text = NULL;
   lList *new_rqs_list = NULL;
   char filename[SGE_PATH_MAX] = "";
   size_t len = 0;
   bool ret = false;
   bool ignore_unchanged_message = false;
   dstring sge_tmpnam_error = DSTRING_INIT;
   int fd;

   DENTER(GUI_LAYER, "qmonRQSGetText");

   if ((fd = sge_mkstemp(filename, sizeof(filename), &sge_tmpnam_error)) < 0) {
      if (sge_dstring_get_string(&sge_tmpnam_error) != NULL) {
         answer_list_add(alpp, sge_dstring_get_string(&sge_tmpnam_error), STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      } else {
         answer_list_add(alpp, MSG_POINTER_NULLPARAMETER, STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
      sge_dstring_free(&sge_tmpnam_error);
      DEXIT;
      return;
   }
   sge_dstring_free(&sge_tmpnam_error);
   close(fd);

   /*
   ** allocates a string
   */
   text = XmTextGetString(tw);
   len = strlen(text);
   ret = sge_string2file(text, len, filename);
   XtFree(text);

   new_rqs_list = spool_flatfile_read_list(alpp, RQS_Type, RQS_fields,
                                            NULL, true, 
                                            &qconf_rqs_sfi,
                                            SP_FORM_ASCII, NULL, filename);
   if (answer_list_output(alpp)) {
      lFreeList(&new_rqs_list);
   }

   if (new_rqs_list != NULL) {
      if (ignore_unchanged_message || object_list_has_differences(new_rqs_list, alpp, rqs_list, false)) {
         ret = true;
      } else {
         lFreeList(&new_rqs_list);
         answer_list_add(alpp, MSG_FILE_NOTCHANGED,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
   } else {
      answer_list_add(alpp, MSG_FILE_ERRORREADINGINFILE,
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   }

   unlink(filename);

   if (ret) {
      ret = rqs_add_del_mod_via_gdi(ctx, new_rqs_list, alpp, SGE_GDI_REPLACE);
   }

   lFreeList(&new_rqs_list);
   DEXIT;
}
