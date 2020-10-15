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

#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>  

#include "uti/sge_rmon.h"
#include "uti/sge_hostname.h"
#include "uti/sge_log.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"
#include "uti/sge_unistd.h"
#include "uti/sge_env.h"

#include "sgeobj/sge_answer.h"

#include "sge.h"

typedef struct {
   char*       sge_root;
   char*       sge_cell;
   u_long32    sge_qmaster_port;
   u_long32    sge_execd_port;
   bool        from_services;
   bool        qmaster_internal;
} sge_env_state_t;

static bool sge_env_state_setup(sge_env_state_class_t *thiz, 
                                const char *sge_root, 
                                const char *sge_cell, 
                                u_long32 sge_qmaster_port, 
                                u_long32 sge_execd_port, 
                                bool from_services,
                                bool is_qmaster_internal,
                                sge_error_class_t *eh);
static void sge_env_state_destroy(void *theState);
static void sge_env_state_dprintf(sge_env_state_class_t *thiz);
static const char* get_sge_root(sge_env_state_class_t *thiz);
static const char* get_sge_cell(sge_env_state_class_t *thiz);
static u_long32 get_sge_qmaster_port(sge_env_state_class_t *thiz);
static u_long32 get_sge_execd_port(sge_env_state_class_t *thiz);
static bool is_from_services(sge_env_state_class_t *thiz);
static bool is_qmaster_internal(sge_env_state_class_t *thiz);
static void set_sge_root(sge_env_state_class_t *thiz, const char *sge_root);
static void set_sge_cell(sge_env_state_class_t *thiz, const char *sge_cell);
static void set_from_services(sge_env_state_class_t *thiz, bool from_services);
static void set_qmaster_internal(sge_env_state_class_t *thiz, bool qmaster_internal);
static void set_sge_qmaster_port(sge_env_state_class_t *thiz, u_long32 sge_qmaster_port);
static void set_sge_execd_port(sge_env_state_class_t *thiz, u_long32 sge_qmaster_port);

sge_env_state_class_t *sge_env_state_class_create(const char *sge_root, const char *sge_cell, int sge_qmaster_port, int sge_execd_port, bool from_services, bool qmaster_internal, sge_error_class_t *eh)
{
   sge_env_state_class_t *ret = (sge_env_state_class_t *)sge_malloc(sizeof(sge_env_state_class_t));

   DENTER(TOP_LAYER, "sge_env_state_class_create");
   if (!ret) {
      eh->error(eh, STATUS_EMALLOC, ANSWER_QUALITY_ERROR, "%s",
                MSG_MEMORY_MALLOCFAILED);
      DEXIT;
      return NULL;
   }   
   
   ret->dprintf = sge_env_state_dprintf;
   
   ret->get_sge_root = get_sge_root;
   ret->get_sge_cell = get_sge_cell;
   ret->get_sge_qmaster_port = get_sge_qmaster_port;
   ret->get_sge_execd_port = get_sge_execd_port;
   ret->is_from_services = is_from_services;
   ret->is_qmaster_internal = is_qmaster_internal;

   ret->set_sge_root = set_sge_root;
   ret->set_sge_cell = set_sge_cell;
   ret->set_sge_qmaster_port = set_sge_qmaster_port;
   ret->set_sge_execd_port = set_sge_execd_port;

   ret->sge_env_state_handle = (sge_env_state_t*)sge_malloc(sizeof(sge_env_state_t));
   if (ret->sge_env_state_handle == NULL) {
      eh->error(eh, STATUS_EMALLOC, ANSWER_QUALITY_ERROR, "%s",
                MSG_MEMORY_MALLOCFAILED);
      sge_env_state_class_destroy(&ret);
      DEXIT;
      return NULL;
   }
   memset(ret->sge_env_state_handle, 0, sizeof(sge_env_state_t));

   if (!sge_env_state_setup(ret, sge_root, sge_cell, sge_qmaster_port, sge_execd_port, from_services, qmaster_internal, eh)) {
      sge_env_state_class_destroy(&ret);
      DEXIT;
      return NULL;
   }

   DEXIT;
   return ret;
}   

void sge_env_state_class_destroy(sge_env_state_class_t **pst)
{
   DENTER(TOP_LAYER, "sge_env_state_class_destroy");

   if (!pst || !*pst) {
      DEXIT;
      return;
   }   
   sge_env_state_destroy((*pst)->sge_env_state_handle);
   sge_free(pst);
   DEXIT;
}

static bool sge_env_state_setup(sge_env_state_class_t *thiz, const char *sge_root, const char *sge_cell, u_long32 sge_qmaster_port, u_long32 sge_execd_port, bool from_services, bool qmaster_internal, sge_error_class_t *eh)
{
   DENTER(TOP_LAYER, "sge_env_state_setup");
 
   thiz->set_sge_qmaster_port(thiz, sge_qmaster_port);
   thiz->set_sge_execd_port(thiz, sge_execd_port);
   thiz->set_sge_root(thiz, sge_root);
   thiz->set_sge_cell(thiz, sge_cell);
   set_from_services(thiz, from_services);
   set_qmaster_internal(thiz, qmaster_internal);

   /*thiz->dprintf(thiz);*/

   DEXIT;
   return true;
}

static void sge_env_state_destroy(void *theState)
{
   sge_env_state_t *s = (sge_env_state_t *)theState;

   DENTER(TOP_LAYER, "sge_env_state_destroy");
   sge_free(&(s->sge_root));
   sge_free(&(s->sge_cell));
   sge_free(&s);
   DEXIT;
}

static void sge_env_state_dprintf(sge_env_state_class_t *thiz)
{
   sge_env_state_t *es = (sge_env_state_t *)thiz->sge_env_state_handle;

   DENTER(TOP_LAYER, "sge_env_state_dprintf");

   DPRINTF(("sge_root            >%s<\n", es->sge_root ? es->sge_root : "NA"));
   DPRINTF(("sge_cell            >%s<\n", es->sge_cell ? es->sge_cell : "NA"));
   DPRINTF(("sge_qmaster_port    >%d<\n", (int)es->sge_qmaster_port));
   DPRINTF(("sge_execd_port      >%d<\n", (int)es->sge_execd_port));
   DPRINTF(("from_services       >%s<\n", es->from_services ? "true" : "false"));
   DPRINTF(("qmaster_internal    >%s<\n", es->qmaster_internal ? "true" : "false"));

   DEXIT;
}

static const char* get_sge_root(sge_env_state_class_t *thiz) 
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   return es->sge_root;
}

static const char* get_sge_cell(sge_env_state_class_t *thiz) 
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   return es->sge_cell;
}

static u_long32 get_sge_qmaster_port(sge_env_state_class_t *thiz)
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   return es->sge_qmaster_port;
}

static u_long32 get_sge_execd_port(sge_env_state_class_t *thiz)
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   return es->sge_execd_port;
}

static bool is_from_services(sge_env_state_class_t *thiz) 
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   return es->from_services;
}

static bool is_qmaster_internal(sge_env_state_class_t *thiz) 
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   return es->qmaster_internal;
}

static void set_sge_root(sge_env_state_class_t *thiz, const char *sge_root)
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   es->sge_root = sge_strdup(es->sge_root, sge_root);
}

static void set_sge_cell(sge_env_state_class_t *thiz, const char *sge_cell)
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   es->sge_cell = sge_strdup(es->sge_cell, sge_cell);
}

static void set_sge_qmaster_port(sge_env_state_class_t *thiz, u_long32 sge_qmaster_port)
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   es->sge_qmaster_port = sge_qmaster_port; 
}

static void set_sge_execd_port(sge_env_state_class_t *thiz, u_long32 sge_execd_port)
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   es->sge_execd_port = sge_execd_port; 
}

static void set_from_services(sge_env_state_class_t *thiz, bool from_services)
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   es->from_services = from_services;
}

static void set_qmaster_internal(sge_env_state_class_t *thiz, bool qmaster_internal)
{
   sge_env_state_t *es = (sge_env_state_t *) thiz->sge_env_state_handle;
   es->qmaster_internal = qmaster_internal;
}

