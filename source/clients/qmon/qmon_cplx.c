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
#define AUTOMATIC_UPDATE   0

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/ScrollBar.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Layout.h>
#include <Xmt/Chooser.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/MsgLine.h>
#include <Xmt/InputField.h>
#include <Xmt/Procedures.h>

#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_appres.h"
#include "qmon_timer.h"
#include "qmon_comm.h"
#include "qmon_matrix.h"
#include "qmon_message.h"
#include "qmon_cplx.h"
#include "qmon_globals.h"
#include "AskForItems.h"
#include "Matrix.h"

#include "uti/sge_io.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_centry.h"

#include "comm/commlib.h"

#include "sched/sge_complex_schedd.h"

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi.h"

#include "spool/flatfile/sge_flatfile.h"
#include "spool/flatfile/sge_flatfile_obj.h"

#include "sge_centry_qconf.h"
#include "msg_qmaster.h"
#include "msg_common.h"

extern sge_gdi_ctx_class_t *ctx;

enum _cplx_mode {
   CPLX_ADD_MODE,
   CPLX_MODIFY_MODE
};

static Widget qmon_cplx = 0;
static Widget attr_add = 0;
static Widget attr_mod = 0;
static Widget attr_del = 0;
static Widget attr_load = 0;
static Widget attr_save = 0;
static Widget attr_mx = 0;
static Widget attr_aname = 0;
static Widget attr_atype = 0;
static Widget attr_ashort = 0;
static Widget attr_arel = 0;
static Widget attr_areq = 0;
static Widget attr_aconsumable = 0;
static Widget attr_adefault = 0;
static Widget attr_aurgency = 0;


/*-------------------------------------------------------------------------*/
static void qmonPopdownCplxConfig(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreateCplxConfig(Widget parent);
static void qmonCplxOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonCplxLoadAttr(Widget matrix);
static void qmonCplxSaveAttr(Widget matrix);
static void qmonCplxDelAttr(Widget matrix);
static void qmonCplxAddAttr(Widget matrix, Boolean modify_mode);
static void qmonCplxNoEdit(Widget w, XtPointer cld, XtPointer cad);
static void qmonCplxSelectAttr(Widget w, XtPointer cld, XtPointer cad);
static void qmonCplxSortAttr(Widget w, XtPointer cld, XtPointer cad);
static void qmonCplxResetEntryLine(void);
static void qmonCplxAtypeAttr(Widget w, XtPointer cld, XtPointer cad);
static void qmonCplxAconsumableAttr(Widget w, XtPointer cld, XtPointer cad);

#if AUTOMATIC_UPDATE
static void qmonCplxStartUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonCplxStopUpdate(Widget w, XtPointer cld, XtPointer cad);
#endif
/*-------------------------------------------------------------------------*/
void qmonPopupCplxConfig(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;
   lList *alp = NULL;
   lList *entries = NULL;

   DENTER(GUI_LAYER, "qmonPopupCplxConfig");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   qmonMirrorMultiAnswer(CENTRY_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }
   entries = qmonMirrorList(SGE_CE_LIST);

   if (!qmon_cplx) {
      shell = XmtGetTopLevelShell(w);
      qmon_cplx = qmonCreateCplxConfig(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonPopdownCplxConfig, NULL);
      XtAddEventHandler(XtParent(qmon_cplx), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   } 
   XSync(XtDisplay(qmon_cplx), 0);
   XmUpdateDisplay(qmon_cplx);


   /*
   ** fill the values into the matrix
   */
   qmonSetCE_Type(attr_mx, entries, CE_TYPE_FULL);

   /*
   ** reset entry line
   */
   qmonCplxResetEntryLine();

   /*
   ** reset attr_mod sensitivity
   */
   XtSetSensitive(attr_mod, False);

   XtManageChild(qmon_cplx);
   XRaiseWindow(XtDisplay(XtParent(qmon_cplx)), 
                  XtWindow(XtParent(qmon_cplx)));
 
   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
/* P R I V A T E                                                           */
/*-------------------------------------------------------------------------*/
static Widget qmonCreateCplxConfig(
Widget parent 
) {
   Widget cplx_layout, cplx_commit, cplx_reset, cplx_main_link;
   static XmtProcedureInfo cplx_procedures[] = {
         {"ComplexAttributeDelete", (XmtProcedure) qmonCplxDelAttr, {XtRWidget}},
         {"ComplexAttributeAdd", (XmtProcedure) qmonCplxAddAttr, {XtRWidget, XtRBoolean}},
         {"ComplexAttributeModify", (XmtProcedure) qmonCplxAddAttr, {XtRWidget, XtRBoolean}},
         {"ComplexAttributeLoad", (XmtProcedure) qmonCplxLoadAttr, {XtRWidget}},
         {"ComplexAttributesSave", (XmtProcedure) qmonCplxSaveAttr, {XtRWidget}}
   };


   DENTER(GUI_LAYER, "qmonCreateCplxConfig");
   
   cplx_layout = XmtBuildQueryDialog( parent, "qmon_cplx",
                           NULL, 0,
                           "cplx_commit", &cplx_commit,
                           "cplx_reset", &cplx_reset,
                           "cplx_main_link", &cplx_main_link,
                           "attr_add", &attr_add,
                           "attr_mod", &attr_mod,
                           "attr_del", &attr_del,
                           "attr_load", &attr_load,
                           "attr_save", &attr_save,
                           "attr_mx", &attr_mx,
                           "attr_aname", &attr_aname,
                           "attr_ashort", &attr_ashort,
                           "attr_atype", &attr_atype,
                           "attr_arel", &attr_arel,
                           "attr_areq", &attr_areq,
                           "attr_aconsumable", &attr_aconsumable,
                           "attr_adefault", &attr_adefault,
                           "attr_aurgency", &attr_aurgency,
                           NULL);

   XtAddCallback(cplx_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);

   XtAddCallback(cplx_commit, XmNactivateCallback, 
                     qmonCplxOk, NULL);
   XtAddCallback(cplx_reset, XmNactivateCallback, 
                     qmonPopdownCplxConfig, NULL);
   XtAddCallback(attr_mx, XmNenterCellCallback, 
                     qmonCplxNoEdit, NULL);
   XtAddCallback(attr_mx, XmNselectCellCallback, 
                     qmonCplxSelectAttr, NULL);
   XtAddCallback(attr_mx, XmNlabelActivateCallback, 
                     qmonCplxSortAttr, NULL);

   XtAddCallback(attr_atype, XmtNvalueChangedCallback,
                     qmonCplxAtypeAttr, NULL);
   XtAddCallback(attr_aconsumable, XmtNvalueChangedCallback,
                     qmonCplxAconsumableAttr, NULL);

   /*
   ** register callback procedures
   */
   XmtRegisterProcedures(cplx_procedures, XtNumber(cplx_procedures));

   DEXIT;
   return cplx_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonPopdownCplxConfig(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPopdownCplxConfig");

   XtUnmanageChild(qmon_cplx);
   XtPopdown(XtParent(qmon_cplx));

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCplxOk(Widget w, XtPointer cld, XtPointer cad)
{
   static lEnumeration *what = NULL;
   lList *entries = NULL;
   lList *old_entries = NULL;
   lList *alp = NULL;
   int error = 0;
   
   DENTER(GUI_LAYER, "qmonCplxOk");

   if (!what)
      what = lWhat("%T(ALL)", CE_Type);

   entries = qmonGetCE_Type(attr_mx);
   old_entries = lCopyList("", qmonMirrorList(SGE_CE_LIST));
   /* centry_list_add_del_mod_via_gdi free entries and old_entries */
   centry_list_add_del_mod_via_gdi(ctx, &entries, &alp, &old_entries);                     
   lFreeList(&old_entries);

   error = qmonMessageBox(w, alp, 0);
   lFreeList(&alp);

   if (error == -1) {
      DEXIT;
      return;
   }   

   qmonMirrorMultiAnswer(CENTRY_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   
   XbaeMatrixDeselectAll(attr_mx);
   XtUnmanageChild(qmon_cplx);
   XtPopdown(XtParent(qmon_cplx));

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCplxAddAttr(Widget matrix, Boolean modify_mode)
{
   String new_str[1][8];
   int rows = 0;
   int num_columns = 8;
   int row, i;
   int row_to_change = 0;
   String str = NULL;
   int req_state;
   int requestable = 0;
   int valtype = 0;
   int consumable = 0;
   int relop = 0;
   Boolean ns_os_conflict = False;
   Boolean nn_os_conflict = False;
   Boolean ns_on_conflict = False;
/*    Pixel color = JobErrPixel; */
   Boolean attr_exists = False;
   lList *alp = NULL;
   lListElem *new_entry = NULL;

   DENTER(GUI_LAYER, "qmonCplxAddAttr");

   /*
   ** check input 
   */
   if (is_empty_word(XmtInputFieldGetString(attr_aname))) {
      qmonMessageShow(matrix, True, "Name required !");
      DEXIT;
      return;
   }
   if (is_empty_word(XmtInputFieldGetString(attr_ashort))) {
      qmonMessageShow(matrix, True, "Shortcut required !");
      DEXIT;
      return;
   }

   if (is_empty_word(XmtInputFieldGetString(attr_adefault))) {
      qmonMessageShow(matrix, True, "Default required !");
      DEXIT;
      return;
   }
      
   if (is_empty_word(XmtInputFieldGetString(attr_aurgency))) {
      qmonMessageShow(matrix, True, "Urgency required !");
      DEXIT;
      return;
   }

   /*
   ** get the values
   */
   new_str[0][0] = XtNewString(XmtInputFieldGetString(attr_aname));
   new_str[0][1] = XtNewString(XmtInputFieldGetString(attr_ashort));
   valtype = XmtChooserGetState(attr_atype) + 1;
   new_str[0][2] = XtNewString(map_type2str(valtype));
   relop = XmtChooserGetState(attr_arel) + 1;
   new_str[0][3] = XtNewString(map_op2str(relop));
   req_state = XmtChooserGetState(attr_areq); 

   switch (req_state) {
      case 1:  
         new_str[0][4] = XtNewString("YES");
         requestable = REQU_YES;
         break;
      case 2:  
         new_str[0][4] = XtNewString("FORCED");
         requestable = REQU_FORCED;
         break;
      default:
         new_str[0][4] = XtNewString("NO");
         requestable = REQU_NO;
   }
      
   consumable = XmtChooserGetState(attr_aconsumable);   
   new_str[0][5] = XtNewString(map_consumable2str(consumable));
   new_str[0][6] = XtNewString(XmtInputFieldGetString(attr_adefault));
   new_str[0][7] = XtNewString(XmtInputFieldGetString(attr_aurgency));
         
   
   /*
   ** check the attribute line entries
   */
   new_entry = lCreateElem(CE_Type);
   lSetString(new_entry, CE_name, new_str[0][0]);
   lSetString(new_entry, CE_shortcut, new_str[0][1]);
   lSetUlong(new_entry, CE_valtype, valtype);
   lSetUlong(new_entry, CE_relop, relop);
   lSetUlong(new_entry, CE_consumable, consumable);
   lSetUlong(new_entry, CE_requestable, requestable);
   if (consumable) {
      lSetString(new_entry, CE_default, new_str[0][6]);
   }   
   lSetString(new_entry, CE_urgency_weight, new_str[0][7]);

   if (!centry_elem_validate(new_entry, NULL, &alp)) {
      qmonMessageBox(matrix, alp, 0);
      lFreeList(&alp);
      lFreeElem(&new_entry);
      goto error;
   }   

   lFreeElem(&new_entry);


   /*
   ** add to attribute matrix, search if item already exists
   */
   rows = XbaeMatrixNumRows(matrix);

   if (!modify_mode) {
      /*
      ** check if the name already exists
      */
      for (row=0; row<rows; row++) {
         /* get name str */
         str = XbaeMatrixGetCell(matrix, row, 0);
         if (!str || (str && !strcmp(str, new_str[0][0])) ||
               (str && is_empty_word(str)))
            break;
      }

      if (str && !strcmp(str, new_str[0][0])) 
         attr_exists = True;

      row_to_change = row;
   } else {
      row_to_change = XbaeMatrixFirstSelectedRow(matrix);
      if (row_to_change == -1) {
         qmonMessageShow(matrix, True, "No attribute entry selected !");
         goto error;
      }   
   }   
   

   if (!modify_mode && attr_exists) {
      qmonMessageShow(matrix, True, "Attribute already exists !");
      goto error;
   }
      

   /*
   ** check if new and old shortcut conflict 
   */
   for (row=0; row<rows; row++) {
      /* get name str */
      str = XbaeMatrixGetCell(matrix, row, 1);
      if (str && !strcmp(str, new_str[0][1]) && !modify_mode) {
         ns_os_conflict = True;   
         break;
      }   
   }

   /*
   ** check if new name and old shortcut conflict 
   */
   for (row=0; row<rows; row++) {
      /* get name str */
      str = XbaeMatrixGetCell(matrix, row, 1);
      if (str && !strcmp(str, new_str[0][0]) && !modify_mode) {
         nn_os_conflict = True;   
         break;
      }   
   }
   
   /*
   ** check if new shortcut and old name conflict 
   */
   for (row=0; row<rows; row++) {
      /* get name str */
      str = XbaeMatrixGetCell(matrix, row, 0);
      if (str && !strcmp(str, new_str[0][1]) && !modify_mode) {
         ns_on_conflict = True;   
         break;
      }   
   }

   if (ns_os_conflict) {
      qmonMessageShow(matrix, True, "New shortcut and old shortcut conflict !");
      goto error;
   }   
   if (ns_on_conflict) {
      qmonMessageShow(matrix, True, "New shortcut and old name conflict !");
      goto error;
   }   
   if (nn_os_conflict) {
      qmonMessageShow(matrix, True, "New name and old shortcut conflict !");
      goto error;
   }   
   
   if (modify_mode)
      XbaeMatrixDeleteRows(matrix, row_to_change, 1);

   /* 
   ** add new rows at the top 
   */
   if (!modify_mode)
      row_to_change=0;
   XbaeMatrixAddRows(matrix, row_to_change, new_str[0], NULL, NULL, 1);
/*    XbaeMatrixSetRowBackgrounds(matrix, modify_mode ? row_to_change : 0, &color, 1); */

   /* reset and jump to attr_aname */
   XbaeMatrixDeselectAll(matrix);
   XtSetSensitive(attr_mod, False);
   /* refresh the matrix */
   XbaeMatrixRefresh(matrix);
   XbaeMatrixMakeCellVisible(matrix, row_to_change, 0);
   
   qmonCplxResetEntryLine();
   XmProcessTraversal(attr_aname, XmTRAVERSE_CURRENT);

error:

   for (i=0; i<num_columns; i++) {
      XtFree((char*)new_str[0][i]);
      new_str[0][i] = NULL;
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCplxDelAttr(Widget matrix)
{
   int rows;
   int i;
   int rows_to_delete = 0;
   lList *alp = NULL;
   

   DENTER(GUI_LAYER, "qmonCplxDelAttr");

   rows = XbaeMatrixNumRows(matrix);

   for (i=0; i<rows; i++) {
      if (XbaeMatrixIsRowSelected(matrix, i)) {
         /* check if its a build in value */
         const char *name = XbaeMatrixGetCell(matrix, i, 0);
         if (get_rsrc(name, true, NULL, NULL, NULL, NULL)==0 || 
             get_rsrc(name, false, NULL, NULL, NULL, NULL)==0) {
            XbaeMatrixDeselectRow(matrix, i);
            answer_list_add_sprintf(&alp, STATUS_EUNKNOWN , ANSWER_QUALITY_ERROR, 
                                    MSG_INVALID_CENTRY_DEL_S, name);
         } else {
            rows_to_delete++;
         }   
      }
   }
   qmonMessageBox(matrix, alp, 0);
   lFreeList(&alp);

   i = 0;
   while (i<rows) {
      if (XbaeMatrixIsRowSelected(matrix, i)) {
         XbaeMatrixDeleteRows(matrix, i, 1);
         rows--;
      }
      else
         i++;
   }

   XbaeMatrixAddRows(matrix, rows, NULL, NULL, NULL, rows_to_delete);

   /* reset entry line */
   qmonCplxResetEntryLine();

   /* refresh the matrix */
   XbaeMatrixRefresh(matrix);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonCplxLoadAttr(Widget matrix)
{
   static char filename[BUFSIZ];
   static char directory[BUFSIZ];
   int status;
   lList *entries;
   lList *alp = NULL;
 
   DENTER(GUI_LAYER, "qmonCplxLoadAttr");

   status = XmtAskForFilename(matrix, "@{File Selection}",
                              "@{Please type or select a filename}",
                              NULL, NULL,
                              filename, sizeof(filename),
                              directory, sizeof(directory),
                              "*", 0,
                              NULL);

   if (status == True) {
      entries = spool_flatfile_read_list(&alp, CE_Type,
                                   CE_fields, NULL, true, &qconf_ce_list_sfi,
                                   SP_FORM_ASCII, NULL, filename);

      /* fill the matrix with the values from list */ 
      if (alp) {
         qmonMessageBox(matrix, alp, 0);
         lFreeList(&alp);
         DEXIT;
         return;
      }

      /*
      ** fill the values into the matrix
      */
      qmonSetCE_Type(matrix, entries, CE_TYPE_FULL);
      lFreeList(&entries);
   }        
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonCplxSaveAttr(Widget matrix)
{
   static char filename[BUFSIZ];
   char *file_tmp = NULL;
   static char directory[BUFSIZ];
   int status;
   lList *entries;
   lList *alp = NULL;
 
   DENTER(GUI_LAYER, "qmonCplxSaveAttr");

   status = XmtAskForFilename(matrix, "@{File Selection}",
                              "@{Please type or select a filename}",
                              NULL, NULL,
                              filename, sizeof(filename),
                              directory, sizeof(directory),
                              "*", 0,
                              NULL);

   if (status == True) {
      entries = qmonGetCE_Type(matrix);
      /* Save Cplx Dialog Box */
      file_tmp = (char*)spool_flatfile_write_list(&alp, entries,
                                           CE_fields,
                                           &qconf_ce_list_sfi,
                                           SP_DEST_TMP, 
                                           SP_FORM_ASCII, 
                                           NULL, false);
      if (file_tmp != NULL) {
         if (sge_copy_append(file_tmp, filename, SGE_MODE_COPY) == -1) {
            answer_list_add_sprintf(&alp, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_ERRORRENAMING_SSS, 
                                    file_tmp, filename, strerror(errno));
         }
      }
      qmonMessageBox(matrix, alp, 0);
      unlink(file_tmp);
      sge_free(&file_tmp);
      lFreeList(&alp);
      lFreeList(&entries);
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCplxAtypeAttr(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = 
            (XmtChooserCallbackStruct*) cad;
   int type = cbs->state  + 1;         
   int i;

   DENTER(GUI_LAYER, "qmonCplxAtypeAttr");

   if (type != TYPE_INT && type != TYPE_DOUBLE &&
       type != TYPE_MEM && type != TYPE_TIM && type != TYPE_BOO) {
      XmtChooserSetSensitive(attr_aconsumable, 1, False);
      XmtChooserSetSensitive(attr_aconsumable, 2, False);
      XmtChooserSetState(attr_aconsumable, 0, True);
   } else {   
      XmtChooserSetSensitive(attr_aconsumable, 1, True); 
      XmtChooserSetSensitive(attr_aconsumable, 2, True);
   }

   /* reset sensitivity to true */
   for (i=0; i<7; i++)
      XmtChooserSetSensitive(attr_arel, i, True); 
   
   if (type == TYPE_STR || type == TYPE_CSTR || type == TYPE_HOST || type == TYPE_RESTR) {
      XmtInputFieldSetString(attr_adefault, "NONE");
      XmtChooserSetState(attr_arel, 0, False);
      for (i=1; i<5; i++)
         XmtChooserSetSensitive(attr_arel, i, False); 
   }
   
   if (type == TYPE_BOO) {
      XmtInputFieldSetString(attr_adefault, "FALSE");
      XmtChooserSetState(attr_arel, 0, False);
      for (i=1; i<7; i++) {
         XmtChooserSetSensitive(attr_arel, i, False); 
      }
   } else {
      XmtChooserSetSensitive(attr_arel, 6, False); 
   }

   if (type == TYPE_MEM || type == TYPE_INT || type == TYPE_DOUBLE) {
      XmtInputFieldSetString(attr_adefault, "0");
   }       

   if (type == TYPE_TIM) {
      XmtInputFieldSetString(attr_adefault, "0:0:0");
   }       

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCplxAconsumableAttr(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = 
            (XmtChooserCallbackStruct*) cad;
   int type;
   int i;

   DENTER(GUI_LAYER, "qmonCplxAconsumableAttr");

   if (cbs->state != 0) { 
      /* reset sensitivity to false for all apart from <= */
      for (i=0; i<7; i++)
         XmtChooserSetSensitive(attr_arel, i, False); 

      type = XmtChooserGetState(attr_atype) + 1;
      if (type == TYPE_BOO) {
         XmtChooserSetSensitive(attr_arel, 6, True); 
         XmtChooserSetState(attr_arel, 6, False);
      } else {
         XmtChooserSetSensitive(attr_arel, 4, True); 
         XmtChooserSetState(attr_arel, 4, False);
      }
      XtSetSensitive(attr_adefault, true);
   } else {   
      /* set default value for all non consumables depending on type */
      XtSetSensitive(attr_adefault, false);
      /* reset sensitivity to true */
      for (i=0; i<7; i++)
         XmtChooserSetSensitive(attr_arel, i, True); 
      
      type = XmtChooserGetState(attr_atype) + 1;
      if (type == TYPE_STR || type == TYPE_CSTR || type == TYPE_HOST || type == TYPE_RESTR) {
         XmtInputFieldSetString(attr_adefault, "NONE"); 
         XmtChooserSetState(attr_arel, 0, False);
         for (i=1; i<5; i++)
            XmtChooserSetSensitive(attr_arel, i, False); 
      }       
      
      if (type == TYPE_BOO) {
         XmtInputFieldSetString(attr_adefault, "FALSE"); 
         XmtChooserSetState(attr_arel, 0, False);
         for (i=1; i<7; i++)
            XmtChooserSetSensitive(attr_arel, i, False); 
      }       

      if (type == TYPE_MEM || type == TYPE_INT || type == TYPE_DOUBLE) {
         XmtInputFieldSetString(attr_adefault, "0"); 
      }   

      if (type == TYPE_TIM) {
         XmtInputFieldSetString(attr_adefault, "0:0:0"); 
      }   

   }

   DEXIT;
}
/*-------------------------------------------------------------------------*/
static void qmonCplxSelectAttr(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixSelectCellCallbackStruct *cbs = 
            (XbaeMatrixSelectCellCallbackStruct*) cad;
   int i;
   String str;
   int type, relop;

   DENTER(GUI_LAYER, "qmonCplxSelectAttr");

   if (cbs->num_params && !strcmp(cbs->params[0], "begin")) {
   
      XtSetSensitive(attr_mod, True);

      /* name */
      str = XbaeMatrixGetCell(w, cbs->row, 0);
      XmtInputFieldSetString(attr_aname, str ? str : ""); 

      /* shortcut */
      str = XbaeMatrixGetCell(w, cbs->row, 1);
      XmtInputFieldSetString(attr_ashort, str ? str : "");
      
      /* type */
      str = XbaeMatrixGetCell(w, cbs->row, 2);
      type = 0;
      for (i=TYPE_FIRST; !type && i<=TYPE_CE_LAST; i++) {
         if (!strcasecmp(str, map_type2str(i)))
            type = i-1;
      }
      XmtChooserSetState(attr_atype, type, True); 

      /* relop */
      str = XbaeMatrixGetCell(w, cbs->row, 3);
      relop = 0;
      for (i = CMPLXEQ_OP; !relop && i <= CMPLXEXCL_OP; i++) {
         if (!strcasecmp(str, map_op2str(i))) {
            relop = i-1;
         }
      }
      XmtChooserSetState(attr_arel, relop, True); 

      str = XbaeMatrixGetCell(w, cbs->row, 4);
      if (str && !strcmp(str, "YES")) 
         XmtChooserSetState(attr_areq, 1, True);
      else if (str && !strcmp(str, "FORCED"))
         XmtChooserSetState(attr_areq, 2, True);
      else
         XmtChooserSetState(attr_areq, 0, True);

      str = XbaeMatrixGetCell(w, cbs->row, 5);
      if (str && !strcmp(str, "YES")) {
         XmtChooserSetState(attr_aconsumable, 1, True);
      } else if (str && !strcmp(str, "JOB")) {
         XmtChooserSetState(attr_aconsumable, 2, True);
      } else {
         XmtChooserSetState(attr_aconsumable, 0, True);
      }

      /* default */
      str = XbaeMatrixGetCell(w, cbs->row, 6);
      XmtInputFieldSetString(attr_adefault, str ? str : ""); 

      /* urgency */
      str = XbaeMatrixGetCell(w, cbs->row, 7);
      XmtInputFieldSetString(attr_aurgency, str ? str : ""); 
   }
   
   if ((XbaeMatrixGetNumSelected(w)/XbaeMatrixNumColumns(w)) != 1) {
         XtSetSensitive(attr_mod, False);
         qmonCplxResetEntryLine();
   }   

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCplxResetEntryLine(void)
{

   DENTER(GUI_LAYER, "qmonCplxResetEntryLine");

   /* name */
   XmtInputFieldSetString(attr_aname, ""); 

   /* shortcut */
   XmtInputFieldSetString(attr_ashort, "");
   
   /* type */
   XmtChooserSetState(attr_atype, 0, True); 

   /* relop */
   XmtChooserSetState(attr_arel, 0, True); 

   /* requestable */
   XmtChooserSetState(attr_areq, 0, True);

   /* consumable */
   XmtChooserSetState(attr_aconsumable, 0, True);

   /* default */
   XmtInputFieldSetString(attr_adefault, "0"); 
   XtSetSensitive(attr_adefault, false);
   
   /* urgency */
   XmtInputFieldSetString(attr_aurgency, "0"); 
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCplxNoEdit(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixEnterCellCallbackStruct *cbs = 
            (XbaeMatrixEnterCellCallbackStruct*) cad;

   DENTER(GUI_LAYER, "qmonCplxNoEdit");

   cbs->doit = False;
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCplxSortAttr(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixLabelActivateCallbackStruct *cbs = 
            (XbaeMatrixLabelActivateCallbackStruct *) cad;
   lList *entries = NULL;
   int column_nm[] = {CE_name, CE_shortcut, CE_valtype, CE_relop, CE_consumable, CE_default};
   
   DENTER(GUI_LAYER, "qmonCplxAttrSort");

   entries = qmonGetCE_Type(attr_mx);
   if (cbs->column > XtNumber(column_nm))
      cbs->column=0;
   lPSortList(entries, "%I+%I+", column_nm[cbs->column], column_nm[0]); 
   qmonSetCE_Type(attr_mx, entries, CE_TYPE_FULL);
   lFreeList(&entries);
   
   DEXIT;
}
