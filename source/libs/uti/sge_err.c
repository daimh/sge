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

#include <stdio.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h"
#include "uti/sge_err.h"
#include "uti/sge_string.h"

#define ERR_LAYER TOP_LAYER

struct _sge_err_object_t {
   sge_err_t id;
   char message[SGE_ERR_MAX_MESSAGE_LENGTH];
};

typedef struct _sge_err_object_t sge_err_object_t;

static pthread_once_t sge_err_once = PTHREAD_ONCE_INIT;
static pthread_key_t sge_err_key;

/* destructor function that will be called when a thread ends */
static void 
sge_err_destroy(void* state)
{
   sge_free(&state);
}

/* init function that initializes the key that holds the pointer
 * to thread local storrage for this module */
static void 
sge_err_once_init(void)
{
   pthread_key_create(&sge_err_key, sge_err_destroy);
}

/* initialization function used to initialize thread local storrage */
static void 
sge_err_object_init(sge_err_object_t *object) {
   object->id = SGE_ERR_SUCCESS;   
   object->message[0] = '\0';
}

/* function that returns thread local storrage for this module */
static bool
sge_err_get_object(sge_err_object_t **object) {
   bool ret = true;

   DENTER(ERR_LAYER, "sge_err_get_object");
   *object = pthread_getspecific(sge_err_key);
   if (*object == NULL) {
      sge_err_object_t *new_object = (sge_err_object_t *)malloc(sizeof(sge_err_object_t));
      int pthread_ret = pthread_setspecific(sge_err_key, (void*)new_object);

      if (pthread_ret == 0) {
         sge_err_object_init(new_object);
         *object = new_object;
      } else {
         ERROR(("pthread_setspecific failed to initialize sge_err_object_t in %s\n", SGE_FUNC));
         abort();
      }
   }
   DRETURN(ret);
}

/* local function that sets the error id and the error message (format + variable arguments) */
static void
sge_err_vset(sge_err_t id, const char *format, va_list args) {
   sge_err_object_t *err_obj = NULL;

   DENTER(ERR_LAYER, "sge_err_vset");
   sge_err_get_object(&err_obj);
   err_obj->id = id;
   vsnprintf(err_obj->message, SGE_ERR_MAX_MESSAGE_LENGTH, format, args);
   DEXIT;
}

/* initialization function that has to be called before threads are spawned */
void 
sge_err_init(void) {
   DENTER(ERR_LAYER, "sge_err_init");
   pthread_once(&sge_err_once, sge_err_once_init);
   DEXIT;
}

void
sge_err_set(sge_err_t id, const char *format, ...) {
   va_list args;
  
   DENTER(ERR_LAYER, "sge_err_set");
   if (format != NULL) {
      va_start(args, format);
      sge_err_vset(id, format, args);
      va_end(args);
   }
   DEXIT;
}

void
sge_err_get(sge_err_t *id, char *message, size_t size) {
   DENTER(ERR_LAYER, "sge_err_get");
   if (id != NULL && message != NULL && size > 0) {
      sge_err_object_t *err_obj = NULL;

      sge_err_get_object(&err_obj);
      if (err_obj->id != SGE_ERR_SUCCESS) {
         *id = err_obj->id;
         sge_strlcpy(message, err_obj->message, size);
      } else {
         *id = SGE_ERR_SUCCESS;
         message[0] = '\0';
      }
   }
   DEXIT;
}

bool
sge_err_has_error(void) {
   sge_err_object_t *err_obj = NULL;
   bool ret;

   DENTER(ERR_LAYER, "sge_err_has_error");
   sge_err_get_object(&err_obj);
   ret = (err_obj->id != SGE_ERR_SUCCESS) ? true : false;
   DRETURN(ret);
}

void
sge_err_clear(void) {
   sge_err_object_t *err_obj = NULL;

   DENTER(ERR_LAYER, "sge_err_clear");
   sge_err_get_object(&err_obj);
   err_obj->id = SGE_ERR_SUCCESS;
   DEXIT;
}
