/* 
 * Motif Tools Library, Version 3.1
 * $Id: HSLtoRGB.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: HSLtoRGB.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
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


/*
 * 0 <= h <= 359
 * 0 <= s,l <= 100
 * 0 <= r,g,b <= 65535
 */
#if NeedFunctionPrototypes
void XmtHSLToRGB(unsigned h, unsigned s, unsigned l,
		 unsigned *r, unsigned *g, unsigned *b)
#else
void XmtHSLToRGB(h, s, l, r, g, b)
unsigned int h;
unsigned int s;
unsigned int l;
unsigned int *r;
unsigned int *g;
unsigned int *b;
#endif
{
    int v;
    int m, sv, fract, vsf, mid1, mid2, sextant;

    v = (l < 50) ? (l * (s + 100) / 100) : (l + s - (l * s / 100));
    if (v <= 0) { *r = *g = *b = 0; return;}

    m = l + l - v;
    sv = 100 * (v - m) / v;

    sextant = h/60;
    fract = 100 * (h - (sextant * 60)) / 60;
    vsf = v * sv * fract / 10000;
    mid1 = m + vsf;
    mid2 = v - vsf;

    switch (sextant) {
	case 0: *r = v; *g = mid1; *b = m; break;
	case 1: *r = mid2; *g = v; *b = m; break;
	case 2: *r = m; *g = v; *b = mid1; break;
	case 3: *r = m; *g = mid2; *b = v; break;
	case 4: *r = mid1; *g = m; *b = v; break;
	case 5: *r = v; *g = m; *b = mid2; break;
    }

    *r = (*r * 65535) / 100;
    *g = (*g * 65535) / 100;
    *b = (*b * 65535) / 100;
}
