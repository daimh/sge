/* 
 * Motif Tools Library, Version 3.1
 * $Id: RGBtoHSL.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: RGBtoHSL.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>

/*
 * RGB <--> HSL conversions were modified from code with the
 * following attributions;
 *
 *    A Fast HSL-to-RGB Transform
 *    by Ken Fishkin, Pixar Inc., January 1989.
 *    from "Graphics Gems", Academic Press, 1990
 */

#define MIN(a,b)        (((a)<(b))?(a):(b))
#define MAX(a,b)        (((a)>(b))?(a):(b))

#if NeedFunctionPrototypes
void XmtRGBToHSL(unsigned r, unsigned g, unsigned b,
		 unsigned *h, unsigned *s, unsigned *l)
#else
void XmtRGBToHSL(r, g, b, h, s, l)
unsigned int r;
unsigned int g;
unsigned int b;
unsigned int *h;
unsigned int *s;
unsigned int *l;
#endif
{
    unsigned int v, m, vm, r2, g2, b2;
    unsigned int h2, s2, l2;

    h2 = s2 = l2 = 0;

    v = MAX(r,g);
    v = MAX(v,b);
    m = MIN(r,g);
    m = MIN(m,b);

    if ((l2 = (m + v)/2) == 0) goto done;
    if ((s2 = vm = (v - m)) == 0) goto done;
    else s2 = (s2 << 16) / ((l2 < 32768) ? (v+m) : (131072 - v - m));

    r2 = ((((v-r) << 16) / vm) * 60) >> 16;
    g2 = ((((v-g) << 16) / vm) * 60) >> 16;
    b2 = ((((v-b) << 16) / vm) * 60) >> 16;

    if (r == v)
	h2 = (g == m ? 300 + b2 : 60 - g2);
    else if (g == v)
	h2 = (b == m ? 60 + r2 : 180 - b2);
    else
	h2 = (r == m ? 180 + g2 : 300 - r2);

 done:
    *h = h2;
    *s = (s2 * 100) >> 16;
    *l = (l2 * 100) >> 16;
}
