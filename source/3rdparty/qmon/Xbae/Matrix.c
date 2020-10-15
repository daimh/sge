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
 * $Id: Matrix.c,v 1.1 2001-07-18 11:05:59 root Exp $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/StringDefs.h>
#include <X11/Xlib.h>
#include <Xm/XmP.h>
#include <Xm/AtomMgr.h>
#include <Xm/ScrollBar.h>
#if XmVersion > 1001
#include <Xm/DrawP.h>
#include <Xm/DragIcon.h>
#include <Xm/DragC.h>
#endif
#include "Input.h"
#include "Clip.h"
#include "MatrixP.h"
#include "Converters.h"
#include "ScrollMgr.h"
#include "Actions.h"
#include "MCreate.h"
#include "Methods.h"
#include "Utils.h"
#include "Shadow.h"

#ifndef XlibSpecificationRelease
#define XrmPermStringToQuark XrmStringToQuark
#endif

/*
 * Translations for Matrix (these will also be used by the Clip child).
 */
static char defaultTranslations[] =
"<Btn1Up>                        :        DefaultAction()\n\
<Btn1Down>                        :        DefaultAction() EditCell(Pointer)\n\
Shift<Btn2Down>                        :        ResizeColumns()\n\
<Btn2Down>                        :        ProcessDrag()\n\
<Btn1Motion>                        :        HandleMotion()";

/*
 * Default translations for XmNtextTranslations resource
 */
static char default_text_translations[] =
"#override\n\
Shift ~Ctrl ~Meta ~Alt <Key>Tab        :        EditCell(Left)\n\
~Ctrl ~Meta ~Alt <Key>Tab        :        EditCell(Right)\n\
<Key>osfUp                        :        EditCell(Up)\n\
<Key>osfDown                        :        EditCell(Down)\n\
<Key>osfActivate                :        CommitEdit(False)\n\
~Shift ~Meta ~Alt <Key>Return        :           CommitEdit(False)\n\
<Key>osfCancel                        :        CancelEdit(False)\n\
Shift Ctrl ~Meta ~Alt <Key>Tab        :        TraversePrev()\n\
Ctrl ~Meta ~Alt <Key>Tab        :        TraverseNext()\n\
<Key>osfPageDown                :        PageDown()\n\
<Key>osfPageUp                        :        PageUp()";

    
#define offset(field)        XtOffsetOf(XbaeMatrixRec, field)

static XtResource resources[] =
{
#ifdef ALLOW_COLUMN_RESIZE
    {XmNallowColumnResize, XmCColumnResize, XmRBoolean, sizeof(Boolean),
     offset(matrix.allow_column_resize), XmRImmediate, (XtPointer) True},
#else
    {XmNallowColumnResize, XmCColumnResize, XmRBoolean, sizeof(Boolean),
     offset(matrix.allow_column_resize), XmRImmediate, (XtPointer) False},
#endif
    {XmNaltRowCount, XmCAltRowCount, XmRInt, sizeof(int),
     offset(matrix.alt_row_count), XmRImmediate, (XtPointer) 1},

    {XmNboldLabels, XmCBoldLabels, XmRBoolean, sizeof(Boolean),
     offset(matrix.bold_labels), XmRImmediate, (XtPointer) False},

    {XmNbuttonLabels, XmCButtonLabels, XmRBoolean, sizeof(Boolean),
     offset(matrix.button_labels), XmRImmediate, (XtPointer) False},

    {XmNbuttonLabelBackground, XmCColor, XmRPixel, sizeof(Pixel),
     offset(matrix.button_label_background), XmRCallProc,
     (XtPointer)xbaeCopyBackground},
    
    {XmNcalcCursorPosition, XmCCalcCursorPosition, XmRBoolean, sizeof(Boolean),
     offset(matrix.calc_cursor_position), XmRImmediate, (XtPointer) False},

    {XmNcellBackgrounds, XmCColors, XmRPixelTable, sizeof(Pixel **),
     offset(matrix.cell_background), XmRImmediate, (XtPointer) NULL},

    {XmNcellHighlightThickness, XmCHighlightThickness, XmRHorizontalDimension,
     sizeof(Dimension), offset(matrix.cell_highlight_thickness),
     XmRImmediate, (XtPointer) 2},

    {XmNcellMarginHeight, XmCMarginHeight, XmRVerticalDimension,
     sizeof(Dimension), offset(matrix.cell_margin_height),
     XmRImmediate, (XtPointer) 3},

    {XmNcellMarginWidth, XmCMarginWidth, XmRHorizontalDimension,
     sizeof(Dimension), offset(matrix.cell_margin_width),
     XmRImmediate, (XtPointer) 3},

    {XmNcellShadowThickness, XmCShadowThickness, XmRDimension,
     sizeof(Dimension), offset(matrix.cell_shadow_thickness),
     XmRImmediate, (XtPointer) 1},

    {XmNcellShadowType, XmCShadowType, XmRShadowType,
     sizeof(unsigned char), offset(matrix.cell_shadow_type), XmRImmediate,
     (XtPointer) XmSHADOW_OUT},
    
    {XmNcellShadowTypes, XmCCellShadowTypes, XmRShadowTypeTable,
     sizeof(unsigned char**), offset(matrix.cell_shadow_types), XmRImmediate,
     (XtPointer) NULL},
    
    {XmNcellUserData, XmCCellUserData, XmRUserDataTable,
      sizeof(XtPointer**), offset(matrix.cell_user_data), XmRImmediate,
      (XtPointer) NULL},
#if CELL_WIDGETS
    {XmNcellWidgets, XmCCellWidgets, XmRWidgetTable,
      sizeof(Widget **), offset(matrix.cell_widgets), XmRImmediate,
      (XtPointer) NULL},
#endif    
    {XmNcells, XmCCells, XmRCellTable, sizeof(String **),
     offset(matrix.cells), XmRImmediate, NULL},

    {XmNclipWindow, XmCClipWindow, XmRWidget, sizeof(Widget),
     offset(matrix.clip_window), XmRImmediate, NULL},

    {XmNcolors, XmCColors, XmRPixelTable, sizeof(Pixel **),
     offset(matrix.colors), XmRImmediate, (XtPointer) NULL},

    {XmNcolumnAlignments, XmCAlignments, XmRAlignmentArray,
     sizeof(unsigned char *), offset(matrix.column_alignments),
     XmRImmediate, (XtPointer) NULL},

    {XmNcolumnButtonLabels, XmCButtonLabels, XmRBooleanArray,
     sizeof(Boolean*), offset(matrix.column_button_labels),
     XmRImmediate, (XtPointer) NULL},

    {XmNcolumnLabelAlignments, XmCAlignments, XmRAlignmentArray,
     sizeof(unsigned char *), offset(matrix.column_label_alignments),
     XmRImmediate, (XtPointer) NULL},

    {XmNcolumnLabelColor, XmCColor, XmRPixel, sizeof(Pixel),
     offset(matrix.column_label_color), XmRCallProc,
     (XtPointer)xbaeCopyForeground},
    
    {XmNcolumnLabels, XmCLabels, XmRStringArray, sizeof(String *),
     offset(matrix.column_labels), XmRImmediate, NULL},

    {XmNcolumnMaxLengths, XmCColumnMaxLengths, XmRMaxLengthArray,
     sizeof(int *), offset(matrix.column_max_lengths),
     XmRImmediate, NULL},

    {XmNcolumnShadowTypes, XmCShadowTypes, XmRShadowTypeArray,
      sizeof(unsigned char*), offset(matrix.column_shadow_types), XmRImmediate,
      (XtPointer) NULL},
    
    {XmNcolumnUserData, XmCUserDatas, XmRUserDataArray,
      sizeof(XtPointer*), offset(matrix.column_user_data), XmRImmediate,
      (XtPointer) NULL},
    
    {XmNcolumnWidths, XmCColumnWidths, XmRWidthArray, sizeof(short *),
     offset(matrix.column_widths), XmRImmediate, NULL},

    {XmNcolumns, XmCColumns, XmRInt, sizeof(int),
     offset(matrix.columns), XmRImmediate, (XtPointer) 1},

    {XmNdefaultActionCallback,XmCCallback,XmRCallback,sizeof(XtCallbackList),
     offset(matrix.default_action_callback),XmRCallback,NULL },    

    {XmNdoubleClickInterval, XmCDoubleClickInterval, XmRInt, sizeof(int),
     offset(matrix.double_click_interval), XmRCallProc,
     (XtPointer)xbaeCopyDoubleClick },

    {XmNdrawCellCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.draw_cell_callback), XmRCallback, NULL},

    {XmNenterCellCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.enter_cell_callback), XmRCallback, NULL},

    {XmNevenRowBackground, XmCBackground, XmRPixel, sizeof(Pixel),
     offset(matrix.even_row_background), XmRCallProc,
     (XtPointer)xbaeCopyBackground},

    {XmNfill, XmCFill, XmRBoolean, sizeof(Boolean),
     offset(matrix.fill), XmRImmediate, (XtPointer) False},

    {XmNfixedColumns, XmCFixedColumns, XmRDimension, sizeof(Dimension),
     offset(matrix.fixed_columns), XmRImmediate, (XtPointer) 0},

    {XmNfixedRows, XmCFixedRows, XmRDimension, sizeof(Dimension),
     offset(matrix.fixed_rows), XmRImmediate, (XtPointer) 0},

    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
     offset(matrix.font_list), XmRString, (XtPointer) "fixed"},

    {XmNgridLineColor, XmCColor, XmRPixel, sizeof(Pixel),
     offset(matrix.grid_line_color), XmRCallProc, 
     (XtPointer)xbaeCopyForeground},

    {XmNgridType, XmCGridType, XmRGridType,
     sizeof(unsigned char), offset(matrix.grid_type),
     XmRImmediate, (XtPointer)XmGRID_CELL_LINE},

#if XmVersion >= 1002
    {XmNhighlightedCells, XmCHighlightedCells, XmRHighlightTable,
     sizeof(unsigned char **), offset(matrix.highlighted_cells),
     XmRImmediate, (XtPointer) NULL},
#endif
    
    {XmNhorizontalScrollBar, XmCHorizontalScrollBar, XmRWidget, sizeof(Widget),
     offset(matrix.horizontal_sb), XmRImmediate, NULL},

    {XmNhorizontalScrollBarDisplayPolicy, XmCMatrixScrollBarDisplayPolicy,
     XmRMatrixScrollBarDisplayPolicy, sizeof(unsigned char),
     offset(matrix.hsb_display_policy), XmRImmediate,
     (XtPointer) XmDISPLAY_AS_NEEDED},

    {XmNlabelActivateCallback, XmCCallback, XmRCallback,
     sizeof(XtCallbackList), offset(matrix.label_activate_callback),
     XmRCallback, NULL},

    {XmNlabelFont, XmCFontList, XmRFontList, sizeof(XmFontList),
     offset(matrix.label_font_list), XmRString, (XtPointer)NULL },

    {XmNleaveCellCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.leave_cell_callback), XmRCallback, NULL},

    {XmNleftColumn, XmCLeftColumn, XmRInt, sizeof(int),
     offset(matrix.left_column), XmRImmediate, (XtPointer) 0 },

    {XmNmodifyVerifyCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.modify_verify_callback), XmRCallback, NULL},

    {XmNoddRowBackground, XmCBackground, XmRPixel, sizeof(Pixel),
     offset(matrix.odd_row_background), XmRCallProc,
     (XtPointer)xbaeCopyBackground},

#if XmVersion > 1001
    {XmNprocessDragCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.process_drag_callback), XmRCallback, NULL},
#endif

    /* Resize callback resource. Added by mjs */
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.resize_callback), XmRCallback, NULL},

    {XmNresizeColumnCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.resize_column_callback), XmRCallback, NULL},

    {XmNreverseSelect, XmCReverseSelect, XmRBoolean, sizeof(Boolean),
     offset(matrix.reverse_select), XmRImmediate, (XtPointer) False},

    {XmNrowButtonLabels, XmCButtonLabels, XmRBooleanArray,
     sizeof(Boolean*), offset(matrix.row_button_labels),
     XmRImmediate, (XtPointer) NULL},

    {XmNrowLabelAlignment, XmCAlignment, XmRAlignment, sizeof(unsigned char),
     offset(matrix.row_label_alignment),
     XmRImmediate, (XtPointer) XmALIGNMENT_END},

    {XmNrowLabelColor, XmCColor, XmRPixel, sizeof(Pixel),
     offset(matrix.row_label_color), XmRCallProc,
     (XtPointer)xbaeCopyForeground },

    {XmNrowLabelWidth, XmCRowLabelWidth, XmRShort, sizeof(short),
     offset(matrix.row_label_width), XmRImmediate, (XtPointer) 0},

    {XmNrowLabels, XmCLabels, XmRStringArray, sizeof(String *),
     offset(matrix.row_labels), XmRImmediate, NULL},

    {XmNrowShadowTypes, XmCShadowTypes, XmRShadowTypeArray,
      sizeof(unsigned char*), offset(matrix.row_shadow_types), XmRImmediate,
      (XtPointer) NULL},
    
    {XmNrowUserData, XmCUserDatas, XmRUserDataArray,
      sizeof(XtPointer*), offset(matrix.row_user_data), XmRImmediate,
      (XtPointer) NULL},
    
    {XmNrows, XmCRows, XmRInt, sizeof(int),
     offset(matrix.rows), XmRImmediate, (XtPointer) 1},

    {XmNscrollBarPlacement, XmCScrollBarPlacement, XmRScrollBarPlacement,
     sizeof(unsigned char), offset(matrix.scrollbar_placement), XmRImmediate,
     (XtPointer) XmBOTTOM_RIGHT },

    {XmNselectCellCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.select_cell_callback), XmRCallback, NULL},

    {XmNselectScrollVisible, XmCSelectScrollVisible, XmRBoolean,
     sizeof(Boolean), offset(matrix.scroll_select), XmRImmediate,
     (XtPointer) True},

    {XmNselectedBackground, XmCColor, XmRPixel, sizeof(Pixel),
     offset(matrix.selected_background), XmRCallProc, 
     (XtPointer)xbaeCopyForeground},

    {XmNselectedCells, XmCSelectedCells, XmRBooleanTable, sizeof(Boolean **),
     offset(matrix.selected_cells), XmRImmediate, (XtPointer) NULL},

    {XmNselectedForeground, XmCColor, XmRPixel, sizeof(Pixel),
     offset(matrix.selected_foreground), XmRCallProc, 
     (XtPointer)xbaeCopyBackground},

    {XmNselectionPolicy, XmCSelectionPolicy, XmRSelectionPolicy,
     sizeof(unsigned char), offset(matrix.selection_policy), XmRImmediate, 
     XmSINGLE_SELECT},

    /* Override Manager default */
    {XmNshadowThickness, XmCShadowThickness, XmRHorizontalDimension,
     sizeof(Dimension), XtOffsetOf(XmManagerRec, manager.shadow_thickness),
     XmRImmediate, (XtPointer) 2},

    {XmNshadowType, XmCShadowType, XmRShadowType,
     sizeof(unsigned char), offset(matrix.shadow_type), XmRImmediate,
     (XtPointer) XmSHADOW_IN},

    {XmNshowArrows, XmCShowArrows, XmRBoolean, sizeof(Boolean),
     offset(matrix.show_arrows), XmRImmediate, (XtPointer) False},

    {XmNspace, XmCSpace, XmRHorizontalDimension, sizeof(Dimension),
     offset(matrix.space), XmRImmediate, (XtPointer) 4},

    {XmNtextBackground, XmCTextBackground, XmRPixel, sizeof(Pixel),
     offset(matrix.text_background), XmRCallProc,
     (XtPointer)xbaeCopyBackground},

    {XmNtextField, XmCTextField, XmRWidget, sizeof(Widget),
     offset(matrix.text_field), XmRImmediate, NULL},

    {XmNtextShadowThickness, XmCTextShadowThickness, XmRDimension,
     sizeof(Dimension), offset(matrix.text_shadow_thickness), XmRImmediate,
     (XtPointer) 0},

    {XmNtextTranslations, XmCTranslations, XmRTranslationTable,
     sizeof(XtTranslations), offset(matrix.text_translations),
     XmRString, (XtPointer) default_text_translations},

    {XmNtopRow, XmCTopRow, XmRInt, sizeof(int),
     offset(matrix.top_row), XmRImmediate, (XtPointer) 0},

    {XmNtrailingAttachedBottom, XmCTrailingAttachedBottom, XmRBoolean, sizeof(Boolean),
     offset(matrix.trailing_attached_bottom), XmRImmediate, (XtPointer) False},

    {XmNtrailingAttachedRight, XmCTrailingAttachedRight, XmRBoolean, sizeof(Boolean),
     offset(matrix.trailing_attached_right), XmRImmediate, (XtPointer) False},

    {XmNtrailingFixedColumns, XmCTrailingFixedColumns, XmRDimension,
     sizeof(Dimension), offset(matrix.trailing_fixed_columns),
     XmRImmediate, (XtPointer) 0},

    {XmNtrailingFixedRows, XmCTrailingFixedRows, XmRDimension,
     sizeof(Dimension), offset(matrix.trailing_fixed_rows),
     XmRImmediate, (XtPointer) 0},

    {XmNtraverseCellCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.traverse_cell_callback), XmRCallback, NULL},

    {XmNtraverseFixedCells, XmCTraverseFixedCells, XmRBoolean, sizeof(Boolean),
     offset(matrix.traverse_fixed), XmRImmediate, (XtPointer) False},

    {XmNverticalScrollBar, XmCVerticalScrollBar, XmRWidget, sizeof(Widget),
     offset(matrix.vertical_sb), XmRImmediate, NULL},

    {XmNverticalScrollBar, XmCVerticalScrollBar, XmRWidget, sizeof(Widget),
     offset(matrix.vertical_sb), XmRImmediate, NULL},

    {XmNverticalScrollBarDisplayPolicy, XmCMatrixScrollBarDisplayPolicy,
     XmRMatrixScrollBarDisplayPolicy, sizeof(unsigned char),
     offset(matrix.vsb_display_policy), XmRImmediate,
     (XtPointer) XmDISPLAY_AS_NEEDED},

    {XmNvisibleColumns, XmCVisibleColumns, XmRDimension, sizeof(Dimension),
     offset(matrix.visible_columns), XmRImmediate, (XtPointer) 0},

    {XmNvisibleRows, XmCVisibleRows, XmRDimension, sizeof(Dimension),
     offset(matrix.visible_rows), XmRImmediate, (XtPointer) 0},

    {XmNwriteCellCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     offset(matrix.write_cell_callback), XmRCallback, NULL},

};

static XmSyntheticResource syn_resources[] =
{
    {XmNcellHighlightThickness, sizeof(Dimension),
     offset(matrix.cell_highlight_thickness), _XmFromHorizontalPixels,
     _XmToHorizontalPixels},

    {XmNcellMarginHeight, sizeof(Dimension), offset(matrix.cell_margin_height),
     _XmFromVerticalPixels, _XmToVerticalPixels},

    {XmNcellMarginWidth, sizeof(Dimension), offset(matrix.cell_margin_width),
     _XmFromHorizontalPixels, _XmToHorizontalPixels},

    {XmNcellShadowThickness, sizeof(Dimension),
     offset(matrix.cell_shadow_thickness), _XmFromHorizontalPixels,
     _XmToHorizontalPixels},

    {XmNspace, sizeof(Dimension), offset(matrix.space),
     _XmFromHorizontalPixels, _XmToHorizontalPixels},
};

#if XmVersion >= 1002
static XtIntervalId TraverseID = 0;
#endif

/*
 * Declaration of methods
 */
static void ClassInitialize P((void));
static void xbaeRegisterConverters P((void));
static void ClassPartInitialize P((XbaeMatrixWidgetClass));
static void Initialize P((XbaeMatrixWidget, XbaeMatrixWidget, ArgList,
                          Cardinal *));
static void Realize P((XbaeMatrixWidget, XtValueMask *,
                       XSetWindowAttributes *));
static void InsertChild P((Widget));
static void Redisplay P((Widget, XEvent *, Region));
static Boolean SetValues P((XbaeMatrixWidget, XbaeMatrixWidget,
                            XbaeMatrixWidget, ArgList, Cardinal *));
static void SetValuesAlmost P((XbaeMatrixWidget, XbaeMatrixWidget,
                               XtWidgetGeometry *, XtWidgetGeometry *));
static void Destroy P((XbaeMatrixWidget));
static XtGeometryResult GeometryManager P((Widget, XtWidgetGeometry *,
                                           XtWidgetGeometry *));
static XtGeometryResult QueryGeometry P((XbaeMatrixWidget, XtWidgetGeometry *,
                                         XtWidgetGeometry *));

/*
 * Redraw function for clip widget
 */
static void ClipRedisplay P((Widget, XEvent *, Region));

/*
 * Private functions unique to Matrix
 */
static void ResizeCells P((XbaeMatrixWidget, XbaeMatrixWidget));
static void ResizeSelectedCells P((XbaeMatrixWidget, XbaeMatrixWidget));
#if XmVersion >= 1002
static void ResizeHighlightedCells P((XbaeMatrixWidget, XbaeMatrixWidget));
#endif
static void ResizeColors P((XbaeMatrixWidget, XbaeMatrixWidget, Boolean));
static void TraverseIn P((XbaeMatrixWidget));

#if XmVersion >= 1002
static void TraverseInTimeOut P((XtPointer, XtIntervalId *));
#endif /* XmVersion >= 1002 */

/*
 * Clip widget focusCallback
 */
static void TraverseInCB P((Widget, XbaeMatrixWidget, XtPointer));

/*
 * Matrix actions
 */
static XtActionsRec actions[] =
{
    {"EditCell",        xbaeEditCellACT},
    {"CancelEdit",        xbaeCancelEditACT},
    {"DefaultAction",        xbaeDefaultActionACT},
    {"CommitEdit",        xbaeCommitEditACT},
    {"ResizeColumns",        xbaeResizeColumnsACT},
    {"SelectCell",        xbaeSelectCellACT},
    {"TraverseNext",        xbaeTraverseNextACT},
    {"TraversePrev",        xbaeTraversePrevACT},
    {"ProcessDrag",        xbaeProcessDragACT},
    {"HandleMotion",        xbaeHandleMotionACT},
    {"PageDown",        xbaePageDownACT},
    {"PageUp",                xbaePageUpACT}
};


static XmBaseClassExtRec BaseClassExtRec = {
    NULL,                                        /* next_extension        */
    NULLQUARK,                                        /* record_type                */
    XmBaseClassExtVersion,                        /* version                */
    sizeof(XmBaseClassExtRec),                        /* record_size                */
    NULL,                                        /* InitializePrehook        */
    NULL,                                        /* SetValuesPrehook        */
    NULL,                                        /* InitializePosthook        */
    NULL,                                        /* SetValuesPosthook        */
    NULL,                                        /* secondaryObjectClass        */
    NULL,                                        /* secondaryCreate        */
    NULL,                                        /* getSecRes data        */
    { 0 },                                        /* fastSubclass flags        */
    NULL,                                        /* get_values_prehook        */
    NULL,                                        /* get_values_posthook        */
    NULL,                                        /* classPartInitPrehook */
    NULL,                                        /* classPartInitPosthook*/
    NULL,                                        /* ext_resources        */
    NULL,                                        /* compiled_ext_resources*/
    0,                                                /* num_ext_resources        */
    FALSE,                                        /* use_sub_resources        */
    XmInheritWidgetNavigable,                        /* widgetNavigable        */
    XmInheritFocusChange,                        /* focusChange                */
};


externaldef(xbaematrixclassrec)
XbaeMatrixClassRec xbaeMatrixClassRec =
{
    {
        /* core_class fields        */
        (WidgetClass) & xmManagerClassRec,        /* superclass                */
        "XbaeMatrix",                                /* class_name                */
        sizeof(XbaeMatrixRec),                        /* widget_size                */
        ClassInitialize,                        /* class_initialize        */
        (XtWidgetClassProc)ClassPartInitialize,        /* class_part_initialize*/
        False,                                        /* class_inited                */
        (XtInitProc)Initialize,                        /* initialize                */
        NULL,                                        /* initialize_hook        */
        (XtRealizeProc)Realize,                        /* realize                */
        actions,                                /* actions                */
        XtNumber(actions),                        /* num_actions                */
        resources,                                /* resources                */
        XtNumber(resources),                        /* num_resources        */
        NULLQUARK,                                /* xrm_class                */
        True,                                        /* compress_motion        */
        XtExposeCompressMultiple |                /* compress_exposure        */
        XtExposeGraphicsExpose |
        XtExposeNoExpose,
        True,                                        /* compress_enterleave        */
        False,                                        /* visible_interest        */
        (XtWidgetProc)Destroy,                        /* destroy                */
        (XtWidgetProc)xbaeResize,                /* resize                */
        (XtExposeProc)Redisplay,                /* expose                */
        (XtSetValuesFunc)SetValues,                /* set_values                */
        NULL,                                        /* set_values_hook        */
        (XtAlmostProc)SetValuesAlmost,                /* set_values_almost        */
        NULL,                                        /* get_values_hook        */
        NULL,                                        /* accept_focus                */
        XtVersionDontCheck,                        /* version                */
        NULL,                                        /* callback_private        */
        defaultTranslations,                        /* tm_table                */
        (XtGeometryHandler)QueryGeometry,        /* query_geometry        */
        NULL,                                        /* display_accelerator        */
        (XtPointer) &BaseClassExtRec                /* extension                */
    },

    {
        /* composite_class fields */
        GeometryManager,                        /* geometry_manager        */
        NULL,                                        /* change_managed        */
        InsertChild,                                /* insert_child                */
        XtInheritDeleteChild,                        /* delete_child                */
        NULL,                                        /* extension                */
    },
    {
        /* constraint_class fields */
        NULL,                                        /* resources                */
        0,                                        /* num_resources        */
        0,                                        /* constraint_size        */
        NULL,                                        /* initialize                */
        NULL,                                        /* destroy                */
        NULL,                                        /* set_values                */
        NULL                                        /* extension                */
    },
    {
        /* manager_class fields */
        XtInheritTranslations,                        /* translations                */
        syn_resources,                                /* syn_resources        */
        XtNumber(syn_resources),                /* num_syn_resources        */
        NULL,                                    /* syn_constraint_resources        */
        0,                                    /* num_syn_constraint_resources */
        XmInheritParentProcess,                        /* parent_process        */
        NULL                                        /* extension                */
    },
    {
        /* matrix_class fields */
        xbaeSetCell,                                /* set_cell                */
        xbaeGetCell,                                /* get_cell                */
        xbaeEditCell,                                /* edit_cell                */
        xbaeSelectCell,                                /* select_cell                */
        xbaeSelectRow,                                /* select_row                */
        xbaeSelectColumn,                        /* select_column        */
        xbaeDeselectAll,                        /* deselect_all                */
        xbaeSelectAll,                                /* select_all                */
        xbaeDeselectCell,                        /* deselect_cell        */
        xbaeDeselectRow,                        /* deselect_row                */
        xbaeDeselectColumn,                        /* deselect_column        */
        xbaeCommitEdit,                                /* commit_edit                */
        xbaeCancelEdit,                                /* cancel_edit                */
        xbaeAddRows,                                /* add_rows                */
        xbaeDeleteRows,                                /* delete_rows                */
        xbaeAddColumns,                                /* add_columns                */
        xbaeDeleteColumns,                        /* delete_columns        */
        xbaeSetRowColors,                        /* set_row_colors        */
        xbaeSetColumnColors,                        /* set_column_colors        */
        xbaeSetCellColor,                        /* set_cell_color        */
        NULL,                                        /* extension                */
    }
};

externaldef(xbaematrixwidgetclass)
WidgetClass xbaeMatrixWidgetClass = (WidgetClass) & xbaeMatrixClassRec;

static XtConvertArgRec convertArg[] = {
    {XtWidgetBaseOffset, (XtPointer) XtOffsetOf(WidgetRec, core.screen),
     sizeof(Screen *)},
    {XtWidgetBaseOffset, (XtPointer) XtOffsetOf(WidgetRec, core.colormap),
     sizeof(Colormap)}
};

static void
xbaeRegisterConverters()
{
    /*
     * String to StringArray is used for XmNrowLabels and XmNcolumnLabels
     * We make a private copy of this table
     */
    XtSetTypeConverter(XmRString, XmRStringArray,
                       CvtStringToStringArray, 
             (XtConvertArgRec *)screenConvertArg, (Cardinal) 1,
                       XtCacheAll | XtCacheRefCount,
                       StringArrayDestructor);

    /*
     * String to String2DArray is used for XmNcells resource 
     * We make a private copy of this table
     */
    XtSetTypeConverter(XmRString, XmRCellTable,
                       CvtStringToCellTable, NULL, 0,
                       XtCacheNone,
                       StringCellDestructor);

    /*
     * String to ShortArray is used for XmNcolumnWidths resource.
     * We make a private copy of this table
     */
    XtSetTypeConverter(XmRString, XmRWidthArray,
                       CvtStringToWidthArray, NULL, 0,
                       XtCacheAll | XtCacheRefCount,
                       WidthArrayDestructor);
    
    /*
     * String to IntArray is used for XmNcolumnMaxLengths resource.
     * We make a private copy of this table
     */
    XtSetTypeConverter(XmRString, XmRMaxLengthArray,
                       CvtStringToMaxLengthArray, NULL, 0,
                       XtCacheAll | XtCacheRefCount,
                       MaxLengthArrayDestructor);

    /*
     * String to PixelTable is used for XmNcolors
     * and XmNcellBackgrounds resources.
     */
    XtSetTypeConverter(XmRString, XmRPixelTable,
                       CvtStringToPixelTable,convertArg, XtNumber(convertArg),
                       XtCacheNone,
                       PixelTableDestructor);
    /*
     * String to BooleanArray is used for XmNcolumnButtonLabels and
     * XmNrowButtonLabels resources.
     */
    XtSetTypeConverter(XmRString, XmRBooleanArray,
                       CvtStringToBooleanArray, NULL, 0,
                       XtCacheAll | XtCacheRefCount,
                       BooleanArrayDestructor);

    /*
     * String to AlignmentArray is used for XmNcolumnAlignments
     * and XmNcolumnLabelAlignments resources.
     */
    XtSetTypeConverter(XmRString, XmRAlignmentArray,
                       CvtStringToAlignmentArray, NULL, 0,
                       XtCacheAll | XtCacheRefCount,
                       AlignmentArrayDestructor);

    /*
     * String to grid type is used for XmNgridType
     */
    XtSetTypeConverter(XmRString, XmRGridType,
                       CvtStringToGridType, NULL, 0,
                       XtCacheAll, NULL);

    /*
     * String to matrix display policy is used for
     * XmN{vertical,horizontal}ScrollBarDisplayPolicy
     */
    XtSetTypeConverter(XmRString, XmRMatrixScrollBarDisplayPolicy,
#ifdef __VMS
                       CvtStringToMatrixScrollBarDisp,
#else
                       CvtStringToMatrixScrollBarDisplayPolicy,
#endif
                       NULL, 0, XtCacheAll, NULL);
}

static void
ClassInitialize()
{
    xbaeRegisterConverters();
}

static void
ClassPartInitialize(mwc)
XbaeMatrixWidgetClass mwc;
{
    register XbaeMatrixWidgetClass super =
        (XbaeMatrixWidgetClass) mwc->core_class.superclass;

    /*
     * Allow subclasses to inherit new Matrix methods
     */
    if (mwc->matrix_class.set_cell == XbaeInheritSetCell)
        mwc->matrix_class.set_cell = super->matrix_class.set_cell;
    if (mwc->matrix_class.get_cell == XbaeInheritGetCell)
        mwc->matrix_class.get_cell = super->matrix_class.get_cell;
    if (mwc->matrix_class.edit_cell == XbaeInheritEditCell)
        mwc->matrix_class.edit_cell = super->matrix_class.edit_cell;
    if (mwc->matrix_class.select_cell == XbaeInheritSelectCell)
        mwc->matrix_class.select_cell = super->matrix_class.select_cell;
    if (mwc->matrix_class.select_row == XbaeInheritSelectRow)
        mwc->matrix_class.select_row = super->matrix_class.select_row;
    if (mwc->matrix_class.select_column == XbaeInheritSelectColumn)
        mwc->matrix_class.select_column = super->matrix_class.select_column;
    if (mwc->matrix_class.deselect_all == XbaeInheritDeselectAll)
        mwc->matrix_class.deselect_all = super->matrix_class.deselect_all;
    if (mwc->matrix_class.select_all == XbaeInheritSelectAll)
        mwc->matrix_class.select_all = super->matrix_class.select_all;
    if (mwc->matrix_class.deselect_cell == XbaeInheritDeselectCell)
        mwc->matrix_class.deselect_cell = super->matrix_class.deselect_cell;
    if (mwc->matrix_class.deselect_row == XbaeInheritDeselectRow)
        mwc->matrix_class.deselect_row = super->matrix_class.deselect_row;
    if (mwc->matrix_class.deselect_column == XbaeInheritDeselectColumn)
        mwc->matrix_class.deselect_column =
            super->matrix_class.deselect_column;
    if (mwc->matrix_class.commit_edit == XbaeInheritCommitEdit)
        mwc->matrix_class.commit_edit = super->matrix_class.commit_edit;
    if (mwc->matrix_class.cancel_edit == XbaeInheritCancelEdit)
        mwc->matrix_class.cancel_edit = super->matrix_class.cancel_edit;
    if (mwc->matrix_class.add_rows == XbaeInheritAddRows)
        mwc->matrix_class.add_rows = super->matrix_class.add_rows;
    if (mwc->matrix_class.delete_rows == XbaeInheritDeleteRows)
        mwc->matrix_class.delete_rows = super->matrix_class.delete_rows;
    if (mwc->matrix_class.add_columns == XbaeInheritAddColumns)
        mwc->matrix_class.add_columns = super->matrix_class.add_columns;
    if (mwc->matrix_class.delete_columns == XbaeInheritDeleteColumns)
        mwc->matrix_class.delete_columns = super->matrix_class.delete_columns;
    if (mwc->matrix_class.set_row_colors == XbaeInheritSetRowColors)
        mwc->matrix_class.set_row_colors = super->matrix_class.set_row_colors;
    if (mwc->matrix_class.set_column_colors == XbaeInheritSetColumnColors)
        mwc->matrix_class.set_column_colors =
            super->matrix_class.set_column_colors;
    if (mwc->matrix_class.set_cell_color == XbaeInheritSetCellColor)
        mwc->matrix_class.set_cell_color = super->matrix_class.set_cell_color;
}


#ifdef NEED_24BIT_VISUAL
static Widget
get_shell_ancestor(w)
Widget w;
{
    Widget sh;

    for (sh=w ; !XtIsShell(sh) ; sh=XtParent(sh))
        ;
}
#endif

/*
 * Callbacks for our scrollbars.
 */
static XtCallbackRec VSCallback[] =
{
    {(XtCallbackProc) xbaeScrollVertCB, (XtPointer) NULL},
    {(XtCallbackProc) NULL, NULL}
};
static XtCallbackRec HSCallback[] =
{
    {(XtCallbackProc) xbaeScrollHorizCB, (XtPointer) NULL},
    {(XtCallbackProc) NULL, NULL}
};

/* ARGSUSED */
static void
Initialize(request, new, args, num_args)
XbaeMatrixWidget request, new;
ArgList args;
Cardinal *num_args;
{
    Dimension marginHeight;

    /*
     * Initialize redisplay counters
     */
    new->matrix.disable_redisplay = 0;
    new->matrix.first_row_offset = 0;

#if XmVersion >= 1002
    /*
     * Initialize highlight location
     */
    new->matrix.highlight_location = HighlightNone;
#endif
    
    /*
     * Check rows/cols set by resources for consistency/validity
     */
    if (new->matrix.rows < 0 || new->matrix.columns < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "initialize", "badSize", "XbaeMatrix",
            "XbaeMatrix: Number of rows or columns is less than zero",
            (String *) NULL, (Cardinal *) NULL);
        if (new->matrix.rows < 0)
            new->matrix.rows = 0;
        if (new->matrix.columns < 0)
            new->matrix.columns = 0;
    }

    /*
     * Make sure column_widths were specified.  If not, use a default value.
     * This should keep the XDesigner users happy....
     */
    if (new->matrix.columns && (new->matrix.column_widths == NULL))
    {
        int i;

        new->matrix.column_widths = (short*)XtMalloc(new->matrix.columns *
                                                     sizeof(short));
        for (i = 0; i < new->matrix.columns; i++)
            new->matrix.column_widths[i] = 5;
            
    }

    /* If no label font is specified, copy the default fontList, but if
       we do then override the bold_labels resource */
    if (!new->matrix.label_font_list)
        new->matrix.label_font_list = XmFontListCopy(new->matrix.font_list);
    else
        new->matrix.bold_labels = False;
    
    /*
     * We must have at least one non-fixed row/column. Only complain if there
     * are some to complain about, though. We may have none at all (yet).
     */
    if ((int)(new->matrix.fixed_rows +
              new->matrix.trailing_fixed_rows) > 0 &&
               (int)(new->matrix.fixed_rows +
              new->matrix.trailing_fixed_rows) >= new->matrix.rows)
    {
        XtAppWarningMsg(XtWidgetToApplicationContext((Widget) new),
                         "initialize", "tooManyFixed", "XbaeMatrix",
                         "XbaeMatrix: At least one row must not be fixed",
                         NULL, 0);
        new->matrix.fixed_rows = 0;
        new->matrix.trailing_fixed_rows = 0;
    }
    if ((int)(new->matrix.fixed_columns +
              new->matrix.trailing_fixed_columns) > 0 &&
        (int)(new->matrix.fixed_columns +
              new->matrix.trailing_fixed_columns) >= new->matrix.columns)
    {
        XtAppWarningMsg(XtWidgetToApplicationContext((Widget) new),
                         "initialize", "tooManyFixed", "XbaeMatrix",
                         "XbaeMatrix: At least one column must not be fixed",
                         NULL, 0);
        new->matrix.fixed_columns = 0;
        new->matrix.trailing_fixed_columns = 0;
    }
    /*
     * We can't have too many visible columns
     */
    if (new->matrix.columns && ((int)new->matrix.visible_columns >
        (new->matrix.columns - (int)new->matrix.fixed_columns -
        (int)new->matrix.trailing_fixed_columns)))
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "initialize", "tooManyVisibleColumns", "XbaeMatrix",
            "XbaeMatrix: visibleColumns must not be greater than\n            (columns - fixedColumns - trailingFixedColumns)",
            (String *) NULL, (Cardinal *) NULL);
        new->matrix.visible_columns = 0;
    }

    if (new->matrix.grid_type >= XmGRID_LINE)
        /* Deprecated types. To be removed in next version. */
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "cvtStringToGridType", "deprecatedType",
            "XbaeMatrix",
            "Value for GridType is deprecated and will be removed in next release",
            NULL, NULL);

    /*
     * Copy the pointed to resources.
     * If cells is NULL, we create an array of "" strings.
     */
    if (new->matrix.cells)
        xbaeCopyCells(new);
    if (new->matrix.row_labels)
        xbaeCopyRowLabels(new);
    if (new->matrix.column_labels)
        xbaeCopyColumnLabels(new);
    else
    {
        new->matrix.column_label_lines = NULL;
        new->matrix.column_label_maxlines = 0;
    }

    xbaeCopyColumnWidths(new);

    if (new->matrix.cell_shadow_types)
        xbaeCopyCellShadowTypes(new);
    if (new->matrix.row_shadow_types)
        xbaeCopyRowShadowTypes(new);
    if (new->matrix.column_shadow_types)
        xbaeCopyColumnShadowTypes(new);
    
    if (new->matrix.cell_user_data)
        xbaeCopyCellUserData(new);
    if (new->matrix.row_user_data)
        xbaeCopyRowUserData(new);
    if (new->matrix.column_user_data)
        xbaeCopyColumnUserData(new);

#if CELL_WIDGETS
    if (new->matrix.cell_widgets)
        xbaeCopyCellWidgets(new);
#endif
    
    if (new->matrix.column_max_lengths)
        xbaeCopyColumnMaxLengths(new);

    if (new->matrix.column_alignments)
        xbaeCopyColumnAlignments(new);

    if (new->matrix.column_button_labels)
        xbaeCopyColumnButtonLabels(new);

    if (new->matrix.column_label_alignments)
        xbaeCopyColumnLabelAlignments(new);

    if (new->matrix.colors)
        xbaeCopyColors(new);

    if (new->matrix.cell_background)
          xbaeCopyBackgrounds(new);

    if (new->matrix.selected_cells)
        xbaeCopySelectedCells(new);

#if XmVersion >= 1002
    if (new->matrix.highlighted_cells)
        xbaeCopyHighlightedCells(new);
#endif
    
    if (new->matrix.row_button_labels)
        xbaeCopyRowButtonLabels(new);

    /*
     * If user didn't specify a rowLabelWidth, then calculate one based on
     * the widest label
     */
    if (new->matrix.row_label_width == 0 && new->matrix.row_labels)
        new->matrix.row_label_width = xbaeMaxRowLabel(new);

    /*
     * Copy the fontList. Get fontStruct from fontList.
     */
    xbaeNewFont(new);
    xbaeNewLabelFont(new);

    /*
     * Create our 4 children (SBs and textField are unmanaged for now)
     * they must be created in this order so our macros work
     * (horiz scroll, vert scroll and then clip and textField).
     * We scroll horizontally by pixels, vertically by rows.
     */
    new->matrix.horizontal_sb = XtVaCreateWidget(
        "horizScroll", xmScrollBarWidgetClass, (Widget) new,
        XmNorientation, XmHORIZONTAL,
        XmNdragCallback, HSCallback,
        XmNvalueChangedCallback, HSCallback,
        XmNincrement, FONT_WIDTH(new),
        XmNsliderSize, 1,
        XmNminimum, 0,
        XmNmaximum, 1,
        XmNbackground, new->core.background_pixel,
        XmNforeground, new->manager.foreground,
        XmNbottomShadowColor, new->manager.bottom_shadow_color,
        XmNbottomShadowPixmap, new->manager.bottom_shadow_pixmap,
        XmNhighlightColor, new->manager.highlight_color,
        XmNhighlightPixmap, new->manager.highlight_pixmap,
        XmNtopShadowColor, new->manager.top_shadow_color,
        XmNtopShadowPixmap, new->manager.top_shadow_pixmap,
        NULL);

    HORIZ_ORIGIN(new) = 0;

    new->matrix.vertical_sb = XtVaCreateWidget(
        "vertScroll", xmScrollBarWidgetClass, (Widget) new,
        XmNorientation, XmVERTICAL,
        XmNdragCallback, VSCallback,
        XmNvalueChangedCallback, VSCallback,
        XmNincrement, 1,
        XmNminimum, 0,
        XmNmaximum, new->matrix.rows ?
                (new->matrix.rows - (int) new->matrix.fixed_rows -
                (int) new->matrix.trailing_fixed_rows) : 1,
        XmNsliderSize, 1,
        XmNbackground, new->core.background_pixel,
        XmNforeground, new->manager.foreground,
        XmNbottomShadowColor, new->manager.bottom_shadow_color,
        XmNbottomShadowPixmap, new->manager.bottom_shadow_pixmap,
        XmNhighlightColor, new->manager.highlight_color,
        XmNhighlightPixmap, new->manager.highlight_pixmap,
        XmNtopShadowColor, new->manager.top_shadow_color,
        XmNtopShadowPixmap, new->manager.top_shadow_pixmap,
        NULL);
    
    /*
     * Create the Clip widget managed so we can use it for traversal
     */
    new->matrix.clip_window = XtVaCreateManagedWidget(
        "clip", xbaeClipWidgetClass, (Widget) new,
        XmNexposeProc, ClipRedisplay,
        XmNtraversalOn, new->manager.traversal_on,
        XmNbackground, new->core.background_pixel,
        NULL);

    /*
     * Add a callback to the Clip widget so we know when it gets the focus
     * and can use it in traversal.
     */
    XtAddCallback(ClipChild(new), XmNfocusCallback,
                  (XtCallbackProc)TraverseInCB, (XtPointer) new);

    /*
     * Calculate an imaginary cellMarginHeight based on the largest of
     * the label and cell font.  If the font for the labels is bigger,
     * the cellMarginHeight needs to be increased to allow the cell font
     * to still appear centrally placed
     */
    if (LABEL_HEIGHT(new) > FONT_HEIGHT(new))
        marginHeight = (int)(LABEL_HEIGHT(new) +
                             (new->matrix.cell_margin_height *
                              2) - FONT_HEIGHT(new)) / 2;
    else 
        marginHeight = new->matrix.cell_margin_height;
   
    /*
     * Create text field (unmanaged for now) - its window will be reparented
     * in Realize to be a subwindow of Clip
     *
     * Set the shadow thickness to 0 to prevent a shadow drawn inside the
     * highlight region.  The shadow thickness is used to draw the shadows
     * *around* the highlight
     */

    new->matrix.text_field = XtVaCreateWidget(
        "textField", xbaeInputWidgetClass, (Widget) new,
        XmNmarginWidth, new->matrix.cell_margin_width,
        XmNmarginHeight, marginHeight,
        XmNtranslations, new->matrix.text_translations,
        XmNfontList, new->matrix.font_list,
        XmNshadowThickness, new->matrix.text_shadow_thickness,
        XmNbackground, new->matrix.text_background,
        XmNforeground, new->manager.foreground,
        XmNbottomShadowColor, new->manager.bottom_shadow_color,
        XmNbottomShadowPixmap, new->manager.bottom_shadow_pixmap,
        XmNhighlightThickness, new->matrix.cell_highlight_thickness,
        XmNhighlightColor, new->manager.highlight_color,
        XmNhighlightPixmap, new->manager.highlight_pixmap,
        XmNeditMode, XmSINGLE_LINE_EDIT,
        NULL);
    
    XtAddCallback(TextChild(new), XmNmodifyVerifyCallback, xbaeModifyVerifyCB,
                  (XtPointer) new);

    /* Add a handler on top of the text field to handle clicks on it */
    XtAddEventHandler(TextChild(new), ButtonPressMask | ButtonReleaseMask,
                      True, (XtEventHandler)xbaeHandleClick, (XtPointer)new);
    XtAddEventHandler((Widget)new, ButtonPressMask | ButtonReleaseMask,
                      True, (XtEventHandler)xbaeHandleClick, (XtPointer)new);

    /*
     * Compute cell text baseline based on TextField widget
     */
    new->matrix.text_baseline = XmTextGetBaseline(TextChild(new)) +
        new->matrix.cell_shadow_thickness;

    /*
     * Adjust the label_baseline according to the larger of the two fonts
     */
    if (LABEL_HEIGHT(new) == FONT_HEIGHT(new))
        new->matrix.label_baseline = new->matrix.text_baseline;
    else
    {
        if (LABEL_HEIGHT(new) < FONT_HEIGHT(new))
            marginHeight = (int)(FONT_HEIGHT(new) +
                                 new->matrix.text_shadow_thickness * 2 +
                                 new->matrix.cell_margin_height * 2 -
                                 LABEL_HEIGHT(new)) / 2;
        else 
            marginHeight = new->matrix.cell_margin_height +
                new->matrix.text_shadow_thickness;

        new->matrix.label_baseline = marginHeight +
            new->matrix.cell_shadow_thickness -
            new->matrix.label_font_y;
    }
    /*
     * Calculate total pixel width of cell area
     */
    xbaeGetCellTotalWidth(new);

    /*
     * Make the clips for the fixed cells which are scrollable (i.e. the
     * fixed rows that can scroll horizontally, and the fixed columns which
     * can scroll vertically. This makes 4 scrollable fixed-cell areas.
     */
    new->matrix.left_clip = XtVaCreateWidget(
        "leftclip", xbaeClipWidgetClass, (Widget) new,
        XmNexposeProc, Redisplay,
        XmNtraversalOn, new->manager.traversal_on,
        XmNbackground, new->core.background_pixel,
        NULL);
    new->matrix.right_clip = XtVaCreateWidget(
        "rightclip", xbaeClipWidgetClass, (Widget) new,
        XmNexposeProc, Redisplay,
        XmNtraversalOn, new->manager.traversal_on,
        XmNbackground, new->core.background_pixel,
        NULL);
    new->matrix.top_clip = XtVaCreateWidget(
        "topclip", xbaeClipWidgetClass, (Widget) new,
        XmNexposeProc, Redisplay,
        XmNtraversalOn, new->manager.traversal_on,
        XmNbackground, new->core.background_pixel,
        NULL);
    new->matrix.bottom_clip = XtVaCreateWidget(
        "bottomclip", xbaeClipWidgetClass, (Widget) new,
        XmNexposeProc, Redisplay,
        XmNtraversalOn, new->manager.traversal_on,
        XmNbackground, new->core.background_pixel,
        NULL);
    /*
     * Cache the pixel position of each column
     */
    new->matrix.column_positions = CreateColumnPositions(new);
    xbaeGetColumnPositions(new);

    /*
     * Now we can set the VSB maximum (relies on data from
     * GetCellTotalWidth above)
     */
    XtVaSetValues(HorizScrollChild(new),
                  XmNmaximum, NON_FIXED_TOTAL_WIDTH(new) ?
                  NON_FIXED_TOTAL_WIDTH(new) : 1,
                  NULL);

    /*
     * Current position starts at the top left editable cell.
     */
    new->matrix.current_row = new->matrix.fixed_rows;
    new->matrix.current_column = new->matrix.fixed_columns;

    new->matrix.current_clip = CLIP_NONE;
    
    /*
     * We aren't trying to traverse out
     */
    new->matrix.traversing = NOT_TRAVERSING;

#ifndef NEED_24BIT_VISUAL
    /*
     * Get/create our GCs
     */
    xbaeCreateDrawGC(new);
    xbaeCreatePixmapGC(new);
    xbaeCreateLabelGC(new);
    xbaeCreateLabelClipGC(new);
    xbaeCreateGridLineGC(new);
    xbaeCreateTopShadowClipGC(new);
    xbaeCreateBottomShadowClipGC(new);
#endif

    /*
     * Now we have created our GCs, check if the widget is sensitive
     */
    if (!new->core.sensitive)
    {
        XGCValues values;
        unsigned long valuemask = GCFillStyle;
        Display *dpy = XtDisplay(new);
        
        if (!new->core.sensitive)
        {
            int i;

            values.fill_style = FillStippled;

            /*
             * Change our drawing GC's to the stipple effect to indicate
             * the widget is insensitive and redraw
             */
            XChangeGC(dpy, new->matrix.draw_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.label_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.label_clip_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.pixmap_gc, valuemask, &values);
            /*
             * Propogate the insensitive feel to our children
             */
            for (i = 0; i < XbaeNumChildren; i++)
                XtSetSensitive(new->composite.children[i], False);
        }
    }

    /*
     * Create ScrollMgrs to manage scrolling events
     */
    new->matrix.matrix_scroll_mgr = xbaeSmCreateScrollMgr();
    new->matrix.clip_scroll_mgr = xbaeSmCreateScrollMgr();

    /*
     * Set the last row and column to an impossible value
     */
    new->matrix.last_row = -1;
    new->matrix.last_column = -1;
    new->matrix.last_click_time = (Time)0;

    /*
     * Compute our size.  If either dimension was explicitly set to 0,
     * then that dimension is computed.
     * Use request because superclasses modify width/height.
     */
    if (request->core.width == 0 || request->core.height == 0)
        xbaeComputeSize(new, request->core.width == 0,
                        request->core.height == 0);

    /*
     * Make sure top_row is sensible before we call Resize
     */
    if (VERT_ORIGIN(new) < 0)
        VERT_ORIGIN(new) = 0;
    else if (VERT_ORIGIN(new) > new->matrix.rows)
        VERT_ORIGIN(new) = new->matrix.rows;

    /*
     * Tweak top_row to make sure it is valid before calling Resize
     */
    if (VERT_ORIGIN(new))
        xbaeAdjustTopRow(new);

    if (new->matrix.left_column)
        xbaeAdjustLeftColumn(new);

    /*
     * Layout the scrollbars and clip widget based on our size
     */
    xbaeResize(new);
}

static void
Realize(mw, valueMask, attributes)
XbaeMatrixWidget mw;
XtValueMask *valueMask;
XSetWindowAttributes *attributes;
{
    *valueMask |= CWDontPropagate;
    attributes->do_not_propagate_mask =
        ButtonPressMask | ButtonReleaseMask |
        KeyPressMask | KeyReleaseMask | PointerMotionMask;

    /*
     * Don't call our superclasses realize method, because Manager sets
     * bit_gravity
     */
    XtCreateWindow((Widget) mw, InputOutput, CopyFromParent,
                   *valueMask, attributes);

#ifdef NEED_24BIT_VISUAL
    /*
     * Now that we have a window...
     * Get/create our GCs
     */
    xbaeCreateDrawGC(mw);
    xbaeCreatePixmapGC(mw);
    xbaeCreateLabelGC(mw);
    xbaeCreateLabelClipGC(mw);
    xbaeCreateTopShadowClipGC(mw);
    xbaeCreateBottomShadowClipGC(mw);
#endif

    /*
     * Reparent the textFields window to be a subwindow of Clip widget
     * (we need to realize them first)
     */
    XtRealizeWidget(TextChild(mw));
    XtRealizeWidget(ClipChild(mw));
    XtRealizeWidget(LeftClip(mw));
    XtRealizeWidget(RightClip(mw));
    XtRealizeWidget(TopClip(mw));
    XtRealizeWidget(BottomClip(mw));
    XReparentWindow(XtDisplay(mw), XtWindow(TextChild(mw)),
                    XtWindow(ClipChild(mw)),
                    TextChild(mw)->core.x, TextChild(mw)->core.y);
    mw->matrix.current_parent = ClipChild(mw);
    /*
     * Set the clip_mask in our clipping GCs.
     */
    xbaeSetClipMask(mw, CLIP_NONE);
}

static void
InsertChild(w)
Widget w;
{
#if 0
    if (((CompositeWidget) XtParent(w))->composite.num_children > 7)
    {
#if XmVersion > 1001
        /* Hah! Cannot use XmIsDragIconObjectClass because it is defined
         * using XtIsSubclass() (at least in the 1.2 and 2.0 headers I have
         * available to me). Nothing like a little QA, huh.
         *
         * Anyways, since the drag icon & context are created as widget
         * children of the matrix, we need to allow just those extra types
         * to be created as our kids.
         */
        if ((!XmIsDragContext(w)) && (!XtIsSubclass(w, xmDragIconObjectClass)))
        {
#endif
            String params[1];
            Cardinal num_params = 1;

            params[0] = XtClass(XtParent(w))->core_class.class_name;
            XtAppWarningMsg(
                XtWidgetToApplicationContext(w),
                "insertChild", "badChild", "XbaeMatrix",
                "XbaeMatrix: Applications cannot add children to %s widgets",
                params, &num_params);
#if XmVersion > 1001
        }
#endif
        return;
    }
#endif
#if XmVersion > 1001
    (*((XmManagerWidgetClass)
       (xbaeMatrixWidgetClass->core_class.superclass))->composite_class.insert_child) (w);
#endif
}

/*
 * This is the expose method for the Matrix widget.
 * It redraws the row and column labels, the cells in fixed rows and columns
 * and the clip window shadow.
 */
/* ARGSUSED */
static void
Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
    Rectangle expose;
    XbaeMatrixWidget mw;

    if (XtIsSubclass(w, xbaeMatrixWidgetClass))
        mw = (XbaeMatrixWidget) w;
    else        /* must be one of the clips */
    {
        int *x = NULL, *y = NULL;

        mw = (XbaeMatrixWidget) XtParent(w);
        /* point at the correct x,y pair */
        switch (event->type)
        {
            case Expose:
                x = &event->xexpose.x;
                y = &event->xexpose.y;
                break;

            case GraphicsExpose:
                x = &event->xgraphicsexpose.x;
                y = &event->xgraphicsexpose.y;
                break;

            case NoExpose:
                break;

            default:
                return;
        }

        /* NoExpose event can't be translated so make sure x is set */
        if (x)
        {
            /*
             * Translate expose event coords into matrix coords if it occured
             * on a clip. xbaeRedrawLabelsAndFixed needs to check intersections
             * in matrix coords, even though the xbaeDraw* routines will
             * correctly retranslate these onto the correct windows for drawing.
             */
            if (w == LeftClip(mw))
            {
                *x += ROW_LABEL_WIDTH(mw) + mw->manager.shadow_thickness;
                *y += FIXED_ROW_LABEL_OFFSET(mw);
            }
            else if (w == RightClip(mw))
            {
                *x += TRAILING_FIXED_COLUMN_LABEL_OFFSET(mw);
                *y += FIXED_ROW_LABEL_OFFSET(mw);
            }
            else if (w == TopClip(mw))
            {
                *x += FIXED_COLUMN_LABEL_OFFSET(mw);
                *y += (COLUMN_LABEL_HEIGHT(mw) + mw->manager.shadow_thickness);
            }
            else if (w == BottomClip(mw))
            {
                *x += FIXED_COLUMN_LABEL_OFFSET(mw);
                *y += TRAILING_FIXED_ROW_LABEL_OFFSET(mw);
            }
            else
                return;        /* eek! Run away quickly */
        }
    }

    if (mw->matrix.disable_redisplay)
        return;

    /*
     * Send our events to the mw ScrollMgr to be adjusted.
     */
    switch (event->type)
    {

    case Expose:
        /*
         * The Expose event will be translated into our scrolled
         * coordinate system.  Then it is put in a Rectangle.
         */
        xbaeSmScrollEvent(mw->matrix.matrix_scroll_mgr, event);
        SETRECT(expose,
                event->xexpose.x, event->xexpose.y,
                event->xexpose.x + event->xexpose.width,
                event->xexpose.y + event->xexpose.height);
        break;

    case GraphicsExpose:
        /*
         * The GraphicsExpose event will cause a scroll to be removed
         * from the managers queue, then the event will be translated
         * into our scrolled coordinate system.         Then it is put in a Rectangle.
         */
        xbaeSmScrollEvent(mw->matrix.matrix_scroll_mgr, event);
        SETRECT(expose,
                event->xgraphicsexpose.x, event->xgraphicsexpose.y,
                event->xgraphicsexpose.x + event->xgraphicsexpose.width,
                event->xgraphicsexpose.y + event->xgraphicsexpose.height);
        break;

    case NoExpose:
        /*
         * The NoExpose event means we won't be getting any GraphicsExpose
         * events, so the scroll will be removed from the queue and
         * we are done.
         */
        xbaeSmScrollEvent(mw->matrix.matrix_scroll_mgr, event);
        return;

    default:        /* maybe handled above already */
        return;
    }

    /*
     * Redraw the row/column labels and fixed rows/columns which are
     * overlapped by the expose Rectangle.
     */
    if ((! mw->matrix.trailing_fixed_columns) &&
         IN_GRID_ROW_MODE(mw) && NEED_HORIZ_FILL(mw))
    {
        Rectangle nonfixed;

        /*
         * We need to ensure that the last column of cells gets
         * redrawn so that the fill is properly redrawn.
         */
        SETRECT(nonfixed,
                COLUMN_POSITION(mw, mw->matrix.columns-1), 0,
                COLUMN_POSITION(mw, mw->matrix.columns-1)+1,
                ClipChild(mw)->core.height-1);

        xbaeRedrawCells(mw, &nonfixed);
    }
    else if ((! mw->matrix.trailing_fixed_rows) &&
              IN_GRID_COLUMN_MODE(mw) && NEED_VERT_FILL(mw))
    {
        Rectangle nonfixed;

        /*
         * We need to ensure that the last row of cells gets
         * redrawn so that the fill is properly redrawn.
         */
        SETRECT(nonfixed,
                0, ROW_HEIGHT(mw) * (mw->matrix.rows - 1),
                ClipChild(mw)->core.width-1,
                ROW_HEIGHT(mw) * mw->matrix.rows);

        xbaeRedrawCells(mw, &nonfixed);
    }

    xbaeRedrawLabelsAndFixed(mw, &expose);
}

/*
 * This is the exposeProc function for the Clip widget.
 * It handles expose events for the Clip widget by redrawing those
 * non-fixed cells which were damaged.
 * It receives Expose, GraphicsExpose and NoExpose events.
 */
/* ARGSUSED */
static void
ClipRedisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
    XbaeMatrixWidget mw = (XbaeMatrixWidget) XtParent(w);
    Rectangle expose, clip, intersect;

    if (mw->matrix.disable_redisplay)
        return;

    /*
     * Send our events to the clip ScrollMgr to be adjusted.
     */
    switch (event->type)
    {

    case Expose:
        /*
         * The Expose event will be translated into our scrolled
         * coordinate system.  Then it is put in a Rectangle.
         */
        xbaeSmScrollEvent(mw->matrix.clip_scroll_mgr, event);
        SETRECT(expose,
                event->xexpose.x, event->xexpose.y,
                event->xexpose.x + event->xexpose.width - 1,
                event->xexpose.y + event->xexpose.height - 1);
        break;

    case GraphicsExpose:
        /*
         * The GraphicsExpose event will cause a scroll to be removed
         * from the managers queue, then the event will be translated
         * into our scrolled coordinate system.         Then it is put in a Rectangle.
         */
        xbaeSmScrollEvent(mw->matrix.clip_scroll_mgr, event);
        SETRECT(expose,
                event->xgraphicsexpose.x, event->xgraphicsexpose.y,
                event->xgraphicsexpose.x + event->xgraphicsexpose.width - 1,
                event->xgraphicsexpose.y + event->xgraphicsexpose.height - 1);
        break;

    case NoExpose:
        /*
         * The NoExpose event means we won't be getting any GraphicsExpose
         * events, so the scroll well be removed from the queue and
         * we are done.
         */
        xbaeSmScrollEvent(mw->matrix.clip_scroll_mgr, event);
        return;

    default:
        return;
    }

    /*
     * We may get an expose event larger than the size of the Clip widget.
     * This is because in set_values we may clear the Clip widget
     * before it gets resized smaller (maybe in set_values_almost).
     * So here we intersect the expose event with the clip widget
     * to ensure the expose Rectangle is not larger than the Clip widget.
     */
    SETRECT(clip,
            0, 0,
            w->core.width - 1, w->core.height - 1);
    INTERSECT(clip, expose, intersect);

    /*
     * Redraw those cells which overlap the intersect Rectangle.
     */
    xbaeRedrawCells(mw, &intersect);
}


/* ARGSUSED */
static Boolean
SetValues(current, request, new, args, num_args)
XbaeMatrixWidget current, request, new;
ArgList args;
Cardinal *num_args;
{
    Boolean redisplay = False;        /* need to redraw */
    Boolean relayout = False;        /* need to layout, but same size */
    Boolean new_column_widths = False;        /* column widths changed */
    Boolean new_cells = False;        /* cells changed */
    Boolean do_top_row = False;        /* reset top_row */
    Boolean do_left_column = False;     /* reset left_column */
    int n;
    Arg wargs[9];

#define NE(field)        (current->field != new->field)
#define EQ(field)        (current->field == new->field)

    /*
     * We cannot re-set either of the scrollbars, the textField or
     * clip window.
     */
    if (NE(matrix.vertical_sb) || NE(matrix.horizontal_sb) ||
        NE(matrix.clip_window) || NE(matrix.text_field))
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "setValues", "set matrix children", "XbaeMatrix",
            "XbaeMatrix: Cannot set matrix widget children",
            NULL, 0);
        new->matrix.vertical_sb = current->matrix.vertical_sb;
        new->matrix.horizontal_sb = current->matrix.horizontal_sb;
        new->matrix.clip_window = current->matrix.clip_window;
        new->matrix.text_field = current->matrix.text_field;
    }

    /*
     * If rows changed, then:
     *        row_labels must change or be NULL
     *        row_button_labels must change or be NULL
     */
    if ((NE(matrix.rows) && (new->matrix.row_labels &&
                             EQ(matrix.row_labels))) ||
        (new->matrix.row_button_labels &&
         EQ(matrix.row_button_labels)))
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "setValues", "rows", "XbaeMatrix",
            "XbaeMatrix: Number of rows changed but dependent resources did not",
            NULL, 0);
        new->matrix.rows = current->matrix.rows;
        new->matrix.row_labels = current->matrix.row_labels;
        new->matrix.row_button_labels = current->matrix.row_button_labels;
    }

    /*
     * If columns changed, then:
     *        column_widths must change
     *        column_max_lengths must change or be NULL
     *        column_labels must change or be NULL
     *        column_alignments must change or be NULL
     *        column_button_labels must change or be NULL
     *        column_label_alignments must change or be NULL
     */
    if (NE(matrix.columns) &&
        ((new->matrix.column_labels && EQ(matrix.column_labels)) ||
         (new->matrix.column_max_lengths && EQ(matrix.column_max_lengths)) ||
         (new->matrix.column_alignments && EQ(matrix.column_alignments)) ||
         (new->matrix.column_button_labels &&
          EQ(matrix.column_button_labels)) ||
         (new->matrix.column_label_alignments &&
          EQ(matrix.column_label_alignments)) ||
         EQ(matrix.column_widths)))
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "setValues", "columns", "XbaeMatrix",
            "XbaeMatrix: Number of columns changed but dependent resources did not",
            NULL, 0);
        new->matrix.columns = current->matrix.columns;
        new->matrix.column_widths = current->matrix.column_widths;
        new->matrix.column_max_lengths = current->matrix.column_max_lengths;
        new->matrix.column_labels = current->matrix.column_labels;
        new->matrix.column_alignments = current->matrix.column_alignments;
        new->matrix.column_button_labels =
            current->matrix.column_button_labels;
        new->matrix.column_label_alignments =
            current->matrix.column_label_alignments;
    }

    /*
     * Make sure we have at least one row/column.
     */
    if (new->matrix.columns < 0 || new->matrix.rows < 0)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "setValues", "size", "XbaeMatrix",
            "XbaeMatrix: Must have at least one row and column",
            NULL, 0);
        if (new->matrix.columns < 0)
            new->matrix.columns = current->matrix.columns;
        if (new->matrix.rows < 0)
            new->matrix.rows = current->matrix.rows;
    }

    
    /*
     * We must have at least one non-fixed row/column.
     * This could be caused by (trailing) fixed_rows/columns or
     * rows/columns changing.
     */
    if ((int)(new->matrix.fixed_rows + new->matrix.trailing_fixed_rows) > 0 &&
        (int)(new->matrix.fixed_rows + new->matrix.trailing_fixed_rows) >=
        new->matrix.rows)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "setValues", "tooManyFixed", "XbaeMatrix",
            "XbaeMatrix: At least one row must not be fixed",
            NULL, 0);

        if (NE(matrix.fixed_rows))
            new->matrix.fixed_rows = current->matrix.fixed_rows;
        if (NE(matrix.trailing_fixed_rows))
            new->matrix.trailing_fixed_rows =
                current->matrix.trailing_fixed_rows;
        if (NE(matrix.rows))
            new->matrix.rows = current->matrix.rows;
    }
    if ((int)(new->matrix.fixed_columns +
              new->matrix.trailing_fixed_columns) > 0 &&
        (int)(new->matrix.fixed_columns +
              new->matrix.trailing_fixed_columns) >= new->matrix.columns)
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "setValues", "tooManyFixed", "XbaeMatrix",
            "XbaeMatrix: At least one column must not be fixed",
            NULL, 0);

        if (NE(matrix.fixed_columns))
            new->matrix.fixed_columns = current->matrix.fixed_columns;
        if (NE(matrix.trailing_fixed_columns))
            new->matrix.trailing_fixed_columns =
                current->matrix.trailing_fixed_columns;
        if (NE(matrix.columns))
            new->matrix.columns = current->matrix.columns;
    }

    /*
     * We can't have too many visible columns.
     * This could be caused by visible_columns or columns or fixed_columns
     * changing.
     */
    if (new->matrix.columns && ((int)new->matrix.visible_columns >
        (new->matrix.columns - (int)new->matrix.fixed_columns -
         (int)new->matrix.trailing_fixed_columns)))
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "setValues", "tooManyVisibleColumns", "XbaeMatrix",
            "XbaeMatrix: visibleColumns must not be greater than\n            (columns - fixedColumns - trailingFixedColumns)",
            (String *) NULL, (Cardinal *) NULL);
        if (NE(matrix.visible_columns))
            new->matrix.visible_columns = current->matrix.visible_columns;
        if (NE(matrix.columns))
            new->matrix.columns = current->matrix.columns;
        if (NE(matrix.fixed_columns))
            new->matrix.fixed_columns = current->matrix.fixed_columns;
        if (NE(matrix.trailing_fixed_columns))
            new->matrix.trailing_fixed_columns =
                current->matrix.trailing_fixed_columns;
    }

    /*
     * Make sure we have column_widths
     */
    if (new->matrix.columns && (new->matrix.column_widths == NULL))
    {
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "setValues", "columnWidths", "XbaeMatrix",
            "XbaeMatrix: Must specify columnWidths",
            NULL, 0);
        new->matrix.column_widths = current->matrix.column_widths;
    }


    /*
     * If rows or columns or fixed rows/columns changed,
     * then we need to relayout.
     */
    if (NE(matrix.rows) || NE(matrix.fixed_rows) ||
        NE(matrix.trailing_fixed_rows))
    {
        /*
         * Reset VSB maximum. sliderSize will be reset later in Resize.
         */
        XtVaSetValues(VertScrollChild(new),
                      XmNmaximum, (new->matrix.rows -
                                   (int) new->matrix.fixed_rows -
                                   (int) new->matrix.trailing_fixed_rows),
                      XmNsliderSize, 1,
                      NULL);
        do_top_row = True;
        relayout = True;
    }
    
    if (NE(matrix.columns) || NE(matrix.fixed_columns) ||
        NE(matrix.trailing_fixed_columns))
        relayout = True;

    /*
     * Copy any pointed to resources if they changed
     */

#if CELL_WIDGETS
    if (NE(matrix.cell_widgets))
    {
        xbaeFreeCellWidgets(current);
        xbaeCopyCellWidgets(current);
        redisplay = True;
    }
#endif

    if (NE(matrix.cells))
    {
        xbaeFreeCells(current);
        xbaeCopyCells(new);
        redisplay = True;
        new_cells = True;
    }
    else if (NE(matrix.rows) || NE(matrix.columns))
        ResizeCells(current, new);
    
    if (NE(matrix.cell_user_data))
    {
        xbaeFreeCellUserData(current);
        if (new->matrix.cell_user_data)
            xbaeCopyCellUserData(new);
    }
    if (NE(matrix.row_user_data))
    {
        xbaeFreeRowUserData(current);
        if (new->matrix.row_user_data)
            xbaeCopyRowUserData(new);
    }
    if (NE(matrix.column_user_data))
    {
        xbaeFreeColumnUserData(current);
        if (new->matrix.column_user_data)
            xbaeCopyColumnUserData(new);
    }
    if (NE(matrix.cell_shadow_types))
    {
        xbaeFreeCellShadowTypes(current);
        if (new->matrix.cell_shadow_types)
            xbaeCopyCellShadowTypes(new);
    }
    if (NE(matrix.row_shadow_types))
    {
        xbaeFreeRowShadowTypes(current);
        if (new->matrix.row_shadow_types)
            xbaeCopyRowShadowTypes(new);
    }
    if (NE(matrix.column_shadow_types))
    {
        xbaeFreeColumnShadowTypes(current);
        if (new->matrix.column_shadow_types)
            xbaeCopyColumnShadowTypes(new);
    }
    
    if (NE(matrix.row_labels))
    {
        /*
         * If we added or deleted row_labels, we need to layout.
         */
        if (!current->matrix.row_labels || !new->matrix.row_labels)
            relayout = True;
        else
            redisplay = True;

        xbaeFreeRowLabels(current);
        if (new->matrix.row_labels)
            xbaeCopyRowLabels(new);
    }
    if (NE(matrix.column_labels))
    {
        xbaeFreeColumnLabels(current);
        if (new->matrix.column_labels)
            xbaeCopyColumnLabels(new);
        else
        {
            new->matrix.column_label_lines = NULL;
            new->matrix.column_label_maxlines = 0;
        }

        /*
         * If the number of lines in column labels changed, we need to relayout
         */
        if (current->matrix.column_label_maxlines !=
            new->matrix.column_label_maxlines)
            relayout = True;
        else
            redisplay = True;
    }
    if (NE(matrix.column_max_lengths))
    {
        xbaeFreeColumnMaxLengths(current);
        if (new->matrix.column_max_lengths)
            xbaeCopyColumnMaxLengths(new);
        redisplay = True;
    }
    if (NE(matrix.column_alignments))
    {
        xbaeFreeColumnAlignments(current);
        if (new->matrix.column_alignments)
            xbaeCopyColumnAlignments(new);
        redisplay = True;
    }
    if (NE(matrix.column_button_labels))
    {
        xbaeFreeColumnButtonLabels(current);
        if (new->matrix.column_button_labels)
            xbaeCopyColumnButtonLabels(new);
        redisplay = True;
    }
    if (NE(matrix.row_button_labels))
    {
        xbaeFreeRowButtonLabels(current);
        if (new->matrix.row_button_labels)
            xbaeCopyRowButtonLabels(new);
        redisplay = True;
    }
    if (NE(matrix.column_label_alignments))
    {
        xbaeFreeColumnLabelAlignments(current);
        if (new->matrix.column_label_alignments)
            xbaeCopyColumnLabelAlignments(new);
        redisplay = True;
    }
    if (NE(matrix.row_label_alignment))
        redisplay = True;
    if (NE(matrix.row_label_color) || NE(matrix.column_label_color) ||
        NE(matrix.button_label_background))
        redisplay = True;
    
    if (NE(matrix.colors))
    {
        xbaeFreeColors(current);
        if (new->matrix.colors)
            xbaeCopyColors(new);
        redisplay = True;
    }
    if (NE(matrix.cell_background))
    {
        xbaeFreeBackgrounds(current);
        if (new->matrix.cell_background)
            xbaeCopyBackgrounds(new);
        redisplay = True;
    }
    if (NE(matrix.grid_line_color))
    {
        XtReleaseGC((Widget)new, new->matrix.grid_line_gc);
        XFreeGC(XtDisplay(new), new->matrix.cell_grid_line_gc);
        xbaeCreateGridLineGC(new);
        if ((new->matrix.grid_type == XmGRID_CELL_LINE) ||
            (new->matrix.grid_type == XmGRID_ROW_LINE) ||
            (new->matrix.grid_type == XmGRID_COLUMN_LINE))
            redisplay = True;
    }

    if (NE(matrix.alt_row_count) &&
       (new->matrix.even_row_background != current->core.background_pixel ||
       new->matrix.odd_row_background != current->core.background_pixel))
        redisplay = True;
    
    if (NE(matrix.even_row_background) || NE(matrix.odd_row_background) || 
        NE(matrix.grid_type) || NE(matrix.selected_foreground) ||
        NE(matrix.selected_background))
        redisplay = True;

    if (NE(matrix.grid_type) && (new->matrix.grid_type >= XmGRID_LINE))
        /* Deprecated types. To be removed in next version. */
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget) new),
            "cvtStringToGridType", "deprecatedType",
            "XbaeMatrix",
            "Value for GridType is deprecated and will be removed in next release",
            NULL, NULL);
    
    if (new->matrix.colors && EQ(matrix.colors)
        && (NE(matrix.rows) || NE(matrix.columns)))
    {
        ResizeColors(current, new, False);
        redisplay = True;
    }
    
    if (new->matrix.cell_background && EQ(matrix.cell_background) &&
        (NE(matrix.rows) || NE(matrix.columns)))
    {
        ResizeColors(current, new, True);
        redisplay = True;
    }

    if (NE(matrix.cell_shadow_type) || NE(matrix.shadow_type) ||
        NE(matrix.reverse_select))
        redisplay = True;

    if (NE(matrix.column_widths))
    {
        xbaeFreeColumnWidths(current);
        xbaeCopyColumnWidths(new);
        relayout = True;
        new_column_widths = True;
    }
    
    if (NE(matrix.selected_cells))
    {
        xbaeFreeSelectedCells(current);
        xbaeCopySelectedCells(new);
        redisplay = True;
    }
    else if (NE(matrix.rows) || NE(matrix.columns))
        ResizeSelectedCells(current, new);

#if XmVersion >= 1002
    if (NE(matrix.highlighted_cells))
    {
        xbaeFreeHighlightedCells(current);
        xbaeCopyHighlightedCells(new);
        redisplay = True;
    }
    else if (NE(matrix.rows) || NE(matrix.columns))
        ResizeHighlightedCells(current, new);
#endif
    
    /*
     * If traversal changes, pass through to Clip and textField children.
     */
    if (NE(manager.traversal_on))
    {
        XtVaSetValues(ClipChild(new),
                      XmNtraversalOn, new->manager.traversal_on,
                      NULL);
    }
    if (NE(matrix.even_row_background) || NE(matrix.odd_row_background))
        redisplay = True;
    
    /*
     * Pass through primitive/manager resources to our children
     */
    n = 0;
    if (NE(core.background_pixel))
    {
        /*
         * Set all clip widgets to the new background (thanks Daiji)
         */
        XtVaSetValues(ClipChild(new),
                      XmNbackground, new->core.background_pixel,
                      NULL);
        XtVaSetValues(LeftClip(new),
                      XmNbackground, new->core.background_pixel,
                      NULL);
        XtVaSetValues(RightClip(new),
                      XmNbackground, new->core.background_pixel,
                      NULL);
        XtVaSetValues(TopClip(new),
                      XmNbackground, new->core.background_pixel,
                      NULL);
        XtVaSetValues(BottomClip(new),
                      XmNbackground, new->core.background_pixel,
                      NULL);
        XtSetArg(wargs[n], XmNbackground, new->core.background_pixel); n++;
    }
    if (NE(manager.foreground))
    {
        XtSetArg(wargs[n], XmNforeground, new->manager.foreground); n++;
    }
    if (NE(manager.bottom_shadow_color))
    {
        XtSetArg(wargs[n], XmNbottomShadowColor,
                 new->manager.bottom_shadow_color); n++;
    }
    if (NE(manager.bottom_shadow_pixmap))
    {
        XtSetArg(wargs[n], XmNbottomShadowPixmap,
                 new->manager.bottom_shadow_pixmap); n++;
    }
    if (NE(manager.highlight_color))
    {
        XtSetArg(wargs[n], XmNhighlightColor,
                 new->manager.highlight_color); n++;
    }
    if (NE(manager.highlight_pixmap))
    {
        XtSetArg(wargs[n], XmNhighlightPixmap,
                 new->manager.highlight_pixmap); n++;
    }
    if (NE(manager.top_shadow_color))
    {
        XtSetArg(wargs[n], XmNtopShadowColor,
                 new->manager.top_shadow_color); n++;
    }
    if (NE(manager.top_shadow_pixmap))
    {
        XtSetArg(wargs[n], XmNtopShadowPixmap,
                 new->manager.top_shadow_pixmap); n++;
    }
    if (n)
    {
        XtSetValues(VertScrollChild(new), wargs, n);
        XtSetValues(HorizScrollChild(new), wargs, n);
        XtSetValues(TextChild(new), wargs, n);
    }

    /*
     * Would really like to do this in the above,
     * but we need to override background_pixel.
     */
    if (EQ(matrix.text_background) &&
        (current->matrix.text_background == current->core.background_pixel))
        new->matrix.text_background = new->core.background_pixel;
    if (NE(matrix.text_background) || NE(core.background_pixel))
    {
        XtVaSetValues(TextChild(new),
                      XmNbackground, new->matrix.text_background,
                      NULL);
        if (XtIsManaged(TextChild(new)))
            redisplay = True;
    }
    /*
     * Get a new XFontStruct and copy the fontList if it changed
     * and pass it to the textField.
     * Reset the HSB increment.
     * redisplay and relayout will be set below.
     */
    if (NE(matrix.font_list))
    {
        XmFontListFree(current->matrix.font_list);
        xbaeNewFont(new);
        XtVaSetValues(TextChild(new),
                      XmNfontList, new->matrix.font_list,
                      NULL);
        XtVaSetValues(HorizScrollChild(new),
                      XmNincrement, FONT_WIDTH(new),
                      NULL);
    }

    if (NE(matrix.label_font_list))
    {
        XmFontListFree(current->matrix.label_font_list);
        xbaeNewLabelFont(new);
        XtVaSetValues(HorizScrollChild(new),
                      XmNincrement, FONT_WIDTH(new),
                      NULL);
    }

    /*
     * Pass the cell resources on to the textField.
     * Both redisplay and relayout will be set below.
     *
     * If anything changed to affect cell total width or column positions,
     * recalc them
     */
    if (new_cells || NE(matrix.fid) || NE(matrix.label_fid) ||
        NE(matrix.cell_margin_width) || NE(matrix.cell_margin_height) ||
        NE(matrix.cell_shadow_thickness) || NE(matrix.fixed_columns) ||
        NE(matrix.trailing_fixed_columns) ||
        NE(matrix.cell_highlight_thickness) || new_column_widths ||
        NE(matrix.text_shadow_thickness))
    {
        /*
         * Recalculate the margin height, based on the larger of the
         * label and general fonts.
         */
        int marginHeight = new->matrix.cell_margin_height; /* by default */

        if (LABEL_HEIGHT(new) > FONT_HEIGHT(new))
            marginHeight = (int)(LABEL_HEIGHT(new) +
                                 (new->matrix.cell_margin_height *
                                  2) - FONT_HEIGHT(new)) / 2;
        /*
         * Cancel the edit -> If I think of a better way of doing this
         * I'll do it, AL.
         */
        (*((XbaeMatrixWidgetClass) XtClass(new))->matrix_class.cancel_edit)
            (new, True);

        XtVaSetValues(
            TextChild(new),
            XmNmarginWidth, new->matrix.cell_margin_width,
            XmNhighlightThickness, new->matrix.cell_highlight_thickness,
            XmNshadowThickness, new->matrix.text_shadow_thickness,
            XmNmarginHeight, marginHeight,
            NULL);

        xbaeGetCellTotalWidth(new);

        /*
         * Reset the HSB maximum.  sliderSize will be reset later in Resize.
         */
        XtVaSetValues(HorizScrollChild(new),
                      XmNmaximum, NON_FIXED_TOTAL_WIDTH(new) ?
                      NON_FIXED_TOTAL_WIDTH(new) : 1,
                      XmNsliderSize, 1,
                      NULL);

        /*
         * If the number of columns changed, we need to allocate a new array.
         */
        if (NE(matrix.columns))
        {
            xbaeFreeColumnPositions(current);
            new->matrix.column_positions = CreateColumnPositions(new);
        }

        /*
         * If anything but (trailing_)fixed_columns or the highlight color
         * changed, we need to recalc column positions.
         */
        if (new_cells || NE(matrix.fid) || NE(matrix.label_fid) ||
            NE(matrix.cell_margin_width) || NE(matrix.cell_margin_height) ||
            NE(matrix.cell_shadow_thickness) ||
            NE(matrix.cell_highlight_thickness) || new_column_widths ||
            NE(matrix.text_shadow_thickness))

            xbaeGetColumnPositions(new);

        /*
         * Recalculate the baselines
         */
        new->matrix.text_baseline = XmTextGetBaseline(TextChild(new)) +
            new->matrix.cell_shadow_thickness /*+
            new->matrix.text_shadow_thickness*/;
        
        /*
         * Adjust the label_baseline according to the larger of the two fonts
         */
        if (LABEL_HEIGHT(new) == FONT_HEIGHT(new))
            new->matrix.label_baseline = new->matrix.text_baseline;
        else
        {
            if (LABEL_HEIGHT(new) < FONT_HEIGHT(new))
                marginHeight = (FONT_HEIGHT(new) +
                                 new->matrix.text_shadow_thickness * 2 +
                                 new->matrix.cell_margin_height * 2 -
                                 (int)LABEL_HEIGHT(new)) / 2;
            else 
                marginHeight = new->matrix.cell_margin_height +
                    new->matrix.text_shadow_thickness;
            
            new->matrix.label_baseline = marginHeight +
                new->matrix.cell_shadow_thickness -
                new->matrix.label_font_y;
        }

        /* JDS: The comment above this section says both redisplay and
         * relayout get set, but only relayout was being set.  I noticed
         * this because the resizeColumns action was setting the
         * columnWidths, but the clip widget wasn't properly redrawing
         * its cells when they were resized. This was in conjunction with
         * calling clipRedisplay only when the matrix redisplayed instead
         * of also when it relayedout :) (see the comment below).
         *
         * However, I'm not sure what else this might impact. But, I'm in
         * favor of the change I made to only redisplay when the matrix
         * redisplays since that's more efficient, I think, and it eliminated
         * some ugly redraws that I at least encountered. So there :).
         */
        redisplay = relayout = True;
    }

    /*
     * Install text_translations on textField
     */
    if (NE(matrix.text_translations))
        XtVaSetValues(TextChild(new),
                      XmNtranslations, new->matrix.text_translations,
                      NULL);

    /*
     * If row_label_width was set to 0, calculate it.
     * Otherwise if it was changed, set flags.
     */
    if (new->matrix.row_label_width == 0 && new->matrix.row_labels)
    {
        new->matrix.row_label_width = xbaeMaxRowLabel(new);
        relayout = True;
    }
    else if (NE(matrix.row_label_width))
        relayout = True;

    /*
     * Check whether the widget is sensitive has changed and set our GC's
     * appropriately
     */
    if (XtIsSensitive((Widget)current) != XtIsSensitive((Widget)new))
    {
        XGCValues values;
        int i;
        unsigned long valuemask = GCFillStyle;
        Display *dpy = XtDisplay(new);
        
        if (!XtIsSensitive((Widget)new))
        {
            values.fill_style = FillStippled;

            /*
             * Change our drawing GC's to the stipple effect to indicate
             * the widget is insensitive and redraw
             */
            XChangeGC(dpy, new->matrix.draw_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.label_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.label_clip_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.pixmap_gc, valuemask, &values);
            /*
             * Propogate the insensitive feel to our children
             */
            for (i = 0; i < XbaeNumChildren; i++)
                XtSetSensitive(new->composite.children[i], False);
        }
        else
        {
            values.fill_style = FillSolid;

            XChangeGC(dpy, new->matrix.draw_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.label_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.label_clip_gc, valuemask, &values);
            XChangeGC(dpy, new->matrix.pixmap_gc, valuemask, &values);

            for (i = 0; i < XbaeNumChildren; i++)
                XtSetSensitive(new->composite.children[i], True);
        }
        redisplay = True;
    }

    /*
     * If our fill policy changed or our bottom attachment,
     * we must redisplay and relayout.
     */
    if (NE(matrix.fill) || NE(matrix.trailing_attached_bottom))
    {
        redisplay = True;
        relayout = True;
    }

    /*
     * If either of the scrollbar display policies changed,
     * we need to redisplay and relayout.
     */
    if (NE(matrix.vsb_display_policy) ||
        NE(matrix.hsb_display_policy))
        relayout = True;

    /*
     * If the place of the scrollbars changes, we must redisplay
     */
    if (NE(matrix.scrollbar_placement))
        relayout = True;

    /*
     * Change created GCs if needed
     */

    if (NE(manager.foreground))
    {
        /*
         * We don't need to put the new foreground in draw_gc or
         * draw_clip_gc because they get a new foreground when they are used.
         */
        XSetForeground(XtDisplay(new), new->matrix.cell_top_shadow_clip_gc,
                       new->manager.foreground);
        XSetBackground(XtDisplay(new), new->matrix.cell_bottom_shadow_clip_gc,
                       new->manager.foreground);
        redisplay = True;
    }
    if (NE(manager.top_shadow_color))
    {
        XSetForeground(XtDisplay(new), new->matrix.cell_top_shadow_clip_gc,
                       new->manager.top_shadow_color);
        XSetForeground(XtDisplay(new), new->matrix.resize_top_shadow_gc,
                       new->manager.top_shadow_color);
        redisplay = True;
    }
    if (NE(manager.top_shadow_pixmap))
    {
        XSetTile(XtDisplay(new), new->matrix.cell_top_shadow_clip_gc,
                 new->manager.top_shadow_pixmap);
        XSetTile(XtDisplay(new), new->matrix.resize_top_shadow_gc,
                 new->manager.top_shadow_pixmap);
        redisplay = True;
    }
    if (NE(manager.bottom_shadow_color))
    {
        XSetForeground(XtDisplay(new), new->matrix.cell_bottom_shadow_clip_gc,
                       new->manager.bottom_shadow_color);
        XSetForeground(XtDisplay(new), new->matrix.resize_bottom_shadow_gc,
                       new->manager.bottom_shadow_color);
        redisplay = True;
    }
    if (NE(manager.bottom_shadow_pixmap))
    {
        XSetTile(XtDisplay(new), new->matrix.cell_bottom_shadow_clip_gc,
                 new->manager.bottom_shadow_pixmap);
        XSetTile(XtDisplay(new), new->matrix.resize_bottom_shadow_gc,
                 new->manager.bottom_shadow_pixmap);
        redisplay = True;
    }
    if (NE(matrix.fid))
    {
        XSetFont(XtDisplay(new), new->matrix.draw_gc, new->matrix.fid);
        redisplay = True;
    }

    if (NE(matrix.label_fid))
    {
        XSetFont(XtDisplay(new), new->matrix.label_gc,
                 new->matrix.label_fid);
        XSetFont(XtDisplay(new), new->matrix.label_clip_gc,
                 new->matrix.label_fid);
        redisplay = True;
    }
    /*
     * See if any other resources changed which will require a relayout
     */
    if (NE(matrix.space) || NE(matrix.cell_shadow_thickness) ||
        NE(manager.shadow_thickness))
        relayout = True;

    /*
     * If bold_labels or button labels changed, and we have labels,
     * we must redisplay
     */
    if ((NE(matrix.bold_labels) || NE(matrix.button_labels)) &&
         (new->matrix.row_labels || new->matrix.column_labels))
         redisplay = True;

    /*
     * If showArrows is changed, redisplay to get rid of existing arrows
     */
    if (NE(matrix.show_arrows))
        redisplay = True;
    
    /*
     * Compute a new size if:
     *         visible_rows or visible_columns changed.
     *         user set our width or height to zero.
     */
    if (NE(matrix.visible_rows) || NE(matrix.visible_columns) ||
        request->core.height == 0 || request->core.width == 0)
        xbaeComputeSize(new, request->core.width == 0,
                        request->core.height == 0);

    /*
     * If our size didn't change, but we need to layout, call Resize.
     * If our size did change, then Xt will call our Resize method for us.
     * If our size did change, but the new size is later refused,
     *         then SetValuesAlmost will call Resize to layout.
     */
    if (EQ(core.width) && EQ(core.height) && relayout)
        xbaeResize(new);

    /*
     * The user forced a new top_row or something changed to force
     * us to recheck the current top_row.
     */
    if (NE(matrix.top_row) || do_top_row)
    {
        XmScrollBarCallbackStruct call_data;

        xbaeAdjustTopRow(new);
         call_data.value = VERT_ORIGIN(new);
        /*
         * Trick xbaeScrollVertCB() into believing it needs to scroll
         */
        VERT_ORIGIN(new) = VERT_ORIGIN(current);
        xbaeScrollVertCB((Widget)VertScrollChild(new), NULL, &call_data);
        VERT_ORIGIN(new) = call_data.value; /* and reset VERT_ORIGIN */
        
        XtVaSetValues(VertScrollChild(new),
                      XmNvalue, VERT_ORIGIN(new),
                      NULL);
    }

    if (NE(matrix.left_column) || do_left_column)
    {
        XmScrollBarCallbackStruct call_data;

        xbaeAdjustLeftColumn(new);
        call_data.value = HORIZ_ORIGIN(new);
        HORIZ_ORIGIN(new) = HORIZ_ORIGIN(current);
        xbaeScrollHorizCB((Widget)HorizScrollChild(new), NULL, &call_data);
        HORIZ_ORIGIN(new) = call_data.value;
        XtVaSetValues(HorizScrollChild(new),
                      XmNvalue, HORIZ_ORIGIN(new),
                      NULL);
    }

    /*
     * Force the Clip widget to redisplay.  Note: this may generate an
     * expose event for the current size of the Clip widget, and the Clip
     * widget may be sized smaller in set_values_almost.  The ClipRedisplay
     * function can handle this case.
     *
     * JDS: Don't need to force a redisplay on a relayout, since the Clip
     * widget's resize method (now non-NULL) will be called and Xt will
     * automatically do an expose after that occurs. Seems to work, anyways :).
     */
    if (redisplay)
        XbaeMatrixRefresh((Widget)new);

    /*
     * We want to return True when we need to redisplay or relayout.
     */
    return redisplay || relayout;

#undef NE
#undef EQ
}

/* ARGSUSED */
static void
SetValuesAlmost(old, new, request, reply)
XbaeMatrixWidget old;
XbaeMatrixWidget new;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
{
    /*
     * If XtGeometryAlmost, accept compromize - Resize will take care of it
     */
    if (reply->request_mode)
    {
        *request = *reply;

#if XtSpecificationRelease > 4
        /*
         * In R5, XtSetValues changed so that when a widgets parent
         * returns XtGeometryAlmost, Xt will only call the widgets resize
         * method if the widgets size actually changed.  It turns out that
         * some manager widgets (old Wcl XmpTable and 1.1.x XmForm) return
         * XtGeometryAlmost with a compromise size which is the widgets
         * original size (not much of a compromise)!  This means as of R5,
         * Matrix's resize method won't get called in that case.
         *
         * So, for R5 we explicitly call our resize method here for the
         * case of XtGeometryAlmost where our size did not change.
         */
        if ((reply->request_mode & CWWidth ||
             reply->request_mode & CWHeight)
            &&
            (old->core.width == new->core.width &&
             old->core.height == new->core.height))
            xbaeResize(new);
#endif
    }

    /*
     * If XtGeometryNo, call Resize to relayout if it was a size change
     * that was denied.
     * Accept the original geometry.
     * (we need to call Resize even though the size
     * didn't change to force a relayout - set_values relies on this)
     */
    else
    {
        if ((request->request_mode & CWWidth ||
             request->request_mode & CWHeight))
            xbaeResize(new);

        request->request_mode = 0;
    }
}

static void
Destroy(mw)
XbaeMatrixWidget mw;
{
#if XmVersion >= 1002
    if (TraverseID)
        XtRemoveTimeOut(TraverseID);
#endif

    XtReleaseGC((Widget) mw, mw->matrix.grid_line_gc);
    XFreeGC(XtDisplay(mw), mw->matrix.cell_grid_line_gc);

    XFreeGC(XtDisplay(mw), mw->matrix.label_gc);
    XFreeGC(XtDisplay(mw), mw->matrix.label_clip_gc);
    XFreeGC(XtDisplay(mw), mw->matrix.draw_gc);
    XFreeGC(XtDisplay(mw), mw->matrix.pixmap_gc);
    XFreeGC(XtDisplay(mw), mw->matrix.cell_top_shadow_clip_gc);
    XFreeGC(XtDisplay(mw), mw->matrix.cell_bottom_shadow_clip_gc);
    XtReleaseGC((Widget) mw, mw->matrix.resize_top_shadow_gc);
    XtReleaseGC((Widget) mw, mw->matrix.resize_bottom_shadow_gc);

    xbaeFreeCells(mw);
#if CELL_WIDGETS
    xbaeFreeCellWidgets(mw);
#endif
    xbaeFreeRowLabels(mw);
    xbaeFreeColumnLabels(mw);
    xbaeFreeColumnWidths(mw);
    xbaeFreeColumnMaxLengths(mw);
    xbaeFreeColumnPositions(mw);
    xbaeFreeColumnAlignments(mw);
    xbaeFreeColumnButtonLabels(mw);
    xbaeFreeRowButtonLabels(mw);
    xbaeFreeColumnLabelAlignments(mw);
    xbaeFreeCellUserData(mw);
    xbaeFreeRowUserData(mw);
    xbaeFreeColumnUserData(mw);
    xbaeFreeCellShadowTypes(mw);
    xbaeFreeRowShadowTypes(mw);
    xbaeFreeColumnShadowTypes(mw);
    xbaeFreeColors(mw);
    xbaeFreeBackgrounds(mw);
    xbaeFreeSelectedCells(mw);
#if XmVersion >= 1002
    xbaeFreeHighlightedCells(mw);
#endif

    XmFontListFree(mw->matrix.font_list);
    XmFontListFree(mw->matrix.label_font_list);

    xbaeSmDestroyScrollMgr(mw->matrix.matrix_scroll_mgr);
    xbaeSmDestroyScrollMgr(mw->matrix.clip_scroll_mgr);
}

/*
 * Since we totally control our childrens geometry, allow anything.
 */
/* ARGSUSED */
static XtGeometryResult
GeometryManager(w, desired, allowed)
Widget w;
XtWidgetGeometry *desired, *allowed;
{
#define Wants(flag) (desired->request_mode & flag)

    if (Wants(XtCWQueryOnly))
        return (XtGeometryYes);

    if (Wants(CWWidth))
        w->core.width = desired->width;
    if (Wants(CWHeight))
        w->core.height = desired->height;
    if (Wants(CWX))
        w->core.x = desired->x;
    if (Wants(CWY))
        w->core.y = desired->y;
    if (Wants(CWBorderWidth))
        w->core.border_width = desired->border_width;

    return (XtGeometryYes);

#undef Wants
}

/*
 * We would prefer to be the size calculated in ComputeSize and saved in
 * desired_width/height
 */
static XtGeometryResult
QueryGeometry(mw, proposed, desired)
XbaeMatrixWidget mw;
XtWidgetGeometry *proposed, *desired;
{
#define Set(bit) (proposed->request_mode & bit)

    desired->width = mw->matrix.desired_width;
    desired->height = mw->matrix.desired_height;
    desired->request_mode = CWWidth | CWHeight;

    if (Set(CWWidth) && proposed->width == desired->width &&
        Set(CWHeight) && proposed->height == desired->height)
        return (XtGeometryYes);

    if (desired->width == mw->core.width && desired->height == mw->core.height)
        return (XtGeometryNo);

    return (XtGeometryAlmost);

#undef Set
}

/*
 * This function is called from either the Clip focus CB or a timeout proc.
 * It is called as a result of the Clip getting the focus. We want to give
 * the focus to the textField if a cell is being edited.  If no cells are
 * being edited, force an edit on the top left most visible cell.
 */
static void
TraverseIn(mw)
XbaeMatrixWidget mw;
{
    /*
     * If the traversing flag is set, then Clip got the focus because
     * textField was trying to traverse out of mw.  We'll help it along.
     * Sickening.
     */
    if (mw->matrix.traversing != NOT_TRAVERSING)
    {
        XmProcessTraversal(ClipChild(mw), mw->matrix.traversing);
        return;
    }

    /*
     * If the textField is managed and not visible, scroll it onto the screen
     * and traverse to it.
     */
    if (XtIsManaged(TextChild(mw)))
    {
        if (mw->matrix.scroll_select)
            xbaeMakeCellVisible(mw, mw->matrix.current_row,
                                 mw->matrix.current_column);
        XmProcessTraversal(TextChild(mw), XmTRAVERSE_CURRENT);
    }

    /*
     * Otherwise, no cell is being edited.  Force an edit on the top-left
     * most visible cell.
     */
    else
    {
        int column = xbaeXtoCol(mw, FIXED_COLUMN_WIDTH(mw) + HORIZ_ORIGIN(mw));
        int row = VERT_ORIGIN(mw) + mw->matrix.fixed_rows;

        /*
         * Call the traverseCellCallback to allow the application to
         * perform custom traversal.
         */
        if (mw->matrix.traverse_cell_callback)
        {
            XbaeMatrixTraverseCellCallbackStruct call_data;

            call_data.reason = XbaeTraverseCellReason;
            call_data.event = (XEvent *)NULL;
            call_data.row = 0;
            call_data.column = 0;
            call_data.next_row = row;
            call_data.next_column = column;
            call_data.fixed_rows = mw->matrix.fixed_rows;
            call_data.fixed_columns = mw->matrix.fixed_columns;
            call_data.trailing_fixed_rows = mw->matrix.trailing_fixed_rows;
            call_data.trailing_fixed_columns =
                mw->matrix.trailing_fixed_columns;
            call_data.num_rows = mw->matrix.rows;
            call_data.num_columns = mw->matrix.columns;
            call_data.param = NULL;
            call_data.qparam = NULLQUARK;

            XtCallCallbackList((Widget) mw, mw->matrix.traverse_cell_callback,
                               (XtPointer) & call_data);

            row = call_data.next_row;
            column = call_data.next_column;
        }

        (*((XbaeMatrixWidgetClass) XtClass(mw))->matrix_class.edit_cell)
            (mw, NULL, row, column, NULL, 0);

        XmProcessTraversal(TextChild(mw), XmTRAVERSE_CURRENT);
    }
}

#if XmVersion >= 1002
/*
 * Under Motif 1.2, TraverseInCB can't call TraverseIn directly, so it
 * adds a zero length timeout and we call it from here.
 */
/* ARGSUSED */
static void
TraverseInTimeOut(mw, timer)
XtPointer mw;
XtIntervalId *timer;
{
    TraverseIn((XbaeMatrixWidget)mw);
}

#endif /* XmVersion >= 1002 */

/*
 * This is the Clip widgets focusCallback. We want to give the focus to
 * the textField if a cell is being edited.  If no cells are being edited,
 * force an edit on the top left most visible cell.
 */
/* ARGSUSED */
static void
TraverseInCB(w, mw, call_data)
Widget w;
XbaeMatrixWidget mw;
XtPointer call_data;
{
#if XmVersion < 1002
    TraverseIn(mw);
#else
    /*
     * Under Motif 1.2, we can't call TraverseIn directly because it
     * calls XmProcessTraversal and recursive calls to XmProcessTraversal
     * are disallowed in 1.2 (we may be in this CB as a result of someone
     * calling XmProcessTraversal).  So we add a zero length timeout
     * and call TraverseIn from there.
     */
    TraverseID = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget) w), 0L,
                                 TraverseInTimeOut, (XtPointer) mw);
#endif
}

/*
 * Add rows/columns of cells when set_values changes our rows/columns
 */
static void
ResizeCells(current, new)
XbaeMatrixWidget current;
XbaeMatrixWidget new;
{
    int i, j;
    int safe_rows = 0;

    /*
     * If there is a draw cell callback, we don't need to allocate any
     * memory for the cells so get outta here.  If there is a draw cell
     * callback, no memory should have been allocated when this point is
     * reached.
     */
    if (!new->matrix.cells)
        return;
    
    if (new->matrix.rows == current->matrix.rows)
        safe_rows = new->matrix.rows;

    /*
     * Adding rows
     */
    if (new->matrix.rows > current->matrix.rows)
    {
        /*
         * Realloc a larger array of row pointers
         */
        new->matrix.cells =
            (String **) XtRealloc((char *) new->matrix.cells,
                                  new->matrix.rows * sizeof(String *));

        /*
         * Malloc a new row array for each row. Initialize it with
         * NULL Strings. Use the new column size.
         */
        for (i = current->matrix.rows; i < new->matrix.rows; i++)
        {
            new->matrix.cells[i] =
                (String *) XtMalloc(new->matrix.columns * sizeof(String));
            for (j = 0; j < new->matrix.columns; j++)
                new->matrix.cells[i][j] = XtNewString("");
        }

        safe_rows = current->matrix.rows;
    }

    /*
     * Deleting rows
     */
    if (new->matrix.rows < current->matrix.rows)
    {
        /*
         * Free the cells in the rows being deleted and the rows themselves
         */
        for (i = new->matrix.rows; i < current->matrix.rows; i++)
        {
            for (j = 0; j < current->matrix.columns; j++)
                XtFree((XtPointer) new->matrix.cells[i][j]);
            XtFree((XtPointer) new->matrix.cells[i]);
        }

        safe_rows = new->matrix.rows;
    }

    /*
     * Adding columns
     */
    if (new->matrix.columns > current->matrix.columns)
    {
        /*
         * Realloc each row array. Do not touch any rows added/deleted above
         * (use safe_rows)
         */
        for (i = 0; i < safe_rows; i++)
        {
            new->matrix.cells[i] =
                (String *) XtRealloc((char *) new->matrix.cells[i],
                                     new->matrix.columns * sizeof(String));
            for (j = current->matrix.columns; j < new->matrix.columns; j++)
                new->matrix.cells[i][j] = XtNewString("");
        }
    }

    /*
     * Deleting columns
     */
    if (new->matrix.columns < current->matrix.columns)
    {
        /*
         * Free all the cells in the deleted columns. Do not touch any
         * rows added/deleted above (use safe_rows).
         * We don't bother to realloc each row, just leave some wasted space.
         * XXX is this a problem?
         */
        for (i = 0; i < safe_rows; i++)
            for (j = new->matrix.columns; j < current->matrix.columns; j++)
                XtFree((XtPointer) new->matrix.cells[i][j]);
    }
}

/*
 * Add rows/columns of selected flags when set_values changes our rows/columns
 */
static void
ResizeSelectedCells(current, new)
XbaeMatrixWidget current;
XbaeMatrixWidget new;
{
    int i;
    int safe_rows = 0;

    /*
     * selectedCells is allocated when the first cell is selected.  If it
     * is still NULL, no cells have been selected up to this point.
     */     
    if (!new->matrix.selected_cells)
        return;
    
    if (new->matrix.rows == current->matrix.rows)
        safe_rows = new->matrix.rows;

    /*
     * Adding rows
     */
    if (new->matrix.rows > current->matrix.rows)
    {
        /*
         * Realloc a larger array of row pointers
         */
        new->matrix.selected_cells =
            (Boolean **) XtRealloc((char *) new->matrix.selected_cells,
                                   new->matrix.rows * sizeof(Boolean *));

        /*
         * Calloc a new row array for each row. Use the new column size.
         */
        for (i = current->matrix.rows; i < new->matrix.rows; i++)
            new->matrix.selected_cells[i] =
                (Boolean *) XtCalloc(new->matrix.columns, sizeof(Boolean));

        safe_rows = current->matrix.rows;
    }

    /*
     * Deleting rows
     */
    if (new->matrix.rows < current->matrix.rows)
    {
        for (i = new->matrix.rows; i < current->matrix.rows; i++)
            XtFree((XtPointer) new->matrix.selected_cells[i]);
        safe_rows = new->matrix.rows;
    }

    /*
     * Adding columns
     */
    if (new->matrix.columns > current->matrix.columns)
    {
        /*
         * Realloc each row array. Do not touch any rows added/deleted above
         * (use safe_rows)
         */
        for (i = 0; i < safe_rows; i++)
        {
            int j;

            new->matrix.selected_cells[i] =
                (Boolean *) XtRealloc((char *) new->matrix.selected_cells[i],
                                      new->matrix.columns * sizeof(Boolean));
            for (j = current->matrix.columns; j < new->matrix.columns; j++)
                new->matrix.selected_cells[i][j] = False;
        }
    }

    /*
     * Deleting columns
     *   if (new->matrix.columns < current->matrix.columns)
     * We don't bother to realloc, just leave some wasted space.
     * XXX is this a problem?
     */
}

#if XmVersion >= 1002
/*
 * Add rows/columns of highlighted flags when set_values changes our rows/columns
 */
static void
ResizeHighlightedCells(current, new)
XbaeMatrixWidget current;
XbaeMatrixWidget new;
{
    int i;
    int safe_rows = 0;

    if (!new->matrix.highlighted_cells)
        return;
    
    if (new->matrix.rows == current->matrix.rows)
        safe_rows = new->matrix.rows;

    /*
     * Adding rows
     */
    if (new->matrix.rows > current->matrix.rows)
    {
        /*
         * Realloc a larger array of row pointers
         */
        new->matrix.highlighted_cells =
            (unsigned char **) XtRealloc(
                (char *) new->matrix.highlighted_cells,
                new->matrix.rows * sizeof(unsigned char *));

        /*
         * Calloc a new row array for each row. Use the new column size.
         */
        for (i = current->matrix.rows; i < new->matrix.rows; i++)
            new->matrix.highlighted_cells[i] =
                (unsigned char *) XtCalloc(new->matrix.columns,
                                           sizeof(unsigned char));

        safe_rows = current->matrix.rows;
    }

    /*
     * Deleting rows
     */
    if (new->matrix.rows < current->matrix.rows)
    {
        for (i = new->matrix.rows; i < current->matrix.rows; i++)
            XtFree((XtPointer) new->matrix.highlighted_cells[i]);
        safe_rows = new->matrix.rows;
    }

    /*
     * Adding columns
     */
    if (new->matrix.columns > current->matrix.columns)
    {
        /*
         * Realloc each row array. Do not touch any rows added/deleted above
         * (use safe_rows)
         */
        for (i = 0; i < safe_rows; i++)
        {
            int j;

            new->matrix.highlighted_cells[i] =
                (unsigned char *) XtRealloc(
                    (char *) new->matrix.highlighted_cells[i],
                    new->matrix.columns * sizeof(unsigned char));
            for (j = current->matrix.columns; j < new->matrix.columns; j++)
                new->matrix.highlighted_cells[i][j] = HighlightNone;
        }
    }

    /*
     * Deleting columns
     *   if (new->matrix.columns < current->matrix.columns)
     * We don't bother to realloc, just leave some wasted space.
     * XXX is this a problem?
     */
}
#endif

/*
 * Add rows/columns of colors when set_values changes our rows/columns
 */
static void
#if NeedFunctionPrototypes
ResizeColors(XbaeMatrixWidget current, XbaeMatrixWidget new, Boolean bg)
#else
ResizeColors(current, new, bg)
XbaeMatrixWidget current;
XbaeMatrixWidget new;
Boolean bg;
#endif
{
    int i, j;
    int safe_rows = 0;

    if (! new->matrix.rows)
        return;
    
    if (new->matrix.rows == current->matrix.rows)
        safe_rows = new->matrix.rows;

    /*
     * Adding rows
     */
    if (new->matrix.rows > current->matrix.rows)
    {
        /*
         * Realloc a larger array of row pointers
         */
        if (bg)
        {
            new->matrix.cell_background =
                (Pixel **) XtRealloc((char *) new->matrix.cell_background,
                                     new->matrix.rows * sizeof(Pixel *));
            for (i = current->matrix.rows; i < new->matrix.rows; i++)
            {
                new->matrix.cell_background[i] =
                    (Pixel *) XtMalloc(new->matrix.columns * sizeof(Pixel));

                for (j = 0; j < new->matrix.columns; j++)
                    new->matrix.cell_background[i][j] =
                        new->core.background_pixel;
            }
        }
        else
        {
            new->matrix.colors =
                (Pixel **) XtRealloc((char *) new->matrix.colors,
                                     new->matrix.rows * sizeof(Pixel *));
            for (i = current->matrix.rows; i < new->matrix.rows; i++)
            {
                new->matrix.colors[i] =
                    (Pixel *) XtMalloc(new->matrix.columns * sizeof(Pixel));

                for (j = 0; j < new->matrix.columns; j++)
                    new->matrix.colors[i][j] = new->manager.foreground;
            }
        }
        /*
         * Malloc a new row array for each row. Initialize it with foreground.
         * Use the new column size.
         */
        safe_rows = current->matrix.rows;
    }

    /*
     * Deleting rows
     */
    if (new->matrix.rows < current->matrix.rows)
    {
        if (bg)
            for (i = new->matrix.rows; i < current->matrix.rows; i++)
                XtFree((XtPointer) new->matrix.cell_background[i]);
        else
            for (i = new->matrix.rows; i < current->matrix.rows; i++)
                XtFree((XtPointer) new->matrix.colors[i]);
            
        safe_rows = new->matrix.rows;
    }

    /*
     * Adding columns
     */
    if (new->matrix.columns > current->matrix.columns)
    {
        /*
         * Realloc each row array. Do not touch any rows added/deleted above
         * (use safe_rows)
         */
        if (bg)
        {
            for (i = 0; i < safe_rows; i++)
            {
                int k;
                
                new->matrix.cell_background[i] =
                    (Pixel *) XtRealloc(
                        (char *) new->matrix.cell_background[i],
                        new->matrix.columns * sizeof(Pixel));
                
                for (k = current->matrix.columns; k < new->matrix.columns;
                     k++)
                    new->matrix.cell_background[i][k] =
                        new->core.background_pixel;                
            }
        }
        else
        {
            for (i = 0; i < safe_rows; i++)
            {
                int k;
                
                new->matrix.colors[i] =
                    (Pixel *) XtRealloc(
                        (char *) new->matrix.colors[i],
                        new->matrix.columns * sizeof(Pixel));
                
                for (k = current->matrix.columns; k < new->matrix.columns;
                     k++)
                    new->matrix.colors[i][k] = new->manager.foreground;
            }
        }
    }

    /*
     * Deleting columns
     *   if (new->matrix.columns < current->matrix.columns)
     * We don't bother to realloc, just leave some wasted space.
     * XXX is this a problem?  AL: Probably! If you are deleting enough
     * columns, it would be nice to make the memory available again.
     * I'll get to it later.
     */
}

