/* 
 * Motif Tools Library, Version 3.1
 * $Id: Create.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Create.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtCreate_h
#define _XmtCreate_h

/*
 * Names and classes for the pseudo-resources we read
 * during automatic widget creation.
 */
#define XmtNxmtChildren		"xmtChildren"
#define XmtCXmtChildren		"XmtChildren"
#define XmtNxmtType		"xmtType"
#define XmtCXmtType		"XmtType"
#define XmtNxmtCreationCallback	"xmtCreationCallback"
#define XmtCXmtCreationCallback "XmtCreationCallback"
#define XmtNxmtChildrenCreationCallback	"xmtChildrenCreationCallback"
#define XmtCXmtChildrenCreationCallback "XmtChildrenCreationCallback"
#define XmtNxmtManagedCreationCallback	"xmtManagedCreationCallback"
#define XmtCXmtManagedCreationCallback	"XmtManagedCreationCallback"


_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtCreateChildren(Widget);
extern Widget XmtCreateChild(Widget, StringConst);
extern Widget XmtBuildDialog(Widget, StringConst, XtResourceList, Cardinal);  
extern Widget XmtBuildToplevel(Widget, StringConst);
extern Widget XmtBuildApplication(StringConst, StringConst, Display *,
				       ArgList, Cardinal);
#else
extern void XmtCreateChildren();
extern Widget XmtCreateChild();
extern Widget XmtBuildDialog();  
extern Widget XmtBuildToplevel();
extern Widget XmtBuildApplication();
#endif

#if NeedVarargsPrototypes
extern void XmtCreateQueryChildren(Widget, ...);
extern Widget XmtCreateQueryChild(Widget, StringConst, ...);
extern Widget XmtBuildQueryDialog(Widget, StringConst, XtResourceList,
				  Cardinal, ...);
extern Widget XmtBuildQueryToplevel(Widget, StringConst, ...);
extern Widget XmtBuildQueryApplication(StringConst, StringConst, Display *,
				       ArgList, Cardinal, ...);
#else
extern void XmtCreateQueryChildren();
extern Widget XmtCreateQueryChild();
extern Widget XmtBuildQueryDialog();
extern Widget XmtBuildQueryToplevel();
extern Widget XmtBuildQueryApplication();
#endif
_XFUNCPROTOEND    

#endif
