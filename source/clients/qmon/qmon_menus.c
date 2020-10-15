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

#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/MenuShell.h>
#include <Xm/MainW.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include <Xmt/Xmt.h>
#include <Xmt/Menu.h>
#include <Xmt/AppRes.h>
#include <Xmt/Dialogs.h>
#include <Xmt/Help.h>
#include <Xmt/HelpBrowser.h>

#include "qmon_menus.h"
#include "qmon_globals.h"
#include "qmon_rmon.h"
#include "qmon_job.h"
#include "qmon_queue.h"
#include "qmon_cq.h"
#include "qmon_host.h"
#include "qmon_manop.h"
#include "qmon_submit.h"
#include "qmon_browser.h"
#include "qmon_init.h"
#include "qmon_comm.h"
#include "qmon_about.h"
#include "qmon_appres.h"
#include "qmon_cluster.h"
#include "qmon_sconf.h"
#include "qmon_pe.h"
#include "qmon_ckpt.h"
#include "qmon_ticket.h"
#include "qmon_project.h"
#include "qmon_cplx.h"
#include "qmon_calendar.h"
#include "qmon_resource_quota.h"
#include "qmon_ar.h"
#include "qmon_arsub.h"

#include "sgeobj/sge_feature.h"

/*-------------------------------------------------------------------------*/
#ifdef QMON_DEBUG
static void reportWidgets(Widget awidget, FILE *fp);
static void reportWidgetsCB(Widget w, XtPointer cld, XtPointer cad);
static void qmonPointTo(Widget w, XtPointer cld, XtPointer cad);
#endif

static void qmonShowTooltip(Widget w, XtPointer cld, XEvent *ev, Boolean *ctd);
static void qmonHideTooltip(Widget w, XtPointer cld, XEvent *ev, Boolean *ctd);
/* static void qmonHelpBrowserPopup(Widget w, XtPointer cld, XtPointer cad); */

/*-------------------------------------------------------------------------*/
/* Menu items for the main menu bar                                        */
/*-------------------------------------------------------------------------*/

static XmtMenuItem file_menu_items[] = {
                                                 /* callback--v, cld--v */
   {XmtMenuItemPushButton, "@{Exit}", '\0', "Ctrl<Key>C", "Ctrl-C",
	 qmonExitCB, NULL},
   {XmtMenuItemEnd}
};

static XmtMenuItem task_menu_items[] = {
                                                 /* callback--v cld--v */
   {XmtMenuItemPushButton, "@{Job Control}", 'J', "Alt<Key>J", "Alt+J",
         qmonJobPopup, NULL },
   {XmtMenuItemPushButton, "@{Queue Control}", 'Q', "Alt<Key>Q", "Alt+Q",
         qmonCQPopup, NULL},
   {XmtMenuItemPushButton, "@{Job Submit}", 'S', "Alt<Key>S", "Alt+S",
         qmonSubmitPopup, NULL},
   {XmtMenuItemPushButton, "@{Complex Configuration}", 'X', "Alt<Key>X", "Alt+X",
         qmonPopupCplxConfig, NULL},
   {XmtMenuItemPushButton, "@{Host Configuration}", 'O', "Alt<Key>O", "Alt+O",
         qmonPopupHostConfig, NULL},
   {XmtMenuItemPushButton, "@{User Configuration}", 'U', "Alt<Key>U", "Alt+U",
         qmonPopupManopConfig, NULL},
   {XmtMenuItemPushButton, "@{Cluster Configuration}", 'L', "Alt<Key>L", "Alt+L",
         qmonPopupClusterConfig, NULL},
   {XmtMenuItemPushButton, "@{Scheduler Configuration}", 'D', "Alt<Key>D", 
         "Alt+D", qmonPopupSchedConfig, NULL},
   {XmtMenuItemPushButton, "@{PE Configuration}", 'P', "Alt<Key>P", "Alt+P",
         qmonPopupPEConfig, NULL},
   {XmtMenuItemPushButton, "@{Checkpointing Configuration}", 'K', "Alt<Key>K", 
         "Alt+K", qmonPopupCkptConfig, NULL},
   {XmtMenuItemPushButton, "@{Calendar Configuration}", 'A', "Alt<Key>A", "Alt+A",
         qmonPopupCalendarConfig, NULL},
   /*
   ** the 'Ticket Overview' is only managed if SGE_MODE is true
   ** !!!!!! Attention the position is referenced in qmonCreateMainControl
   */
   {XmtMenuItemPushButton, "@{Policy Configuration}", 'I', "Alt<Key>I", "Alt+I",
         qmonPopupTicketOverview, NULL},

   {XmtMenuItemPushButton, "@{Project Configuration}", 'R', "Alt<Key>R", "Alt+R",
         qmonPopupProjectConfig, NULL},

   {XmtMenuItemPushButton, "@{Browser Dialog}", 'B', "Alt<Key>B", "Alt+B",
         qmonBrowserOpen, NULL},
   {XmtMenuItemPushButton, "@{Resource Quota Configuration}", 'L', "Alt<Key>L", "Alt+L",
         qmonRQSPopup, NULL},
   {XmtMenuItemPushButton, "@{Advance Reservation Configuration}", 'V', "Alt<Key>V", "Alt+V",
         qmonARPopup, NULL},
   {XmtMenuItemPushButton, "@{Advance Reservation Submission}", 'U', "Alt<Key>U", "Alt+U",
         qmonARSubPopup, NULL},
   {XmtMenuItemEnd}
};

#ifdef QMON_DEBUG
static XmtMenuItem mirror_menu_items[] = {
   {XmtMenuItemPushButton, "Queue", 'Q', "Alt<Key>J", "Alt+J",
         qmonShowMirrorList, (XtPointer) SGE_CQ_LIST},
   {XmtMenuItemPushButton, "Job", 'S', "Alt<Key>S", "Alt+S",
         qmonShowMirrorList, (XtPointer) SGE_JB_LIST},
   {XmtMenuItemPushButton, "Exechost", 'Q', "Alt<Key>Q", "Alt+Q",
         qmonShowMirrorList, (XtPointer) SGE_EH_LIST},
   {XmtMenuItemPushButton, "Adminhost", 'B', "Alt<Key>B", "Alt+B",
         qmonShowMirrorList, (XtPointer) SGE_AH_LIST},
   {XmtMenuItemPushButton, "Submithost", 'R', "Alt<Key>R", "Alt+R",
         qmonShowMirrorList, (XtPointer) SGE_SH_LIST},
   {XmtMenuItemPushButton, "Complex Attributes", 'O', "Alt<Key>O", "Alt+O",
         qmonShowMirrorList, (XtPointer) SGE_CE_LIST},
   {XmtMenuItemPushButton, "Manager", 'H', "Alt<Key>H", "Alt+H",
         qmonShowMirrorList, (XtPointer) SGE_UM_LIST},
   {XmtMenuItemPushButton, "Operator", 'U', "Alt<Key>U", "Alt+U",
         qmonShowMirrorList, (XtPointer) SGE_UO_LIST},
   {XmtMenuItemPushButton, "Userset", 'U', "Alt<Key>U", "Alt+U",
         qmonShowMirrorList, (XtPointer) SGE_US_LIST},
   {XmtMenuItemSeparator},
   {XmtMenuItemPushButton, "ReportWidgets", 'R', "Alt<Key>R", "Alt+R",
         reportWidgetsCB, NULL},
   {XmtMenuItemPushButton, "Point To Widget", 'R', "Alt<Key>R", "Alt+R",
         qmonPointTo, NULL},
   {XmtMenuItemEnd}
};
#endif


static XmtMenuItem help_menu_items[] = {
   {XmtMenuItemPushButton, "@{Context Help}", 'H', "Ctrl<Key>H", "Ctrl+H",
         (XtCallbackProc) XmtHelpDoContextHelp, NULL},
/*    {XmtMenuItemPushButton, "@{Help Index}", 'E', "Alt<Key>E", "Alt+E", */
/*          qmonHelpBrowserPopup, NULL}, */
/*    {XmtMenuItemPushButton, "@{Netscape Help}", 'N', "Alt<Key>N", "Alt+N", */
   {XmtMenuItemPushButton, "@{About}", 'A', "Ctrl<Key>A", "Ctrl+A",
         qmonAboutMsg, NULL},
   {XmtMenuItemEnd}
};


static XmtMenuItem menubar_items[] = {
   {XmtMenuItemCascadeButton /* + XmtMenuItemTearoff */,
      "@{File}", 'F', NULL, NULL, NULL, NULL, file_menu_items},
   {XmtMenuItemCascadeButton + XmtMenuItemTearoff ,
      "@{Task}", 'T', NULL, NULL, NULL, NULL, task_menu_items},
#ifdef QMON_DEBUG
   {XmtMenuItemCascadeButton /* + XmtMenuItemTearoff */,
      "MirrorList", 'M', NULL, NULL, NULL, NULL, mirror_menu_items},
#endif
   {XmtMenuItemCascadeButton /* + XmtMenuItemTearoff */ + XmtMenuItemHelp,
      "@{Help}", 'H', NULL, NULL, NULL, NULL, help_menu_items}
};


/*-------------------------------------------------------------------------*/

typedef struct _tCallbacksUsed {
   XtCallbackProc    callback;
   XtPointer         cld;
   char              *tooltip;
} tCallbacksUsed;


static tCallbacksUsed callback_array[] = {
   { qmonJobPopup, NULL, "@{@fBJob Control}" },
   { qmonCQPopup, NULL, "@{@fBQueue Control}" },
   { qmonSubmitPopup, NULL, "@{@fBSubmit Jobs}" },
   { qmonPopupCplxConfig, NULL, "@{@fBComplex Configuration}" },
   { qmonPopupHostConfig, NULL, "@{@fBHost Configuration}" },
   { qmonPopupClusterConfig, NULL, "@{@fBCluster Configuration}" },
   { qmonPopupSchedConfig, NULL, "@{@fBScheduler Configuration}" },
   { qmonPopupCalendarConfig, NULL, "@{@fBCalendar Configuration}" },
   { qmonPopupManopConfig, NULL, "@{@fBUser Configuration}" },
   { qmonPopupPEConfig, NULL, "@{@fBParallel Environment Configuration}" },
   { qmonPopupCkptConfig, NULL, "@{@fBCheckpoint Configuration}" },
   { qmonPopupTicketOverview, NULL, "@{@fBPolicy Configuration}" },
   { qmonPopupProjectConfig, NULL, "@{@fBProject Configuration}" },
   { qmonRQSPopup, NULL, "@{@fBResource Quota Configuration}" },
   { qmonARPopup, NULL, "@{@fBAdvance Reservation}" },
   { qmonBrowserOpen, NULL, "@{@fBBrowser}" },
   { qmonExitCB, NULL, "@{@fBExit}" }
};
    
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
Widget qmonCreateMainMenu(
Widget parent 
) {
   Arg args[5];
   int ac;
   Widget MenuBar;
   
   DENTER(GUI_LAYER, "qmonCreateMainMenu");

   ac = 0;
   XtSetArg(args[ac], XmtNitems, menubar_items); ac++;
   XtSetArg(args[ac], XmtNnumItems, XtNumber(menubar_items)); ac++;
   
   MenuBar = XmtCreateMenubar(parent, "MainMenuBar", args, ac);

   DEXIT;
   return MenuBar;
}

/*-------------------------------------------------------------------------*/
Widget qmonCreateMainControl(
Widget parent 
) {
   Widget MainControl = NULL;
   Widget MainRowCol;
   Widget menubar;
   Widget Icon;
   int i;
   static String button_name[] = { 
      "JOB_CONTROL", 
      "QUEUE_CONTROL",
      "SUBMIT_JOB",
      "COMPLEX_CONFIG",
      "HOST_CONFIG",
      "CLUSTER_CONFIG",
      "SCHED_CONFIG",
      "CALENDAR_CONFIG",
      "USER_CONFIG",
      "PE_CONFIG",
      "CKPT_CONFIG",
      "TICKET_OVERVIEW",
      "PROJECT_CONFIG",
      "RQS_CONFIG",
      "AR_CONFIG",
      "BROWSER",
      "EXIT" };
#define AUTOMATIC_MAINBAR   
#ifdef AUTOMATIC_MAINBAR   
   XtWidgetGeometry geometry;
   XtWidgetGeometry menugeo;
#endif

   DENTER(GUI_LAYER, "qmonCreateMainControl");

DTRACE;   
   MainControl = XtVaCreateWidget("MainControl", 
                  xmMainWindowWidgetClass,
                  parent,
                  XmNscrollBarDisplayPolicy, XmAS_NEEDED,
                  XmNscrollingPolicy, XmAUTOMATIC,
                  NULL);

   /* 
   ** Create a menubar 
   */
   menubar = qmonCreateMainMenu(MainControl);

   /* 
   ** Create a RowColumn Widget as the parent of   checkBox with a variable
   ** number of buttons 
   */
   MainRowCol = XtVaCreateWidget( "MainRowCol",
                              xmRowColumnWidgetClass,
                              MainControl,
                              NULL);
   /* 
   ** Create the Icon Label combinations to be displayed on the topmost shell 
   ** Get these entries from the resource or a config file 
   ** DON'T KNOW HOW TO DO IT YET 
   */

   for (i = 0; i < XtNumber(callback_array); i++) {
      /* This is only for demo purposes */
      Icon = XtVaCreateManagedWidget(button_name[i],
                                    xmPushButtonWidgetClass,
                                    MainRowCol,
                                    NULL);
      XtAddCallback(Icon, XmNactivateCallback, 
               callback_array[i].callback , callback_array[i].cld);
      XtAddEventHandler(Icon, EnterWindowMask,
                           False, qmonShowTooltip, 
                           (XtPointer) callback_array[i].tooltip);
   }

   /*
   ** manage menubar and task buttons
   */
   XtManageChild(menubar);
   /*
   ** get the geometry of the MainRowCol widget
   */
#ifdef AUTOMATIC_MAINBAR   
   XtQueryGeometry(MainRowCol, NULL, &geometry);
   XtQueryGeometry(menubar, NULL, &menugeo);
   DPRINTF(("width: %d, height: %d\n", geometry.width, geometry.height));
#endif   
   XtManageChild(MainRowCol);

   XtVaSetValues( MainControl, 
#ifdef AUTOMATIC_MAINBAR   
                  XmNwidth, geometry.width + 5,
                  XmNheight, geometry.height + menugeo.height + 5,
#endif                  
                  XmNmenuBar, menubar, 
                  XmNworkWindow, MainRowCol,
                  NULL);

   XtManageChild(MainControl);
   
   DEXIT;
   return MainControl;
}


/*-------------------------------------------------------------------------*/
Widget qmonCreatePopup(
Widget parent,
char *name,
XmtMenuItem *menu_items,
Cardinal num_menu_items 
) {
   Arg args[10];
   int ac;
   Widget popup;
   
   DENTER(GUI_LAYER, "qmonCreatePopup");

   ac = 0;
   XtSetArg(args[ac], XmtNitems, menu_items); ac++;    
   XtSetArg(args[ac], XmtNnumItems, num_menu_items); ac++;    
   popup = XmtCreatePopupMenu(parent, name, args, ac);

   DEXIT;
   return popup;

}




#ifdef QMON_DEBUG
/*-------------------------------------------------------------------------*/
static void reportWidgetsCB(Widget w, XtPointer cld, XtPointer cad)
{
   Widget top = XmtGetTopLevelShell(w);
   FILE *fp;

   if (!(fp = fopen("WidgetTree.txt", "w")))
      return;

   reportWidgets(top, fp);

   FCLOSE(fp);
   return;
FCLOSE_ERROR:
   /* TODO: error handling */
   return;
}
   
/*-------------------------------------------------------------------------*/
static void reportWidgets(
Widget awidget,
FILE *fp 
) {
   WidgetList childrenList;
   int numChildren, i;
    

   /* report */
   fprintf(fp,"%s\n", XtName(awidget));

   if (XtIsComposite(awidget)){
      XtVaGetValues( awidget,
                     XmNchildren, &childrenList,
                     XmNnumChildren, &numChildren,
                     NULL);

      for (i=0; i < numChildren; i++) {
         fprintf(fp,"   ");
         reportWidgets(childrenList[i], fp); /* recursive call */
      }
   }
}

/*-------------------------------------------------------------------------*/
static void qmonPointTo(Widget w, XtPointer cld, XtPointer cad)
{
   Widget result, top;
   Cursor cursor;
   Display *dpy = XtDisplay(w);
   char buf[BUFSIZ];
   
   DENTER(GUI_LAYER, "qmonPointTo");
   cursor = XCreateFontCursor(dpy, XC_hand2);
   top = XmtGetTopLevelShell(w);
   result = XmTrackingLocate(top, cursor, False);

   if (result && qmonBrowserObjectEnabled(BROWSE_STDOUT)) {
      sprintf(buf, "Pointing to %s\n", XtName(result));   
      qmonBrowserShow(buf);
   }

   XFreeCursor(dpy, cursor);
   DEXIT;
}

#endif


#if 0

/*-------------------------------------------------------------------------*/
static void qmonHelpBrowserPopup(Widget w, XtPointer cld, XtPointer cad)
{
   static Widget help = 0;
   Widget top;
   
   DENTER(GUI_LAYER, "qmonHelpBrowserPopup");
   top = XmtGetTopLevelShell(w);
   if (!help)
      help = XmtCreateHelpBrowser(top, "HelpBrowser", NULL, 0);

   XtManageChild(help);
   
   DEXIT;
}

#endif


/*-------------------------------------------------------------------------*/
/* static int enr; */
static Widget tooltip;

static void qmonShowTooltip(
Widget w,
XtPointer cld,
XEvent *ev,
Boolean *ctd 
) {
   char *text = (char *) cld;
   Widget tooltipRC, tooltipLabel;
   Position x = 0, y = 0;
   XmString xtext;

   DENTER(GUI_LAYER, "qmonShowTooltip");

   if (ev->xcrossing.type == EnterNotify && !tooltip) {
      XtRemoveEventHandler(w, EnterWindowMask, False, qmonShowTooltip, cld);
/*       printf("%.5d: EnterNotify ==> ev->xcrossing.detail = %d\n",  */
/*                enr++, ev->xcrossing.detail); */
      x = ev->xcrossing.x_root + 5;
      y = ev->xcrossing.y_root + 5;
      tooltip = XtVaCreateWidget("Tooltip",
                                       xmMenuShellWidgetClass,
                                       w,
                                       XmNoverrideRedirect, True,
                                       XmNwidth, 1,
                                       XmNheight, 1,
                                       XmNx, x,
                                       XmNy, y,
                                       NULL);
      tooltipRC = XtVaCreateWidget( "tooltipRC",
                                              xmRowColumnWidgetClass,
                                              tooltip,
                                              XmNx, x,
                                              XmNy, y,
                                              XmNmarginWidth, 0,
                                              XmNmarginHeight, 0,
                                              NULL);
      xtext = XmtCreateLocalizedXmString(w, text);
      tooltipLabel = XtVaCreateManagedWidget( "TooltipLabel",
                                              xmLabelWidgetClass,
                                              tooltipRC,
                                              XmNalignment, XmALIGNMENT_CENTER,
                                              XmNmarginWidth, 2,
                                              XmNmarginHeight, 2,
                                              XmNlabelString, xtext,
                                              XmNshadowThickness, 2,
                                              XmNbackground, TooltipBackground, 
                                              XmNforeground, TooltipForeground, 
                                              NULL); 
      XmStringFree(xtext);
      XtManageChild(tooltipRC);
      XtRealizeWidget(tooltip);
      XtPopup(tooltip, XtGrabNone);
      XmtWaitUntilMapped(tooltip);
      XtAddEventHandler(w, LeaveWindowMask, False, qmonHideTooltip, cld);
   }
   *ctd = False;
}

static void qmonHideTooltip(
Widget w,
XtPointer cld,
XEvent *ev,
Boolean *ctd 
) {
   DENTER(GUI_LAYER, "qmonHideTooltip");
   
   if (ev->xcrossing.type == LeaveNotify && tooltip) {
/*       printf("%.5d: LeaveNotify ==> ev->xcrossing.detail = %d\n",  */
/*                enr++, ev->xcrossing.detail); */
      XtRemoveEventHandler(w, LeaveWindowMask, False, qmonHideTooltip, cld);
      XtDestroyWidget(tooltip);
      tooltip = 0;
      XtAddEventHandler(w, EnterWindowMask, False, qmonShowTooltip, cld);
   }
   *ctd = False;
}
