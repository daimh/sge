/* 
 * Motif Tools Library, Version 3.1
 * $Id: Procedures.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Procedures.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/ProceduresP.h>
#include <Xmt/Hash.h>

static XmtHashTable XmtProcedureTable = NULL;

XrmQuark _XmtQCallbackWidget = NULLQUARK;
XrmQuark _XmtQCallbackData = NULLQUARK;
XrmQuark _XmtQCallbackAppContext = NULLQUARK;
XrmQuark _XmtQCallbackWindow = NULLQUARK;
XrmQuark _XmtQCallbackDisplay = NULLQUARK;
XrmQuark _XmtQCallbackUnused = NULLQUARK;

#if NeedFunctionPrototypes
void XmtRegisterProcedures(XmtProcedureInfo *procedures,
			   Cardinal num_procedures)
#else
void XmtRegisterProcedures(procedures, num_procedures)
XmtProcedureInfo *procedures;
Cardinal num_procedures;
#endif
{
    XrmQuark nameq, q;
    int i,j;
    int num_args;
    
    /* one time initialization */
    if (XmtProcedureTable == NULL){
	XmtProcedureTable = XmtHashTableCreate(4);

        _XmtQCallbackData = XrmPermStringToQuark(XmtRCallbackData);
        _XmtQCallbackWidget = XrmPermStringToQuark(XmtRCallbackWidget);
        _XmtQCallbackAppContext = XrmPermStringToQuark(XmtRCallbackAppContext);
        _XmtQCallbackWindow = XrmPermStringToQuark(XmtRCallbackWindow);
        _XmtQCallbackDisplay = XrmPermStringToQuark(XmtRCallbackDisplay);
	_XmtQCallbackUnused = XrmPermStringToQuark(XmtRCallbackUnused);
    }
    
    for(i = 0; i < num_procedures; i++) {
        XmtProcedureInfo *p; 

	/* quarkify the procedure name */
        nameq = XrmStringToQuark(procedures[i].name);

	/* check if this procedure has been registered before */
	if (XmtHashTableLookup(XmtProcedureTable, (XtPointer) nameq,
			       (XtPointer *) &p)) {
	    /* and if so, whether it was registered with the same value */
	    if (p == &procedures[i]) {	
		XmtWarningMsg("XmtRegisterProcedures", "procedure",
			      "multiple registration of procedure %s ignored.",
			      procedures[i].name);
		/*
		 * We've don't reregister it here, because the procedure
		 * fields have already been quarkified, and doing it again
		 * will cause chaos.  Thanks to Ulrich Ring.
		 */
		continue;
	    } 
	}

	/* figure out how many args this proc has. */
	for(num_args = 0, j=0; j < XmtMAX_PROCEDURE_ARGS; j++) {
	    if (procedures[i].argument_types[j] != NULL)
		num_args = j+1;
	}

	/*
	 * quarkify the argument types for each of those arguments,
	 * and count the # that are not automatically passed.
	 * Note that a NULL argument type will be turned into the
	 * internal _XmtQCallbackUnused type, and NULL will always be
	 * be passed to that argument.
	 */
	procedures[i].expected_args = 0;
	for(j=0; j < num_args; j++) {
	    if (procedures[i].argument_types[j] == NULL)
		procedures[i].argument_types[j] = (String) _XmtQCallbackUnused;
	    else {
		procedures[i].argument_types[j] =
		    (String)XrmStringToQuark(procedures[i].argument_types[j]);
		q = (XrmQuark) procedures[i].argument_types[j];
		if ((q != _XmtQCallbackWidget) &&
		    (q != _XmtQCallbackData) &&
		    (q != _XmtQCallbackAppContext) &&
		    (q != _XmtQCallbackWindow) &&
		    (q != _XmtQCallbackDisplay) &&
		    (q != _XmtQCallbackUnused))
		    procedures[i].expected_args++;
	    }
	}

	/* quarkify the return type */
	procedures[i].return_type =
	    (String)XrmStringToQuark(procedures[i].return_type);

	/* assoicate procedure with name in the table */
	XmtHashTableStore(XmtProcedureTable,
			  (XtPointer)nameq,
			  (XtPointer)&procedures[i]);
    }
}

#if NeedFunctionPrototypes
void XmtRegisterCallbackProcedure(StringConst name, XtCallbackProc proc,
				  StringConst type)
#else
void XmtRegisterCallbackProcedure(name, proc, type)
StringConst name;
XtCallbackProc proc;
StringConst type;
#endif
{
    XmtProcedureInfo *info = XtNew(XmtProcedureInfo);  /* never freed */
    int i;

    info->name = name;
    info->function = (XmtProcedure) proc;
    info->argument_types[0] = XmtRCallbackWidget;
    info->argument_types[1] = type;
    info->argument_types[2] = XmtRCallbackData;
    for(i = 3; i < XmtMAX_PROCEDURE_ARGS; i++)
	info->argument_types[i] = NULL;
    info->return_type = NULL;

    XmtRegisterProcedures(info, (Cardinal) 1);
}

#if NeedVarargsPrototypes
void XmtVaRegisterCallbackProcedures(StringConst name, XtCallbackProc proc,
				     StringConst type, ...)
#else
void XmtVaRegisterCallbackProcedures(name, proc, type, va_alist)
StringConst name;
XtCallbackProc proc;
StringConst type;
va_dcl
#endif
{
    va_list var;

    XmtRegisterCallbackProcedure(name, proc, type);
    Va_start(var, type);

    while((name = va_arg(var, StringConst)) != NULL) {
	proc = va_arg(var, XtCallbackProc);
	type = va_arg(var, StringConst);
	XmtRegisterCallbackProcedure(name, proc, type);
    }

    va_end(var);
}

#if NeedFunctionPrototypes
XmtProcedureInfo *XmtLookupProcedure(StringConst name)
#else
XmtProcedureInfo *XmtLookupProcedure(name)
StringConst name;
#endif
{
    XrmQuark q;
    XmtProcedureInfo *p;

    if (XmtProcedureTable == NULL) return NULL;

    q = XrmStringToQuark(name);

    if (XmtHashTableLookup(XmtProcedureTable,
			   (XtPointer) q,
			   (XtPointer *) &p))
	return p;
    else
	return NULL;
}
