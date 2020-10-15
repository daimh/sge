/* 
 * Motif Tools Library, Version 3.1
 * $Id: CallbackCvt.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: CallbackCvt.c,v $
 * Revision 1.1  2001-07-18 11:06:01  root
 * Initial revision
 *
 * Revision 1.1  2001/06/12 15:00:21  andre
 * AA-2001-06-12-0: replaced Xmt212 by Xmt310
 *                  (http://sourceforge.net/projects/motiftools) with
 *                  our xmt212 patches applied
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/ProceduresP.h>
#include <Xmt/Symbols.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/Util.h>
#include <Xmt/Lexer.h>
#include <Xmt/QuarksP.h>

typedef struct {
    String procedure_name;
    XmtProcedureInfo *procedure;
    String argument_strings[XmtMAX_PROCEDURE_ARGS];
    XtPointer arguments[XmtMAX_PROCEDURE_ARGS];
    XmtSymbol symbols[XmtMAX_PROCEDURE_ARGS];
    Boolean error;
} CallInfo;
    

typedef struct {
    CallInfo *calls;
    int num_calls;
} Callback;

typedef void (*XmtProcedure0)(
#if NeedFunctionPrototypes
			      void
#endif
			      );
typedef void (*XmtProcedure1)(
#if NeedFunctionPrototypes
  				   XtPointer
#endif
			      );
typedef void (*XmtProcedure2)(
#if NeedFunctionPrototypes
  				   XtPointer, XtPointer
#endif
			      );
typedef void (*XmtProcedure3)(
#if NeedFunctionPrototypes
  				   XtPointer, XtPointer, XtPointer
#endif
			      );
typedef void (*XmtProcedure4)(
#if NeedFunctionPrototypes
  				   XtPointer, XtPointer, 
				   XtPointer, XtPointer
#endif
			      );
typedef void (*XmtProcedure5)(
#if NeedFunctionPrototypes
  				   XtPointer, XtPointer, XtPointer,
				   XtPointer, XtPointer
#endif
			      );
typedef void (*XmtProcedure6)(
#if NeedFunctionPrototypes
  				   XtPointer, XtPointer, XtPointer,
				   XtPointer, XtPointer, XtPointer
#endif
			      );
typedef void (*XmtProcedure7)(
#if NeedFunctionPrototypes
  				   XtPointer, XtPointer, XtPointer,
				   XtPointer, XtPointer, XtPointer,
				   XtPointer
#endif
			      );
typedef void (*XmtProcedure8)(
#if NeedFunctionPrototypes
  				   XtPointer, XtPointer, XtPointer,
				   XtPointer, XtPointer, XtPointer,
				   XtPointer, XtPointer
#endif
			      );

#if NeedFunctionPrototypes
static void ExpectedWarning(String s)
#else
static void ExpectedWarning(s)
String s;
#endif
{
    XmtWarningMsg("XmtConvertStringToCallback", "expected",
		  "%s expected.",
		  s);
}

#if NeedFunctionPrototypes
static Boolean ParseCall(XmtLexer l, CallInfo *ci)
#else
static Boolean ParseCall(l, ci)
XmtLexer l;
CallInfo *ci;
#endif
{
    XmtLexerToken tok;
    Cardinal num_args;
    int i;

    ci->procedure_name = NULL;
    ci->procedure = NULL;
    for(i=0; i < XmtMAX_PROCEDURE_ARGS; i++) {
	ci->argument_strings[i] = NULL;
	ci->arguments[i] = NULL;
	ci->symbols[i] = NULL;
    }
    ci->error = False;

    tok = XmtLexerGetToken(l);
    if (tok != XmtLexerIdent) {
	ExpectedWarning("procedure name");
	goto error;
    }

    ci->procedure_name = XmtLexerStrValue(l);
    XmtLexerConsumeToken(l);

    if (XmtLexerGetToken(l) != XmtLexerLParen) {
	ExpectedWarning("'('");
	goto error;
    }

    XmtLexerGetArgList(l,ci->argument_strings,XmtMAX_PROCEDURE_ARGS,&num_args);
    return True;
    
 error:
    /* free any strings allocated so far.  XtFree is okay with NULL */
    XtFree(ci->procedure_name);
    ci->procedure_name = NULL;
    for(i=0; i < XmtMAX_PROCEDURE_ARGS; i++) {
	XtFree(ci->argument_strings[i]);
	ci->argument_strings[i] = NULL;
    }
    return False;
}
    

#if NeedFunctionPrototypes
static Callback *StringToCallback(String str)
#else
static Callback *StringToCallback(str)
String str;
#endif
{
    static XmtLexer l = NULL;
    XmtLexerToken tok;
    int num, max;
    CallInfo *calls;
    Callback *cb;
    Boolean stat;

    if (l == NULL)
	l = XmtLexerCreate((String *)NULL, 0);

    XmtLexerInit(l, str);

    num = max = 0;
    calls = NULL;

    while(XmtLexerGetToken(l) != XmtLexerEndOfString) {
	if (num == max) {
	    if (max == 0) max = 4; else max *= 2;
	    calls = (CallInfo*)XtRealloc((char*)calls, sizeof(CallInfo) * max);
	}
	/* parse a single procedure call */
	stat = ParseCall(l, &calls[num]);
	if (stat) num++;
	else { /* on error read 'till next semicolon */
	    while(1) {
		tok = XmtLexerGetToken(l);
		if ((tok == XmtLexerSemicolon) ||
		    (tok == XmtLexerEndOfString)) break;
		XmtLexerConsumeToken(l);
	    }
	}
	    
	/* and discard any semicolons */
	while (XmtLexerGetToken(l) == XmtLexerSemicolon)
	    XmtLexerConsumeToken(l);
    }

    if (num == 0) { /* if empty string or errors */
	XtFree((char *)calls);
	cb = NULL;
    }
    else {
	calls = (CallInfo *)XtRealloc((char *)calls, sizeof(CallInfo)*num);
	cb = XtNew(Callback);
	cb->calls = calls;
	cb->num_calls = num;
    }
    return cb;
}


#if NeedFunctionPrototypes
static void DestroyCallback(Callback *cb)
#else
static void DestroyCallback(cb)
Callback *cb;
#endif
{
    int i,j;
    CallInfo *info;

    for(i=0; i < cb->num_calls; i++) {
	info = &cb->calls[i];
	XtFree(info->procedure_name);
	for(j=0; j < XmtMAX_PROCEDURE_ARGS; j++)
	    XtFree(info->argument_strings[j]);
    }
    XtFree((char *) cb->calls);
    XtFree((char *)cb);
}

#if NeedFunctionPrototypes
static void ConvertArg(Widget w, CallInfo *call, int i, int j)
#else
static void ConvertArg(w, call, i, j)
Widget w;
CallInfo *call;
int i;
int j;
#endif
{
    XrmValue from, to;
    Boolean stat;
    XmtProcedureInfo *proc = call->procedure;
    
    /*
     * if the argument is a symbol name, bind it,
     * check the type, and remember the symbol
     */
    if (call->argument_strings[j][0] == '$') {
	call->symbols[i] = XmtLookupSymbol(&call->argument_strings[j][1]);
	
	if (!call->symbols[i]) {
	    XmtWarningMsg("XmtCallCallbacks", "symbol",
			  "argument %d of procedure %s.\n\tSymbol '%s' is undefined.",
			  j, call->procedure_name,
			  call->argument_strings[j]);
	}
	else {
	    /*
	     * check symbol type.
	     * A buffer symbol will match a string argument.
	     */
	    XrmQuark symtype = XmtSymbolTypeQuark(call->symbols[i]);
	    XrmQuark argtype = (XrmQuark) proc->argument_types[i];
	    if ((symtype != argtype) &&
		!((symtype == XmtQBuffer) && (argtype == XmtQString))) {
		XmtWarningMsg("XmtCallCallbacks", "symbolType",
			      "type mismatch:\n\targument %d of procedure %s.\n\tSymbol '%s' is not of type %s",
			      j, call->procedure_name,
			      XmtSymbolName(call->symbols[i]),
			      XrmQuarkToString((XrmQuark)
					       proc->argument_types[i]));
		call->symbols[i] = NULL;
	    }
	}
	if (!call->symbols[i]) call->error = True;
    }
    else {
	/*
	 * if it is not a symbol, then if it is a string constant, we're done.
	 * otherwise, we've got to convert our string to the appropriate type.
	 */
	if ((XrmQuark)proc->argument_types[i] == XmtQString) {
	    call->arguments[i] = (XtPointer)call->argument_strings[j];
	}
	else {
	    from.addr = (XPointer) call->argument_strings[j];
	    from.size = strlen(from.addr)+1;
	    to.addr = (XPointer) NULL;
	    /* XtConvertAndStore() will ensure that converted
	     * values are freed when the widget w is destroyed.
	     */
	    stat = XtConvertAndStore(w, XtRString, &from,
			   XrmQuarkToString((XrmQuark)proc->argument_types[i]),
				     &to);
	    
	    if (!stat) {
		XmtWarningMsg("XmtCallCallbacks", "typeMismatch",
			      "type mismatch: \n\targument %d of procedure %s.\n\tCan't convert \"%s\" to type %s",
			      j, call->procedure_name,
			      call->argument_strings[i],
			      XrmQuarkToString((XrmQuark)
					       proc->argument_types[i]));
		call->error = True;
	    }
	    else {
		/* XXX
		 * This code isn't as portable as it should be, but it
		 * does seem to work on 32-bit architectures, and 64-bit Alpha.
		 * I don't know if there are any architectures that have
		 * calling conventions unlike those assumed here.
		 *
		 * All functions registered with the callback converter
		 * must expect arguments of the same size as XtPointer.
		 * This means that chars and shorts must be "widened"
		 * to longs.  Also, arguments of type double are not
		 * supported
		 */
		switch(to.size) {
		case 1:
		    call->arguments[i] = (XtPointer)(long)*(char *)to.addr;
		    break;
		case 2:
		    /* XXX signed vs. unsigned??? */
		    call->arguments[i] = (XtPointer)(long)*(short *)to.addr;
		    break;
 		case 4:
 		    call->arguments[i] = (XtPointer)(long)*(int *)to.addr;
 		    break;
		default:
		    call->arguments[i] = (XtPointer)*(long *)to.addr;
		}
	    }
	}
    }
}


#if NeedFunctionPrototypes
static void XmtCallCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void XmtCallCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    Callback *cb = (Callback *) tag;
    CallInfo *call;
    XmtProcedureInfo *proc;
    Boolean firsttime;
    int n;
    int i, j;

    for(n=0; n < cb->num_calls; n++) {
	call = &cb->calls[n];

	if (call->error) {
	    XmtWarningMsg("XmtCallCallback", "badCall",
			  "not calling %s() because of previous errors.",
			  call->procedure_name);
	    continue;
	}

	/* if this is the 1st time, figure out the procedure info */
	/* also set a flag for use below */
	if (call->procedure == NULL) {
	    firsttime = True;
	    call->procedure = XmtLookupProcedure(call->procedure_name);
	    if (call->procedure == NULL) {
		XmtWarningMsg("XmtCallCallback", "unknown",
			      "unknown procedure %s",
			      call->procedure_name);
		continue;
	    }
	}
	else firsttime = False;

	proc = call->procedure;

	/* if this is the first time, check the # of arguments */
	if (firsttime) {
	    for(i=0;
		(i < XmtMAX_PROCEDURE_ARGS) && (call->argument_strings[i]);
		i++);

	    if (i != proc->expected_args) {
		XmtWarningMsg("XmtCallCallback", "wrongNum",
			      "procedure %s expects %d arguments.",
			      call->procedure_name, proc->expected_args);
		call->error = True;
		continue;
	    }
	}

	/* build up the array of arguments */
	i = j = 0;
	while((i < XmtMAX_PROCEDURE_ARGS) &&
	      (proc->argument_types[i] != (String)NULLQUARK)) {
	    if ((XrmQuark)proc->argument_types[i] == _XmtQCallbackWidget)
		call->arguments[i] = (XtPointer) w;
	    else if ((XrmQuark)proc->argument_types[i] == _XmtQCallbackData)
		call->arguments[i] = data;
	    else if((XrmQuark)proc->argument_types[i]==_XmtQCallbackAppContext)
		call->arguments[i] =(XtPointer)XtWidgetToApplicationContext(w);
	    else if ((XrmQuark)proc->argument_types[i] == _XmtQCallbackWindow)
		call->arguments[i] = (XtPointer) XtWindow(w);
	    else if ((XrmQuark)proc->argument_types[i] == _XmtQCallbackDisplay)
		call->arguments[i] = (XtPointer) XtDisplay(w);
	    else if ((XrmQuark)proc->argument_types[i] == _XmtQCallbackUnused)
		call->arguments[i] = NULL;
	    else {
		/*
		 * if this is the first time this CallInfo has been called,
		 * we've got to convert the argument string to the expected
		 * type, or to a symbol of the expected type.  If the argument
		 * is a constant, then we can use it for all subsequent calls.
		 * if the argument is a symbol, we save the bound symbol,
		 * and look up its value for each subsequent call.
		 * Note we convert argument_string[j] to argument[i].
		 */
		if (firsttime) {
		    ConvertArg(w, call, i, j);
		}
		/*
		 * if the arg is a symbol, get its value.
		 * Otherwise, its a constant, already converted, so do nothing.
		 */
		if (call->symbols[i]) {
		    XmtSymbolGetValue(call->symbols[i],
				      (XtArgVal *)&call->arguments[i]);
		}
		j++;
	    }
	    if (call->error) break;
	    i++;
	}

	if (call->error) continue;
	
	/*
	 * Call the function.
	 * If you change XmtMAX_PROCEDURE_ARGS, change this code too.
	 * Note that all functions declared this way must expect
	 * all their arguments to be the same size as XtPointer.
	 * On most (all?) architectures it is safe to pass extra arguments
	 * to functions, so we could just do case 8: here in all cases.
	 * But this is a little cleaner when using a debugger, for example.
	 */
	switch(i) {  /* number of arguments*/
	case 0:
	    (*((XmtProcedure0)proc->function))();
	    break;
	case 1:
	    (*((XmtProcedure1)proc->function))(call->arguments[0]);
	    break;
	case 2:
	    (*((XmtProcedure2)proc->function))
		(call->arguments[0], call->arguments[1]);
	    break;
	case 3:
	    (*((XmtProcedure3)proc->function))
		(call->arguments[0], call->arguments[1],
		 call->arguments[2]);
	    break;
	case 4:
	    (*((XmtProcedure4)proc->function))
		(call->arguments[0], call->arguments[1],
		 call->arguments[2], call->arguments[3]);
	    break;
	case 5:
	    (*((XmtProcedure5)proc->function))
		(call->arguments[0], call->arguments[1],
		 call->arguments[2], call->arguments[3],
		 call->arguments[4]);
	    break;
	case 6:
	    (*((XmtProcedure6)proc->function))
		(call->arguments[0], call->arguments[1],
		 call->arguments[2], call->arguments[3],
		 call->arguments[4], call->arguments[5]);
	    break;
	case 7:
	    (*((XmtProcedure7)proc->function))
		(call->arguments[0], call->arguments[1],
		 call->arguments[2], call->arguments[3],
		 call->arguments[4], call->arguments[5],
		 call->arguments[6]);
	    break;
	case 8:
	    (*((XmtProcedure8)proc->function))
		(call->arguments[0], call->arguments[1],
		 call->arguments[2], call->arguments[3],
		 call->arguments[4], call->arguments[5],
		 call->arguments[6], call->arguments[7]);
	    break;
	}

    }
}


/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToCallback(Display *dpy,
				   XrmValue *args, Cardinal *num_args,
				   XrmValue *from, XrmValue *to,
				   XtPointer *converter_data)
#else
Boolean XmtConvertStringToCallback(dpy, args, num_args, from, to,
				   converter_data)
Display *dpy;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *converter_data;
#endif
{
    Callback *callback;
    XtCallbackList list;

    callback = StringToCallback((char *)from->addr);

    if (callback == NULL) { /* error, couldn't convert */
	/* XXX print the string here, and mark where the error occured. */
	return False;
    }
    else {
	list = (XtCallbackList)XtCalloc(sizeof(XtCallbackRec), 2);
	list[0].callback = (XtCallbackProc) XmtCallCallback;
	list[0].closure = (XtPointer) callback;
	done(XtCallbackList, list);
    }
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void XmtDestroyCallback(XtAppContext app, XrmValue *to,
			       XtPointer converter_data,
			       XrmValue *args, Cardinal *num_args)
#else
static void XmtDestroyCallback(app, to, converter_data, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer converter_data;
XrmValue *args;
Cardinal *num_args;
#endif
{
    XtCallbackList list = *(XtCallbackList *)to->addr;

    DestroyCallback((Callback *)list[0].closure);
    XtFree((char *)list);
}

#if NeedFunctionPrototypes
void XmtRegisterCallbackConverter(void)
#else
void XmtRegisterCallbackConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
	registered = True;
	/*
	 * Note that the results of this converter cannot be cached, because
	 * the callback arguments need to be converted in the context of
	 * their particular widget and thus cannot be shared.  It doesn't
	 * make much sense, however, to have lots of widgets with the same
	 * callbacks and arguments, however, so this is no great loss.
	 */
	XtSetTypeConverter(XtRString, XtRCallback,
			   XmtConvertStringToCallback, NULL, 0,
			   XtCacheNone | XtCacheRefCount,
			   XmtDestroyCallback);

	/* ensure that the quarks we need will exist when the
	 * converter is called
	 */
	_XmtInitQuarks();

	/*
	 * we also register the converter in variables declared in
	 * GlobalsI.h so that it can be looked up by the automatic
	 * widget creation routines.  We don't want those routines to
	 * reference these directly because then they would be linked
	 * together.
	 */
	_XmtCallbackConverter = XmtConvertStringToCallback;
    }
}

#if NeedFunctionPrototypes
Boolean XmtCallbackCheckList(XtCallbackList list, XmtProcedure proc)
#else
Boolean XmtCallbackCheckList(list, proc)
XtCallbackList list;
XmtProcedure proc;
#endif
{
    Callback *cb;
    int i;
    
    /* for each item in the callback list */
    for(; list->callback != NULL; list++) {
	/* if the procedures match exactly */
	if (list->callback == (XtCallbackProc) proc) return True;
	/* else if it is the result of the converter */
	else if (list->callback == (XtCallbackProc) XmtCallCallback) {
	    /* the client data is a list of calls */
	    cb = (Callback *) list->closure;
	    for (i=0; i < cb->num_calls; i++) {
		/* if the callback has been called once and already converted*/
		if (cb->calls[i].procedure) {
		    if (cb->calls[i].procedure->function == proc) return True;
		}
		/* otherwise, we've got to look up procedures by name */
		else {
		    XmtProcedureInfo *procedure;
		    procedure =XmtLookupProcedure(cb->calls[i].procedure_name);
		    if (procedure && procedure->function == proc) return True;
		}
	    }
	}
    }
    return False;
}

