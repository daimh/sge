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



#define QALTER_RUNNING


#include <stdio.h>
#include <stdlib.h>
#include <Xm/ToggleB.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Icon.h>
#include <Xmt/Dialogs.h>

#include "Matrix.h"
#include "Tab.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_ar.h"
#include "qmon_arsub.h"
#include "qmon_arcustom.h"
#include "qmon_comm.h"
#include "qmon_timer.h"
#include "qmon_globals.h"
#include "qmon_browser.h"
#include "qmon_message.h"
#include "qmon_init.h"
#include "qmon_util.h"
#include "qmon_matrix.h"

#include "uti/sge_dstring.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_advance_reservation.h"

#include "sched/sgeee.h"

#include "gdi/sge_gdi_ctx.h"

#include "basis_types.h"

extern sge_gdi_ctx_class_t *ctx;

enum {
   AR_DISPLAY_MODE_RUNNING,
   AR_DISPLAY_MODE_PENDING
};

typedef struct _tAskHoldInfo {
   Boolean blocked;
   XmtButtonType button;
   Widget AskHoldDialog;
   Widget AskHoldFlags;
   Widget AskHoldTasks;
} tAskHoldInfo;
   
/* a boolean for the sort order */
enum {
  AR_sort_descending = 0,
  AR_sort_ascending
};



static Widget qmon_ar = 0;
static Widget ar_running_ars = 0;
#ifdef AR_PENDING
static Widget ar_pending_ars = 0;
#endif
static Widget current_matrix = 0;
static Widget ar_customize = 0;
static Widget ar_delete = 0;
static Widget ar_select_all = 0;
static Widget force_toggle = 0;


/*-------------------------------------------------------------------------*/
static void qmonCreateARControl(Widget parent);
static void qmonARToMatrix(Widget w, lListElem *ar, int mode, int row);
static void qmonARFolderChange(Widget w, XtPointer cld, XtPointer cad);
static void qmonDeleteARCB(Widget w, XtPointer cld, XtPointer cad);
static void qmonSelectAllARCB(Widget w, XtPointer cld, XtPointer cad);
static void qmonARPopdown(Widget w, XtPointer cld, XtPointer cad);
static void qmonARStartUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonARStopUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonARHandleEnterLeave(Widget w, XtPointer cld, XEvent *ev, Boolean *ctd);
static void qmonARShowBrowserInfo(dstring *info, lListElem *jep);
/* static void qmonARChangeState(Widget w, XtPointer cld, XtPointer cad); */
static void qmonARNoEdit(Widget w, XtPointer cld, XtPointer cad);
static Pixel qmonARStateToColor(Widget w, lListElem *jep);
static Boolean qmonDeleteARForMatrix(Widget w, Widget matrix, lList **local);
/* static lList* qmonARBuildSelectedList(Widget matrix, lDescr *dp, int nm); */
/* static void qmonResizeCB(Widget w, XtPointer cld, XtPointer cad); */
static void qmonARSort(Widget w, XtPointer cld, XtPointer cad);
/*-------------------------------------------------------------------------*/
static int field_sort_by;
static int field_sort_direction;
static int arnum_sort_direction;

/*-------------------------------------------------------------------------*/
/* P U B L I C                                                             */
/*-------------------------------------------------------------------------*/
void qmonARPopup(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonARPopup");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_ar) {
      qmonMirrorMultiAnswer(AR_T, &alp);
      if (alp) {
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);
         /* set normal cursor */
         XmtDisplayDefaultCursor(w);
         DEXIT;
         return;
      }
         
      qmonCreateARControl(AppShell);
      /*
      ** create the ar customize dialog
      */
      qmonCreateARCU(qmon_ar, NULL);
      /* 
      ** set the close button callback 
      ** set the icon and iconName
      */
      XmtCreatePixmapIcon(qmon_ar, qmonGetIcon("toolbar_ar"), None);
      XtVaSetValues(qmon_ar, XtNiconName, "qmon:AR Control", NULL);
      XmtAddDeleteCallback(qmon_ar, XmDO_NOTHING, qmonARPopdown,  NULL);
      XtAddEventHandler(qmon_ar, StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
      XtAddEventHandler(qmon_ar, StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);
   }

   updateARListCB(qmon_ar, NULL, NULL);

   xmui_manage(qmon_ar);

   /* set normal cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonARReturnMatrix(
Widget *run_m,
Widget *pen_m
) {
   DENTER(GUI_LAYER, "qmonARReturnMatrix");

   *run_m = ar_running_ars;
#ifdef AR_PENDING   
   *pen_m = ar_pending_ars;
#endif   

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updateARListCB(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   qmonMirrorMultiAnswer(AR_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      return;
   }

   updateARList();

   /* set default cursor */
   XmtDiscardButtonEvents(w);
   XmtDisplayDefaultCursor(w);
}


/*-------------------------------------------------------------------------*/
/* P R I V A T E                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static void qmonARPopdown(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonARPopdown");

   xmui_unmanage(qmon_ar);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonARStartUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonARStartUpdate");
  
   qmonTimerAddUpdateProc(AR_T, "updateARList", updateARList);
   qmonStartTimer(AR_T);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonARStopUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonARStopUpdate");
  
   qmonStopTimer(AR_T);
   qmonTimerRmUpdateProc(AR_T, "updateARList");
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonCreateARControl(
Widget parent 
) {
   Widget ar_submit, ar_update, ar_done,
          ar_main_link, ar_pending, ar_folder;
#ifdef AR_PENDING          
   static Widget pw[2];
#endif   
   static Widget rw[2];
   
   DENTER(GUI_LAYER, "qmonCreateARControl");

   qmon_ar = XmtBuildQueryToplevel( parent, 
                                     "qmon_ar",
                                     "ar_running_ars", &ar_running_ars,
#ifdef AR_PENDING                                     
                                     "ar_pending_ars", &ar_pending_ars,
#endif
                                     "ar_delete", &ar_delete,
                                     "ar_select_all", &ar_select_all,
                                     "ar_update", &ar_update,
                                     "ar_customize", &ar_customize,
                                     "ar_submit", &ar_submit,
                                     "ar_done", &ar_done,
                                     "ar_main_link", &ar_main_link,
                                     "ar_folder", &ar_folder,
                                     "ar_pending", &ar_pending,
                                     "ar_force",  &force_toggle,
                                     NULL);
#ifdef AR_PENDING
   pw[0] = ar_pending_ars;
   pw[1] = NULL;
#endif   
   rw[0] = ar_running_ars;
   rw[1] = NULL;


   /* start the needed timers and the corresponding update routines */
   XtAddCallback(qmon_ar, XmNpopupCallback, 
                     qmonARStartUpdate, NULL);
   XtAddCallback(qmon_ar, XmNpopdownCallback,
                     qmonARStopUpdate, NULL);
                     
   XtAddCallback(ar_folder, XmNvalueChangedCallback, 
                     qmonARFolderChange, NULL);
   XmTabSetTabWidget(ar_folder, ar_pending, True);

   XtAddCallback(ar_delete, XmNactivateCallback, 
                     qmonDeleteARCB, NULL);
   XtAddCallback(ar_select_all, XmNactivateCallback, 
                     qmonSelectAllARCB, NULL);
   XtAddCallback(ar_update, XmNactivateCallback, 
                     updateARListCB, NULL);
   XtAddCallback(ar_customize, XmNactivateCallback,  
                     qmonPopupARCU, NULL); 
   XtAddCallback(ar_submit, XmNactivateCallback, 
                     qmonARSubPopup, NULL);
   XtAddCallback(ar_done, XmNactivateCallback, 
                     qmonARPopdown, NULL);
   XtAddCallback(ar_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
/*    XtAddCallback(ar_running_ars, XmNresizeColumnCallback,  */
/*                      qmonResizeCB, NULL); */
   XtAddCallback(ar_running_ars, XmNenterCellCallback, 
                     qmonARNoEdit, NULL);
   XtAddCallback(ar_running_ars, XmNselectCellCallback, 
                     qmonMatrixSelect, (XtPointer) rw);
   XtAddCallback(ar_running_ars, XmNlabelActivateCallback, 
                     qmonARSort, NULL);
   /* Event Handler to display additional ar info */
   XtAddEventHandler(ar_running_ars, PointerMotionMask, 
                     False, qmonARHandleEnterLeave,
                     NULL); 

#ifdef AR_PENDING                     
   XtAddCallback(ar_pending_ars, XmNenterCellCallback, 
                     qmonARNoEdit, NULL);
   XtAddCallback(ar_pending_ars, XmNselectCellCallback, 
                     qmonMatrixSelect, (XtPointer) pw);
   XtAddCallback(ar_pending_ars, XmNlabelActivateCallback, 
                     qmonARSort, NULL);
   /* Event Handler to display additional ar info */
   XtAddEventHandler(ar_pending_ars, PointerMotionMask, 
                     False, qmonARHandleEnterLeave,
                     NULL); 

   current_matrix = ar_pending_ars;
#else
   current_matrix = ar_running_ars;
#endif

   /* initialising sort order to decreasing priority then increasing ar number */
   field_sort_by = AR_id;
   field_sort_direction = AR_sort_ascending;
   arnum_sort_direction = AR_sort_ascending; 

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonARToMatrix(
Widget w,
lListElem *ar,
int mode,
int row                                                                       
) {
   int rows, columns;
   int col;
   String *rowa = NULL; 
   Pixel color;
   
   DENTER(GUI_LAYER, "qmonARToMatrix");

   rows = XbaeMatrixNumRows(w);
   columns = XbaeMatrixNumColumns(w);

   /*
   ** running ars
   */
   if (mode == AR_DISPLAY_MODE_RUNNING) {
      rowa = PrintARField(ar, columns);
      if (row < rows)
         XbaeMatrixDeleteRows(w, row, 1);
      XbaeMatrixAddRows(w, row, rowa, NULL, NULL, 1);
      /*
      ** do show the different ar states we show the ar id in a different           ** color
      */
      color = qmonARStateToColor(w, ar);
      XbaeMatrixSetRowColors(w, row, &color, 1);
   }

   /*
   ** pending ars
   */
   if (mode == AR_DISPLAY_MODE_PENDING) {
      if (row < rows)
         XbaeMatrixDeleteRows(w, row, 1);
      rowa = PrintARField(ar, columns);
      XbaeMatrixAddRows(w, row, rowa, NULL, NULL, 1);
      /*
      ** do show the different ar states we show the ar id in a different           ** color
      */
      color = qmonARStateToColor(w, ar);
      XbaeMatrixSetRowColors(w, row, &color, 1);
   }

   /*
   ** free row array
   */
   if (rowa) {
      for (col=0; col<columns; col++)
         XtFree((char*)rowa[col]);
      XtFree((char*)rowa);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Pixel qmonARStateToColor(
Widget w,
lListElem *ar 
) {
   Pixel color;
   static Pixel fg = 0;
/*    int state; */

   DENTER(GUI_LAYER, "qmonARStateToColor");

   if (!fg) {
      XtVaGetValues(w, XmNforeground, &fg, NULL);
   }   

   if (!ar) {
      DEXIT;
      return fg;
   }
      

#if 0
   state = (int)lGetUlong(ar, AR_state);
#endif

   /*
   ** the order is important
   */

   color = fg;

#if 0
   if (state & AR_WAITING)
      color = ARWaitingPixel;

   if (state & AR_RUNNING)
      color = ARRunningPixel;

   if (state & AR_Exited)
      color = ARExitedPixel;

   if (state & AR_DELETED)
      color = ARDeletedPixel;

   if (state & AR_ERROR)
      color = ARErrorPixel;

   if (state & AR_WARNING)
      color = ARWarningPixel;
#endif      

   DEXIT;
   return color;

}

/*-------------------------------------------------------------------------*/
static void qmonARNoEdit(
Widget w,
XtPointer cld,
XtPointer cad  
) {
   XbaeMatrixEnterCellCallbackStruct *cbs =
         (XbaeMatrixEnterCellCallbackStruct*) cad;

   DENTER(GUI_LAYER, "qmonARNoEdit");

   cbs->doit = False;
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonARSort(
Widget w,
XtPointer cld,
XtPointer cad  
) {
   XbaeMatrixLabelActivateCallbackStruct *cbs = 
            (XbaeMatrixLabelActivateCallbackStruct *) cad;
   int column_nm[] = {AR_id, AR_name, AR_owner, AR_state, AR_start_time, AR_end_time, AR_duration};
   int* col_nm_addr = NULL;
   int col_nm = -1;

   DENTER(GUI_LAYER, "qmonARSort");
 
   DPRINTF(("ARSort = cbs->column = %d\n", cbs->column));
   
#if 0   
   /* not coping beyond 4th column */
   if ( cbs->column > 3) {
      DEXIT;
      return;
   }
#endif   

   /* 
   ** Here is what we are going to do for sorting:
   ** if the user clicks on a column, we'll sort ascending,
   ** doing a 2nd time toggles to decending; except
   ** we always toggle on the ar number the first time
   ** it is selected from a different column, since it is
   ** always a secondary sort key and if it is selected
   ** the user most likely wants a toggle.
   */

   col_nm_addr = XbaeMatrixGetColumnUserData(w, cbs->column);
   if (col_nm_addr != NULL) {
      col_nm = *col_nm_addr;
   }    
      
   if (col_nm == field_sort_by || column_nm[cbs->column] == field_sort_by) {
      /* toggling sort order */
      field_sort_direction = !field_sort_direction;     
   } else {
      if (col_nm != -1) {
         field_sort_by = col_nm;
      } else {
         field_sort_by = column_nm[cbs->column];
      }
      field_sort_direction = AR_sort_ascending;
      /* switching from another field to ar number also toggles */
      if (field_sort_by == AR_id) {
         field_sort_direction = !arnum_sort_direction;
      }
   }

   /* recording the last chosen ar number sort direction */
   if (field_sort_by == AR_id) {
      arnum_sort_direction = field_sort_direction;
   }
   updateARList();
    
   DEXIT;
}

/*----------------------------------------------------------------------------*/
void updateARList(void)
{
   lList *rl = NULL;
   lListElem *ar = NULL;
   int row = 0;
   int current_rows = 0, desired_rows = 0; /* used at the end to shrink tabs */
   
#ifdef AR_PENDING
   lList *pl = NULL;
   lCondition *where_run = NULL;
   lEnumeration *what = NULL;
   int pow = 0;
#endif   
         
   DENTER(GUI_LAYER, "updateARList");
   
   /*
   ** has dialog been created
   */
   if (!qmon_ar) {
      DEXIT;
      return;
   }
   
#ifdef AR_PENDING
   what = lWhat("%T(ALL)", AR_Type);
   where_run = lWhere("%T(%I == %u)", AR_Type, AR_state, AR_RUNNING);
#endif
 
   rl = lCopyList("rl", qmonMirrorList(SGE_AR_LIST));

   /*
   ** loop over the ars and the tasks
   */
   XbaeMatrixDisableRedisplay(ar_running_ars);

#ifdef AR_PENDING
   XbaeMatrixDisableRedisplay(ar_pending_ars);
#endif
 
   /*
   ** reset matrices
   */
   XtVaSetValues( ar_running_ars,
                  XmNcells, NULL,
                  NULL);
#ifdef AR_PENDING                  
   XtVaSetValues( ar_pending_ars,
                  XmNcells, NULL,
                  NULL);
   /*
   ** update the ar entries
   */
   lSplit(&rl, &pl, "rl", where_run);
#endif

   /*
   ** sort the ars according to start time
   */
   if (lGetNumberOfElem(rl)>0 ) {
      if (field_sort_direction == AR_sort_ascending && arnum_sort_direction == AR_sort_ascending) {
         lPSortList(rl, "%I+ %I+", field_sort_by, AR_id);
      } else if (field_sort_direction == AR_sort_ascending && arnum_sort_direction == AR_sort_descending) {
         lPSortList(rl, "%I+ %I-", field_sort_by, AR_id);
      } else if (field_sort_direction == AR_sort_descending && arnum_sort_direction == AR_sort_ascending) {
         lPSortList(rl, "%I- %I+", field_sort_by, AR_id);
      } else if (field_sort_direction == AR_sort_descending && arnum_sort_direction == AR_sort_descending) {
         lPSortList(rl, "%I- %I-", field_sort_by, AR_id);
      }
   }

   /*
   ** running ars
   */
   for_each(ar, rl) {
      qmonARToMatrix(ar_running_ars, ar, AR_DISPLAY_MODE_RUNNING, row);
      row++;                                                            
   }            

#ifdef AR_PENDING
   /*
   ** sort the ars according to start time
   */
   if (lGetNumberOfElem(pl)>0 ) {
      if (field_sort_direction == AR_sort_ascending && arnum_sort_direction == AR_sort_ascending) {
         lPSortList(pl, "%I+ %I+", field_sort_by, AR_id);
      } else if (field_sort_direction == AR_sort_ascending && arnum_sort_direction == AR_sort_descending) {
         lPSortList(pl, "%I+ %I-", field_sort_by, AR_id);
      } else if (field_sort_direction == AR_sort_descending && arnum_sort_direction == AR_sort_ascending) {
         lPSortList(pl, "%I- %I+", field_sort_by, AR_id);
      } else if (field_sort_direction == AR_sort_descending && arnum_sort_direction == AR_sort_descending) {
         lPSortList(pl, "%I- %I-", field_sort_by, AR_id);
      }
   }

   /*
   ** pending ars
   */
   for_each(ar, pl) {
      qmonARToMatrix(ar_pending_ars, ar, AR_DISPLAY_MODE_PENDING, pow);
      pow++;                                                            
   }            

   /*
   ** free the where/what
   */
   lFreeWhere(&where_run);
   lFreeWhat(&what);
   lFreeList(&pl);
#endif

   lFreeList(&rl);

   /* shrinking excessive vertical size of tabs from previous runs of updateARList() */  
   current_rows = XbaeMatrixNumRows(ar_running_ars);
   desired_rows = MAX(row, 20);
   if (current_rows > desired_rows) {
     XbaeMatrixDeleteRows(ar_running_ars, desired_rows, current_rows - desired_rows);
   }   

#ifdef AR_PENDING   
   current_rows = XbaeMatrixNumRows(ar_pending_ars);
   desired_rows = MAX(pow, 20);
   if (current_rows > desired_rows) {
     XbaeMatrixDeleteRows(ar_pending_ars, desired_rows, current_rows - desired_rows);
   }
#endif   

   XbaeMatrixEnableRedisplay(ar_running_ars, True);

#ifdef AR_PENDING
   XbaeMatrixEnableRedisplay(ar_pending_ars, True);
#endif 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Boolean qmonDeleteARForMatrix(
Widget w,
Widget matrix,
lList **local 
) {
   int i;
   int rows;
   lList *ardl = NULL;
   lList *alp = NULL;
   char *str = NULL;
   int force;

   DENTER(GUI_LAYER, "qmonDeleteARForMatrix");

/*
   if (!user_list) {
      lAddElemStr(&user_list, ST_name, "*", ST_Type);
   }   
*/   
   force = XmToggleButtonGetState(force_toggle);

   /* 
   ** create delete ar list 
   */
   rows = XbaeMatrixNumRows(matrix);
   for (i=0; i<rows; i++) {
      /* is this row selected */ 
      if (XbaeMatrixIsRowSelected(matrix, i)) {
         str = XbaeMatrixGetCell(matrix, i, 0);
         if (str && *str != '\0') { 
            DPRINTF(("ARId to delete: %s\n", str));
            lAddElemStr(&ardl, ID_str, str, ID_Type);
         }
      }
      if (force != 0) {
         lListElem *id;
         for_each(id, ardl){
            lSetUlong(id, ID_force, force);
         }
      }
   }

   if (ardl) {
      alp = ctx->gdi(ctx, SGE_AR_LIST, SGE_GDI_DEL, &ardl, NULL, NULL, false);
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);

      updateARList();
      XbaeMatrixDeselectAll(matrix);

      lFreeList(&ardl);
   }
#if 0   
   else {
      qmonMessageShow(w, True, "@{There are no ARs selected !}");
      DEXIT;
      return False;
   }
#endif

   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
static void qmonSelectAllARCB(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   
   DENTER(GUI_LAYER, "qmonSelectAllARCB");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   XbaeMatrixSelectAll(current_matrix);

   /* set normal cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonDeleteARCB(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   Boolean status;
   
   DENTER(GUI_LAYER, "qmonDeleteARCB");

   /*
   ** we have to delete the running ars independently
   ** they are not removed from the local list only their
   ** state is set to JDELETED, this is marked with a NULL pointer
   ** instead of a valid local list address
   */

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

#if 0
   status = qmonDeleteARForMatrix(w, ar_running_ars, NULL);
   if (status)
      XbaeMatrixDeselectAll(ar_running_ars);

   status = qmonDeleteARForMatrix(w, ar_pending_ars,
                           qmonMirrorListRef(SGE_AR_LIST));
   if (status)
      XbaeMatrixDeselectAll(ar_pending_ars);
#else   
   status = qmonDeleteARForMatrix(w, current_matrix, NULL);
   if (status)
      XbaeMatrixDeselectAll(current_matrix);

#endif

   updateARListCB(w, NULL, NULL);

   /* set normal cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonARHandleEnterLeave(
Widget w,
XtPointer cld,
XEvent *ev,
Boolean *ctd 
) {
   /*
   int root_x, root_y, pos_x, pos_y;
   Window root, child;
   unsigned int keys_buttons;
   */
   char line[BUFSIZ];
   static int prev_row = -1;
   int row, col;
   String str;
   int ar_nr;
   lListElem *ar;
   
   DENTER(GUI_LAYER, "qmonARHandleEnterLeave");
   
   switch (ev->type) {
      case MotionNotify:
         if (qmonBrowserObjectEnabled(BROWSE_AR)) {
            /* 
            ** XQueryPointer(XtDisplay(w), XtWindow(w), 
            **            &root, &child, &root_x, &root_y,
            **            &pos_x, &pos_y, &keys_buttons);
            */
            if (XbaeMatrixGetEventRowColumn(w, ev, &row, &col)) {
               if ( row != prev_row ) {
                  prev_row = row;
                  /* locate the ar */
                  str = XbaeMatrixGetCell(w, row, 0);
                  if (str && *str != '\0') {
                     ar_nr = atoi(str);
                     ar = ar_list_locate(qmonMirrorList(SGE_AR_LIST), 
                                          (u_long32)ar_nr);
                     if (ar) {
                        dstring ar_info = DSTRING_INIT;
                        sprintf(line, "+++++++++++++++++++++++++++++++++++++++++++\n");  
                        qmonBrowserShow(line);
                        qmonARShowBrowserInfo(&ar_info, ar);      
                        qmonBrowserShow(sge_dstring_get_string(&ar_info));
                        sge_dstring_free(&ar_info);
                     }
                  }
               }
            }
         }
         break;
   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonARShowBrowserInfo(
dstring *info,
lListElem *ar 
) {

   DENTER(GUI_LAYER, "qmonARShowBrowserInfo");

   sge_dstring_sprintf(info, "%-30.30s"sge_u32"\n", "AR Id:  ", lGetUlong(ar, AR_id));
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "AR name:", lGetString(ar, AR_name));
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "AR account:",
                  lGetString(ar, AR_account) ? 
                  lGetString(ar, AR_account): "-NA-");
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "AR owner:", 
                  lGetString(ar, AR_owner));
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "AR checkpoint Object:", 
            lGetString(ar, AR_checkpoint_name) ? 
            lGetString(ar, AR_checkpoint_name) : "-NA-");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonARFolderChange(Widget w, XtPointer cld, XtPointer cad)
{
   XmTabCallbackStruct *cbs = (XmTabCallbackStruct *) cad;

   DENTER(GUI_LAYER, "qmonARFolderChange");
   
   DPRINTF(("%s\n", XtName(cbs->tab_child)));

#ifdef AR_PENDING
   if (!strcmp(XtName(cbs->tab_child), "ar_pending")) {
      current_matrix=ar_pending_ars;
   }
#endif   

   if (!strcmp(XtName(cbs->tab_child), "ar_running")) {
      current_matrix=ar_running_ars;
   }

   updateARList();

   DEXIT;
}

