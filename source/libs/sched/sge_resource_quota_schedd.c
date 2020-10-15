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

#include <string.h>
#include <limits.h>

#include "uti/sge_rmon.h"
#include "uti/sge_hostname.h"
#include "uti/sge_log.h"
#include "uti/sge_parse_num_par.h"

#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_host.h"

#include "sge_ct_SCT_L.h"
#include "sge_ct_REF_L.h"
#include "sge_ct_CT_L.h"
#include "sge_ct_CCT_L.h"
#include "sge_ct_CTI_L.h"

#include "sge_complex_schedd.h"
#include "sge_select_queue.h"
#include "sge_resource_quota_schedd.h"
#include "sort_hosts.h"
#include "sge_schedd_text.h"
#include "schedd_message.h"

static void rqs_can_optimize(const lListElem *rule, bool *host, bool *queue, sge_assignment_t *a);

static void rqs_expand_cqueues(const lListElem *rule, sge_assignment_t *a);
static void rqs_expand_hosts(const lListElem *rule, sge_assignment_t *a);

static bool is_cqueue_global(const lListElem *rule);
static bool is_host_global(const lListElem *rule);

static bool is_cqueue_expand(const lListElem *rule);
static bool is_host_expand(const lListElem *rule);

static bool cqueue_shadowed(const lListElem *rule, sge_assignment_t *a);
static bool host_shadowed(const lListElem *rule, sge_assignment_t *a);

static void rqs_excluded_hosts(const lListElem *rule, sge_assignment_t *a);
static void rqs_excluded_cqueues(const lListElem *rule, sge_assignment_t *a);


/****** sge_resource_quota_schedd/rqs_set_dynamical_limit() ***********************
*  NAME
*     rqs_set_dynamical_limit() -- evaluate dynamical limit
*
*  SYNOPSIS
*     bool rqs_set_dynamical_limit(lListElem *limit, lListElem 
*     *global_host, lListElem *exec_host, lList *centry) 
*
*  FUNCTION
*     The function evaluates if necessary the dynamical limit for a host and
*     sets the evaluated double value in the given limitation element (RQRL_dvalue).
*
*     An evaluation is necessary if the limit boolean RQRL_dynamic is true. This
*     field is set by qmaster during the rule set verification
*
*  INPUTS
*     lListElem *limit       - limitation (RQRL_Type)
*     lListElem *global_host - global host (EH_Type)
*     lListElem *exec_host   - exec host (EH_Type)
*     lList *centry          - consumable resource list (CE_Type)
*
*  RESULT
*     bool - always true
*
*  NOTES
*     MT-NOTE: rqs_set_dynamical_limit() is MT safe 
*
*******************************************************************************/
bool
rqs_set_dynamical_limit(lListElem *limit, lListElem *global_host, lListElem *exec_host, lList *centry) {

   DENTER(TOP_LAYER, "rqs_set_dynamical_limit");

   if (lGetBool(limit, RQRL_dynamic)) {
      double dynamic_limit = scaled_mixed_load(lGetString(limit, RQRL_value), global_host, exec_host, centry);
      DPRINTF(("found a dynamic limit for host %s with value %d\n", lGetHost(exec_host, EH_name), (int)dynamic_limit));
      lSetDouble(limit, RQRL_dvalue, dynamic_limit);
   } 

   DRETURN(true);
}

/****** sge_resource_quota_schedd/rqs_match_assignment() ***********************
*  NAME
*     rqs_match_assignment() -- match resource quota rule any queue instance
*
*  SYNOPSIS
*     static bool rqs_match_assignment(const lListElem *rule, sge_assignment_t 
*     *a) 
*
*  FUNCTION
*     Check whether a resource quota rule can match any queue instance. If
*     if does not match due to users/projects/pes scope one can rule this
*     out.
*    
*     Note: As long as rqs_match_assignment() is not used for parallel jobs
*           passing NULL as PE request is perfectly fine.
*
*  INPUTS
*     const lListElem *rule - Resource quota rule
*     sge_assignment_t *a   - Scheduler assignment 
*
*  RESULT
*     static bool - True if it matches 
*
*  NOTES
*     MT-NOTE: rqs_match_assignment() is MT safe 
*******************************************************************************/
static bool rqs_match_assignment(const lListElem *rule, sge_assignment_t *a)
{
   return (rqs_filter_match(lGetObject(rule, RQR_filter_projects), FILTER_PROJECTS, a->project, NULL, NULL, NULL) &&
           rqs_filter_match(lGetObject(rule, RQR_filter_users), FILTER_USERS, a->user, a->acl_list, NULL, a->group) &&
           rqs_filter_match(lGetObject(rule, RQR_filter_pes), FILTER_PES, NULL, NULL, NULL, NULL))?true:false;
}


/****** sge_resource_quota_schedd/cqueue_shadowed() ****************************
*  NAME
*     cqueue_shadowed() -- Check for cluster queue rule before current rule
*
*  SYNOPSIS
*     static bool cqueue_shadowed(const lListElem *rule, sge_assignment_t *a)
*
*  FUNCTION
*     Check whether there is any cluster queue specific rule before the
*     current rule.
*
*  INPUTS
*     const lListElem *rule - Current rule
*     sge_assignment_t *a   - Scheduler assignment
*
*  RESULT
*     static bool - True if shadowed
*
*  EXAMPLE
*     limit queue Q001 to F001=1
*     limit host gridware to F001=0  (--> returns 'true' due to 'Q001' meaning
*                               that gridware can't be generally ruled out )
*
*  NOTES
*     MT-NOTE: cqueue_shadowed() is MT safe
*******************************************************************************/
static bool cqueue_shadowed(const lListElem *rule, sge_assignment_t *a)
{
   while ((rule = lPrev(rule))) {
      if (rqs_match_assignment(rule, a) && !is_cqueue_global(rule)) {
         return true;
      }
   }
   return false;
}

/****** sge_resource_quota_schedd/host_shadowed() ******************************
*  NAME
*     host_shadowed() -- Check for host rule before current rule
*
*  SYNOPSIS
*     static bool host_shadowed(const lListElem *rule, sge_assignment_t *a)
*
*  FUNCTION
*     Check whether there is any host specific rule before the
*     current rule.
*
*  INPUTS
*     const lListElem *rule - Current rule
*     sge_assignment_t *a   - Scheduler assignment
*
*  RESULT
*     static bool - True if shadowed
*
*  EXAMPLE
*     limit host gridware to F001=1
*     limit queue Q001 to F001=0  (--> returns 'true' due to 'gridware' meaning
*                               that Q001 can't be generally ruled out )
*
*  NOTES
*     MT-NOTE: host_shadowed() is MT safe
*******************************************************************************/
static bool host_shadowed(const lListElem *rule, sge_assignment_t *a)
{
   while ((rule = lPrev(rule))) {
      if (rqs_match_assignment(rule, a) && !is_host_global(rule)) {
         return true;
      }
   }
   return false;
}

/****** sge_resource_quota_schedd/cqueue_shadowed_by() *************************
*  NAME
*     cqueue_shadowed_by() -- Check rules shadowing current cluster queue rule
*
*  SYNOPSIS
*     static bool cqueue_shadowed_by(const char *cqname, const lListElem *rule,
*     sge_assignment_t *a)
*
*  FUNCTION
*     Check if cluster queue in current rule is shadowed.
*
*  INPUTS
*     const char *cqname    - Cluster queue name to check
*     const lListElem *rule - Current rule
*     sge_assignment_t *a   - Assignment
*
*  RESULT
*     static bool - True if shadowed
*
*  EXAMPLE
*     limits queues Q001,Q002 to F001=1
*     limits queues Q002,Q003 to F001=1 (--> returns 'true' for Q002 and 'false' for Q003)
*
*  NOTES
*     MT-NOTE: cqueue_shadowed_by() is MT safe
*******************************************************************************/
static bool cqueue_shadowed_by(const char *cqname, const lListElem *rule, sge_assignment_t *a)
{
   while ((rule = lPrev(rule))) {
      if (rqs_match_assignment(rule, a) &&
          rqs_filter_match(lGetObject(rule, RQR_filter_queues), FILTER_QUEUES, cqname, NULL, NULL, NULL)) {
         return true;
      }
   }

   return false;
}

/****** sge_resource_quota_schedd/host_shadowed_by() ***************************
*  NAME
*     host_shadowed_by() -- ???
*
*  SYNOPSIS
*     static bool host_shadowed_by(const char *host, const lListElem *rule,
*     sge_assignment_t *a)
*
*  FUNCTION
*     Check if host in current rule is shadowed.
*
*  INPUTS
*     const char *cqname    - Host name to check
*     const lListElem *rule - Current rule
*     sge_assignment_t *a   - Assignment
*
*  RESULT
*     static bool - True if shadowed
*
*  EXAMPLE
*     limits hosts host1,host2 to F001=1
*     limits hosts host2,host3 to F001=1 (--> returns 'true' for host2 and 'false' for host3)
*
*  NOTES
*     MT-NOTE: host_shadowed_by() is MT safe
*******************************************************************************/
static bool host_shadowed_by(const char *host, const lListElem *rule, sge_assignment_t *a)
{
   while ((rule = lPrev(rule))) {
      if (rqs_match_assignment(rule, a) &&
          rqs_filter_match(lGetObject(rule, RQR_filter_hosts), FILTER_HOSTS, host, NULL, a->hgrp_list, NULL)) {
         return true;
      }
   }

   return false;
}

/****** sge_resource_quota_schedd/rqs_can_optimize() ***************************
*  NAME
*     rqs_can_optimize() -- Poke whether a queue/host negation can be made
*
*  SYNOPSIS
*     static void rqs_can_optimize(const lListElem *rule, bool *host, bool
*     *queue, sge_assignment_t *a)
*
*  FUNCTION
*     A global limit was hit with 'rule'. This function helps to determine
*     to what extent we can profit from that situation. If there is no
*     previous matching rule within the same rule set any other queue/host
*     can be skipped.
*
*  INPUTS
*     const lListElem *rule - Rule
*     bool *host            - Any previous rule with a host scope?
*     bool *queue           - Any previous rule with a queue scope?
*     sge_assignment_t *a   - Scheduler assignment
*
*  NOTES
*     MT-NOTE: rqs_can_optimize() is MT safe
*******************************************************************************/
static void rqs_can_optimize(const lListElem *rule, bool *host, bool *queue, sge_assignment_t *a)
{
   bool host_shadowed = false, queue_shadowed = false;

   const lListElem *prev = rule;
   while ((prev = lPrev(prev))) {
      if (!rqs_match_assignment(rule, a))
         continue;
      if (!is_host_global(prev))
         host_shadowed = true;
      if (!is_cqueue_global(prev))
         queue_shadowed = true;
   }

   *host = host_shadowed;
   *queue = queue_shadowed;

   return;
}

/****** sge_resource_quota_schedd/rqs_excluded_cqueues() ***********************
*  NAME
*     rqs_excluded_cqueues() -- Find excluded queues
*
*  SYNOPSIS
*     static void rqs_excluded_cqueues(const lListElem *rule, sge_assignment_t *a)
*
*  FUNCTION
*     Find queues that are excluded by previous rules.
*
*  INPUTS
*     const lListElem *rule    - The rule
*     sge_assignment_t *a      - Scheduler assignment
*
*  EXAMPLE
*      limit        projects {*} queues !Q001 to F001=1
*      limit        to F001=0   ( ---> returns Q001 in a->skip_cqueue_list)
*
*  NOTES
*     MT-NOTE: rqs_excluded_cqueues() is MT safe
*******************************************************************************/
static void rqs_excluded_cqueues(const lListElem *rule, sge_assignment_t *a)
{
   lListElem *cq;
   const lListElem *prev;
   int ignored = 0, excluded = 0;

   DENTER(TOP_LAYER, "rqs_excluded_cqueues");

   for_each (cq, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
      const char *cqname = lGetString(cq, CQ_name);
      bool exclude = true;

      if (lGetElemStr(a->skip_cqueue_list, CTI_name, cqname)) {
         ignored++;
         continue;
      }

      prev = rule;
      while ((prev = lPrev(prev))) {
         if (!rqs_match_assignment(rule, a))
            continue;

         if (rqs_filter_match(lGetObject(prev, RQR_filter_queues), FILTER_QUEUES, cqname, NULL, NULL, NULL)) {
            exclude = false;
            break;
         }
      }
      if (exclude) {
         lAddElemStr(&(a->skip_cqueue_list), CTI_name, cqname, CTI_Type);
         excluded++;
      }
   }

   if (ignored + excluded == 0) {
      CRITICAL((SGE_EVENT, "not a single queue excluded in rqs_excluded_cqueues()"));
   }

   DRETURN_VOID;
}

/****** sge_resource_quota_schedd/rqs_excluded_hosts() *************************
*  NAME
*     rqs_excluded_hosts() -- Find excluded hosts
*
*  SYNOPSIS
*     static void rqs_excluded_hosts(const lListElem *rule, sge_assignment_t *a)
*
*  FUNCTION
*     Find hosts that are excluded by previous rules.
*
*  INPUTS
*     const lListElem *rule    - The rule
*     sge_assignment_t *a      - Scheduler assignment
*
*  EXAMPLE
*      limit        projects {*} queues !gridware to F001=1
*      limit        to F001=0   ( ---> returns gridware in skip_host_list)
*
*  NOTES
*     MT-NOTE: rqs_excluded_hosts() is MT safe
*******************************************************************************/
static void rqs_excluded_hosts(const lListElem *rule, sge_assignment_t *a)
{
   lListElem *eh;
   const lListElem *prev;
   int ignored = 0, excluded = 0;

   DENTER(TOP_LAYER, "rqs_excluded_hosts");

   for_each (eh, a->host_list) {
      const char *hname = lGetHost(eh, EH_name);
      bool exclude = true;

      if (lGetElemStr(a->skip_host_list, CTI_name, hname)) {
         ignored++;
         continue;
      }

      prev = rule;
      while ((prev = lPrev(prev))) {
         if (!rqs_match_assignment(rule, a))
            continue;

         if (rqs_filter_match(lGetObject(prev, RQR_filter_hosts), FILTER_HOSTS, hname, NULL, a->hgrp_list, NULL)) {
            exclude = false;
            break;
         }
      }
      if (exclude) {
         lAddElemStr(&(a->skip_host_list), CTI_name, hname, CTI_Type);
         excluded++;
      }
   }

   if (ignored + excluded == 0) {
      CRITICAL((SGE_EVENT, "not a single host excluded in rqs_excluded_hosts()"));
   }

   DRETURN_VOID;
}

/****** sge_resource_quota_schedd/rqs_expand_cqueues() *************************
*  NAME
*     rqs_expand_cqueues() -- Add all matching cqueues to the list
*
*  SYNOPSIS
*     void rqs_expand_cqueues(const lListElem *rule)
*
*  FUNCTION
*     The names of all cluster queues that match the rule are added to
*     the skip list without duplicates.
*
*  INPUTS
*     const lListElem *rule    - RQR_Type
*
*  NOTES
*     MT-NOTE: rqs_expand_cqueues() is not MT safe
*******************************************************************************/
static void rqs_expand_cqueues(const lListElem *rule, sge_assignment_t *a)
{
   const lListElem *cq;
   const char *cqname;
   lListElem *qfilter = lGetObject(rule, RQR_filter_queues);

   DENTER(TOP_LAYER, "rqs_expand_cqueues");

   for_each (cq, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
      cqname = lGetString(cq, CQ_name);
      if (lGetElemStr(a->skip_cqueue_list, CTI_name, cqname))
         continue;
      if (rqs_filter_match(qfilter, FILTER_QUEUES, cqname, NULL, NULL, NULL) && !cqueue_shadowed_by(cqname, rule, a))
         lAddElemStr(&(a->skip_cqueue_list), CTI_name, cqname, CTI_Type);
   }

   DEXIT;
   return;
}

/****** sge_resource_quota_schedd/rqs_expand_hosts() ***************************
*  NAME
*     rqs_expand_hosts() -- Add all matching hosts to the list
*
*  SYNOPSIS
*     void rqs_expand_hosts(const lListElem *rule, lList **skip_host_list,
*     const lList *host_list, lList *hgrp_list)
*
*  FUNCTION
*     The names of all hosts that match the rule are added to
*     the skip list without duplicates.
*
*  INPUTS
*     const lListElem *rule  - RQR_Type
*     const lList *host_list - EH_Type
*
*  NOTES
*     MT-NOTE: rqs_expand_hosts() is MT safe
*******************************************************************************/
static void rqs_expand_hosts(const lListElem *rule, sge_assignment_t *a)
{
   const lListElem *eh;
   const char *hname;
   lListElem *hfilter = lGetObject(rule, RQR_filter_hosts);

   for_each (eh, a->host_list) {
      hname = lGetHost(eh, EH_name);
      if (lGetElemStr(a->skip_host_list, CTI_name, hname))
         continue;
      if (rqs_filter_match(hfilter, FILTER_HOSTS, hname, NULL, a->hgrp_list, NULL) && !host_shadowed_by(hname, rule, a))
         lAddElemStr(&(a->skip_host_list), CTI_name, hname, CTI_Type);
   }

   return;
}

static bool is_global(const lListElem *rule, int nm)
{
   lListElem *filter = lGetObject(rule, nm);
   if (!filter)
      return true;
   if (lGetSubStr(filter, ST_name, "*", RQRF_scope) && lGetNumberOfElem(lGetList(filter, RQRF_xscope))==0)
      return true;
   return false;
}

/****** sge_resource_quota_schedd/is_cqueue_global() ***************************
*  NAME
*     is_cqueue_global() -- Global rule with regards to cluster queues?
*
*  SYNOPSIS
*     bool is_cqueue_global(const lListElem *rule)
*
*  INPUTS
*     const lListElem *rule - RQR_Type
*
*  RESULT
*     bool - True if cluster queues play no role with the rule
*
*  NOTES
*     MT-NOTE: is_cqueue_global() is MT safe
*******************************************************************************/
static bool is_cqueue_global(const lListElem *rule)
{
   return is_global(rule, RQR_filter_queues);
}


/****** sge_resource_quota_schedd/is_host_global() *****************************
*  NAME
*     is_host_global() -- Global rule with regards to hosts?
*
*  SYNOPSIS
*     bool is_host_global(const lListElem *rule)
*
*  FUNCTION
*     Return true if hosts play no role with the rule
*
*  INPUTS
*     const lListElem *rule - RQR_Type
*
*  RESULT
*     bool - True if hosts play no role with the rule
*
*  NOTES
*     MT-NOTE: is_host_global() is MT safe
*******************************************************************************/
static bool is_host_global(const lListElem *rule)
{
   return is_global(rule, RQR_filter_hosts);
}

static bool is_expand(const lListElem *rule, int nm)
{
   lListElem *filter = lGetObject(rule, nm);
   if (filter && lGetBool(filter, RQRF_expand) == true)
      return true;
   else
      return false;
}


/****** sge_resource_quota_schedd/is_host_expand() *****************************
*  NAME
*     is_host_expand() -- Returns true if rule expands on hosts
*
*  SYNOPSIS
*     bool is_host_expand(const lListElem *rule)
*
*  FUNCTION
*     Returns true if rule expands on hosts.
*
*  INPUTS
*     const lListElem *rule - RQR_Type
*
*  RESULT
*     bool - True if rule expands on hosts
*
*  EXAMPLE
*      "hosts {*}" returns true
*      "hosts @allhosts" returns false
*
*  NOTES
*     MT-NOTE: is_host_expand() is MT safe
*******************************************************************************/
static bool is_host_expand(const lListElem *rule)
{
   return is_expand(rule, RQR_filter_hosts);
}

/****** sge_resource_quota_schedd/is_cqueue_expand() ***************************
*  NAME
*     is_cqueue_expand() -- Returns true if rule expands on cluster queues
*
*  SYNOPSIS
*     bool is_cqueue_expand(const lListElem *rule)
*
*  FUNCTION
*     Returns true if rule expands on cluster queues.
*
*  INPUTS
*     const lListElem *rule - RQR_Type
*
*  RESULT
*     bool - True if rule expands on hosts
*
*  EXAMPLE
*      "queues {*}" returns true
*      "queues Q001,Q002" returns false
*
*  NOTES
*     MT-NOTE: is_cqueue_expand() is MT safe
*******************************************************************************/
static bool is_cqueue_expand(const lListElem *rule)
{
   return is_expand(rule, RQR_filter_queues);
}

/****** sge_resource_quota_schedd/rqs_exceeded_sort_out() **********************
*  NAME
*     rqs_exceeded_sort_out() -- Rule out queues/hosts whenever possible
*
*  SYNOPSIS
*     bool rqs_exceeded_sort_out(sge_assignment_t *a, const lListElem *rule,
*     const dstring *rule_name, const char* queue_name, const char* host_name)
*
*  FUNCTION
*     This function tries to rule out hosts and cluster queues after a
*     quota exceeding was found for a limitation rule with specific queue
*     instance.
*
*     When a limitation was exceeded that applies to the entire
*     cluster 'true' is returned, 'false' otherwise.
*
*  INPUTS
*     sge_assignment_t *a      - Scheduler assignment type
*     const lListElem *rule    - The exceeded rule
*     const dstring *rule_name - Name of the rule (monitoring only)
*     const char* queue_name   - Cluster queue name
*     const char* host_name    - Host name
*
*  RESULT
*     bool - True upon global limits exceeding
*
*  NOTES
*     MT-NOTE: rqs_exceeded_sort_out() is MT safe
*******************************************************************************/
static bool rqs_exceeded_sort_out(sge_assignment_t *a, const lListElem *rule, const dstring *rule_name,
   const char* queue_name, const char* host_name)
{
   bool cq_global = is_cqueue_global(rule);
   bool eh_global = is_host_global(rule);

   DENTER(TOP_LAYER, "rqs_exceeded_sort_out");

   if ((!cq_global && !eh_global) || (cq_global && eh_global &&
         (is_cqueue_expand(rule) || is_host_expand(rule)))) { /* failure at queue instance limit */
      DPRINTF(("QUEUE INSTANCE: resource quota set %s deny job execution on %s@%s\n", 
            sge_dstring_get_string(rule_name), queue_name, host_name));
      DRETURN(false);
   }

   if (cq_global && eh_global) { /* failure at a global limit */
      bool host_shadowed, queue_shadowed;

      rqs_can_optimize(rule, &host_shadowed, &queue_shadowed, a);
      if (!host_shadowed && !queue_shadowed) {
         DPRINTF(("GLOBAL: resource quota set %s deny job execution globally\n", 
               sge_dstring_get_string(rule_name)));
         DRETURN(true);
      }

      if (host_shadowed && queue_shadowed) {
         rqs_excluded_cqueues(rule, a);
         rqs_excluded_hosts(rule, a);
         DPRINTF(("QUEUE INSTANCE: resource quota set %s deny job execution on %s@%s\n", 
               sge_dstring_get_string(rule_name), queue_name, host_name));
         DRETURN(false);
      }

      if (queue_shadowed) {
         rqs_excluded_cqueues(rule, a);
         DPRINTF(("QUEUE: resource quota set %s deny job execution in all its queues\n", 
               sge_dstring_get_string(rule_name)));
      } else { /* must be host_shadowed */
         rqs_excluded_hosts(rule, a);
         DPRINTF(("HOST: resource quota set %s deny job execution in all its queues\n", 
               sge_dstring_get_string(rule_name)));
      }

      DRETURN(false);
   }

   if (!cq_global) { /* failure at a cluster queue limit */

      if (host_shadowed(rule, a)) {
         DPRINTF(("QUEUE INSTANCE: resource quota set %s deny job execution on %s@%s\n", 
               sge_dstring_get_string(rule_name), queue_name, host_name));
         DRETURN(false);
      }

      if (lGetBool(lGetObject(rule, RQR_filter_queues), RQRF_expand) == true) {
         lAddElemStr(&(a->skip_cqueue_list), CTI_name, queue_name, CTI_Type);
         DPRINTF(("QUEUE: resource quota set %s deny job execution in queue %s\n", 
               sge_dstring_get_string(rule_name), queue_name));
      } else {
         rqs_expand_cqueues(rule, a);
         DPRINTF(("QUEUE: resource quota set %s deny job execution in all its queues\n", 
               sge_dstring_get_string(rule_name)));
      }

      DRETURN(false);
   }

   /* must be (!eh_global) */
   { /* failure at a host limit */

      if (cqueue_shadowed(rule, a)) {
         DPRINTF(("QUEUE INSTANCE: resource quota set %s deny job execution on %s@%s\n", 
               sge_dstring_get_string(rule_name), queue_name, host_name));
         DRETURN(false);
      }

      if (lGetBool(lGetObject(rule, RQR_filter_hosts), RQRF_expand) == true) {
         lAddElemStr(&(a->skip_host_list), CTI_name, host_name, CTI_Type);
         DPRINTF(("HOST: resource quota set %s deny job execution at host %s\n",    
               sge_dstring_get_string(rule_name), host_name));
      } else {
         rqs_expand_hosts(rule, a);
         DPRINTF(("HOST: resource quota set %s deny job execution at all its hosts\n", 
               sge_dstring_get_string(rule_name)));
      }

      DRETURN(false);
   }
}

/****** sge_resource_quota_schedd/rqs_exceeded_sort_out_par() ******************
*  NAME
*     rqs_exceeded_sort_out_par() -- Rule out queues/hosts whenever possible
*
*  SYNOPSIS
*     void rqs_exceeded_sort_out_par(sge_assignment_t *a, const lListElem 
*     *rule, const dstring *rule_name, const char* queue_name, const char* 
*     host_name) 
*
*  FUNCTION
*     Function wrapper around rqs_exceeded_sort_out() for parallel jobs.
*     In contrast to the sequential case global limit exceeding is handled
*     by adding all cluster queue names to the a->skip_cqueue_list.
*
*  INPUTS
*     sge_assignment_t *a      - Scheduler assignment type
*     const lListElem *rule    - The exceeded rule
*     const dstring *rule_name - Name of the rule (monitoring only)
*     const char* queue_name   - Cluster queue name
*     const char* host_name    - Host name
*
*  NOTES
*     MT-NOTE: rqs_exceeded_sort_out_par() is MT safe 
*******************************************************************************/
static void rqs_exceeded_sort_out_par(sge_assignment_t *a, const lListElem *rule, const dstring *rule_name,
   const char* queue_name, const char* host_name)
{
   if (rqs_exceeded_sort_out(a, rule, rule_name, queue_name, host_name)) {
      rqs_expand_hosts(rule, a);
   }
}

/****** sge_resource_quota_schedd/sge_user_is_referenced_in_rqs() ********************
*  NAME
*     sge_user_is_referenced_in_rqs() -- search for user reference in rqs 
*
*  SYNOPSIS
*     bool sge_user_is_referenced_in_rqs(const lList *rqs, const char *user, 
*     lList *acl_list) 
*
*  FUNCTION
*     Search for a user reference in the resource quota sets
*
*  INPUTS
*     const lList *rqs - resource quota set list
*     const char *user  - user to search
*     const char *group - user's group
*     lList *acl_list   - acl list for user resolving
*
*  RESULT
*     bool - true if user was found
*            false if user was not found
*
*  NOTES
*     MT-NOTE: sge_user_is_referenced_in_rqs() is MT safe 
*
*******************************************************************************/
bool sge_user_is_referenced_in_rqs(const lList *rqs, const char *user, const char *group, lList *acl_list)
{
   bool ret = false;
   lListElem *ep;

   for_each(ep, rqs) {
      lList *rule_list = lGetList(ep, RQS_rule);
      lListElem *rule;
      for_each(rule, rule_list) {
         /* there may be no per-user limitation and also not limitation that is special for this user */
         if ((is_expand(rule, RQR_filter_users) || !is_global(rule, RQR_filter_users)) &&
             rqs_filter_match(lGetObject(rule, RQR_filter_users), FILTER_USERS, user,
             acl_list, NULL, group)) {
            ret = true;
            break;
         }
      }
      if (ret == true) {
         break;
      }
   }
   return ret;
}


/****** sge_resource_quota_schedd/check_and_debit_rqs_slots() *********************
*  NAME
*     check_and_debit_rqs_slots() -- Determine RQS limit slot amount and debit
*
*  SYNOPSIS
*     static void check_and_debit_rqs_slots(sge_assignment_t *a, const char 
*     *host, const char *queue, int *slots, int *slots_qend, dstring 
*     *rule_name, dstring *rue_name, dstring *limit_name) 
*
*  FUNCTION
*     The function determines the final slot and slots_qend amount due
*     to all resource quota limitations that apply for the queue instance. 
*     Both slot amounts get debited from the a->limit_list to keep track 
*     of still available amounts per resource quota limit.
*
*  INPUTS
*     sge_assignment_t *a - Assignment data structure
*     const char *host    - hostname
*     const char *queue   - queuename
*     int *slots          - needed/available slots
*     int *slots_qend     - needed/available slots_qend
*     dstring *rule_name  - caller maintained buffer
*     dstring *rue_name   - caller maintained buffer
*     dstring *limit_name - caller maintained buffer
*
*  NOTES
*     MT-NOTE: check_and_debit_rqs_slots() is MT safe 
*******************************************************************************/
void parallel_check_and_debit_rqs_slots(sge_assignment_t *a, const char *host, const char *queue, 
      int *slots, int *slots_qend, dstring *rule_name, dstring *rue_name, dstring *limit_name)
{
   lListElem *rqs, *rule;
   const char* user = a->user;
   const char* group = a->group;
   const char* project = a->project;
   const char* pe = a->pe_name;

   DENTER(TOP_LAYER, "parallel_check_and_debit_rqs_slots");

   /* first step - see how many slots are left */
   for_each(rqs, a->rqs_list) {

      /* ignore disabled rule sets */
      if (!lGetBool(rqs, RQS_enabled)) {
         continue;
      }
      sge_dstring_clear(rule_name);
      rule = rqs_get_matching_rule(rqs, user, group, project, pe, host, queue, a->acl_list, a->hgrp_list, rule_name);
      if (rule != NULL) {
         lListElem *rql;
         rqs_get_rue_string(rue_name, rule, user, project, host, queue, pe);
         sge_dstring_sprintf(limit_name, "%s=%s", sge_dstring_get_string(rule_name), sge_dstring_get_string(rue_name));
         if ((rql = lGetElemStr(a->limit_list, RQL_name, sge_dstring_get_string(limit_name)))) {
            *slots = MIN(*slots, lGetInt(rql, RQL_slots));
            *slots_qend = MIN(*slots_qend, lGetInt(rql, RQL_slots_qend));
         } else {
            *slots = *slots_qend = 0;
         }
      }

      if (*slots == 0 && *slots_qend == 0) {
         break;
      }
   }

   /* second step - reduce number of remaining slots  */
   if (*slots != 0 || *slots_qend != 0) {
      for_each(rqs, a->rqs_list) {

         /* ignore disabled rule sets */
         if (!lGetBool(rqs, RQS_enabled)) {
            continue;
         }
         sge_dstring_clear(rule_name);
         rule = rqs_get_matching_rule(rqs, user, group, project, pe, host, queue, a->acl_list, a->hgrp_list, rule_name);
         if (rule != NULL) {
            lListElem *rql;
            rqs_get_rue_string(rue_name, rule, user, project, host, queue, pe);
            sge_dstring_sprintf(limit_name, "%s=%s", sge_dstring_get_string(rule_name), sge_dstring_get_string(rue_name));
            rql = lGetElemStr(a->limit_list, RQL_name, sge_dstring_get_string(limit_name));
            lSetInt(rql, RQL_slots,      lGetInt(rql, RQL_slots) - *slots);
            lSetInt(rql, RQL_slots_qend, lGetInt(rql, RQL_slots_qend) - *slots_qend);
         }
      }
   }

   DPRINTF(("check_and_debit_rqs_slots(%s@%s) slots: %d slots_qend: %d\n", queue, host, *slots, *slots_qend));

   DRETURN_VOID;
}

void parallel_revert_rqs_slot_debitation(sge_assignment_t *a, const char *host, const char *queue, 
      int slots, int slots_qend, dstring *rule_name, dstring *rue_name, dstring *limit_name)
{
   lListElem *rqs, *rule;
   const char* user = a->user;
   const char* group = a->group;
   const char* project = a->project;
   const char* pe = a->pe_name;

   DENTER(TOP_LAYER, "parallel_revert_rqs_slot_debitation");

   for_each(rqs, a->rqs_list) {

      /* ignore disabled rule sets */
      if (!lGetBool(rqs, RQS_enabled)) {
         continue;
      }
      sge_dstring_clear(rule_name);
      rule = rqs_get_matching_rule(rqs, user, group, project, pe, host, queue, a->acl_list, a->hgrp_list, rule_name);
      if (rule != NULL) {
         lListElem *rql;
         rqs_get_rue_string(rue_name, rule, user, project, host, queue, pe);
         sge_dstring_sprintf(limit_name, "%s=%s", sge_dstring_get_string(rule_name), sge_dstring_get_string(rue_name));
         rql = lGetElemStr(a->limit_list, RQL_name, sge_dstring_get_string(limit_name));
         DPRINTF(("limit: %s %d <--- %d\n", sge_dstring_get_string(limit_name), 
               lGetInt(rql, RQL_slots), lGetInt(rql, RQL_slots)+slots));
         lSetInt(rql, RQL_slots,      lGetInt(rql, RQL_slots) + slots);
         lSetInt(rql, RQL_slots_qend, lGetInt(rql, RQL_slots_qend) + slots_qend);
      }
   }

   DRETURN_VOID;
}

/****** sge_resource_quota_schedd/parallel_limit_slots_by_time() ********************
*  NAME
*     parallel_limit_slots_by_time() -- Determine number of slots avail. within
*                                       time frame
*
*  SYNOPSIS
*     static dispatch_t parallel_limit_slots_by_time(const sge_assignment_t *a, 
*     lList *requests, int *slots, int *slots_qend, lListElem *centry, lListElem 
*     *limit, dstring rue_name) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     const sge_assignment_t *a - job info structure (in)
*     lList *requests           - Job request list (CE_Type)
*     int *slots                - out: free slots
*     int *slots_qend           - out: free slots in the far far future
*     lListElem *centry         - Load information for the resource
*     lListElem *limit          - limitation (RQRL_Type)
*     dstring rue_name          - rue_name saved in limit sublist RQRL_usage
*     lListElem *qep            - queue instance (QU_Type)
*
*  RESULT
*     static dispatch_t - DISPATCH_OK        got an assignment
*                       - DISPATCH_NEVER_CAT no assignment for all jobs of that category
*
*  NOTES
*     MT-NOTE: parallel_limit_slots_by_time() is not MT safe 
*
*  SEE ALSO
*     parallel_rc_slots_by_time
*******************************************************************************/
static dispatch_t 
parallel_limit_slots_by_time(const sge_assignment_t *a, lList *requests, 
                 int *slots, int *slots_qend, lListElem *centry, lListElem *limit, dstring *rue_name, lListElem *qep)
{
   lList *tmp_centry_list = lCreateList("", CE_Type);
   lList *tmp_rue_list = lCreateList("", RUE_Type);
   lListElem *tmp_centry_elem = NULL;
   lListElem *tmp_rue_elem = NULL;
   lList *rue_list = lGetList(limit, RQRL_usage);
   dispatch_t result = DISPATCH_NEVER_CAT;

   DENTER(TOP_LAYER, "parallel_limit_slots_by_time");

   /* create tmp_centry_list */
   tmp_centry_elem = lCopyElem(centry);
   lSetDouble(tmp_centry_elem, CE_doubleval, lGetDouble(limit, RQRL_dvalue));
   lAppendElem(tmp_centry_list, tmp_centry_elem);

   /* create tmp_rue_list */
   lWriteListTo(rue_list, stdout);
   fflush(stdout);
   tmp_rue_elem = lCopyElem(lGetElemStr(rue_list, RUE_name, sge_dstring_get_string(rue_name)));
   if (tmp_rue_elem == NULL) {
      DPRINTF(("RD: 1\n"));
      tmp_rue_elem = lCreateElem(RUE_Type);
   }
{
   const char *object_name = "bla";
   lListElem *rde;
   DPRINTF(("resource utilization: %s \"%s\" %f utilized now\n", 
         object_name?object_name:"<unknown_object>", lGetString(tmp_rue_elem, RUE_name),
            lGetDouble(tmp_rue_elem, RUE_utilized_now)));
   for_each (rde, lGetList(tmp_rue_elem, RUE_utilized)) {
      DPRINTF(("\t"sge_U32CFormat"  %f\n", lGetUlong(rde, RDE_time), lGetDouble(rde, RDE_amount))); 
   }
   DPRINTF(("resource utilization: %s \"%s\" %f utilized now non-exclusive\n", 
         object_name?object_name:"<unknown_object>", lGetString(tmp_rue_elem, RUE_name),
            lGetDouble(tmp_rue_elem, RUE_utilized_now_nonexclusive)));
   for_each (rde, lGetList(tmp_rue_elem, RUE_utilized_nonexclusive)) {
      DPRINTF(("\t"sge_U32CFormat"  %f\n", lGetUlong(rde, RDE_time), lGetDouble(rde, RDE_amount))); 
   }
}

   lSetString(tmp_rue_elem, RUE_name, lGetString(limit, RQRL_name));
   lAppendElem(tmp_rue_list, tmp_rue_elem);

   result = parallel_rc_slots_by_time(a, requests, slots, 
                                      slots_qend, tmp_centry_list, tmp_rue_list, NULL,  
                                      false, qep, DOMINANT_LAYER_RQS, 0.0, RQS_TAG,
                                      false, SGE_RQS_NAME, true);
   
   lFreeList(&tmp_centry_list);
   lFreeList(&tmp_rue_list);

   DRETURN(result);
}


/****** sge_resource_quota_schedd/parallel_rqs_slots_by_time() ******************
*  NAME
*     parallel_rqs_slots_by_time() -- Determine number of slots avail within
*                                      time frame
*
*  SYNOPSIS
*     dispatch_t parallel_rqs_slots_by_time(const sge_assignment_t *a, 
*     int *slots, int *slots_qend, const char *host, const char *queue) 
*
*  FUNCTION
*     This function iterates for a queue instance over all resource quota sets
*     and evaluates the number of slots available.
*
*  INPUTS
*     const sge_assignment_t *a - job info structure (in)
*     int *slots                - out: # free slots
*     int *slots_qend           - out: # free slots in the far far future
*     lListElem *qep            - QU_Type Elem
*
*  RESULT
*     static dispatch_t - DISPATCH_OK        got an assignment
*                       - DISPATCH_NEVER_CAT no assignment for all jobs of that category
*
*  NOTES
*     MT-NOTE: parallel_rqs_slots_by_time() is not MT safe 
*
*  SEE ALSO
*     ri_slots_by_time()
*     
*******************************************************************************/
dispatch_t
parallel_rqs_slots_by_time(sge_assignment_t *a, int *slots, int *slots_qend, lListElem *qep)
{
   dispatch_t result = DISPATCH_OK;
   int tslots = INT_MAX;
   int tslots_qend = INT_MAX;
   const char* queue = lGetString(qep, QU_qname);
   const char* host = lGetHost(qep, QU_qhostname);

   DENTER(TOP_LAYER, "parallel_rqs_slots_by_time");

   if (lGetNumberOfElem(a->rqs_list) != 0) {
      const char* user = a->user;
      const char* group = a->group;
      const char* project = a->project;
      const char* pe = a->pe_name;
      lListElem *rql, *rqs;
      dstring rule_name = DSTRING_INIT;
      dstring rue_string = DSTRING_INIT;
      dstring limit_name = DSTRING_INIT;
      lListElem *exec_host = host_list_locate(a->host_list, host);

      if (a->pi)
         a->pi->par_rqs++;

      for_each(rqs, a->rqs_list) {
         lListElem *rule = NULL;

         /* ignore disabled rule sets */
         if (!lGetBool(rqs, RQS_enabled)) {
            continue;
         }
         sge_dstring_clear(&rule_name);
         rule = rqs_get_matching_rule(rqs, user, group, project, pe, host, queue, a->acl_list, a->hgrp_list, &rule_name);
         if (rule != NULL) {
            lListElem *limit = NULL;
            const char *limit_s;
            rqs_get_rue_string(&rue_string, rule, user, project, host, queue, pe);
            sge_dstring_sprintf(&limit_name, "%s=%s", sge_dstring_get_string(&rule_name), sge_dstring_get_string(&rue_string));
            limit_s = sge_dstring_get_string(&limit_name);

            /* reuse earlier result */
            if ((rql=lGetElemStr(a->limit_list, RQL_name, limit_s))) {
              u_long32 tagged4schedule = lGetUlong(rql, RQL_tagged4schedule); 
               result = (dispatch_t)lGetInt(rql, RQL_result);
               tslots = MIN(tslots, lGetInt(rql, RQL_slots));
               tslots_qend = MIN(tslots_qend, lGetInt(rql, RQL_slots_qend));

               lSetUlong(qep, QU_tagged4schedule, MIN(tagged4schedule, lGetUlong(qep, QU_tagged4schedule)));

               DPRINTF(("parallel_rqs_slots_by_time(%s@%s) result %d slots %d slots_qend %d for "SFQ" (cache)\n",
                     queue, host, result, tslots, tslots_qend, limit_s));
            } else {
               int ttslots = INT_MAX;
               int ttslots_qend = INT_MAX;
               
               u_long32 tagged_for_schedule_old = lGetUlong(qep, QU_tagged4schedule);  /* default value or set in match_static_queue() */
               lSetUlong(qep, QU_tagged4schedule, 2);
               
               for_each(limit, lGetList(rule, RQR_limit)) {
                  const char *limit_name = lGetString(limit, RQRL_name);

                  lListElem *raw_centry = centry_list_locate(a->centry_list, limit_name);
                  lList *job_centry_list = lGetList(a->job, JB_hard_resource_list);
                  lListElem *job_centry = centry_list_locate(job_centry_list, limit_name);
                  if (raw_centry == NULL) {
                     DPRINTF(("ignoring limit %s because not defined", limit_name));
                     continue;
                  } else {
                     DPRINTF(("checking limit %s\n", lGetString(raw_centry, CE_name)));
                  }

                  /* found a rule, now check limit */
                  if (lGetUlong(raw_centry, CE_consumable)) {

                     rqs_get_rue_string(&rue_string, rule, user, project, host, queue, pe);

                     if (rqs_set_dynamical_limit(limit, a->gep, exec_host, a->centry_list)) {
                        int tttslots = INT_MAX;
                        int tttslots_qend = INT_MAX;
                        result = parallel_limit_slots_by_time(a, job_centry_list, &tttslots, &tttslots_qend, raw_centry, limit, &rue_string, qep);
                        ttslots = MIN(ttslots, tttslots);
                        ttslots_qend = MIN(ttslots_qend, tttslots_qend);
                        if (result == DISPATCH_NOT_AT_TIME) {
                           /* can still be interesting for reservation and as slave task for per_job_consumables */ 
                           result = DISPATCH_OK;
                        } else if (result != DISPATCH_OK) {
                           break;
                        }
                     } else {
                        result = DISPATCH_NEVER_CAT;
                        break;
                     }
                  } else if (job_centry != NULL) {
                     char availability_text[2048];

                     lSetString(raw_centry, CE_stringval, lGetString(limit, RQRL_value));
                     if (compare_complexes(1, raw_centry, job_centry, availability_text, false, false) != 1) {
                        result = DISPATCH_NEVER_CAT;
                        break;
                     }
                  }

               }

               DPRINTF(("parallel_rqs_slots_by_time(%s@%s) result %d slots %d slots_qend %d for "SFQ" (fresh)\n",
                     queue, host, result, ttslots, ttslots_qend, limit_s));

               /* store result for reuse */
               rql = lAddElemStr(&(a->limit_list), RQL_name, limit_s, RQL_Type);
               lSetInt(rql, RQL_result, result);
               lSetInt(rql, RQL_slots, ttslots);
               lSetInt(rql, RQL_slots_qend, ttslots_qend);
               lSetUlong(rql, RQL_tagged4schedule, lGetUlong(qep, QU_tagged4schedule)); 
               
               /* reset tagged4schedule if necessary */
               lSetUlong(qep, QU_tagged4schedule, MIN(tagged_for_schedule_old, lGetUlong(qep, QU_tagged4schedule)));

               tslots = MIN(tslots, ttslots);
               tslots_qend = MIN(tslots_qend, ttslots_qend);
            }

            if (result != DISPATCH_OK || (tslots == 0 && ( a->is_reservation || !a->care_reservation || tslots_qend == 0))) {
               DPRINTF(("RQS PARALLEL SORT OUT\n"));
               schedd_mes_add(a->monitor_alpp, a->monitor_next_run, a->job_id,
                              SCHEDD_INFO_CANNOTRUNRQSGLOBAL_SS,
                     sge_dstring_get_string(&rue_string), sge_dstring_get_string(&rule_name));
               rqs_exceeded_sort_out_par(a, rule, &rule_name, queue, host);
            }

            if (result != DISPATCH_OK || tslots == 0) {
               break;
            }
         }
      }
      sge_dstring_free(&rue_string);
      sge_dstring_free(&rule_name);
      sge_dstring_free(&limit_name);
   }

   *slots = tslots;
   *slots_qend = tslots_qend;

   DPRINTF(("parallel_rqs_slots_by_time(%s@%s) finalresult %d slots %d slots_qend %d\n", 
         queue, host, result, *slots, *slots_qend));

   DRETURN(result);
}

/****** sge_resource_quota_schedd/rqs_limitation_reached() *********************
*  NAME
*     rqs_limitation_reached() -- is the limitation reached for a queue instance
*
*  SYNOPSIS
*     static bool rqs_limitation_reached(sge_assignment_t *a, lListElem *rule, 
*     const char* host, const char* queue) 
*
*  FUNCTION
*     The function verifies no limitation is reached for the specific job request
*     and queue instance
*
*  INPUTS
*     sge_assignment_t *a    - job info structure
*     const lListElem *rule        - resource quota rule (RQR_Type)
*     const char* host       - host name
*     const char* queue      - queue name
*    u_long32 *start         - start time of job
*
*  RESULT
*     static dispatch_t - DISPATCH_OK job can be scheduled
*                         DISPATCH_NEVER_CAT no jobs of this category will be scheduled
*                         DISPATCH_NOT_AT_TIME job can be scheduled later
*                         DISPATCH_MISSING_ATTR rule does not match requested attributes
*
*  NOTES
*     MT-NOTE: rqs_limitation_reached() is not MT safe 
*
*******************************************************************************/
static dispatch_t rqs_limitation_reached(sge_assignment_t *a, const lListElem *rule, const char* host, const char* queue, u_long32 *start) 
{
   dispatch_t ret = DISPATCH_MISSING_ATTR;
   lList *limit_list = NULL;
   lListElem * limit = NULL;
   static lListElem *implicit_slots_request = NULL;
   lListElem *exec_host = host_list_locate(a->host_list, host);
   dstring rue_name = DSTRING_INIT;
   dstring reason = DSTRING_INIT;

   DENTER(TOP_LAYER, "rqs_limitation_reached");

    if (implicit_slots_request == NULL) {
      implicit_slots_request = lCreateElem(CE_Type);
      lSetString(implicit_slots_request, CE_name, SGE_ATTR_SLOTS);
      lSetString(implicit_slots_request, CE_stringval, "1");
      lSetDouble(implicit_slots_request, CE_doubleval, 1);
   }

   limit_list = lGetList(rule, RQR_limit);
   for_each(limit, limit_list) {
      bool       is_forced = false;
      lList      *job_centry_list = NULL;
      lListElem  *job_centry = NULL;
      const char *limit_name = lGetString(limit, RQRL_name);
      lListElem  *raw_centry = centry_list_locate(a->centry_list, limit_name);

      if (raw_centry == NULL) {
         DPRINTF(("ignoring limit %s because not defined", limit_name));
         continue;
      } else {
         DPRINTF(("checking limit %s\n", lGetString(raw_centry, CE_name)));
      }

      is_forced = lGetUlong(raw_centry, CE_requestable) == REQU_FORCED ? true : false;
      job_centry_list = lGetList(a->job, JB_hard_resource_list);
      job_centry = centry_list_locate(job_centry_list, limit_name);

      /* check for implicit slot and default request */
      if (job_centry == NULL) {
         if (strcmp(lGetString(raw_centry, CE_name), SGE_ATTR_SLOTS) == 0) {
            job_centry = implicit_slots_request;
         } else if (lGetString(raw_centry, CE_default) != NULL && lGetUlong(raw_centry, CE_consumable)) {
            double request;
            parse_ulong_val(&request, NULL, lGetUlong(raw_centry, CE_valtype), lGetString(raw_centry, CE_default), NULL, 0);

            /* default requests with zero value are ignored */
            if (request == 0.0 && lGetUlong(raw_centry, CE_relop) != CMPLXEXCL_OP) {
               continue;
            }
            lSetString(raw_centry, CE_stringval, lGetString(raw_centry, CE_default));
            lSetDouble(raw_centry, CE_doubleval, request);
            job_centry = raw_centry; 
            DPRINTF(("using default request for %s!\n", lGetString(raw_centry, CE_name)));
         } else if (is_forced == true) {
            schedd_mes_add(a->monitor_alpp, a->monitor_next_run, a->job_id,
                           SCHEDD_INFO_NOTREQFORCEDRES); 
            ret = DISPATCH_NEVER_CAT;
            break;
         } else {
            /* ignoring because centry was not requested and is no consumable */
            DPRINTF(("complex not requested!\n"));
            continue;
         }
      }

      {
         lList *tmp_centry_list = lCreateList("", CE_Type);
         lList *tmp_rue_list = lCreateList("", RUE_Type);
         lListElem *tmp_centry_elem = NULL;
         lListElem *tmp_rue_elem = NULL;
            
         if (rqs_set_dynamical_limit(limit, a->gep, exec_host, a->centry_list)) {
            lList *rue_list = lGetList(limit, RQRL_usage);
            u_long32 tmp_time = a->start;

            /* create tmp_centry_list */
            tmp_centry_elem = lCopyElem(raw_centry);
            lSetString(tmp_centry_elem, CE_stringval, lGetString(limit, RQRL_value));
            lSetDouble(tmp_centry_elem, CE_doubleval, lGetDouble(limit, RQRL_dvalue));
            lAppendElem(tmp_centry_list, tmp_centry_elem);

            /* create tmp_rue_list */
            rqs_get_rue_string(&rue_name, rule, a->user, a->project, host, queue, NULL);
            tmp_rue_elem = lCopyElem(lGetElemStr(rue_list, RUE_name, sge_dstring_get_string(&rue_name)));
            if (tmp_rue_elem == NULL) {
               tmp_rue_elem = lCreateElem(RUE_Type);
            }
            lSetString(tmp_rue_elem, RUE_name, limit_name);
            lAppendElem(tmp_rue_list, tmp_rue_elem);
           
            sge_dstring_clear(&reason);
            ret = ri_time_by_slots(a, job_centry, NULL, tmp_centry_list,  tmp_rue_list,
                                       NULL, &reason, false, 1, DOMINANT_LAYER_RQS, 0.0, &tmp_time,
                                       SGE_RQS_NAME);
            if (ret != DISPATCH_OK) {
               DPRINTF(("denied because: %s\n", sge_dstring_get_string(&reason)));
               lFreeList(&tmp_rue_list);
               lFreeList(&tmp_centry_list);
               break;
            }

            if (a->is_reservation && ret == DISPATCH_OK) {
               *start = tmp_time;
            }

            lFreeList(&tmp_rue_list);
            lFreeList(&tmp_centry_list);
         }
      }
   }

   sge_dstring_free(&reason);
   sge_dstring_free(&rue_name);

   DRETURN(ret);
}

/****** sge_resource_quota_schedd/rqs_by_slots() ***********************************
*  NAME
*     rqs_by_slots() -- Check queue instance suitability due to RQS
*
*  SYNOPSIS
*     dispatch_t rqs_by_slots(sge_assignment_t *a, const char *queue, 
*     const char *host, u_long32 *tt_rqs_all, bool *is_global, 
*     dstring *rue_string, dstring *limit_name, dstring *rule_name) 
*
*  FUNCTION
*     Checks (or determines earliest time) queue instance suitability 
*     according to resource quota set limits. 
*     
*     For performance reasons RQS verification results are cached in 
*     a->limit_list. In addition unsuited queues and hosts are collected
*     in a->skip_cqueue_list and a->skip_host_list so that ruling out 
*     chunks of queue instance becomes quite cheap.
*
*  INPUTS
*     sge_assignment_t *a  - assignment
*     const char *queue    - cluster queue name
*     const char *host     - host name
*     u_long32 *tt_rqs_all - returns earliest time over all resource quotas
*     bool *is_global      - returns true if result is valid for any other queue
*     dstring *rue_string  - caller maintained buffer
*     dstring *limit_name  - caller maintained buffer
*     dstring *rule_name   - caller maintained buffer
*     u_long32 tt_best     - time of best solution found so far
*
*  RESULT
*     static dispatch_t - usual return values
*
*  NOTES
*     MT-NOTE: rqs_by_slots() is MT safe 
*******************************************************************************/
dispatch_t rqs_by_slots(sge_assignment_t *a, const char *queue, const char *host, 
  u_long32 *tt_rqs_all, bool *is_global, dstring *rue_string, dstring *limit_name, dstring *rule_name, u_long32 tt_best)
{
   lListElem *rqs;
   dispatch_t result = DISPATCH_OK;

   DENTER(TOP_LAYER, "rqs_by_slots");

   *is_global = false;

   if (a->pi && lGetNumberOfElem(a->rqs_list) > 0) {
      a->pi->seq_rqs++;
   }

   for_each(rqs, a->rqs_list) {
      u_long32 tt_rqs = a->start;
      const char *user = a->user;
      const char *group = a->group;
      const char *project = a->project;
      const lListElem *rule;

      if (!lGetBool(rqs, RQS_enabled)) {
         continue;
      }

      sge_dstring_clear(rule_name);
      rule = rqs_get_matching_rule(rqs, user, group, project, NULL, host, queue, a->acl_list, a->hgrp_list, rule_name);
      if (rule != NULL) {
         const char *limit;
         lListElem *rql;

         /* need unique identifier for cache */
         rqs_get_rue_string(rue_string, rule, user, project, host, queue, NULL);
         sge_dstring_sprintf(limit_name, "%s=%s", sge_dstring_get_string(rule_name), sge_dstring_get_string(rue_string));
         limit = sge_dstring_get_string(limit_name);

         /* check limit or reuse earlier results */
         if ((rql=lGetElemStr(a->limit_list, RQL_name, limit))) {
            tt_rqs = lGetUlong(rql, RQL_time);
            result = (dispatch_t)lGetInt(rql, RQL_result);
         } else {
            /* Check booked usage */
            result = rqs_limitation_reached(a, rule, host, queue, &tt_rqs);

            rql = lAddElemStr(&(a->limit_list), RQL_name, limit, RQL_Type);
            lSetInt(rql, RQL_result, result);
            lSetUlong(rql, RQL_time, tt_rqs);
            /* init with same value as QU_tagged4schedule */
            lSetUlong(rql, RQL_tagged4schedule, 2);

            if (result != DISPATCH_OK && result != DISPATCH_MISSING_ATTR) {
               schedd_mes_add(a->monitor_alpp, a->monitor_next_run, a->job_id,
                              SCHEDD_INFO_CANNOTRUNRQSGLOBAL_SS, 
                     sge_dstring_get_string(rue_string), sge_dstring_get_string(rule_name));
               if (rqs_exceeded_sort_out(a, rule, rule_name, queue, host)) {
                  *is_global = true;
               }
            }
         }

         if (result == DISPATCH_MISSING_ATTR) {
            result = DISPATCH_OK;
            continue;
         }
         if (result != DISPATCH_OK)
            break;

         if (a->is_reservation && tt_rqs >= tt_best) {
            /* no need to further investigate these ones */
            if (rqs_exceeded_sort_out(a, rule, rule_name, queue, host))
               *is_global = true;
         }

         *tt_rqs_all = MAX(*tt_rqs_all, tt_rqs);
      }
   }

   if (!rqs) 
      result = DISPATCH_OK;

   if (result == DISPATCH_OK || result == DISPATCH_MISSING_ATTR) {
      DPRINTF(("rqs_by_slots(%s@%s) returns <at specified time> "sge_u32"\n", queue, host, *tt_rqs_all));
   } else {
      DPRINTF(("rqs_by_slots(%s@%s) returns <later> "sge_u32" (%s)\n",
               queue, host, *tt_rqs_all,
               *is_global?"global":"not global"));
   }

   DRETURN(result);
}
