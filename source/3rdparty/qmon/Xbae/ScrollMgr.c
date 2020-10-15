/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
 * Copyright(c) 1995-99 Andrew Lister
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
 * $Id: ScrollMgr.c,v 1.1 2001-07-18 11:06:00 root Exp $
 */

/*
 * ScrollMgr.c created by Andrew Lister (7 August, 1995)
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Xm/Xm.h>
#include "MatrixP.h"
#include "Draw.h"
#include "Shadow.h"
#include "ScrollMgr.h"
#include "Utils.h"
#include <Xm/ScrollBar.h>

/*
 * ScrollMgr implementation.
 * When we scroll using XCopyArea, occluding windows will cause GraphicsExpose
 * events to be generated, if there are no occluding windows then NoExpose
 * events will be generated. The removal of occluding windows will cause Expose
 * events.  If a number of scrolls (XCopyAreas) occur in quick succession,
 * the events will contain obsolete x/y information since our internal
 * coordinates have been scrolled to a new location.  The ScrollMgr
 * keeps track of scrolls and offsets required to relocate the events to the
 * current coordinate system.
 *
 * The Matrix widget has two ScrollMgrs, one for the Matrix's window
 * and one for the Clip widget's window.
 *
 * Each widgets compress_exposures field should be XtExposeCompressSeries
 * or XtExposeNoCompress.
 *
 * The idea behind this code is based on the PanHandler posted by Chuck Ocheret
 * (chuck@fid.morgan.com)
 */


/*
 * Create and initialize a ScrollMgr
 */
SmScrollMgr
xbaeSmCreateScrollMgr()
{
    SmScrollMgr scrollMgr = XtNew(SmScrollMgrRec);

    scrollMgr->offset_x = 0;
    scrollMgr->offset_y = 0;
    scrollMgr->scroll_count = 0;
    scrollMgr->scroll_queue = NULL;
    scrollMgr->scrolling = False;

    return scrollMgr;
}

/*
 * Destroy a ScrollMgr, including any queued scrolls
 */
void
xbaeSmDestroyScrollMgr(scrollMgr)
SmScrollMgr scrollMgr;
{
    if (scrollMgr->scroll_queue)
    {
        SmScrollNode node = scrollMgr->scroll_queue->next;

        while (node != scrollMgr->scroll_queue)
        {
            SmScrollNode d = node;

            node = node->next;
            XtFree((XtPointer) d);
        }
        XtFree((XtPointer) node);
    }

    XtFree((XtPointer) scrollMgr);
}

/*
 * Record a new scroll request in the ScrollMgr
 */
void
xbaeSmAddScroll(scrollMgr, delta_x, delta_y)
SmScrollMgr scrollMgr;
int delta_x;
int delta_y;
{
    SmScrollNode node = XtNew(SmScrollNodeRec);

    node->x = delta_x;
    node->y = delta_y;

    scrollMgr->offset_x += delta_x;
    scrollMgr->offset_y += delta_y;
    scrollMgr->scroll_count++;

    /*
     * Insert the node at the end of the queue
     */
    if (!scrollMgr->scroll_queue)
    {
        scrollMgr->scroll_queue = node;
        node->next = node;
        node->prev = node;
    }
    else
    {
        SmScrollNode last = scrollMgr->scroll_queue->prev;

        last->next = node;
        node->next = scrollMgr->scroll_queue;
        node->prev = last;
        scrollMgr->scroll_queue->prev = node;
    }
}

/*
 * Remove a scroll from the ScrollMgr queue
 */
void
xbaeSmRemoveScroll(scrollMgr)
SmScrollMgr scrollMgr;
{
    if (scrollMgr->scroll_count)
    {
        SmScrollNode node = scrollMgr->scroll_queue;

        scrollMgr->offset_x -= node->x;
        scrollMgr->offset_y -= node->y;

        /*
         * Remove node from head of queue
         */
        if (node->next == node)
            scrollMgr->scroll_queue = NULL;
        else
        {
            scrollMgr->scroll_queue = node->next;
            node->next->prev = node->prev;
            node->prev->next = node->next;
        }
        XtFree((XtPointer) node);

        scrollMgr->scroll_count--;
    }
}

/*
 * Handle an expose event
 */
void
xbaeSmScrollEvent(scrollMgr, event)
SmScrollMgr scrollMgr;
XEvent *event;
{
    switch (event->type)
    {

    case Expose:

        /*
         * Normal Expose event, translate it into our scrolled
         * coordinate system.
         */
        event->xexpose.x += scrollMgr->offset_x;
        event->xexpose.y += scrollMgr->offset_y;
        break;

    case GraphicsExpose:

        /*
         * If we are not scrolling, then this must be the first
         * GraphicsExpose event.  Remove the corresponding scroll from the
         * queue, and if we have more GraphicsExposes to come, set scrolling
         * to True.
         */
        if (scrollMgr->scrolling == False)
        {
            xbaeSmRemoveScroll(scrollMgr);
            if (event->xgraphicsexpose.count != 0)
                scrollMgr->scrolling = True;
        }

        /*
         * This is the last GraphicsExpose so set scrolling to False.
         */
        else if (event->xgraphicsexpose.count == 0)
            scrollMgr->scrolling = False;

        /*
         * Translate the event into our scrolled coordinate system.
         */
        event->xgraphicsexpose.x += scrollMgr->offset_x;
        event->xgraphicsexpose.y += scrollMgr->offset_y;
        break;

    case NoExpose:

        /*
         * A NoExpose event means we won't be getting any GraphicsExpose
         * events, so remove the scroll from the queue and set scrolling
         * to False.
         */
        xbaeSmRemoveScroll(scrollMgr);
        scrollMgr->scrolling = False;
        break;

    default:
        break;
    }
}

/*
 * Callback for vertical scrollbar
 */
/* ARGSUSED */
void
xbaeScrollVertCB(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmScrollBarCallbackStruct *call_data;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget) XtParent(w);
    Rectangle fixed, nonfixed;
    int src_y, dest_y, height;
    int vert_sb_offset = VERT_SB_OFFSET(mw);
    int row_height = ROW_HEIGHT(mw);
    int trailing_fixed_row_label_offset = TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
    int trailing_fixed_column_label_offset =
        TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw);
    int trailing_fixed_column_width = TRAILING_FIXED_COLUMN_WIDTH(mw);
    int row_label_width = ROW_LABEL_WIDTH(mw);
    int fixed_row_label_offset = FIXED_ROW_LABEL_OFFSET(mw);
    
    /*
     * Not managed yet
     */
    if (!XtIsRealized((Widget)mw))
        return;

    /*
     * Didn't scroll
     */
    if (call_data->value == VERT_ORIGIN(mw))
        return;

    /*
     * Scrolled forward. We want to copy a chunk starting at src_y up
     * to the top (dest_y=0)
     */
    else if (call_data->value > VERT_ORIGIN(mw))
    {
        dest_y = 0;
        src_y = (call_data->value - VERT_ORIGIN(mw)) * row_height;
        height = ClipChild(mw)->core.height - src_y;
    }

    /*
     * Scrolled backward. We want to copy a chunk starting at the top
     * (src_y=0) down to dest_y.
     */
    else
    {
        dest_y = (VERT_ORIGIN(mw) - call_data->value) * row_height;
        src_y = 0;
        height = ClipChild(mw)->core.height - dest_y;
    }

    /*
     * The textField needs to scroll along with the cells.
     */
    if (XtIsManaged(TextChild(mw)) &&
                mw->matrix.current_row >= (int)mw->matrix.fixed_rows &&
                mw->matrix.current_row < TRAILING_VERT_ORIGIN(mw))
        XtMoveWidget(TextChild(mw),
                     TextChild(mw)->core.x, TextChild(mw)->core.y +
                     (VERT_ORIGIN(mw) - call_data->value) * row_height);

    /*
     * Now we can adjust our vertical origin
     */
    VERT_ORIGIN(mw) = call_data->value;

    /*
     * If we scrolled more than a screenful, just clear and
     * redraw the whole thing
     */
    if (height <= 0)
    {
        /*
         * Clear the whole clip window.
         */
        XClearArea(XtDisplay(mw), XtWindow(ClipChild(mw)),
                   0, 0,
                   0 /*Full Width*/, 0 /*Full Height*/,
                   False);

        /*
         * Clear the whole Left and Right Clips
         */
        if (XtIsManaged(LeftClip(mw)))
            XClearArea(XtDisplay(mw), XtWindow(LeftClip(mw)),
                       0, 0,
                       0 /*Full Width*/, 0 /*Full Height*/,
                       False);
        if (XtIsManaged(RightClip(mw)))
            XClearArea(XtDisplay(mw), XtWindow(RightClip(mw)),
                       0, 0,
                       0 /*Full Width*/, 0 /*Full Height*/,
                       False);
        /*
         * Redraw all the non-fixed cells in the clip window
         */
        SETRECT(nonfixed,
                0, 0,
                ClipChild(mw)->core.width - 1,
                ClipChild(mw)->core.height - 1);

        /*
         * Clear non-fixed row labels, if necessary.
         * If we don't have a row label width, then
         * it would clear the entire width.
         */
        if (row_label_width)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       vert_sb_offset, fixed_row_label_offset,
                       row_label_width - 1,
                       VISIBLE_HEIGHT(mw),
                       False);

        /*
         * Clear the trailing filled rows, if necessary
         */
        if (IN_GRID_ROW_MODE(mw) && NEED_HORIZ_FILL(mw))
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       trailing_fixed_column_label_offset +
                       trailing_fixed_column_width,
                       fixed_row_label_offset,
                       FILL_HORIZ_WIDTH(mw),
                       ClipChild(mw)->core.height,
                       False);

        /*
         * Redraw non-fixed row labels and cells in fixed columns
         */
        SETRECT(fixed,
                vert_sb_offset, fixed_row_label_offset,
                trailing_fixed_column_label_offset +
                trailing_fixed_column_width - 1,
                trailing_fixed_row_label_offset - 1);
    }

    /*
     * If we scrolled less than a screenful, we want to copy as many
     * pixels as we can and then clear and redraw the newly scrolled data.
     */
    else
    {
        int y_clear = src_y > dest_y ? height : 0;

        /*
         * Queue this scroll with the ScrollMgr
         */
        xbaeSmAddScroll(mw->matrix.clip_scroll_mgr, 0, dest_y - src_y);

        /*
         * Copy the non-fixed cells in the clip widget
         */
        XCopyArea(XtDisplay(mw),
                  XtWindow(ClipChild(mw)), XtWindow(ClipChild(mw)),
                  mw->matrix.draw_gc,
                  0, src_y,
                  ClipChild(mw)->core.width, height,
                  0, dest_y);

        /*
         * Clear the newly scrolled chunk of the clip widget
         */
        XClearArea(XtDisplay(mw), XtWindow(ClipChild(mw)),
                   0, y_clear,
                   0 /*Full Width*/, ClipChild(mw)->core.height - height,
                   False);

        /*
         * Redraw the non-fixed cells into the new chunk
         */
        SETRECT(nonfixed,
                0, y_clear,
                ClipChild(mw)->core.width - 1,
                (y_clear + (ClipChild(mw)->core.height - height)) - 1);

        /*
         * Queue this scroll with the ScrollMgr
         */
        xbaeSmAddScroll(mw->matrix.matrix_scroll_mgr, 0, dest_y - src_y);

        /*
         * Copy cells in the fixed columns on LeftClip
         */
        if (XtIsManaged(LeftClip(mw)))
            XCopyArea(XtDisplay(mw),
                      XtWindow(LeftClip(mw)), XtWindow(LeftClip(mw)),
                      mw->matrix.draw_gc,
                      0, src_y,
                      FIXED_COLUMN_WIDTH(mw), height,
                      0, dest_y);

        /*
         * Copy cells in the trailing fixed columns on RightClip
         */
        if (XtIsManaged(RightClip(mw)))
            XCopyArea(XtDisplay(mw),
                      XtWindow(RightClip(mw)), XtWindow(RightClip(mw)),
                      mw->matrix.draw_gc,
                      0, src_y,
                      trailing_fixed_column_width, height,
                      0, dest_y);

        /*
         * Translate coordinates for row labels (on the parent matrix window).
         */
        src_y += fixed_row_label_offset;
        dest_y += fixed_row_label_offset;

        /*
         * Copy the row labels
         */
        if (row_label_width)
            XCopyArea(XtDisplay(mw),
                      XtWindow(mw), XtWindow(mw),
                      mw->matrix.draw_gc,
                      vert_sb_offset, src_y,
                      row_label_width, height,
                      vert_sb_offset, dest_y);
        /*
         * Copy trailing filled portion of the rows if necessary
         */
        if (IN_GRID_ROW_MODE(mw) && NEED_HORIZ_FILL(mw))
        {
            XCopyArea(XtDisplay(mw), XtWindow(mw), XtWindow(mw),
                      mw->matrix.draw_gc,
                      trailing_fixed_column_label_offset +
                      trailing_fixed_column_width, src_y,
                      FILL_HORIZ_WIDTH(mw), height,
                      trailing_fixed_column_label_offset +
                      trailing_fixed_column_width, dest_y);
        }

        /*
         * Clear newly scrolled chunk of fixed columns on LeftClip
         */
        if (XtIsManaged(LeftClip(mw)))
            XClearArea(XtDisplay(mw), XtWindow(LeftClip(mw)),
                       0, y_clear, 0 /* Full Width */,
                       ClipChild(mw)->core.height - height, False);
        /*
         * Clear newly scrolled chunk of trailing fixed columns on RightClip
         */
        if (XtIsManaged(RightClip(mw)))
            XClearArea(XtDisplay(mw), XtWindow(RightClip(mw)),
                       0, y_clear, 0 /* Full Width */,
                       ClipChild(mw)->core.height - height, False);
        /*
         * Translate coordinates for row labels on Matrix
         */
        y_clear += fixed_row_label_offset;

        /*
         * Clear the newly scrolled chunk of row labels
         */
        if (row_label_width)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       vert_sb_offset, y_clear,
                       row_label_width,
                       CLIP_VERT_VISIBLE_SPACE(mw) +
                       mw->manager.shadow_thickness,
                       False);

        /*
         * Clear the trailing filled rows, if necessary
         */
        if ((src_y > dest_y) && IN_GRID_ROW_MODE(mw) && NEED_HORIZ_FILL(mw))
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       trailing_fixed_column_label_offset +
                       trailing_fixed_column_width, y_clear, 
                       FILL_HORIZ_WIDTH(mw),
                       ClipChild(mw)->core.height - height,
                       False);
         
        /*
         * Redraw the new chunk of fixed columns and row labels
         */
        SETRECT(fixed,
                vert_sb_offset, y_clear,
                trailing_fixed_column_label_offset +
                trailing_fixed_column_width - 1,
                y_clear + CLIP_VERT_VISIBLE_SPACE(mw));
    }

    /*
     * Perform the actual redraw. The call to draw the grid shadows
     * must be done after any XClearAreas() to ensure we don't redraw
     * more than we have to; that is, we could put the call in the
     * redraw cells routine, but we would end up occasionally redrawing
     * more than once.
     */
    xbaeRedrawLabelsAndFixed(mw, &fixed);
    xbaeRedrawCells(mw, &nonfixed);
}

/*
 * Callback for horizontal scrollbar
 */
/* ARGSUSED */
void
xbaeScrollHorizCB(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmScrollBarCallbackStruct *call_data;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget) XtParent(w);
    Rectangle fixed, nonfixed;
    int src_x, dest_x, width;
    int horiz_sb_offset = HORIZ_SB_OFFSET(mw);
    int trailing_fixed_row_label_offset = TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
    int trailing_fixed_row_height = TRAILING_FIXED_ROW_HEIGHT(mw);
/*    int trailing_fixed_column_label_offset =
        TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw);*/
    int column_label_height = COLUMN_LABEL_HEIGHT(mw);
    int fixed_column_label_offset = FIXED_COLUMN_LABEL_OFFSET(mw);
    int fixed_column_width = FIXED_COLUMN_WIDTH(mw);
    int vert_dead_space_height = VERT_DEAD_SPACE_HEIGHT(mw);
    Boolean need_vert_fill = NEED_VERT_FILL(mw);
    Boolean has_attached_trailing_rows = HAS_ATTACHED_TRAILING_ROWS(mw);
    Boolean need_vert_dead_space_fill = NEED_VERT_DEAD_SPACE_FILL(mw);
    
    /*printf("%d\n", HORIZ_ORIGIN(mw));*/
    /*
     * Didn't scroll
     */
    if (call_data->value == HORIZ_ORIGIN(mw))
        return;

    /*
     * Scrolled right. We want to copy a chunk starting at src_x over to
     * the left (dest_x=0)
     */
    else if (call_data->value > HORIZ_ORIGIN(mw))
    {
        dest_x = 0;
        src_x = call_data->value - HORIZ_ORIGIN(mw);
        width = ClipChild(mw)->core.width - src_x;
    }

    /*
     * Scrolled left. We want to copy a chunk starting at the left (src_x=0)
     * over to the right to dest_x
     */
    else
    {
        dest_x = HORIZ_ORIGIN(mw) - call_data->value;
        src_x = 0;
        width = ClipChild(mw)->core.width - dest_x;
    }

    /*
     * The textField needs to scroll along with the cells.
     */
    if (XtIsManaged(TextChild(mw)) &&
                mw->matrix.current_column >= (int)mw->matrix.fixed_columns &&
                mw->matrix.current_column < TRAILING_HORIZ_ORIGIN(mw))
    {
        XtMoveWidget(TextChild(mw),
                     TextChild(mw)->core.x + (HORIZ_ORIGIN(mw) -
                                              call_data->value),
                     TextChild(mw)->core.y);
    }

    /*
     * Now we can adjust our horizontal origin
     */
    HORIZ_ORIGIN(mw) = call_data->value;
    mw->matrix.left_column = xbaeXtoCol(mw, fixed_column_width +
                                        HORIZ_ORIGIN(mw)) -
        mw->matrix.fixed_columns;

    if (!XtIsRealized((Widget)mw))
        return;

    /*
     * If we scrolled more than a screenful, just clear and
     * redraw the whole thing
     */
    if (width <= 0)
    {
        /*
         * Clear the whole clip window
         */
        XClearArea(XtDisplay(mw), XtWindow(ClipChild(mw)),
                   0, 0,
                   0 /* Full Width */, 0 /* Full Height */,
                   False);

        /*
         * Clear the whole Top and Bottom Clips
         */
        if (XtIsManaged(TopClip(mw)))
            XClearArea(XtDisplay(mw), XtWindow(TopClip(mw)),
                       0, 0,
                       0 /*Full Width*/, 0 /*Full Height*/,
                       False);
        if (XtIsManaged(BottomClip(mw)))
            XClearArea(XtDisplay(mw), XtWindow(BottomClip(mw)),
                       0, 0,
                       0 /*Full Width*/, 0 /*Full Height*/,
                       False);
        /*
         * Redraw all the non-fixed cells in the clip window
         */
        SETRECT(nonfixed,
                0, 0,
                ClipChild(mw)->core.width - 1,
                ClipChild(mw)->core.height - 1);

        /*
         * Clear the non-fixed column labels
         */
        if (column_label_height)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       fixed_column_label_offset, horiz_sb_offset,
                       VISIBLE_WIDTH(mw),
                       column_label_height - 1, False);
        /*
         * Clear the trailing fixed column
         */
        if (IN_GRID_COLUMN_MODE(mw))
        {
            if (need_vert_fill && (! has_attached_trailing_rows))
                XClearArea(XtDisplay(mw), XtWindow(mw),
                           fixed_column_label_offset,
                           trailing_fixed_row_label_offset +
                           trailing_fixed_row_height,
                           MATRIX_HORIZ_VISIBLE_SPACE(mw), 0, False);
            
            if (need_vert_dead_space_fill)
                XClearArea(XtDisplay(mw), XtWindow(mw),
                           fixed_column_label_offset,
                           UNATTACHED_TRAILING_ROWS_OFFSET(mw),
                           MATRIX_HORIZ_VISIBLE_SPACE(mw), 0, False);
        }

        /*
         * Redraw non-fixed column labels and cells in fixed rows
         */
        SETRECT(fixed,
                fixed_column_label_offset, horiz_sb_offset,
                 fixed_column_label_offset + CLIP_HORIZ_VISIBLE_SPACE(mw),
                trailing_fixed_row_label_offset +
                trailing_fixed_row_height - 1);
    }

    /*
     * If we scrolled less than a screenful, we want to copy as many
     * pixels as we can and then clear and redraw the newly scrolled data.
     */
    else
    {
        int x_clear = src_x > dest_x ? width : 0;
        int unattached_trailing_rows_offset =
            UNATTACHED_TRAILING_ROWS_OFFSET(mw);

        /*
         * Queue this scroll with the ScrollMgr
         */
        xbaeSmAddScroll(mw->matrix.clip_scroll_mgr, dest_x - src_x, 0);

        /*
         * Copy the non-fixed cells in the clip widget
         */
        XCopyArea(XtDisplay(mw),
                  XtWindow(ClipChild(mw)), XtWindow(ClipChild(mw)),
                  mw->matrix.draw_gc, src_x, 0, width,
                  ClipChild(mw)->core.height, dest_x, 0);

        /*
         * Clear the newly scrolled chunk of the clip widget
         */
        XClearArea(XtDisplay(mw), XtWindow(ClipChild(mw)),
                   x_clear, 0,
                   ClipChild(mw)->core.width - width, 0 /*Full Height*/,
                   False);

        /*
         * Redraw the non-fixed cells into the new chunk
         */
        SETRECT(nonfixed,
                x_clear, horiz_sb_offset,
                (x_clear + (ClipChild(mw)->core.width - width)) - 1,
                ClipChild(mw)->core.height - 1);

        /*
         * Queue this scroll with the ScrollMgr
         */
        xbaeSmAddScroll(mw->matrix.matrix_scroll_mgr, dest_x - src_x, 0);

        /*
         * Copy cells across in fixed rows on TopClip
         */
        if (XtIsManaged(TopClip(mw)))
            XCopyArea(XtDisplay(mw),
                      XtWindow(TopClip(mw)), XtWindow(TopClip(mw)),
                      mw->matrix.draw_gc, src_x, 0,
                      width, FIXED_ROW_HEIGHT(mw), dest_x, 0);

        /*
         * Copy cells across in trailing fixed rows on BottomClip
         */
        if (XtIsManaged(BottomClip(mw)))
            XCopyArea(XtDisplay(mw),
                      XtWindow(BottomClip(mw)), XtWindow(BottomClip(mw)),
                      mw->matrix.draw_gc, src_x, horiz_sb_offset, width,
                      trailing_fixed_row_height, dest_x, 0);

        /*
         * Translate coordinates for column labels on the Matrix.
         */
        src_x += fixed_column_label_offset;
        dest_x += fixed_column_label_offset;

        /*
         * Copy the column labels
         */
        if (column_label_height)
            XCopyArea(XtDisplay(mw),
                      XtWindow(mw), XtWindow(mw),
                      mw->matrix.draw_gc,
                      src_x, horiz_sb_offset,
                      width, column_label_height,
                      dest_x, horiz_sb_offset);
        
        /*
         * Copy trailing filled portion of the columns if necessary
         */
        if (IN_GRID_COLUMN_MODE(mw))
        {
            if (need_vert_fill && (! has_attached_trailing_rows))
                XCopyArea(XtDisplay(mw), XtWindow(mw), XtWindow(mw),
                          mw->matrix.draw_gc,
                          src_x, trailing_fixed_row_label_offset +
                          trailing_fixed_row_height,
                          width, FILL_VERT_HEIGHT(mw),
                          dest_x, trailing_fixed_row_label_offset +
                          trailing_fixed_row_height);
            
            if (need_vert_dead_space_fill)
                XCopyArea(XtDisplay(mw), XtWindow(mw), XtWindow(mw),
                          mw->matrix.draw_gc,
                          src_x, unattached_trailing_rows_offset,
                          width, vert_dead_space_height,
                          dest_x, unattached_trailing_rows_offset);
        }
        
        /*
         * Clear newly scrolled chunk of fixed rows on TopClip
         */
        if (XtIsManaged(TopClip(mw)))
            XClearArea(XtDisplay(mw), XtWindow(TopClip(mw)),
                       x_clear, 0,
                       ClipChild(mw)->core.width - width, 0 /* Full Height */,
                       False);

        /*
         * Clear newly scrolled chunk of trailing fixed rows on BottomClip
         */
        if (XtIsManaged(BottomClip(mw)))
            XClearArea(XtDisplay(mw), XtWindow(BottomClip(mw)),
                       x_clear, 0,
                       ClipChild(mw)->core.width - width, 0 /* Full Height */,
                       False);

        /*
         * Translate coordinates for row labels on Matrix
         */
        x_clear += fixed_column_label_offset;

        /*
         * Clear the newly scrolled chunk of column labels
         */
        if (column_label_height)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       x_clear, horiz_sb_offset,
                       ClipChild(mw)->core.width - width,
                       column_label_height,
                       False);

        /*
         * Clear the dead space if necessary
         */
        if (IN_GRID_COLUMN_MODE(mw) && need_vert_dead_space_fill)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       x_clear,unattached_trailing_rows_offset,
                       ClipChild(mw)->core.width - width,
                       vert_dead_space_height, False);

        /*
         * Redraw the new chunk of fixed rows and column labels
         */
        SETRECT(fixed,
                x_clear, horiz_sb_offset,
                x_clear + CLIP_HORIZ_VISIBLE_SPACE(mw),
                trailing_fixed_row_label_offset +
                trailing_fixed_row_height - 1);
    }

    /*
     * Perform the actual redraw. The call to draw the grid shadows
     * must be done after any XClearAreas() to ensure we don't redraw
     * more than we have to; that is, we could put the call in the
     * redraw cells routine, but we would end up occasionally redrawing
     * more than once.
     */
    xbaeRedrawLabelsAndFixed(mw, &fixed);
    xbaeRedrawCells(mw, &nonfixed);
}

/*
 * Redraw all cells in the clip widget damaged by the passed Rectangle.
 * The Rectangle must be within the bounds of the cells. These are the
 * non-fixed cells.
 */
void
xbaeRedrawCells(mw, expose)
XbaeMatrixWidget mw;
Rectangle *expose;
{
    int startCol, endCol, startRow, endRow, i, j;
    Rectangle rect;
    Boolean set_mask = False;

    if ((mw->matrix.disable_redisplay) || (!mw->matrix.rows) ||
        (!mw->matrix.columns))
        return;

    /*
     * Translate the 'expose' Rectangle to take into account the
     * fixed rows or columns.
     */
    SETRECT(rect,
            expose->x1 + FIXED_COLUMN_WIDTH(mw),
            expose->y1 + FIXED_ROW_HEIGHT(mw),
            expose->x2 + FIXED_COLUMN_WIDTH(mw),
            expose->y2 + FIXED_ROW_HEIGHT(mw));

    /*
     * Calculate the starting and ending rows/columns of the cells
     * which must be redrawn.
     */
    startCol = xbaeXtoCol(mw, rect.x1 + HORIZ_ORIGIN(mw));
    endCol = xbaeXtoCol(mw, rect.x2 + HORIZ_ORIGIN(mw));
    startRow = YtoRow(mw, rect.y1 + mw->matrix.first_row_offset) +
        VERT_ORIGIN(mw);
    endRow = YtoRow(mw, rect.y2 + mw->matrix.first_row_offset) +
        VERT_ORIGIN(mw);

    SANITY_CHECK_ROW(mw, startRow);
    SANITY_CHECK_ROW(mw, endRow);
    SANITY_CHECK_COLUMN(mw, startCol);
    SANITY_CHECK_COLUMN(mw, endCol);

    /*
     * Redraw all cells which were exposed.
     */
    for (i = startRow; i <= endRow; i++)
    {
        /*
         * If we need to clip the vertical fill
         */
        if ((!set_mask) && IN_GRID_COLUMN_MODE(mw) &&
            ((mw->matrix.rows - 1) == i) && NEED_VERT_FILL(mw))
        {
            set_mask = True;
            xbaeSetClipMask(mw, CLIP_TRAILING_FIXED_ROWS);
        }

        for (j = startCol; j <= endCol; j++)
            xbaeDrawCell(mw, i, j);
    }

    if (set_mask)
        xbaeSetClipMask(mw, CLIP_NONE);
}

/*
 * Redraw the row and column labels and the cells in fixed rows/columns
 * that are overlapped by the Rectangle argument.
 */
void
xbaeRedrawLabelsAndFixed(mw, expose)
XbaeMatrixWidget mw;
Rectangle *expose;
{
    /*
     * Set up some local variables to avoid calling too many macros :p
     */
    int horiz_sb_offset = HORIZ_SB_OFFSET(mw);
    int vert_sb_offset = VERT_SB_OFFSET(mw);
    int column_label_height = COLUMN_LABEL_HEIGHT(mw);
    int row_label_offset = ROW_LABEL_OFFSET(mw);
    int row_label_width = ROW_LABEL_WIDTH(mw);
    int fixed_row_label_offset = FIXED_ROW_LABEL_OFFSET(mw);
    int trailing_fixed_row_label_offset = TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
    int trailing_fixed_row_height = TRAILING_FIXED_ROW_HEIGHT(mw);
    int fixed_column_label_offset = FIXED_COLUMN_LABEL_OFFSET(mw);
    int fixed_column_width = FIXED_COLUMN_WIDTH(mw);
    int trailing_fixed_column_label_offset =
        TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw);
    int trailing_fixed_column_width = TRAILING_FIXED_COLUMN_WIDTH(mw);
    int column_label_offset = COLUMN_LABEL_OFFSET(mw);
    Boolean need_vert_fill = NEED_VERT_FILL(mw);
    Boolean has_attached_trailing_rows = HAS_ATTACHED_TRAILING_ROWS(mw);

    if (mw->matrix.disable_redisplay)
        return;

    /*
     * Handle the row labels that are in fixed rows
     */
    if (mw->matrix.rows && mw->matrix.fixed_rows && mw->matrix.row_labels)
    {
        Rectangle rect;
        
        /*
         * Get the Rectangle enclosing the fixed row labels
         */
        SETRECT(rect, vert_sb_offset, row_label_offset,
                vert_sb_offset + row_label_width - 1,
                fixed_row_label_offset - 1);
        
        /*
         * If the expose Rectangle overlaps, then some labels must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int endRow, i;

            /*
             * Intersect the fixed-row-labels Rectangle with the expose
             * Rectangle along the Y axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            Y_INTERSECT(rect, *expose, intersect);

            /*
             * Redraw each label that was intersected
             */
            endRow = YtoRow(mw, intersect.y2 + mw->matrix.first_row_offset);
            SANITY_CHECK_ROW(mw, endRow);
            for (i = YtoRow(mw, intersect.y1 + mw->matrix.first_row_offset),
                     SANITY_CHECK_ROW(mw, i);
                 i <= endRow; i++)
                xbaeDrawRowLabel(mw, i, False);
        }
    }

    /*
     * Handle the row labels that are in trailing fixed rows
     */
    if (mw->matrix.rows && mw->matrix.trailing_fixed_rows &&
        mw->matrix.row_labels)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the fixed row labels
         */
        SETRECT(rect, vert_sb_offset, trailing_fixed_row_label_offset,
                vert_sb_offset + row_label_width - 1,
                trailing_fixed_row_label_offset +
                trailing_fixed_row_height - 1);

        /*
         * If the expose Rectangle overlaps, then some labels must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int endRow, i;

            /*
             * Intersect the fixed-row-labels Rectangle with the expose
             * Rectangle along the Y axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            Y_INTERSECT(rect, *expose, intersect);

            /*
             * Redraw each label that was intersected
             */
            endRow = YtoRow(mw, intersect.y2) + TRAILING_VERT_ORIGIN(mw);
            SANITY_CHECK_ROW(mw, endRow);
            for (i = YtoRow(mw, intersect.y1) + TRAILING_VERT_ORIGIN(mw),
                 SANITY_CHECK_ROW(mw, i); i <= endRow; i++)
                xbaeDrawRowLabel(mw, i, False);
        }
    }

    /*
     * Handle row labels that aren't in fixed rows
     */
    if (mw->matrix.row_labels && mw->matrix.rows)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the non-fixed row labels
         */
        SETRECT(rect, vert_sb_offset, fixed_row_label_offset,
                vert_sb_offset + row_label_width - 1,
                fixed_row_label_offset + VISIBLE_HEIGHT(mw) - 1);

        /*
         * If the expose Rectangle overlaps, then some labels must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int endRow, i;

            /*
             * Intersect the fixed-row-labels Rectangle with the expose
             * Rectangle along the Y axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            Y_INTERSECT(rect, *expose, intersect);

            /*
             * Translate 'intersect' to take into account any fixed rows.
             * This gets it back into the coord system expected by YtoRow().
             */
            intersect.y1 += FIXED_ROW_HEIGHT(mw);
            intersect.y2 += FIXED_ROW_HEIGHT(mw);

            /*
             * Redraw each label that was intersected
             */
            endRow = YtoRow(mw, intersect.y2) + VERT_ORIGIN(mw);
            SANITY_CHECK_ROW(mw, endRow);
            for (i = YtoRow(mw, intersect.y1) + VERT_ORIGIN(mw),
                 SANITY_CHECK_ROW(mw, i); i <= endRow;         i++)
                xbaeDrawRowLabel(mw, i, False);
        }
    }

    /*
     * Handle the column labels that are in fixed columns
     */
    if (mw->matrix.columns && mw->matrix.fixed_columns &&
        mw->matrix.column_labels)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the portion of the column labels
         * that are in fixed columns
         */
        SETRECT(rect,
                column_label_offset, horiz_sb_offset,
                fixed_column_label_offset - 1,
                horiz_sb_offset + column_label_height - 1);

        /*
         * If the expose Rectangle overlaps, then some labels must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int endCol, i;

            /*
             * Intersect the fixed-column-labels Rectangle with the expose
             * Rectangle along the X axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            X_INTERSECT(rect, *expose, intersect);

            /*
             * Redraw each label that was intersected
             */
            endCol = xbaeXtoCol(mw, intersect.x2);
            SANITY_CHECK_COLUMN(mw, endCol);
            for (i = xbaeXtoCol(mw, intersect.x1), SANITY_CHECK_COLUMN(mw, i);
                 i <= endCol; i++)
                xbaeDrawColumnLabel(mw, i, False);
        }
    }

    /*
     * Handle the column labels that are in trailing fixed columns
     */
    if (mw->matrix.columns && mw->matrix.trailing_fixed_columns &&
        mw->matrix.column_labels)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the portion of the column labels
         * that are in fixed columns
         */
        SETRECT(rect,
                trailing_fixed_column_label_offset, horiz_sb_offset,
                trailing_fixed_column_label_offset +
                trailing_fixed_column_width,
                horiz_sb_offset + column_label_height);

        /*
         * If the expose Rectangle overlaps, then some labels must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int endCol, i;

            /*
             * Intersect the fixed-column-labels Rectangle with the expose
             * Rectangle along the X axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            X_INTERSECT(rect, *expose, intersect);

            /*
             * Redraw each label that was intersected
             */
            endCol = xbaeXtoTrailingCol(mw, intersect.x2);
            SANITY_CHECK_COLUMN(mw, endCol);
            for (i = xbaeXtoTrailingCol(mw, intersect.x1),
                 SANITY_CHECK_COLUMN(mw, i); i <= endCol; i++)
                xbaeDrawColumnLabel(mw, i, False);
        }
    }

    /*
     * Handle column labels that aren't in fixed columns
     */
    if (mw->matrix.column_labels && mw->matrix.columns)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the non-fixed column labels
         */
        SETRECT(rect,
                fixed_column_label_offset, horiz_sb_offset,
                fixed_column_label_offset + VISIBLE_WIDTH(mw) - 1,
                horiz_sb_offset + column_label_height - 1);

        /*
         * If the expose Rectangle overlaps, then some labels must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int endCol, i;

            /*
             * Intersect the non-fixed-column-labels Rectangle with the expose
             * Rectangle along the X axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            X_INTERSECT(rect, *expose, intersect);

            /*
             * Translate 'intersect' to take into account any fixed columns.
             * This gets it back into the coord system expected by XtoCol().
             */
            intersect.x1 += fixed_column_width;
            intersect.x2 += fixed_column_width;

            /*
             * Redraw each label that was intersected
             */
            endCol = xbaeXtoCol(mw, intersect.x2 + HORIZ_ORIGIN(mw));
            SANITY_CHECK_COLUMN(mw, endCol);
            for (i = xbaeXtoCol(mw, intersect.x1 + HORIZ_ORIGIN(mw)),
                 SANITY_CHECK_COLUMN(mw, i); i <= endCol; i++)
                xbaeDrawColumnLabel(mw, i, False);
        }
    }

    /*
     * Handle cells in fixed rows except those also in fixed columns
     */
    if (mw->matrix.rows && mw->matrix.columns && mw->matrix.fixed_rows)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the cells in fixed rows
         */
        SETRECT(rect,
                fixed_column_label_offset, row_label_offset,
                fixed_column_label_offset + VISIBLE_WIDTH(mw) - 1,
                fixed_row_label_offset - 1);

        /*
         * If the expose Rectangle overlaps, then some cells must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int startCol, endCol, startRow, endRow, i, j;

            /*
             * Intersect the fixed-cells Rectangle with the expose
             * Rectangle along the X and Y axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            INTERSECT(rect, *expose, intersect);

            intersect.x1 += fixed_column_width;
            intersect.x2 += fixed_column_width;

            /*
             * Get starting and ending rows/columns. Always take into
             * account the scrolling origins for the columns; for rows
             * only if we are fixed in that dimension.
             */
            startCol = xbaeXtoCol(mw, intersect.x1 + HORIZ_ORIGIN(mw));
            endCol = xbaeXtoCol(mw, intersect.x2 + HORIZ_ORIGIN(mw));

            startRow = YtoRow(mw, intersect.y1) +
                (mw->matrix.fixed_rows
                 ? 0
                 : VERT_ORIGIN(mw));
            endRow = YtoRow(mw, intersect.y2) +
                (mw->matrix.fixed_rows
                 ? 0
                 : VERT_ORIGIN(mw));

            /*
             * Redraw each cell that was intersected
             */
            SANITY_CHECK_ROW(mw, startRow);
            SANITY_CHECK_ROW(mw, endRow);
            SANITY_CHECK_COLUMN(mw, startCol);
            SANITY_CHECK_COLUMN(mw, endCol);
            for (i = startRow; i <= endRow; i++)
                for (j = startCol; j <= endCol; j++)
                    xbaeDrawCell(mw, i, j);
        }
    }

    /*
     * Handle cells in trailing fixed rows except those also in fixed columns
     */
    if (mw->matrix.rows && mw->matrix.columns &&
        mw->matrix.trailing_fixed_rows)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the cells in trailing fixed rows
         */
        SETRECT(rect, fixed_column_label_offset,
                trailing_fixed_row_label_offset,
                fixed_column_label_offset + VISIBLE_WIDTH(mw) - 1,
                trailing_fixed_row_label_offset +
                trailing_fixed_row_height - 1);

        /*
         * If the expose Rectangle overlaps, then some cells must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int startCol, endCol, startRow, endRow, i, j;

            /*
             * Intersect the fixed-cells Rectangle with the expose
             * Rectangle along the X and Y axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            INTERSECT(rect, *expose, intersect);

            intersect.x1 += fixed_column_width;
            intersect.x2 += fixed_column_width;

            /*
             * Get starting and ending rows/columns. Always take into
             * account the scrolling origins for the columns and never
             * for the rows.
             */
            startCol = xbaeXtoCol(mw, intersect.x1 + HORIZ_ORIGIN(mw));
            endCol = xbaeXtoCol(mw, intersect.x2 + HORIZ_ORIGIN(mw));

            startRow = YtoRow(mw, intersect.y1) + TRAILING_VERT_ORIGIN(mw);
            endRow = YtoRow(mw, intersect.y2) + TRAILING_VERT_ORIGIN(mw);

            /*
             * Redraw each cell that was intersected
             */
            SANITY_CHECK_ROW(mw, startRow);
            SANITY_CHECK_ROW(mw, endRow);
            SANITY_CHECK_COLUMN(mw, startCol);
            SANITY_CHECK_COLUMN(mw, endCol);
            xbaeSetClipMask(mw, CLIP_TRAILING_FIXED_ROWS);
            for (i = startRow; i <= endRow; i++)
                for (j = startCol; j <= endCol; j++)
                    xbaeDrawCell(mw, i, j);
            xbaeSetClipMask(mw, CLIP_NONE);
        }
    }

    /*
     * Handle cells in fixed columns
     */
    if (mw->matrix.rows && mw->matrix.columns && mw->matrix.fixed_columns)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the cells in fixed columns
         */
        SETRECT(rect,
                column_label_offset, row_label_offset,
                fixed_column_label_offset - 1,
                trailing_fixed_row_label_offset +
                trailing_fixed_row_height - 1);

        /*
         * If the expose Rectangle overlaps, then some cells must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int startCol, endCol, startRow, endRow, skipRow = -1, i, j;
            Boolean redrawFixedRows, redrawTrailingFixedRows;
            unsigned int clip_reason = CLIP_NONE;

            /*
             * Intersect the fixed-cells Rectangle with the expose
             * Rectangle along the X and Y axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            INTERSECT(rect, *expose, intersect);

            /*
             * If we have any fixed rows, we might need to redraw the cells
             * located in the intersection of the fixed rows and columns.
             * These cells may force use to be different than our current
             * VERT_ORIGIN.
             */
            redrawFixedRows = redrawTrailingFixedRows = False;
            if (mw->matrix.fixed_rows)
            {
                clip_reason = CLIP_FIXED_COLUMNS;

                SETRECT(rect,
                        column_label_offset, row_label_offset,
                        fixed_column_label_offset - 1,
                        fixed_row_label_offset - 1);

                if (OVERLAP(*expose, rect))
                    redrawFixedRows = True;
            }

            if (mw->matrix.trailing_fixed_rows)
            {
                clip_reason = CLIP_FIXED_COLUMNS;

                SETRECT(rect,
                        column_label_offset,
                        trailing_fixed_row_label_offset,
                        fixed_column_label_offset - 1,
                        trailing_fixed_row_label_offset +
                        trailing_fixed_row_height - 1);

                if (OVERLAP(*expose, rect))
                    redrawTrailingFixedRows = True;
            }

            if (CLIP_NONE != clip_reason)
                xbaeSetClipMask(mw, clip_reason);

            /*
             * Get starting and ending rows/columns. Never take into
             * account the scrolling origins for the rows; for columns
             * only if we are fixed in that dimension.
             */
            startCol = xbaeXtoCol(mw, intersect.x1 +
                              (mw->matrix.fixed_columns
                               ? 0
                               : HORIZ_ORIGIN(mw)));
            endCol = xbaeXtoCol(mw, intersect.x2 +
                            (mw->matrix.fixed_columns
                             ? 0
                             : HORIZ_ORIGIN(mw)));

            startRow = redrawFixedRows ? 0 : YtoRow(mw, intersect.y1) +
                VERT_ORIGIN(mw);
            if (redrawTrailingFixedRows)
            {
                skipRow = YtoRow(mw, intersect.y2) + VERT_ORIGIN(mw) -
                    mw->matrix.trailing_fixed_rows + 1;
                endRow = mw->matrix.rows - 1;
            }
            else
                endRow = YtoRow(mw, intersect.y2) + VERT_ORIGIN(mw);

            /*
             * Redraw each cell that was intersected
             */
            SANITY_CHECK_ROW(mw, startRow);
            SANITY_CHECK_ROW(mw, endRow);
            SANITY_CHECK_COLUMN(mw, startCol);
            SANITY_CHECK_COLUMN(mw, endCol);
            for (i = startRow; i <= endRow; i++)
                for (j = startCol; j <= endCol; j++)
                {
                    /*
                     * If we had to redraw cells located in both fixed rows
                     * and columns, when we are done redrawing those cells,
                     * we need to skip to the correct non-fixed row to draw,
                     * or alternatively, jump to the trailing fixed row
                     * to draw.
                     */
                    if (redrawFixedRows && i == mw->matrix.fixed_rows)
                        i += VERT_ORIGIN(mw);

                    if (redrawTrailingFixedRows && i == skipRow &&
                        endRow > skipRow && i < TRAILING_VERT_ORIGIN(mw))
                        i = TRAILING_VERT_ORIGIN(mw);

                    /*
                     * If we need to clip the vertical fill
                     */
                    if (!(clip_reason & CLIP_TRAILING_FIXED_ROWS) &&
                        IN_GRID_COLUMN_MODE(mw) &&
                        ((mw->matrix.rows - 1) == i) && 
                        (need_vert_fill && (! has_attached_trailing_rows)))
                    {
                        clip_reason |= CLIP_FIXED_COLUMNS |
                            CLIP_TRAILING_FIXED_ROWS;
                        xbaeSetClipMask(mw, clip_reason);
                    }

                    xbaeDrawCell(mw, i, j);
                }

            if (CLIP_NONE != clip_reason)
                xbaeSetClipMask(mw, CLIP_NONE);
        }
    }

    /*
     * Handle cells in trailing fixed columns
     */
    if (mw->matrix.rows && mw->matrix.columns &&
        mw->matrix.trailing_fixed_columns)
    {
        Rectangle rect;

        /*
         * Get the Rectangle enclosing the cells in trailing fixed columns
         */
        SETRECT(rect,
                trailing_fixed_column_label_offset, row_label_offset,
                trailing_fixed_column_label_offset +
                trailing_fixed_column_width - 1,
                trailing_fixed_row_label_offset +
                trailing_fixed_row_height - 1);

        /*
         * If the expose Rectangle overlaps, then some cells must be redrawn
         */
        if (OVERLAP(*expose, rect))
        {
            Rectangle intersect;
            int startCol, endCol, startRow, endRow, skipRow = -1, i, j;
            Boolean redrawFixedRows, redrawTrailingFixedRows;
            unsigned int clip_reason = CLIP_NONE;

            /*
             * Intersect the fixed-cells Rectangle with the expose
             * Rectangle along the X and Y axis.  The resulting Rectangle will
             * be in 'rect's coordinate system.
             */
            INTERSECT(rect, *expose, intersect);

            /*
             * If we have any fixed rows, we might need to redraw the cells
             * located in the intersection of the fixed rows and columns.
             * These cells may force us to be different than our current
             * VERT_ORIGIN.
             */
            redrawFixedRows = redrawTrailingFixedRows = False;
            if (mw->matrix.fixed_rows)
            {
                clip_reason = CLIP_FIXED_COLUMNS;

                SETRECT(rect, trailing_fixed_column_label_offset,
                        row_label_offset,
                        trailing_fixed_column_label_offset +
                        trailing_fixed_column_width - 1,
                        fixed_row_label_offset - 1);

                if (OVERLAP(*expose, rect))
                    redrawFixedRows = True;
            }

            if (mw->matrix.trailing_fixed_rows)
            {
                clip_reason = CLIP_FIXED_COLUMNS;

                SETRECT(rect, trailing_fixed_column_label_offset,
                        trailing_fixed_row_label_offset,
                        trailing_fixed_column_label_offset +
                        trailing_fixed_column_width - 1,
                        trailing_fixed_row_label_offset +
                        trailing_fixed_row_height - 1);

                if (OVERLAP(*expose, rect))
                    redrawTrailingFixedRows = True;
            }

            if (CLIP_NONE != clip_reason)
                xbaeSetClipMask(mw, clip_reason);

            /*
             * Get starting and ending rows/columns. Never take into
             * account the scrolling origins for the rows; for columns
             * only if we are fixed in that dimension.
             */
            startCol = xbaeXtoTrailingCol(mw, intersect.x1);
            endCol = xbaeXtoTrailingCol(mw, intersect.x2);

            startRow = redrawFixedRows ? 0 : YtoRow(mw, intersect.y1) +
                VERT_ORIGIN(mw);

            if (redrawTrailingFixedRows)
            {
                skipRow = YtoRow(mw, intersect.y2) + VERT_ORIGIN(mw) -
                    mw->matrix.trailing_fixed_rows + 1;
                endRow = mw->matrix.rows - 1;
            }
            else
                endRow = YtoRow(mw, intersect.y2) + VERT_ORIGIN(mw);

            /*
             * Redraw each cell that was intersected
             */
            SANITY_CHECK_ROW(mw, startRow);
            SANITY_CHECK_ROW(mw, endRow);
            SANITY_CHECK_COLUMN(mw, startCol);
            SANITY_CHECK_COLUMN(mw, endCol);
            for (i = startRow; i <= endRow; i++)
                for (j = startCol; j <= endCol; j++)
                {
                    /*
                     * If we had to redraw cells located in both fixed rows
                     * and columns, when we are done redrawing those cells,
                     * we need to skip to the correct non-fixed row to draw
                     */                    
                    if (redrawFixedRows && (i == mw->matrix.fixed_rows))
                        i += VERT_ORIGIN(mw);

                    if (redrawTrailingFixedRows && (i == skipRow) &&
                        (endRow > skipRow))
                        i = TRAILING_VERT_ORIGIN(mw);
                    
                    /*
                     * If we need to clip the vertical fill
                     */
                    if (!(clip_reason & CLIP_TRAILING_FIXED_ROWS) &&
                        IN_GRID_COLUMN_MODE(mw) &&
                        ((mw->matrix.rows - 1) == i) && 
                        (need_vert_fill && (! has_attached_trailing_rows)))
                    {
                        clip_reason |= CLIP_TRAILING_FIXED_COLUMNS |
                            CLIP_TRAILING_FIXED_ROWS;
                        xbaeSetClipMask(mw, clip_reason);
                    }

                    xbaeDrawCell(mw, i, j);
                }

            if (CLIP_NONE != clip_reason)
                xbaeSetClipMask(mw, CLIP_NONE);
        }
    }

    /*
     * Draw a shadow just inside row/column labels and around outer edge
     * of clip widget.        We can't use height of clip widget because it is
     * truncated to nearest row.  We use cell_visible_height instead.
     */
    if (mw->manager.shadow_thickness)
    {
        Dimension width, height;

        if (! mw->matrix.fill)
        {
            width = ClipChild(mw)->core.width + fixed_column_width +
                trailing_fixed_column_width +
                2 * mw->manager.shadow_thickness;
            height = mw->matrix.cell_visible_height + FIXED_ROW_HEIGHT(mw) +
                trailing_fixed_row_height +
                2 * mw->manager.shadow_thickness;
        }
        else
        {
            width = mw->core.width - row_label_width -
                VERT_SB_SPACE(mw);
            
            height = mw->core.height - column_label_height -
                HORIZ_SB_SPACE(mw) ;
        }
        
        DRAW_SHADOW(XtDisplay(mw), XtWindow(mw),
                    mw->manager.top_shadow_GC,
                    mw->manager.bottom_shadow_GC,
                    mw->manager.shadow_thickness,
                    row_label_width + vert_sb_offset,
                    column_label_height + horiz_sb_offset,
                    width, height, mw->matrix.shadow_type);
    }
}
