#ifndef __SGE_STDLIB_H
#define __SGE_STDLIB_H
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

#include <stdlib.h>
#include <sge.h>                /* for __attribute__ */
#include <sys/types.h>
#include <unistd.h>
void *sge_malloc(size_t size) __attribute__ ((__malloc__));

void *sge_realloc(void *ptr, int size, int do_abort);

void sge_free(void *cp);        

int sge_putenv(const char *var);
int sge_setenv(const char *name, const char *value);

int sge_setuid(uid_t);
int sge_seteuid(uid_t);
int sge_setgid(gid_t);
int sge_setegid(gid_t);
void sge_maybe_set_dumpable(void);

#endif /* __SGE_STDLIB_H */
