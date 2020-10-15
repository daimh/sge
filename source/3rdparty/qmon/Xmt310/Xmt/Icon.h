/* 
 * Motif Tools Library, Version 3.1
 * $Id: Icon.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Icon.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtIcon_h
#define _XmtIcon_h

_XFUNCPROTOBEGIN    
#if NeedFunctionPrototypes
extern void XmtCreatePixmapIcon(Widget w, Pixmap icon, Pixmap shape);
extern void XmtDestroyPixmapIcon(Widget w);
extern void XmtChangePixmapIcon(Widget w, Pixmap icon, Pixmap shape);
#else
extern void XmtCreatePixmapIcon();
extern void XmtDestroyPixmapIcon();
extern void XmtChangePixmapIcon();
#endif
_XFUNCPROTOEND

#endif /* _XmtIcon_h */
