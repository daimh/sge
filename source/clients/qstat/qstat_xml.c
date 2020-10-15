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
#include <stdlib.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_prog.h"
#include "uti/sge_dstring.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_cull_xml.h"

#include "gdi/sge_gdi.h"

#include "sched/sgeee.h" 
#include "sched/sge_sched.h"
#include "sched/sge_schedd_text.h"

#include "symbols.h"
#include "qstat_printing.h"
#include "sge.h"
#include "sge_qstat.h"
#include "qstat_xml.h"

/* ----------------------- qselect xml handler ------------------------------ */

static int qselect_xml_report_queue(qselect_handler_t *thiz, const char* qname, lList** alpp);
static int qselect_xml_finished(qselect_handler_t *thiz, lList** alpp);
static int qselect_xml_destroy(qselect_handler_t *thiz, lList** alpp);
static int qselect_xml_started(qselect_handler_t *thiz, lList** alpp);

int qselect_xml_init(qselect_handler_t* handler, lList **alpp _UNUSED) {
   
   memset(handler, 0, sizeof(qselect_handler_t));
   
   handler->ctx = sge_malloc(sizeof(dstring));
   if (handler->ctx == NULL ) {
      answer_list_add(alpp, "malloc of dstring buffer failed",
                            STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      return -1;
   }
   memset(handler->ctx, 0, sizeof(dstring));

   handler->report_started = qselect_xml_started;
   handler->report_finished = qselect_xml_finished;
   handler->destroy = qselect_xml_destroy;
   handler->report_queue = qselect_xml_report_queue;
   return 0;
}

static int qselect_xml_destroy(qselect_handler_t *thiz, lList** alpp _UNUSED) {
   if (thiz != NULL ) {
      sge_dstring_free((dstring*)thiz->ctx);
      sge_free(&(thiz->ctx));
   }
   return 0;
}

static int qselect_xml_started(qselect_handler_t *thiz _UNUSED, lList** alpp _UNUSED) {
   printf("<qselect>\n");
   return 0;
}

static int qselect_xml_finished(qselect_handler_t *thiz _UNUSED, lList** alpp _UNUSED) {
   printf("</qselect>\n");
   return 0;
}

static int qselect_xml_report_queue(qselect_handler_t *thiz, const char* qname, lList** alpp _UNUSED) {
   escape_string(qname, (dstring*)thiz->ctx);
   printf("   <queue>%s</queue>\n", sge_dstring_get_string((dstring*)thiz->ctx));
   sge_dstring_clear((dstring*)thiz->ctx);
   return 0;
}
                           
/* --------------- Cluster Queue Summary To XML Handler -------------------*/
static int cqueue_summary_xml_report_finished(cqueue_summary_handler_t *handler, lList **alpp);
static int cqueue_summary_xml_report_cqueue(cqueue_summary_handler_t *handler, const char* cqname, cqueue_summary_t *summary, lList **alpp);

int cqueue_summary_xml_handler_init(cqueue_summary_handler_t *handler, lList **alpp _UNUSED) {
   memset(handler, 0, sizeof(cqueue_summary_handler_t));
   
   handler->report_finished = cqueue_summary_xml_report_finished;
   handler->report_cqueue = cqueue_summary_xml_report_cqueue; 
   return 0;
}


static int cqueue_summary_xml_report_finished(cqueue_summary_handler_t *handler, lList **alpp _UNUSED) {
   if (handler->ctx != NULL) {
      lListElem *xml_elem = NULL;
    
      xml_elem = xml_getHead("job_info", (lList*)handler->ctx, NULL);
      handler->ctx = NULL;
      lWriteElemXMLTo(xml_elem, stdout, -1);  
      lFreeElem(&xml_elem);
   }
   return 0;
}

static int cqueue_summary_xml_report_cqueue(cqueue_summary_handler_t *handler, const char* cqname, cqueue_summary_t *summary, lList **alpp _UNUSED) {
   
   lListElem *elem = NULL;
   lList *attributeList = NULL;
   qstat_env_t *qstat_env = handler->qstat_env;
   bool show_states = (qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED) ? true : false;
   
   elem = lCreateElem(XMLE_Type);
   attributeList = lCreateList("attributes", XMLE_Type);
   lSetList(elem, XMLE_List, attributeList);
 
   xml_append_Attr_S(attributeList, "name", cqname);
   if (summary->is_load_available) {
      xml_append_Attr_D(attributeList, "load", summary->load);
   }
   xml_append_Attr_I(attributeList, "used", (int)summary->used);
   xml_append_Attr_I(attributeList, "resv", (int)summary->resv);
   xml_append_Attr_I(attributeList, "available", (int)summary->available);
   xml_append_Attr_I(attributeList, "total", (int)summary->total);
   xml_append_Attr_I(attributeList, "temp_disabled", (int)summary->temp_disabled);
   xml_append_Attr_I(attributeList, "manual_intervention", (int)summary->manual_intervention);
   if (show_states) {
      xml_append_Attr_I(attributeList, "suspend_manual", (int)summary->suspend_manual);
      xml_append_Attr_I(attributeList, "suspend_threshold", (int)summary->suspend_threshold);
      xml_append_Attr_I(attributeList, "suspend_on_subordinate", (int)summary->suspend_on_subordinate);
      xml_append_Attr_I(attributeList, "suspend_calendar", (int)summary->suspend_calendar);
      xml_append_Attr_I(attributeList, "unknown", (int)summary->unknown);
      xml_append_Attr_I(attributeList, "load_alarm", (int)summary->load_alarm);
      xml_append_Attr_I(attributeList, "disabled_manual", (int)summary->disabled_manual);
      xml_append_Attr_I(attributeList, "disabled_calendar", (int)summary->disabled_calendar);
      xml_append_Attr_I(attributeList, "ambiguous", (int)summary->ambiguous);
      xml_append_Attr_I(attributeList, "orphaned", (int)summary->orphaned);
      xml_append_Attr_I(attributeList, "error", (int)summary->error);
   }
   if (elem) {
      if (handler->ctx == NULL){
         handler->ctx = lCreateList("cluster_queue_summary", XMLE_Type);
      }
      lAppendElem((lList*)handler->ctx, elem); 
   } 
   return 0;
}
 
                           
static int qstat_xml_queue_started(qstat_handler_t* handler, const char* qname, lList **alpp);
static int qstat_xml_queue_summary(qstat_handler_t* handler, const char* qname, queue_summary_t *summary, lList **alpp);
static int qstat_xml_queue_finished(qstat_handler_t* handler, const char* qname, lList **alpp);
static int qstat_xml_queue_load_alarm(qstat_handler_t* handler, const char* qname, const char* reason, lList **alpp);
static int qstat_xml_queue_suspend_alarm(qstat_handler_t* handler, const char* qname, const char* reason, lList **alpp);
static int qstat_xml_queue_message(qstat_handler_t* handler, const char* qname, const char *message, lList **alpp);
static int qstat_xml_queue_resource(qstat_handler_t* handler, const char* dom, const char* name, const char* value, lList **alpp);

static int qstat_xml_job(job_handler_t* handler, u_long32 jid, job_summary_t *summary, lList **alpp);
static int qstat_xml_sub_task(job_handler_t* handler, task_summary_t *summary, lList **alpp);
static int qstat_xml_job_additional_info(job_handler_t* handler, job_additional_info_t name, const char* value, lList **alpp);
static int qstat_xml_job_requested_pe(job_handler_t *handler, const char* pe_name, const char* pe_range, lList **alpp);

static int qstat_xml_job_granted_pe(job_handler_t *handler, const char* pe_name, int pe_slots, lList **alpp);
static int qstat_xml_job_request(job_handler_t* handler, const char* name, const char* value, lList **alpp);
static int qstat_xml_job_report_hard_resource(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp);
static int qstat_xml_job_soft_resource(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp);
static int qstat_xml_job_hard_requested_queue(job_handler_t *handler, const char* name, lList **alpp);
static int qstat_xml_job_soft_requested_queue(job_handler_t *handler, const char* name, lList **alpp);
static int qstat_xml_job_master_hard_requested_queue(job_handler_t* handler, const char* name, lList **alpp);
static int qstat_xml_job_predecessor_requested(job_handler_t* handler, const char* name, lList **alpp);
static int qstat_xml_job_predecessor(job_handler_t* handler, u_long32 jid, lList **alpp);
static int qstat_xml_job_ad_predecessor_requested(job_handler_t* handler, const char* name, lList **alpp);
static int qstat_xml_job_binding(job_handler_t* handler, const char* name, lList **alpp);
static int qstat_xml_job_ad_predecessor(job_handler_t* handler, u_long32 jid, lList **alpp);

static int qstat_xml_job_finished(job_handler_t* handler, u_long32 jid, lList **alpp);


static int qstat_xml_create_job_list(qstat_handler_t *handler, lList **alpp);
static int qstat_xml_finish_job_list(qstat_handler_t *handler, const char* state, lList* target_list, lList **alpp);

static int qstat_xml_queue_jobs_started(qstat_handler_t *handler, const char* qname, lList **alpp);
static int qstat_xml_queue_jobs_finished(qstat_handler_t *handler, const char* qname, lList **alpp);

static int qstat_xml_pending_jobs_started(qstat_handler_t *handler, lList **alpp);
static int qstat_xml_pending_jobs_finished(qstat_handler_t *handler, lList **alpp);
static int qstat_xml_finished_jobs_started(qstat_handler_t *handler, lList **alpp);
static int qstat_xml_finished_jobs_finished(qstat_handler_t *handler, lList **alpp);
static int qstat_xml_error_jobs_started(qstat_handler_t *handler, lList **alpp);
static int qstat_xml_error_jobs_finished(qstat_handler_t *handler, lList **alpp);
static int qstat_xml_zombie_jobs_started(qstat_handler_t *handler, lList **alpp);
static int qstat_xml_zombie_jobs_finished(qstat_handler_t *handler, lList **alpp);

static int qstat_xml_started(qstat_handler_t* handler, lList** alpp);
static int qstat_xml_finished(qstat_handler_t* handler, lList** alpp);

static int qstat_xml_dummy_started(job_handler_t* handler, lList** alpp);
static int qstat_xml_dummy_finished(job_handler_t* handler, lList** alpp);

typedef struct {

   lListElem *queue_list_elem;
   lListElem *queue_elem;

   lListElem *job_list_elem;
   lList     *job_list;
   lListElem *job_elem;
   
} qstat_xml_ctx_t;


int qstat_xml_handler_init(qstat_handler_t* handler, lList **alpp) {
   int ret = 0;
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)sge_malloc(sizeof(qstat_xml_ctx_t));
   
   DENTER(TOP_LAYER, "qstat_xml_handler_init");
   
   if (ctx == NULL) {
      answer_list_add(alpp, "malloc of qstat_xml_ctx_t failed",
                            STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      ret = -1;
      goto error;
   }
   
   memset(handler, 0, sizeof(qstat_handler_t));
   memset(ctx, 0, sizeof(qstat_xml_ctx_t));
   
   handler->ctx = ctx;
   handler->job_handler.ctx = ctx;
   
   handler->report_queue_started = qstat_xml_queue_started;
   handler->report_queue_finished = qstat_xml_queue_finished;
   handler->report_queue_summary = qstat_xml_queue_summary;
   handler->report_queue_load_alarm = qstat_xml_queue_load_alarm;
   handler->report_queue_suspend_alarm = qstat_xml_queue_suspend_alarm;
   handler->report_queue_message = qstat_xml_queue_message;
   handler->report_queue_resource = qstat_xml_queue_resource;

   handler->job_handler.report_job = qstat_xml_job;

   handler->job_handler.report_sub_tasks_started = qstat_xml_dummy_started;
   handler->job_handler.report_sub_task = qstat_xml_sub_task;
   handler->job_handler.report_sub_tasks_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_additional_info = qstat_xml_job_additional_info;

   handler->job_handler.report_requested_pe = qstat_xml_job_requested_pe;
   handler->job_handler.report_granted_pe = qstat_xml_job_granted_pe;

   handler->job_handler.report_request = qstat_xml_job_request;

   handler->job_handler.report_hard_resources_started = qstat_xml_dummy_started;
   handler->job_handler.report_hard_resource = qstat_xml_job_report_hard_resource;
   handler->job_handler.report_hard_resources_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_soft_resources_started = qstat_xml_dummy_started;
   handler->job_handler.report_soft_resource = qstat_xml_job_soft_resource;
   handler->job_handler.report_soft_resources_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_hard_requested_queues_started = qstat_xml_dummy_started;
   handler->job_handler.report_hard_requested_queue = qstat_xml_job_hard_requested_queue;
   handler->job_handler.report_hard_requested_queues_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_soft_requested_queues_started = qstat_xml_dummy_started;
   handler->job_handler.report_soft_requested_queue = qstat_xml_job_soft_requested_queue;
   handler->job_handler.report_soft_requested_queues_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_master_hard_requested_queues_started = qstat_xml_dummy_started;
   handler->job_handler.report_master_hard_requested_queue = qstat_xml_job_master_hard_requested_queue;
   handler->job_handler.report_master_hard_requested_queues_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_predecessors_requested_started = qstat_xml_dummy_started;
   handler->job_handler.report_predecessor_requested = qstat_xml_job_predecessor_requested;
   handler->job_handler.report_predecessors_requested_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_predecessors_started = qstat_xml_dummy_started;
   handler->job_handler.report_predecessor = qstat_xml_job_predecessor;
   handler->job_handler.report_predecessors_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_ad_predecessors_requested_started = qstat_xml_dummy_started;
   handler->job_handler.report_ad_predecessor_requested = qstat_xml_job_ad_predecessor_requested;
   handler->job_handler.report_ad_predecessors_requested_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_ad_predecessors_started = qstat_xml_dummy_started;
   handler->job_handler.report_ad_predecessor = qstat_xml_job_ad_predecessor;
   handler->job_handler.report_ad_predecessors_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_binding_started = qstat_xml_dummy_started;
   handler->job_handler.report_binding = qstat_xml_job_binding;
   handler->job_handler.report_binding_finished = qstat_xml_dummy_finished;

   handler->job_handler.report_job_finished = qstat_xml_job_finished;

   handler->report_queue_jobs_started = qstat_xml_queue_jobs_started;
   handler->report_queue_jobs_finished = qstat_xml_queue_jobs_finished;
   
   handler->report_pending_jobs_started = qstat_xml_pending_jobs_started;
   handler->report_pending_jobs_finished = qstat_xml_pending_jobs_finished;
   
   handler->report_finished_jobs_started = qstat_xml_finished_jobs_started;
   handler->report_finished_jobs_finished = qstat_xml_finished_jobs_finished;

   handler->report_error_jobs_started = qstat_xml_error_jobs_started;
   handler->report_error_jobs_finished = qstat_xml_error_jobs_finished;
   
   handler->report_zombie_jobs_started = qstat_xml_zombie_jobs_started;
   handler->report_zombie_jobs_finished = qstat_xml_zombie_jobs_finished;
   
   handler->report_finished = qstat_xml_finished; 
   handler->report_started = qstat_xml_started;
error:
   DEXIT;
   return ret;
}

static int qstat_xml_started(qstat_handler_t* handler, lList** alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *attr_elem = NULL;
   
   DENTER(TOP_LAYER, "qstat_xml_started" );
   
   ctx->queue_list_elem = lCreateElem(XMLE_Type);
   attr_elem = lCreateElem(XMLA_Type);
   lSetString(attr_elem, XMLA_Name, "queue_info");
   lSetObject(ctx->queue_list_elem, XMLE_Element, attr_elem);
   lSetBool(ctx->queue_list_elem, XMLE_Print, true);
   
   ctx->job_list_elem = lCreateElem(XMLE_Type);
   attr_elem = lCreateElem(XMLA_Type);
   lSetString(attr_elem, XMLA_Name, "job_info");
   lSetObject(ctx->job_list_elem, XMLE_Element, attr_elem);
   lSetBool(ctx->job_list_elem, XMLE_Print, true);
   lSetList(ctx->job_list_elem, XMLE_List, lCreateList("job_list", XMLE_Type));

   DEXIT;
   return 0;
}

static int qstat_xml_finished(qstat_handler_t* handler, lList** alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *xml_elem = NULL;
   lList *XML_out = NULL;
   DENTER(TOP_LAYER, "qstat_xml_finished");

   XML_out = lCreateList("job_info", XMLE_Type);
   
   lAppendElem(XML_out, ctx->queue_list_elem);
   ctx->queue_list_elem = NULL;
   
   lAppendElem(XML_out, ctx->job_list_elem);
   ctx->job_list_elem = NULL;
   
   xml_elem = xml_getHead("job_info", XML_out, NULL);
   /*lWriteListTo(XML_out, stdout);*/
   lWriteElemXMLTo(xml_elem, stdout, -1);
   lFreeElem(&xml_elem);

   DEXIT;
   return 0;
}

/*
** start and finished functions needed for clients/common/sge_qstat.c do work
*/
static int qstat_xml_dummy_started(job_handler_t* handler _UNUSED, lList** alpp _UNUSED) {
   return 0;
}

static int qstat_xml_dummy_finished(job_handler_t* handler _UNUSED, lList** alpp _UNUSED) {
   return 0;
}

static int qstat_xml_job(job_handler_t* handler, u_long32 jid, job_summary_t *summary, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   qstat_env_t *qstat_env = handler->qstat_env;
   int sge_ext, tsk_ext, sge_urg, sge_pri, sge_time;
   dstring ds = DSTRING_INIT;
   lList *attribute_list = NULL;
   
   DENTER(TOP_LAYER, "qstat_xml_job");

   ctx->job_elem = lCreateElem(XMLE_Type);
   attribute_list = lCreateList("attributes", XMLE_Type);
   lSetList(ctx->job_elem, XMLE_List, attribute_list);
   
   sge_ext = (qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED);
   tsk_ext = (qstat_env->full_listing & QSTAT_DISPLAY_TASKS);
   sge_urg = (qstat_env->full_listing & QSTAT_DISPLAY_URGENCY);
   sge_pri = (qstat_env->full_listing & QSTAT_DISPLAY_PRIORITY);
   sge_time = !sge_ext;
   sge_time = sge_time | tsk_ext | sge_urg | sge_pri;
   
   xml_append_Attr_I(attribute_list, "JB_job_number", jid); 
   xml_append_Attr_D(attribute_list, "JAT_prio", summary->nprior);
   if(sge_ext || sge_pri) {
      xml_append_Attr_D(attribute_list, "JAT_ntix", summary->ntckts);
   }
   if (sge_pri) {
      xml_append_Attr_D(attribute_list, "JB_nppri", summary->nppri);
   }

   if (sge_urg || sge_pri) {
      xml_append_Attr_D(attribute_list, "JB_nurg", summary->nurg);
   }
   if (sge_urg) {
      xml_append_Attr_D8(attribute_list, "JB_rrcontr", summary->rrcontr);
      xml_append_Attr_D8(attribute_list, "JB_wtcontr", summary->wtcontr);
      xml_append_Attr_D8(attribute_list, "JB_dlcontr", summary->dlcontr);
   }

   if (sge_pri) {
      xml_append_Attr_I(attribute_list, "JB_priority", summary->priority);
   } 

   xml_append_Attr_S(attribute_list, "JB_name", summary->name);
   xml_append_Attr_S(attribute_list, "JB_owner", summary->user);

   if (sge_ext) {
      xml_append_Attr_S(attribute_list, "JB_project", summary->project);
      xml_append_Attr_S(attribute_list, "JB_department", summary->department);
   }


   xml_append_Attr_S(attribute_list, "state", summary->state);

   if (sge_time) {
      if (summary->is_running) {
         xml_append_Attr_S(attribute_list, "JAT_start_time", sge_ctimeXML(summary->start_time, &ds));
      }   
      else {
         xml_append_Attr_S(attribute_list, "JB_submission_time", sge_ctimeXML(summary->submit_time, &ds));
      }
   }

   /* deadline time */
   if (sge_urg) {
      if (summary->deadline) {
         xml_append_Attr_S(attribute_list, "JB_deadline", sge_ctimeXML(summary->deadline, &ds));
      }
   }

   if (sge_ext) {
      /* scaled cpu usage */
      if (summary->has_cpu_usage) {
         xml_append_Attr_D(attribute_list, "cpu_usage", summary->cpu_usage);
      } 
      /* scaled mem usage */
      if (summary->has_mem_usage) 
         xml_append_Attr_D(attribute_list, "mem_usage", summary->mem_usage);  
  
      /* scaled io usage */
      if (summary->has_io_usage) 
         xml_append_Attr_D(attribute_list, "io_usage", summary->io_usage);  

      if (!summary->is_zombie) {
         if (sge_ext ||summary->is_queue_assigned) {
            xml_append_Attr_I(attribute_list, "tickets", (int)summary->tickets);
            xml_append_Attr_I(attribute_list, "JB_override_tickets", (int)summary->override_tickets);
            xml_append_Attr_I(attribute_list, "JB_jobshare", (int)summary->share);
            xml_append_Attr_I(attribute_list, "otickets", (int)summary->otickets);
            xml_append_Attr_I(attribute_list, "ftickets", (int)summary->ftickets);
            xml_append_Attr_I(attribute_list, "stickets", (int)summary->stickets);
            /* RH TODO: asked Stefan want is the difference between JAT_share and JB_jobshare */
            xml_append_Attr_D(attribute_list, "JAT_share", summary->share);
         }
      }
   }

   /* if not full listing we need the queue's name in each line */
   if (!(qstat_env->full_listing & QSTAT_DISPLAY_FULL)) {
      xml_append_Attr_S(attribute_list, "queue_name", summary->queue);
   }   

   if ((qstat_env->group_opt & GROUP_NO_PETASK_GROUPS)) {
      /* MASTER/SLAVE information needed only to show parallel job distribution */
      xml_append_Attr_S(attribute_list, "master", summary->master);
   }

   xml_append_Attr_I(attribute_list, "slots", (int)summary->slots);

   if (summary->task_id && summary->is_array) {
      xml_append_Attr_S(attribute_list, "tasks", summary->task_id);
   }

   sge_dstring_free(&ds);   
   DEXIT;
   return 0;
}

static int qstat_xml_job_finished(job_handler_t* handler, u_long32 jid _UNUSED, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   
   DENTER(TOP_LAYER,"qstat_xml_job_finished");
   
   lAppendElem(ctx->job_list, ctx->job_elem); 
   ctx->job_elem = NULL;

   DEXIT;
   return 0;
}

static int qstat_xml_sub_task(job_handler_t* handler, task_summary_t *summary, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *xml_elem = NULL;
   lList *attribute_list = NULL;
   
   DENTER(TOP_LAYER,"qstat_xml_sub_task");
   
   xml_elem = lCreateElem(XMLE_Type);
   attribute_list = lCreateList("attributes", XMLE_Type);
   lSetList(xml_elem, XMLE_List, attribute_list);

   if(summary->task_id) {
      xml_append_Attr_S(attribute_list, "task-id", summary->task_id);      
   }

   xml_append_Attr_S(attribute_list, "state", summary->state);

   if (summary->has_cpu_usage) {
      xml_append_Attr_D(attribute_list, "cpu-usage", summary->cpu_usage);
   }

   if (summary->has_mem_usage) {
      xml_append_Attr_D(attribute_list, "mem-usage", summary->mem_usage);  
   }   

   if (summary->has_io_usage) {
      xml_append_Attr_D(attribute_list, "io-usage", summary->io_usage);  
   }   

   if (summary->has_exit_status) {
      xml_append_Attr_I(attribute_list, "stat", summary->exit_status);
   }   
   
   /* add the sub task to the current job element */
   attribute_list = lGetList(ctx->job_elem, XMLE_Attribute);
   lAppendElem(attribute_list, xml_elem);
   
   DEXIT;
   return 0;
}

static char* ADDITIONAL_TAG_NAMES[] = {
   "ERROR",
   "checkpoint_env",
   "master_queue",
   "full_job_name"
};

static int qstat_xml_job_additional_info(job_handler_t* handler, job_additional_info_t name, const char* value, lList **alpp _UNUSED) {
   
   DENTER(TOP_LAYER,"qstat_xml_job_additional_info");
   
   switch(name) {
      case CHECKPOINT_ENV:
      case MASTER_QUEUE:
         DPRINTF(("Skip additional info %s(%d) = %s\n", ADDITIONAL_TAG_NAMES[name], name, value));
         DEXIT;
         return 0;
      default:
         {
            qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
            /*lListElem *xml_elem = NULL;*/
            lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);
            
            DPRINTF(("Add additional info %s(%d) = %s\n", ADDITIONAL_TAG_NAMES[name], name, value));
            xml_append_Attr_S(attribute_list, ADDITIONAL_TAG_NAMES[name], value);
            /* xml_elem = xml_append_Attr_S(attribute_list, ADDITIONAL_TAG_NAMES[name], value); 
            TODO: xml_addAttribute(xmlElem, "name", lGetString(job, JB_pe)); */
         }
   }
   DEXIT;
   return 0;
}

static int qstat_xml_job_requested_pe(job_handler_t *handler, const char* pe_name, const char* pe_range, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *xml_elem = NULL;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_requested_pe");
   
   xml_elem = xml_append_Attr_S(attribute_list, "requested_pe", pe_range); 
   xml_addAttribute(xml_elem, "name", pe_name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_granted_pe(job_handler_t *handler, const char* pe_name, int pe_slots, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *xml_elem = NULL;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_granted_pe");
   
   xml_elem = xml_append_Attr_I(attribute_list, "granted_pe", pe_slots); 
   xml_addAttribute(xml_elem, "name", pe_name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_request(job_handler_t* handler, const char* name, const char* value, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *xml_elem = NULL;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);
   
   DENTER(TOP_LAYER,"qstat_xml_job_request");
   
   xml_elem = xml_append_Attr_S(attribute_list, "def_hard_request", value); 
   xml_addAttribute(xml_elem, "name", name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_report_hard_resource(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *xml_elem = NULL;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);
   
   DENTER(TOP_LAYER,"qstat_xml_job_report_hard_resource");

   xml_elem = xml_append_Attr_S(attribute_list, "hard_request", value);
   xml_addAttribute(xml_elem, "name", name);
   xml_addAttributeD(xml_elem, "resource_contribution", uc); 
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_soft_resource(job_handler_t *handler, const char* name, const char* value, double uc _UNUSED, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *xml_elem = NULL;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);
   
   DENTER(TOP_LAYER, "qstat_xml_job_soft_resource");

   xml_elem = xml_append_Attr_S(attribute_list, "soft_request", value);
   xml_addAttribute(xml_elem, "name", name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_hard_requested_queue(job_handler_t *handler, const char* name, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_hard_requested_queue");

   xml_append_Attr_S(attribute_list, "hard_req_queue", name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_soft_requested_queue(job_handler_t *handler, const char* name, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_soft_requested_queue");

   xml_append_Attr_S(attribute_list, "soft_req_queue", name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_master_hard_requested_queue(job_handler_t* handler, const char* name, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_master_hard_requested_queue");

   xml_append_Attr_S(attribute_list, "master_hard_req_queue", name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_predecessor_requested(job_handler_t* handler, const char* name, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_predecessor_requested");

   xml_append_Attr_S(attribute_list, "predecessor_jobs_req", name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_predecessor(job_handler_t* handler, u_long32 jid, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_predecessor");

   xml_append_Attr_I(attribute_list, "predecessor_jobs", (int)jid);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_ad_predecessor_requested(job_handler_t* handler, const char* name, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_ad_predecessor_requested");

   xml_append_Attr_S(attribute_list, "ad_predecessor_jobs_req", name);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_ad_predecessor(job_handler_t* handler, u_long32 jid, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_ad_predecessor");

   xml_append_Attr_I(attribute_list, "ad_predecessor_jobs", (int)jid);
   
   DEXIT;
   return 0;
}

static int qstat_xml_job_binding(job_handler_t* handler, const char *binding, lList **alpp) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = lGetList(ctx->job_elem, XMLE_List);

   DENTER(TOP_LAYER, "qstat_xml_job_binding");

   xml_append_Attr_S(attribute_list, "binding", binding);
   
   DEXIT;
   return 0;
}

static int qstat_xml_create_job_list(qstat_handler_t *handler, lList **alpp) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   
   DENTER(TOP_LAYER,"qstat_xml_create_job_list");
   
   if (ctx->job_list != NULL) {
      answer_list_add(alpp, "job_list is not NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }
   
   ctx->job_list = lCreateList("job_list", XMLE_Type);
   if (ctx->job_list == NULL) {
      answer_list_add(alpp, "lCreateElem failed for job_list", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }
   
   
   DEXIT;
   return 0;
}

static int qstat_xml_finish_job_list(qstat_handler_t *handler, const char* state, lList* target_list, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lListElem *job_elem = NULL;
   lListElem *state_elem = NULL;
   lList *attributes = NULL;
   lList *job_list = NULL;
   
   DENTER(TOP_LAYER, "qstat_xml_finish_job_list");
   
   for_each(job_elem, ctx->job_list) {
      attributes = lGetList(job_elem, XMLE_Attribute);
      if (!attributes){
         attributes = lCreateList("attributes", XMLA_Type);
         lSetList(job_elem, XMLE_Attribute, attributes);
      }
      state_elem = lCreateElem(XMLA_Type);
      lSetString(state_elem, XMLA_Name, "state");
      lSetString(state_elem, XMLA_Value, state);
      lAppendElem(attributes, state_elem);
      
      lAppendElem(job_list, job_elem);
   }
   
   lAddList(target_list, &(ctx->job_list));

   DEXIT;
   return 0;
}

static int qstat_xml_queue_jobs_started(qstat_handler_t *handler, const char* qname _UNUSED, lList **alpp) {
   int ret = 0;
   DENTER(TOP_LAYER, "qstat_xml_queue_jobs_started" );

   ret = qstat_xml_create_job_list(handler, alpp);

   DEXIT;
   return ret;
}

static int qstat_xml_queue_jobs_finished(qstat_handler_t *handler, const char* qname _UNUSED, lList **alpp) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   qstat_env_t *qstat_env = handler->qstat_env;
   int ret = 0;
   DENTER(TOP_LAYER, "qstat_xml_queue_jobs_finished");
   
   if (lFirst(ctx->job_list)) {
      lList *job_list = NULL;
      if (qstat_env->full_listing & QSTAT_DISPLAY_FULL ) {
         job_list = lGetList(ctx->queue_elem, XMLE_List);
         if(job_list == NULL) {
            job_list = lCreateList("job_list", XMLE_Type);
            lSetList(ctx->queue_elem, XMLE_List, job_list);
         }
      } else {
        job_list = lGetList(ctx->queue_list_elem, XMLE_List);
         if(job_list == NULL) {
            job_list = lCreateList("job_list", XMLE_Type);
            lSetList(ctx->queue_list_elem, XMLE_List, job_list);
         }
      }
      ret = qstat_xml_finish_job_list(handler, "running", job_list, alpp);
   } else {
      lFreeList(&(ctx->job_list));
   }
   
   DEXIT;
   return ret;   
}


static int qstat_xml_pending_jobs_started(qstat_handler_t *handler, lList **alpp) {
   int ret = 0;
   DENTER(TOP_LAYER, "qstat_xml_pending_jobs_started" );
   ret = qstat_xml_create_job_list(handler, alpp);
   DEXIT;
   return ret;
}

static int qstat_xml_pending_jobs_finished(qstat_handler_t *handler, lList **alpp) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   int ret = 0;
   lList *target_list = NULL;

   DENTER(TOP_LAYER, "qstat_xml_pending_jobs_finished");
   
   target_list = lGetList(ctx->job_list_elem, XMLE_List);
   ret = qstat_xml_finish_job_list(handler, "pending", target_list, alpp);

   DEXIT;
   return ret;
}

static int qstat_xml_finished_jobs_started(qstat_handler_t *handler, lList **alpp) {
   int ret = 0;

   DENTER(TOP_LAYER, "qstat_xml_finished_jobs_started" );

   ret = qstat_xml_create_job_list(handler, alpp);

   DEXIT;
   return ret;
}

static int qstat_xml_finished_jobs_finished(qstat_handler_t *handler, lList **alpp) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   int ret = 0;
   lList *target_list = NULL;

   DENTER(TOP_LAYER, "qstat_xml_finished_jobs_finished");
   
   target_list = lGetList(ctx->job_list_elem, XMLE_List);
   ret = qstat_xml_finish_job_list(handler, "finished", target_list, alpp);
   
   DEXIT;
   return ret;
}

static int qstat_xml_error_jobs_started(qstat_handler_t *handler, lList **alpp) {
   int ret = 0;
   DENTER(TOP_LAYER, "qstat_xml_error_jobs_started" );

   ret = qstat_xml_create_job_list(handler, alpp);

   DEXIT;
   return ret;
}

static int qstat_xml_error_jobs_finished(qstat_handler_t *handler, lList **alpp) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   int ret = 0;
   lList *target_list = NULL;

   DENTER(TOP_LAYER, "qstat_xml_error_jobs_finished");
   
   target_list = lGetList(ctx->job_list_elem, XMLE_List);
   ret = qstat_xml_finish_job_list(handler, "error", target_list, alpp);
   
   DEXIT;
   return ret;
}

static int qstat_xml_zombie_jobs_started(qstat_handler_t *handler, lList **alpp) {
   int ret = 0;

   DENTER(TOP_LAYER, "qstat_xml_zombie_jobs_started" );

   ret = qstat_xml_create_job_list(handler, alpp);

   DEXIT;
   return ret;
}

static int qstat_xml_zombie_jobs_finished(qstat_handler_t *handler, lList **alpp) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   int ret = 0;
   lList *target_list = NULL;
   
   DENTER(TOP_LAYER, "qstat_xml_zombie_jobs_finished");
   
   target_list = lGetList(ctx->job_list_elem, XMLE_List);
   ret = qstat_xml_finish_job_list(handler, "zombie", target_list, alpp);
   
   DEXIT;
   return ret;
}

static int qstat_xml_queue_finished(qstat_handler_t* handler, const char* qname, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   qstat_env_t *qstat_env = handler->qstat_env;   
   lList* queue_list = NULL;

   DENTER(TOP_LAYER, "qstat_xml_queue_finished");
   
   if (qstat_env->full_listing & QSTAT_DISPLAY_FULL) {
      if (ctx->queue_elem == NULL) {
         DPRINTF(("Illegal State: ctx->queue_elem is NULL !!!\n"));
         abort();
      }
      DPRINTF(("add queue_info for queue %s to queue_list\n", qname ));
      
      queue_list = lGetList(ctx->queue_list_elem, XMLE_List);
      if (queue_list == NULL) {
         DPRINTF(("Had empty queue list, create new one\n"));
         /* It is actually defined as "Queue-List" in the schema.  */
         queue_list = lCreateList("Queue-List", XMLE_Type);
         lSetList(ctx->queue_list_elem, XMLE_List, queue_list);
      }
      lAppendElem(queue_list, ctx->queue_elem);
      ctx->queue_elem = NULL;
   } else {
      lFreeElem(&(ctx->queue_elem));
   }

   DEXIT;
   return 0;
}

static int qstat_xml_queue_started(qstat_handler_t* handler, const char* qname, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   qstat_env_t *qstat_env = handler->qstat_env;
   lList *attribute_list = NULL;
   lListElem *temp = NULL;

   DENTER(TOP_LAYER, "qstat_xml_queue_started");
   
   if (qstat_env->full_listing & QSTAT_DISPLAY_FULL) {
      
      if (ctx->queue_elem != NULL) {
         DPRINTF(("Illegal state: ctx->queue_elem has to be NULL"));
         abort();
      }
      
      DPRINTF(("Create ctx->queue_elem for queue %s\n", qname));
   
      temp = lCreateElem(XMLE_Type);
      lSetBool(temp, XMLE_Print, false);
      ctx->queue_elem = lCreateElem(XMLE_Type);
      attribute_list = lCreateList("attributes", XMLE_Type);
      lSetList(temp, XMLE_List, attribute_list);
      lSetObject(ctx->queue_elem, XMLE_Element, temp);
   }   

   DEXIT;
   return 0;
}

static int qstat_xml_queue_summary(qstat_handler_t* handler, const char* qname, queue_summary_t *summary, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = NULL;
   lListElem *xml_elem = NULL;
   DENTER(TOP_LAYER, "qstat_xml_queue_summary");

   if (ctx->queue_elem == NULL) {
      DPRINTF(("Ilegal state: ctx->queue_elem must not be NULL"));
      abort();
   }
   xml_elem = lGetObject(ctx->queue_elem, XMLE_Element);
   attribute_list = lGetList(xml_elem, XMLE_List);
   
   xml_append_Attr_S(attribute_list, "name", qname);        
   xml_append_Attr_S(attribute_list, "qtype", summary->queue_type); 

   /* number of used/free slots */
   xml_append_Attr_I(attribute_list, "slots_used", summary->used_slots); 
   xml_append_Attr_I(attribute_list, "slots_resv", summary->resv_slots); 
   xml_append_Attr_I(attribute_list, "slots_total", summary->total_slots);

   /* load avg */
   if (!summary->has_load_value && summary->has_load_value_from_object) {
      xml_append_Attr_D(attribute_list, "load_avg", summary->load_avg);
   }
   
   /* arch */
   if(summary->arch) {
      xml_append_Attr_S(attribute_list, "arch", summary->arch);
   }

   xml_append_Attr_S(attribute_list, "state", summary->state);

   DEXIT;
   return 0;
}

static int qstat_xml_queue_load_alarm(qstat_handler_t* handler, const char* qname _UNUSED, const char* reason, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   int ret = 0;
   lList *attribute_list = NULL;
   lListElem *xml_elem = NULL;
   DENTER(TOP_LAYER,"qstat_xml_queue_load_alarm");
   
   xml_elem = lGetObject(ctx->queue_elem, XMLE_Element);
   attribute_list = lGetList(xml_elem, XMLE_List);
   
   xml_append_Attr_S(attribute_list, "load-alarm-reason", reason);
   
   DEXIT;
   return ret;
}

static int qstat_xml_queue_suspend_alarm(qstat_handler_t* handler, const char* qname _UNUSED, const char* reason, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = NULL;
   lListElem *xml_elem = NULL;
   DENTER(TOP_LAYER,"qstat_xml_queue_suspend_alarm");
   
   xml_elem = lGetObject(ctx->queue_elem, XMLE_Element);
   attribute_list = lGetList(xml_elem, XMLE_List);
   
   xml_append_Attr_S(attribute_list, "suspend-alarm-reason", reason);
   
   DEXIT;
   return 0;
}

static int qstat_xml_queue_message(qstat_handler_t* handler, const char* qname _UNUSED, const char *message, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = NULL;
   lListElem *xml_elem = NULL;
   DENTER(TOP_LAYER,"qstat_xml_queue_message");

   xml_elem = lGetObject(ctx->queue_elem, XMLE_Element);
   attribute_list = lGetList(xml_elem, XMLE_List);
   
   xml_append_Attr_S(attribute_list, "message", message);
   
   DEXIT;
   return 0;
}

static int qstat_xml_queue_resource(qstat_handler_t* handler, const char* dom, const char* name, const char* value, lList **alpp _UNUSED) {
   qstat_xml_ctx_t *ctx = (qstat_xml_ctx_t*)handler->ctx;
   lList *attribute_list = NULL;
   lListElem *xml_elem = NULL;
   
   DENTER(TOP_LAYER,"qstat_xml_queue_resource");
   
   xml_elem = lGetObject(ctx->queue_elem, XMLE_Element);
   attribute_list = lGetList(xml_elem, XMLE_List);
   
   DPRINTF(("queue resource: %s, %s, %s\n", dom, name, value));
   xml_elem = xml_append_Attr_S(attribute_list, "resource", value);
   xml_addAttribute(xml_elem, "name", name);  
   xml_addAttribute(xml_elem, "type", dom);
   
   DEXIT;
   return 0;
}



void xml_qstat_show_job_info(lList **list, lList **answer_list, qstat_env_t *qstat_env) {
   lListElem *answer = NULL;
   lListElem *xml_elem = NULL;
   bool error = false;
   lListElem* mes;
   lListElem *sme;
   lList *mlp = NULL;
   lListElem *jid_ulng = NULL; 
   DENTER(TOP_LAYER, "xml_qstat_show_job");

   for_each(answer, *answer_list) {
      if (lGetUlong(answer, AN_status) != STATUS_OK) {
         error = true;
         break;
      }
   }

   if (error) {
      xml_elem = xml_getHead("comunication_error", *answer_list, NULL);
      lWriteElemXMLTo(xml_elem, stdout, 
         (qstat_env->full_listing & QSTAT_DISPLAY_BINDING) == QSTAT_DISPLAY_BINDING ? -1 : JB_binding);
      lFreeElem(&xml_elem);
   }
   else {
      /* need to modify list to display correct message */
      
      sme = lFirst(*list);
      if (sme) {
         mlp = lGetList(sme, SME_message_list);         
      }      
      for_each(mes, mlp) {
         lPSortList (lGetList(mes, MES_job_number_list), "I+", ULNG_value);

         for_each(jid_ulng, lGetList(mes, MES_job_number_list)) {
            u_long32 mid;            

            mid = lGetUlong(mes, MES_message_number);
            lSetString(mes,MES_message,sge_schedd_text(mid+SCHEDD_INFO_OFFSET));
         }
            
      }
           
      /* print out xml info from list */
      
      xml_elem = xml_getHead("message", *list, NULL);
      lWriteElemXMLTo(xml_elem, stdout, 
         (qstat_env->full_listing & QSTAT_DISPLAY_BINDING) == QSTAT_DISPLAY_BINDING ? -1 : JB_binding);
      lFreeElem(&xml_elem);
      *list = NULL;
   }

   lFreeList(answer_list);
 
   DEXIT;
   return;
}

void xml_qstat_show_job(lList **job_list, lList **msg_list, lList **answer_list, lList **id_list, qstat_env_t *qstat_env){
   lListElem *answer = NULL;
   lListElem *xml_elem = NULL;
   bool error = false;
   bool suppress_binding_data = (qstat_env->full_listing & QSTAT_DISPLAY_BINDING) == QSTAT_DISPLAY_BINDING ? false : true;

   DENTER(TOP_LAYER, "xml_qstat_show_job");
   
   for_each(answer, *answer_list) {
      if (lGetUlong(answer, AN_status) != STATUS_OK) {
         error = true;
         break;
      }
   }

   if (suppress_binding_data) {
      DPRINTF(("Data concerning binding will not ne shown\n"));
   }

   if (error) {
      xml_elem = xml_getHead("comunication_error", *answer_list, NULL);
      lWriteElemXMLTo(xml_elem, stdout, suppress_binding_data ? JB_binding : -1); 
      lFreeElem(&xml_elem);
   } else {
      if (lGetNumberOfElem(*job_list) == 0) {
         xml_elem = xml_getHead("unknown_jobs", *id_list, NULL);
         lWriteElemXMLTo(xml_elem, stdout, suppress_binding_data ? JB_binding : -1);
         lFreeElem(&xml_elem);
         *id_list = NULL;
      } else {
         lList *XML_out = lCreateList("detailed_job_info", XMLE_Type);
         lListElem *xmlElem = NULL;
         lListElem *attrElem = NULL;
        
         /* add job infos */
         xmlElem = lCreateElem(XMLE_Type);
         attrElem = lCreateElem(XMLA_Type); 
         lSetString(attrElem, XMLA_Name, "djob_info");
         lSetObject(xmlElem, XMLE_Element, attrElem);
         lSetBool(xmlElem, XMLE_Print, true);
         lSetList(xmlElem, XMLE_List, *job_list);
         lAppendElem(XML_out, xmlElem);

         /* add messages */
         xmlElem = lCreateElem(XMLE_Type);
         attrElem = lCreateElem(XMLA_Type);         
         lSetString(attrElem, XMLA_Name, "messages");
         lSetObject(xmlElem, XMLE_Element, attrElem);
         lSetBool(xmlElem, XMLE_Print, true);
         lSetList(xmlElem, XMLE_List, *msg_list);
         lAppendElem(XML_out, xmlElem);
         
         xml_elem = xml_getHead("detailed_job_info", XML_out, NULL);
         lWriteElemXMLTo(xml_elem, stdout, suppress_binding_data ? JB_binding : -1);
         lFreeElem(&xml_elem);
         *job_list = NULL;
         *msg_list = NULL;
      }
   }

   lFreeList(answer_list);
   DEXIT;
   return;
}


