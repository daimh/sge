/* 
 * Motif Tools Library, Version 3.1
 * $Id: Procedures.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Procedures.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtProcedures_h
#define _XmtProcedures_h

/*
 * This is maximum number of arguments that an Xmt callback procedure
 * can take.  If you change it here, you've also got to change the code
 * that actually passes the arguments and calls the functions, and
 * also change the definition of an XmtProcedure below.
 */
#define XmtMAX_PROCEDURE_ARGS 8

typedef XtPointer (*XmtProcedure) (
#if NeedFunctionPrototypes
  				   XtPointer, XtPointer, XtPointer,
				   XtPointer, XtPointer, XtPointer,
				   XtPointer, XtPointer
#endif
				   );
/*
 * Each callback has a name and procedure pointer, of course.
 * Each procedure will be passed 8 (this number is arbitrary) arguments.
 * The argument_types array specifies the types of each of these arguments.
 * Specify the type of each argument that the procedure expects, and
 * set the remaining fields to NULL.  Use the special values
 * XmtRCallbackWidget and XmtRCallbackData to specify that the widget
 * or call_data values are expected in a particular position.
 * Also, use XmtRCallbackAppContext, XmtRCallbackWindow, XmtCallbackDisplay,
 * to pass the app context, Window, and Display of the callback widget.
 * Also, use XmtRCallbackUnused if the procedure has an unused argument--
 * Xmt will always pass NULL for that arg.
 *
 * expected_args is a private field; leave it uninitialized.
 *
 * Procedures that return values should specify the type of the return
 * value in return_type, or NULL if they don't return anything.
 * return_type is currently unimplemented, so always specify NULL.
 */
typedef struct {
    /* Only the first 2 are always required */
    StringConst name;
    XmtProcedure function;
    /* The expected arg. types and return value of the function */
    StringConst argument_types[XmtMAX_PROCEDURE_ARGS];
    StringConst return_type;
    /* private state, not initialized */
    int expected_args;
} XmtProcedureInfo;

/*
 * Special resource types that tell the callback converter to figure
 * out the procedure argument from information it already has, not
 * from a passed argument
 */
#define XmtRCallbackWidget "XmtCallbackWidget"
#define XmtRCallbackData "XmtCallbackData"
#define XmtRCallbackAppContext "XmtCallbackAppContext"
#define XmtRCallbackWindow "XmtCallbackWindow"
#define XmtRCallbackDisplay "XmtCallbackDisplay"
#define XmtRCallbackUnused "XmtCallbackUnused"


_XFUNCPROTOBEGIN    
#if NeedFunctionPrototypes
extern void XmtRegisterProcedures(XmtProcedureInfo *, Cardinal);
extern void XmtRegisterCallbackProcedure(StringConst, XtCallbackProc,
					 StringConst);
extern XmtProcedureInfo *XmtLookupProcedure(StringConst);
extern Boolean XmtCallbackCheckList(XtCallbackList, XmtProcedure);
extern void XmtRegisterXtProcedures(void);
extern void XmtRegisterUnixProcedures(void);
extern void XmtRegisterXmtProcedures(void);
#else
extern void XmtRegisterProcedures();
extern void XmtRegisterCallbackProcedure();
extern XmtProcedureInfo *XmtLookupProcedure();
extern Boolean XmtCallbackCheckList();
extern void XmtRegisterXtProcedures();
extern void XmtRegisterUnixProcedures();
extern void XmtRegisterXmtProcedures();
#endif

#if NeedVarargsPrototypes
extern void XmtVaRegisterCallbackProcedures(StringConst,XtCallbackProc,
					    StringConst,...);
#else
extern void XmtVaRegisterCallbackProcedures();
#endif
_XFUNCPROTOEND

#endif /* _XmtProcedures_h */
