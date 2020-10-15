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
 * $Id: MCreate.c,v 1.2 2007-05-29 11:52:34 andre Exp $
 */

/*
 * Create.c created by Andrew Lister (28 Jan, 1996)
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
#include "MCreate.h"

static Pixmap createInsensitivePixmap P((XbaeMatrixWidget mw));

#ifndef MAXSCREENS
#define MAXSCREENS 20
#endif

void
xbaeCopyBackground(widget, offset, value)
Widget widget;
int offset;
XrmValue *value;
{
    value->addr = (XtPointer)&(widget->core.background_pixel);
}


void
xbaeCopyForeground(widget, offset, value)
Widget widget;
int offset;
XrmValue *value;
{
    value->addr = (XtPointer)&(((XmManagerWidget)widget)->manager.foreground);
}

void
xbaeCopyDoubleClick(widget, offset, value)
Widget widget;
int offset;
XrmValue *value;
{
    static int interval;
  
    interval = XtGetMultiClickTime(XtDisplay(widget));
    value->addr = (XtPointer)&interval;
}

void
xbaeCopyCells(mw)
XbaeMatrixWidget mw;
{
    String **copy = NULL;
    int i, j;
    Boolean empty_row;

    if (mw->matrix.rows && mw->matrix.columns)
    {
        /*
         * Malloc an array of row pointers
         */
        copy = (String **) XtMalloc(mw->matrix.rows * sizeof(String *));

        /*
         * Malloc an array of Strings for each row pointer
         */
        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (String *) XtMalloc(mw->matrix.columns * sizeof(String));

        /*
         * Create a bunch of "" cells if cells was NULL
         */
        if (!mw->matrix.cells)
        {
            for (i = 0; i < mw->matrix.rows; i++)
                for (j = 0; j < mw->matrix.columns; j++)
                    copy[i][j] = XtNewString("");
        }

        /*
         * Otherwise copy the table passed in
         */
        else
        {
            for (i = 0, empty_row = False; i < mw->matrix.rows; i++)
            {
                if (!empty_row && !mw->matrix.cells[i])
                    empty_row = True;
                for (j = 0; j < mw->matrix.columns; j++)
                {
                    if (empty_row || !mw->matrix.cells[i][j])
                    {
                        XtAppWarningMsg(
                            XtWidgetToApplicationContext((Widget)mw),
                            "copyCells", "badValue", "XbaeMatrix",
                            "XbaeMatrix: NULL entry found in cell table",
                            NULL, 0);
                        for (;j < mw->matrix.columns; j++)
                            copy[i][j] = XtNewString("");
                    }
                    else
                        copy[i][j] = XtNewString(mw->matrix.cells[i][j]);
                }
            }
        }
    }
    mw->matrix.cells = copy;
}

#if CELL_WIDGETS
void
xbaeCopyCellWidgets(mw)
XbaeMatrixWidget mw;
{
    Widget **copy = NULL;
    int i, j;

    /*
     * Malloc an array of row pointers
     */
    if (mw->matrix.rows && mw->matrix.columns)
    {
        copy = (Widget **) XtCalloc((Cardinal)mw->matrix.rows,
                                    sizeof(Widget *));

        for (i = 0; i < mw->matrix.rows; i++)
        {
            copy[i] = (Widget *) XtCalloc((Cardinal)mw->matrix.columns,
                                          sizeof(Widget));
            if (mw->matrix.cell_widgets)
                for (j = 0; j < mw->matrix.columns; j++)
                    if (mw->matrix.cell_widgets[i][j])
                        copy[i][j] = mw->matrix.cell_widgets[i][j];
        }
    }
    mw->matrix.cell_widgets = copy;
}
#endif

void
xbaeCopyCellShadowTypes(mw)
XbaeMatrixWidget mw;
{
    unsigned char **copy = NULL;
    int i, j;

    if (mw->matrix.rows && mw->matrix.columns)
    {
        copy = (unsigned char **) XtMalloc(mw->matrix.rows *
                                           sizeof(unsigned char*));

        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (unsigned char*) XtMalloc(mw->matrix.columns *
                                                sizeof(unsigned char));

        for (i = 0; i < mw->matrix.rows; i++)
            for (j = 0; j < mw->matrix.columns; j++)
            {
                if (!mw->matrix.cell_shadow_types[i][j])

                {
                    XtAppWarningMsg(
                        XtWidgetToApplicationContext((Widget) mw),
                        "xbaeCopyCellShadowTypes", "badValue", "XbaeMatrix",
                        "XbaeMatrix: NULL entry found in cellShadowTypes array",
                        NULL, 0);
                    copy[i][j] = XmSHADOW_OUT;
                }
                else
                    copy[i][j] = mw->matrix.cell_shadow_types[i][j];
            }
    }
    mw->matrix.cell_shadow_types = copy;
}

void
xbaeCopyRowShadowTypes(mw)
XbaeMatrixWidget mw;
{
    unsigned char *copy = NULL;
    int i;

    if (mw->matrix.rows)
    {
        copy = (unsigned char *) XtMalloc(mw->matrix.rows *
                                          sizeof(unsigned char));

        for (i = 0; i < mw->matrix.rows; i++)
            if (!mw->matrix.row_shadow_types[i])
            {
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) mw),
                    "xbaeCopyRowShadowTypes", "badValue", "XbaeMatrix",
                    "XbaeMatrix: NULL entry found in rowShadowTypes array",
                    NULL, 0);
                copy[i] = XmSHADOW_OUT;
            }
            else
                copy[i] = mw->matrix.row_shadow_types[i];
    }
    mw->matrix.row_shadow_types = copy;
}

void
xbaeCopyColumnShadowTypes(mw)
XbaeMatrixWidget mw;
{
    unsigned char *copy = NULL;
    int i;

    if (mw->matrix.columns)
    {
        copy = (unsigned char *) XtMalloc(mw->matrix.columns *
                                          sizeof(unsigned char));

        for (i = 0; i < mw->matrix.columns; i++)
            if (!mw->matrix.column_shadow_types[i])
            {
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) mw),
                    "xbaeCopyColumnShadowTypes", "badValue", "XbaeMatrix",
                    "XbaeMatrix: NULL entry found in columnShadowTypes array",
                    NULL, 0);
                copy[i] = XmSHADOW_OUT;
            }
            else
                copy[i] = mw->matrix.column_shadow_types[i];
    }
    mw->matrix.column_shadow_types = copy;
}


void
xbaeCopyCellUserData(mw)
XbaeMatrixWidget mw;
{
    XtPointer **copy = NULL;
    int i, j;

    if (mw->matrix.rows && mw->matrix.columns)
    {
        copy = (XtPointer **) XtMalloc(mw->matrix.rows * sizeof(XtPointer*));

        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (XtPointer*) XtMalloc(mw->matrix.columns *
                                            sizeof(XtPointer));

        for (i = 0; i < mw->matrix.rows; i++)
            for (j = 0; j < mw->matrix.columns; j++)
                copy[i][j] = mw->matrix.cell_user_data[i][j];
    }
    mw->matrix.cell_user_data = copy;
}

void
xbaeCopyRowUserData(mw)
XbaeMatrixWidget mw;
{
    XtPointer *copy = NULL;
    int i;

    if (mw->matrix.rows)
    {
        copy = (XtPointer *) XtMalloc(mw->matrix.rows * sizeof(XtPointer));

        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = mw->matrix.row_user_data[i];
    }
    mw->matrix.row_user_data = copy;
}

void
xbaeCopyColumnUserData(mw)
XbaeMatrixWidget mw;
{
    XtPointer *copy = NULL;
    int i;

    if (mw->matrix.columns)
    {
        copy = (XtPointer *) XtMalloc(mw->matrix.columns * sizeof(XtPointer));

        for (i = 0; i < mw->matrix.columns; i++)
            copy[i] = mw->matrix.column_user_data[i];
    }
    mw->matrix.column_user_data = copy;
}

void
xbaeCopyRowLabels(mw)
XbaeMatrixWidget mw;
{
    String *copy = NULL;
    int i;
    Boolean empty_label;

    if (mw->matrix.rows)
    {
        copy = (String *) XtMalloc(mw->matrix.rows * sizeof(String));

        for (i = 0, empty_label = False; i < mw->matrix.rows; i++)
            if (empty_label || !mw->matrix.row_labels[i])
            {
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) mw),
                    "copyRowLabels", "badValue", "XbaeMatrix",
                    "XbaeMatrix: NULL entry found in rowLabels array",
                    NULL, 0);
                copy[i] = XtNewString("");
                empty_label = True;
            }
            else
                copy[i] = XtNewString(mw->matrix.row_labels[i]);
    }
    mw->matrix.row_labels = copy;
}

void
xbaeCopyColumnLabels(mw)
XbaeMatrixWidget mw;
{
    String *copy = NULL;
    int i;
    Boolean empty_column;
    
    if (mw->matrix.columns)
    {
        copy = (String *) XtMalloc(mw->matrix.columns * sizeof(String));

        mw->matrix.column_label_lines = (ColumnLabelLines)
            XtMalloc(mw->matrix.columns * sizeof(ColumnLabelLinesRec));

        for (i = 0, empty_column = False; i < mw->matrix.columns; i++)
            if (empty_column || !mw->matrix.column_labels[i])
            {
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) mw),
                    "copyColumnLabels", "badValue", "XbaeMatrix",
                    "XbaeMatrix: NULL entry found in columnLabels array",
                    NULL, 0);
                copy[i] = XtNewString("");
                empty_column = True;
                xbaeParseColumnLabel(
                    copy[i], &mw->matrix.column_label_lines[i]);
            }
            else
            {
                copy[i] = XtNewString(mw->matrix.column_labels[i]);
                xbaeParseColumnLabel(mw->matrix.column_labels[i],
                                     &mw->matrix.column_label_lines[i]);
            }

        /*
         * Determine max number of lines in column labels
         */
        mw->matrix.column_label_maxlines =
            mw->matrix.column_label_lines[0].lines;

        for (i = 1; i < mw->matrix.columns; i++)
            if (mw->matrix.column_label_lines[i].lines >
                mw->matrix.column_label_maxlines)
                mw->matrix.column_label_maxlines =
                    mw->matrix.column_label_lines[i].lines;
    }
    mw->matrix.column_labels = copy;
}

void
xbaeCopyColumnWidths(mw)
XbaeMatrixWidget mw;
{
    short *copy = NULL;
    int i;
    Boolean bad = False;

    if (mw->matrix.columns)
    {
        copy = (short *) XtMalloc(mw->matrix.columns * sizeof(short));

        for (i = 0; i < mw->matrix.columns; i++)
        {
            if (!bad && mw->matrix.column_widths[i] == BAD_WIDTH)
            {
                bad = True;
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) mw),
                    "copyColumnWidths", "tooShort", "XbaeMatrix",
                    "XbaeMatrix: Column widths array is too short",
                    NULL, 0);
                copy[i] = 1;
            }
            else if (bad)
                copy[i] = 1;
            else
                copy[i] = mw->matrix.column_widths[i];
        }
    }
    mw->matrix.column_widths = copy;
}

void
xbaeCopyColumnMaxLengths(mw)
XbaeMatrixWidget mw;
{
    int *copy = NULL;
    int i;
    Boolean bad = False;

    if (mw->matrix.columns)
    {
        copy = (int *) XtMalloc(mw->matrix.columns * sizeof(int));

        for (i = 0; i < mw->matrix.columns; i++)
        {
            if (!bad && mw->matrix.column_max_lengths[i] == BAD_MAXLENGTH)
            {
                bad = True;
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) mw),
                    "copyColumnMaxLengths", "tooShort", "XbaeMatrix",
                    "XbaeMatrix: Column max lengths array is too short",
                    NULL, 0);
                copy[i] = 1;
            }
            else if (bad)
                copy[i] = 1;
            else
                copy[i] = mw->matrix.column_max_lengths[i];
        }
    }
    mw->matrix.column_max_lengths = copy;
}

void
xbaeCopyColumnAlignments(mw)
XbaeMatrixWidget mw;
{
    unsigned char *copy = NULL;
    int i;
    Boolean bad = False;

    if (mw->matrix.columns)
    {
        copy = (unsigned char *) XtMalloc(mw->matrix.columns *
                                          sizeof(unsigned char));

        for (i = 0; i < mw->matrix.columns; i++)
        {
            if (!bad && mw->matrix.column_alignments[i] == BAD_ALIGNMENT)
            {
                bad = True;
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) mw),
                    "copyColumnAlignments", "tooShort", "XbaeMatrix",
                    "XbaeMatrix: Column alignments array is too short",
                    NULL, 0);
                copy[i] = XmALIGNMENT_BEGINNING;
            }
            else if (bad)
                copy[i] = XmALIGNMENT_BEGINNING;
            else
                copy[i] = mw->matrix.column_alignments[i];
        }
    }
    mw->matrix.column_alignments = copy;
}

void
xbaeCopyColumnButtonLabels(mw)
XbaeMatrixWidget mw;
{
    Boolean *copy = NULL;
    int i;

    if (mw->matrix.columns)
    {
        copy = (Boolean *) XtMalloc(mw->matrix.columns *
                                    sizeof(Boolean));

        for (i = 0; i < mw->matrix.columns; i++)
        {
            copy[i] = mw->matrix.column_button_labels[i];
        }
    }
    mw->matrix.column_button_labels = copy;
}

void
xbaeCopyRowButtonLabels(mw)
XbaeMatrixWidget mw;
{
    Boolean *copy = NULL;
    int i;

    if (mw->matrix.rows)
    {
        copy = (Boolean *) XtMalloc(mw->matrix.rows *
                                    sizeof(Boolean));

        for (i = 0; i < mw->matrix.rows; i++)
        {
            copy[i] = mw->matrix.row_button_labels[i];
        }
    }
    mw->matrix.row_button_labels = copy;
}

void
xbaeCopyColumnLabelAlignments(mw)
XbaeMatrixWidget mw;
{
    unsigned char *copy = NULL;
    int i;
    Boolean bad = False;

    if (mw->matrix.columns)
    {
        copy = (unsigned char *) XtMalloc(mw->matrix.columns *
                                          sizeof(unsigned char));

        for (i = 0; i < mw->matrix.columns; i++)
        {
            if (!bad &&
                mw->matrix.column_label_alignments[i] == BAD_ALIGNMENT)
            {
                bad = True;
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget) mw),
                    "copyColumnLabelAlignments", "tooShort",
                    "XbaeMatrix",
                    "XbaeMatrix: Column label alignments array is too short",
                    NULL, 0);
                copy[i] = XmALIGNMENT_BEGINNING;
            }
            else if (bad)
                copy[i] = XmALIGNMENT_BEGINNING;
            else
                copy[i] = mw->matrix.column_label_alignments[i];
        }
    }
    mw->matrix.column_label_alignments = copy;
}

void
xbaeCopyColors(mw)
XbaeMatrixWidget mw;
{
    Pixel **copy = NULL;
    int i, j;
    Boolean badrow = False;
    Boolean badcol;


    if (mw->matrix.rows && mw->matrix.columns)
    {
        /*
         * Malloc an array of row pointers
         */
        copy = (Pixel **) XtMalloc(mw->matrix.rows * sizeof(Pixel *));

        /*
         * Malloc an array of Pixels for each row pointer
         */
        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (Pixel *) XtMalloc(mw->matrix.columns * sizeof(Pixel));

        if (!mw->matrix.colors)
        {
            for (i = 0; i < mw->matrix.rows; i++)
                for (j = 0; j < mw->matrix.columns; j++)
                    copy[i][j] = mw->manager.foreground;
        }
        else         for (i = 0; i < mw->matrix.rows; i++)
        {
            if (!badrow && !mw->matrix.colors[i]) {
                badrow = True;
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget)mw),
                    "copyCellColors", "tooShort",
                    "XbaeMatrix",
                    "XbaeMatrix: Cell ColorPixelTable is too short",
                    NULL, 0);
            }
            badcol = badrow;
            for (j = 0; j < mw->matrix.columns; j++)
            {
                if (badcol || mw->matrix.colors[i][j] == BAD_PIXEL)
                {
                    badcol = True;
                    if (j > 0)
                        copy[i][j] = copy[i][j-1] ;
                    else if (i > 0)
                        copy[i][j] = copy[i-1][j] ;
                    else
                        copy[i][j] = mw->manager.foreground;
                }
                else
                {
                    copy[i][j] = mw->matrix.colors[i][j];
                }
            }
        }
    }
    mw->matrix.colors = copy;
}

void
xbaeCopyBackgrounds(mw)
XbaeMatrixWidget mw;
{
    Pixel **copy = NULL;
    int i, j;
    Boolean badrow = False;
    Boolean badcol;

    if (mw->matrix.rows && mw->matrix.columns)
    {
        /*
         * Malloc an array of row pointers
         */
        copy = (Pixel **) XtMalloc(mw->matrix.rows * sizeof(Pixel *));

        /*
         * Malloc an array of Pixels for each row pointer
         */
        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (Pixel *) XtMalloc(mw->matrix.columns * sizeof(Pixel));

        if (!mw->matrix.cell_background)
        {
            for (i = 0; i < mw->matrix.rows; i++)
            {
                Boolean alt = (mw->matrix.alt_row_count &&
                               i >= (int)mw->matrix.fixed_rows) ? (
                                   ((i - (int)mw->matrix.fixed_rows) /
                                    mw->matrix.alt_row_count) % 2) : False;

                /*
                 * Assign the even and odd row colours appropriately.  These
                 * will be a copy of the core->background if they have not
                 * been explicitly set but if they have, we want to
                 * preserve the colours as they appear now
                 */
                for (j = 0; j < mw->matrix.columns; j++)
                    copy[i][j] = (alt ? mw->matrix.odd_row_background :
                                  mw->matrix.even_row_background);
            }
        }
        else for (i = 0; i < mw->matrix.rows; i++)
        {
            if (!badrow && !mw->matrix.cell_background[i]) {
                badrow = True;
                XtAppWarningMsg(
                    XtWidgetToApplicationContext((Widget)mw),
                    "copyCellColors", "tooShort",
                    "XbaeMatrix",
                    "XbaeMatrix: Cell BackgroundPixelTable is too short",
                    NULL, 0);
            }
            badcol = badrow;
            for (j = 0; j < mw->matrix.columns; j++)
            {
                if (badcol || mw->matrix.cell_background[i][j] == BAD_PIXEL)
                {
                    badcol = True;
                    if (j > 0)
                        copy[i][j] = copy[i][j-1] ;
                    else if (i > 0)
                        copy[i][j] = copy[i-1][j] ;
                    else
                        copy[i][j] = mw->core.background_pixel;
                }
                else
                {
                    copy[i][j] = mw->matrix.cell_background[i][j];
                }
            }
        }
    }
    mw->matrix.cell_background = copy;
}

/*
 * Copy the selectedCells resource. Create a 2D array of Booleans to
 * represent selected cells if it is NULL.
 */
void
xbaeCopySelectedCells(mw)
XbaeMatrixWidget mw;
{
    Boolean **copy = NULL;
    int i, j;

    if (mw->matrix.rows && mw->matrix.columns)
    {
        /*
         * Malloc an array of row pointers
         */
        mw->matrix.num_selected_cells = 0;
        copy = (Boolean **) XtMalloc(mw->matrix.rows * sizeof(Boolean *));

        /*
         * Malloc an array of Booleans for each row pointer
         */
        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (Boolean *) XtCalloc(mw->matrix.columns,
                                           sizeof(Boolean));

        /*
         * If selected_cells is not NULL, copy the table passed in
         */
        if (mw->matrix.selected_cells)
            for (i = 0; i < mw->matrix.rows; i++)
                for (j = 0; j < mw->matrix.columns; j++)
                {
                    copy[i][j] = mw->matrix.selected_cells[i][j];
                    if (mw->matrix.selected_cells[i][j])
                        mw->matrix.num_selected_cells++;
                }
    }
    mw->matrix.selected_cells = copy;
}

#if XmVersion >= 1002
/*
 * Copy the highlightedCells resource. Create a 2D array of Booleans to
 * represent highlighted cells if it is NULL.
 */
void
xbaeCopyHighlightedCells(mw)
XbaeMatrixWidget mw;
{
    unsigned char **copy = NULL;
    int i, j;

    if (mw->matrix.rows && mw->matrix.columns)
    {
        /*
         * Malloc an array of row pointers
         */
        copy = (unsigned char **) XtMalloc(mw->matrix.rows *
                                           sizeof(Boolean *));

        /*
         * Malloc an array of Booleans for each row pointer
         */
        for (i = 0; i < mw->matrix.rows; i++)
            copy[i] = (unsigned char *) XtCalloc(mw->matrix.columns,
                                                 sizeof(Boolean));

        /*
         * If highlighted_cells is not NULL, copy the table passed in
         */
        if (mw->matrix.highlighted_cells)
            for (i = 0; i < mw->matrix.rows; i++)
                for (j = 0; j < mw->matrix.columns; j++)
                    copy[i][j] = mw->matrix.highlighted_cells[i][j];
    }
    mw->matrix.highlighted_cells = copy;
}
#endif

/*
 * Create a matrix of Pixels
 */
void
xbaeCreateColors(mw)
XbaeMatrixWidget mw;
{
    int i;

    if (mw->matrix.rows && mw->matrix.columns)
    {
        /*
         * Malloc an array of row pointers
         */
        mw->matrix.colors = (Pixel **) XtMalloc(mw->matrix.rows *
                                                sizeof(Pixel *));

        /*
         * Malloc an array of Pixels for each row pointer
         */
        for (i = 0; i < mw->matrix.rows; i++)
            mw->matrix.colors[i] = (Pixel *) XtMalloc(mw->matrix.columns *
                                                      sizeof(Pixel));
    }
    else
        mw->matrix.colors = NULL;
}

/*
 * Create a pixmap to be used for drawing the matrix contents when
 * XmNsensitive is set to False
 */
static Pixmap
createInsensitivePixmap(mw)
XbaeMatrixWidget mw;
{
    static char stippleBits[] = { 0x01, 0x02 };
    static Pixmap stipple[MAXSCREENS] = {(Pixmap)NULL};
    Display *dpy = XtDisplay(mw);
    Screen *scr  = XtScreen (mw);
    int i;
    int maxScreens = ScreenCount(dpy);
    
    if (maxScreens > MAXSCREENS)
        maxScreens = MAXSCREENS;
    
    if (!stipple[0])
    {
        for (i = 0 ; i < maxScreens ; i++)
            stipple[i] = XCreatePixmapFromBitmapData(
                dpy, RootWindow(dpy,i), stippleBits, 2, 2, 0, 1, 1);
    }
    for (i = 0; i < maxScreens; i++)
    {
        if (ScreenOfDisplay(dpy, i) == scr)
            return stipple[i];
    }
    return (Pixmap)NULL;
}
    
void
xbaeCreateGridLineGC(mw)
XbaeMatrixWidget mw;
{
    XGCValues values;
    XtGCMask mask = GCForeground | GCBackground;

    values.foreground = mw->matrix.grid_line_color;
    values.background = mw->manager.foreground;
    
    /*
     * GC for drawing grid lines
     */
    mw->matrix.grid_line_gc = XtGetGC((Widget) mw, mask, &values);
    
    /*
     * GC for drawing grid lines with clipping
     */
    mw->matrix.cell_grid_line_gc = XCreateGC(XtDisplay(mw),
                                             GC_PARENT_WINDOW(mw),
                                             mask, &values);
}

void
xbaeCreateDrawGC(mw)
XbaeMatrixWidget mw;
{
    XGCValues values;
    unsigned long mask = GCForeground | GCFont | GCStipple;

    /*
     * GC for drawing cells. We create it instead of using a cached one,
     * since the foreground may change frequently.
     */
    values.foreground = mw->manager.foreground;
    values.font = mw->matrix.fid;
    values.stipple = createInsensitivePixmap(mw);

    mw->matrix.draw_gc = XCreateGC(XtDisplay(mw),
                                   GC_PARENT_WINDOW(mw),
                                   mask, &values);
}

void
xbaeCreatePixmapGC(mw)
XbaeMatrixWidget mw;
{
    XGCValues values;
    unsigned long mask = GCForeground | GCGraphicsExposures | GCStipple;

    values.foreground = mw->manager.foreground;
    values.graphics_exposures = False;
    values.stipple = createInsensitivePixmap(mw);

    mw->matrix.pixmap_gc = XCreateGC(XtDisplay(mw),
                                     GC_PARENT_WINDOW(mw),
                                     mask, &values);
}

void
xbaeCreateLabelGC(mw)
XbaeMatrixWidget mw;
{
    XGCValues values;
    unsigned long mask = GCForeground | GCFont | GCStipple;

    /*
     * GC for drawing labels
     */
    values.foreground = mw->manager.foreground;
    values.font = mw->matrix.label_fid;
    values.stipple = createInsensitivePixmap(mw);
    mw->matrix.label_gc = XCreateGC(XtDisplay(mw),
                                    GC_PARENT_WINDOW(mw),
                                    mask, &values);
}

void
xbaeCreateLabelClipGC(mw)
XbaeMatrixWidget mw;
{
    XGCValues values;
    unsigned long mask = GCForeground | GCFont | GCStipple;

    /*
     * GC for drawing labels with clipping.
     */
    values.foreground = mw->manager.foreground;
    values.font = mw->matrix.label_fid;
    values.stipple = createInsensitivePixmap(mw);
    mw->matrix.label_clip_gc = XCreateGC(XtDisplay(mw),
                                         GC_PARENT_WINDOW(mw),
                                         mask, &values);
}

void
xbaeCreateTopShadowClipGC(mw)
XbaeMatrixWidget mw;
{
    XGCValues values;
    XtGCMask mask = GCForeground | GCBackground;

    /*
     * GC for drawing top shadow inside cells with clipping.
     */
    values.foreground = mw->manager.top_shadow_color;
    values.background = mw->manager.foreground;

    if (mw->manager.top_shadow_pixmap != XmUNSPECIFIED_PIXMAP)
    {
        mask |= GCFillStyle | GCTile;
        values.fill_style = FillTiled;
        values.tile = mw->manager.top_shadow_pixmap;
    }
    mw->matrix.cell_top_shadow_clip_gc = XCreateGC(
        XtDisplay(mw), GC_PARENT_WINDOW(mw), mask, &values);

    mask |= GCFunction;
    values.function = GXxor;
    mw->matrix.resize_top_shadow_gc = XtGetGC(
        (Widget) mw, mask, &values);
}

void
xbaeCreateBottomShadowClipGC(mw)
XbaeMatrixWidget mw;
{
    XGCValues values;
    XtGCMask mask = GCForeground | GCBackground;

    /*
     * GC for drawing bottom shadow inside cells with clipping.
     */
    values.foreground = mw->manager.bottom_shadow_color;
    values.background = mw->manager.foreground;

    if (mw->manager.bottom_shadow_pixmap != XmUNSPECIFIED_PIXMAP)
    {
        mask |= GCFillStyle | GCTile;
        values.fill_style = FillTiled;
        values.tile = mw->manager.bottom_shadow_pixmap;
    }
    mw->matrix.cell_bottom_shadow_clip_gc = XCreateGC(
        XtDisplay(mw), GC_PARENT_WINDOW(mw), mask, &values);

    mask |= GCFunction;
    values.function = GXxor;
    mw->matrix.resize_bottom_shadow_gc = XtGetGC(
        (Widget) mw, mask, &values);
}

void
xbaeNewFont(mw)
XbaeMatrixWidget mw;
{
    XmFontContext context;
    XFontStruct *font;
    XmFontListEntry font_list_entry;
    XmFontType type;
    XFontSetExtents *extents;
    XFontStruct **fonts;
    char **font_names;

    /*
     * Make a private copy of the FontList
     */
    mw->matrix.font_list = XmFontListCopy(mw->matrix.font_list);

    /*
     * Get XmFontListEntry from FontList
     */
    if (!XmFontListInitFontContext(&context, mw->matrix.font_list))
        XtAppErrorMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "newFont", "badFont", "XbaeMatrix",
            "XbaeMatrix: XmFontListInitFontContext failed, bad fontList",
            NULL, 0);

    if ((font_list_entry = XmFontListNextEntry(context)) == NULL)
        XtAppErrorMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "newFont", "badFont", "XbaeMatrix",
            "XbaeMatrix: XmFontListNextEntry failed, no next fontList",
            NULL, 0);

    font = (XFontStruct*)XmFontListEntryGetFont(font_list_entry, &type);

    if (type == XmFONT_IS_FONTSET)
    {
        mw->matrix.font_set = (XFontSet)font;
        mw->matrix.font_struct = (XFontStruct*)NULL;

        extents = XExtentsOfFontSet((XFontSet)font);
        mw->matrix.font_width = extents->max_logical_extent.width;
        mw->matrix.font_height = extents->max_logical_extent.height;
        mw->matrix.font_y = extents->max_logical_extent.y;

        XFontsOfFontSet((XFontSet)font, &fonts, &font_names);
        mw->matrix.fid = fonts[0]->fid;
    }
    else
    {
        mw->matrix.font_set = (XFontSet)NULL;
        mw->matrix.font_struct = font;

        mw->matrix.font_width = (font->max_bounds.width + font->min_bounds.width) /2;
        mw->matrix.font_height = (font->max_bounds.descent + font->max_bounds.ascent);
        mw->matrix.font_y = -font->max_bounds.ascent;

        mw->matrix.fid = font->fid;
    }

    XmFontListFreeFontContext(context);
}

void
xbaeNewLabelFont(mw)
XbaeMatrixWidget mw;
{
    XmFontContext context;
    XFontStruct *font;
    XmFontListEntry font_list_entry;
    XmFontType type;
    XFontSetExtents *extents;
    XFontStruct **fonts;
    char **font_names;

    /*
     * Make a private copy of the FontList
     */
    mw->matrix.label_font_list = XmFontListCopy(mw->matrix.label_font_list);

    /*
     * Get XmFontListEntry from FontList
     */
    if (!XmFontListInitFontContext(&context, mw->matrix.label_font_list))
        XtAppErrorMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "newFont", "badLabelFont", "XbaeMatrix",
            "XbaeMatrix: XmFontListInitFontContext failed, bad labelFontList",
            NULL, 0);

    if ((font_list_entry = XmFontListNextEntry(context)) == NULL)
        XtAppErrorMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "newFont", "badLabelFont", "XbaeMatrix",
            "XbaeMatrix: XmFontListNextEntry failed, no next fontList",
            NULL, 0);

    font = (XFontStruct*)XmFontListEntryGetFont(font_list_entry, &type);

    if (type == XmFONT_IS_FONTSET)
    {
        mw->matrix.label_font_set = (XFontSet)font;
        mw->matrix.label_font_struct = (XFontStruct*)NULL;

        extents = XExtentsOfFontSet((XFontSet)font);
        mw->matrix.label_font_width = extents->max_logical_extent.width;
        mw->matrix.label_font_height = extents->max_logical_extent.height;
        mw->matrix.label_font_y = extents->max_logical_extent.y;

        XFontsOfFontSet((XFontSet)font, &fonts, &font_names);
        mw->matrix.label_fid = fonts[0]->fid;
    }
    else
    {
        mw->matrix.label_font_set = (XFontSet)NULL;
        mw->matrix.label_font_struct = font;

        mw->matrix.label_font_width = (font->max_bounds.width + font->min_bounds.width) /2;
        mw->matrix.label_font_height = (font->max_bounds.descent + font->max_bounds.ascent);
        mw->matrix.label_font_y = -font->max_bounds.ascent;

        mw->matrix.label_fid = font->fid;
    }

    XmFontListFreeFontContext(context);
}

void
xbaeFreeCells(mw)
XbaeMatrixWidget mw;
{
    int i, j;

    if (!mw->matrix.cells)
        return;

    /*
     * Free each cell in a row, then free the row and go to the next one
     */
    for (i = 0; i < mw->matrix.rows; i++)
    {
        for (j = 0; j < mw->matrix.columns; j++)
            XtFree((XtPointer) mw->matrix.cells[i][j]);
        XtFree((XtPointer) mw->matrix.cells[i]);
    }

    /*
     * Free the array of row pointers
     */
    XtFree((XtPointer) mw->matrix.cells);
    mw->matrix.cells = NULL;
}

#if CELL_WIDGETS
void
xbaeFreeCellWidgets(mw)
XbaeMatrixWidget mw;
{
    int i, j;

    if (!mw->matrix.cell_widgets)
        return;

    /*
     * Free each cell in a row, then free the row and go to the next one
     */
    for (i = 0; i < mw->matrix.rows; i++)
    {
        for (j = 0; j < mw->matrix.columns; j++)
            XtFree((XtPointer) mw->matrix.cell_widgets[i][j]);
        XtFree((XtPointer) mw->matrix.cell_widgets[i]);
    }

    /*
     * Free the array of row pointers
     */
    XtFree((XtPointer) mw->matrix.cell_widgets);
    mw->matrix.cell_widgets = NULL;
}
#endif

void
xbaeFreeRowLabels(mw)
XbaeMatrixWidget mw;
{
    int i;

    if (!mw->matrix.row_labels)
        return;

    for (i = 0; i < mw->matrix.rows; i++)
        XtFree((XtPointer) mw->matrix.row_labels[i]);

    XtFree((XtPointer) mw->matrix.row_labels);
    mw->matrix.row_labels = NULL;
}

void
xbaeFreeColumnLabels(mw)
XbaeMatrixWidget mw;
{
    int i;

    if (!mw->matrix.column_labels)
        return;

    for (i = 0; i < mw->matrix.columns; i++)
    {
        XtFree((XtPointer) mw->matrix.column_labels[i]);
        XtFree((XtPointer) mw->matrix.column_label_lines[i].lengths);
    }

    XtFree((XtPointer) mw->matrix.column_label_lines);
    XtFree((XtPointer) mw->matrix.column_labels);
    mw->matrix.column_labels = NULL;
}


void
xbaeFreeColors(mw)
XbaeMatrixWidget mw;
{
    int i;

    if (!mw->matrix.colors)
        return;

    /*
     * Free each row of Pixels
     */
    for (i = 0; i < mw->matrix.rows; i++)
        XtFree((XtPointer) mw->matrix.colors[i]);

    /*
     * Free the array of row pointers
     */
    XtFree((XtPointer) mw->matrix.colors);
    mw->matrix.colors = NULL;
}

void
xbaeFreeBackgrounds(mw)
XbaeMatrixWidget mw;
{
    int i;

    if (!mw->matrix.cell_background)
        return;

    /*
     * Free each row of Pixels
     */
    for (i = 0; i < mw->matrix.rows; i++)
        XtFree((XtPointer) mw->matrix.cell_background[i]);

    /*
     * Free the array of row pointers
     */
    XtFree((XtPointer) mw->matrix.cell_background);
    mw->matrix.cell_background = NULL;
}

void
xbaeFreeSelectedCells(mw)
XbaeMatrixWidget mw;
{
    int i;

    /*
     * Free each row of XtPointer pointers
     */
    if (!mw->matrix.selected_cells)
        return;

    for (i = 0; i < mw->matrix.rows; i++)
        XtFree((XtPointer) mw->matrix.selected_cells[i]);

    /*
     * Free the array of row pointers
     */
    XtFree((XtPointer) mw->matrix.selected_cells);
    mw->matrix.selected_cells = NULL;
}

#if XmVersion >= 1002
void
xbaeFreeHighlightedCells(mw)
XbaeMatrixWidget mw;
{
    int i;

    if (!mw->matrix.highlighted_cells)
        return;
    
    /*
     * Free each row of XtPointer pointers
     */
    for (i = 0; i < mw->matrix.rows; i++)
        XtFree((XtPointer) mw->matrix.highlighted_cells[i]);

    /*
     * Free the array of row pointers
     */
    XtFree((XtPointer) mw->matrix.highlighted_cells);
    mw->matrix.highlighted_cells = NULL;
}
#endif

void
xbaeFreeCellUserData(mw)
XbaeMatrixWidget mw;
{
    if (mw->matrix.cell_user_data)
    {
        int i;

        /*
         * Free each row of Booleans
         */
        for (i = 0; i < mw->matrix.rows; i++)
            XtFree((XtPointer) mw->matrix.cell_user_data[i]);

        /*
         * Free the array of row pointers
         */
        XtFree((XtPointer) mw->matrix.cell_user_data);
    }
    mw->matrix.cell_user_data = NULL;
}


void
xbaeFreeCellShadowTypes(mw)
XbaeMatrixWidget mw;
{
    if (mw->matrix.cell_shadow_types)
    {
        int i;

        /*
         * Free each row of unsigned char pointers
         */
        for (i = 0; i < mw->matrix.rows; i++)
            XtFree((XtPointer) mw->matrix.cell_shadow_types[i]);

        /*
         * Free the array of row pointers
         */
        XtFree((XtPointer) mw->matrix.cell_shadow_types);
    }
    mw->matrix.cell_shadow_types = NULL;
}

