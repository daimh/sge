/* 
 * Motif Tools Library, Version 3.1
 * $Id: LookupP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LookupP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtLookupP_h
#define _XmtLookupP_h

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
String _XmtLookupResource(Screen *, StringConst, StringConst);
#else
String _XmtLookupResource();
#endif
_XFUNCPROTOEND

#endif
