/*                                                               -*- C -*-
 *  LIBFUN.H
 *
 *  (c)Copyright 1995 by Tobias Ferber,  All Rights Reserved
 *
 *  This file is part of ADOC.
 *
 *  ADOC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 1 of the License,
 *  or (at your option) any later version.
 *
 *  ADOC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $VER: $Id: libfun.h,v 1.1 2001-07-18 11:05:53 root Exp $ */

#ifndef LIBFUN_H
#define LIBFUN_H

/* prototypes */

#if defined(__cplusplus) || defined(cplusplus)
extern "C" {
#endif

#ifndef __P

#if defined (__STDC__) || defined(__cplusplus)
#define __P(protos) protos
#else /* !(__STDC__ || __cplusplus) */
#define __P(protos) ()
#endif /* __STDC__ || __cplusplus */

#endif /* !__P */


extern int newfun   __P( (char *libfun) );
/* add a function `libfun' = "library/function" */

extern int newsec   __P( (char *title) );
/* begin a new section `title' in the description of the current function */

extern int addtext  __P( (char *text) );
/* add a portion of text to the current section */

extern void funsort  __P( (void) );
/* sort the functions alphabetically */

extern int funexpand __P( (char **macros) );
/* expand the body text of all functions via strexpand() */

extern int funindent __P( (int indent, int tabsize) );
/* rework indentation of all body text lines */

extern char *getfun __P( (char *name) );
/* get the name of function `name' or the current function if `name' == (char *)0 */

extern char *getsec __P( (char *name) );
/* get the text of section `name' or the current section if `name' == (char *)0 */

extern char *stepfun __P( (int trigger) );
/* get the name of a function: next = 1, previous = -1, first = 0 */

extern char *stepsec __P( (int trigger) );
/* get the name of a section: next = 1, previous = -1, first = 0 */

extern char *pushfun __P( (void) );
extern char *popfun __P( (void) );

extern int islib __P( (char *name) );
/* return 1 if there is at least one function `name/...' or 0 otherwise */

extern void funfree  __P( (void) );
/* delete all functions and their descriptive data */

#if defined(__cplusplus) || defined(cplusplus)
}
#endif /* C++ */

#endif /* !LIBFUN_H */
