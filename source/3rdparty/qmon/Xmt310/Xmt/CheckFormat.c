/* 
 * Motif Tools Library, Version 3.1
 * $Id: CheckFormat.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: CheckFormat.c,v $
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
#include <string.h>
#include <ctype.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern long int strtol();
#endif

#if NeedFunctionPrototypes
static int gettype(char *s, char *type, int *pos, char **newstr)
#else
static int gettype(s, type, pos, newstr)
char *s;
char *type;
int *pos;
char **newstr;
#endif
{
    int longer = False;

    /* look for a format specifiction, but not a %% */
    while(*s) {
	if (*s == '%') {
	    s++;
	    if (*s && (*s != '%')) break;
	}
	s++;
    }

    if (!*s) return 0;  /* end of string */

    /*
     * here, s[-1] is the '%' char.
     * s[0] is known not to be '%'.
     */

    /* first check for a positional argument */
    if (isdigit(*s) && (*(s + strspn(s, "0123456789")) == '$')) {
	*pos = strtol(s, &s, 0);
	*pos -= 1;  /* convered 1-based to 0-based */
	s++;  /* skip the '$' */
    }
    else
	*pos = -1;

    /* now skip any flags */
    s += strspn(s, "-+ #");

    /*
     * skip any field width and precision.
     * note that we don't handle the '*' or '*ddd$' syntax
     * here for widths and precisions from the arg list.
     * tough luck.  These probably aren't portable, anyway.
     */
    s += strspn(s, "0123456789.");

    /*
     * now there are three possible size characters: 'h', 'l', and 'L'.
     * we can ignore h; it just controls how printing is done; it doesn't
     * specify anything about the required argument type.  The other 2 do.
     */
    if (*s == 'h') s++;
    else if ((*s == 'l') || (*s == 'L')) { s++; longer = True; }

    /*
     * Now check for the type character.  All we care about is the
     * type of the expected argument, not the actual format, and
     * args are widened, so we don't care about short or float.
     * We map all the format characters to 8 types:
     *   d  integer
     *   D  long
     *   f  double
     *   F  long double
     *   c  char
     *   s  string
     *   p  pointer
     *   n  address of int
     */
    switch(*s) {
    case 'd':
    case 'i':
    case 'o':
    case 'u':
    case 'x':
    case 'X':
	if (longer) *type = 'D'; else *type = 'd';
	break;
    case 'f':
    case 'g':
    case 'e':
    case 'G':
    case 'E':
	if (longer) *type = 'F'; else *type = 'f';
	break;
    case 'c':
	*type = 'c';
	break;
    case 's':
	*type = 's';
	break;
    case 'p':
	*type = 'p';
	break;
    case 'n':
	*type = 'n';
	break;
    default:
	*type = '\0';  /* bad format */
    }

    if (*s) s++;
    *newstr = s;
    return 1;
}

#if NeedFunctionPrototypes
Boolean XmtCheckPrintfFormat(StringConst template, StringConst msg)
#else
Boolean XmtCheckPrintfFormat(template, msg)
String template;
String msg;
#endif
{
    char types[100];  /* no conversion will have > 100 args */
    char type;
    int pos;
    char *s;
    int nextpos;

    memset(types, 0, sizeof(types));

    /* figure out what args are expected by the template string */
    s = (char *) template;
    nextpos = 0;
    while(gettype(s, &type, &pos, &s)) {
	if (!type) return False;  /* error; bad format */
	if (pos >= 0) types[pos] = type;
	else types[nextpos++] = type;
    }    

    /* now see if the other string matches these */
    s = (char *)msg;
    nextpos = 0;
    while(gettype(s, &type, &pos, &s)) {
	if (!type) return False;  /* error; bad format */
	if (pos == -1) pos = nextpos++;
	if (type != types[pos]) return False; /* error; bad type */
    }
    return True;
}

