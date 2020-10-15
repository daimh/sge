#ifndef __SGE_SCHEDCONF_SC_L_H
#define __SGE_SCHEDCONF_SC_L_H

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

#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */ 

/* 
 * This is the list type we use to hold the configuration of the scheduler.
 */
enum {
   SC_algorithm = SC_LOWERBOUND,
   SC_schedule_interval,
   SC_maxujobs,
   SC_queue_sort_method,
   SC_job_load_adjustments,
   SC_load_adjustment_decay_time,
   SC_load_formula,
   SC_schedd_job_info,
   SC_flush_submit_sec,   
   SC_flush_finish_sec, 
   SC_params,
   
   SC_reprioritize_interval,
   SC_halftime,
   SC_usage_weight_list,
   SC_compensation_factor,
   SC_weight_user,
   SC_weight_project,
   SC_weight_department,
   SC_weight_job,
   SC_weight_tickets_functional,
   SC_weight_tickets_share,
   SC_weight_tickets_override,
   SC_share_override_tickets,  
   SC_share_functional_shares,  
   SC_max_functional_jobs_to_schedule, 
   SC_report_pjob_tickets,    
   SC_max_pending_tasks_per_job,  
   SC_halflife_decay_list, 
   SC_policy_hierarchy,
   SC_weight_ticket,
   SC_weight_waiting_time,
   SC_weight_deadline,
   SC_weight_urgency,
   SC_weight_priority,
   SC_max_reservation,
   SC_default_duration
   };

LISTDEF(SC_Type)
   JGDI_ROOT_OBJ(SchedConf, SGE_SC_LIST, MODIFY | GET)
   JGDI_EVENT_OBJ(MODIFY(sgeE_SCHED_CONF))

   SGE_STRING_D(SC_algorithm, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "default")
   SGE_STRING_D(SC_schedule_interval, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "0:0:15")
   SGE_ULONG_D(SC_maxujobs, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0)
   SGE_ULONG_D(SC_queue_sort_method, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0)    /* see at top of file for valid values */
   SGE_LIST(SC_job_load_adjustments, CE_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)        /* CE_Type */
   SGE_STRING_D(SC_load_adjustment_decay_time, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "0:7:30")
   SGE_STRING_D(SC_load_formula, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "np_load_avg")
   SGE_STRING_D(SC_schedd_job_info, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "true")
   SGE_ULONG_D(SC_flush_submit_sec, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0)            /* specifies the time after a job submit       *
                                                                         * to run the scheduler                        */
   SGE_ULONG_D(SC_flush_finish_sec, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0)            /* specifies the time after a job has finished *   
                                                                         * to run the scheduler                        */
   SGE_STRING_D(SC_params, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "none")

   SGE_STRING_D(SC_reprioritize_interval, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "0:0:0")
   SGE_ULONG_D(SC_halftime, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 168)
   SGE_MAP(SC_usage_weight_list, UA_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)   /* UA_Type; gives    *
                                                                         * weights for building the  * 
                                                                         * usage usage = cpu * w_cpu *
                                                                         * + xxx * w_xxx + ...       */
   SGE_DOUBLE_D(SC_compensation_factor, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 5.0)

   SGE_DOUBLE_D(SC_weight_user, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0.25)  /* gives weights between different *
                                                                                    * functional scheduling targets   */
   SGE_DOUBLE_D(SC_weight_project, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0.25)
   SGE_DOUBLE_D(SC_weight_department, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0.25)
   SGE_DOUBLE_D(SC_weight_job, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0.25)

   SGE_ULONG_D(SC_weight_tickets_functional, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0)      /* weight between different scheduling targets */
   SGE_ULONG_D(SC_weight_tickets_share, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0)
   SGE_ULONG_D(SC_weight_tickets_override, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0)
   SGE_BOOL(SC_share_override_tickets, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)          /* Override tickets of any object instance *
                                                                                             * are shared equally among all jobs       *
                                                                                             * associated with the object.             */
   SGE_BOOL(SC_share_functional_shares, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)         /* Functional shares of any object instance*
                                                                                             * are shared among all the jobs associated*
                                                                                             * with the object.                        */
   SGE_ULONG_D(SC_max_functional_jobs_to_schedule, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 200)   /* The maximum number of functional pending* 
                                                                                                       * jobs to scheduling using the brute-force* 
                                                                                                       * method.                                 */
   SGE_BOOL(SC_report_pjob_tickets, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)             /* report tickets to the qmaster or not    */ 
   SGE_ULONG_D(SC_max_pending_tasks_per_job, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 50)      /* The number of subtasks per pending job  *
                                                                                                   * to schedule. This parameter exists in   *
                                                                                                   * order to reduce overhead.               */
   SGE_STRING_D(SC_halflife_decay_list,CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "none")        /* A list of halflife decay values (UA_Type)*/
   SGE_STRING_D(SC_policy_hierarchy,CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "OFS")            /* defines the order of the ticket         *
                                                                                                   * computation                             */
   SGE_DOUBLE_D(SC_weight_ticket, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0.01)               /* weight in priority formula applied */
                                                                                                  /* on normalized ticket amount             */
   SGE_DOUBLE_D(SC_weight_waiting_time, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0.0)          /* weight applied in static urgency  *
                                                                                                   * formula on waiting time                 */
   SGE_DOUBLE_D(SC_weight_deadline, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 3600000.0)        /* dividend used in static urgency   *
                                                                                                   * formula with deadline initiation time   */
   SGE_DOUBLE_D(SC_weight_urgency, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0.1)               /* weight in priority formula applied *
                                                                                                   * on normalized urgency                   */
   SGE_DOUBLE_D(SC_weight_priority, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 1.0)              /* weight in priority formula applied *
                                                                                                   * on normalized posix priority */
   SGE_ULONG_D(SC_max_reservation, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 0)                 /* Maximum number of reservations within a *
                                                                                                   * schedule run. The value U_LONG32_MAX    *
                                                                                                   * stands for 'infinity'                   */
   SGE_STRING_D(SC_default_duration, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "0:10:0")        /* Default duration assumed for in         *
                                                                                                   * reseration scheduling mode for jobs     *
                                                                                                   * that specify no h_rt/s_rt. May not be   *
                                                                                                   * null if reservation is enabled */
LISTEND 

NAMEDEF(SCN)
   NAME("SC_algorithm")
   NAME("SC_schedule_interval")
   NAME("SC_maxujobs")
   NAME("SC_queue_sort_method")
   NAME("SC_job_load_adjustments")
   NAME("SC_load_adjustment_decay_time")
   NAME("SC_load_formula")
   NAME("SC_schedd_job_info")
   NAME("SC_flush_submit_sec")
   NAME("SC_flush_finish_sec")
   NAME("SC_params")
   
   NAME("SC_reprioritize_interval")
   NAME("SC_halftime")
   NAME("SC_usage_weight_list")
   NAME("SC_compensation_factor")

   NAME("SC_weight_user")
   NAME("SC_weight_project")
   NAME("SC_weight_department")
   NAME("SC_weight_job")

   NAME("SC_weight_tickets_functional")
   NAME("SC_weight_tickets_share")
   NAME("SC_weight_tickets_override")

   NAME("SC_share_override_tickets")
   NAME("SC_share_functional_shares")
   NAME("SC_max_functional_jobs_to_schedule")
   NAME("SC_report_pjob_tickets")
   NAME("SC_max_pending_tasks_per_job")
   NAME("SC_halflife_decay_list")
   NAME("SC_policy_hierarchy")

   NAME("SC_weight_ticket")
   NAME("SC_weight_waiting_time")
   NAME("SC_weight_deadline")
   NAME("SC_weight_urgency")
   NAME("SC_weight_priority")
   NAME("SC_max_reservation")
   NAME("SC_default_duration")
NAMEEND
#define SCS sizeof(SCN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_SCHEDCONFL_H */
