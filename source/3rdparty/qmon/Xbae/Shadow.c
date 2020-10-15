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
 * $Id: Shadow.c,v 1.1 2001-07-18 11:06:00 root Exp $
 */


/*
 * Shadow.c created by Andrew Lister (30 October, 1995)
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion > 1001
#include <Xm/DrawP.h>
#endif
#include "MatrixP.h"
#include "Shadow.h"
#include "Draw.h"
#include "Utils.h"

static void DrawRowShadow P((XbaeMatrixWidget, Window, int, int, int,
                             int, int, int, GC, GC));

static void DrawColumnShadow P((XbaeMatrixWidget, Window, int, int, int,
                                int, int, int, GC, GC));

static void DrawRowHighlight P((XbaeMatrixWidget, Window, GC, int, int,
                                 int, int, int, int, int));
static void DrawColumnHighlight P((XbaeMatrixWidget, Window, GC, int,
                                    int, int, int, int, int, int));

void
#if NeedFunctionPrototypes
xbaeDrawCellShadow(XbaeMatrixWidget mw, Window win, int row, int column, int x,
                   int y, int width, int height, Boolean label,
                   Boolean clipped, Boolean pressed)
#else
xbaeDrawCellShadow(mw, win, row, column, x, y, width, height, label, clipped,
                   pressed)
XbaeMatrixWidget mw;
Window win;
int row;
int column;
int x;
int y;
int width;
int height;
Boolean label;
Boolean clipped;
Boolean pressed;
#endif
{
    unsigned char grid_type;
    unsigned char shadow;
    
    if ((mw->matrix.cell_shadow_thickness == 0) &&
        (!IN_GRID_ROW_MODE(mw)) && (!IN_GRID_COLUMN_MODE(mw)))
        return;

    /*
     * Surround the cell with a shadow.
     */
    if(label)
    {
        shadow = pressed ? XmSHADOW_IN : XmSHADOW_OUT;
        grid_type = XmGRID_CELL_SHADOW;
    }
    else
    {
        shadow = mw->matrix.cell_shadow_types ?
            mw->matrix.cell_shadow_types[row][column] :
            mw->matrix.cell_shadow_type;
        grid_type = mw->matrix.grid_type;
    }
        
    if (clipped)
    {
        switch (grid_type)
        {
            case XmGRID_CELL_SHADOW:
                DRAW_SHADOW(XtDisplay(mw), win,
                            mw->matrix.cell_top_shadow_clip_gc,
                            mw->matrix.cell_bottom_shadow_clip_gc,
                            mw->matrix.cell_shadow_thickness,
                            x, y, width, height, shadow);
                break;

            /* Deprecated types. To be removed in next version. */
            case XmGRID_SHADOW_OUT:
                DRAW_SHADOW(XtDisplay(mw), win,
                            mw->matrix.cell_bottom_shadow_clip_gc,
                            mw->matrix.cell_top_shadow_clip_gc,
                            mw->matrix.cell_shadow_thickness,
                            x, y, width, height, shadow);
                break;
            case XmGRID_SHADOW_IN:
                DRAW_SHADOW(XtDisplay(mw), win,
                            mw->matrix.cell_top_shadow_clip_gc,
                            mw->matrix.cell_bottom_shadow_clip_gc,
                            mw->matrix.cell_shadow_thickness,
                            x, y, width, height, shadow);
                break;
        }
    }
    else
    {
        switch (grid_type)
        {
        case XmGRID_NONE:
            break;
        case XmGRID_ROW_SHADOW:
            DrawRowShadow(mw, win, row, column, x, y, width, height,
                          mw->manager.top_shadow_GC,
                          mw->manager.bottom_shadow_GC);
            break;
        case XmGRID_ROW_LINE:
            DrawRowShadow(mw, win, row, column, x, y, width, height,
                          mw->matrix.grid_line_gc, mw->matrix.grid_line_gc);
            break;
        case XmGRID_COLUMN_SHADOW:
            DrawColumnShadow(mw, win, row, column, x, y, width, height,
                             mw->manager.top_shadow_GC,
                             mw->manager.bottom_shadow_GC);
            break;
        case XmGRID_COLUMN_LINE:
            DrawColumnShadow(mw, win, row, column, x, y, width, height,
                             mw->matrix.grid_line_gc, mw->matrix.grid_line_gc);
            break;
        case XmGRID_CELL_LINE:
            DRAW_SHADOW(XtDisplay(mw), win,
                        mw->matrix.grid_line_gc,
                        mw->matrix.grid_line_gc,
                        mw->matrix.cell_shadow_thickness,
                        x, y, width, height, shadow);
            break;
        case XmGRID_CELL_SHADOW:
            DRAW_SHADOW(XtDisplay(mw), win,
                        mw->manager.top_shadow_GC,
                        mw->manager.bottom_shadow_GC,
                        mw->matrix.cell_shadow_thickness,
                        x, y, width, height, shadow);
            break;

        /* Deprecated types. To be removed in next version. */
        case XmGRID_LINE:
            DRAW_SHADOW(XtDisplay(mw), win,
                        mw->matrix.grid_line_gc,
                        mw->matrix.grid_line_gc,
                        mw->matrix.cell_shadow_thickness,
                        x, y, width, height, shadow);
            break;
        case XmGRID_SHADOW_OUT:
            DRAW_SHADOW(XtDisplay(mw), win,
                        mw->manager.bottom_shadow_GC,
                        mw->manager.top_shadow_GC,
                        mw->matrix.cell_shadow_thickness,
                        x, y, width, height, shadow);
            break;
        case XmGRID_SHADOW_IN:
            DRAW_SHADOW(XtDisplay(mw), win,
                        mw->manager.top_shadow_GC,
                        mw->manager.bottom_shadow_GC,
                        mw->matrix.cell_shadow_thickness,
                        x, y, width, height, shadow);
            break;
        }
    }    
}

#if XmVersion >= 1002
void
xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, width, height, reason)
XbaeMatrixWidget mw;
Window win;
GC gc;
int row;
int column;
int x;
int y;
int width;
int height;
int reason;
{    
    int thick;

    if (!mw->matrix.highlighted_cells) /* Just a precaution */
        return;

    if (!mw->matrix.highlighted_cells[row][column])
        return;                        /* Nothing to do! */

    if (reason & HIGHLIGHTING_SOMETHING)
        gc = mw->manager.highlight_GC;

    if (IN_GRID_ROW_MODE(mw) &&
        (reason & HighlightRow || reason & UnhighlightRow) &&
        mw->matrix.highlighted_cells[row][column] == HighlightRow)
        DrawRowHighlight(mw, win, gc, row, column, x, y, width, height,
                         reason);
    else if (IN_GRID_COLUMN_MODE(mw) &&
             (reason & HighlightColumn || reason & UnhighlightColumn) &&
        mw->matrix.highlighted_cells[row][column] == HighlightColumn)
        DrawColumnHighlight(mw, win, gc, row, column, x, y, width, height,
                            reason);
    else
    {
        thick = (2 * mw->matrix.cell_shadow_thickness);

        DRAW_HIGHLIGHT(XtDisplay(mw), win, gc,
                        x + mw->matrix.cell_shadow_thickness,
                        y + mw->matrix.cell_shadow_thickness,
                        width - thick, height - thick,
                        mw->matrix.cell_highlight_thickness,
                        LineSolid);
    }
}

static void
DrawRowHighlight(mw, win, gc, row, column, x, y, width, height, reason)
XbaeMatrixWidget mw;
Window win;
GC gc;
int row;
int column;
int x;
int y;
int width;
int height;
int reason;
{    
    XRectangle rect[1];
    
    rect[0].x = 0;
    rect[0].y = 0;
    rect[0].width = width;
    rect[0].height = height;

    XSetClipRectangles(XtDisplay(mw), gc, x, y, rect, 1, Unsorted);

    y += mw->matrix.cell_shadow_thickness;
    height -= 2 * mw->matrix.cell_shadow_thickness;

    if (column != mw->matrix.columns - 1)
    {
        if (column != 0)
            x -= (mw->matrix.cell_shadow_thickness +
                mw->matrix.cell_highlight_thickness);
        else
            x += mw->matrix.cell_shadow_thickness;
        DRAW_HIGHLIGHT(XtDisplay(mw), win, gc, x, y, mw->core.width, height,
                        mw->matrix.cell_highlight_thickness,
                        LineSolid);
    }
    else
    {
        if (NEED_HORIZ_FILL(mw))
            width = mw->core.width;
        
        x -= (mw->matrix.cell_shadow_thickness +
              mw->matrix.cell_highlight_thickness);
        width += mw->matrix.cell_highlight_thickness;

        DRAW_HIGHLIGHT(XtDisplay(mw), win, gc, x, y, width,
                        height, mw->matrix.cell_highlight_thickness,
                        LineSolid);

        if (NEED_HORIZ_FILL(mw))
        {
            int ax, ay;

            xbaeCalcHorizFill(mw, win, x, y, row, column, &ax, &ay,
                              &width, &height);
            rect[0].width = width;
            rect[0].height = height;
            XSetClipRectangles(XtDisplay(mw), gc, ax, ay, rect, 1,
                               Unsorted);
            height -= mw->matrix.cell_shadow_thickness * 2;
            ax -= mw->matrix.cell_highlight_thickness;
            width += (mw->matrix.cell_highlight_thickness -
                      mw->matrix.cell_shadow_thickness);
                
            DRAW_HIGHLIGHT(XtDisplay(mw), XtWindow(mw), gc, ax, ay,
                            width, height,
                            mw->matrix.cell_highlight_thickness,
                            LineSolid);
        }
    }
    XSetClipMask(XtDisplay(mw), gc, None);
}

static void
DrawColumnHighlight(mw, win, gc, row, column, x, y, width, height, reason)
XbaeMatrixWidget mw;
Window win;
GC gc;
int row;
int column;
int x;
int y;
int width;
int height;
int reason;
{    
    XRectangle rect[1];
    int vert_dead_space_height = VERT_DEAD_SPACE_HEIGHT(mw);
    int clip_vert_visible_space = CLIP_VERT_VISIBLE_SPACE(mw);
    int vert_sb_height = VERT_SB_HEIGHT(mw);
    Boolean need_vert_dead_space_fill = NEED_VERT_DEAD_SPACE_FILL(mw);

    /* This adjustment takes care of that little open area
     * between the last nonfixed row and the first trailing
     * fixed row when the matrix is smaller than its max height */
    if ((vert_dead_space_height == 0) &&
        ((TRAILING_VERT_ORIGIN(mw) - 1) == row) &&
        (vert_sb_height < clip_vert_visible_space))
        height += (mw->matrix.cell_shadow_thickness +
                    clip_vert_visible_space - vert_sb_height);

    rect[0].x = 0;
    rect[0].y = 0;
    rect[0].width = width;
    rect[0].height = height;

    XSetClipRectangles(XtDisplay(mw), gc, x, y, rect, 1, Unsorted);

    x += mw->matrix.cell_shadow_thickness;
    width -= 2 * mw->matrix.cell_shadow_thickness;

    if (row != mw->matrix.rows - 1)
    {
        if (row != 0)
            y -= (mw->matrix.cell_shadow_thickness +
                  mw->matrix.cell_highlight_thickness);
        else
            y += mw->matrix.cell_shadow_thickness;
        DRAW_HIGHLIGHT(XtDisplay(mw), win, gc, x, y, width, mw->core.height,
                        mw->matrix.cell_highlight_thickness,
                        LineSolid);
    }
    else
    {
        if (NEED_VERT_FILL(mw) && (! HAS_ATTACHED_TRAILING_ROWS(mw)))
            height = mw->core.height;
        else
            height += mw->matrix.cell_highlight_thickness;
        
        y -= (mw->matrix.cell_shadow_thickness +
              mw->matrix.cell_highlight_thickness);
        
        DRAW_HIGHLIGHT(XtDisplay(mw), win, gc, x, y, width,
                        height, mw->matrix.cell_highlight_thickness,
                        LineSolid);

        if (NEED_VERT_FILL(mw) || need_vert_dead_space_fill)
        {
            int ax, ay;

            xbaeCalcVertFill(mw, win, x, y, row, column, &ax, &ay,
                             &width, &height);
            
            /* If we're filling the dead space, then we only use
             * the width and ax from above call. */
            if (need_vert_dead_space_fill)
            {
                ay = UNATTACHED_TRAILING_ROWS_OFFSET(mw);
                height = mw->matrix.cell_shadow_thickness +
                    mw->matrix.cell_highlight_thickness +
                    vert_dead_space_height;
            }

            rect[0].width = width;
            rect[0].height = height;
            XSetClipRectangles(XtDisplay(mw), gc, ax, ay, rect, 1,
                               Unsorted);
            width -= mw->matrix.cell_shadow_thickness * 2;
            ay -= mw->matrix.cell_highlight_thickness;
            height += (mw->matrix.cell_highlight_thickness -
                       mw->matrix.cell_shadow_thickness);
                
            /* Make sure height extends past bottom clip */
            if (need_vert_dead_space_fill && IS_FIXED_COLUMN(mw, column))
                height += (mw->matrix.cell_shadow_thickness +
                           mw->matrix.cell_highlight_thickness);

            DRAW_HIGHLIGHT(XtDisplay(mw), XtWindow(mw), gc, ax, ay,
                            width, height,
                            mw->matrix.cell_highlight_thickness,
                            LineSolid);
        }
    }
    XSetClipMask(XtDisplay(mw), gc, None);
}

#endif

static void
DrawRowShadow(mw, win, row, column, x, y, width, height, topGC, bottomGC)
XbaeMatrixWidget mw;
Window win;
int row;
int column;
int x;
int y;
int width;
int height;
GC  topGC;
GC  bottomGC;
{
    XRectangle rect[1];
    unsigned char shadow = mw->matrix.row_shadow_types ?
        mw->matrix.row_shadow_types[row] : mw->matrix.cell_shadow_type;

    rect[0].x = 0;
    rect[0].y = 0;
    rect[0].width = width;
    rect[0].height = height;

    /*
     * Set up the clipping rectangle to be only over the current cell
     */
    XSetClipRectangles(XtDisplay(mw),
                       topGC,
                       x, y, rect, 1, Unsorted);
    if (topGC != bottomGC)
        XSetClipRectangles(XtDisplay(mw),
                           bottomGC,
                           x, y, rect, 1, Unsorted);
    /*
     * Now, convert our coordinates to what we need to draw
     */
    if (column != mw->matrix.columns - 1)
    {
        /*
         * If column is 0, then we need to show the left hand side of the
         * box, otherwise just draw the edges outside the clipping rectangle
         */
        width = mw->core.width;
        if (column != 0)
            x -= mw->matrix.cell_shadow_thickness;

        DRAW_SHADOW(XtDisplay(mw), win,
                    topGC, bottomGC,
                    mw->matrix.cell_shadow_thickness,
                    x, y, width, height, shadow);
    }
    else
    {
        if (NEED_HORIZ_FILL(mw))
            /*
             * If we are going to fill, the right hand side of the shadow
             * shouldn't be drawn - we'll do it later!
             */
            width = mw->core.width;
        else
            width += mw->matrix.cell_shadow_thickness;

        DRAW_SHADOW(XtDisplay(mw), win,
                    topGC, bottomGC,
                    mw->matrix.cell_shadow_thickness,
                    x - mw->matrix.cell_shadow_thickness, y, width,
                    height, shadow);

        if (NEED_HORIZ_FILL(mw))
        {
            /*
             * The filled part is drawn on the matrix's window so we need to
             * do a bit of extra work.
             */
            int ax, ay;

            xbaeCalcHorizFill(mw, win, x, y, row, column, &ax, &ay,
                              &width, &height);
            
            rect[0].width = width;
            rect[0].height = height;

            XSetClipRectangles(XtDisplay(mw),
                               topGC,
                               ax, ay, rect, 1, Unsorted);
            if (topGC != bottomGC)
                XSetClipRectangles(XtDisplay(mw),
                                   bottomGC,
                                   ax, ay, rect, 1, Unsorted);
             
            /*
             * Final tweaks. Note that these _cannot_ be part of the calc
             * horiz fill logic, since that is used to set the clipping
             * rectangle
             */

            if ((win == XtWindow(ClipChild(mw))) &&
                (height != ROW_HEIGHT(mw)))
            {
                if (height == (ClipChild(mw)->core.height +
                               ClipChild(mw)->core.y - ay))
                    height += mw->matrix.cell_shadow_thickness;

                if (ay == ClipChild(mw)->core.y)
                {
                    height += mw->matrix.cell_shadow_thickness;
                    ay -= mw->matrix.cell_shadow_thickness;
                }
            }

            /* Do same check for RightClip. Now, why this is even
             * necessary when we're drawing on the Matrix's window,
             * I dunno. This one is only necessary when
             * we've got trailing fixed columns. */
            if (mw->matrix.trailing_fixed_columns &&
                (win == XtWindow(RightClip(mw))) &&
                (height != ROW_HEIGHT(mw)))
            {
                if (height == (RightClip(mw)->core.height +
                               RightClip(mw)->core.y - ay))
                    height += mw->matrix.cell_shadow_thickness;

                if (ay == RightClip(mw)->core.y)
                {
                    height += mw->matrix.cell_shadow_thickness;
                    ay -= mw->matrix.cell_shadow_thickness;
                }
            }
            
            /*
             * Draw the remaining shadow directly onto the matrix window
             */
            DRAW_SHADOW(XtDisplay(mw), XtWindow(mw),
                        topGC, bottomGC,
                        mw->matrix.cell_shadow_thickness,
                        ax - mw->matrix.cell_shadow_thickness,
                        ay, width + mw->matrix.cell_shadow_thickness,
                        height, shadow);
        }
    }
    /*
     * Reset our GC's clip mask
     */
    XSetClipMask(XtDisplay(mw), topGC,
                 None);
    if (topGC != bottomGC)
        XSetClipMask(XtDisplay(mw), bottomGC,
                     None);
}

static void
DrawColumnShadow(mw, win, row, column, x, y, width, height, topGC, bottomGC)
XbaeMatrixWidget mw;
Window win;
int row;
int column;
int x;
int y;
int width;
int height;
GC  topGC;
GC  bottomGC;
{
    XRectangle rect[1];
    unsigned char shadow = mw->matrix.column_shadow_types ?
        mw->matrix.column_shadow_types[column] : mw->matrix.cell_shadow_type;
    int vert_dead_space_height = VERT_DEAD_SPACE_HEIGHT(mw);
    int clip_vert_visible_space = CLIP_VERT_VISIBLE_SPACE(mw);
    int vert_sb_height = VERT_SB_HEIGHT(mw);
    Boolean need_vert_dead_space_fill = NEED_VERT_DEAD_SPACE_FILL(mw);

    /* This adjustment takes care of that little open area
     * between the last nonfixed row and the first trailing
     * fixed row when the matrix is smaller than its max height */
    if ((vert_dead_space_height == 0) &&
        ((TRAILING_VERT_ORIGIN(mw) - 1) == row) &&
        (vert_sb_height < clip_vert_visible_space))
        height += (mw->matrix.cell_shadow_thickness +
                    clip_vert_visible_space - vert_sb_height);

    rect[0].x = 0;
    rect[0].y = 0;
    rect[0].width = width;
    rect[0].height = height;

    /*
     * Set up the clipping rectangle to be only over the current cell
     */
    XSetClipRectangles(XtDisplay(mw),
                       topGC,
                       x, y, rect, 1, Unsorted);
    if (topGC != bottomGC)
        XSetClipRectangles(XtDisplay(mw),
                           bottomGC,
                           x, y, rect, 1, Unsorted);
    /*
     * Now, convert our coordinates to what we need to draw
     */
    if (row != mw->matrix.rows - 1)
    {
        /*
         * If column is 0, then we need to show the left hand side of the
         * box, otherwise just draw the edges outside the clipping rectangle
         */
        height = mw->core.height;
        if (row != 0)
            y -= mw->matrix.cell_shadow_thickness;

        DRAW_SHADOW(XtDisplay(mw), win,
                    topGC, bottomGC,
                    mw->matrix.cell_shadow_thickness,
                    x, y, width, height, shadow);
    }
    else
    {
        if (NEED_VERT_FILL(mw) && (! HAS_ATTACHED_TRAILING_ROWS(mw)))
            /*
             * If we are going to fill, the bottom of the shadow
             * shouldn't be drawn
             */
            height = mw->core.height;
        else
            height += mw->matrix.cell_shadow_thickness;

        DRAW_SHADOW(XtDisplay(mw), win,
                    topGC, bottomGC,
                    mw->matrix.cell_shadow_thickness,
                    x, y - mw->matrix.cell_shadow_thickness,
                    width, height, shadow);

        /*
         * The filled part is drawn on the matrix's window so we need to
         * do a bit of extra work. We may need to fill either the entire
         * bottom portion of the matrix, or the dead space, that is, the
         * space between the last nonfixed row and the first trailing
         * fixed row. We only need to do the latter case when we've got
         * bottom attached trailing fixed rows with vertical dead space
         * (vertical dead space only occurs when the matrix is bigger
         * than its maximum height, i.e., we're filling).
         */
        if (NEED_VERT_FILL(mw) || need_vert_dead_space_fill)
        {
            int ax, ay;
            
            xbaeCalcVertFill(mw, win, x, y, row, column, &ax, &ay,
                             &width, &height);

            /* If we're filling the dead space, then we only use
             * the width and ax from above call. */
            if (need_vert_dead_space_fill)
            {
                ay = UNATTACHED_TRAILING_ROWS_OFFSET(mw);
                height = mw->matrix.cell_shadow_thickness +
                    vert_dead_space_height;
            }

            rect[0].width = width;
            rect[0].height = height;

            XSetClipRectangles(XtDisplay(mw),
                               topGC,
                               ax, ay, rect, 1, Unsorted);
            if (topGC != bottomGC)
                XSetClipRectangles(XtDisplay(mw),
                                   bottomGC,
                                   ax, ay, rect, 1, Unsorted);
 
             /*
             * Final tweaks. Note that these _cannot_ be part of the
             * calc vert fill logic, since that is used to set the
             * clipping rectangle
             */
             
             if ((win == XtWindow(ClipChild(mw))) &&
                (width != COLUMN_WIDTH(mw, column)))
             {
                 /* Make sure we don't draw a shadow along the matrix's
                 * shadow by extending our width past the edge of the
                 * clipping rectangle
                 */
                 if (width == (ClipChild(mw)->core.width +
                              ClipChild(mw)->core.x - ax))
                     width += mw->matrix.cell_shadow_thickness;
 
                 /*
                 * Make sure the shadow doesn't get drawn along the left
                 * side by moving the x pos outside the clipping
                 * rectangle.
                 */
                 if (ax == ClipChild(mw)->core.x)
                 {
                     width += mw->matrix.cell_shadow_thickness;
                     ax -= mw->matrix.cell_shadow_thickness;
                 }
             }
 
             /*
             * Do same check for BottomClip. Now, why this is even
             * necessary when we're drawing on the Matrix's window,
             * I dunno. This one is only necessary when we've got
             * trailing fixed rows.
             */
             if (mw->matrix.trailing_fixed_rows &&
                 (win == XtWindow(BottomClip(mw))) &&
                (width != COLUMN_WIDTH(mw, column)))
             {
                 if (width == (BottomClip(mw)->core.width +
                              BottomClip(mw)->core.x - ax))
                     width += mw->matrix.cell_shadow_thickness;
 
                 if (ax == BottomClip(mw)->core.x)
                 {
                     width += mw->matrix.cell_shadow_thickness;
                     ax -= mw->matrix.cell_shadow_thickness;
                 }
            }

            /* Make sure height extends past bottom clip */
            if (need_vert_dead_space_fill && IS_FIXED_COLUMN(mw, column))
                height += mw->matrix.cell_shadow_thickness;
 
            /*
             * Draw the remaining shadow directly onto the matrix window
             */
            DRAW_SHADOW(XtDisplay(mw), XtWindow(mw),
                        topGC, bottomGC,
                        mw->matrix.cell_shadow_thickness,
                        ax, ay - mw->matrix.cell_shadow_thickness,
                        width, height + mw->matrix.cell_shadow_thickness,
                        shadow);
        }
    }

    /*
     * Reset our GC's clip mask
     */
    XSetClipMask(XtDisplay(mw), topGC,
                 None);
    if (topGC != bottomGC)
        XSetClipMask(XtDisplay(mw), bottomGC,
                     None);
}
