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
#include <Xm/List.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Layout.h>
#include <Xmt/Chooser.h>
#include <Xmt/InputField.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_qcustom.h"
#include "qmon_cq.h"
#include "qmon_util.h"
#include "qmon_request.h"
#include "qmon_comm.h"
#include "qmon_preferences.h"
#include "qmon_message.h"
#include "IconList.h"

#include "uti/sge_bitfield.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_qinstance_state.h"

#include "sched/sge_sched.h"      

/*-------------------------------------------------------------------------*/
/* Prototypes */
static void okCB(Widget w, XtPointer cld, XtPointer cad);
static void cancelCB(Widget w, XtPointer cld, XtPointer cad);
static void saveCB(Widget w, XtPointer cld, XtPointer cad);
static void qmonCreateQCU(Widget parent, XtPointer cld);
static void qmonResFilterSet(Widget w, XtPointer cld, XtPointer cad);
static void qmonResFilterClear(Widget w, XtPointer cld, XtPointer cad);
static void qmonResFilterEditResource(Widget w, XtPointer cld, XtPointer cad);
static void qmonResFilterRemoveResource(Widget w, XtPointer cld, XtPointer cad);
static void qmonPEFilterSet(Widget w, XtPointer cld, XtPointer cad);
static void qmonPEAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonPERemove(Widget w, XtPointer cld, XtPointer cad);
static void qmonUserFilterSet(Widget w, XtPointer cld, XtPointer cad);
static void qmonQFilterSet(Widget w, XtPointer cld, XtPointer cad);
static void qmonQStateFilterSet(Widget w, XtPointer cld, XtPointer cad);
static void qmonQCUAddEntryToList(Widget w, XtPointer cld, XtPointer cad);

/*
** this list restricts the selected qs, queues displayed
*/
static lList *queue_filter_resources = NULL;
static lList *queue_filter_pe = NULL;
static lList *queue_filter_user = NULL;
static lList *queue_filter_q = NULL;
static char queue_filter_state[20];

static Widget qcu = 0;
static Widget r_filter_ar = 0;
static Widget r_filter_sr = 0;
static Widget pe_filter_ap = 0;
static Widget pe_filter_sp = 0;
static Widget misc_filter_user_sp = 0;
static Widget misc_filter_user = 0;
static Widget misc_filter_q_sp = 0;
static Widget misc_filter_q = 0;
static Widget misc_filter_state = 0;

/*-------------------------------------------------------------------------*/
void qmonPopupQCU(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;
   DENTER(TOP_LAYER, "qmonPopupQCU");

   qmonMirrorMultiAnswer(PE_T | CENTRY_T,  &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }

   if (!qcu)
      qmonCreateQCU(w, NULL);

   /*
   ** preset qfilter resources
   */
   qmonResFilterSet(qcu, NULL, NULL);
   qmonPEFilterSet(qcu, NULL, NULL);
   qmonUserFilterSet(qcu, NULL, NULL);
   qmonQFilterSet(qcu, NULL, NULL);
   qmonQStateFilterSet(qcu, NULL, NULL);

   xmui_manage(qcu);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void qmonQCUReadPreferences(void)
{
   static bool preferences_set = false;

   DENTER(GUI_LAYER, "qmonQCUReadPreferences");

   if (!preferences_set) {
      const char *qfs;
      queue_filter_resources = lCopyList("", lGetList(qmonGetPreferences(), 
                                          PREF_queue_filter_resources));
      queue_filter_pe = lCopyList("", lGetList(qmonGetPreferences(), 
                                          PREF_queue_filter_pe));
      queue_filter_user = lCopyList("", lGetList(qmonGetPreferences(), 
                                          PREF_queue_filter_user));
      queue_filter_q = lCopyList("", lGetList(qmonGetPreferences(), 
                                          PREF_queue_filter_q));
      qfs = lGetString(qmonGetPreferences(), PREF_queue_filter_state);
      strcpy(queue_filter_state, qfs ? qfs: ""); 
      preferences_set = true;
   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
lList *qmonGetQCUResourceList(void)
{
   return queue_filter_resources;
}

/*-------------------------------------------------------------------------*/
lList *qmonGetQCUQrefList(void)
{
   return queue_filter_q;
}

/*-------------------------------------------------------------------------*/
lList *qmonGetQCUUserRefList(void)
{
   return queue_filter_user;
}

/*-------------------------------------------------------------------------*/
lList *qmonGetQCUPERefList(void)
{
   return queue_filter_pe;
}

/*-------------------------------------------------------------------------*/
u_long32 qmonGetQCUQueueState(void)
{
   u_long32 filter = 0xFFFFFFFF;
   u_long32 state = qinstance_state_from_string(queue_filter_state, 
                                                NULL, filter);
   return state;
}


/*-------------------------------------------------------------------------*/
static void okCB(Widget w, XtPointer cld, XtPointer cad)
{
   static char state_string[] = "acdosuACDES";
   char *qp;
   int i;
   int state;
   lList *lp;

   DENTER(GUI_LAYER, "okCB");

   /*
   ** pe filter
   */
   lp = XmStringToCull(pe_filter_sp, ST_Type, ST_name, ALL_ITEMS);
   lFreeList(&queue_filter_pe);
   queue_filter_pe = lp;
   
   /*
   ** user filter
   */
   lp = XmStringToCull(misc_filter_user_sp, ST_Type, ST_name, ALL_ITEMS);
   lFreeList(&queue_filter_user);
   queue_filter_user = lp;

   /*
   ** q wildcard filter
   */
   lp = XmStringToCull(misc_filter_q_sp, QR_Type, QR_name, ALL_ITEMS);
   lFreeList(&queue_filter_q);
   queue_filter_q = lp;
   
   for (i=0; i<sizeof(queue_filter_state); i++) {
      queue_filter_state[i]='\0';
   }   

   state = XmtChooserGetState(misc_filter_state);
   if (state == 0) {
      strcpy(queue_filter_state, "NONE");
   } else {   
      for (qp=queue_filter_state, i=0;i<sizeof(state_string); i++) {
         if (ISSET(state,(1<<i))) {
            *qp = state_string[i];
            qp++;
         }   
      }         
   }

   /*
   ** update the CQ dialog 
   */
   qmonCQUpdate(w, NULL, NULL);

   xmui_unmanage(qcu);

   DEXIT;
}
   
/*-------------------------------------------------------------------------*/
static void cancelCB(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "cancelCB");
   
   xmui_unmanage(qcu);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void saveCB(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp;

   DENTER(GUI_LAYER, "saveCB");
   
   okCB(w, NULL, NULL);

   lSetList(qmonGetPreferences(), PREF_queue_filter_resources, 
                     lCopyList("", queue_filter_resources));
   lSetList(qmonGetPreferences(), PREF_queue_filter_pe,
                     lCopyList("", queue_filter_pe));
   lSetList(qmonGetPreferences(), PREF_queue_filter_user,
                     lCopyList("", queue_filter_user));
   lSetList(qmonGetPreferences(), PREF_queue_filter_q,
                     lCopyList("", queue_filter_q));
   lSetString(qmonGetPreferences(), PREF_queue_filter_state, 
                     queue_filter_state);

   alp = qmonWritePreferences();

   qmonMessageBox(w, alp, 0);

   lFreeList(&alp);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonCreateQCU(
Widget parent,
XtPointer cld 
) {

   Widget qcu_ok, qcu_cancel, qcu_folder, r_filter_clear,
          qcu_save, pe_add, pe_remove, misc_filter_user_del,
          misc_filter_q_del;
   
   DENTER(GUI_LAYER, "qmonCreateQCU");

   /*
   ** the resource file for qc_dialog_shell must contain 
   ** qc_dialog as unmanaged XmtLayout child otherwise
   ** it is managed by XmtBuildQueryDialog and managed again
   ** in qmonPopupQCU
   */
   qcu = XmtBuildQueryDialog( parent, "qcu_shell",
                                 NULL, 0,
                                 "qcu_ok", &qcu_ok,
                                 "qcu_cancel", &qcu_cancel,
                                 "qcu_save", &qcu_save,
                                 "qcu_folder", &qcu_folder,
                                 "r_filter_ar", &r_filter_ar,
                                 "r_filter_sr", &r_filter_sr,
                                 "r_filter_clear", &r_filter_clear,
                                 "pe_filter_ap", &pe_filter_ap,
                                 "pe_filter_sp", &pe_filter_sp,
                                 "misc_filter_user_sp", &misc_filter_user_sp,
                                 "misc_filter_q_sp", &misc_filter_q_sp,
                                 "misc_filter_user", &misc_filter_user,
                                 "misc_filter_user_del", &misc_filter_user_del,
                                 "misc_filter_q", &misc_filter_q,
                                 "misc_filter_q_del", &misc_filter_q_del,
                                 "misc_filter_state", &misc_filter_state,
                                 "pe_add", &pe_add,
                                 "pe_remove", &pe_remove,
                                 NULL); 

      
   XtAddCallback(r_filter_ar, XmNactivateCallback,
                        qmonResFilterEditResource, (XtPointer)0);
   XtAddCallback(r_filter_sr, XmNactivateCallback,
                        qmonResFilterEditResource, (XtPointer)1);
   XtAddCallback(r_filter_sr, XmNremoveCallback, 
                     qmonResFilterRemoveResource, NULL);
   XtAddCallback(r_filter_clear, XmNactivateCallback,
                        qmonResFilterClear, NULL);

   XtAddCallback(pe_add, XmNactivateCallback,
                        qmonPEAdd, NULL);
   XtAddCallback(pe_remove, XmNactivateCallback,
                        qmonPERemove, NULL);

   XtAddCallback(misc_filter_q_del, XmNactivateCallback,
                        DeleteItems, (XtPointer) misc_filter_q_sp);
   XtAddCallback(misc_filter_user_del, XmNactivateCallback,
                        DeleteItems, (XtPointer) misc_filter_user_sp);

   XtAddCallback(misc_filter_q, XmtNinputCallback,
                        qmonQCUAddEntryToList, (XtPointer) misc_filter_q_sp);
   XtAddCallback(misc_filter_user, XmtNinputCallback,
                        qmonQCUAddEntryToList, (XtPointer) misc_filter_user_sp);



   XtAddCallback(qcu_ok, XmNactivateCallback,
                        okCB, NULL);
   XtAddCallback(qcu_cancel, XmNactivateCallback,
                        cancelCB, NULL);
   XtAddCallback(qcu_save, XmNactivateCallback,
                        saveCB, NULL);
   XtAddCallback(qcu_folder, XmNvalueChangedCallback, 
                     qmonResFilterSet, NULL);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonResFilterClear(Widget w, XtPointer cld, XtPointer cad)
{


   DENTER(GUI_LAYER, "qmonResFilterClear");

   lFreeList(&queue_filter_resources);
   qmonRequestDraw(r_filter_sr, queue_filter_resources, 1);
   
   DEXIT;

}

/*-------------------------------------------------------------------------*/
static void qmonResFilterSet(Widget w, XtPointer cld, XtPointer cad)
{
   lList *arl = NULL;
   lListElem *ep = NULL;
   lListElem *rp = NULL;


   DENTER(GUI_LAYER, "qmonResFilterSet");

   arl = qmonGetResources(qmonMirrorList(SGE_CE_LIST), ALL_RESOURCES);

   for_each (ep, queue_filter_resources) {
      rp = lGetElemStr(arl, CE_name, lGetString(ep, CE_name));
      if (!rp)
         rp = lGetElemStr(arl, CE_shortcut, lGetString(ep, CE_name));
      if (rp) {
         lSetString(ep, CE_name, lGetString(rp, CE_name));
         lSetUlong(ep, CE_valtype, lGetUlong(rp, CE_valtype));
      }
   }
   /*
   ** do the drawing
   */
   qmonRequestDraw(r_filter_ar, arl, 0);
   qmonRequestDraw(r_filter_sr, queue_filter_resources, 1);

   lFreeList(&arl);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonResFilterEditResource(Widget w, XtPointer cld, XtPointer cad)
{
   XmIconListCallbackStruct *cbs = (XmIconListCallbackStruct*) cad;
   long how = (long)cld;
   lList *arl = NULL;
   int type;
   char stringval[CL_MAXHOSTLEN];
   Boolean status = False;
   StringConst name, value, strval;
   Boolean found = False;
   lListElem *fill_in_request = NULL;

   DENTER(GUI_LAYER, "qmonResFilterEditResource");

   arl = qmonGetResources(qmonMirrorList(SGE_CE_LIST), ALL_RESOURCES);

   if (!how) {
      fill_in_request = lGetElemStr(arl, CE_name, cbs->element->string[0]);
   } else {
      for_each (fill_in_request, queue_filter_resources) {
         name = lGetString(fill_in_request, CE_name);
         value = lGetString(fill_in_request, CE_stringval);
         if (cbs->element->string[0] && name && 
            !strcmp(cbs->element->string[0], name) &&
            cbs->element->string[2] && value &&
            !strcmp(cbs->element->string[2], value) ) {
               found = True;
               break;
         }
      }       
            
      if (!found) {
         fill_in_request = NULL; 
      }
   }

   if (!fill_in_request) {
      lFreeList(&arl);
      DRETURN_VOID;
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
    
      /* put it in the request list if necessary */
      if (!how) {
         if (!queue_filter_resources) {
            queue_filter_resources = lCreateList("queue_filter_resources", CE_Type);
         }
         if (!lGetElemStr(queue_filter_resources, CE_name, cbs->element->string[0]))
            lAppendElem(queue_filter_resources, lCopyElem(fill_in_request));
      }
      qmonRequestDraw(r_filter_sr, queue_filter_resources, 1);
   }

   lFreeList(&arl);
   DRETURN_VOID;
}

/*-------------------------------------------------------------------------*/
static void qmonResFilterRemoveResource(Widget w, XtPointer cld, XtPointer cad)
{

   XmIconListCallbackStruct *cbs = (XmIconListCallbackStruct*) cad;
   lListElem *dep = NULL;
   Boolean found = False;
   StringConst name, value;

   DENTER(GUI_LAYER, "qmonResFilterRemoveResource");

   if (queue_filter_resources) {
      for_each (dep, queue_filter_resources) {
         name = lGetString(dep, CE_name);
         value = lGetString(dep, CE_stringval);
         if (cbs->element->string[0] && name && 
            !strcmp(cbs->element->string[0], name) &&
            cbs->element->string[2] && value &&
            !strcmp(cbs->element->string[2], value) ) {
               found = True;
               break;
         }
      }       
            
      if (found) {
         lRemoveElem(queue_filter_resources, &dep);
         if (lGetNumberOfElem(queue_filter_resources) == 0) {
            lFreeList(&queue_filter_resources);
         }
         qmonRequestDraw(r_filter_sr, queue_filter_resources, 1);
      }
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPEFilterSet(Widget w, XtPointer cld, XtPointer cad)
{
   lList *apl = NULL;

   DENTER(GUI_LAYER, "qmonPEFilterSet");

   apl = qmonMirrorList(SGE_PE_LIST);
   UpdateXmListFromCull(pe_filter_ap, XmFONTLIST_DEFAULT_TAG, apl, PE_name);
   UpdateXmListFromCull(pe_filter_sp, XmFONTLIST_DEFAULT_TAG, 
                           queue_filter_pe, ST_name);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPEAdd(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount, i;
   String text;
   
   DENTER(GUI_LAYER, "qmonPEAdd");

   XtVaGetValues( pe_filter_ap,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount) {
      XmtLayoutDisableLayout(qcu);
      for (i=0; i<selectedItemCount; i++) {
         if (!XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text))
            continue;
         XmListAddItemUniqueSorted(pe_filter_sp, text);
         XtFree(text); 
      }
      XmtLayoutEnableLayout(qcu);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPERemove(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   
   DENTER(GUI_LAYER, "qmonPERemove");

   XtVaGetValues( pe_filter_sp,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(pe_filter_sp, selectedItems, selectedItemCount); 

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonUserFilterSet(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonUserFilterSet");

   UpdateXmListFromCull(misc_filter_user_sp, XmFONTLIST_DEFAULT_TAG, 
                           queue_filter_user, ST_name);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQFilterSet(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonQFilterSet");

   UpdateXmListFromCull(misc_filter_q_sp, XmFONTLIST_DEFAULT_TAG, 
                           queue_filter_q, QR_name);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQStateFilterSet(Widget w, XtPointer cld, XtPointer cad)
{
   int state = 0;
   int i;

   DENTER(GUI_LAYER, "qmonQStateFilterSet");

   if (!strcmp(queue_filter_state, "NONE")) {
      state = 0;
   } else {
      for (i=0; i <sizeof(queue_filter_state); i++) {
         switch (queue_filter_state[i]) {
            case 'a':
               state |= (1<<0);
               break;
            case 'c':    
               state |= (1<<1);
               break;
            case 'd':    
               state |= (1<<2);
               break;
            case 'o':    
               state |= (1<<3);
               break;
            case 's':    
               state |= (1<<4);
               break;
            case 'u':    
               state |= (1<<5);
               break;
            case 'A':    
               state |= (1<<6);
               break;
            case 'C':    
               state |= (1<<7);
               break;
            case 'D':    
               state |= (1<<8);
               break;
            case 'E':    
               state |= (1<<9);
               break;
            case 'S':    
               state |= (1<<10);
               break;
         }
      }
   }   
   XmtChooserSetState(misc_filter_state, state, False);

   DEXIT;
}

static void qmonQCUAddEntryToList(Widget w, XtPointer cld, XtPointer cad)
{
   Widget list = (Widget) cld;
   char *text = NULL;
   
   DENTER(GUI_LAYER, "qmonQCUAddEntryToList");

   text = XmtInputFieldGetString(w);
   if (text && text[0] != '\0') {
      XmListAddItemUniqueSorted(list, text);
   }
   XmtInputFieldSetString(w, "");
   
   DEXIT;
}
