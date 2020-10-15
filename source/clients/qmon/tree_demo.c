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

#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include <Xmt/Xmt.h>
#include <Xmt/Xpm.h>
#include <Xmt/Pixmap.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "ListTree.h"
#include "sgeobj/sge_all_listsL.h"

/* #define NO_CULL */

#if 0
static char * folder_xpm[] = {
"32 32 6 1",
/* colors */
"    s None   c None",
".   c black",
"X   c #FEFE00",
"o   c #FFCC9A",
"O   c #FE0000",
"+   c #00FE00",
/* pixels */
"       .     ..                 ",
"        .    .X.                ",
"         .   .XX.    ..         ",
"       .  .X..XXX.  .X.         ",
"        . .XXXXXXX..XX.         ",
"         .XXXXXXXXXXXX.   .     ",
"       .XXXXXXXXXXXXXX.  .      ",
"      .oXXXXXXXXXXXoXX. .   .   ",
"     .oooXXXXXXXooooXX..X  .    ",
"     .ooooooooooooooXXXXXX.     ",
"     .oooooooooooooooXXXXXX...  ",
"     .ooo.ooooo.ooooooXXXXX     ",
"     .ooo.ooooo.oooooooo.XX...  ",
"     .oooo...oooooooooooo.X     ",
"    .oooo.oooooooooooooooo      ",
"    .oooo.oooooooooooooooo...   ",
"    .ooooo..ooooooooooooooooo.  ",
"    .oooooooooooooooooooooooo.  ",
"    .ooooooooooooooooooooooo.   ",
"     .ooooooooooooooooooooo.    ",
"      .oooooooooooooooooo..     ",
"       .oooooOOOOOoooooo.       ",
"        .oooooooooooooo.        ",
"         ..ooooooooooo.         ",
"           ..ooooooo..          ",
"          .++.......++..        ",
"         ...++++++++++...       ",
"        ...+..........+...      ",
"       .+.++++++++++++++.+.     ",
"      .+.++............++.+.    ",
"       .+.++++++++++++++.+.     ",
"        ...+..........+.+.      "};
static char * folderopen_xpm[] = {
"32 32 6 1",
/* colors */
"    s None   c None",
".   c black",
"X   c #FEFE00",
"o   c #FFCC9A",
"O   c #FE0000",
"+   c #00FE00",
/* pixels */
"       .     ..                 ",
"        .    .X.                ",
"         .   .XX.    ..         ",
"       .  .X..XXX.  .X.         ",
"        . .XXXXXXX..XX.         ",
"         .XXXXXXXXXXXX.   .     ",
"       .XXXXXXXXXXXXXX.  .      ",
"      .oXXXXXXXXXXXoXX. .   .   ",
"     .oooXXXXXXXooooXX..X  .    ",
"     .ooooooooooooooXXXXXX.     ",
"     .oooooooooooooooXXXXXX...  ",
"     .ooo.ooooo.ooooooXXXXX     ",
"     .ooo.ooooo.oooooooo.XX...  ",
"     .oooo...oooooooooooo.X     ",
"    .oooo.oooooooooooooooo      ",
"    .oooo.oooooooooooooooo...   ",
"    .ooooo..ooooooooooooooooo.  ",
"    .oooooooooooooooooooooooo.  ",
"    .oo.................oooo.   ",
"     .oo...............oooo.    ",
"      .oo....OO.O.....ooo..     ",
"       .oo...OOOOO...ooo.       ",
"        .ooo...OO..oooo.        ",
"         ..ooo...ooooo.         ",
"           ..ooooooo..          ",
"          .++.......++..        ",
"         ...++++++++++...       ",
"        ...+..........+...      ",
"       .+.++++++++++++++.+.     ",
"      .+.++............++.+.    ",
"       .+.++++++++++++++.+.     ",
"        ...+..........+.+.      "};
static char * fileopen_xpm[] = {
"32 32 4 1",
/* colors */
"` c black",
"a c tomato",
"b c white",
"c c gold",
/* pixels */
"bbbbbbbbbbbbbbbb```````bbbbbbbbb",
"bbbbbbbbbbbb````cc`cc`bbbbbbbbbb",
"bbbbbbbbbbb`ccc`c`cc`bbbbbbbbbbb",
"bbbbbbbb````c`cccccc`bbbbbbbbbbb",
"bbbbbbb`ccc``ccccccc`bbbbbbbbbbb",
"bbbbb`````ccccccccccc`bbbbbbbbbb",
"bbbb`cccccccccccccccc`bbbbbbbbbb",
"bbb`cc`ccccccccccccccc`bbbbbbbbb",
"bbb```cccccccccccccccc``bbbbbbbb",
"bbbbbb`ccccccccccccccccc`bbbbbbb",
"bbbbbb`ccccccccccccccccc```bbbbb",
"bbbbbbb`ccccccccccccccc`bbb`bbbb",
"bbbbbbb`ccccccccc````c`bbbbb`bbb",
"bbbbbbbb`ccccccc`bbbb``bb`bb`bbb",
"bbbbbbbbb`ccccc`bbbbbbb`bbbb`bbb",
"bbbbbbbbb`ccccc`bb`bbbb`bbbb`bbb",
"bbbbbbbbbb`cccc`bbbbbbb`````bbbb",
"bbbbbbbbbb`cccc`bbbbbb`cccc`bbbb",
"bbbbbbbbbbb`cccc`bbbb`ccccc`bbbb",
"bbbbbbbbbbbb`cccc````ccc```bbbbb",
"bbbbbbbbbbbb``ccccccccccccc`bbbb",
"bbbbbbbbbbb`cccccccccccccccc`bbb",
"bbbbbbbbbbb`c`ccccccccccccccc`bb",
"bbbbbbbbbbbb`cc`ccccccccccccc`bb",
"bbbbbbbbbbbb`c`cccccccccccc``bbb",
"bbbbbbbbbbbb`ccc```````````bbbbb",
"bbbbbbbbbbbb`cc`b`bb`bb`bbbbbbbb",
"bbbbbbbbbbb`cccc`bb`bb`bbbbbbbbb",
"bbbbbbbbbb`b``ccc`````bbbbbbbbbb",
"bbbbbbbbb`bbbb`cccccc`bbbbbbbbbb",
"bbbbbbbb`bbbbbb`aaaca`aaabbbbbbb",
"bbbbbbb``````````````````bbbbbbb"
};
static char * fileclosed_xpm[] = {
"32 32 6 1",
/* colors */
" 	c white",
".	c black",
"X	c gold",
"o	c red",
"O	c darkslategrey",
"+	c magenta",
/* pixels */
"             . . .              ",
"             ..... .            ",
"             .XXXX.. .          ",
"             .XXXXXX.. .        ",
"            .XXXXXXXXX.. .      ",
"            .XXXXXXXXXXX.. .    ",
"           .XXXXXXXXXXXXXX..    ",
"           .XXXXXXXXXXXXXXX...  ",
"          .XXXXXXXXXXXXXXXXX.   ",
"      .....XXXXXXXXXXXXXXXX.    ",
"      o.OOoXXXXXXXXXXXXXXX.     ",
"      .O OOoXXXXXXXXXXXXXX.     ",
"      oOOOO.oo.oooXXXXXXX.      ",
"      ..OO.oo.O..ooXXXXX.       ",
"       .oooX.O OOooXXXXX.       ",
"      .XXXXX.OOOO.ooXXX.        ",
"      .XXXXXo.OO.oXooX.         ",
"       ...XXXo..oXXXoo.         ",
"       .XXXXXXXXXXXXX.o.        ",
"      .XXXXXXXXXXXXXX..o.       ",
"      .XXXXXXXXXXXX.XX. o.      ",
"       ..........XXX..  o.      ",
"           .  .  .XX.   o.      ",
"          ..  .  .XX.           ",
"         .X......XXX.           ",
"         .XXXXXXXXXX.           ",
"          ...XXXXXXX.           ",
"            .XXXXXX.+.          ",
"            .XXXXX.+++.         ",
"           .+.....++.++.        ",
"          .++++++++.+++.        ",
"          ...............       "};

#endif

static char * fileclosed_xpm[] = {
"16 16 3 1",
"  c None s None",
". c #808080",
"X c red",
"                ",
"     ..XXX.     ",
"    .XXXXXXX    ",
"   .XX.    .X   ",
"   XX.          ",
"  XXX       X   ",
"XXXXXXX    XXX  ",
" XXXXX    XXXXX ",
"  XXX    XXXXXXX",
"   X       XXX  ",
"          .XX   ",
"   X.    .XX.   ",
"    XXXXXXX.    ",
"     .XXX..     ",
"                ",
"                "};

static char * fileopen_xpm[] = {
"16 16 3 1",
"  c None s None",
". c #808080",
"X c blue",
"                ",
"     ..XXX.     ",
"    .XXXXXXX    ",
"   .XX.    .X   ",
"   XX.          ",
"  XXX       X   ",
"XXXXXXX    XXX  ",
" XXXXX    XXXXX ",
"  XXX    XXXXXXX",
"   X       XXX  ",
"          .XX   ",
"   X.    .XX.   ",
"    XXXXXXX.    ",
"     .XXX..     ",
"                ",
"                "};
 
static char * folderopen_xpm[] = {
"16 16 6 1",
"  c None s None",
". c #808080",
"X c #c0c0c0",
"o c #ffff00",
"O c black",
"# c white",
"                ",
"  .....         ",
" .XoXoX.        ",
".XoXoXoX......  ",
".############.O ",
".#oXoXoXoXoXo.O ",
".#XoXoXoXoXoX.O ",
".#oXoXoXoXoXo.O ",
".#XoXoXoXoXoX.O ",
".#oXoXoXoXoXo.O ",
".#XoXoXoXoXoX.O ",
".#oXoXoXoXoXo.O ",
"..............O ",
" OOOOOOOOOOOOOO ",
"                ",
"                "};

static char * folder_xpm[] = {
"16 16 6 1",
"  c None s None",
". c #808080",
"X c #c0c0c0",
"o c #ffff00",
"O c cyan4",
"# c white",
"                ",
"  .....         ",
" .XoXoX.        ",
".XoXoXoX......  ",
".############.O ",
".#oXoXoXoXoXo.O ",
".#XoXoXoXoXoX.O ",
".#oXoXoXoXoXo.O ",
".#XoXoXoXoXoX.O ",
".#oXoXoXoXoXo.O ",
".#XoXoXoXoXoX.O ",
".#oXoXoXoXoXo.O ",
"..............O ",
" OOOOOOOOOOOOOO ",
"                ",
"                "};

typedef struct _tShareTreeData {
   int share;
   int usage;
} tShareTreeData;



/*-------------------------------------------------------------------------*/
static void HighlightCallback(w,client,call)
{
   ListTreeMultiReturnStruct *ret;
   ListTreeItem *item;
   int i;

   DENTER(GUI_LAYER, "HighlightCallback");

   if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
      ret=(ListTreeMultiReturnStruct *)call;
      printf("HIGHLIGHT: count=%d\n",ret->count);
      for (i=0; i<ret->count; i++) {
        item=ret->items[i];
        printf("%s",item->text);
        while (item->parent) {
          item=item->parent;
          printf("<--%s",item->text);
        }
        printf("\n");
      }
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void MenuCallback(w,client,call)
{
   ListTreeItemReturnStruct *ret = (ListTreeItemReturnStruct *)call;

   DENTER(GUI_LAYER, "MenuCallback");

   if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
      printf ("MENU: item=%s\n", ret->item->text);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void ActivateCallback(w,client,call)
{
   ListTreeActivateStruct *ret;
   int count;

   DENTER(GUI_LAYER, "ActivateCallback");

   if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
      ret=(ListTreeActivateStruct *)call;
      printf("ACTIVATE: item=%s count=%d\n",ret->item->text,ret->count);
      count=0;
      while (count<ret->count) {
         printf(" path: %s\n",ret->path[count]->text);
         count++;
      }
   }
   DEXIT;
}


#ifdef NO_CULL

/*-------------------------------------------------------------------------*/
static void Init(
Widget tree 
) {
   int i,j,k,l;
   char test[64];
   ListTreeItem *level1,*level2,*level3,*level4;

   DENTER(GUI_LAYER, "Init");

   for (i=0; i<10; i++) {
      sprintf(test,"%c Level #1, entry #%d", (char)('A'+i),i);
      level1=ListTreeAdd(tree, NULL, test);
      for (j=0; j<i; j++) {
         sprintf(test,"%c Level #2, entry #%d", (char)('A'+j+i),j);
         level2=ListTreeAdd(tree, level1, test);
         for (k=0; k<i; k++) {
            sprintf(test,"%c Level #3, entry #%d", (char)('A'+k+j+i),k);
            level3=ListTreeAdd(tree, level2, test);
            for (l=0; l<i; l++) {
               sprintf(test,"%c Level #4, entry #%d", (char)('A'+l+k+j+i),l);
               level4=ListTreeAdd(tree, level3, test);
            }
         }
      }
   }
   DEXIT;
}

#else

static lList *buildShac(lListElem *parent, int nodes, int depth)
{
   lList *shac;
   lListElem *ep;
   char buf[BUFSIZ];
   int i;

   DENTER(GUI_LAYER, "buildShac");

   shac = lCreateElemList("ShareTree", STN_Type, nodes);
   
   for (i=0, ep = lFirst(shac); i<nodes; i++, ep = lNext(ep)) {
      if (parent)
         sprintf(buf, "%s.%d", lGetString(parent, STN_name), i);
      else
         sprintf(buf, "Node %d", i);
      lSetString(ep, STN_name, buf);
      lSetUlong(ep, STN_type, STT_USER);
      lSetUlong(ep, STN_shares, i+1);
      if (depth > 0) {
         lSetList(ep, STN_children, buildShac(ep, 20, depth-1));
      }
   }   

   DEXIT;
   return shac;
}
      

/*-------------------------------------------------------------------------*/
static void CullToTree(
Widget tree,
ListTreeItem **parent,
lList *shac 
) {
   lListElem *ep = NULL;
   ListTreeItem *node = NULL;
   String name = NULL;
   lList *sublist = NULL;
   tShareTreeData *user_data;
   char buf[1024];
   

   DENTER(GUI_LAYER, "CullToTree");

   if (!parent) {
      DEXIT;
      return;
   }
   
   /*
   ** add the root node
   */
/*    if (!*parent) { */
/*       *parent = ListTreeAdd(tree, NULL, "+"); */
/*    } */

   for_each(ep, shac) {
      user_data = (tShareTreeData*)XtMalloc(sizeof(tShareTreeData));
      user_data->share = (int)lGetUlong(ep, STN_shares);
      user_data->usage = user_data->share;
      name = lGetString(ep, STN_name);
      sublist = lGetList(ep, STN_children);
      sprintf(buf, "Share %d: %s", user_data->share, name ? name : "-NoName-"); 
      node = ListTreeAdd(tree, *parent, buf); 
      node->user_data = (XtPointer)user_data; 
      if (sublist)
         CullToTree(tree, &node, sublist);
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static lList* TreeToCull(
Widget tree,
ListTreeItem *item 
) {
   lList *lp = NULL;
   lListElem *ep = NULL;
   ListTreeItem *node = NULL;
   
   DENTER(GUI_LAYER, "TreeToCull");

   if (!item) 
      item = ListTreeFirstItem(tree);

   node = ListTreeFirstChild(item);

   while (node) {
      ep = lAddElemStr(&lp, STN_name, node->text, STN_Type);

      if (ListTreeFirstChild(node))
         lSetList(ep, STN_children, TreeToCull(tree, node));
      
      node = ListTreeNextSibling(node);
   }

   DEXIT;
   return lp;
}

/*-------------------------------------------------------------------------*/
static double calculate_share(
Widget tree,
ListTreeItem *item 
) {
   ListTreeItem *parent;
   ListTreeItem *sibling;
   double level_sum = 0.0;
   double share;
   tShareTreeData *user_data;
   
   DENTER(GUI_LAYER, "calculate_share");

   if (!item) {
      DEXIT;
      return 1.0;
   }


   parent = ListTreeParent(item);

   /*
   ** get parent share
   */
   share = calculate_share(tree, parent);

   if (parent) {
      sibling = ListTreeFirstChild(parent);
      while (sibling) {
         /*
         ** sum the shares
         */
         user_data = (tShareTreeData *)sibling->user_data;
         level_sum += user_data->share;
         
         sibling = ListTreeNextSibling(sibling);
      }

      user_data = (tShareTreeData *)item->user_data;
/* printf("item: %s\n", item->text); */
/* printf("user_data->share = %d user_data-usage = %d\n", user_data->share,  */
/*                user_data->usage); */
      share *= (double)user_data->share/level_sum;
   }

/* printf("Share = %f\n", share); */
   DEXIT;

   return share;
}
   
/*-------------------------------------------------------------------------*/
static double calculate_simple_share(
Widget tree,
ListTreeItem *item 
) {
   ListTreeItem *parent;
   ListTreeItem *sibling;
   double level_sum = 0.0;
   double share = 1.0;
   tShareTreeData *user_data;
   
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
         user_data = (tShareTreeData *)sibling->user_data;
         level_sum += user_data->share;
         
         sibling = ListTreeNextSibling(sibling);
      }

      user_data = (tShareTreeData *)item->user_data;
/* printf("item: %s\n", item->text); */
/* printf("user_data->share = %d user_data-usage = %d\n", user_data->share,  */
/*                user_data->usage); */
      share = (double)user_data->share/level_sum;
   }

/* printf("Share = %f\n", share); */
   DEXIT;

   return share;
}

/*-------------------------------------------------------------------------*/
static double calculate_usage(
Widget tree,
ListTreeItem *item 
) {
   ListTreeItem *child;
   double level_sum = 0.0;
   tShareTreeData *user_data;
   
   DENTER(GUI_LAYER, "calculate_usage");

   if (item) {
      child = ListTreeFirstChild(item);
      if (!child) {
         user_data = (tShareTreeData *)item->user_data;
         level_sum += user_data->usage; 
      }
      else {
         while (child) {
            level_sum += calculate_usage(tree, child);
            child = ListTreeNextSibling(child);
         }
      }
   }

   DEXIT;

   return (double)level_sum;
}
#endif

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
               calculate_share(tree, item));  

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
               calculate_simple_share(tree, item));  

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
               calculate_usage(tree, item));  

   DEXIT;
}
   
/*-------------------------------------------------------------------------*/
static void showpath(Widget w, XtPointer cld, XtPointer cad)
{
   Widget tree = (Widget) cld;
   ListTreeItem *item = NULL;
   ListTreeMultiReturnStruct ret;

   DENTER(GUI_LAYER, "showshare");

   ListTreeGetHighlighted(tree, &ret);
   if (ret.count == 1)
      item = ret.items[0];
   
   printf("Node %s\n", item->text);
   ListTreeOpenLikeTree(tree, item, NULL);

   DEXIT;
}
   
   

/*-------------------------------------------------------------------------*/
void main(
int argc,
char **argv 
) {
   Widget   toplevel, tree, rowcol, pb;
   XtAppContext app_con;
   lList *share_tree = NULL;

   Pixmap open, closed, file_open, file_closed; 
   Arg args[20];
   int ac;

#ifndef NO_CULL
   ListTreeItem *root_node = NULL;
#endif

   DENTER_MAIN(GUI_LAYER, "demo_main");

   lInit(nmv);       /* nmv is the lNameSpace array address */

   toplevel = XtAppInitialize(&app_con,"ListTreeDemo",NULL,0,
            &argc,argv,NULL,NULL,0);

   rowcol = XtVaCreateManagedWidget("RowCol",
                                    xmRowColumnWidgetClass,
                                    toplevel,
                                    XmNorientation, XmVERTICAL,
                                    NULL);


   XmtRegisterImage("folderopen", XmtParseXpmData(folderopen_xpm));
   XmtRegisterImage("folder", XmtParseXpmData(folder_xpm));
   XmtRegisterImage("fileopen", XmtParseXpmData(fileopen_xpm));
   XmtRegisterImage("fileclosed", XmtParseXpmData(fileclosed_xpm));

   open = XmtLookupWidgetPixmap(toplevel, "folderopen");
   closed = XmtLookupWidgetPixmap(toplevel, "folder");
   file_open = XmtLookupWidgetPixmap(toplevel, "fileopen");
   file_closed = XmtLookupWidgetPixmap(toplevel, "fileclosed");


   ac = 0;
   XtSetArg(args[ac], XtNbranchPixmap, closed); ac++; 
   XtSetArg(args[ac], XtNbranchOpenPixmap, open); ac++; 
   XtSetArg(args[ac], XtNleafPixmap, file_closed); ac++; 
   XtSetArg(args[ac], XtNleafOpenPixmap, file_open); ac++; 
   XtSetArg(args[ac], XtNindent, 5); ac++; 
   tree=XmCreateScrolledListTree(rowcol, "tree", args, ac);
   XtVaSetValues( tree,
                  XtNheight, 300,
                  XtNwidth, 200,
                  XtNhorizontalSpacing, 5,
                  XtNverticalSpacing, 5,
                  XtNhighlightPath, False,
                  XtNdoIncrementalHighlightCallback, True,
                  NULL);
   XtManageChild(tree);
        
   pb = XtVaCreateManagedWidget("Dump Tree",
                                xmPushButtonWidgetClass,
                                rowcol,
                                NULL);
   XtAddCallback(pb, XmNactivateCallback, showpath, (XtPointer) tree);

   pb = XtVaCreateManagedWidget("Dump Share",
                                xmPushButtonWidgetClass,
                                rowcol,
                                NULL);
   XtAddCallback(pb, XmNactivateCallback, showshare, (XtPointer) tree);

   pb = XtVaCreateManagedWidget("Dump Simple Share",
                                xmPushButtonWidgetClass,
                                rowcol,
                                NULL);
   XtAddCallback(pb, XmNactivateCallback, showsimpleshare, (XtPointer) tree);

   pb = XtVaCreateManagedWidget("Dump Usage",
                                xmPushButtonWidgetClass,
                                rowcol,
                                NULL);
   XtAddCallback(pb, XmNactivateCallback, showusage, (XtPointer) tree);

#ifdef NO_CULL
   Init(tree);
#else
/*    share_tree = buildShac(NULL, 3, 5); */
   share_tree = buildShac(NULL, 10000, 1);
/*    lWriteListTo(share_tree, stdout); */
   CullToTree(tree, &root_node, share_tree);
   lFreeList(&share_tree);
#endif

   
   
   XtAddCallback(tree, XtNhighlightCallback, 
                     HighlightCallback, (XtPointer) NULL);
   XtAddCallback(tree, XtNactivateCallback, 
                     ActivateCallback, (XtPointer) NULL);
   XtAddCallback(tree, XtNmenuCallback, 
                     MenuCallback, (XtPointer) NULL);

   XtRealizeWidget(toplevel);

   XtAppMainLoop(app_con);
}
