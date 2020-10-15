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
#include <fnmatch.h>

#include <Xm/Xm.h>
#include <Xm/List.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>

#include "Matrix.h" 
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_ar.h"
#include "qmon_arcustom.h"
#include "qmon_util.h"
#include "qmon_comm.h"
#include "qmon_widgets.h"
#include "qmon_preferences.h"
#include "qmon_message.h"

#include "uti/sge_htable.h"
#include "uti/sge_time.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_advance_reservation.h"

#include "sched/sge_sched.h"      

#include "sge.h"
#include "symbols.h"

/*-------------------------------------------------------------------------*/
/* Prototypes */
static void okCB(Widget w, XtPointer cld, XtPointer cad);
static void cancelCB(Widget w, XtPointer cld, XtPointer cad);
static void addToSelected(Widget w, XtPointer cld, XtPointer cad);
static void rmFromSelected(Widget w, XtPointer cld, XtPointer cad);

static String PrintUlong(lListElem *ep, int nm);
static String PrintString(lListElem *ep, int nm);
static String PrintTime(lListElem *ep, int nm);
static String PrintDuration(lListElem *ep, int nm);
static String PrintQueues(lListElem *ep, int nm);
static String PrintAcls(lListElem *ep, int nm);
static String PrintState(lListElem *ep, int nm);
static String PrintType(lListElem *ep, int nm);
/* static String PrintNotImpl(lListElem *ep, int nm); */
static String PrintMailList(lListElem *ep, int nm);
static String PrintMailOptions(lListElem *ep, int nm);
static String PrintPERange(lListElem *ep, int nm);
static void SetARLabels(Widget w);
static void qmonWhatSetItems(Widget list, int how);

#if 0
static void qmonARFilterClear(Widget w, XtPointer cld, XtPointer cad);
static void qmonARFilterSet(Widget w, XtPointer cld, XtPointer cad);
static void qmonARFilterEditResource(Widget w, XtPointer cld, XtPointer cad);
static void qmonARFilterRemoveResource(Widget w, XtPointer cld, XtPointer cad);
#endif

static int is_owner_ok(lListElem *ar, lList *owner_list);

/*--------------------------------------------------------------------------------*/
/* selectable items, the names must match the names in sge_advance_reservationL.h */
/* there are fixed columns at the beginning of the natrix                         */
/*--------------------------------------------------------------------------------*/

static htable ARColumnPrintHashTable = NULL;
static htable NameMappingHashTable = NULL;

#define AR_FIRST_FIELD     7

static tARField ar_items[] = {
   { 1, AR_id, "@{Id}", 12, 20, PrintUlong }, 
   { 1, AR_name, "@{Name}", 10, 50, PrintString },
   { 1, AR_owner, "@{Owner}", 10, 50, PrintString },
   { 1, AR_state, "@{Status}", 7, 30, PrintState },
   { 1, AR_start_time, "@{StartTime}", 19, 30, PrintTime },
   { 1, AR_end_time, "@{EndTime}", 19, 30, PrintTime },
   { 1, AR_duration, "@{Duration}", 19, 30, PrintDuration },
   { 0, AR_type, "@{Type}", 7, 30, PrintType },
   { 0, AR_submission_time, "@{SubmitTime}", 19, 30, PrintTime },
   { 0, AR_account, "@{Account}", 15, 50, PrintString },
   { 0, AR_checkpoint_name, "@{Checkpoint Name}", 11, 30, PrintString },
/*    { 0, AR_resource_list, "@{ResourceList}", 11, 30, PrintNotImpl }, */
/*    { 0, AR_resource_utilization, "@{ResourceUtilization}", 15, 100, PrintNotImpl }, */
   { 0, AR_queue_list, "@{Queues}", 6, 100, PrintQueues },
/*    { 0, AR_granted_slots, "@{GrantedSlots}", 15, 50, PrintNotImpl }, */
   { 0, AR_mail_options, "@{MailOptions}", 11, 30, PrintMailOptions },
   { 0, AR_mail_list, "@{MailTo}", 15, 30, PrintMailList },
   { 0, AR_pe, "@{PE}", 10, 30, PrintString },
   { 0, AR_pe_range, "@{PERange}", 15, 30, PrintPERange },
   { 0, AR_acl_list, "@{ACL}", 12, 30, PrintAcls },
   { 0, AR_xacl_list, "@{XACL}", 12, 30, PrintAcls }
};

/*
** this list restricts the selected ars, queues displayed
*/
static Widget arfield_available = 0;
static Widget arfield_selected = 0;

static Widget arcu = 0;
static Widget arfield = 0;

static lList *arfilter_fields = NULL;

/*-------------------------------------------------------------------------*/
static void ar_get_type_string(char *buf, size_t buflen, u_long32 type)
{
   if (type != 0) {
      snprintf(buf, buflen-1, "-now");  
   } else {   
      strcat(buf, "");  
   }   
}   

/*-------------------------------------------------------------------------*/
static void ar_get_state_string(char *buf, size_t buflen, u_long32 state)
{
   switch(state) {
      case AR_WAITING:
         snprintf(buf, buflen-1, "w");  
         break;
      case AR_RUNNING:
         snprintf(buf, buflen-1, "r");  
         break;
      case AR_EXITED:
         snprintf(buf, buflen-1, "x");  
         break;
      case AR_DELETED:
         snprintf(buf, buflen-1, "d");  
         break;
      case AR_ERROR:
         snprintf(buf, buflen-1, "E");  
         break;
      case AR_WARNING:
         snprintf(buf, buflen-1, "W");  
         break;
      default:
         snprintf(buf, buflen-1, "u");  
         break;
   }
}   

/*-------------------------------------------------------------------------*/
static String PrintPERange(
lListElem *ep,
int nm 
) {
   dstring range_string = DSTRING_INIT;
   String str;

   DENTER(GUI_LAYER, "PrintPERange");

   range_list_print_to_string(lGetList(ep, nm), &range_string, true, false, false);
   str = XtNewString(sge_dstring_get_string(&range_string));
   sge_dstring_free(&range_string);

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintMailList(
lListElem *ep,
int nm 
) {
   String str;
   lList *mail_list = NULL;

   DENTER(GUI_LAYER, "PrintMailList");

   mail_list = lGetList(ep, nm); 

   str = XtNewString(write_pair_list(mail_list, MAIL_TYPE));

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintMailOptions(
lListElem *ep,
int nm 
) {
   char buf[BUFSIZ];
   String str;
   u_long32 ca;
   static u_long32 mail_at[] = { MAIL_AT_BEGINNING, MAIL_AT_EXIT,
                                 MAIL_AT_ABORT, MAIL_AT_SUSPENSION};
   static char mailsym[] ="beas";
   int i;

   DENTER(GUI_LAYER, "PrintMailOptions");

   ca = lGetUlong(ep, nm);
   strcpy(buf, "");
   
   if (ca) {
      for (i=0; i<4; i++) {
         if (ca & mail_at[i])
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%c", mailsym[i]);
      }
   }
   else
      strcpy(buf, "None"); 
   
   str = XtNewString(buf);

   DEXIT;
   return str;
}
   
/*-------------------------------------------------------------------------*/
static String PrintUlong(
lListElem *ep,
int nm 
) {
   char buf[BUFSIZ];
   String str;

   DENTER(GUI_LAYER, "PrintUlong");

   sprintf(buf, sge_u32, lGetUlong(ep, nm));

   str = XtNewString(buf);

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintState(
lListElem *ep,
int nm 
) {
   char buf[128] = "";
   String str;

   DENTER(GUI_LAYER, "PrintState");

   /* write states into string */
   ar_get_state_string(buf, sizeof(buf), lGetUlong(ep, AR_state));

   str = XtNewString(buf);

   DEXIT;
   return str;
}
   
/*-------------------------------------------------------------------------*/
static String PrintType(
lListElem *ep,
int nm 
) {
   char buf[128] = "";
   String str;

   DENTER(GUI_LAYER, "PrintType");

   /* write states into string */
   ar_get_type_string(buf, sizeof(buf), lGetUlong(ep, AR_type));

   str = XtNewString(buf);

   DEXIT;
   return str;
}
   
#if 0   
/*-------------------------------------------------------------------------*/
static String PrintNotImpl(
lListElem *ep,
int nm 
) {
   String str;

   DENTER(GUI_LAYER, "PrintNotImpl");

   str = XtNewString("NotImpl");

   DEXIT;
   return str;
}
#endif
   
/*-------------------------------------------------------------------------*/
static String PrintString(
lListElem *ep,
int nm 
) {

   String str;
   StringConst temp;
   
   DENTER(GUI_LAYER, "PrintString");

   temp = lGetString(ep, nm); 
   
   str = XtNewString(temp?temp:"");

   DEXIT;
   return str;
}
   
/*-------------------------------------------------------------------------*/
static String PrintTime(
lListElem *ep,
int nm 
) {

   String str;
   dstring ds;
   char buffer[128];

   DENTER(GUI_LAYER, "PrintTime");

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   if (lGetUlong(ep, nm))
      str = XtNewString(sge_ctime(lGetUlong(ep, nm), &ds));
   else
      str = XtNewString("");

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintDuration(
lListElem *ep,
int nm 
) {

   String str;
   char buffer[128];
   u_long32 value = 0;
   int seconds = 0;
   int minutes = 0;
   int hours = 0;

   DENTER(GUI_LAYER, "PrintDuration");

   value = lGetUlong(ep, nm);
   seconds = value % 60;
   minutes = ((value - seconds) / 60) % 60;
   hours = ((value - seconds - minutes * 60) / 3600);
   snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds); 
   str = XtNewString(buffer);

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintQueues(
lListElem *ep,
int nm 
) {

   String str;
   dstring ds = DSTRING_INIT;
   lListElem *qep = NULL;
   lList *ql = NULL;

   DENTER(GUI_LAYER, "PrintQueues");

   ql = lGetList(ep, nm);
   if (ql) {
      bool first = true;
      for_each(qep, ql) {
         if (!first) {
            sge_dstring_append(&ds, ",");
         } else {
            first = false;
         }   
         sge_dstring_append(&ds, lGetString(qep, QR_name));
      }      
      str = XtNewString(sge_dstring_get_string(&ds));
      sge_dstring_free(&ds);
   } else {
      str = XtNewString("");
   }   

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintAcls(
lListElem *ep,
int nm 
) {

   String str;
   dstring ds = DSTRING_INIT;
   lListElem *qep = NULL;
   lList *ql = NULL;

   DENTER(GUI_LAYER, "PrintAcls");

   ql = lGetList(ep, nm);
   if (ql) {
      bool first = true;
      for_each(qep, ql) {
         if (!first) {
            sge_dstring_append(&ds, ",");
         } else {
            first = false;
         }   
         sge_dstring_append(&ds, lGetString(qep, ARA_name));
      }      
      str = XtNewString(sge_dstring_get_string(&ds));
      sge_dstring_free(&ds);
   } else {
      str = XtNewString("");
   }   

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
String* PrintARField(
lListElem *ep,
int cols 
) {
   int i;
   String* row = NULL;
   int col;
   String entry;

   DENTER(GUI_LAYER, "PrintARField");

   /*
   ** check col against XtNumber(ar_items)
   ** if cols > XtNumber(ar_items) we screw the whole thing up
   */

   row = (String*)XtMalloc(sizeof(String)*cols);

   if (ep) {
      for (i=0, col=0; row && i<XtNumber(ar_items) && col<cols; i++) {
         if (ar_items[i].show) {
            if (ar_items[i].printARField &&
               (entry = ar_items[i].printARField(ep, ar_items[i].nm))) { 
               row[col] = entry;
            } else {
               row[col] = XtNewString("");
            }
            col++;
         }
      }
   } else {
      for (col=0; row && col<cols; col++) {
         row[col] = XtNewString("");
      }
   }

   DEXIT;
   return row;
}

/*-------------------------------------------------------------------------*/
static void okCB(Widget w, XtPointer cld, XtPointer cad)
{
   int nr_fields = 0;
   int i, j;
   XmString *strlist;
   Widget run_m;
   Widget pen_m;
/*    String owner_str = NULL; */
   tARField *ar_item = NULL;
   
   DENTER(GUI_LAYER, "okCB");

   XtVaGetValues( arfield_selected, 
                  XmNitemCount, &nr_fields, 
                  XmNitems, &strlist,
                  NULL);

   /*
   ** reset show flag
   */ 
   for (j=AR_FIRST_FIELD; j<XtNumber(ar_items); j++) {           
      ar_items[j].show = 0; 
   }



   for (i=0; i<nr_fields; i++) {
      if (strlist[i]) {
         String text;
         u_long32 temp;
         XmStringGetLtoR(strlist[i], XmFONTLIST_DEFAULT_TAG, &text);
         temp = XrmStringToQuark(text);
         if (sge_htable_lookup(NameMappingHashTable, 
                             &temp,
                             (const void **) &ar_item)) {
            ar_item->show = 1;
         }
         XtFree(text);
         DPRINTF(("ar_item[%d].name: <%s> show = %d\n",
                  i, ar_items[i].name, ar_items[i].show));
      }
   }        

   qmonARReturnMatrix(&run_m, &pen_m);
   SetARLabels(run_m);

#ifdef AR_PENDING
   SetARLabels(pen_m);
#endif   

   updateARList();

   xmui_unmanage(arcu);

   DEXIT;
}
   
/*-------------------------------------------------------------------------*/
static void cancelCB(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "cancelCB");
   
   qmonWhatSetItems(arfield_selected, FILL_SELECTED);
   xmui_unmanage(arcu);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void saveCB(Widget w, XtPointer cld, XtPointer cad)
{
   int j;
   lList *lp = NULL;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "saveCB");
   
   okCB(w, NULL, NULL);

   for (j=AR_FIRST_FIELD; j<XtNumber(ar_items); j++) {           
      if (ar_items[j].show)
         lAddElemStr(&lp, ST_name, ar_items[j].name, ST_Type); 
   }
   lSetList(qmonGetPreferences(), PREF_ar_filter_fields, lp);

   alp = qmonWritePreferences();

   qmonMessageBox(w, alp, 0);

   lFreeList(&alp);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void addToSelected(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *items;
   Cardinal itemCount;
   int i;

   DENTER(GUI_LAYER, "addToSelected");

   XtVaGetValues( arfield_available,
                  XmNselectedItems, &items,
                  XmNselectedItemCount, &itemCount,
                  NULL);

   for (i=0; i<itemCount; i++) {
      if (! XmListItemExists(arfield_selected, items[i]))
         XmListAddItem(arfield_selected, items[i], 0); 
   }
                     
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void rmFromSelected(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *items;
   Cardinal itemCount;

   DENTER(GUI_LAYER, "rmFromSelected");

   XtVaGetValues( arfield_selected,
                  XmNselectedItems, &items,
                  XmNselectedItemCount, &itemCount,
                  NULL);
   
   if (itemCount)
      XmListDeleteItems(arfield_selected, items, itemCount);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void SetARLabels(
Widget w 
) {
   int cols;
   int col;
   int i;
   String *labels = NULL;
   short *widths = NULL;
   int *max_lengths = NULL;
   int additional = 0;
   Screen *screen = XtScreen(w);

   DENTER(GUI_LAYER, "SetARLabels");

   for (i=AR_FIRST_FIELD; i<XtNumber(ar_items); i++) {
      if (ar_items[i].show)
         additional++;
   }

   XtVaGetValues( w,
                  XmNcolumns, &cols,
                  NULL);

   /*
   ** delete the additional columns
   */
   if (cols > AR_FIRST_FIELD)
      XbaeMatrixDeleteColumns(w, AR_FIRST_FIELD, cols - AR_FIRST_FIELD);

   if (additional > 0) {
      labels = (String*)XtMalloc(sizeof(String)*additional);
      widths = (short*)XtMalloc(sizeof(short)*additional);
      max_lengths = (int*)XtMalloc(sizeof(int)*additional);
      for (i=AR_FIRST_FIELD, col=0; i<XtNumber(ar_items) && col<additional; i++) {
         if (ar_items[i].show) {
            String str, s, category, tag, defaultstr, free_me = NULL;
            str = ar_items[i].name; 
            /* if this string has a tag, localize it first */
            if ((str[0] == '@') && (str[1] == '{')) {
               s = XtNewString(str);
               free_me = s;
               s += 2;
               category = NULL;
               tag = NULL;
               defaultstr = s;
               while(*s) {
                  if (*s == '.' && !category) {
                     *s = '\0';
                     category = defaultstr;
                     defaultstr = s+1;
                  }
                  if (*s == '.' && !tag) {
                     *s = '\0';
                     tag = defaultstr;
                     defaultstr = s+1;
                  }
                  if (*s == '}') {
                     *s = '\0';
                     s++;
                     break;
                  }
                  s++;
               }
               if (!tag)
                  tag = defaultstr;
/*                if (!tag[0]) goto error; */
/*                if (category && !category[0]) goto error; */
               s = _XmtLocalize(screen, defaultstr, category, tag);
/* printf("--> '%s'\n", s);                  */
            }
            else {
               s = (String)str;
            }
    
            labels[col] = XtNewString(s);
            widths[col] = ar_items[i].width;
            max_lengths[col] = ar_items[i].max_length;
            if (free_me) XtFree(free_me);
            col++;
         } 
      }
      XbaeMatrixAddColumns(w,
                           AR_FIRST_FIELD,
                           NULL,
                           labels,
                           widths,
                           max_lengths,
                           NULL,
                           NULL,
                           NULL,
                           additional);
      for (i=0, col=0; i<XtNumber(ar_items) && col<(AR_FIRST_FIELD + additional); i++) {
         if (ar_items[i].show) {
            XbaeMatrixSetColumnUserData(w, col, &ar_items[i].nm);
            col++;
         }   
      }
#if 0
      for (col=0; col<XbaeMatrixNumColumns(w); col++) {
         int* nm = (int*) XbaeMatrixGetColumnUserData(w, col);
         printf("nm = %s\n", lNm2Str(*nm));
      }
#endif
      for(i=0; i<additional; i++) {
         XtFree(labels[i]);
      }   
      XtFree((char*)labels);
      XtFree((char*)widths);
      XtFree((char*)max_lengths);
   }
                           

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonWhatSetItems(
Widget list,
int how 
) {
/*    XmString *items; */
/*    Cardinal itemCount = 0; */
/*    int i, j; */
   int num_ars, i;

   DENTER(GUI_LAYER, "qmonWhatSetItems");

   num_ars = XtNumber(ar_items);

#if 0
   if (how == FILL_ALL)
      itemCount = num_ars - AR_FIRST_FIELD;
   else
      for (i=AR_FIRST_FIELD; i<num_ars; i++) {
         if (ar_items[i].show)
            itemCount++;
      } 

   items = (XmString*)XtMalloc(sizeof(XmString)*itemCount);
   for (i=AR_FIRST_FIELD,j=0; i<num_ars && j<itemCount; i++) {
      if (how == FILL_ALL) {
         items[j] = XmtCreateLocalizedXmString(list, ar_items[i].name); 
         j++;
      }
      else {
         if (ar_items[i].show) {
            items[j] = XmtCreateLocalizedXmString(list, ar_items[i].name);
            j++;
         }
      }
   }
         
   XtVaSetValues( list, 
                  XmNitems, items,
                  XmNitemCount, itemCount,
                  NULL);
   XmStringTableFree(items, itemCount);

#else   

   XtVaSetValues( list, 
                  XmNitems, NULL,
                  XmNitemCount, 0,
                  NULL);
   for (i=AR_FIRST_FIELD; i<num_ars; i++) {
      if (how == FILL_ALL) {
         DPRINTF(("XmListAddItemUniqueSorted: '%s'\n", ar_items[i].name));
         XmListAddItemUniqueSorted(list, ar_items[i].name);
      }
      else {
         if (ar_items[i].show) {
            XmListAddItemUniqueSorted(list, ar_items[i].name);
         }
      }
   }
         
#endif

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonPopupARCU(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(TOP_LAYER, "qmonPopupARCU");

   if (!arcu)
      qmonCreateARCU(w, NULL);
   xmui_manage(arcu);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonCreateARCU(
Widget parent,
XtPointer cld 
) {
   Widget   arcu_ok, arcu_cancel, arcu_save, arcu_folder,
            arfield_add, arfield_remove;
   Widget run_m;
   Widget pen_m;
   int i;
 
   DENTER(GUI_LAYER, "qmonCreateARCU");

   /*
   ** the resource file for qc_dialog_shell must contain 
   ** qc_dialog as unmanaged XmtLayout child otherwise
   ** it is managed by XmtBuildQueryDialog and managed again
   ** in qmonPopupARCU
   */
   arcu = XmtBuildQueryDialog(parent, "arcu_shell",
                           NULL, 0,
                           "arcu_folder", &arcu_folder,
                           "arfield_selected", &arfield_selected,
                           "arfield_available", &arfield_available,
                           "arfield_add", &arfield_add,
                           "arfield_remove", &arfield_remove,
                           "arcu_ok", &arcu_ok,
                           "arcu_cancel", &arcu_cancel,
                           "arcu_save", &arcu_save,
                           NULL);
   /*
   ** create ARColumnPrintHashTable
   */
   ARColumnPrintHashTable = sge_htable_create(5, dup_func_u_long32, hash_func_u_long32, hash_compare_u_long32);
   for (i=0; i<sizeof(ar_items)/sizeof(tARField); i++) {
      u_long32 temp = XrmStringToQuark(ar_items[i].name);
      sge_htable_store(ARColumnPrintHashTable,
                     &temp,
                     (void *)&ar_items[i]);
   }

   /*
   ** create NameMappingHashTable
   */
   NameMappingHashTable = sge_htable_create(5, dup_func_u_long32, hash_func_u_long32, hash_compare_u_long32);
   for (i=0; i<sizeof(ar_items)/sizeof(tARField); i++) {
      String text;
      u_long32 temp;
      XmString xstr = XmtCreateLocalizedXmString(arcu, ar_items[i].name);
      XmStringGetLtoR(xstr, XmFONTLIST_DEFAULT_TAG, &text);
      temp = XrmStringToQuark(text);
      sge_htable_store(NameMappingHashTable,
                     &temp,
                     (void *)&ar_items[i]);
      XmStringFree(xstr);
      XtFree(text);
   }

   /*
   ** reference to preferences
   */
   if (qmonGetPreferences()) {
      lListElem *field;
      tARField *ar_item = NULL;
      lList *ar_filter_field_list = lGetList(qmonGetPreferences(), PREF_ar_filter_fields);
      
      arfilter_fields = lCopyList("", ar_filter_field_list);

/* lWriteListTo(arfilter_fields, stdout); */

      /*
      ** set the fields which shall be shown
      */
      for_each(field, arfilter_fields) {
         u_long32 temp = XrmStringToQuark(lGetString(field, ST_name));
         if (sge_htable_lookup(ARColumnPrintHashTable, 
             &temp,
             (const void **) &ar_item)) {
            ar_item->show = 1;
         }
      }
   }


   /*
   ** preset the fields
   */
   qmonWhatSetItems(arfield_available, FILL_ALL); 
   qmonWhatSetItems(arfield_selected, FILL_SELECTED); 

   /*
   ** preset the column labels
   */
   qmonARReturnMatrix(&run_m, &pen_m);
   SetARLabels(run_m);
#ifdef AR_PENDING   
   SetARLabels(pen_m);
#endif   
   
                           
   XtAddCallback(arcu_ok, XmNactivateCallback,
                        okCB, NULL);
   XtAddCallback(arcu_cancel, XmNactivateCallback,
                        cancelCB, NULL);
   XtAddCallback(arcu_save, XmNactivateCallback,
                        saveCB, NULL);
   XtAddCallback(arfield_add, XmNactivateCallback,
                        addToSelected, (XtPointer) arfield);
   XtAddCallback(arfield_remove, XmNactivateCallback,
                        rmFromSelected, (XtPointer) arfield);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* this function destroys the delivered ar list partially               */
/*-------------------------------------------------------------------------*/
int match_ar(
lList **ar_list,
lList *owner_list,
lList *queue_list, 
lList *complex_list,
lList *exec_host_list,
lList *request_list 
) {
   lListElem *ar;
   lListElem *dep;
   int show;

   DENTER(GUI_LAYER, "match_ar");

   ar = lFirst(*ar_list);
   while (ar) {
      /*
      ** first of all we assume that the ar should be displayed
      */
      show = 1;
      
      /*
      ** check if ar fulfills user_list and set show flag
      */
      if (show && owner_list)
         show = is_owner_ok(ar, owner_list);


      if (show) {
         ar = lNext(ar);
      }
      else {
         dep = ar;
         ar = lNext(ar);
         lRemoveElem(*ar_list, &dep);
      }
   } 

   if (lGetNumberOfElem(*ar_list) == 0) {
      lFreeList(ar_list);
   }

   DEXIT;
   return True;
}
         
/*-------------------------------------------------------------------------*/
static int is_owner_ok(
lListElem *ar,
lList *owner_list 
) {
   lListElem *op;

   if (!owner_list)
      return True;

   for_each(op, owner_list) {
      if (!fnmatch(lGetString(op, ST_name), lGetString(ar, AR_owner), 0)) {
         return True;
      }
   }
   return False;
}

