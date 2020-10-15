/*
 * $XConsortium: ifparser.h /main/4 1996/09/28 16:15:24 rws $
 *
 * Copyright 1992 Network Computing Devices, Inc.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices may not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Network Computing Devices makes
 * no representations about the suitability of this software for any purpose.
 * It is provided ``as is'' without express or implied warranty.
 * 
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * 
 * Author:  Jim Fulton
 *          Network Computing Devices, Inc.
 * 
 * Simple if statement processor
 *
 * This module can be used to evaluate string representations of C language
 * if constructs.  It accepts the following grammar:
 * 
 *     EXPRESSION	:=	VALUE
 * 			 |	VALUE  BINOP	EXPRESSION
 * 
 *     VALUE		:=	'('  EXPRESSION  ')'
 * 			 |	'!'  VALUE
 * 			 |	'-'  VALUE
 * 			 |	'defined'  '('  variable  ')'
 * 			 |	variable
 * 			 |	number
 * 
 *     BINOP		:=	'*'	|  '/'	|  '%'
 * 			 |	'+'	|  '-'
 * 			 |	'<<'	|  '>>'
 * 			 |	'<'	|  '>'	|  '<='  |  '>='
 * 			 |	'=='	|  '!='
 * 			 |	'&'	|  '|'
 * 			 |	'&&'	|  '||'
 * 
 * The normal C order of precidence is supported.
 * 
 * 
 * External Entry Points:
 * 
 *     ParseIfExpression		parse a string for #if
 */

#include <stdio.h>

#define const /**/
typedef int Bool;
#define False 0
#define True 1

typedef struct _if_parser IfParser;

#ifdef __STDC__
typedef char *(*ErrorHandlerT) (IfParser *, const char *, const char *); 
typedef long (*EvalVariableT) (IfParser *, const char *, int); 
typedef int (*EvalDefinedT) (IfParser *, const char *, int); 
#else
typedef char *(*ErrorHandlerT) (); 
typedef long (*EvalVariableT) (); 
typedef int (*EvalDefinedT) (); 
#endif

struct _if_parser {
    struct {				/* functions */
      ErrorHandlerT handle_error;
      EvalVariableT eval_variable;
      EvalDefinedT eval_defined;
    } funcs;
    char *data;
};

char *ParseIfExpression (
#ifdef __STDC__
    IfParser *, 
    const char *, 
    long *
#endif
);

