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
 * MatrixWidget Author: Andrew Wason, Bellcore, aw@bae.bellcore.com
 *
 * $Id: Utils.c,v 1.1 2001-07-18 11:06:00 root Exp $
 */

/*
 * Utils.c created by Andrew Lister (7 August, 1995)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Xm/Xm.h>
#include "MatrixP.h"
#include "Macros.h"
#include "Utils.h"
#include "Actions.h"
#include <Xm/ScrollBar.h>
/*
 * Return the top and bottom-most visible non-fixed row
 */
void
xbaeGetVisibleRows(mw, top_row, bottom_row)
XbaeMatrixWidget mw;
int *top_row, *bottom_row;
{
    *top_row = VERT_ORIGIN(mw) + mw->matrix.fixed_rows;
    *bottom_row = *top_row + (VISIBLE_HEIGHT(mw) - 1) / ROW_HEIGHT(mw);
    SANITY_CHECK_ROW(mw, *bottom_row);
}

/*
 * Return the left and right-most visible non-fixed column
 */
void
xbaeGetVisibleColumns(mw, left_column, right_column)
XbaeMatrixWidget mw;
int *left_column, *right_column;
{
    *left_column = xbaeXtoCol(mw, FIXED_COLUMN_WIDTH(mw) + HORIZ_ORIGIN(mw));
    *right_column = xbaeXtoCol(mw, FIXED_COLUMN_WIDTH(mw) + HORIZ_ORIGIN(mw) +
                               VISIBLE_WIDTH(mw) - 1);
}

/*
 * Return the top and bottom row and left and right column of
 * the visible non-fixed cells
 */
void
xbaeGetVisibleCells(mw, top_row, bottom_row, left_column, right_column)
XbaeMatrixWidget mw;
int *top_row, *bottom_row, *left_column, *right_column;
{
    xbaeGetVisibleRows(mw, top_row, bottom_row);
    xbaeGetVisibleColumns(mw, left_column, right_column);
}

/*
 * Try to make the column specified by the leftColumn resource
 * be the left column. The column is relative to fixed_columns - so 0 would
 * be the first non-fixed column.
 * If we can't make leftColumn the left column, make it as close as possible.
 */
void
xbaeAdjustLeftColumn(mw)
XbaeMatrixWidget mw;
{
    int y;
    int i;
    int required_width;
    int visible_width = VISIBLE_WIDTH(mw);
    int dynamic_columns;

    if (visible_width < 0)        /* will happen on initialisation */
        return;

    /* Adjust the column if it is out of bounds */

    dynamic_columns =  mw->matrix.columns - mw->matrix.fixed_columns -
        mw->matrix.trailing_fixed_columns;

    if (mw->matrix.left_column < 0)
        mw->matrix.left_column = 0;
    else if (mw->matrix.left_column > dynamic_columns-1)
        mw->matrix.left_column =  dynamic_columns-1;

    /* Find out where the horiz_origin will be if we tried setting the
       given left column and adjust till it all fits */
    do
    {
        required_width = 0;
        HORIZ_ORIGIN(mw) = 0;
        xbaeRowColToXY(mw, mw->matrix.fixed_rows,
                       mw->matrix.left_column + mw->matrix.fixed_columns,
                       &mw->matrix.horiz_origin, &y);
        /* Check how much space is remaining */
        for (i = mw->matrix.left_column + mw->matrix.fixed_columns;
             i < mw->matrix.columns -
                 (int)mw->matrix.trailing_fixed_columns; i++)
        {
            required_width += COLUMN_WIDTH(mw, i);
             if (required_width >= visible_width)
                break;
        }
        if (required_width < visible_width)
            mw->matrix.left_column--;
    }
    while (required_width < visible_width);
}

/*
 * Try to make the row specified by the topRow resource (VERT_ORIGIN)
 * be the top row. The row is relative to fixed_rows - so 0 would
 * be the first non-fixed row.
 * If we can't make topRow the top row, make it as close as possible.
 */
void
xbaeAdjustTopRow(mw)
XbaeMatrixWidget mw;
{
    int rows_visible = VISIBLE_HEIGHT(mw) / ROW_HEIGHT(mw);

    /*
     * If we have less than one full row visible, then count it as a full row
     */
    if (rows_visible <= 0)
        rows_visible = 1;
    /*
     * rows_visible might be inaccurate since Clip may not have been resized
     */
    else if (rows_visible > mw->matrix.rows)
        rows_visible = mw->matrix.rows;
    
    if (VERT_ORIGIN(mw) > (int)(mw->matrix.rows - rows_visible -
                                mw->matrix.fixed_rows -
                                mw->matrix.trailing_fixed_rows))
        mw->matrix.top_row = mw->matrix.rows - rows_visible -
            mw->matrix.fixed_rows - mw->matrix.trailing_fixed_rows;
    else if (VERT_ORIGIN(mw) < 0)
        mw->matrix.top_row = 0;
}

/*
 * Utility function to clear a cell so we can draw something new in it.
 * Does not generate expose events on the cell.
 * Does not check if the cell is actually visible before clearing it.
 */
void
xbaeClearCell(mw, row, column)
XbaeMatrixWidget mw;
int row, column;
{
    int x, y;
    Boolean fixed = IS_FIXED(mw, row, column);
    Window win = fixed ? XtWindow(mw) : XtWindow(ClipChild(mw));

    if (!win || mw->matrix.disable_redisplay)
        return;

    xbaeRowColToXY(mw, row, column, &x, &y);

    /*
     * Make sure y coord is valid
     */
#if 0
    if ((win == XtWindow(mw)) &&
        ((y > (int)(CLIP_VERT_VISIBLE_SPACE(mw) +
                    ROW_LABEL_OFFSET(mw) - 1)) ||
         (y < (int)ROW_LABEL_OFFSET(mw))))
        return;
#endif
    XClearArea(XtDisplay(mw), win, x, y, COLUMN_WIDTH(mw, column),
               ROW_HEIGHT(mw), fixed);
}

/*
 * Return True if a row is visible on the screen (not scrolled totally off)
 */
Boolean
xbaeIsRowVisible(mw, row)
XbaeMatrixWidget mw;
int row;
{
    /*
     * If we are not in a fixed row or trailing fixed row,
     * see if we are on the screen vertically
     * (fixed rows are always on the screen)
     */
    if (! IS_FIXED_ROW(mw, row))
    {
        row -= mw->matrix.fixed_rows;
        
        if (row >= VERT_ORIGIN(mw))
        {
            double height = ((double)ClipChild(mw)->core.height /
                             ROW_HEIGHT(mw)) + VERT_ORIGIN(mw);
            if (row < height)
                return True;
            
            if ((int)ClipChild(mw)->core.height >
                (int)TEXT_HEIGHT_OFFSET(mw) &&
                (int)ClipChild(mw)->core.height < ROW_HEIGHT(mw) &&
                row == VERT_ORIGIN(mw))

                return True;
        }
    }
    else
        return True;

    return False;
}

/*
 * Return True if a column is visible on the screen (not scrolled totally off)
 */
Boolean
xbaeIsColumnVisible(mw, column)
XbaeMatrixWidget mw;
int column;
{
    /*
     * If we are not in a fixed column, see if we are on the screen
     * horizontally (fixed columns are always on the screen)
     */
    if (! IS_FIXED_COLUMN(mw, column))
    {
        int x;

        /*
         * Calculate the x endpoints of this column
         */
        x = COLUMN_POSITION(mw, column) -
            COLUMN_POSITION(mw, mw->matrix.fixed_columns);

        /*
         * Check if we are visible horizontally
         */
        if (x + COLUMN_WIDTH(mw, column) > HORIZ_ORIGIN(mw) &&
            x < (int)(ClipChild(mw)->core.width) + HORIZ_ORIGIN(mw))
            return True;
    }
    else
        return True;

    return False;
}

/*
 * Return True if a cell is visible on the screen (not scrolled totally off)
 */
Boolean
xbaeIsCellVisible(mw, row, column)
XbaeMatrixWidget mw;
int row, column;
{
    return xbaeIsRowVisible(mw, row) && xbaeIsColumnVisible(mw, column);
}

/*
 * Scroll a row so it is visible on the screen.
 */
void
xbaeMakeRowVisible(mw, row)
XbaeMatrixWidget mw;
int row;
{
    int rows_visible;
    int value, slider_size, increment, page_increment, vert_value;

    /*
     * If we are in a fixed row, we are already visible.
     */
    if (IS_FIXED_ROW(mw, row))
        return;

    /*
     * Take into account fixed_rows.
     * Calculate the number of rows visible. If less than one full
     * row is visible, use one full row.
     */
    row -= mw->matrix.fixed_rows;
    rows_visible = VISIBLE_HEIGHT(mw) / ROW_HEIGHT(mw);
    if (rows_visible == 0)
        rows_visible = 1;

    /*
     * Figure out the new value of the VSB to scroll this cell
     * onto the screen (the VSB uses row coordinates instead of pixels)
     */
    if (row < VERT_ORIGIN(mw))
        vert_value = row;
    else if (row >= rows_visible + VERT_ORIGIN(mw))
        vert_value = row - rows_visible + 1;
    else
        vert_value = VERT_ORIGIN(mw);

    /*
     * Give the VSB the new value and pass a flag to make it call
     * our scroll callbacks
     */
    if (vert_value != VERT_ORIGIN(mw))
    {
        XmScrollBarGetValues(VertScrollChild(mw), &value,
                             &slider_size, &increment, &page_increment);
        XmScrollBarSetValues(VertScrollChild(mw), vert_value,
                             slider_size, increment, page_increment, True);
    }
}

/*
 * Scroll a column so it is visible on the screen.
 */
void
xbaeMakeColumnVisible(mw, column)
XbaeMatrixWidget mw;
int column;
{
    int value, slider_size, increment, page_increment, x, horiz_value;

    /*
     * If we are in a fixed column, we are already visible.
     */
    if (IS_FIXED_COLUMN(mw, column))
        return;

    /*
     * Calculate the x position of this column
     */
    x = COLUMN_POSITION(mw, column) -
        COLUMN_POSITION(mw, mw->matrix.fixed_columns);

    /*
     * Figure out the new value of the HSB to scroll this cell
     * onto the screen. If the whole cell won't fit, scroll so its
     * left edge is visible.
     */
    if (x < HORIZ_ORIGIN(mw))
        horiz_value = x;
    else if (x + COLUMN_WIDTH(mw, column) >
             VISIBLE_WIDTH(mw) + HORIZ_ORIGIN(mw))
    {
        int off = (x + COLUMN_WIDTH(mw, column)) - (VISIBLE_WIDTH(mw) +
                                                    HORIZ_ORIGIN(mw));

        if (x - off < HORIZ_ORIGIN(mw))
            horiz_value = x;
        else
            horiz_value = HORIZ_ORIGIN(mw) + off;
    }
    else
        horiz_value = HORIZ_ORIGIN(mw);

    /*
     * Give the HSB the new value and pass a flag to make it
     * call our scroll callbacks
     */
    if (horiz_value != HORIZ_ORIGIN(mw))
    {
        XmScrollBarGetValues(HorizScrollChild(mw), &value,
                             &slider_size, &increment, &page_increment);
        XmScrollBarSetValues(HorizScrollChild(mw), horiz_value,
                             slider_size, increment, page_increment, True);
    }
}

/*
 * Scrolls a fixed or non-fixed cell so it is visible on the screen.
 */
void
xbaeMakeCellVisible(mw, row, column)
XbaeMatrixWidget mw;
int row, column;
{
    if (!xbaeIsRowVisible(mw, row))
        xbaeMakeRowVisible(mw, row);
    if (!xbaeIsColumnVisible(mw, column))
        xbaeMakeColumnVisible(mw, column);
}

/*
 * Set the clip_mask in our draw and shadow GCs.  This is necessary for
 * drawing non-fixed column labels and fixed rows.
 */
void
xbaeSetClipMask(mw, clip_reason)
XbaeMatrixWidget mw;
unsigned int clip_reason;
{
    XRectangle r[2];
    int n = 1;
    /*
     * Set new clip reason
     */
    mw->matrix.current_clip = clip_reason;
    
    /*
     * XRectangle enclosing column labels and fixed rows
     */
    if ((CLIP_FIXED_COLUMNS & clip_reason) && mw->matrix.fixed_columns)
    {
        r[0].x = COLUMN_LABEL_OFFSET(mw);
        r[0].width = FIXED_COLUMN_WIDTH(mw);
    }
    else if ((CLIP_TRAILING_FIXED_COLUMNS & clip_reason) &&
             (mw->matrix.trailing_fixed_columns || mw->matrix.fill))
    {
        r[0].x = TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw);
        r[0].width = TRAILING_FIXED_COLUMN_WIDTH(mw);
        if (NEED_HORIZ_FILL(mw))
            r[0].width += FILL_HORIZ_WIDTH(mw);
    }
    else
    {
        r[0].x = FIXED_COLUMN_LABEL_OFFSET(mw);
        r[0].width = ClipChild(mw)->core.width;
        if (NEED_HORIZ_FILL(mw))
            r[0].width += FILL_HORIZ_WIDTH(mw);
    }
        
    if (CLIP_VISIBLE_HEIGHT & clip_reason)
    {
        r[0].y = ROW_LABEL_OFFSET(mw);
        r[0].height = ClipChild(mw)->core.height +
            FIXED_ROW_HEIGHT(mw) + TRAILING_FIXED_ROW_HEIGHT(mw);
        if (NEED_VERT_FILL(mw))
              r[0].height += FILL_VERT_HEIGHT(mw);
    }
    else if ((CLIP_TRAILING_FIXED_ROWS & clip_reason) &&
             (mw->matrix.trailing_fixed_rows || mw->matrix.fill))
    {
         r[0].y = TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
         r[0].height = TRAILING_FIXED_ROW_HEIGHT(mw);
          if (NEED_VERT_FILL(mw))
              r[0].height += FILL_VERT_HEIGHT(mw);
    }
    else if (CLIP_BETWEEN_FIXED_ROWS & clip_reason)
    {
        r[0].y = FIXED_ROW_LABEL_OFFSET(mw);
        r[0].height = ClipChild(mw)->core.height;
    }
    else    /* clip fixed rows & clip_reason */
    {
        r[0].y = HORIZ_SB_OFFSET(mw);
        r[0].height = FIXED_ROW_LABEL_OFFSET(mw);
    }
    if (mw->matrix.row_labels)
    {
        r[1].x = VERT_SB_OFFSET(mw);
        r[1].y = FIXED_ROW_LABEL_OFFSET(mw);
        r[1].width = ROW_LABEL_WIDTH(mw);
        r[1].height = ClipChild(mw)->core.height;
        n = 2;
    }
    /*
     * Reset the clip_mask in our clipping GCs
     */
    XSetClipRectangles(XtDisplay(mw), mw->matrix.cell_grid_line_gc,
                       0, 0, r, n, Unsorted);
    XSetClipRectangles(XtDisplay(mw), mw->matrix.cell_top_shadow_clip_gc,
                       0, 0, r, n, Unsorted);
    XSetClipRectangles(XtDisplay(mw), mw->matrix.cell_bottom_shadow_clip_gc,
                       0, 0, r, n, Unsorted);
    XSetClipRectangles(XtDisplay(mw), mw->matrix.label_clip_gc,
                       0, 0, r, n, Unsorted);
}


/*
 * Get the total pixel width of the non-fixed cell area
 */
void
xbaeGetCellTotalWidth(mw)
XbaeMatrixWidget mw;
{
    int i, columns;

    /*
     * Calculate width of non-fixed cell area.
     */
    columns = TRAILING_HORIZ_ORIGIN(mw);
    for (i = mw->matrix.fixed_columns, mw->matrix.non_fixed_total_width = 0;
         i < columns;
         i++)
        mw->matrix.non_fixed_total_width += COLUMN_WIDTH(mw, i);
}

/*
 * Cache the pixel position of each column
 */
void
xbaeGetColumnPositions(mw)
XbaeMatrixWidget mw;
{
    int i, x;

    for (i = 0, x = 0;
         i < mw->matrix.columns;
         x += COLUMN_WIDTH(mw, i), i++)
        COLUMN_POSITION(mw, i) = x;
}

void
#if NeedFunctionPrototypes
xbaeComputeSize(XbaeMatrixWidget mw, Boolean compute_width,
                Boolean compute_height)
#else
xbaeComputeSize(mw, compute_width, compute_height)
XbaeMatrixWidget mw;
Boolean compute_width;
Boolean compute_height;
#endif
{
    unsigned long full_width = NON_FIXED_TOTAL_WIDTH(mw) +
        FIXED_COLUMN_WIDTH(mw) + TRAILING_FIXED_COLUMN_WIDTH(mw) +
        ROW_LABEL_WIDTH(mw) + 2 * mw->manager.shadow_thickness;
    unsigned long full_height = CELL_TOTAL_HEIGHT(mw) +
        FIXED_ROW_HEIGHT(mw) + TRAILING_FIXED_ROW_HEIGHT(mw) +
        COLUMN_LABEL_HEIGHT(mw) + 2 * mw->manager.shadow_thickness;
    unsigned long width, height;

    /*
     * Calculate our width.
     * If visible_columns is set, then base it on that.
     * Otherwise, if the compute_width flag is set, then we are full width.
     * Otherwise we keep whatever width we are.
     */
    if (mw->matrix.visible_columns)
        width = ROW_LABEL_WIDTH(mw) + 2 * mw->manager.shadow_thickness +
            COLUMN_WIDTH(mw, (mw->matrix.visible_columns +
                              mw->matrix.fixed_columns) - 1) +
            COLUMN_POSITION(mw, mw->matrix.fixed_columns +
                            mw->matrix.visible_columns - 1) +
            TRAILING_FIXED_COLUMN_WIDTH(mw);
    else if (compute_width)
        width = full_width;
    else
        width = mw->core.width;

    /*
     * Calculate our height.
     * If visible_rows is set, then base it on that.
     * Otherwise, if the compute_height flag is set, then we are full height.
     * Otherwise we keep whatever height we are.
     */
    if (mw->matrix.visible_rows)
        height = mw->matrix.visible_rows * ROW_HEIGHT(mw) +
            TRAILING_FIXED_ROW_HEIGHT(mw) + FIXED_ROW_HEIGHT(mw) +
            COLUMN_LABEL_HEIGHT(mw) + 2 * mw->manager.shadow_thickness;
    else if (compute_height)
        height = full_height;
    else
        height = mw->core.height;

    /*
     * Store our calculated size.
     */
    mw->core.width = width;
    mw->core.height = height;

    /*
     * If we are less than full width or our horizontal display policy is
     * constant, then we need an HSB, so increment our height by the size
     * of the HSB (if we are allowed to modify our height and we are allowed
     * to have an HSB).
     */
    if (((width < full_width) ||
         (XmDISPLAY_STATIC == mw->matrix.hsb_display_policy)) &&
        (compute_height || mw->matrix.visible_rows) &&
        (XmDISPLAY_NONE != mw->matrix.hsb_display_policy))
        mw->core.height += HORIZ_SB_HEIGHT(mw);

    /*
     * If we are less than full height or our vertical display policy is
     * constant, then we need a VSB, so increment our width by the size
     * of the VSB (if we are allowed to modify our width and we are allowed
     * to have a VSB).
     */
    if (((height < full_height) ||
         (XmDISPLAY_STATIC == mw->matrix.vsb_display_policy)) &&
        (compute_width || mw->matrix.visible_columns) &&
        (XmDISPLAY_NONE != mw->matrix.vsb_display_policy))
        mw->core.width += VERT_SB_WIDTH(mw);

    /*
     * Save our calculated size for use in our query_geometry method.
     * This is the size we really want to be (not necessarily the size
     * we will end up being).
     */
    mw->matrix.desired_width = mw->core.width;
    mw->matrix.desired_height = mw->core.height;
}

/*
 * Return the length of the longest row label
 */
short
xbaeMaxRowLabel(mw)
XbaeMatrixWidget mw;
{
    int i;
    short max = 0, len;

    /*
     * Determine the length of the longest row label
     */
    for (i = 0; i < mw->matrix.rows; i++)
    {
        len = strlen(mw->matrix.row_labels[i]);
        if (len > max)
            max = len;
    }
    return max;
}

void
xbaeParseColumnLabel(label, lines)
String label;
ColumnLabelLines lines;
{
    char *nl;

    /*
     * First count the number of lines in the label
     */
    lines->lines = 1;
    nl = label;
    while ((nl = strchr(nl, '\n')) != NULL)
    {
        nl++;
        lines->lines++;
    }

    /*
     * Now malloc a lengths array of the correct size.
     */
    lines->lengths = (int *) XtMalloc(lines->lines * sizeof(int));

    /*
     * An entry in the lengths array is the length of that line (substring).
     */

    /*
     * Handle the case of one line (no strchr() needed)
     */
    if (lines->lines == 1)
        lines->lengths[0] = strlen(label);
    else
    {
        int i;

        nl = label;
        i = 0;
        while ((nl = strchr(nl, '\n')) != NULL)
        {
            lines->lengths[i] = nl - label;
            nl++;
            label = nl;
            i++;
        }
        lines->lengths[i] = strlen(label);
    }
}

/*
 * Convert an x/y pixel position to the row/column cell position it picks.
 * 'cell' specifies whether the x/y coord is relative to the fixed cells
 * window or the non-fixed cells window.
 * The coords x,y are adjusted so they are relative to the origin of the
 * picked cell.
 * If we are "out of bounds" on the ``low'' side, go ahead and return False
 * to show an error, but also set the row/column to -1 to indicate this could
 * be a row/column header.  This is currently undocumented behaviour, but
 * may be useful nevertheless.
 */
Boolean
xbaeXYToRowCol(mw, x, y, row, column, cell)
XbaeMatrixWidget mw;
int *x, *y;
int *row, *column;
CellType cell;
{
    Rectangle rect;
    unsigned int inBox = CLIP_NONE;
    Boolean need_vert_dead_space_fill = NEED_VERT_DEAD_SPACE_FILL(mw);
    int horiz_sb_offset = HORIZ_SB_OFFSET(mw);
    int vert_sb_offset = VERT_SB_OFFSET(mw);
    int column_label_height = COLUMN_LABEL_HEIGHT(mw);
    int row_label_offset = ROW_LABEL_OFFSET(mw);
    int row_label_width = ROW_LABEL_WIDTH(mw);
    int fixed_row_label_offset = FIXED_ROW_LABEL_OFFSET(mw);
    int trailing_fixed_row_label_offset = TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
    int trailing_fixed_row_height = TRAILING_FIXED_ROW_HEIGHT(mw);
    int fixed_column_label_offset = FIXED_COLUMN_LABEL_OFFSET(mw);
    int trailing_fixed_column_label_offset =
        TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw);
    int trailing_fixed_column_width = TRAILING_FIXED_COLUMN_WIDTH(mw);
    int column_label_offset = COLUMN_LABEL_OFFSET(mw);
    int row_height = ROW_HEIGHT(mw);
    int unattached_trailing_rows_offset = UNATTACHED_TRAILING_ROWS_OFFSET(mw);
    
    *row = -1;
    *column = -1;

    switch (cell)
    {
    case FixedCell:
        /*
         * Check the various rectangles enclosing the cells in fixed rows
         * or columns.
         *
         * If we don't have fixed rows or columns, then we didn't hit a cell.
         */

        /*
         * Upper left
         */
        if (!inBox && mw->matrix.fixed_columns && mw->matrix.fixed_rows)
        {
            SETRECT(rect,
                    column_label_offset, row_label_offset,
                    fixed_column_label_offset - 1,
                    fixed_row_label_offset - 1);

            if (INBOX(rect, *x, *y)) inBox = CLIP_FIXED_COLUMNS |
                                         CLIP_FIXED_ROWS;
        }

        /*
         * Lower left
         */
        if (!inBox && mw->matrix.fixed_columns &&
            mw->matrix.trailing_fixed_rows)
        {
            SETRECT(rect,
                    column_label_offset,
                    trailing_fixed_row_label_offset,
                    fixed_column_label_offset - 1,
                    trailing_fixed_row_label_offset +
                    trailing_fixed_row_height - 1);

            if (INBOX(rect, *x, *y)) inBox = CLIP_FIXED_COLUMNS |
                                         CLIP_TRAILING_FIXED_ROWS;
        }

        /*
         * Upper right
         */
        if (!inBox && mw->matrix.trailing_fixed_columns &&
            mw->matrix.fixed_rows)
        {
            SETRECT(rect,
                    trailing_fixed_column_label_offset,
                    row_label_offset,
                    trailing_fixed_column_label_offset +
                    trailing_fixed_column_width - 1,
                    fixed_row_label_offset - 1);

            if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_FIXED_COLUMNS |
                                         CLIP_FIXED_ROWS;
        }

        /*
         * Lower right
         */
        if (!inBox && mw->matrix.trailing_fixed_columns &&
            mw->matrix.trailing_fixed_rows)
        {
            SETRECT(rect,
                    trailing_fixed_column_label_offset,
                    trailing_fixed_row_label_offset,
                    trailing_fixed_column_label_offset +
                    trailing_fixed_column_width - 1,
                    trailing_fixed_row_label_offset +
                    trailing_fixed_row_height - 1);

            if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_FIXED_COLUMNS |
                                         CLIP_TRAILING_FIXED_ROWS;
        }

        /*
         * Right (mid)
         */
        if (!inBox && mw->matrix.fixed_columns)
        {
            SETRECT(rect,
                    column_label_offset, fixed_row_label_offset,
                    fixed_column_label_offset - 1,
                    need_vert_dead_space_fill ?
                    unattached_trailing_rows_offset :
                    trailing_fixed_row_label_offset - 1);

            if (INBOX(rect, *x, *y)) inBox = CLIP_FIXED_COLUMNS;
        }

        /*
         * Upper (mid)
         */
        if (!inBox && mw->matrix.fixed_rows)
        {
            SETRECT(rect,
                    fixed_column_label_offset, row_label_offset,
                    trailing_fixed_column_label_offset - 1,
                    fixed_row_label_offset - 1);

            if (INBOX(rect, *x, *y)) inBox = CLIP_FIXED_ROWS;
        }

        /*
         * Left (mid)
         */
        if (!inBox && mw->matrix.trailing_fixed_columns)
        {
            SETRECT(rect,
                    trailing_fixed_column_label_offset,
                    fixed_row_label_offset,
                    trailing_fixed_column_label_offset +
                    trailing_fixed_column_width - 1,
                    need_vert_dead_space_fill ?
                    unattached_trailing_rows_offset :
                    trailing_fixed_row_label_offset - 1);

            if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_FIXED_COLUMNS;
        }

        /*
         * Lower (mid)
         */
        if (!inBox && mw->matrix.trailing_fixed_rows)
        {
            SETRECT(rect,
                    fixed_column_label_offset,
                    trailing_fixed_row_label_offset,
                    trailing_fixed_column_label_offset - 1,
                    trailing_fixed_row_label_offset +
                    trailing_fixed_row_height - 1);

            if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_FIXED_ROWS;
        }

        /*
         * Trailing horizontal fill. This is trickier because
         * even though we're in the fixed cell section, this really
         * might not be a fixed cell.
         */
        if (!inBox && IN_GRID_ROW_MODE(mw) && NEED_HORIZ_FILL(mw))
        {
            int rx = trailing_fixed_column_label_offset +
                trailing_fixed_column_width;

            /*
             * Upper fill
             */
            if (!inBox && mw->matrix.fixed_rows)
            {
                SETRECT(rect,
                        rx, row_label_offset,
                        rx + FILL_HORIZ_WIDTH(mw) - 1,
                        fixed_row_label_offset - 1);

                if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_HORIZ_FILL |
                                             CLIP_FIXED_ROWS;
            }

            /*
             * Lower fill
             */
            if (!inBox && mw->matrix.trailing_fixed_rows)
            {
                SETRECT(rect,
                        rx, trailing_fixed_row_label_offset,
                        rx + FILL_HORIZ_WIDTH(mw) - 1,
                        trailing_fixed_row_label_offset +
                        trailing_fixed_row_height - 1);

                if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_HORIZ_FILL |
                                             CLIP_TRAILING_FIXED_ROWS;
            }

            /*
             * Mid fill
             */
            if (!inBox)
            {
                SETRECT(rect,
                        rx, row_label_offset,
                        rx + FILL_HORIZ_WIDTH(mw) - 1,
                        trailing_fixed_row_label_offset +
                        trailing_fixed_row_height - 1);

                if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_HORIZ_FILL;
            }
        }

        /*
         * Trailing vertical fill
         */
        if (!inBox && IN_GRID_COLUMN_MODE(mw) && NEED_VERT_FILL(mw))
        {
            int ry = trailing_fixed_row_label_offset +
                trailing_fixed_row_height;

            /*
             * Left fill
             */
            if (mw->matrix.fixed_columns)
            {
                SETRECT(rect,
                        column_label_offset, ry,
                        fixed_column_label_offset - 1,
                        ry + FILL_VERT_HEIGHT(mw) - 1);

                if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_VERT_FILL |
                                             CLIP_FIXED_COLUMNS;
            }

            /*
             * Right fill
             */
            if (!inBox && mw->matrix.trailing_fixed_columns)
            {
                SETRECT(rect,
                        trailing_fixed_column_label_offset, ry,
                        trailing_fixed_column_label_offset +
                        trailing_fixed_column_width - 1,
                        ry + FILL_VERT_HEIGHT(mw) - 1);

                if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_VERT_FILL |
                                             CLIP_TRAILING_FIXED_COLUMNS;
            }

            /*
             * Mid fill
             */
            if (!inBox)
            {
                SETRECT(rect,
                        column_label_offset, ry,
                        trailing_fixed_column_label_offset +
                        trailing_fixed_column_width - 1,
                        ry + FILL_VERT_HEIGHT(mw) - 1);

                if (INBOX(rect, *x, *y)) inBox = CLIP_TRAILING_VERT_FILL;
            }
        }

        /*
         * If the point is in this rectangle, calculate the row/column
         * it hits. Otherwise we didn't hit a cell.
         */
        if (inBox)
        {
            /*
             * Translate the point to rect's coord system
             */
            if (mw->matrix.fixed_columns &&
                (CLIP_TRAILING_FIXED_ROWS == inBox ||
                 CLIP_FIXED_ROWS == inBox))
                *x -= column_label_offset;
            else
                *x -= rect.x1;
            *y -= rect.y1;

            /*
             * Convert this point to a row/column.  We need to take into
             * account the scrolling origins depending on which fixed
             * areas the point is located in.
             */
            if (CLIP_TRAILING_VERT_FILL & inBox)
            {
                *row = mw->matrix.rows - 1;
                *y += row_height;
            }
            else
                *row = YtoRow(mw, *y) +
                    ((((CLIP_FIXED_COLUMNS & inBox) ||
                       (CLIP_TRAILING_FIXED_COLUMNS & inBox))) &&
                     !((CLIP_FIXED_ROWS & inBox) ||
                       (CLIP_TRAILING_FIXED_ROWS & inBox))
                     ? mw->matrix.fixed_rows : 0) +
                    ((CLIP_FIXED_ROWS & inBox) ? 0 : VERT_ORIGIN(mw)) +
                    ((CLIP_TRAILING_FIXED_ROWS & inBox) ?
                     TRAILING_VERT_ORIGIN(mw) - VERT_ORIGIN(mw) : 0);

            if ((CLIP_TRAILING_FIXED_COLUMNS & inBox) ||
                (CLIP_TRAILING_HORIZ_FILL & inBox))
                *column = xbaeXtoTrailingCol(mw, *x);
            else
                *column = xbaeXtoCol(
                    mw, *x + ((CLIP_FIXED_COLUMNS & inBox) ? 0 :
                              HORIZ_ORIGIN(mw)));

            /*
             * Sanity check the result, making sure it is in fixed location
             */
            if (((*row < 0) || (*column < 0)) ||
                (((mw->matrix.fixed_rows &&
                   (*row >= (int)mw->matrix.fixed_rows) &&
                   (!mw->matrix.trailing_fixed_rows ||
                    (mw->matrix.trailing_fixed_rows &&
                     (*row < TRAILING_VERT_ORIGIN(mw))))) ||
                  (!mw->matrix.fixed_rows &&
                   mw->matrix.trailing_fixed_rows &&
                   (*row < TRAILING_VERT_ORIGIN(mw)))) &&
                 ((mw->matrix.fixed_columns &&
                   (*column >= (int)mw->matrix.fixed_columns) &&
                   (!mw->matrix.trailing_fixed_columns ||
                    (mw->matrix.trailing_fixed_columns &&
                     (*column < TRAILING_HORIZ_ORIGIN(mw))))) ||
                  (!mw->matrix.fixed_columns &&
                   mw->matrix.trailing_fixed_columns &&
                   (*column < TRAILING_HORIZ_ORIGIN(mw))))))
                return False;

            /*
             * Adjust x,y so they are relative to this cells origin.
             */
            if (CLIP_TRAILING_HORIZ_FILL & inBox)
                *x += COLUMN_WIDTH(mw, *column);
            else
                *x -= COLUMN_POSITION(mw, *column) -
                    (mw->matrix.fixed_columns ||
                     (CLIP_TRAILING_FIXED_COLUMNS & inBox) ?
                     0 : HORIZ_ORIGIN(mw)) -
                    ((CLIP_TRAILING_FIXED_COLUMNS & inBox) ?
                     COLUMN_POSITION(mw, TRAILING_HORIZ_ORIGIN(mw)) : 0);
            if (!(CLIP_TRAILING_VERT_FILL & inBox))
                *y %= row_height;

            return True;
        }
        else
            return False;

        /* NOTREACHED */
        break;

    case RowLabelCell:
        if (!inBox && mw->matrix.row_labels)
        {
            /* Check the various regions of the matrix to make sure we
               get the correct row label */
            if (mw->matrix.fixed_rows)
            {
                SETRECT(rect,
                        vert_sb_offset, row_label_offset,
                        row_label_width + vert_sb_offset,
                        fixed_row_label_offset - 1);
                if (INBOX(rect, *x, *y)) inBox = CLIP_ROW_LABELS |
                                             CLIP_FIXED_ROWS;
            }
            
            if (!inBox && mw->matrix.trailing_fixed_rows)
            {
                SETRECT(rect,
                        vert_sb_offset,
                        trailing_fixed_row_label_offset,
                        row_label_width + vert_sb_offset,
                        trailing_fixed_row_label_offset +
                        trailing_fixed_row_height - 1);

                if (INBOX(rect, *x, *y)) inBox = CLIP_ROW_LABELS |
                                             CLIP_TRAILING_FIXED_ROWS;
            }

            /* check the rows in the non fixed regions */
            if (!inBox)
            {
                SETRECT(rect,
                        vert_sb_offset,
                        fixed_row_label_offset,
                        row_label_width + vert_sb_offset,
                        trailing_fixed_row_label_offset - 1);

                if (INBOX(rect, *x, *y)) inBox = CLIP_ROW_LABELS;
            }
        }            
        
        if (CLIP_ROW_LABELS & inBox)
        {
            *y -= row_label_offset;
            *row = YtoRow(mw, *y);
            *row += (CLIP_FIXED_ROWS & inBox ? 0 : mw->matrix.top_row);
            *row += ((CLIP_TRAILING_FIXED_ROWS & inBox) ?
                     (CELL_TOTAL_HEIGHT(mw) - VISIBLE_HEIGHT(mw)) /
                     row_height - mw->matrix.top_row: 0);
                        
            *column = -1;
        }
        else
        {
            *row = -1;
            *column = -1;
        }

        /* Sanity check - make sure we don't over step the mark */
        if (*row >= mw->matrix.rows || *column >= mw->matrix.columns)
        {
            *row = -1;
            *column = -1;
        }

        return False;

        /*NOTREACHED*/
        break;

    case ColumnLabelCell:
        if (!inBox && mw->matrix.column_labels)
        {
            if (mw->matrix.fixed_columns)
            {
                SETRECT(rect,
                        column_label_offset, horiz_sb_offset,
                        fixed_column_label_offset - 1,
                        column_label_height + horiz_sb_offset);

                if (INBOX(rect, *x, *y)) inBox = CLIP_FIXED_COLUMNS |
                                             CLIP_COLUMN_LABELS;
            }
            if (!inBox && mw->matrix.trailing_fixed_columns)
            {
                SETRECT(rect,
                        trailing_fixed_column_label_offset,
                        horiz_sb_offset,
                        trailing_fixed_column_label_offset +
                        trailing_fixed_column_width - 1,
                        column_label_height + horiz_sb_offset);

                if (INBOX(rect, *x, *y)) inBox = CLIP_COLUMN_LABELS |
                                             CLIP_TRAILING_FIXED_COLUMNS;
            }
            /* check the columns in the non fixed regions */
            if (!inBox)
            {
                SETRECT(rect,
                        fixed_column_label_offset, horiz_sb_offset,
                        trailing_fixed_column_label_offset,
                        column_label_height + horiz_sb_offset);

                if (INBOX(rect, *x, *y)) inBox = CLIP_COLUMN_LABELS;
            }
        }
        
        if (CLIP_COLUMN_LABELS & inBox)
        {
            *x -= column_label_offset;

            if (CLIP_TRAILING_FIXED_COLUMNS & inBox)
                *column = xbaeXtoTrailingCol(mw, *x);
            else
                *column = xbaeXtoCol(mw, *x +
                                     ((CLIP_FIXED_COLUMNS & inBox) ? 0 :
                                      HORIZ_ORIGIN(mw)));            
            *row = -1;
        }
        else
        {
            *row = -1;
            *column = -1;
        }
        /* Sanity check - make sure we don't overstep the mark */
        if (*row >= mw->matrix.rows || *column >= mw->matrix.columns)
        {
            *row = -1;
            *column = -1;
        }

        return False;

        /* NOTREACHED */
        break;

    case NonFixedCell:

        /*
         * Translate the point to take into account fixed rows or columns.
         */
        *x += FIXED_COLUMN_WIDTH(mw);
        *y += FIXED_ROW_HEIGHT(mw);

        /*
         * Convert the new point to a row/column position
         */
        *row = YtoRow(mw, *y + mw->matrix.first_row_offset) + VERT_ORIGIN(mw);
        *column = xbaeXtoCol(mw, *x + HORIZ_ORIGIN(mw));

        /*
         * Sanity check the result
         */
        if (*row >= TRAILING_VERT_ORIGIN(mw) ||
            *column >= TRAILING_HORIZ_ORIGIN(mw) ||
            *row < 0 || *column < 0)
            return False;

        /*
         * Adjust x,y so they are relative to this cell's origin.
         */
        *x -= COLUMN_POSITION(mw, *column) - HORIZ_ORIGIN(mw);
        *y %= row_height;

        return True;

        /* NOTREACHED */
        break;

    default:
        *row = -1;
        *column = -1;
        return False;
    }
}

/*
 * Convert the coordinates in an event to be relative to the Clip
 * window or the Matrix window.  Set the cell to indicate which one.
 * Used by some actions.
 */
/* ARGSUSED */
Boolean
xbaeEventToXY(mw, event, x, y, cell)
XbaeMatrixWidget mw;
XEvent *event;
int *x, *y;
CellType *cell;
{
    switch (event->type)
    {
    case ButtonPress:
    case ButtonRelease:
        *x = event->xbutton.x;
        *y = event->xbutton.y;
        break;
    case KeyPress:
    case KeyRelease:
        *x = event->xkey.x;
        *y = event->xkey.y;
        break;
    case MotionNotify:
        *x = event->xmotion.x;
        *y = event->xmotion.y;
        break;
    default:
        return False;
    }

    if (event->xbutton.subwindow == XtWindow(ClipChild(mw)))
    {
        *cell = NonFixedCell;
        *x -= FIXED_COLUMN_LABEL_OFFSET(mw);
        *y -= FIXED_ROW_LABEL_OFFSET(mw);
    }
    else if (event->xbutton.window == XtWindow(mw))
        if (*x < (int)COLUMN_LABEL_OFFSET(mw) &&
            *x > (int)VERT_SB_OFFSET(mw))
            *cell = RowLabelCell;
        else if (*y < (int)ROW_LABEL_OFFSET(mw) &&
                 *y > (int)HORIZ_SB_OFFSET(mw))
            *cell = ColumnLabelCell;
        else
            *cell = FixedCell;
    else if (event->xbutton.window == XtWindow(ClipChild(mw)))
        *cell = NonFixedCell;
    else if (event->xbutton.window == XtWindow(TextChild(mw)))
    {
        Position tx, ty;

        if (mw->matrix.current_parent == ClipChild(mw))
            *cell = NonFixedCell;
        else if (mw->matrix.current_parent == (Widget)mw)
            *cell = FixedCell;
        else                        /* We're on one of the extra clips */
        {
            *cell = FixedCell;
            *x += mw->matrix.current_parent->core.x;
            *y += mw->matrix.current_parent->core.y;
        }

        XtVaGetValues(TextChild(mw),
                      XmNx, &tx,
                      XmNy, &ty,
                      NULL);
        *x += tx;
        *y += ty;
    }
    else
        return False;

    return True;
}

/*
 * Convert a pixel position to the column it is contained in.
 */
int
xbaeXtoCol(mw, x)
XbaeMatrixWidget mw;
int x;
{
    int i;

    for (i = 0; i < mw->matrix.columns; i++)
        if (COLUMN_POSITION(mw, i) > x)
            return i - 1;
    /*
     * I have seen cases where this function returned mw->matrix.columns
     * causing a crash as array bounds are read - AL
     */
    if (i > mw->matrix.columns)
        return mw->matrix.columns - 1;

    return i - 1;
}

/*
 * Convert a pixel position to the trailing
 * fixed column it is contained in
 */
int
xbaeXtoTrailingCol(mw, x)
XbaeMatrixWidget mw;
int x;
{
    int i;

    x += COLUMN_POSITION(mw, TRAILING_HORIZ_ORIGIN(mw));
    for (i = TRAILING_HORIZ_ORIGIN(mw); i < mw->matrix.columns; i++)
    {
        if (COLUMN_POSITION(mw, i) > x)
            return i - 1;
    }

    return i - 1;
}

/*
 * Convert a row/column cell position to the x/y of its upper left corner
 * wrt the window it will be drawn in (either the matrix window for
 * fixed cells, or the clip window for non-fixed).
 */
void
xbaeRowColToXY(mw, row, column, x, y)
XbaeMatrixWidget mw;
int row;
int column;
int *x;
int *y;
{
    /*
     * If we are in a fixed cell, calculate x/y relative to Matrixs
     * window (take into account labels etc)
     */
    if (IS_FIXED(mw, row, column))
    {
        /*
         * Ignore horiz_origin if we are in a fixed column
         */
        if (IS_LEADING_FIXED_COLUMN(mw, column))
        {
            if (IS_FIXED_ROW(mw, row))
                *x = COLUMN_LABEL_OFFSET(mw) + COLUMN_POSITION(mw, column);
            else
                *x = COLUMN_POSITION(mw, column);        /* LeftClip */
        }
        else if (IS_TRAILING_FIXED_COLUMN(mw, column))
        {
            int m;
            if (IS_FIXED_ROW(mw, row))
                *x = TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw);
            else
                *x = 0;                                        /* RightClip */
            for (m = TRAILING_HORIZ_ORIGIN(mw); m < column; m++)
                *x += COLUMN_WIDTH(mw, m);
        }
        else if (IS_FIXED_ROW(mw, row))
            *x = (COLUMN_POSITION(mw, column) -
                  COLUMN_POSITION(mw, mw->matrix.fixed_columns)) -
                HORIZ_ORIGIN(mw);
        else
            *x = COLUMN_LABEL_OFFSET(mw) +
                COLUMN_POSITION(mw, column) - HORIZ_ORIGIN(mw);

        /*
         * Ignore vert_origin if we are in a fixed row
         */
        if (IS_LEADING_FIXED_ROW(mw, row))
        {
            if (IS_FIXED_COLUMN(mw, column))
                *y = ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) * row;
            else
                *y = ROW_HEIGHT(mw) * row;        /* TopClip */
        }
        else if (IS_TRAILING_FIXED_ROW(mw, row))
        {
            int m;
            if (IS_FIXED_COLUMN(mw, column))
                *y = TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
            else
                *y = 0;                                /* BottomClip */
            for (m = TRAILING_VERT_ORIGIN(mw); m < row; m++)
                *y += ROW_HEIGHT(mw);
        }
        else if (IS_FIXED_COLUMN(mw, column))

            *y = ROW_HEIGHT(mw) * ((row - (int)mw->matrix.fixed_rows) -
                                   VERT_ORIGIN(mw));
        else
            *y = ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) *
                (row - VERT_ORIGIN(mw));
    }

    /*
     * If we are not fixed we must account for fixed rows/columns
     * and scrolling origins.
     */
    else
    {
        *x = (COLUMN_POSITION(mw, column) -
              COLUMN_POSITION(mw, mw->matrix.fixed_columns)) -
            HORIZ_ORIGIN(mw);
        *y = ROW_HEIGHT(mw) * ((row - (int) mw->matrix.fixed_rows) -
                               VERT_ORIGIN(mw));
        *y -= mw->matrix.first_row_offset;
    }
}

#define FIXED_NONE        0
#define FIXED_LEFT        1
#define FIXED_RIGHT        2
#define FIXED_TOP        4
#define FIXED_BOTTOM        8
#define FIXED_TOPLEFT        5
#define FIXED_TOPRIGHT        6
#define FIXED_BOTLEFT        9
#define FIXED_BOTRIGHT        10
/* Gotta love that numbering scheme! - AL */


/*
 * Returns the window on which a cell is displayed, i.e. the matrix, the clip,
 * or one of the extra clips which handle the fixed row/col cells.
 */
Window
xbaeGetCellWindow(mw, w, row, column)
XbaeMatrixWidget mw;
Widget *w;
int row, column;
{
    int posn;
    Window win;

    if (IS_LEADING_FIXED_ROW(mw, row))
        posn = FIXED_TOP;
    else if (IS_TRAILING_FIXED_ROW(mw, row))
        posn = FIXED_BOTTOM;
    else
        posn = FIXED_NONE;
    if (IS_LEADING_FIXED_COLUMN(mw, column))
        posn += FIXED_LEFT;
    else if (IS_TRAILING_FIXED_COLUMN(mw, column))
        posn += FIXED_RIGHT;
    else
        posn += FIXED_NONE;        /* add zero!? */

    switch(posn)
    {
    case FIXED_TOPLEFT:
    case FIXED_TOPRIGHT:
    case FIXED_BOTLEFT:
    case FIXED_BOTRIGHT:
        /* total fixed cell - on parent matrix window */
        *w = (Widget)mw;
        win = XtWindow(mw);
        break;
        
    case FIXED_NONE:
        /* not fixed at all - on clip child */
        *w = ClipChild(mw);
        win = XtWindow(ClipChild(mw));
        break;

    case FIXED_LEFT:
        /* fixed col only - on left clip */
        *w = LeftClip(mw);
        win = XtWindow(LeftClip(mw));
        break;
            
    case FIXED_RIGHT:
        /* fixed trailing col only - on right clip */
        win = XtWindow(RightClip(mw));
        *w = RightClip(mw);
        break;

    case FIXED_TOP:
        /* fixed row only - on top clip */
        win = XtWindow(TopClip(mw));
        *w = TopClip(mw);
        break;

    case FIXED_BOTTOM:
        /* fixed trailing row only - on bottom clip */
        win = XtWindow(BottomClip(mw));
        *w = BottomClip(mw);
        break;
    default:        /* bogus */
        win = (Window)NULL;
        *w = (Widget)NULL;
    }
    return win;
}

void
xbaeCalcVertFill(mw, win, x, y, row, column, ax, ay, width, height)
XbaeMatrixWidget mw;
Window win;
int x;
int y;
int row;
int column;
int *ax;
int *ay;
int *width;
int *height;
{
    *ax = x;
    *width = COLUMN_WIDTH(mw, column);

    if (win == XtWindow(LeftClip(mw)))
    {
        *ax += COLUMN_LABEL_OFFSET(mw);
        *ay = LeftClip(mw)->core.y + LeftClip(mw)->core.height;
    }
    else if (win == XtWindow(RightClip(mw)))
    {
        *ax += TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw);
        *ay = RightClip(mw)->core.y + RightClip(mw)->core.height;
    }
    else if (win == XtWindow(BottomClip(mw)))
    {
        *ax += BottomClip(mw)->core.x;
        *ay = BottomClip(mw)->core.y + BottomClip(mw)->core.height; 
    }
    else if (win == XtWindow(ClipChild(mw)))
    {
        if (XtIsManaged(LeftClip(mw)))
            *ax += FIXED_COLUMN_LABEL_OFFSET(mw);
        else
            *ax += COLUMN_LABEL_OFFSET(mw);
        *ay = ClipChild(mw)->core.y + ClipChild(mw)->core.height;
        if (*ax < (int)COLUMN_LABEL_OFFSET(mw))
        {
            *width += *ax - COLUMN_LABEL_OFFSET(mw);
            *ax = COLUMN_LABEL_OFFSET(mw);
        }
    }
    else                        /* must be in a corner */
        *ay = y;

    *height = MATRIX_VERT_VISIBLE_SPACE(mw) + ROW_LABEL_OFFSET(mw) +
        HORIZ_SB_OFFSET(mw) - *ay;
    /*
     * Unfortunately, on the filled area, we don't have the luxury
     * of the clip widgets to help us out with the edges of the area.
     * Check our width isn't going to draw outside the left or right clip
     */

    if (! IS_FIXED_COLUMN(mw, column))
    {
        if (XtIsManaged(LeftClip(mw)) &&
            *ax < (int)FIXED_COLUMN_LABEL_OFFSET(mw))
        {
            *width -= (FIXED_COLUMN_LABEL_OFFSET(mw) - *ax);
            *ax = FIXED_COLUMN_LABEL_OFFSET(mw);
        }

        if (XtIsManaged(RightClip(mw)) &&
            ((*ax + *width) > (int)RightClip(mw)->core.x))
            *width = RightClip(mw)->core.x - *ax;

        if (win == XtWindow(BottomClip(mw)))
        {
            if ((*ax + *width) >
                 (int)(BottomClip(mw)->core.x + BottomClip(mw)->core.width))
                *width = BottomClip(mw)->core.width +
                    BottomClip(mw)->core.x - *ax;

            if (*ax < (int)COLUMN_LABEL_OFFSET(mw))
            {
                *width += *ax - COLUMN_LABEL_OFFSET(mw);
                *ax = COLUMN_LABEL_OFFSET(mw);
            }
        }

        if ((win == XtWindow(ClipChild(mw))) &&
            ((*ax + *width) >
              (int)(ClipChild(mw)->core.x + ClipChild(mw)->core.width)))
            *width = ClipChild(mw)->core.width +
                ClipChild(mw)->core.x - *ax;
    }
}
    
void
xbaeCalcHorizFill(mw, win, x, y, row, column, ax, ay, width, height)
XbaeMatrixWidget mw;
Window win;
int x;
int y;
int row;
int column;
int *ax;
int *ay;
int *width;
int *height;
{
    *ay = y;
    *height = ROW_HEIGHT(mw);

    if (win == XtWindow(TopClip(mw)))
    {
        *ax = TopClip(mw)->core.x + TopClip(mw)->core.width;
        *ay += ROW_LABEL_OFFSET(mw);
    }
    else if (win == XtWindow(BottomClip(mw)))
    {
        *ax = BottomClip(mw)->core.x + BottomClip(mw)->core.width;
        *ay += TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
    }
    else if (win == XtWindow(RightClip(mw)))
    {
        *ax = RightClip(mw)->core.x + RightClip(mw)->core.width; 
        *ay += RightClip(mw)->core.y;
    }
    else if (win == XtWindow(ClipChild(mw)))
    {
        if (XtIsManaged(TopClip(mw)))
            *ay += FIXED_ROW_LABEL_OFFSET(mw);
        else
            *ay += ROW_LABEL_OFFSET(mw);
        *ax = ClipChild(mw)->core.x + ClipChild(mw)->core.width;
        if (*ay < (int)ROW_LABEL_OFFSET(mw))
            *ay = ROW_LABEL_OFFSET(mw);
    }
    else                        /* must be in a corner */
        *ax = x;

    *width = MATRIX_HORIZ_VISIBLE_SPACE(mw) + COLUMN_LABEL_OFFSET(mw) +
        VERT_SB_OFFSET(mw) - *ax;

    /*
     * Unfortunately, on the filled area, we don't have the luxury
     * of the clip widgets to help us out with the edges of the area.
     * Check our width isn't going to draw outside the left or right clip,
     * or out past our matrix region.
     */

    if (! IS_FIXED_ROW(mw, row))
    {
        if (XtIsManaged(LeftClip(mw)) &&
            (*ay < (int)FIXED_ROW_LABEL_OFFSET(mw)))
        {
            *height -= (FIXED_ROW_LABEL_OFFSET(mw) - *ay);
            *ay = FIXED_ROW_LABEL_OFFSET(mw);
        }

        if (XtIsManaged(RightClip(mw)) &&
            ((*ay + *height) >
              (int)(RightClip(mw)->core.y + RightClip(mw)->core.height)))
            *height = RightClip(mw)->core.height +
                RightClip(mw)->core.y - *ay;

        if ((win == XtWindow(ClipChild(mw))) &&
            ((*ay + *height) >
              (int)(ClipChild(mw)->core.y + ClipChild(mw)->core.height)))
            *height = ClipChild(mw)->core.height +
                ClipChild(mw)->core.y - *ay;
    }
}
