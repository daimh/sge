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
 * MatrixWidget Author: Andrew Wason, Bellcore, aw@bae.bellcore.com
 *
 * $Id: Public.c,v 1.1 2001-07-18 11:06:00 root Exp $
 */

/*
 * Public.c created by Andrew Lister (7 August, 1995)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Xm/Xm.h>
#include <Xm/ScrollBar.h>
#include "MatrixP.h"
#include "Shadow.h"
#include "Draw.h"
#include "ScrollMgr.h"
#include "Actions.h"
#include "Utils.h"
#include "Clip.h"
#include "MCreate.h"

/*
 * Public interface to set_cell method
 */
void
XbaeMatrixSetCell(w, row, column, value)
Widget w;
int row;
int column;
const String value;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
     * Call the set_cell method
     */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.set_cell)
        ((XbaeMatrixWidget)w, row, column, value, True);
}


/*
 * Public interface to edit_cell method
 */
void
XbaeMatrixEditCell(w, row, column)
Widget w;
int row, column;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the edit_cell method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.edit_cell)
        ((XbaeMatrixWidget)w, NULL, row, column, NULL, 0);

    XmProcessTraversal(TextChild(((XbaeMatrixWidget) w)), XmTRAVERSE_CURRENT);
}

/*
 * Public interface to select_cell method
 */
void
XbaeMatrixSelectCell(w, row, column)
Widget w;
int row, column;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the select_cell method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.select_cell)
        ((XbaeMatrixWidget)w, row, column);
}

/*
 * Public interface to select_row method
 */
void
XbaeMatrixSelectRow(w, row)
Widget w;
int row;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the select_row method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.select_row)
        ((XbaeMatrixWidget)w, row);
}

/*
 * Public interface to select_column method
 */
void
XbaeMatrixSelectColumn(w, column)
Widget w;
int column;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the select_column method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.select_column)
        ((XbaeMatrixWidget)w, column);
}

/*
 * Public interface to deselect_all method
 */
void
XbaeMatrixDeselectAll(w)
Widget w;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
     * Call the deselect_all method
     */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.deselect_all)
        ((XbaeMatrixWidget)w);
}

/*
 * Public interface to select_all method
 */
void
XbaeMatrixSelectAll(w)
Widget w;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
     * Call the deselect_all method
     */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.select_all)
        ((XbaeMatrixWidget)w);
}

/*
 * Public interface to deselect_cell method
 */
void
XbaeMatrixDeselectCell(w, row, column)
Widget w;
int row;
int column;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
     * Call the deselect_cell method
     */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.deselect_cell)
        ((XbaeMatrixWidget)w, row, column);
}

/*
 * Public interface to deselect_row method
 */
void
XbaeMatrixDeselectRow(w, row)
Widget w;
int row;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the deselect_row method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.deselect_row)
        ((XbaeMatrixWidget)w, row);
}

/*
 * Public interface to deselect_column method
 */
void
XbaeMatrixDeselectColumn(w, column)
Widget w;
int column;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the deselect_column method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.deselect_column)
        ((XbaeMatrixWidget)w, column);
}

/*
 * Public interface to get_cell method
 */
String
XbaeMatrixGetCell(w, row, column)
Widget w;
int row, column;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
     * Call the get_cell method
     */
    return (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.get_cell)
        ((XbaeMatrixWidget)w, row, column);
}

/*
 * Public interface to commit_edit method
 */
Boolean
#if NeedFunctionPrototypes
XbaeMatrixCommitEdit(Widget w, Boolean unmap)
#else
XbaeMatrixCommitEdit(w, unmap)
Widget w;
Boolean unmap;
#endif
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the commit_edit method
   */
    return (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.commit_edit)
        ((XbaeMatrixWidget)w, NULL, unmap);
}

/*
 * Public interface to cancel_edit method
 */
void
#if NeedFunctionPrototypes
XbaeMatrixCancelEdit(Widget w, Boolean unmap)
#else
XbaeMatrixCancelEdit(w, unmap)
Widget w;
Boolean unmap;
#endif
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);
    
    /*
     * Call the cancel_edit method
     */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.cancel_edit)
        ((XbaeMatrixWidget)w, unmap);
}

/*
 * Public interface to add_rows method
 */
void
XbaeMatrixAddRows(w, position, rows, labels, colors, num_rows)
Widget w;
int position;
String *rows;
String *labels;
Pixel *colors;
int num_rows;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the add_rows method
   */
    (*((XbaeMatrixWidgetClass)XtClass(w))->matrix_class.add_rows)
        ((XbaeMatrixWidget)w, position, rows, labels, colors, NULL, num_rows);
}

/*
 * Public interface to delete_rows method
 */
void
XbaeMatrixDeleteRows(w, position, num_rows)
Widget w;
int position;
int num_rows;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
     * Call the delete_rows method
     */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.delete_rows)
        ((XbaeMatrixWidget)w, position, num_rows);
}

/*
 * Public interface to add_columns method
 */
void
XbaeMatrixAddColumns(w, position, columns, labels, widths, max_lengths,
                     alignments, label_alignments, colors, num_columns)
Widget w;
int position;
String *columns;
String *labels;
short *widths;
int *max_lengths;
unsigned char *alignments;
unsigned char *label_alignments;
Pixel *colors;
int num_columns;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the add_columns method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.add_columns)
        ((XbaeMatrixWidget)w, position, columns, labels, widths,
         max_lengths, alignments, label_alignments, colors, NULL, num_columns);
}

/*
 * Public interface to delete_columns method
 */
void
XbaeMatrixDeleteColumns(w, position, num_columns)
Widget w;
int position;
int num_columns;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
     * Call the delete_columns method
     */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.delete_columns)
        ((XbaeMatrixWidget)w, position, num_columns);
}

/*
 * Public interface to set_row_colors method
 */
void
XbaeMatrixSetRowColors(w, position, colors, num_colors)
Widget w;
int position;
Pixel *colors;
int num_colors;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the set_row_colors method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.set_row_colors)
        ((XbaeMatrixWidget)w, position, colors, num_colors, False);
}

/*
 * Public interface to set_column_colors method
 */
void
XbaeMatrixSetColumnColors(w, position, colors, num_colors)
Widget w;
int position;
Pixel *colors;
int num_colors;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the set_column_colors method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.set_column_colors)
        ((XbaeMatrixWidget)w, position, colors, num_colors, False);
}

/*
 * Public interface to set_cell_color method
 */
void
XbaeMatrixSetCellColor(w, row, column, color)
Widget w;
int row;
int column;
Pixel color;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the set_cell_color method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.set_cell_color)
        ((XbaeMatrixWidget)w, row, column, color, False);
}

/*
 * Public interface to set_row_colors method
 */
void
XbaeMatrixSetRowBackgrounds(w, position, colors, num_colors)
Widget w;
int position;
Pixel *colors;
int num_colors;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the set_row_colors method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.set_row_colors)
        ((XbaeMatrixWidget)w, position, colors, num_colors, True);
}

/*
 * Public interface to set_column_colors method
 */
void
XbaeMatrixSetColumnBackgrounds(w, position, colors, num_colors)
Widget w;
int position;
Pixel *colors;
int num_colors;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the set_column_colors method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.set_column_colors)
        ((XbaeMatrixWidget)w, position, colors, num_colors, True);
}

/*
 * Public interface to set_cell_color method
 */
void
XbaeMatrixSetCellBackground(w, row, column, color)
Widget w;
int row;
int column;
Pixel color;
{
    /*
     * Make sure w is a Matrix or a subclass
     */
    XtCheckSubclass(w, xbaeMatrixWidgetClass, NULL);

    /*
   * Call the set_cell_color method
   */
    (*((XbaeMatrixWidgetClass) XtClass(w))->matrix_class.set_cell_color)
        ((XbaeMatrixWidget)w, row, column, color, True);
}

/*
 * Help the user know what row & column he is in given an x & y (via an event).
 * Return True on success, False on failure.
 */
int
XbaeMatrixGetEventRowColumn(w, event, row, column)
Widget w;
XEvent * event;
int *row;
int *column;
{
    XbaeMatrixWidget mw;
    int x, y;
    CellType cell;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    /* Convert the event to the correct XY for the matrix widget. */
    mw = (XbaeMatrixWidget) w;
    if (!xbaeEventToXY(mw, event, &x, &y, &cell))
        return False;

    /* Convert the point to a row,column. If it does not pick a valid cell,
       then return. */
    if (!xbaeXYToRowCol(mw, &x, &y, row, column, cell))
        return False;

    return True;
}

/*
 * Public interface for xbaeEventToXY()
 */
Boolean
XbaeMatrixEventToXY(w, event, x, y)
Widget w;
XEvent *event;  
int *x;
int *y;
{
    XbaeMatrixWidget mw;
    CellType cell;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    mw = (XbaeMatrixWidget)w;

    if (!xbaeEventToXY(mw, event, x, y, &cell))
        return False;

    return True;
}

/*
 * Public interface for xbaeRowColToXY().  From Philip Aston
 * (philipa@parallax.co.uk)
 */
Boolean
XbaeMatrixRowColToXY(w, row, column, x, y)
Widget w;
int row;
int column;
int *x;
int *y;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    mw = (XbaeMatrixWidget)w;

    xbaeRowColToXY(mw, row, column, x, y);

    return True;
}

/*
 * Help the programmer to know what row & column we are currently at.
 * Set the row & column to -1 if bad widget.  Maybe the program will core. :)
 */
void
XbaeMatrixGetCurrentCell(w, row, column)
Widget w;
int *row;
int *column;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        *row = *column = -1;
    else
    {
        mw = (XbaeMatrixWidget)w;
        *row = mw->matrix.current_row;
        *column = mw->matrix.current_column;
    }
}


/*
 * Allow the programmer to call the Expose method directly if he feels
 * that it is really needed.
 */
void
XbaeMatrixRefresh(w)
Widget w;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget)w;
    int x, y;
    
    if (mw->matrix.disable_redisplay)
        return;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass) || !XtIsRealized((Widget)mw))
        return;

    /*
     * We're about to flush scroll exposure events, so reset the
     * scroll managers.
     */
    xbaeSmDestroyScrollMgr(mw->matrix.matrix_scroll_mgr);
    xbaeSmDestroyScrollMgr(mw->matrix.clip_scroll_mgr);

    mw->matrix.matrix_scroll_mgr = xbaeSmCreateScrollMgr();
    mw->matrix.clip_scroll_mgr = xbaeSmCreateScrollMgr();
 
    /*
     * Don't respond to exposures.
     */
    mw->matrix.disable_redisplay = 1;
 
    /*
     * Flush pending expose events.
     */
    XmUpdateDisplay(w);
 
    /*
     * Respond to exposures.
     */
    mw->matrix.disable_redisplay = 0;
 
    /*
     * Generate expose events on the Matrix to force the redrawing.
     */
    x = 0;
    y = 0;
    if (mw->matrix.column_labels)
        XClearArea(XtDisplay(mw), XtWindow(mw), x, y, mw->core.width,
                   ROW_LABEL_OFFSET(mw), True);
    y += ROW_LABEL_OFFSET(mw);
    if (mw->matrix.row_labels)
        XClearArea(XtDisplay(mw), XtWindow(mw), x, y,
                   COLUMN_LABEL_OFFSET(mw), mw->core.height - x, True);
    x += COLUMN_LABEL_OFFSET(mw);
    if (mw->matrix.fixed_rows)
    {
        /* Clear the top clip completely */
        XClearArea(XtDisplay(mw), XtWindow(TopClip(mw)), 0, 0, 0, 0, True);

        /* Don't forget the corner areas! */
        if (mw->matrix.fixed_columns)
            XClearArea(XtDisplay(mw), XtWindow(mw), x, y,
                       LeftClip(mw)->core.width, TopClip(mw)->core.height,
                       True);
        if (mw->matrix.trailing_fixed_columns)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       RightClip(mw)->core.x, y, RightClip(mw)->core.width,
                       TopClip(mw)->core.height, True);
    }
    if (mw->matrix.fixed_columns)
        XClearArea(XtDisplay(mw), XtWindow(LeftClip(mw)), 0, 0, 0, 0, True);
    if (mw->matrix.trailing_fixed_columns)
        XClearArea(XtDisplay(mw), XtWindow(RightClip(mw)), 0, 0, 0, 0, True);
    if (mw->matrix.trailing_fixed_rows)
    {
        XClearArea(XtDisplay(mw), XtWindow(BottomClip(mw)), 0, 0, 0, 0, True);
        if (mw->matrix.fixed_columns)
            XClearArea(XtDisplay(mw), XtWindow(mw), x, BottomClip(mw)->core.y,
                       LeftClip(mw)->core.width, BottomClip(mw)->core.height,
                       True);
        if (mw->matrix.trailing_fixed_columns)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       RightClip(mw)->core.x, BottomClip(mw)->core.y,
                       RightClip(mw)->core.width, BottomClip(mw)->core.height,
                       True);
    }
    /*
     * The areas to the right and bottom of the matrix also need to be
     * exposed. First to do is the right hand side.
     */
    x = COLUMN_POSITION(mw, mw->matrix.columns-1) +
        COLUMN_WIDTH(mw, mw->matrix.columns-1);
    XClearArea(XtDisplay(mw), XtWindow(mw), x, 0, mw->core.width - x,
               mw->core.height, True);
    y = ClipChild(mw)->core.y + ClipChild(mw)->core.height +
        TRAILING_FIXED_ROW_HEIGHT(mw);
    XClearArea(XtDisplay(mw), XtWindow(mw), 0, y, mw->core.width,
               mw->core.height - y, True);
    XbaeClipRedraw(ClipChild(mw));
}

/*
 * Public interface for redrawing one cell
 */
void
XbaeMatrixRefreshCell(w, row, column)
Widget w;
int row;
int column;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget)w;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    if (xbaeIsCellVisible(mw, row, column))
    {
        /* this doesn't seem to be necessary -cg 16/7/99 */
        /*xbaeClearCell(mw, row, column);*/
        xbaeDrawCell(mw, row, column);
    }
}

/*
 * Redraw an entire column
 */
void
XbaeMatrixRefreshColumn(w, column)
Widget w;
int column;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget)w;
    int row;
    int found = 0;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    /* we attempt to be mildly efficient about this */
    if (xbaeIsColumnVisible(mw, column))
    {
        /* fixed always visible */
        for (row = 0; row < mw->matrix.fixed_rows; row++)
            xbaeDrawCell(mw, row, column);
        /* now the scrollable clip */
        for (; row < mw->matrix.rows - mw->matrix.trailing_fixed_rows; row++)
            if (xbaeIsRowVisible(mw, row))
            {
                found = 1;
                xbaeDrawCell(mw, row, column);
            }
            else if (found)
                break;  /* came to the end of the clip */
        /* and finally trailing fixed are always visible */
        for (row = mw->matrix.rows - mw->matrix.trailing_fixed_rows;
             row < mw->matrix.rows; row++)
            xbaeDrawCell(mw, row, column);
    }
}

/*
 * Redraw an entire row
 */
void
XbaeMatrixRefreshRow(w, row)
Widget w;
int row;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget)w;
    int column;
    int found = 0;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    /* we attempt to be mildly efficient about this */
    if (xbaeIsRowVisible(mw, row))
    {
        /* fixed always visible */
        for (column = 0; column < mw->matrix.fixed_columns; column++)
            xbaeDrawCell(mw, row, column);
        /* now the scrollable clip */
        for (; column < mw->matrix.columns - mw->matrix.trailing_fixed_columns;
             column++)
            if (xbaeIsColumnVisible(mw, column))
            {
                found = 1;
                xbaeDrawCell(mw, row, column);
            }
            else if (found)
                break;  /* came to the end of the clip */
        /* and finally trailing fixed are always visible */
        for (column = mw->matrix.columns - mw->matrix.trailing_fixed_columns;
             column < mw->matrix.columns; column++)
            xbaeDrawCell(mw, row, column);
    }
}

/*
 *  XbaeMatrixVisibleRows()
 *
 *  This routine returns the number of rows that are visible in the matrix.
 *
 *  D. Craig Wilson  5-MAY-1995
 *      - Cloned from the local "xbaeAdjustTopRow(mw)" routine.
 */
int
XbaeMatrixVisibleRows(w)
Widget w;
{
    XbaeMatrixWidget    matrix = (XbaeMatrixWidget) w;

    int rows_visible = VISIBLE_HEIGHT(matrix) / ROW_HEIGHT(matrix);

    /*
     *  If we have less than one full row visible, then count it as a full row.
     */
    if (rows_visible == 0)
        rows_visible = 1;

    /*
     *  rows_visible might be inaccurate since Clip may not have been resized.
     *  Test this routine and see if we need to call XbaeMatrixRefresh() to
     *  ensure accuracy.
     */
    else if (rows_visible > matrix->matrix.rows)
        rows_visible = matrix->matrix.rows;

    return rows_visible;

} /* XbaeMatrixVisibleRows */



/*
 *  XbaeMatrixVisibleColumns()
 *
 *  This routine returns the number of columns that are visible in the matrix.
 *
 *  D. Craig Wilson  5-MAY-1995
 *      - Cloned from the local "xbaeAdjustTopRow(mw)" routine.
 */
int
XbaeMatrixVisibleColumns (w)
Widget w;
{
    XbaeMatrixWidget    matrix = (XbaeMatrixWidget)w;

    int left_column;
    int right_column;

    xbaeGetVisibleColumns(matrix, &left_column, &right_column);

    return right_column - left_column + 1;

} /* XbaeMatrixVisibleColumns */

/*
 * Get per-cell user data
 */
XtPointer
XbaeMatrixGetCellUserData(w, row, column)
Widget w;
int row;
int column;
{
    XbaeMatrixWidget mw;
    XtPointer data;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return NULL;

    mw = (XbaeMatrixWidget) w;
    if (mw->matrix.cell_user_data)
        data = mw->matrix.cell_user_data[row][column];
    else
        return NULL;

    return data;
}

/*
 * Set per-cell user data
 */
void
XbaeMatrixSetCellUserData(w, row, column, data)
Widget w;
int row;
int column;
XtPointer data;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    mw = (XbaeMatrixWidget) w;
    if (! mw->matrix.cell_user_data)
    {
        XtPointer **copy;
        register int i;

        copy = (XtPointer **) XtMalloc(mw->matrix.rows * sizeof(XtPointer*));

        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (XtPointer*) XtCalloc(mw->matrix.columns,
                                            sizeof(XtPointer));

        mw->matrix.cell_user_data = copy;
    }

    mw->matrix.cell_user_data[row][column] = data;
    return;
}

/*
 * Get per-row user data
 */
XtPointer
XbaeMatrixGetRowUserData(w, row)
Widget w;
int row;
{
    XbaeMatrixWidget mw;
    XtPointer data;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return NULL;

    mw = (XbaeMatrixWidget) w;
    if (mw->matrix.row_user_data)
        data = mw->matrix.row_user_data[row];
    else
        return NULL;

    return data;
}


/*
 * Set per-row user data
 */
void
XbaeMatrixSetRowUserData(w, row, data)
Widget w;
int row;
XtPointer data;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    mw = (XbaeMatrixWidget) w;
    if (! mw->matrix.row_user_data)
    {
        XtPointer *copy;

        copy = (XtPointer *) XtCalloc(mw->matrix.rows, sizeof(XtPointer));

        mw->matrix.row_user_data = copy;
    }

    mw->matrix.row_user_data[row]= data;
    return;
}

/*
 * Get per-column user data
 */
XtPointer
XbaeMatrixGetColumnUserData(w, column)
Widget w;
int column;
{
    XbaeMatrixWidget mw;
    XtPointer data;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return NULL;

    mw = (XbaeMatrixWidget) w;
    if (mw->matrix.column_user_data)
        data = mw->matrix.column_user_data[column];
    else
        return NULL;

    return data;
}

/*
 * Set per-column user data
 */
void
XbaeMatrixSetColumnUserData(w, column, data)
Widget w;
int column;
XtPointer data;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    mw = (XbaeMatrixWidget) w;
    if (! mw->matrix.column_user_data)
    {
        XtPointer *copy;

        copy = (XtPointer *) XtCalloc(mw->matrix.columns, sizeof(XtPointer));

        mw->matrix.column_user_data = copy;
    }

    mw->matrix.column_user_data[column]= data;
    return;
}

#if CELL_WIDGETS
/*
 * Set per-cell widget
 */
void
XbaeMatrixSetCellWidget(w, row, column, widget)
Widget w;
int row;
int column;
Widget widget;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    mw = (XbaeMatrixWidget) w;

    if (! mw->matrix.cell_widgets)
    {
        Widget **copy;
        register int i;

        copy = (Widget **) XtMalloc(mw->matrix.rows * sizeof(Widget*));

        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (Widget*) XtCalloc(mw->matrix.columns, sizeof(Widget));

        mw->matrix.cell_widgets = copy;
    }

    mw->matrix.cell_widgets[row][column] = widget;
    return;
}
#endif

Boolean
XbaeMatrixIsRowSelected(w, row)
Widget w;
int row;
{
    int col;
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    mw = (XbaeMatrixWidget) w;

    if (row < 0 || row >= mw->matrix.rows)
    {
        XtAppContext appcontext = XtWidgetToApplicationContext(w);
        XtAppError(appcontext,
                   "Invalid row passed to XbaeMatrixIsRowSelected()");
        return False;
    }

    if (!mw->matrix.selected_cells)
        return False;

    /*
     * Check all the cells in the row
     */
    for (col = 0 ; col < mw->matrix.columns ; col++)
        if (! mw->matrix.selected_cells[row][col])
            return False;

    /*
     * Return success
     */
    return True;
}

Boolean
XbaeMatrixIsColumnSelected(w, col)
Widget w;
int col;
{
    int row;
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    mw = (XbaeMatrixWidget) w;

    if (col < 0 || col >= mw->matrix.columns)
    {
        XtAppContext appcontext = XtWidgetToApplicationContext(w);
        XtAppError(appcontext,
                   "Invalid column passed to XbaeMatrixIsColumnSelected()");
        return False;
    }

    if (!mw->matrix.selected_cells)
        return False;

    /*
     * Check all the cells in the row
     */
    for (row = 0 ; row < mw->matrix.rows ; row++)
        if (! mw->matrix.selected_cells[row][col])
            return False;

    /*
     * Return success
     */
    return True;
}

Boolean
XbaeMatrixIsCellSelected(w, row, column)
Widget w;
int row;
int column;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    mw = (XbaeMatrixWidget) w;

    if (column < 0 || column >= mw->matrix.columns || row < 0 ||
        row >= mw->matrix.rows)
    {
        XtAppContext appcontext = XtWidgetToApplicationContext(w);
        XtAppError(
            appcontext,
            "Invalid coordinates passed to XbaeMatrixIsCellSelected()");
        return False;
    }

    if (!mw->matrix.selected_cells)
        return False;

    if (! mw->matrix.selected_cells[row][column])
        return False;

    /*
     * Return success
     */
    return True;
}

int
XbaeMatrixFirstSelectedRow(w)
Widget w;
{
    int i;
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return -1;

    mw = (XbaeMatrixWidget) w;

    if (!mw->matrix.selected_cells)
        return -1;

    /*
     * Linear search for first selected
     */
    for (i = 0 ; i < mw->matrix.rows ; i++)
        if (XbaeMatrixIsRowSelected(w, i))
            return i;
    /*
     * No selection - return an invalid row id
     */
    return -1;
}

int
XbaeMatrixFirstSelectedColumn(w)
Widget w;
{
    int i;
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return -1;

    mw = (XbaeMatrixWidget) w;

    if (!mw->matrix.selected_cells)
        return -1;

    /*
     * Linear search for first selected
     */
    for (i = 0 ; i < mw->matrix.columns ; i++)
        if (XbaeMatrixIsColumnSelected(w, i))
            return i;
    /*
     * No selection - return an invalid row id
     */
    return -1;
}

void
XbaeMatrixFirstSelectedCell(w, row, column)
Widget w;
int *row;
int *column;
{
    int i, j;
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
    {
        *row = *column = -1;
        return;
    }

    mw = (XbaeMatrixWidget) w;

    if (!mw->matrix.selected_cells)
    {
        *row = -1;
        *column = -1;
        return;
    }

    for (i = 0; i < mw->matrix.rows; i++)
        for (j = 0; j < mw->matrix.columns; j++)
            if (mw->matrix.selected_cells[ i ][ j ])
            {
                *row = i;
                *column = j;
                return;
            }
    *row = *column = -1;
}

int
XbaeMatrixGetNumSelected(w)
Widget w;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return 0;

    mw = (XbaeMatrixWidget) w;

    return mw->matrix.selected_cells ? mw->matrix.num_selected_cells : 0;
}


int
XbaeMatrixNumColumns(w)
Widget w;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return 0;

    mw = (XbaeMatrixWidget) w;

    return mw->matrix.columns;
}


int
XbaeMatrixNumRows(w)
Widget w;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return 0;

    mw = (XbaeMatrixWidget) w;

    return mw->matrix.rows;
}

#if XmVersion >= 1002
void
XbaeMatrixUnhighlightAll(w)
Widget w;
{
    XbaeMatrixWidget mw;
    int row, column;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;
    
    if (!mw->matrix.highlighted_cells)
        return;

    mw->matrix.highlight_location = UnhighlightAll;

    for (row = 0; row < mw->matrix.rows; row++)
    {
        for (column = 0; column < mw->matrix.columns; column++)
        {
            /*
             * If the cell is visible and highlighted
             */        
            if (mw->matrix.highlighted_cells[row][column] &&
                xbaeIsCellVisible(mw, row, column))
                xbaeDrawCell(mw, row, column);
            mw->matrix.highlighted_cells[row][column] = HighlightNone;
        }
    }

    mw->matrix.highlight_location = HighlightNone;
}


void
XbaeMatrixHighlightCell(w, row, column)
Widget w;
int row;
int column;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;

    if (row >= mw->matrix.rows || row < 0 ||
        column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "highlightCell", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row or column out of bounds for HighlightCell.",
            NULL, 0);
        return;
    }

    /*
     * Scroll the cell onto the screen
     */
    if (mw->matrix.scroll_select)
        xbaeMakeCellVisible(mw, row, column);

    if (!mw->matrix.highlighted_cells)
        xbaeCopyHighlightedCells(mw);

    /*
     * Establish location -- must be after scroll,
     * otherwise may not redraw properly
     */
    mw->matrix.highlight_location = HighlightCell;

    /*
     * If the cell is not already highlighted
     */
    if (!(mw->matrix.highlighted_cells[row][column] & HighlightCell))
    {
        mw->matrix.highlighted_cells[row][column] |= HighlightCell;

        /*
         * Only redraw if cell is visible
         */
        if (xbaeIsCellVisible(mw, row, column))
        {
            int x, y;
            Window win;

            /*
             * Get the correct window
             */
            win = xbaeGetCellWindow(mw, &w, row, column);

            /*
             * Convert row,column coordinates to the
             * coordinates relative to the correct window
             */
            xbaeRowColToXY(mw, row, column, &x, &y);

            DRAW_HIGHLIGHT(XtDisplay(mw), win, 
                           mw->manager.highlight_GC,
                           x + mw->matrix.cell_shadow_thickness,
                           y + mw->matrix.cell_shadow_thickness,
                           COLUMN_WIDTH(mw, column) -
                           (2 * mw->matrix.cell_shadow_thickness),
                           ROW_HEIGHT(mw) -
                           (2 * mw->matrix.cell_shadow_thickness),
                           mw->matrix.cell_highlight_thickness,
                           LineSolid);
        }
    }
    mw->matrix.highlight_location = HighlightNone;
}


void
XbaeMatrixUnhighlightCell(w, row, column)
Widget w;
int row;
int column;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;

    if (row >= mw->matrix.rows || row < 0 ||
        column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "unhighlightCell", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row or column out of bounds for UnhighlightCell.",
            NULL, 0);
        return;
    }

    if (!mw->matrix.highlighted_cells)
        return;
    
    mw->matrix.highlight_location = UnhighlightCell;
    
    if (xbaeIsCellVisible(mw, row, column))
        xbaeDrawCell(mw, row, column);

    mw->matrix.highlighted_cells[row][column] &= ~HighlightCell;

    mw->matrix.highlight_location = HighlightNone;
}


void
XbaeMatrixHighlightRow(w, row)
Widget w;
int row;
{
    XbaeMatrixWidget mw;
    int j, lc, rc;
    Boolean visible;
    unsigned char highlight;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;

    if (row >= mw->matrix.rows || row < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "highlightRow", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row out of bounds for HighlightRow.",
            NULL, 0);
        return;
    }

    if (!mw->matrix.highlighted_cells)
        xbaeCopyHighlightedCells(mw);

    /*
     * Scroll the row onto the screen
     */
    if (mw->matrix.scroll_select)
        xbaeMakeRowVisible(mw, row);

    /*
     * Establish location -- must be after scroll,
     * otherwise may not redraw properly
     */
    mw->matrix.highlight_location = HighlightRow;

    /*
     * For each cell in the row, if the cell is not already highlighted,
     * highlight it and redraw it if it is visible
     */
     
    visible = xbaeIsRowVisible(mw, row);
    xbaeGetVisibleColumns(mw, &lc, &rc);
    highlight = (IN_GRID_ROW_MODE(mw) ? HighlightRow : HighlightOther);

    for (j = 0; j < mw->matrix.columns; j++)
    {
        if (!(mw->matrix.highlighted_cells[row][j] & highlight))
        {
            mw->matrix.highlighted_cells[row][j] |= highlight;
                    
            if (visible)
                xbaeDrawCell(mw, row, j);
        }
    }

    mw->matrix.highlight_location = HighlightNone;
}


void
XbaeMatrixUnhighlightRow(w, row)
Widget w;
int row;
{
    XbaeMatrixWidget mw;
    int column, lc, rc;
    Boolean visible;
    unsigned char highlight;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;

    if (row >= mw->matrix.rows || row < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "highlightRow", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row out of bounds for UnhighlightRow.",
            NULL, 0);
        return;
    }

    if (!mw->matrix.highlighted_cells)
        return;
    
    mw->matrix.highlight_location = UnhighlightRow;

    visible = xbaeIsRowVisible(mw, row);

    /*
     * For each cell in the row, if the cell is highlighted,
     * unhighlight it and redraw it if it is visible
     */
     
    xbaeGetVisibleColumns(mw, &lc, &rc);
    highlight = (IN_GRID_ROW_MODE(mw) ? HighlightRow : HighlightOther);
    for (column = 0; column < mw->matrix.columns; column++)
    {
        if (mw->matrix.highlighted_cells[row][column] & highlight)
        {
            if (visible)
                xbaeDrawCell(mw, row, column);
            mw->matrix.highlighted_cells[row][column] &= ~highlight;
        }
    }

    mw->matrix.highlight_location = HighlightNone;
}


void
XbaeMatrixHighlightColumn(w, column)
Widget w;
int column;
{
    XbaeMatrixWidget mw;
    int row, tr, br;
    Boolean visible;
    unsigned char highlight;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;

    if (column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "highlightColumn", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Column out of bounds for HighlightColumn.",
            NULL, 0);
        return;
    }
    
    if (!mw->matrix.highlighted_cells)
        xbaeCopyHighlightedCells(mw);
    /*
     * Scroll the row onto the screen
     */
    if (mw->matrix.scroll_select)
        xbaeMakeColumnVisible(mw, column);

    mw->matrix.highlight_location = HighlightColumn;
    
    /*
     * For each cell in the column, if the cell is not already highlighted,
     * highlight it and redraw it if it is visible
     */
     
    visible = xbaeIsColumnVisible(mw, column);
    xbaeGetVisibleRows(mw, &tr, &br);
    highlight =        (IN_GRID_COLUMN_MODE(mw) ? HighlightColumn : HighlightOther);
    for (row = 0; row < mw->matrix.rows; row++)
    {
        if (!(mw->matrix.highlighted_cells[row][column] & highlight))
        {
            mw->matrix.highlighted_cells[row][column] |= highlight;
            
            if (visible)
                xbaeDrawCell(mw, row, column);
        }
    }
    mw->matrix.highlight_location = HighlightNone;
}


void
XbaeMatrixUnhighlightColumn(w, column)
Widget w;
int column;
{
    XbaeMatrixWidget mw;
    int row, tr, br;
    Boolean visible;
    unsigned char highlight;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;

    if (column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "highlightColumn", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Column out of bounds for UnhighlightColumn.",
            NULL, 0);
        return;
    }

    if (!mw->matrix.highlighted_cells)
        return;
    
    mw->matrix.highlight_location = UnhighlightColumn;

    visible = xbaeIsColumnVisible(mw, column);

    /*
     * For each cell in the row, if the cell is highlighted,
     * unhighlight it and redraw it if it is visible.
     */
    xbaeGetVisibleRows(mw, &tr, &br);
    highlight = (IN_GRID_COLUMN_MODE(mw) ? HighlightColumn : HighlightOther);
    for (row = 0; row < mw->matrix.rows; row++)
    {
        if (mw->matrix.highlighted_cells[row][column] & highlight)
        {
            if (visible)
                xbaeDrawCell(mw, row, column);
            mw->matrix.highlighted_cells[row][column] &= ~highlight;
        }
    }
    mw->matrix.highlight_location = HighlightNone;
}
#endif

void
XbaeMatrixDisableRedisplay(w)
Widget w;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;

    mw->matrix.disable_redisplay++ ;
}


void
#if NeedFunctionPrototypes
XbaeMatrixEnableRedisplay(Widget w, Boolean redisplay)
#else
XbaeMatrixEnableRedisplay(w, redisplay)
Widget w;
Boolean redisplay;
#endif
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;
    mw = (XbaeMatrixWidget) w;

    if (mw->matrix.disable_redisplay)
        mw->matrix.disable_redisplay--;

#undef FORCE_REDISPLAY_IF_TRUE
#ifndef FORCE_REDISPLAY_IF_TRUE
    if (redisplay && mw->matrix.disable_redisplay == 0)
        XbaeMatrixRefresh(w);
#else
    if (redisplay)
    {
        mw->matrix.disable_redisplay = 0;
        XbaeMatrixRefresh(w);
    }
#endif
}

/*
 * Public interface for xbaeMakeCellVisible()
 */
void
XbaeMatrixMakeCellVisible(w, row, column)
Widget w;
int row;
int column;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    mw = (XbaeMatrixWidget) w;

    xbaeMakeCellVisible(mw, row, column);
}

/*
 * Public interface for xbaeIsRowVisible()
 */
Boolean
XbaeMatrixIsRowVisible(w, row)
Widget w;
int row;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    mw = (XbaeMatrixWidget) w;

    return xbaeIsRowVisible(mw, row);
}

/*
 * Public interface for xbaeIsColumnVisible()
 */
Boolean
XbaeMatrixIsColumnVisible(w, col)
Widget w;
int col;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    mw = (XbaeMatrixWidget) w;

    return xbaeIsColumnVisible(mw, col);
}

/*
 * Public interface for xbaeIsCellVisible()
 */
Boolean
XbaeMatrixIsCellVisible(w, row, col)
Widget w;
int row;
int col;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return False;

    mw = (XbaeMatrixWidget) w;

    return xbaeIsCellVisible(mw, row, col);
}

/*
 *  XbaeMatrixVisibleCells()
 *
 *  This routine returns the range of cells that are visible in the matrix.
 *
 */
void
XbaeMatrixVisibleCells(w, top_row, bottom_row, left_column, right_column)
Widget w;
int *top_row;
int *bottom_row;
int *left_column;
int *right_column;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    mw = (XbaeMatrixWidget) w;

    xbaeGetVisibleRows(mw, top_row, bottom_row);
    xbaeGetVisibleColumns(mw, left_column, right_column);

}

/*
 * Get the label of the column passed here.
 */
String
XbaeMatrixGetColumnLabel(w, column)
Widget w;
int column;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return NULL;

    mw = (XbaeMatrixWidget)w;
        
    if (!(mw->matrix.column_labels) || column > mw->matrix.columns)
        return NULL;
    else
        return mw->matrix.column_labels[column];
}

/*
 * Get the label of the row passed here.
 */
String
XbaeMatrixGetRowLabel(w, row)
Widget w;
int row;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return NULL;

    mw = (XbaeMatrixWidget)w;
        
    if (!(mw->matrix.row_labels) || row > mw->matrix.rows)
        return NULL;
    else
        return mw->matrix.row_labels[row];
}

void
XbaeMatrixSetColumnLabel(w, column, value)
Widget w;
int column;
String value;
{
    XbaeMatrixWidget mw;
    ColumnLabelLines lines;
    String copy;
    
    if (!XtIsSubclass(w, xbaeMatrixWidgetClass) || !value)
        return;

    mw = (XbaeMatrixWidget) w;

    /*
     * Setting a column label when none are defined or changing the number
     * of lines in a column label would cause the need for a redraw of the
     * widget so we won't allow it.
     */
    if (!mw->matrix.column_labels)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "setColumnLabel", "noLabels", "XbaeMatrix",
            "XbaeMatrix: Cannot set column labels when none defined",
            NULL, 0);
        return;
    }

    lines = (ColumnLabelLines)XtMalloc(sizeof(ColumnLabelLinesRec));
    copy = XtNewString(value);
    xbaeParseColumnLabel(copy, lines);

    if (lines->lines != mw->matrix.column_label_lines[column].lines)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "setColumnLabel", "changeLines", "XbaeMatrix",
            "XbaeMatrix: Cannot change number of lines when setting column labels",
            NULL, 0);
        XtFree((XtPointer)copy);
        XtFree((XtPointer)lines->lengths);
        XtFree((XtPointer)lines);
        return;
    }        
    /*
     * OK to make the change
     */
    XtFree((XtPointer) mw->matrix.column_labels[column]);
    XtFree((XtPointer) mw->matrix.column_label_lines[column].lengths);

    mw->matrix.column_labels[column] = copy;
    mw->matrix.column_label_lines[column] = *lines;

    /*
     * Redraw the column label if it is visible.  No expose event is
     * generated by simply changing the label
     */
    if (xbaeIsColumnVisible(mw, column))
    {
        /*
         * Don't generate expose events and let xbaeRedrawLabelsAndFixed
         * work it out - we know where we need to draw
         */
        XClearArea(XtDisplay(mw), XtWindow(mw), COLUMN_LABEL_OFFSET(mw) +
                   COLUMN_POSITION(mw, column), 0, COLUMN_WIDTH(mw, column),
                   COLUMN_LABEL_HEIGHT(mw), False);
        xbaeDrawColumnLabel(mw, column, False);
    }
    return;
}

void
XbaeMatrixSetRowLabel(w, row, value)
Widget w;
int row;
String value;
{
    XbaeMatrixWidget mw;

    if (!XtIsSubclass(w, xbaeMatrixWidgetClass))
        return;

    mw = (XbaeMatrixWidget) w;

    if (!mw->matrix.row_labels || !value)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "setRowLabel", "noLabels", "XbaeMatrix",
            "XbaeMatrix: Cannot set row labels when none defined",
            NULL, 0);
        return;
    }
    XtFree((XtPointer) mw->matrix.row_labels[row]);
    mw->matrix.row_labels[row] = XtNewString(value);

    if (xbaeIsRowVisible(mw, row))
    {
        int y;

        if (IS_LEADING_FIXED_ROW(mw, row))
            y = ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) * row;
        else if (IS_TRAILING_FIXED_ROW(mw, row))
            y = TRAILING_FIXED_ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) *
                (row - TRAILING_VERT_ORIGIN(mw));
        else
            y = ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) *
                (row - VERT_ORIGIN(mw));

        XClearArea(XtDisplay(mw), XtWindow(mw), 0, y, ROW_LABEL_WIDTH(mw),
                   ROW_HEIGHT(mw), False);
        
        xbaeDrawRowLabel(mw, row, False);
    }
}

Widget
XbaeCreateMatrix(parent, name, args, ac) 
Widget parent;
String name;
ArgList args;
Cardinal ac;
{ 
    return XtCreateWidget(name, xbaeMatrixWidgetClass, parent, args, ac); 
} 

