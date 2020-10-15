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
 * $Id: Draw.c,v 1.2 2003-10-10 19:49:47 joga Exp $
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
#include "Utils.h"
#include "Shadow.h"
#include "Draw.h"
#include <stdlib.h>

static void xbaeDrawCellString P((XbaeMatrixWidget, int, int, int, int,
                                  String, Pixel, Pixel));
#if CELL_WIDGETS
static void xbaeDrawCellWidget P((XbaeMatrixWidget, int, int, int, int,
                                  Widget, Pixel, Pixel));
#endif
static void xbaeDrawCellPixmap P((XbaeMatrixWidget, int, int, int, int,
                                  Pixmap, Pixmap, int, int, Pixel,
                                  Pixel, int));

/*
 * Draw a fixed or non-fixed cell. The coordinates are calculated relative
 * to the correct window and pixmap is copied to that window.
 */
static void
xbaeDrawCellPixmap(mw, row, column, x, y, pixmap, mask, width, height, bg,
                   fg, depth)
XbaeMatrixWidget mw;
int row;
int column;
int x;
int y;
Pixmap pixmap;
Pixmap mask;
int width;
int height;
Pixel bg;
Pixel fg;
int depth;
{
    int src_x = 0, src_y, dest_x, dest_y;
    int copy_width, copy_height;
    int cell_height = ROW_HEIGHT(mw);
    int cell_width = COLUMN_WIDTH(mw, column);
    Widget w;
    unsigned char alignment = mw->matrix.column_alignments ?
        mw->matrix.column_alignments[column] : XmALIGNMENT_BEGINNING;
    Display *display = XtDisplay(mw);
    GC gc;
    Window win = xbaeGetCellWindow(mw, &w, row, column);

    if (!win)
        return;

    /*
     * Convert the row/column to the coordinates relative to the correct
     * window
     */
    dest_x = x + TEXT_WIDTH_OFFSET(mw);

    gc = mw->matrix.pixmap_gc;

    XSetForeground(display, gc, bg);

#if XmVersion >= 1002
    /*
     * If we are only changing the highlighting of a cell, we don't need
     * to do anything other than draw (or undraw) the highlight
     */
    if (mw->matrix.highlighted_cells &&
        mw->matrix.highlight_location != HighlightNone)
    {
        xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, cell_width,
                              cell_height, mw->matrix.highlight_location);
        return;
    }
#endif

    XFillRectangle(display, win, gc, x, y,
                   COLUMN_WIDTH(mw, column), ROW_HEIGHT(mw));

    XSetForeground(display, gc, fg);
    XSetBackground(display, gc, bg);

    /*
     * Adjust the x and y drawing destinations as appropriate.  First the
     * y value....
     */
    dest_y = y;
    if (height > cell_height)
    {
        /* Adjust the starting location in the src image */
        src_y = (height - cell_height) / 2;
        copy_height = cell_height;
    }
    else
    {
        /* Adjust the destination point */
        src_y = 0;
        dest_y += ((cell_height - height) / 2);
        copy_height = height;
    }

    /*
     * Adjust the x value, paying attention to the columnAlignment
     */
    if (width > cell_width)
        copy_width = cell_width;
    else
        copy_width = width;
    
    switch (alignment)
    {
    case XmALIGNMENT_BEGINNING:
        src_x = 0;
        break;
    case XmALIGNMENT_CENTER:
        if (width > cell_width)
            src_x = (width - cell_width) / 2;
        else
        {
            src_x = 0;
            dest_x += ((cell_width - width) / 2);
        }
        break;
    case XmALIGNMENT_END:        
        if (width > cell_width)
            src_x = width - cell_width;
        else
        {
            src_x = 0;
            dest_x = x + COLUMN_WIDTH(mw, column) - TEXT_WIDTH_OFFSET(mw) -
                width;
        }
        break;
    }

    /*
     * Draw the pixmap.  Clip it, if necessary
     */
    if (pixmap)
    {
        if (depth > 1)                /* A pixmap using xpm */
        {
            if (mask)
            {
                XSetClipMask(display, gc, mask);

                XSetClipOrigin(display, gc, dest_x - src_x, dest_y - src_y);
            }
            XCopyArea(display, pixmap, win, gc, src_x, src_y, copy_width,
                      copy_height, dest_x, dest_y);
            if (mask)
                XSetClipMask(display, gc, None);
        }
        else                        /* A plain old bitmap */
            XCopyPlane(display, pixmap, win, gc, src_x, src_y, copy_width,
                       copy_height, dest_x, dest_y, 1L);
    }
    
    /*
     * If we need to fill the rest of the space, do so
     */
    if (IN_GRID_COLUMN_MODE(mw) && NEED_VERT_FILL(mw) &&
        (row == (mw->matrix.rows - 1)))
    {
        int ax, ay;
        int fill_width, fill_height;
        /*
         * Need to check the actual window we are drawing on to ensure
         * the correct visual
         */
        xbaeCalcVertFill(mw, win, x, y, row, column, &ax, &ay,
                         &fill_width, &fill_height);
        XFillRectangle(XtDisplay(mw), XtWindow(mw), gc,
                       ax, ay, fill_width, fill_height);
    }
    else if (IN_GRID_ROW_MODE(mw) && NEED_HORIZ_FILL(mw) &&
            (column == (mw->matrix.columns - 1)))
    {
        int ax, ay;
        int fill_width, fill_height;

        xbaeCalcHorizFill(mw, win, x, y, row, column, &ax, &ay,
                          &fill_width, &fill_height);
        XFillRectangle(XtDisplay(mw), XtWindow(mw), gc,
                       ax, ay, fill_width, fill_height);
    }

#if XmVersion >= 1002
    if (mw->matrix.highlighted_cells &&
        mw->matrix.highlighted_cells[row][column])
    {
        xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, cell_width,
                              cell_height, HIGHLIGHTING_SOMETHING);
    }
#endif
    xbaeDrawCellShadow(mw, win, row, column, x, y, cell_width,
                       cell_height, False, False, False);
}

/*
 * Draw a fixed or non-fixed cell. The coordinates are calculated relative
 * to the correct window and the cell is drawn in that window.
 */
static void
xbaeDrawCellString(XbaeMatrixWidget mw, int row, int column, int x, int y, String string, Pixel bg, Pixel fg)
{
    GC gc;
    Widget w;
    Window win = xbaeGetCellWindow(mw, &w, row, column);
    Dimension column_width = COLUMN_WIDTH(mw, column);
    Dimension row_height = ROW_HEIGHT(mw);
    Dimension width = column_width;
    Dimension height = row_height;
    Boolean selected = mw->matrix.selected_cells ?
        mw->matrix.selected_cells[row][column] : False;
    String str = string;
    
    if (!win)
        return;

#if 0
    /*
     * Probably not needed - time will tell!  If anybody gets a segv on
     * ignoring this code, be sure to let me know - AL 11/96
     *
     * Make sure y coordinate is valid 
     */
    if ((win == XtWindow(mw)) &&
        ((y > (CLIP_VERT_VISIBLE_SPACE(mw) + ROW_LABEL_OFFSET(mw) - 1)) ||
         (y < ROW_LABEL_OFFSET(mw))))
        return;
#endif
    gc = mw->matrix.draw_gc;
    XSetForeground(XtDisplay(mw), gc, bg);

    /*
     * If we are only changing the highlighting of a cell, we don't need
     * to do anything other than draw (or undraw) the highlight
     */
    if (mw->matrix.highlighted_cells &&
        mw->matrix.highlight_location != HighlightNone)
    {
        xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, width, height,
                              mw->matrix.highlight_location);
        return;
    }

    /*
     * Fill the cell's background if it can be done
     * without duplicating work below
     */
    if ((XtWindow(mw) != win) ||
        (!(IN_GRID_COLUMN_MODE(mw) && NEED_VERT_FILL(mw) &&
           ((mw->matrix.rows - 1) == row)) &&
         !(IN_GRID_ROW_MODE(mw) && NEED_HORIZ_FILL(mw) &&
           ((mw->matrix.columns - 1) == column))))
        XFillRectangle(XtDisplay(mw), win, gc, x, y,
                       column_width, row_height);

    /*
     * If we need to fill the rest of the space, do so
     */
    if (IN_GRID_COLUMN_MODE(mw) && NEED_VERT_FILL(mw) &&
        ((mw->matrix.rows - 1) == row))
    {
        int ax, ay;
        int fill_width, fill_height;
        /*
         * Need to check the actual window we are drawing on to ensure
         * the correct visual
         */
        xbaeCalcVertFill(mw, win, x, y, row, column, &ax, &ay,
                         &fill_width, &fill_height);
        XFillRectangle(XtDisplay(mw), XtWindow(mw), gc,
                       ax, ay, fill_width, fill_height);
    }
    else if (IN_GRID_ROW_MODE(mw) && NEED_HORIZ_FILL(mw) &&
             (column == (mw->matrix.columns - 1)))
    {
        int ax, ay;
        int fill_width, fill_height;

        xbaeCalcHorizFill(mw, win, x, y, row, column, &ax, &ay,
                          &fill_width, &fill_height);
        XFillRectangle(XtDisplay(mw), XtWindow(mw), gc,
                       ax, ay, fill_width, fill_height);
    }

    /*
     * Draw the string in the cell.
     */
    xbaeDrawString(mw, win, gc, str, strlen(str),
                   x + TEXT_X_OFFSET(mw), y + TEXT_Y_OFFSET(mw),
                   mw->matrix.column_widths[column],
                   mw->matrix.column_alignments ?
                   mw->matrix.column_alignments[column] :
                   XmALIGNMENT_BEGINNING, selected,
                   False, False, False, fg);

#if XmVersion >= 1002
    if (mw->matrix.highlighted_cells &&
        mw->matrix.highlighted_cells[row][column])
    {
        xbaeDrawCellHighlight(mw, win, gc, row, column, x, y, width, height,
                              HIGHLIGHTING_SOMETHING);
    }
#endif

    xbaeDrawCellShadow(mw, win, row, column, x, y, COLUMN_WIDTH(mw, column),
                       ROW_HEIGHT(mw), False, False, False);
}
#if CELL_WIDGETS
/*
 * Draw a user defined widget in the cell
 */
static void
xbaeDrawCellWidget(mw, row, column, x, y, widget, bg, fg)
XbaeMatrixWidget mw;
int row, column;
int x, y;
Widget widget;
Pixel bg, fg;
{
    GC gc;
    Widget w;
    Window win = xbaeGetCellWindow(mw, &w, row, column);

    if (!win)
        return;

    gc = mw->matrix.draw_gc;
    XSetForeground(XtDisplay(mw), gc, bg);
    XFillRectangle(XtDisplay(mw), win, gc, x, y,
                   COLUMN_WIDTH(mw, column), ROW_HEIGHT(mw));

    /*
     * Draw the widget in the cell.
     */
    XtMoveWidget(widget,
                 x + mw->matrix.cell_shadow_thickness +
                 mw->matrix.cell_highlight_thickness,
                 y + mw->matrix.cell_shadow_thickness +
                 mw->matrix.cell_highlight_thickness);

    xbaeDrawCellShadow(mw, win, row, column, x, y, COLUMN_WIDTH(mw, column),
                       ROW_HEIGHT(mw), False, clipped, False);
}
#endif

/*
 * Width in pixels of a character in a given font
 */
#define charWidth(fs,c) ((fs)->per_char ? \
                         (fs)->per_char[((c) < (fs)->min_char_or_byte2 ? \
                                         (fs)->default_char : \
                                         (c) - \
                                         (fs)->min_char_or_byte2)].width : \
                         (fs)->min_bounds.width)


/*
 * Draw a string with specified attributes. We want to avoid having to
 * use a GC clip_mask, so we clip by characters. This complicates the code.
 */
void
#if NeedFunctionPrototypes
xbaeDrawString(XbaeMatrixWidget mw, Window win, GC gc, String string,
               int length, int x, int y, int maxlen, unsigned char alignment,
               Boolean highlight, Boolean bold, Boolean rowLabel,
               Boolean colLabel, Pixel color)
#else
xbaeDrawString(mw, win, gc, string, length, x, y, maxlen, alignment,
               highlight, bold, rowLabel, colLabel, color)
XbaeMatrixWidget mw;
Window win;
GC gc;
String string;
int length;
int x;
int y;
int maxlen;
unsigned char alignment;
Boolean highlight;
Boolean bold;
Boolean rowLabel;
Boolean colLabel;
Pixel color;
#endif
{
    int start, width, maxwidth;
    XFontStruct        *font_struct;
    XFontSet        font_set;
    Boolean choppedStart = False;
    Boolean choppedEnd = False;
    XRectangle *ink_array = NULL;
    XRectangle *logical_array = NULL;
    int num_chars;
    XRectangle overall_logical;

    if (rowLabel || colLabel)
    {
        font_struct = mw->matrix.label_font_struct;
        font_set = mw->matrix.label_font_set;
    }
    else
    {
        font_struct = mw->matrix.font_struct;
        font_set = mw->matrix.font_set;
    }
    /*
     * Initialize starting character in string
     */
    start = 0;

    if (!rowLabel)
        maxwidth = maxlen * FONT_WIDTH(mw);
    else 
        maxwidth = maxlen * LABEL_WIDTH(mw);

    if (font_set)
    {
        ink_array = (XRectangle*)XtMalloc(length * sizeof(XRectangle));
        logical_array = (XRectangle*)XtMalloc(length * sizeof(XRectangle));

        XmbTextPerCharExtents(font_set, string, length,
            ink_array, logical_array, length, &num_chars,
            NULL, &overall_logical);

        /*
         * If the width of the string is greater than the width of this cell,
         * we need to clip. We don't want to use the server to clip because
         * it is slow, so we truncate characters if we exceed a cells pixel
         * width.
         */
        if (overall_logical.width > maxwidth)
        {
            switch (alignment)
            {

            case XmALIGNMENT_CENTER:
            {
                int startx;
                int endx;
                int i;
                int end;

                /*
                 * If we are going to draw arrows at both ends, allow for them.
                 */
                if (mw->matrix.show_arrows)
                {
                    maxwidth -= 2 * mw->matrix.font_width;
                    choppedStart = True;
                    choppedEnd = True;
                }

                /*
                 * Find limits of cell relative to the origin of the string.
                 */
                startx = overall_logical.x + overall_logical.width / 2 -
                    maxwidth / 2;
                endx = startx + maxwidth - 1;

                /*
                 * Find the first character which fits into the cell.
                 */
                for (i = 0; i < num_chars && logical_array[i].x < startx; ++i)
                {
                    int cl = mblen(string + start, length);
                    start += cl;
                    length -= cl;
                }

                /*
                 * Find the last character which fits into the cell.
                 * At this point length represents the number of bytes
                 * between the end of the cell and the end of the full
                 * string. Note that the scan continues from above.
                 */
                for (end = start; i < num_chars && (logical_array[i].x +
                                                    logical_array[i].width) <
                         endx; ++i)
                {
                    int cl = mblen(string + end, length);
                    end += cl;
                    length -= cl;
                }

                /*
                 * Now reset length so that it represents the number of bytes
                 * in the string.
                 */
                length = end - start;

                break;
            }

            case XmALIGNMENT_END:
            {
                int startx;
                int i;

                /*
                 * We are going to an draw arrow at the end, allow for it.
                 */
                if (mw->matrix.show_arrows)
                {
                    maxwidth -= mw->matrix.font_width;
                    choppedEnd = True;
                }

                /*
                 * Find limits of cell relative to the origin of the string.
                 */
                startx = overall_logical.x + overall_logical.width - maxwidth;

                /*
                 * Find the first character which fits into the cell.
                 */
                for (i = 0; i < num_chars && logical_array[i].x < startx; ++i)
                {
                    int cl = mblen(string + start, length);
                    start += cl;
                    length -= cl;
                }

                break;
            }

            case XmALIGNMENT_BEGINNING:
            default:
            {
                int endx;
                int i;
                int end;

                /*
                 * We are going to an draw arrow at the start, allow for it.
                 */
                if (mw->matrix.show_arrows)
                {
                    maxwidth -= mw->matrix.font_width;
                    choppedStart = True;
                }

                /*
                 * Find limits of cell relative to the origin of the string.
                 */
                endx = overall_logical.x + maxwidth - 1;

                /*
                 * Find the last character which fits into the cell.
                 * At this point length represents the number of bytes
                 * between the end of the cell and the end of the full
                 * string.
                 */
                for (i = 0, end = start;
                     i < num_chars && (logical_array[i].x +
                                       logical_array[i].width) < endx; ++i)
                {
                    int cl = mblen(string + end, length);
                    end += cl;
                    length -= cl;
                    choppedEnd = True;
                }

                /*
                 * Now reset length so that it represents the number of bytes
                 * in the string.
                 */
                length = end - start;

                break;
            }
            }

            /*
             * Having truncated string recalculate extents to find origin
             */
            XmbTextPerCharExtents(font_set, string, length,
                ink_array, logical_array, length, &num_chars,
                NULL, &overall_logical);
        }
        /*
         * We fit inside our cell, so just compute the x of the start of
         * our string
         */
        else
        {
            switch (alignment)
            {

            case XmALIGNMENT_CENTER:
                x += maxwidth / 2 - overall_logical.width / 2;
                break;

            case XmALIGNMENT_END:
                x += maxwidth - overall_logical.width;
                break;

            case XmALIGNMENT_BEGINNING:
            default:
                /*
                 * Leave x alone
                 */
                break;
            }
        }

        /*
         * Don't worry, XSetForeground is smart about avoiding unnecessary
         * protocol requests.
         */
        XSetForeground(XtDisplay(mw), gc, color);

        if (mw->matrix.show_arrows && choppedStart)
        {
            XPoint points[ 3 ];
            points[ 0 ].x = points[ 1 ].x = x + mw->matrix.font_width;
            points[ 0 ].y = y + mw->matrix.font_y;
            points[ 1 ].y = y + mw->matrix.font_y + mw->matrix.font_height;
            points[ 2 ].x = x;
            points[ 2 ].y = y + mw->matrix.font_y + mw->matrix.font_height / 2;

            XFillPolygon(XtDisplay(mw), win, gc, points, 3,
                         Convex, CoordModeOrigin);

            /* Offset the start point so as to not draw on the triangle */
            x += FONT_WIDTH(mw);
        }

        if (mw->matrix.show_arrows && choppedEnd)
        {
            XPoint points[ 3 ];
            points[ 0 ].x = points[ 1 ].x = x + overall_logical.width;
            points[ 0 ].y = y + mw->matrix.font_y;
            points[ 1 ].y = y + mw->matrix.font_y + mw->matrix.font_height;
            points[ 2 ].x = x + overall_logical.width + mw->matrix.font_width;
            points[ 2 ].y = y + mw->matrix.font_y + mw->matrix.font_height / 2;

            XFillPolygon(XtDisplay(mw), win, gc, points, 3,
                         Convex, CoordModeOrigin);
        }

        /*
         * Adjust x for origin of string.
         */
        x -= overall_logical.x;

        /*
         * Now draw the string at x starting at char 'start' and of
         * length 'length'
         */
        XmbDrawString(XtDisplay(mw), win, font_set, gc, x, y, &string[start],
                      length);

        /*
         * If bold is on, draw the string again offset by 1 pixel (overstrike)
         */
        if (bold)
            XmbDrawString(XtDisplay(mw), win, font_set, gc, x - 1, y,
                          &string[start], length);
        if (ink_array)
            XtFree((char*)ink_array);
        if (logical_array)
            XtFree((char*)logical_array);
    }
    else
    {
        width = XTextWidth(font_struct, string, length);

        /*
         * If the width of the string is greater than the width of this cell,
         * we need to clip. We don't want to use the server to clip because
         * it is slow, so we truncate characters if we exceed a cells pixel
         * width.
         */
        if (width > maxwidth)
        {
            switch (alignment)
            {

            case XmALIGNMENT_CENTER:
            {
                int startx = x;
                int endx = x + maxwidth - 1;
                int newendx;

                /*
                 * Figure out our x for the centered string.  Then loop
                 * and chop characters off the front until we are within
                 * the cell.
                 *
                 * Adjust x, the starting character and the length of the
                 * string for each char.
                 */
                x += maxwidth / 2 - width / 2;
                while (x < startx)
                {
                    int cw = charWidth(font_struct,
                                       (unsigned char)string[start]);

                    x += cw;
                    width -= cw;
                    length--;
                    start++;
                    choppedStart = True;
                }

                /*
                 * Now figure out the end x of the string.  Then loop and chop
                 * characters off the end until we are within the cell.
                 */
                newendx = x + width - 1;
                while (newendx > endx && *(string + start))
                {
                    int cw = charWidth(font_struct,
                                       (unsigned char)string[start]);

                    newendx -= cw;
                    width -= cw;
                    length--;
                    choppedEnd = True;
                }

                break;
            }

            case XmALIGNMENT_END:
            {

                /*
                 * Figure out our x for the right justified string.
                 * Then loop and chop characters off the front until we fit.
                 * Adjust x for each char lopped off. Also adjust the starting
                 * character and length of the string for each char.
                 */
                x += maxwidth - width;
                while (width > maxwidth)
                {
                    int cw = charWidth(font_struct,
                                       (unsigned char)string[start]);

                    width -= cw;
                    x += cw;
                    length--;
                    start++;
                    choppedStart = True;
                }
                break;
            }

            case XmALIGNMENT_BEGINNING:
            default:
                /*
                 * Leave x alone, but chop characters off the end until we fit
                 */
                while (width > maxwidth)
                {
                    width -= charWidth(font_struct,
                                       (unsigned char)string[length - 1]);
                    length--;
                    choppedEnd = True;
                }
                break;
            }
        }

        /*
         * We fit inside our cell, so just compute the x of the start of
         * our string
         */
        else
        {
            switch (alignment)
            {

            case XmALIGNMENT_CENTER:
                x += maxwidth / 2 - width / 2;
                break;

            case XmALIGNMENT_END:
                x += maxwidth - width;
                break;

            case XmALIGNMENT_BEGINNING:
            default:
                /*
                 * Leave x alone
                 */
                break;
            }
        }
        
        /*
         * Don't worry, XSetForeground is smart about avoiding unnecessary
         * protocol requests.
         */
        XSetForeground(XtDisplay(mw), gc, color);

        if (mw->matrix.show_arrows && choppedEnd)
        {
            XPoint points[3];
            points[0].x = points[1].x = x + width - mw->matrix.font_width;
            points[0].y = y + mw->matrix.font_y;
            points[1].y = y + mw->matrix.font_y + mw->matrix.font_height;
            points[2].x = x + width;
            points[2].y = y + mw->matrix.font_y + mw->matrix.font_height / 2;

            XFillPolygon(XtDisplay(mw), win, gc, points, 3,
                         Convex, CoordModeOrigin);

            /* Reduce the length to allow for our foreign character */
            length--;
        }
        if (mw->matrix.show_arrows && choppedStart)
        {
            XPoint points[3];
            points[0].x = points[1].x = x + mw->matrix.font_width;
            points[0].y = y + mw->matrix.font_y; 
            points[1].y = y + mw->matrix.font_y + mw->matrix.font_height;
            points[2].x = x;
            points[2].y = y + mw->matrix.font_y + mw->matrix.font_height / 2;

            XFillPolygon(XtDisplay(mw), win, gc, points, 3,
                         Convex, CoordModeOrigin);

            /* Offset the start point so as to not draw on the triangle */
            x += mw->matrix.font_width;
            start++;
            length--;
        }

        /*
         * Now draw the string at x starting at char 'start' and of length
         * 'length'
         */
#ifdef NEED_WCHAR
        if (TWO_BYTE_FONT(mw))
            XDrawString16(XtDisplay(mw), win, gc, x, y, &string[start],
                          length);
        else
#endif
            XDrawString(XtDisplay(mw), win, gc, x, y, &string[start], length);

        /*
         * If bold is on, draw the string again offset by 1 pixel (overstrike)
         */
        if (bold)
#ifdef NEED_WCHAR
            if (TWO_BYTE_FONT(mw))
                XDrawString16(XtDisplay(mw), win, gc, x - 1, y,
                              &string[start], length);
            else
#endif
                XDrawString(XtDisplay(mw), win, gc, x - 1, y,
                            &string[start], length);
    }
}

void
xbaeComputeCellColors(mw, row, column, fg, bg)
XbaeMatrixWidget mw;
int row, column;
Pixel *fg, *bg;
{
    Boolean alt = mw->matrix.alt_row_count ?
        (row / mw->matrix.alt_row_count) % 2 : False;

    /*
     * Compute the background and foreground colours of the cell
     */
    if (mw->matrix.selected_cells && mw->matrix.selected_cells[row][column]) {
        if (mw->matrix.reverse_select) {
            if (mw->matrix.colors)
                *bg = mw->matrix.colors[row][column];
            else 
                *bg = mw->manager.foreground;
        } 
        else 
            *bg = mw->matrix.selected_background;
    }
    else if (mw->matrix.cell_background &&
             mw->matrix.cell_background[row][column] !=
             mw->core.background_pixel) {
        *bg = mw->matrix.cell_background[row][column];
    }    
    else {
        if (alt)
            *bg = mw->matrix.odd_row_background;
        else
            *bg = mw->matrix.even_row_background;
    }

    if (mw->matrix.selected_cells && mw->matrix.selected_cells[row][column]) {
        if (mw->matrix.reverse_select) {
            if (mw->matrix.cell_background)
                *fg = mw->matrix.cell_background[row][column];
            else
                *fg = mw->core.background_pixel;
        }
        else
            *fg = mw->matrix.selected_foreground;
    }
    else if (mw->matrix.colors)
        *fg = mw->matrix.colors[row][column];
    else
        *fg = mw->manager.foreground;    
}

void
xbaeDrawCell(mw, row, column)
XbaeMatrixWidget mw;
int row, column;
{
    Pixel bg, fg;
    String string;
    int x, y;
    
    if (mw->matrix.disable_redisplay || mw->matrix.rows == 0 ||
        mw->matrix.columns == 0)
        return;

    /*
     * Convert the row/column to the coordinates relative to the correct
     * window
     */
    xbaeRowColToXY(mw, row, column, &x, &y);

    xbaeComputeCellColors(mw, row, column, &fg, &bg);

#if CELL_WIDGETS
    if (mw->matrix.cell_widgets[row][column])
        xbaeDrawCellWidget(mw, row, column, x, y,
                           mw->matrix.cell_widgets[row][column], bg, fg);
    else
#endif

        if (!mw->matrix.draw_cell_callback)
        {
            if (row < mw->matrix.rows && column < mw->matrix.columns)
            {
                if (mw->matrix.cells && mw->matrix.cells[row])
                    string = mw->matrix.cells[row][column];
                else
                    string = "";

                xbaeDrawCellString(mw, row, column, x, y, string, bg, fg);
            }
        }
        else
        {
            Pixmap pixmap;
            Pixmap mask;
            XbaeCellType type;
            int width, height;
            int depth;
        
            if (row < mw->matrix.rows && column < mw->matrix.columns)
            {
                type = xbaeGetDrawCellValue(mw, row, column, &string, &pixmap,
                                            &mask, &width, &height, &bg, &fg,
                                            &depth);
                if (type == XbaeString)
                    xbaeDrawCellString(mw, row, column, x, y, string, bg, fg);
                else if (type == XbaePixmap)
                    xbaeDrawCellPixmap(mw, row, column, x, y, pixmap, mask,
                                       width, height, bg, fg, depth);
            }
        }
}

XbaeCellType
xbaeGetDrawCellValue(mw, row, column, string, pixmap, mask, width,
                     height, bg, fg, depth)
XbaeMatrixWidget mw;
int row;
int column;
String *string;
Pixmap *pixmap;
Pixmap *mask;
int *width, *height;
Pixel *bg, *fg;
int *depth;
{
    XbaeMatrixDrawCellCallbackStruct call_data;

    call_data.reason = XbaeDrawCellReason;
    call_data.event = (XEvent *)NULL;
    call_data.row = row;
    call_data.column = column;
    call_data.width = COLUMN_WIDTH(mw, column) - TEXT_WIDTH_OFFSET(mw) * 2;
    call_data.height = ROW_HEIGHT(mw) - TEXT_HEIGHT_OFFSET(mw) * 2;
    call_data.type = XbaeString;
    call_data.string = "";
    call_data.pixmap = (Pixmap)NULL;
    call_data.mask = (Pixmap)NULL;
    call_data.foreground = *fg;
    call_data.background = *bg;
    call_data.depth = 0;

    XtCallCallbackList((Widget)mw, mw->matrix.draw_cell_callback,
                       (XtPointer) &call_data);

    *pixmap = call_data.pixmap;
    *mask = call_data.mask;
    *string = call_data.string ? call_data.string : ""; /* Handle NULLs */

    if (mw->matrix.reverse_select && mw->matrix.selected_cells &&
        mw->matrix.selected_cells[row][column])
    {
        /*
         * if colours were set by the draw cell callback, handle reverse
         * selection
         */
        if (*bg != call_data.background)
        {
            if (*fg != call_data.foreground)
                *bg = call_data.foreground;
            *fg = call_data.background;
        }
        else if (*fg != call_data.foreground)
            *bg = call_data.foreground;
    }
    else
    {
        *fg = call_data.foreground;
        *bg = call_data.background;
    }
    *width = call_data.width;
    *height = call_data.height;
    *depth = call_data.depth;
    
    if (call_data.type == XbaePixmap)
    {
        if (*mask == XmUNSPECIFIED_PIXMAP || *mask == BadPixmap)
            call_data.mask = 0;

        if (*pixmap == XmUNSPECIFIED_PIXMAP || *pixmap == BadPixmap)
        {
            XtAppWarningMsg(
                XtWidgetToApplicationContext((Widget)mw),
                "drawCellCallback", "Pixmap", "XbaeMatrix",
                "XbaeMatrix: Bad pixmap passed from drawCellCallback",
                NULL, 0);
            call_data.type = XbaeString;
            *string = "";
        }
        else if (!*depth)
        {
             /*
              * If we know the depth, width and height don't do a round
              * trip to find the
              * geometry
              */
            Window root_return;
            int x_return, y_return;
            unsigned int width_return, height_return;
            unsigned int border_width_return;
            unsigned int depth_return;

            if (XGetGeometry(XtDisplay(mw), *pixmap, &root_return,
                             &x_return, &y_return, &width_return,
                             &height_return, &border_width_return,
                             &depth_return))
            {
                *width = width_return;
                *height = height_return;
                *depth = depth_return;
            }
        }
    }
    return (call_data.type);
}

/*
 * Draw the column label for the specified column.  Handles labels in
 * fixed and non-fixed columns.
 */
void
#if NeedFunctionPrototypes
xbaeDrawColumnLabel(XbaeMatrixWidget mw, int column, Boolean pressed)
#else
xbaeDrawColumnLabel(mw, column, pressed)
XbaeMatrixWidget mw;
int column;
Boolean pressed;
#endif
{
    String label;
    int labelX, labelY;
    int buttonX;
    int i;
    GC gc;
    Window win = XtWindow(mw);
    Boolean clipped = (column >= (int)mw->matrix.fixed_columns &&
                       column < TRAILING_HORIZ_ORIGIN(mw));

    Boolean button = mw->matrix.button_labels ||
        (mw->matrix.column_button_labels &&
         mw->matrix.column_button_labels[column]);

    if (mw->matrix.column_labels[column][0] == '\0' && !button)
        return;

    /*
     * If the column label is in a fixed column, we don't need to account
     * for the horiz_origin
     */
    if (column < (int)mw->matrix.fixed_columns)
    {
        labelX = COLUMN_LABEL_OFFSET(mw) + COLUMN_POSITION(mw, column) +
            TEXT_X_OFFSET(mw);
        buttonX = COLUMN_LABEL_OFFSET(mw) + COLUMN_POSITION(mw, column);
    }
    else if (column >= TRAILING_HORIZ_ORIGIN(mw))
    {
        labelX = TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw) +
            COLUMN_POSITION(mw, column) -
            COLUMN_POSITION(mw, TRAILING_HORIZ_ORIGIN(mw)) +
            TEXT_X_OFFSET(mw);
        buttonX = TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw) +
            COLUMN_POSITION(mw, column) -
            COLUMN_POSITION(mw, TRAILING_HORIZ_ORIGIN(mw));
    }
    else
    {
        labelX = COLUMN_LABEL_OFFSET(mw) +
            (COLUMN_POSITION(mw, column) - HORIZ_ORIGIN(mw)) +
            TEXT_X_OFFSET(mw);
        buttonX = COLUMN_LABEL_OFFSET(mw) + (COLUMN_POSITION(mw, column) -
                                             HORIZ_ORIGIN(mw));
    }        

    /*
     * Set our y to the baseline of the first line in this column
     */
    labelY = -mw->matrix.label_font_y +
        mw->matrix.cell_shadow_thickness +
        mw->matrix.cell_highlight_thickness +
        mw->matrix.cell_margin_height +
        mw->matrix.text_shadow_thickness +
        (mw->matrix.column_label_maxlines -
         mw->matrix.column_label_lines[column].lines) * LABEL_HEIGHT(mw) +
        HORIZ_SB_OFFSET(mw);

    if (clipped)
        gc = mw->matrix.label_clip_gc;
    else
        gc = mw->matrix.label_gc;
    
    if (button)
    {
        XSetForeground(XtDisplay(mw), gc, mw->matrix.button_label_background);
        XFillRectangle(XtDisplay(mw), win, gc, buttonX, HORIZ_SB_OFFSET(mw),
                       COLUMN_WIDTH(mw, column), COLUMN_LABEL_HEIGHT(mw));
    }

    XSetForeground(XtDisplay(mw), gc, mw->matrix.column_label_color);
    XSetBackground(XtDisplay(mw), gc, mw->matrix.button_label_background);

    label = mw->matrix.column_labels[column];

    if (label[0] != '\0')
        for (i = 0; i < mw->matrix.column_label_lines[column].lines; i++)
        {
            xbaeDrawString(mw, XtWindow(mw), gc, label,
                           mw->matrix.column_label_lines[column].lengths[i],
                           labelX, labelY, mw->matrix.column_widths[column],
                           mw->matrix.column_label_alignments ?
                           mw->matrix.column_label_alignments[column] :
                           XmALIGNMENT_BEGINNING, False,
                           mw->matrix.bold_labels, False, True,
                           mw->matrix.column_label_color);
        
            labelY += LABEL_HEIGHT(mw);
            label += mw->matrix.column_label_lines[column].lengths[i] + 1;
        }
    if (button)
        xbaeDrawCellShadow(mw, XtWindow(mw), -1, column,
                           buttonX, HORIZ_SB_OFFSET(mw),
                           COLUMN_WIDTH(mw, column),
                           COLUMN_LABEL_HEIGHT(mw), True, clipped, pressed);
}

/*
 * Draw the row label for the specified row. Handles labels in fixed and
 * non-fixed rows.
 */
void
#if NeedFunctionPrototypes
xbaeDrawRowLabel(XbaeMatrixWidget mw, int row, Boolean pressed)
#else
xbaeDrawRowLabel(mw, row, pressed)
XbaeMatrixWidget mw;
int row;
Boolean pressed;
#endif
{
    int y;
    GC gc;
    Window win = XtWindow(mw);
    Boolean clipped = (row >= (int)mw->matrix.fixed_rows &&
                       row < TRAILING_VERT_ORIGIN(mw));
    
    Boolean button = mw->matrix.button_labels ||
        (mw->matrix.row_button_labels && mw->matrix.row_button_labels[row]);

    if (mw->matrix.row_labels[row][0] == '\0' && !button)
        return;

    /*
     * If the row label is in a fixed row we don't need to account
     * for the vert_origin
     */
    if (row < (int)mw->matrix.fixed_rows)
        y = ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) * row + TEXT_Y_OFFSET(mw);
    else if (row >= TRAILING_VERT_ORIGIN(mw))
        y = TRAILING_FIXED_ROW_LABEL_OFFSET(mw) +
            ROW_HEIGHT(mw) * (row - TRAILING_VERT_ORIGIN(mw)) +
            TEXT_Y_OFFSET(mw);
    else
        y = ROW_LABEL_OFFSET(mw) + ROW_HEIGHT(mw) * (row - VERT_ORIGIN(mw)) +
            LABEL_Y_OFFSET(mw) - mw->matrix.first_row_offset;

    if (clipped)
        gc = mw->matrix.label_clip_gc;
    else
        gc = mw->matrix.label_gc;
    
    if (button)
    {
        XSetForeground(XtDisplay(mw), gc, mw->matrix.button_label_background);
        XFillRectangle(XtDisplay(mw), win, gc, VERT_SB_OFFSET(mw),
                       y - TEXT_Y_OFFSET(mw), ROW_LABEL_WIDTH(mw),
                       ROW_HEIGHT(mw));
    }

    XSetForeground(XtDisplay(mw), gc, mw->matrix.row_label_color);
    XSetBackground(XtDisplay(mw), gc, mw->matrix.button_label_background);

    if (mw->matrix.row_labels[row][0] != '\0')
        xbaeDrawString(mw, win, gc,
                       mw->matrix.row_labels[row],
                       strlen(mw->matrix.row_labels[row]),
                       TEXT_X_OFFSET(mw) + VERT_SB_OFFSET(mw), y,
                       mw->matrix.row_label_width,
                       mw->matrix.row_label_alignment, False,
                       mw->matrix.bold_labels, True, False,
                       mw->matrix.row_label_color);
    
    if (button)
        xbaeDrawCellShadow(mw, win, row, -1, VERT_SB_OFFSET(mw),
                           y - TEXT_Y_OFFSET(mw), ROW_LABEL_WIDTH(mw),
                           ROW_HEIGHT(mw), True, clipped, pressed);
}
