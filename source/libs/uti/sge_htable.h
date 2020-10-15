#ifndef __SGE_HASH_H
#define __SGE_HASH_H
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
/* Portions of this code are Copyright (c) 2011 Univa Corporation. */
/*___INFO__MARK_END__*/
/*
 * Based on the code of David Flanagan's Xmt library
 */

#define True   1
#define False  0 

#include "sge_dstring.h"

typedef struct _htable_rec *htable;

typedef void (*sge_htable_for_each_proc)(
    htable, const void*, const void**
);

htable sge_htable_create(int size, const void *(*dup_func)(const void *), int (*hash_func)(const void *), int (*compare_func)(const void *, const void *));
void sge_htable_destroy(htable ht);
void sge_htable_store(htable ht, const void* key, const void* data);
int sge_htable_lookup(htable ht, const void* key, const void** data);
void sge_htable_delete(htable ht, const void* key);
void sge_htable_for_each(htable ht, sge_htable_for_each_proc proc);

const char *sge_htable_statistics(htable ht, dstring *buffer);

const void *dup_func_u_long32(const void *key) __attribute__ ((malloc));
const void *dup_func_u_long64(const void *key) __attribute__ ((malloc));
const void *dup_func_string(const void *key) __attribute__ ((malloc));
const void *dup_func_long(const void *key) __attribute__ ((malloc));
const void *dup_func_pointer(const void *key) __attribute__ ((malloc));

/* fixme:  candidates for inline?  */
int hash_func_u_long32(const void *key) __attribute__ ((__pure__));
int hash_func_u_long64(const void *key) __attribute__ ((__pure__));
int hash_func_string(const void *key) __attribute__ ((__pure__));
int hash_func_long(const void *key) __attribute__ ((__pure__));
int hash_func_pointer(const void *key) __attribute__ ((__pure__));

/* fixme:  candidates for inline?  */
int hash_compare_u_long32(const void *a, const void *b) __attribute__ ((__pure__));
int hash_compare_u_long64(const void *a, const void *b) __attribute__ ((__pure__));
int hash_compare_string(const void *a, const void *b) __attribute__ ((__pure__));
int hash_compare_long(const void *a, const void *b) __attribute__ ((__pure__));
int hash_compare_pointer(const void *a, const void *b) __attribute__ ((__pure__));

int hash_compute_size(int number_of_elem) __attribute__ ((__pure__));

#endif /* __SGE_HASH_H */
