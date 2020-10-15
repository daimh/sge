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

#include "uti/sge_rmon.h"

#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_order.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_job.h"

#include "sge_orders.h"
#include "sge_interactive_sched.h"

static void 
order_remove_order_and_immediate(lListElem *job, lListElem *ja_task, 
                                 order_t *orders);

/****** SCHEDD/remove_immediate_jobs()******************************************
*  NAME
*     remove_immediate_jobs() -- test for and remove immediate jobs which can't
*                                be scheduled
*
*  SYNOPSIS
*     int remove_immediate_jobs(lList *pending_job_list,
                                lList *running_job_list, order_t *orders)
*
*  FUNCTION
*     Goes through all jobs in the pending list to see if any are immediate and
*     not idle.  If any are, they are removed.  This is done by generating an
*     order of type ORT_remove_immediate_job.  If any array jobs are removed,
*     the running list is checked for tasks belonging to the job, which are
*     also removed.  This is done by removing the ORT_start_job orders and
*     adding an order of type ORT_remove_immediate_job.
*
*  INPUTS
*     lList *pending_job_list   - The list of pending jobs for this scheduler
*                                 pass (JB_Type)
*     lList *running_job_list   - The list of running jobs for this scheduler
*                                 pass (JB_Type)
*     order_t *orders           - The order structure for this scheduler pass
*
*  RESULT
*     int - Error code: 0 = OK, 1 = Errors -- always returns 0
*
*  NOTES
*     MT-NOTE: remove_immediate_jobs() is MT safe
*
*******************************************************************************/
int remove_immediate_jobs(lList *pending_job_list, lList *running_job_list, order_t *orders) 
{
   lListElem *next_job, *job, *ep; 
   lList* lp;

   DENTER (TOP_LAYER, "remove_immediate_jobs");

   next_job = lFirst (pending_job_list);
   
   while ((job = next_job)) {
      lCondition *where = NULL;
      next_job = lNext(job);
      
      /* skip non immediate .. */
      if (!JOB_TYPE_IS_IMMEDIATE(lGetUlong(job, JB_type))) {
         continue;
      }

      /* .. and non idle jobs */
      if ((lp = lGetList(job, JB_ja_tasks)) && 
            (ep = lFirst(lp)) && 
            lGetUlong(ep, JAT_status)==JIDLE) {
         continue;    
      }

      /* Prepare search condition for running list */
      where = lWhere("%T(%I==%u)", JB_Type, JB_job_number, lGetUlong(job, JB_job_number));
      
      /* Remove the job from the pending list */
      remove_immediate_job(pending_job_list, job, orders, 0);
      
      /* If the job also exists in the running list, we need to remove it there
       * as well since array jobs are all or nothing. */
      if ((job = lFindFirst(running_job_list, where)) != NULL) {
         remove_immediate_job(running_job_list, job, orders, 1);
      }
      
      lFreeWhere(&where);
   }

   DEXIT;
   return 0;
}

/****** SCHEDD/remove_immediate_job()*******************************************
*  NAME
*     remove_immediate_job() -- test for and remove immediate job which can't
*                                be scheduled
*
*  SYNOPSIS
*     int remove_immediate_job(lList *job_list, lListElem *job, order_t *orders,
                               int remove_orders)
*
*  FUNCTION
*     Removes immediate jobs which cannot be scheduled from the given job list.
*     This is done by generating an order of type ORT_remove_immediate_job.  If
*     remove_orders is set, the ORT_start_job orders are first removed from the
*     order list before adding the remove order.
*
*  INPUTS
*     lList     *job_list     - The list of jobs from which the job should be
*                               removed (JB_Type)
*     lListElem *job          - The job to remove (JB_Type)
*     order_t *orders         - The order structure for this scheduler pass
*     int       remove_orders - Whether the ORT_start_job orders should also be
*                               be removed
*
*  NOTES
*     MT-NOTE: remove_immediate_job() is MT safe
*
*******************************************************************************/
void 
remove_immediate_job(lList *job_list, lListElem *job, order_t *orders, int remove_orders) 
{
   lListElem *ja_task; 
   lListElem *range = NULL;
   lList *range_list = NULL;
   u_long32 ja_task_id;

   DENTER (TOP_LAYER, "remove_immediate_job");

   for_each (ja_task, lGetList(job, JB_ja_tasks)) {
      if (remove_orders) {
         order_remove_order_and_immediate(job, ja_task, orders);
      }
      else {
         order_remove_immediate(job, ja_task, orders);
      }
   }
   
   range_list = lGetList(job, JB_ja_n_h_ids);
   
   for_each(range, range_list) {
      for(ja_task_id = lGetUlong(range, RN_min);
          ja_task_id <= lGetUlong(range, RN_max);
          ja_task_id += lGetUlong(range, RN_step)) {  
         ja_task = job_get_ja_task_template_pending(job, ja_task_id);
         
         /* No need to remove the orders here because tasks in JB_ja_n_h_ids
          * haven't been scheduled, and hence don't have start orders to
          * remove. */
         order_remove_immediate(job, ja_task, orders);
      }
   }
   lRemoveElem(job_list, &job);
   
   DEXIT;
}

/****** SCHEDD/order_remove_order_and_immediate()*******************************
*  NAME
*     order_remove_order_and_immediate() -- add a remove order for the job task
*
*  SYNOPSIS
*     int order_remove_order_and_immediate(lListElem *job, lListElem *ja_task,
                                 order_t *orders)
*
*  FUNCTION
*     Generates an order of type ORT_remove_immediate_job for the given job
*     task.  Also removes the ORT_start_job order for this task from the order
*     list.
*
*  INPUTS
*     lListElem *job       - The job to remove  (JB_Type)
*     lListElem *ja_task   - The task to remove (JAT_Type)
*     order_t *orders      - The order structurestructure  for this scheduler pass be removed
*
*  RESULT
*     int - Error code: 0 = OK, 1 = Errors
*
*  NOTES
*     MT-NOTE: order_remove_order_and_immediate() is MT safe
*
*******************************************************************************/
static void 
order_remove_order_and_immediate( lListElem *job, lListElem *ja_task, order_t *orders) 
{
   /* The possibility exists that this task is part of an array task, that it
    * already has earned an order to be scheduled, and that one or more other
    * tasks in this same job were not scheduled, resulting in this delete
    * order.  In this case, we have to remove the schedule order before we add
    * the delete order.  Otherwise, the qmaster will think we're trying to do
    * an ORT_remove_immediate_job on a non-idle task. */   

   /* Warning: we have a problem, if we send start orders during the dispatch run. We
    * might have started a array task of an immediate array job without verifying, that
    * the whole job can run
    */  
   lList *orderList = orders->jobStartOrderList;
   lCondition *where = lWhere("%T(%I==%u && %I==%u && %I==%u)", OR_Type,
                      OR_type, ORT_start_job,
                      OR_job_number, lGetUlong(job, JB_job_number),
                      OR_ja_task_number, lGetUlong(ja_task, JAT_task_number));
   lListElem *ep = lFindFirst (orderList, where);
   
   DENTER(TOP_LAYER, "order_remove_order_and_immediate");
   
   if (ep != NULL) {
      DPRINTF (("Removing job start order for job task "sge_u32"."sge_u32"\n",
                lGetUlong(job, JB_job_number),
                lGetUlong(ja_task, JAT_task_number)));
      lRemoveElem(orderList, &ep);
   }
   
   order_remove_immediate(job, ja_task, orders);
   lFreeWhere(&where);
   
   DEXIT;
}

/****** SCHEDD/order_remove_immediate()*****************************************
*  NAME
*     order_remove_immediate() -- add a remove order for the job task
*
*  SYNOPSIS
*     int order_remove_immediate(lListElem *job, lListElem *ja_task,
                                 order_t *orders)
*
*  FUNCTION
*     Generates an order of type ORT_remove_immediate_job for the given job
*     task.
*
*  INPUTS
*     lListElem *job       - The job to remove (JB_Type)
*     lListElem *ja_task   - The task to remove (JAT_Type)
*     order_t *orders      - The order structure will be extended by one del order
*
*  RESULT
*     int - Error code: 0 = OK, 1 = Errors
*
*  NOTES
*     MT-NOTE: order_remove_immediate() is MT safe
*
*******************************************************************************/
int 
order_remove_immediate(lListElem *job, lListElem *ja_task, order_t *orders) 
{
   DENTER(TOP_LAYER, "order_remove_immediate");

   DPRINTF(("JOB "sge_u32"."sge_u32" can't get dispatched - removing\n", 
      lGetUlong(job, JB_job_number), lGetUlong(ja_task, JAT_task_number)));
   
   orders->jobStartOrderList = sge_create_orders(orders->jobStartOrderList, 
                                                 ORT_remove_immediate_job, 
                                                 job, ja_task, NULL, true);
   
   DEXIT;
   return (orders->jobStartOrderList == NULL);
}
