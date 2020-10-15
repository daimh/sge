#ifndef __SGE_ERROR_CLASS_H
#define __SGE_ERROR_CLASS_H
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
#include "cull.h"
#include "sge.h"                /* for __attribute__ */

typedef struct sge_error_class_str sge_error_class_t;


typedef struct sge_error_iterator_class_str sge_error_iterator_class_t;

struct sge_error_iterator_class_str {
   void *sge_error_iterator_handle;
   
   const char* (*get_message)(sge_error_iterator_class_t *thiz);
   u_long32    (*get_quality)(sge_error_iterator_class_t *thiz);
   u_long32    (*get_type)(sge_error_iterator_class_t *thiz);
   bool        (*next)(sge_error_iterator_class_t *thiz);
};

struct sge_error_class_str {
   void *sge_error_handle;
   void (*error)(sge_error_class_t* thiz, int error_type, int error_quality, const char*fmt, ...) __attribute__ ((format (printf, 4, 5)));
   void (*verror)(sge_error_class_t* thiz, int error_type, int error_quality, const char*fmt, va_list ap);
   
   void (*clear)(sge_error_class_t *thiz);
   
   bool (*has_error)(sge_error_class_t *thiz);
   bool (*has_quality)(sge_error_class_t *thiz, int error_quality);
   bool (*has_type)(sge_error_class_t *thiz, int error_type);   
   sge_error_iterator_class_t* (*iterator)(sge_error_class_t *thiz);
};


sge_error_class_t* sge_error_class_create(void);
void sge_error_class_destroy(sge_error_class_t **error_handler);
void sge_error_iterator_class_destroy(sge_error_iterator_class_t** emc);


void showError(sge_error_class_t *eh);
void sge_error_to_answer_list(sge_error_class_t *eh, lList **alpp, bool clear_errors);
void sge_error_to_dstring(sge_error_class_t *eh, dstring *ds);

#endif /* __SGE_ERROR_CLASS_H */


