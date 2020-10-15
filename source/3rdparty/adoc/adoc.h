/*                                                               -*- C -*-
 *  ADOC.H
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

/* $VER: $Id: adoc.h,v 1.1 2001-07-18 11:05:53 root Exp $ */

#ifndef ADOC_H
#define ADOC_H

#include <stdio.h>

/* prototypes */

#if defined(__cplusplus) || defined(cplusplus)
extern "C" {
#endif

/* args.c */
extern char *convert_args(char *);
extern void display_args(int which);

/* strarg.c */
extern int strarg(char *key, ...);

/* strexpand.c */
extern char *strexpand(char *, char **);

/* gencode.c */
extern int gen_autodoc_toc(FILE *fp);
extern int gen_autodoc(FILE *fp, int cols, int tabsize, int flags, char **body_macros);
extern int gen_texinfo_header(FILE *fp, char *fname, char **header_macros);
extern int gen_texinfo(FILE *fp, int tabsize, int flags, char *body_environment, char **body_macros);

/* adoc.yy */
extern int read_source(char *, int, int);

/* main.c */
extern FILE *get_ferr(void);

/* flag whether or not autodoc comments must begin in column 1 or not */
#define SCANNER_ALLOW_INDENTED_COMMENTS  (1<<0)

/* flag whether or not to remove the indentation of body text lines */
#define SCANNER_UNINDENT_BODYTEXT        (1<<1)

/* flags for the texinfo code generator */

#define TEXI_GROUP_SECTIONS      (1<<0)
#define TEXI_FUNCTION_NEWPAGE    (1<<1)
#define TEXI_PARSE_REFERENCES    (1<<2)
#define TEXI_ITEMIZE_REFERENCES  (1<<3)
#define TEXI_CREATE_HEADER       (1<<4)
#define TEXI_TABLE_FUNCTIONS     (1<<5)
#define TEXI_NO_INDEX            (1<<6)

/* flags for the autodoc code generator */

#define ADOC_FORM_FEEDS          (1<<0)

/* initially `warn_mask' is set to WARN_NORMAL, so importatnt warnings are visible */

#define WARN_NORMAL              (1<<0)
#define WARN_BROKEN_COMMENTS     (1<<0)
#define WARN_STRANGE_TEXT        (1<<0)
#define WARN_UNKNOWN_KEYWORDS    (1<<1)
#define WARN_MISSING_KEYWORDS    (1<<2)
#define WARN_UNTITLED_SECTION    (1<<3)

#define WARN_ALL                 (~0)
#define WARN_NONE                (0)

#if defined(__cplusplus) || defined(cplusplus)
}
#endif /* C++ */

#endif /* !ADOC_H */
