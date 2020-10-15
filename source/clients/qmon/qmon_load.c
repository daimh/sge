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
#include <Xmt/Xmt.h>
#include <Xmt/InputField.h>
#include <Xmt/Dialogs.h>

#include "qmon_rmon.h"
#include "qmon_matrix.h"
#include "qmon_load.h"
#include "qmon_comm.h"
#include "qmon_message.h"
#include "qmon_util.h"

#include "sgeobj/sge_str.h"
#include "sgeobj/sge_centry.h"

#include "gdi/sge_gdi.h"

typedef struct _tFFM {
   Widget matrix;
   Widget name;
   Widget value;
} tFFM;


/*-------------------------------------------------------------------------*/
static void qmonLoadAddEntry(Widget matrix, String name);

/*-------------------------------------------------------------------------*/
void qmonLoadSelectEntry(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixSelectCellCallbackStruct *cbs = 
            (XbaeMatrixSelectCellCallbackStruct*) cad;
   String str;

   DENTER(GUI_LAYER, "qmonLoadSelectEntry");

   if (cbs->num_params && !strcmp(cbs->params[0], "begin")) {
      /* name */
      str = XbaeMatrixGetCell(w, cbs->row, 0);

      /* value */
      str = XbaeMatrixGetCell(w, cbs->row, 1);
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonLoadAddEntry(
Widget matrix,
String name 
) {
   int rows = 0;
   int row;
   String str;
   String new_row[2];

   DENTER(GUI_LAYER, "qmonLoadAddEntry");

   /*
   ** check input 
   */
   if (is_empty_word(name)) {
      qmonMessageShow(matrix, True, "Name required !");
      DEXIT;
      return;
   }

   /*
   ** add to attribute matrix, search if item already exists
   */
   rows = XbaeMatrixNumRows(matrix);

   for (row=0; row<rows; row++) {
      /* get name str */
      str = XbaeMatrixGetCell(matrix, row, 0);
      if (!str || (str && !strcmp(str, name)) ||
            (str && is_empty_word(str))) 
         break;
   }
   
/*    if (row <= rows) */
   if (row < rows)
      XbaeMatrixSetCell(matrix, row, 0, name);
   else {
      new_row[0] = name;
      new_row[1] = NULL;
      XbaeMatrixAddRows(matrix, row, new_row, NULL, NULL, 1);
   }

   /* refresh the matrix */
   XbaeMatrixDeselectAll(matrix);
   XbaeMatrixRefresh(matrix);

   /* set value field to edit mode */
   XbaeMatrixEditCell(matrix, row, 1);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
void ShowLoadNames(
Widget w,
lList *entries 
) {
   lListElem *ep = NULL;
   char stringval[BUFSIZ];
   Boolean status = False;
   String *strs = NULL;
   int n, i;

   DENTER(GUI_LAYER, "ShowLoadNames");

   if (entries) {
      n = lGetNumberOfElem(entries);
      if (n>0) {
         lPSortList(entries, "%I+", CE_name);
         strs = (String*)XtMalloc(sizeof(String)*n);
         for (ep=lFirst(entries), i=0; i<n; ep=lNext(ep), i++) {
           /*
           ** build the strings to display 
           */
           StringConst name = lGetString(ep, CE_name);
/*            String type = lGetUlong(ep, CE_consumable) ? "C" : "F"; */
/*            sprintf(stringval, "%s: %s", type, name ? name : "");  */
           strcpy(stringval, name ? name : ""); 
           strs[i] = XtNewString(stringval);
         }
      }

      strcpy(stringval, "");
      status = XmtAskForItem(w, NULL, "@{Select an attribute",
                     "@{Available attributes}", strs, n, False,
                     stringval, sizeof(stringval), NULL);
      /*
      ** don't free referenced strings, they are in the ce list
      */
      StringTableFree(strs, n); 
   }
   else
      XmtDisplayInformationMsg(w, NULL, "@{No attributes available}", 
                                 NULL, NULL);

   if (status) {
      qmonLoadAddEntry(w, stringval);
   }

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonLoadNames(
Widget w,
XtPointer cld, 
XtPointer cad) 
{
   lList *cl = NULL;
   lList *alp = NULL;
   lList *entries = NULL;
   static lCondition *where = NULL;
   static lEnumeration *what = NULL; 

   DENTER(GUI_LAYER, "qmonLoadNamesQueue");

   qmonMirrorMultiAnswer(CENTRY_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   cl = qmonMirrorList(SGE_CE_LIST);

   if (!where)
      where = lWhere("%T(%I != %s)", CE_Type, CE_name, "slots");
   if(!what)
      what = lWhatAll();

   /*
   ** slots are only excluded if load names are shown for queues
   */
   entries = lSelect(lGetListName(cl), cl, cld ? NULL : where, what);


   ShowLoadNames(w, entries);

   /*
   ** free the copied list
   */
   lFreeList(&entries);
}         
