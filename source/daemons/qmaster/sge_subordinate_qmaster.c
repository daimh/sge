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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_signal.h"
#include "uti/sge_string.h"
#include "uti/sge_hostname.h"
#include "uti/sge_sl.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_subordinate.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_ja_task_JAT_L.h"

#include "sched/sge_sched.h"

#include "sge_event_master.h"
#include "sge_qmod_qmaster.h"
#include "sge_qinstance_qmaster.h"
#include "sge_subordinate_qmaster.h"
#include "msg_qmaster.h"

static bool
qinstance_x_on_subordinate(sge_gdi_ctx_class_t *ctx,
                           lListElem *this_elem, bool suspend,
                           bool send_event, monitoring_t *monitor);


/****** sge_subordinate_qmaster/get_slotwise_sos_threshold() *******************
*  NAME
*     get_slotwise_sos_threshold() -- Retrieves the "threshold" value of the
*                                     slotwise sos configuration of the queue
*
*  SYNOPSIS
*     static u_long32 get_slotwise_sos_threshold(lListElem *qinstance) 
*
*  FUNCTION
*     Retrieves the "threshold" value of the slotwise sos configuration. This
*     is the value after the "slots=" keyword.
*
*  INPUTS
*     lListElem *qinstance - The qinstance from which the threshold is to
*                            be retrieved.
*
*  RESULT
*     u_long32 - The "threshold" value. 0 if no slotwise suspend on
*                subordinate is defined.
*
*  NOTES
*     MT-NOTE: get_slotwise_sos_threshold() is MT safe 
*******************************************************************************/
static u_long32
get_slotwise_sos_threshold(const lListElem *qinstance)
{
   u_long32  slots_sum = 0;
   lList     *so_list = NULL;
   lListElem *so = NULL;

   if (qinstance != NULL) {
      so_list = lGetList(qinstance, QU_subordinate_list);
      if (so_list != NULL) {
         so = lFirst(so_list);
         if (so != NULL) {
            slots_sum = lGetUlong(so, SO_slots_sum);
         }
      }
   }
   return slots_sum;
}

/****** sge_subordinate_qmaster/slotwise_x_on_subordinate() ********************
*  NAME
*     slotwise_x_on_subordinate() -- Execute the (un)suspend
*
*  SYNOPSIS
*     static bool slotwise_x_on_subordinate(sge_gdi_ctx_class_t *ctx, lListElem 
*     *qinstance_where_task_is_running, u_long32 job_id, u_long32 task_id, bool 
*     suspend, bool send_signal, monitoring_t *monitor) 
*
*  FUNCTION
*     Executes the (un)suspend of the specified task. 
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx                   - GDI context
*     lListElem *qinstance_where_task_is_running - QU_Type Element of the qinstance
*                                                  in which the task to (un)suspend
*                                                  is running/suspended.
*     u_long32 job_id                            - Job ID of the task to (un)suspend
*     u_long32 task_id                           - Task ID of the task to (un)suspend
*     bool suspend                               - suspend or unsuspend
*     monitoring_t *monitor                      - monitor
*
*  RESULT
*     bool - true:  Task is (un)suspended.
*            false: An error occcurred.
*
*  NOTES
*     MT-NOTE: slotwise_x_on_subordinate() is not MT safe, the global lock
*              must be set outside before calling this function
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static bool
slotwise_x_on_subordinate(sge_gdi_ctx_class_t *ctx,
                          lListElem *qinstance_where_task_is_running,
                          u_long32 job_id, u_long32 task_id, bool suspend,
                          monitoring_t *monitor)
{
   bool      ret = false;
   lListElem *jep = NULL;
   lListElem *jatep = NULL;
   lList     *master_job_list = NULL;
   u_long32  state = 0;

   DENTER(TOP_LAYER, "slotwise_x_on_subordinate");

   master_job_list = *object_type_get_master_list(SGE_TYPE_JOB);
   jep = lGetElemUlong(master_job_list, JB_job_number, job_id);
   if (jep != NULL) {
      jatep = lGetSubUlong(jep, JAT_task_number, task_id, JB_ja_tasks);
      if (jatep != NULL) {
         sge_signal_queue(ctx, suspend?SGE_SIGSTOP:SGE_SIGCONT,
               qinstance_where_task_is_running, jep, jatep, monitor);
         /* Set status */
         state = lGetUlong(jatep, JAT_state);

         if (suspend == true) {
            SETBIT(JSUSPENDED_ON_SLOTWISE_SUBORDINATE, state);
            DPRINTF(("Setting status JSUSPENDED_ON_SLOTWISE_SUBORDINATE for job %lu.%lu\n",
                     (unsigned long) job_id, (unsigned long) task_id));
         } else {
            CLEARBIT(JSUSPENDED_ON_SLOTWISE_SUBORDINATE, state);
            DPRINTF(("Clearing status JSUSPENDED_ON_SLOTWISE_SUBORDINATE for job %lu.%lu\n",
                     (unsigned long) job_id, (unsigned long) task_id));
         }
         lSetUlong(jatep, JAT_state, state);
         ret = true;
      } else {
         /* TODO: HP: Add error handling! */
      }
   } else {
      /* TODO: HP: Add error handling! */
   }
   DRETURN(ret);
}

/****** sge_subordinate_qmaster/get_slotwise_sos_tree_root() *******************
*  NAME
*     get_slotwise_sos_tree_root() -- Gets the root qinstance of the slotwise
*        suspend on subordinate tree.
*
*  SYNOPSIS
*     static lListElem* get_slotwise_sos_tree_root(lListElem 
*     *node_queue_instance) 
*
*  FUNCTION
*     Returns the qinstance that is the root of the slotwise suspend on
*     subordinate tree where the provided qinstance is a member of.
*     Returns NULL if the give qinstance is not a member of a slotwise suspend
*     on subordinate tree.
*
*  INPUTS
*     lListElem *node_queue_instance -  For this queue instance the slotwise
*                                       suspend on subordinate tree root is
*                                       searched.
*
*  RESULT
*     lListElem* - The root node of the slotwise suspend on subordinate
*                  tree, node_queue_instance if it is the root node,
*                  or NULL if node_queue_instance is not part of any
*                  slotwise suspend on subordinate definition.
*
*  NOTES
*     MT-NOTE: get_slotwise_sos_tree_root() is not MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
/* TODO: HP: Use get_slotwise_super_qinstance() recursively instead of this function */
static lListElem*
get_slotwise_sos_tree_root(lListElem *node_queue_instance)
{
   const char *node_queue_name = NULL;
   const char *node_host_name = NULL;
   lListElem  *cqueue = NULL;
   lListElem  *root_qinstance = NULL;
   lList      *cqueue_master_list = *object_type_get_master_list(SGE_TYPE_CQUEUE);

   DENTER(TOP_LAYER, "get_slotwise_sos_tree_root");

   if (node_queue_instance != NULL) {
      if (get_slotwise_sos_threshold(node_queue_instance) > 0) {
         /* For now, assume this qinstance is the root node of the slotwise sos tree. */
         root_qinstance = node_queue_instance;
      }

      /* Search the superordinated queue instance of our node_queue_instance.
       * Then search the superordinated queue instance of this superordinated
       * queue instance, and so on.
       * We have to search in queue instances, not in queues, because possibly
       * for our current host there is no superordinated queue instance of our
       * node_queue_instance, while there might be one on another host.
       */
      node_queue_name = lGetString(node_queue_instance, QU_qname);
      node_host_name  = lGetHost(node_queue_instance, QU_qhostname);

      for_each(cqueue, cqueue_master_list) {
         lListElem *qinstance;
         lListElem *sub;
         qinstance = cqueue_locate_qinstance(cqueue, node_host_name);

         if (qinstance == NULL) {
            /* There is no instance of this cluster queue on this host. Continue
             * with another branch of the tree.
             */
            continue;
         }

         sub = lGetSubStr(qinstance, SO_name, node_queue_name, QU_subordinate_list);
         if (sub != NULL && lGetUlong(sub, SO_slots_sum) != 0) {
            /* Our node queue is mentioned in the subordinate_list of
             * this queue. This queue is our superordinated queue,
             * i.e. the parent in the slotwise sos tree!
             * Now we can look for the parent of our parent.
             */
            root_qinstance = get_slotwise_sos_tree_root(qinstance);
         }
      }
   }
   DRETURN(root_qinstance);
}

/****** sge_subordinate_qmaster/get_slotwise_suspend_superordinate() ***********
*  NAME
*     get_slotwise_suspend_superordinate() -- Get the superordinate of the
*                                             qinstance with the provided name
*
*  SYNOPSIS
*     static lListElem* get_slotwise_suspend_superordinate(const char 
*     *queue_name, const char *hostname) 
*
*  FUNCTION
*     Returns the slotwise superordinated queue instance of the queue instance
*     with the provided name.
*
*  INPUTS
*     const char *queue_name - cluster queue name of the subordinated qeueue
*                              instance
*     const char *hostname   - host name of the subordinated queue instance
*
*  RESULT
*     lListElem* - The slotwise superordinated queue instance (QU_Type) of
*                  the provided queue instance, or NULL if there is none.
*
*  NOTES
*     MT-NOTE: get_slotwise_suspend_superordinate() is not MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static lListElem*
get_slotwise_suspend_superordinate(const char *queue_name, const char *hostname)
{
   lListElem *cqueue = NULL;
   lListElem *qinstance = NULL;
   lListElem *super_qinstance = NULL;
   lListElem *so = NULL;
   lList *cqueue_master_list = *object_type_get_master_list(SGE_TYPE_CQUEUE);

   DENTER(TOP_LAYER, "get_slotwise_suspend_superordinates");

   for_each(cqueue, cqueue_master_list) {
      qinstance = cqueue_locate_qinstance(cqueue, hostname);

      if (qinstance != NULL) {
         if (get_slotwise_sos_threshold(qinstance) > 0) {
            so = lGetSubStr(qinstance, SO_name, queue_name, QU_subordinate_list);
            if (so != NULL && lGetUlong(so, SO_slots_sum) != 0) {
               /* the queue_name is listed in the subordinate list of this
                * queue instance, so it's our superordinated queue instance.
                */
               super_qinstance = qinstance;
               break;
            }
         }
      }
   }
   DRETURN(super_qinstance);
}

/****** sge_subordinate_qmaster/get_slotwise_sos_super_qinstance() *************
*  NAME
*     get_slotwise_sos_super_qinstance() -- Get the superordinate of the
*                                           qinstance with the provided name
*
*  SYNOPSIS
*     static lListElem* get_slotwise_sos_super_qinstance(lListElem *qinstance) 
*
*  FUNCTION
*     Returns the slotwise superordinated queue instance of the provided queue
*     instance.
*
*  INPUTS
*     lListElem *qinstance - The subordinated queue instance (QU_Type)
*
*  RESULT
*     lListElem* - The slotwise superordinated queue instance (QU_Type) of
*                  the provided queue instance, or NULL if there is none.
*
*  NOTES
*     MT-NOTE: get_slotwise_sos_super_qinstance() is not MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static lListElem* 
get_slotwise_sos_super_qinstance(lListElem *qinstance)
{
   lListElem  *super_qinstance = NULL;
   const char *qinstance_name = NULL;
   const char *qinstance_host = NULL;

   if (qinstance != NULL) {
      qinstance_name = lGetString(qinstance, QU_qname);
      qinstance_host = lGetHost(qinstance, QU_qhostname);
      super_qinstance = get_slotwise_suspend_superordinate(qinstance_name, qinstance_host);
   }
   return super_qinstance;
}

typedef struct {
   u_long32  job_id;
   lListElem *task; /* JAT_Type */
} ssos_task_t;

typedef struct {
   u_long32      depth;
   u_long32      seq_no;     /* from the parents QU_subordinate_list */
   u_long32      action;     /* from the parents QU_subordinate_list */
   lListElem     *qinstance; /* QU_Type */
   lListElem     *parent;    /* QU_Type */
   sge_sl_list_t *tasks;
} ssos_qinstance_t;

/****** sge_subordinate_qmaster/destroy_slotwise_sos_task_elem() ***************
*  NAME
*     destroy_slotwise_sos_task_elem() -- Destructor for the elements of a 
*                                         sge simple list of ssos_taks_t elements
*
*  SYNOPSIS
*     static bool destroy_slotwise_sos_task_elem(ssos_task_t **ssos_task) 
*
*  FUNCTION
*     Destroys the data members of sge simple list of ssos_task_t elements.
*
*  INPUTS
*     ssos_task_t **ssos_task - The ssos_task_t element to destroy
*
*  RESULT
*     bool -  Always true to continue the destruction of all list elements.
*
*  NOTES
*     MT-NOTE: destroy_slotwise_sos_task_elem() is MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static bool
destroy_slotwise_sos_task_elem(ssos_task_t **ssos_task)
{
   if (ssos_task != NULL && *ssos_task != NULL) {
      sge_free(ssos_task);
   }
   return true;
}

/****** sge_subordinate_qmaster/destroy_slotwise_sos_tree_elem() ***************
*  NAME
*     destroy_slotwise_sos_tree_elem() -- Destructor for the elements of a
*                                         sge simple list of ssos_qinstance_t
*                                         elements
*
*  SYNOPSIS
*     static bool destroy_slotwise_sos_tree_elem(ssos_qinstance_t 
*     **ssos_qinstance) 
*
*  FUNCTION
*     Destroys the data members of a sge simple list of ssos_qinstance_t
*     elements. Takes care of the destruction of the sge simple list sublist of
*     ssos_task_t members.
*
*  INPUTS
*     ssos_qinstance_t **ssos_qinstance - The ssos_qinstance_t element to
*                                         destroy
*
*  RESULT
*     bool - Always true to continue the destruction of all list elements.
*
*  NOTES
*     MT-NOTE: destroy_slotwise_sos_tree_elem() is MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static bool
destroy_slotwise_sos_tree_elem(ssos_qinstance_t **ssos_qinstance)
{
   if (ssos_qinstance != NULL && *ssos_qinstance != NULL) {
      if ((*ssos_qinstance)->tasks != NULL) {
         sge_sl_destroy(&((*ssos_qinstance)->tasks), (sge_sl_destroy_f)destroy_slotwise_sos_task_elem);
      }
      sge_free(ssos_qinstance);
   }
   
   return true;
}

/****** sge_subordinate_qmaster/is_ssos() **************************************
*  NAME
*     is_ssos() -- Checks if a task is suspended by slotwise subordination
*                  and not suspended otherwise, too.
*
*  SYNOPSIS
*     static bool is_ssos(bool only_ssos, lListElem *task) 
*
*  FUNCTION
*     If only_ssos is true, this function checks if a task is suspended by
*     slotwise subordination and not suspended otherwise at the same time, too.
*     If only_ssos is false, this function checks if a task is suspended by
*     slotwise subordination. This task may additionally be suspended by
*     another suspend method.
*     A task that is suspended by slotwise subordination could also be suspended
*     manually, or by threshold, or by queue wise subordination or by calendar,
*     too.
*
*  INPUTS
*     bool only_ssos  - If true, checks if task is only suspended by slotwise
*                       suspend on subordinate,
*                       if false, checks if task is suspended by slotwise
*                       suspend on subordinate, but it might be also
*                       suspended by some other suspend method at the same time.
*     lListElem *task - The task to check. CULL type "JAT_Type". 
*
*  RESULT
*     bool - true if the task is (only) suspended by slotwise suspend
*            on subordinate.
*            false else.
*
*  NOTES
*     MT-NOTE: is_ssos() is MT safe 
*******************************************************************************/
static bool
is_ssos(bool only_ssos, lListElem *task)
{
   bool     ret = false;
   u_long32 state = 0;

   if (task != NULL) {
      state = lGetUlong(task, JAT_state);
      if (only_ssos == true) {
         ret = (bool)(ISSET(state, JSUSPENDED) == false &&
               ISSET(state, JSUSPENDED_ON_THRESHOLD) == false &&
               ISSET(state, JSUSPENDED_ON_SUBORDINATE) == false &&
               ISSET(state, JSUSPENDED_ON_SLOTWISE_SUBORDINATE) == true);
      } else {
         ret = (bool)ISSET(state, JSUSPENDED_ON_SLOTWISE_SUBORDINATE);
      }
   }
   return ret; 
}

/****** sge_subordinate_qmaster/has_ssos_task() ********************************
*  NAME
*     has_ssos_task() -- Checks if a qinstance has tasks that are suspended
*                        by slotwise suspend on subordinate (only).
*
*  SYNOPSIS
*     static bool has_ssos_task(bool only_ssos_task, ssos_qinstance_t 
*     *ssos_qinstance) 
*
*  FUNCTION
*     If only_ssos is true, this function checks if this queue has at least one
*     task that is suspended by slotwise subordination and not suspended
*     otherwise at the same time, too.
*     If only_ssos is false, this function checks if this queue has at least one
*     task that is suspended by slotwise subordination. This task may
*     additionally by suspended by some other suspend method.
*     A task that is suspended by slotwise subordination could also be suspended
*     manually, or by threshold, or by queue wise subordination or by calendar,
*     too.
*
*  INPUTS
*     bool only_ssos  - If true, checks if task is only suspended by ssos,
*                       if false, checks if task is suspended by ssos, but
*                       it might also be suspended by other methods at the
*                       same time.
*     ssos_qinstance_t *ssos_qinstance - The qinstance to check.
*
*  RESULT
*     bool - true if the qinstance has at least one task that is
*            suspended by slotwise suspend on subordinate (only).
*            false if there is no such task.
*
*  NOTES
*     MT-NOTE: has_ssos_task() is MT safe 
*
*  SEE ALSO
*     sge_subordinate_qmaster/is_ssos()
*******************************************************************************/
static bool
has_ssos_task(bool only_ssos_task, ssos_qinstance_t *ssos_qinstance)
{
   bool          ret = false;
   sge_sl_elem_t *ssos_task_elem = NULL;
   ssos_task_t   *ssos_task = NULL;

   for_each_sl(ssos_task_elem, ssos_qinstance->tasks) {
      ssos_task = sge_sl_elem_data(ssos_task_elem);
      ret = is_ssos(only_ssos_task, ssos_task->task);
      if (ret == true) {
         break;
      }
   }
   return ret;
}

/****** sge_subordinate_qmaster/get_task_to_x_in_depth() ***********************
*  NAME
*     get_task_to_x_in_depth() -- Searches the slotwise subordinate tree in a
*                                 specific depth for a task to (un)suspend
*
*  SYNOPSIS
*     static void get_task_to_x_in_depth(sge_sl_list_t 
*     *slotwise_sos_tree_qinstances, u_long32 depth, bool suspend,
*     bool only_slotwise_suspended, ssos_qinstance_t **ssos_qinstance_to_x,
*     ssos_task_t **ssos_task_to_x) 
*
*  FUNCTION
*     Searches in the provided slotwise subordination tree in a specific depth
*     for a task to (un)suspend.
*
*  INPUTS
*     sge_sl_list_t *slotwise_sos_tree_qinstances - The slotwise suspend on
*                                                   subordinate tree as a list
*     u_long32 depth                              - The depth in this tree where
*                                                   the task is to be searched
*     bool suspend                                - Are we going to suspend or
*                                                   unsuspend a task?
*     bool only_slotwise_suspended                - Are we looking for tasks to
*                                                   unsuspend that are only
*                                                   slotwise suspended, or are
*                                                   we looking for tasks that are
*                                                   also suspended manually, by
*                                                   threshold or by queue wise
*                                                   subordination?
*                                                   Ignored if suspend == true.
*     ssos_qinstance_t **ssos_qinstance_to_x      - The queue instance where
*                                                   the found task is running
*     ssos_task_t **ssos_task_to_x                - The task we found
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: get_task_to_x_in_depth() is MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static void
get_task_to_x_in_depth(sge_sl_list_t *slotwise_sos_tree_qinstances,
      u_long32 depth, bool suspend, bool only_slotwise_suspended,
      ssos_qinstance_t **ssos_qinstance_to_x, ssos_task_t **ssos_task_to_x)
{
   sge_sl_elem_t *ssos_tree_elem = NULL;
   u_long32      extreme_seq_no     = (suspend==true) ? 0 : (u_long32)-1;
   u_long32      oldest_start_time = (u_long32)-1;
   u_long32      youngest_start_time = 0;

   for_each_sl(ssos_tree_elem, slotwise_sos_tree_qinstances) {
      ssos_qinstance_t *ssos_qinstance = sge_sl_elem_data(ssos_tree_elem);
     
      /* Get highest/lowest seq_no of queues with running/suspended tasks
       * jobs among all queues of current depth
       */
      if (ssos_qinstance->depth == depth &&
          ssos_qinstance->tasks != NULL &&
          ((suspend == true && ssos_qinstance->seq_no > extreme_seq_no) ||
           (suspend == false && ssos_qinstance->seq_no < extreme_seq_no &&
            has_ssos_task(only_slotwise_suspended, ssos_qinstance)))) {
         extreme_seq_no = ssos_qinstance->seq_no;
      }
   }
   
   for_each_sl(ssos_tree_elem, slotwise_sos_tree_qinstances) {
      ssos_qinstance_t *ssos_qinstance = sge_sl_elem_data(ssos_tree_elem);
      
      
      /* Search in all queues in current depth, with highest/lowest seq_no
       * and with running/suspended tasks for the oldest/youngest task
       */
      if (ssos_qinstance->depth == depth &&
          ssos_qinstance->seq_no == extreme_seq_no) {
         sge_sl_elem_t *ssos_task_elem = NULL;
         bool oldest = (bool)(ssos_qinstance->action == SO_ACTION_LR);

         /* If we have to unsuspend and if we would look for the youngest job
          * for suspend, we have to look for the oldest job to unsuspend.
          */
         if (suspend == false) {
            oldest = (bool)!oldest;
         }

         for_each_sl(ssos_task_elem, ssos_qinstance->tasks) {
            u_long32    start_time = 0;
            ssos_task_t *ssos_task = sge_sl_elem_data(ssos_task_elem);
           
            if (suspend == true ||
                (suspend == false &&
                 is_ssos(only_slotwise_suspended, ssos_task->task) == true)) {
               start_time = lGetUlong(ssos_task->task, JAT_start_time);
               if (oldest == true && start_time < oldest_start_time) {
                  oldest_start_time = start_time;
                  *ssos_task_to_x = ssos_task;
                  *ssos_qinstance_to_x = ssos_qinstance;
               }
               if (oldest == false && start_time > youngest_start_time) {
                  youngest_start_time = start_time;
                  *ssos_task_to_x = ssos_task;
                  *ssos_qinstance_to_x = ssos_qinstance;
               }
            }
         }
      }
   }
}

/****** sge_subordinate_qmaster/remove_task_from_slotwise_sos_tree() ***********
*  NAME
*     remove_task_from_slotwise_sos_tree() -- Removes a task from the slotwise
*                                             suspend on subordinate tree list
*
*  SYNOPSIS
*     static void remove_task_from_slotwise_sos_tree(sge_sl_list_t 
*     *slotwise_sos_tree_qinstances, u_long32 job_id, u_long32 task_id) 
*
*  FUNCTION
*     Removes a specific task from the slotwiese suspend on subordinate tree
*     list. For this, it searches the task in all queue instances of the
*     list.
*
*  INPUTS
*     sge_sl_list_t *slotwise_sos_tree_qinstances - The slotwise suspend on
*                                                   subordinate tree as a list
*     u_long32 job_id                             - The job ID of the task
*     u_long32 task_id                            - The task ID of the task
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: remove_task_from_slotwise_sos_tree() is not MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static void
remove_task_from_slotwise_sos_tree(sge_sl_list_t *slotwise_sos_tree_qinstances,
                                   u_long32 job_id, u_long32 task_id)
{
   sge_sl_elem_t    *ssos_tree_elem = NULL;
   sge_sl_elem_t    *ssos_task_elem = NULL;
   ssos_qinstance_t *ssos_qinstance = NULL;
   ssos_task_t      *ssos_task = NULL;

   for_each_sl(ssos_tree_elem, slotwise_sos_tree_qinstances) {
      ssos_qinstance = sge_sl_elem_data(ssos_tree_elem);
      for_each_sl(ssos_task_elem, ssos_qinstance->tasks) {
         ssos_task = sge_sl_elem_data(ssos_task_elem);
         if (ssos_task->job_id == job_id &&
             lGetUlong(ssos_task->task, JAT_task_number) == task_id) {
            sge_sl_dechain(ssos_qinstance->tasks, ssos_task_elem);
            if (sge_sl_get_elem_count(ssos_qinstance->tasks) == 0) {
               sge_sl_destroy(&(ssos_qinstance->tasks), NULL);
            }
            sge_sl_elem_destroy(&ssos_task_elem, (sge_sl_destroy_f)destroy_slotwise_sos_task_elem);
         }
      }
   }
}

static bool
x_most_extreme_task(sge_gdi_ctx_class_t *ctx, sge_sl_list_t *slotwise_sos_tree_qinstances,
      bool suspend, monitoring_t *monitor)
{
   bool             suspended_a_task = false;
   u_long32         depth = 0;
   u_long32         i;
   sge_sl_elem_t    *ssos_tree_elem = NULL;
   ssos_qinstance_t *ssos_qinstance = NULL;
   ssos_task_t      *ssos_task = NULL;

   /* Walk over the whole list and find biggest depth */
   for_each_sl(ssos_tree_elem, slotwise_sos_tree_qinstances) {
      ssos_qinstance = sge_sl_elem_data(ssos_tree_elem);
      depth = MAX(ssos_qinstance->depth, depth);
   }

   ssos_qinstance = NULL;
   if (suspend == true) {
      /* Walk over list, get oldest (youngest) job from qinstances of biggest depth */
      /* If there was no running job in one of the qinstances of biggest depth, repeat
       * with qinstances of max_depth-1, and so on.
       */
      for (i=depth; i>0 && ssos_task==NULL; i--) {
         /* find youngest running task to suspend */
         get_task_to_x_in_depth(slotwise_sos_tree_qinstances, i, suspend, true,
               &ssos_qinstance, &ssos_task);
      }
   } else {
      /* First we look for the best task to unsuspend that is only slotwise
       * suspended and not manually, by threshold or by queue wise subordination.
       * If there is no such task, we look for the best task that is also
       * otherwise suspended to remove the JSUSPENDED_ON_SLOTWISE_SUBORDINATE
       * flag from this task, so it might be unsuspended as soon as the other
       * suspends are removed.
       */
/* TODO: HP: Optimize this: The first loop could also detect and store a
 *           candidate for the second case, so the second loop could be
 *           removed.
 */
      for (i=1; i<=depth && ssos_task==NULL; i++) {
         /* find oldest only slotwise suspended task to unsuspend */
         get_task_to_x_in_depth(slotwise_sos_tree_qinstances, i, suspend, true,
               &ssos_qinstance, &ssos_task);
      }
      if (ssos_task == NULL) {
         for (i=1; i<=depth && ssos_task==NULL; i++) {
            /* find oldest also otherwise suspended task to unsuspend */
            get_task_to_x_in_depth(slotwise_sos_tree_qinstances, i, suspend, false,
                  &ssos_qinstance, &ssos_task);
         }
      }
   }

   if (ssos_task != NULL && ssos_qinstance != NULL) {
       /* (un)suspend this task */
       suspended_a_task = slotwise_x_on_subordinate(ctx, ssos_qinstance->qinstance,
                     ssos_task->job_id, lGetUlong(ssos_task->task, JAT_task_number),
                     suspend, monitor);
       if (suspended_a_task == true) {
          remove_task_from_slotwise_sos_tree(slotwise_sos_tree_qinstances, ssos_task->job_id,
                lGetUlong(ssos_task->task, JAT_task_number));
       }
   }
   return suspended_a_task;
}

static void
get_slotwise_sos_sub_tree_qinstances(lListElem *qinstance,
      sge_sl_list_t **tree_qinstances, u_long32 depth)
{
   lList     *so_list = NULL;
   lList     *cqueue_master_list = *object_type_get_master_list(SGE_TYPE_CQUEUE);
   lListElem *so = NULL;
   lListElem *sub_qinstance = NULL;

   /* get all qinstances in the slotwise sos tree excluding the super qinstance
    * (i.e. the root node of the tree), it was already added in the iteration before.
    */
   if (depth == 0) {
      ssos_qinstance_t *ssos_qinstance = NULL;

      if (tree_qinstances == NULL || *tree_qinstances != NULL) {
         return;
      }

      /* special handling for the root node of the tree */
      sge_sl_create(tree_qinstances);

      /* first add the super qinstance to the list */
      ssos_qinstance = (ssos_qinstance_t*)calloc(1, sizeof(ssos_qinstance_t));
      ssos_qinstance->seq_no    = 0;    /* the super qinstance has always top priority */
      ssos_qinstance->depth     = 0;    /* the super qinstance is on top */
      ssos_qinstance->action    = 0;    /* the super qinstances tasks don't get modified */
      ssos_qinstance->qinstance = qinstance;
      ssos_qinstance->parent    = NULL; /* the super qinstance has no parent */
      ssos_qinstance->tasks     = NULL; /* gets filled later */ 

      sge_sl_insert(*tree_qinstances, ssos_qinstance, SGE_SL_FORWARD);
      depth++;
   }

   so_list = lGetList(qinstance, QU_subordinate_list);
   for_each(so, so_list) {
      const char *so_name = NULL;
      const char *so_full_name = NULL;
      dstring dstr_so_full_name = DSTRING_INIT;
      ssos_qinstance_t *ssos_qinstance = NULL;
     
      /* if it is slotwise subordinated, get the pointer to this qinstance list elem */
      if (lGetUlong(so, SO_slots_sum) > 0 ) {
         so_name = lGetString(so, SO_name);
         if (strstr(so_name, "@") == NULL) {
            const char *host_name = NULL;

            host_name = lGetHost(qinstance, QU_qhostname);
            sge_dstring_sprintf(&dstr_so_full_name, "%s@%s", so_name, host_name);
            so_full_name = sge_dstring_get_string(&dstr_so_full_name);
         } else {
            so_full_name = so_name;
         }
         sub_qinstance = cqueue_list_locate_qinstance(cqueue_master_list, so_full_name);
         sge_dstring_free(&dstr_so_full_name);

         if (sub_qinstance != NULL) {
             ssos_qinstance = (ssos_qinstance_t*)calloc(1, sizeof(ssos_qinstance_t));
             ssos_qinstance->seq_no    = lGetUlong(so, SO_seq_no);
             ssos_qinstance->action    = lGetUlong(so, SO_action);
             ssos_qinstance->depth     = depth;
             ssos_qinstance->qinstance = sub_qinstance;
             ssos_qinstance->parent    = qinstance;
             ssos_qinstance->tasks     = NULL; /* gets filled later */

             sge_sl_insert(*tree_qinstances, ssos_qinstance, SGE_SL_FORWARD);

             get_slotwise_sos_sub_tree_qinstances(sub_qinstance, tree_qinstances, depth+1);
         }
      }
   }
}

static u_long32
count_running_jobs_in_slotwise_sos_tree(sge_sl_list_t *qinstances_in_slotwise_sos_tree,
      bool suspend)
{
   /* Walk over job list and get the tasks that are running in the qinstances
    * of the slotwise sos tree. Store informations about these tasks in the tree list.
    */
   lList     *master_job_list = NULL;
   lListElem *job = NULL;
   u_long32  sum = 0;

   if (qinstances_in_slotwise_sos_tree != NULL &&
       sge_sl_get_elem_count(qinstances_in_slotwise_sos_tree) > 0) {
      const char *host_name = NULL;
      ssos_qinstance_t *first_qinstance = NULL;

      sge_sl_data(qinstances_in_slotwise_sos_tree, (void**)&first_qinstance, SGE_SL_FORWARD);
      host_name = lGetHost(first_qinstance->qinstance, QU_qhostname);

      master_job_list = *object_type_get_master_list(SGE_TYPE_JOB);

      for_each(job, master_job_list) {
         lList     *task_list = NULL;
         lListElem *task = NULL;

         task_list = lGetList(job, JB_ja_tasks);
         for_each(task, task_list) {
            u_long32   state = 0;
            lListElem  *task_gdi = NULL;
            lList      *task_gdi_list = NULL;
            const void *iterator = NULL;

            task_gdi_list = lGetList(task, JAT_granted_destin_identifier_list);
            /* Get all destination identifiers of the current task that are on the
             * current host using lGetElemHostFirst()/lGetElemHostNext().
             */
            task_gdi = lGetElemHostFirst(task_gdi_list, JG_qhostname, host_name, &iterator);
            while (task_gdi != NULL) {
               const char    *qinstance_name = NULL;
               const char    *task_gdi_qname = NULL;
               u_long32       status = 0;
               sge_sl_elem_t *sl_elem = NULL;

               /* Count all tasks in state JRUNNING and store tasks to suspend. */
               state = lGetUlong(task, JAT_state);
               status = lGetUlong(task, JAT_status);

               if (ISSET(state, JRUNNING) == true &&
                   ISSET(state, JSUSPENDED) == false &&
                   ISSET(state, JSUSPENDED_ON_THRESHOLD) == false &&
                   ISSET(state, JSUSPENDED_ON_SUBORDINATE) == false &&
                   ISSET(state, JSUSPENDED_ON_SLOTWISE_SUBORDINATE) == false &&
                   ISSET(state, JDELETED) == false && 
                   ISSET(status, JEXITING) == false &&
                   ISSET(status, JFINISHED) == false) {
                  /* The current task is in state JRUNNING and not suspended in
                   * any way. 
                   * Check if the qinstance name where the current task is
                   * running is in the list of qinstances in the slotwise sos
                   * tree.
                   */
                  task_gdi_qname = lGetString(task_gdi, JG_qname);
                  for_each_sl(sl_elem, qinstances_in_slotwise_sos_tree) {
                     ssos_qinstance_t *ssos_qinstance = NULL;

                     ssos_qinstance = sge_sl_elem_data(sl_elem);
                     qinstance_name = lGetString(ssos_qinstance->qinstance, QU_full_name);
                     if (strcmp(task_gdi_qname, qinstance_name) == 0) {
                        /* The qinstance of the current task is in our ssos tree, now
                         * we can count this task.
                         */
                        sum++;
                        if (suspend == true) {
                           /* Store the running tasks in our ssos tree list. */
                           ssos_task_t *ssos_task = (ssos_task_t*)calloc(1, sizeof(ssos_task_t));

                           if (ssos_qinstance->tasks == NULL) {
                              sge_sl_create(&(ssos_qinstance->tasks));
                           }
                          
                           ssos_task->job_id = lGetUlong(job, JB_job_number);
                           ssos_task->task   = task;
                           sge_sl_insert(ssos_qinstance->tasks, ssos_task, SGE_SL_FORWARD);
                           break;
                        }
                     }
                  }
               } else if (suspend == false &&
                          ISSET(state, JSUSPENDED_ON_SLOTWISE_SUBORDINATE) == true &&
                          ISSET(state, JDELETED) == false &&
                          ISSET(status, JEXITING) == false) {

                  /* We have to remember all tasks that are slotwise suspended,
                   * even if they are also manually or by threshold or queue
                   * wise subordination suspended, because it might be that
                   * we don't find a task to unsuspend but can remove the
                   * JSUSPENDED_ON_SLOTWISE_SUBORDINATE flag from a doubly
                   * suspended task.
                   */
                  /* Check if the qinstance where the task is slotwise suspended is
                   * in the slotwise sos tree list.
                   */
                  task_gdi_qname = lGetString(task_gdi, JG_qname);
                  for_each_sl(sl_elem, qinstances_in_slotwise_sos_tree) {
                     ssos_qinstance_t *ssos_qinstance = NULL;

                     ssos_qinstance = sge_sl_elem_data(sl_elem);
                     qinstance_name = lGetString(ssos_qinstance->qinstance, QU_full_name);
                     if (strcmp(task_gdi_qname, qinstance_name) == 0) {
                        /* The qinstance of the current task is in our ssos tree, so
                         * we can store this task in our ssos tree list.
                         */
                        ssos_task_t *ssos_task = (ssos_task_t*)calloc(1, sizeof(ssos_task_t));
                        if (ssos_qinstance->tasks == NULL) {
                           sge_sl_create(&(ssos_qinstance->tasks));
                        }
                       
                        ssos_task->job_id = lGetUlong(job, JB_job_number);
                        ssos_task->task   = task;
                        sge_sl_insert(ssos_qinstance->tasks, ssos_task, SGE_SL_FORWARD);
                        break;
                     }
                  }
               }
               task_gdi = lGetElemHostNext(task_gdi_list, JG_qhostname, host_name, &iterator);
            }
         }
      }
   }
   return sum;
}

/****** sge_subordinate_qmaster/unsuspend_all_tasks_in_slotwise_sub_tree() *****
*  NAME
*     unsuspend_all_tasks_in_slotwise_sub_tree() -- unsuspends all slotwise
*        suspended tasks in the subtree of this qinstance
*
*  SYNOPSIS
*     void unsuspend_all_task_in_slotwise_sub_tree(sge_gdi_ctx_class_t *ctx,
*     lListElem *qinstance, monitoring_t *monitor) 
*
*  FUNCTION
*     Unsuspends all slotwise suspended tasks in the subtree of the provided
*     qinstance.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - context class
*     lListElem *qinstance     - The root of the slotwise preemption sub tree.
*     monitoring_t *monitor    - monitor
*
*  RESULT
*     void -  none
*
*  NOTES
*     MT-NOTE: unsuspend_all_tasks_in_slotwise_sub_tree() is MT safe 
*******************************************************************************/
void
unsuspend_all_tasks_in_slotwise_sub_tree(sge_gdi_ctx_class_t *ctx,
                                         lListElem *qinstance, monitoring_t *monitor)
{
   const bool       suspend = false;
   sge_sl_list_t    *qinstances_in_slotwise_sos_tree = NULL;
   sge_sl_elem_t    *ssos_qinstance_elem = NULL;
   ssos_qinstance_t *ssos_qinstance = NULL;
   sge_sl_elem_t    *ssos_task_elem = NULL;
   ssos_task_t      *ssos_task = NULL;

   if (qinstance != NULL) {
      /* get the slotwise sos sub tree of this qinstance as a list */
      get_slotwise_sos_sub_tree_qinstances(qinstance, &qinstances_in_slotwise_sos_tree, 0);

      /* fill the tree list with all slotwise suspended tasks */
      count_running_jobs_in_slotwise_sos_tree(qinstances_in_slotwise_sos_tree, suspend);

      for_each_sl(ssos_qinstance_elem, qinstances_in_slotwise_sos_tree) {
         ssos_qinstance = sge_sl_elem_data(ssos_qinstance_elem);
         for_each_sl(ssos_task_elem, ssos_qinstance->tasks) {
            ssos_task = sge_sl_elem_data(ssos_task_elem);
            slotwise_x_on_subordinate(ctx, ssos_qinstance->qinstance, ssos_task->job_id,
               lGetUlong(ssos_task->task, JAT_task_number), suspend, monitor);
         }
      }
      sge_sl_destroy(&qinstances_in_slotwise_sos_tree, (sge_sl_destroy_f)destroy_slotwise_sos_tree_elem);
   }
}

/****** sge_subordinate_qmaster/check_new_slotwise_subordinate_tree() **********
*  NAME
*     check_new_slotwise_subordinate_tree() -- checks if the new slotwise
*        preemption configuration is valid
*
*  SYNOPSIS
*     bool check_new_slotwise_subordinate_tree(lListElem *qinstance, lList 
*     *new_so_list, lList **answer_list) 
*
*  FUNCTION
*     Checks if the slotwise preemption configuration would still be valid if
*     a specific change would be made.
*
*  INPUTS
*     lListElem *qinstance - For this qinstance, the "subordinate_list"
*                            configuration value is to be changed.
*     lList *new_so_list   - These configuration will be added to the existing
*                            one.
*     lList **answer_list  - answer list for errors.
*
*  RESULT
*     bool - true if the new config will be valid, false otherwise.
*
*  NOTES
*     MT-NOTE: check_new_slotwise_subordinate_tree() is MT safe 
*******************************************************************************/
bool
check_new_slotwise_subordinate_tree(lListElem *qinstance, lList *new_so_list, lList **answer_list)
{
   bool          success = true;
   lListElem     *root_qinstance = NULL; /* QU_Type */

   DENTER(TOP_LAYER, "check_new_slotwise_subordinate_tree");
   /*
    * Check if the queues that will be added to the "subordinate_list" are
    * already in the subordinate tree. If they are, there is a loop in the
    * tree, which is not allowed.
    */
   root_qinstance = get_slotwise_sos_tree_root(qinstance);
   if (root_qinstance != NULL) {
      u_long32      slots_sum = 0;

      slots_sum = get_slotwise_sos_threshold(root_qinstance);
      if (slots_sum > 0) {
         sge_sl_list_t    *qinstances_in_slotwise_sos_tree = NULL;
         sge_sl_elem_t    *ssos_qinstance_elem = NULL;
         ssos_qinstance_t *ssos_qinstance = NULL;
         const char       *cqueue_name = NULL;
         lListElem        *new_so = NULL; /* SO_Type */

         /* get the slotwise sos tree as a list */
         get_slotwise_sos_sub_tree_qinstances(root_qinstance,
            &qinstances_in_slotwise_sos_tree, 0);

         for_each(new_so, new_so_list) {
            const char *new_so_name = NULL;

            new_so_name = lGetString(new_so, SO_name);
            if (new_so_name != NULL) {
               for_each_sl(ssos_qinstance_elem, qinstances_in_slotwise_sos_tree) {
                  ssos_qinstance = sge_sl_elem_data(ssos_qinstance_elem);
                  cqueue_name = lGetString(ssos_qinstance->qinstance, QU_qname);

                  if (strcmp(cqueue_name, new_so_name) == 0) {
                     /*
                      * If this queue will be subordinated to "qinstance",
                      * there will be a loop in the tree -> error!
                      */
                     ERROR((SGE_EVENT, MSG_PARSE_LOOP_IN_SSOS_TREE_SS,
                        new_so_name, lGetString(qinstance, QU_qname)));
                     answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX,
                        ANSWER_QUALITY_ERROR);
                     success = false;
                     break;
                  }
               }
            }
            if (success == false) {
               break;
            }
         }
         sge_sl_destroy(&qinstances_in_slotwise_sos_tree,
            (sge_sl_destroy_f)destroy_slotwise_sos_tree_elem);
      }
   }
   DRETURN(success);
}

/****** sge_subordinate_qmaster/do_slotwise_x_on_subordinate_check() ***********
*  NAME
*     do_slotwise_x_on_subordinate_check() -- 'calculates' and executes all
*        suspends and unsuspends for the slotwise subordination tree where the
*        provided qinstance is member of
*                                               
*
*  SYNOPSIS
*     bool do_slotwise_x_on_subordinate_check(sge_gdi_ctx_class_t *ctx, 
*     lListElem *qinstance, bool suspend, bool check_subtree_only, monitoring_t 
*     *monitor) 
*
*  FUNCTION
*     Calculates and executes all suspends and unsuspends for the tasks in the
*     qinstances on the same host as the provided qinstance for the slotwise
*     subordination tree where the provided qinstance is member of.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - context
*     lListElem *qinstance     - start from this qinstance doing this check
*     bool suspend             - calculate for suspend or for unsuspend?
*     bool check_subtree_only  - check only the subtree of the provided
*                                qinstance, not the whole slotwise subordinate
*                                tree the provided qinstance is member of
*     monitoring_t *monitor    - monitor
*
*  RESULT
*     bool - true if the provided qinstance is member of a slotwise subordinate
*            tree, false if it is not.
*
*  NOTES
*     MT-NOTE: do_slotwise_x_on_subordinate_check() is MT safe 
*******************************************************************************/
bool
do_slotwise_x_on_subordinate_check(sge_gdi_ctx_class_t *ctx, lListElem *qinstance,
      bool suspend, bool check_subtree_only, monitoring_t *monitor)
{
   sge_sl_list_t *qinstances_in_slotwise_sos_tree = NULL;
   lListElem     *super_qinstance = NULL;
   lListElem     *super_super = NULL;
   u_long32      running_jobs = 0;
   u_long32      slots_sum    = 0;

   if (check_subtree_only == false) {
      if (suspend == true) {
         /* Always check a sub tree from a tree node, don't do checking from 
          * a leaf node.
          */
         if (get_slotwise_sos_threshold(qinstance) == 0) {
            /* qinstance doesn't have a slotwise sos list defined, so it is a leaf node.
             * We begin searching at our parent queue instance.
             */
            super_qinstance = get_slotwise_sos_super_qinstance(qinstance);
         } else {
            super_qinstance = qinstance;
         }
      } else {
         super_qinstance = get_slotwise_sos_tree_root(qinstance);
      }
   } else {
      /*
       * Assume our qinstance is the root node of a slotwise subordination tree.
       */
      super_qinstance = qinstance;
   }

   if (super_qinstance == NULL) {
      return false; 
   }

   slots_sum = get_slotwise_sos_threshold(super_qinstance);
   if (slots_sum == 0) {
      /* no slotwise suspend on subordinate! */
      return false;
   }

   /* get the slotwise sos tree as a list */
   get_slotwise_sos_sub_tree_qinstances(super_qinstance, &qinstances_in_slotwise_sos_tree, 0);

   /* count the number and store informations about all running tasks in the list */
   running_jobs = count_running_jobs_in_slotwise_sos_tree(qinstances_in_slotwise_sos_tree, suspend);
   if ((suspend == true  && running_jobs > slots_sum) ||
       (suspend == false && running_jobs < slots_sum)) {
      bool   ret = false;
      int    diff = 0;
      /* we have to (un)suspend as many running/suspended jobs as new jobs
       * were scheduled/finished or (un)suspended by other ways.
       */
      diff = abs(running_jobs - slots_sum);
      do {
         /* suspend/unsuspend the highest/lowest running/suspended task */
         ret = x_most_extreme_task(ctx, qinstances_in_slotwise_sos_tree, suspend, monitor);
      } while (ret == true && (--diff) > 0);
   } 
   sge_sl_destroy(&qinstances_in_slotwise_sos_tree, (sge_sl_destroy_f)destroy_slotwise_sos_tree_elem);

   if (suspend == true && check_subtree_only == false) {
      /* Walk the tree from the leaves to the root */
      super_super = get_slotwise_sos_super_qinstance(super_qinstance);
      if (super_super != NULL) {
         do_slotwise_x_on_subordinate_check(ctx, super_super, suspend,
            check_subtree_only, monitor);
      }
   }
   return true;
}

/****** sge_subordinate_qmaster/do_slotwise_subordinate_lists_differ() *********
*  NAME
*     do_slotwise_subordinate_lists_differ() -- checks if the old and the new
*                                         subordinate list of this queue differ
*
*  SYNOPSIS
*     bool do_slotwise_subordinate_lists_differ(const lList* old_so_list, const 
*     lList *new_so_list) 
*
*  FUNCTION
*     Compares the old and the new subordinate list configured in a queue and
*     returnes true if they differ. The order of subordinates doesn't matter.
*
*  INPUTS
*     const lList* old_so_list - The old subordinate list. SO_Type list.
*     const lList *new_so_list - The new subordinate list. SO_Type list.
*
*  RESULT
*     bool - true if the lists differ, false if they are identical. If just the
*            order of subordinates is changed, the lists are considered as
*            identical.
*
*  NOTES
*     MT-NOTE: do_slotwise_subordinate_lists_differ() is MT safe 
*******************************************************************************/
bool do_slotwise_subordinate_lists_differ(const lList* old_so_list, const lList *new_so_list)
{
   bool      ret = false;
   lListElem *old_so = NULL; /* SO_Type */
   lListElem *new_so = NULL; /* SO_Type */

   for_each(old_so, old_so_list) {
      new_so = lGetElemStr(new_so_list, SO_name, lGetString(old_so, SO_name));
      /* find all queues that are in the old list but not in the new list */
      if (new_so == NULL) {
         ret = true;
         break;
      } else {
         /*
          * find all queues that are both in the old and in the new list,
          * but whose values were changed
          */
         u_long32 new_seq_no = lGetUlong(new_so, SO_seq_no);
         u_long32 new_action = lGetUlong(new_so, SO_action);
         u_long32 old_seq_no = lGetUlong(old_so, SO_seq_no);
         u_long32 old_action = lGetUlong(old_so, SO_action);

         if (old_seq_no != new_seq_no || old_action != new_action) {
            ret = true;
            break;
         }
      }
   }

   /* find all queues that are in the new list but weren't in the old list */
   if (ret == false) {
      for_each(new_so, new_so_list) {
         old_so = lGetElemStr(old_so_list, SO_name, lGetString(new_so, SO_name));
         if (old_so == NULL) {
            ret = true;
            break;
         }
      }
   }
   return ret;
}

/*
   (un)suspend on subordinate using granted_destination_identifier_list

   NOTE:
      we assume the associated job is already/still
      debited on all the queues that are referenced in gdil
*/
bool
cqueue_list_x_on_subordinate_gdil(sge_gdi_ctx_class_t *ctx,
                                  lList *this_list, bool suspend,
                                  const lList *gdil, monitoring_t *monitor)
{
   bool ret = true;
   lListElem *gdi = NULL;

   DENTER(TOP_LAYER, "cqueue_list_x_on_subordinate_gdil");

   for_each(gdi, gdil) {
      const char *full_name = lGetString(gdi, JG_qname);
      const char *hostname = lGetHost(gdi, JG_qhostname);
      lListElem *qinstance = cqueue_list_locate_qinstance(this_list, full_name);

      if (qinstance != NULL) {
         lList *so_list = lGetList(qinstance, QU_subordinate_list);
         lList *resolved_so_list = NULL;
         lListElem *so = NULL;
         u_long32 slots = lGetUlong(qinstance, QU_job_slots);
         u_long32 slots_used = qinstance_slots_used(qinstance);
         u_long32 slots_granted = lGetUlong(gdi, JG_slots);

         do_slotwise_x_on_subordinate_check(ctx, qinstance, suspend, false, monitor);
         /*
          * Only if this qinstance has not-slotwise subordinates
          * (SO_slots_sum == 0), we must check for queue wise subordination.
          */
         if (lGetSubUlong(qinstance, SO_slots_sum, 0, QU_subordinate_list) != NULL) {
            /* Do queue wise suspend on subordinate */
            /*
             * Resolve cluster queue names into qinstance names
             */
            so_list_resolve(so_list, NULL, &resolved_so_list, NULL, hostname);

            for_each(so, resolved_so_list) {
               const char *so_queue_name = lGetString(so, SO_name);
               
               /* We have to check this because so_list_resolve() didn't. */
               if (strcmp(full_name, so_queue_name) == 0) {
                  /* Queue can't be subordinate to itself. */
                  DPRINTF (("Removing circular reference.\n"));
                  continue;
               }
                  
               /*
                * suspend:
                *    no sos before this job came on this queue AND
                *    sos since job is on this queue
                *
                * unsuspend:
                *    no sos after job gone from this queue AND
                *    sos since job is on this queue
                */
               if (tst_sos(slots_used - slots_granted, slots, so) == false &&
                   tst_sos(slots_used, slots, so) == true) {
                  lListElem *so_queue =
                            cqueue_list_locate_qinstance(this_list, so_queue_name);

                  if (so_queue != NULL) {
                     /* Suspend/unsuspend the subordinated queue instance */
                     ret &= qinstance_x_on_subordinate(ctx, so_queue, suspend, true, monitor);
                     /* This change could also trigger slotwise (un)suspend on
                      * subordinate in related queue instances. If it was a
                      * queuewise suspend, it must be a slotwise unsuspend,
                      * and vice versa.
                      */
                     do_slotwise_x_on_subordinate_check(ctx, so_queue, (bool)!suspend, false, monitor);
                  } else {
                     ERROR((SGE_EVENT, MSG_QINSTANCE_NQIFOUND_SS, 
                            so_queue_name, SGE_FUNC));
                     ret = false;
                  }
               }
            }
            lFreeList(&resolved_so_list);
         }
      } else {
         /* should never happen */
         ERROR((SGE_EVENT, MSG_QINSTANCE_NQIFOUND_SS, full_name, SGE_FUNC));
         ret = false;
      } 
   }
   DRETURN(ret);
}

static bool
qinstance_x_on_subordinate(sge_gdi_ctx_class_t *ctx,
                           lListElem *this_elem, bool suspend,
                           bool send_event, monitoring_t *monitor)
{
   bool ret = true;
   u_long32 sos_counter;
   bool do_action;
   bool send_qinstance_signal;
   const char *hostname;
   const char *cqueue_name;
   const char *full_name;
   int signal;
   ev_event event;

   DENTER(TOP_LAYER, "qinstance_x_on_subordinate");

   /* increment sos counter */
   sos_counter = lGetUlong(this_elem, QU_suspended_on_subordinate);
   if (suspend) {
      sos_counter++;
   } else {
      sos_counter--;
   }
   lSetUlong(this_elem, QU_suspended_on_subordinate, sos_counter);

   /* 
    * prepare for operation
    *
    * suspend:  
    *    send a signal if it is not already suspended by admin or calendar 
    *
    * !suspend:
    *    send a signal if not still suspended by admin or calendar
    */
   hostname = lGetHost(this_elem, QU_qhostname);
   cqueue_name = lGetString(this_elem, QU_qname);
   full_name = lGetString(this_elem, QU_full_name);
   send_qinstance_signal = (qinstance_state_is_manual_suspended(this_elem) ||
                            qinstance_state_is_cal_suspended(this_elem)) ? false : true;
   if (suspend) {
      do_action = (sos_counter == 1) ? true : false;
      signal = SGE_SIGSTOP;
      event = sgeE_QINSTANCE_SOS;
   } else {
      do_action = (sos_counter == 0) ? true : false;
      signal = SGE_SIGCONT;
      event = sgeE_QINSTANCE_USOS;
   }

   /*
    * do operation
    */
   DPRINTF(("qinstance "SFQ" "SFN" "SFN" on subordinate\n", full_name,
            (do_action ? "" : "already"),
            (suspend ? "suspended" : "unsuspended")));
   if (do_action) {
      DPRINTF(("Due to other suspend states signal will %sbe delivered\n",
               send_qinstance_signal ? "NOT " : "")); 
      if (send_qinstance_signal) {
         ret = (sge_signal_queue(ctx, signal, this_elem, NULL, NULL, monitor) == 0) ? true : false;
      }

      sge_qmaster_qinstance_state_set_susp_on_sub(this_elem, suspend);
      if (send_event) {
         sge_add_event(0, event, 0, 0, cqueue_name, hostname, NULL, NULL);
      }
      lListElem_clear_changed_info(this_elem);
      /*
       * this queue instance was (un)suspended by queue wise suspend on subordinate,
       * now check if it has slotwise subordinates that must be handled.
       */
      do_slotwise_x_on_subordinate_check(ctx, this_elem, (bool)!suspend, false, monitor);
   }
   DRETURN(ret);
}

bool
cqueue_list_x_on_subordinate_so(sge_gdi_ctx_class_t *ctx,
                                lList *this_list, lList **answer_list,
                                bool suspend, const lList *resolved_so_list,
                                monitoring_t *monitor)
{
   bool ret = true;
   const lListElem *so = NULL;

   DENTER(TOP_LAYER, "cqueue_list_x_on_subordinate_qref");

   /*
    * Locate all qinstances which are mentioned in resolved_so_list and 
    * (un)suspend them
    */
   for_each(so, resolved_so_list) {
      const char *full_name = lGetString(so, SO_name);
      lListElem *qinstance = cqueue_list_locate_qinstance(this_list, full_name);

      if (qinstance != NULL) {
         ret &= qinstance_x_on_subordinate(ctx, qinstance, suspend,
                                           true, monitor);
         if (!ret) {
            break;
         }
      }
   }
   DRETURN(ret);
}

void
qinstance_find_suspended_subordinates(const lListElem *this_elem,
                                      lList **answer_list,
                                      lList **resolved_so_list)
{
   DENTER(TOP_LAYER, "qinstance_find_suspended_subordinates");
   
   if (this_elem != NULL && resolved_so_list != NULL) {
      /* Temporary storage for subordinates */
      lList *so_list = NULL;
      lListElem *so = NULL;
      lListElem *next_so = NULL;
      const char *hostname = NULL;
      /* Slots calculations */
      u_long32 slots = 0;
      u_long32 slots_used = 0;

      if (get_slotwise_sos_threshold(this_elem) == 0) {
         so_list = lGetList(this_elem, QU_subordinate_list);
         hostname = lGetHost(this_elem, QU_qhostname);
         slots = lGetUlong(this_elem, QU_job_slots);
         slots_used = qinstance_slots_used(this_elem);
         /*
          * Resolve cluster queue names into qinstance names
          */
         so_list_resolve(so_list, answer_list, resolved_so_list, NULL,
                         hostname);
         /* 
          * If the number of used slots on this qinstance is greater than a
          * subordinate's threshold (if it has one), this subordinate should
          * be suspended.
          *
          * Remove all subordinated queues from "resolved_so_list" which
          * are not actually suspended by "this_elem" 
          */
         DTRACE;
         next_so = lFirst(*resolved_so_list);
         while ((so = next_so) != NULL) {
            next_so = lNext(so);
            if (!tst_sos(slots_used, slots, so)) {
               DPRINTF (("Removing %s because it's not suspended\n",
                         lGetString (so, SO_name)));
               lRemoveElem(*resolved_so_list, &so);
            }
         }
      }
   }

   DRETURN_VOID;
}

bool
qinstance_initialize_sos_attr(sge_gdi_ctx_class_t *ctx, lListElem *this_elem, monitoring_t *monitor) 
{
   bool ret = true;
   lListElem *cqueue = NULL;
   lList *master_list = NULL;
   const char *full_name = NULL;
   const char *qinstance_name = NULL;
   const char *hostname = NULL;

   DENTER(TOP_LAYER, "qinstance_initialize_sos_attr");
   
   master_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
   full_name = lGetString(this_elem, QU_full_name);
   qinstance_name = lGetString(this_elem, QU_qname);
   hostname = lGetHost(this_elem, QU_qhostname);
   
   for_each(cqueue, master_list) {
      lListElem *qinstance = lGetSubHost(cqueue, QU_qhostname, hostname, CQ_qinstances);

      if (qinstance != NULL) {
         if (get_slotwise_sos_threshold(qinstance) > 0) {
            do_slotwise_x_on_subordinate_check(ctx, this_elem, true, false, monitor);
         } else {
            u_long32  slots = 0;
            u_long32  slots_used = 0;
            lListElem *so = NULL;
            lList     *resolved_so_list = NULL;
            lList     *so_list = lGetList(qinstance, QU_subordinate_list);

            /* queue instance-wise suspend on subordinate */
            slots = lGetUlong(qinstance, QU_job_slots);
            slots_used = qinstance_slots_used(qinstance);

            /*
             * Resolve cluster queue names into qinstance names
             */
            so_list_resolve(so_list, NULL, &resolved_so_list, qinstance_name,
                            hostname);

            for_each(so, resolved_so_list) {
               const char *so_full_name = lGetString(so, SO_name);

               if (!strcmp(full_name, so_full_name)) {
                  /* suspend the queue if neccessary */
                  if (tst_sos(slots_used, slots, so)) {
                     qinstance_x_on_subordinate(ctx, this_elem, true, false, monitor); 
                  }
               } 
            }
            lFreeList(&resolved_so_list);
         }
      }
   }
   DRETURN(ret);
}
