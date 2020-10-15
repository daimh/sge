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
#include <string.h>
#include <limits.h>
#include <math.h>
#include <fnmatch.h>

#include "uti/sge_rmon.h"
#include "uti/setup_path.h"
#include "uti/sge_hostname.h"
#include "uti/sge_prog.h"
#include "uti/sge_bootstrap.h"

#include "sched/sort_hosts.h"
#include "sched/sge_select_queue.h"

#include "sgeobj/parse.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_all_listsL.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "basis_types.h"
#include "sge_qstat.h"
#include "sge_qquota.h"
#include "sge.h"

#include "msg_clients_common.h"

#define HEAD_FORMAT "%-18s %-20.20s %s\n"

typedef struct {
   const char* user;
   const char* project;
   const char* pe;
   const char* queue;
   const char* host;
} qquota_filter_t;

static bool get_all_lists(sge_gdi_ctx_class_t *ctx, lList **rqs_l, lList **centry_l, lList **userset_l, lList **hgroup_l, lList **exechost_l, lList *hostref_list, lList **alpp);

static char *qquota_get_next_filter(stringT filter, const char *cp);
static bool qquota_print_out_rule(lListElem *rule, dstring rule_name, const char *limit_name,
                                  const char *usage_value, const char *limit_value, qquota_filter_t filter,
                                  report_handler_t* report_handler, lList *printed_rules, lList **alpp);

static bool qquota_print_out_filter(lListElem *filter, const char *name, const char *value, dstring *buffer, report_handler_t *report_handler, lList **alpp);

/****** qquota_output/qquota_output() ********************************************
*  NAME
*     qquota_output() -- qquota output function
*
*  SYNOPSIS
*     bool qquota_output(void *ctx, lList *host_list, lList *resource_match_list, 
*     lList *user_list, lList *pe_list, lList *project_list, lList 
*     *cqueue_list, lList **alpp, report_handler_t* report_handler) 
*
*  FUNCTION
*     print resource quota rule and the limit
*
*  INPUTS
*     void *ctx                        - gdi handler
*     lList *host_list                 - selected hosts
*     lList *resource_match_list       - selected resources
*     lList *user_list                 - selected users
*     lList *pe_list                   - selected pes
*     lList *project_list              - selected projects
*     lList *cqueue_list               - selected cluster queues
*     lList **alpp                     - answer list
*     report_handler_t* report_handler - report handler for xml output
*
*  RESULT
*     bool - true on success
*            false on error
*
*  NOTES
*     MT-NOTE: qquota_output() is MT safe 
*
*******************************************************************************/
bool qquota_output(sge_gdi_ctx_class_t *ctx, lList *host_list, lList *resource_match_list, lList *user_list,
                 lList *pe_list, lList *project_list, lList *cqueue_list, lList **alpp,
                 report_handler_t* report_handler) 
{
   lList *rqs_list = NULL;
   lList *centry_list = NULL;
   lList *userset_list = NULL;
   lList *hgroup_list = NULL;
   lList *exechost_list = NULL;

   lListElem* global_host = NULL;
   lListElem* exec_host = NULL;
   lList* printed_rules = NULL;  /* Hash list of already printed resource quota rules (possible with -u user1,user2,user3...) */

   bool ret = true;
   int xml_ret = 0;

   qquota_filter_t qquota_filter = { "*",
                                     "*",
                                     "*",
                                     "*",
                                     "*" };

   dstring rule_name = DSTRING_INIT;

   DENTER(TOP_LAYER, "qquota_output");

   /* If no user is requested on command line we set the current user as default */
   qquota_filter.user = ctx->get_username(ctx);

   ret = get_all_lists(ctx, &rqs_list, &centry_list, &userset_list, &hgroup_list, &exechost_list, host_list, alpp);

   if (ret == true) {
      lListElem *rqs = NULL;
      printed_rules = lCreateList("rule_hash", ST_Type); 
      global_host = host_list_locate(exechost_list, SGE_GLOBAL_NAME);

      if (report_handler != NULL) {
         xml_ret = report_handler->report_started(report_handler, alpp);
         if (xml_ret != QQUOTA_SUCCESS) {
            ret = false;
            goto qquota_output_error;
         }
      }

      for_each(rqs, rqs_list) {
         lListElem *rule = NULL;
         int rule_count = 1;

         if (lGetBool(rqs, RQS_enabled) == false) {
            continue;
         }

         for_each(rule, lGetList(rqs, RQS_rule)) { 
            lListElem *user_ep = lFirst(user_list);
            lListElem *project_ep = lFirst(project_list);
            lListElem *pe_ep = lFirst(pe_list);
            lListElem *queue_ep = lFirst(cqueue_list);
            lListElem *host_ep = lFirst(host_list);
            /* fixme: for #1315, check lGetString(rqs, RQS_name) below
               However, this routine is used in jgdi, so that needs
               changing in sync  */
            do {
               if (user_ep != NULL) {
                  qquota_filter.user = lGetString(user_ep, ST_name);
               }
               do {
                  if (project_ep != NULL) {
                     qquota_filter.project = lGetString(project_ep, ST_name);
                  }
                  do {
                     if (pe_ep != NULL) {
                        qquota_filter.pe = lGetString(pe_ep, ST_name);
                     }
                     do {
                        if (queue_ep != NULL) {
                           qquota_filter.queue = lGetString(queue_ep, ST_name);
                        }
                        do {
                           if (host_ep != NULL) {
                              qquota_filter.host = lGetString(host_ep, ST_name);
                           }
                         
                           if (rqs_is_matching_rule(rule, qquota_filter.user, NULL, qquota_filter.project,
                                                     qquota_filter.pe, qquota_filter.host,
                                                     qquota_filter.queue, userset_list, hgroup_list)) {
                              lListElem *limit = NULL;

                              for_each(limit, lGetList(rule, RQR_limit)) {
                                 const char *limit_name = lGetString(limit, RQRL_name);
                                 double limit_val;
                                 lList *rue_list = lGetList(limit, RQRL_usage);
                                 lListElem *raw_centry = centry_list_locate(centry_list, limit_name);
                                 lListElem *rue_elem = NULL;

                                 if (raw_centry == NULL) {
                                    /* undefined centries can be ignored */
                                    DPRINTF(("centry %s not defined -> IGNORING\n", limit_name));
                                    continue;
                                 }

                                 if ((resource_match_list != NULL) && 
                                     ((centry_list_locate(resource_match_list, limit_name) == NULL) &&
                                     (centry_list_locate(resource_match_list, lGetString(raw_centry, CE_shortcut)) == NULL))) {
                                    DPRINTF(("centry %s was not requested on CLI -> IGNORING\n", limit_name));
                                    continue;
                                 }

                                 if (lGetString(rule, RQR_name)) {
                                    sge_dstring_sprintf(&rule_name, "%s/%s", lGetString(rqs, RQS_name), lGetString(rule, RQR_name));
                                 } else {
                                    sge_dstring_sprintf(&rule_name, "%s/%d", lGetString(rqs, RQS_name), rule_count);
                                 }

                                 if (lGetUlong(raw_centry, CE_consumable)) {
                                    /* for consumables we need to walk through the utilization and search for matching values */
                                    DPRINTF(("found centry %s - consumable\n", limit_name));
                                    for_each(rue_elem, rue_list) {
                                       u_long32 dominant = 0;
                                       const char *rue_name = lGetString(rue_elem, RUE_name);
                                       char *cp = NULL;
                                       stringT user, project, pe, queue, host;
                                       dstring limit_str = DSTRING_INIT; 
                                       dstring value_str = DSTRING_INIT;
                                       qquota_filter_t qf = { NULL, NULL, NULL, NULL, NULL };

                                       /* check user name */
                                       cp = qquota_get_next_filter(user, rue_name);
                                       /* usergroups have the same beginning character @ as host groups */
                                       if (is_hgroup_name(qquota_filter.user)) {
                                          lListElem *ugroup = NULL;

                                          if ((ugroup = userset_list_locate(userset_list, &qquota_filter.user[1])) != NULL) {
                                             if (sge_contained_in_access_list(user, NULL, ugroup, NULL) == 0) {
                                                continue;
                                             }
                                          }
                                       } else {
                                          if ((strcmp(user, "-") != 0) && (strcmp(qquota_filter.user, "*") != 0)
                                               && (fnmatch(qquota_filter.user, user, 0) != 0)) {
                                             continue;
                                          }
                                       }

                                       /* check project */
                                       cp = qquota_get_next_filter(project, cp);
                                       if ((strcmp(project, "-") != 0) && (strcmp(qquota_filter.project, "*") != 0) 
                                             && (fnmatch(qquota_filter.project, project, 0) != 0)) {
                                          continue;
                                       }
                                       /* check parallel environment */
                                       cp = qquota_get_next_filter(pe, cp);
                                       if ((strcmp(pe, "-") != 0) && (strcmp(qquota_filter.pe, "*") != 0) &&
                                           (fnmatch(qquota_filter.pe, pe, 0) != 0) ) {
                                          continue;
                                       }
                                       /* check cluster queue */
                                       cp = qquota_get_next_filter(queue, cp);
                                       if ((strcmp(queue, "-") != 0) && (strcmp(qquota_filter.queue, "*") != 0) &&
                                           (fnmatch(qquota_filter.queue, queue, 0) != 0)) {
                                          continue;
                                       }
                                       /* check host name */
                                       cp = qquota_get_next_filter(host, cp);
                                       if (is_hgroup_name(qquota_filter.host)) {
                                          lListElem *hgroup = NULL;

                                          if ((hgroup = hgroup_list_locate(hgroup_list, qquota_filter.host)) != NULL) {
                                             lList *host_list = NULL;
                                             hgroup_find_all_references(hgroup, NULL, hgroup_list, &host_list, NULL);
                                             if (host_list == NULL && lGetElemHost(host_list, HR_name, host) == NULL) {
                                                lFreeList(&host_list);
                                                continue;
                                             }
                                             lFreeList(&host_list);
                                          }
                                       } else {
                                          if ((strcmp(host, "-") != 0) && (strcmp(qquota_filter.host, "*") != 0) &&
                                              (fnmatch(qquota_filter.host, host, 0) != 0) ) {
                                             continue;
                                          }
                                       }
                                       if (lGetBool(limit, RQRL_dynamic)) {
                                          exec_host = host_list_locate(exechost_list, host);
                                          limit_val = scaled_mixed_load(lGetString(limit, RQRL_value), global_host, exec_host, centry_list);
                                       } else {
                                          limit_val = lGetDouble(limit, RQRL_dvalue);
                                       }
                                       lSetDouble(raw_centry, CE_pj_doubleval, limit_val);
                                       sge_get_dominant_stringval(raw_centry, &dominant, &limit_str);

                                       lSetDouble(raw_centry,CE_pj_doubleval, lGetDouble(rue_elem, RUE_utilized_now));
                                       sge_get_dominant_stringval(raw_centry, &dominant, &value_str);

                                       qf.user = user;
                                       qf.project = project;
                                       qf.pe = pe;
                                       qf.queue = queue;
                                       qf.host = host;
                                       ret = qquota_print_out_rule(rule, rule_name, limit_name, 
                                                                   sge_dstring_get_string(&value_str), sge_dstring_get_string(&limit_str),
                                                                   qf, report_handler, printed_rules, alpp);

                                       sge_dstring_free(&limit_str);
                                       sge_dstring_free(&value_str);
                                    }
                                 } else {
                                    /* static values */
                                    qquota_filter_t qf = { NULL, NULL, NULL, NULL, NULL };

                                    DPRINTF(("found centry %s - static value\n", limit_name));
                                    ret = qquota_print_out_rule(rule, rule_name, limit_name, 
                                                                NULL, lGetString(limit, RQRL_value),
                                                                qf, report_handler, printed_rules, alpp);

                                 }
                              }
                           }
                        } while ((host_ep = lNext(host_ep)));
                     } while ((queue_ep = lNext(queue_ep)));
                  } while ((pe_ep = lNext(pe_ep)));
               } while ((project_ep = lNext(project_ep)));
            } while ((user_ep = lNext(user_ep)));
            rule_count++;
         }
      }

      if (report_handler != NULL) {
         report_handler->report_finished(report_handler, alpp);
      }
   }

qquota_output_error:
   sge_dstring_free(&rule_name);
   lFreeList(&rqs_list);
   lFreeList(&centry_list);
   lFreeList(&userset_list);
   lFreeList(&hgroup_list);
   lFreeList(&exechost_list);
   lFreeList(&printed_rules);

   DRETURN(ret);
}

/****** qquota_output/get_all_lists() ******************************************
*  NAME
*     get_all_lists() -- get all lists from qmaster
*
*  SYNOPSIS
*     static bool get_all_lists(sge_gdi_ctx_class_t *ctx, lList **rqs_l, lList 
*     **centry_l, lList **userset_l, lList **hgroup_l, lList **exechost_l, 
*     lList *hostref_l, lList **alpp) 
*
*  FUNCTION
*     Gets copies of queue-, job-, complex-, exechost-list  from qmaster.
*      The lists are stored in the .._l pointerpointer-parameters.
*      WARNING: Lists previously stored in this pointers are not destroyed!!
*
*  INPUTS
*     void *context      - gdi context
*     lList **rqs_l     -  resource quota set list (RQS_Type)
*     lList **centry_l   - consumable resource list (CE_Type)
*     lList **userset_l  - userset list (US_Type)
*     lList **hgroup_l   - host group list (HG_Type)
*     lList **exechost_l - exechost list (EH_Type)
*     lList *hostref_l   - selected hosts (ST_Type)
*     lList **alpp       - answer list
*
*  RESULT
*     static bool - true on success
*                   false on error
*
*  NOTES
*     MT-NOTE: get_all_lists() is MT safe 
*
*******************************************************************************/
static bool
get_all_lists(sge_gdi_ctx_class_t *ctx, lList **rqs_l, lList **centry_l, lList **userset_l,
              lList **hgroup_l, lList **exechost_l, lList *hostref_l, lList **alpp)
{
   lListElem *ep = NULL;
   lEnumeration *what = NULL;
   lCondition *where = NULL, *nw = NULL;
   lList *mal = NULL;
   int rqs_id, ce_id, userset_id, hgroup_id, eh_id;
   state_gdi_multi state = STATE_GDI_MULTI_INIT;
   
   DENTER(TOP_LAYER, "get_all_lists");

   /*
   ** resource quota sets
   */
   what = lWhat("%T(ALL)", RQS_Type);
   rqs_id = ctx->gdi_multi(ctx, 
                          alpp, SGE_GDI_RECORD, SGE_RQS_LIST, SGE_GDI_GET, 
                          NULL, NULL, what, false, &state, true);
   lFreeWhat(&what);

   if (answer_list_has_error(alpp)) {
      DRETURN(false);
   }
   
   /*
   ** complexes
   */
   what = lWhat("%T(ALL)", CE_Type);
   ce_id = ctx->gdi_multi(ctx, 
                          alpp, SGE_GDI_RECORD, SGE_CE_LIST, SGE_GDI_GET, 
                          NULL, NULL, what, false, &state, true);
   lFreeWhat(&what);

   if (answer_list_has_error(alpp)) {
      DRETURN(false);
   }
   /*
   ** usersets 
   */
   what = lWhat("%T(ALL)", US_Type);
   userset_id = ctx->gdi_multi(ctx, 
                          alpp, SGE_GDI_RECORD, SGE_US_LIST, SGE_GDI_GET, 
                          NULL, NULL, what, false, &state, true);
   lFreeWhat(&what);

   if (answer_list_has_error(alpp)) {
      DRETURN(false);
   }
   /*
   ** host groups 
   */
   what = lWhat("%T(ALL)", HGRP_Type);
   hgroup_id = ctx->gdi_multi(ctx, 
                          alpp, SGE_GDI_RECORD, SGE_HGRP_LIST, SGE_GDI_GET, 
                          NULL, NULL, what, false, &state, true);
   lFreeWhat(&what);
   /*
   ** exec hosts
   */
   for_each(ep, hostref_l) {
      nw = lWhere("%T(%I h= %s)", EH_Type, EH_name, lGetString(ep, ST_name));
      if (!where)
         where = nw;
      else
         where = lOrWhere(where, nw);
   }
   /* the global host has to be retrieved as well */
   if (where != NULL) {
      nw = lWhere("%T(%I == %s)", EH_Type, EH_name, SGE_GLOBAL_NAME);
      where = lOrWhere(where, nw);
   }
   
   nw = lWhere("%T(%I != %s)", EH_Type, EH_name, SGE_TEMPLATE_NAME);
   if (where)
      where = lAndWhere(where, nw);
   else
      where = nw;

   what = lWhat("%T(%I %I %I %I)", EH_Type, EH_name, EH_load_list, EH_consumable_config_list, EH_resource_utilization);
   eh_id = ctx->gdi_multi(ctx, alpp, SGE_GDI_SEND, SGE_EH_LIST, SGE_GDI_GET, 
                          NULL, where, what, false, &state, true);
   ctx->gdi_wait(ctx, alpp, &mal, &state);
   lFreeWhat(&what);
   lFreeWhere(&where);

   if (answer_list_has_error(alpp)) {
      DRETURN(false);
   }

   /* --- resource quota sets */
   lFreeList(alpp);
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_RQS_LIST, rqs_id,
                                 mal, rqs_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(false);
   }

   /* --- complex attribute */
   lFreeList(alpp);
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_CE_LIST, ce_id,
                                 mal, centry_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(false);
   }
   /* --- usersets */
   lFreeList(alpp);
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_US_LIST, userset_id,
                                 mal, userset_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(false);
   }
   /* --- hostgroups */
   lFreeList(alpp);
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_HGRP_LIST, hgroup_id,
                                 mal, hgroup_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(false);
   }
   /* --- exec hosts*/
   lFreeList(alpp);
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_EH_LIST, eh_id,
                                 mal, exechost_l);

   lFreeList(&mal);

   if (answer_list_has_error(alpp)) {
      DRETURN(false);
   }

   DRETURN(true);
}

/****** qquota_output/qquota_get_next_filter() *********************************
*  NAME
*     qquota_get_next_filter() -- tokenize rue_name of usage
*
*  SYNOPSIS
*     static char* qquota_get_next_filter(char *filter, const char *cp) 
*
*  FUNCTION
*     The rue_name has the type /user_name/project_name/pe_name/queue_name/host_name.
*     This function tokenizes the rue_name and gives always one element back
*
*  INPUTS
*     char *filter   - store for the token 
*     const char *cp - pointer to rue_name
*
*  RESULT
*     static char* - pointer for the next token
*
*  NOTES
*     MT-NOTE: qquota_get_next_filter() is not MT safe 
*
*******************************************************************************/
static char *qquota_get_next_filter(stringT filter, const char *cp)
{
   char *ret = NULL;

   ret = strchr(cp, '/')+1;
   if (ret - cp < MAX_STRING_SIZE && ret - cp > 1) { 
      snprintf(filter, ret - cp, "%s", cp);
   } else {
      sprintf(filter, "-");
   }

   return ret;
}

/****** qquota_output/qquota_print_out_rule() **********************************
*  NAME
*     qquota_print_out_rule() -- print out rule
*
*  SYNOPSIS
*     static bool qquota_print_out_rule(lListElem *rule, dstring rule_name, 
*     const char *limit_name, const char *usage_value, const char *limit_value, 
*     qquota_filter_t qfilter, report_handler_t* report_handler, lList **alpp)
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lListElem *rule                  - resource quota rule (RQR_Type)
*     dstring rule_name                - rule name (eg. ruleset1/rule1)
*     const char *limit_name           - limitation name (eg. slots)
*     const char *usage_value          - debited usage
*     const char *limit_value          - configured limitation
*     qquota_filter_t qfilter          - filter tuple
*     report_handler_t* report_handler - handler for xml output
*     lList **alpp                     - answer list
*
*  RESULT
*     static bool - true on success
*                   false on error
*
*  NOTES
*     MT-NOTE: qquota_print_out_rule() is not MT safe 
*
*******************************************************************************/
static bool qquota_print_out_rule(lListElem *rule, dstring rule_name, const char *limit_name,
                                  const char *usage_value, const char *limit_value, qquota_filter_t qfilter,
                                  report_handler_t* report_handler, lList *printed_rules, lList **alpp)
{
   static bool printheader = true;
   bool ret = true;
   dstring filter_str = DSTRING_INIT;
   dstring limitation = DSTRING_INIT;
   dstring token = DSTRING_INIT;

   sge_dstring_sprintf(&token, "%s,%s,%s,%s,%s,%s,%s", sge_dstring_get_string(&rule_name),
                                                             limit_name,
                                                             qfilter.user? qfilter.user: "",
                                                             qfilter.project? qfilter.project: "",
                                                             qfilter.pe? qfilter.pe: "",
                                                             qfilter.queue? qfilter.queue: "",
                                                             qfilter.host? qfilter.host: "");

   if (lGetElemStr(printed_rules, ST_name, sge_dstring_get_string(&token)) != NULL) {
      sge_dstring_free(&token);
      sge_dstring_free(&filter_str);
      sge_dstring_free(&limitation);
      return ret;
   }

   lAddElemStr(&printed_rules, ST_name, sge_dstring_get_string(&token), ST_Type);

   if (report_handler != NULL) {
      report_handler->report_limit_rule_begin(report_handler, sge_dstring_get_string(&rule_name), alpp);
   } else {
      if (printheader == true) {
         printheader = false;
         printf(HEAD_FORMAT, MSG_HEADER_RULE, MSG_HEADER_LIMIT, MSG_HEADER_FILTER);
         printf("--------------------------------------------------------------------------------\n");
      }
   }

   qquota_print_out_filter(lGetObject(rule, RQR_filter_users), "users", qfilter.user, &filter_str, report_handler, alpp);
   qquota_print_out_filter(lGetObject(rule, RQR_filter_projects), "projects", qfilter.project, &filter_str, report_handler, alpp);
   qquota_print_out_filter(lGetObject(rule, RQR_filter_pes), "pes", qfilter.pe, &filter_str, report_handler, alpp);
   qquota_print_out_filter(lGetObject(rule, RQR_filter_queues), "queues", qfilter.queue, &filter_str, report_handler, alpp);
   qquota_print_out_filter(lGetObject(rule, RQR_filter_hosts), "hosts", qfilter.host, &filter_str, report_handler, alpp);

   if (report_handler != NULL) {
      report_handler->report_resource_value(report_handler, limit_name,
                                            limit_value,
                                            usage_value,
                                            alpp);
      report_handler->report_limit_rule_finished(report_handler, sge_dstring_get_string(&rule_name), alpp);
   } else {
      if (usage_value == NULL) {
         sge_dstring_sprintf(&limitation, "%s=%s", limit_name, limit_value);
      } else {
         sge_dstring_sprintf(&limitation, "%s=%s/%s", limit_name, usage_value, limit_value);
      }
      if (sge_dstring_strlen(&filter_str) == 0) {
         sge_dstring_append(&filter_str, "-");
      }
      printf(HEAD_FORMAT, sge_dstring_get_string(&rule_name), sge_dstring_get_string(&limitation), sge_dstring_get_string(&filter_str));
   }

   sge_dstring_free(&token);
   sge_dstring_free(&filter_str);
   sge_dstring_free(&limitation);
   return ret;
}

/****** qquota_output/qquota_print_out_filter() ********************************
*  NAME
*     qquota_print_out_filter() -- prints out filter element
*
*  SYNOPSIS
*     static bool qquota_print_out_filter(lListElem *filter, const char *name, 
*     const char *value, dstring *buffer, report_handler_t *report_handler, 
*     lList **alpp) 
*
*  FUNCTION
*     this function prints out the filter configured in the rule
*
*  INPUTS
*     lListElem *filter                - filter element (RQRF_Type)
*     const char *name                 - filter type name
*     const char *value                - filter value
*     dstring *buffer                  - buffer
*     report_handler_t *report_handler - handler for report handler
*     lList **alpp                     - answer list
*
*  RESULT
*     static bool - true on success
*                   false on error
*
*  NOTES
*     MT-NOTE: qquota_print_out_filter() is MT safe 
*
*******************************************************************************/
static bool
qquota_print_out_filter(lListElem *filter, const char *name, const char *value,
                        dstring *buffer, report_handler_t *report_handler, lList **alpp) 
{
   bool ret = true;
   lListElem *scope;
   
   if (filter != NULL) {
      if (!lGetBool(filter, RQRF_expand) || value == NULL) {
         if (report_handler != NULL) {
            for_each(scope, lGetList(filter, RQRF_scope)) {
               report_handler->report_limit_string_value(report_handler, name, lGetString(scope, ST_name), false, alpp);
            }
            for_each(scope, lGetList(filter, RQRF_xscope)) {
               report_handler->report_limit_string_value(report_handler, name, lGetString(scope, ST_name), true, alpp);
            }
         } else {
            if (sge_dstring_strlen(buffer) != 0) {
               sge_dstring_append(buffer, " ");
            }
            sge_dstring_append(buffer, name);
            sge_dstring_append(buffer, " ");
            rqs_append_filter_to_dstring(filter, buffer, alpp);
         }
      } else {
        if (report_handler != NULL) {
          report_handler->report_limit_string_value(report_handler, name, value, false, alpp);
        } else {
            if (sge_dstring_strlen(buffer) != 0) {
               sge_dstring_append(buffer, " ");
            }
          sge_dstring_append(buffer, name);
          sge_dstring_append(buffer, " ");
          sge_dstring_append(buffer, value);
        }
      }
   }

   return ret;
}
