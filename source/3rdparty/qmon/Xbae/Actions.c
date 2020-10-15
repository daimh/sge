/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
 * Copyright(c) 1995-99 Andrew Lister
 *
 *                        All rights reserved
 * Permission to use, copy, modify and distribute this material for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of Bellcore not be used in advertising
 * or publicity pertaining to this material without the specific,
 * prior written permission of an authorized representative of
 * Bellcore.
 *
 * BELLCORE MAKES NO REPRESENTATIONS AND EXTENDS NO WARRANTIES, EX-
 * PRESS OR IMPLIED, WITH RESPECT TO THE SOFTWARE, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR ANY PARTICULAR PURPOSE, AND THE WARRANTY AGAINST IN-
 * FRINGEMENT OF PATENTS OR OTHER INTELLECTUAL PROPERTY RIGHTS.  THE
 * SOFTWARE IS PROVIDED "AS IS", AND IN NO EVENT SHALL BELLCORE OR
 * ANY OF ITS AFFILIATES BE LIABLE FOR ANY DAMAGES, INCLUDING ANY
 * LOST PROFITS OR OTHER INCIDENTAL OR CONSEQUENTIAL DAMAGES RELAT-
 * ING TO THE SOFTWARE.
 *
 * $Id: Actions.c,v 1.1 2001-07-18 11:05:59 root Exp $
 */

/*
 * Actions.c created by Andrew Lister (7 August, 1995)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion > 1001
#include <Xm/DrawP.h>
#endif
#include <Xm/ScrollBar.h>
#include "MatrixP.h"
#include "Clip.h"
#include "Draw.h"
#include "Actions.h"
#include "Utils.h"
#include "ScrollMgr.h"
#include "ClipP.h"
#include <X11/cursorfont.h>

#ifndef XlibSpecificationRelease
#define XrmPermStringToQuark XrmStringToQuark
#endif

#if !defined(DRAW_RESIZE_LINE) && !defined(DRAW_RESIZE_SHADOW)
/* One of DRAW_RESIZE_LINE and DRAW_RESIZE_SHADOW must be defined. */
#define DRAW_RESIZE_SHADOW
#endif

#ifndef DEFAULT_SCROLL_SPEED
#define DEFAULT_SCROLL_SPEED 500
#endif

typedef struct {
    XbaeMatrixWidget mw;
    GC gc;
    int row;
    int column;
    int startx;
    int lastx;
    int currentx;
    int y, height;
    short *columnWidths;
    Boolean grabbed;
    Boolean haveVSB;
} XbaeMatrixResizeColumnStruct;

typedef struct {
    XbaeMatrixWidget mw;
    int row;
    int column;
    Boolean pressed;
    Boolean grabbed;
} XbaeMatrixButtonPressedStruct;

typedef struct {
    XbaeMatrixWidget mw;
    XbaeClipWidget cw;
    XEvent *event;
    XtIntervalId timerID;
    XtAppContext app_context;
    unsigned long interval;
    Boolean inClip;
    Boolean grabbed;
    Boolean above;
    Boolean below;
    Boolean left;
    Boolean right;
} XbaeMatrixScrollStruct;

static int DoubleClick P((XbaeMatrixWidget, XEvent *, int, int));
static void DrawSlideColumn P((XbaeMatrixWidget, int));
static void SlideColumn P((Widget, XtPointer, XEvent *, Boolean *));
static void PushButton P((Widget, XtPointer, XEvent *, Boolean *));
static void updateScroll P((XtPointer));
static void checkScrollValues P((Widget, XtPointer, XEvent *, Boolean *));
static void callSelectCellAction P((XbaeMatrixWidget, XEvent *));

static int last_row = 0;
static int last_column = 0;

static int last_selected_row = 0;
static int last_selected_column = 0;

static Boolean scrolling = False;

/* ARGSUSED */
void
xbaeDefaultActionACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    int x, y;
    int row, column;
    CellType cell;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w could be Matrix, or the Clip or textField children of Matrix
     */
    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)w;
    else if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "defaultActionACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to DefaultAction action",
            NULL, 0);
        return;
    }

    if (!mw->matrix.default_action_callback)
        return;

    if (!xbaeEventToXY(mw, event, &x, &y, &cell))
        return;

    if (!xbaeXYToRowCol(mw, &x, &y, &row, &column, cell))
        return;

    if (DoubleClick(mw, event, row, column))
    {
        XbaeMatrixDefaultActionCallbackStruct call_data;

        call_data.reason = XbaeDefaultActionReason;
        call_data.event = event;
        call_data.row = row;
        call_data.column = column;

        XtCallCallbackList((Widget)mw, mw->matrix.default_action_callback,
                           (XtPointer)&call_data);
      
    }
}

static void
DrawSlideColumn(mw, x)
XbaeMatrixWidget mw;
int x;
{
#ifdef DRAW_RESIZE_SHADOW
    /* These values derived through that age-old process
     * of what looks good to me */
#define SHADOW_WIDTH 2
#define RESIZE_COLUMN_LINE_WIDTH 4
    Dimension width = RESIZE_COLUMN_LINE_WIDTH;
    Dimension shadow_width = SHADOW_WIDTH;
#endif
    Dimension height;
    Window win;
    Display *display = XtDisplay(mw);
    int column = xbaeXtoCol(mw, x - COLUMN_LABEL_OFFSET(mw));
    int top, bottom;
    int adjusted_x;
    int y;
#ifdef DRAW_RESIZE_LINE
    GC gc = mw->matrix.draw_gc;
#endif
    Boolean need_vert_dead_space_fill = NEED_VERT_DEAD_SPACE_FILL(mw);
    unsigned int clip_reason;

    /*
     * If the column being resized is a fixed one then we don't need to
     * bother with the clip region
     */
    if (column < (int)mw->matrix.fixed_columns)
    {
        y = ROW_LABEL_OFFSET(mw);
        height = VISIBLE_HEIGHT(mw) + FIXED_ROW_HEIGHT(mw) +
            TRAILING_FIXED_ROW_HEIGHT(mw);
        win = XtWindow (mw);

        if (need_vert_dead_space_fill)
            height += VERT_DEAD_SPACE_HEIGHT(mw);

#ifdef DRAW_RESIZE_LINE
        XDrawLine(display, win, gc, x, y, x, y + height);
        if (XtIsManaged(LeftClip(mw)))
            XDrawLine(display, XtWindow(LeftClip(mw)), gc,
                      x - COLUMN_LABEL_OFFSET(mw), 0,
                      x - COLUMN_LABEL_OFFSET(mw),
                      LeftClip(mw)->core.height);
#endif
#ifdef DRAW_RESIZE_SHADOW
        DRAW_SHADOW(display, win,
                    mw->matrix.resize_top_shadow_gc,
                    mw->matrix.resize_bottom_shadow_gc,
                    shadow_width, x, y, width, height, XmSHADOW_OUT);
        if (XtIsManaged(LeftClip(mw)))
            DRAW_SHADOW(display, XtWindow(LeftClip(mw)),
                        mw->matrix.resize_top_shadow_gc,
                        mw->matrix.resize_bottom_shadow_gc,
                        shadow_width, x - COLUMN_LABEL_OFFSET(mw),
                        0, width, LeftClip(mw)->core.height, XmSHADOW_OUT);
#endif
        return;
    }

    /*
     * Similarly for trailingFixedColumns - beware going off the clip child
     * here also
     */
    if (column >= TRAILING_HORIZ_ORIGIN(mw) ||
        x >= (int)(ClipChild(mw)->core.x + ClipChild(mw)->core.width))
    {
        y = ROW_LABEL_OFFSET(mw);
        height = VISIBLE_HEIGHT(mw) + FIXED_ROW_HEIGHT(mw) +
            TRAILING_FIXED_ROW_HEIGHT(mw);
        win = XtWindow(mw);

        if (need_vert_dead_space_fill)
            height += VERT_DEAD_SPACE_HEIGHT(mw);

#ifdef DRAW_RESIZE_LINE
        XDrawLine(display, win, gc, x, y, x, y + height);
        if (XtIsManaged(RightClip(mw)))
            XDrawLine(display, XtWindow(RightClip(mw)),
                      gc, x - TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw), 0,
                      x - TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw),
                      RightClip(mw)->core.height);
        
#endif
#ifdef DRAW_RESIZE_SHADOW
        DRAW_SHADOW(display, win,
                    mw->matrix.resize_top_shadow_gc,
                    mw->matrix.resize_bottom_shadow_gc,
                    shadow_width, x, y, width, height, XmSHADOW_OUT);
        if (XtIsManaged(RightClip(mw)))
            DRAW_SHADOW(display, XtWindow(RightClip(mw)),
                        mw->matrix.resize_top_shadow_gc,
                        mw->matrix.resize_bottom_shadow_gc,
                        shadow_width,
                               x - TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw), 0,
                               width, RightClip(mw)->core.height, XmSHADOW_OUT);
#endif
        return;
    }
    
    xbaeGetVisibleRows(mw, &top, &bottom);
    /*
     * we need all non-fixed rows, so add 1 to bottom
     * to include the last one as the return values
     * are inclusive
     */
    bottom += 1;

    /*
     * The area between top and bottom rows are the non fixed rows.  They
     * fall on the ClipChild
     */
    y = -mw->matrix.cell_shadow_thickness; /* relative to clip */

    height = ROW_HEIGHT(mw) * (bottom - top) +
        2 * mw->matrix.cell_shadow_thickness;

    /*
     * If we are on the clip, the x location is offset by the
     * fixed column width, label offset and label width
     */
    adjusted_x = x - FIXED_COLUMN_LABEL_OFFSET(mw);

    win = XtWindow(ClipChild(mw));

#ifdef DRAW_RESIZE_LINE
    XDrawLine(display, win, gc, adjusted_x, y, adjusted_x, y + height);
#endif
#ifdef DRAW_RESIZE_SHADOW
    DRAW_SHADOW(display, win,
                mw->matrix.resize_top_shadow_gc,
                mw->matrix.resize_bottom_shadow_gc,
                shadow_width, adjusted_x, y, width, height, XmSHADOW_OUT);
#endif
    /*
     * Now draw the line (or shadow) on the non clipped region - that is
     * the fixed and trailingFixed rows.  First, do the leading rows.
     */
    if (mw->matrix.fixed_rows)
    {
        y = ROW_LABEL_OFFSET(mw);
        height = FIXED_ROW_HEIGHT(mw) + 2 * mw->matrix.cell_shadow_thickness;
        win = XtWindow(mw);
        xbaeSetClipMask(mw, CLIP_FIXED_ROWS);

#ifdef DRAW_RESIZE_LINE
        if (XtIsManaged(TopClip(mw)))
            XDrawLine(display, XtWindow(TopClip(mw)), gc, adjusted_x,
                      -mw->matrix.cell_shadow_thickness, adjusted_x, height);
#endif
#ifdef DRAW_RESIZE_SHADOW
        if (XtIsManaged(TopClip(mw)))
            DRAW_SHADOW(display, XtWindow(TopClip(mw)),
                        mw->matrix.resize_top_shadow_gc,
                        mw->matrix.resize_bottom_shadow_gc,
                        shadow_width, adjusted_x,
                        -mw->matrix.cell_shadow_thickness,
                        width, height, XmSHADOW_OUT);
#endif
        xbaeSetClipMask(mw, CLIP_NONE);                         
    }
    
    /*
     * The trailingFixedRows
     */
    if (mw->matrix.trailing_fixed_rows)
    {
        y = TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
        height = TRAILING_FIXED_ROW_HEIGHT(mw) +
            2 * mw->matrix.cell_shadow_thickness;

        clip_reason = CLIP_TRAILING_FIXED_ROWS;
        if (IS_LEADING_FIXED_COLUMN(mw, column))
            clip_reason |= CLIP_FIXED_COLUMNS;
        else if (IS_TRAILING_FIXED_COLUMN(mw, column))
            clip_reason |= CLIP_TRAILING_FIXED_COLUMNS;
        
        xbaeSetClipMask(mw, clip_reason);

#ifdef DRAW_RESIZE_LINE
        if (XtIsManaged(BottomClip(mw)))
            XDrawLine(display, XtWindow(BottomClip(mw)), gc, adjusted_x,
                      -mw->matrix.cell_shadow_thickness, adjusted_x, height);
#endif
#ifdef DRAW_RESIZE_SHADOW
        if (XtIsManaged(BottomClip(mw)))
            DRAW_SHADOW(display, XtWindow(BottomClip(mw)),
                        mw->matrix.resize_top_shadow_gc,
                        mw->matrix.resize_bottom_shadow_gc,
                        shadow_width, adjusted_x,
                        -mw->matrix.cell_shadow_thickness,
                        width, height, XmSHADOW_OUT);
#endif
        xbaeSetClipMask(mw, CLIP_NONE);
    }

    if ((NEED_VERT_FILL(mw) && (! HAS_ATTACHED_TRAILING_ROWS(mw))) ||
        need_vert_dead_space_fill)
    {
        if (need_vert_dead_space_fill)
        {
            y = UNATTACHED_TRAILING_ROWS_OFFSET(mw) -
                mw->matrix.cell_shadow_thickness;
            height = 2 * mw->matrix.cell_shadow_thickness +
                VERT_DEAD_SPACE_HEIGHT(mw);
        }
        else
        {
            y = TRAILING_FIXED_ROW_LABEL_OFFSET(mw) +
                TRAILING_FIXED_ROW_HEIGHT(mw);
            height = FILL_VERT_HEIGHT(mw) - HORIZ_SB_SPACE(mw);
        }

#ifdef DRAW_RESIZE_LINE
        XDrawLine(display, XtWindow(mw), gc,
                  adjusted_x, y, adjusted_x, height);
#endif
#ifdef DRAW_RESIZE_SHADOW
        DRAW_SHADOW(display, XtWindow(mw),
                    mw->matrix.resize_top_shadow_gc,
                    mw->matrix.resize_bottom_shadow_gc,
                    shadow_width, x, y,
                    width, height, XmSHADOW_OUT);
#endif
    }
}


static void
SlideColumn(w, data, event, cont)
Widget w;
XtPointer data;
XEvent *event;
Boolean *cont;
{
    XbaeMatrixResizeColumnStruct *rd = (XbaeMatrixResizeColumnStruct *)data;
    XMotionEvent *motionEvent;
    Boolean relayout = False;
    int numCharacters;
    int i;
    
    if (event->type == ButtonRelease)
    {
        DrawSlideColumn(rd->mw, rd->lastx);
        XUngrabPointer(XtDisplay(w), CurrentTime);
        rd->grabbed = False;
        /*
         * Remanage the VSB if we unmapped it earlier
         */
        if (rd->haveVSB)
            XtManageChild(VertScrollChild(rd->mw));

        if (rd->mw->matrix.resize_column_callback)
        {
            XbaeMatrixResizeColumnCallbackStruct call_data;

            call_data.reason = XbaeResizeColumnReason;
            call_data.event = event;
            call_data.row = rd->row;
            call_data.column = rd->column - 1;
            call_data.which = rd->column - 1;
            call_data.columns = rd->mw->matrix.columns;
            call_data.column_widths  = rd->columnWidths;
            XtCallCallbackList ((Widget)rd->mw,
                                rd->mw->matrix.resize_column_callback,
                                (XtPointer)&call_data);            
        }

        for (i = 0; i < rd->mw->matrix.columns; i++)
            if (rd->columnWidths[i] != rd->mw->matrix.column_widths[i])
            {
                /* Make sure everything is handled correctly with SetValues */
                XtVaSetValues((Widget)rd->mw, XmNcolumnWidths,
                               rd->columnWidths, NULL);
                break;
            }
        /*
         * If maxColumnLengths are set and we have resized the column to
         * larger, reset the corresponding maxColumnLength
         */
        if (rd->mw->matrix.column_max_lengths &&
            rd->columnWidths[rd->column - 1] >
            rd->mw->matrix.column_max_lengths[rd->column - 1])
            rd->mw->matrix.column_max_lengths[rd->column - 1] =
                rd->columnWidths[rd->column - 1];
        XtFree((char *)rd->columnWidths);
        return;
    }

    if (event->type != MotionNotify) /* Double check! */
        return;

    motionEvent = (XMotionEvent *)event;

    if (rd->currentx - motionEvent->x > FONT_WIDTH(rd->mw))
    {
        /* If we're only one character wide, we cannae get any smaller */
        if (rd->columnWidths[rd->column - 1] == BAD_WIDTH + 1)
            return;        
        /*
         * Moved left a full character - update the column widths and force
         * a redisplay
         */        
        numCharacters = (rd->currentx - motionEvent->x) /
            FONT_WIDTH(rd->mw);
        if (numCharacters >= rd->columnWidths[rd->column - 1])
            /* Must keep a column at least one character wide */
            numCharacters = rd->columnWidths[rd->column - 1] - 1;
                
        rd->columnWidths[rd->column - 1] -= numCharacters;
        rd->currentx -= numCharacters * FONT_WIDTH(rd->mw);
        relayout = True;
    }        
    
    if (motionEvent->x - rd->currentx > FONT_WIDTH(rd->mw))
    {
        /*
         * Moved right a full character - update the column widths and force
         * a redisplay
         */
        numCharacters = (motionEvent->x - rd->currentx) /
            FONT_WIDTH(rd->mw);
        rd->columnWidths[rd->column - 1] += numCharacters;
        rd->currentx += numCharacters * FONT_WIDTH(rd->mw);
        relayout = True;
    }

    if (relayout)
    {
        /* Draw the marker line in the new location */
        if (rd->lastx != rd->currentx)
        {
            DrawSlideColumn(rd->mw, rd->currentx);
            DrawSlideColumn(rd->mw, rd->lastx);

            rd->lastx = rd->currentx;
        }
    }
}

/* ARGSUSED */
void
xbaeResizeColumnsACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    int x, y;
    int eventx;
    int i;
    int row, column;
    CellType cell;
    static Cursor cursor;
    XbaeMatrixResizeColumnStruct resizeData;
    XGCValues values;
    XtAppContext appcontext;
#ifdef DRAW_RESIZE_LINE
    XGCValues save;
#endif
    unsigned long gcmask, event_mask;
    Display *display = XtDisplay(w);
#define FUZZ_FACTOR        3
    int        fuzzy = FUZZ_FACTOR;
#undef FUZZ_FACTOR
    
    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w could be Matrix, or the Clip or textField children of Matrix
     */
    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)w;
    else if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "resizeColumnsACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to ResizeColumns action",
            NULL, 0);
        return;
    }
    
    /*
     * If we won't allow dynamic column resize, leave.
     */
    if (!mw->matrix.allow_column_resize)
        return;
    
    if (!xbaeEventToXY(mw, event, &x, &y, &cell))
        return;
    
    eventx = x;
    
    if (!xbaeXYToRowCol(mw, &x, &y, &row, &column, cell))
        return;

    /*
     * Calculate if the x and y of the middle button event is on
     * a column border.  Allow the width of the shadow to be the
     * allowed delta.  x is modified in xbaeXYToRowCol() to be
     * the x distance from the cell's border
     */
    if ((int)mw->matrix.cell_shadow_thickness > fuzzy)
        fuzzy = mw->matrix.cell_shadow_thickness;
    
    if (x > fuzzy && COLUMN_WIDTH(mw, column) - x > fuzzy)
        return;
    
    /*
     * Looks like we hit a column border, determine the column that is
     * intended to be resized
     */
    if ((COLUMN_WIDTH(mw, column) - x) <= fuzzy)
        column++;
    
    /* Can't adjust the origin or should you be able to?? */
    if (column == 0)
        return;
    
    /*
     * Make it here and it's time to start the fun stuff!
     */
    
    /* Create the left / right cursor */
    if (!cursor)
        cursor = XCreateFontCursor(display, XC_sb_h_double_arrow);
    
    /* Commit any edit in progress and unmap the text field -
       it's just bad luck */
    (*((XbaeMatrixWidgetClass)XtClass(mw))->matrix_class.commit_edit)
        (mw, event, True);

    /*
     * Redraw the cell that had the text field in it or it might stay blank
     */
    xbaeDrawCell(mw, mw->matrix.current_row, mw->matrix.current_column);

    /*
     * Say goodbye to the Vertical ScrollBar -> it only gets in the way!
     */
    if ((resizeData.haveVSB = XtIsManaged(VertScrollChild(mw)) &&
           ((mw->matrix.scrollbar_placement == XmTOP_RIGHT) ||
            (mw->matrix.scrollbar_placement == XmBOTTOM_RIGHT))))
        XtUnmanageChild(VertScrollChild(mw));
    /*
     * Flush the commit events out to the server.  Otherwise, our changes
     * to the GCs below have a bad effect.
     */
    XSync(display, False);
    
    event_mask = PointerMotionMask | ButtonReleaseMask;
    XtAddEventHandler(w, event_mask,
                       True, (XtEventHandler)SlideColumn,
                       (XtPointer)&resizeData);
    
    XGrabPointer(display, XtWindow(w), True, event_mask,
                  GrabModeAsync, GrabModeAsync, XtWindow((Widget)mw),
                  cursor, CurrentTime);
    
    /* Copy the columnWidth array */
    resizeData.columnWidths =
         (short *)XtMalloc(mw->matrix.columns * sizeof(short));
    for (i = 0; i < mw->matrix.columns; i++)
        resizeData.columnWidths[i] = mw->matrix.column_widths[i];
    resizeData.grabbed = True;
    resizeData.mw = mw;
    resizeData.column = column;
    resizeData.startx = resizeData.currentx = resizeData.lastx =
        event->xbutton.x;

    gcmask = GCForeground | GCBackground | GCFunction;
    values.function = GXxor;
#ifdef DRAW_RESIZE_LINE
    XGetGCValues(display, mw->matrix.draw_gc, gcmask, &save);
    values.foreground = values.background = save.background;

    XChangeGC(display, mw->matrix.draw_gc, gcmask, &values);
#endif
    
    DrawSlideColumn(mw, resizeData.currentx);

    appcontext = XtWidgetToApplicationContext(w);
    
    while (resizeData.grabbed)
        XtAppProcessEvent(appcontext, XtIMAll);
    
    XtRemoveEventHandler(w, event_mask, True,
                           (XtEventHandler)SlideColumn,
                           (XtPointer)&resizeData);

#ifdef DRAW_RESIZE_LINE
    XSetFunction(display, mw->matrix.draw_gc, GXcopy);
#endif
}

/*
 * Action to process a drag out
 */
/* ARGSUSED */
void
xbaeProcessDragACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
#if XmVersion > 1001
    XbaeMatrixWidget mw;
    int x, y;
    int row, column;
    CellType cell;
    XbaeMatrixProcessDragCallbackStruct call_data;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w could be Matrix, or the Clip or textField children of Matrix
     */
    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)w;
    else if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "processDragACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to ProcessDrag action",
            NULL, 0);
        return;
    }

    if (!mw->matrix.process_drag_callback)
        return;

    if (!xbaeEventToXY(mw, event, &x, &y, &cell))
        return;

    if (!xbaeXYToRowCol(mw, &x, &y, &row, &column, cell))
        return;

    call_data.reason = XbaeProcessDragReason;
    call_data.event = event;
    call_data.row = row;
    call_data.column = column;

    if (mw->matrix.draw_cell_callback)
    {
        Pixel bgcolor, fgcolor;
        int width, height, depth;
        
        call_data.type = xbaeGetDrawCellValue(
            mw, row, column, &call_data.string, &call_data.pixmap,
            &call_data.mask, &width, &height, &bgcolor, &fgcolor, &depth);
    }
    else
        call_data.string = mw->matrix.cells ?
            mw->matrix.cells[row][column] : "";
    
    call_data.num_params = *nparams;
    call_data.params = params;

    XtCallCallbackList((Widget)mw, mw->matrix.process_drag_callback,
                       (XtPointer)&call_data);
#endif
}

/*
 * Action to edit a non-fixed cell.
 */
/* ARGSUSED */
void
xbaeEditCellACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    int row, column;
    XrmQuark q;
    static XrmQuark QPointer, QLeft, QRight, QUp, QDown;
    static Boolean haveQuarks = False;
    /*
     * Get static quarks for the parms we understand
     */
    if (!haveQuarks)
    {
        QPointer = XrmPermStringToQuark("Pointer");
        QLeft = XrmPermStringToQuark("Left");
        QRight = XrmPermStringToQuark("Right");
        QUp = XrmPermStringToQuark("Up");
        QDown = XrmPermStringToQuark("Down");
        haveQuarks = True;
    }

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w could be Matrix, or the Clip or textField children of Matrix
     */
    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)w;
    else if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "editCellACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to EditCell action",
            NULL, 0);
        return;
    }

    /*
     * Make sure we have a single parm
     */
    if (*nparams != 1)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "editCellACT", "badParms", "XbaeMatrix",
            "XbaeMatrix: Wrong params passed to EditCell action, needs 1",
            NULL, 0);
        return;
    }

    /*
     * Initialize row/column to the current position
     */
    row = mw->matrix.current_row;
    column = mw->matrix.current_column;

    /*
     * Quarkify the string param
     */
    q = XrmStringToQuark(params[0]);

    /*
     * If we aren't currently editing, then the only kind of traversal that
     * makes sense is pointer.
     */
    if (!XtIsManaged(TextChild(mw)) && q != QPointer)
        return;

    if (q == QPointer)
    {
        CellType cellType = NonFixedCell;
        int x, y;

        /*
         * Get the x,y point coordinate relative to the Clip window.
         * Return if this event did not occur in the Clip subwindow
         * (since we can only edit non-fixed cells).
         */
        switch(event->type)
        {
        case ButtonPress:
        case ButtonRelease:
            x = event->xbutton.x;
            y = event->xbutton.y;
            break;
        case KeyPress:
        case KeyRelease:
            x = event->xkey.x;
            y = event->xkey.y;
            break;
        case MotionNotify:
            x = event->xmotion.x;
            y = event->xmotion.y;
            break;
        default:
            return;
        }

        if (event->xbutton.subwindow == XtWindow(ClipChild(mw)))
        {
            x -= FIXED_COLUMN_LABEL_OFFSET(mw);
            y -= FIXED_ROW_LABEL_OFFSET(mw);
            cellType = NonFixedCell;
        }
        else if (event->xbutton.window != XtWindow(ClipChild(mw)))
        {
            if (!mw->matrix.traverse_fixed)
                return;
            cellType = FixedCell;
        }

        /*
         * Convert the point to a row,column. If it does not pick a valid
         * cell, then return.
         */
        if (!xbaeXYToRowCol(mw, &x, &y, &row, &column, cellType))
            return;
    }
    else if (q == QRight)
    {
        /*
         * If we are in the lower right corner, stay there.
         * Otherwise move over a column. If we move off to the right of the
         * final column to which traversing is allowed then move down a row
         * and back to the first column to which traversing is allowed.
         */
        if (!mw->matrix.traverse_fixed)
        {
                /* check scrollable boundary */
            if (mw->matrix.current_row != TRAILING_VERT_ORIGIN(mw) - 1 ||
                       mw->matrix.current_column != TRAILING_HORIZ_ORIGIN(mw) - 1)
            {
                column++;
                if (IS_TRAILING_FIXED_COLUMN(mw, column))
                {
                    column = mw->matrix.fixed_columns;
                    row++;
                }
            }
        }
        else
        {
            /* check matrix boundary */
            if (mw->matrix.current_row != mw->matrix.rows - 1 ||
                       mw->matrix.current_column != mw->matrix.columns - 1)
            {
                column++;
                if (column >= mw->matrix.columns)
                {
                    column = 0;
                    row++;
                }
            }
        }
    }
    else if (q == QLeft)
    {
        /*
         * If we are in the upper left corner, stay there.
         * Otherwise move back a column. If we move before the first column
         * to which traversing is allowed, move up a row and over to the last
         * column to which traversing is allowed.
         */
        if (!mw->matrix.traverse_fixed)
        {
                /* check scrollable boundary */
            if (mw->matrix.current_row != mw->matrix.fixed_rows ||
                mw->matrix.current_column != mw->matrix.fixed_columns)
            {
                column--;
                if (IS_LEADING_FIXED_COLUMN(mw, column))
                {
                    column = TRAILING_HORIZ_ORIGIN(mw) - 1;
                    row--;
                }
            }
        }
        else
        {
            if (mw->matrix.current_row != 0 || mw->matrix.current_column != 0)
            {
                column--;
                if (column < 0)
                {
                    column = mw->matrix.columns - 1;
                    row--;
                }
            }
        }
    }
    else if (q == QDown)
    {
        row++;

        /* adjust row for allowable traversable regions */
        if (!mw->matrix.traverse_fixed)
        {
            if (IS_TRAILING_FIXED_ROW(mw, row))
                row = mw->matrix.fixed_rows;
        }
        else
        {
            if (row >= mw->matrix.rows)
                row = 0;
        }
    }
    else if (q == QUp)
    {
        row--;

        if (!mw->matrix.traverse_fixed)
        {
            if (IS_LEADING_FIXED_ROW(mw, row))
                row = TRAILING_VERT_ORIGIN(mw) - 1;
        }
        else
        {
            if (row < 0)
                row = mw->matrix.rows - 1;
        }
    }

    /*
     * Call the traverseCellCallback to allow the application to
     * perform custom traversal.
     */
    if (mw->matrix.traverse_cell_callback)
    {
        XbaeMatrixTraverseCellCallbackStruct call_data;

        call_data.reason = XbaeTraverseCellReason;
        call_data.event = event;
        call_data.row = mw->matrix.current_row;
        call_data.column = mw->matrix.current_column;
        call_data.next_row = row;
        call_data.next_column = column;
        call_data.fixed_rows = mw->matrix.fixed_rows;
        call_data.fixed_columns = mw->matrix.fixed_columns;
        call_data.trailing_fixed_rows = mw->matrix.trailing_fixed_rows;
        call_data.trailing_fixed_columns = mw->matrix.trailing_fixed_columns;
        call_data.num_rows = mw->matrix.rows;
        call_data.num_columns = mw->matrix.columns;
        call_data.param = params[0];
        call_data.qparam = q;

        XtCallCallbackList((Widget)mw, mw->matrix.traverse_cell_callback,
                           (XtPointer)&call_data);

        row = call_data.next_row;
        column = call_data.next_column;
    }

    /*
     * Attempt to edit the new cell using the edit_cell method.
     * If we are editing a cell based on pointer position, we always
     * call edit_cell.        Otherwise, we must be editing a new cell to
     * call edit_cell.
     */
    if (q == QPointer || (row != mw->matrix.current_row ||
                          column != mw->matrix.current_column))
        (*((XbaeMatrixWidgetClass)XtClass(mw))->matrix_class.edit_cell)
            (mw, event, row, column, params, *nparams);

    /*
     * Traverse to the textField
     */
    (void)XmProcessTraversal(TextChild(mw), XmTRAVERSE_CURRENT);
}

/*
 * Action to unmap the textField and discard any edits made
 */
/* ARGSUSED */
void
xbaeCancelEditACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    Boolean unmap;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w could be Matrix, or the Clip or textField children of Matrix
     */
    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)w;
    else if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "cancelEditACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to CancelEdit action",
            NULL, 0);
        return;
    }

    /*
     * Make sure we have a single param
     */
    if (*nparams != 1)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "cancelEditACT", "badParms", "XbaeMatrix",
            "XbaeMatrix: Wrong params passed to CancelEdit action, needs 1",
            NULL, 0);
        return;
    }

    /*
     * Validate our param
     */
    if (!strcmp(params[0], "True"))
        unmap = True;
    else if (!strcmp(params[0], "False"))
        unmap = False;
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "cancelEditACT", "badParm", "XbaeMatrix",
            "XbaeMatrix: Bad parameter for CancelEdit action",
            NULL, 0);
        return;
    }
    /*
     * Call the cancel_edit method
     */
    (*((XbaeMatrixWidgetClass)XtClass(mw))->matrix_class.cancel_edit)
        (mw, unmap);
}

/*
 * Action save any edits made and unmap the textField if params[0] is True
 */
/* ARGSUSED */
void
xbaeCommitEditACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    Boolean unmap;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w could be Matrix, or the Clip or textField children of Matrix
     */
    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)w;
    else if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "commitEditACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to CommitEdit action",
            NULL, 0);
        return;
    }

    /*
     * Make sure we have a single param
     */
    if (*nparams != 1)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "commitEditACT", "badParms", "XbaeMatrix",
            "XbaeMatrix: Wrong params for CommitEdit action, needs 1",
            NULL, 0);
        return;
    }

    /*
     * Validate our param
     */
    if (!strcmp(params[0], "True"))
        unmap = True;
    else if (!strcmp(params[0], "False"))
        unmap = False;
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "commitEditACT", "badParm", "XbaeMatrix",
            "XbaeMatrix: Bad parameter for CommitEdit action",
            NULL, 0);
        return;
    }

    (void)(*((XbaeMatrixWidgetClass)XtClass(mw))->matrix_class.commit_edit)
        (mw, event, unmap);
}

static int
DoubleClick(mw, event, row, column)
XbaeMatrixWidget mw;
XEvent *event;
int row;
int column;
{
    /* A double click in this instance is two clicks in the
       same cell in a time period < double_click_interval */
    Time current_time;
    unsigned long delta;
    static int ret = 0;

    if (event->type == ButtonRelease)
    {
        /* If the button is released, store the current location and time -
           next time through, if it's a button press event, we check for
           double click */
        mw->matrix.last_row = row;
        mw->matrix.last_column = column;
        if (ret)                /* just had a double click */
            mw->matrix.last_click_time = (Time)0;
        else
            mw->matrix.last_click_time = event->xbutton.time;
        ret = 0;
        return ret;
    }

    current_time = event->xbutton.time;
    delta = current_time - mw->matrix.last_click_time;

    if (row == mw->matrix.last_row && column == mw->matrix.last_column &&
        delta < (unsigned long)mw->matrix.double_click_interval)
        ret = 1;
    else
        ret = 0;

    return ret;
}

/*ARGSUSED*/
static void
PushButton(w, data, event, cont)
Widget w;
XtPointer data;
XEvent *event;
Boolean *cont;
{
    XbaeMatrixButtonPressedStruct *button =
         (XbaeMatrixButtonPressedStruct *)data;
    XMotionEvent *motionEvent;
    int x, y;
    int row, column;
    Boolean pressed = button->pressed;
    CellType cell;

    if (event->type == ButtonRelease)
    {
        button->grabbed = False;
        XtRemoveGrab(w);
        scrolling = False;

        if (button->pressed)
        {
            /* If the button is still pressed, it has been released in the
               same button that was pressed.  "Unpress" it and call the
               callbacks */        
            if (button->column == -1)
                xbaeDrawRowLabel(button->mw, button->row, False);
            else if (button->row == -1)
                xbaeDrawColumnLabel(button->mw, button->column, False);    

            if (button->mw->matrix.label_activate_callback)
            {
                XbaeMatrixLabelActivateCallbackStruct call_data;
                
                call_data.reason = XbaeLabelActivateReason;
                call_data.event = event;
                call_data.row_label = (button->column == -1);
                call_data.row = button->row;
                call_data.column = button->column;

                if (button->column == -1)
                    call_data.label =
                        button->mw->matrix.row_labels[button->row];
                else
                    call_data.label =
                        button->mw->matrix.column_labels[button->column];

                XtCallCallbackList((Widget)button->mw,
                                   button->mw->matrix.label_activate_callback,
                                   (XtPointer)&call_data);
            }
        }
        return;
    }

    if (event->type != MotionNotify) /* We want to be sure about this! */
        return;

    motionEvent = (XMotionEvent *)event;
    x = motionEvent->x;
    y = motionEvent->y;
    
    if (!xbaeEventToXY(button->mw, event, &x, &y, &cell))
        return;
    
    if (xbaeXYToRowCol(button->mw, &x, &y, &row, &column, cell))
        /* Moved off the labels */
        pressed = False;
    else
    {
        if (button->column != column || button->row != row)
            /* Moved out of the button that was originally pressed */
            pressed = False;
        else if (button->column == column || button->row == row)
            pressed = True;
    }        
    /* If the status of whether or not the button should be pressed has
       changed, redraw the appropriate visual */
    if (pressed != button->pressed)
    {
        if (button->column == -1)
            xbaeDrawRowLabel(button->mw, button->row, pressed);
        else if (button->row == -1)
            xbaeDrawColumnLabel(button->mw, button->column, pressed);    
        /* And set our struct's pressed member to the current setting */
        button->pressed = pressed;
    }
}

/*ARGSUSED*/
void
xbaeHandleClick(w, data, event, cont)
Widget w;
XtPointer data;
XEvent *event;
Boolean *cont;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget)data;
    int x, y;
    CellType cell;
    int row, column;
    Boolean translation;
    
    /* if we have a double click and a callback - break out! */
    if (event->type != ButtonPress && event->type != ButtonRelease)
        return;

    if (!xbaeEventToXY(mw, event, &x, &y, &cell))
        return;

    translation = xbaeXYToRowCol(mw, &x, &y, &row, &column, cell);

    if (!translation &&
        (mw->matrix.button_labels ||
         (row == -1 && mw->matrix.column_button_labels &&
          mw->matrix.column_button_labels[column]) ||
         (column == -1 && mw->matrix.row_button_labels &&
          mw->matrix.row_button_labels[row])) &&
        ((row == -1) ^ (column == -1)))
    {
        unsigned long event_mask;
        XtAppContext appcontext;
        XbaeMatrixButtonPressedStruct button;
        
        /* If the row and column are invalid, return. If it is ButtonRelease
           event, also return - the ButtonRelease events are handled in the
           event handler loop below */
        if (event->type != ButtonPress)
            return;

        if (column == -1 && event->type == ButtonPress)
            /* row label */
            xbaeDrawRowLabel(mw, row, True);
        else if (row == -1 && event->type == ButtonPress)
            /* Column label */
            xbaeDrawColumnLabel(mw, column, True);

        /* Action stations! */
        event_mask = ButtonReleaseMask | PointerMotionMask;

        scrolling = True;

        XtAddGrab(w, True, False);
        /* Copy the data needed to be passed to the event handler */
        button.mw = mw;
        button.row = row;
        button.column = column;
        button.pressed = True;
        button.grabbed = True;
                
        XtAddEventHandler(w, event_mask,
                           True, (XtEventHandler)PushButton,
                           (XtPointer)&button);
        XtAddEventHandler(TextChild(mw), event_mask,
                           True, (XtEventHandler)PushButton,
                           (XtPointer)&button);
    
        appcontext = XtWidgetToApplicationContext(w);

        while (button.grabbed)
            XtAppProcessEvent(appcontext, XtIMAll);

        XtRemoveEventHandler(w, event_mask, True,
                              (XtEventHandler)PushButton,
                              (XtPointer)&button);
        XtRemoveEventHandler(TextChild(mw), event_mask, True,
                              (XtEventHandler)PushButton,
                              (XtPointer)&button);

    }
    else if (translation && mw->matrix.default_action_callback &&
             w != (Widget)mw &&
             DoubleClick(mw, event, mw->matrix.current_row,
                         mw->matrix.current_column))
    {
        /* Put this as an else -> we don't want double clicks on labels
           to be recognised */
        XbaeMatrixDefaultActionCallbackStruct call_data;

        if (row == -1 || column == -1)
            return;
        
        call_data.reason = XbaeDefaultActionReason;
        call_data.event = event;
        call_data.row = row;
        call_data.column = column;

        XtCallCallbackList((Widget)mw, mw->matrix.default_action_callback,
                           (XtPointer)&call_data);
    }        
}

/* ARGSUSED */
void
xbaeSelectCellACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    int x, y;
    int row, column;
    CellType cell;
    XbaeMatrixSelectCellCallbackStruct call_data;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w could be Matrix, or the Clip or textField children of Matrix
     */
    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)w;
    else if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "xbaeSelectCellACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to SelectCell action",
            NULL, 0);
        return;
    }

    /*
     * If we don't have a selectCellCallback, then return now
     */
    if (!mw->matrix.select_cell_callback)
        return;

    if (!xbaeEventToXY(mw, event, &x, &y, &cell))
        return;

    /*
     * Convert the point to a row,column. If it does not pick a valid
     * cell, then return. If button up then use the last selected cell
     * to make sure a valid button up event occurs when dragging out of
     * the matrix
     */
    if (!xbaeXYToRowCol(mw, &x, &y, &row, &column, cell))
    {
        if (event->type == ButtonRelease)
        {
            column = last_selected_column;
            row = last_selected_row;
        }
        else
        {
            return;
        }
    }

    /*
     * Call our select_cell callbacks
     */
    call_data.reason = XbaeSelectCellReason;
    call_data.event = event;

    if (scrolling)
    {
        call_data.row = last_row;
        call_data.column = last_column;
    }
    else
    {
        call_data.row = row;
        call_data.column = column;
    }
    
    last_selected_column = call_data.column;
    last_selected_row = call_data.row;
    
    call_data.selected_cells = mw->matrix.selected_cells;
    call_data.cells = mw->matrix.cells;
    call_data.num_params = *nparams;
    call_data.params = params;

    XtCallCallbackList((Widget)mw, mw->matrix.select_cell_callback,
                       (XtPointer)&call_data);
}


/* ARGSUSED */
void
xbaeTraverseNextACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w should be the textField widget.
     */
    if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "traverseNextACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to TraverseNext action",
            NULL, 0);
        return;
    }

    /*
     * Set the traversing direction flag.  XmProcessTraversal may traverse
     * to the Clip widget. If it does, then we will see this flag in
     * the Clip focusCallback, TraverseInCB, and we will continue to traverse
     * on out of the mw.  yuck!
     */
    mw->matrix.traversing = XmTRAVERSE_NEXT_TAB_GROUP;
    (void)XmProcessTraversal(TextChild(mw), XmTRAVERSE_NEXT_TAB_GROUP);
    mw->matrix.traversing = NOT_TRAVERSING;
}

/* ARGSUSED */
void
xbaeTraversePrevACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w should be the textField widget.
     */
    if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "traversePrevACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to TraversePrev action",
            NULL, 0);
        return;
    }

    /*
     * Set the traversing direction flag.  XmProcessTraversal may traverse
     * to the Clip widget. If it does, then we will see this flag in
     * the Clip focusCallback, TraverseInCB, and we will continue to traverse
     * on out of the mw.  yuck!
     */
    mw->matrix.traversing = (int)XmTRAVERSE_PREV_TAB_GROUP;
    (void)XmProcessTraversal(TextChild(mw), XmTRAVERSE_PREV_TAB_GROUP);
    mw->matrix.traversing = NOT_TRAVERSING;
}

static void
callSelectCellAction(mw, event)
XbaeMatrixWidget mw;
XEvent *event;
{
    XbaeMatrixSelectCellCallbackStruct call_data;
    Boolean old_scroll_select = mw->matrix.scroll_select;
    
    mw->matrix.scroll_select = False;

    call_data.reason = XbaeSelectCellReason;
    call_data.event = event;
    call_data.row = last_row;
    call_data.column = last_column;
    call_data.selected_cells = mw->matrix.selected_cells;
    call_data.cells = mw->matrix.cells;
    call_data.num_params = 1;
    call_data.params = (char **)XtMalloc(sizeof(char *));
    call_data.params[0] = "extend";
    
    XtCallCallbackList(
         (Widget)mw, mw->matrix.select_cell_callback,
         (XtPointer)&call_data);

     (void)XtFree((char *)call_data.params);

     mw->matrix.scroll_select = old_scroll_select;
}


/*ARGSUSED*/
static void
checkScrollValues(w, data, event, cont)
Widget w;
XtPointer data;
XEvent *event;
Boolean *cont;
{
    XbaeMatrixScrollStruct *ss = (XbaeMatrixScrollStruct *)data;
    XMotionEvent *motionEvent;
    int x, y;
    CellType cell;
    Boolean inMatrix;
    int distance = 0;
    int halfRows;
    int denom = 1;
    int row, column;
    int i;

    ss->event = event;

    if (event->type == ButtonRelease)
    {
        XtRemoveTimeOut(ss->timerID);
        ss->grabbed = False;

        if (ss->mw->matrix.selection_policy == XmMULTIPLE_SELECT ||
            ss->mw->matrix.selection_policy == XmEXTENDED_SELECT)
            callSelectCellAction(ss->mw, ss->event);

        return;
    }

    if (!xbaeEventToXY(ss->mw, event, &x, &y, &cell))
        return;

    motionEvent = (XMotionEvent *)event;

    /*
     * In this instance, we don't care if a valid row and column are
     * returned as we'll be the judge of the result
     */
    inMatrix = xbaeXYToRowCol(ss->mw, &x, &y, &row, &column, cell);

    /*
     * Reset the flags, so the matrix stops scrolling when the
     * pointer is moved back into the fixed columns/rows after a drag
     * select in the fixed columns/rows which caused the matrix to
     * scroll vertically/horizontally. 
     */
    ss->below = False;
    ss->above = False;
    ss->left = False;
    ss->right = False;

    if (inMatrix && cell == NonFixedCell)
    {
        ss->inClip = True;
        return;
    }
    else
    {
        /*
         * Calculate our position relative to the clip and adjust.
         */
        if (motionEvent->y >= (int)(ss->cw->core.y + ss->cw->core.height))
        {
            /* Below the matrix */
            distance = motionEvent->y - ss->cw->core.y - ss->cw->core.height;
            ss->below = True;
            ss->above = False;
            /*
             * If we are below the matrix, the current column may have
             * still changed from horizontal motion.
             */
            i = 0;
            while (COLUMN_POSITION(ss->mw, i) < HORIZ_ORIGIN(ss->mw) +
                   motionEvent->x)
                i++;

            if (i <= ss->mw->matrix.columns && i > 0)
                last_column = i - 1;
        }
        else if (motionEvent->y <= ss->cw->core.y)
        {
            /*
             * Above the matrix - can't be both above and below at the same
             * time unless we have two mouses!
             */
            distance = ss->cw->core.y - motionEvent->y;
            ss->below = False;
            ss->above = True;
            i = 0;
            while (COLUMN_POSITION(ss->mw, i) < HORIZ_ORIGIN(ss->mw) +
                   motionEvent->x)
                i++;
            if (i > 0 && i <= ss->mw->matrix.columns)
                last_column = i - 1;
        }
        if (motionEvent->x <= ss->cw->core.x)
        {
            /* To the left */
            ss->left = True;
            ss->right = False;
            distance = Min(distance, ss->cw->core.x - motionEvent->x);
            /*
             * Check for any vertical motion
             */
            if (!ss->below && !ss->above)
            {
                last_row = YtoRow(ss->mw, motionEvent->y -
                                   COLUMN_LABEL_HEIGHT(ss->mw)) +
                    VERT_ORIGIN(ss->mw);
                SANITY_CHECK_ROW(ss->mw, last_row);
            }
        }
        else if (motionEvent->x >= (int)(ss->cw->core.x + ss->cw->core.width))
        {
            /* To the right */
            ss->left = False;
            ss->right = True;
            distance = Min(distance, (int)(motionEvent->x - ss->cw->core.x -
                            ss->cw->core.width));
            if (!ss->below && !ss->above)
            {
                last_row = YtoRow(ss->mw, motionEvent->y -
                                   COLUMN_LABEL_HEIGHT(ss->mw)) +
                    VERT_ORIGIN(ss->mw);
                SANITY_CHECK_ROW(ss->mw, last_row);
            }
        }
        /*
         * Adjust the value of the update interval based on the distance we
         * are away from the matrix
         */
        halfRows = distance / (ROW_HEIGHT(ss->mw) / 2);
        /*
         * Avoid use of the math library by doing a simple calculation
         */
        for (i = 0; i < halfRows; i++)
            denom *= 2;

        ss->interval = DEFAULT_SCROLL_SPEED / (denom > 0 ? denom : 1);

        if (ss->interval <= 0)        /* Just to be on the safe side */
            ss->interval = 1;
    }
}

static void
updateScroll(data)
XtPointer data;
{
    XbaeMatrixScrollStruct *ss = (XbaeMatrixScrollStruct *)data;
    Boolean callCallback = False;
    static int my_last_row = -1, my_last_column = -1;

    if (!scrolling)
        return;
    
    if (my_last_column != last_column || my_last_row != last_row)
        callCallback = True;

    my_last_row = last_row;
    my_last_column = last_column;
    /*
     * Off the clip widget - check there are cells that could
     * be scrolled into a visible position.  If there are,
     * scroll them into view.  If not, start setting the fixed
     * rows and columns as the current row and column.
     */
    if (ss->below && last_row < TRAILING_VERT_ORIGIN(ss->mw) - 1)
    {
        xbaeMakeRowVisible(ss->mw, ++last_row);
        callCallback = True;
    }
    else if (ss->above && last_row > (int)ss->mw->matrix.fixed_rows)
    {
        xbaeMakeRowVisible(ss->mw, --last_row);
        callCallback = True;
    }
    if (ss->right && last_column < TRAILING_HORIZ_ORIGIN(ss->mw) - 1)
    {
        xbaeMakeColumnVisible(ss->mw, ++last_column);
        callCallback = True;
    }
    else if (ss->left && last_column > (int)ss->mw->matrix.fixed_columns)
    {
        xbaeMakeColumnVisible(ss->mw, --last_column);
        callCallback = True;
    }

    if (callCallback &&
         (ss->mw->matrix.selection_policy == XmMULTIPLE_SELECT ||
          ss->mw->matrix.selection_policy == XmEXTENDED_SELECT))
        callSelectCellAction(ss->mw, ss->event);
    
    /*
     * Flush the updates out to the server so we don't end up lagging
     * behind too far and end up with a million redraw requests.
     * Particularly for higher update speeds
     */
    XFlush(XtDisplay((Widget)ss->mw));

    ss->timerID = XtAppAddTimeOut(
        ss->app_context, ss->interval, (XtTimerCallbackProc)updateScroll,
        (XtPointer)ss);    
}

/* ARGSUSED */
void
xbaeHandleMotionACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    XbaeClipWidget cw;
    XMotionEvent *motionEvent;
    XButtonEvent *buttonEvent;
    int x, y, row, column;
    CellType cell;
    Boolean inMatrix;

    if (scrolling)
        return;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w could be Matrix, or the Clip or textField children of Matrix
     */
    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)w;
    else if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "handleMotionACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to HandleMotion action",
            NULL, 0);
        return;
    }
    motionEvent = (XMotionEvent *)event;
    buttonEvent = (XButtonEvent *)event;

    cw = (XbaeClipWidget)ClipChild(mw);

    if (!xbaeEventToXY(mw, event, &x, &y, &cell))
        return;

    /*
     * In this instance, we don't care if a valid row and column are
     * returned as we'll be the judge of the result
     */
    inMatrix = xbaeXYToRowCol(mw, &x, &y, &row, &column, cell);

    if (inMatrix && cell == NonFixedCell)
    {
        /*
         * If we are in a NonFixedCell, then we're cruisin'.  Just
         * update our position
         */
        if (row != last_row || column != last_column)
        {
            if (row < mw->matrix.rows && column < mw->matrix.columns)
            {
                last_row = row;
                last_column = column;

                if (mw->matrix.selection_policy == XmMULTIPLE_SELECT ||
                    mw->matrix.selection_policy == XmEXTENDED_SELECT)
                    callSelectCellAction(mw, event);
            }
        }
    }
    else
    {
        XbaeMatrixScrollStruct scrollData;
        Boolean cont;

        /*
         * Grab the pointer and add a timeout routine to start modifying
         * the current row and/or column in the matrix.  Also add an
         * event handler to monitor the current distance outside the
         * matrix so we can adjust the timeout routine to go faster when
         * the pointer is further away from the matrix.
         */

        scrolling = True;

        XtAddGrab(w, True, False);

        scrollData.mw = mw;
        scrollData.cw = cw;
        scrollData.event = event;
        scrollData.interval = DEFAULT_SCROLL_SPEED;
        scrollData.inClip = False;
        scrollData.grabbed = True;
        scrollData.app_context = XtWidgetToApplicationContext(w);
        scrollData.above = scrollData.below = False;
        scrollData.left = scrollData.right = False;
        
        XtAddEventHandler(w, PointerMotionMask | ButtonReleaseMask,
                           True, (XtEventHandler)checkScrollValues,
                           (XtPointer)&scrollData);
        /*
         * Call checkScrollValues() to find out where exactly we are in
         * relation to the clip widget
         */
        checkScrollValues(w, (XtPointer)&scrollData, event, &cont);

        /*
         * The above / below / left / right members of the scrollData struct
         * should now be set so we know where we should be moving.  Let's
         * get on with it, eh?
         */
        updateScroll((XtPointer)&scrollData);

        while (scrollData.grabbed && !scrollData.inClip)
            XtAppProcessEvent(scrollData.app_context, XtIMAll);

        XtRemoveEventHandler(w, PointerMotionMask | ButtonReleaseMask,
                              True, (XtEventHandler)checkScrollValues,
                              (XtPointer)&scrollData);

        XtRemoveGrab(w);
        /*
         * We don't want the timeout getting called again as, in two lines,
         * we'll be way out of scope!
         */
        XtRemoveTimeOut(scrollData.timerID);
        scrolling = False;
    }
}

/* ARGSUSED */
void
xbaePageDownACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    char *down = "0";
    int top;
    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w should be the textField widget.
     */
    if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "pageDownACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to PageDown action",
            NULL, 0);
        return;
    }

    if (!XtIsManaged(VertScrollChild(mw)))
        return;

    /*
     * Save the top row - if scrolling occurs, the text widget needs
     * to be moved
     */
    top = VERT_ORIGIN(mw);

    XtCallActionProc(VertScrollChild(mw), "PageDownOrRight",
                     event, &down, 1);

    if (VERT_ORIGIN(mw) != top)
        /*
         * Position the cursor at the top most non fixed row if there was
         * a page down
         */
        XbaeMatrixEditCell((Widget)mw, VERT_ORIGIN(mw) +
                           mw->matrix.fixed_rows, mw->matrix.current_column);
}

/* ARGSUSED */
void
xbaePageUpACT(w, event, params, nparams)
Widget w;
XEvent *event;
String *params;
Cardinal *nparams;
{
    XbaeMatrixWidget mw;
    char *up = "0";
    int top;

    /*
     * Get Matrix widget and make sure it is a Matrix subclass.
     * w should be the textField widget.
     */
    if (XtIsSubclass(XtParent(w), xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget)XtParent(w);
    else
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "pageUpACT", "badWidget", "XbaeMatrix",
            "XbaeMatrix: Bad widget passed to PageUp action",
            NULL, 0);
        return;
    }

    if (!XtIsManaged(VertScrollChild(mw)))
        return;

    /*
     * Save the top row - if scrolling occurs, the text widget needs
     * to be moved
     */
    top = VERT_ORIGIN(mw);

    XtCallActionProc(VertScrollChild(mw), "PageUpOrLeft",
                     event, &up, 1);

    if (VERT_ORIGIN(mw) != top)
        /*
         * Position the cursor at the top most non fixed row if there was
         * a page down
         */
        XbaeMatrixEditCell((Widget)mw, VERT_ORIGIN(mw) +
                           mw->matrix.fixed_rows, mw->matrix.current_column);
}

