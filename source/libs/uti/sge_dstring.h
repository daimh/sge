#ifndef __SGE_STRING_APPEND_H
#define __SGE_STRING_APPEND_H
/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "sge.h"                /* for __attribute__ */

/****** uti/dstring/DSTRING_INIT **********************************************
*  NAME
*     DSTRING_INIT -- Define to initialize dstring variables 
*
*  SYNOPSIS
*     #define DSTRING_INIT {NULL, 0, 0}
*
*  FUNCTION
*     Define to preinitialize dstring variables 
*
*  EXAMPLE
*     {
*        dstring error_msg = DSTRING_INIT;
*     }
*  NOTE 
*     The DSTRING_INIT counterpart for static buffers is sge_dstring_init()
******************************************************************************/

#include <sys/types.h>
#include <stdarg.h>

#include "basis_types.h"

#include "sge_stdlib.h"

#define DSTRING_INIT {NULL, 0, 0, false}

typedef struct {
   char *s;        /* refers to allocated buffer with dynamic dstrings
                    *  or static buffer with static dstrings 
                    */
   size_t length;  /* length of the string */
   size_t size;    /* size of the string buffer */
   bool is_static;  /* is it a static or a dynamic buffer? */
} dstring;

/* DSTRING_INIT counterpart when static buffers are wrapped with dstring */
void sge_dstring_init(dstring *sb, char *buffer, size_t size);

const char* sge_dstring_append(dstring *sb, const char *a);
const char* sge_dstring_append_dstring(dstring *sb, const dstring *a);
const char* sge_dstring_append_char(dstring *sb, const char a);
const char* sge_dstring_append_time(dstring *sb, time_t time, bool as_xml);
const char* sge_dstring_append_mailopt(dstring *sb, u_long32 mailopt);

const char* sge_dstring_sprintf(dstring *sb, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
const char* sge_dstring_vsprintf(dstring *sb, const char *fmt, va_list ap);
const char* sge_dstring_sprintf_append(dstring *sb, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

void sge_dstring_clear(dstring *sb);
void sge_dstring_free(dstring *sb);

/* fixme: make inline?  */
const char *sge_dstring_get_string(const dstring *string) __attribute__ ((__pure__));

const char* sge_dstring_copy_string(dstring *sb, const char* str);

const char* sge_dstring_copy_dstring(dstring *sb1, const dstring *sb2);

size_t sge_dstring_strlen(const dstring *string) __attribute__ ((__pure__));

size_t sge_dstring_remaining(const dstring *string) __attribute__ ((__pure__));

const char *sge_dstring_ulong_to_binstring(dstring *sb, u_long32 number);

bool sge_dstring_split(dstring *string, char character, dstring *before, dstring *after);

void sge_dstring_strip_white_space_at_eol(dstring *string);

#endif /* __SGE_STRING_APPEND_H */
