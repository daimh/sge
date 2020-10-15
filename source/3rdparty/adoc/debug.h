/*                                                               -*- C -*-
 *  DEBUG.H
 *
 *  (c)Copyright 1991-93 by Tobias Ferber,  All Rights Reserved.
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

/* $VER: $Id: debug.h,v 1.1 2001-07-18 11:05:53 root Exp $ */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stdio.h>

#ifdef DEBUG

extern int   bug_level;
extern FILE *bug_stream;

/*
#define D(x)  (x)
#define DD(x) while(0)
*/

#define D(x)  (x)
#define DD(x) if(bug_level > 20) (x)

#include "smartmem.h"
#define AUTO_ADVANCE_MEM_HANDLE

#include "timer.h"
#define TIMER_HANDLE 1

/*#define bug  printf*/
extern void bug(const char *, ...);
extern void bug_enter(const char *, ...);
extern void bug_leave(const char *, ...);

extern void bug_init(int, FILE *);
extern void bug_exit(void);

#else /* !DEBUG */

#define D(x)   while(0)
#define DD(x)  while(0)

#endif /* DEBUG */

#endif /* !DEBUG_H */
