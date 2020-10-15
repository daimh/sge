/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutParse.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutParse.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/Lexer.h>
#include <Xmt/LayoutP.h>

/*
 * we use this structure to accumulate information about rows, columns,
 * and other things that we'll be creating while parsing the layout
 * string.  This info never has to leave the parser.
 */
typedef struct {
    Boolean unmanaged;
    Boolean equal;
    Boolean color_set;
    Pixel color;
    XmtLayoutSpaceType space_type;
    int space;
    int space_stretch;
    int item_stretch;
} XmtLayoutLocalInfo;

/* tokens for the parser */
enum {
    BITMAP,
    BOTTOM,
    BOXED,
    CTABBED,
    CAPTION,
    CENTERED,
    COL,
    COLOR,
    DOUBLEBOXED,
    DOUBLELINE,
    EQUAL,
    ETCHED,
    EVEN,
    EVENSPACED,
    FILLED,
    FIXED,
    FLUSHBOTTOM,
    FLUSHLEFT,
    FLUSHRIGHT,
    FLUSHTOP,
    HSEP,
    HEIGHT,
    HIGH,
    IN,
    INF,
    INSIDE,
    INTERVALSPACED,
    LCRSPACED,
    LREVENSPACED,
    LTABBED,
    LEFT,
    LINE,
    MARGIN,
    OUT,
    OUTSIDE,
    PIXMAP,
    RTABBED,
    RIGHT,
    ROW,
    SHADOWED,
    SPACE,
    STRETCHABLE,
    STRING,
    THROUGH,
    TOP,
    UNMANAGED,
    UNRESIZABLE,
    VSEP,
    WIDE,
    WIDTH,
    EMS,
    ENS,
    INCHES,
    MILLIMETERS,
    POINTS
};    

static String keywords[] = {    
    "Bitmap",
    "Bottom",
    "Boxed",
    "CTabbed",
    "Caption",
    "Centered",
    "Col",
    "Color",
    "DoubleBoxed",
    "DoubleLine",
    "Equal",
    "Etched",
    "Even",
    "EvenSpaced",
    "Filled",
    "Fixed",
    "FlushBottom",
    "FlushLeft",
    "FlushRight",
    "FlushTop",
    "HSep",
    "Height",
    "High",
    "In",
    "Inf",
    "Inside",
    "IntervalSpaced",
    "LCRSpaced",
    "LREvenSpaced",
    "LTabbed",
    "Left",
    "Line",
    "Margin",
    "Out",
    "Outside",
    "Pixmap",
    "RTabbed",
    "Right",
    "Row",
    "Shadowed",
    "Space",
    "Stretchable",
    "String",
    "Through",
    "Top",
    "Unmanaged",
    "Unresizable",
    "VSep",
    "Wide",
    "Width",
    "em",
    "en",
    "in",
    "mm",
    "pt",
};

#define GetToken() XmtLexerGetToken(lw->layout.lexer)
#define NextToken() XmtLexerNextToken(lw->layout.lexer)
#define ConsumeToken() XmtLexerConsumeToken(lw->layout.lexer)
#define GetIntValue() XmtLexerIntValue(lw->layout.lexer)
#define GetKeyword() XmtLexerKeyValue(lw->layout.lexer)
#define GetStrValue() XmtLexerStrValue(lw->layout.lexer)
#define GetStrLength() XmtLexerStrLength(lw->layout.lexer)

#if NeedFunctionPrototypes
static int ParsePixelSize(XmtLayoutWidget lw)
#else
static int ParsePixelSize(lw)
XmtLayoutWidget lw;
#endif
{
    XmtLexerToken tok = GetToken();
    double total, fraction;

    /* parse optional integer part */
    if (tok == XmtLexerInteger) {
	total = (double) GetIntValue();
	ConsumeToken();
    }
    else
	total = 0.0;

    /* parse optional fractional part */
    if (GetToken() == XmtLexerPeriod) {
	ConsumeToken();
	if (GetToken() == XmtLexerInteger) {
	    fraction = (double) GetIntValue();
	    ConsumeToken();
	    while(fraction > 1.0) fraction = fraction / 10.0;
	    total += fraction;
	}
    }

    /* parse optional units */
    if (GetToken() == XmtLexerKeyword) {
	XmtLayoutUnitType units;

	switch(GetKeyword()) {
	case INCHES:  units = XmtLayoutInches;  break;
	case MILLIMETERS: units =  XmtLayoutMillimeters; break;
	case POINTS: units =  XmtLayoutPoints; break;
	case EMS: units =  XmtLayoutEms; break;
	case ENS: units = XmtLayoutEns; break;
	default:  return (int) total;
	}

	ConsumeToken();
	return XmtLayoutConvertSizeToPixels((Widget)lw, total, units);
    }

    return (int) total;
}

#if NeedFunctionPrototypes
static void ParseSpace(XmtLayoutWidget lw, XmtLayoutLocalInfo *li)
#else
static void ParseSpace(lw, li)
XmtLayoutWidget lw;
XmtLayoutLocalInfo *li;
#endif
{
    switch(GetKeyword()) {
    case EVEN:
    case EVENSPACED:
	li->space_type = XmtLayoutSpaceEven;
	break;
    case LREVENSPACED:
	li->space_type = XmtLayoutSpaceLREven;
	break;
    case INTERVALSPACED:
	li->space_type = XmtLayoutSpaceInterval;
	break;
    case LCRSPACED:
	li->space_type = XmtLayoutSpaceLCR;
	break;
    case LTABBED:
	li->space_type = XmtLayoutSpaceLTabbed;
	break;
    case CTABBED:
	li->space_type = XmtLayoutSpaceCTabbed;
	break;
    case RTABBED:
	li->space_type = XmtLayoutSpaceRTabbed;
	break;
    }

    if (NextToken() == XmtLexerInteger)
	li->space = ParsePixelSize(lw);

    if (GetToken() == XmtLexerPlus) {
	if (NextToken() != XmtLexerInteger)
	    XmtWarningMsg("XmtLayout", "space0",
			  "Widget '%s':\n\tinteger expected following '+' in space specification",
			  XtName((Widget)lw));
	else{
	    li->space_stretch = GetIntValue();

	    if (NextToken() == XmtLexerSlash) {
		if (NextToken() != XmtLexerInteger)
		    XmtWarningMsg("XmtLayout", "space1",
				  "Widget '%s':\n\tinteger expected following '/' in space specification",
				  XtName((Widget)lw));
		else {
		    li->item_stretch = GetIntValue();
		    ConsumeToken();
		}
	    }
	}
    }
}

#if NeedFunctionPrototypes
static void ParseCaption(XmtLayoutWidget lw, XmtLayoutInfo *i)
#else
static void ParseCaption(lw, i)
XmtLayoutWidget lw;
XmtLayoutInfo *i;
#endif
{
    String args;

    ConsumeToken();
    if (GetToken() == XmtLexerIdent) {
	args = GetStrValue();

	if (args[0]) {
	    i->dummy_constraints.caption_position = 1;
	    switch(args[0]) {
	    case 'l':
		i->constraints.caption_position = XmtLayoutLeft;
		break;
	    case 'r':
		i->constraints.caption_position = XmtLayoutRight;
		break;
	    case 't':
		i->constraints.caption_position = XmtLayoutTop;
		break;
	    case 'b':
		i->constraints.caption_position = XmtLayoutBottom;
		break;
	    default:
		XmtWarningMsg("XmtLayout", "caption0",
			      "Widget '%s':Unknown caption position '%c' in layout string.\n\tl, r, t, or b expected.",
			      XtName((Widget)lw), args[0]);	
		i->dummy_constraints.caption_position = 0;
		break;
	    }
	}

	if (args[0] && args[1]) {
	    i->dummy_constraints.caption_justification = 1;
	    switch(args[1]) {
	    case 'l':
	    case 't':
		i->constraints.caption_justification=XmtLayoutFlushLeft;
		break;
	    case 'r':
	    case 'b':
		i->constraints.caption_justification=XmtLayoutFlushRight;
		break;
	    case 'c':
		i->constraints.caption_justification=XmtLayoutCentered;
		break;
	    default:
		XmtWarningMsg("XmtLayout", "caption1",
			      "Widget '%s':Unknown caption justification '%c' in layout string.\n\tl, r, t, b, or c expected.",
			      XtName((Widget)lw), args[1]);
		i->dummy_constraints.caption_justification = 0;
		break;
	    }
	}

	if (args[0] && args[1] && args[2]) {
	    i->dummy_constraints.caption_alignment = 1;
	    switch(args[2]) {
	    case 'l':
		i->constraints.caption_alignment = XmALIGNMENT_BEGINNING;
		break;
	    case 'r':
		i->constraints.caption_alignment = XmALIGNMENT_END;
		break;
	    case 'c':
		i->constraints.caption_alignment = XmALIGNMENT_CENTER;
		break;
	    default:
		XmtWarningMsg("XmtLayout", "caption2",
			      "Widget '%s': Unknown caption alignment '%c' in layout string.\n\tl, r, or c expected.",
			      XtName((Widget)lw), args[2]);
		i->dummy_constraints.caption_alignment = 0;
		break;
	    }
	}
	
	XtFree(args);
	ConsumeToken();
    }

    if (GetToken() == XmtLexerInteger) {
	i->dummy_constraints.caption_margin = 1;
	i->constraints.caption_margin = GetIntValue();
	ConsumeToken();
    }

    if (GetToken() == XmtLexerString) {
	i->dummy_constraints.caption = (XmString)1;
	i->constraints.caption = XmtCreateLocalizedXmString((Widget) lw, GetStrValue());
	i->constraints.dont_copy_caption = 1;
	XtFree(GetStrValue());
	ConsumeToken();
    }
    else
	XmtWarningMsg("XmtLayout", "caption3",
		      "Widget '%s': string expected following 'Caption' keyword.",
		      XtName((Widget)lw));
}

#if NeedFunctionPrototypes
static void ParseMargin(XmtLayoutWidget lw, XmtLayoutInfo *i)
#else
static void ParseMargin(lw, i)
XmtLayoutWidget lw;
XmtLayoutInfo *i;
#endif
{
    Boolean width = True;
    Boolean height = True;

    ConsumeToken();
    if (GetToken() == XmtLexerKeyword){
	if (GetKeyword() == WIDTH) height = False;
	else if (GetKeyword() == HEIGHT) width = False;
	else goto error;
	ConsumeToken();
    }

    if (GetToken() == XmtLexerInteger) {
	if (width) {
	    i->dummy_constraints.margin_width = 1;
	    i->constraints.margin_width = GetIntValue();
	}
	if (height) {
	    i->dummy_constraints.margin_height = 1;
	    i->constraints.margin_height = GetIntValue();
	}
	ConsumeToken();
    }
    else
	goto error;

    return;

 error:
    XmtWarningMsg("XmtLayout", "margin",
		  "Widget '%s':\n\t'Width', 'Height', or integer expected following 'Margin' keyword.",
		  XtName((Widget)lw));
}

#if NeedFunctionPrototypes
static void ParseFrame(XmtLayoutWidget lw, XmtLayoutInfo *i)
#else
static void ParseFrame(lw, i)
XmtLayoutWidget lw;
XmtLayoutInfo *i;
#endif
{
    int type, linetype;

    i->dummy_constraints.frame_line_type = 1;
    i->dummy_constraints.frame_type = 1;
    type = XmtLayoutFrameBox;
    
    switch(GetKeyword()) {
    case SHADOWED: linetype = XmtLayoutFrameShadowIn; break;
    default:
    case ETCHED:   linetype = XmtLayoutFrameEtchedIn; break;
    case LINE:
    case BOXED:    linetype = XmtLayoutFrameSingleLine; break;
    case DOUBLELINE:
    case DOUBLEBOXED: linetype = XmtLayoutFrameDoubleLine; break;
    }
    ConsumeToken();

    if (GetToken() == XmtLexerKeyword) {
	if (GetKeyword() == OUT) {
	    if (linetype == XmtLayoutFrameShadowIn)
		linetype = XmtLayoutFrameShadowOut;
	    else if (linetype == XmtLayoutFrameEtchedIn)
		linetype = XmtLayoutFrameEtchedOut;
	    ConsumeToken();
	}
	else if (GetKeyword() == IN) ConsumeToken();
    }
    i->constraints.frame_line_type = linetype;
    
    if (GetToken() == XmtLexerKeyword) {
	switch(GetKeyword()) {
	case LEFT:
	    type = XmtLayoutFrameLeft; break;
	case RIGHT:
	    type = XmtLayoutFrameRight; break;
	case TOP:
	    type = XmtLayoutFrameTop; break;
	case BOTTOM:
	    type = XmtLayoutFrameBottom; break;
	}
	if (type != XmtLayoutFrameBox) ConsumeToken();
    }

    i->constraints.frame_type = type;

    if (GetToken() == XmtLexerKeyword) {
	i->dummy_constraints.frame_position = 1;
	if (GetKeyword() == INSIDE)
	    i->constraints.frame_position = XmtLayoutFrameInside;
	else if (GetKeyword() == OUTSIDE)
	    i->constraints.frame_position = XmtLayoutFrameOutside;
	else if (GetKeyword() == THROUGH)
	    i->constraints.frame_position = XmtLayoutFrameThrough;
	else
	    i->dummy_constraints.frame_position = 0;

	if (i->dummy_constraints.frame_position) ConsumeToken();
    }

    if (GetToken() == XmtLexerInteger) {
	i->dummy_constraints.frame_margin = 1;
	i->constraints.frame_margin = GetIntValue();
	ConsumeToken();
	if (GetToken() == XmtLexerInteger) {
	    i->dummy_constraints.frame_thickness = 1;
	    i->constraints.frame_thickness = GetIntValue();
	    ConsumeToken();
	}
    }
}


#if NeedFunctionPrototypes
static void ParseModifiers(XmtLayoutWidget lw,
			   XmtLayoutInfo *i, XmtLayoutLocalInfo *li)
#else
static void ParseModifiers(lw, i, li)
XmtLayoutWidget lw;
XmtLayoutInfo *i;
XmtLayoutLocalInfo *li;
#endif
{
    XmtLexerToken tok; 
    int size;

    for(;;) {
	tok = GetToken();
	if ((tok == XmtLexerInteger) || (tok == XmtLexerPeriod)) {
	    size = ParsePixelSize(lw);

	    if ((GetToken() == XmtLexerKeyword) &&
		((GetKeyword() == WIDE) || (GetKeyword() == HIGH))) {
		if (GetKeyword() == WIDE) {
		    i->dummy_constraints.width = 1;
		    i->constraints.width = size;
		    ConsumeToken();
		}
		else if (GetKeyword() == HIGH) {
		    i->dummy_constraints.height = 1;
		    i->constraints.height = size;
		    ConsumeToken();
		}
	    }
	    else if (GetToken() == XmtLexerPercent) {
		i->dummy_constraints.width = 1;
		i->constraints.width = size;
		ConsumeToken();
		if (GetToken() == XmtLexerInteger) {
		    i->dummy_constraints.height = 1;
		    i->constraints.height = ParsePixelSize(lw);
		}
		else XmtWarningMsg("XmtLayout", "size",
			       "%s: number must follow '%%' in layout string.",
				   XtName((Widget)lw));
	    }
	    else { /* a size alone is always assumed to be  a width */
		i->dummy_constraints.width = 1;
		i->constraints.width = size;
	    }
	}
	else if (tok == XmtLexerPlus) {
	    ConsumeToken();
	    if ((GetToken() == XmtLexerInteger) ||
		(GetToken() == XmtLexerPeriod)) {
		i->dummy_constraints.stretchability = 1;
		i->constraints.stretchability = ParsePixelSize(lw);
	    }
	    else XmtWarningMsg("XmtLayout", "stretch",
			       "%s: number must follow '+' in layout string.",
			       XtName((Widget)lw));
	}
	else if (tok == XmtLexerMinus) {
	    ConsumeToken();
	    if ((GetToken() == XmtLexerInteger) ||
		(GetToken() == XmtLexerPeriod)) {
		i->dummy_constraints.shrinkability = 1;
		i->constraints.shrinkability = ParsePixelSize(lw);
	    }
	    else XmtWarningMsg("XmtLayout", "shrink",
			       "%s: number must follow '-' in layout string.",
			       XtName((Widget)lw));
	}
	else if (tok == XmtLexerKeyword) {
	    Boolean dont_consume = False;
	    switch(GetKeyword()) {
	    case FIXED:	
		i->dummy_constraints.stretchability = 1;
		i->dummy_constraints.shrinkability = 1;
		i->constraints.stretchability = 0;
		i->constraints.shrinkability = 0;
		break;
	    case STRETCHABLE:
		i->dummy_constraints.stretchability = 1;
		i->dummy_constraints.shrinkability = 1;
		i->constraints.stretchability = INFINITY;
		i->constraints.shrinkability = INFINITY;
		break;
	    case CAPTION:
		ParseCaption(lw, i);
		dont_consume = True;
		break;
	    case SHADOWED: 
	    case ETCHED:
	    case BOXED:
	    case DOUBLEBOXED:
	    case LINE:
	    case DOUBLELINE:
		ParseFrame(lw,i);
		dont_consume = True;
		break;
	    case MARGIN: 
		ParseMargin(lw, i);
		dont_consume = True;
		break;
	    case EQUAL:
		li->equal = True;
		break;
	    case EVEN:
	    case EVENSPACED:
	    case LREVENSPACED:
	    case INTERVALSPACED:
	    case LCRSPACED:
	    case LTABBED:
	    case CTABBED:
	    case RTABBED:
		ParseSpace(lw, li);
		dont_consume = True;
		break;
	    case UNMANAGED:
		li->unmanaged = True;
		break;
	    case FILLED:
		i->dummy_constraints.justification = 1;
		i->constraints.justification = XmtLayoutFilled;
		break;
	    case CENTERED:
		i->dummy_constraints.justification = 1;
		i->constraints.justification = XmtLayoutCentered;
		break;
	    case FLUSHLEFT:
	    case FLUSHTOP:
		i->dummy_constraints.justification = 1;
		i->constraints.justification = XmtLayoutFlushLeft;
		break;
	    case FLUSHRIGHT:
	    case FLUSHBOTTOM:
		i->dummy_constraints.justification = 1;
		i->constraints.justification = XmtLayoutFlushRight;
		break;
	    case UNRESIZABLE:
		i->dummy_constraints.allow_resize = 1;
		i->constraints.allow_resize = False;
		break;
	    case COLOR:
		ConsumeToken();
		if (GetToken() != XmtLexerString) {
		    XmtWarningMsg("XmtLayout", "color",
				"%s: color name expected after Color keyword.",
				  XtName((Widget)lw));
		}
		else {
		    XrmValue from, to;
		    from.addr = GetStrValue();
		    from.size = GetStrLength()+1;
		    to.addr = (XPointer) &li->color;
		    to.size = sizeof(Pixel);
		    if (XtConvertAndStore((Widget)lw,
					  XtRString, &from, XtRPixel, &to))
			li->color_set = True;
		    XtFree(GetStrValue());
		    ConsumeToken();
		}
		dont_consume = True;
		break;
	    default:
		return;
	    }
	    if (!dont_consume) ConsumeToken();
	}
	else return;
    }
}
    

#if NeedFunctionPrototypes
static void GetArgList(XmtLayoutInfo *i, XmtLayoutLocalInfo *li,
		       WidgetClass wclass, ArgList al, Cardinal *count)
#else
static void GetArgList(i, li, wclass, al, count)
XmtLayoutInfo *i;
XmtLayoutLocalInfo *li;
WidgetClass wclass;
ArgList al;
Cardinal *count;
#endif
{
    Cardinal ac = *count;

#define SetArg(resource, field)\
    if (i->dummy_constraints.field) {\
        XtSetArg(al[ac], resource, i->constraints.field);\
        ac++;\
    }

    SetArg(XmtNlayoutWidth, width);
    SetArg(XmtNlayoutHeight, height);
    SetArg(XmtNlayoutStretchability, stretchability);
    SetArg(XmtNlayoutShrinkability, shrinkability);
    SetArg(XmtNlayoutAllowResize, allow_resize);
    SetArg(XmtNlayoutJustification, justification);
    SetArg(XmtNlayoutMarginWidth, margin_width);
    SetArg(XmtNlayoutMarginHeight, margin_height);
    SetArg(XmtNlayoutFrameType, frame_type);
    SetArg(XmtNlayoutFrameLineType, frame_line_type);
    SetArg(XmtNlayoutFramePosition, frame_position);
    SetArg(XmtNlayoutFrameMargin, frame_margin);
    SetArg(XmtNlayoutFrameThickness, frame_thickness);
    SetArg(XmtNlayoutCaption, caption);
    SetArg(XmtNlayoutCaptionPosition, caption_position);
    SetArg(XmtNlayoutCaptionJustification, caption_justification);
    SetArg(XmtNlayoutCaptionAlignment, caption_alignment);
    SetArg(XmtNlayoutCaptionMargin, caption_margin);
#undef SetArg

    if (wclass == xmtLayoutBoxGadgetClass) {
	if (li->equal) {
	    XtSetArg(al[ac], XmtNequal, True); ac++;
	}
	
	if (li->space_type != XmtLayoutSpaceNone) {
	    XtSetArg(al[ac], XmtNspaceType, li->space_type); ac++;
	}

	if (li->space != 0) {
	    XtSetArg(al[ac], XmtNspace, li->space); ac++;
	}

	if (li->space_stretch != 1) {
	    XtSetArg(al[ac], XmtNspaceStretch, li->space_stretch); ac++;
	}

	if (li->item_stretch != 1) {
	    XtSetArg(al[ac], XmtNitemStretch, li->item_stretch);
	    ac++;
	}
    }


    if (li->color_set) {
	if (wclass == xmtLayoutBoxGadgetClass)
	    XtSetArg(al[ac], XmtNbackground, li->color);
	else
	    XtSetArg(al[ac], XmtNforeground, li->color);
	ac++;
    }
    
    *count = ac;
}

#if NeedFunctionPrototypes
static void ParseName(XmtLayoutWidget lw, XmtLayoutInfo *i)
#else
static void ParseName(lw, i)
XmtLayoutWidget lw;
XmtLayoutInfo *i;
#endif
{
    if (GetToken() == XmtLexerIdent) {
	i->name = XrmStringToQuark(GetStrValue());
	XtFree(GetStrValue());
	ConsumeToken();
    }
}    

#if NeedFunctionPrototypes
static Boolean ParseItem(XmtLayoutWidget lw, Widget container)
#else
static Boolean ParseItem(lw, container)
XmtLayoutWidget lw;
Widget container;
#endif
{
    XmtLexerToken tok = GetToken();
    XmtLayoutInfo *i;
    XmtLayoutLocalInfo li;
    WidgetClass wclass;
    int j;
    Arg al[30];
    Cardinal ac;
    Boolean named_child = False;
    XtPointer child_type = NULL;
    String child_name = NULL;
    String label = NULL;
    Boolean is_bitmap;
    
    if ((tok == XmtLexerEndOfString) || (tok == XmtLexerError)) return False;

    i = (XmtLayoutInfo *) XtCalloc(1, sizeof(XmtLayoutInfo));

    /* set "local info" field to their defaults */
    li.unmanaged = False;
    li.equal = False;
    li.color_set = False;
    li.space_type = XmtLayoutSpaceNone;
    li.space = 0;
    li.space_stretch = 1;
    li.item_stretch = 1;

    /*
     * any item can have any modifiers, though some are overridden or ignored
     */
    ParseModifiers(lw, i, &li);
    wclass = NULL;
    ac = 0;
    tok = GetToken();

    switch(tok) {
    case XmtLexerBar:
    ParseVSep:
	ConsumeToken();
	wclass = xmtLayoutSeparatorGadgetClass;
	XtSetArg(al[ac], XmtNorientation, XmVERTICAL); ac++;
	break;
    case XmtLexerEqual:
    ParseHSep:
	ConsumeToken();
	wclass = xmtLayoutSeparatorGadgetClass;
	XtSetArg(al[ac], XmtNorientation, XmHORIZONTAL); ac++;
	break;
    case XmtLexerString:
    ParseString:
	wclass = xmtLayoutStringGadgetClass;
	label = GetStrValue();
	XtSetArg(al[ac], XmtNlabel, label); ac++;
	ConsumeToken();
	break;
    case XmtLexerSharp:
	wclass = xmtLayoutSpaceGadgetClass;
	ConsumeToken();
	for(j=1; GetToken() == XmtLexerSharp; j++) ConsumeToken();
	i->constraints.width = i->constraints.height =
	    j*lw->layout.default_spacing;
	i->constraints.stretchability = 0;
	i->constraints.shrinkability = 0;
	i->dummy_constraints.width = i->dummy_constraints.height = 1;
	i->dummy_constraints.stretchability = 1;
	i->dummy_constraints.shrinkability = 1;
	break;
    case XmtLexerTwiddle:
	wclass = xmtLayoutSpaceGadgetClass;
	ConsumeToken();
	for(j=1; GetToken() == XmtLexerTwiddle; j++) ConsumeToken();
	i->constraints.width = 0;
	i->constraints.height = 0;
	i->constraints.stretchability = j*XmtLAYOUT_DEFAULT_STRETCHABILITY;
	i->constraints.shrinkability = 0;
	i->dummy_constraints.width = i->dummy_constraints.height = 1;
	i->dummy_constraints.stretchability = 1;
	i->dummy_constraints.shrinkability = 1;
	break;
    case XmtLexerLess:
	ConsumeToken();
	for(j=1; GetToken() == XmtLexerMinus; j++) ConsumeToken();
	if (GetToken() != XmtLexerGreater)
	    XmtWarningMsg("XmtLayout", "greater",
			  "%s: '>' expected after '<' in layout string.",
			  XtName((Widget)lw));
	else
	    ConsumeToken();
	wclass = xmtLayoutSpaceGadgetClass;
	i->constraints.width = 0;
	i->constraints.height = 0;
	i->constraints.stretchability = j*INFINITY;
	i->constraints.shrinkability = 0;
	i->dummy_constraints.width = i->dummy_constraints.height = 1;
	i->dummy_constraints.stretchability = 1;
	i->dummy_constraints.shrinkability = 1;
	break;
    case XmtLexerIdent:
	/* get type and name to create, or just a name */
	if ((xmtLayoutClassRec.layout_class.lookup_type_proc) &&
	    (child_type = (*xmtLayoutClassRec.layout_class.lookup_type_proc)
	     (GetStrValue()))) {
	    XtFree(GetStrValue());
	    ConsumeToken();
	    if (GetToken() != XmtLexerIdent) {
		XmtWarningMsg("XmtLayout", "create",
			      "Widget '%s':\n\twidget name expected following widget type in layout string.",
			      XtName((Widget)lw));
		goto error;
	    }
	    else {
		child_name = GetStrValue();
		ConsumeToken();
	    }
	}
	else {
	    named_child = True;       /* a child that will be created later */
	    i->name = XrmStringToQuark(GetStrValue());
	    XtFree(GetStrValue());
	    ConsumeToken();
	}
	break;
    case XmtLexerKeyword:
	switch (GetKeyword()) {
	case ROW:
	    wclass = xmtLayoutBoxGadgetClass;
	    ConsumeToken();
	    ParseName(lw, i);
	    XtSetArg(al[ac], XmtNorientation, XmHORIZONTAL); ac++;
	    break;
	case COL:
	    wclass = xmtLayoutBoxGadgetClass;
	    ConsumeToken();
	    ParseName(lw, i);
	    XtSetArg(al[ac], XmtNorientation, XmVERTICAL); ac++;
	    break;
	case SPACE:
	    wclass = xmtLayoutSpaceGadgetClass;
	    ConsumeToken();
	    break;
	case HSEP:
	    goto ParseHSep;
	case VSEP:
	    goto ParseVSep;
	case STRING:  /* string keyword, not XmtLexerString token */
	    ConsumeToken();
	    ParseName(lw, i);
	    if (GetToken() != XmtLexerString) {
		XmtWarningMsg("XmtLayout", "string",
			      "%s: string must follow String keyword",
			      XtName((Widget)lw));
	    }
	    else
		goto ParseString;
	    break;
	case BITMAP:
	case PIXMAP:
	    if (GetKeyword() == PIXMAP) is_bitmap = False;
	    else is_bitmap = True;
	    wclass = xmtLayoutPixmapGadgetClass;
	    ConsumeToken();
	    ParseName(lw, i);
	    if (GetToken() != XmtLexerString) {
		XmtWarningMsg("XmtLayout", "pixmap",
			   "%s: string must follow Pixmap or Bitmap keyword.",
			      XtName((Widget)lw));
	    }
	    else {
		XrmValue from, to;
		from.addr = GetStrValue();
		from.size = GetStrLength()+1;
		al[ac].name = (is_bitmap)?XmtNbitmap:XmtNpixmap;
		to.addr = (XPointer) &al[ac].value;
		to.size = sizeof(Pixel);
		if (XtConvertAndStore((Widget)lw,
				   XtRString, &from,
				   (is_bitmap)?XtRBitmap:XtRPixmap,
				   &to))
		    ac++;
		XtFree(GetStrValue());
		ConsumeToken();

		/*
		 * Now, look for ` , "mask_name"' to set the XmtNbitmask
		 * resource
		 */
		if (GetToken() == XmtLexerComma) {
		    if (NextToken() != XmtLexerString) {
			XmtWarningMsg("XmtLayout", "pixmap",
				      "%s: mask name must follow comma in Pixmap or Bitmap specification.",
				      XtName((Widget)lw));
		    }
		    else {
			XrmValue from, to;
			from.addr = GetStrValue();
			from.size = GetStrLength()+1;
			al[ac].name = XmtNbitmask;
			to.addr = (XPointer) &al[ac].value;
			to.size = sizeof(Pixel);
			if (XtConvertAndStore((Widget)lw,
					      XtRString, &from,
					      XmtRBitmask,
					      &to))
			    ac++;
			XtFree(GetStrValue());
			ConsumeToken();
		    }
		}
	    }
	    break;
	default:
	    XmtWarningMsg("XmtLayout", "keyword",
			  "Widget %s:\n\tunexpected keyword '%s' in layout string.",
			  XtName((Widget)lw), GetStrValue());
	    ConsumeToken();
	    goto error;
	}
	break;
    case XmtLexerEndOfString:
	XmtWarningMsg("XmtLayout", "end",
		      "%s: unexpected end of layout string.",
		      XtName((Widget)lw));
	goto error;
    default:
	XmtWarningMsg("XmtLayout", "syntax",
		      "%s: syntax error in layout string.",XtName((Widget)lw));
	ConsumeToken();
	goto error;
    }

    if (named_child) {  /* a widget to be created later */
	Widget marker;

	/* create a gadget to mark the position of the widget */
	marker = XtVaCreateWidget("", xmtLayoutSpaceGadgetClass, (Widget)lw,
				  XmtNlayoutIn, container, NULL);

	/* store the constraint info away for later lookup */
	/* this is special; we don't set dummy_constraints */
	i->constraints.after = marker;
	i->next = lw->layout.widget_info;
	lw->layout.widget_info = i;
    }
    else {  /* a typed widget or a layout gadget to create now */
	Widget w = NULL;
	
	/* set position arg, get constraint args, and create the child */
	XtSetArg(al[ac], XmtNlayoutIn, container); ac++;
	GetArgList(i, &li, wclass, al, &ac);
	if (child_type) {
	    w = (*xmtLayoutClassRec.layout_class.create_proc)
		(child_name, child_type, (Widget)lw, al, ac);
	    XtFree(child_name);
	}
	else if (wclass != NULL)
	    w = XtCreateWidget((i->name)?XrmQuarkToString(i->name):"",
			       wclass, (Widget)lw, al, ac);
	
	/* if it is a row or column, parse its children */
	if (wclass == xmtLayoutBoxGadgetClass) {  /* a row or column */
	    if (GetToken() != XmtLexerLBrace) {
		XmtWarningMsg("XmtLayout", "open",
			      "%s: '{' expected at beginning of row.",
			      XtName((Widget)lw));
	    }
	    else {
		ConsumeToken();
		while ((tok = GetToken())) {
		    if ((tok == XmtLexerRBrace) || (tok == XmtLexerError) ||
			(tok == XmtLexerEndOfString)) break;
		    if (!ParseItem(lw, w)) {
			/* error recovery: read till '}' or end of string */
			while(tok != XmtLexerEndOfString &&
			      tok != XmtLexerRBrace)
			    tok = NextToken();
			break;
		    }
		}
	    }

	    if (GetToken() != XmtLexerRBrace) {
		XmtWarningMsg("XmtLayout", "close",
			      "%s: '}' expected at end of row.",
			      XtName((Widget)lw));
	    }
	    else ConsumeToken();
	}

	/* manage it unless specified otherwise */
	if (w && !li.unmanaged) XtManageChild(w);

	/* free our info structure */
	XtFree((char *)i);
    }
    if (label) XtFree(label);
    return True;

 error:
    if (i->constraints.caption) XmStringFree(i->constraints.caption);
    XtFree((char *)i);
    if (label) XtFree(label);
    return False;
}

#if NeedFunctionPrototypes
static void XmtLayoutParse(XmtLayoutWidget lw, String str)
#else
static void XmtLayoutParse(lw, str)
XmtLayoutWidget lw;
String str;
#endif
{
    XmtLexerToken tok;
    
    lw->layout.lexer = XmtLexerCreate(keywords, XtNumber(keywords));
    XmtLexerInit(lw->layout.lexer, str);

    while(1) {
	tok = GetToken();
	if ((tok == XmtLexerEndOfString)  || (tok == XmtLexerError)) break;
	/* NULL means place items in the toplevel container */
	if (!ParseItem(lw, NULL)) break;
    }

    if (tok == XmtLexerError)
	XmtWarningMsg("XmtLayout", "syntax", "%s: syntax error.",
		      XtName((Widget)lw));

    XmtLexerDestroy(lw->layout.lexer);
}

#if NeedFunctionPrototypes
void XmtRegisterLayoutParser(void)
#else
void XmtRegisterLayoutParser()
#endif
{
    xmtLayoutClassRec.layout_class.parser = XmtLayoutParse;
}
