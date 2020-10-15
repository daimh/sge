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
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/DrawingA.h>
#include <Xm/TextF.h>
#include <Xmt/Xmt.h>
#include <Xmt/Dialogs.h>
#include <Xmt/Create.h>
#include <Xmt/Chooser.h>
#include <Xmt/InputField.h>
#include <Xmt/Pixmap.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Layout.h>
#include <Xmt/LayoutG.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_request.h"
#include "qmon_submit.h"
#include "qmon_globals.h"
#include "qmon_comm.h"
#include "qmon_init.h"
#include "qmon_util.h"
#include "qmon_message.h"
#include "AskForTime.h"
#include "IconList.h"

#include "uti/sge_string.h"

#include "sgeobj/sge_host.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_centry.h"

#include "gdi/sge_gdi.h"

#include "comm/commlib.h"



/*-------------------------------------------------------------------------*/
static void qmonRequestRemoveResource(Widget w, XtPointer cld, XtPointer cad);
static void qmonToggleHardSoft2(Widget w, XtPointer cld, XtPointer cad);
static void qmonRequestClear(Widget w, XtPointer cld, XtPointer cad);
static void qmonRequestOkay(Widget w, XtPointer cld, XtPointer cad);
static void qmonRequestCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonRequestEditResource(Widget w, XtPointer cld, XtPointer cad);
static int qmonCullToIconList(lList *cel, int how, IconListElement **ice, int *iceCount);
static Pixmap qmonFetchTypeIcon(int type);


/*-------------------------------------------------------------------------*/
static lList *hard_requests = 0;
static lList *soft_requests = 0;

static lListElem *fill_in_request;

static int hard_soft = 0; 
static Widget request_dialog = 0; 
static Widget request_rtype = 0; 
static Widget request_rr = 0; 
static Widget request_hr = 0; 
static Widget request_sr = 0; 




/*-------------------------------------------------------------------------*/
void qmonRequestPopup(Widget w, XtPointer cld, XtPointer cad)
{
   Widget request_hardsoft, parent, request_clear, request_okay,
         request_cancel; 
   lList *hrl = NULL;
   lList *srl = NULL;
   lList *rll = NULL;
   lListElem *ep = NULL;
   lListElem *rp = NULL;
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonRequestPopup");

   parent = XmtGetShell(w);
   DPRINTF(("parent = %s\n", XtName(parent) ));
   
   if (!request_dialog) {
      request_dialog = XmtBuildQueryDialog(parent, "request_shell", 
                              NULL, 0,
                              "request_rtype", &request_rtype,
                              "request_rr", &request_rr,
                              "request_hr", &request_hr,
                              "request_sr", &request_sr,
                              "request_hardsoft", &request_hardsoft,
                              "request_clear", &request_clear,
                              "request_okay", &request_okay,
                              "request_cancel", &request_cancel,
                              NULL);
      XtAddCallback(request_rr, XmNactivateCallback,
                     qmonRequestEditResource, (XtPointer)0); 
      XtAddCallback(request_hr, XmNactivateCallback, 
                     qmonRequestEditResource, (XtPointer)1);
      XtAddCallback(request_hr, XmNremoveCallback, 
                     qmonRequestRemoveResource, NULL);
      XtAddCallback(request_sr, XmNactivateCallback, 
                     qmonRequestEditResource, (XtPointer)1);
      XtAddCallback(request_sr, XmNremoveCallback, 
                     qmonRequestRemoveResource, NULL);

      XtAddCallback(request_hardsoft, XmtNvalueChangedCallback,
                     qmonToggleHardSoft2, NULL);
      XtAddCallback(request_clear, XmNactivateCallback, 
                     qmonRequestClear, NULL);
      XtAddCallback(request_okay, XmNactivateCallback, 
                     qmonRequestOkay, NULL);
      XtAddCallback(request_cancel, XmNactivateCallback, 
                     qmonRequestCancel, NULL);

      XtAddEventHandler(XtParent(request_dialog), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
      
   }

   /*
   ** set the type labelString
   */
   XtVaSetValues(request_rtype, XmtNlabel, qmonSubmitRequestType(), NULL);

   qmonMirrorMultiAnswer(CENTRY_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set normal cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }
   rll = qmonGetResources(qmonMirrorList(SGE_CE_LIST), 
                                       REQUESTABLE_RESOURCES);

   hrl = qmonSubmitHR();
   lFreeList(&hard_requests);
   if (hrl) {
      hard_requests = lCopyList("hr", hrl);
      for_each(ep, hard_requests) {
         rp = lGetElemStr(rll, CE_name, lGetString(ep, CE_name));
         if (!rp)
            rp = lGetElemStr(rll, CE_shortcut, lGetString(ep, CE_name));
         if (rp) {
            lSetString(ep, CE_name, lGetString(rp, CE_name));
            lSetString(ep, CE_default, NULL);
            lSetUlong(ep, CE_valtype, lGetUlong(rp, CE_valtype)); 
         }
      }
   }   
   
   srl = qmonSubmitSR();
   lFreeList(&soft_requests);
   if (srl) {
      soft_requests = lCopyList("sr", srl);
      for_each(ep, soft_requests) {
         rp = lGetElemStr(rll, CE_name, lGetString(ep, CE_name));
         if (!rp)
            rp = lGetElemStr(rll, CE_shortcut, lGetString(ep, CE_name));
         if (rp) {
            lSetString(ep, CE_name, lGetString(rp, CE_name));
            lSetUlong(ep, CE_valtype, lGetUlong(rp, CE_valtype)); 
         }
      }
   }   

   /*
   ** do the drawing
   */
   qmonRequestDraw(request_rr, rll, 0);
   qmonRequestDraw(request_hr, hard_requests, 1);
   qmonRequestDraw(request_sr, soft_requests, 1);

   XtRealizeWidget(request_dialog);

   xmui_manage(request_dialog);

   lFreeList(&rll);
   DEXIT;
}

/*-------------------------------------------------------------------------*/
void qmonRequestDraw(
Widget w,
lList *lp,
int how 
) {
   IconListElement *items = NULL;
   int itemCount = 0;
   
   DENTER(GUI_LAYER, "qmonRequestDraw");

   XmIconListGetItems(w, &items, &itemCount);
   qmonCullToIconList(lp, how,  &items, &itemCount);
   XmIconListSetItems(w, items, itemCount);
   
   DEXIT;
}
   

/*-------------------------------------------------------------------------*/
static void qmonRequestRemoveResource(Widget w, XtPointer cld, XtPointer cad)
{

   XmIconListCallbackStruct *cbs = (XmIconListCallbackStruct*) cad;

   DENTER(GUI_LAYER, "qmonRequestRemoveResource");

   if (hard_soft) {
      if (soft_requests) {
         lDelElemStr(&soft_requests, CE_name, cbs->element->string[0]);
         qmonRequestDraw(request_sr, soft_requests, 1);
      }
   }
   else {
      if (hard_requests) {
         lDelElemStr(&hard_requests, CE_name, cbs->element->string[0]);
         qmonRequestDraw(request_hr, hard_requests, 1);
      }
   }
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonRequestClear(Widget w, XtPointer cld, XtPointer cad)
{


   DENTER(GUI_LAYER, "qmonRequestClear");

   if (hard_soft) {
      if (soft_requests) {
         lFreeList(&soft_requests);
         qmonRequestDraw(request_sr, soft_requests, 1);
      }
   }
   else {
      if (hard_requests) {
         lFreeList(&hard_requests);
         qmonRequestDraw(request_hr, hard_requests, 1);
      }
   }

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonToggleHardSoft2(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct*) cad;

   DENTER(GUI_LAYER, "qmonToggleHardSoft");
   
   hard_soft = cbs->state; 
   if (hard_soft) {
      XtSetSensitive(request_sr, True);
      XtSetSensitive(request_hr, False);
   }
   else {
      XtSetSensitive(request_sr, False);
      XtSetSensitive(request_hr, True);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonRequestOkay(Widget w, XtPointer cld, XtPointer cad)
{

   DENTER(GUI_LAYER, "qmonRequestOkay");
   
   xmui_unmanage(request_dialog);

   /*
   ** give the resources to the submit dialog
   */
   qmonSubmitSetResources(&hard_requests, &soft_requests);
   
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonRequestCancel(Widget w, XtPointer cld, XtPointer cad)
{

   DENTER(GUI_LAYER, "qmonRequestCancel");

   xmui_unmanage(request_dialog);
   
   DEXIT;
}



/*-------------------------------------------------------------------------*/
lList *qmonGetResources(lList *ce_list, int how) {
   lList *lp = NULL;
   lList *entries = NULL;

   DENTER(GUI_LAYER, "qmonGetResources");

   entries = ce_list;
   
   if (entries) {
      if (!lp) {
         lp = lCopyList("CE_entries", entries);
      } else {
         lList *copy = lCopyList("CE_entries", entries);
         lAddList(lp, &copy);
      }
   }

   lUniqStr(lp, CE_name);

   if (how == REQUESTABLE_RESOURCES) { 
      lCondition *where = lWhere("%T(%I == %u)", CE_Type, CE_requestable, REQU_YES);
      if (where) {
         lp = lSelectDestroy(lp, where); 
      }
      lFreeWhere(&where);
   }

   DRETURN(lp);
}


/*-------------------------------------------------------------------------*/
static int qmonCullToIconList(
lList *cel,
int how,
IconListElement **ice,
int *iceCount 
) {
   lListElem *cep;
   int count;
   IconListElement *elements, *current;
   int i, j;
   String *str;
   
   DENTER(GUI_LAYER, "qmonCullToIconList");

   count = lGetNumberOfElem(cel);
   if (count > 0) {
      elements = (IconListElement*)XtMalloc(sizeof(IconListElement)*count);
      current = elements;
      for_each(cep, cel) { 
         if (!how) {
            str = (String*)XtMalloc(sizeof(String));
            str[0] = XtNewString(lGetString(cep, CE_name));
            current->string = str;
            current->numStrings = 1;
         }
         else {
            str = (String*)XtMalloc(sizeof(String) * 3);
            str[0] = XtNewString(lGetString(cep, CE_name)); 
            str[1] = XtNewString("=="); 
            str[2] = XtNewString(lGetString(cep, CE_stringval)); 
            current->string = str;
            current->numStrings = 3;
         }
         current->iconPixmap.pixmap = 
                     qmonFetchTypeIcon((int)lGetUlong(cep, CE_valtype)); 
         current->iconPixmap.mask = 0;
         current->iconPixmap.isBitmap = False;
         current->iconPixmap.width = 16;
         current->iconPixmap.height = 16;
         
         current++;
      } 
   }
   else {
      elements = NULL;
      count = 0;
   }

   /*
   ** free old IconListElement entrys
   */
   for (i=0; *ice && i<*iceCount; i++) {
      if ((*ice)[i].string) {
         for (j=0; j<(*ice)[i].numStrings; j++) {
            if ((*ice)[i].string[j])
               XtFree((*ice)[i].string[j]);
         }
         XtFree((char*)(*ice)[i].string);
      }
   }
   XtFree((char*)*ice); 

   /* attach new entries */
   *ice = elements;
   *iceCount = count;

   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
static Pixmap qmonFetchTypeIcon(
int type 
) {
   Pixmap pix;
   
   DENTER(GUI_LAYER, "qmonFetchTypeIcon");
   
   switch (type) {
      case TYPE_INT:
      case TYPE_DOUBLE:
         pix = qmonGetIcon("int");
         break;
      case TYPE_TIM:
         pix = qmonGetIcon("time");
         break;
      case TYPE_STR:
         pix = qmonGetIcon("str");
         break;
      case TYPE_RESTR:
         pix = qmonGetIcon("unknown");
      case TYPE_BOO:
         pix = qmonGetIcon("bool");
         break;
      case TYPE_MEM:
         pix = qmonGetIcon("mem");
         break;
      case TYPE_HOST:
         pix = qmonGetIcon("host");
         break;
      case TYPE_CSTR:
         pix = qmonGetIcon("cstr");
         break;
      default:
         pix = qmonGetIcon("unknown");
   }
   DEXIT;
   return pix;
}


/*-------------------------------------------------------------------------*/
static void qmonRequestEditResource(Widget w, XtPointer cld, XtPointer cad)
{
   XmIconListCallbackStruct *cbs = (XmIconListCallbackStruct*) cad;
   long how = (long)cld;
   lList *rll;
   int type;
   char stringval[CL_MAXHOSTLEN];
   int status = 0;
   StringConst strval;
   lListElem *ep = NULL;
   

   DENTER(GUI_LAYER, "qmonRequestEditResource");

   rll = qmonGetResources(qmonMirrorList(SGE_CE_LIST), 
                                       REQUESTABLE_RESOURCES); 

   if (!how) {
      fill_in_request = lGetElemStr(rll, CE_name, cbs->element->string[0]);
   } else {
      if (!hard_soft) {
         fill_in_request = lGetElemStr(hard_requests, CE_name, 
                                          cbs->element->string[0]);
      } else {
         fill_in_request = lGetElemStr(soft_requests, CE_name, 
                                          cbs->element->string[0]);
      }
   }


   if (!fill_in_request) {
      goto error_exit;
   }


   type = lGetUlong(fill_in_request, CE_valtype);
   strval = lGetString(fill_in_request, CE_stringval);
   sge_strlcpy(stringval, strval, CL_MAXHOSTLEN);

   status = qmonRequestInput(w, type, cbs->element->string[0], 
                              stringval, sizeof(stringval));
   /* 
   ** put the value in the CE_Type elem 
   */
   if (status) {
      lSetString(fill_in_request, CE_stringval, stringval);
      lSetString(fill_in_request, CE_default, NULL);
    
      /* put it in the hard or soft resource list if necessary */
      if (!how) {
         if (hard_soft) {
            if (!soft_requests) {
               soft_requests = lCreateList("soft_requests", CE_Type);
            }
            if (!(ep = lGetElemStr(soft_requests, CE_name, 
                                    cbs->element->string[0]))) {
               lAppendElem(soft_requests, lCopyElem(fill_in_request));
            } else {
               lSetString(ep, CE_stringval, lGetString(fill_in_request, CE_stringval));
               lSetString(ep, CE_default, NULL);
            }   
               
         } else {
            if (!hard_requests) {
               hard_requests = lCreateList("hard_requests", CE_Type);
            }
            if (!(ep = lGetElemStr(hard_requests, CE_name, 
                                    cbs->element->string[0]))) {
               lAppendElem(hard_requests, lCopyElem(fill_in_request));
            } else {
               lSetString(ep, CE_stringval, lGetString(fill_in_request, CE_stringval));
               lSetString(ep, CE_default, NULL);
            }
         }
      }
      qmonRequestDraw(request_sr, soft_requests, 1);
      qmonRequestDraw(request_hr, hard_requests, 1);
   }

error_exit:
   lFreeList(&rll);
   DEXIT;
}


/*-------------------------------------------------------------------------*/
Boolean qmonRequestInput(
Widget w,
int type,
String resource,
String stringval,
int maxlen 
) {
   int ret = 0;
   char unique[CL_MAXHOSTLEN];
   int intval=0;
   Boolean status = False;
   double dval = 0.0;
   
   DENTER(GUI_LAYER, "qmonRequestInput");

   /* 
   ** call the type specific dialog
   ** stringval contains the string that has to be displayed in  
   ** the hard/soft resource list
   */
   switch (type) {
      case TYPE_INT:
         status = XmtAskForInteger(w, NULL, 
                     "@{Enter an integer value}", &intval, 0, 0,
                     NULL);
         sprintf(stringval, "%d", intval);
         break;
      case TYPE_DOUBLE:
         status = XmtAskForDouble(w, NULL, 
                     "@{Enter a double value}", &dval, 0, 0,
                     NULL);
         sprintf(stringval, "%f", dval);
         break;
      case TYPE_STR:
         status = XmtAskForString(w, NULL, "@{Enter a string value}",
                     stringval, maxlen, NULL);
         if (stringval[0] == '\0')
            status = False;
         break;
      case TYPE_CSTR:
         status = XmtAskForString(w, NULL, "@{Enter a uppercase string value}", stringval, maxlen, NULL);
         if (stringval[0] == '\0')
            status = False;
         break;
       case TYPE_RESTR:
         status = XmtAskForString(w, NULL, "@{Enter a string value}", stringval, maxlen, NULL);
         if (stringval[0] == '\0')
            status = False;
         break;
      case TYPE_HOST:
         status = XmtAskForString(w, NULL, "@{Enter a valid hostname}", stringval, maxlen, NULL);
         if (status && stringval[0] != '\0') {
            if (!sge_is_pattern(stringval)) {
               /* try to resolve hostname */
               ret=sge_resolve_hostname(stringval, unique, EH_name, sizeof(unique));
               switch ( ret ) {
                  case CL_RETVAL_GETHOSTNAME_ERROR:
                     qmonMessageShow(w, True, "Can't resolve host '%s'", stringval);
                     status = False;
                     break;
                  case CL_RETVAL_OK:
                     strcpy(stringval, unique);
                     break; 
                  default:
                     DPRINTF(("sge_resolve_hostname() failed resolving: %s\n", cl_get_error_text(ret)));
               }
            }
         }
         else
            status = False;
         break;
      case TYPE_TIM:
         status = XmtAskForTime(w, NULL, "@{Enter a time value}", stringval, maxlen, NULL, True);
         if (stringval[0] == '\0')
            status = False;
         break;
      case TYPE_MEM:
         status = XmtAskForMemory(w, NULL, "@{Enter a memory value}", stringval, maxlen, NULL);
         if (stringval[0] == '\0')
            status = False;
         break;
      case TYPE_BOO:
         XmtAskForBoolean(w, NULL, "@{Click TRUE or FALSE}", "TRUE", "FALSE", NULL,
                                    XmtYesButton, XmDIALOG_INFORMATION, False,
                                    &status, NULL);
         if (status)
            strcpy(stringval, "true");
         else
            strcpy(stringval, "false");
            
         status = True;
         break;
   }

   DEXIT;
   return status;
}
