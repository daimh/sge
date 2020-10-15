/* 
 * Motif Tools Library, Version 3.1
 * $Id: BSearch.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: BSearch.c,v $
 * Revision 1.1  2001-07-18 11:06:01  root
 * Initial revision
 *
 * Revision 1.1  2001/06/12 15:00:21  andre
 * AA-2001-06-12-0: replaced Xmt212 by Xmt310
 *                  (http://sourceforge.net/projects/motiftools) with
 *                  our xmt212 patches applied
 *
 *
 */

#include <Xmt/Xmt.h>

#if NeedFunctionPrototypes
int XmtBSearch(StringConst target, String *list, int num)
#else
int XmtBSearch(target, list, num)
StringConst target;
String *list;
int num;
#endif
{
    int half = num/2;
    int result;

    /* do a linear search if there aren't may keywords left */
    /* avoids the overhead of recursion */
    if (num <= 4) {
	for(num--; num >= 0; num--)
	    if (strcmp(target, list[num]) == 0) break;
	return num;
    }

    /* otherwise do a binary search */
    result = strcmp(target, list[half]);
    if (result == 0) return half;
    else if (result < 0)
	return XmtBSearch(target, list, half);
    else {
	result = XmtBSearch(target, &list[half+1], num-half-1);
	if (result != -1) result += half+1;
	return result;
    }
}

