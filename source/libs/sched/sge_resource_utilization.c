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

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_hostname.h"
#include "uti/sge_time.h"

#include "cull/cull.h"

#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_object.h"

#include "sge.h"
#include "sge_select_queue.h"
#include "sge_resource_utilization.h"
#include "sge_serf.h"
#include "debit.h"
#include "sge_job_schedd.h"

#include "sge_resource_utilization_RUE_L.h"
#include "sge_resource_utilization_RDE_L.h"

#include "msg_common.h"
#include "msg_qmaster.h"
#include "msg_schedd.h"

static void utilization_normalize(lList *diagram);
static u_long32 utilization_endtime(u_long32 start, u_long32 duration);

static void utilization_find_time_or_prevstart_or_prev(lList *diagram, 
      u_long32 time, lListElem **hit, lListElem **before);

static int 
rqs_add_job_utilization(lListElem *jep, u_long32 task_id, const char *type, 
                        lListElem *rule, dstring rue_name, lList *centry_list,
                        int slots, const char *obj_name, u_long32 start_time,
                        u_long32 end_time, bool is_master_task);

static void add_calendar_to_schedule(lList *queue_list, u_long32 now);

static void set_utilization(lList *uti_list, u_long32 from, u_long32 till, double uti);

static lListElem *newResourceElem(u_long32 time, double amount);

#if 0
#define DEBUG_RESOURCE_UTILIZATION
#endif

#ifdef DEBUG_RESOURCE_UTILIZATION
static void utilization_print_all(const lList* pe_list, lList *host_list, const lList *queue_list, const lList *ar_list);
#endif

/****** sge_resource_utilization/utilization_print_to_dstring() ****************
*  NAME
*     utilization_print_to_dstring() -- Print resource utilization to dstring
*
*  SYNOPSIS
*     bool utilization_print_to_dstring(const lListElem *this_elem, dstring 
*     *string) 
*
*  FUNCTION
*     Print resource utlilzation as plain number to dstring.
*
*  INPUTS
*     const lListElem *this_elem - A RUE_Type element
*     dstring *string            - The string 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: utilization_print_to_dstring() is MT safe
*******************************************************************************/
bool utilization_print_to_dstring(const lListElem *this_elem, dstring *string)
{
   if (!this_elem || !string) 
      return true;
   return double_print_to_dstring(lGetDouble(this_elem, RUE_utilized_now), string);
}


#ifdef DEBUG_RESOURCE_UTILIZATION
static void utilization_print_all(const lList* pe_list, lList *host_list, const lList *queue_list, const lList *ar_list)
{
   lListElem *ep, *cr;
   const char *name;

   DENTER(TOP_LAYER, "utilization_print_all");

   /* pe_list */
   for_each (ep, pe_list) {
      name = lGetString(ep, PE_name);
      DPRINTF(("-------------------------------------------\n"));
      DPRINTF(("PARALLEL ENVIRONMENT \"%s\"\n", name));
      for_each (cr, lGetList(ep, PE_resource_utilization)) {
         utilization_print(cr, name);
      }
   }

   /* global */
   if ((ep=host_list_locate(host_list, SGE_GLOBAL_NAME))) {
      DPRINTF(("-------------------------------------------\n"));
      DPRINTF(("GLOBAL HOST RESOURCES\n"));
      for_each (cr, lGetList(ep, EH_resource_utilization)) {
         utilization_print(cr, SGE_GLOBAL_NAME);
      }
   }

   /* exec hosts */
   for_each (ep, host_list) {
      name = lGetHost(ep, EH_name);
      if (sge_hostcmp(name, SGE_GLOBAL_NAME)) {
         DPRINTF(("-------------------------------------------\n"));
         DPRINTF(("EXEC HOST \"%s\"\n", name));
         for_each (cr, lGetList(ep, EH_resource_utilization)) {
            utilization_print(cr, name);
         }
      }
   }

   /* queue instances */
   for_each (ep, queue_list) {
      name = lGetString(ep, QU_full_name);
      if (strcmp(name, SGE_TEMPLATE_NAME)) {
         DPRINTF(("-------------------------------------------\n"));
         DPRINTF(("QUEUE \"%s\"\n", name));
         for_each (cr, lGetList(ep, QU_resource_utilization)) {
            utilization_print(cr, name);
         }
      }
   }
   DPRINTF(("-------------------------------------------\n"));

   /* advance reservations */
   for_each (ep, ar_list) {
      u_long32 ar_id = lGetUlong(ep, AR_id);
      lListElem *queue;

      for_each(queue, lGetList(ep, AR_reserved_queues)) {
         name = lGetString(queue, QU_full_name);
         if (strcmp(name, SGE_TEMPLATE_NAME)) {
            DPRINTF(("-------------------------------------------\n"));
            DPRINTF(("AR "sge_U32CFormat" QUEUE \"%s\"\n", ar_id, name));
            for_each (cr, lGetList(queue, QU_resource_utilization)) {
               utilization_print(cr, name);
            }
         }
      }
   }
   DPRINTF(("-------------------------------------------\n"));
   
   DRETURN_VOID;
}
#endif

void utilization_print(const lListElem *cr, const char *object_name) 
{ 
   lListElem *rde;
   DENTER(TOP_LAYER, "utilization_print");

   DPRINTF(("resource utilization: %s \"%s\" %f utilized now\n", 
         object_name?object_name:"<unknown_object>", lGetString(cr, RUE_name),
            lGetDouble(cr, RUE_utilized_now)));
   for_each (rde, lGetList(cr, RUE_utilized)) {
      DPRINTF(("\t"sge_U32CFormat"  %f\n", lGetUlong(rde, RDE_time), lGetDouble(rde, RDE_amount))); 
   }
   DPRINTF(("resource utilization: %s \"%s\" %f utilized now non-exclusive\n", 
         object_name?object_name:"<unknown_object>", lGetString(cr, RUE_name),
            lGetDouble(cr, RUE_utilized_now_nonexclusive)));
   for_each (rde, lGetList(cr, RUE_utilized_nonexclusive)) {
      DPRINTF(("\t"sge_U32CFormat"  %f\n", lGetUlong(rde, RDE_time), lGetDouble(rde, RDE_amount))); 
   }

   DRETURN_VOID;
}

static u_long32 utilization_endtime(u_long32 start, u_long32 duration)
{
   u_long32 end_time;

   DENTER(BASIS_LAYER, "utilization_endtime");

   if (((double)start + (double)duration) < ((double)U_LONG32_MAX)) {
      end_time = start + duration;
   } else {
      end_time = U_LONG32_MAX;
   }

   DRETURN(end_time);
}

/****** sge_resource_utilization/utilization_add() *****************************
*  NAME
*     utilization_add() -- Debit a jobs resource utilization
*
*  SYNOPSIS
*     int utilization_add(lListElem *cr, u_long32 start_time, u_long32 
*     duration, double utilization, u_long32 job_id, u_long32 ja_taskid, 
*     u_long32 level, const char *object_name, const char *type) 
*
*  FUNCTION
*     A jobs resource utilization is debited into the resource 
*     utilization diagram at the given time for the given duration.
*
*  INPUTS
*     lListElem *cr           - Resource utilization entry (RUE_Type)
*     u_long32 start_time     - Start time of utilization
*     u_long32 duration       - Duration
*     double utilization      - Amount
*     u_long32 job_id         - Job id 
*     u_long32 ja_taskid      - Task id
*     u_long32 level          - *_TAG
*     const char *object_name - The objects name
*     const char *type        - String denoting type of utilization entry.
*     bool is_job             - reserve for job or for advance reservation
*     bool implicit_non_exclusive - add implicit entry for non-exclusive jobs
*                                   requesting a exclusive centry
*
*  RESULT
*     int - 0 on success
*
*  NOTES
*     MT-NOTE: utilization_add() is not MT safe 
*******************************************************************************/
int utilization_add(lListElem *cr, u_long32 start_time, u_long32 duration, double utilization, 
                     u_long32 job_id, u_long32 ja_taskid, u_long32 level, const char *object_name,
                     const char *type, bool for_job, bool implicit_non_exclusive) 
{
   lList *resource_diagram;
   lListElem *this, *prev, *start, *end;
   const char *name = lGetString(cr, RUE_name);
   char level_char = CENTRY_LEVEL_TO_CHAR(level);
   u_long32 end_time;
   int nm;
   double util_prev;
   
   DENTER(TOP_LAYER, "utilization_add");

   if (implicit_non_exclusive) {
      nm = RUE_utilized_nonexclusive;
   } else {
      nm = RUE_utilized;
   }
   resource_diagram=lGetList(cr, nm);

   /* A reservation is only neccessary in one of the following cases:
      - for_job is true (this means no advance reservation request) 
      - reservation is enabled and job duration not zero 
      - queue is already reserved by an advance reservation (resource_diagram != NULL)
   */
   if (for_job && (sconf_get_max_reservations() == 0 || duration == 0)
      && resource_diagram == NULL) /* AR queues have a resource diagram and we must reflect changes for this queues */
   { 
      DPRINTF(("max reservations reached or duration is 0\n"));

      DRETURN(0);
   }

   end_time = utilization_endtime(start_time, duration);

   serf_record_entry(job_id, ja_taskid, (type!=NULL)?type:"<unknown>", start_time, end_time, 
         level_char, object_name, name, utilization);

   /* ensure resource diagram is initialized */
   if (resource_diagram == NULL) {
      resource_diagram = lCreateList(name, RDE_Type);
      lSetList(cr, nm, resource_diagram);
   }

   utilization_find_time_or_prevstart_or_prev(resource_diagram, 
         start_time, &start, &prev);

   if (start) {
      /* if we found one add the utilization amount to it */
      lAddDouble(start, RDE_amount, utilization);
   } else {
      /* otherwise insert a new one after the element before resp. at the list begin */
      if (prev)
         util_prev = lGetDouble(prev, RDE_amount);
      else 
         util_prev = 0;
      start = lCreateElem(RDE_Type);
      lSetUlong(start, RDE_time, start_time);
      lSetDouble(start, RDE_amount, utilization + util_prev);
      lInsertElem(resource_diagram, prev, start);
   }

   end = NULL;
   prev = start;
   this = lNext(start);

   /* find existing end element or the element before */
   while (this) {
      if (end_time == lGetUlong(this, RDE_time)) {
         end = this;
         break;
      }
      if (end_time < lGetUlong(this, RDE_time)) {
         break;
      }
      /* increment number of elements in-between */
      lAddDouble(this, RDE_amount, utilization);
      prev = this;
      this = lNext(this);
   }

   if (!end) {
      util_prev = lGetDouble(prev, RDE_amount);
      end = lCreateElem(RDE_Type);
      lSetUlong(end, RDE_time, end_time);
      lSetDouble(end, RDE_amount, util_prev - utilization);
      lInsertElem(resource_diagram, prev, end);
   }

#if 1
   utilization_print(cr, "pe_slots");
   printf("this was before utilization_normalize()\n");
#endif

   utilization_normalize(resource_diagram);
   DRETURN(0);
}

/* 
   Find element with specified time or the element before it 

   If the element exists it is returned in 'hit'. Otherwise
   the element before it is returned or NULL if no such exists.

*/
static void utilization_find_time_or_prevstart_or_prev(lList *diagram, u_long32 time, lListElem **hit, lListElem **before)
{ 
   lListElem *start, *this, *prev;

   start = NULL;
   this = lFirst(diagram); 
   prev = NULL;

   while (this) {
      if (time == lGetUlong(this, RDE_time)) {
         start = this;
         break;
      }
      if (time < lGetUlong(this, RDE_time)) {
         break;
      }
      prev = this;
      this = lNext(this);
   }

   *hit    = start;
   *before = prev;
}

/* normalize utilization diagram 

   Note: Diagram doesn't need to vanish (NULL list) 
         as long as utilization is added only
*/
static void utilization_normalize(lList *diagram)
{
   lListElem *this, *next;
   double util_prev;

   this = lFirst(diagram);

   while (this && lGetDouble(this, RDE_amount) == 0.0) {
      lRemoveElem(diagram, &this);
      this = lFirst(diagram);
   }

   if (this == NULL) {
      return;
   }
   
   if ((next = lNext(this)) == NULL) {
      return;
   }

   util_prev = lGetDouble(this, RDE_amount);

   while ((this=next)) {
      next = lNext(this);
      if (util_prev == lGetDouble(this, RDE_amount))
         lRemoveElem(diagram, &this);
      else
         util_prev = lGetDouble(this, RDE_amount);
   }

   return;
}

/****** sge_resource_utilization/utilization_queue_end() ***********************
*  NAME
*     utilization_queue_end() -- Determine utilization at queue end time
*
*  SYNOPSIS
*     double utilization_queue_end(const lListElem *cr) 
*
*  FUNCTION
*     Determine utilization at queue end time. Jobs that last until 
*     ever can cause a non-zero utilization.
*
*  INPUTS
*     const lListElem *cr - Resource utilization entry (RUE_utilized)
*     bool for_excl_request - For exclusive request
*
*  RESULT
*     double - queue end utilization
*
*  NOTES
*     MT-NOTE: utilization_queue_end() is MT safe 
*******************************************************************************/
double utilization_queue_end(const lListElem *cr, bool for_excl_request)
{
   const lListElem *ep = lLast(lGetList(cr, RUE_utilized));
   double max = 0.0;

   DENTER(TOP_LAYER, "utilization_queue_end");

#if 1
   utilization_print(cr, "the object");
#endif

   if (ep) {
      if (lGetUlong(ep, RDE_time) != U_LONG32_MAX) {
         max = lGetDouble(ep, RDE_amount);
      } else {
         max = lGetDouble(lPrev(ep), RDE_amount);
      }
   }

   if (for_excl_request) {
      double max_nonexclusive;
      ep = lLast(lGetList(cr, RUE_utilized_nonexclusive));
      if (ep) {
         if (lGetUlong(ep, RDE_time) != U_LONG32_MAX) {
            max_nonexclusive = lGetDouble(ep, RDE_amount);
         } else {
            max_nonexclusive = lGetDouble(lPrev(ep), RDE_amount);
         }
         max = MAX(max, max_nonexclusive);
      }
   }

   DPRINTF(("returning %f\n", max));
   DRETURN(max);
}


/****** sge_resource_utilization/utilization_max() *****************************
*  NAME
*     utilization_max() -- Determine max utilization within timeframe
*
*  SYNOPSIS
*     double utilization_max(const lListElem *cr, u_long32 start_time, u_long32 
*     duration) 
*
*  FUNCTION
*     Determines the maximum utilization at the given timeframe.
*
*  INPUTS
*     const lListElem *cr - Resource utilization entry (RUE_utilized)
*     u_long32 start_time - Start time of the timeframe
*     u_long32 duration   - Duration of timeframe
*     bool for_excl_request - For exclusive request
*
*  RESULT
*     double - Maximum utilization
*
*  NOTES
*     MT-NOTE: utilization_max() is MT safe 
*******************************************************************************/
double utilization_max(const lListElem *cr, u_long32 start_time, u_long32 duration, bool for_excl_request)
{
   const lListElem *rde;
   lListElem *start, *prev;
   double max = 0.0;
   u_long32 end_time = utilization_endtime(start_time, duration);

   DENTER(TOP_LAYER, "utilization_max");

   /* someone is asking for the current utilization */
   if (start_time == DISPATCH_TIME_NOW) {
      max = lGetDouble(cr, RUE_utilized_now);

      if (for_excl_request) {
         max = MAX(lGetDouble(cr, RUE_utilized_now_nonexclusive), max);
      }

      DPRINTF(("returning(1) %f\n", max));
      DRETURN(max);
   }

   /* someone is asking for queue end utilization */
   if (start_time == -1) {
      DRETURN(utilization_queue_end(cr, for_excl_request));
   }
   
#if 1
   utilization_print(cr, "the object");
#endif

   utilization_find_time_or_prevstart_or_prev(lGetList(cr, RUE_utilized), start_time, 
         &start, &prev);

   if (start) {
      max = lGetDouble(start, RDE_amount);
      rde = lNext(start);
   } else {
      if (prev) {
         max = lGetDouble(prev, RDE_amount);
         rde = lNext(prev);
      } else {
         rde = lFirst(lGetList(cr, RUE_utilized));
      }
   }

   /* now watch out for the maximum before end time */ 
   while (rde && end_time > lGetUlong(rde, RDE_time)) {
      max = MAX(max, lGetDouble(rde, RDE_amount));
      rde = lNext(rde);
   }
   
   if (for_excl_request) {
      double max_nonexclusive = 0.0;
      utilization_find_time_or_prevstart_or_prev(lGetList(cr, RUE_utilized_nonexclusive), start_time, 
            &start, &prev);

      if (start) {
         max_nonexclusive = lGetDouble(start, RDE_amount);
         rde = lNext(start);
      } else {
         if (prev) {
            max_nonexclusive = lGetDouble(prev, RDE_amount);
            rde = lNext(prev);
         } else {
            rde = lFirst(lGetList(cr, RUE_utilized_nonexclusive));
         }
      }

      /* now watch out for the maximum before end time */ 
      while (rde && end_time > lGetUlong(rde, RDE_time)) {
         max_nonexclusive = MAX(max_nonexclusive, lGetDouble(rde, RDE_amount));
         rde = lNext(rde);
      }
      max = MAX(max, max_nonexclusive);
   }

   DPRINTF(("returning(2) %f\n", max));
   DRETURN(max); 
}

/****** sge_resource_utilization/utilization_below() ***************************
*  NAME
*     utilization_below() -- Determine earliest time util is below max_util
*
*  SYNOPSIS
*     u_long32 utilization_below(const lListElem *cr, double max_util, const 
*     char *object_name) 
*
*  FUNCTION
*     Determine and return earliest time utilization is below max_util.
*
*  INPUTS
*     const lListElem *cr     - Resource utilization entry (RUE_utilized)
*     double max_util         - The maximum utilization we're asking
*     const char *object_name - Name of the queue/host/global for monitoring 
*                               purposes.
*     bool for_excl_request   - match for exclusive request
*
*  RESULT
*     u_long32 - The earliest time or DISPATCH_TIME_NOW.
*
*  NOTES
*     MT-NOTE: utilization_below() is MT safe 
*******************************************************************************/
u_long32 utilization_below(const lListElem *cr, double max_util, const char *object_name, bool for_excl_request)
{
   const lListElem *rde;
   double util = 0;
   u_long32 when = DISPATCH_TIME_NOW;

   DENTER(TOP_LAYER, "utilization_below");

#if 0
   utilization_print(cr, object_name);
#endif

   /* search backward starting at the diagrams end */
   for_each_rev (rde, lGetList(cr, RUE_utilized)) {
      util = lGetDouble(rde, RDE_amount);
      if (util <= max_util) {
         lListElem *p = lPrev(rde);
         if (p && lGetDouble(p, RDE_amount) > max_util) {
            when = lGetUlong(rde, RDE_time);
            break;
         }
      }
   }
   if (for_excl_request) {
      u_long32 when_nonexclusive = DISPATCH_TIME_NOW;
      for_each_rev (rde, lGetList(cr, RUE_utilized_nonexclusive)) {
         util = lGetDouble(rde, RDE_amount);
         if (util <= max_util) {
            lListElem *p = lPrev(rde);
            if (p && lGetDouble(p, RDE_amount) > max_util) {
               when_nonexclusive = lGetUlong(rde, RDE_time);
               break;
            }
         }
      }
      when = MAX(when, when_nonexclusive);
   }

   if (when == DISPATCH_TIME_NOW) {
      DPRINTF(("no utilization\n"));
   } else {
      DPRINTF(("utilization below %f (%f) starting at "sge_U32CFormat"\n", 
         max_util, util, when));
   }

   DRETURN(when); 
}


/****** sge_resource_utilization/add_job_utilization() *************************
*  NAME
*     add_job_utilization() -- Debit assignments' utilization from all schedules
*
*  SYNOPSIS
*     int add_job_utilization(const sge_assignment_t *a, const char *type) 
*
*  FUNCTION
*     The resouce utilization of an assignment is debited into the schedules 
*     of global, host and queue instance resource containers and limitation
*     rule sets. For parallel jobs debiting is made done from the parallel
*     environment schedule.
*
*  INPUTS
*     const sge_assignment_t *a - The assignment
*     const char *type          - A string that is used to monitor assignment
*                                 type
*     bool for_job_scheduling   - utilize for job or for advance reservation
*
*  RESULT
*     int - 
*
*  NOTES
*     MT-NOTE: add_job_utilization() is MT safe 
*******************************************************************************/
int add_job_utilization(const sge_assignment_t *a, const char *type, bool for_job_scheduling)
{
   lListElem *gel, *qep, *hep; 
   u_long32 ar_id = lGetUlong(a->job, JB_ar);

   DENTER(TOP_LAYER, "add_job_utilization");

   if (ar_id == 0) {
      bool is_master_task = true;
      /* debit non-AR-job */

      dstring rue_name = DSTRING_INIT;
      /* parallel environment  */
      if (a->pe) {
         utilization_add(lFirst(lGetList(a->pe, PE_resource_utilization)), a->start, a->duration, a->slots,
               a->job_id, a->ja_task_id, PE_TAG, lGetString(a->pe, PE_name), type, for_job_scheduling, false);
      }

      /* global */
      rc_add_job_utilization(a->job, a->ja_task_id, type, a->gep, a->centry_list, a->slots, 
            EH_consumable_config_list, EH_resource_utilization, SGE_GLOBAL_NAME, 
              a->start, a->duration, GLOBAL_TAG, for_job_scheduling, true);  

      for_each(gel, a->gdil) {
         int slots = lGetUlong(gel, JG_slots); 
         const char *eh_name = lGetHost(gel, JG_qhostname);
         const char *qname = lGetString(gel, JG_qname);
         const char* pe = (a->pe)?lGetString(a->pe, PE_name):NULL;
         const char *queue_instance = lGetString(gel, JG_qname);
         char *queue = cqueue_get_name_from_qinstance(queue_instance);

         lListElem *rqs = NULL;

         /* hosts */
         if ((hep = host_list_locate(a->host_list, eh_name)) != NULL) {
            rc_add_job_utilization(a->job, a->ja_task_id, type, hep, a->centry_list, slots,
                     EH_consumable_config_list, EH_resource_utilization, eh_name, a->start, 
                     a->duration, HOST_TAG, for_job_scheduling, is_master_task);
         }

         /* queues */
         if ((qep = qinstance_list_locate2(a->queue_list, qname)) != NULL) {
            /* 
             * The NULL case happens in case of queues that were sorted out b/c they 
             * are unknown, in some suspend state or in calendar disable state. As long 
             * as we do not intend to schedule future resource utilization for those 
             * queues it's valid to simply ignore resource utilizations decided in former 
             * schedule runs: running/suspneded/migrating jobs.
             * 
             */
            rc_add_job_utilization(a->job, a->ja_task_id, type, qep, a->centry_list, slots,
                     QU_consumable_config_list, QU_resource_utilization, qname, a->start, 
                     a->duration, QUEUE_TAG, for_job_scheduling, is_master_task);
         }

         /* resource quotas */
         for_each(rqs, a->rqs_list) {
            lListElem *rule = NULL;

            if (!lGetBool(rqs, RQS_enabled)) {
               continue;
            }

            rule = rqs_get_matching_rule(rqs, a->user, a->group, a->project, pe, eh_name, queue, a->acl_list,
                                          a->hgrp_list, NULL);
            if (rule != NULL) {

               rqs_get_rue_string(&rue_name, rule, a->user, a->project,
                                   eh_name, queue, pe);

               rqs_add_job_utilization(a->job, a->ja_task_id, type, rule, rue_name,
                                        a->centry_list, slots, lGetString(rqs, RQS_name),
                                        a->start, a->duration, is_master_task);
            }
         }

         sge_free(&queue);
         is_master_task = false;
      }

      sge_dstring_free(&rue_name);
   } else {
      bool is_master_task = true;
      /* debit AR-job */
      for_each(gel, a->gdil) {
         lListElem *ar = NULL;
         int slots = lGetUlong(gel, JG_slots);
         const char *qname = lGetString(gel, JG_qname);
         
         if ((ar = lGetElemUlong(a->ar_list, AR_id, ar_id)) != NULL) {
            if ((qep = lGetSubStr(ar, QU_full_name, qname, AR_reserved_queues)) != NULL) {
               rc_add_job_utilization(a->job, a->ja_task_id, type, qep, a->centry_list, slots,
                     QU_consumable_config_list, QU_resource_utilization, qname, a->start, 
                     a->duration, QUEUE_TAG, for_job_scheduling, is_master_task);
            }
         } 
         is_master_task = false;
      }
   }

   DRETURN(0);
}

int rc_add_job_utilization(lListElem *jep, u_long32 task_id, const char *type, 
   lListElem *ep, lList *centry_list, int slots, int config_nm, int actual_nm, 
   const char *obj_name, u_long32 start_time, u_long32 duration, u_long32 tag,
   bool for_job_scheduling, bool is_master_task) 
{
   lListElem *cr, *cr_config, *dcep;
   int mods = 0;

   DENTER(TOP_LAYER, "rc_add_job_utilization");

   if (!ep) {
      ERROR((SGE_EVENT, "rc_add_job_utilization NULL object "
            "(job "sge_u32" obj %s type %s) slots %d ep %p\n", 
            lGetUlong(jep, JB_job_number), obj_name, type, slots, ep));
      DRETURN(0);
   }

   if (!slots) {
      ERROR((SGE_EVENT, "rc_add_job_utilization 0 slot amount "
            "(job "sge_u32" obj %s type %s) slots %d ep %p\n", 
            lGetUlong(jep, JB_job_number), obj_name, type, slots, ep));
      DRETURN(0);
   }

   for_each(cr_config, lGetList(ep, config_nm)) {
      u_long32 consumable;
      const char *name = lGetString(cr_config, CE_name);
      double dval = 0.0;
      int debit_slots = slots;

      /* search default request */  
      if (!(dcep = centry_list_locate(centry_list, name))) {
         ERROR((SGE_EVENT, MSG_ATTRIB_MISSINGATTRIBUTEXINCOMPLEXES_S , name));
         DRETURN(-1);
      } 

      consumable = lGetUlong(dcep, CE_consumable);
      if (consumable == CONSUMABLE_NO) {
         /* no error */
         continue;
      }

      if (consumable == CONSUMABLE_JOB) {
         if (!is_master_task) {
            /* no error, only master_task is debited */
            continue;
         }
         /* it's a job consumable, we don't multiply with slots */
         if (slots > 0) {
            debit_slots = 1;
         } else if (slots < 0) {
            debit_slots = -1;
         }
      }

      /* ensure attribute is in actual list */
      if (!(cr = lGetSubStr(ep, RUE_name, name, actual_nm))) {
         cr = lAddSubStr(ep, RUE_name, name, actual_nm, RUE_Type);
         /* CE_double is implicitly set to zero */
      }
   
      if (job_get_contribution(jep, NULL, name, &dval, dcep) && dval != 0.0) {
         /* update RUE_utilized resource diagram to reflect jobs utilization */
         utilization_add(cr, start_time, duration, debit_slots * dval,
            lGetUlong(jep, JB_job_number), task_id, tag, obj_name, type, for_job_scheduling, false);
         mods++;
      } else if (lGetUlong(dcep, CE_relop) == CMPLXEXCL_OP) {
         dval = 1.0;
         /* update RUE_utilized resource diagram to reflect jobs utilization */
         utilization_add(cr, start_time, duration, debit_slots * dval,
            lGetUlong(jep, JB_job_number), task_id, tag, obj_name, type, for_job_scheduling, true);
         mods++;
      }

   }

   DRETURN(mods);
}

/****** sge_resource_utilization/rqs_add_job_utilization() ********************
*  NAME
*     rqs_add_job_utilization() -- Debit assignment's utilization in a limitation
*                                  rule
*
*  SYNOPSIS
*     static int rqs_add_job_utilization(lListElem *jep, u_long32 task_id, 
*     const char *type, lListElem *rule, dstring rue_name, lList *centry_list, 
*     int slots, const char *obj_name, u_long32 start_time, u_long32 end_time,
*     bool is_master_task) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lListElem *jep       - job element (JB_Type)
*     u_long32 task_id     - task id to debit
*     const char *type     - String denoting type of utilization entry 
*     lListElem *rule      - limitation rule (RQR_Type)
*     dstring rue_name     - rue_name where to debit
*     lList *centry_list   - master centry list (CE_Type)
*     int slots            - slots to debit
*     const char *obj_name - name of the object where to debit
*     u_long32 start_time  - start time of utilization
*     u_long32 end_time    - end time of utilization
*     bool is_master_task  - is this the master task going to be debit
*
*  RESULT
*     static int - number of modified limits
*
*  NOTES
*     MT-NOTE: rqs_add_job_utilization() is MT safe 
*
*  SEE ALSO
*     sge_resource_utilization/rc_add_job_utilization()
*     sge_resource_utilization/add_job_utilization()
*******************************************************************************/
static int 
rqs_add_job_utilization(lListElem *jep, u_long32 task_id, const char *type, 
   lListElem *rule, dstring rue_name, lList *centry_list, int slots, const char *obj_name,
   u_long32 start_time, u_long32 end_time, bool is_master_task)
{
   lList *limit_list;
   lListElem *limit;
   const char *centry_name;
   int mods = 0;

   DENTER(TOP_LAYER, "rqs_add_job_utilization");

   if (jep != NULL) {
      limit_list = lGetList(rule, RQR_limit);

      for_each(limit, limit_list) {
         u_long32 consumable;
         lListElem *raw_centry;
         lListElem *rue_elem;
         double dval = 0.0;
         int debit_slots = slots;

         centry_name = lGetString(limit, RQRL_name);
         
         if (!(raw_centry = centry_list_locate(centry_list, centry_name))) {
            /* ignoring not defined centry */
            continue;
         }

         consumable = lGetUlong(raw_centry, CE_consumable);
         if (consumable == CONSUMABLE_NO) {
            continue;
         }

         if (consumable == CONSUMABLE_JOB) {
            if (!is_master_task) {
               /* no error, only master_task is debited */
               continue;
            }
            /* it's a job consumable, we don't multiply with slots */
            if (slots > 0) {
               debit_slots = 1;
            } else if (slots < 0) {
               debit_slots = -1;
            }
         }

         rue_elem = lGetSubStr(limit, RUE_name, sge_dstring_get_string(&rue_name), RQRL_usage);
         if(rue_elem == NULL) {
            rue_elem = lAddSubStr(limit, RUE_name, sge_dstring_get_string(&rue_name), RQRL_usage, RUE_Type);
            /* RUE_utilized_now is implicitly set to zero */
         }

         if (job_get_contribution(jep, NULL, centry_name, &dval, raw_centry) && dval != 0.0) {
            /* update RUE_utilized resource diagram to reflect jobs utilization */
            utilization_add(rue_elem, start_time, end_time, debit_slots * dval,
               lGetUlong(jep, JB_job_number), task_id, RQS_TAG, obj_name, type, true, false);
            mods++;
         } else if (lGetUlong(raw_centry, CE_relop) == CMPLXEXCL_OP) {
            dval = 1.0;
            utilization_add(rue_elem, start_time, end_time, debit_slots * dval,
               lGetUlong(jep, JB_job_number), task_id, RQS_TAG, obj_name, type, true, true);
            mods++;
         }
      }
   }

   DRETURN(mods);
}

static int 
add_job_list_to_schedule(const lList *job_list, bool suspended, lList *pe_list, 
                         lList *host_list, lList *queue_list, lList *rqs_list,
                         lList *centry_list, lList *acl_list, lList *hgroup_list,
                         lList *ar_list, bool for_job_scheduling, u_long32 now)
{
   lListElem *jep, *ja_task;
   lListElem *gep = host_list_locate(host_list, SGE_GLOBAL_NAME);
   const char *pe_name;
   const char *type;
   u_long32 interval = sconf_get_schedule_interval();

   DENTER(TOP_LAYER, "add_job_list_to_schedule");

   if (suspended) {
      type = SCHEDULING_RECORD_ENTRY_TYPE_SUSPENDED;
   } else {
      type = SCHEDULING_RECORD_ENTRY_TYPE_RUNNING;
   }   

   for_each (jep, job_list) {
      for_each (ja_task, lGetList(jep, JB_ja_tasks)) {  
         sge_assignment_t a = SGE_ASSIGNMENT_INIT;

         assignment_init(&a, jep, ja_task, false);

         a.start = lGetUlong(ja_task, JAT_start_time);

         task_get_duration(&a.duration, ja_task);

         a.duration = duration_add_offset(a.duration, sconf_get_duration_offset());

         /* Prevent jobs that exceed their prospective duration are not reflected 
            in the resource schedules. Note duration enforcement is domain of 
            sge_execd and default_duration is not enforced at all anyways.
            All we can do here is hope the job will be finished in the next interval. */
         if (duration_add_offset(a.start, a.duration) <= now) {
            /* That logging is disabled as it can cause schedd messages file
               be filled up with loggings. There are cases when it can't be 
               considered a misconfiguration if jobs do not complete within the
               time foreseen. If jobs are submitted without -l h_rt limit and 
               aren't cancelled due to default_duration only be in effect */

            if (for_job_scheduling && sconf_get_max_reservations() > 0) {
               WARNING((SGE_EVENT, MSG_SCHEDD_SHOULDHAVEFINISHED_UUU, 
                     sge_u32c(a.job_id), sge_u32c(a.ja_task_id), 
                     sge_u32c(now - a.duration - a.start + 1)));
            }
            a.duration = (now - a.start) + interval;
         }

         a.gdil = lGetList(ja_task, JAT_granted_destin_identifier_list);
         a.slots = nslots_granted(a.gdil, NULL);
         if ((pe_name = lGetString(ja_task, JAT_granted_pe)) && 
             !(a.pe = pe_list_locate(pe_list, pe_name))) {
            ERROR((SGE_EVENT, MSG_OBJ_UNABLE2FINDPE_S, pe_name));
            continue;
         }
         /* no need (so far) for passing ckpt information to debit_scheduled_job() */

         a.host_list = host_list;
         a.queue_list = queue_list;
         a.centry_list = centry_list;
         a.rqs_list = rqs_list;
         a.acl_list = acl_list;
         a.hgrp_list = hgroup_list;
         a.ar_list = ar_list;
         a.gep     = gep;

         DPRINTF(("Adding job "sge_U32CFormat"."sge_U32CFormat" into schedule " "start "
                  sge_U32CFormat" duration "sge_U32CFormat"\n", lGetUlong(jep, JB_job_number), 
                  lGetUlong(ja_task, JAT_task_number), a.start, a.duration));

         /* only update resource utilization schedule  
            RUE_utililized_now is already set through events */
         debit_scheduled_job(&a, NULL, NULL, false, type, for_job_scheduling);
      }
   }

   DRETURN(0);
}

/****** sge_resource_utilization/prepare_resource_schedules() *********************************
*  NAME
*     prepare_resource_schedules() -- Debit non-pending jobs in resource schedule
*
*  SYNOPSIS
*     static void prepare_resource_schedules(const lList *running_jobs, const 
*     lList *suspended_jobs, lList *pe_list, lList *host_list, lList 
*     *queue_list, lList *centry_list, lList *rqs_list) 
*
*  FUNCTION
*     In order to reflect current and future resource utilization of running 
*     and suspended jobs in the schedule we iterate through all jobs and debit
*     resources requested by those jobs.
*
*  INPUTS
*     const lList *running_jobs   - The running ones (JB_Type)
*     const lList *suspended_jobs - The susepnded ones (JB_Type)
*     lList *pe_list              - ??? 
*     lList *host_list            - ??? 
*     lList *queue_list           - ??? 
*     lList *rqs_list             - configured resource quota sets
*     lList *centry_list          - ??? 
*     lList *acl_list             - ??? 
*     lList *hgroup_list          - ??? 
*     lList *prepare_resource_schedules - create schedule for job or advance reservation
*                                         scheduling
*     bool for_job_scheduling     - prepare for job or for advance reservation
*     u_long32 now                - now time of assignment
*
*  NOTES
*     MT-NOTE: prepare_resource_schedules() is not MT safe 
*******************************************************************************/
void prepare_resource_schedules(const lList *running_jobs, const lList *suspended_jobs, 
   lList *pe_list, lList *host_list, lList *queue_list, lList *rqs_list, lList *centry_list,
   lList *acl_list, lList *hgroup_list, lList *ar_list, bool for_job_scheduling, u_long32 now)
{
   DENTER(TOP_LAYER, "prepare_resource_schedules");

   add_job_list_to_schedule(running_jobs, false, pe_list, host_list, queue_list,
                            rqs_list, centry_list, acl_list, hgroup_list,
                            ar_list, for_job_scheduling, now);
   add_job_list_to_schedule(suspended_jobs, true, pe_list, host_list, queue_list,
                            rqs_list, centry_list, acl_list, hgroup_list,
                            ar_list, for_job_scheduling, now);
   add_calendar_to_schedule(queue_list, now); 

#ifdef DEBUG_RESOURCE_UTILIZATION  /* just for information purposes... */
   utilization_print_all(pe_list, host_list, queue_list, ar_list); 
#endif   

   DRETURN_VOID;
}

/****** sge_resource_utilization/add_calendar_to_schedule() ***********************************
*  NAME
*     add_calendar_to_schedule() -- addes the queue calendar to the resource
*                                   schedule
*
*  SYNOPSIS
*     static void add_calendar_to_schedule(lList *queue_list) 
*
*  FUNCTION
*     Adds the queue calendars to the resource schedule. It is using
*     the slot entry for simulating and enabled / disabled calendar.
*
*  INPUTS
*     lList *queue_list - all queues, which can posibly run jobs
*     u_long32 now      - now time of assignment
*
*  NOTES
*     MT-NOTE: add_calendar_to_schedule() is MT safe 
*
*  SEE ALSO
*     sge_resource_utilization/set_utilization
*     scheduler/newResourceElem
*     scheduler/prepare_resource_schedules
*******************************************************************************/
static void 
add_calendar_to_schedule(lList *queue_list, u_long32 now) 
{
   lListElem *queue;

   DENTER(TOP_LAYER, "add_calendar_to_schedule");

   for_each(queue, queue_list) {
      lList *queue_states = lGetList(queue, QU_state_changes);
      u_long32 from       = now;

      if (queue_states != NULL) {
      
         lList *consumable_list = lGetList(queue, QU_consumable_config_list);
         lListElem *slot_elem   = lGetElemStr(consumable_list, CE_name, "slots"); 
         double slot_count      = lGetDouble(slot_elem, CE_doubleval); 

         lList *queue_uti_list = lGetList(queue, QU_resource_utilization);
         lListElem *slot_uti   = lGetElemStr(queue_uti_list, RUE_name, "slots");
         lList *slot_uti_list  = lGetList(slot_uti, RUE_utilized);
         
         lListElem *queue_state = NULL;     

         DPRINTF(("queue: %s time "sge_u32"\n", lGetString(queue, QU_full_name), from));

         if (slot_uti_list == NULL) {
            slot_uti_list = lCreateList("slot_uti", RDE_Type);
            lSetList(slot_uti, RUE_utilized, slot_uti_list);
         }

         for_each(queue_state, queue_states) {
            bool is_full = (lGetUlong(queue_state, CQU_state) != QI_DO_NOTHING)?true:false;
            u_long32 till = lGetUlong(queue_state, CQU_till);
          
            /* check for now, and set it if it is now */
            if (is_full && (from == now)) {
               lSetDouble(slot_uti, RUE_utilized_now, slot_count);
            }
          
            set_utilization(slot_uti_list, from, till, is_full?slot_count:0);
            
            from = till;     
         } /* end for_each */

      }/* end if*/
   }
   
   DRETURN_VOID;
}

/****** sge_resource_utilization/set_utilization() ********************************************
*  NAME
*     set_utilization() -- adds one specific calendar entry to the resource schedule
*
*  SYNOPSIS
*     static void set_utilization(lList *uti_list, u_long32 from, u_long32 
*     till, double uti) 
*
*  FUNCTION
*     This set utilization function is unique for calendars. It removes all other
*     uti settings in the given time interval and replaces it with the given one.
*
*  INPUTS
*     lList *uti_list - the uti list for a specifiy resource and queue
*     u_long32 from   - starting time for this uti
*     u_long32 till   - endtime for this uti.
*     double uti      - utilization (needs to bigger than 1 (schould be max)
*
*  NOTES
*     MT-NOTE: set_utilization() is MT safe 
*
*  SEE ALSO
*     sge_resource_utilization/add_calendar_to_schedule
*     sge_resource_utilization/newResourceElem
*     sge_resource_utilizationscheduler/prepare_resource_schedules
*******************************************************************************/
static void 
set_utilization(lList *uti_list, u_long32 from, u_long32 till, double uti)
{
   DENTER(TOP_LAYER, "set_utilization");

   if (uti > 0) {
      bool is_from_added = false;
      bool is_till_added = false;
      double past_uti = 0;
      lListElem *uti_elem_next = NULL;

      if (till == 0) {
         till = DISPATCH_TIME_QUEUE_END;
      }

      DPRINTF(("queue cal. schedule entry time "sge_u32" till "sge_u32" util: %f\n", from, till, uti));

      uti_elem_next = lFirst(uti_list);
     
      /* search for the starting point */
      while (uti_elem_next != NULL) {
         if (lGetUlong(uti_elem_next, RDE_time) > from) { /*insert before this elem */
            lInsertElem(uti_list, lPrev(uti_elem_next), newResourceElem(from, uti));
            past_uti = lGetDouble(uti_elem_next, RDE_amount);
            is_from_added = true; 
            break;
         }
         else if (lGetUlong(uti_elem_next, RDE_time) == from) { /* modify found elem */
            /* override utilization is maximun */
            past_uti = lGetDouble(uti_elem_next, RDE_amount);
            lSetDouble(uti_elem_next, RDE_amount, uti);
            is_from_added = true;
            break;
         }
         else { /* did not find it, continue */
            uti_elem_next = lNext(uti_elem_next);
         }
      }

      if (is_from_added) { /* searc for the endpoint */
          while (uti_elem_next != NULL) {
            if (lGetUlong(uti_elem_next, RDE_time) > till) { /*insert before this elem */
               lInsertElem(uti_list, lPrev(uti_elem_next), newResourceElem(till, past_uti));
               is_till_added = true; 
               break;
            }
            else if (lGetUlong(uti_elem_next, RDE_time) == till) { /* do not override utilization is maximun */
               is_till_added = true;
               break;
            }
            else { /* did not find it, remove the current elem and continue*/
               lListElem *next = lNext(uti_elem_next);
               past_uti = lGetDouble(uti_elem_next, RDE_amount);
               lRemoveElem(uti_list, &uti_elem_next);
               uti_elem_next = next;
            }
         }
      }
      else {
         lAppendElem(uti_list, newResourceElem(from, uti));
      }

      if (!is_till_added) {
         lAppendElem(uti_list, newResourceElem(till, 0));
      }   
   }

   DRETURN_VOID;
}

/****** sge_resource_utilization/newResourceElem() ********************************************
*  NAME
*     newResourceElem() -- creates new resource schedule entry
*
*  SYNOPSIS
*     static lListElem* newResourceElem(u_long32 time, double amount) 
*
*  FUNCTION
*     creates new resource schedule entry and returns it
*
*  INPUTS
*     u_long32 time - specific time
*     double amount - the utilized amount
*
*  RESULT
*     static lListElem* - new resource schedule entry
*
*  NOTES
*     MT-NOTE: newResourceElem() is MT safe 
*
*  SEE ALSO
*     sge_resource_utilization/add_calendar_to_schedule
*     sge_resource_utilization/set_utilization
*     sge_resource_utilization/prepare_resource_schedules
*******************************************************************************/

static lListElem *newResourceElem(u_long32 time, double amount) 
{
   lListElem *elem = NULL;

   elem = lCreateElem(RDE_Type);
   if (elem != NULL) {
      lSetUlong(elem, RDE_time, time);
      lSetDouble(elem, RDE_amount, amount);    
   }

   return elem;
}
