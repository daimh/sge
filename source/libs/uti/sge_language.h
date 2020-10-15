#ifndef __SGE_LANGUAGE_H
#define __SGE_LANGUAGE_H
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

#include "basis_types.h"

#ifdef __SGE_COMPILE_WITH_GETTEXT__ 
#define sge_init_language(x, y) sge_init_languagefunc((x), (y))
int sge_init_languagefunc(char *package, char *localeDir);
#else 
#define sge_init_language(x, y) 
#endif

#ifndef EXTRACT_MESSAGES
/* define all language function types */
typedef char* (*gettext_func_type)(char *);
typedef char* (*setlocale_func_type)(int lc, const char *name);
typedef char* (*bindtextdomain_func_type)(const char *domainname, const char *dirname);
typedef char* (*textdomain_func_type)(const char *donainname);

void sge_init_language_func(gettext_func_type, setlocale_func_type, bindtextdomain_func_type, textdomain_func_type);

const char *sge_gettext__(char *x);
const char *sge_gettext(char *x); /* applicatio code */
const char *sge_gettext_(int msg_id, const char *msg_str);
void sge_set_message_id_output(int flag);
int sge_get_message_id_output(void);

#endif

#endif /* __SGE_LANGUAGE_H */

