/* 
 * Motif Tools Library, Version 3.1
 * $Id: Xbm.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Xbm.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtXbm_h
#define _XmtXbm_h

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Boolean XmtParseXbmFile(StringConst, char **, int *, int *,int *,int *);
extern Boolean XmtParseXbmString(StringConst, char **,int *,int *,int *,int *);
#else
extern Boolean XmtParseXbmFile();
extern Boolean XmtParseXbmString();
#endif
_XFUNCPROTOEND

#endif  /* _XmtXbm_h */
