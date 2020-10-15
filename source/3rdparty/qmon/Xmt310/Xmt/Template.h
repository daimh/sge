/* 
 * Motif Tools Library, Version 3.1
 * $Id: Template.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Template.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtTemplate_h
#define _XmtTemplate_h

/*
 * The names of special branches of the db where
 * tempates and styles are stored
 */
#define XmtNxmtTemplates	"_Templates_"
#define XmtNxmtStyles		"_Styles_"

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtTemplateInstantiate(Widget, StringConst, StringConst,
				   String *, Cardinal);
extern String XmtTemplateSubstituteArgs(StringConst, String *,
					Cardinal, Cardinal *);
extern void XmtRegisterTemplate(Widget, StringConst, StringConst);
extern void XmtRegisterStyle(Widget, StringConst, StringConst);
extern String XmtLookupTemplate(Widget, StringConst);
extern String XmtLookupStyle(Widget, StringConst);
#else
extern void XmtTemplateInstantiate();
extern String XmtTemplateSubstituteArgs();
extern void XmtRegisterTemplate();
extern void XmtRegisterStyle();
extern String XmtLookupTemplate();
extern String XmtLookupStyle();
#endif
_XFUNCPROTOEND    

#endif
