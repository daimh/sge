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
#include <ctype.h>

#include <Xm/Xm.h>
#include <Xm/List.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Layout.h>
#include <Xmt/Chooser.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/MsgLine.h>
#include <Xmt/InputField.h>

#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_appres.h"
#include "qmon_timer.h"
#include "qmon_comm.h"
#include "qmon_widgets.h"
#include "qmon_message.h"
#include "qmon_calendar.h"
#include "qmon_globals.h"

#include "uti/sge_dstring.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_calendar.h"

#include "gdi/sge_gdi.h"

static Widget qmon_cal = 0;
static Widget cal_names = 0;
static Widget cal_name_w = 0;
static Widget cal_year_w = 0;
static Widget cal_week_w = 0;
static Widget cal_ask_layout = 0;
static Widget cal_conf_list = 0;
static int add_mode = 0;

/*-------------------------------------------------------------------------*/
static void qmonPopdownCalendarConfig(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreateCalendarConfig(Widget parent);
static void qmonCalendarFillConf(Widget w, lListElem *ep);
static void qmonSelectCalendar(Widget w, XtPointer cld, XtPointer cad);
static void qmonCalendarAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonCalendarModify(Widget w, XtPointer cld, XtPointer cad);
static void qmonCalendarDelete(Widget w, XtPointer cld, XtPointer cad);
static void qmonCalendarOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonCalendarCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonCalendarResetAsk(void);
static void qmonCalendarSetAsk(lListElem *calp);
static Widget qmonCreateCalendarAsk(Widget parent);
static Boolean qmonCalendarGetAsk(lListElem *calp);

/*-------------------------------------------------------------------------*/
void qmonPopupCalendarConfig(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonPopupCalendarConfig");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_cal) {
      shell = XmtGetTopLevelShell(w);
      qmon_cal = qmonCreateCalendarConfig(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonPopdownCalendarConfig, NULL);
      /*
      ** create ask layout
      */
      cal_ask_layout = qmonCreateCalendarAsk(qmon_cal);

   } 
   XSync(XtDisplay(qmon_cal), 0);
   XmUpdateDisplay(qmon_cal);

   qmonMirrorMultiAnswer(CALENDAR_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }
   qmonTimerAddUpdateProc(CALENDAR_T, "updateCalendarList", updateCalendarList);
   qmonStartTimer(CALENDAR_T);
   updateCalendarList();
   XmListSelectPos(cal_names, 1, True);

   XtManageChild(qmon_cal);
   XRaiseWindow(XtDisplay(XtParent(qmon_cal)), XtWindow(XtParent(qmon_cal)));

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updateCalendarList(void)
{
   lList *cl;
   
   DENTER(GUI_LAYER, "updateCalendarList");

   cl = qmonMirrorList(SGE_CAL_LIST);
   lPSortList(cl, "%I+", CAL_name);
   UpdateXmListFromCull(cal_names, XmFONTLIST_DEFAULT_TAG, cl, CAL_name);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonPopdownCalendarConfig(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPopdownCalendarConfig");

   XtUnmanageChild(qmon_cal);
   qmonStopTimer(CALENDAR_T);
   qmonTimerRmUpdateProc(CALENDAR_T, "updateCalendarList");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCalendarFillConf(
Widget w,
lListElem *ep 
) {
   XmString *items;
   Cardinal itemCount; 
   const char *s;
   int i;
   dstring sb = DSTRING_INIT;

   DENTER(GUI_LAYER, "qmonCalendarFillConf");
   
   if (!ep) {
      /*
      ** clear the cal_conf_list
      */
      XtVaSetValues( cal_conf_list, 
                  XmNitems, NULL,
                  XmNitemCount, 0,
                  NULL);
      DEXIT;
      return;
   }
   
   itemCount = 2;
   items = (XmString*) XtMalloc(sizeof(XmString)*itemCount); 

   i = 0;

   /* year calendar */
   sge_dstring_sprintf(&sb, "%-20.20s ", XmtLocalize(w, "Year", "Year"));
   sge_dstring_append(&sb, (s=lGetString(ep, CAL_year_calendar))?s:
                              XmtLocalize(w, "NONE", "NONE"));
   items[i++] = XmStringCreateLtoR(sb.s, "LIST");
   sge_dstring_free(&sb);

   /* week calendar */
   sge_dstring_sprintf(&sb, "%-20.20s ", XmtLocalize(w, "Week", "Week"));
   sge_dstring_append(&sb, (s=lGetString(ep, CAL_week_calendar))?s:
                              XmtLocalize(w, "NONE", "NONE"));
   items[i++] = XmStringCreateLtoR((char*)sge_dstring_get_string(&sb), "LIST");
   sge_dstring_free(&sb);

   XtVaSetValues( cal_conf_list, 
                  XmNitems, items,
                  XmNitemCount, itemCount,
                  NULL);
   XmStringTableFree(items, itemCount);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSelectCalendar(Widget w, XtPointer cld, XtPointer cad)
{
   XmListCallbackStruct *cbs = (XmListCallbackStruct*) cad;
   char *calname;
   lListElem *ep;
   
   DENTER(GUI_LAYER, "qmonSelectCalendar");

   if (! XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &calname)) {
      fprintf(stderr, "XmStringGetLtoR failed\n");
      DEXIT;
      return;
   }

   ep = calendar_list_locate(qmonMirrorList(SGE_CAL_LIST), calname);

   XtFree((char*) calname);

   qmonCalendarFillConf(cal_conf_list, ep);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static Widget qmonCreateCalendarConfig(
Widget parent 
) {
   Widget cal_layout, cal_add, cal_delete, cal_modify, cal_done, cal_main_link;

   DENTER(GUI_LAYER, "qmonCreateCalendarConfig");
   
   cal_layout = XmtBuildQueryDialog( parent, "qmon_cal",
                           NULL, 0,
                           "cal_names", &cal_names,
                           "cal_conf_list", &cal_conf_list,
                           "cal_add", &cal_add,
                           "cal_delete", &cal_delete,
                           "cal_done", &cal_done,
                           "cal_modify", &cal_modify,
                           "cal_main_link", &cal_main_link,
                           NULL);

   XtAddCallback(cal_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(cal_names, XmNbrowseSelectionCallback, 
                     qmonSelectCalendar, NULL);
   XtAddCallback(cal_done, XmNactivateCallback, 
                     qmonPopdownCalendarConfig, NULL);
   XtAddCallback(cal_add, XmNactivateCallback, 
                     qmonCalendarAdd, NULL); 
   XtAddCallback(cal_modify, XmNactivateCallback, 
                     qmonCalendarModify, NULL); 
   XtAddCallback(cal_delete, XmNactivateCallback, 
                     qmonCalendarDelete, NULL); 

   XtAddEventHandler(XtParent(cal_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
   XtAddEventHandler(XtParent(cal_layout), StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);

   DEXIT;
   return cal_layout;
}



/*-------------------------------------------------------------------------*/
static Widget qmonCreateCalendarAsk(
Widget parent 
) {
   Widget cal_ok, cal_cancel;

   DENTER(GUI_LAYER, "qmonCreateCalendarAsk");
   
   cal_ask_layout = XmtBuildQueryDialog( parent, "cal_ask_shell",
                           NULL, 0,
                           "cal_ok", &cal_ok,
                           "cal_cancel", &cal_cancel,
                           "cal_name", &cal_name_w,
                           "cal_year", &cal_year_w,
                           "cal_week", &cal_week_w,
                           NULL);

   XtAddCallback(cal_ok, XmNactivateCallback, 
                     qmonCalendarOk, NULL);
   XtAddCallback(cal_cancel, XmNactivateCallback, 
                     qmonCalendarCancel, NULL);

   XtAddEventHandler(XtParent(cal_ask_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
   XtAddEventHandler(XtParent(cal_ask_layout), StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);

   DEXIT;
   return cal_ask_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonCalendarAdd(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonCalendarAdd");

   qmonCalendarResetAsk();
   XtVaSetValues( cal_name_w,
                  XmNeditable, True,
                  NULL);
   add_mode = 1;
   XtManageChild(cal_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCalendarModify(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *calnames;
   Cardinal calnum;
   String calstr;
   lListElem *calp = NULL;

   DENTER(GUI_LAYER, "qmonCalendarModify");

   /*
   ** on opening the dialog fill in the old values
   */
   XtVaGetValues( cal_names,
                  XmNselectedItems, &calnames,
                  XmNselectedItemCount, &calnum,
                  NULL);
   if (calnum == 1 && 
         XmStringGetLtoR(calnames[0], XmFONTLIST_DEFAULT_TAG, &calstr)) {
      XmtInputFieldSetString(cal_name_w, calstr);
      XtVaSetValues( cal_name_w,
                     XmNeditable, False,
                     NULL);
      calp = calendar_list_locate(qmonMirrorList(SGE_CAL_LIST), calstr);
      XtFree((char*)calstr);
      if (calp) {
         add_mode = 0;
         qmonCalendarSetAsk(calp);
         XtManageChild(cal_ask_layout);
      }
   }

   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonCalendarOk(Widget w, XtPointer cld, XtPointer cad)
{
   lList *cal = NULL;
   lList *alp;
   lEnumeration *what;
   Boolean status = False;
   XmString xcalname = NULL;
   StringConst calname = NULL;

   DENTER(GUI_LAYER, "qmonCalendarOk");
   /*
   ** get the contents of the dialog fields here,
   ** build the cull list and send gdi request
   ** decalnding on success of gdi request close the dialog or stay ocaln
   */
   cal = lCreateElemList("CALENDAR_ADD", CAL_Type, 1);
   
   if (cal) {
      if (qmonCalendarGetAsk(lFirst(cal))) {
         calname = (StringConst)lGetString(lFirst(cal), CAL_name);
         /*
         ** gdi call 
         */
         what = lWhat("%T(ALL)", CAL_Type);
         
         if (add_mode) {
            alp = qmonAddList(SGE_CAL_LIST, 
                              qmonMirrorListRef(SGE_CAL_LIST),
                              CAL_name, &cal, NULL, what);
         }
         else {
            alp = qmonModList(SGE_CAL_LIST, 
                              qmonMirrorListRef(SGE_CAL_LIST),
                              CAL_name, &cal, NULL, what);
         }

         if (lFirst(alp) && lGetUlong(lFirst(alp), AN_status) == STATUS_OK)
            status = True;

         qmonMessageBox(w, alp, 0);

         if (status) {
            XtUnmanageChild(cal_ask_layout);
            updateCalendarList();
            /*
            ** select the modified or added Calendar
            */
            xcalname = XmtCreateXmString(calname);
            XmListSelectItem(cal_names, xcalname, True);
            XmStringFree(xcalname);
         }
         lFreeWhat(&what);
         lFreeList(&cal);
         lFreeList(&alp);
      }
   }


   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonCalendarCancel(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonCalendarCancel");

   XtUnmanageChild(cal_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCalendarDelete(Widget w, XtPointer cld, XtPointer cad)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   Cardinal itemCount = 0;
   Boolean status, answer;

   DENTER(GUI_LAYER, "qmonCalendarDelete");
    
   lp = XmStringToCull(cal_names, CAL_Type, CAL_name, SELECTED_ITEMS); 

   if (lp) {
      status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                     "@{calendar.askdel.Do you really want to delete the\nselected Calendar Configuration ?}",
                     "@{Delete}", "@{Cancel}", NULL, XmtNoButton, XmDIALOG_WARNING, 
                     False, &answer, NULL);
      if (answer) { 
         what = lWhat("%T(ALL)", CAL_Type);
         alp = qmonDelList(SGE_CAL_LIST, 
                           qmonMirrorListRef(SGE_CAL_LIST),
                                 CAL_name, &lp, NULL, what);

         qmonMessageBox(w, alp, 0);

         lFreeWhat(&what);
         lFreeList(&alp);

         updateCalendarList();
         XtVaGetValues( cal_names,
                        XmNitemCount, &itemCount,
                        NULL);
         if (itemCount) {
            XmListSelectPos(cal_names, 1, True);
         } else {
            qmonCalendarFillConf(cal_names, NULL);
         }   

      }
      lFreeList(&lp);
   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCalendarSetAsk(
lListElem *calp 
) {
   StringConst cal_name = NULL;
   StringConst year_calendar = NULL;
   StringConst week_calendar = NULL;

   DENTER(GUI_LAYER, "qmonCalendarSetAsk");

   if (!calp) {
      DEXIT;
      return;
   }

   cal_name = (StringConst)lGetString(calp, CAL_name);
   if (cal_name)
      XmtInputFieldSetString(cal_name_w, cal_name);

   year_calendar = (StringConst)lGetString(calp, CAL_year_calendar);
   if (year_calendar)
      XmtInputFieldSetString(cal_year_w, year_calendar);

   week_calendar = (StringConst)lGetString(calp, CAL_week_calendar);
   if (week_calendar)
      XmtInputFieldSetString(cal_week_w, week_calendar);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCalendarResetAsk(void)
{

   DENTER(GUI_LAYER, "qmonCalendarResetAsk");

   XmtInputFieldSetString(cal_name_w, "");

   XmtInputFieldSetString(cal_year_w, "");

   XmtInputFieldSetString(cal_week_w, "");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Boolean qmonCalendarGetAsk(
lListElem *calp 
) {
   String cal_name = NULL;
   String year_calendar = NULL;
   String week_calendar = NULL;

   DENTER(GUI_LAYER, "qmonCalendarGetAsk");

   if (!calp) {
      DEXIT;
      return False;
   }

   cal_name = XmtInputFieldGetString(cal_name_w);
   if (!cal_name || cal_name[0] == '\0') {
      qmonMessageShow(cal_ask_layout, True, "Calendar name required !");
      DEXIT;
      return False;
   }
   lSetString(calp, CAL_name, cal_name);
  
   year_calendar = XmtInputFieldGetString(cal_year_w);
   if (year_calendar && year_calendar[0] != '\0') {
      year_calendar = qmon_trim(year_calendar);
      lSetString(calp, CAL_year_calendar, year_calendar);
   } else {
      lSetString(calp, CAL_year_calendar, "NONE");
   }   

   week_calendar = XmtInputFieldGetString(cal_week_w);
   if (week_calendar && week_calendar[0] != '\0') {
      week_calendar = qmon_trim(week_calendar);
      lSetString(calp, CAL_week_calendar, week_calendar);
   } else {  
      lSetString(calp, CAL_week_calendar, "NONE");
   }

   DEXIT;
   return True;
}
