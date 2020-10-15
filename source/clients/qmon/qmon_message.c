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
#include "qmon_browser.h"
#include "qmon_appres.h"

#include "sgeobj/sge_answer.h"

static Widget qmon_msg_box = 0;
static Widget msg_text_w = 0;

static Boolean message_blocked = False;


static Widget qmonMessageCreateMsgBox(Widget parent);
static void qmonMessageOk(Widget w, XtPointer cld, XtPointer cad);

/*-------------------------------------------------------------------------*/
void qmonMessageShow(Widget w, Boolean msg_box, StringConst fmt, ...)
{
   va_list arg_list;
   char buf[BUFSIZ];
   char *l10nfmt;
   
   DENTER(GUI_LAYER, "qmonMessageShow");

   Va_start(arg_list, fmt);

#if 0
   switch (MSG_HOW) {
      case MSG_MSGLINE:
      
         (void) vsprintf(buf, fmt, arg_list);
         XmtMsgLineSet(w, buf);
         XmtMsgLineClear(w, DISPLAY_MESSAGE_DURATION);
         break;
      
      case MSG_BROWSER:
#endif
         l10nfmt = XmtLocalize(w, fmt, fmt);
         (void)vsprintf(buf, l10nfmt, arg_list);
         if (qmonBrowserObjectEnabled(BROWSE_MSG)) {
            qmonBrowserShow(buf);
            qmonBrowserShow("\n");
         }

         if (msg_box) {
            XmtDisplayWarningMsg(w, "XmtMessageBox", "%s", "Warning", NULL, buf);
            XSync(XtDisplay(w), 0);
            XmUpdateDisplay(w);
         }
#if 0         
         break;
         
      default:
         DPRINTF(("No valid Message Type\n"));
   }
         
#endif
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
int qmonMessageBox(
Widget parent,
lList *alp,
int show_always 
) {
   lListElem *aep = NULL;
   XmTextPosition pos;
   Boolean error = False;
   StringConst msg;
   u_long32 as = 0;
   u_long32 aq = 0;
   Boolean show = False;

   DENTER(GUI_LAYER, "qmonMessageBox");

   DPRINTF(("show_always = %d\n", show_always));

   if (!alp || !parent) {
      DEXIT;
      return -1;
   }

   /*
   ** create the Message Box if necessary
   */
   if (!qmon_msg_box) {
      qmon_msg_box = qmonMessageCreateMsgBox(parent);
   }

   /*
   ** fill the message texts into the text field
   */
   XmTextDisableRedisplay(msg_text_w);
   pos = 0;
   XmTextSetString(msg_text_w, "");
   /*
   ** extract messages
   */
   for_each(aep, alp) {
      as = lGetUlong(aep, AN_status);
      aq = lGetUlong(aep, AN_quality);
      msg = lGetString(aep, AN_text);

      if (qmonBrowserObjectEnabled(BROWSE_MSG)) {
         qmonBrowserShow(msg);
         qmonBrowserShow("\n");
      }

      if (!msg)
         continue;
      if (aq == ANSWER_QUALITY_ERROR)
         error = True;
      if ((as != STATUS_OK) || show_always) { 
         show = True;
         /* FIX_CONST_GUI */
         XmTextInsert(msg_text_w, pos, (String)msg);
         pos += strlen(msg);
         XmTextInsert(msg_text_w, pos, "\n");
         pos += strlen("\n");
      }
   }
   XmTextShowPosition(msg_text_w, 0);
   XmTextEnableRedisplay(msg_text_w);
   
   /*
   ** manage the msg_box
   */
   if (show) {
      XtManageChild(qmon_msg_box);
      XSync(XtDisplay(parent), 0);
      XmUpdateDisplay(parent);
      message_blocked = True;
      XmtBlock(qmon_msg_box, &message_blocked);
   }

   
   if (error) {
      DEXIT;
      return -1;
   }

   DEXIT;
   return 0;
}

/*-------------------------------------------------------------------------*/
static Widget qmonMessageCreateMsgBox(
Widget parent 
) {
   Widget msg_layout, msg_ok;

   DENTER(GUI_LAYER, "qmonMessageCreateMsgBox");
   
   msg_layout = XmtBuildQueryDialog( parent, "qmon_msg_box",
                           NULL, 0,
                           "msg_text", &msg_text_w,
                           "msg_ok", &msg_ok,
                           NULL);
   XtAddCallback(msg_ok, XmNactivateCallback,
                  qmonMessageOk, NULL);

   DEXIT;
   return msg_layout;
}


/*-------------------------------------------------------------------------*/
static void qmonMessageOk(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonMessageOk");

   message_blocked = False;

   DEXIT;
}





#if 0
         for_each(aep, alp) {
	    u_long32 quality;

            answer_status = lGetUlong(aep, AN_status);
	    quality = lGetUlong(aep, AN_quality);
            if (quality == ANSWER_QUALITY_ERROR) {
               sprintf(msg, "%s\nPlease correct above errors first !\n", 
                        lGetString(aep, AN_text));
               qmonMessageShow(w, True, msg);
               DEXIT;
               return;
            }
            else if (quality == ANSWER_QUALITY_WARNING) {
               sprintf(msg, "WARNING!\n%s\n", 
                        lGetString(aep, AN_text));
               qmonMessageShow(w, True, msg);
	    }
            else if (quality == ANSWER_QUALITY_INFO) {
               qmonMessageShow(w, True, lGetString(aep, AN_text));
	    }
         }
#endif
