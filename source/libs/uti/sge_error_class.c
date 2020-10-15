/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of thiz file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of thiz file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use thiz file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under thiz License is provided on an "AS IS" basis,
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

#include <string.h>
#include <stdio.h>

#include "uti/sge_rmon.h"
#include "uti/sge_error_class.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_answer.h"

#include "sge.h"

typedef struct sge_error_message_str sge_error_message_t;


struct sge_error_message_str {
   int error_quality;
   int error_type;
   char *message;
   sge_error_message_t *next;
};

typedef struct sge_error_iterator_str sge_error_iterator_t;

struct sge_error_iterator_str {
   bool is_first_flag;
   sge_error_message_t *current;
};

typedef struct {
   sge_error_message_t *first;
   sge_error_message_t *last;
} sge_error_t;

static sge_error_iterator_class_t* sge_error_class_iterator(sge_error_class_t *thiz);
static bool sge_error_has_error(sge_error_class_t* eh);
static bool sge_error_has_quality(sge_error_class_t* thiz, int error_quality);
static bool sge_error_has_type(sge_error_class_t* thiz, int error_type);

static void sge_error_verror(sge_error_class_t* thiz, int error_type, int error_quality, 
                            const char*format, va_list ap);
static void sge_error_error(sge_error_class_t* thiz, int error_type, int error_quality, 
                            const char*fmt, ...);
                            
static void sge_error_clear(sge_error_t* et);
static void sge_error_class_clear(sge_error_class_t* thiz);
void sge_error_destroy(sge_error_t **t);
void sge_error_message_destroy(sge_error_message_t** elem);

static sge_error_iterator_class_t* sge_error_iterator_class_create(sge_error_class_t *ec);
static const char* sge_error_iterator_get_message(sge_error_iterator_class_t* emc);
static u_long32 sge_error_iterator_get_quality(sge_error_iterator_class_t* emc);
static u_long32 sge_error_iterator_get_type(sge_error_iterator_class_t* emc);
static bool sge_error_iterator_next(sge_error_iterator_class_t* thiz);

                               
sge_error_class_t* sge_error_class_create(void) {
   
   sge_error_class_t *ret = (sge_error_class_t*)sge_malloc(sizeof(sge_error_class_t));
   if( ret == NULL ) {
      return NULL;
   }
   memset(ret,0,sizeof(sge_error_class_t));

   ret->sge_error_handle = sge_malloc(sizeof(sge_error_t));
   memset(ret->sge_error_handle, 0, sizeof(sge_error_t));
   
   ret->has_error = sge_error_has_error;
   ret->has_quality = sge_error_has_quality;
   ret->has_type = sge_error_has_type;
   ret->error = sge_error_error;
   ret->verror = sge_error_verror;
   ret->clear = sge_error_class_clear;
   ret->iterator = sge_error_class_iterator;
   return ret;
}

static sge_error_iterator_class_t* sge_error_class_iterator(sge_error_class_t *thiz)
{
   return sge_error_iterator_class_create(thiz);
}

void sge_error_class_destroy(sge_error_class_t **ec) 
{
   sge_error_t *et = NULL;
   
   if ( ec == NULL  || *ec == NULL )
      return;
   
   et = (sge_error_t*)(*ec)->sge_error_handle;
   
   sge_error_destroy(&et);   
   sge_free(ec);
}

static void sge_error_class_clear(sge_error_class_t* thiz) {
   if (thiz != NULL) {
      sge_error_t *et = (sge_error_t*)thiz->sge_error_handle;
      sge_error_clear(et);
   }
}

static void sge_error_clear(sge_error_t *et) {
   DENTER(TOP_LAYER, "sge_error_clear");
   
   if (et != NULL) {
      sge_error_message_t *elem = et->first;
      sge_error_message_t *next;
      while (elem != NULL) {
         next = elem->next;
         sge_error_message_destroy(&elem);
         elem = next;
      }
      et->first = NULL;
      et->last = NULL;
   }

   DEXIT;
}


void sge_error_destroy(sge_error_t **t) {
   if (t == NULL || *t == NULL) {
      return;
   }

   sge_error_clear(*t);
   sge_free(t);
}


void sge_error_message_destroy(sge_error_message_t** elem) {
   if (elem == NULL || *elem == NULL) {
      return;
   }
   sge_free(&((*elem)->message));
   sge_free(elem);
}

static bool sge_error_has_error(sge_error_class_t* eh) {   
   sge_error_t *et = (sge_error_t*)eh->sge_error_handle;
   return (et->first != NULL) ? true : false;
}

static bool sge_error_has_quality(sge_error_class_t* thiz, int error_quality) {
   bool ret = false;
   
   DENTER(TOP_LAYER, "sge_error_has_quality");
   if(thiz) {
      sge_error_t *et = (sge_error_t*)thiz->sge_error_handle;
      sge_error_message_t *elem = et->first;
      while(elem) {
         if(elem->error_quality == error_quality ) {
            ret = true;
            break;
         }
      }
   }
   DEXIT;
   return ret;
}

static bool sge_error_has_type(sge_error_class_t* thiz, int error_type) 
{
   bool ret = false;
   
   DENTER(TOP_LAYER, "sge_error_has_type");
   if(thiz) {
      sge_error_t *et = (sge_error_t*)thiz->sge_error_handle;
      sge_error_message_t *elem = et->first;
      while(elem) {
         if(elem->error_type == error_type ) {
            ret = true;
            break;
         }
      }
   }
   DEXIT;
   return ret;
}

static sge_error_iterator_class_t* sge_error_iterator_class_create(sge_error_class_t *ec)
{
   sge_error_iterator_class_t* ret = NULL;
   sge_error_iterator_t *elem = NULL;
   sge_error_t *et = (sge_error_t*)ec->sge_error_handle;
   
   DENTER(TOP_LAYER, "sge_error_message_class_create");
   
   elem = (sge_error_iterator_t*)sge_malloc(sizeof(sge_error_iterator_t));
   elem->current = et->first;
   elem->is_first_flag = true;
   
   ret = (sge_error_iterator_class_t*)sge_malloc(sizeof(sge_error_iterator_class_t));
   ret->sge_error_iterator_handle = elem;
   ret->get_message = sge_error_iterator_get_message;
   ret->get_quality = sge_error_iterator_get_quality;
   ret->get_type = sge_error_iterator_get_type;
   ret->next = sge_error_iterator_next;
   
   DEXIT;
   return ret;
}

static void sge_error_verror(sge_error_class_t* thiz, int error_type, int error_quality, 
                            const char*format, va_list ap) {
   
   sge_error_message_t *error = NULL;
   sge_error_t *et = (sge_error_t*)thiz->sge_error_handle;
   dstring ds = DSTRING_INIT;
   
   DENTER(TOP_LAYER, "sge_error_verror");

   error = (sge_error_message_t*)sge_malloc(sizeof(sge_error_message_t));

   error->error_quality = error_quality;
   error->error_type = error_type;

   sge_dstring_vsprintf(&ds, format, ap);
   
   error->message = strdup(sge_dstring_get_string(&ds));
   error->next = NULL;
   sge_dstring_free(&ds);
   
   DPRINTF(("error: %s\n", error->message ? error->message : ""));

   if (et->first == NULL) {
      et->first = error;      
      et->last = error;
   } else {
      et->last->next = error;
      et->last = error;
   }

   DEXIT;
}


static void sge_error_error(sge_error_class_t* thiz, int error_type, int error_quality, 
                            const char*format, ...) {
   DENTER(TOP_LAYER, "sge_error_error");
   if (format != NULL) {
      va_list ap;

      va_start(ap, format);
      sge_error_verror(thiz, error_type, error_quality, format, ap);
      va_end(ap);
   }
   DEXIT;
}


void sge_error_iterator_class_destroy(sge_error_iterator_class_t** thiz)
{
   sge_error_iterator_t *elem = NULL;
   
   if (!thiz) {
      return;
   }

   elem = (sge_error_iterator_t *)(*thiz)->sge_error_iterator_handle;

   sge_free(&elem);
   sge_free(thiz);
}

static const char* sge_error_iterator_get_message(sge_error_iterator_class_t* thiz)
{
   sge_error_iterator_t *elem = (sge_error_iterator_t*)thiz->sge_error_iterator_handle;

   if (elem && elem->current) {
      return elem->current->message;
   }
   return NULL;
}

static u_long32 sge_error_iterator_get_quality(sge_error_iterator_class_t* thiz)
{
   sge_error_iterator_t *elem = (sge_error_iterator_t*)thiz->sge_error_iterator_handle;

   if (elem && elem->current) {
      return elem->current->error_quality;
   }
   return -1;
}

static u_long32 sge_error_iterator_get_type(sge_error_iterator_class_t* thiz)
{
   sge_error_iterator_t *elem = (sge_error_iterator_t*)thiz->sge_error_iterator_handle;

   if (elem && elem->current) {
      return elem->current->error_type;
   }
   return -1;
}

static bool sge_error_iterator_next(sge_error_iterator_class_t* thiz)
{
   sge_error_iterator_t *elem = (sge_error_iterator_t*)thiz->sge_error_iterator_handle;

   if (!elem) {
     return false;
   }
   if (elem->is_first_flag ) {
      elem->is_first_flag = false;
      return (elem->current != NULL) ? true : false;
   } 
   if (elem->current) {
      elem->current = elem->current->next;
      
      return (elem->current != NULL) ? true : false;
   }
   return false;
}


void showError(sge_error_class_t *eh) {
   sge_error_iterator_class_t *iter = NULL;
   dstring ds = DSTRING_INIT;
   bool first = true;
   
   iter = eh->iterator(eh);

   while (iter && iter->next(iter)) {
      if (first) {
         first = true;
      } else {   
         sge_dstring_append(&ds, "\n");
      }   
      sge_dstring_append(&ds, iter->get_message(iter));
   }
   printf("%s\n", sge_dstring_get_string(&ds));
   sge_dstring_free(&ds);

}

void sge_error_to_answer_list(sge_error_class_t *eh, lList **alpp, bool clear_errors) {
   sge_error_iterator_class_t *iter = NULL;
   
   if (eh == NULL || alpp == NULL) {
      return;
   }
   iter = eh->iterator(eh);
   while (iter && iter->next(iter)) {
      answer_list_add(alpp, 
                      iter->get_message(iter), 
                      iter->get_type(iter), 
                      (answer_quality_t)iter->get_quality(iter));
   }
   if (clear_errors) {
      sge_error_class_clear(eh);
   }

   sge_error_iterator_class_destroy(&iter);
}

void sge_error_to_dstring(sge_error_class_t *eh, dstring *ds) {
   sge_error_iterator_class_t *iter = NULL;
   bool first = true;
   
   iter = eh->iterator(eh);

   while (iter && iter->next(iter)) {
      if (first) {
         first = false;
      } else {   
         sge_dstring_append(ds, "\n");
      }   
      sge_dstring_append(ds, iter->get_message(iter));
   }
}

