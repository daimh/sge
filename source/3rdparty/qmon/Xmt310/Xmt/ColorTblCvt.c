/* 
 * Motif Tools Library, Version 3.1
 * $Id: ColorTblCvt.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: ColorTblCvt.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <ctype.h>
#include <Xmt/Xmt.h>
#include <Xmt/Color.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/Lexer.h>
#include <Xmt/LookupP.h>
#include <X11/IntrinsicP.h>

/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToColorTable(Display *dpy,
				     XrmValue *args, Cardinal *num_args,
				     XrmValue *from, XrmValue *to,
				     XtPointer *data)
#else
Boolean XmtConvertStringToColorTable(dpy, args, num_args, from, to, data)
Display *dpy;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *data;
#endif
{
    static XmtLexer l = NULL;
    XmtColorTable parent = *((XmtColorTable *)args[0].addr);
    Screen *screen = *((Screen **)args[1].addr);
    String s = (String)from->addr;
    XmtColorTable table;
    String symbol, color;
    XmtLexerToken token;

    /*
     * First, check if this is a symbolic color table name.
     * If so, look it up, and recurse to parse its value
     */
    if (s[0] == '$') {
	String value;
	XrmValue new_from;
	s++;
	while(isspace(*s)) s++;

	/*
	 * lookup value of that symbolic name under:
	 *   _Colortables_.palette.visual.depth
	 * Note that we don't have to free the return value.
	 */
	value = _XmtLookupResource(screen, "TVDp", s);
	
	/* if no value defined, warn and fail */
	if (!value) {
	    XmtWarningMsg("XmtConvertStringToColorTable", "nosymbol",
			  "No color table named '%s' defined in resource file under _ColorTables_",
			  s);
	    XtDisplayStringConversionWarning(dpy, from->addr,
					     XmtRXmtColorTable);
	    return False;
	}

	/*
	 * Otherwise, recurse to convert the definition we've found 
	 * The programmer must be smart enough to avoid infinite recursion
	 */
	new_from.addr = (XPointer) value;
	new_from.size = strlen(value) + 1;
	return XmtConvertStringToColorTable(dpy, args, num_args,
					    &new_from, to, data);

    }

    table = XmtCreateColorTable(parent);
    
    if (l == NULL) l = XmtLexerCreate(NULL, 0);
    XmtLexerInit(l, s);

    for(;;) {
	symbol = color = NULL;
	if (XmtLexerGetToken(l) != XmtLexerIdent) goto error;
	symbol = XmtLexerStrValue(l);
	XmtLexerConsumeToken(l);
	if (XmtLexerGetToken(l) != XmtLexerEqual) goto error;
	XmtLexerConsumeToken(l);
	token = XmtLexerGetToken(l);
	if ((token != XmtLexerString) && (token != XmtLexerIdent)) goto error;
	color = XmtLexerStrValue(l);
	XmtLexerConsumeToken(l);
	XmtRegisterColor(table, symbol, color);
	XtFree(symbol);
	XtFree(color);
	if (XmtLexerGetToken(l) == XmtLexerComma)
	    XmtLexerConsumeToken(l);
	else
	    break;
    }

    if (XmtLexerGetToken(l) == XmtLexerEndOfString)
	done(XmtColorTable, table);

 error:
    XmtWarningMsg("XmtConvertStringToColorTable", "syntax", "syntax error");
    XtDisplayStringConversionWarning(dpy, from->addr, XmtRXmtColorTable);
    XtFree(symbol);
    XtFree(color);
    XmtDestroyColorTable(table);
    return False;
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void FreeConvertedColorTable(XtAppContext app, XrmValue *to,
				    XtPointer closure,
				    XrmValue *args, Cardinal *num_args)
#else
static void FreeConvertedColorTable(app, to, closure, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer closure;
XrmValue *args;
Cardinal *num_args;
#endif
{
    XmtDestroyColorTable(*((XmtColorTable *) to->addr));
}

static XtConvertArgRec color_table_args[] = {
    {XtProcedureArg, (XtPointer)_XmtFetchColorTable, 0},
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.screen),
     sizeof(Screen *)},
};

#if NeedFunctionPrototypes
void XmtRegisterColorTableConverter(void)
#else
void XmtRegisterColorTableConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
	XtSetTypeConverter(XtRString, XmtRXmtColorTable,
			   XmtConvertStringToColorTable,
			   color_table_args, XtNumber(color_table_args),
			   XtCacheNone | XtCacheRefCount,
			   FreeConvertedColorTable);
	_XmtColorTableConverter = XmtConvertStringToColorTable;
	registered = True;
    }
}


