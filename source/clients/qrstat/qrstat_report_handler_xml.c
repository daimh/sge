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
 *  Portions of this code are Copyright 2011 Univa Corporation.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <string.h>

#include "uti/sge_rmon.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_advance_reservation.h"

#include "qrstat_report_handler.h"
#include "qrstat_report_handler_xml.h"

#include "basis_types.h"
#include "sge.h"
#include "msg_common.h"

static bool
qrstat_report_start(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_finish(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_start_ar(qrstat_report_handler_t* handler, qrstat_env_t *qrstat_env, lList **alpp);

static bool
qrstat_report_start_unknown_ar(qrstat_report_handler_t* handler, qrstat_env_t *qrstat_env, lList **alpp);

static bool
qrstat_report_finish_ar(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_finish_unknown_ar(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_ar_node_ulong(qrstat_report_handler_t* handler, qrstat_env_t *qrstat_env, lList **alpp,
                                const char *name, u_long32 value);

static bool
qrstat_report_ar_node_duration(qrstat_report_handler_t* handler, lList **alpp,
                               const char *name, u_long32 value);

static bool
qrstat_report_ar_node_string(qrstat_report_handler_t* handler, lList **alpp,
                             const char *name, const char *value);

static bool
qrstat_report_ar_node_time(qrstat_report_handler_t* handler, lList **alpp,
                           const char *name, time_t value);

static bool
qrstat_report_ar_node_state(qrstat_report_handler_t* handler, lList **alpp,
                                const char *name, u_long32 value);

static bool
qrstat_report_start_resource_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_finish_resource_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_resource_list_node(qrstat_report_handler_t* handler, lList **alpp,
                                     const char *name, const char *value);
static bool
qrstat_report_ar_node_boolean(qrstat_report_handler_t* handler, lList **alpp,
                                     const char *name, bool value);
static bool
qrstat_report_start_granted_slots_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_finish_granted_slots_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_granted_slots_list_node(qrstat_report_handler_t* handler,
                                          lList **alpp,
                                          const char *name, u_long32 value);

static bool
qrstat_report_start_granted_parallel_environment(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_finish_granted_parallel_environment(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_granted_parallel_environment_node(qrstat_report_handler_t* handler,
                                                    lList **alpp,
                                                    const char *name, const char *slots_range);
static bool
qrstat_report_start_mail_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_finish_mail_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_mail_list_node(qrstat_report_handler_t* handler,
                             lList **alpp,
                             const char *name, const char *hostname);

static bool
qrstat_report_start_acl_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_finish_acl_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_acl_list_node(qrstat_report_handler_t* handler,
                            lList **alpp, const char *name);

static bool
qrstat_report_start_xacl_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_finish_xacl_list(qrstat_report_handler_t* handler, lList **alpp);

static bool
qrstat_report_xacl_list_node(qrstat_report_handler_t* handler,
                             lList **alpp, const char *name);

static bool
qrstat_report_newline(qrstat_report_handler_t* handler, lList **alpp);


qrstat_report_handler_t *
qrstat_create_report_handler_xml(qrstat_env_t *qrstat_env, lList **answer_list)
{
   qrstat_report_handler_t* ret = NULL;

   DENTER(TOP_LAYER, "qrstat_create_report_handler_xml");

   ret = (qrstat_report_handler_t*)sge_malloc(sizeof(qrstat_report_handler_t));
   if (ret == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EMALLOC, ANSWER_QUALITY_ERROR,
                              MSG_MEM_MEMORYALLOCFAILED_S, SGE_FUNC);      
   } else {
     /*
      * xml report handler ctx is a dstring
      */
      ret->ctx = sge_malloc(sizeof(dstring));
      if (ret->ctx == NULL ) {
         answer_list_add_sprintf(answer_list, STATUS_EMALLOC, ANSWER_QUALITY_ERROR,
                                 MSG_MEM_MEMORYALLOCFAILED_S, SGE_FUNC);
      } else {
         /*
          * corresponds to initializing with DSTRING_INIT
          */
         memset(ret->ctx, 0, sizeof(dstring));

         ret->show_summary = qrstat_env->is_summary;

         ret->report_start = qrstat_report_start;
         ret->report_finish = qrstat_report_finish;
         ret->report_start_ar = qrstat_report_start_ar;
         ret->report_start_unknown_ar = qrstat_report_start_unknown_ar;
         ret->report_finish_ar = qrstat_report_finish_ar;
         ret->report_finish_unknown_ar = qrstat_report_finish_unknown_ar;
         ret->report_ar_node_ulong = qrstat_report_ar_node_ulong;
         ret->report_ar_node_ulong_unknown = qrstat_report_ar_node_ulong;
         ret->report_ar_node_duration = qrstat_report_ar_node_duration;
         ret->report_ar_node_string = qrstat_report_ar_node_string;
         ret->report_ar_node_time = qrstat_report_ar_node_time;
         ret->report_ar_node_state = qrstat_report_ar_node_state;

         ret->report_start_resource_list = qrstat_report_start_resource_list;
         ret->report_finish_resource_list = qrstat_report_finish_resource_list;
         ret->report_resource_list_node = qrstat_report_resource_list_node;

         ret->report_ar_node_boolean = qrstat_report_ar_node_boolean;

         ret->report_start_granted_slots_list = qrstat_report_start_granted_slots_list;
         ret->report_finish_granted_slots_list = qrstat_report_finish_granted_slots_list;
         ret->report_granted_slots_list_node = qrstat_report_granted_slots_list_node;

         ret->report_start_granted_parallel_environment = qrstat_report_start_granted_parallel_environment;
         ret->report_finish_granted_parallel_environment = qrstat_report_finish_granted_parallel_environment;
         ret->report_granted_parallel_environment_node = qrstat_report_granted_parallel_environment_node;

         ret->report_start_mail_list = qrstat_report_start_mail_list;
         ret->report_finish_mail_list = qrstat_report_finish_mail_list;
         ret->report_mail_list_node = qrstat_report_mail_list_node;

         ret->report_start_acl_list = qrstat_report_start_acl_list;
         ret->report_finish_acl_list = qrstat_report_finish_acl_list;
         ret->report_acl_list_node = qrstat_report_acl_list_node;

         ret->report_start_xacl_list = qrstat_report_start_xacl_list;
         ret->report_finish_xacl_list = qrstat_report_finish_xacl_list;
         ret->report_xacl_list_node = qrstat_report_xacl_list_node;
         ret->report_newline = qrstat_report_newline;
      }
   }

   DRETURN(ret);
}

bool
qrstat_destroy_report_handler_xml(qrstat_report_handler_t** handler, lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "qrstat_destroy_report_handler_xml");

   if (handler != NULL && *handler != NULL ) {
      sge_dstring_free((dstring*)(*handler)->ctx);
      sge_free(&((*handler)->ctx));
      sge_free(handler);
   }

   DRETURN(ret);
}

static bool
qrstat_report_start(qrstat_report_handler_t* handler, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start");

   sge_dstring_append(buffer, "<?xml version='1.0'?>\n");
   sge_dstring_append(buffer, "<qrstat xmlns:xsd=\"http://arc.liv.ac.uk/repos/darcs/sge/source/dist/util/resources/schemas/qrstat/qrstat.xsd\">\n");

   DRETURN(ret); 
}

static bool
qrstat_report_finish(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish");
  
   sge_dstring_append(buffer, "</qrstat>\n");
   printf("%s", sge_dstring_get_string((dstring *)handler->ctx));

   DRETURN(ret); 
}

static bool
qrstat_report_start_ar(qrstat_report_handler_t* handler, qrstat_env_t *qrstat_env, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start_ar");

   sge_dstring_append(buffer, "   <ar_summary>\n");
     
   DRETURN(ret); 
}

static bool
qrstat_report_start_unknown_ar(qrstat_report_handler_t* handler, qrstat_env_t *qrstat_env, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start_unknown_ar");

   sge_dstring_append(buffer, "   <ar_unknown>\n");
     
   DRETURN(ret); 
}

static bool
qrstat_report_finish_ar(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish_ar");
  
   sge_dstring_append(buffer, "   </ar_summary>\n");

   DRETURN(ret); 
}

static bool
qrstat_report_finish_unknown_ar(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish_unknown_ar");
  
   sge_dstring_append(buffer, "   </ar_unknown>\n");

   DRETURN(ret); 
}

static bool
qrstat_report_ar_node_ulong(qrstat_report_handler_t* handler, qrstat_env_t *qrstat_env, lList **alpp,
                            const char *name, u_long32 value)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_ar_node_ulong");
  
   sge_dstring_sprintf_append(buffer, "      <"SFN">"sge_U32CFormat"</"SFN">\n", 
                              name, value, name);

   DRETURN(ret); 
}

static bool
qrstat_report_ar_node_duration(qrstat_report_handler_t* handler, lList **alpp,
                               const char *name, u_long32 value)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;
   int seconds = value % 60;      
   int minutes = ((value - seconds) / 60) % 60;      
   int hours = ((value - seconds - minutes * 60) / 3600);

   DENTER(TOP_LAYER, "qrstat_report_ar_node_duration");
  
   sge_dstring_sprintf_append(buffer, "      <"SFN">%02d:%02d:%02d</"SFN">\n", 
                              name, hours, minutes, seconds, name);

   DRETURN(ret); 
}

static bool
qrstat_report_ar_node_string(qrstat_report_handler_t* handler, lList **alpp,
                                 const char *name, const char *value)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_ar_node_string");
 
   if (value != NULL) { 
      sge_dstring_sprintf_append(buffer, "      <"SFN">"SFN"</"SFN">\n", name, value, name);
   } else {
      sge_dstring_sprintf_append(buffer, "      <"SFN"/>\n", name);
   }

   DRETURN(ret); 
} 

static bool
qrstat_report_ar_node_time(qrstat_report_handler_t* handler, lList **alpp,
                               const char *name, time_t value)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;
   dstring time_string = DSTRING_INIT;

   DENTER(TOP_LAYER, "qrstat_report_ar_node_time");
 
   sge_dstring_append_time(&time_string, value, true); 
   sge_dstring_sprintf_append(buffer, "      <"SFN">"SFN"</"SFN">\n", 
                              name, sge_dstring_get_string(&time_string), name);
   sge_dstring_free(&time_string);

   DRETURN(ret); 
} 
 
static bool
qrstat_report_ar_node_state(qrstat_report_handler_t* handler, lList **alpp,
                                const char *name, u_long32 state)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;
   dstring state_string = DSTRING_INIT;

   DENTER(TOP_LAYER, "qrstat_report_ar_node_time");
 
   ar_state2dstring((ar_state_t)state, &state_string);
   sge_dstring_sprintf_append(buffer, "      <"SFN">"SFN"</"SFN">\n", 
                              name, sge_dstring_get_string(&state_string), name);
   sge_dstring_free(&state_string);

   DRETURN(ret); 
} 

static bool
qrstat_report_start_resource_list(qrstat_report_handler_t* handler, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start_resource_list");

   sge_dstring_append(buffer, "      <resource_list>\n");
     
   DRETURN(ret); 
}

static bool
qrstat_report_finish_resource_list(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish_resource_list");
  
   sge_dstring_append(buffer, "      </resource_list>\n");

   DRETURN(ret); 
}

static bool
qrstat_report_resource_list_node(qrstat_report_handler_t* handler, lList **alpp,
                                     const char *name, const char *value)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_resource_list_node");
  
   sge_dstring_sprintf_append(buffer, "         <resource name="SFQ" type="SFQ"/>\n",
                              name, value);

   DRETURN(ret); 
}


static bool
qrstat_report_ar_node_boolean(qrstat_report_handler_t* handler, lList **alpp, const char *name, bool value)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_ar_node_boolean");
  
   sge_dstring_sprintf_append(buffer,"      <"SFN">"SFN"</"SFN">\n", 
                              name, value ? "true":"false", name);

   DRETURN(ret); 
}

static bool
qrstat_report_start_granted_slots_list(qrstat_report_handler_t* handler, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start_granted_slots_list");

   sge_dstring_append(buffer, "      <granted_slots_list>\n");
     
   DRETURN(ret); 
}

static bool
qrstat_report_finish_granted_slots_list(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish_granted_slots_list");
  
   sge_dstring_append(buffer, "      </granted_slots_list>\n");

   DRETURN(ret); 
}

static bool
qrstat_report_granted_slots_list_node(qrstat_report_handler_t* handler, 
                                      lList **alpp,
                                      const char *name, u_long32 value)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_granted_slots_list_node");
  
   sge_dstring_sprintf_append(buffer, "         <granted_slots  queue_instance="SFQ
                              " slots=\""sge_U32CFormat"\"/>\n", name, value);

   DRETURN(ret); 
}
 
static bool
qrstat_report_start_granted_parallel_environment(qrstat_report_handler_t* handler, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start_granted_parallel_environment");

   sge_dstring_append(buffer, "      <granted_parallel_environment>\n");
     
   DRETURN(ret); 
}

static bool
qrstat_report_finish_granted_parallel_environment(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish_parallel_environment");
  
   sge_dstring_append(buffer, "      </granted_parallel_environment>\n");

   DRETURN(ret); 
}

static bool
qrstat_report_granted_parallel_environment_node(qrstat_report_handler_t* handler, 
                                                    lList **alpp,
                                                    const char *name, const char *slots_range)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_granted_granted_parallel_environment_node");
  
   sge_dstring_sprintf_append(buffer, "         <parallel_environment>"SFN"</parallel_environment>\n", (name != NULL) ? name : "");
   sge_dstring_sprintf_append(buffer, "         <slots>"SFN"</slots>\n", (slots_range != NULL) ? slots_range : "");

   DRETURN(ret); 
}

static bool 
qrstat_report_start_mail_list(qrstat_report_handler_t* handler, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start_mail_list");

   sge_dstring_append(buffer, "      <mail_list>\n");
     
   DRETURN(ret); 
}

static bool
qrstat_report_finish_mail_list(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish_mail_list");
  
   sge_dstring_append(buffer, "      </mail_list>\n");

   DRETURN(ret); 
}

static bool
qrstat_report_mail_list_node(qrstat_report_handler_t* handler, 
                                 lList **alpp,
                                 const char *name, const char *host)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_mail_list_node");
  
   sge_dstring_sprintf_append(buffer, "         <mail  user="SFQ
                              " host="SFQ"/>\n", name, host?host:"NULL");

   DRETURN(ret); 
}

static bool
qrstat_report_start_acl_list(qrstat_report_handler_t* handler, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start_acl_list");

   sge_dstring_append(buffer, "      <acl_list>\n");
     
   DRETURN(ret); 
}

static bool
qrstat_report_finish_acl_list(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish_acl_list");
  
   sge_dstring_append(buffer, "      </acl_list>\n");

   DRETURN(ret); 
}

static bool
qrstat_report_acl_list_node(qrstat_report_handler_t* handler, 
                            lList **alpp,
                            const char *name)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_acl_list_node");
  
   sge_dstring_sprintf_append(buffer, "         <acl  user="SFQ"/>\n", name);

   DRETURN(ret); 
}

static bool 
qrstat_report_start_xacl_list(qrstat_report_handler_t* handler, lList **alpp) 
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_start_xacl_list");

   sge_dstring_append(buffer, "      <xacl_list>\n");
     
   DRETURN(ret); 
}

static bool
qrstat_report_finish_xacl_list(qrstat_report_handler_t* handler, lList **alpp)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_finish_xacl_list");
  
   sge_dstring_append(buffer, "      </xacl_list>\n");

   DRETURN(ret); 
}

static bool
qrstat_report_xacl_list_node(qrstat_report_handler_t* handler, 
                             lList **alpp,
                             const char *name)
{
   bool ret = true;
   dstring *buffer = (dstring*)handler->ctx;

   DENTER(TOP_LAYER, "qrstat_report_xacl_list_node");
  
   sge_dstring_sprintf_append(buffer, "         <acl  user="SFQ"/>\n", name);

   DRETURN(ret); 
}
 
static bool
qrstat_report_newline(qrstat_report_handler_t* handler, lList **alpp)
{
   return true;
}
