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

#include <ctype.h>
#include <string.h>
#include <jni.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_log.h"
#include "uti/sge_error_class.h"

#include "cull/cull.h"

#include "comm/commlib.h"

#include "cl_errors.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi2.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_host.h"

#include "basis_types.h"
#include "sge_qquota.h"
#include "jgdi_common.h"
#include "jgdi.h"
#include "jgdi_wrapper.h"
#include "jgdi_logging.h"

#include "msg_common.h"


typedef struct jgdi_report_handler_str jgdi_report_handler_t;

struct jgdi_report_handler_str {
   JNIEnv  *env;
   jgdi_result_t result;
   jobject qquota_result;
   jobject qquota_info;
};

static report_handler_t* jgdi_report_handler_create(JNIEnv *env, jobject qquota_result, lList **alpp);
static int jgdi_report_finished(report_handler_t* handler, lList **alpp);
static int jgdi_report_started(report_handler_t* handler, lList **alpp);

static int jgdi_report_limit_rule_begin(report_handler_t* handler, const char* limit_name, lList **alpp);
static int jgdi_report_limit_rule_finished(report_handler_t* handler, const char* limit_name, lList **alpp);
static int jgdi_report_limit_string_value(report_handler_t* handler, const char* name, const char *value, bool exclude, lList **alpp);
static int jgdi_report_resource_value(report_handler_t* handler, const char* resource, const char* limit, const char* value, lList **alpp);
static int jgdi_destroy_report_handler(report_handler_t** handler, lList **alpp);

static report_handler_t* jgdi_report_handler_create(JNIEnv *env, jobject qquota_result, lList **alpp) {
   jgdi_report_handler_t* jgdi_handler = (jgdi_report_handler_t*)sge_malloc(sizeof(jgdi_report_handler_t));
   report_handler_t *ret = NULL;

   DENTER( JGDI_LAYER, "jgdi_report_handler_create" );
   
   if (jgdi_handler == NULL ) {
      answer_list_add(alpp, "malloc of jgdi_report_handler_t failed",
                            STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      DRETURN(NULL);
   }
   
   ret = (report_handler_t*)sge_malloc(sizeof(report_handler_t));
   if (ret == NULL ) {
      answer_list_add(alpp, "malloc of report_handler_t failed",
                            STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      sge_free(&jgdi_handler);  
      DRETURN(NULL);
   }
   
   memset(jgdi_handler, 0, sizeof(jgdi_report_handler_t));
   memset(ret, 0, sizeof(report_handler_t));
   
   jgdi_handler->result = JGDI_SUCCESS;
   
   ret->ctx = jgdi_handler;
   ret->report_started = jgdi_report_started;
   ret->report_finished = jgdi_report_finished;
   ret->report_limit_rule_begin = jgdi_report_limit_rule_begin;
   ret->report_limit_string_value = jgdi_report_limit_string_value;
   ret->report_limit_rule_finished = jgdi_report_limit_rule_finished;
   ret->report_resource_value = jgdi_report_resource_value;
   ret->destroy = jgdi_destroy_report_handler;
   
   jgdi_handler->qquota_result = qquota_result;
   jgdi_handler->env = env;
   
   DRETURN(ret);
}

static int jgdi_destroy_report_handler(report_handler_t** handler, lList **alpp) {
   DENTER( JGDI_LAYER, "jgdi_destroy_report_handler" );
   if (*handler != NULL ) {
      
      /* We ensurce that the global ref on the qquota_info object is deleted */
      jgdi_report_handler_t* jgdi_handler = (jgdi_report_handler_t*)(*handler)->ctx;
      if (jgdi_handler->qquota_info != NULL) {
         (*(jgdi_handler->env))->DeleteGlobalRef(jgdi_handler->env, jgdi_handler->qquota_info);
         jgdi_handler->qquota_info = NULL;
      }
      
      /* Free the internal context */
      sge_free(&((*handler)->ctx));
   }
   DRETURN(QQUOTA_SUCCESS);
}


static int jgdi_report_finished(report_handler_t* handler, lList **alpp) {
   DENTER( JGDI_LAYER, "jgdi_report_finished" );
   DRETURN(QQUOTA_SUCCESS);
}

static int jgdi_report_started(report_handler_t* handler, lList **alpp) {
   DENTER( JGDI_LAYER, "jgdi_report_started" );
   DRETURN(QQUOTA_SUCCESS);
}

static int jgdi_report_limit_rule_begin(report_handler_t* handler, const char* limit_rule_name, lList **alpp) {
   jgdi_report_handler_t* jgdi_handler = (jgdi_report_handler_t*)handler->ctx;
   JNIEnv *env = jgdi_handler->env;
   jobject qquota_info = NULL;
   
   DENTER( JGDI_LAYER, "jgdi_report_limit_rule_begin" );

   jgdi_log_printf(env, JGDI_LOGGER, FINER, "Create new limit rule info object for limit rule %s\n", limit_rule_name);
  
   jgdi_handler->result = ResourceQuotaRuleInfoImpl_init_0(env, &qquota_info, limit_rule_name, alpp);
   if (jgdi_handler->result != JGDI_SUCCESS) {
      DRETURN(QQUOTA_ERROR);
   }
   
   jgdi_handler->qquota_info = (*env)->NewGlobalRef(env, qquota_info);
   
   (*env)->DeleteLocalRef(env, qquota_info);
   
   if (jgdi_handler->qquota_info == NULL) {
      answer_list_add(alpp , "Can not create global reference for qquota info object", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(QQUOTA_ERROR);
   }
   
   DRETURN(QQUOTA_SUCCESS);
}

static int jgdi_report_limit_string_value(report_handler_t* handler, const char* name, const char *value, bool exclude, lList** alpp) {
   
   jgdi_report_handler_t* jgdi_handler = (jgdi_report_handler_t*)handler->ctx;
   JNIEnv *env = jgdi_handler->env;
   
   DENTER( JGDI_LAYER, "jgdi_report_limit_string_value" );
   
   jgdi_log_printf(env, JGDI_LOGGER, FINER, "add filter value %s=%s\n", name, value);
   
   /* include lists */
   if (!strcmp(name, "users") && exclude == false) {
      if (ResourceQuotaRuleInfoImpl_addUser(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   if (!strcmp(name, "projects") && exclude == false) {
      if (ResourceQuotaRuleInfoImpl_addProject(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   if (!strcmp(name, "pes") && exclude == false) {
      if (ResourceQuotaRuleInfoImpl_addPe(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   if (!strcmp(name, "queues") && exclude == false) {
      if (ResourceQuotaRuleInfoImpl_addQueue(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   if (!strcmp(name, "hosts") && exclude == false) {
      if (ResourceQuotaRuleInfoImpl_addHost(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   /* exclude lists */
   if (!strcmp(name, "users") && exclude == true) {
      if (ResourceQuotaRuleInfoImpl_addXUser(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   if (!strcmp(name, "projects") && exclude == true) {
      if (ResourceQuotaRuleInfoImpl_addXProject(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   if (!strcmp(name, "pes") && exclude == true) {
      if (ResourceQuotaRuleInfoImpl_addXPe(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   if (!strcmp(name, "queues") && exclude == true) {
      if (ResourceQuotaRuleInfoImpl_addXQueue(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   if (!strcmp(name, "hosts") && exclude == true) {
      if (ResourceQuotaRuleInfoImpl_addXHost(env, jgdi_handler->qquota_info, value, alpp) != JGDI_SUCCESS) {
        DRETURN(QQUOTA_ERROR);
      }
   }
   
   DRETURN(QQUOTA_SUCCESS);
}

static int jgdi_report_resource_value(report_handler_t* handler, const char* resource, const char* limit, const char* value, lList** alpp) {
   jgdi_report_handler_t* jgdi_handler = (jgdi_report_handler_t*)handler->ctx;
   JNIEnv *env = jgdi_handler->env;
   jobject resource_obj = NULL;
   
   DENTER( JGDI_LAYER, "jgdi_report_resource_value" );
   
   if (jgdi_handler->qquota_info == NULL) {
      answer_list_add(alpp, "jgdi_report_resource_value: qquota_info object not set", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(QQUOTA_ERROR);
   }

   jgdi_log_printf(env, JGDI_LOGGER, FINER,
                     "resource='%s', limit='%s', value='%s'\n", resource, limit, value);

   if (ResourceQuotaImpl_init(env, &resource_obj, alpp) != JGDI_SUCCESS) {
     DRETURN(QQUOTA_ERROR);
   }



   if (resource_obj != NULL) {
      if (ResourceQuotaImpl_setName(env, resource_obj, resource, alpp) != JGDI_SUCCESS) {
         DRETURN(QQUOTA_ERROR);
      }

      if (ResourceQuotaImpl_setLimitValue(env, resource_obj, limit, alpp) != JGDI_SUCCESS) {
         DRETURN(QQUOTA_ERROR);
      }

      if (ResourceQuotaImpl_setUsageValue(env, resource_obj, value, alpp) != JGDI_SUCCESS) {
         DRETURN(QQUOTA_ERROR);
      }
      if (ResourceQuotaRuleInfoImpl_addLimit(env, jgdi_handler->qquota_info, resource_obj, alpp) != JGDI_SUCCESS) { 
        DRETURN(QQUOTA_ERROR);
      }
   }

   DRETURN(QQUOTA_SUCCESS);
}


static int jgdi_report_limit_rule_finished(report_handler_t* handler, const char* limit_name, lList** alpp) {
   jgdi_report_handler_t* jgdi_handler = (jgdi_report_handler_t*)handler->ctx;
   JNIEnv *env = jgdi_handler->env;

   DENTER( JGDI_LAYER, "jgdi_report_limit_rule_finished" );
   
   if (jgdi_handler->qquota_info == NULL) {
      answer_list_add(alpp, "qquota_info object is not available in jgdi_handler",
                      STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(QQUOTA_ERROR);
   }

   if (jgdi_handler->qquota_result == NULL) {
      DPRINTF(("jgdi_handler->qquota_result is NULL\n"));
      abort();
   }
   
   if (QQuotaResultImpl_addResourceQuotaRuleInfo(env, jgdi_handler->qquota_result, jgdi_handler->qquota_info, alpp) != JGDI_SUCCESS) {
     DRETURN(QQUOTA_ERROR);
   }
   DPRINTF(("DeleteGlobalRef\n"));
   (*env)->DeleteGlobalRef(env, jgdi_handler->qquota_info);
   jgdi_handler->qquota_info = NULL;
   
   DRETURN(QQUOTA_SUCCESS);
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIImpl
 * Method:    getQQuota
 * Signature: (Lcom/sun/grid/jgdi/monitoring/QQuotaOptions;Lcom/sun/grid/jgdi/monitoring/QQuotaResultImpl;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_getQQuota
  (JNIEnv *env, jobject jgdi, jobject qquota_options, jobject qquota_result) {
   
   jclass    cls = NULL;
   jmethodID mid = NULL;
   jobject   sub_object = NULL;
   lList     *alp = NULL;
   
   struct filter_def {
      const char* getter;
      const char* signature;
      const char* getstFunc;
      lList *list;
   };
   
   /*
   ** order in filters[] is important -> qquota_output
   */
   struct filter_def filters[] = {
      { "getHostFilter", "()Lcom/sun/grid/jgdi/monitoring/filter/HostFilter;", "getHosts", NULL },
      { "getResourceFilter", "()Lcom/sun/grid/jgdi/monitoring/filter/ResourceFilter;", "getResources", NULL },
      { "getUserFilter", "()Lcom/sun/grid/jgdi/monitoring/filter/UserFilter;", "getUsers", NULL },
      { "getPeFilter", "()Lcom/sun/grid/jgdi/monitoring/filter/ParallelEnvironmentFilter;", "getPEList", NULL },
      { "getProjectFilter", "()Lcom/sun/grid/jgdi/monitoring/filter/ProjectFilter;", "getProjectList", NULL },
      { "getQueueFilter", "()Lcom/sun/grid/jgdi/monitoring/filter/QueueFilter;", "getQueues", NULL },
      { NULL, NULL, NULL, NULL }
   };
   
   int i;
   sge_gdi_ctx_class_t * ctx = NULL;
   
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   
   DENTER( JGDI_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_getQQuota" );
   
   jgdi_init_rmon_ctx(env, JGDI_QHOST_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);
   
   ret = getGDIContext(env, jgdi, &ctx, &alp);
   
   if (ret != JGDI_SUCCESS) {
      goto error;
   }
   
   sge_gdi_set_thread_local_ctx(ctx);

   cls = QQuotaOptions_find_class(env, &alp);
   if (cls == NULL) {
      ret = JGDI_ILLEGAL_STATE;
      goto error;
   }
   
   for(i = 0; filters[i].getter != NULL; i++ ) {
      
      mid = get_methodid(env, cls, filters[i].getter, filters[i].signature, &alp);
      if (!mid) {
         ret = JGDI_ILLEGAL_STATE;
         break;
      }
      sub_object = (*env)->CallObjectMethod(env, qquota_options, mid);
      
      if (test_jni_error( env, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_getQQuota: Unexpected error while getting sub_object", &alp)) {
         ret = JGDI_ILLEGAL_STATE;
         break;
      }
      
      if (sub_object != NULL) {
         if ((ret=get_string_list(env, sub_object, filters[i].getstFunc, &(filters[i].list), ST_Type, ST_name, &alp)) != JGDI_SUCCESS) {
            break;
         } else if (strcmp(filters[i].getter, "getHostFilter") == 0) {
            lListElem *ep = NULL;
            /* 
            ** resolve hostnames and replace them in list
            */
            for_each(ep, filters[i].list) {
               if (sge_resolve_host(ep, ST_name) != CL_RETVAL_OK) {
                  answer_list_add_sprintf(&alp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_SGETEXT_CANTRESOLVEHOST_S, lGetString(ep,ST_name) );
                  ret = JGDI_ERROR;
                  break;
               }
            }
         }
      }
      /*
      printf("Build filter %s -------------------------\n", filters[i].getter);
      lWriteListTo(filters[i].list, stdout);
      */
   }
   
   /* send gdi request */
   if (ret == JGDI_SUCCESS) {
      report_handler_t *report_handler = jgdi_report_handler_create(env, qquota_result, &alp);
      
      if (report_handler != NULL) {
         qquota_output(ctx, 
                    filters[0].list, /* -h host_list */
                    filters[1].list, /* -l resource_match_list */
                    filters[2].list, /* -u user_list */
                    filters[3].list, /* -pe pe_list */
                    filters[4].list, /* -P project_list */
                    filters[5].list, /* -q cqueue_list */
                    &alp,
                    report_handler);
         ret = ((jgdi_report_handler_t*)report_handler->ctx)->result;
         if (report_handler->destroy != NULL) {
            report_handler->destroy(&report_handler, &alp);
         }         
         sge_free(&report_handler);
      } else {
         ret = JGDI_ILLEGAL_STATE;
      }
   }

error:
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }
   for(i=0; filters[i].getter != NULL; i++) {
      lFreeList(&(filters[i].list));
   }

   lFreeList(&alp);   
   
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);
   
   DRETURN_VOID;
   
}
