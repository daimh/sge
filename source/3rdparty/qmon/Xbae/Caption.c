/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
 * Copyright(c) 1999 Andrew Lister
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
 * CaptionWidget Author: Andrew Wason, Bellcore, aw@bae.bellcore.com
 * 
 * $Id: Caption.c,v 1.1 2001-07-18 11:05:59 root Exp $
 */

/*
 * Caption.c - displays a caption label next to it's child.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <X11/StringDefs.h>
#include <Xm/XmP.h>
#include <Xm/Label.h>
#include "Macros.h"
#include "CaptionP.h"

#ifndef tolower
#define tolower(c)      ((c) - 'A' + 'a')
#endif

#define OffsetOf(field)        XtOffsetOf(XbaeCaptionRec, field)

static XtResource resources[] = {
    { XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
          OffsetOf(caption.font_list), XmRImmediate, (XtPointer) NULL },
    { XmNlabelAlignment, XmCLabelAlignment, XmRLabelAlignment,
          sizeof(XbaeLabelAlignment), OffsetOf(caption.label_alignment),
          XmRImmediate, (XtPointer) XbaeAlignmentCenter },
    { XmNlabelOffset, XmCLabelOffset, XmRInt, sizeof(int),
          OffsetOf(caption.label_offset), XmRImmediate, (XtPointer) 0 },
    { XmNlabelPixmap, XmCLabelPixmap, XmRManForegroundPixmap, sizeof(Pixmap),
          OffsetOf(caption.label_pixmap),
          XmRImmediate, (XtPointer) XmUNSPECIFIED_PIXMAP },
    { XmNlabelPosition, XmCLabelPosition, XmRLabelPosition,
          sizeof(XbaeLabelPosition), OffsetOf(caption.label_position),
          XmRImmediate, (XtPointer) XbaePositionLeft },
    { XmNlabelString, XmCXmString, XmRXmString, sizeof(XmString),
          OffsetOf(caption.label_string), XmRImmediate, (XtPointer) NULL },
    { XmNlabelTextAlignment, XmCAlignment, XmRAlignment, sizeof(unsigned char),
          OffsetOf(caption.label_text_alignment),
          XmRImmediate, (XtPointer) XmALIGNMENT_CENTER },
    { XmNlabelType, XmCLabelType, XmRLabelType, sizeof(unsigned char),
          OffsetOf(caption.label_type), XmRImmediate, (XtPointer) XmSTRING },
};

#undef OffsetOf

/*
 * Macro to retrieve our label and the user's child.
 */
#define LabelChild(w)     (((CompositeWidget)w)->composite.children[0])
#define UserChild(w)      (((CompositeWidget)w)->composite.children[1])

#define HaveUserChild(w)  (((CompositeWidget)w)->composite.num_children > 1 \
                           && XtIsManaged(UserChild(w)))


/*
 * Declaration of methods
 */
static void ClassInitialize P((void));
static void Initialize P((Widget, Widget, ArgList, Cardinal *));
static void InsertChild P((Widget));
static Boolean SetValues P((Widget, Widget, Widget, ArgList, Cardinal *));
static void SetValuesAlmost P((Widget, Widget, XtWidgetGeometry *,
                               XtWidgetGeometry *));
static void GetValuesHook P((Widget, ArgList, Cardinal *));
static void Resize P((Widget));
static void ChangeManaged P((Widget));
static XtGeometryResult GeometryManager P((Widget, XtWidgetGeometry *,
                                           XtWidgetGeometry *));
static XtGeometryResult QueryGeometry P((Widget, XtWidgetGeometry *,
                                         XtWidgetGeometry *));

/*
 * Private functions
 */
static void ComputeSize P((XbaeCaptionWidget, Dimension *, Dimension *,
                            Dimension, Dimension, Dimension));
static void ComputeUserChildSize P((XbaeCaptionWidget, Dimension, Dimension,
                                     Dimension *, Dimension *, Dimension));

static void Layout P((XbaeCaptionWidget, Boolean));
static Boolean CompareStrings P((String, String));

/*
 * Type converters
 */
static Boolean CvtStringToLabelPosition P((Display *, XrmValuePtr, Cardinal *,
                                            XrmValuePtr, XrmValuePtr,
                                            XtPointer *));
static Boolean CvtStringToLabelAlignment P((Display *, XrmValuePtr,
                                             Cardinal *, XrmValuePtr,
                                             XrmValuePtr, XtPointer *));


XbaeCaptionClassRec xbaeCaptionClassRec = {
    {
    /* core_class fields */
        /* superclass                        */ (WidgetClass) &xmManagerClassRec,
        /* class_name                        */ "XbaeCaption",
        /* widget_size                        */ sizeof(XbaeCaptionRec),
        /* class_initialize                */ ClassInitialize,
        /* class_part_initialize        */ NULL,
        /* class_inited                        */ False,
        /* initialize                        */ (XtInitProc)Initialize,
        /* initialize_hook                */ NULL,
        /* realize                        */ XtInheritRealize,
        /* actions                        */ NULL,
        /* num_actions                        */ 0,
        /* resources                        */ resources,
        /* num_resources                */ XtNumber(resources),
        /* xrm_class                        */ NULLQUARK,
        /* compress_motion                */ True,
        /* compress_exposure                */ XtExposeCompressMaximal,
        /* compress_enterleave                */ True,
        /* visible_interest                */ False,
        /* destroy                        */ NULL,
        /* resize                        */ (XtWidgetProc)Resize,
        /* expose                        */ _XmRedisplayGadgets,
        /* set_values                        */ (XtSetValuesFunc)SetValues,
        /* set_values_hook                */ NULL,
        /* set_values_almost                */ (XtAlmostProc)SetValuesAlmost,
        /* get_values_hook                */ (XtArgsProc)GetValuesHook,
        /* accept_focus                        */ NULL,
        /* version                        */ XtVersion,
        /* callback_private                */ NULL,
        /* tm_table                        */ XtInheritTranslations,
        /* query_geometry                */ (XtGeometryHandler)QueryGeometry,
        /* display_accelerator                */ NULL,
        /* extension                        */ NULL
    },
    {
    /* composite_class fields */
        /* geometry_manager                */ GeometryManager,
        /* change_managed                */ (XtWidgetProc)ChangeManaged,
        /* insert_child                        */ InsertChild,
        /* delete_child                        */ XtInheritDeleteChild,
        /* extension                        */ NULL,
    },
    {
    /* constraint_class fields */
        /* resources                        */ NULL,
        /* num_resources                */ 0,
        /* constraint_size                */ 0,
        /* initialize                        */ NULL,
        /* destroy                        */ NULL,
        /* set_values                        */ NULL,
        /* extension                        */ NULL
    },
    {
    /* manager_class fields */
        /* translations                        */  XtInheritTranslations,
        /* syn_resources                */  NULL,
        /* num_syn_resources                */  0,
        /* syn_constraint_resources        */  NULL,
        /* num_syn_constraint_resources */  0,
        /* parent_process                */  XmInheritParentProcess,
        /* extension                        */  NULL
    },
    {
    /* caption_class fields */
        /* extension                        */ NULL,
    }
};

WidgetClass xbaeCaptionWidgetClass = (WidgetClass)&xbaeCaptionClassRec;


static void
ClassInitialize()
{
    XtSetTypeConverter(XmRString, XmRLabelAlignment,
                       CvtStringToLabelAlignment, NULL, 0,
                       XtCacheAll, NULL);

    XtSetTypeConverter(XmRString, XmRLabelPosition,
                       CvtStringToLabelPosition, NULL, 0,
                       XtCacheAll, NULL);
}

/* ARGSUSED */
static void
Initialize(req, n, args, num_args)
Widget req, n;
ArgList args;
Cardinal *num_args;
{
    XbaeCaptionWidget new = (XbaeCaptionWidget)n;
    /*
     * Validate our labelPosition
     */
    switch (new->caption.label_position) {
    case XbaePositionLeft:
    case XbaePositionRight:
    case XbaePositionTop:
    case XbaePositionBottom:
        break;
    default:
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget)new),
            "initialize", "badLabelPosition", "XbaeCaption",
            "XbaeCaption: Invalid label position.",
            (String *) NULL, (Cardinal *) NULL);
        new->caption.label_position = XbaePositionLeft;
        break;
    }

    /*
     * Validate our labelAlignment
     */
    switch (new->caption.label_alignment) {
    case XbaeAlignmentTopOrLeft:
    case XbaeAlignmentCenter:
    case XbaeAlignmentBottomOrRight:
        break;
    default:
        XtAppWarningMsg(
            XtWidgetToApplicationContext((Widget)new),
            "initialize", "badLabelAlignment", "XbaeCaption",
            "XbaeCaption: Invalid label alignment.",
            (String *) NULL, (Cardinal *) NULL);
        new->caption.label_alignment = XbaeAlignmentCenter;
    }


    /*
     * Create the label with our name, so if no labelString is specified,
     * it will use our name as it's label.
     */
    (void)
        XtVaCreateManagedWidget(XtName((Widget)new),
                                xmLabelWidgetClass, (Widget)new,
                                XmNbackground,        new->core.background_pixel,
                                XmNforeground,        new->manager.foreground,
                                XmNfontList,        new->caption.font_list,
                                XmNlabelType,        new->caption.label_type,
                                XmNalignment,
                                        new->caption.label_text_alignment,
                                XmNlabelString,        new->caption.label_string,
                                XmNlabelPixmap,        new->caption.label_pixmap,
                                XmNborderWidth,        0,
                                NULL);

    /*
     * The label makes a copy of these, so NULL them out
     * (we don't want to point to the user's data).
     * get_values_hook will handle a get_values on these.
     */
    new->caption.label_string = NULL;
    new->caption.font_list = NULL;

    /*
     * We are the same size as our label.  We ignore user specified
     * width/height.
     */
    new->core.width = LabelChild(new)->core.width;
    new->core.height = LabelChild(new)->core.height;
}

static void
InsertChild(w)
Widget w;
{
    if (((CompositeWidget)XtParent(w))->composite.num_children > 1) {
        XtAppWarningMsg(
            XtWidgetToApplicationContext(w),
            "insertChild", "badChild", "XbaeCaption",
            "XbaeCaption: Cannot add more than one child.",
            (String *)NULL, (Cardinal *)NULL);
        return;
    }

    (*((CompositeWidgetClass)
       (xbaeCaptionWidgetClass->core_class.superclass))->composite_class.
     insert_child) (w);
}


/* ARGSUSED */
static Boolean
SetValues(cur, req, nw, args, num_args)
Widget cur, req, nw;
ArgList args;
Cardinal *num_args;
{
    XbaeCaptionWidget current = (XbaeCaptionWidget)cur;
    XbaeCaptionWidget new = (XbaeCaptionWidget)nw;
    Dimension old_label_width, old_label_height;
    Boolean layout = False;
    int n;
    Arg largs[7];

#define NE(field)       (current->field != new->field)
#define EQ(field)       (current->field == new->field)

    /*
     * Validate our labelPosition
     */
    if (NE(caption.label_position)) {
        switch (new->caption.label_position) {
        case XbaePositionLeft:
        case XbaePositionRight:
        case XbaePositionTop:
        case XbaePositionBottom:
            break;
        default:
            XtAppWarningMsg(
                XtWidgetToApplicationContext((Widget)new),
                "setValues", "badLabelPosition", "XbaeCaption",
                "XbaeCaption: Invalid label position.",
                (String *) NULL, (Cardinal *) NULL);
            new->caption.label_position = current->caption.label_position;
        }
    }

    /*
     * Validate our labelAlignment
     */
    if (NE(caption.label_alignment)) {
        switch (new->caption.label_alignment) {
        case XbaeAlignmentTopOrLeft:
        case XbaeAlignmentCenter:
        case XbaeAlignmentBottomOrRight:
            break;
        default:
            XtAppWarningMsg(
                XtWidgetToApplicationContext((Widget)new),
                "setValues", "badLabelAlignment", "XbaeCaption",
                "XbaeCaption: Invalid label alignment.",
                (String *) NULL, (Cardinal *) NULL);
            new->caption.label_alignment = current->caption.label_alignment;
        }
    }

    /*
     * Save labels size in case XtSetValues changes it below.
     */
    old_label_width = LabelChild(new)->core.width;
    old_label_height = LabelChild(new)->core.height;

    /*
     * Pass through resources to our label.  Our geometry_manager
     * will let it change size if it needs to.
     */
    n = 0;
    if (NE(caption.label_type)) {
        XtSetArg(largs[n], XmNlabelType,new->caption.label_type);        n++;
    }
    if (NE(caption.label_text_alignment)) {
        XtSetArg(largs[n], XmNalignment,
                 new->caption.label_text_alignment);                        n++;
    }
    if (NE(caption.label_string)) {
        XtSetArg(largs[n], XmNlabelString, new->caption.label_string);        n++;
    }
    if (NE(caption.label_pixmap)) {
        XtSetArg(largs[n], XmNlabelPixmap, new->caption.label_pixmap);        n++;
    }
    if (NE(caption.font_list)) {
        XtSetArg(largs[n], XmNfontList, new->caption.font_list);        n++;
    }
    if (NE(core.background_pixel)) {
        XtSetArg(largs[n], XmNbackground, new->core.background_pixel);        n++;
    }
    if (NE(manager.foreground)) {
        XtSetArg(largs[n], XmNforeground, new->manager.foreground);        n++;
    }
    if (n) {
        XtSetValues(LabelChild(new), largs, n);

        /*
         * The label makes a copy of these, so NULL them out
         * (we don't want to point to the user's data).
         * get_values_hook will handle a get_values on these.
         */
        new->caption.label_string = NULL;
        new->caption.font_list = NULL;
    }

    /*
     * Calculate a new size if the label changed size, or if offset changed,
     * or if alignment or position changed in a way which requires a new
     * size.  Our resize or set_values_almost methods will lay things out.
     */
    if (old_label_width != LabelChild(new)->core.width ||
        old_label_height != LabelChild(new)->core.height ||
        NE(caption.label_offset) || NE(caption.label_position)) {

        if (!HaveUserChild(new)) {
            new->core.width = LabelChild(new)->core.width;
            new->core.height = LabelChild(new)->core.height;
        }
        else
            ComputeSize(new, &new->core.width, &new->core.height,
                        UserChild(new)->core.width,
                        UserChild(new)->core.height,
                        UserChild(new)->core.border_width);

        /*
         * If, after all that, our size didn't change, then we need
         * to layout (since resize and set_values_almost won't be called).
         */
        if (EQ(core.width) && EQ(core.height))
            layout = True;
    }

    /*
     * If label alignment changed, but our size didn't, then we need to layout
     * (since resize and set_values_almost won't be called).
     */
    if (NE(caption.label_alignment) && EQ(core.width) && EQ(core.height))
        layout = True;

    if (layout)
        Layout(new, True);

    return False;

#undef EQ
#undef NE
}

/* ARGSUSED */
static void
SetValuesAlmost(o, n, request, reply)
Widget o;
Widget n;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
{
    XbaeCaptionWidget new = (XbaeCaptionWidget)n;
    /*
     * If XtGeometryAlmost, accept compromize - our resize method
     * will take care of it.
     */
    if (reply->request_mode)
        *request = *reply;

    /*
     * If XtGeometryNo, then layout if it was a size change that was denied,
     * and accept the original geometry.
     */
    else {
        if (request->request_mode & CWWidth ||
            request->request_mode & CWHeight)
            Layout(new, True);

        request->request_mode = 0;
    }
}

static void
GetValuesHook(w, args, num_args)
Widget w;
ArgList args;
Cardinal *num_args;
{
    XbaeCaptionWidget cw = (XbaeCaptionWidget)w;
    int i;

    /*
     * We don't save a copy of the label_string or font_list.
     * If the user wants these, we get them from the label widget.
     */
    for (i = 0; i < *num_args; i++)
        if (strcmp(args[i].name, XmNlabelString) == 0)
            XtGetValues(LabelChild(cw), &args[i], 1);
        else if (strcmp(args[i].name, XmNfontList) == 0)
            XtGetValues(LabelChild(cw), &args[i], 1);
}

/*
 * Do not configure the user's child if configure is False.
 * This is for when we are laying out from within our GeometryManager.
 * Since we are following a XtGeometryYes policy, we shouldn't config
 * the initiating child (the label never initiates)
 */
static void
#if NeedFunctionPrototypes
Layout(XbaeCaptionWidget cw, Boolean configure)
#else
Layout(cw, configure)
XbaeCaptionWidget cw;
Boolean configure;
#endif
{
    Position label_x = 0, label_y = 0;
    Position user_x = 0, user_y = 0;
    Dimension user_width, user_height;

    /*
     * If we only have the label, position it at 0,0.
     */
    if (!HaveUserChild(cw)) {
        XtMoveWidget(LabelChild(cw), 0, 0);
        return;
    }

    /*
     * Calculate the positions of our label and user's children.
     */
    switch (cw->caption.label_position) {

    case XbaePositionLeft:
    case XbaePositionRight:
        switch (cw->caption.label_alignment) {
        case XbaeAlignmentTopOrLeft:
            label_y = 0;
            break;
        case XbaeAlignmentCenter:
            label_y = (int) (cw->core.height / 2) -
                (int) (LabelChild(cw)->core.height / 2);
            break;
        case XbaeAlignmentBottomOrRight:
            label_y = (int)cw->core.height - (int)LabelChild(cw)->core.height;
            break;
        }
        user_y = 0;
        break;

    case XbaePositionTop:
    case XbaePositionBottom:
        switch (cw->caption.label_alignment) {
        case XbaeAlignmentTopOrLeft:
            label_x = 0;
            break;
        case XbaeAlignmentCenter:
            label_x = (int) (cw->core.width / 2) -
                (int) (LabelChild(cw)->core.width / 2);
            break;
        case XbaeAlignmentBottomOrRight:
            label_x = (int)cw->core.width - (int)LabelChild(cw)->core.width;
            break;
        }
        user_x = 0;
        break;
    }


    /*
     * Calculate the positions of our label and user's children.
     */
    switch (cw->caption.label_position) {

    case XbaePositionLeft:
        if ((int)LabelChild(cw)->core.width + cw->caption.label_offset > 0) {
            label_x = 0;
            user_x = (int)LabelChild(cw)->core.width +
                cw->caption.label_offset;
        }
        else {
            label_x = -cw->caption.label_offset;
            user_x = 0;
        }
        break;

    case XbaePositionRight:
        if ((int)LabelChild(cw)->core.width + cw->caption.label_offset > 0)
            label_x = (int)cw->core.width - (int)LabelChild(cw)->core.width;
        else
            label_x = (int)cw->core.width - ((int)LabelChild(cw)->core.width -
                                             cw->caption.label_offset);
        user_x = 0;
        break;

    case XbaePositionTop:
        if ((int)LabelChild(cw)->core.height + cw->caption.label_offset > 0) {
            label_y = 0;
            user_y = (int)LabelChild(cw)->core.height +
                cw->caption.label_offset;
        }
        else {
            label_y = -cw->caption.label_offset;
            user_y = 0;
        }
        break;

    case XbaePositionBottom:
        user_y = 0;
        label_y = cw->core.height - LabelChild(cw)->core.height;

        if ((int)LabelChild(cw)->core.height + cw->caption.label_offset > 0)
            label_y = (int)cw->core.height - (int)LabelChild(cw)->core.height;
        else
            label_y = (int)cw->core.height -
                ((int)LabelChild(cw)->core.height - cw->caption.label_offset);
        user_y = 0;
        break;
    }

    /*
     * Position our label widget.
     */
    XtMoveWidget(LabelChild(cw), label_x, label_y);


    if (configure) {
        /*
         * Calculate the size of the user's child widget.
         */
        ComputeUserChildSize(cw, cw->core.width, cw->core.height,
                             &user_width, &user_height,
                             UserChild(cw)->core.border_width);

        _XmConfigureObject((Widget)UserChild(cw),
                           (int)user_x, (int)user_y,
                           (int)user_width, (int)user_height,
                           (int)UserChild(cw)->core.border_width);
    }
}

static void
Resize(w)
Widget w;
{
    Layout((XbaeCaptionWidget)w, True);
}

/*
 * Given a width/height for the caption widget and the border width
 * of the user's child, compute the width and height of the user's child.
 */
static void
#if NeedFunctionPrototypes
ComputeUserChildSize(XbaeCaptionWidget cw, Dimension cwWidth,
                     Dimension cwHeight, Dimension *childWidth,
                     Dimension *childHeight, Dimension childBorderWidth)
#else
ComputeUserChildSize(cw, cwWidth, cwHeight, childWidth, childHeight,
                     childBorderWidth)
XbaeCaptionWidget cw;
Dimension cwWidth;
Dimension cwHeight;
Dimension *childWidth;
Dimension *childHeight;
Dimension childBorderWidth;
#endif
{
    int width = cwWidth - 2 * childBorderWidth;
    int height = cwHeight - 2 * childBorderWidth;

    /*
     * Remember, cw->caption.label_offset can be negative.
     * If the label width plus the offset is positive, then the label
     * is off the edge of the user's child, so we subtract that space
     * from the user's child space.  Otherwise the label is offset into
     * the space of the user's child widget, so the user's child gets
     * the full space and the label will be on top of it (or off the opposite
     * side).
     */

    switch (cw->caption.label_position) {
    case XbaePositionLeft:
    case XbaePositionRight:
        if ((int)LabelChild(cw)->core.width + cw->caption.label_offset > 0)
            width -= (int)LabelChild(cw)->core.width +
                cw->caption.label_offset;
        break;
    case XbaePositionTop:
    case XbaePositionBottom:
        if ((int)LabelChild(cw)->core.height + cw->caption.label_offset > 0)
            height -= (int)LabelChild(cw)->core.height +
                cw->caption.label_offset;
        break;
    }

    if (width <= 0)
        *childWidth = 1;
    else
        *childWidth = width;

    if (height <= 0)
        *childHeight = 1;
    else
        *childHeight = height;
}

/*
 * Compute our size, taking into account the sizes of
 * both of our children (the user's child size is passed in; we use the
 * current size of the label child)
 */
static void
#if NeedFunctionPrototypes
ComputeSize(XbaeCaptionWidget cw, Dimension *cwWidth, Dimension *cwHeight,
            Dimension childWidth, Dimension childHeight,
            Dimension childBorderWidth)
#else
ComputeSize(cw, cwWidth, cwHeight, childWidth, childHeight, childBorderWidth)
XbaeCaptionWidget cw;
Dimension *cwWidth;
Dimension *cwHeight;
Dimension childWidth;
Dimension childHeight;
Dimension childBorderWidth;
#endif
{
    childWidth += 2 * childBorderWidth;
    childHeight += 2 * childBorderWidth;

    /*
     * Remember, cw->caption.label_offset can be negative.
     */

    switch (cw->caption.label_position) {
    case XbaePositionRight:
    case XbaePositionLeft:
        if ((int)LabelChild(cw)->core.width + cw->caption.label_offset > 0)
            *cwWidth = childWidth + LabelChild(cw)->core.width +
                cw->caption.label_offset;
        else
            *cwWidth = childWidth;

        *cwHeight = childHeight > LabelChild(cw)->core.height
                            ? childHeight
                        : LabelChild(cw)->core.height;
        break;

    case XbaePositionTop:
    case XbaePositionBottom:
        if ((int)LabelChild(cw)->core.height + cw->caption.label_offset > 0)
            *cwHeight = childHeight + LabelChild(cw)->core.height +
                cw->caption.label_offset;
        else
            *cwHeight = childHeight;

        *cwWidth = childWidth > LabelChild(cw)->core.width
                            ? childWidth
                        : LabelChild(cw)->core.width;
        break;
    }
}

static void
ChangeManaged(w)
Widget w;
{
    XbaeCaptionWidget cw = (XbaeCaptionWidget)w;
    Dimension width, height;
    XtGeometryResult result;

    /*
     * Figure out what size we want to be.  If we don't have a user child,
     * we just want to be as big as the label.  Otherwise we must
     * take the label and user child into account.
     */
    if (!HaveUserChild(cw)) {
        width = LabelChild(cw)->core.width;
        height = LabelChild(cw)->core.height;
    }
    else {
        ComputeSize(cw, &width, &height,
                    UserChild(cw)->core.width, UserChild(cw)->core.height,
                    UserChild(cw)->core.border_width);
    }

    /*
     * If our calculated size is not our current size,
     * then request our calculated size.
     */
    if (width != cw->core.width || height != cw->core.height) {
        do {
            result = XtMakeResizeRequest((Widget)cw, width, height,
                                         &width, &height);
        } while (result == XtGeometryAlmost);
    }

    /*
     * Layout for the new configuration
     */
    Layout(cw, True);
}

/* ARGSUSED */
static XtGeometryResult
GeometryManager(w, desired, allowed)
Widget w;
XtWidgetGeometry *desired, *allowed;
{
    XbaeCaptionWidget cw = (XbaeCaptionWidget) XtParent(w);
    Dimension save_width, save_height, save_border_width;

#define Wants(flag) (desired->request_mode & flag)

    /*
     * If this is our label widget child, and it is querying, then return
     * Yes since we always grant the labels requests.
     */
    if (w == LabelChild(cw) && Wants(XtCWQueryOnly))
        return XtGeometryYes;

    /*
     * Disallow position-only changes for the user's child.
     */
    if (w == UserChild(cw) &&
        !Wants(CWWidth) && !Wants(CWHeight) && !Wants(CWBorderWidth))
        return XtGeometryNo;

    /*
     * Save the childs current geometry in case we have to back it out.
     */
    save_width = w->core.width;
    save_height = w->core.height;
    save_border_width = w->core.border_width;

    /*
     * Store the childs desired geometry into it's widget record.
     */
    if (Wants(CWWidth))
        w->core.width = desired->width;
    if (Wants(CWHeight))
        w->core.height = desired->height;
    if (Wants(CWBorderWidth))
        w->core.border_width = desired->border_width;

    /*
     * If this is our label widget child, then return Yes.  We stored the
     * changes into the widget above (except for position which we do now),
     * so Xt will reconfigure the label.
     * We let our label widget do whatever it wants since we control
     * when it requests a new size in our set_values.
     */
    if (w == LabelChild(cw)) {
        if (Wants(CWX))
            w->core.x = desired->x;
        if (Wants(CWY))
            w->core.y = desired->y;
        return XtGeometryYes;
    }

    /*
     * Otherwise this must be our user's child widget.
     * We will attempt to resize to accomodate it.
     */
    else {
        XtWidgetGeometry request;
        XtGeometryResult result;

        /*
         * Compute the size we want to be based on the new geometry
         * stored in the user's child above.
         */
        ComputeSize(cw, &request.width, &request.height,
                    w->core.width, w->core.height, w->core.border_width);

        /*
         * If our calculated size is not our current size,
         * then request our calculated size.
         */
        if (request.width != cw->core.width ||
            request.height != cw->core.height) {
            request.request_mode = 0;
            if (request.width != cw->core.width)
                request.request_mode |= CWWidth;
            if (request.height != cw->core.height)
                request.request_mode |= CWHeight;
            if (Wants(XtCWQueryOnly))
                request.request_mode |= XtCWQueryOnly;
            do {
                result = XtMakeGeometryRequest((Widget)cw, &request, &request);
            } while (result == XtGeometryAlmost);

            /*
             * If our request was granted, we need to layout (we are assuming
             * our parent implements an XtGeometryYes policy and not
             * XtGeometryDone)
             */
            if (result == XtGeometryYes && !Wants(XtCWQueryOnly))
                Layout(cw, False);
        }
        else
            result = XtGeometryYes;

        /*
         * A Yes result means we either got the size we wanted, or we agreed
         * to a compromise.
         */
        if (result == XtGeometryYes) {
            Dimension childWidth, childHeight;

            /*
             * Compute the size of the user's child given the size
             * we got from our geometry negotiations above.
             */
            ComputeUserChildSize(cw, request.width, request.height,
                                 &childWidth, &childHeight,
                                 w->core.border_width);

            /*
             * If the child wants to change it's position, or it wants
             * to change it's size but our compromize size is not an
             * exact fit, then we need to return Almost and the new geometry.
             */
            if (((Wants(CWX) || Wants(CWY)) ||
                 (Wants(CWWidth) && childWidth != w->core.width) ||
                 (Wants(CWHeight) && childHeight != w->core.height))) {
                result = XtGeometryAlmost;
                allowed->request_mode = desired->request_mode & ~(CWX | CWY);
                allowed->width = childWidth;
                allowed->height = childHeight;
                allowed->border_width = w->core.border_width;
            }
        }

        /*
         * Restore the childs geometry for No or Almost or QueryOnly.
         */
        if (result == XtGeometryNo || result == XtGeometryAlmost ||
            Wants(XtCWQueryOnly)) {
            w->core.width = save_width;
            w->core.height = save_height;
            w->core.border_width = save_border_width;
        }

        return result;
    }

#undef Wants
}

static XtGeometryResult
QueryGeometry(w, proposed, desired)
Widget w;
XtWidgetGeometry *proposed, *desired;
{
    XbaeCaptionWidget cw = (XbaeCaptionWidget)w;
#define Set(bit) (proposed->request_mode & bit)

    /*
     * If we don't have a user child, we want to be the size of the label.
     */
    if (!HaveUserChild(cw)) {
        desired->width = LabelChild(cw)->core.width;
        desired->height = LabelChild(cw)->core.height;
        desired->request_mode = CWWidth | CWHeight;

        if (Set(CWWidth) && proposed->width == desired->width &&
            Set(CWHeight) && proposed->height == desired->height)
            return XtGeometryYes;

        if (desired->width == cw->core.width &&
            desired->height == cw->core.height)
            return XtGeometryNo;

        return XtGeometryAlmost;
    }

    /*
     * Otherwise we must take into account what size the user child wants to be
     */
    else {
        XtWidgetGeometry childProposed, childDesired;
        Dimension childWidth, childHeight, childBorderWidth = 0;
        Dimension cwWidth, cwHeight;
        XtGeometryResult result;

        /*
         * Get our size based on the proposed size for use in computing
         * the user's child size.
         */
        if (Set(CWWidth))
            cwWidth = proposed->width;
        else
            cwWidth = cw->core.width;
        if (Set(CWHeight))
            cwHeight = proposed->height;
        else
            cwHeight = cw->core.height;

        /*
         * Compute the size of the user's child based on our proposed new size.
         */
        ComputeUserChildSize(cw, cwWidth, cwHeight,
                             &childWidth, &childHeight,
                             UserChild(cw)->core.border_width);

        /*
         * Build a geometry request to query the user's child with.
         */
        childProposed.request_mode = 0;
        if (Set(CWWidth)) {
            childProposed.width = childWidth;
            childProposed.request_mode |= CWWidth;
        }
        if (Set(CWHeight)) {
            childProposed.height = childHeight;
            childProposed.request_mode |= CWHeight;
        }

        /*
         * Query the child.
         */
        result = XtQueryGeometry(UserChild(cw), &childProposed, &childDesired);

        /*
         * Save the childs desired geometry.
         */
        switch (result)
        {
        case XtGeometryYes:
            /* use our computed childWidth and childHeight */
            childBorderWidth = UserChild(cw)->core.border_width;
            break;
        case XtGeometryAlmost:
            childWidth = childDesired.width;
            childHeight = childDesired.height;
            childBorderWidth = childDesired.border_width;
            break;
        case XtGeometryNo:
            childWidth = UserChild(cw)->core.width;
            childHeight = UserChild(cw)->core.height;
            childBorderWidth = UserChild(cw)->core.border_width;
            break;
        default:
            break;
        }

        /*
         * Calculate what size we need to be to handle the childs
         * desired geometry and store it in our own desired record.
         */
        ComputeSize(cw, &desired->width, &desired->height,
                    childWidth, childHeight, childBorderWidth);

        /*
         * If the proposed geometry changed, or if the child cares about
         * it's geometry, then set the flag in desired
         */
        desired->request_mode = 0;
        if ((Set(CWWidth) && proposed->width != desired->width) ||
            childDesired.request_mode & CWWidth)
            desired->request_mode |= CWWidth;
        if ((Set(CWHeight) && proposed->height != desired->height) ||
            childDesired.request_mode & CWHeight)
            desired->request_mode |= CWHeight;

        /*
         * If our desired geometry differs from the proposed one, or if
         * we care about a geometry which was not proposed, we return
         * Almost.  Otherwise we return whatever our child returned.
         */
        if ((Set(CWWidth) && proposed->width != desired->width) ||
            (!Set(CWWidth) && desired->request_mode & CWWidth) ||
            (Set(CWHeight) && proposed->height != desired->height) ||
            (!Set(CWHeight) && desired->request_mode & CWHeight))
            return XtGeometryAlmost;
        else
            return result;
    }

#undef Set
}


/*
 * Compare two strings.  The test string must be lower case
 * and NULL terminated.  Leading and trailing whitespace in the in
 * string is ignored.
 */
static Boolean
CompareStrings(in, test)
String in, test;
{
    /*
     * Strip leading whitespace off the in string.
     */
    while (isspace(*in))
        in++;

    for (; *test != '\0' && !isspace(*in); test++, in++) {
        char c = *in;

        if (isupper(c))
            c = tolower(c);

        if (c != *test)
            return False;
    }

    if (*test == '\0' && (*in == '\0' || isspace(*in)))
        return True;
    else
        return False;
}

/* ARGSUSED */
static Boolean
CvtStringToLabelPosition(dpy, args, num_args, from, to, data)
Display *dpy;
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr from, to;
XtPointer *data;
{
    static XbaeLabelPosition position;

    if (*num_args != 0)
        XtAppWarningMsg(
            XtDisplayToApplicationContext(dpy),
            "cvtStringToLabelPosition", "wrongParameters",
            "XbaeCaption",
            "String to LabelPosition conversion needs no extra arguments",
            NULL, NULL);

    /*
     * User didn't provide enough space
     */
    if (to->addr != NULL && to->size < sizeof(XbaeLabelPosition)) {
        to->size = sizeof(XbaeLabelPosition);
        return False;
    }

    if (CompareStrings(from->addr, "left"))
        position = XbaePositionLeft;
    else if (CompareStrings(from->addr, "right"))
        position = XbaePositionRight;
    else if (CompareStrings(from->addr, "top"))
        position = XbaePositionTop;
    else if (CompareStrings(from->addr, "bottom"))
        position = XbaePositionBottom;
    else {
        XtDisplayStringConversionWarning(dpy, from->addr, XmRLabelPosition);
        return False;
    }

    /*
     * Store our return value
     */
    if (to->addr == NULL)
        to->addr = (XtPointer) &position;
    else
        *(XbaeLabelPosition *) to->addr = position;
    to->size = sizeof(XbaeLabelPosition);

    return True;
}

/* ARGSUSED */
static Boolean
CvtStringToLabelAlignment(dpy, args, num_args, from, to, data)
Display *dpy;
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr from, to;
XtPointer *data;
{
    static XbaeLabelAlignment alignment;

    if (*num_args != 0)
        XtAppWarningMsg(
            XtDisplayToApplicationContext(dpy),
            "cvtStringToLabelAlignment", "wrongParameters",
            "XbaeCaption",
            "String to LabelAlignment conversion needs no extra arguments",
            NULL, NULL);

    /*
     * User didn't provide enough space
     */
    if (to->addr != NULL && to->size < sizeof(XbaeLabelAlignment)) {
        to->size = sizeof(XbaeLabelAlignment);
        return False;
    }

    if (CompareStrings(from->addr, "toporleft") ||
        CompareStrings(from->addr, "top") ||
        CompareStrings(from->addr, "left"))
        alignment = XbaeAlignmentTopOrLeft;
    else if (CompareStrings(from->addr, "center"))
        alignment = XbaeAlignmentCenter;
    else if (CompareStrings(from->addr, "bottomorright") ||
             CompareStrings(from->addr, "bottom") ||
             CompareStrings(from->addr, "right"))
        alignment = XbaeAlignmentBottomOrRight;
    else {
        XtDisplayStringConversionWarning(dpy, from->addr, XmRLabelAlignment);
        return False;
    }

    /*
     * Store our return value
     */
    if (to->addr == NULL)
        to->addr = (XtPointer) &alignment;
    else
        *(XbaeLabelAlignment *) to->addr = alignment;
    to->size = sizeof(XbaeLabelAlignment);

    return True;
}

Widget
XbaeCreateCaption(parent, name, args, ac) 
Widget parent;
String name;
ArgList args;
Cardinal ac;
{ 
    return XtCreateWidget(name, xbaeCaptionWidgetClass, parent, args, ac); 
} 
