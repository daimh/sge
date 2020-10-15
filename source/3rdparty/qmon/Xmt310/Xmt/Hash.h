/* 
 * Motif Tools Library, Version 3.1
 * $Id: Hash.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Hash.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtHash_h
#define _XmtHash_h

typedef struct _XmtHashTableRec *XmtHashTable;

typedef void (*XmtHashTableForEachProc)(
#if NeedFunctionPrototypes					
    XmtHashTable, XtPointer, XtPointer *
#endif
);

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern XmtHashTable XmtHashTableCreate(int);
extern void XmtHashTableDestroy(XmtHashTable);
extern void XmtHashTableStore(XmtHashTable, XtPointer, XtPointer);
extern Boolean XmtHashTableLookup(XmtHashTable, XtPointer, XtPointer *);
extern void XmtHashTableDelete(XmtHashTable, XtPointer);
extern void XmtHashTableForEach(XmtHashTable, XmtHashTableForEachProc);
#else
extern XmtHashTable XmtHashTableCreate();
extern void XmtHashTableDestroy();
extern void XmtHashTableStore();
extern Boolean XmtHashTableLookup();
extern void XmtHashTableDelete();
extern void XmtHashTableForEach();
#endif
_XFUNCPROTOEND

#endif
