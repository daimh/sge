/* 
 * Motif Tools Library, Version 3.1
 * $Id: MenuCvt.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: MenuCvt.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <Xmt/Xmt.h>
#include <Xmt/MenuP.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/Lexer.h>

/*
 * These are keywords and symbolic names for keywords, used by the lexer.
 */
static String keywords[] = {
    "Alt",
    "Button",
    "Ctrl",
    "DoubleLine",
    "Help",
    "Line",
    "Lock",
    "Meta",
    "Off",
    "On",
    "Pixmap",
    "Shift",
    "Submenu",
    "Tearoff",
    "Title",
    "Toggle"
};
    
#define ALT		0
#define BUTTON		1
#define CTRL		2
#define DOUBLELINE	3
#define HELP		4
#define LINE		5
#define LOCK		6
#define META		7
#define OFF		8
#define ON		9
#define PIXMAP		10
#define SHIFT		11
#define SUBMENU		12
#define TEAROFF		13
#define TITLE		14
#define TOGGLE		15


#if NeedFunctionPrototypes
static int ParseAccelerator(XmtLexer l, XmtMenuItem *item)
#else
static int ParseAccelerator(l, item)
XmtLexer l;
XmtMenuItem *item;
#endif
{
    int tok;
    char accel[200];
    char accel_label[200];
    
    accel[0] = '\0';
    accel_label[0] = '\0';

    while((XmtLexerGetToken(l) == XmtLexerKeyword) &&
	  ((XmtLexerKeyValue(l) == CTRL) ||
	   (XmtLexerKeyValue(l) == SHIFT) ||
	   (XmtLexerKeyValue(l) == META) ||
	   (XmtLexerKeyValue(l) == ALT) ||
	   (XmtLexerKeyValue(l) == LOCK))) {
	strcat(accel, XmtLexerStrValue(l));
	strcat(accel_label, XmtLexerStrValue(l));
	XmtLexerConsumeToken(l);
	tok = XmtLexerGetToken(l);
	if (tok == XmtLexerPlus) {
	    strcat(accel, " ");
	    strcat(accel_label, "+");
	    XmtLexerConsumeToken(l);
	}
	else if (tok == XmtLexerMinus) {
	    strcat(accel, " ");
	    strcat(accel_label, "-");
	    XmtLexerConsumeToken(l);
	}
	else {
	    XmtWarningMsg("XmtMenu", "missingPlus",
			  "item \"%s\": modifier name in accelerator must be followed by `+' or `-'",
			  item->label);
	    goto error;
	}
    }

    strcat(accel, "<Key>");
    tok = XmtLexerGetToken(l);

    if ((tok == XmtLexerIdent) || (tok == XmtLexerString)) {
	char *sym = XmtLexerStrValue(l);

	if (isalnum(sym[0]))
	    strcat(accel, sym);
	else {
	    char buf[10];
	    sprintf(buf, "%d", sym[0]);
	    strcat(accel, buf);
	}

	strcat(accel_label, sym);
	XtFree(sym);
	XmtLexerConsumeToken(l);
    }
    else if (tok == XmtLexerInteger) {
	char buf[10];
	sprintf(buf, "%d", XmtLexerIntValue(l));
	strcat(accel, buf);
	strcat(accel_label, buf);
	XmtLexerConsumeToken(l);
    }
    else {
	XmtWarningMsg("XmtMenu", "missingKeysym",
		      "item \"%s\": accelerator is missing keysym",
		      item->label);
	goto error;
    }
    
    item->accelerator = XtNewString(accel);
    item->accelerator_label = XtNewString(accel_label);
    return 0;

 error:
    /* read forward until matching ']' */
    while(1) {
	tok = XmtLexerGetToken(l);
	if ((tok == XmtLexerRBracket) || (tok == XmtLexerEndOfString)) break;
	XmtLexerConsumeToken(l);
    }
    item->accelerator = item->accelerator_label = NULL;
    return 1;
}

#if NeedFunctionPrototypes
static void ParseLabelAndMnemonic(XmtLexer l, String *label,
				  int look_for_mnemonic, char *mnemonic)
#else
static void ParseLabelAndMnemonic(l, label, look_for_mnemonic, mnemonic)
XmtLexer l;
String *label;
int look_for_mnemonic;
char *mnemonic;
#endif
{
    register int i;
    register char *c;
    
    if (XmtLexerGetToken(l) != XmtLexerString) return;

    if (look_for_mnemonic) {
	*label = XtMalloc(XmtLexerStrLength(l) + 1);
	*mnemonic = '\0';
	
	for(i=0, c = XmtLexerStrValue(l); *c != '\0'; c++) {
	    if ((*c == '_') && (*mnemonic == '\0'))
		*mnemonic = *(c+1);
	    else
		(*label)[i++] = *c;
	}
	(*label)[i] = '\0';
	
	XtFree(XmtLexerStrValue(l));
    }
    else {
	*label = XmtLexerStrValue(l);
	*mnemonic = '\0';
    }

    XmtLexerConsumeToken(l);
}    


/*
 *
 * item:: [name] '-' '-'* ';'      // a separator
 *      | [name] '=' '='* ';'      // a double separator
 *      | [name] [type] flags* [label] [accelerator]
 *             [submenu] [symbol] callbacks
 * 
 * name::         ident ':'
 * type::         'Title' | 'Button' | 'Toggle' |
 *                    'Line' | 'DoubleLine' | 'Submenu'
 * flags::        'On' | 'Off' | 'Help' | 'Tearoff' | 'Pixmap'
 * label::        string-with-embedded-mnemonic [ '|' string ]
 * accelerator::  '[' (modifier ('+'|'-'))* keysym ']'
 * modifier::     'Ctrl' | 'Shift' | 'Meta' | 'Alt' | 'Lock'
 * keysym::       ident
 * submenu::      '-' '>' ident
 * symbol::       '$' ident
 * callbacks::    callback | '{' callback+ '}' | ';'
 * callback::     ident '(' [args] ')' ';'
 * args::         list-of-comma-separated-strings
 */

#if NeedFunctionPrototypes
static int ParseItem(Widget w, XmtLexer l, XmtMenuItem *item)
#else
static int ParseItem(w, l, item)
Widget w;
XmtLexer l;
XmtMenuItem *item;
#endif
{
    int tok;
    static XmtMenuItem null_item;  /* static memory; all fields NULL */

    /* initialize item structure */
    null_item.type = XmtMenuItemEnd;  /* undefined type, no flags */
    *item = null_item;

    /* parse item name */
    if (XmtLexerGetToken(l) == XmtLexerIdent) {
	item->name = XmtLexerStrValue(l);
	XmtLexerConsumeToken(l);
	if (XmtLexerGetToken(l) == XmtLexerColon)
	   XmtLexerConsumeToken(l);
	else
	    XmtWarningMsg("XmtMenu", "colonExpected",
			  "colon expected after item name %s.",
			  item->name);
    }

    /* parse separators as a special case */
    if (((tok = XmtLexerGetToken(l)) == XmtLexerMinus) ||
	(tok == XmtLexerEqual))   { /* a separator */
	if (tok == XmtLexerMinus) item->type = XmtMenuItemSeparator;
	else item->type = XmtMenuItemDoubleSeparator;
	    
	while (((tok = XmtLexerGetToken(l)) == XmtLexerMinus) ||
	       (tok == XmtLexerEqual))
	    XmtLexerConsumeToken(l);

	if (tok == XmtLexerSemicolon) {
	    XmtLexerConsumeToken(l);
	    return 0;
	}
	else {
	    XmtWarningMsg("XmtMenu", "semicolonAfterSeparator",
			  "semicolon expeced after separator.");
	    goto error;
	}
    }

    /*
     * parse an optional type keyword.
     * We can set the type directly, because we know that no flags are set yet.
     */
    if (XmtLexerGetToken(l) == XmtLexerKeyword) {
	switch(XmtLexerKeyValue(l)) {
	case TITLE:      item->type = XmtMenuItemLabel; break;
	case BUTTON:     item->type = XmtMenuItemPushButton; break;
	case TOGGLE:     item->type = XmtMenuItemToggleButton; break;
	case LINE:       item->type = XmtMenuItemSeparator; break;
	case DOUBLELINE: item->type = XmtMenuItemDoubleSeparator; break;
	case SUBMENU:    item->type = XmtMenuItemCascadeButton; break;
	}
	/* if we set a type eat the token */
	if (item->type != XmtMenuItemEnd) XmtLexerConsumeToken(l);
    }

    /*
     * If the type is not set, assume it is a push button until
     * we find a flag or something else that indicates otherwise.
     */
    if (item->type == XmtMenuItemEnd)
	item->type = XmtMenuItemPushButton;

    /*
     * now that we're going to set flags, we've got to be
     * careful when setting the type.  Use this macro:
     */
#define SetType(item, value) ((item)->type = ((item)->type & ~0x7) | (value))

    /*
     * parse any flags that modify the type.
     */
    while (XmtLexerGetToken(l) == XmtLexerKeyword) {
	switch(XmtLexerKeyValue(l)) {
	case ON:
	    SetType(item, XmtMenuItemToggleButton);
	    item->type |= XmtMenuItemOn;
	    break;
	case OFF: 
	    SetType(item, XmtMenuItemToggleButton);
	    item->type &= ~XmtMenuItemOn;
	    break;
	case HELP:
	    item->type |= XmtMenuItemHelp;
	    break;
	case TEAROFF:
	    item->type |= XmtMenuItemTearoff;
	    break;
	case PIXMAP:
	    item->type |= XmtMenuItemPixmap;
	    break;
	default:
	    XmtWarningMsg("XmtMenu", "unexpectedKeyword",
			  "item \"%s\": unexpected keyword \"%s\"",
			  (item->label)?item->label:"",
			  XmtLexerStrValue(l));
	    break;
	}
	XmtLexerConsumeToken(l);
    }
    
    
    /* parse item label */
    if (XmtLexerGetToken(l) == XmtLexerString) {
	/* the item label string; figure out label and mnemonic  */
	ParseLabelAndMnemonic(l, &item->label,
			      !(item->type & XmtMenuItemPixmap),
			      &item->mnemonic);
	if (XmtLexerGetToken(l) == XmtLexerBar) {  /* alternate label */
	    SetType(item, XmtMenuItemToggleButton);
	    if (XmtLexerNextToken(l) != XmtLexerString) {
		XmtWarningMsg("XmtMenu", "missingLabel",
			      "item '%s': alternate label expected after '|'",
			      item->label);
	    }
	    else
		ParseLabelAndMnemonic(l, &item->alt_label,
				      !(item->type & XmtMenuItemPixmap),
				      &item->alt_mnemonic);
	}
    }

    /* parse item accelerator */
    if (XmtLexerGetToken(l) == XmtLexerLBracket) { /* an accelerator */
	XmtLexerConsumeToken(l);
	(void) ParseAccelerator(l, item);
	if (XmtLexerGetToken(l) == XmtLexerRBracket)
	    XmtLexerConsumeToken(l);
	else
	    XmtWarningMsg("XmtMenu", "missingRBracket",
			  "`]' expected at end of accelerator for item \"%s\"",
			  item->label);
    }

    /* parse optional arrow and submenu name */
    if (XmtLexerGetToken(l) == XmtLexerMinus) {
	SetType(item, XmtMenuItemCascadeButton);
	if (XmtLexerNextToken(l) == XmtLexerGreater)
	    XmtLexerConsumeToken(l);
	if (XmtLexerGetToken(l) == XmtLexerIdent) {
	    item->submenu_name = XmtLexerStrValue(l);
	    XmtLexerConsumeToken(l);
	}
	else {
	    XmtWarningMsg("XmtMenu", "missingSubmenuName",
			  "item \"%s\": submenu name expected after `->'.",
			  item->label);
	    SetType(item, XmtMenuItemPushButton);
	    goto error;
	}
    }

    /*
     * Parse an optional symbol name, to be used by toggle buttons
     */
    if (XmtLexerGetToken(l) == XmtLexerDollar) {
	if (XmtLexerNextToken(l) != XmtLexerIdent) {
	    XmtWarningMsg("XmtMenu", "missingSymbol",
			  "item '%s': symbol name expected after '$'.",
			  item->label);
	    goto error;
	}
	item->symbol_name = XmtLexerStrValue(l);
	XmtLexerConsumeToken(l);
    }

    /*
     * parse callbacks.
     * Expect an identifier, a '{' or a ';'.
     * To parse the callbacks, we extract the callback string from the
     * private insides of the lexer and invoke whatever string-to-callback
     * converter is registered.
     */
    tok = XmtLexerGetToken(l);
    if ((tok == XmtLexerIdent) || (tok == XmtLexerLBrace)) { /* a callback */
	char *callback_string;
	int len = 0;
	XrmValue from, to;
	Boolean stat = False;
	
	if (tok == XmtLexerIdent) { /* a single callback */
	    /* back up by the length of the current identifier */
	    l->c -= XmtLexerStrLength(l);
	    XtFree(XmtLexerStrValue(l));
	
	    /* scan forward just past an unquoted semicolon */
	    tok = XmtLexerScan(l, ";", True);
	    if (tok == XmtLexerString) {
		callback_string = XmtLexerStrValue(l);
		len = XmtLexerStrLength(l);
		XmtLexerConsumeToken(l);
	    }
	    else callback_string = NULL;
	}
	else {  /* a compound callback */
	    XmtLexerConsumeToken(l);  /* eat the open brace */
	    XmtLexerSkipWhite(l);     /* skip whitespace */
	    /* scan forward to just before an unquoted close brace */
	    tok = XmtLexerScan(l, "}", False);
	    if (tok == XmtLexerString) {
		callback_string = XmtLexerStrValue(l);
		len = XmtLexerStrLength(l);
		XmtLexerConsumeToken(l);
	    }
	    else callback_string = NULL;

	    /* Now get and eat the '}' token */
	    XmtLexerNextToken(l);
	    XmtLexerConsumeToken(l);
	}

	/* convert the string to an XtCallbackList */
	if (!callback_string) {
	    XmtWarningMsg("XmtMenu", "badCallback",
			  "bad callback string for item '%s'",
			  item->label);
	}
	else {
#if 0
	    /*
	     * The ref counting on the callback converter somehow causes
	     * a core dump when the Menu widget is destroyed.  So we don't
	     * do it this way.
	     */
	    from.addr = callback_string;
	    from.size = len + 1;
	    to.addr = (XPointer) &item->callback;
	    to.size = sizeof(XtCallbackList);
	    stat = XtConvertAndStore(w, XtRString, &from, XtRCallback, &to);
#endif
	    if (_XmtCallbackConverter == NULL) {
		XmtWarningMsg("XmtMenu", "noConverter",
			      "no String to XtCallbackList converter registered.\n\tCall XmtRegisterCallbackConverter().");
		stat = False;
	    }
	    else {
		XtCacheRef ref_return;
		/*
		 * Here we just call the converter directly.
		 * We ignore the returned cache reference.  This will only
		 * cause a memory leak if the menu is destroyed, and in
		 * that case, a memory leak is better than the core dump
		 * we had above.
		 */
		from.addr = (XPointer) callback_string;
		from.size = len + 1;
		to.addr = (XPointer) &item->callback;
		to.size = sizeof(XtCallbackList);
		stat = XtCallConverter(XtDisplay(w), _XmtCallbackConverter,
				       NULL, 0,
				       &from, &to, &ref_return);
	    }

	}
	
	/* if conversion failed, a warning will already be issued */
	if (!callback_string || !stat) item->callback = NULL;
	
	/* set flag that says we have a XtCallbackList, not XtCallbackProc */
	item->type |= XmtMenuItemCallbackList;
	
	/* and free the callback string */
	XtFree(callback_string);
    }
    else if (tok == XmtLexerSemicolon) {  /* no callback at all */
	XmtLexerConsumeToken(l);
    }
    else {
	XmtWarningMsg("XmtMenu", "semicolonExpected",
		      "item \"%s\": semicolon expected at end of item description.",
		      item->label);
	goto error;
    }
    
    return 0;

 error:
    /* error recovery: free strings and read 'till `;' or end of string */
    XtFree(item->label);
    XtFree(item->accelerator);
    XtFree(item->accelerator_label);
    XtFree(item->name);
    XtFree(item->submenu_name);
    
    while(1) {
	tok = XmtLexerGetToken(l);
	if ((tok == XmtLexerSemicolon) || (tok == XmtLexerEndOfString)) break;
	XmtLexerConsumeToken(l);
    }
    XmtLexerConsumeToken(l);
    return 1;
}    

#if NeedFunctionPrototypes
Boolean XmtParseMenuString(Widget w, String str, XmtMenuItem **items_return)
#else
Boolean XmtParseMenuString(w, str, items_return)
Widget w;
String str;
XmtMenuItem **items_return;
#endif
{
    static XmtLexer l = NULL;
    XmtMenuItem *items;
    int num_items, max_items;

    if (l == NULL)
	l = XmtLexerCreate(keywords, XtNumber(keywords));

    XmtLexerInit(l, str);

    max_items = 8;
    num_items = 0;
    items = (XmtMenuItem *) XtMalloc(sizeof(XmtMenuItem) * max_items);

    while (XmtLexerGetToken(l) != XmtLexerEndOfString) {
	if (num_items == max_items) {
	    max_items *= 2;
	    items = (XmtMenuItem *) XtRealloc((char *) items,
					      sizeof(XmtMenuItem) * max_items);
	}
	if (ParseItem(w, l, &items[num_items]) == 0) /* if no error */
	    num_items++;
    }

    if (num_items == max_items) {
	max_items++;
	items = (XmtMenuItem *) XtRealloc((char *) items,
					  sizeof(XmtMenuItem) * max_items);
    }
    items[num_items].type = XmtMenuItemEnd;  /* NULL-termination */

    *items_return = items;
    return True;
}


/*ARGSUSED*/
#if NeedFunctionPrototypes
Boolean XmtConvertStringToXmtMenuItems(Display *dpy,
				       XrmValuePtr args, Cardinal *num_args,
				       XrmValuePtr from, XrmValuePtr to,
				       XtPointer *closure_return)
#else
Boolean XmtConvertStringToXmtMenuItems(dpy, args, num_args,
				       from, to, closure_return)
Display *dpy;
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr from;
XrmValuePtr to;
XtPointer *closure_return;
#endif
{
    String str = (String)from->addr;
    XmtMenuItem *value;
    Boolean status;
    Widget w = *((Widget *)args[0].addr);

    /* convert string to NULL-terminated array of XmtMenuItems */
    status = XmtParseMenuString(w, str, &value);

    if (status == False) return False;

    done(XmtMenuItem *, value);  /* a macro in ConvertersP.h */
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void MenuItemsDestructor(XtAppContext app, XrmValue *to,
				XtPointer converter_data,
				XrmValue *args, Cardinal *num_args)
#else
static void MenuItemsDestructor(app, to, converter_data, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer converter_data;
XrmValue *args;
Cardinal *num_args;
#endif
{
    XmtMenuItem *items = *(XmtMenuItem **) to->addr;
    int i;

    for(i=0; items[i].type != XmtMenuItemEnd; i++) {
	XtFree(items[i].label);
	XtFree(items[i].accelerator);
	XtFree(items[i].accelerator_label);
	XtFree(items[i].name);
	XtFree(items[i].submenu_name);
    }
    XtFree((char *)items);
}

static XtConvertArgRec convert_args[] = {
  {XtBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.self), sizeof(Widget)}
};


#if NeedFunctionPrototypes
void XmtRegisterMenuItemsConverter(void)
#else
void XmtRegisterMenuItemsConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
	registered = True;
	XtSetTypeConverter(XmRString, XmtRXmtMenuItemList,
			   XmtConvertStringToXmtMenuItems,
			   convert_args, XtNumber(convert_args),
			   XtCacheNone, MenuItemsDestructor);
    }
}
