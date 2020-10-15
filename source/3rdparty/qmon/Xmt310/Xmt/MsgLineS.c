/* 
 * Motif Tools Library, Version 3.1
 * $Id: MsgLineS.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: MsgLineS.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */
#include <X11/Intrinsic.h>
#ifndef _Xconst
#if __STDC__ || defined(__cplusplus) || defined(c_plusplus)
#define _Xconst const
#else
#define _Xconst
#endif
#endif /* _Xconst */

externaldef(xmtmsglinestrings) _Xconst char XmtMsgLineStrings[] = {
    'a','l','l','o','w','A','s','y','n','c','I','n','p','u','t',0,
    'i','n','p','u','t','C','a','l','l','b','a','c','k',0,
    'm','s','g','L','i','n','e','T','r','a','n','s','l','a','t','i','o','n','s',0,
    'A','l','l','o','w','A','s','y','n','c','I','n','p','u','t',0,
    'M','s','g','L','i','n','e','T','r','a','n','s','l','a','t','i','o','n','s',0
};

