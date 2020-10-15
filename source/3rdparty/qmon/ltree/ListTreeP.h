/*-----------------------------------------------------------------------------
 * ListTree	A list widget that displays a file manager style tree
 *
 * Copyright (c) 1996 Robert W. McMullen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Author: Rob McMullen <rwmcm@mail.ae.utexas.edu>
 *         http://www.ae.utexas.edu/~rwmcm
 */

#ifndef _ListTreeP_H
#define _ListTreeP_H

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/DrawP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>

#include "ListTree.h"

/* LessTif definitions that are convenient for Motif */
#ifndef Prim_HighlightThickness
#define Prim_HighlightThickness(w) \
    (((XmPrimitiveWidget)(w))->primitive.highlight_thickness)
#endif

#ifndef Prim_ShadowThickness
#define Prim_ShadowThickness(w) \
    (((XmPrimitiveWidget)(w))->primitive.shadow_thickness)
#endif

#ifndef Prim_TopShadowGC
#define Prim_TopShadowGC(w) \
    (((XmPrimitiveWidget)(w))->primitive.top_shadow_GC)
#endif

#ifndef Prim_BottomShadowGC
#define Prim_BottomShadowGC(w) \
    (((XmPrimitiveWidget)(w))->primitive.bottom_shadow_GC)
#endif


#define ListTreeRET_ALLOC 10

#define TIMER_CLEAR 0
#define TIMER_SINGLE 1
#define TIMER_DOUBLE 2
#define TIMER_WAITING 3

/*
** DND support and debugging
*/
/* #define DEBUG */
/* #define DND */


typedef struct {
  int dummy;			/* keep compiler happy with dummy field */
} ListTreeClassPart;

typedef struct _ListTreeClassRec {
   CoreClassPart core_class;
   XmPrimitiveClassPart primitive_class;
   ListTreeClassPart ListTree_class;
} ListTreeClassRec;

extern ListTreeClassRec listtreeClassRec;

typedef struct {
   Pixmap bitmap;
   Pixmap pix;
   int width, height;
   int xoff;
}  Pixinfo;

typedef struct {
   ListTreeWidget list_tree;
   ListTreeItem *item;
} DNDInfo;

typedef struct {
   /* Public stuff ... */
   long foreground_pixel;
   XFontStruct *font;
   int NumItems;
   Dimension HSpacing;
   Dimension VSpacing;
   /*      Dimension       LabelSpacing; */
   Dimension Margin;
   Dimension Indent;
   Pixinfo Open;
   Pixinfo Closed;
   Pixinfo Leaf;
   Pixinfo LeafOpen;
   Dimension LineWidth;
   Boolean HighlightPath;
   Boolean ClickPixmapToOpen;
   Boolean DoIncrementalHighlightCallback;

   XtCallbackList HighlightCallback;
   XtCallbackList ActivateCallback;
   XtCallbackList MenuCallback;
   XtCallbackList DestroyItemCallback;
   XtCallbackList CreateItemCallback;

   Boolean ListMode;

   /* Private stuff ... */
   GC drawGC;
   GC eraseGC;
   GC eorGC;
   GC highlightGC;
   Pixinfo ItemPix;              /* temporary storage for GetItemPix */
   int exposeTop, exposeBot;
   int pixWidth;
   int preferredWidth, preferredHeight;
   ListTreeItem *first, /* always points to a top level entry */
                *highlighted, *drop_highlighted;

   XtIntervalId timer_id;     /* timer for double click test */
   ListTreeItem *timer_item;  /* item to make sure both clicks */
                              /* occurred on the same item */
   int timer_type;            /* flag for type of click that just happened */
   int timer_y;
   int timer_x;
   int multi_click_time;

   ListTreeItem **ret_item_list;
   int ret_item_alloc;

   Boolean Refresh;
   Boolean HasFocus;

   /* New stuff for maintaining its own scrolling state */
   Widget mom;       /* scrolled window */
   Widget hsb;       /* horizontal scrollbar */
   Widget vsb;       /* vertical scrollbar */
   Dimension viewX;
   Dimension viewY;
   Dimension viewWidth;
   Dimension viewHeight;
   int XOffset;
   int hsbPos;
   int hsbMax;

   int lastXOffset;
   int topItemPos;               /* position of topItem in itemCount */
   int bottomItemPos;            /* last position drawn in window */
   int lastItemPos;              /* last value of topItempos */
   ListTreeItem *topItem;        /* first visible item on screen */
   int itemCount;                /* total number of open ListTreeItems */
   Dimension itemHeight;
   Dimension maxPixHeight;
   int visibleCount;             /* number currently visible on screen */
   Boolean recount;

} ListTreePart;

typedef struct _ListTreeRec {
   CorePart core;
   XmPrimitivePart primitive;
   ListTreePart list;
} ListTreeRec;


#endif /* _ListTreeP_H */
