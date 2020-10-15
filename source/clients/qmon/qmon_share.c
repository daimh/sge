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
#include <math.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Layout.h>
#include <Xmt/Chooser.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/MsgLine.h>
#include <Xmt/InputField.h>

#include "ListTree.h"
#include "Matrix.h"

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_init.h"
#include "qmon_timer.h"
#include "qmon_comm.h"
#include "qmon_share.h"
#include "qmon_appres.h"
#include "qmon_message.h"
#include "qmon_widgets.h"
#include "qmon_manop.h"
#include "qmon_project.h"
#include "qmon_globals.h"

#include "uti/sge_string.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_schedd_conf.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sched/sge_support.h"
#include "sched/sgeee.h"

extern sge_gdi_ctx_class_t *ctx;

enum _modes {
   ADD_MODE,
   UPDATE_MODE
};

typedef struct _tAskNodeInfo {
   Boolean blocked;
   XmtButtonType button;
   Widget AskNodeDialog;
   Widget AskNodeName;
   Widget AskNodeShare;
} tAskNodeInfo;
   
typedef struct _tNodeData {
   Cardinal share;
   lListElem *ep;
} tNodeData;
   
typedef struct _tSTNUserData {
   String name;
   Cardinal share;
   double l_percentage;
   double t_percentage;
   double actual_proportion;
   double targetted_share;
   double usage;
   int temp;
} tSTNUserData;

typedef struct _tSTNEntry {
   String   id;
   Cardinal share;
   char     l_percentage[10];
   char     t_percentage[10];
   char     actual_proportion[10];
   char     targetted_share[10];
   double   usage;
} tSTNEntry;

typedef struct _tSTREntry {
   int cpu_weight;
   int mem_weight;
   int io_weight;
   Cardinal halftime;
   char *halflife_decay_list;
   double   compensation_factor;
} tSTREntry;

   
static Widget qmon_sharetree = 0;
#if 0
static Widget st_sharetree_type = 0;
#endif
static Widget st_ratio = 0;
static Widget st_tree = 0;
static Widget st_copy = 0;
static Widget st_cut = 0;
static Widget st_paste = 0;
static Widget st_halflife_unit = 0;
static Widget st_message = 0;

/* static int sharetree_mode = STT_USER;  User Sharetree */
static lList *paste_tree = NULL;

static tSTREntry ratio_data;
static tScaleWeight UsageWeightData[3];

static Boolean dirty = False;

static XtResource node_resources[] = {
   { "id", "id", XtRString,
      sizeof(String),
      XtOffsetOf(tSTNEntry, id),
      XtRImmediate, NULL },

   { "share", "share", XtRCardinal,
      sizeof(Cardinal),
      XtOffsetOf(tSTNEntry, share),
      XtRImmediate, NULL },

   { "l_percentage", "l_percentage", XmtRBuffer,
      XmtSizeOf(tSTNEntry, l_percentage),
      XtOffsetOf(tSTNEntry, l_percentage[0]),
      XtRImmediate, NULL },

   { "t_percentage", "t_percentage", XmtRBuffer,
      XmtSizeOf(tSTNEntry, t_percentage),
      XtOffsetOf(tSTNEntry, t_percentage),
      XtRImmediate, NULL },

   { "actual_proportion", "actual_proportion", XmtRBuffer,
      XmtSizeOf(tSTNEntry, actual_proportion),
      XtOffsetOf(tSTNEntry, actual_proportion),
      XtRImmediate, NULL },

   { "targetted_share", "targetted_share", XmtRBuffer,
      XmtSizeOf(tSTNEntry, targetted_share),
      XtOffsetOf(tSTNEntry, targetted_share),
      XtRImmediate, NULL },

   { "usage", "usage", XmtRDouble,
      sizeof(double),
      XtOffsetOf(tSTNEntry, usage),
      XtRImmediate, NULL }
};

static XtResource ratio_resources[] = {
   { "cpu_weight", "cpu_weight", XtRInt,
      sizeof(int),
      XtOffsetOf(tSTREntry, cpu_weight),
      XtRImmediate, NULL },

   { "mem_weight", "mem_weight", XtRInt,
      sizeof(int),
      XtOffsetOf(tSTREntry, mem_weight),
      XtRImmediate, NULL },

   { "io_weight", "io_weight", XtRInt,
      sizeof(int),
      XtOffsetOf(tSTREntry, io_weight),
      XtRImmediate, NULL },

   { "halftime", "halftime", XtRCardinal,
      sizeof(Cardinal),
      XtOffsetOf(tSTREntry, halftime),
      XtRImmediate, NULL },

   { "halflife_decay_list", "halflife_decay_list", XtRString,
      sizeof(String),
      XtOffsetOf(tSTREntry, halflife_decay_list),
      XtRImmediate, (XtPointer) 0 },

   { "compensation_factor", "compensation_factor", XmtRDouble,
      sizeof(double),
      XtOffsetOf(tSTREntry, compensation_factor),
      XtRImmediate, NULL }
};



/*-------------------------------------------------------------------------*/
static void qmonShareTreePopdown(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeOkay(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeShowMore(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeClearUsage(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeHighlight(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeMenu(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeActivate(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeDestroyItem(Widget w, XtPointer cld, XtPointer cad);
/* static void qmonShareTreeCreateItem(Widget w,  */
/*                                           XtPointer cld,  */
/*                                           XtPointer cad); */
static void qmonShareTreeDeleteNode(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeAddNode(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeAddLeaf(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeModifyNode(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeCopy(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeCut(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreePaste(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeFindNode(Widget w, XtPointer cld, XtPointer cad);
static void qmonShareTreeFindNextNode(Widget w, XtPointer cld, XtPointer cad);
/* static void qmonShareTreeLink(Widget w, XtPointer cld, XtPointer cad); */
/* static void qmonShareTreeToggleType(Widget w, XtPointer cld, XtPointer cad); */
static Widget qmonShareTreeCreate(Widget parent);

static double node_share(XtPointer data);
/* static String node_targetted_resource_share(XtPointer data); */
/* static String node_actual_resource_share(XtPointer data); */
/* static double calculate_usage(ListTreeItem *item); */
static double calculate_simple_share(ListTreeItem *item);
static double calculate_share(ListTreeItem *item);
/* static lList *buildShac(lListElem *parent, int depth); */
static ListTreeItem* CullToTree(Widget tree, ListTreeItem *parent, lList *shac);
static lList* TreeToCull(Widget tree, ListTreeItem *item);
static void CullToParameters(tSTREntry *data, const lListElem *sep);
static void ParametersToCull(lListElem *sep, tSTREntry *data);
static void qmonShareTreeSetValues(ListTreeItem *item);
static void node_data(ListTreeItem *node, Cardinal share, lListElem *ep);
static ListTreeItem* add_node(Widget tree, ListTreeItem *parent, String name, ListTreeItemType type, int mode);

/* static void showtree(Widget w, XtPointer cld, XtPointer cad); */
/* static void showshare(Widget w, XtPointer cld, XtPointer cad); */
/* static void showsimpleshare(Widget w, XtPointer cld, XtPointer cad); */
/* static void showusage(Widget w, XtPointer cld, XtPointer cad); */

static void AskNodeOkCallback(Widget w, XtPointer cld, XtPointer cad);
static void AskNodeCancelCallback(Widget w, XtPointer cld, XtPointer cad);
static Boolean AskForNode(Widget w, String name, int name_len, Cardinal *share);

/*-------------------------------------------------------------------------*/
/* P U B L I C                                                             */
/*-------------------------------------------------------------------------*/
void qmonShareTreePopup(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;

   DENTER(GUI_LAYER, "qmonShareTreePopup");

   /*
   ** reset dirty flag
   */
   dirty = False;

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_sharetree) {
      shell = XmtGetTopLevelShell(w);
      qmon_sharetree = qmonShareTreeCreate(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonShareTreePopdown, NULL);
   } 
   
   /*
   ** set the Sharetree and highlight the root node
   */
   qmonShareTreeUpdate(qmon_sharetree, (XtPointer)st_tree, NULL);
   
   xmui_manage(qmon_sharetree);

/*    XtManageChild(qmon_sharetree); */
/*    XRaiseWindow(XtDisplay(XtParent(qmon_sharetree)),  */
/*                   XtWindow(XtParent(qmon_sharetree))); */

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
/* P R I V A T E                                                           */
/*-------------------------------------------------------------------------*/
static void qmonShareTreePopdown(Widget w, XtPointer cld, XtPointer cad)
{
   Boolean answer = False;
   Boolean status = False;

   DENTER(GUI_LAYER, "qmonShareTreePopdown");

   if (dirty) {
      status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                     "@{stree.asksave.Do you want to save your changes ?}", 
                     "@{Yes}", "@{No}", "@{Cancel}", XmtNoButton, 
                     XmDIALOG_WARNING, False, &answer, NULL);
         
      if(!status) {
         DEXIT;
         return;
      }
      else {
         if (answer)
            qmonShareTreeOkay(w, NULL, NULL);
         else
            dirty = False;
      }      
   }   

   if (!dirty)
      XtUnmanageChild(qmon_sharetree);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Widget qmonShareTreeCreate(Widget parent)
{
   Widget st_layout, st_close, st_update, st_more, st_apply, st_delete,
            st_cpu, st_io, st_mem, st_cpu_t, st_mem_t, st_io_t, st_mod_node,
            st_add_leaf, st_add_node, st_main_link, st_next, st_find, 
            st_clear_usage;
   static int cpu_index = 0;
   static int mem_index = 1;
   static int io_index = 2;

   DENTER(GUI_LAYER, "qmonShareTreeCreate");

   st_layout = XmtBuildQueryDialog( parent, "qmon_sharetree",
                           node_resources, XtNumber(node_resources),
                           "st_close", &st_close,
                           "st_apply", &st_apply,
                           "st_update", &st_update,
                           "st_tree", &st_tree,
                           "st_more", &st_more,
                           "st_clear_usage", &st_clear_usage,
                           "st_delete", &st_delete,
                           "st_add_leaf", &st_add_leaf,
                           "st_add_node", &st_add_node,
                           "st_mod_node", &st_mod_node,
                           "st_copy", &st_copy,
                           "st_cut", &st_cut,
                           "st_paste", &st_paste,
                           "st_find", &st_find,
                           "st_next", &st_next,
                           "st_main_link", &st_main_link,
                           "st_message", &st_message,
#if 0
                           "st_new", &st_new,
                           "st_sharetree_type", &st_sharetree_type,
#endif
                           NULL);
   
   st_ratio = XmtCreateLayout(st_layout, "st_ratio", NULL, 0);

   XmtDialogBindResourceList(st_ratio, ratio_resources, 
                              XtNumber(ratio_resources));
   XmtCreateQueryChildren(st_ratio, 
                          "st_cpu", &st_cpu,
                          "st_mem", &st_mem,
                          "st_io", &st_io,
                          "st_cpu_t", &st_cpu_t,
                          "st_mem_t", &st_mem_t,
                          "st_io_t", &st_io_t,
                          "st_halflife_unit", &st_halflife_unit,
                          NULL);

   /*
   ** initialize userData of ToggleButtons and Scales
   ** and set UsageWeightData to initial values
   */
   UsageWeightData[0].lock = 0;
   UsageWeightData[0].scale = st_cpu;
   UsageWeightData[0].toggle = st_cpu_t;
   UsageWeightData[0].nr = XtNumber(UsageWeightData);
   XtVaSetValues( st_cpu, XmNuserData, (XtPointer) &cpu_index, NULL); 
   XtVaSetValues(st_cpu_t, XmNuserData, (XtPointer) &cpu_index, NULL); 

   UsageWeightData[1].lock = 0;
   UsageWeightData[1].scale = st_mem;
   UsageWeightData[1].toggle = st_mem_t;
   XtVaSetValues( st_mem, XmNuserData, (XtPointer) &mem_index, NULL); 
   XtVaSetValues(st_mem_t, XmNuserData, (XtPointer) &mem_index, NULL); 

   UsageWeightData[2].lock = 0;
   UsageWeightData[2].scale = st_io;
   UsageWeightData[2].toggle = st_io_t;
   XtVaSetValues( st_io, XmNuserData, (XtPointer) &io_index, NULL); 
   XtVaSetValues(st_io_t, XmNuserData, (XtPointer) &io_index, NULL); 


   /*
   ** callbacks
   */
   XtAddCallback(st_cpu, XmNvalueChangedCallback,
                     qmonScaleWeight, (XtPointer)UsageWeightData);
   XtAddCallback(st_mem, XmNvalueChangedCallback,
                     qmonScaleWeight, (XtPointer)UsageWeightData);
   XtAddCallback(st_io, XmNvalueChangedCallback,
                     qmonScaleWeight, (XtPointer)UsageWeightData);

   XtAddCallback(st_cpu_t, XmNvalueChangedCallback,
                     qmonScaleWeightToggle, (XtPointer)UsageWeightData);
   XtAddCallback(st_mem_t, XmNvalueChangedCallback,
                     qmonScaleWeightToggle, (XtPointer)UsageWeightData);
   XtAddCallback(st_io_t, XmNvalueChangedCallback,
                     qmonScaleWeightToggle, (XtPointer)UsageWeightData);
                     

   XtAddCallback(st_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(st_close, XmNactivateCallback, 
                     qmonShareTreePopdown, NULL);
   XtAddCallback(st_apply, XmNactivateCallback, 
                     qmonShareTreeOkay, NULL);
   XtAddCallback(st_more, XmNactivateCallback, 
                     qmonShareTreeShowMore, (XtPointer)st_ratio);
   XtAddCallback(st_clear_usage, XmNactivateCallback, 
                     qmonShareTreeClearUsage, NULL);
   XtAddCallback(st_update, XmNactivateCallback, 
                     qmonShareTreeUpdate, (XtPointer)st_tree);
   XtAddCallback(st_delete, XmNactivateCallback, 
                     qmonShareTreeDeleteNode, (XtPointer)st_tree);
   XtAddCallback(st_add_node, XmNactivateCallback, 
                     qmonShareTreeAddNode, (XtPointer)st_tree);
   XtAddCallback(st_add_leaf, XmNactivateCallback, 
                     qmonShareTreeAddLeaf, (XtPointer)st_tree);
   XtAddCallback(st_mod_node, XmNactivateCallback, 
                     qmonShareTreeModifyNode, (XtPointer)st_tree);
   XtAddCallback(st_copy, XmNactivateCallback, 
                     qmonShareTreeCopy, (XtPointer)st_tree);
   XtAddCallback(st_cut, XmNactivateCallback, 
                     qmonShareTreeCut, (XtPointer)st_tree);
   XtAddCallback(st_paste, XmNactivateCallback, 
                     qmonShareTreePaste, (XtPointer)st_tree);
   XtAddCallback(st_find, XmNactivateCallback, 
                     qmonShareTreeFindNode, (XtPointer)st_tree);
   XtAddCallback(st_next, XmNactivateCallback, 
                     qmonShareTreeFindNextNode, (XtPointer)st_tree);


   XtAddCallback(st_tree, XtNactivateCallback, 
                     qmonShareTreeActivate, NULL);
   XtAddCallback(st_tree, XtNdestroyItemCallback, 
                     qmonShareTreeDestroyItem, NULL);
/*    XtAddCallback(st_tree, XtNcreateItemCallback,  */
/*                      qmonShareTreeCreateItem, NULL); */
   XtAddCallback(st_tree, XtNhighlightCallback, 
                     qmonShareTreeHighlight, NULL);
   XtAddCallback(st_tree, XtNmenuCallback, 
                     qmonShareTreeMenu, (XtPointer)st_tree);

   XtAddEventHandler(XtParent(st_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return st_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeOkay(Widget w, XtPointer cld, XtPointer cad)
{
   lListElem *sep = NULL;
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;

   DENTER(GUI_LAYER, "qmonShareTreeOkay");

   /*
   ** get the tree, check it and send it to qmaster
   */
   lp = TreeToCull(st_tree, NULL);    

   what = lWhat("%T(ALL)", STN_Type);
   if (lp)
      alp = qmonModList(SGE_STN_LIST, 
                     qmonMirrorListRef(SGE_STN_LIST),
                     0, &lp, NULL, what);
   else
      alp = qmonDelList(SGE_STN_LIST,
                     qmonMirrorListRef(SGE_STN_LIST),
                     0, &lp, NULL, what);
                     

   qmonMessageBox(w, alp, 0);

   if (alp && lGetUlong(lFirst(alp), AN_status) != STATUS_OK) {
      XmtMsgLinePrintf(st_message, "Failure");
      XmtMsgLineClear(st_message, DISPLAY_MESSAGE_DURATION); 
      lFreeList(&lp);
      lFreeList(&alp);
      lFreeWhat(&what);
      DEXIT;
      return;
   }   
   else {
      XmtMsgLinePrintf(st_message, "Success");
      XmtMsgLineClear(st_message, DISPLAY_MESSAGE_DURATION); 
      lFreeList(&lp);
      lFreeList(&alp);
      lFreeWhat(&what);
      dirty = False;
   }   

   /*
   ** get the ratio data and send them to qmaster
   */
   XmtDialogGetDialogValues(st_ratio, &ratio_data);

   qmonMirrorMultiAnswer(SC_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   lp = qmonMirrorList(SGE_SC_LIST);
   sep = lFirst(lp);
   if (sep) {
      ParametersToCull(sep, &ratio_data);
      
      what = lWhat("%T(ALL)", SC_Type);
      alp = qmonModList(SGE_SC_LIST, 
                        qmonMirrorListRef(SGE_SC_LIST),
                        0, &lp, NULL, what);

      qmonMessageBox(w, alp, 0);
   
      if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
         printf("---Share Tree Parameters--------------\n");
         lWriteListTo(lp, stdout);
         printf("_______________________________________\n");
      }

      lFreeList(&alp);
      lFreeWhat(&what);
   }


   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeAddLeaf(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget)cld;
   ListTreeItem *item = NULL;
   ListTreeItem *new = NULL;
   ListTreeMultiReturnStruct ret;
   char name[256];
   Cardinal share = 0;
   Boolean status;

   DENTER(GUI_LAYER, "qmonShareTreeAddLeaf");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];
   else {
      DEXIT;
      return;
   }

   /*
   ** if the node or the parent node is default don't allow editing
   ** the nodes are read-only
   */
   if ((item->text && !strcasecmp(item->text, "default")) ||
       (item->parent && item->parent->text && !strcasecmp(item->parent->text, "default"))) {
      DEXIT;
      return;
   }   

   /*
   ** create new_node
   */
   if (item->type == ItemLeafType)
      item = item->parent;
   new = add_node(tree, item, "", ItemLeafType, ADD_MODE);

   strcpy(name, "");
   status = AskForNode(w, name, 256, &share);

   if (!status) {
      ListTreeDelete(tree, new);
   }
   else {
      node_data(new, share, NULL);
      ListTreeRenameItem(tree, new, name);
      item = new;
      dirty = True;
   }
   /*
   ** Highlight the new_node
   */
   ListTreeHighlightItem(tree, item, True);
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeAddNode(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget)cld;
   ListTreeItem *item = NULL;
   ListTreeItem *new = NULL;
   ListTreeMultiReturnStruct ret;
   char name[256] = "";
   Cardinal share = 0;
   Boolean status;
   Boolean isDefault;

   DENTER(GUI_LAYER, "qmonShareTreeAddNode");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];
   else {
      if (!ListTreeFirstItem(tree)) {
         new = add_node(tree, item, "Root", ItemBranchType, ADD_MODE);
         node_data(new, 1, NULL);
      }
      DEXIT;
      return;
   }
      
   /*
   ** if the node or the parent node is default don't allow editing
   ** the nodes are read-only
   */
   if ((item->text && !strcasecmp(item->text, "default")) ||
       (item->parent && item->parent->text && !strcasecmp(item->parent->text, "default"))) {
      DEXIT;
      return;
   }   


   /*
   ** create new_node
   */
   if (item->type == ItemLeafType)
      item = item->parent;
   new = add_node(tree, item, "", ItemBranchType, ADD_MODE);

   status = AskForNode(w, name, 256, &share);

   isDefault =  (strcmp(name, "default") == 0);
   if (!status || isDefault) {
      if (isDefault) {
         qmonMessageShow(w, True, "@{Nodes named 'default' are not allowed !}");
      }   
      ListTreeDelete(tree, new);
   } else {
      node_data(new, share, NULL);
      ListTreeRenameItem(tree, new, name);
      item = new;
      dirty = True;
   }

   /*
   ** Highlight the new_node
   */
   ListTreeHighlightItem(tree, item, True);
      

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeCut(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget)cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;
   tSTNUserData *data;

   DENTER(GUI_LAYER, "qmonShareTreeCut");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count <= 0) {
      DEXIT;
      return;
   }
   item = ret.items[0];

   /*
   ** if the node or the parent node is default don't allow editing
   ** the nodes are read-only
   */
   if (item && item->parent && item->parent->text && !strcasecmp(item->parent->text, "default")) {
      DEXIT;
      return;
   }   

   if (item != ListTreeFirstItem(tree)) {
      paste_tree = lCreateElemList("Tree", STN_Type, 1);
      if (paste_tree) {
         lSetString(lFirst(paste_tree), STN_name, item->text);
         data = (tSTNUserData*)(item->user_data);
         lSetUlong(lFirst(paste_tree), STN_shares, data->share);
         lSetList(lFirst(paste_tree), STN_children, TreeToCull(tree, item));
         XtSetSensitive(st_cut, False);
         XtSetSensitive(st_copy, False);
         XtSetSensitive(st_paste, True);
      }
      ListTreeDelete(tree, item);
      dirty = True;
   }
   else {
      qmonMessageShow(w, True, "@{Can't cut the whole tree !}");
   }
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeCopy(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget)cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;
   tSTNUserData *data;

   DENTER(GUI_LAYER, "qmonShareTreeCopy");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count <= 0) {
      DEXIT;
      return;
   }
   item = ret.items[0];

   if (item != ListTreeFirstItem(tree)) {
      paste_tree = lCreateElemList("Tree", STN_Type, 1);
      if (paste_tree) {
         lSetString(lFirst(paste_tree), STN_name, item->text);
         data = (tSTNUserData*)(item->user_data);
         lSetUlong(lFirst(paste_tree), STN_shares, data->share);
         lSetList(lFirst(paste_tree), STN_children, TreeToCull(tree, item));
         XtSetSensitive(st_cut, False);
         XtSetSensitive(st_copy, False);
         XtSetSensitive(st_paste, True);
      }
   }
   else {
      qmonMessageShow(w, True, "@{Can't copy the whole tree !}");
   }
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreePaste(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget)cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;

   DENTER(GUI_LAYER, "qmonShareTreePaste");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count <= 0) {
      DEXIT;
      return;
   }
   item = ret.items[0];

   /*
   ** if the node or the parent node is default don't allow editing
   ** the nodes are read-only
   */
   if ((item->text && !strcasecmp(item->text, "default")) || 
        (item->parent && item->parent->text && !strcasecmp(item->parent->text, "default"))) {
      DEXIT;
      return;
   }   

   CullToTree(tree, item, paste_tree);
   lFreeList(&paste_tree);
   ListTreeRefresh(tree);
   dirty = True;
   XtSetSensitive(st_paste, False);
   XtSetSensitive(st_copy, True);
   XtSetSensitive(st_cut, True);
      
   DEXIT;
}

static char search_for[256] = "";
static ListTreeMultiReturnStruct *matches;

/*-------------------------------------------------------------------------*/
static void qmonShareTreeFindNode(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget)cld;
   ListTreeItem *item = NULL;
   char name[256];
   Boolean status;

   DENTER(GUI_LAYER, "qmonShareTreeFindNode");

   /*
   ** create new_node
   */
   strcpy(name, "");
   status = XmtAskForString(w, "xmtAskForString", "@{Enter item name}", name, 256, NULL);

   if (status && name[0] != '\0') {
      item = ListTreeFindChildName(tree, ListTreeFirstItem(tree), name);
      if (!item) {
         DEXIT;
         return;
      }   
      sge_strlcpy(search_for, name, 256);
      matches = NULL;
      /*
      ** Highlight the new_node
      */
      ListTreeOpenNode(tree, item);
      ListTreeMakeItemVisible(tree, item);
      ListTreeHighlightItem(tree, item, True);
   }
   else {
      strcpy(search_for, "");
   }   

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeFindNextNode(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget)cld;
   static int count = 0;
   ListTreeItem *current;
   int first = 0;

   DENTER(GUI_LAYER, "qmonShareTreeFindNode");

   if (!matches && strcmp(search_for, "")) {
      first = 1;
      count = 0;
      matches = ListTreeBuildSearchList(tree, ListTreeFirstItem(tree),search_for, 1);
      /*
      {
         int i;
         for (i=0; i<matches->count; i++) 
            printf("matches->items[%d]->text: %s\n", i, matches->items[i]->text);
      }
      */
   }

   /*
   ** Highlight the new_node
   */
   if (matches && matches->count > 1 && count < matches->count) {
      if (first) {
         first = 0;
         count++;
      }
      current = matches->items[count];
      DPRINTF(("current->text: %s, current->count = %d\n", current->text, current->count));
      /*
      ** open branch and highlight item
      */
      ListTreeOpenNode(tree, current);
      ListTreeMakeItemVisible(tree, current);
      ListTreeHighlightItem(tree, current, True);

      count++;
      if (count == matches->count)
         count = 0;
   }

   DEXIT;
}

#if 0
/*-------------------------------------------------------------------------*/
static void qmonShareTreeToggleType(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct*) cad;

   DENTER(GUI_LAYER, "qmonShareTreeToggleType");

   if (cbs->state)
      sharetree_mode = STT_PROJECT;
   else
      sharetree_mode = STT_USER;
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeLink(Widget w, XtPointer cld, XtPointer cad)
{

   DENTER(GUI_LAYER, "qmonShareTreeToggleType");

   switch (sharetree_mode) {
      case STT_USER:  /* User Sharetree */
         qmonPopupManopConfig(w, (XtPointer) USER_CONFIG, NULL);
         break;
      case STT_PROJECT:  /* Project Sharetree */
         qmonPopupProjectConfig(w, NULL, NULL);   
         break;
   }
   
   DEXIT;
}
#endif

/*-------------------------------------------------------------------------*/
static void qmonShareTreeShowMore(Widget w, XtPointer cld, XtPointer cad)
{
   static Boolean managed = False;
   Widget ratio = (Widget)cld;
   static Pixmap open;
   static Pixmap closed;

   DENTER(GUI_LAYER, "qmonShareTreeShowMore");

   if (!open) {
      open = qmonGetIcon("leftarrow");
      closed = qmonGetIcon("rightarrow");
   }
  
   if (!managed) {
      managed = True;
      XtVaSetValues(w, XmNlabelPixmap, open, NULL);
      XtManageChild(ratio);
   }
   else {
      managed = False;
      XtVaSetValues(w, XmNlabelPixmap, closed, NULL);
      XtUnmanageChild(ratio);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeClearUsage(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;
   lList *alp2 = NULL;
   lList *ul = NULL;
   lList *pl = NULL;
   lListElem *ep = NULL;
   Boolean status, answer;

   DENTER(GUI_LAYER, "qmonShareTreeClearUsage");


   status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                  "@{stree.askdel.Do you really want to\nclear the usage ?}", 
                  "@{Clear}", "@{Cancel}", NULL, XmtNoButton, XmDIALOG_WARNING, 
                  False, &answer, NULL);
      
   if (!answer) { 
      DEXIT;
      return;
   }   

   qmonMirrorMultiAnswer(USER_T | PROJECT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   ul = qmonMirrorList(SGE_UU_LIST);
   pl = qmonMirrorList(SGE_PR_LIST);

   /* clear user usage */
   for_each(ep, ul) {
      lSetList(ep, UU_usage, NULL);
      lSetList(ep, UU_project, NULL);
   }

   /* clear project usage */
   for_each(ep, pl) {
      lSetList(ep, PR_usage, NULL);
      lSetList(ep, PR_project, NULL);
   }

   /* update user usage */
   if (ul) {
      alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_MOD, &ul, NULL, NULL, false);
   }

   /* update project usage */
   if (pl) {
      alp2 = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_MOD, &pl, NULL, NULL, false);
   }

   if (alp || alp2) {
      if (alp)
         lAddList(alp, &alp2);
      else   
         alp = alp2;
      if (!qmonMessageBox(w, alp, 0)) {
         qmonShareTreeUpdate(w, st_tree, NULL);
         XmtMsgLinePrintf(st_message, "Success");
         XmtMsgLineClear(st_message, DISPLAY_MESSAGE_DURATION); 
      } else {  
         XmtMsgLinePrintf(st_message, "Failure");
         XmtMsgLineClear(st_message, DISPLAY_MESSAGE_DURATION); 
      }
   }

   lFreeList(&alp);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeDeleteNode(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget) cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;

   DENTER(GUI_LAYER, "qmonShareTreeDeleteNode");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];

   if (!item) {
      DEXIT;
      return;
   }   
      
   
   /*
   ** if the parent node is default don't allow editing
   ** the nodes are read-only
   */
   if (item->parent && item->parent->text && !strcasecmp(item->parent->text, "default")) {
      DEXIT;
      return;
   }   

   ListTreeDelete(tree, item);
   dirty = True;

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget) cld;
   lList *share_tree = NULL;
   ListTreeItem *old_root = NULL;
   lList *scl = NULL;
   lList *ul = NULL;
   lList *usl = NULL;
   lList *pl = NULL;
   ListTreeItem *root_node = NULL;
   Boolean answer = False;
   Boolean status = False;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonShareTreeUpdate");

   if (dirty) {
      status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                     "@{stree.asksave.Do you want to save your changes ?}", 
                     "@{Yes}", "@{No}", "@{Cancel}", XmtNoButton, 
                     XmDIALOG_WARNING, False, &answer, NULL);
         
      if(!status) {
         DEXIT;
         return;
      } else {
         if (answer) {
            qmonShareTreeOkay(w, NULL, NULL);
            /*
            ** answer yes and storing sharetree failed
            */
            if (dirty == True) {
               DEXIT;
               return;
            }
         } else {
            dirty = False;
         }
      }      
   }   
   /*
   ** set the share tree insensitive to disable editing
   */
/*    XtSetSensitive(tree, False); */
   XmtDisplayBusyCursor(w);

   
   /*
   ** set and get several lists
   */
   qmonMirrorMultiAnswer(SHARETREE_T | USER_T | PROJECT_T | SC_T| USERSET_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }

   share_tree = qmonMirrorList(SGE_STN_LIST);
   ul = qmonMirrorList(SGE_UU_LIST);
   usl = qmonMirrorList(SGE_US_LIST);
   pl = qmonMirrorList(SGE_PR_LIST);
   
   /* 
   ** actual values from master  must be set for sconf_get_config() 
   ** every time otherwise changes are not shown correctly in the dialog
   ** IZ: 1058
   */ 
   scl = lCopyList("", qmonMirrorList(SGE_SC_LIST));
   sconf_set_config(&scl, NULL);

   /*
    * add default user nodes for display purposes
    */
   sge_add_default_user_nodes(lFirst(share_tree), ul, pl, usl);

   /*
   ** fill the share tree with the actual values
   */
   sge_calc_share_tree_proportions(share_tree, ul, pl, NULL);

   ListTreeRefreshOff(tree);
   /*
   ** unchain the old tree, what about the old user_data struct ????
   */
   old_root = ListTreeFirstItem(tree);
   ListTreeUnchainItem(tree, old_root);
      
   root_node = CullToTree(tree, NULL, share_tree);

   /*
   ** open the new tree according to old tree or to
   ** a defined level
   */
   if (old_root) {
      ListTreeOpenLikeTree(tree, root_node, old_root);
      ListTreeDelete(tree, old_root);
   }
   else
      ListTreeOpenToLevel(tree, NULL, 1);
   
   ListTreeHighlightItem(tree, root_node, True);
   ListTreeRefreshOn(tree);
   ListTreeMakeItemVisible(tree, root_node);

   /*
   ** set the share tree sensitive to enable editing
   */
   XmtDisplayDefaultCursor(w);

   /*
   ** refresh the Sharetree Parameters section 
   */
   if (sconf_is()) {
      lListElem *schedd_conf = sconf_get_config();

      CullToParameters(&ratio_data, schedd_conf);
      XmtDialogSetDialogValues(st_ratio, &ratio_data);

      lFreeElem(&schedd_conf);
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeHighlight(Widget w, XtPointer cld, XtPointer cad)
{
   ListTreeMultiReturnStruct *ret = (ListTreeMultiReturnStruct*)cad;
   ListTreeItem *item;
   int i;

   DENTER(GUI_LAYER, "qmonShareTreeHighlight");

   if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
      printf("HIGHLIGHT: count=%d\n",ret->count);
      for (i=0; i<ret->count; i++) {
        item=ret->items[i];
        printf("%s",item->text ? item->text : "-NA-");
        while (item->parent) {
          item=item->parent;
          printf("<--%s",item->text ? item->text : "-NA-");
        }
        printf("\n");
      }
   }

   if (ret && ret->count > 0) {
      item = ret->items[0];
      qmonShareTreeSetValues(item);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeSetValues(ListTreeItem *item)
{
   tSTNUserData *data;
   tSTNEntry newdata;
   char buf[10];

   DENTER(GUI_LAYER, "qmonShareTreeSetValues");

   if (item && item->user_data) {
      data = (tSTNUserData*)item->user_data;
      
      newdata.id = item->text ? item->text : "";
      newdata.share = data->share;
      snprintf(buf, sizeof buf, "%3.1f %%", 100*data->actual_proportion);
      strcpy(newdata.actual_proportion, buf);
      snprintf(buf, sizeof buf, "%3.1f %%", 100*data->targetted_share);
      strcpy(newdata.targetted_share, buf);
      newdata.usage = data->usage;
      snprintf(buf, sizeof buf, "%3.1f %%", 100*calculate_simple_share(item));
      strcpy(newdata.l_percentage, buf);
      snprintf(buf, sizeof buf, "%3.1f %%", 100*calculate_share(item));
      strcpy(newdata.t_percentage, buf);

      XmtDialogSetDialogValues(qmon_sharetree, &newdata);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeMenu(Widget w, XtPointer cld, XtPointer cad)
{
   ListTreeWidget tree = (ListTreeWidget) cld;
   ListTreeItemReturnStruct *ret = (ListTreeItemReturnStruct *)cad;
   Boolean status;
   char name[256];
   Cardinal share;

   DENTER(GUI_LAYER, "qmonShareTreeMenu");

   /*
   ** if the node is Root don't allow editing
   ** the node is read-only
   */
   if (ret->item && ret->item->text && !strcasecmp(ret->item->text, "Root")) {
      DEXIT;
      return;
   }   

   /*
   ** if the parent node is default don't allow editing
   ** the nodes are read-only
   */
   if (ret->item->parent && ret->item->parent->text && !strcasecmp(ret->item->parent->text, "default")) {
      DEXIT;
      return;
   }   

   if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
      printf ("MENU: item=%s\n", ret->item->text);
   }


   /*
   ** set the values in the item for the node popup
   */
   strcpy(name, ret->item->text);
   share = ((tSTNUserData*)ret->item->user_data)->share; 
   status = AskForNode(w, name, 256, &share);

   if (status) {
      node_data(ret->item, share, NULL);
      ListTreeRenameItem((Widget)tree, ret->item, name);
      /*
      ** Highlight the new_node
      */
      ListTreeHighlightItem((Widget)tree, ret->item, True);
      dirty = True;
   }
      

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeModifyNode(Widget w, XtPointer cld, XtPointer cad)
{
   ListTreeWidget tree = (ListTreeWidget) cld;
   Boolean status;
   char name[256];
   Cardinal share;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;
   lListElem *ep = NULL;
   lList *shl = NULL;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonShareTreeModifyNode");

   ListTreeGetHighlighted((Widget)tree, &ret);

   if (ret.count == 1)
      item = ret.items[0];
   else {
      DEXIT;
      return;
   }

   /*
   ** if the node is Root don't allow editing
   ** the node is read-only
   */
   if (item && item->text && !strcasecmp(item->text, "Root")) {
      DEXIT;
      return;
   }   

   /*
   ** if the parent node is default don't allow editing
   ** the nodes are read-only
   */
   if (item->parent && item->parent->text && !strcasecmp(item->parent->text, "default")) {
      DEXIT;
      return;
   }   

   /*
   ** set the values in the item for the node popup
   */
   strcpy(name, item->text);
   share = ((tSTNUserData*)item->user_data)->share; 
   status = AskForNode(w, name, 256, &share);

   if (status) {
      /*
      ** this is not correctly handled and the modify of the shares
      ** should be done by the master without affecting the usage
      */
      qmonMirrorMultiAnswer(SHARETREE_T, &alp);
      if (alp) {
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);
         DEXIT;
         return;
      }
      shl = qmonMirrorList(SGE_STN_LIST);  
      ep = lGetElemStr(shl, STN_name, name);

      node_data(item, share, ep);
      ListTreeRenameItem((Widget)tree, item, name);
      /*
      ** Highlight the new_node
      */
      ListTreeHighlightItem((Widget)tree, item, True);
      dirty = True;
   }
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeActivate(Widget w, XtPointer cld, XtPointer cad)
{
   ListTreeActivateStruct *ret = (ListTreeActivateStruct*)cad;
   int count;

   DENTER(GUI_LAYER, "qmonShareTreeActivate");

   if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
      printf("ACTIVATE: item=%s count=%d\n",ret->item->text,ret->count);
      count=0;
      while (count<ret->count) {
         printf(" path: %s\n",ret->path[count]->text);
         count++;
      }
   }

   qmonShareTreeSetValues(ret->item);

   DEXIT;
}

#if 0
/*-------------------------------------------------------------------------*/
static lList *buildShac(lListElem *parent, int depth)
{
   lList *shac;
   lListElem *ep;
   char buf[BUFSIZ];
   int i;
   int nodes = 3;

   DENTER(GUI_LAYER, "buildShac");

   shac = lCreateElemList("ShareTree", STN_Type, nodes);
   
   for (i=0, ep = lFirst(shac); i<nodes; i++, ep = lNext(ep)) {
      if (parent)
         sprintf(buf, "%s.%d", lGetString(parent, STN_name), i);
      else
         sprintf(buf, "Node %d", i);
      lSetString(ep, STN_name, buf);
/*       lSetUlong(ep, STN_type, STT_USER); */
      lSetUlong(ep, STN_shares, i+1);
      if (depth > 0) {
         lSetList(ep, STN_children, buildShac(ep, depth-1));
      }
   }   

   DEXIT;
   return shac;
}
      

/*-------------------------------------------------------------------------*/
static void qmonShareTreeCreateItem(Widget w, XtPointer cld, XtPointer cad)
{
   ListTreeItemReturnStruct *cbs = (ListTreeItemReturnStruct*)cad;
   tNodeData *data = (tNodeData *) cld;

   DENTER(GUI_LAYER, "qmonShareTreeCreateItem");
   
printf("qmonShareTreeCreateItem\n");

   
   if (cbs->item && !cbs->item->user_data && data) {
      node_data(cbs->item, data->share, data->ep); 
   }
   
   DEXIT;
}

#endif

/*-------------------------------------------------------------------------*/
static void qmonShareTreeDestroyItem(Widget w, XtPointer cld, XtPointer cad)
{
   ListTreeItemReturnStruct *cbs = (ListTreeItemReturnStruct*)cad;

   DENTER(GUI_LAYER, "qmonShareTreeDestroyItem");
   
   if (cbs->item && cbs->item->user_data)
      XtFree((char*)cbs->item->user_data);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static ListTreeItem* add_node(
Widget tree,
ListTreeItem *parent,
String name,
ListTreeItemType type,
int mode 
) {
   ListTreeItem *node;

   DENTER(GUI_LAYER, "add_node");

   if (mode == ADD_MODE) {
      /*
      ** go up to nearest branch and open it
      */
      if (parent && parent->type == ItemLeafType) 
#ifdef notdef
         parent = parent->parent; 
#else
         parent->type = ItemBranchType;
#endif

      ListTreeOpenNode(tree, parent);
   }
   /*
   ** add a new node of type 'type'
   */
   node = ListTreeAddType(tree, parent, name, type);
#if 1
   if (!strcasecmp(name, "default")) {
      Pixmap leaf_plus = qmonGetIcon("leaf_plus");
      Pixmap leaf_minus = qmonGetIcon("leaf_minus");
      ListTreeSetItemPixmaps(tree, node, leaf_minus, leaf_plus);
   }
#endif      

   if (mode == ADD_MODE) {
      /*
      ** Highlight the new_node
      */
      ListTreeHighlightItem(tree, node, True);
   }

   DEXIT;

   return node;
}



/*-------------------------------------------------------------------------*/
static void node_data(
ListTreeItem *node,
Cardinal share,
lListElem *ep 
) {
   tSTNUserData *user_data = NULL;

   DENTER(GUI_LAYER, "node_data");

   /*
   ** malloc user_data struct
   */
   user_data = XtNew(tSTNUserData);

   /* 
   ** attach the user data to the node 
   */
   node->user_data = user_data; 

   /*
   ** initialize the user_data
   */
   if (ep) {
      user_data->share = lGetUlong(ep, STN_shares);
      user_data->actual_proportion = lGetDouble(ep, STN_actual_proportion); 
      user_data->targetted_share = lGetDouble(ep, STN_m_share);
      user_data->usage = lGetDouble(ep, STN_combined_usage);
      user_data->temp = lGetUlong(ep, STN_temp);
   }
   else {
      user_data->share = share;
      user_data->actual_proportion = 0;
      user_data->targetted_share = 0;
      user_data->usage = 0;
      user_data->temp = 0;
   }
   
   DEXIT;
}
   

/*-------------------------------------------------------------------------*/
static ListTreeItem* CullToTree(
Widget tree,
ListTreeItem *parent,
lList *shac 
) {
   lListElem *ep = NULL;
   ListTreeItem *node = NULL;
   ListTreeItem *ret = NULL;
   StringConst name = NULL;
   lList *sublist = NULL;
   char buf[1024];
   static int level = 0;
   Boolean branch = False;

   DENTER(GUI_LAYER, "CullToTree");


   for_each(ep, shac) {
      /*
      ** create new node
      ** 
      */
      name = lGetString(ep, STN_name);
      sublist = lGetList(ep, STN_children);
      if (sublist || (lGetUlong(ep, STN_type) == STT_PROJECT))
         branch = True;
      else   
         branch = False;
      sprintf(buf, "%s", name ? name : "-NoName-"); 
      node = add_node(tree, parent, buf, 
                  branch ? ItemBranchType : ItemLeafType, UPDATE_MODE);
      node_data(node, 0, (XtPointer) ep);

      /*
      ** process subtree
      */
      if (sublist) {
         level++;
         CullToTree(tree, node, sublist);
         level--;
      }
   }

   if (!level) {
      ret = ListTreeFirstItem(tree);
      ListTreeOrderChildren(tree, ret);
      ret = ListTreeFirstItem(tree);
   }


   DEXIT;
   return ret; 
}

/*-------------------------------------------------------------------------*/
static lList* TreeToCull(
Widget tree,
ListTreeItem *item 
) {
   lList *lp = NULL;
   lListElem *ep = NULL;
   ListTreeItem *node = NULL;
   tSTNUserData *data;
   
   DENTER(GUI_LAYER, "TreeToCull");

   if (!item) 
      node = ListTreeFirstItem(tree);
   else
      node = ListTreeFirstChild(item);

   while (node) {
      data = (tSTNUserData*)node->user_data;
      if (!data->temp) {
         ep = lAddElemStr(&lp, STN_name, node->text, STN_Type);
         lSetUlong(ep, STN_shares, data->share);
/*       lSetUlong(ep, STN_type, sharetree_mode); */

         if (ListTreeFirstChild(node))
            lSetList(ep, STN_children, TreeToCull(tree, node));
      }
      
      node = ListTreeNextSibling(node);
   }

   DEXIT;
   return lp;
}

/*-------------------------------------------------------------------------*/
static void CullToParameters(
tSTREntry *data,
const lListElem *sep 
) {
   lList *usage_weight_list;
   lListElem *ep;
   double cpu_w, io_w, mem_w;

   DENTER(GUI_LAYER, "CullToParameters");

   usage_weight_list = lGetList(sep, SC_usage_weight_list);

   ep = lGetElemStr(usage_weight_list, UA_name, USAGE_ATTR_CPU);
   if (ep) {
      /* round */
      cpu_w = floor(lGetDouble(ep, UA_value)*1000 + 0.5);
      data->cpu_weight = (int)cpu_w;
   }
   else
      data->cpu_weight = 0;

   ep = lGetElemStr(usage_weight_list, UA_name, USAGE_ATTR_MEM);
   if (ep) {
      /* round */
      mem_w = floor(lGetDouble(ep, UA_value)*1000 + 0.5);
      data->mem_weight = (int)mem_w;
   }
   else
      data->mem_weight = 0;
   
   ep = lGetElemStr(usage_weight_list, UA_name, USAGE_ATTR_IO);
   if (ep) {
      /* round */
      io_w = floor(lGetDouble(ep, UA_value)*1000 + 0.5);
      data->io_weight = (int)io_w;
   }
   else
      data->io_weight = 0;
   
   DPRINTF(("cpu_weight: %d, mem_weight: %d, io_weight: %d\n",
         data->cpu_weight, data->mem_weight, data->io_weight));

   if ((data->cpu_weight+data->io_weight+data->mem_weight) != 1000) {
      data->cpu_weight = 333;
      data->mem_weight = 333;
      data->io_weight = 334;
   }

   /*
   ** set the UsageWeightData
   */
   UsageWeightData[0].weight = data->cpu_weight;
   UsageWeightData[1].weight = data->mem_weight;
   UsageWeightData[2].weight = data->io_weight;
   
   data->compensation_factor = lGetDouble(sep, SC_compensation_factor);

   data->halftime = lGetUlong(sep, SC_halftime);
   if ((data->halftime/24) > 0 && (data->halftime % 24) == 0) {
      data->halftime /= 24;
      XmtChooserSetState(st_halflife_unit, 1, False);
   }

   data->halflife_decay_list = sge_strdup(data->halflife_decay_list,
                                    lGetString(sep, SC_halflife_decay_list));

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void ParametersToCull(
lListElem *sep,
tSTREntry *data 
) {
   lList *usage_weight_list;
   lListElem *ep;
   int state;

   DENTER(GUI_LAYER, "ParametersToCull");
   
   usage_weight_list = lCreateElemList("UWL", UA_Type, 3);

   if (usage_weight_list) {
      /* CPU */
      ep = lFirst(usage_weight_list);
      lSetString(ep, UA_name, USAGE_ATTR_CPU);
      lSetDouble(ep, UA_value, ((double)data->cpu_weight)/1000);
      
      /* MEM */
      ep = lNext(ep);
      lSetString(ep, UA_name, USAGE_ATTR_MEM);
      lSetDouble(ep, UA_value, ((double)data->mem_weight)/1000);

      /* IO */
      ep = lNext(ep);
      lSetString(ep, UA_name, USAGE_ATTR_IO);
      lSetDouble(ep, UA_value, ((double)data->io_weight)/1000);
   }

   lSetList(sep, SC_usage_weight_list, usage_weight_list);
   lSetDouble(sep, SC_compensation_factor, data->compensation_factor);

   state = XmtChooserGetState(st_halflife_unit); 
   if (state)
      state = 24;
   else
      state = 1;

   lSetUlong(sep, SC_halftime, data->halftime * state);

   if (data->halflife_decay_list && data->halflife_decay_list[0] !='\0')
      lSetString(sep, SC_halflife_decay_list, data->halflife_decay_list);
   else   
      lSetString(sep, SC_halflife_decay_list, "none");


   DEXIT;
}

/*-------------------------------------------------------------------------*/
static double node_share(
XtPointer data 
) {
   tSTNUserData *ud = (tSTNUserData *)data;
   double ret = 0.0;
   
   DENTER(GUI_LAYER, "node_share");

   if (ud) 
      ret = (double)(ud->share);

   DEXIT;
   return ret;
}


/*-------------------------------------------------------------------------*/
static double calculate_share(
ListTreeItem *item 
) {
   ListTreeItem *parent;
   ListTreeItem *sibling;
   double level_sum = 0.0;
   double share;
   
   DENTER(GUI_LAYER, "calculate_share");

   if (!item) {
      DEXIT;
      return 1.0;
   }

   parent = ListTreeParent(item);

   /*
   ** get parent share
   */
   share = calculate_share(parent);

   if (parent) {
      sibling = ListTreeFirstChild(parent);
      while (sibling) {
         /*
         ** sum the shares
         */
         level_sum += node_share(sibling->user_data);

         sibling = ListTreeNextSibling(sibling);
      }

      if (level_sum > 0.0)
         share *= node_share(item->user_data)/level_sum;
      else
         share = 0.0;
   }

/* printf("Share = %f\n", share); */
   DEXIT;

   return share;
}
   
/*-------------------------------------------------------------------------*/
static double calculate_simple_share(
ListTreeItem *item 
) {
   ListTreeItem *parent;
   ListTreeItem *sibling;
   double level_sum = 0.0;
   double share = 1.0;
   
   DENTER(GUI_LAYER, "calculate_simple_share");

   if (!item) {
      DEXIT;
      return 0.0;
   }

   parent = ListTreeParent(item);

   if (parent) {
      sibling = ListTreeFirstChild(parent);
      while (sibling) {
         /*
         ** sum the shares
         */
         level_sum += node_share(sibling->user_data);
         
         sibling = ListTreeNextSibling(sibling);
      }

      if (level_sum > 0.0)
         share = node_share(item->user_data)/level_sum;
      else
         share = 0.0;
   }

/* printf("Share = %f\n", share); */
   DEXIT;

   return share;
}

#if 0
/*-------------------------------------------------------------------------*/
static double node_usage(
XtPointer data
) {
   tSTNUserData *ud = (tSTNUserData *)data;
   double ret = 0.0;

   DENTER(GUI_LAYER, "node_usage");

   if (ud)
      ret = ud->usage;

   DEXIT;
   return ret;
}

/*-------------------------------------------------------------------------*/
static double calculate_usage(
ListTreeItem *item 
) {
   ListTreeItem *child;
   double level_sum = 0.0;
   
   DENTER(GUI_LAYER, "calculate_usage");

   if (item) {
      child = ListTreeFirstChild(item);
      if (!child) {
         level_sum += node_usage(item->user_data); 
      }
      else {
         while (child) {
            level_sum += calculate_usage(child);
            child = ListTreeNextSibling(child);
         }
      }
   }

   DEXIT;

   return level_sum;
}




/*-------------------------------------------------------------------------*/
static void showtree(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget) cld;
   lList *shac = NULL;
   FILE *fp;

   DENTER(GUI_LAYER, "showtree");

   shac = TreeToCull(tree, NULL);

   fp = fopen("tree_out", "w");

   printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
   lWriteListTo(shac, stdout);
   
   lDumpList(fp, shac, 0);

   FCLOSE(fp);
   printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

   DEXIT;
   return;
FCLOSE_ERROR:
   DEXIT;
   return;
}
   
/*-------------------------------------------------------------------------*/
static void showshare(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget) cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;

   DENTER(GUI_LAYER, "showshare");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];
   
   printf("Share of node %s: %f\n", item->text, 
               calculate_share(item));  

   DEXIT;
}
   
/*-------------------------------------------------------------------------*/
static void showsimpleshare(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget) cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;

   DENTER(GUI_LAYER, "showshare");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];
   
   printf("Share of node %s: %f\n", item->text, 
               calculate_simple_share(item));  

   DEXIT;
}
   
/*-------------------------------------------------------------------------*/
static void showusage(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget) cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;

   DENTER(GUI_LAYER, "showshare");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];
   
   printf("Share of node %s: %f\n", item->text, 
               calculate_usage(item));  

   DEXIT;
}
   
#endif
   


/*-------------------------------------------------------------------------*/
static Boolean AskForNode(
Widget w,
String name,
int name_len,
Cardinal *share 
) {
   static tAskNodeInfo AskNodeInfo;      
   Widget shell = XmtGetShell(w);
   Widget node_layout, node_cancel, node_ok;
   static Widget node_name, node_share;
   String name_str = NULL;
   String share_str = NULL;
   String rest = NULL;
   char buf[256];

   DENTER(GUI_LAYER, "AskForNode");

   /* 
   ** make sure the shell we pop up over is not a menu shell! 
   */
   while(XtIsOverrideShell(shell)) 
      shell = XmtGetShell(XtParent(shell));
 
   /*
   ** create the node popup dialog
   */
   if (!AskNodeInfo.AskNodeDialog) {
      AskNodeInfo.AskNodeDialog = XmtBuildQueryDialog( shell, "qmon_node",
                           node_resources, XtNumber(node_resources),
                           "node_cancel", &node_cancel,
                           "node_ok", &node_ok,
                           "node_name", &node_name,
                           "node_share", &node_share,
                           NULL);
      AskNodeInfo.AskNodeName = node_name;
      AskNodeInfo.AskNodeShare = node_share;
      XtVaSetValues( AskNodeInfo.AskNodeDialog,
                     XmNdefaultButton, node_ok,
                     NULL);
      XtAddCallback(node_ok, XmNactivateCallback, 
                  AskNodeOkCallback, (XtPointer)&AskNodeInfo);
      XtAddCallback(node_cancel, XmNactivateCallback, 
                  AskNodeCancelCallback, (XtPointer)&AskNodeInfo);
      
      XmtAddDeleteCallback(XtParent(AskNodeInfo.AskNodeDialog), XmDO_NOTHING,
                      AskNodeCancelCallback, (XtPointer)&AskNodeInfo);


   }
   node_layout = AskNodeInfo.AskNodeDialog;
   node_name = AskNodeInfo.AskNodeName;
   node_share = AskNodeInfo.AskNodeShare;

   if (name && !strcmp(name, "default")) {
      XtSetSensitive(node_name, False);
      XmtSetInitialFocus(node_layout, node_share);
   } else {   
      XtSetSensitive(node_name, True); 
      XmtSetInitialFocus(node_layout, node_name);
   }   
   
   /* 
   ** Tell the dialog who it is transient for 
   */
   XtVaSetValues( XtParent(node_layout), 
                  XtNtransientFor, shell, 
                  NULL);
   /*
   ** preset with default values
   */
   sprintf(buf, sge_u32, (u_long32) *share);
   XmtInputFieldSetString(node_name, name);
   XmtInputFieldSetString(node_share, buf);

   /* 
   ** position, set initial focus, and pop up the dialog 
   */
   XmtDialogPosition(XtParent(node_layout), shell);
   XtManageChild(node_layout);

   /*
   ** Enter a recursive event loop.
   ** The callback registered on the okay and cancel buttons when
   ** this dialog was created will cause info->button to change
   ** when one of those buttons is pressed.
   */
   AskNodeInfo.blocked = True;
   XmtBlock(shell, &AskNodeInfo.blocked);

   /* pop down the dialog */
   XtUnmanageChild(node_layout);

   /* make sure what is underneath gets cleaned up
   ** (the calling routine might act on the user's returned
   ** input and not handle events for awhile.)
   */
   XSync(XtDisplay(node_layout), 0);
   XmUpdateDisplay(node_layout);

   /*
   ** if the user clicked Ok, return the strings
   */
   if (AskNodeInfo.button == XmtOkButton) {
      name_str = XmtInputFieldGetString(node_name);
      share_str = XmtInputFieldGetString(node_share);
      if (!name_str || name_str[0] == '\0') {
         DEXIT;
         return False;
      }
      strncpy(name, name_str, name_len-1);
      name[name_len-1] = '\0';
      if (share_str && share_str != '\0') {
         long l = strtol(qmon_trim(share_str), &rest, 10);
         if (l < 0 || (rest && *rest != '\0')) {
            qmonMessageShow(w, True, "@{Only unsigned integers are allowed !}");
            DEXIT;
            return False;
         } else {
            *share = (Cardinal)l;
         }   
      } else
         *share = 0;
      DEXIT;
      return True;
   }
   else {
      DEXIT;
      return False;
   }
}

   
/*-------------------------------------------------------------------------*/
static void AskNodeOkCallback(Widget w, XtPointer cld, XtPointer cad)
{
    tAskNodeInfo *info = (tAskNodeInfo *)cld;
    info->blocked = False;
    info->button = XmtOkButton;
}

/*-------------------------------------------------------------------------*/
static void AskNodeCancelCallback(Widget w, XtPointer cld, XtPointer cad)
{
    tAskNodeInfo *info = (tAskNodeInfo *)cld;
    info->blocked = False;
    info->button = XmtCancelButton;
}

   
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
   
#if 0
/*-------------------------------------------------------------------------*/
static String node_actual_resource_share(
XtPointer data 
) {
   tSTNUserData *ud = (tSTNUserData *)data;
   static char buf[10];

   DENTER(GUI_LAYER, "node_actual_resource_share");

   if (ud) {
      sprintf(buf, "%3.1f %%", 100*ud->actual_proportion); 
      buf[9] = '\0';    /* just in case !?! */
   }
   else
      strcpy(buf, "-");

   DEXIT;
   return buf;
}

/*-------------------------------------------------------------------------*/
static String node_targetted_resource_share(
XtPointer data 
) {
   tSTNUserData *ud = (tSTNUserData *)data;
   static char buf[10];

   DENTER(GUI_LAYER, "node_targetted_resource_share");
   
   if (ud) { 
      sprintf(buf, "%3.1f %%", 100*ud->targetted_share);
   }
   else
      strcpy(buf, "-");

   DEXIT;
   return buf;
}

/*-------------------------------------------------------------------------*/
static void qmonShareTreeSetName(Widget w, XtPointer cld, XtPointer cad)
{
   String name = (String)cad;
   Widget tree = (Widget)cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;

   DENTER(GUI_LAYER, "qmonShareTreeSetName");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];

   if (item) {
      ListTreeRenameItem(tree, item, name);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void set_share(
XtPointer user_data,
XtPointer share 
) {
   tSTNUserData *ud = (tSTNUserData *)user_data;
   int value = (int) share;

   DENTER(GUI_LAYER, "set_share");

   ud->share = value;

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonShareTreeSetShare(Widget w, XtPointer cld, XtPointer cad)
{
   String share_str = (String)cad;
   Widget tree = (Widget)cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;
   int share;

   DENTER(GUI_LAYER, "qmonShareTreeSetShare");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];

   if (item) {
      if (share_str && *share_str != '\0')
         share = atoi(share_str);
      else
         share = 0;
      set_share(item->user_data, (XtPointer) share);
   }

   /*
   ** recalculate the perc values
   */
   qmonShareTreeSetValues(item);

   DEXIT;
}

#endif
