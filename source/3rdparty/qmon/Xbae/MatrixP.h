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
 * $Id: MatrixP.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

/*
 * MatrixP.h - Private definitions for Matrix widget
 */

#ifndef _Xbae_MatrixP_h
#define _Xbae_MatrixP_h

#if XmVersion <= 1001
#        include <Xm/XmP.h>
#else
#        include <Xm/ManagerP.h>
#endif

#include "Matrix.h"

#ifndef P
#if defined(__STDC__) || defined (__cplusplus)
#define P(x)                x
#else
#define P(x)                ()
#define const
#define volatile
#endif
#endif

/*
 * A few definitions we like to use, but those with R4 won't have.
 * From Xfuncproto.h in R5.
 */

#ifndef XlibSpecificationRelease
# ifndef _XFUNCPROTOBEGIN
#   ifdef __cplusplus                      /* for C++ V2.0 */
#     define _XFUNCPROTOBEGIN extern "C" {   /* do not leave open across includes */
#     define _XFUNCPROTOEND }
#   else
#     define _XFUNCPROTOBEGIN
#     define _XFUNCPROTOEND
#   endif
# endif /* _XFUNCPROTOBEGIN */
#else
#include <X11/Xfuncproto.h>
#endif

/*
 * New types for the class methods
 */
_XFUNCPROTOBEGIN

typedef void (*XbaeMatrixSetCellProc) P((XbaeMatrixWidget, int, int,
                                         const String, Boolean));

typedef String (*XbaeMatrixGetCellProc) P((XbaeMatrixWidget, int, int));

typedef void (*XbaeMatrixEditCellProc) P((XbaeMatrixWidget, XEvent *,
                                          int, int, String *, Cardinal));

typedef void (*XbaeMatrixSelectCellProc) P((XbaeMatrixWidget, int, int));

typedef void (*XbaeMatrixSelectRowProc) P((XbaeMatrixWidget, int));

typedef void (*XbaeMatrixSelectColumnProc) P((XbaeMatrixWidget, int));

typedef void (*XbaeMatrixDeselectAllProc) P((XbaeMatrixWidget));

typedef void (*XbaeMatrixSelectAllProc) P((XbaeMatrixWidget));

typedef void (*XbaeMatrixDeselectCellProc) P((XbaeMatrixWidget, int, int));

typedef void (*XbaeMatrixDeselectRowProc) P((XbaeMatrixWidget, int));

typedef void (*XbaeMatrixDeselectColumnProc) P((XbaeMatrixWidget, int));

typedef Boolean (*XbaeMatrixCommitEditProc) P((XbaeMatrixWidget, XEvent *,
                                               Boolean));

typedef void (*XbaeMatrixCancelEditProc) P((XbaeMatrixWidget, Boolean));

typedef void (*XbaeMatrixAddRowsProc) P((XbaeMatrixWidget, int, String *,
                                         String *, Pixel *, Pixel *, int));

typedef void (*XbaeMatrixDeleteRowsProc) P((XbaeMatrixWidget, int, int));

typedef void (*XbaeMatrixAddColumnsProc) P((XbaeMatrixWidget, int, String *,
                                            String *, short *, int *,
                                            unsigned char*, unsigned char *,
                                            Pixel *, Pixel *, int));

typedef void (*XbaeMatrixDeleteColumnsProc) P((XbaeMatrixWidget, int, int));

typedef void (*XbaeMatrixSetRowColorsProc) P((XbaeMatrixWidget, int, Pixel *,
                                              int, Boolean));

typedef void (*XbaeMatrixSetColumnColorsProc) P((XbaeMatrixWidget, int,
                                                 Pixel *, int, Boolean));

typedef void (*XbaeMatrixSetCellColorProc) P((XbaeMatrixWidget, int, int,
                                              Pixel, Boolean));

_XFUNCPROTOEND

/*
 * Different than the traversal directions in Xm.h
 */
#define NOT_TRAVERSING        -1

/*
 * New fields for the Matrix widget class record
 */
typedef struct {
    XbaeMatrixSetCellProc                set_cell;
    XbaeMatrixGetCellProc                get_cell;
    XbaeMatrixEditCellProc                edit_cell;
    XbaeMatrixSelectCellProc                select_cell;
    XbaeMatrixSelectRowProc                select_row;
    XbaeMatrixSelectColumnProc                select_column;
    XbaeMatrixDeselectAllProc                deselect_all;
    XbaeMatrixSelectAllProc                select_all;
    XbaeMatrixDeselectCellProc                deselect_cell;
    XbaeMatrixDeselectRowProc                deselect_row;
    XbaeMatrixDeselectColumnProc        deselect_column;
    XbaeMatrixCommitEditProc                commit_edit;
    XbaeMatrixCancelEditProc                cancel_edit;
    XbaeMatrixAddRowsProc                add_rows;
    XbaeMatrixDeleteRowsProc                delete_rows;
    XbaeMatrixAddColumnsProc                add_columns;
    XbaeMatrixDeleteColumnsProc                delete_columns;
    XbaeMatrixSetRowColorsProc                set_row_colors;
    XbaeMatrixSetColumnColorsProc        set_column_colors;
    XbaeMatrixSetCellColorProc                set_cell_color;
    XtPointer                                extension;
} XbaeMatrixClassPart;

/*
 * Full class record declaration
 */
typedef struct _XbaeMatrixClassRec {
    CoreClassPart                core_class;
    CompositeClassPart                composite_class;
    ConstraintClassPart                constraint_class;
    XmManagerClassPart                manager_class;
    XbaeMatrixClassPart                matrix_class;
} XbaeMatrixClassRec;

externalref XbaeMatrixClassRec xbaeMatrixClassRec;


/*
 * Inheritance constants for set/get/edit methods
 */
#define XbaeInheritGetCell ((XbaeMatrixGetCellProc) _XtInherit)
#define XbaeInheritSetCell ((XbaeMatrixSetCellProc) _XtInherit)
#define XbaeInheritEditCell ((XbaeMatrixEditCellProc) _XtInherit)
#define XbaeInheritSelectCell ((XbaeMatrixSelectCellProc) _XtInherit)
#define XbaeInheritSelectRow ((XbaeMatrixSelectRowProc) _XtInherit)
#define XbaeInheritSelectColumn ((XbaeMatrixSelectColumnProc) _XtInherit)
#if XmVersion >= 1002
#define XbaeInheritHighlightCell ((XbaeMatrixHighlightCellProc) _XtInherit)
#define XbaeInheritHighlightRow ((XbaeMatrixHighlightRowProc) _XtInherit)
#define XbaeInheritHighlightColumn ((XbaeMatrixHighlightColumnProc) _XtInherit)
#endif
#define XbaeInheritDeselectAll ((XbaeMatrixDeselectAllProc) _XtInherit)
#define XbaeInheritSelectAll ((XbaeMatrixSelectAllProc) _XtInherit)
#define XbaeInheritDeselectCell ((XbaeMatrixDeselectCellProc) _XtInherit)
#define XbaeInheritDeselectRow ((XbaeMatrixDeselectRowProc) _XtInherit)
#define XbaeInheritDeselectColumn ((XbaeMatrixDeselectColumnProc) _XtInherit)
#define XbaeInheritCommitEdit ((XbaeMatrixCommitEditProc) _XtInherit)
#define XbaeInheritCancelEdit ((XbaeMatrixCancelEditProc) _XtInherit)
#define XbaeInheritAddRows ((XbaeMatrixAddRowsProc) _XtInherit)
#define XbaeInheritDeleteRows ((XbaeMatrixDeleteRowsProc) _XtInherit)
#define XbaeInheritAddColumns ((XbaeMatrixAddColumnsProc) _XtInherit)
#define XbaeInheritDeleteColumns ((XbaeMatrixDeleteColumnsProc)_XtInherit)
#define XbaeInheritSetRowColors ((XbaeMatrixSetRowColorsProc)_XtInherit)
#define XbaeInheritSetColumnColors ((XbaeMatrixSetColumnColorsProc)_XtInherit)
#define XbaeInheritSetCellColor ((XbaeMatrixSetCellColorProc)_XtInherit)

/*
 * New data structures for the ScrollMgr code
 */
typedef struct _SmScrollNode {
    int x;
    int y;
    struct _SmScrollNode *next;
    struct _SmScrollNode *prev;
} SmScrollNodeRec, *SmScrollNode;

typedef struct _SmScrollMgr {
    int offset_x;
    int offset_y;
    int scroll_count;
    SmScrollNode scroll_queue;
    Boolean scrolling;
} SmScrollMgrRec, *SmScrollMgr;

/*
 * Rectangle struct used for internal calculations.  (x1,y1) are the upper
 * left corner, (x2,y2) are the lower right.
 */
typedef struct _Rectangle
{
    int x1, y1;
    int x2, y2;
}
Rectangle;

/*
 * Data structure for column labels
 */
typedef struct _ColumnLabelLines {
    int lines;
    int *lengths;
} ColumnLabelLinesRec, *ColumnLabelLines;

/*
 * New fields for the Matrix widget record
 */
typedef struct {
    /*
     * resources
     */
    Boolean        allow_column_resize;        /* can columns dynamically resize?   */
    Boolean        bold_labels;                /* draw bold row/column labels?             */
    Boolean        button_labels;                /* draw labels as buttons?             */
    Boolean        fill;                        /* fill available space?             */
    Boolean        trailing_attached_right;  /* trailing columns fixed to right */
    Boolean        trailing_attached_bottom; /* trailing rows fixed to bottom   */
    Boolean        reverse_select;                /* reverse colours - selected cells? */
    Boolean        scroll_select;                /* flag to scroll a selected cell    */
    Boolean        **selected_cells;        /* 2D array of selected cells             */
    Boolean        show_arrows;                /* sow arrows when text obscured?    */
    Boolean     *column_button_labels;  /* which column labels are butons */
    Boolean     *row_button_labels;     /* which row labels are butons */
    Boolean        traverse_fixed;                /* allow traversal to fixed cells?   */
    Boolean        calc_cursor_position;        /* calculate insert pos from click   */
    unsigned char cell_shadow_type;        /* cell shadow type                     */
    unsigned char **cell_shadow_types;        /* 2D array of per cell shadow type  */
    unsigned char *column_alignments;        /* alignment of each column             */
    unsigned char *column_label_alignments;/* alignment of each column label */
    unsigned char *column_shadow_types;        /* 1D array of per col shadow types  */
    unsigned char grid_type;                /* shadowed in/shadowed out/plain    */
    unsigned char hsb_display_policy;        /* horiz scroll bar display policy   */
    unsigned char row_label_alignment;        /* alignment of row labels             */
    unsigned char *row_shadow_types;        /* 1D array of per row shadow types  */
    unsigned char scrollbar_placement;        /* placement of the scrollbars             */
    unsigned char selection_policy;        /* as for XmList */
    unsigned char shadow_type;                /* matrix window shadow type             */
#if XmVersion >= 1002
    unsigned char **highlighted_cells;        /* 2D array of highlighted cells     */
#endif
    unsigned char vsb_display_policy;        /* vert scroll bar display policy    */

    String        **cells;                /* 2D array of strings                     */
    String        *column_labels;                /* array of labels above each column */
    String        *row_labels;                /* array of labels next to each row  */

    XtPointer        **cell_user_data;        /* 2D array of per cell user data    */
    XtPointer        *column_user_data;        /* 1D array of per column user data  */
    XtPointer        *row_user_data;                /* 1D array of per row user data     */

    short        *column_widths;                /* width of each column in chars     */
    short        row_label_width;        /* max width of row labels in chars  */

    int                alt_row_count;                /* # of rows for e/o background             */
    int                columns;                /* number of cells per row             */
    int                *column_max_lengths;        /* max length of each col in chars   */
    int         double_click_interval;        /* interval between clicks             */
    int         left_column;                /* horizontal origin (in col space)  */
    int                rows;                        /* number of rows per column             */
    int                top_row;                /* vertical origin (in row space)    */

    Dimension        cell_highlight_thickness; /* hilite thickness for textField  */
    Dimension        cell_margin_height;        /* margin height for textField             */
    Dimension        cell_margin_width;        /* margin width for textField             */
    Dimension        cell_shadow_thickness;        /* shadow thickness for each cell    */
    Dimension        fixed_columns;                /* number of leading fixed columns   */
    Dimension        fixed_rows;                /* number of leading fixed rows             */
    Dimension        space;                        /* spacing for scrollbars             */
    Dimension        text_shadow_thickness;        /* shadow thickness for text field   */
    Dimension        trailing_fixed_columns;        /* number of trailing fixed columns  */
    Dimension        trailing_fixed_rows;        /* number of trailing fixed rows     */
    Dimension        visible_columns;        /* number of columns to make visible */
    Dimension        visible_rows;                /* number of rows to make visible    */

    Pixel        button_label_background; /* color of button label background */
    Pixel        **cell_background;        /* 2D array of Pixels                     */
    Pixel        **colors;                /* 2D array of Pixels                     */
    Pixel        column_label_color;        /* color of column label             */
    Pixel        even_row_background;        /* even row background color             */
    Pixel        grid_line_color;        /* color of grid, for XmGrid_LINE    */
    Pixel       odd_row_background;        /* odd row background color             */
    Pixel        row_label_color;        /* color of row label                     */
    Pixel        selected_background;        /* background for selected cells     */
    Pixel        selected_foreground;        /* foreground for selected cells     */
    Pixel        text_background;        /* background for the "text" field   */

    XtTranslations text_translations;        /* translations for textField widget */

    XtCallbackList default_action_callback; /* called for a double click     */
    XtCallbackList draw_cell_callback;        /* called when a cell is drawn             */
    XtCallbackList enter_cell_callback;        /* called when a cell is entered     */
    XtCallbackList label_activate_callback; /* called when label pressed     */
    XtCallbackList leave_cell_callback;        /* called when a cell is left             */
    XtCallbackList modify_verify_callback; /* verify change to textField     */
                                        /* and a draw_cell_callback is set   */
#if XmVersion > 1001
    XtCallbackList process_drag_callback; /* called when a drag is initiated */
#endif
    XtCallbackList resize_callback;        /* called when Matrix is resized     */
    XtCallbackList resize_column_callback; /* called when column is resized  */
    XtCallbackList select_cell_callback; /* called when cells are selected   */
    XtCallbackList traverse_cell_callback; /* next cell to traverse to             */
    XtCallbackList write_cell_callback;        /*called when a cell needs to be set*/ 

    XmFontList        font_list;                /* fontList of widget and textField  */
    XmFontList        label_font_list;        /* fontList of labels                     */

    
    /*
     * private state
     */
#if XmVersion >= 1002
    unsigned char highlight_location;        /* What is being highlighted             */
#endif
    short        first_row_offset;        /* hidden first row */
    int                cell_visible_height;        /* height of visible cells in pixels */
    int                column_label_maxlines;        /* max # lines in column labels             */
    int                *column_positions;        /* pixel position of each column     */
    int                current_column;                /* column of the text field             */
    int                current_row;                /* row of the text field             */
    int                horiz_origin;                /* horiz origin (in pixel space)     */
    int                label_baseline;                /* baseline of label                     */
    int                last_column;                /* The last selected column             */
    int                last_row;                /* The last selected row             */
    int                num_selected_cells;        /* The number selected cells             */
    int                text_baseline;                /* baseline of text in each cell     */
    int                traversing;                /* direction we are traversing             */

    unsigned int non_fixed_total_width;        /* width of cell area in pixels             */
    unsigned int current_clip;                /* current clip mask setting             */
    unsigned int disable_redisplay;        /* disable redisplay counter             */

    Dimension        desired_height;                /* height widget wants to be             */
    Dimension        desired_width;                /* width widget wants to be             */

    Time        last_click_time;        /* when last ButtonPress occurred    */

    Widget        text_field;                /* the text field                     */
    Widget        horizontal_sb;                /* the horizontal scrollbar             */
    Widget      vertical_sb;                /* the vertical scrollbar             */
    Widget        clip_window;                /* the clip child                     */
    Widget        left_clip;                /* clips for scrolling fixed cells   */
    Widget        right_clip;
    Widget        top_clip;
    Widget        bottom_clip;
    Widget        current_parent;                /* Current textField parent window   */
#if CELL_WIDGETS
    Widget        **cell_widgets;                /* array of widgets for cells             */
#endif

    GC                cell_bottom_shadow_clip_gc; /* GC for clipped bottom shadow  */
    GC          cell_grid_line_gc;
    GC                cell_top_shadow_clip_gc; /* GC for clipped top shadow             */
    GC                draw_gc;                /* GC for drawing cells                     */
    GC          grid_line_gc;                /* GC for grid line                     */
    GC                label_clip_gc;                /* GC for clipped labels             */
    GC                label_gc;                /* GC for drawing labels             */
    GC                pixmap_gc;                /* GC for drawing pixmap cells             */
    GC                resize_bottom_shadow_gc;
    GC                resize_top_shadow_gc;

    ColumnLabelLines column_label_lines; /* structs for multi line labels    */
                                     
    XFontStruct        *font_struct;                /* fontStruct from fontList             */
    XFontStruct        *label_font_struct;        /* fontStruct from fontList             */

    SmScrollMgr clip_scroll_mgr;        /* ScrollMgr for Clip                     */
    SmScrollMgr matrix_scroll_mgr;        /* ScrollMgr for Matrix                     */

    XFontSet        font_set;                /* fontSet from fontList             */
    short        font_y;
    short        font_width;
    short        font_height;
    Font        fid;
    XFontSet        label_font_set;                /* fontSet from fontList             */
    short        label_font_y;
    short        label_font_width;
    short        label_font_height;
    Font        label_fid;

} XbaeMatrixPart;

/*
 * Full instance record declaration
 */
typedef struct _XbaeMatrixRec {
    CorePart                core;
    CompositePart        composite;
    ConstraintPart        constraint;
    XmManagerPart        manager;
    XbaeMatrixPart        matrix;
} XbaeMatrixRec;

/* provide clean-up for those with R4 */
#ifndef XlibSpecificationRelease
# undef _Xconst
# undef _XFUNCPROTOBEGIN
# undef _XFUNCPROTOEND
#endif

#undef P
#endif /* _Xbae_MatrixP_h */

