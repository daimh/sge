/* 
 * Motif Tools Library, Version 3.1
 * $Id: QuarksP.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: QuarksP.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtQuarksP_h
#define _XmtQuarksP_h

externalref XrmQuark XmtQBool;
externalref XrmQuark XmtQBoolean;
externalref XrmQuark XmtQCardinal;
externalref XrmQuark XmtQDimension;
externalref XrmQuark XmtQEnum;
externalref XrmQuark XmtQInt;
externalref XrmQuark XmtQPosition;
externalref XrmQuark XmtQShort;
externalref XrmQuark XmtQUnsignedChar;
externalref XrmQuark XmtQDouble;
externalref XrmQuark XmtQFloat;
externalref XrmQuark XmtQString;
externalref XrmQuark XmtQBuffer;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void _XmtInitQuarks(void);
#else
extern void _XmtInitQuarks();
#endif
_XFUNCPROTOEND
    
#endif /* _XmtQuarksP_h */
