#ifndef __SGE_STRING_H
#define __SGE_STRING_H
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

#ifdef  __cplusplus
extern "C" {
#endif

#include "sge.h"
#include "sge_dstring.h"

struct saved_vars_s {
   char *static_cp;
   char *static_str;
};

const char *sge_basename(const char *name, int delim);

const char *sge_jobname(const char *name);

char *sge_delim_str(char *str, char **delim_pos, const char *delim);

char *sge_dirname(const char *name, int delim);

char *sge_strdup(char *old, const char *src) __attribute__ ((malloc));

char *sge_strtok(const char *str, const char *delimitor);

bool sge_is_pattern(const char *p) __attribute__ ((__pure__));

bool sge_is_expression(const char *p) __attribute__ ((__pure__));

char *sge_strtok_r(const char *str, const char *delimitor, 
                   struct saved_vars_s **last);

void sge_free_saved_vars(struct saved_vars_s *last);

int sge_strnullcasecmp(const char *a, const char *b) __attribute__ ((__pure__));

int sge_strnullcmp(const char *a, const char *b) __attribute__ ((__pure__));

int sge_patternnullcmp(const char *str, const char *pattern); 

void sge_strip_blanks(char *str);

void sge_strip_white_space_at_eol(char *str);

void sge_strip_slash_at_eol(char *str);

void sge_strtoupper(char *buffer, int max_len);

void sge_strtolower(char *buffer, int max_len);

int sge_strisint(const char *str) __attribute__ ((__pure__));

char **sge_stradup(char **cpp, int n);

void sge_strafree(char ***cpp); 

char **sge_stramemncpy(const char *cp, char **cpp, int n);

size_t sge_strlcat(char *dst, const char *src, size_t dstsize);
size_t sge_strlcpy(char *dst, const char *src, size_t dstsize);

char **sge_stracasecmp(const char *cp, char **cpp);

char **stra_from_str(const char *, const char *delim);

void stra_printf(char *stra[]);

void sge_compress_slashes(char *str);

void sge_strip_quotes(char **pstr);

char **string_list(char *str, char *delis, char **pstr);

const char *sge_strerror(int errnum, dstring *buffer);

bool sge_str_is_number(const char *string) __attribute__ ((__pure__));

const char *sge_replace_substring(const char *input, const char *old, const char *new);

const char *unescape_env_value(const char *value);

#ifndef WIN32NATIVE
#define SGE_STRCASECMP(a, b) strcasecmp(a, b)
#else
#define SGE_STRCASECMP(a, b) stricmp(a, b)
#endif

#ifdef  __cplusplus
}
#endif
 
#endif /* __SGE_STRING_H */

