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
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Xmt/Xmt.h>
#include <Xmt/WidgetType.h>
#include <Xmt/InputField.h>
#include <Xmt/Procedures.h>

#include "qmon_rmon.h"
#include "qmon_quarks.h"
#include "qmon_matrix.h"
#include "qmon_load.h"
#include "qmon_message.h"
#include "qmon_util.h"

#include "uti/sge_parse_num_par.h"

#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_subordinate.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_var.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_mailrec.h"

#include "cull/cull.h"

#include "gdi/sge_gdi.h"

#include "sched/sge_complex_schedd.h"

static Widget CreateMatrixSimple(Widget parent, String name, ArgList arglist, Cardinal argcount);
static Widget CreateMatrix(Widget parent, String name, ArgList arglist, Cardinal argcount);
static void qmonMatrixLoseFocus(Widget w, XtPointer cld, XtPointer cad);
static void qmonMatrixTraverse(Widget w, XtPointer cld, XtPointer cad);
static void DeleteLines(Widget w, XtPointer cld, XtPointer cad);
static void ColumnZeroNoEdit(Widget w, XtPointer cld, XtPointer cad);
static void ColumnNoEdit(Widget w, XtPointer cld, XtPointer cad);
static void set_2xN(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void get_2xN(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void set_CE_Type(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static Boolean getCE_TypeValues(lListElem *ep, const char **ce_entry); 
static Boolean setCE_TypeValues(lListElem *ep, char **ce_entry);
static void set_CX_Type(Widget w, XtPointer address, XrmQuark type, Cardinal size);
static void get_CX_Type(Widget w, XtPointer address, XrmQuark type, Cardinal size);



static XmtWidgetType widgets[] = {
   {
      "Matrix_2xN",        /* name     */
      NULL,                   /* class    */
      CreateMatrix,       /* constructor */
      set_2xN,            /* procedure to set a value on widget */
      get_2xN,            /* procedure to get a value from widget */
      False                   /* not a popup widget */
   },
   {
      "UserMatrix",        /* name     */
      NULL,                   /* class    */
      CreateMatrix,       /* constructor */
      NULL,            /* procedure to set a value on widget */
      NULL,            /* procedure to get a value from widget */
      False                   /* not a popup widget */
   },
   {
      "CE_TypeMatrix",         
      NULL,
      CreateMatrixSimple,
      set_CE_Type,
      NULL,
      False
   },
   {
      "CX_TypeMatrix",         
      NULL,
      CreateMatrix,
      set_CX_Type,
      get_CX_Type,
      False
   },
   {
      "UU_TypeMatrix",         
      NULL,
      CreateMatrixSimple,
      set_2xN,
      get_2xN,
      False
   },
   {
      "LoadMatrix",         
      NULL,
      CreateMatrixSimple,
      NULL,
      NULL,
      False
   }
};

/*-------------------------------------------------------------------------*/
/*   P U B L I C                                                           */
/*-------------------------------------------------------------------------*/
void QmonRegisterMatrixWidgets(void)
{
   QmonInitQuarks();   /* initialize the quarks */
   XmtRegisterWidgetTypes(widgets, XtNumber(widgets));
}
   
/*-------------------------------------------------------------------------*/
/*   P R I V A T E                                                         */
/*-------------------------------------------------------------------------*/
static Widget CreateMatrixSimple(
Widget parent,
String name,
ArgList arglist,
Cardinal argcount 
) {
   Widget matrix;
#ifndef TEXTCHILD_BUG
   Widget textChild;
#endif
   
   matrix = XtCreateWidget(name, xbaeMatrixWidgetClass, parent,
                              arglist, argcount);

#ifndef TEXTCHILD_BUG
   XtVaGetValues(matrix, XmNtextField, &textChild, NULL);
   XtAddCallback(textChild, XmNlosingFocusCallback,
                  qmonMatrixLoseFocus, (XtPointer) matrix);
#endif
   /*
   ** register callback procedures
   */
   XmtVaRegisterCallbackProcedures(
         "DeleteLines", DeleteLines, XtRWidget,
         "ColumnZeroNoEdit", ColumnZeroNoEdit, NULL,
         "ColumnNoEdit", ColumnNoEdit, NULL,
         NULL);

   return matrix;
}


/*-------------------------------------------------------------------------*/
static Widget CreateMatrix(
Widget parent,
String name,
ArgList arglist,
Cardinal argcount 
) {
   Widget matrix;
   Widget textChild;
   
   matrix = XtCreateWidget(name, xbaeMatrixWidgetClass, parent,
                              arglist, argcount);

#ifndef TEXTCHILD_BUG
   XtVaGetValues(matrix, XmNtextField, &textChild, NULL);
   XtAddCallback(textChild, XmNlosingFocusCallback,
                  qmonMatrixLoseFocus, (XtPointer) matrix);
#endif
   XtAddCallback(matrix, XmNtraverseCellCallback, 
                     qmonMatrixTraverse, NULL);
   XtAddCallback(matrix, XmNselectCellCallback, 
                     qmonMatrixSelect, NULL);
   /*
   ** register callback procedures
   */
   XmtVaRegisterCallbackProcedures(
         "DeleteLines", DeleteLines, XtRWidget,
         "ColumnZeroNoEdit", ColumnZeroNoEdit, NULL,
         "ColumnNoEdit", ColumnNoEdit, NULL,
         NULL);

   return matrix;
}

/*-------------------------------------------------------------------------*/
void qmonMatrixSelect(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixSelectCellCallbackStruct *cbs = 
               (XbaeMatrixSelectCellCallbackStruct*) cad;
   int select_begin=0;
   int i;
   Widget* matrices = (Widget*) cld;

   DENTER(GUI_LAYER, "qmonMatrixSelect");

   if (cbs->params && !strcmp(cbs->params[0], "begin")) {
      XbaeMatrixDeselectAll(w);
      while (matrices && *matrices) {
         XbaeMatrixDeselectAll(*matrices);
         matrices++;
      }
      XbaeMatrixSelectRow(w, cbs->row);
   }

   if (cbs->params && !strcmp(cbs->params[0], "end")) {
      while (matrices && *matrices) {
         XbaeMatrixDeselectAll(*matrices);
         matrices++;
      }
      select_begin = XbaeMatrixFirstSelectedRow(w);
      if (select_begin == -1) {
         XbaeMatrixSelectRow(w, cbs->row);
         DEXIT;
         return;
      }
      if (cbs->row < select_begin) {
         for (i=cbs->row; i<select_begin; i++)
            XbaeMatrixSelectRow(w, i);
      }
      else {
         for (i=select_begin; i<=cbs->row; i++)
            XbaeMatrixSelectRow(w, i);
      }
   }
   if (cbs->params && !strcmp(cbs->params[0], "toggle")) {
      if (XbaeMatrixIsRowSelected(w, cbs->row)) {
         DPRINTF(("XbaeMatrixDeselectRow\n"));
         XbaeMatrixDeselectRow(w, cbs->row);
      }
      else {
         XbaeMatrixSelectRow(w, cbs->row);
         DPRINTF(("XbaeMatrixSelectRow\n"));
      }
   }

   if (cbs->params && !strcmp(cbs->params[0], "editall")) {
      XbaeMatrixEditCell(w, cbs->row, cbs->column);
   }

   if (cbs->params && !strcmp(cbs->params[0], "edit") && cbs->column != 0) {
      XbaeMatrixEditCell(w, cbs->row, cbs->column);
   }

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void DeleteLines(Widget w, XtPointer cld, XtPointer cad)
{
   Widget matrix = (Widget) cld;
   int rows;
   int i;
   int rows_to_delete = 0;
   int rows_old;
   int max_rows = 7;
   

   DENTER(GUI_LAYER, "DeleteLines");

   if (!matrix) {
      DEXIT;
      return;
   }

   /* max_rows = XbaeMatrixVisibleRows(matrix); */

   rows = rows_old = XbaeMatrixNumRows(matrix);

   for (i=0; i<rows; i++)
      if (XbaeMatrixIsRowSelected(matrix, i))
         rows_to_delete++;

   i = 0;
   while (i<rows) {
      if (XbaeMatrixIsRowSelected(matrix, i)) {
         XbaeMatrixDeleteRows(matrix, i, 1);
         rows--;
      }
      else
         i++;
   }

   if ((rows_old - rows_to_delete) < max_rows) 
      XbaeMatrixAddRows(matrix, rows, NULL, NULL, NULL, (max_rows - rows));

   /* reset attribute line */
   XbaeMatrixDeselectAll(matrix);

   /* refresh the matrix */
   XbaeMatrixRefresh(matrix);

   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void ColumnZeroNoEdit(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixEnterCellCallbackStruct *cbs = 
            (XbaeMatrixEnterCellCallbackStruct*) cad;

   DENTER(GUI_LAYER, "ColumnZeroNoEdit");

   if (cbs->column == 0)
      cbs->doit = False;
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void ColumnNoEdit(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixEnterCellCallbackStruct *cbs = 
            (XbaeMatrixEnterCellCallbackStruct*) cad;

   DENTER(GUI_LAYER, "ColumnNoEdit");

   cbs->doit = False;
   
   DEXIT;
}




/*-------------------------------------------------------------------------*/
static void qmonMatrixTraverse(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixTraverseCellCallbackStruct *cbs = 
            (XbaeMatrixTraverseCellCallbackStruct*) cad;
   static XrmQuark QDown = NULLQUARK;
   static XrmQuark QRight = NULLQUARK;
   static XrmQuark QUp = NULLQUARK;
   
   DENTER(GUI_LAYER, "qmonMatrixTraverse");

   /*
    * Get the Quarks we care about
    */
   if (QDown == NULLQUARK) {
      QDown = XrmStringToQuark("Down");
      QUp = XrmStringToQuark("Up");
      QRight = XrmStringToQuark("Right");
   }

   /*
   ** if we are moving up don't jump to last row when we are in row 0
   */
   if ((cbs->qparam == QUp && cbs->row == 0)) {
      cbs->next_row = cbs->row;
   }

   /*
    * If we are moving down, and we are at the last row, add a new row
    * and traverse to it.
    */
   if ((cbs->qparam == QDown && cbs->row == cbs->num_rows - 1) ||
       (cbs->qparam == QRight && cbs->row == cbs->num_rows - 1 &&
            cbs->column == cbs->num_columns)) {
      XbaeMatrixAddRows(w, cbs->num_rows, NULL, NULL, NULL, 1);
      cbs->next_row = cbs->num_rows;
      cbs->next_column = cbs->column;
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonMatrixLoseFocus(Widget w, XtPointer cld, XtPointer cad)
{
   Widget matrix = (Widget) cld;
   XbaeMatrixCommitEdit(matrix, False);
}

/*-------------------------------------------------------------------------*/
static void set_2xN(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *lp = NULL;

   if (type != QmonQVA_Type && type != QmonQCE2_Type &&
       type != QmonQHS_Type && type != QmonQMR_Type &&
       type != QmonQPN_Type && type != QmonQAT_Type &&
       type != QmonQSO_Type && type != QmonQUA_Type) {
      XmtWarningMsg("XmtDialogSetDialogValues", "XbaeMatrix",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   if (size == sizeof(lList*))
      lp = *(lList**) address;
   else
      return;

   if (type == QmonQVA_Type) 
      qmonSet2xN(w, lp, VA_variable, VA_value);
   else if ( type == QmonQCE2_Type ) 
      qmonSet2xN(w, lp, CE_name, CE_stringval);
   else if ( type ==  QmonQHS_Type )
      qmonSet2xN(w, lp, HS_name, HS_value);
   else if ( type == QmonQMR_Type )
      qmonSet2xN(w, lp, MR_user, MR_host);
   else if ( type == QmonQPN_Type )
      qmonSet2xN(w, lp, PN_path, PN_host);
   else if ( type == QmonQAT_Type )
      qmonSet2xN(w, lp, AT_account, AT_cell);
   else if ( type == QmonQSO_Type )
      qmonSet2xN(w, lp, SO_name, SO_threshold);
   else if ( type == QmonQUA_Type )
      qmonSet2xN(w, lp, UA_name, UA_value);
      
}


/*-------------------------------------------------------------------------*/
static void get_2xN(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *lp = NULL;
   
   if (type != QmonQVA_Type && type != QmonQCE2_Type &&
       type != QmonQHS_Type && type != QmonQMR_Type &&
       type != QmonQPN_Type && type != QmonQAT_Type && 
       type != QmonQSO_Type && type != QmonQUA_Type) {
      XmtWarningMsg("XmtDialogGetDialogValues", "XbaeMatrix",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   if (type == QmonQVA_Type) 
      lp = qmonGet2xN(w, VA_Type, VA_variable, VA_value);
   else if ( type == QmonQCE2_Type ) 
      lp = qmonGet2xN(w, CE_Type, CE_name, CE_stringval);
   else if ( type ==  QmonQHS_Type )
      lp = qmonGet2xN(w, HS_Type, HS_name, HS_value);
   else if ( type == QmonQMR_Type ) 
      lp = qmonGet2xN(w, MR_Type, MR_user, MR_host);
   else if ( type == QmonQPN_Type )
      lp = qmonGet2xN(w, PN_Type, PN_path, PN_host);
   else if ( type == QmonQAT_Type )
      lp = qmonGet2xN(w, AT_Type, AT_account, AT_cell);
   else if ( type == QmonQSO_Type )
      lp = qmonGet2xN(w, SO_Type, SO_name, SO_threshold);
   else if ( type == QmonQUA_Type )
      lp = qmonGet2xN(w, UA_Type, UA_name, UA_value);

#if 0
   /* 
    * AA-2009-02-12
    * leads to crash reverting memory leak fix in qmon_qaction.c
    */
   if (*(lList**)address != NULL) {
      lFreeList((lList**)address);
   }   
#endif   
   *(lList**)address = lp;
}


/*-------------------------------------------------------------------------*/
void qmonSetNxN(
Widget w,
lList *lp,
int num_fields,
...
) {
   lListElem *ep;
   int i, row;
   int max_rows;
   int val;
   double dval;
   char buf[128];
   int *field;
   const char **col;
   va_list ap;
   
   DENTER(GUI_LAYER, "qmonSetNxN");
   
   /* clear the area */
   XtVaSetValues(w, XmNcells, NULL, NULL);
   
   if (!lp) {
      DEXIT;
      return;
   }

   field = sge_malloc(num_fields*sizeof(int));
   col = sge_malloc(num_fields*sizeof(char *));
   if (field == NULL || col == NULL) {
      abort();
   }

   va_start(ap, num_fields);
   for(i=0; i<num_fields; i++)
      field[i] = va_arg(ap, int);

   XtVaGetValues(w, XmNrows, &max_rows, NULL);

   for (ep = lFirst(lp), row = 0; ep; ep = lNext(ep), row++) {
      if (row == max_rows) {
         XbaeMatrixAddRows(w, 
                           max_rows, 
                           NULL,       /* empty rows  */
                           NULL,       /* no lables   */
                           NULL,       /* no different colors */
                           1);         /* we add 1 rows      */
         max_rows++;
      }

      memset(col, 0, num_fields*sizeof(char *));

      /*
       * get column values
       */

      for(i=0; i<num_fields; i++) {

         switch (lGetType(lGetListDescr(lp), field[i])) {
            case lStringT:
               col[i] = (StringConst)lGetString(ep, field[i]);
               break;
            case lHostT:
               col[i] = (StringConst)lGetHost(ep,field[i]);
               break;
            case lUlongT:
               val = (int)lGetUlong(ep, field[i]);
#if 0
               if (val) {
                  sprintf(buf, "%d", val);
                  col[i] = buf;
               }
               else
                  col[i] = NULL;
#else
               sprintf(buf, "%d", val);
               col[i] = buf;
#endif
               break;
            case lDoubleT:
               dval = lGetDouble(ep, field[i]);
               sprintf(buf, "%.2f", dval);
               col[i] = buf;
               break;
         }
      }

      if (col[0]) {
         /* FIX_CONST_GUI */
         for(i=0; i<num_fields; i++)
            XbaeMatrixSetCell(w, row, i, col[i] ? (String)col[i] : "");
      }
   }
  
   va_end(ap);
   sge_free(&field);
   sge_free(&col);
   DEXIT;
}

/*-------------------------------------------------------------------------*/
lList* qmonGetNxN(
Widget w,
lDescr *dp,
int num_fields,
...
) {
   lList *lp = NULL;
   lListElem *ep;
   int i, row;
   int max_rows;
   va_list ap;
   char **col;
   int *field;

   DENTER(GUI_LAYER, "qmonGetNxN");

   field = sge_malloc(num_fields*sizeof(int));
   col = sge_malloc(num_fields*sizeof(char *));
   if (field == NULL || col == NULL) {
      abort();
   }

   va_start(ap, num_fields);
   for(i=0; i<num_fields; i++)
      field[i] = va_arg(ap, int);

   XtVaGetValues(w, XmNrows, &max_rows, NULL);
   
   for (row=0; row<max_rows; row++) {
      memset(col, 0, num_fields*sizeof(char *));
      for(i=0; i<num_fields; i++)
         col[i] = XbaeMatrixGetCell(w, row, i);
      if (col[0] && col[0][0] != '\0') {
         if (!lp)
            lp = lCreateList(XtName(w), dp);
         ep = lCreateElem(dp);
         lAppendElem(lp, ep);

         /*
          * retrieve values from columns
          */

         for(i=0; i<num_fields; i++) {
            switch(lGetType(lGetListDescr(lp), field[i])) {
               case lStringT: 
                  lSetString(ep, field[i], col[i] ? col[i] : "" );
                  break;
               case lHostT:
                  lSetHost(ep, field[i], col[i] ? col[i] : "");
                  break;
               case lUlongT:
                  lSetUlong(ep, field[i], col[i] ? atoi(col[i]) : 0);
                  break;
               case lDoubleT:
                  lSetDouble(ep, field[i], col[i] ? atof(col[i]) : 0.0);
                  break;
            }
         }
      }
      else
         continue;
   }
   
   va_end(ap);
   sge_free(&field);
   sge_free(&col);

   DEXIT;
   return lp;
}

   
#if 1

/*-------------------------------------------------------------------------*/
void qmonSet2xN(
Widget w,
lList *lp,
int field1,
int field2 
) {
   qmonSetNxN(w, lp, 2, field1, field2);
}


/*-------------------------------------------------------------------------*/
lList* qmonGet2xN(
Widget w,
lDescr *dp,
int field1,
int field2 
) {
   return qmonGetNxN(w, dp, 2, field1, field2);
}

#else

/*-------------------------------------------------------------------------*/
void qmonSet2xN(
Widget w,
lList *lp,
int field1,
int field2 
) {
   lListElem *ep;
   int row;
   int max_rows;
   const char *col1 = NULL, *col2 = NULL;
   int val;
   double dval;
   char buf[128];
   
   DENTER(GUI_LAYER, "qmonSet2xN");
   
   /* clear the area */
   XtVaSetValues(w, XmNcells, NULL, NULL);
   
   if (!lp) {
      DEXIT;
      return;
   }
      
   XtVaGetValues(w, XmNrows, &max_rows, NULL);

   for (ep = lFirst(lp), row = 0; ep; ep = lNext(ep), row++) {
      if (row == max_rows) {
         XbaeMatrixAddRows(w, 
                           max_rows, 
                           NULL,       /* empty rows  */
                           NULL,       /* no lables   */
                           NULL,       /* no different colors */
                           1);         /* we add 1 rows      */
         max_rows++;
      }
      /*
      ** the first column of the matrix can be string or host type
      **
      */
      switch (lGetType(lGetListDescr(lp), field1)) {
         case lStringT:
            col1 = (StringConst)lGetString(ep, field1);
            break;
         case lHostT:
            col1 = (StringConst)lGetHost(ep,field1);
            break;
      }
      /*
      ** the second column can be of different type
      */
      switch (lGetType(lGetListDescr(lp), field2)) {
         case lStringT:
            col2 = (StringConst)lGetString(ep, field2);
            break;
         case lHostT:
            col2 = (StringConst)lGetHost(ep,field2);
            break;
         case lUlongT:
            val = (int)lGetUlong(ep, field2);
            if (val) {
               sprintf(buf, "%d", val);
               col2 = buf;
            }
            else
               col2 = NULL;
            break;
         case lDoubleT:
            dval = lGetDouble(ep, field2);
            sprintf(buf, "%.2f", dval);
            col2 = buf;
            break;
      }

      if (col1) {
         /* FIX_CONST_GUI */
         XbaeMatrixSetCell(w, row, 0 , col1 ? (String)col1 : "");
         /* FIX_CONST_GUI */
         XbaeMatrixSetCell(w, row, 1 , col2 ? (String)col2 : "");
      }
   }
       
   DEXIT;
}

/*-------------------------------------------------------------------------*/
lList* qmonGet2xN(
Widget w,
lDescr *dp,
int field1,
int field2 
) {
   lList *lp = NULL;
   lListElem *ep;
   int row;
   int max_rows;
   char *col1, *col2;

   DENTER(GUI_LAYER, "qmonGet2xN");

   XtVaGetValues(w, XmNrows, &max_rows, NULL);
   
   for (row=0; row<max_rows; row++) {
      col1 = XbaeMatrixGetCell(w, row, 0);
      col2 = XbaeMatrixGetCell(w, row, 1);
      if (col1 && col1[0] != '\0') {
         if (!lp)
            lp = lCreateList(XtName(w), dp);
         ep = lCreateElem(dp);
         lAppendElem(lp, ep);

         /*
         ** the first field in the column can be host or string
         */

         switch(lGetType(lGetListDescr(lp), field1)) {
            case lStringT:
               lSetString(ep, field1, col1 ? col1 : "");
               break;
            case lHostT:
               lSetHost(ep, field1, col1 ? col1 : "");
               break;
         }

         /*
         ** the second field can be of different type
         */
         switch(lGetType(lGetListDescr(lp), field2)) {
            case lStringT: 
               lSetString(ep, field2, col2 ? col2 : "" );
               break;
            case lHostT:
               lSetHost(ep, field2, col2 ? col2 : "");
               break;
            case lUlongT:
               lSetUlong(ep, field2, col2 ? atoi(col2) : 0);
               break;
            case lDoubleT:
               lSetDouble(ep, field2, col2 ? atof(col2) : 0.0);
               break;
         }   
      }
      else
         continue;
   }

   DEXIT;
   return lp;
}

#endif

   
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static void set_CE_Type(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *lp = NULL;

   if (type != QmonQCE_Type) {
      XmtWarningMsg("XmtDialogSetDialogValues", "XbaeMatrix",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'; QmonQCE_Type expected.",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   if (size == sizeof(lList*))
      lp = *(lList**) address;
   else
      return;

   qmonSetCE_Type(w, lp, CE_TYPE_REDUCED);

}

/*-------------------------------------------------------------------------*/
void qmonSetCE_Type(
Widget w,
lList *lp,
int full 
) {
   lListElem *ep;
   int row;
   int max_rows;
   const char *ce_entry[CE_MAX];
   int i;
   
   DENTER(GUI_LAYER, "qmonSetCE_Type");
   
   XtVaGetValues(w, XmNrows, &max_rows, NULL);
   
   /* clear the area */
   XtVaSetValues(w, XmNcells, NULL, NULL);
   
   for (ep = lFirst(lp), row = 0; ep; ep = lNext(ep), row++) {
      if (row == max_rows) {
         XbaeMatrixAddRows(w, 
                           max_rows, 
                           NULL,       /* empty rows  */
                           NULL,       /* no lables   */
                           NULL,       /* no different colors */
                           1);         /* we add 1 row */
         max_rows++;
      }
      if (getCE_TypeValues(ep, ce_entry)) {
         if (full) {
            for (i=0; i<CE_MAX; i++) {
               /* FIX_CONST_GUI */
               XbaeMatrixSetCell(w, row, i, 
                  ce_entry[i] ? (String)ce_entry[i] : "");
            }
         }
         else {
            /* FIX_CONST_GUI */
            XbaeMatrixSetCell(w, row, 0, 
               ce_entry[CE_NAME] ? (String)ce_entry[CE_NAME] : "");
            /* FIX_CONST_GUI */
            XbaeMatrixSetCell(w, row, 1, 
               ce_entry[CE_TYPE] ? (String)ce_entry[CE_TYPE] : "");
            /* FIX_CONST_GUI */
            XbaeMatrixSetCell(w, row, 2, 
               ce_entry[CE_RELOP] ? (String)ce_entry[CE_RELOP] : "");
         }
      }
      else
         DPRINTF(("qmonSetCE_Type failure\n"));
   }
       
   DEXIT;
}
   

/*-------------------------------------------------------------------------*/
static Boolean getCE_TypeValues(
lListElem *ep,
StringConst *ce_entry 
) {
   DENTER(GUI_LAYER, "getCE_TypeValues");
   
   if (!ep || !ce_entry ) {
      DEXIT;
      return False;
   }
      
   /* name type relation value */ 
   ce_entry[CE_NAME] = (StringConst)lGetString(ep, CE_name);
   ce_entry[CE_SHORTCUT] = (StringConst)lGetString(ep, CE_shortcut);
   ce_entry[CE_TYPE] = (StringConst)map_type2str(lGetUlong(ep, CE_valtype));
   ce_entry[CE_RELOP] = map_op2str(lGetUlong(ep, CE_relop));
   ce_entry[CE_REQUEST] = map_req2str(lGetUlong(ep, CE_requestable));
   ce_entry[CE_CONSUMABLE] = map_consumable2str(lGetUlong(ep, CE_consumable));
   ce_entry[CE_DEFAULT] = (StringConst)lGetString(ep, CE_default);
   ce_entry[CE_URGENCY] = (StringConst)lGetString(ep, CE_urgency_weight);
      
   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
static Boolean setCE_TypeValues(
lListElem *ep,
String *ce_entry 
) {
   int i, type, relop; 
   u_long32 requestable = REQU_NO;

   DENTER(GUI_LAYER, "setCE_TypeValues");
   
   if (!ep) {
      DEXIT;
      return False;
   }
      
   /* name type relation value */ 
   lSetString(ep, CE_name, ce_entry[CE_NAME] ? qmon_trim(ce_entry[CE_NAME]): "");
   lSetString(ep, CE_shortcut, ce_entry[CE_SHORTCUT] ? qmon_trim(ce_entry[CE_SHORTCUT]): "" );

   type = 0;
   for (i=TYPE_FIRST; !type && i<=TYPE_CE_LAST; i++) {
      if (!strcasecmp(ce_entry[CE_TYPE], map_type2str(i)))
         type = i;
   }
   if (!type) {
      DPRINTF(("setCE_TypeValues: unknown type\n"));
      DEXIT;
      return False;
   }
   lSetUlong(ep, CE_valtype, type);

   relop = 0;
   for (i=CMPLXEQ_OP; !relop && i<=CMPLXEXCL_OP; i++) {
      if (!strcmp(ce_entry[CE_RELOP], map_op2str(i)))
         relop = i;
   }
   if (!relop) {
      DPRINTF(("invalid relation operator: %s\n", ce_entry[CE_RELOP]));
      DEXIT;
      return False;
   }
   lSetUlong(ep, CE_relop, relop);

   if (!strcasecmp(ce_entry[CE_REQUEST], "y") 
            || !strcasecmp(ce_entry[CE_REQUEST], "yes"))
      requestable = REQU_YES;
   else if (!strcasecmp(ce_entry[CE_REQUEST], "n") 
            || !strcasecmp(ce_entry[CE_REQUEST], "no"))
      requestable = REQU_NO;
   else if (!strcasecmp(ce_entry[CE_REQUEST], "f") 
            || !strcasecmp(ce_entry[CE_REQUEST], "forced")) {
      requestable = REQU_FORCED;
   }
   else {
      DPRINTF(("invalid requestable entry: %s\n", ce_entry[CE_REQUEST]));
   }

   lSetUlong(ep, CE_requestable, requestable);

   if (!strcasecmp(ce_entry[CE_CONSUMABLE], "y") 
            || !strcasecmp(ce_entry[CE_CONSUMABLE], "yes"))
      lSetUlong(ep, CE_consumable, 1);
   else if (!strcasecmp(ce_entry[CE_CONSUMABLE], "j") 
            || !strcasecmp(ce_entry[CE_CONSUMABLE], "job"))
      lSetUlong(ep, CE_consumable, 2);
   else if (!strcasecmp(ce_entry[CE_CONSUMABLE], "n") 
            || !strcasecmp(ce_entry[CE_CONSUMABLE], "no"))
      lSetUlong(ep, CE_consumable, 0);

   lSetString(ep, CE_default, ce_entry[CE_DEFAULT] ? qmon_trim(ce_entry[CE_DEFAULT]): "");
   lSetString(ep, CE_urgency_weight, ce_entry[CE_URGENCY] ? qmon_trim(ce_entry[CE_URGENCY]): "");

   
   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static void set_CX_Type(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *lp = NULL;

   if (type != QmonQCX_Type) {
      XmtWarningMsg("XmtDialogSetDialogValues", "XbaeMatrix",
         "Type Mismatch: Widget '%s':\n\tCan't set widget values"
         " from a resource of type '%s'; QmonQCX_Type expected.",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   if (size == sizeof(lList*))
      lp = *(lList**) address;
   else
      return;

   qmonSetCE_Type(w, lp, CE_TYPE_FULL);

}


/*-------------------------------------------------------------------------*/
static void get_CX_Type(
Widget w,
XtPointer address,
XrmQuark type,
Cardinal size 
) {
   lList *lp = NULL;
   
   if (type != QmonQCX_Type) {
      XmtWarningMsg("XmtDialogSetDialogValues", "XbaeMatrix",
         "Type Mismatch: Widget '%s':\n\tCan't get widget values"
         " from a resource of type '%s'; QmonQCX_Type expected.",
          XtName(w), XrmQuarkToString(type));

      return;
   }

   lp = qmonGetCE_Type(w);
   
   *(lList**)address = lp;
}



/*-------------------------------------------------------------------------*/
lList* qmonGetCE_Type(
Widget w 
) {
   lList *lp = NULL;
   lListElem *ep;
   int row;
   int max_rows;
   char *ce_entry[CE_MAX]; 
   int k;

   DENTER(GUI_LAYER, "qmonGetCE_Type");
   
   XtVaGetValues(w, XmNrows, &max_rows, NULL);
   lp = lCreateList(XtName(w), CE_Type);
   for (row=0; row<max_rows && lp; row++) {
      for (k=0; k<CE_MAX; k++) 
         ce_entry[k] = XbaeMatrixGetCell(w, row, k);
      if (ce_entry[CE_NAME] && ce_entry[CE_NAME][0] != '\0') {
         ep = lCreateElem(CE_Type);
         setCE_TypeValues(ep, ce_entry);
         lAppendElem(lp, ep);
      }
      else
         break;
   }

   DEXIT;
   return lp;
}
   
