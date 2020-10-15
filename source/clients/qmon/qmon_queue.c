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
 *   Copyright 2012 Dave Love, University of Liverpool
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <X11/Xos.h>
#include <stdlib.h>
#include <ctype.h>
 
#include <Xm/Xm.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>

#include <Xmt/Xmt.h>
#include <Xmt/Menu.h>
#include <Xmt/Icon.h>
#include <Xmt/Hash.h>
#include <Xmt/Create.h>
#include <Xmt/Pixmap.h>
#include <Xmt/Dialogs.h>

#include "qmon_rmon.h"
#include "qmon_queue.h"
#include "qmon_qaction.h"
#include "qmon_menus.h"
#include "qmon_comm.h"
#include "qmon_timer.h"
#include "qmon_globals.h"
#include "qmon_init.h"
#include "qmon_ticket.h"
#include "qmon_util.h"
#include "qmon_message.h"
#include "qmon_browser.h"
#include "qmon_qcustom.h"
#include "Matrix.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_host.h"

#include "sched/sge_sched.h"
#include "sched/sge_complex_schedd.h"
#include "sched/load_correction.h"

#include "sge.h"

/*-------------------------------------------------------------------------*/
static void qmonBuildQBG(Widget parent, XtPointer cld, XtPointer cad);
static void qmonQueuePopdown(Widget w, XtPointer cld, XtPointer cad);
static void qmonQueueStartUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonQueueStopUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonQueueHash(lList *qlp, lList *hl);
static void qmonQueueSetPos(lList *qlp);
static void qmonQueueRemove(tQueueIcon *qI);
static char *qmonQueueGetArch(const char *qhostname);
static char *qmonQueueGetSymbol(char *arch);
static String qmonQueueShowBrowserInfo(lListElem *qep);
static void qmonDrawQueueButton(Widget w, XtPointer cld, XtPointer cad);
static void qmonQueueModify(Widget w, XtPointer cld, XtPointer cad);
static void qmonQueueDeleteQuick(Widget w, XtPointer cld, XtPointer cad);
static void qmonQueueChangeState(Widget w, XtPointer cld, XtPointer cad);
static void qmonChangeBackground(Widget w, int selected);
static void HandleButtonPress(Widget w, XtPointer cld, XEvent *event, Boolean *continue_to_dispatch);
static void HandleEnter(Widget w, XtPointer cld, XEvent *event, Boolean *continue_to_dispatch);
static void qmonCreateQueueControl(Widget w);
static void qmonQueueSetLoad(Widget matrix, lListElem *qep);
static void qmonQueueShowLoadEvent(Widget w, XtPointer cld, XEvent *event);
/* static void showQueueHashTable(XmtHashTable table, XtPointer key, XtPointer *data); */

/*-------------------------------------------------------------------------*/
static Widget qmon_queue = 0;
static Widget queue_da = 0;
static Widget queue_customize = 0;

static XmtHashTable QueueHashTable = NULL;
static tQueueButton* QBG[QUEUE_MAX_VERT];

static XmtMenuItem queue_popup_items[] = {
   {XmtMenuItemLabel, "@{@fBQUEUE ACTIONS}"},
   {XmtMenuItemSeparator},
   {XmtMenuItemPushButton, "@{Add}", 'A', "Meta<Key>A", "Meta+A",
         qmonQCPopup, NULL },
   {XmtMenuItemPushButton, "@{Modify}", 'M', "Meta<Key>M", "Meta+M",
         qmonQueueModify, NULL},
/*    {XmtMenuItemPushButton, "DeleteDialog", 'l', "Meta<Key>L", "Meta+L", */
/*          qmonQCPopup, (XtPointer)QC_DELETE }, */
   {XmtMenuItemPushButton, "@{Delete}", 'D', "Meta<Key>D", "Meta+D",
         qmonQueueDeleteQuick, NULL},
   {XmtMenuItemPushButton, "@{Suspend}", 'S', "Meta<Key>S", "Meta+S",
         qmonQueueChangeState, (XtPointer)QI_DO_SUSPEND},
   {XmtMenuItemPushButton, "@{Resume}", 'R', "Meta<Key>R", "Meta+R",
         qmonQueueChangeState, (XtPointer)QI_DO_UNSUSPEND},
   {XmtMenuItemPushButton, "@{Disable}", 'i', "Meta<Key>I", "Meta+I",
         qmonQueueChangeState, (XtPointer)QI_DO_DISABLE},
   {XmtMenuItemPushButton, "@{Enable}", 'E', "Meta<Key>E", "Meta+E",
         qmonQueueChangeState, (XtPointer)QI_DO_ENABLE}
};


#define WIDTH  "%-30.30s"

/*-------------------------------------------------------------------------*/
/*    P U B L I C    F U N C T I O N S                                     */
/*-------------------------------------------------------------------------*/
void qmonQueuePopup(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonQueuePopup");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   qmonMirrorMultiAnswer(CQUEUE_T | EXECHOST_T | CENTRY_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set busy cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }

   if (!qmon_queue) {

      qmonCreateQueueControl(AppShell);

      /*
      ** create queue customize dialog
      */
/*       qmonCreateQCU(qmon_queue, NULL); */

      /* 
      ** set the close button callback 
      ** set the icon and icon name
      */
      XmtCreatePixmapIcon(qmon_queue, qmonGetIcon("toolbar_queue"), None); 
      XtVaSetValues(qmon_queue, XtNiconName, "qmon:Queue Control", NULL);
      XmtAddDeleteCallback(qmon_queue, XmDO_NOTHING, qmonQueuePopdown,  NULL);
      XtAddEventHandler(qmon_queue, StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
      XtAddEventHandler(qmon_queue, StructureNotifyMask, False, 
                        SetMaxShellSize, (XtPointer) SHELL_WIDTH);
      
   }

   xmui_manage(qmon_queue);
/*    ForceUpdate(qmon_queue); */

   updateQueueList();

#if 0
   /*
   ** workaround for display problem of DrawingArea under some WMs
   */
   XtUnmapWidget(queue_da);
   XtMapWidget(queue_da);
#endif   

   /* set busy cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updateQueueList(void)
{
   lList *qlp = NULL;
   lList *hl = NULL;
   lList *rl = NULL;
   lList *cl = NULL;
   lEnumeration *whatall = NULL;
   lCondition *where = NULL;
   static Boolean filter_on = False;
   
   DENTER(GUI_LAYER, "updateQueueList");

   cl = qmonMirrorList(SGE_CE_LIST);
   /*
   ** copy of host list
   */
   hl = lCopyList("HL", qmonMirrorList(SGE_EH_LIST));

   /* 
   **
   ** select a subset of the whole queue list (->where) 
   ** and get the list sorted 
   **
   */
#ifdef FIXME   
   where = lWhere("%T(%I!=%s)", QU_Type, QU_qname, QU_TEMPLATE);
   whatall = lWhat("%T(ALL)", QU_Type);
#else   
   whatall = lWhat("%T(ALL)", CQ_Type);
#endif  
   qlp = lSelect("SQL", qmonMirrorList(SGE_CQ_LIST), where, whatall); 
   lFreeWhere(&where);
   lFreeWhat(&whatall);

#ifdef FIXME
   /*
   ** additional filtering
   */
   rl = qmonQFilterRequest();
   if (rl) {
      if (!filter_on) {
         setButtonLabel(queue_customize, "@{Customize +}");
         filter_on = True;
      }
      match_queue(&qlp, rl, cl, hl);
   }  
   else {
      if (filter_on) {
         setButtonLabel(queue_customize, "@{Customize}");
         filter_on = False;
      }
   }
   
   /*
   ** sort the queues according to sequence number and alphabetically
   */
   lPSortList(qlp, "%I+ %I+ %I+", QU_seq_no, QU_qhostname, QU_qname);
#endif
   /*
   ** save the queue in hash table
   */
   qmonQueueHash(qlp, hl);

   qmonQueueSetPos(qlp);

   /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
   ** qlp must not be freed it is referenced in qmonHashQueue
   ** and freed there
   */
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updateQueueListCB(Widget w, XtPointer cld, XtPointer cad)
{

   lList *alp = NULL;

   if (qmon_queue) {
      qmonMirrorMultiAnswer(CQUEUE_T | EXECHOST_T | CENTRY_T, &alp);
      if (alp) {
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);
         return;
      }
      updateQueueList();
   }
}

/*-------------------------------------------------------------------------*/
/*    P R I V A T E   F U N C T I O N S                                    */
/*-------------------------------------------------------------------------*/
static void qmonQueuePopdown(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonQueuePopdown");

   qmonQCPopdown(w, NULL, NULL);
   xmui_unmanage(qmon_queue);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQueueStartUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonQueueStartUpdate");
  
   /* 
    * register the update procedure
    * start queue timer for queue info and exechost timer for infos
    * of host that the queue is attached to
    */
   qmonTimerAddUpdateProc(CQUEUE_T, "updateQueueList", updateQueueList);
   qmonStartTimer(CQUEUE_T | EXECHOST_T | CENTRY_T);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonQueueStopUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonQueueStopUpdate");
  
   /* 
    * remove the update procedure
    * stop queue timer for queue info and exechost timer for infos
    * of host that the queue is attached to
    */
   qmonStopTimer(CQUEUE_T | EXECHOST_T | CENTRY_T);
   qmonTimerRmUpdateProc(CQUEUE_T, "updateQueueList");
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCreateQueueControl(
Widget parent 
) {
   Widget  queue_add, queue_modify, queue_update, 
           queue_delete, queue_done, queue_suspend, queue_unsuspend,
           queue_enable, queue_disable, queue_reschedule, 
           queue_error, queue_tickets, queue_main_link;
   
   DENTER(GUI_LAYER, "qmonCreateQueueControl");

   qmon_queue = XmtBuildQueryToplevel( parent, "qmon_queue",
                                     "queue_da", &queue_da,
                                     "queue_add", &queue_add,
                                     "queue_modify", &queue_modify,
                                     "queue_customize", &queue_customize,
                                     "queue_done", &queue_done,
                                     "queue_update", &queue_update,
                                     "queue_delete", &queue_delete,
                                     "queue_suspend", &queue_suspend,
                                     "queue_unsuspend", &queue_unsuspend,
                                     "queue_enable", &queue_enable,
                                     "queue_disable", &queue_disable,
                                     "queue_reschedule", &queue_reschedule,
                                     "queue_error", &queue_error,
                                     "queue_tickets", &queue_tickets,
                                     "queue_main_link", &queue_main_link,
                                     NULL);


   XtAddCallback(queue_tickets, XmNactivateCallback,
                  qmonPopupTicketOverview, NULL);

   XtAddCallback(queue_add, XmNactivateCallback, 
                     qmonQCPopup, NULL);
   XtAddCallback(queue_modify, XmNactivateCallback, 
                     qmonQueueModify, NULL);
   XtAddCallback(queue_customize, XmNactivateCallback, 
                     qmonPopupQCU, NULL); 
   XtAddCallback(queue_done, XmNactivateCallback, 
                     qmonQueuePopdown, NULL);
   XtAddCallback(queue_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(queue_update, XmNactivateCallback, 
                     updateQueueListCB, NULL);
   XtAddCallback(queue_delete, XmNactivateCallback, 
                     qmonQueueDeleteQuick, NULL);
   XtAddCallback(queue_suspend, XmNactivateCallback, 
                     qmonQueueChangeState, (XtPointer)QI_DO_SUSPEND);
   XtAddCallback(queue_unsuspend, XmNactivateCallback, 
                     qmonQueueChangeState, (XtPointer)QI_DO_UNSUSPEND);
   XtAddCallback(queue_disable, XmNactivateCallback, 
                     qmonQueueChangeState, (XtPointer)QI_DO_DISABLE);
   XtAddCallback(queue_enable, XmNactivateCallback, 
                     qmonQueueChangeState, (XtPointer)QI_DO_ENABLE);
   XtAddCallback(queue_reschedule, XmNactivateCallback, 
                     qmonQueueChangeState, (XtPointer)QI_DO_RESCHEDULE);
   XtAddCallback(queue_error, XmNactivateCallback, 
                     qmonQueueChangeState, (XtPointer)QI_DO_CLEARERROR);
/*    XtAddCallback(queue_load, XmNvalueChangedCallback,  */
/*                      qmonQueueToggleLoad, NULL); */

#ifdef FIXME
   /* start the needed timers and the corresponding update routines */
   XtAddCallback(qmon_queue, XmNpopupCallback, 
                     qmonQueueStartUpdate, NULL);
   XtAddCallback(qmon_queue, XmNpopdownCallback,
                     qmonQueueStopUpdate, NULL);
#endif

   /* register event handler for queue popup */
   qmonCreatePopup(queue_da, "QueuePopup", queue_popup_items, 
                           XtNumber(queue_popup_items));
   qmonBuildQBG(queue_da, NULL, NULL);
   XtManageChild(queue_da);

   DEXIT;
}


   
/*-------------------------------------------------------------------------*
 * 
 * In order to bypass annoying flickering effects, we manage a whole
 * bunch of PushB's, but they are all unmapped. By looping through
 * the actual list we give the actual queue a grid position and
 * map the corresponding PushB, changing its labelString or labelPixmap
 * We register an enter/leave Event Handler to have the ability to
 * show some additional useful information about the queue (->Browser)
 * The handle to the information is a Quark build from qhostname and qname
 * therefore we need the same qname/qhostname in distinct accesses to the
 * hash table through the quark mechanism. (->tolower(qhostname),if there
 * are several versions of qhostname (FRODO.adomain, frodo.adomain))
 * The QueueButtonGrid (QBG) sizes are settable through resources at startup
 * of the application.
 *
 *-------------------------------------------------------------------------*/
static void qmonBuildQBG(
Widget parent,
XtPointer cld,
XtPointer cad 
) {
   int i, j;
   int x, y;
   int x0 = (QUEUE_GRID_WIDTH - QUEUE_BUTTON_WIDTH) / 2;
   int y0 = (QUEUE_GRID_HEIGHT - QUEUE_BUTTON_HEIGHT) / 2;

   DENTER(GUI_LAYER, "qmonBuildQBG");

   for (i=0; i<QUEUE_MAX_VERT; i++) {
      QBG[i] = (tQueueButton*) XtMalloc(sizeof(tQueueButton) *
                                                QUEUE_MAX_HORIZ);
   }         
   
   for (i=0; i<QUEUE_MAX_VERT; i++) {
      for (j=0; j< QUEUE_MAX_HORIZ; j++) {
         x = QUEUE_GRID_XOFFSET + j * QUEUE_GRID_WIDTH;
         y = QUEUE_GRID_YOFFSET + i * QUEUE_GRID_HEIGHT;

         QBG[i][j].qI = NULL;        
         QBG[i][j].bgid = XtVaCreateWidget(
                                    "QBG", 
                                    xmDrawingAreaWidgetClass,
                                    parent,
                                    XmNborderWidth, 0,
                                    XmNmarginWidth, 0,
                                    XmNmarginHeight, 0,
                                    XmNx, x,
                                    XmNy, y,
                                    XmNheight, QUEUE_GRID_HEIGHT,
                                    XmNwidth, QUEUE_GRID_WIDTH,
                                    XmNresizePolicy, XmRESIZE_NONE,
                                    XmNshadowThickness, 0,
                                    XmNhighlightThickness, 0,
                                    NULL);

         XtAddEventHandler(QBG[i][j].bgid, 
                        ButtonPressMask, 
                        False, HandleButtonPress, 
                        (XtPointer)&QBG[i][j]);

         QBG[i][j].id = XtVaCreateManagedWidget("QBG", 
                                                xmDrawnButtonWidgetClass,
                                                QBG[i][j].bgid,
                                                XmNmappedWhenManaged, False,
                                                XmNx, x0,
                                                XmNy, y0,
                                                XmNheight, QUEUE_BUTTON_HEIGHT,
                                                XmNwidth, QUEUE_BUTTON_WIDTH,
                                                XmNrecomputeSize, False,
                                                NULL);
         XtAddCallback( QBG[i][j].id, 
                     XmNexposeCallback, 
                     qmonDrawQueueButton, 
                     (XtPointer)&QBG[i][j]);

         XtAddEventHandler(QBG[i][j].id, 
                        ButtonPressMask, 
                        False, HandleButtonPress, 
                        (XtPointer)&QBG[i][j]);

         XtAddEventHandler(QBG[i][j].id,
                        EnterWindowMask | LeaveWindowMask,
                        False, HandleEnter,
                        (XtPointer)&QBG[i][j]);

/*          XtRealizeWidget(QBG[i][j].bgid); */
         XtManageChild(QBG[i][j].bgid);
         
/*
         XtAddCallback(QBG[i][j].id, XmNarmCallback,
                           qmonQueueShowLoadCB, (XtPointer)&QBG[i][j]); 
         XtAddCallback(QBG[i][j].id, XmNdisarmCallback,
                           qmonQueueShowLoadCB, (XtPointer)&QBG[i][j]); 
*/    
      }
   }

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonQueueHash(
lList *new_ql,
lList *new_hl 
) {

   static lList *prev_ql = NULL;
   static lList *prev_hl = NULL;
   lListElem *qep;
   tQueueIcon *queueIcon;
   long id;
   const char *qname, *qhostname;
   Boolean already_hashed;
   
   DENTER(GUI_LAYER, "qmonQueueHash");

   /* Create QueueHashTable if necessary */
   if (!QueueHashTable)
      QueueHashTable = XmtHashTableCreate(5);

   for_each(qep, new_ql) {

      qname = lGetString(qep, CQ_name);
      /* quarkify  CQ_name */
      id = (long) XrmStringToQuark(qname);
      
      /*
       * if the queue has already been hashed XmtHashTableLookup()
       * returns True 
       * 
       */
      already_hashed = XmtHashTableLookup(QueueHashTable,
                                          (XtPointer)id,
                                          (XtPointer*) &queueIcon);
      if (already_hashed) {
         queueIcon->qp = qep;
#ifdef FIXME         
         if (!queueIcon->arch)
            queueIcon->arch = qmonQueueGetArch(qhostname);
#endif            
      }
      else {
         /* create a new tQueueIcon structure */
         queueIcon = (tQueueIcon *)XtMalloc(sizeof(tQueueIcon));
         /* initialize */
         queueIcon->quark = id;
         queueIcon->grid_x = 0;
         queueIcon->grid_y = 0;
         queueIcon->selected = False;
         queueIcon->deleted = False;
         queueIcon->pixmap = 0;
         queueIcon->arch = "solaris"; /* qmonQueueGetArch(qhostname); */
         queueIcon->qp = qep; 
         XmtHashTableStore(QueueHashTable, 
                           (XtPointer) id, 
                           (XtPointer) queueIcon);
      }
      
      /* remove element from previous ql */
      lDelElemStr(&prev_ql, CQ_name, qname);
   }

   /* 
   ** remove the no longer used Hash entries 
   ** free the tQueueIcon structs
   */
   for_each(qep, prev_ql) {
      qname = lGetString(qep, CQ_name);
      id = (long) XrmStringToQuark(qname);
      if (XmtHashTableLookup( QueueHashTable, (XtPointer) id,
                                 (XtPointer *)&queueIcon)) {
         qmonQueueRemove(queueIcon);
      }
   }

   /*
   ** free the previously referenced queue elements
   */
   lFreeList(&prev_ql);
   lFreeList(&prev_hl);

   /* now the new_ql becomes the prev_ql */
   prev_ql = new_ql;
   prev_hl = new_hl;
   
/*    XmtHashTableForEach(QueueHashTable, showQueueHashTable); */
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQueueRemove(
tQueueIcon *qI 
) {
   DENTER(GUI_LAYER, "qmonQueueRemove");

   XmtHashTableDelete(QueueHashTable, (XtPointer)qI->quark);
   
   /* 
   ** free architecture entry 
   */
#ifdef FIXME   
   if (qI->arch)
      XtFree((char*) qI->arch);
#endif   
   qI->arch = NULL;

   /*
   ** release reference to queue element
   ** queue element is freed in qmonQueueHash
   */
   qI->qp = NULL;

   /*
   ** free tQueueIcon struct
   */
   XtFree((char*)qI);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonQueueSetPos(
lList *qlp 
) {
   lListElem *ep = NULL;
   int grid_x = 0;
   int grid_y = 0;
   long q;
   const char *qname = NULL;
   tQueueIcon *qI = NULL;
   int i, j;
   int max_count = 0;
   
   DENTER(GUI_LAYER, "qmonQueueSetPos");

   /*
    * every element in the list is attached to a special grid position
    */

   /* 
   ** delete previously attached queues  
   ** we release here only the references, the corresponding data
   ** are freed in qmonQueueHash
   */
   for (i=0; i<QUEUE_MAX_VERT; i++)
      for (j=0; j<QUEUE_MAX_HORIZ; j++) 
         QBG[i][j].qI = NULL; 

   for_each(ep, qlp) {

      /*
      ** FIXME: workaround for exceeding the max number of queues
      **        exceeding queues are not displayed
      */
      max_count++;
      if (max_count >= QUEUE_MAX_HORIZ * QUEUE_MAX_VERT)
            break; 

      /* lookup queue struct qI */
      qname = lGetString(ep, CQ_name);
      q = (long) XrmStringToQuark(qname);
/* printf("----> q = %ld\n", q); */
/*    XmtHashTableForEach(QueueHashTable, showQueueHashTable); */
/* printf("---->\n"); */

      if (!XmtHashTableLookup(QueueHashTable, (XtPointer) q, (XtPointer*) &qI))
         fprintf(stderr, "++++++++++++Hash Table Failure++++++++++++++++\n");
      
      /* the algorithm to attach the q to a special pos */
      if (grid_x == QUEUE_MAX_HORIZ) {
         grid_x = 0;
         grid_y++;
      }

      if (qI) {
         qI->grid_x = grid_x++;
         qI->grid_y = grid_y;
         /* map/unmap Buttons if qI is set/NULL; reset selection state */
         QBG[qI->grid_y][qI->grid_x].qI = qI; 
      }   
   }

   /* loop through the button list and map/unmap */
   for (i=0; i<QUEUE_MAX_VERT; i++) {
      for (j=0; j<QUEUE_MAX_HORIZ; j++) {
         if (QBG[i][j].qI)
            XtSetMappedWhenManaged(QBG[i][j].id, True);
         else
            XtSetMappedWhenManaged(QBG[i][j].id, False);

         if (QBG[i][j].qI && QBG[i][j].qI->selected 
            && XtWindow(QBG[i][j].bgid))
            qmonChangeBackground(QBG[i][j].bgid, True);
         else
            qmonChangeBackground(QBG[i][j].bgid, False);
         /* we need qI->deleted in qmonDrawQueueButton */
         if (QBG[i][j].qI && XtWindow(QBG[i][j].id))
            qmonDrawQueueButton(QBG[i][j].id, 
                               (XtPointer) &QBG[i][j], 
                               (XtPointer) NULL);

         if (QBG[i][j].qI && QBG[i][j].qI->deleted) 
            XtSetSensitive(QBG[i][j].id, False);
         else
            XtSetSensitive(QBG[i][j].id, True);
      } 
   }
   XmUpdateDisplay(AppShell);
   
/*    XmtHashTableForEach(QueueHashTable, showQueueHashTable); */
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void HandleEnter(
Widget w,
XtPointer cld,
XEvent *ev,
Boolean *ctd 
) {
   tQueueButton *qB = (tQueueButton*)cld;


   char info[BUFSIZ];
   String browser_info;
   
   DENTER(GUI_LAYER, "HandleEnter");

   if (ev->type == EnterNotify) {
      if (qB->qI) { 
         if (qmonBrowserObjectEnabled(BROWSE_QUEUE)) {
            sprintf(info, "+++++++++++++++++++++++++++++++++++++++++++\n");  
            qmonBrowserShow(info);
            {
               lListElem *qp;
               for_each(qp, lGetList(qB->qI->qp, CQ_qinstances)) {
                  browser_info = qmonQueueShowBrowserInfo(qp); 
               }
            }   
            qmonBrowserShow(browser_info);
            sprintf(info, "+++++++++++++++++++++++++++++++++++++++++++\n");  
            qmonBrowserShow(info);
         }
         /*
         if (show_load) {
            qmonQueueShowLoadEvent(w, cld, ev);
         }
         */
      }
   }
   
#if 0   
   if (ev->type == LeaveNotify) {
      qmonQueueDeleteLoadEvent(w, cld, ev);
   }   
#endif   
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQueueShowLoadEvent(
Widget w,
XtPointer cld,
XEvent *event 
) {
   tQueueButton *qb = (tQueueButton*) cld;
   Widget parent = w;
   static Widget lmon=0;
   static Widget matrix;
   
   DENTER(GUI_LAYER, "qmonQueueShowLoadEvent");
   
   if (!qb->qI) {
      DEXIT;
      return;
   }

   parent = XmtNameToWidget(w, "~*queue_sw");

   if (!lmon)
      lmon = XmtBuildQueryDialog(parent, "lmon_shell", 
                                       NULL, 0,
                                       "lmon_matrix", &matrix,
                                       NULL);

#if 0
   qmonQueueSetLoad(matrix, qb->qI->qp);
   XtManageChild(lmon);
#else
   updateQueueListCB(w, NULL, NULL);
   XtManageChild(lmon);
   qmonQueueSetLoad(matrix, qb->qI->qp);
#endif

   XtAddEventHandler(XtParent(lmon), StructureNotifyMask, False, 
                        SetMinShellSize, (XtPointer) SHELL_WIDTH);
   XtAddEventHandler(XtParent(lmon), StructureNotifyMask, False, 
                        SetMaxShellSize, (XtPointer) SHELL_WIDTH);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQueueSetLoad(
Widget matrix,
lListElem *qep 
) {
   static char info[10000];
   lListElem *ep;
   lList *ncl = NULL;
   lList *ehl = NULL;
   lList *cl = NULL; 
   StringConst new_row[3];
   int rows;
   float fval;
   XmString xstr;

   DENTER(GUI_LAYER, "qmonQueueSetLoad");

   ehl = qmonMirrorList(SGE_EH_LIST);
   cl = qmonMirrorList(SGE_CE_LIST);

   correct_capacities(ehl, cl);
   queue_complexes2scheduler(&ncl, qep, ehl, cl);

   sprintf(info, "%s %s", XmtLocalize(matrix, "Attributes for queue", "Attributes for queue"), lGetString(qep, QU_qname));

   xstr = XmtCreateXmString(info);
   XtVaSetValues(XtParent(matrix), XmNdialogTitle, xstr, NULL);
   XmStringFree(xstr);

   rows = XbaeMatrixNumRows(matrix);
   XbaeMatrixDeleteRows(matrix, 0, rows);

   rows = 0;
   for_each_rev (ep, ncl) {
      int n;
      u_long32 type;
      char unit;
      StringConst name;
      StringConst slot_limit;
      StringConst job_limit;
      if (!(name = lGetString(ep, CE_name))) 
         continue;
      /* don't view value entry from complex */
      slot_limit = (lGetUlong(ep, CE_dominant)&DOMINANT_TYPE_VALUE)?
                        NULL:lGetString(ep, CE_stringval);
      type = lGetUlong(ep, CE_valtype);
      if (slot_limit && (type == TYPE_MEM || type == TYPE_DOUBLE) &&
         (n=sscanf(slot_limit, "%f%c", &fval, &unit))>=1) {
         sprintf(info, "%8.3f%c", fval, (n>1) ? unit : '\0');
         lSetString(ep, CE_stringval, info);
         slot_limit = lGetString(ep, CE_stringval);
      }
      if (slot_limit)
         while (*slot_limit && isspace(*slot_limit))
            slot_limit++;

      job_limit = lGetString(ep, CE_pj_stringval);
      type = lGetUlong(ep, CE_valtype);
      if (job_limit && (type == TYPE_MEM || type == TYPE_DOUBLE) &&
         (n = sscanf(job_limit, "%f%c", &fval, &unit))>=1) {
         sprintf(info, "%8.3f%c", fval, (n>1) ? unit : '\0');
         lSetString(ep, CE_pj_stringval, info);
         job_limit = lGetString(ep, CE_pj_stringval);
      }

      if (job_limit)
         while (*job_limit && isspace(*job_limit))
            job_limit++;

      new_row[0] = name; 
      new_row[1] = slot_limit ? slot_limit : "";
      new_row[2] = job_limit ? job_limit : ""; 
      /* FIX_CONST_GUI */
      XbaeMatrixAddRows(matrix, 0, (String*) new_row, NULL, NULL, 1); 

      rows++;
   }

   lFreeList(&ncl);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static String qmonQueueShowBrowserInfo(
lListElem *qep 
) {

   static char info[60000];
   lListElem *ep;
   int qtype;
   const char *str, *str2;

   DENTER(GUI_LAYER, "qmonQueueShowBrowserInfo");

   sprintf(info, WIDTH"%s\n", "\n","Queue:", lGetString(qep, QU_full_name));

   qtype = lGetUlong(qep, QU_qtype);

   {
      dstring type_buffer = DSTRING_INIT;

      qinstance_print_qtype_to_dstring(qep, &type_buffer, false);
      snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
               "Type:", sge_dstring_get_string(&type_buffer));
      sge_dstring_free(&type_buffer);
   }
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%d\n",
            "Sequence Nr:", (int)lGetUlong(qep, QU_seq_no));

   str = lGetString(qep, QU_tmpdir);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "tmpdir:", str ? str : "");
   str = lGetString(qep, QU_shell);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Shell:", str ? str : "");
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%d\n",
            "Job Slots:", (int)lGetUlong(qep, QU_job_slots));
   snprintf(info + strlen(info), sizoef(info) - strlen(info), WIDTH"%d\n",
            "Job Slots Used:", qinstance_slots_used(qep));
   str = lGetString(qep, QU_priority);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Priority:", str?str:"");
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"", "Load Thresholds:");
   for_each(ep, lGetList(qep, QU_load_thresholds)) {
      str = lGetString(ep, CE_name);
      str2 = lGetString(ep, CE_stringval);
      snprintf(info + strlen(info), sizeof(info) - strlen(info), "%s = %s ", str?str:"",
               str2?str2:"");
   }
   sge_strlcat(info, "\n", sizeof(info));

   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Rerun Job:", lGetBool(qep, QU_rerun) ? "True" : "False");

   str = lGetString(qep, QU_notify);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Notify Job Interval:",  str ? str : "");

   str = lGetString(qep, QU_processors);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Processors:", str ? str : "");

   str = lGetString(qep, QU_s_rt);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Soft Real Time:", str ? str : "");
   str = lGetString(qep, QU_h_rt);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Hard Real Time:", str ? str : "");
   str = lGetString(qep, QU_s_cpu);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Soft Cpu:", str ? str : "");
   str = lGetString(qep, QU_h_cpu);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Hard Cpu:", str ? str : "");
   str = lGetString(qep, QU_s_fsize);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Soft File Size:", str ? str : "");
   str = lGetString(qep, QU_h_fsize);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Hard File Size:", str ? str : "");
   str = lGetString(qep, QU_s_data);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Soft Data Size:", str ? str : "");
   str = lGetString(qep, QU_h_data);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Hard Data Size:", str ? str : "");
   str = lGetString(qep, QU_s_stack);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Soft Stack Size:", str ? str : "");
   str = lGetString(qep, QU_h_stack);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Hard Stack Size:", str ? str : "");
   str = lGetString(qep, QU_s_core);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Soft Core Size:", str ? str : "");
   str = lGetString(qep, QU_h_core);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Hard Core Size:", str ? str : "");
   str = lGetString(qep, QU_s_rss);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Soft Resident Set Size:", str ? str : "");
   str = lGetString(qep, QU_h_rss);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Hard Resident Set Size:", str ? str : "");

   str = lGetString(qep, QU_min_cpu_interval);
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Min Cpu Interval:", str ? str : "");

   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"", "Access List:");
   for_each(ep, lGetList(qep, QU_acl)) {
      snprintf(info + strlen(info), sizeof(info) - strlen(info), "%s ",
               lGetString(ep, US_name));
   }
   sge_strlcat(info, "\n", sizeof(info));
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"", "No Access List:");
   for_each(ep, lGetList(qep, QU_xacl)) {
      sge_strlcat(info, lGetString(ep, US_name), sizeof(info));
   }
   sge_strlcat(info, "\n", sizeof(info));

   DPRINTF(("info is %d long\n", strlen(info)));
   
   DEXIT;
   return info;
}

/*-------------------------------------------------------------------------*/
static void HandleButtonPress(
Widget w,
XtPointer cld,
XEvent *ev,
Boolean *ctd 
) {
   tQueueButton *qB = (tQueueButton*) cld;
   
   DENTER(GUI_LAYER, "HandleButtonPress");
   
   switch (ev->type) {
      case ButtonPress:
         switch ( ev->xbutton.button ) { 
            case Button1:
               if (ev->xbutton.state & ShiftMask) {
                  qmonQueueShowLoadEvent(w, cld, ev);
               }
               else if (ev->xbutton.state & ControlMask) {
                  char hostname[SGE_PATH_MAX];

                  strcpy(hostname, "global");
                  if (qB && qB->qI && qB->qI->qp) {
                     sge_strlcpy(hostname, lGetHost(qB->qI->qp, QU_qhostname),
                                 SGE_PATH_MAX);
                     strtok(hostname, ".");
                  }
                  qmonBrowserMessages(w, (XtPointer)hostname, NULL);
               }
               else {
                  if (qB->qI) {
                     qB->qI->selected = !(qB->qI->selected);
                     qmonChangeBackground(qB->bgid, qB->qI->selected ); 
                  }
               }
               
               break;
         }
         break;
   }

   /*
   ** call no other event handler
   */
   ctd = False;

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonChangeBackground(
Widget w,
int selected 
) {
   static int first_time = 1;
   static Pixel background;
   
   DENTER(BASIS_LAYER, "qmonChangeBackground");
   
   if (first_time) {
      first_time = 0;
      XtVaGetValues( w, 
                     XmNbackground, &background, 
                     NULL);
   }

   XtVaSetValues( w, 
                  XmNbackground, selected ? QueueSelectedPixel : background, 
                  NULL);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonDrawQueueButton(Widget w, XtPointer cld, XtPointer cad)
{
   static XmFontList defaultFontList = NULL;
   tQueueButton *qB = (tQueueButton*) cld;
   Dimension ht, st, bw, width, height, sw, sh;
   int sbh, sbw;
   Window root;
   unsigned int border_width, depth;
   Position x, y;
   XmString str = NULL;
   XRectangle rect;
   char buf[BUFSIZ];
   char hostname[128];
   const char *qname = NULL, *qhostname = NULL;
   unsigned long job_slots = 0, job_slots_used = 0;
   unsigned long alarm_set = 0, suspend_threshold_alarm = 0;
   double load = 0.0;
   u_long32 is_load_available = 0;
   u_long32 used = 0;
   u_long32 total = 0;
   u_long32 suspend_manual = 0;
   u_long32 suspend_threshold = 0;
   u_long32 suspend_on_subordinate = 0;
   u_long32 suspend_calendar = 0;
   u_long32 unknown = 0;
   u_long32 load_alarm = 0;
   u_long32 disabled_manual = 0;
   u_long32 disabled_calendar = 0;
   u_long32 ambiguous = 0;
   u_long32 orphaned = 0;
   u_long32 error = 0;
   u_long32 available = 0;
   u_long32 temp_disabled = 0;
   u_long32 manual_intervention = 0;
   int i; 
   GC draw_gc = qb_gc;
   lList *ehl = NULL;
   lList *cl = NULL;
   lListElem *q = NULL;
   
   DENTER(GUI_LAYER, "qmonDrawQueueButton");

   if (!defaultFontList) {
      XmFontList theFontList = NULL;
      XmFontContext fc = NULL;
      XtVaGetValues(w, XmNfontList, &theFontList, NULL);
      if (XmFontListInitFontContext(&fc, theFontList)) {
         XmFontListEntry entry;
         while ((entry = XmFontListNextEntry(fc))) {
            char *tag = XmFontListEntryGetTag(entry);
            DPRINTF(("tag = %s\n", tag ? tag : ""));
            if (!strcmp(tag, "QUEUEICON")){
               defaultFontList = XmFontListAppendEntry(NULL, entry); 
               break;
            }   
         }     
         XmFontListFreeFontContext(fc);
      }
      if (!defaultFontList) {
         XmFontListEntry entry = XmFontListEntryLoad(XtDisplay(w),
                                    "-*-helvetica-medium-r-*-*-*-60-*-*-*-*-*-*",
                                    XmFONT_IS_FONT,
                                    XmFONTLIST_DEFAULT_TAG);
         defaultFontList = XmFontListAppendEntry(NULL, entry);
         XmFontListEntryFree(&entry);
      }   
   }

   /* Button active ? */ 
   if (qB->qI) {
      /* 
      ** get info from queue 
      */
      if (qB->qI->qp) {
         ehl = qmonMirrorList(SGE_EH_LIST);
         cl = qmonMirrorList(SGE_CE_LIST);
         q = qB->qI->qp;

         qname     = lGetString(q, CQ_name);
#ifdef FIXME         
         cqueue_calculate_summary(q, ehl, cl, 
                                  &load, &is_load_available, &used, &total,
                                  &suspend_manual, &suspend_threshold,
                                  &suspend_on_subordinate, &suspend_calendar,
                                  &unknown, &load_alarm, &disabled_manual,
                                  &disabled_calendar, &ambiguous, &orphaned,
                                  &error, &available, &temp_disabled,
                                  &manual_intervention);

         DPRINTF(("<<Queue: %s/%f/%d/%d>>\n", qname, load, used, total);
#endif         
      } 
      else {
         DPRINTF(("Queue Button Grid corrupted\n"));
         DEXIT;
         return;   
      }
      
      /*
      ** get the right pixmap and get the size
      */
      qB->qI->pixmap = qmonGetIcon(qmonQueueGetSymbol(qB->qI->arch));
      XGetGeometry( XtDisplay(qB->id), 
                              qB->qI->pixmap,
                              &root, &(qB->qI->x), &(qB->qI->y), 
                              &(qB->qI->icon_width), &(qB->qI->icon_height), 
                              &border_width, &depth );
      /*
      ** get the dimensions of the button
      */
      XtVaGetValues( w,
                     XmNwidth, &width,
                     XmNheight, &height,
                     XmNborderWidth, &bw,
                     XmNhighlightThickness, &ht,
                     XmNshadowThickness, &st,
                     NULL);

      bw = bw + st + ht;

      if (bw == 0)
         bw = 2;

      /*
      ** Clear the effective drawing area
      */
      XClearArea(XtDisplay(w), XtWindow(w), bw, bw, width-2*bw, 
                  height-2*bw, False);

      /*
      ** Center the icon horizontally
      */
      x =  (width - qB->qI->icon_width)/2; 
      XCopyArea( XtDisplay(w), qB->qI->pixmap, XtWindow(w),
                  DefaultGCOfScreen(XtScreen(w)), 
                  0, 0, qB->qI->icon_width - bw, qB->qI->icon_height - bw, 
                  bw + x, bw);

      /* draw a string into the pixmap */
      sprintf(buf, "@f[SMALL]%s\nSlots: " sge_u32 "("sge_u32")", 
                  qname, used, total);
                  
      str = XmtCreateXmString(buf);
      XmStringExtent(defaultFontList, str, &sw, &sh);
      rect.x = bw;
      rect.y = bw;
      rect.width = width - 2 * bw;
      rect.height = height - 2 * bw;

#ifdef FIXME
      /* if string is to long use unqualified hostname */
      if (sw > rect.width) {
         XmStringFree(str);
         for (i=0; qhostname[i] != '.' && i < strlen(qhostname) && i<128; i++) 
            hostname[i] = qhostname[i];
         hostname[i] = '\0';
         sprintf(buf,"@f[SMALL]%s\n%s\nSlots: %ld (%ld)", 
                  qname, hostname, job_slots_used, 
                  job_slots);
         str = XmtCreateXmString(buf);
         XmStringExtent(defaultFontList, str, &sw, &sh);
      }
#endif

      /*
      ** draw the status bar if necessary
      */
      sbh = (width - 2 * bw - qB->qI->icon_height - sh)/2;
      sbw = (width - 2 * bw)/9;
      
      x = bw + sbw; 
      y = height - 2 * bw - sbh;
      
#ifdef FIXME      
      if (!qinstance_state_is_unknown(q)) {

         for (i=0; i<7; i++) {
            XDrawRectangle(XtDisplay(w), XtWindow(w), qb_gc, 
                              x + sbw * i, y,
                              sbw - 1, sbh);
         }

         /* jobs are running */
         if (job_slots_used)
            XFillRectangle(XtDisplay(w), XtWindow(w), running_gc,
                              x + 1, y + 1,
                              sbw - 1, sbh - 1); 

         /* queue suspended */
         if (qinstance_state_is_susp_on_sub(q))
            XFillRectangle(XtDisplay(w), XtWindow(w), suspend_gc,
                              x + 1 * sbw + 1, y + 1,
                              sbw - 2, (sbh/2 + sbh % 2)); 

         /* queue suspended */
         if (suspend_threshold_alarm) {
            XFillRectangle(XtDisplay(w), XtWindow(w), suspend_gc,
                              x + 1 * sbw + 1, y + (sbh/2 + sbh % 2),
                              sbw - 2, (sbh/2 + sbh % 2 - 1));
         }
         /* queue suspended */
         if (qinstance_state_is_manual_suspended(q)) 
            XFillRectangle(XtDisplay(w), XtWindow(w), suspend_gc,
                              x + 1 * sbw + 1, y + 1,
                              sbw - 1, sbh - 1); 

         /* queue disabled */
         if (qinstance_state_is_manual_disabled(q))
            XFillRectangle(XtDisplay(w), XtWindow(w), disable_gc,
                              x + 2 * sbw + 1, y + 1,
                              sbw - 1, sbh - 1); 
         if (alarm_set)
            XFillRectangle(XtDisplay(w), XtWindow(w), alarm_gc,
                              x + 3 * sbw + 1, y + 1,
                              sbw - 1, sbh - 1); 

         if (qinstance_state_is_error(q))
            XFillRectangle(XtDisplay(w), XtWindow(w), error_gc,
                              x + 4 * sbw + 1, y + 1,
                              sbw - 1, sbh - 1); 

         if (qinstance_state_is_cal_suspended(q))
            XFillRectangle(XtDisplay(w), XtWindow(w), calsuspend_gc,
                              x + 5 * sbw + 1, y + 1,
                              sbw - 1, sbh - 1); 

         if (qinstance_state_is_cal_disabled(q))
            XFillRectangle(XtDisplay(w), XtWindow(w), caldisable_gc,
                              x + 6 * sbw + 1, y + 1,
                              sbw - 2, sbh - 1); 

      }
         
      
      x = rect.x + (rect.width - sw)/2;
      y = height - sh - 2 * sbh - bw;
      
      if (qinstance_state_is_unknown(q))
         draw_gc = error_gc;
      else
         draw_gc = qb_gc;
#endif

      qmonXmStringDraw( XtDisplay(w), XtWindow(w), draw_gc, x, y, 
                        defaultFontList, str, XmALIGNMENT_CENTER , 
                        &rect, &sw, &sh) ;
      XmStringFree(str); 
      if (qB->qI->deleted) {
         XPoint p[4];
         p[0].x = bw;
         p[0].y = bw;
         p[1].x = width - bw;
         p[1].y = height - bw;
         p[2].x = width - bw;
         p[2].y = bw;
         p[3].x = bw;
         p[3].y = height - bw;
/*          XSetForeground(XtDisplay(w), qb_gc, 50);     */
         XDrawLines( XtDisplay(w), XtWindow(w), qb_gc, 
                     p, XtNumber(p), CoordModeOrigin );
      }
   } 
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQueueModify(Widget w, XtPointer cld, XtPointer cad)
{
   int n = 0, i, j;
   lList *lp = NULL;
   
   DENTER(GUI_LAYER, "qmonQueueModify");

   /* 
   ** get the selected queues 
   */
   for (i=0; i<QUEUE_MAX_VERT; i++) {
      for (j=0; j<QUEUE_MAX_HORIZ; j++) {
         if (QBG[i][j].qI && QBG[i][j].qI->selected) {
            if (!lp) {
               lp = lCreateList("DQ", CQ_Type);
            }
            lAppendElem(lp, lCopyElem(QBG[i][j].qI->qp));
         }
      }
   }

   if (lp && ((n = lGetNumberOfElem(lp)) == 1)) {
      /* 
      ** open up the queue configuration dialog and give him a list 
      ** of queues to modify
      */
      qmonQCPopup(w, (XtPointer)lGetString(lFirst(lp), CQ_name), NULL);
   }
   else {
      if (n > 1)
         qmonMessageShow(w, True, "@{Select only one queue !}");
      else
         qmonMessageShow(w, True, "@{To modify a queue select this queue !}");
   }

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonQueueDeleteQuick(Widget w, XtPointer cld, XtPointer cad)
{
   int i, j;
   lList *lp = NULL;
   lList *alp = NULL;
   static lEnumeration *what = NULL;
   Boolean status, answer;
   
   
   DENTER(GUI_LAYER, "qmonQueueDeleteQuick");
   
   
   /* 
   ** we need only the queue name 
   */
   if (!what)
      what = lWhat("%T(%I)", CQ_Type, CQ_name);
   
   /* 
   ** get the selected queues 
   */
   for (i=0; i<QUEUE_MAX_VERT; i++) {
      for (j=0; j<QUEUE_MAX_HORIZ; j++) {
         if (QBG[i][j].qI && QBG[i][j].qI->selected) {
            if (!lp) {
               lp = lCreateList("DQ", CQ_Type);
            }
            lAppendElem(lp, lCopyElem(QBG[i][j].qI->qp));
         }
      }
   }

   if (lp && (lGetNumberOfElem(lp) > 0)) {
      status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                     "@{queue.askdel.Do you really want to\ndelete the selected queues ?}", 
                     "@{Delete}", "@{Cancel}", NULL, XmtNoButton, XmDIALOG_WARNING, 
                     False, &answer, NULL);
         
      if (answer) { 
         alp = qmonDelList(SGE_CQ_LIST, qmonMirrorListRef(SGE_CQ_LIST), 
                           CQ_name, &lp, NULL, what);

         qmonMessageBox(w, alp, 0);

         lFreeList(&alp);
      }
      lFreeList(&lp);

      updateQueueList();
/*       updateQCQ(); */
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonQueueChangeState(Widget w, XtPointer cld, XtPointer cad)
{
   int i, j;
   lList *ql = NULL;
   lList *alp = NULL;
   lListElem *qep = NULL;
   int force = 0;
   long action = (long) cld;
   Widget force_toggle;
   
   DENTER(GUI_LAYER, "qmonQueueChangeState");

   /*
   ** cld contains the action we need, check if a force is involved
   */
   force_toggle = XmtNameToWidget(w, "*queue_force"); 
   force = XmToggleButtonGetState(force_toggle);
   
   /* 
   ** get the selected queues 
   */
   for (i=0; i<QUEUE_MAX_VERT; i++) {
      for (j=0; j<QUEUE_MAX_HORIZ; j++) {
         if (QBG[i][j].qI && QBG[i][j].qI->selected) {
            if (!ql) {
               ql = lCreateList("CQ", ST_Type);
            }
            qep = lCreateElem(ST_Type);
            lSetString(qep, ST_name, lGetString(QBG[i][j].qI->qp, CQ_name));
            lAppendElem(ql, qep);
         }
      }
   }

   if (ql) {
      alp = qmonChangeStateList(SGE_CQ_LIST, ql, force, action); 
   
      qmonMessageBox(w, alp, 0);

      updateQueueList();

      lFreeList(&ql);
      lFreeList(&alp);
   }
   
      
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static char *qmonQueueGetArch(
const char *qhostname 
) {
   lList *ehl = NULL;
   lListElem *ehp = NULL;
   lListElem *lep = NULL;
   char *arch = NULL;
   
   DENTER(GUI_LAYER, "qmonQueueGetArch");

   if (!qhostname) {
      DPRINTF(("no qhostname\n"));
      DEXIT;
      return NULL;
   }
   
   ehl = qmonMirrorList(SGE_EH_LIST);
   ehp = host_list_locate(ehl, qhostname);
   if (ehp)
      lep = lGetSubStr(ehp, HL_name, "arch", EH_load_list);

   if (!lep || !ehp) {
      DPRINTF(("no load arch\n"));
      DEXIT;
      return NULL;
   }

   arch = XtNewString(lGetString(lep, HL_value));

   DEXIT;
   return arch;
}

/*-------------------------------------------------------------------------*/
static char *qmonQueueGetSymbol(
char *arch 
) {
   static char *ICONS[] =  {  
      "xterm", 
      "xterm-dec", 
      "xterm-sgi", 
      "xterm-sun", 
      "xterm-axp", 
      "xterm-sol", 
      "xterm-hp", 
      "xterm-linux", 
      "xterm-ibm"
   };
   int index;

   DENTER(GUI_LAYER, "qmonQueueGetSymbol");
   
   if (!arch) {
      DEXIT;
      return ICONS[0];
   }

   if (!strncmp(arch, "irix", 4))
      index = 2;
   else if (!strncmp(arch, "sgi", 3))
      index = 2;
   else if (!strncmp(arch, "osf", 3))
      index = 1;
   else if (!strncmp(arch, "sun", 3))
      index = 3;
   else if (!strncmp(arch, "axp", 3))
      index = 4;
   else if (!strncmp(arch, "solaris", 7))
      index = 5;
   else if (!strncmp(arch, "hp", 2))
      index = 6;
   else if (!strncmp(arch, "linux", 5))
      index = 7;
   else if (!strncmp(arch, "alinux", 6))
      index = 7;
   else if (!strncmp(arch, "glinux", 6))
      index = 7;
   else if (!strncmp(arch, "slinux", 6))
      index = 7;
   else if (!strncmp(arch, "rs6000", 6))
      index = 8;
   else if (!strncmp(arch, "aix5", 4))
      index = 8;
   else
      index = 0;
   
   DPRINTF(("+++++++++++ARCH: %s, %d\n", arch, index));

   DEXIT;
   return ICONS[index];
}

#if 0
/* don't remove this function */
static void showQueueHashTable(XmtHashTable table, XtPointer key, XtPointer *data)
{
   printf("%s(%ld)\n", XrmQuarkToString((long)key), (long)key);
}   
#endif

