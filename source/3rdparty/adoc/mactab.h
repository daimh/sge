/*                                                               -*- C -*-
 *  MACTAB.H
 *
 *  (c)Copyright 1995 by Tobias Ferber,  All Rights Reserved
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 1 of the License,
 *  or (at your option) any later version.
 *
 *  This file is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $VER: $Id: mactab.h,v 1.1 2001-07-18 11:05:53 root Exp $ */

#ifndef MACTAB_H
#define MACTAB_H

/* prototypes */

#if defined(__cplusplus) || defined(cplusplus)
extern "C" {
#endif

/* see mactab.doc for further details */

extern int      mactab_new      (int num_macros);
extern int      mactab_dispose  (int handle);
extern int      mactab_add      (int handle, ...);
extern int      mactab_remove   (int handle, ...);
extern char *   mactab_get      (int handle, char *lhs);
extern char **  mactab          (int handle);

#ifdef DEBUG
#include <stdio.h>
extern void     mactab_debug    (FILE *fp);
#endif

#if defined(__cplusplus) || defined(cplusplus)
}
#endif /* C++ */

#endif /* !MACTAB_H */
