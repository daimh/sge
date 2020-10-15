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
 * $Id: Methods.c,v 1.1 2001-07-18 11:06:00 root Exp $
 */

/*
 * Methods.c created by Andrew Lister (7 August, 1995)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Xm/Xm.h>
#include <X11/Intrinsic.h>
#include "MatrixP.h"
#include "Methods.h"
#include "Actions.h"
#include "ScrollMgr.h"
#include "Utils.h"
#include "Shadow.h"
#include "Draw.h"
#include "MCreate.h"
#include "ClipP.h"
#include "Input.h"
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Xm/TextP.h>

/* For memmove/bcopy */
#include <string.h>

#if !defined(HAVE_MEMMOVE) && !defined(XBAE_NEED_BCOPY)
#define XBAE_NEED_BCOPY
#endif

/* Earl R.
 * Added another BCOPY macro for porting purposes. Porting to 15+ UNIX
 * platforms. Renamed bcopy to BCOPY and typecast to fix compiler warnings
 * on some platforms.
 */
#if !defined(XBAE_NEED_BCOPY) || defined(SVR4) || defined(VMS) || defined(__EMX__)
#define BCOPY(src, dest, n)        memmove((void *)(dest), (void *)(src), (n))
#else
#define BCOPY(src, dest, n)     bcopy((void *)(src), (void *)(dest), (n))
#endif

static void AddRowsToTable P((XbaeMatrixWidget, int, String *, String *,
                               Pixel *, Pixel *, int));
static void DeleteRowsFromTable P((XbaeMatrixWidget, int, int));
static void AddColumnsToTable P((XbaeMatrixWidget, int, String *, String *,
                                  short *, int *, unsigned char *,
                                  unsigned char *, Pixel *, Pixel *, int));
static void DeleteColumnsFromTable P((XbaeMatrixWidget, int, int));
static Boolean DoCommitEdit P((XbaeMatrixWidget, XEvent *));

/*
 * Add rows to the internal cells data structure.
 * If rows or labels is NULL, add empty rows.
 */
static void
AddRowsToTable(mw, position, rows, labels, colors, backgrounds, num_rows)
XbaeMatrixWidget mw;
int position;
String *rows;
String *labels;
Pixel *colors;
Pixel *backgrounds;
int num_rows;
{
    int i, j;

    /*
     * Realloc a larger array of row pointers and a larger label arrays
     */
    if (mw->matrix.cells || rows)
        mw->matrix.cells = (String **) XtRealloc((char *) mw->matrix.cells,
                                                 (mw->matrix.rows + num_rows) *
                                                 sizeof(String *));
    if (mw->matrix.row_labels || labels)
        mw->matrix.row_labels =
            (String *) XtRealloc((char *) mw->matrix.row_labels,
                                 (mw->matrix.rows + num_rows) *
                                 sizeof(String));

    if (mw->matrix.row_button_labels)
        mw->matrix.row_button_labels =
            (Boolean *) XtRealloc((char *) mw->matrix.
                                  row_button_labels,
                                  (mw->matrix.rows + num_rows) *
                                  sizeof(Boolean));

    if (mw->matrix.colors || colors)
        mw->matrix.colors = (Pixel **) XtRealloc((char *) mw->matrix.colors,
                                                 (mw->matrix.rows + num_rows) *
                                                 sizeof(Pixel *));

    if (mw->matrix.cell_background || backgrounds)
        mw->matrix.cell_background =
            (Pixel **) XtRealloc((char *) mw->matrix.cell_background,
                                 (mw->matrix.rows + num_rows) *
                                 sizeof(Pixel *));

    if (mw->matrix.cell_user_data)
        mw->matrix.cell_user_data = (XtPointer **)
            XtRealloc((char*) mw->matrix.cell_user_data,
                      (mw->matrix.rows + num_rows) *
                      sizeof(XtPointer *));

#if CELL_WIDGETS
    if (mw->matrix.cell_widgets)
        mw->matrix.cell_widgets = (Widget **)
            XtRealloc((char*) mw->matrix.cell_widgets,
                      (mw->matrix.rows + num_rows) *
                      sizeof(Widget *));
#endif

    if (mw->matrix.row_user_data)
        mw->matrix.row_user_data = (XtPointer*)
            XtRealloc((char*) mw->matrix.row_user_data,
                      (mw->matrix.rows + num_rows) *
                      sizeof(XtPointer));

    if (mw->matrix.cell_shadow_types)
        mw->matrix.cell_shadow_types = (unsigned char **)
            XtRealloc((char*) mw->matrix.cell_shadow_types,
                      (mw->matrix.rows + num_rows) *
                      sizeof(unsigned char *));

    if (mw->matrix.row_shadow_types)
        mw->matrix.row_shadow_types = (unsigned char *)
            XtRealloc((char*) mw->matrix.row_shadow_types,
                      (mw->matrix.rows + num_rows) *
                      sizeof(unsigned char));

    if (mw->matrix.selected_cells)
        mw->matrix.selected_cells =
            (Boolean **) XtRealloc((char *) mw->matrix.selected_cells,
                                   (mw->matrix.rows + num_rows) *
                                   sizeof(Boolean *));

#if XmVersion >= 1002
    if (mw->matrix.highlighted_cells)
        mw->matrix.highlighted_cells =
            (unsigned char **) XtRealloc((char *) mw->matrix.highlighted_cells,
                                         (mw->matrix.rows + num_rows) *
                                         sizeof(unsigned char *));
#endif
    /*
     * If we are inserting rows into the middle, we need to make room.
     */
    if (position < mw->matrix.rows)
    {
        if (mw->matrix.cells)
            BCOPY(&mw->matrix.cells[position],
                  &mw->matrix.cells[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(String *));
        if (mw->matrix.row_labels)
            BCOPY(&mw->matrix.row_labels[position],
                  &mw->matrix.row_labels[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(String));
        if (mw->matrix.row_button_labels)
            BCOPY(&mw->matrix.row_button_labels[position],
                  &mw->matrix.row_button_labels[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(Boolean));
        if (mw->matrix.colors)
            BCOPY(&mw->matrix.colors[position],
                  &mw->matrix.colors[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(Pixel *));
        if (mw->matrix.cell_background)
            BCOPY(&mw->matrix.cell_background[position],
                  &mw->matrix.cell_background[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(Pixel *));
#if CELL_WIDGETS
        if (mw->matrix.cell_widgets)
            BCOPY(&mw->matrix.cell_widgets[position],
                  &mw->matrix.cell_widgets[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(Widget *));
#endif
        if (mw->matrix.cell_user_data)
            BCOPY(&mw->matrix.cell_user_data[position],
                  &mw->matrix.cell_user_data[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(XtPointer *));
        if (mw->matrix.row_user_data)
            BCOPY(&mw->matrix.row_user_data[position],
                  &mw->matrix.row_user_data[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(XtPointer));
        if (mw->matrix.cell_shadow_types)
            BCOPY(&mw->matrix.cell_shadow_types[position],
                  &mw->matrix.cell_shadow_types[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(unsigned char *));
        if (mw->matrix.row_shadow_types)
            BCOPY(&mw->matrix.row_shadow_types[position],
                  &mw->matrix.row_shadow_types[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(unsigned char));
        if (mw->matrix.selected_cells)
            BCOPY(&mw->matrix.selected_cells[position],
                  &mw->matrix.selected_cells[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(Boolean *));
#if XmVersion >= 1002
        if (mw->matrix.highlighted_cells)
            BCOPY(&mw->matrix.highlighted_cells[position],
                  &mw->matrix.highlighted_cells[position + num_rows],
                  (mw->matrix.rows - position) * sizeof(unsigned char *));
#endif
    }

    /*
     * Malloc a new row array for each new row. Copy the label for each row.
     * If no label was passed in, use a NULL String. Malloc a new Pixel
     * and Boolean row array for each new row.
     * Use False for new  button label flags.
     */
    for (i = 0; i < num_rows; i++)
    {
        if (mw->matrix.cells)
            mw->matrix.cells[i + position] =
                (String *) XtMalloc(mw->matrix.columns * sizeof(String));
        if (mw->matrix.row_labels)
            mw->matrix.row_labels[i + position] =
                labels ? XtNewString(labels[i]) : XtNewString("");
        if (mw->matrix.row_button_labels)
            mw->matrix.row_button_labels[i + position] = False;
        if (mw->matrix.colors)
            mw->matrix.colors[i + position] =
                (Pixel *) XtMalloc(mw->matrix.columns * sizeof(Pixel));
        if (mw->matrix.cell_background)
            mw->matrix.cell_background[i + position] =
                (Pixel *) XtMalloc(mw->matrix.columns * sizeof(Pixel));
#if CELL_WIDGETS
        if (mw->matrix.cell_widgets)
            mw->matrix.cell_widgets[i + position] =
                (Widget *) XtMalloc(mw->matrix.columns * sizeof(Widget));
#endif
        if (mw->matrix.cell_user_data)
            mw->matrix.cell_user_data[i + position] =
                (XtPointer *) XtMalloc(mw->matrix.columns * sizeof(XtPointer));
        if (mw->matrix.row_user_data)
            mw->matrix.row_user_data[i + position] = (XtPointer) NULL;
        if (mw->matrix.cell_shadow_types)
            mw->matrix.cell_shadow_types[i + position] =
                (unsigned char *) XtMalloc(mw->matrix.columns *
                                           sizeof(unsigned char));
        if (mw->matrix.row_shadow_types)
            mw->matrix.row_shadow_types[i + position] =
                mw->matrix.cell_shadow_type;
        if (mw->matrix.selected_cells)
            mw->matrix.selected_cells[i + position] =
                (Boolean *) XtMalloc(mw->matrix.columns * sizeof(Boolean));
#if XmVersion >= 1002
        if (mw->matrix.highlighted_cells)
            mw->matrix.highlighted_cells[i + position] =
                (unsigned char *) XtMalloc(mw->matrix.columns *
                                           sizeof(unsigned char));
#endif
    }

    /*
     * Copy the rows arrays passed in into each new row, or if NULL
     * was passed in initialize each row to NULL Strings. Copy the colors
     * arrays passed in into each new row, if NULL was passed use foreground.
     */
    for (i = 0; i < num_rows; i++)
        for (j = 0; j < mw->matrix.columns; j++)
        {
            if (mw->matrix.cells) /* NULL row[j] is empty string. Earl R. */
                mw->matrix.cells[i + position][j] = rows ?
                    XtNewString((rows[i * mw->matrix.columns + j] ?
                                 rows[i * mw->matrix.columns + j] : "")) :
                XtNewString("");
            if (mw->matrix.colors)
                mw->matrix.colors[i + position][j] =
                    colors ? colors[i] : mw->manager.foreground;
            if (mw->matrix.cell_background)
                mw->matrix.cell_background[i + position][j] =
                    backgrounds ? backgrounds[i] : mw->core.background_pixel;
#if CELL_WIDGETS
            if (mw->matrix.cell_widgets)
                mw->matrix.cell_widgets[i + position][j] = NULL;
#endif
            if (mw->matrix.cell_user_data)
                mw->matrix.cell_user_data[i + position][j] = (XtPointer) NULL;
            if (mw->matrix.cell_shadow_types)
                mw->matrix.cell_shadow_types[i + position][j] =
                    mw->matrix.cell_shadow_type;
            if (mw->matrix.selected_cells)
                mw->matrix.selected_cells[i + position][j] = False;
#if XmVersion >= 1002
            if (mw->matrix.highlighted_cells)
                mw->matrix.highlighted_cells[i + position][j] = HighlightNone;
#endif
        }

    mw->matrix.rows += num_rows;
}

/*
 * Delete rows from the internal cells data structure.
 */
static void
DeleteRowsFromTable(mw, position, num_rows)
XbaeMatrixWidget mw;
int position;
int num_rows;
{
    int i, j;

    /*
     * We don't bother to realloc, we will just have some wasted space.
     * XXX is this a problem?
     */

    /*
     * Free all the cells in the rows being deleted and the rows themselves.
     * Also free the String row labels and label button flags.  Free the color
     * arrays for the rows being deleted.
     */
    for (i = position; i < position + num_rows; i++)
    {
        /* Fixed a crash I was getting, Since I allow NULL cells. Earl R. */
        if (mw->matrix.cells && mw->matrix.cells[i])
        {
            for (j = 0; j < mw->matrix.columns; j++)
                if (mw->matrix.cells[i][j])
                    XtFree((XtPointer) mw->matrix.cells[i][j]);
            XtFree((XtPointer) mw->matrix.cells[i]);
            mw->matrix.cells[i] = NULL;
        }
        if (mw->matrix.row_labels) {
            XtFree((XtPointer) mw->matrix.row_labels[i]);
            mw->matrix.row_labels[i] = NULL;
        }    
        if (mw->matrix.colors) {
            XtFree((XtPointer) mw->matrix.colors[i]);
            mw->matrix.colors[i] = NULL;
        }    
        if (mw->matrix.cell_background) {
            XtFree((XtPointer) mw->matrix.cell_background[i]);
            mw->matrix.cell_background[i] = NULL;
        }    
#if CELL_WIDGETS
        if (mw->matrix.cell_widgets) {
            XtFree((XtPointer) mw->matrix.cell_widgets[i]);
            mw->matrix.cell_widgets[i] = NULL;
        }    
#endif
        if (mw->matrix.cell_user_data) {
            XtFree((XtPointer) mw->matrix.cell_user_data[i]);
            mw->matrix.cell_user_data[i] = NULL;
        }    
        if (mw->matrix.cell_shadow_types) {
            XtFree((XtPointer) mw->matrix.cell_shadow_types[i]);
            mw->matrix.cell_shadow_types[i] = NULL;
        }    
        if (mw->matrix.selected_cells) {
            /*
             * Deselect the row so num_selected_cells gets updated
             */
            xbaeDeselectRow(mw, i);
            XtFree((XtPointer) mw->matrix.selected_cells[i]);
            mw->matrix.selected_cells[i] = NULL;
        }
#if XmVersion >= 1002
        if (mw->matrix.highlighted_cells) {
            XtFree((XtPointer) mw->matrix.highlighted_cells[i]);
            mw->matrix.highlighted_cells[i] = NULL;
        }    
#endif
    }

    /*
     * Copy those rows which are below the ones deleted, up.
     * (unless we deleted rows from the bottom).
     */
    if (position + num_rows < mw->matrix.rows)
    {
        if (mw->matrix.cells)        
            BCOPY(&mw->matrix.cells[position + num_rows],
                  &mw->matrix.cells[position],
                  (mw->matrix.rows - position - num_rows) * sizeof(String *));
        if (mw->matrix.row_labels)
            BCOPY(&mw->matrix.row_labels[position + num_rows],
                  &mw->matrix.row_labels[position],
                  (mw->matrix.rows - position - num_rows) * sizeof(String));
        if (mw->matrix.row_button_labels)
            BCOPY(&mw->matrix.row_button_labels[position + num_rows],
                  &mw->matrix.row_button_labels[position],
                  (mw->matrix.rows - position - num_rows) * sizeof(Boolean));
        if (mw->matrix.colors)
            BCOPY(&mw->matrix.colors[position + num_rows],
                  &mw->matrix.colors[position],
                  (mw->matrix.rows - position - num_rows) * sizeof(Pixel *));
        if (mw->matrix.cell_background)
            BCOPY(&mw->matrix.cell_background[position + num_rows],
                  &mw->matrix.cell_background[position],
                  (mw->matrix.rows - position - num_rows) * sizeof(Pixel *));
#if CELL_WIDGETS
        if (mw->matrix.cell_widgets)
            BCOPY(&mw->matrix.cell_widgets[position + num_rows],
                  &mw->matrix.cell_widgets[position],
                  (mw->matrix.rows - position - num_rows) * sizeof(Widget *));
#endif
        if (mw->matrix.cell_user_data)
            BCOPY(&mw->matrix.cell_user_data[position + num_rows],
                  &mw->matrix.cell_user_data[position],
                  (mw->matrix.rows - position - num_rows) *
                  sizeof(XtPointer *));
        if (mw->matrix.row_user_data)
            BCOPY(&mw->matrix.row_user_data[position + num_rows],
                  &mw->matrix.row_user_data[position],
                  (mw->matrix.rows - position - num_rows) * sizeof(XtPointer *));
        if (mw->matrix.cell_shadow_types)
            BCOPY(&mw->matrix.cell_shadow_types[position + num_rows],
                  &mw->matrix.cell_shadow_types[position],
                  (mw->matrix.rows - position - num_rows) *
                  sizeof(unsigned char *));
        if (mw->matrix.row_shadow_types)
            BCOPY(&mw->matrix.row_shadow_types[position + num_rows],
                  &mw->matrix.row_shadow_types[position],
                  (mw->matrix.rows - position - num_rows) *
                  sizeof(unsigned char *));
        if (mw->matrix.selected_cells)
            BCOPY(&mw->matrix.selected_cells[position + num_rows],
                  &mw->matrix.selected_cells[position],
                  (mw->matrix.rows - position - num_rows) * sizeof(Boolean *));
#if XmVersion >= 1002
        if (mw->matrix.highlighted_cells)
            BCOPY(&mw->matrix.highlighted_cells[position + num_rows],
                  &mw->matrix.highlighted_cells[position],
                  (mw->matrix.rows - position - num_rows) *
                  sizeof(unsigned char *));
#endif
    }

    mw->matrix.rows -= num_rows;
}

/*
 * Add columns to the internal cells data structure.
 * If columns or labels is NULL, add empty columns.
 * If max_lengths is NULL, widths will be used.
 * If alignments is NULL, use XmALIGNMENT_BEGINNING.
 * If label_alignments is NULL, use alignments, or if it is NULL
 *   XmALIGNMENT_BEGINNING.
 * widths must not be NULL.
 */
static void
AddColumnsToTable(mw, position, columns, labels, widths, max_lengths,
                  alignments, label_alignments, colors, backgrounds,
                  num_columns)
XbaeMatrixWidget mw;
int position;
String *columns;
String *labels;
short *widths;
int *max_lengths;
unsigned char *alignments;
unsigned char *label_alignments;
Pixel *colors;
Pixel *backgrounds;
int num_columns;
{
    int i, j;

    /*
     * Realloc larger cells, widths, max_lengths, alignments, colors,
     * highlighted_cells, selected_cells, labels and label lines arrays.
     */

    if (mw->matrix.rows == 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "AddColumns", "noRows", "XbaeMatrix",
            "XbaeMatrix: Attempting to add columns with no rows.",
            NULL, 0);
        return;
    }

    for (i = 0; i < mw->matrix.rows; i++)
    {
        if (mw->matrix.cells || columns)
        {
            if (!mw->matrix.cells)
            {
                mw->matrix.columns += num_columns;
                xbaeCopyCells(mw);
                mw->matrix.columns -= num_columns;
            }
            else
                mw->matrix.cells[i] =
                    (String *) XtRealloc((char *) mw->matrix.cells[i],
                                         (mw->matrix.columns + num_columns) *
                                         sizeof(String));
        }
        if (mw->matrix.colors || colors)
        {
            if (!mw->matrix.colors)
            {
                mw->matrix.columns += num_columns;
                xbaeCopyColors(mw);
                mw->matrix.columns -= num_columns;
            }
            else
                mw->matrix.colors[i] =
                    (Pixel *) XtRealloc((char *) mw->matrix.colors[i],
                                        (mw->matrix.columns + num_columns) *
                                        sizeof(Pixel));
        }
        if (mw->matrix.cell_background || backgrounds)
        {
            if (!mw->matrix.cell_background)
            {
                mw->matrix.columns += num_columns;
                xbaeCopyBackgrounds(mw);
                mw->matrix.columns -= num_columns;
            }
            else
                mw->matrix.cell_background[i] =
                    (Pixel *) XtRealloc((char *) mw->matrix.cell_background[i],
                                        (mw->matrix.columns + num_columns) *
                                        sizeof(Pixel));
        }
#if CELL_WIDGETS
        if (mw->matrix.cell_widgets)
            mw->matrix.cell_widgets[i] =
                (Widget *) XtRealloc((char *) mw->matrix.cell_widgets[i],
                                     (mw->matrix.columns + num_columns) *
                                     sizeof(Widget));
#endif
        if (mw->matrix.cell_user_data)
            mw->matrix.cell_user_data[i] = (XtPointer *)
                XtRealloc((char*) mw->matrix.cell_user_data[i],
                          (mw->matrix.columns + num_columns) *
                          sizeof(XtPointer));
        if (mw->matrix.cell_shadow_types)
            mw->matrix.cell_shadow_types[i] = (unsigned char *)
                XtRealloc((char*) mw->matrix.cell_shadow_types[i],
                          (mw->matrix.columns + num_columns) *
                          sizeof(unsigned char));
        if (mw->matrix.selected_cells)
            mw->matrix.selected_cells[i] =
                (Boolean *) XtRealloc((char *) mw->matrix.selected_cells[i],
                                      (mw->matrix.columns + num_columns) *
                                      sizeof(Boolean));
#if XmVersion >= 1002
        if (mw->matrix.highlighted_cells)
            mw->matrix.highlighted_cells[i] =
                (unsigned char *) XtRealloc(
                    (char *) mw->matrix.highlighted_cells[i],
                    (mw->matrix.columns + num_columns) *
                    sizeof(unsigned char));
#endif
    }

    mw->matrix.column_widths =
        (short *) XtRealloc((char *) mw->matrix.column_widths,
                            (mw->matrix.columns + num_columns) *
                            sizeof(short));

    if (mw->matrix.column_max_lengths)
        mw->matrix.column_max_lengths =
            (int *) XtRealloc((char *) mw->matrix.column_max_lengths,
                              (mw->matrix.columns + num_columns) *
                              sizeof(int));

    if (mw->matrix.column_alignments)
        mw->matrix.column_alignments =
            (unsigned char *)
            XtRealloc((char *) mw->matrix.column_alignments,
                      (mw->matrix.columns + num_columns) *
                      sizeof(unsigned char));

    if (mw->matrix.column_button_labels)
        mw->matrix.column_button_labels =
            (Boolean *) XtRealloc((char *) mw->matrix.
                                  column_button_labels,
                                  (mw->matrix.columns + num_columns) *
                                  sizeof(Boolean));

    if (mw->matrix.column_label_alignments)
        mw->matrix.column_label_alignments =
            (unsigned char *) XtRealloc((char *) mw->matrix.
                                        column_label_alignments,
                                        (mw->matrix.columns + num_columns) *
                                        sizeof(unsigned char));

    if (mw->matrix.column_user_data)
        mw->matrix.column_user_data = (XtPointer*)
            XtRealloc((char*) mw->matrix.column_user_data,
                      (mw->matrix.columns + num_columns) *
                      sizeof(XtPointer));

    if (mw->matrix.column_shadow_types)
        mw->matrix.column_shadow_types = (unsigned char *)
            XtRealloc((char*) mw->matrix.column_shadow_types,
                      (mw->matrix.columns + num_columns) *
                      sizeof(unsigned char));

    if (mw->matrix.column_labels)
    {
        mw->matrix.column_labels =
            (String *) XtRealloc((char *) mw->matrix.column_labels,
                                 (mw->matrix.columns + num_columns) *
                                 sizeof(String));
        mw->matrix.column_label_lines =
            (ColumnLabelLines) XtRealloc(
                (char *) mw->matrix.column_label_lines, (mw->matrix.columns +
                                                         num_columns) *
                sizeof(ColumnLabelLinesRec));
    }

    /*
     * If we are inserting columns into the middle, we need to make room.
     */
    if (position < mw->matrix.columns)
    {
        BCOPY(&mw->matrix.column_widths[position],
              &mw->matrix.column_widths[position + num_columns],
              (mw->matrix.columns - position) * sizeof(short));

        if (mw->matrix.column_max_lengths)
            BCOPY(&mw->matrix.column_max_lengths[position],
                  &mw->matrix.column_max_lengths[position + num_columns],
                  (mw->matrix.columns - position) * sizeof(int));

        if (mw->matrix.column_alignments)
            BCOPY(&mw->matrix.column_alignments[position],
                  &mw->matrix.column_alignments[position + num_columns],
                  (mw->matrix.columns - position) * sizeof(unsigned char));

        if (mw->matrix.column_button_labels)
            BCOPY(&mw->matrix.column_button_labels[position],
                  &mw->matrix.column_button_labels[position + num_columns],
                  (mw->matrix.columns - position) * sizeof(Boolean));

        if (mw->matrix.column_label_alignments)
            BCOPY(&mw->matrix.column_label_alignments[position],
                  &mw->matrix.column_label_alignments[position + num_columns],
                  (mw->matrix.columns - position) * sizeof(unsigned char));

        if (mw->matrix.column_user_data)
            BCOPY(&mw->matrix.column_user_data[position],
                  &mw->matrix.column_user_data[position + num_columns],
                  (mw->matrix.columns - position) * sizeof(XtPointer));

        if (mw->matrix.column_shadow_types)
            BCOPY(&mw->matrix.column_shadow_types[position],
                  &mw->matrix.column_shadow_types[position + num_columns],
                  (mw->matrix.columns - position) * sizeof(unsigned char));
        
        if (mw->matrix.column_labels)
        {
            BCOPY(&mw->matrix.column_labels[position],
                  &mw->matrix.column_labels[position + num_columns],
                  (mw->matrix.columns - position) * sizeof(String));
            BCOPY(&mw->matrix.column_label_lines[position],
                  &mw->matrix.column_label_lines[position + num_columns],
                  (mw->matrix.columns - position) *
                  sizeof(ColumnLabelLinesRec));
        }

        /*
         * Shift the columns in each row.
         */
        for (i = 0; i < mw->matrix.rows; i++)
        {
            if (mw->matrix.cells)
                BCOPY(&mw->matrix.cells[i][position],
                      &mw->matrix.cells[i][position + num_columns],
                      (mw->matrix.columns - position) * sizeof(String));
            if (mw->matrix.colors)
                BCOPY(&mw->matrix.colors[i][position],
                      &mw->matrix.colors[i][position + num_columns],
                      (mw->matrix.columns - position) * sizeof(Pixel));
            if (mw->matrix.cell_background)
                BCOPY(&mw->matrix.cell_background[i][position],
                      &mw->matrix.cell_background[i][position + num_columns],
                      (mw->matrix.columns - position) * sizeof(Pixel));
#if CELL_WIDGETS
            if (mw->matrix.cell_widgets)
                BCOPY(&mw->matrix.cell_widgets[i][position],
                      &mw->matrix.cell_widgets[i][position + num_columns],
                      (mw->matrix.columns - position) * sizeof(Widget));
#endif
            if (mw->matrix.cell_user_data)
                BCOPY(&mw->matrix.cell_user_data[i][position],
                      &mw->matrix.cell_user_data[i][position + num_columns],
                      (mw->matrix.columns - position) * sizeof(XtPointer));
            if (mw->matrix.cell_shadow_types)
                BCOPY(&mw->matrix.cell_shadow_types[i][position],
                      &mw->matrix.cell_shadow_types[i][position + num_columns],
                      (mw->matrix.columns - position) * sizeof(unsigned char));
            if (mw->matrix.selected_cells)
                BCOPY(&mw->matrix.selected_cells[i][position],
                      &mw->matrix.selected_cells[i][position + num_columns],
                      (mw->matrix.columns - position) * sizeof(Boolean));
#if XmVersion >= 1002
            if (mw->matrix.highlighted_cells)
                BCOPY(&mw->matrix.highlighted_cells[i][position],
                      &mw->matrix.highlighted_cells[i][position + num_columns],
                      (mw->matrix.columns - position) * sizeof(unsigned char));
#endif
        }
    }

    /*
     * Copy all of the passed in info into each new column
     * (except column_positions which will be recalculated below).
     * If columns or labels is NULL, add empty columns.
     * If max_lengths is NULL, widths will be used.
     * If alignments is NULL, use XmALIGNMENT_BEGINNING.
     * If label_alignments is NULL, use XmALIGNMENT_BEGINNING
     * If labels is NULL, use NULL strings.
     * If colors is NULL, use foreground.
     * Use False for new  button label flags.
     */
    for (j = 0; j < num_columns; j++)
    {
        mw->matrix.column_widths[j + position] = widths[j];

        if (mw->matrix.column_max_lengths)
            mw->matrix.column_max_lengths[j + position] =
                max_lengths ? max_lengths[j] : (int) widths[j];

        if (mw->matrix.column_alignments)
            mw->matrix.column_alignments[j + position] =
                alignments ? alignments[j] : XmALIGNMENT_BEGINNING;

        if (mw->matrix.column_button_labels)
            mw->matrix.column_button_labels[j + position] = False;
        
        if (mw->matrix.column_label_alignments)
            mw->matrix.column_label_alignments[j + position] =
                label_alignments ? label_alignments[j] : XmALIGNMENT_BEGINNING;
        
        if (mw->matrix.column_user_data)
            mw->matrix.column_user_data[j + position] = (XtPointer) NULL;

#if CELL_WIDGETS
        if (mw->matrix.cell_widgets)
            mw->matrix.cell_widgets[j + position] = NULL;
#endif

        if (mw->matrix.column_shadow_types)
            mw->matrix.column_shadow_types[j + position] =
                mw->matrix.cell_shadow_type;

        if (mw->matrix.column_labels)
        {
            mw->matrix.column_labels[j + position] =
                labels ? XtNewString(labels[j]) : XtNewString("");
            xbaeParseColumnLabel(mw->matrix.column_labels[j + position],
                                 &mw->matrix.column_label_lines[j + position]);
        }

        /*
         * Add this new column to each row.
         */
        for (i = 0; i < mw->matrix.rows; i++)
        {
            if (mw->matrix.cells)
                mw->matrix.cells[i][j + position] = columns ?
                    XtNewString(columns[i * num_columns + j]) :
                XtNewString("");
            if (mw->matrix.colors)
                mw->matrix.colors[i][j + position] =
                    colors ? colors[j] : mw->manager.foreground;
            if (mw->matrix.cell_background)
                mw->matrix.cell_background[i][j + position] =
                    backgrounds ? backgrounds[j] : mw->core.background_pixel;
#if CELL_WIDGETS
            if (mw->matrix.cell_widgets)
                mw->matrix.cell_widgets[i][j + position] = (Widget) NULL;
#endif
            if (mw->matrix.cell_user_data)
                mw->matrix.cell_user_data[i][j + position] = (XtPointer) NULL;
            if (mw->matrix.cell_shadow_types)
                mw->matrix.cell_shadow_types[i][j + position] =
                    mw->matrix.cell_shadow_type;
            if (mw->matrix.selected_cells)
                mw->matrix.selected_cells[i][j + position] = False;
#if XmVersion >= 1002
            if (mw->matrix.highlighted_cells)
                mw->matrix.highlighted_cells[i][j + position] = HighlightNone;
#endif
        }
    }

    mw->matrix.columns += num_columns;
    xbaeGetCellTotalWidth(mw);

    /*
     * See if the max number of column label lines changed
     */
    if (mw->matrix.column_labels)
    {
        int end;

        end = position + num_columns;
        for (i = position; i < end; i++)
            if (mw->matrix.column_label_lines[i].lines >
                mw->matrix.column_label_maxlines)
                mw->matrix.column_label_maxlines =
                    mw->matrix.column_label_lines[i].lines;
    }

    /*
     * Recalculate the column positions
     */
    xbaeFreeColumnPositions(mw);
    mw->matrix.column_positions = CreateColumnPositions(mw);
    xbaeGetColumnPositions(mw);
}

/*
 * Delete columns from the internal cells data structure.
 */
static void
DeleteColumnsFromTable(mw, position, num_columns)
XbaeMatrixWidget mw;
int position;
int num_columns;
{
    int i, j;

    /*
     * Free all the cells in the columns being deleted.
     * Also free the String column labels and the associated ColumnLabelLines
     * lengths arrays, and the column button label flags.
     */
    for (j = position; j < position + num_columns; j++)
    {
        if (mw->matrix.cells)
            for (i = 0; i < mw->matrix.rows; i++) {
                XtFree((XtPointer) mw->matrix.cells[i][j]);
                mw->matrix.cells[i][j] = NULL;
            }    
        if (mw->matrix.column_labels)
        {
            XtFree((XtPointer) mw->matrix.column_labels[j]);
            mw->matrix.column_labels[j] = NULL;
            XtFree((XtPointer) mw->matrix.column_label_lines[j].lengths);
            mw->matrix.column_label_lines[j].lengths = NULL;
        }
    }

    /*
     * Shift those columns after the ones being deleted, left.
     * (unless we deleted columns from the right).
     */
    if (position + num_columns < mw->matrix.columns)
    {
        BCOPY(&mw->matrix.column_widths[position + num_columns],
              &mw->matrix.column_widths[position],
              (mw->matrix.columns - position - num_columns) * sizeof(short));

        if (mw->matrix.column_max_lengths)
            BCOPY(&mw->matrix.column_max_lengths[position + num_columns],
                  &mw->matrix.column_max_lengths[position],
                  (mw->matrix.columns - position - num_columns) * sizeof(int));

        if (mw->matrix.column_alignments)
            BCOPY(&mw->matrix.column_alignments[position + num_columns],
                  &mw->matrix.column_alignments[position],
                  (mw->matrix.columns - position - num_columns) *
                  sizeof(unsigned char));

        if (mw->matrix.column_button_labels)
            BCOPY(&mw->matrix.column_button_labels[position + num_columns],
                  &mw->matrix.column_button_labels[position],
                  (mw->matrix.columns - position - num_columns) *
                  sizeof(Boolean));

        if (mw->matrix.column_label_alignments)
            BCOPY(&mw->matrix.column_label_alignments[position + num_columns],
                  &mw->matrix.column_label_alignments[position],
                  (mw->matrix.columns - position - num_columns) *
                  sizeof(unsigned char));

        if (mw->matrix.column_user_data)
            BCOPY(&mw->matrix.column_user_data[position + num_columns],
                  &mw->matrix.column_user_data[position],
                  (mw->matrix.columns - position - num_columns) *
                  sizeof(XtPointer));

#if CELL_WIDGETS
        if (mw->matrix.cell_widgets)
            BCOPY(&mw->matrix.cell_widgets[position + num_columns],
                  &mw->matrix.cell_widgets[position],
                  (mw->matrix.columns - position - num_columns) *
                  sizeof(Widget));
#endif

        if (mw->matrix.column_shadow_types)
            BCOPY(&mw->matrix.column_shadow_types[position + num_columns],
                  &mw->matrix.column_shadow_types[position],
                  (mw->matrix.columns - position - num_columns) *
                  sizeof(unsigned char));

        if (mw->matrix.column_labels)
        {
            BCOPY(&mw->matrix.column_labels[position + num_columns],
                  &mw->matrix.column_labels[position],
                  (mw->matrix.columns - position - num_columns) *
                  sizeof(String));
            BCOPY(&mw->matrix.column_label_lines[position + num_columns],
                  &mw->matrix.column_label_lines[position],
                  (mw->matrix.columns - position - num_columns) *
                  sizeof(ColumnLabelLinesRec));
        }

        /*
         * Shift the columns in each row.
         */
        for (i = 0; i < mw->matrix.rows; i++)
        {
            if (mw->matrix.cells)
                BCOPY(&mw->matrix.cells[i][position + num_columns],
                      &mw->matrix.cells[i][position],
                      (mw->matrix.columns - position - num_columns) *
                      sizeof(String));
            if (mw->matrix.colors)
                BCOPY(&mw->matrix.colors[i][position + num_columns],
                      &mw->matrix.colors[i][position],
                      (mw->matrix.columns - position - num_columns) *
                      sizeof(Pixel));
            if (mw->matrix.cell_background)
                BCOPY(&mw->matrix.cell_background[i][position + num_columns],
                      &mw->matrix.cell_background[i][position],
                      (mw->matrix.columns - position - num_columns) *
                      sizeof(Pixel));
#if CELL_WIDGETS
            if (mw->matrix.cell_widgets)
                BCOPY(&mw->matrix.cell_widgets[i][position + num_columns],
                      &mw->matrix.cell_widgets[i][position],
                      (mw->matrix.columns - position - num_columns) *
                      sizeof(Widget));
#endif
            if (mw->matrix.cell_user_data)
                BCOPY(&mw->matrix.cell_user_data[i][position + num_columns],
                      &mw->matrix.cell_user_data[i][position],
                      (mw->matrix.columns - position - num_columns) *
                      sizeof(XtPointer));
            if (mw->matrix.cell_shadow_types)
                BCOPY(&mw->matrix.cell_shadow_types[i][position + num_columns],
                      &mw->matrix.cell_shadow_types[i][position],
                      (mw->matrix.columns - position - num_columns) *
                      sizeof(unsigned char));
            if (mw->matrix.selected_cells)
                BCOPY(&mw->matrix.selected_cells[i][position + num_columns],
                      &mw->matrix.selected_cells[i][position],
                      (mw->matrix.columns - position - num_columns) *
                      sizeof(Boolean));
#if XmVersion >= 1002
            if (mw->matrix.highlighted_cells)
                BCOPY(&mw->matrix.highlighted_cells[i][position + num_columns],
                      &mw->matrix.highlighted_cells[i][position],
                      (mw->matrix.columns - position - num_columns) *
                      sizeof(unsigned char));
#endif
        }
    }

    mw->matrix.columns -= num_columns;
    xbaeGetCellTotalWidth(mw);

    /*
     * See if the max number of column label lines changed
     */
    if (mw->matrix.column_labels)
    {
        mw->matrix.column_label_maxlines =
            mw->matrix.column_label_lines[0].lines;
        for (i = 1; i < mw->matrix.columns; i++)
            if (mw->matrix.column_label_lines[i].lines >
                mw->matrix.column_label_maxlines)
                mw->matrix.column_label_maxlines =
                    mw->matrix.column_label_lines[i].lines;
    }

    /*
     * Recalculate the column positions
     */
    xbaeFreeColumnPositions(mw);
    mw->matrix.column_positions = CreateColumnPositions(mw);
    xbaeGetColumnPositions(mw);
}

/*
 * Matrix set_cell method
 */
void
#if NeedFunctionPrototypes
xbaeSetCell(XbaeMatrixWidget mw, int row, int column, const String value,
            Boolean update_text)
#else
xbaeSetCell(mw, row, column, value, update_text)
XbaeMatrixWidget mw;
int row;
int column;
const String value;
Boolean update_text;
#endif
{
    if (row >= mw->matrix.rows || row < 0 ||
        column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "xbaeSetCell", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row or column out of bounds for xbaeSetCell.",
            NULL, 0);
        return;
    }

    /*
     * If we have a draw cell callback, we must have a write cell callback
     * also if we want to set the data.  Use this callback to write the
     * new data back to the application.
     */
    if (mw->matrix.draw_cell_callback)
    {
         XbaeMatrixWriteCellCallbackStruct call_data;

        if (mw->matrix.write_cell_callback)
        {
            call_data.reason = XbaeWriteCellReason;
            call_data.event = (XEvent *)NULL;
            call_data.row = row;
            call_data.column = column;
            call_data.string = value;
            call_data.type = XbaeString;
            call_data.pixmap = (Pixmap)NULL;
            call_data.mask = (Pixmap)NULL;

            XtCallCallbackList((Widget)mw, mw->matrix.write_cell_callback,
                               (XtPointer) &call_data);
        }
    }
    else
    {
        /*
         * Store the new value in the cell.
         */
        if (!mw->matrix.cells && value[0] != 0)
            /*
             * The user typed something, there is no drawCellCallback and
             * our cells have not been allocated :-(
             * The typed value must be stored, so allocate the cells array
             * now.
             */
            xbaeCopyCells(mw);
        /*
         * Now we are free to store the value in the widget's cell array
         */
        if (mw->matrix.cells &&        /* It's OK to store the value */
            strcmp(mw->matrix.cells[row][column], value))
        {
            /*
             * I'm not particularly keen on this code - ie. checking twice
             * for mw->matrix.cells but it seemed like the only way around
             * the problem AL (Nov 5, 1995).
             */
            XtFree((XtPointer) mw->matrix.cells[row][column]);
            mw->matrix.cells[row][column] = XtNewString(value);
        }
        else
            return;
            
    }
    /*
     * Draw the cell.
     */
    if (xbaeIsCellVisible(mw, row, column))
    {
        xbaeClearCell(mw, row, column);
        xbaeDrawCell(mw, row, column);
    }

    /*
     * If we are editing this cell, load the textField too if update_text set.
     */
    if (update_text && XtIsManaged(TextChild(mw)) &&
        mw->matrix.current_row == row && mw->matrix.current_column == column)
    {
        String string;

        /* Remove the modify verify callback when the text field is set.
           It thinks we are modifying the value - Motif thinks that
           it knows best but we know better! */
        XtRemoveCallback(TextChild(mw), XmNmodifyVerifyCallback,
                         xbaeModifyVerifyCB, (XtPointer)mw);

        /*
         * We need to get the value to put back into the textField if the
         * application has a draw cell callback so that any reformatting will
         * be displayed. -cg May 13, 1999.
         */
        if (mw->matrix.draw_cell_callback)
        {
            Pixmap pixmap, mask;
            Pixel bg, fg;
            int width, height, depth;

            xbaeGetDrawCellValue(mw, mw->matrix.current_row,
                                 mw->matrix.current_column, &string,
                                 &pixmap, &mask, &width, &height,
                                 &bg, &fg, &depth);
        }
        else
            string = value;

        if (string[0] == '\0')
            XtVaSetValues(TextChild(mw),
                          XmNvalue, string,
                          NULL);
        else
            XmTextSetString(TextChild(mw), string);

        XtAddCallback(TextChild(mw), XmNmodifyVerifyCallback,
                      xbaeModifyVerifyCB, (XtPointer)mw);
    }
}

static Boolean
DoCommitEdit(mw, event)
XbaeMatrixWidget mw;
XEvent *event;
{
    String cell;

    if (!XtIsManaged(TextChild(mw)))
        return True;

    /*
     * Get the value the user entered in the textField (this is a copy)
     */
    cell = XmTextGetString(TextChild(mw));

    /*
     * Call the leaveCellCallback to see if we can leave the current cell.
     */
    if (mw->matrix.leave_cell_callback)
    {
        XbaeMatrixLeaveCellCallbackStruct call_data;

        call_data.reason = XbaeLeaveCellReason;
        call_data.event = event;
        call_data.row = mw->matrix.current_row;
        call_data.column = mw->matrix.current_column;
        call_data.value = cell;
        call_data.doit = True;

        XtCallCallbackList((Widget) mw, mw->matrix.leave_cell_callback,
                           (XtPointer)&call_data);

        /*
         * Application doesn't want to leave this cell. Make the cell visible
         * and traverse to it so the user can see where they screwed up.
         */
        if (!call_data.doit)
        {
            xbaeMakeCellVisible(
                mw, mw->matrix.current_row, mw->matrix.current_column);
            XmProcessTraversal(TextChild(mw), XmTRAVERSE_CURRENT);
            XtFree((XtPointer) cell);
            return False;
        }

        /*
         * Use the applications value if it is different.
         * If the application modified the string inplace, we will pick that
         * up automatically.
         */
        if (call_data.value != cell)
        {
            XtFree((XtPointer) cell);
            cell = call_data.value;
        }
    }

    /*
     * Call the set_cell method to store the new value in the cell and redraw.
     */
    (*((XbaeMatrixWidgetClass) XtClass(mw))->matrix_class.set_cell)
        (mw, mw->matrix.current_row, mw->matrix.current_column, cell, True);

    XtFree((XtPointer) cell);

    return True;
}

/*
 * Position and size the scrollbars and clip widget for our new size.
 */
void
xbaeResize(mw)
XbaeMatrixWidget mw;
{
    int cell_width, cell_height, rows_visible;
    Boolean has_horiz, has_vert;
    Boolean scrollbar_top;
    Boolean scrollbar_left;
    int width = mw->core.width;
    int height = mw->core.height;

    /*
     * Full size of widget (no SBs needed) - may be very large
     */
    long int full_width = NON_FIXED_TOTAL_WIDTH(mw) + FIXED_COLUMN_WIDTH(mw) +
        TRAILING_FIXED_COLUMN_WIDTH(mw) + ROW_LABEL_WIDTH(mw) +
        2 * mw->manager.shadow_thickness;
    long int full_height = CELL_TOTAL_HEIGHT(mw) + FIXED_ROW_HEIGHT(mw) +
        TRAILING_FIXED_ROW_HEIGHT(mw) + COLUMN_LABEL_HEIGHT(mw) +
        2 * mw->manager.shadow_thickness;

    /*
     * Portion of cells which are visible in clip widget
     */
    int horiz_visible = NON_FIXED_TOTAL_WIDTH(mw) - HORIZ_ORIGIN(mw);
    int vert_visible = CELL_TOTAL_HEIGHT(mw) -
        VERT_ORIGIN(mw) * ROW_HEIGHT(mw);
    
    /*
     * Check the location of the scrollbars
     */
    scrollbar_top = (mw->matrix.scrollbar_placement == XmTOP_LEFT ||
                     mw->matrix.scrollbar_placement == XmTOP_RIGHT);
    scrollbar_left = (mw->matrix.scrollbar_placement == XmBOTTOM_LEFT ||
                      mw->matrix.scrollbar_placement == XmTOP_LEFT);
    /*
     * If our horizontal scrollbar display policy is constant,
     * then we always have the horizontal scrollbar. If it is
     * none, then we never have it. Otherwise, check if it
     * is needed: if we are wider than the matrix's width,
     * then we don't need it; if we are smaller, we do.
     */
    if (mw->matrix.hsb_display_policy == XmDISPLAY_STATIC)
        has_horiz = TRUE;
    else if (mw->matrix.hsb_display_policy == XmDISPLAY_NONE)
        has_horiz = FALSE;
    else
    {
        if (width >= full_width)
            has_horiz = False;
        else
            has_horiz = True;
    }
    if (has_horiz)
        height -= HORIZ_SB_HEIGHT(mw);
    
    /*
     * Same reasoning for the vertical scrollbar.
     */
    if (mw->matrix.vsb_display_policy == XmDISPLAY_STATIC)
        has_vert = TRUE;
    else if (mw->matrix.vsb_display_policy == XmDISPLAY_NONE)
        has_vert = FALSE;
    else
    {
        if (height >= full_height)
            has_vert = False;
        else
            has_vert = True;
    }

    /*
     * If we have a vertical scrollbar, adjust the width and
     * recheck if we need the horizontal scrollbar.
     */
    if (has_vert)
    {
        width -= VERT_SB_WIDTH(mw);
        if ((XmDISPLAY_NONE != mw->matrix.hsb_display_policy) &&
            (! has_horiz) && (width < full_width))
        {
            has_horiz = True;
            height -= HORIZ_SB_HEIGHT(mw);
        }
    }

    /*
     * If widget is smaller than full size, move/resize the scrollbar and
     * set sliderSize, also if cell_width/cell_height is greater than
     * the amount of cell area visible, then we need to drag the cells
     * back into the visible part of the clip widget and set the
     * scrollbar value.
     *
     * Otherwise, the widget is larger than full size, so set
     * cell_width/cell_height to size of cells and set origin to 0
     * to force full cell area to be displayed
     *
     * We also need to move the textField correspondingly
     */

    /*
     * We were resized smaller than our max width.
     */
    if (width < full_width)
    {
        int HSBwidth;
        
        /*
         * Calculate the width of the non-fixed visible cells.
         */
        cell_width = mw->core.width - (FIXED_COLUMN_WIDTH(mw) +
                                       TRAILING_FIXED_COLUMN_WIDTH(mw) +
                                       ROW_LABEL_WIDTH(mw) +
                                       2 * mw->manager.shadow_thickness);

        /*
         * Subtract the VSB if we have one.
         */
        if (has_vert)
            cell_width -= VERT_SB_WIDTH(mw);

        if (cell_width <= 0)
            cell_width = 1;
            
        /*
         * Adjust for shadow thickness.
         */
        HSBwidth = cell_width + mw->manager.shadow_thickness *
            (mw->matrix.fixed_columns ||
             mw->matrix.trailing_fixed_columns ?
             (mw->matrix.fixed_columns &&
              mw->matrix.trailing_fixed_columns ? 0 : 1) : 2);
        
        /*
         * If the window is not full height, then place the HSB at the edge
         * of the window.  Is the window is larger than full height, then
         * place the HSB immediately below the cell region.
         */
        XtConfigureWidget(
            HorizScrollChild(mw),
            HSB_X_POSITION(mw),
            (scrollbar_top && has_horiz) ? (Position) 0 :
            ((height < full_height) || mw->matrix.fill ?
             (Position) (mw->core.height -
                         (HorizScrollChild(mw)->core.height +
                          2 * HorizScrollChild(mw)->core.border_width)) :
             (Position) (full_height + mw->matrix.space)), HSBwidth,
            HorizScrollChild(mw)->core.height,
            HorizScrollChild(mw)->core.border_width);

        /*
         * If the cells are scrolled off to the left, then drag them
         * back onto the screen.
         */
        if (cell_width > horiz_visible)
        {
            if ((HORIZ_ORIGIN(mw) -= (cell_width - horiz_visible)) < 0)
            {
                HORIZ_ORIGIN(mw) = 0;
                mw->matrix.left_column = 0;
            }

            if (XtIsManaged(TextChild(mw)))
                XtMoveWidget(TextChild(mw),
                             TextChild(mw)->core.x +
                             (cell_width - horiz_visible),
                             TextChild(mw)->core.y);
        }

        /*
         * Setup the HSB to reflect our new size.
         */
        XtVaSetValues(HorizScrollChild(mw),
                      XmNpageIncrement, cell_width,
                      XmNsliderSize, cell_width,
                      XmNvalue, HORIZ_ORIGIN(mw),
                      NULL);
    }

    /*
     * We were resized larger than the our max width.  Drag the cells back
     * onto the screen if they were scrolled off to the left.
     */
    else
    {
        if (XtIsManaged(TextChild(mw)))
            XtMoveWidget(TextChild(mw),
                         TextChild(mw)->core.x + HORIZ_ORIGIN(mw),
                         TextChild(mw)->core.y);

        cell_width = NON_FIXED_TOTAL_WIDTH(mw);
        if (cell_width <= 0) cell_width = 1;
        
        HORIZ_ORIGIN(mw) = 0;
        mw->matrix.left_column = 0;

        if (has_horiz)
        {
            XtConfigureWidget(
                HorizScrollChild(mw),
                HSB_X_POSITION(mw),
                (scrollbar_top && has_horiz) ? (Position) 0 :
                ((height < full_height) || mw->matrix.fill ?
                 (Position) (mw->core.height -
                             (HorizScrollChild(mw)->core.height +
                              2 * HorizScrollChild(mw)->core.border_width)) :
                 (Position) (full_height + mw->matrix.space)),
                HSB_WIDTH(mw),
                HorizScrollChild(mw)->core.height,
                HorizScrollChild(mw)->core.border_width);

            XtVaSetValues(HorizScrollChild(mw),
                          XmNpageIncrement, cell_width,
                          XmNsliderSize, cell_width,
                          XmNvalue, HORIZ_ORIGIN(mw),
                          NULL);
        }
    }

    /*
     * We were resized smaller than our max height.
     */
    if (height < full_height)
    {
        int VSBheight;

        /*
         * Calculate the height of the non-fixed visible cells.
         */
        cell_height = mw->core.height -
            (FIXED_ROW_HEIGHT(mw) +  TRAILING_FIXED_ROW_HEIGHT(mw) +
             COLUMN_LABEL_HEIGHT(mw) + 2 * mw->manager.shadow_thickness);

        /*
         * Subtract the HSB if we have one.
         */
        if (has_horiz)
            cell_height -= HORIZ_SB_HEIGHT(mw);

        if (cell_height <= 0)
            cell_height = 1;

        /*
         * Adjust for shadow thickness.
         */
        if (TRAILING_FIXED_ROW_HEIGHT(mw) > 0)
            VSBheight = (cell_height / ROW_HEIGHT(mw)) * ROW_HEIGHT(mw) +
                (mw->matrix.fixed_rows ? 0 : mw->manager.shadow_thickness);
        else
            VSBheight = cell_height +
               ((mw->matrix.fixed_rows ? 1 : 2) *
                mw->manager.shadow_thickness);

        /*
         * If the window is not full width, then place the VSB at the edge
         * of the window.  Is the window is larger than full width, then
         * place the VSB immediately to the right of the cell region.
         */
        XtConfigureWidget(
            VertScrollChild(mw),
            (scrollbar_left && has_vert) ? (Position) 0 :
            ((width < full_width) || mw->matrix.fill ?
             (Position) (mw->core.width -
                         (VertScrollChild(mw)->core.width + 2 *
                          VertScrollChild(mw)->core.border_width)) :
             (Position) full_width + mw->matrix.space),
            VSB_Y_POSITION(mw), 
            VertScrollChild(mw)->core.width, VSBheight > 0 ? VSBheight : 1,
            VertScrollChild(mw)->core.border_width);
        /*
         * If the cells are scrolled off the top, then drag them
         * back onto the screen.
         */
        if (cell_height > vert_visible)
        {
            int rows = (cell_height - vert_visible) / ROW_HEIGHT(mw);

            VERT_ORIGIN(mw) -= rows;

            if (XtIsManaged(TextChild(mw)))
                XtMoveWidget(TextChild(mw),
                             TextChild(mw)->core.x,
                             TextChild(mw)->core.y + rows * ROW_HEIGHT(mw));
        }

        /*
         * Setup the VSB to reflect our new size.
         */
        rows_visible = cell_height / ROW_HEIGHT(mw);
        XtVaSetValues(VertScrollChild(mw),
                      XmNpageIncrement, rows_visible <= 0 ? 1 : rows_visible,
                      XmNsliderSize, rows_visible <= 0 ? 1 : rows_visible,
                      XmNvalue, VERT_ORIGIN(mw),
                      NULL);
    }

    /*
     * We were resized larger than the our max height.        Drag the cells back
     * onto the screen if they were scrolled off the top.
     */
    else
    {
        if (XtIsManaged(TextChild(mw)))
            XtMoveWidget(TextChild(mw),
                         TextChild(mw)->core.x,
                         TextChild(mw)->core.y +
                         VERT_ORIGIN(mw) * ROW_HEIGHT(mw));

        cell_height = CELL_TOTAL_HEIGHT(mw);

        if (cell_height <= 0)
            cell_height = 1;
        
        rows_visible = mw->matrix.rows - mw->matrix.fixed_rows -
            mw->matrix.trailing_fixed_rows;
        VERT_ORIGIN(mw) = 0;

        if (has_vert)
        {
            XtConfigureWidget(
                VertScrollChild(mw),
                (scrollbar_left && has_vert) ? (Position) 0 :
                ((width < full_width) || mw->matrix.fill ?
                 (Position)(mw->core.width -
                            (VertScrollChild(mw)->core.width +
                             2 * VertScrollChild(mw)->core.border_width)) :
                 (Position)full_width + mw->matrix.space),
                VSB_Y_POSITION(mw),
                VertScrollChild(mw)->core.width,
                VSB_HEIGHT(mw),
                VertScrollChild(mw)->core.border_width);
            
            XtVaSetValues(
                VertScrollChild(mw),
                XmNpageIncrement, rows_visible <= 0 ? 1 : rows_visible,
                XmNsliderSize, rows_visible <= 0 ? 1 : rows_visible,
                XmNvalue, VERT_ORIGIN(mw),
                NULL);
        }
    }

    /*
     * Map/unmap scrollbars based on flags set above
     */
    if (has_horiz && !HorizScrollChild(mw)->core.managed)
    {
        XtManageChild(HorizScrollChild(mw));
        /*
         * Generate an expose over the horizontal scrollbar to ensure it gets
         * drawn properly
         */
        if (!mw->matrix.disable_redisplay && XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw), 0, HORIZ_SB_POSITION(mw),
                       mw->core.width, scrollbar_top ? HORIZ_SB_HEIGHT(mw) :
                       mw->core.height - HORIZ_SB_POSITION(mw), True);
        /*
         * Take into account the little bit at the bottom of the screen
         * if the scrollbar is in the top location.
         */
        if (scrollbar_top && XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw), 0,
                       TRAILING_FIXED_ROW_LABEL_OFFSET(mw), mw->core.width,
                       mw->core.height - TRAILING_FIXED_ROW_LABEL_OFFSET(mw),
                       True);
    }
    else if (!has_horiz && HorizScrollChild(mw)->core.managed)
    {
        /*
         * Generate an expose over the horizontal scrollbar to ensure it gets
         * drawn properly
         */
        XtUnmanageChild(HorizScrollChild(mw));
        if (!mw->matrix.disable_redisplay && XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw), 0, HORIZ_SB_POSITION(mw),
                       mw->core.width, scrollbar_top ? HORIZ_SB_HEIGHT(mw) :
                       mw->core.height - HORIZ_SB_POSITION(mw), True);
        /*
         * Take into account the little bit at the bottom of the screen
         * if the scrollbar is in the top location.
         */
        if (scrollbar_top && XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw), 0,
                       TRAILING_FIXED_ROW_LABEL_OFFSET(mw), mw->core.width,
                       mw->core.height - TRAILING_FIXED_ROW_LABEL_OFFSET(mw),
                       True);

    }
    if (has_vert && !VertScrollChild(mw)->core.managed)
    {
        /*
         * Generate an expose over the vertical scrollbar region to ensure
         * it gets drawn properly
         */
        XtManageChild(VertScrollChild(mw));
        if (!mw->matrix.disable_redisplay && XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw), VERT_SB_POSITION(mw), 0,
                       VERT_SB_POSITION(mw) + VERT_SB_WIDTH(mw),
                       mw->core.height, True);
        /*
         * This one's a bit trickier!  If the scrollbar appears on the
         * left hand side then it's possible to have some area *under* the
         * matrix that isn't redrawn properly.  This will be most
         * noticeable at the top and bottom of the matrix.  As the matrix
         * will move to the right by it's width, this width is the area
         * that needs to be redrawn.
         */
        if (scrollbar_left && XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       FIXED_COLUMN_LABEL_OFFSET(mw) -
                       TRAILING_FIXED_COLUMN_WIDTH(mw), 0,
                       TRAILING_FIXED_COLUMN_WIDTH(mw) + VERT_SB_WIDTH(mw),
                       mw->core.height, True);
        /*
         * Also clear the area below the matrix as it can get a little
         * confused too
         */
        if(XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw), 0,
                       TRAILING_FIXED_ROW_LABEL_OFFSET(mw), mw->core.width,
                       mw->core.height - TRAILING_FIXED_ROW_LABEL_OFFSET(mw),
                       True);
    }
    else if (!has_vert && VertScrollChild(mw)->core.managed)
    {
        /*
         * Generate an expose over the vertical scrollbar region to ensure
         * it gets drawn properly
         */
        XtUnmanageChild(VertScrollChild(mw));
        if (!mw->matrix.disable_redisplay && XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw), VERT_SB_POSITION(mw), 0,
                       VERT_SB_POSITION(mw) + VERT_SB_WIDTH(mw),
                       mw->core.height, True);
        /*
         * Similar to the case above but we need to clear the are to the
         * right of the matrix and only the width of the scrollbar itself
         */
        if (scrollbar_left && XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw) +
                       TRAILING_FIXED_COLUMN_WIDTH(mw), 0,
                       VERT_SB_WIDTH(mw) + mw->manager.shadow_thickness,
                       mw->core.height, True);
        /*
         * And also clear the area below the matrix as it can get a little
         * confused too
         */
        if (XtIsRealized((Widget)mw))
            XClearArea(XtDisplay(mw), XtWindow(mw), 0,
                       TRAILING_FIXED_ROW_LABEL_OFFSET(mw), mw->core.width,
                       mw->core.height - TRAILING_FIXED_ROW_LABEL_OFFSET(mw),
                       True);
    }
    /*
     * Now that we have cell_width & cell_height,
     * make the clip widget this size.        Height is truncated to the
     * nearest row.
     */
    XtConfigureWidget(ClipChild(mw),
                      FIXED_COLUMN_LABEL_OFFSET(mw),
                      FIXED_ROW_LABEL_OFFSET(mw),
                      cell_width, cell_height, 0);

    /* Resize all the other clips */
    if (mw->matrix.fixed_columns <= 0)
    {
        if (XtIsManaged(LeftClip(mw)))
            XtUnmanageChild(LeftClip(mw));
    }
    else
    {
        XtConfigureWidget(LeftClip(mw),
                          COLUMN_LABEL_OFFSET(mw), FIXED_ROW_LABEL_OFFSET(mw),
                          FIXED_COLUMN_WIDTH(mw), cell_height, 0);
        if (!XtIsManaged(LeftClip(mw)))
            XtManageChild(LeftClip(mw));
    }
    if (mw->matrix.trailing_fixed_columns <= 0)
    {
               if (XtIsManaged(RightClip(mw)))
            XtUnmanageChild(RightClip(mw));
    }
    else
    {
        XtConfigureWidget(RightClip(mw),
                          cell_width + FIXED_COLUMN_LABEL_OFFSET(mw),
                          FIXED_ROW_LABEL_OFFSET(mw),
                          TRAILING_FIXED_COLUMN_WIDTH(mw),
                          cell_height, 0);
        if (!XtIsManaged(RightClip(mw)))
            XtManageChild(RightClip(mw));
    }
    if (mw->matrix.fixed_rows <= 0)
    {
               if (XtIsManaged(TopClip(mw)))
            XtUnmanageChild(TopClip(mw));
    }
    else
    {
        XtConfigureWidget(TopClip(mw),
                          FIXED_COLUMN_LABEL_OFFSET(mw), ROW_LABEL_OFFSET(mw),
                          cell_width, FIXED_ROW_HEIGHT(mw), 0);
        if (!XtIsManaged(TopClip(mw)))
            XtManageChild(TopClip(mw));
    }
    if (mw->matrix.trailing_fixed_rows <= 0)
    {
               if (XtIsManaged(BottomClip(mw)))
            XtUnmanageChild(BottomClip(mw));
    }
    else
    {
        XtConfigureWidget(BottomClip(mw),
                          FIXED_COLUMN_LABEL_OFFSET(mw),
                          TRAILING_FIXED_ROW_LABEL_OFFSET(mw),
                          cell_width, TRAILING_FIXED_ROW_HEIGHT(mw), 0);
        if (!XtIsManaged(BottomClip(mw)))
            XtManageChild(BottomClip(mw));
    }

    /*
     * The text field needs to be moved manually as we don't have
     * the convenience of a clip widget to do it for us.
     */
    if (mw->matrix.current_column >= TRAILING_HORIZ_ORIGIN(mw) &&
        mw->matrix.current_parent == (Widget)mw &&
        XtIsManaged(TextChild(mw)))
        XtMoveWidget(TextChild(mw), RightClip(mw)->core.x +
                     mw->matrix.cell_shadow_thickness, TextChild(mw)->core.y);
    /*
     * Save the non-truncated height.  We need this so we can draw
     * the shadow correctly.
     */
    mw->matrix.cell_visible_height = cell_height;

    /*
     * Set the clip_mask in our clipping GCs.  This function relies on
     * the Clip widget being the correct size (above).
     */
    if (XtIsRealized((Widget)mw))
        xbaeSetClipMask(mw, CLIP_NONE);

    if (mw->matrix.resize_callback != NULL)
    {
        XbaeMatrixResizeCallbackStruct call_data;

        call_data.reason = XbaeResizeReason;
        call_data.event = (XEvent *)NULL;
        call_data.row = mw->matrix.rows;
        call_data.column = mw->matrix.columns;
        call_data.width  = mw->core.width;
        call_data.height = mw->core.height;
        XtCallCallbackList ((Widget)mw, mw->matrix.resize_callback,
                            (XtPointer) &call_data);
    }
}

/*
 * This is the modifyVerifyCallback we added to textField. We need to
 * call Matrix's modifyVerifyCallback list with the textField info
 * and the row/col that is changing.
 */
/* ARGSUSED */
void
xbaeModifyVerifyCB(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget)client;
    XmTextVerifyCallbackStruct *verify = (XmTextVerifyCallbackStruct *)call;
    XbaeMatrixModifyVerifyCallbackStruct call_data;

    if (!mw->matrix.modify_verify_callback)
        return;

    call_data.reason = XbaeModifyVerifyReason;
    call_data.row = mw->matrix.current_row;
    call_data.column = mw->matrix.current_column;
    call_data.event = (XEvent *)NULL;
    call_data.verify = verify;

    call_data.prev_text = ((XmTextRec*)w)->text.value;

    XtCallCallbackList((Widget) mw, mw->matrix.modify_verify_callback,
                       (XtPointer) & call_data);
}


/*
 * Matrix edit_cell method
 */
void
xbaeEditCell(mw, event, row, column, params, nparams)
XbaeMatrixWidget mw;
XEvent *event;
int row;
int column;
String *params;
Cardinal nparams;
{
    XbaeMatrixEnterCellCallbackStruct call_data;
    Window newWin, oldWin;
    int x, y;
    Pixel fgcolor, bgcolor;
    Boolean alt;
    String string;
    Widget oldWidget, newWidget;
#if CELL_WIDGETS
    Widget userWidget;
#endif

    if (row >= mw->matrix.rows || row < 0 ||
        column >= mw->matrix.columns || column < 0)
    {
        /*
         * If we have zero rows or columns, there are no cells
         * available on which to place the text field so just return
         */
        if (mw->matrix.rows == 0 || mw->matrix.columns == 0)
            return;

        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "editCell", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row or column out of bounds for EditCell.",
            NULL, 0);
        return;
    }

    /*
     * Attempt to commit the edit in the current cell. Return if we fail.
     */
    if (!DoCommitEdit(mw, event))
        return;

    /*
     * Scroll the cell onto the screen
     */
    xbaeMakeCellVisible(mw, row, column);

    /*
     * Fixed cells may not be editable.
     */
    if (IS_FIXED(mw, row, column) && !mw->matrix.traverse_fixed)
        return;

    /* get the window of the new cell position */
    newWin = xbaeGetCellWindow(mw, &newWidget, row, column);

    /*
     * If we have an enterCellCallback, call it to see if the cell is
     * editable.
     */
    call_data.map = True;
    call_data.doit = True;
    call_data.position = -1;
    call_data.pattern = NULL;
    
    XtVaGetValues(TextChild(mw),
                  XmNoverwriteMode, &call_data.overwrite_mode,
                  XmNautoFill, &call_data.auto_fill,
                  XmNconvertCase, &call_data.convert_case,
                  NULL);

    call_data.select_text = False;

    if (mw->matrix.enter_cell_callback)
    {
        call_data.reason = XbaeEnterCellReason;
        call_data.event = event;
        call_data.row = row;
        call_data.column = column;
        call_data.map = True;
        call_data.num_params = nparams;
        call_data.params = params;

        XtCallCallbackList((Widget) mw, mw->matrix.enter_cell_callback,
                           (XtPointer) & call_data);
    }

    /* Get the window of the current cell so we can see if we need to move. */
    oldWin = xbaeGetCellWindow(mw, &oldWidget, mw->matrix.current_row,
                               mw->matrix.current_column);
    mw->matrix.current_row = row;
    mw->matrix.current_column = column;

    /*
     * Unmap the textField to avoid flashing.
     */
    if (XtIsManaged(TextChild(mw)) && XtIsRealized(TextChild(mw)))
        XtUnmapWidget(TextChild(mw));

    /*
     * Convert the row/column to an xy position and move the textField
     * to this position. (the xy position will be relative to the Clip
     * widget if a non-fixed cells is being edited, relative to Matrix if
     * a totally fixed cell is being edited (one of the corners), or one of
     * the other clips otherwise.
     */
    xbaeRowColToXY(mw, row, column, &x, &y);

#if CELL_WIDGETS
    userWidget = mw->matrix.cell_widgets[row][column];
    
    if (!userWidget)
    {
#endif
        /*
         * We actually don't check for traverse_fixed here even though
         * it looks like it might be needed. The reason is that we may
         * need to reparent back onto the clip in case we were on the
         * fixed area and then traverse_fixed has been set to False
         * via SetValues. Doing this on the next traversal is probably
         * preferable to magically warping the textField off the
         * matrix on to the clip when traverseFixedCells changes. It
         * also allows the user to finish editing the existing cell,
         * but won't allow continued traversal on the fixed area. -CG
         */
        
        /*
         * The old check (oldWin != newWin) as criteria to reparent
         * wasn't quite correct in the case of editable fixed columns;
         * In this case the first time the cell was edited 'oldWin'
         * and 'newWin' where both the left clip widget (which was correct)
         * but the 'current_parent' was still the initial parent set in the
         * 'Reslize' function (I think the clip widget).
         * The result was that the text field was moved relative to wrong
         * window and therefore appearing at a complete different position;
         * I check now as additional criteria if the 'current_parent' widget
         * is the same as 'newWidget'.
         * It should fix the my problem without breaking anything else.
         * The check (oldWin && newWin) for apps which call on startup
         * editCell() without a realized widget tree. Without this check
         * X errors would be the result.
         *
         * donato petrino, 1997/11/
         */
        if ((oldWin != newWin ||
             mw->matrix.current_parent != newWidget) &&
            (oldWin && newWin))
        {
            XReparentWindow(XtDisplay(mw), XtWindow(TextChild(mw)), newWin,
                            x + mw->matrix.cell_shadow_thickness,
                            y + mw->matrix.cell_shadow_thickness);
            mw->matrix.current_parent = newWidget;
            /*
             * Widget still needs moving, because all we have done
             * above is redraw it's window. The widget itself doesn't
             * know where it is and must be repositioned relative to
             * it's (possibly new) window.
             */
        }
        XtMoveWidget(TextChild(mw), 
                     x + mw->matrix.cell_shadow_thickness,
                     y + mw->matrix.cell_shadow_thickness);
#if CELL_WIDGETS
    }
    else
    {
        /*
         * A user defined widget does not take into account the
         * cell_highlight_thickness, so we must do it!
         */         
        XtMoveWidget(userWidget,
                     x + mw->matrix.cell_shadow_thickness +
                     mw->matrix.cell_highlight_thickness,
                     y + mw->matrix.cell_shadow_thickness +
                     mw->matrix.cell_highlight_thickness);
        /* Force editing to be disabled */
        call_data.doit = False;
    }
#endif
    /*
     * Compute the foreground and background of the text widget
     */
    alt = mw->matrix.alt_row_count ?
        (row / mw->matrix.alt_row_count) % 2 : False;

    if (mw->matrix.colors)
        fgcolor = mw->matrix.colors[row][column];
    else
        fgcolor = mw->manager.foreground;

    if (mw->matrix.text_background != mw->core.background_pixel)
        bgcolor = mw->matrix.text_background;
    else if (mw->matrix.cell_background && 
             mw->matrix.cell_background[row][column] != mw->core.background_pixel)
        bgcolor = mw->matrix.cell_background[row][column];
    else
    {
        if (alt)
            bgcolor = mw->matrix.odd_row_background;
        else
            bgcolor = mw->matrix.even_row_background;
    }
    /*
     * If we're doing a drawCell, go ask the app what to put there.
     */
    if (mw->matrix.draw_cell_callback)
    {
        Pixmap pixmap;
        Pixmap mask;
        int width, height, depth;
        Pixel orig_bg, orig_fg;
        
        orig_bg = bgcolor;
        orig_fg = fgcolor;
        if (xbaeGetDrawCellValue(
            mw, row, column, &string, &pixmap, &mask, &width, &height,
            &bgcolor, &fgcolor, &depth) == XbaePixmap)
        {
            /*
             * If we're showing a pixmap, we don't want the TextField.
             */
            return;
        }
        /*
         * If we reverse selected then we would have reversed things we
         * shouldn't have. We can detect this by checking bgcolor against
         * orig_fg and fgcolor against orig_bg and setting the colors back
         * to their non-selected values (as with an ordinary selected when
         * it is being edited). -cg 23/7/99
         */
        if (mw->matrix.reverse_select && mw->matrix.selected_cells &&
            mw->matrix.selected_cells[row][column])
        {
            int new_fg = fgcolor;
            int new_bg = bgcolor;

            /* callback changed bg */
            if (orig_fg != fgcolor)
                new_bg = fgcolor;
            else        /* reset it */
                new_bg = orig_bg;
            /* callback changed fg */
            if (orig_bg != bgcolor)
                new_fg = bgcolor;
            else        /* reset it */
                new_fg = orig_fg;
            bgcolor = new_bg;
            fgcolor = new_fg;
        }
    }
    else
        string = mw->matrix.cells ? mw->matrix.cells[row][column] : "";


    /*
     * Setup the textField for the new cell. If the modifyVerify CB
     * rejects the new value, then it is the applications fault for
     * loading the cell with a bad value to begin with.
     */
#if CELL_WIDGETS
    if (!mw->matrix.cell_widgets[row][column])
    {
#endif
        /*
         * Remove the modify verify callback when the text field is set.
         * It thinks we are modifying the value but Motif thinks that
         * it knows best and we know better!
         */
        XtRemoveCallback(TextChild(mw), XmNmodifyVerifyCallback,
                         xbaeModifyVerifyCB, (XtPointer)mw);
        
        XtVaSetValues(TextChild(mw),
                      XmNwidth, COLUMN_WIDTH(mw, column) -
                      mw->matrix.cell_shadow_thickness * 2,
                      XmNheight, (ROW_HEIGHT(mw) -
                                  mw->matrix.cell_shadow_thickness * 2),
                      XmNmaxLength, (mw->matrix.column_max_lengths ?
                                     mw->matrix.column_max_lengths[column] :
                                     (int) mw->matrix.column_widths[column]),
                      XmNeditable, call_data.doit,
                      XmNcursorPositionVisible, call_data.doit,
                      XmNbackground, bgcolor,
                      XmNforeground, fgcolor,
                      XmNpattern, call_data.pattern,
                      XmNoverwriteMode, call_data.overwrite_mode,
                      XmNautoFill, call_data.auto_fill,
                      XmNconvertCase, call_data.convert_case,
                      NULL);

        XtVaSetValues(TextChild(mw), XmNvalue, string, NULL);

        XtAddCallback(TextChild(mw), XmNmodifyVerifyCallback,
                      xbaeModifyVerifyCB, (XtPointer)mw);
#if CELL_WIDGETS
    }
    else
        XtVaSetValues(userWidget,
                      XmNwidth, COLUMN_WIDTH(mw, column) -
                      mw->matrix.cell_shadow_thickness * 2,
                      XmNheight, ROW_HEIGHT(mw)
                      - mw->matrix.cell_shadow_thickness * 2,
                      XmNbackground, bgcolor,
                      XmNforeground, fgcolor,
                      NULL);
#endif

    /*
     * No need to do anything else if the text field is not going to
     * be mapped
     */
    if (!call_data.map)
        return;

    /*
     * Manage and map the textField
     */
#if CELL_WIDGETS
    if (userWidget)
    {
        XtUnmanageChild(TextChild(mw));
        XtManageChild(userWidget);
    }
    else
#endif
        XtManageChild(TextChild(mw));

    if (XtIsRealized(TextChild(mw))
#if CELL_WIDGETS
        && !userWidget
#endif
        )
        XtMapWidget(TextChild(mw));
#if CELL_WIDGETS
    else if (XtIsRealized(userWidget) && userWidget)
        XtMapWidget(userWidget);

    if (call_data.doit && !userWidget) 
#endif
        /*
         * Set the insert position of the cursor
         */
        if (call_data.doit)
        {
            int position = call_data.position;
            int length = strlen(string);

            if (event && (event->type == ButtonPress ||
                          event->type == ButtonRelease ) &&
                position < 0 && mw->matrix.calc_cursor_position)
            {
                /*
                 * The location of the pointer click needs to be calculated
                 * so the cursor can be positioned.  If position is >= 0,
                 * it has been set in the enterCellCallback and must
                 * be honoured elsewhere.
                 */
                CellType cell;
                int r, c;
                
                /*
                 * The event must have occurred in a legal position
                 * otherwise control wouldn't have made it here
                 */
                (void)xbaeEventToXY(mw, event, &x, &y, &cell);
                (void)xbaeXYToRowCol(mw, &x, &y, &r, &c, cell);
                x -= mw->matrix.cell_shadow_thickness;
                y = ROW_HEIGHT(mw) / 2; /* XXX should be real y! */
                position = XmTextXYToPos(TextChild(mw), x, y);
            }
            
            if (call_data.select_text)
                XmTextSetSelection(TextChild(mw), 0, length,
                                        CurrentTime);            
            if (position < 0)
                XmTextSetInsertionPosition(TextChild(mw), length);
            else
                XmTextSetInsertionPosition(
                    TextChild(mw), position > length ? length : position);
        }
        
}

/*
 * Matrix select_cell method
 */
void
xbaeSelectCell(mw, row, column)
XbaeMatrixWidget mw;
int row, column;
{
    Boolean visible;

    if (row >= mw->matrix.rows || row < 0 ||
        column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "selectCell", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row or column out of bounds for SelectCell.",
            NULL, 0);
        return;
    }

    /* If no cells have been selected yet, allocate memory here */
    if (!mw->matrix.selected_cells)
        xbaeCopySelectedCells(mw);

    /*
     * Scroll the cell onto the screen
     */
    visible = xbaeIsCellVisible(mw, row, column);

    if (mw->matrix.scroll_select && !visible)
        xbaeMakeCellVisible(mw, row, column);

    /*
     * If the cell is not already selected, select it and redraw it
     */
    if (!mw->matrix.selected_cells[row][column])
    {
        mw->matrix.selected_cells[row][column] = True;
        mw->matrix.num_selected_cells++;
        if (mw->matrix.scroll_select || visible)
        {
            if (row >= TRAILING_VERT_ORIGIN(mw))
                xbaeSetClipMask(mw, CLIP_TRAILING_FIXED_ROWS);
            
            xbaeDrawCell(mw, row, column);

            if (row >= TRAILING_VERT_ORIGIN(mw))
                xbaeSetClipMask(mw, CLIP_NONE);
        }
    }
}

/*
 * Matrix select_row method
 */
void
xbaeSelectRow(mw, row)
XbaeMatrixWidget mw;
int row;
{
    int j, lc, rc;
    Boolean fixed = False, trailing_fixed = False;
    Boolean visible;
    unsigned int clip_reason = CLIP_NONE, save_clip = CLIP_NONE;

    if (row >= mw->matrix.rows || row < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "selectRow", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row out of bounds for SelectRow.",
            NULL, 0);
        return;
    }

    /* If no cells have been selected yet, allocate memory here */
    if (!mw->matrix.selected_cells)
        xbaeCopySelectedCells(mw);

    visible = xbaeIsRowVisible(mw, row);
    /*
     * Scroll the row onto the screen
     */
    if (mw->matrix.scroll_select)
        xbaeMakeRowVisible(mw, row);

    /*
     * If the row is not visible, there's no need to redraw - but, we do
     * need to update the selected cell resource
     */
    if(!mw->matrix.scroll_select && !visible)
    {
        for (j = 0; j < mw->matrix.columns; j++)
            if (!mw->matrix.selected_cells[row][j])
            {
                mw->matrix.num_selected_cells++;
                mw->matrix.selected_cells[row][j] = True;
            }
        return;
    }
    
    /*
     * Establish any necessary clipping for redrawing the cells
     */
    save_clip = mw->matrix.current_clip;
    if (row >= TRAILING_VERT_ORIGIN(mw))
        clip_reason = CLIP_TRAILING_FIXED_ROWS;
    if (CLIP_NONE != clip_reason)
        xbaeSetClipMask(mw, CLIP_TRAILING_FIXED_ROWS);

    /*
     * For each cell in the row, if the cell is not already selected,
     * select it and redraw it
     */
    xbaeGetVisibleColumns(mw, &lc, &rc);
    for (j = 0; j < mw->matrix.columns; j++)
    {
        if (!mw->matrix.selected_cells[row][j])
        {
            mw->matrix.selected_cells[row][j] = True;
            mw->matrix.num_selected_cells++;
            if ((j >= lc && j <= rc) ||
                (j < (int)mw->matrix.fixed_columns) ||
                (j >= TRAILING_HORIZ_ORIGIN(mw)))
            {
                if ((! fixed) && (j < (int)mw->matrix.fixed_columns))
                {
                    fixed = True;
                    xbaeSetClipMask(mw, clip_reason | CLIP_FIXED_COLUMNS);
                }
                else if (fixed && (j >= (int)mw->matrix.fixed_columns) &&
                         (j < TRAILING_HORIZ_ORIGIN(mw)))
                {
                    fixed = False;
                    xbaeSetClipMask(mw, clip_reason);
                }
                else if ((! trailing_fixed) && (j >= TRAILING_HORIZ_ORIGIN(mw)))
                {
                    trailing_fixed = True;
                    xbaeSetClipMask(mw, clip_reason |
                                    CLIP_TRAILING_FIXED_COLUMNS);
                }
                
                xbaeClearCell(mw, row, j);
                xbaeDrawCell(mw, row, j);
            }
        }
    }
    if (save_clip != mw->matrix.current_clip)
        xbaeSetClipMask(mw, CLIP_NONE);
}

/*
 * Matrix select_column method
 */
void
xbaeSelectColumn(mw, column)
XbaeMatrixWidget mw;
int column;
{
    int i, tr, br;
    Boolean once = False;

    unsigned int clip_reason = CLIP_NONE;

    if (column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "selectColumn", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Column out of bounds for SelectColumn.",
            NULL, 0);
        return;
    }

    /* If no cells have been selected yet, allocate memory here */
    if (!mw->matrix.selected_cells)
        xbaeCopySelectedCells(mw);

    /*
     * Scroll the column onto the screen
     */
    if (mw->matrix.scroll_select)
        xbaeMakeColumnVisible(mw, column);

    /*
     * No need to redraw unless the column is visible
     */
    if (!mw->matrix.scroll_select && !xbaeIsColumnVisible(mw, column))
    {
        for (i = 0; i < mw->matrix.rows; i++)
            if (!mw->matrix.selected_cells[i][column])
            {
                mw->matrix.num_selected_cells++;
                mw->matrix.selected_cells[i][column] = True;
            }
        return;
    }
    
    /*
     * Establish any necessary clipping for redrawing the cells
     */
    if (column < (int)mw->matrix.fixed_columns)
        clip_reason = CLIP_FIXED_COLUMNS;
    else if (column >= TRAILING_HORIZ_ORIGIN(mw))
        clip_reason = CLIP_TRAILING_FIXED_COLUMNS;
    if (CLIP_NONE != clip_reason)
        xbaeSetClipMask(mw, clip_reason | CLIP_VISIBLE_HEIGHT);
        
    /*
     * For each cell in the column, if the cell is not already selected,
     * select it and redraw it
     */
    xbaeGetVisibleRows(mw, &tr, &br);
    for (i = 0; i < mw->matrix.rows; i++)
    {
        if (!mw->matrix.selected_cells[i][column])
        {
            mw->matrix.selected_cells[i][column] = True;
            mw->matrix.num_selected_cells++;
            if ((i >= tr && i <= br) ||
                (i < (int)mw->matrix.fixed_rows) ||
                (i >= TRAILING_VERT_ORIGIN(mw)))
            {
                if ((! once) && (i >= TRAILING_VERT_ORIGIN(mw)))
                {
                    once = True;
                    xbaeSetClipMask(mw, clip_reason | CLIP_TRAILING_FIXED_ROWS);
                }
                
                xbaeClearCell(mw, i, column);
                xbaeDrawCell(mw, i, column);                
            }
        }
    }
    
    if (once || (CLIP_NONE != clip_reason))
        xbaeSetClipMask(mw, CLIP_NONE);
}

/*
 * Matrix deselect_all method
 */
void
xbaeDeselectAll(mw)
XbaeMatrixWidget mw;
{
    int i, j;
    int tr, br, lc, rc;
    register Boolean do_row, once = False;

    mw->matrix.num_selected_cells = 0;
    /* If selected_cells is NULL, no cells have been selected yet  */
    if (!mw->matrix.selected_cells)
        return;
    
    xbaeGetVisibleCells(mw, &tr, &br, &lc, &rc);
    
    for (i = 0; i < mw->matrix.rows; i++)
    {
        do_row = False;
        if ((! once) && (i >= TRAILING_VERT_ORIGIN(mw)))
        {
            once = True;
            xbaeSetClipMask(mw, CLIP_TRAILING_FIXED_ROWS);
        }
        for (j = 0; j < mw->matrix.columns; j++)
        {
            if (mw->matrix.selected_cells[i][j])
            {
                mw->matrix.selected_cells[i][j] = False;
                if (((i < (int)mw->matrix.fixed_rows) ||
                     (i >= TRAILING_VERT_ORIGIN(mw)) ||
                     (i >= tr && i <= br)) &&
                    ((j < (int)mw->matrix.fixed_columns) ||
                     (j >= TRAILING_HORIZ_ORIGIN(mw)) ||
                     (j >= lc && j <= rc)))
                {
                    xbaeClearCell(mw, i, j);
                    xbaeDrawCell(mw, i, j);

                    do_row = True;
                }
            }
        }
    }
    if (once)
        xbaeSetClipMask(mw, CLIP_NONE);
}

/*
 * Matrix select_all method
 */
void
xbaeSelectAll(mw)
XbaeMatrixWidget mw;
{
    int i, j;
    int tr, br, lc, rc;
    register Boolean do_row, once = False;

    xbaeGetVisibleCells(mw, &tr, &br, &lc, &rc);
    
    if (!mw->matrix.selected_cells)
        xbaeCopySelectedCells(mw);

    for (i = 0; i < mw->matrix.rows; i++)
    {
        do_row = False;
        if ((! once) && (i >= TRAILING_VERT_ORIGIN(mw)))
        {
            once = True;
            xbaeSetClipMask(mw, CLIP_TRAILING_FIXED_ROWS);
        }
        for (j = 0; j < mw->matrix.columns; j++)
        {
            if (!mw->matrix.selected_cells[i][j])
            {
                mw->matrix.num_selected_cells++;
                mw->matrix.selected_cells[i][j] = True;
                if (((i < (int)mw->matrix.fixed_rows) ||
                     (i >= TRAILING_VERT_ORIGIN(mw)) ||
                     (i >= tr && i <= br)) &&
                    ((j < (int)mw->matrix.fixed_columns) ||
                     (j >= TRAILING_HORIZ_ORIGIN(mw)) ||
                     (j >= lc && j <= rc)))
                {
                    xbaeClearCell(mw, i, j);
                    xbaeDrawCell(mw, i, j);

                    do_row = True;
                }
            }
        }

    }
    if (once)
        xbaeSetClipMask(mw, CLIP_NONE);
}

/*
 * Matrix deselect_cell method
 */
void
xbaeDeselectCell(mw, row, column)
XbaeMatrixWidget mw;
int row;
int column;
{
    if (row >= mw->matrix.rows || row < 0 ||
        column > mw->matrix.columns - 1 || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "deselectCell", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row or column out of bounds for DeselectCell.",
            NULL, 0);
        return;
    }

    if (!mw->matrix.selected_cells)
        return;

    if (mw->matrix.selected_cells[row][column])
    {
        mw->matrix.num_selected_cells--;
        mw->matrix.selected_cells[row][column] = False;
        if (xbaeIsCellVisible(mw, row, column))
        {
            if (row >= TRAILING_VERT_ORIGIN(mw))
                xbaeSetClipMask(mw, CLIP_TRAILING_FIXED_ROWS);

            xbaeClearCell(mw, row, column);
            xbaeDrawCell(mw, row, column);

            if (row >= TRAILING_VERT_ORIGIN(mw))
                xbaeSetClipMask(mw, CLIP_NONE);

        }
    }
}

/*
 * Matrix deselect_row method
 */
void
xbaeDeselectRow(mw, row)
XbaeMatrixWidget mw;
int row;
{
    int j, lc, rc;
    Boolean fixed = False, trailing_fixed = False;
    unsigned int clip_reason = CLIP_NONE, save_clip;

    if (row >= mw->matrix.rows || row < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "deselectRow", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row parameter out of bounds for DeselectRow.",
            NULL, 0);
        return;
    }

    if (!mw->matrix.selected_cells)
        return;

    /*
     * Establish any necessary clipping for redrawing the cells
     */
    save_clip = mw->matrix.current_clip;
    if (row >= TRAILING_VERT_ORIGIN(mw))
        clip_reason = CLIP_TRAILING_FIXED_ROWS;
    if (CLIP_NONE != clip_reason)
        xbaeSetClipMask(mw, CLIP_TRAILING_FIXED_ROWS);

    /*
     * For each cell in the row, if the cell is selected,
     * deselect it and redraw it
     */
    xbaeGetVisibleColumns(mw, &lc, &rc);
    for (j = 0; j < mw->matrix.columns; j++)
    {
        if (mw->matrix.selected_cells[row][j])
        {
            mw->matrix.num_selected_cells--;
            mw->matrix.selected_cells[row][j] = False;
            if ((j >= lc && j <= rc) ||
                (j < (int)mw->matrix.fixed_columns) ||
                (j >= TRAILING_HORIZ_ORIGIN(mw)))
            {
                if ((! fixed) && (j < (int)mw->matrix.fixed_columns))
                {
                    fixed = True;
                    xbaeSetClipMask(mw, clip_reason | CLIP_FIXED_COLUMNS);
                }
                else if (fixed && (j >= (int)mw->matrix.fixed_columns) &&
                         (j < TRAILING_HORIZ_ORIGIN(mw)))
                {
                    fixed = False;
                    xbaeSetClipMask(mw, clip_reason);
                }
                else if ((! trailing_fixed) && (j >= TRAILING_HORIZ_ORIGIN(mw)))
                {
                    trailing_fixed = True;
                    xbaeSetClipMask(mw, clip_reason | CLIP_TRAILING_FIXED_COLUMNS);
                }
                
                xbaeClearCell(mw, row, j);
                xbaeDrawCell(mw, row, j);
            }
        }
    }

    if (save_clip != mw->matrix.current_clip)
        xbaeSetClipMask(mw, CLIP_NONE);
}

/*
 * Matrix deselect_column method
 */
void
xbaeDeselectColumn(mw, column)
XbaeMatrixWidget mw;
int column;
{
    int i, tr, br;
    Boolean once = False;
    unsigned int clip_reason = CLIP_NONE;

    if (column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "deselectColumn", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Column parameter out of bounds for DeselectColumn.",
            NULL, 0);
        return;
    }

    if (!mw->matrix.selected_cells)
        return;

    /*
     * Establish any necessary clipping for redrawing the cells
     */
    if (column < (int)mw->matrix.fixed_columns)
        clip_reason = CLIP_FIXED_COLUMNS;
    else if (column >= TRAILING_HORIZ_ORIGIN(mw))
        clip_reason = CLIP_TRAILING_FIXED_COLUMNS;
    if (CLIP_NONE != clip_reason)
        xbaeSetClipMask(mw, clip_reason | CLIP_VISIBLE_HEIGHT);

    /*
     * For each cell in the column, if the cell is selected,
     * deselect it and redraw it
     */
    xbaeGetVisibleRows(mw, &tr, &br);
    for (i = 0; i < mw->matrix.rows; i++)
    {
        if (mw->matrix.selected_cells[i][column])
        {
            mw->matrix.num_selected_cells--;
            mw->matrix.selected_cells[i][column] = False;
            if ((i >= tr && i <= br) ||
                (i < (int)mw->matrix.fixed_rows) ||
                (i >= TRAILING_VERT_ORIGIN(mw)))
            {
                if ((! once) && (i >= TRAILING_VERT_ORIGIN(mw)))
                {
                    once = True;
                    xbaeSetClipMask(mw, clip_reason | CLIP_TRAILING_FIXED_ROWS);
                }

                xbaeClearCell(mw, i, column);
                xbaeDrawCell(mw, i, column);

            }
        }
    }
    if (once || (CLIP_NONE != clip_reason))
        xbaeSetClipMask(mw, CLIP_NONE);
}

/*
 * Matrix get_cell method
 */
String
xbaeGetCell(mw, row, column)
XbaeMatrixWidget mw;
int row, column;
{
    String value;
    
    if (row >= mw->matrix.rows || row < 0 ||
        column > mw->matrix.columns - 1 || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "getCell", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row or column out of bounds for GetCell.",
            NULL, 0);
        return (NULL);
    }
    if (mw->matrix.draw_cell_callback)
    {
        Pixel bgcolor, fgcolor;
        Pixmap pixmap, mask;
        int width, height, depth;
        
        if (xbaeGetDrawCellValue(mw, row, column, &value, &pixmap,
                                 &mask, &width, &height, &bgcolor, &fgcolor,
                                 &depth) == XbaePixmap)
            value = "";
    }
    else if (!mw->matrix.cells)
        return "";
    else
        value = mw->matrix.cells[row][column];

    return value;
}

/*
 * Matrix commit_edit method
 */
Boolean
#if NeedFunctionPrototypes
xbaeCommitEdit(XbaeMatrixWidget mw, XEvent *event, Boolean unmap)
#else
xbaeCommitEdit(mw, event, unmap)
XbaeMatrixWidget mw;
XEvent *event;
Boolean unmap;
#endif
{
    Boolean commit;

    if (!XtIsManaged(TextChild(mw)))
        return True;

    /*
     * Attempt to commit the edit
     */
    commit = DoCommitEdit(mw, event);

    /*
     * If the commit succeeded and we are supposed to unmap the textField,
     * then hide the textField and traverse out
     */
    if (commit && unmap)
    {
        XtUnmanageChild(TextChild(mw));
        XmProcessTraversal(TextChild(mw), XmTRAVERSE_RIGHT);
    }

    return commit;
}

/*
 * Matrix cancel_edit method
 */
void
#if NeedFunctionPrototypes
xbaeCancelEdit(XbaeMatrixWidget mw, Boolean unmap)
#else
xbaeCancelEdit(mw, unmap)
XbaeMatrixWidget mw;
Boolean unmap;
#endif
{
    if (!XtIsManaged(TextChild(mw)))
        return;

    /*
     * If unmap is set, hide the textField and traverse out.
     */
    if (unmap)
    {
        XtUnmanageChild(TextChild(mw));
        XmProcessTraversal(TextChild(mw), XmTRAVERSE_RIGHT);
    }

    /*
     * Don't unmap, just restore original contents
     */
    else if (!mw->matrix.draw_cell_callback)
    {
        XtVaSetValues(TextChild(mw),
                      XmNvalue, (mw->matrix.cells ?
                                 mw->matrix.cells[mw->matrix.current_row]
                                 [mw->matrix.current_column] : ""),
                      NULL);
    }
    else
    {
        /* Ask the application what should be in the cell */
        String string;
        Pixmap pixmap, mask;
        Pixel bg, fg;
        int width, height, depth;
        
        if (xbaeGetDrawCellValue(mw, mw->matrix.current_row,
                                 mw->matrix.current_column, &string,
                                 &pixmap, &mask, &width, &height,
                                 &bg, &fg, &depth) == XbaeString)

            XtVaSetValues(TextChild(mw), XmNvalue, string, NULL);
    }
}

/*
 * Matrix add_rows method
 */
void
xbaeAddRows(mw, position, rows, labels, colors, backgrounds, num_rows)
XbaeMatrixWidget mw;
int position;
String *rows;
String *labels;
Pixel *colors;
Pixel *backgrounds;
int num_rows;
{
    Boolean haveVSB, haveHSB;

    /*
     * Do some error checking.
     */
    if (num_rows <= 0)
        return;
    if (position < 0 || position > mw->matrix.rows)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "addRows", "badPosition", "XbaeMatrix",
            "XbaeMatrix: Position out of bounds in AddRows.",
            NULL, 0);
        return;
    }

    haveVSB = XtIsManaged(VertScrollChild(mw));
    haveHSB = XtIsManaged(HorizScrollChild(mw));
    /*
     * If we add rows, and there is no drawCellCallback, we must allocate
     * the cells array to prevent potential disaster
     */
    if (!mw->matrix.cells && !mw->matrix.draw_cell_callback)
        xbaeCopyCells(mw);
    
    /*
     * Add the new rows into the internal cells/labels data structure.
     */
    AddRowsToTable(mw, position, rows, labels, colors, backgrounds, num_rows);

    /*
     * Reconfig the VSB maximum.
     */
    XtVaSetValues(VertScrollChild(mw),
                  XmNmaximum, mw->matrix.rows ?
                  (mw->matrix.rows - (int) mw->matrix.fixed_rows -
                   (int) mw->matrix.trailing_fixed_rows) : 1,
                  NULL);

    /*
     * Relayout.
     */
    xbaeResize(mw);

    /*
     * Call our cancel_edit method since the rows shifted underneath us
     */
    (*((XbaeMatrixWidgetClass) XtClass(mw))->matrix_class.cancel_edit)
        (mw, True);

    if (!mw->matrix.disable_redisplay && XtIsRealized((Widget)mw))
    {
        Rectangle rect;
        int x, y;

        /*
         * Determine which part of the non clip region needs to be
         * redisplayed
         */
        if (position >= (int)mw->matrix.fixed_rows)
        {
            xbaeRowColToXY(mw, position, mw->matrix.fixed_columns, &x, &y);
            if (mw->matrix.scrollbar_placement == XmTOP_LEFT ||
                mw->matrix.scrollbar_placement == XmTOP_RIGHT)
                y += HORIZ_SB_SPACE(mw);
            y += ROW_HEIGHT(mw) * mw->matrix.fixed_rows +
                COLUMN_LABEL_HEIGHT(mw);
        }
        else
        {
            if (mw->matrix.scrollbar_placement == XmTOP_LEFT ||
                mw->matrix.scrollbar_placement == XmTOP_RIGHT)
                y = HORIZ_SB_SPACE(mw);
            else
                y = 0;
            y += ROW_HEIGHT(mw) * position + COLUMN_LABEL_HEIGHT(mw);
        }
        SETRECT(rect, 0, y, mw->core.width, mw->core.height);
        xbaeRedrawLabelsAndFixed(mw, &rect);
        XClearArea(XtDisplay(mw), XtWindow(ClipChild(mw)),
                   rect.x1, rect.y1,
                   rect.x2 - rect.x1, rect.y2 - rect.y1, True);
        /*
         * If the scrollbars have just been mapped and there are
         * labels then the labels shift around. The labels need
         * to be redrawn
         */
        if (!haveVSB && XtIsManaged(VertScrollChild(mw)) &&
            mw->matrix.column_labels)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       0, HORIZ_SB_OFFSET(mw), 0,
                       COLUMN_LABEL_HEIGHT(mw), True);
        if ((!haveHSB && XtIsManaged(VertScrollChild(mw)) &&
             mw->matrix.row_labels) ||
            ((mw->matrix.scrollbar_placement == XmTOP_LEFT ||
              mw->matrix.scrollbar_placement == XmBOTTOM_LEFT) &&
             !haveVSB && XtIsManaged(VertScrollChild(mw))))
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       VERT_SB_OFFSET(mw), 0, ROW_LABEL_WIDTH(mw), 0, True);
    }
}

/*
 * Matrix delete_rows method
 */
void
xbaeDeleteRows(mw, position, num_rows)
XbaeMatrixWidget mw;
int position;
int num_rows;
{
    int        max, value;
    Boolean haveVSB;
    Boolean haveHSB;

    /*
     * Do some error checking.
     */
    if (num_rows <= 0)
        return;
    if (position < 0 || position + num_rows > mw->matrix.rows)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "deleteRows", "badPosition", "XbaeMatrix",
            "XbaeMatrix: Position out of bounds in DeleteRows.",
            NULL, 0);
        return;
    }
    if (num_rows > (mw->matrix.rows - (int)mw->matrix.fixed_rows -
                    (int)mw->matrix.trailing_fixed_rows))
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "deleteRows", "tooMany", "XbaeMatrix",
            "XbaeMatrix: Attempting to delete too many rows in DeleteRows.",
            NULL, 0);
        return;
    }

    haveVSB = XtIsManaged(VertScrollChild(mw));
    haveHSB = XtIsManaged(HorizScrollChild(mw));

    /*
     * Delete the new rows from the internal cells/labels data structure.
     */
    DeleteRowsFromTable(mw, position, num_rows);

    /*
     * Reconfig the VSB maximum. Reset the sliderSize to avoid warnings.
     * Also check the scrollbar value to see that it's not out of range.
     */
    XtVaGetValues(VertScrollChild(mw),
                      XmNvalue, &value,
                      NULL);

    max = mw->matrix.rows ?
        (mw->matrix.rows - (int) mw->matrix.fixed_rows -
         (int) mw->matrix.trailing_fixed_rows) : 1;

    XtVaSetValues(VertScrollChild(mw),
                  XmNvalue, (value >= max) ? max - 1 : value,
                  XmNmaximum, mw->matrix.rows - (int) mw->matrix.fixed_rows -
                  (int) mw->matrix.trailing_fixed_rows ?
                  (mw->matrix.rows - (int) mw->matrix.fixed_rows -
                   (int) mw->matrix.trailing_fixed_rows) : 1,
                  XmNsliderSize, 1,
                  NULL);

    /*
     * Relayout.
     */
    xbaeResize(mw);

    /*
     * Call our cancel_edit method since the rows shifted underneath us
     */
    (*((XbaeMatrixWidgetClass) XtClass(mw))->matrix_class.cancel_edit)
        (mw, True);

    if (!mw->matrix.disable_redisplay && XtIsRealized((Widget)mw))
    {
        Rectangle rect;
        int y;

        /*
         * Determine which part of the non clip region needs to be
         * redisplayed
         */
#if 0
        dest_y = (position - mw->matrix.fixed_rows) * ROW_HEIGHT(mw);
        src_y = dest_y + num_rows * ROW_HEIGHT(mw);
        if (XtIsManaged(LeftClip(mw)))
        {
            if (src_y < LeftClip(mw)->core.height)
            {
                /* Copy what we can up to replace the deleted rows */
                XCopyArea(XtDisplay(mw), XtWindow(LeftClip(mw)),
                          XtWindow(LeftClip(mw)), mw->matrix.draw_gc,
                          0, src_y, LeftClip(mw)->core.width,
                          LeftClip(mw)->core.height - src_y,
                          0, dest_y);
                /* And clear the new area that needs to be redrawn */
                XClearArea(XtDisplay(mw), XtWindow(LeftClip(mw)),
                           0, LeftClip(mw)->core.height - src_y,
                           LeftClip(mw)->core.width,
                           LeftClip(mw)->core.height - src_y, True);
            }
        }
        if (XtIsManaged(RightClip(mw)))
        {
            if (src_y < RightClip(mw)->core.height)
            {
                XCopyArea(XtDisplay(mw), XtWindow(RightClip(mw)),
                          XtWindow(RightClip(mw)), mw->matrix.draw_gc,
                          0, src_y, RightClip(mw)->core.width,
                          RightClip(mw)->core.height - src_y,
                          0, dest_y);
                XClearArea(XtDisplay(mw), XtWindow(RightClip(mw)),
                           0, RightClip(mw)->core.height - src_y,
                           RightClip(mw)->core.width,
                           RightClip(mw)->core.height - src_y, True);
            }
        }
#endif
        y = ROW_LABEL_OFFSET(mw) + position * ROW_HEIGHT(mw);
        SETRECT(rect, 0, y, mw->core.width, mw->core.height - y);
        /* xxx could this use an XCopyArea() instead */
        XClearArea(XtDisplay(mw), XtWindow(mw),
                   0, y, mw->core.width, mw->core.height - y, True);

        xbaeRedrawLabelsAndFixed(mw, &rect);

        y = (position - mw->matrix.fixed_rows) * ROW_HEIGHT(mw);

        XClearArea(XtDisplay(mw), XtWindow(ClipChild(mw)),
                   0, y, rect.x2, mw->core.height - y, True);
        /*
         * If the scrollbars have just been unmapped and there are
         * labels then the labels shift around. The labels need
         * to be redrawn
         */
        if (haveVSB && !XtIsManaged(VertScrollChild(mw)) &&
            mw->matrix.column_labels)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       0, HORIZ_SB_OFFSET(mw), 0,
                       COLUMN_LABEL_HEIGHT(mw), True);
        if (haveHSB && !XtIsManaged(VertScrollChild(mw)) &&
            mw->matrix.row_labels)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       VERT_SB_OFFSET(mw), 0, ROW_LABEL_WIDTH(mw), 0, True);
#if 0
        /*
         * If we are deleting rows and there are different cell backgrounds
         * or foregrounds and the deleted row was on the visible clip, then
         * the colours can get confused.
         */
        if (mw->matrix.colors || mw->matrix.even_row_background !=
            mw->core.background_pixel || mw->matrix.odd_row_background !=
            mw->core.background_pixel)
            XbaeClipRedraw(ClipChild(mw));
#endif
    }
}

/*
 * Matrix add_columns method.
 */
void
xbaeAddColumns(mw, position, columns, labels, widths, max_lengths,
               alignments, label_alignments, colors, backgrounds, num_columns)
XbaeMatrixWidget mw;
int position;
String *columns;
String *labels;
short *widths;
int *max_lengths;
unsigned char *alignments;
unsigned char *label_alignments;
Pixel *colors;
Pixel *backgrounds;
int num_columns;
{
    Boolean haveVSB;
    Boolean haveHSB;

    /*
     * Do some error checking.
     */
    if (num_columns <= 0)
        return;
    if (position < 0 || position > mw->matrix.columns)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "addColumns", "badPosition", "XbaeMatrix",
            "XbaeMatrix: Position out of bounds in AddColumns.",
            NULL, 0);
        return;
    }
    if (!widths)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "addColumns", "noWidths", "XbaeMatrix",
            "XbaeMatrix: Must specify column widths in AddColumns.",
            NULL, 0);
        return;
    }

    /*
     * If we add columns, and there is no drawCellCallback, we must allocate
     * the cells array to prevent potential disaster
     */
    if (!mw->matrix.cells && !mw->matrix.draw_cell_callback)
        xbaeCopyCells(mw);
    
    haveVSB = XtIsManaged(VertScrollChild(mw));
    haveHSB = XtIsManaged(HorizScrollChild(mw));

    /*
     * Add the new rows into the internal cells/labels data structure.
     */
    AddColumnsToTable(mw, position, columns, labels, widths, max_lengths,
                      alignments, label_alignments, colors, backgrounds,
                      num_columns);

    /*
     * Reconfig the HSB maximum.
     */
    XtVaSetValues(HorizScrollChild(mw),
                  XmNmaximum, NON_FIXED_TOTAL_WIDTH(mw) ?
                  NON_FIXED_TOTAL_WIDTH(mw) : 1,
                  NULL);
    
    /*
     * Relayout.
     */
    xbaeResize(mw);
    
    /*
     * Call our cancel_edit method since the columns shifted underneath us
     */
    (*((XbaeMatrixWidgetClass) XtClass(mw))->matrix_class.cancel_edit)
        (mw, True);
    
    if (!mw->matrix.disable_redisplay && XtIsRealized((Widget)mw))
    {
        Rectangle rect;
        int x, y;

        /*
         * Determine which part of the non clip region needs to be
         * redisplayed
         */
        if (position >= (int)mw->matrix.fixed_columns)
        {
            xbaeRowColToXY(mw, mw->matrix.fixed_columns, position, &x, &y);
            if (mw->matrix.scrollbar_placement == XmTOP_LEFT ||
                mw->matrix.scrollbar_placement == XmBOTTOM_LEFT)
                x += VERT_SB_SPACE(mw);
            x += COLUMN_POSITION(mw, mw->matrix.fixed_columns) +
                ROW_LABEL_WIDTH(mw);
        }
        else
        {
            if (mw->matrix.scrollbar_placement == XmTOP_LEFT ||
                mw->matrix.scrollbar_placement == XmBOTTOM_LEFT)
                x = VERT_SB_SPACE(mw);
            else
                x = 0;
            x += COLUMN_POSITION(mw, position) + ROW_LABEL_WIDTH(mw);
        }
        SETRECT(rect, x, 0, mw->core.width, mw->core.height);
        xbaeRedrawLabelsAndFixed(mw, &rect);
        XClearArea(XtDisplay(mw), XtWindow(ClipChild(mw)),
                   rect.x1, rect.y1,
                   rect.x2 - rect.x1, rect.y2 - rect.y1, True);
        /*
         * If the scrollbars have just been mapped and there are
         * labels then the labels shift around. The labels need
         * to be redrawn
         */
        if ((!haveVSB && XtIsManaged(VertScrollChild(mw)) &&
             mw->matrix.column_labels) ||
            ((mw->matrix.scrollbar_placement == XmTOP_LEFT ||
              mw->matrix.scrollbar_placement == XmTOP_RIGHT) &&
             !haveHSB && XtIsManaged(HorizScrollChild(mw))))
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       0, HORIZ_SB_OFFSET(mw), 0,
                       COLUMN_LABEL_HEIGHT(mw), True);
        if ((!haveHSB && XtIsManaged(VertScrollChild(mw)) &&
             mw->matrix.row_labels) ||
            ((mw->matrix.scrollbar_placement == XmTOP_LEFT ||
              mw->matrix.scrollbar_placement == XmTOP_RIGHT)))
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       VERT_SB_OFFSET(mw), 0, ROW_LABEL_WIDTH(mw), 0, True);
    }
}

/*
 * Matrix delete_columns method
 */
void
xbaeDeleteColumns(mw, position, num_columns)
XbaeMatrixWidget mw;
int position;
int num_columns;
{
    int maxlines;
    Boolean haveVSB;
    Boolean haveHSB;

    /*
     * Do some error checking.
     */
    if (num_columns <= 0)
        return;
    if (position < 0 || position + num_columns > mw->matrix.columns)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "deleteColumns", "badPosition", "XbaeMatrix",
            "XbaeMatrix: Position out of bounds in DeleteColumns.",
            NULL, 0);
        return;
    }
    if (num_columns > (mw->matrix.columns - (int)mw->matrix.fixed_columns -
                       (int)mw->matrix.trailing_fixed_columns))
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "deleteColumns", "tooMany", "XbaeMatrix",
            "XbaeMatrix: Attempting to delete too many columns in DeleteColumns.",
            NULL, 0);
        return;
    }

    haveVSB = XtIsManaged(VertScrollChild(mw));
    haveHSB = XtIsManaged(HorizScrollChild(mw));

    maxlines = mw->matrix.column_label_maxlines;

    /*
     * Delete the new columns from the internal cells/labels data structure.
     */
    DeleteColumnsFromTable(mw, position, num_columns);

    /*
     * Reconfig the HSB maximum. Reset the sliderSize to avoid warnings.
     */
    XtVaSetValues(HorizScrollChild(mw),
                  XmNvalue, 0,    /* value to 0 to stop sb from whinging */
                  XmNmaximum, NON_FIXED_TOTAL_WIDTH(mw) ?
                  NON_FIXED_TOTAL_WIDTH(mw) : 1,
                  XmNsliderSize, 1,
                  NULL);

    /*
     * Relayout.
     */
    xbaeResize(mw);

    /*
     * Call our cancel_edit method since the columns shifted underneath us
     */
    (*((XbaeMatrixWidgetClass) XtClass(mw))->matrix_class.cancel_edit)
        (mw, True);

    if (!mw->matrix.disable_redisplay && XtIsRealized((Widget)mw))
    {
        Rectangle rect;
        int x, y;

        if (maxlines != mw->matrix.column_label_maxlines)
        {
            /*
             * If a column with a high label gets deleted, then the matrix
             * gets pulled up the screen and leaves dangly bits underneath.
             * Clear the whole area to ensure correct display and forget the
             * rest of the calculation
             */
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       0, 0, 0 /*Full Width*/, 0 /*Full Height*/,
                       True);
            return;
        }
        /*
         * Determine which part of the non clip region needs to be
         * redisplayed
         */
        if (position >= (int)mw->matrix.fixed_columns)
        {
            xbaeRowColToXY(mw, mw->matrix.fixed_columns, position, &x, &y);
            if (mw->matrix.scrollbar_placement == XmTOP_LEFT ||
                mw->matrix.scrollbar_placement == XmBOTTOM_LEFT)
                x += VERT_SB_SPACE(mw);
            x += COLUMN_POSITION(mw, mw->matrix.fixed_columns) +
                ROW_LABEL_WIDTH(mw);
        }
        else
        {
            if (mw->matrix.scrollbar_placement == XmTOP_LEFT ||
                mw->matrix.scrollbar_placement == XmBOTTOM_LEFT)
                x = VERT_SB_SPACE(mw);
            else
                x = 0;
            x += COLUMN_POSITION(mw, position) + ROW_LABEL_WIDTH(mw);
        }
        SETRECT(rect, x, 0, mw->core.width, mw->core.height);
        XClearArea(XtDisplay(mw), XtWindow(mw),
                   VISIBLE_WIDTH(mw) + FIXED_COLUMN_WIDTH(mw) +
                   TRAILING_FIXED_COLUMN_WIDTH(mw),
                   0,
                   mw->core.width, mw->core.height, True);
        xbaeRedrawLabelsAndFixed(mw, &rect);
        XClearArea(XtDisplay(mw), XtWindow(ClipChild(mw)),
                   rect.x1, rect.y1,
                   rect.x2 - rect.x1, rect.y2 - rect.y1, True);
        /*
         * If the scrollbars have just been unmapped and there are
         * labels then the labels shift around. The labels need
         * to be redrawn
         */
        if (haveVSB && !XtIsManaged(VertScrollChild(mw)) &&
            mw->matrix.column_labels)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       0, HORIZ_SB_OFFSET(mw), 0,
                       COLUMN_LABEL_HEIGHT(mw), True);
        if (haveHSB && !XtIsManaged(VertScrollChild(mw)) &&
            mw->matrix.row_labels)
            XClearArea(XtDisplay(mw), XtWindow(mw),
                       VERT_SB_OFFSET(mw), 0, ROW_LABEL_WIDTH(mw), 0, True);
    }
}

/*
 * Matrix set_row_colors method
 */
void
#if NeedFunctionPrototypes
xbaeSetRowColors(XbaeMatrixWidget mw, int position, Pixel *colors,
                 int num_colors, Boolean bg)
#else
xbaeSetRowColors(mw, position, colors, num_colors, bg)
XbaeMatrixWidget mw;
int position;
Pixel *colors;
int num_colors;
Boolean bg;
#endif
{
    Rectangle rect;
    int i, j;
    Pixel **set;
    Pixel pixel;
    
    /*
     * Do some error checking.
     */
    if (num_colors <= 0)
        return;
    if (position < 0 || position + num_colors > mw->matrix.rows)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "setRowColors", "badPosition", "XbaeMatrix",
            "XbaeMatrix: Position out of bounds or too many colors in SetRowColors.",
            NULL, 0);
        return;
    }

    /*
     * If we don't have any colors yet, malloc them, and initialize
     * unused entries to the appropriate color
     */
    if ((!bg && !mw->matrix.colors) ||
        (bg && !mw->matrix.cell_background))
    {
        if (!bg)
        {
            xbaeCreateColors(mw);
            set = &mw->matrix.colors[0];
            pixel = mw->manager.foreground;
            for (i = 0; i < position; i++)
                for (j = 0; j < mw->matrix.columns; j++)
                    set[i][j] = pixel;
            for (i = position + num_colors; i < mw->matrix.rows; i++)
                for (j = 0; j < mw->matrix.columns; j++)
                    set[i][j] = pixel;
        }
        else
            xbaeCopyBackgrounds(mw);
    }
    
    if (!bg)
        set = &mw->matrix.colors[0];
    else
        set = &mw->matrix.cell_background[0];

    /*
     * Set each row to the appropriate color
     */
    for (i = 0; i < num_colors; i++)
        for (j = 0; j < mw->matrix.columns; j++)
            set[i + position][j] = colors[i];
        
    if (XtIsRealized((Widget)mw))
    {
        /*
         * Redraw all the visible non-fixed cells. We don't need to clear first
         * since only the color changed.
         */
        SETRECT(rect,
                0, 0,
                ClipChild(mw)->core.width - 1, ClipChild(mw)->core.height - 1);
        xbaeRedrawCells(mw, &rect);

        /*
         * Redraw all the visible fixed cells (but not the labels).
         * We don't need to clear first since only the color changed.
         */
        SETRECT(rect,
                ROW_LABEL_WIDTH(mw), COLUMN_LABEL_HEIGHT(mw),
                mw->core.width - 1, mw->core.height - 1);
        xbaeRedrawLabelsAndFixed(mw, &rect);
        
    }
    if (position <= mw->matrix.current_row && 
        position + num_colors > mw->matrix.current_row &&
        XtIsRealized(TextChild(mw)))
    {
        if (bg)
            XtVaSetValues(TextChild(mw), XmNbackground, 
                          mw->matrix.cell_background[mw->matrix.current_row]
                          [mw->matrix.current_column],
                          NULL);
        else
            XtVaSetValues(TextChild(mw), XmNforeground, 
                          mw->matrix.colors[mw->matrix.current_row]
                          [mw->matrix.current_column],
                          NULL);
    }
}

/*
 * Matrix set_column_colors method
 */
void
#if NeedFunctionPrototypes
xbaeSetColumnColors(XbaeMatrixWidget mw, int position, Pixel *colors,
                    int num_colors, Boolean bg)
#else
xbaeSetColumnColors(mw, position, colors, num_colors, bg)
XbaeMatrixWidget mw;
int position;
Pixel *colors;
int num_colors;
Boolean bg;
#endif
{
    Rectangle rect;
    int i, j;
    Pixel **set;
    Pixel pixel;
    
    /*
     * Do some error checking.
     */
    if (num_colors <= 0)
        return;
    if (position < 0 || position + num_colors > mw->matrix.columns)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "setColumnColors", "badPosition", "XbaeMatrix",
            "XbaeMatrix: Position out of bounds or too many colors in SetColumnColors.",
            NULL, 0);
        return;
    }

    /*
     * If we don't have any colors yet, malloc them, and initialize
     * unused entries to foreground
     */
    if ((!bg && !mw->matrix.colors) ||
        (bg && !mw->matrix.cell_background))
    {
        if (!bg)
        {
            xbaeCreateColors(mw);
            set = &mw->matrix.colors[0];
            pixel = mw->manager.foreground;
            for (i = 0; i < mw->matrix.rows; i++)
                for (j = 0; j < position; j++)
                    set[i][j] = pixel;
            for (i = 0; i < mw->matrix.rows; i++)
                for (j = position + num_colors; j < mw->matrix.columns; j++)
                    set[i][j] = pixel;
        }
        else
            xbaeCopyBackgrounds(mw);
    }

    if (!bg)
        set = &mw->matrix.colors[0];
    else
        set = &mw->matrix.cell_background[0];

    /*
     * Set each column to the appropriate color
     */
    for (i = 0; i < mw->matrix.rows; i++)
        for (j = 0; j < num_colors; j++)
            set[i][j + position] = colors[j];

    if (XtIsRealized((Widget)mw))
    {
        /*
         * Redraw all the visible non-fixed cells. We don't need to clear first
         * since only the color changed.
         */
        SETRECT(rect,
                0, 0,
                ClipChild(mw)->core.width - 1, ClipChild(mw)->core.height - 1);
        xbaeRedrawCells(mw, &rect);

        /*
         * Redraw all the visible fixed cells (but not the labels).
         * We don't need to clear first since only the color changed.
         */
        SETRECT(rect,
                ROW_LABEL_WIDTH(mw), COLUMN_LABEL_HEIGHT(mw),
                mw->core.width - 1, mw->core.height - 1);
        xbaeRedrawLabelsAndFixed(mw, &rect);
    }
    if (position <= mw->matrix.current_column && 
        position + num_colors > mw->matrix.current_column &&
        XtIsRealized(TextChild(mw)))
    {
        if (bg)
            XtVaSetValues(TextChild(mw), XmNbackground, 
                          mw->matrix.cell_background[mw->matrix.current_row]
                          [mw->matrix.current_column],
                          NULL);
        else
            XtVaSetValues(TextChild(mw), XmNforeground, 
                          mw->matrix.colors[mw->matrix.current_row]
                          [mw->matrix.current_column],
                          NULL);
    }
}

/*
 * Matrix set_cell_color method
 */
void
#if NeedFunctionPrototypes
xbaeSetCellColor(XbaeMatrixWidget mw, int row, int column, Pixel color, Boolean bg)
#else
xbaeSetCellColor(mw, row, column, color, bg)
XbaeMatrixWidget mw;
int row;
int column;
Pixel color;
Boolean bg;
#endif
{
    int i, j;
    Pixel **set;
    Pixel pixel;
    
    /*
     * Do some error checking.
     */
    if (row >= mw->matrix.rows || row < 0 ||
        column >= mw->matrix.columns || column < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) mw),
            "xbaeSetCellColor", "badIndex", "XbaeMatrix",
            "XbaeMatrix: Row or column out of bounds for xbaeSetCellColor.",
            NULL, 0);
        return;
    }

    /*
     * If we don't have any colors yet, malloc them and initialize them
     */
    if ((!bg && !mw->matrix.colors) ||
        (bg && !mw->matrix.cell_background))
    {
        if (!bg)
        {
            xbaeCreateColors(mw);
            set = &mw->matrix.colors[0];
            pixel = mw->manager.foreground;
            for (i = 0; i < mw->matrix.rows; i++)
                for (j = 0; j < mw->matrix.columns; j++)
                    set[i][j] = pixel;
        }
        else
            xbaeCopyBackgrounds(mw);
    }

    if (!bg)
        set = &mw->matrix.colors[0];
    else
        set = &mw->matrix.cell_background[0];

    /*
     * Set the cell's color
     */
    set[row][column] = color;

    if (XtIsRealized((Widget)mw))
    {
        /*
         * Redraw the cell if it is visible
         */
        if (xbaeIsCellVisible(mw, row, column))
            xbaeDrawCell(mw, row, column);
    }
    if (row == mw->matrix.current_row &&
        column == mw->matrix.current_column && XtIsRealized(TextChild(mw)))
    {
        if (bg)
            XtVaSetValues(TextChild(mw), XmNbackground, 
                          mw->matrix.cell_background[mw->matrix.current_row]
                          [mw->matrix.current_column],
                          NULL);
        else
            XtVaSetValues(TextChild(mw), XmNforeground, 
                          mw->matrix.colors[mw->matrix.current_row]
                          [mw->matrix.current_column],
                          NULL);
    }
}

