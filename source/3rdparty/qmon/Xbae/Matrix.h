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
 * $Id: Matrix.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

#ifndef _Xbae_Matrix_h
#define _Xbae_Matrix_h

/*
 * Matrix Widget public include file
 */

#include <Xm/Xm.h>
#include <X11/Core.h>
#include "patchlevel.h"

/*
 * A few definitions we like to use, but those with R4 won't have.
 * From Xfuncproto.h in R5.
 */

#ifndef XlibSpecificationRelease
# ifndef _XFUNCPROTOBEGIN
#   ifdef __cplusplus                      /* for C++ V2.0 */
#     define _XFUNCPROTOBEGIN extern "C" {
#     define _XFUNCPROTOEND }
#   else
#     define _XFUNCPROTOBEGIN
#     define _XFUNCPROTOEND
#   endif
# endif /* _XFUNCPROTOBEGIN */
#else
#include <X11/Xfuncproto.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Resources:
 * Name                        Class                        RepType                Default Value
 * ----                        -----                        -------                -------------
 * allowColumnResize        ColumnResize                Boolean                False
 * altRowCount                AltRowCount                int                1
 * boldLabels                BoldLabels                Boolean                False
 * buttonLabels                ButtonLabels                Boolean                False
 * buttonLabelBackground Color                        Pixel                dynamic
 * calcCursorPosition        CalcCursorPosition        Boolean                False
 * cellBackgrounds        Colors                        PixelTable        NULL
 * cellHighlightThickness HighlightThickness        HorizontalDimension 2
 * cellMarginHeight        MarginHeight                VerticalDimension   5
 * cellMarginWidth        MarginWidth                HorizontalDimension 5
 * cells                Cells                        StringTable        NULL
 * cellShadowThickness        ShadowThickness                Dimension        2
 * cellShadowType        ShadowType                unsigned char        SHADOW_OUT
 * cellShadowTypes        CellShadowTypes                ShadowTypeTable NULL
 * cellUserData                CellUserData                UserDataTable        NULL
 * clipWindow                XmCClipWindow                Widget                NULL (get only)
 * colors                Colors                        PixelTable        NULL
 * columnAlignments        Alignments                AlignmentArray        dynamic
 * columnButtonLabels        ButtonLabels                BooleanArray        NULL
 * columnLabelAlignments Alignments                AlignmentArray        dynamic
 * columnLabelColor        Color                        Pixel                dynamic
 * columnLabels                Labels                        StringArray        NULL
 * columnMaxLengths        ColumnMaxLengths        MaxLengthArray        NULL
 * columnShadowTypes        ShadowTypes                ShadowTypeArray NULL
 * columnUserData        UserDatas                UserDataArray        NULL
 * columnWidths                ColumnWidths                WidthArray        NULL
 * columns                Columns                        int                0
 * defaultActionCallback Callback               Callback        NULL
 * doubleClickInterval  Interval                int             dynamic
 * drawCellCallback        Callback                Callback        NULL
 * enterCellCallback        Callback                Callback        NULL
 * evenRowBackground        Background                Pixel                dynamic
 * fill                        Fill                        Boolean                False
 * fixedColumns                FixedColumns                Dimension        0
 * fixedRows                FixedRows                Dimension        0
 * fontList                FontList                FontList        fixed
 * labelFont                FontList                FontList        fixed
 * gridLineColor        Color                        Pixel                dynamic
 * gridType                GridType                GridType        XmGRID_CELL_LINE
 * highlightedCells        HighlightedCells        HighlightTable        dynamic
 * horizonalScrollBar        HorizonalScrollBar        Widget                NULL (get only)
 * horizontalScrollBarDisplayPolicy
 *                        XmCMatrixScrollBarDisplayPolicy
 *                                                unsigned char        AS_NEEDED
 * labelActivateCallback Callback                Callback        NULL
 * leaveCellCallback        Callback                Callback        NULL
 * leftColumn           LeftColumn              int             0
 * modifyVerifyCallback        Callback                Callback        NULL
 * oddRowBackground        Background                Pixel                NULL
 * processDragCallback        Callback                Callback        NULL
 * resizeCallback        Callback                Callback        NULL
 * resizeColumnCallback        Callback                Callback        NULL
 * reverseSelect        reverseSelect                Boolean                False
 * rowButtonLabels                ButtonLabels                BooleanArray        NULL
 * rowLabelAlignment        Alignment                Alignment        XmALIGNMENT_END
 * rowLabelColor        Color                        Pixel                dynamic
 * rowLabelWidth        RowLabelWidth                Short                dynamic
 * rowLabels                Labels                        StringArray        NULL
 * rowShadowTypes        ShadowTypes                ShadowTypeArray NULL
 * rowUserData                UserDatas                UserDataArray        NULL
 * rows                        Rows                        int                0
 * selectCellCallback        Callback                Callback        NULL
 * selectedBackground        Color                        Pixel                dynamic
 * selectedCells        SelectedCells                BooleanTable        dynamic
 * selectedForeground        Color                        Pixel                dynamic
 * selectScrollVisible        SelectScrollVisible        Boolean                True
 * space                Space                        Dimension        6
 * shadowType                ShadowType                unsigned char        SHADOW_IN
 * textBackground        Backgound                Pixel           dynamic
 * textField                TextField                Widget                NULL (get only)
 * textShadowThickness        TextShadowThickness        Dimension        0
 * textTranslations        Translations                TranslationTable dynamic
 * topRow                TopRow                        int                0
 * trailingFixedColumns        TrailingFixedColumns        Dimension        0
 * trailingFixedRows        TrailingFixedRows        Dimension        0
 * traverseCellCallback        Callback                Callback        NULL
 * traverseFixedCells        TraverseFixedCells        Boolean                False
 * verticalScrollBar        VerticalScrollBar        Widget                NULL (get only)
 * verticalScrollBarDisplayPolicy
 *                        XmCMatrixScrollBarDisplayPolicy
 *                                                unsigned char        AS_NEEDED
 * visibleColumns        VisibleColumns                Dimension        0
 * visibleRows                VisibleRows                Dimension        0
 * writeCellCallback        Callback                Callback        NULL
 */

#ifndef XmNallowColumnResize
#define XmNallowColumnResize                "allowColumnResize"
#endif
#ifndef XmNaltRowCount
#define XmNaltRowCount                        "altRowCount"
#endif
#ifndef XmNboldLabels
#define XmNboldLabels                        "boldLabels"
#endif
#ifndef XmNbuttonLabels
#define XmNbuttonLabels                        "buttonLabels"
#endif
#ifndef XmNbuttonLabelBackground
#define XmNbuttonLabelBackground        "buttonLabelBackground"
#endif
#ifndef XmNcalcCursorPosition
#define XmNcalcCursorPosition                "calcCursorPosition"
#endif
#ifndef XmNcellBackgrounds
#define XmNcellBackgrounds                "cellBackgrounds"
#endif
#ifndef XmNcellHighlightThickness
#define XmNcellHighlightThickness        "cellHighlightThickness"
#endif
#ifndef XmNcellMarginHeight
#define XmNcellMarginHeight                "cellMarginHeight"
#endif
#ifndef XmNcellMarginWidth
#define XmNcellMarginWidth                "cellMarginWidth"
#endif
#ifndef XmNcellShadowType
#define XmNcellShadowType                "cellShadowType"
#endif
#ifndef XmNcellShadowTypes
#define XmNcellShadowTypes                "cellShadowTypes"
#endif
#ifndef XmNcellShadowThickness
#define XmNcellShadowThickness                "cellShadowThickness"
#endif
#ifndef XmNcellUserData
#define XmNcellUserData                        "cellUserData"
#endif
#if CELL_WIDGETS
#ifndef XmNcellWidgets
#define XmNcellWidgets                        "cellWidgets"
#endif
#endif
#ifndef XmNcells
#define XmNcells                        "cells"
#endif
#ifndef XmNcolors
#define XmNcolors                        "colors"
#endif
#ifndef XmNcolumnAlignments
#define XmNcolumnAlignments                "columnAlignments"
#endif
#ifndef XmNcolumnButtonLabels
#define XmNcolumnButtonLabels                "columnButtonLabels"
#endif
#ifndef XmNcolumnLabelAlignments
#define XmNcolumnLabelAlignments        "columnLabelAlignments"
#endif
#ifndef XmNcolumnLabelBackground
#define XmNcolumnLabelBackground        "columnLabelBackground"
#endif
#ifndef XmNcolumnLabelColor
#define XmNcolumnLabelColor                "columnLabelColor"
#endif
#ifndef XmNcolumnLabels
#define XmNcolumnLabels                        "columnLabels"
#endif
#ifndef XmNcolumnMaxLengths
#define XmNcolumnMaxLengths                "columnMaxLengths"
#endif
#ifndef XmNcolumnShadowTypes
#define XmNcolumnShadowTypes                "columnShadowTypes"
#endif
#ifndef XmNcolumnUserData
#define XmNcolumnUserData                "columnUserData"
#endif
#ifndef XmNcolumnWidths
#define XmNcolumnWidths                        "columnWidths"
#endif
#ifndef XmNdrawCellCallback
#define XmNdrawCellCallback                "drawCellCallback"
#endif
#ifndef XmNenterCellCallback
#define XmNenterCellCallback                "enterCellCallback"
#endif
#ifndef XmNevenRowBackground
#define XmNevenRowBackground                "evenRowBackground"
#endif
#ifndef XmNfill
#define XmNfill                                "fill"
#endif
#ifndef XmNfixedColumns
#define XmNfixedColumns                        "fixedColumns"
#endif
#ifndef XmNfixedRows
#define XmNfixedRows                        "fixedRows"
#endif
#ifndef XmNgridLineColor
#define XmNgridLineColor                "gridLineColor"
#endif
#ifndef XmNgridType
#define XmNgridType                        "gridType"
#endif
#if XmVersion >= 1002
#ifndef XmNhighlightedCells
#define XmNhighlightedCells                "highlightedCells"
#endif
#endif
#ifndef XmNhorizontalScrollBarDisplayPolicy
#define XmNhorizontalScrollBarDisplayPolicy "horizontalScrollBarDisplayPolicy"
#endif
#ifndef XmNlabelActivateCallback
#define XmNlabelActivateCallback        "labelActivateCallback"
#endif
#ifndef XmNlabelFont
#define XmNlabelFont                        "labelFont"
#endif
#ifndef XmNleaveCellCallback
#define XmNleaveCellCallback                "leaveCellCallback"
#endif
#ifndef XmNleftColumn
#define XmNleftColumn                        "leftColumn"
#endif
#ifndef XmNoddRowBackground
#define XmNoddRowBackground                "oddRowBackground"
#endif
#if XmVersion > 1001
#ifndef XmNprocessDragCallback
#define XmNprocessDragCallback                "processDragCallback"
#endif
#endif
#ifndef XmNresizeCallback
#define XmNresizeCallback               "resizeCallback"
#endif
#ifndef XmNresizeColumnCallback
#define XmNresizeColumnCallback                "resizeColumnCallback"
#endif
#ifndef XmNreverseSelect
#define XmNreverseSelect                "reverseSelect"
#endif
#ifndef XmNrowButtonLabels
#define XmNrowButtonLabels                "rowButtonLabels"
#endif
#ifndef XmNrowLabelAlignment
#define XmNrowLabelAlignment                "rowLabelAlignment"
#endif
#ifndef XmNrowLabelWidth
#define XmNrowLabelWidth                "rowLabelWidth"
#endif
#ifndef XmNrowLabelBackground
#define XmNrowLabelBackground                "rowLabelBackground"
#endif
#ifndef XmNrowLabelColor
#define XmNrowLabelColor                "rowLabelColor"
#endif
#ifndef XmNrowLabels
#define XmNrowLabels                        "rowLabels"
#endif
#ifndef XmNrowShadowTypes
#define XmNrowShadowTypes                "rowShadowTypes"
#endif
#ifndef XmNrowUserData
#define XmNrowUserData                        "rowUserData"
#endif
#ifndef XmNselectedCells
#define XmNselectedCells                "selectedCells"
#endif
#ifndef XmNselectedBackground
#define XmNselectedBackground                "selectedBackground"
#endif
#ifndef XmNselectCellCallback
#define XmNselectCellCallback                "selectCellCallback"
#endif
#ifndef XmNselectedForeground
#define XmNselectedForeground                "selectedForeground"
#endif
#ifndef XmNselectScrollVisible
#define XmNselectScrollVisible                "selectScrollVisible"
#endif
#ifndef XmNtextBackground
#define XmNtextBackground                "textBackground"
#endif
#ifndef XmNtextField
#define XmNtextField                        "textField"
#endif
#ifndef XmNtopRow
#define XmNtopRow                        "topRow"
#endif
#ifndef XmNtrailingAttachedBottom
#define XmNtrailingAttachedBottom        "trailingAttachedBottom"
#endif
#ifndef XmNtrailingAttachedRight
#define XmNtrailingAttachedRight        "trailingAttachedRight"
#endif
#ifndef XmNtrailingFixedColumns
#define XmNtrailingFixedColumns                "trailingFixedColumns"
#endif
#ifndef XmNtrailingFixedRows
#define XmNtrailingFixedRows                "trailingFixedRows"
#endif
#ifndef XmNleftColumn
#define XmNleftColumn                        "leftColumn"
#endif
#ifndef XmNtextShadowThickness
#define XmNtextShadowThickness                "textShadowThickness"
#endif
#ifndef XmNtraverseCellCallback
#define XmNtraverseCellCallback                "traverseCellCallback"
#endif
#ifndef XmNtraverseFixedCells
#define XmNtraverseFixedCells                "traverseFixedCells"
#endif
#ifndef XmNverticalScrollBarDisplayPolicy
#define XmNverticalScrollBarDisplayPolicy "verticalScrollBarDisplayPolicy"
#endif
#ifndef XmNvisibleColumns
#define XmNvisibleColumns                "visibleColumns"
#endif
#ifndef XmNvisibleRows
#define XmNvisibleRows                        "visibleRows"
#endif
#ifndef XmNwriteCellCallback
#define XmNwriteCellCallback                "writeCellCallback"
#endif


#ifndef XmCAlignments
#define XmCAlignments                        "Alignments"
#endif
#ifndef XmCAltRowCount
#define XmCAltRowCount                        "AltRowCount"
#endif
#ifndef XmCBoldLabels
#define XmCBoldLabels                        "BoldLabels"
#endif
#ifndef XmCButtonLabels
#define XmCButtonLabels                        "ButtonLabels"
#endif
#ifndef XmCCalcCursorPosition
#define XmCCalcCursorPosition                "CalcCursorPosition"
#endif
#ifndef XmCCells
#define XmCCells                        "Cells"
#endif
#ifndef XmCCellShadowTypes
#define XmCCellShadowTypes                "CellShadowTypes"
#endif
#ifndef XmCCellUserData
#define XmCCellUserData                        "CellUserData"
#endif
#if CELL_WIDGETS
#ifndef XmCCellWidgets
#define XmCCellWidgets                        "CellWidgets"
#endif
#endif
#ifndef XmCColors
#define XmCColors                        "Colors"
#endif
#ifndef XmCColumnMaxLengths
#define XmCColumnMaxLengths                "ColumnMaxLengths"
#endif
#ifndef XmCColumnResize
#define XmCColumnResize                        "ColumnResize"
#endif
#ifndef XmCColumnWidths
#define XmCColumnWidths                        "ColumnWidths"
#endif
#ifndef XmCFill
#define XmCFill                                "Fill"
#endif
#ifndef XmCFixedColumns
#define XmCFixedColumns                        "FixedColumns"
#endif
#ifndef XmCFixedRows
#define XmCFixedRows                        "FixedRows"
#endif
#ifndef XmCGridType
#define XmCGridType                        "GridType"
#endif
#if XmVersion >= 1002
#ifndef XmCHighlightedCells
#define XmCHighlightedCells                "HighlightedCells"
#endif
#endif
#ifndef XmCLabels
#define XmCLabels                        "Labels"
#endif
#ifndef XmCLeftColumn
#define XmCLeftColumn                        "LeftColumn"
#endif
#ifndef XmCMatrixScrollBarDisplayPolicy
#define XmCMatrixScrollBarDisplayPolicy        "MatrixScrollBarDisplayPolicy"
#endif
#ifndef XmCReverseSelect
#define XmCReverseSelect                "ReverseSelect"
#endif
#ifndef XmCRowLabelWidth
#define XmCRowLabelWidth                "RowLabelWidth"
#endif
#ifndef XmCSelectedCells
#define XmCSelectedCells                "SelectedCells"
#endif
#ifndef XmCSelectScrollVisible
#define XmCSelectScrollVisible                "SelectScrollVisible"
#endif
#ifndef XmCShadowTypes
#define XmCShadowTypes                        "ShadowTypes"
#endif
#ifndef XmCTextBackground
#define XmCTextBackground                "TextBackground"
#endif
#ifndef XmCTextField
#define XmCTextField                        "TextField"
#endif
#ifndef XmCTextShadowThickness
#define XmCTextShadowThickness                "TextShadowThickness"
#endif
#ifndef XmCTraverseFixedCells
#define XmCTraverseFixedCells                "TraverseFixedCells"
#endif
#ifndef XmCTopRow
#define XmCTopRow                        "TopRow"
#endif
#ifndef XmCTrailingAttachedBottom
#define XmCTrailingAttachedBottom        "TrailingAttachedBottom"
#endif
#ifndef XmCTrailingAttachedRight
#define XmCTrailingAttachedRight        "TrailingAttachedRight"
#endif
#ifndef XmCTrailingFixedColumns
#define XmCTrailingFixedColumns                "TrailingFixedColumns"
#endif
#ifndef XmCTrailingFixedRows
#define XmCTrailingFixedRows                "TrailingFixedRows"
#endif
#ifndef XmCUserDatas
#define XmCUserDatas                        "UserDatas"
#endif
#ifndef XmCVisibleColumns
#define XmCVisibleColumns                "VisibleColumns"
#endif
#ifndef XmCVisibleRows
#define XmCVisibleRows                        "VisibleRows"
#endif

#ifndef XmRStringArray
#define XmRStringArray                        "StringArray"
#endif
#ifndef XmRBooleanArray
#define XmRBooleanArray                        "BooleanArray"
#endif
#ifndef XmRAlignmentArray
#define XmRAlignmentArray                "AlignmentArray"
#endif
#ifndef XmRBooleanTable
#define XmRBooleanTable                        "BooleanTable"
#endif
#ifndef XmRCellTable
#define XmRCellTable                        "CellTable"
#endif
#ifndef XmRWidgetTable
#define XmRWidgetTable                        "WidgetTable"
#endif
#ifndef XmRGridType
#define XmRGridType                        "GridType"
#endif
#if XmVersion >= 1002
#ifndef XmRHighlightTable
#define XmRHighlightTable                "HighlightTable"
#endif
#endif
#ifndef XmRMatrixScrollBarDisplayPolicy
#define XmRMatrixScrollBarDisplayPolicy "MatrixScrollBarDisplayPolicy"
#endif
#ifndef XmRMaxLengthArray
#define XmRMaxLengthArray                "MaxLengthArray"
#endif
#ifndef XmRPixelTable
#define XmRPixelTable                        "PixelTable"
#endif
#ifndef XmRShadowTypeTable
#define XmRShadowTypeTable                "ShadowTypeTable"
#endif
#ifndef XmRShadowTypeArray
#define XmRShadowTypeArray                "ShadowTypeArray"
#endif
#ifndef XmRUserDataTable
#define XmRUserDataTable                "UserDataTable"
#endif
#ifndef XmRUserDataArray
#define XmRUserDataArray                "UserDataArray"
#endif
#ifndef XmRWidthArray
#define XmRWidthArray                        "WidthArray"
#endif


#ifndef XbaeIsXbaeMatrix
#define XbaeIsXbaeMatrix( w)        XtIsSubclass(w, xbaeMatrixWidgetClass)
#endif /* XbaeIsXbaeMatrix */

/* Class record constants */

externalref WidgetClass xbaeMatrixWidgetClass;

typedef struct _XbaeMatrixClassRec *XbaeMatrixWidgetClass;
typedef struct _XbaeMatrixRec *XbaeMatrixWidget;

/*
 * Prototype wrapper
 */
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
 * External interfaces to class methods
 */
_XFUNCPROTOBEGIN

extern void XbaeMatrixAddColumns P((Widget, int, String *, String *, short *,
                                     int *, unsigned char *, unsigned char *,
                                     Pixel *, int));
extern void XbaeMatrixAddRows P((Widget,  int , String *, String *,
                                  Pixel *, int));
extern void XbaeMatrixCancelEdit P((Widget, Boolean));
extern Boolean XbaeMatrixCommitEdit P((Widget, Boolean));
extern void XbaeMatrixDeleteColumns P((Widget, int, int));
extern void XbaeMatrixDeleteRows P((Widget, int, int));
extern void XbaeMatrixDeselectAll P((Widget));
extern void XbaeMatrixDeselectCell P((Widget, int, int));
extern void XbaeMatrixDeselectColumn P((Widget, int));
extern void XbaeMatrixDeselectRow P((Widget, int));
extern void XbaeMatrixEditCell P((Widget, int, int));
extern void XbaeMatrixFirstSelectedCell P((Widget, int *, int *));
extern int XbaeMatrixFirstSelectedColumn P((Widget));
extern int XbaeMatrixFirstSelectedRow P((Widget));
extern String XbaeMatrixGetCell P((Widget, int, int));
extern XtPointer XbaeMatrixGetCellUserData P((Widget, int, int));
extern XtPointer XbaeMatrixGetColumnUserData P((Widget, int));
extern void XbaeMatrixGetCurrentCell P((Widget, int *, int *));
extern int XbaeMatrixGetEventRowColumn P((Widget, XEvent *, int *, int *));
extern Boolean XbaeMatrixEventToXY P((Widget, XEvent *, int *, int *));
extern Boolean XbaeMatrixRowColToXY P((Widget, int, int, int *, int *));
extern int XbaeMatrixGetNumSelected P((Widget));
extern XtPointer XbaeMatrixGetRowUserData P((Widget, int));
extern Boolean XbaeMatrixIsCellSelected P((Widget, int, int));
extern Boolean XbaeMatrixIsColumnSelected P((Widget, int));
extern Boolean XbaeMatrixIsRowSelected P((Widget, int));
extern void XbaeMatrixRefresh P((Widget));
extern void XbaeMatrixRefreshCell P((Widget, int, int));
extern void XbaeMatrixRefreshColumn P((Widget, int));
extern void XbaeMatrixRefreshRow P((Widget, int));
extern void XbaeMatrixSelectAll P((Widget));
extern void XbaeMatrixSelectCell P((Widget, int, int));
extern void XbaeMatrixSelectColumn P((Widget, int));
extern void XbaeMatrixSelectRow P((Widget, int));
#if XmVersion >= 1002
extern void XbaeMatrixHighlightCell P((Widget, int, int));
extern void XbaeMatrixHighlightRow P((Widget, int));
extern void XbaeMatrixHighlightColumn P((Widget, int));
extern void XbaeMatrixUnhighlightCell P((Widget, int, int));
extern void XbaeMatrixUnhighlightRow P((Widget, int));
extern void XbaeMatrixUnhighlightColumn P((Widget, int));
extern void XbaeMatrixUnhighlightAll P((Widget));
#endif
extern void XbaeMatrixSetCell P((Widget, int, int, const String));
extern void XbaeMatrixSetCellBackground P((Widget, int, int, Pixel));
extern void XbaeMatrixSetCellColor P((Widget, int, int, Pixel));
extern void XbaeMatrixSetCellUserData P((Widget, int, int, XtPointer));
#if CELL_WIDGETS
extern void XbaeMatrixSetCellWidget P((Widget, int, int, Widget));
#endif
extern void XbaeMatrixSetColumnBackgrounds P((Widget, int, Pixel *, int));
extern void XbaeMatrixSetColumnColors P((Widget, int, Pixel *, int));
extern void XbaeMatrixSetColumnUserData P((Widget, int, XtPointer));
extern void XbaeMatrixSetRowBackgrounds P((Widget, int, Pixel *, int));
extern void XbaeMatrixSetRowColors P((Widget, int , Pixel *, int));
extern void XbaeMatrixSetRowUserData P((Widget, int, XtPointer));
extern int XbaeMatrixVisibleColumns P((Widget));
extern int XbaeMatrixVisibleRows P((Widget));
extern int XbaeMatrixNumColumns P((Widget));
extern int XbaeMatrixNumRows P((Widget));
extern void XbaeMatrixDisableRedisplay P((Widget));
extern void XbaeMatrixEnableRedisplay P((Widget, Boolean));
extern void XbaeMatrixMakeCellVisible P((Widget, int, int));
extern Boolean XbaeMatrixIsRowVisible P((Widget, int));
extern Boolean XbaeMatrixIsColumnVisible P((Widget, int));
extern Boolean XbaeMatrixIsCellVisible P((Widget, int, int));
extern void XbaeMatrixVisibleCells P((Widget, int *, int *, int *, int *));
extern String XbaeMatrixGetColumnLabel P((Widget, int));
extern String XbaeMatrixGetRowLabel P((Widget, int));
extern void XbaeMatrixSetColumnLabel P((Widget, int, String));
extern void XbaeMatrixSetRowLabel P((Widget, int, String));
extern Widget XbaeCreateMatrix P((Widget, String, ArgList, Cardinal));
_XFUNCPROTOEND

typedef unsigned char        Alignment;
typedef Alignment *        AlignmentArray;
typedef String *        StringTable;
typedef short                 Width;
typedef Width *                WidthArray;
typedef int                 MaxLength;
typedef MaxLength *        MaxLengthArray;

/*
 * cell shadow types
 */
 
enum
{
    XmGRID_NONE                    = 0x00,
    XmGRID_CELL_LINE            = 0x02,
    XmGRID_CELL_SHADOW            = 0x03,
    XmGRID_ROW_LINE            = 0x04,
    XmGRID_ROW_SHADOW            = 0x05,
    XmGRID_COLUMN_LINE            = 0x08,
    XmGRID_COLUMN_SHADOW    = 0x09,

    /* Deprecated types. Use will cause
     * a run-time warning to be issued. */
    XmGRID_LINE                    = 0x20,
    XmGRID_SHADOW_IN            = 0x40,
    XmGRID_SHADOW_OUT            = 0x80
};


/*
 * Enumeration for Matrix ScrollBar Display Policy
 */
enum
{
    XmDISPLAY_NONE,
    XmDISPLAY_AS_NEEDED,
    XmDISPLAY_STATIC
};


/*
 * Enumeration for type of a cell
 */
typedef enum {
    FixedCell, NonFixedCell, RowLabelCell, ColumnLabelCell
} CellType;

#if XmVersion >= 1002
/*
 * Enumeration for highlight reason/location
 */
enum {
    HighlightNone        = 0x0000,
    HighlightCell        = 0x0001,
    HighlightRow        = 0x0002,
    HighlightColumn        = 0x0004,
    HighlightOther        = 0x0008,
    UnhighlightCell        = 0x0010,
    UnhighlightRow        = 0x0020,
    UnhighlightColumn        = 0x0040,
    UnhighlightAll        = UnhighlightCell | UnhighlightRow | UnhighlightColumn
};
#endif

/*
 * Callback reasons.  Try to stay out of range of the Motif XmCR_* reasons.
 */
typedef enum _XbaeReasonType
{
    XbaeModifyVerifyReason = 102,
    XbaeEnterCellReason,
    XbaeLeaveCellReason,
    XbaeTraverseCellReason,
    XbaeSelectCellReason,
    XbaeDrawCellReason,
    XbaeWriteCellReason,
    XbaeResizeReason,
    XbaeResizeColumnReason,
    XbaeDefaultActionReason,
    XbaeProcessDragReason,
    XbaeLabelActivateReason
}
XbaeReasonType;

/*
 * DrawCell types.
 */
typedef enum
{
    XbaeString=1,
    XbaePixmap
}
XbaeCellType;

/*
 * The 'Any' struct which can be used in callbacks used with different
 * Callback structs but only need to access its 4 members
 */
typedef struct _XbaeMatrixAnyCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
}
XbaeMatrixAnyCallbackStruct;
    
/*
 * Struct passed to modifyVerifyCallback
 */
typedef struct _XbaeMatrixModifyVerifyCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    XmTextVerifyCallbackStruct *verify;
    const char *prev_text;
}
XbaeMatrixModifyVerifyCallbackStruct;

/*
 * Struct passed to enterCellCallback
 */
typedef struct _XbaeMatrixEnterCellCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    int position;
    String pattern;
    Boolean auto_fill;
    Boolean convert_case;
    Boolean overwrite_mode;
    Boolean select_text;
    Boolean map;
    Cardinal num_params;
    String *params;
    Boolean doit;

}
XbaeMatrixEnterCellCallbackStruct;

/*
 * Struct passed to leaveCellCallback
 */
typedef struct _XbaeMatrixLeaveCellCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    String value;
    Boolean doit;
}
XbaeMatrixLeaveCellCallbackStruct;

/*
 * Struct passed to traverseCellCallback
 */
typedef struct _XbaeMatrixTraverseCellCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    int next_row;
    int next_column;
    int fixed_rows;
    int fixed_columns;
    int trailing_fixed_rows;
    int trailing_fixed_columns;
    int num_rows;
    int num_columns;
    String param;
    XrmQuark qparam;
}
XbaeMatrixTraverseCellCallbackStruct;

/*
 * Struct passed to selectCellCallback
 */
typedef struct _XbaeMatrixSelectCellCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    Boolean **selected_cells;
    String **cells;
    Cardinal num_params;
    String *params;
}
XbaeMatrixSelectCellCallbackStruct;

/*
 * Struct passed to drawCellCallback
 */
typedef struct _XbaeMatrixDrawCellCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    int width;
    int height;
    XbaeCellType type;
    String string;
    Pixmap pixmap;
    Pixmap mask;
    Pixel foreground;
    Pixel background;
    int depth;
}
XbaeMatrixDrawCellCallbackStruct;

/*
 * Struct passed to writeCellCallback
 */
typedef struct _XbaeMatrixWriteCellCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    XbaeCellType type;
    String string;
    Pixmap pixmap;
    Pixmap mask;
}
XbaeMatrixWriteCellCallbackStruct;


/*
 * Struct passed to resizeCallback
 */
typedef struct _XbaeMatrixResizeCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    Dimension width;
    Dimension height;
}
XbaeMatrixResizeCallbackStruct;

/*
 * Struct passed to resizeColumnCallback
 *
 */
typedef struct _XbaeMatrixResizeColumnCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    int which;
    int columns;    
    short *column_widths;
}
XbaeMatrixResizeColumnCallbackStruct;

#if XmVersion > 1001
/*
 * Struct passed to processDragCallback
 */
typedef struct _XbaeMatrixProcessDragCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    String string;
    XbaeCellType type;
    Pixmap pixmap;
    Pixmap mask;
    Cardinal num_params;
    String *params;
}
XbaeMatrixProcessDragCallbackStruct;
#endif

/*
 * Struct passed to defaultActionCallback
 */
typedef struct _XbaeMatrixDefaultActionCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
}
XbaeMatrixDefaultActionCallbackStruct;

/*
 * Struct passed to labelActivateCallback
 */
typedef struct _XbaeMatrixLabelActivateCallbackStruct
{
    XbaeReasonType reason;
    XEvent *event;
    int row;
    int column;
    Boolean row_label;
    String label;
}
XbaeMatrixLabelActivateCallbackStruct;


/* provide clean-up for those with R4 */
#ifndef XlibSpecificationRelease
# undef _Xconst
# undef _XFUNCPROTOBEGIN
# undef _XFUNCPROTOEND
#endif

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _Xbae_Matrix_h */

