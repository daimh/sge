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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "uti/sge_rmon.h"
#include "uti/setup_path.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h"
#include "uti/sge_spool.h"
#include "uti/sge_stdio.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"
#include "uti/sge_time.h"
#include "uti/sge_unistd.h"
#include "uti/sge_lock.h"
#include "uti/sge_mtutil.h"

#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_resource_utilization_RUE_L.h"

#include "sched/sge_resource_utilization.h"
#include "sched/sge_sharetree_printing.h"

#include "sge_rusage.h"
#include "sge_reporting_qmaster.h"

#include "category.h"
#include "msg_common.h"
#include "msg_qmaster.h"


/* do not change, the ":" is hard coded into the qacct file
   parsing routines. */
static const char REPORTING_DELIMITER = ':';

typedef enum {
   ACCOUNTING_BUFFER = 0,
   REPORTING_BUFFER
} rep_buf;

typedef struct {
   dstring buffer;
   pthread_mutex_t mtx;
   const char *mtx_name;
} rep_buf_t;

static rep_buf_t reporting_buffer[2] = {
   { DSTRING_INIT, PTHREAD_MUTEX_INITIALIZER, "mtx_accounting" },
   { DSTRING_INIT, PTHREAD_MUTEX_INITIALIZER, "mtx_reporting" }
};

static bool 
reporting_flush_accounting(const char *acct_file, lList **answer_list);

static bool 
reporting_flush_reporting(const char *reporting_file, lList **answer_list);

static bool 
reporting_flush_report_file(lList **answer_list,
                            const char *filename, rep_buf_t *buf);

static bool 
reporting_flush(const char *acct_file, const char *reporting_file, lList **answer_list);

static bool 
reporting_create_record(lList **answer_list, 
                        const char *type,
                        const char *data);

static bool
reporting_create_sharelog_record(lList **answer_list, monitoring_t *monitor);

static bool
reporting_write_load_values(lList **answer_list, dstring *buffer, 
                            const lList *load_list, const lList *variables);

static const char *
reporting_get_job_log_name(const job_log_t type);

static const char *
lGetStringNotNull(const lListElem *ep, int nm);

static void 
config_sharelog(void);

static bool 
intermediate_usage_written(const lListElem *job_report, 
                           const lListElem *ja_task);

static bool
reporting_create_ar_acct_record(lList **answer_list,
                                const lListElem *ar,
                                const char *cqueue_name,
                                const char *hostname,
                                u_long32 slots,
                                u_long32 report_time);

/****** qmaster/reporting/--Introduction ***************************
*  NAME
*     qmaster reporting -- creation of a reporting file 
*
*  FUNCTION
*     This module provides a set of functions that are used to write
*     the SGE reporting file and the accounting file.
*
*     See the manpages accounting.5 and qacct.1 for information about the
*     accounting file.
*
*     The reporting file contains entries for queues, hosts, accounting
*     and sharetree usage.
*     It can for example be used to transfer SGE data useful for reporting
*     and analysis purposes to a database.
*
*  SEE ALSO
*     qmaster/reporting/reporting_initialize()
*     qmaster/reporting/reporting_shutdown()
*     qmaster/reporting/reporting_trigger_handler()
*     qmaster/reporting/reporting_create_acct_record()
*     qmaster/reporting/reporting_create_host_record()
*     qmaster/reporting/reporting_create_host_consumable_record()
*     qmaster/reporting/reporting_create_queue_record()
*     qmaster/reporting/reporting_create_queue_consumable_record()
*     qmaster/reporting/reporting_create_sharelog_record()
*******************************************************************************/

/* ------------- public functions ------------- */
/****** qmaster/reporting/reporting_initialize() ***************************
*  NAME
*     reporting_initialize() -- initialize the reporting module
*
*  SYNOPSIS
*     bool reporting_initialize(lList **answer_list) 
*
*  FUNCTION
*     Register reporting and sharelog trigger as well as the respective event
*     handler.
*
*  INPUTS
*     lList **answer_list - used to return error messages
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: reporting_initialize() is MT safe.
*
*  SEE ALSO
*     qmaster/reporting/reporting_shutdown()
*     Timeeventmanager/te_add()
*******************************************************************************/
bool
reporting_initialize(lList **answer_list)
{
   bool ret = true;
   te_event_t ev = NULL;

   DENTER(TOP_LAYER, "reporting_initialize");

   te_register_event_handler(reporting_trigger_handler, TYPE_REPORTING_TRIGGER);
   te_register_event_handler(reporting_trigger_handler, TYPE_ACCOUNTING_TRIGGER);
   te_register_event_handler(reporting_trigger_handler, TYPE_SHARELOG_TRIGGER);

   /* we always have the reporting trigger for flushing reporting files and
    * checking for new reporting configuration
    */
   ev = te_new_event(time(NULL), TYPE_REPORTING_TRIGGER, ONE_TIME_EVENT, 1, 0, NULL);
   te_add_event(ev);
   te_free_event(&ev);

   /* we have the accounting trigger for flushing accounting files only when not
    * doing immediate flushing.
    */
   if (mconf_get_accounting_flush_time() != 0) {
      ev = te_new_event(time(NULL), TYPE_ACCOUNTING_TRIGGER, ONE_TIME_EVENT, 1, 0, NULL);
      te_add_event(ev);
      te_free_event(&ev);
   }

   /* the sharelog timed events can be switched on or off */
   config_sharelog();

   DRETURN(ret);
} /* reporting_initialize() */

/****** qmaster/reporting/reporting_shutdown() *****************************
*  NAME
*     reporting_shutdown() -- shutdown the reporting module
*
*  SYNOPSIS
*     bool reporting_shutdown(lList **answer_list, bool do_spool) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list - used to return error messages
*     bool do_spool       - if set to true changes must be spooled
*                           if set to false don't spool changes
*
*  RESULT
*     bool - true on success, false on error.
*
*  NOTES
*     MT-NOTE: reporting_shutdown() is MT safe
*
*  SEE ALSO
*     qmaster/reporting/reporting_initialize()
*******************************************************************************/
bool
reporting_shutdown(sge_gdi_ctx_class_t *ctx, lList **answer_list, bool do_spool)
{
   bool ret = true;
   lList* alp = NULL;
   rep_buf_t *buf;
   const char *reporting_file = ctx->get_reporting_file(ctx);
   const char *acct_file = ctx->get_acct_file(ctx);

   DENTER(TOP_LAYER, "reporting_shutdown");

   if (do_spool == true) {
      /* flush the last reporting values, suppress adding new timer */
      if (!reporting_flush(acct_file, reporting_file, &alp)) {
         answer_list_output(&alp);
      }
   }

   buf = &reporting_buffer[ACCOUNTING_BUFFER];
   /* free memory of buffers */
   sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   sge_dstring_free(&(buf->buffer));
   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));

   buf = &reporting_buffer[REPORTING_BUFFER];
   sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   sge_dstring_free(&(buf->buffer));
   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   
   DRETURN(ret);
}

/****** qmaster/reporting/reporting_trigger_handler() **********************
*  NAME
*     reporting_trigger_handler() -- process timed event
*
*  SYNOPSIS
*     void 
*     reporting_trigger_handler(te_event_t anEvent)
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     te_event_t - timed event
*
*  NOTES
*     MT-NOTE: reporting_trigger_handler() is MT safe.
*
*  SEE ALSO
*     Timeeventmanager/te_deliver()
*     Timeeventmanager/te_add()
*******************************************************************************/
void
reporting_trigger_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   u_long32 flush_interval = 0;
   lList *answer_list = NULL;
   const char *reporting_file = ctx->get_reporting_file(ctx);
   const char *acct_file = ctx->get_acct_file(ctx);

   DENTER(TOP_LAYER, "reporting_trigger_handler");

   config_sharelog();

   switch (te_get_type(anEvent)) {
      case TYPE_SHARELOG_TRIGGER:
         /* dump sharetree usage and flush reporting file */
         if (!reporting_create_sharelog_record(&answer_list, monitor)) {
            answer_list_output(&answer_list);
         }
         flush_interval = mconf_get_sharelog_time();
         
         /* flush the reporting data */
         if (reporting_flush_reporting(reporting_file, &answer_list) == 0) {
            answer_list_output(&answer_list);
         }
         
         break;
      case TYPE_REPORTING_TRIGGER:
         /* only flush reporting file */
         flush_interval = mconf_get_reporting_flush_time();
         
         /* flush the reporting data */
         if (reporting_flush_reporting(reporting_file, &answer_list) == 0) {
            answer_list_output(&answer_list);
         }
         
         break;
      case TYPE_ACCOUNTING_TRIGGER:
         /* only flush accounting file */
         flush_interval = mconf_get_accounting_flush_time();
         
         /* Flush the accounting data.  There's need to worry about
          * multi-threading issues with immediate flushisg, because the
          * reporting_flush_report_file() function uses a mutex and checks the
          * buffer length before trying to flush. */
         if (reporting_flush_accounting(acct_file, &answer_list) == 0) {
            answer_list_output(&answer_list);
         }
         
         break;
      default:
         DRETURN_VOID;
   }

   /* set next flushing interval and add timer.
    * flush_interval can be 0, if sharelog is switched off or accounting or
    * reporting flush time is immediate, then don't add trigger 
    */
   if (flush_interval > 0) {
      te_event_t ev = NULL;
      time_t next_flush = (time_t)(time(NULL) + flush_interval);

      ev = te_new_event(next_flush, te_get_type(anEvent), ONE_TIME_EVENT, 1, 0,
                        NULL);
      te_add_event(ev);
      te_free_event(&ev);
   }

   DRETURN_VOID;
} /* reporting_trigger_handler() */

bool
reporting_create_new_job_record(lList **answer_list, const lListElem *job)
{
   bool ret = true;

   DENTER(TOP_LAYER, "reporting_create_new_job_record");

   if (mconf_get_do_reporting() && mconf_get_do_joblog() && job != NULL) {
      dstring job_dstring = DSTRING_INIT;

      u_long32 job_number, priority, submission_time;
      const char *job_name, *owner, *group, *project, *department, *account;

      job_number        = lGetUlong(job, JB_job_number);
      priority          = lGetUlong(job, JB_priority);
      submission_time   = lGetUlong(job, JB_submission_time);
      job_name          = lGetStringNotNull(job, JB_job_name);
      owner             = lGetStringNotNull(job, JB_owner);
      group             = lGetStringNotNull(job, JB_group);
      project           = lGetStringNotNull(job, JB_project);
      department        = lGetStringNotNull(job, JB_department);
      account           = lGetStringNotNull(job, JB_account);

      sge_dstring_sprintf(&job_dstring, 
                          sge_U32CFormat"%c"
                          sge_U32CFormat"%c"
                          "%d%c"
                          "%s%c"
                          "%s%c"
                          "%s%c"
                          "%s%c"
                          "%s%c"
                          "%s%c"
                          "%s%c"
                          sge_U32CFormat
                          "\n", 
                          sge_u32c(submission_time), REPORTING_DELIMITER,
                          sge_u32c(job_number), REPORTING_DELIMITER,
                          -1, REPORTING_DELIMITER, /* means: no ja_task yet */
                          NONE_STR, REPORTING_DELIMITER,
                          job_name, REPORTING_DELIMITER,
                          owner, REPORTING_DELIMITER,
                          group, REPORTING_DELIMITER,
                          project, REPORTING_DELIMITER,
                          department, REPORTING_DELIMITER,
                          account, REPORTING_DELIMITER,
                          sge_u32c(priority));

      /* write record to reporting buffer */
      ret = reporting_create_record(answer_list, "new_job", 
                                    sge_dstring_get_string(&job_dstring));
      sge_dstring_free(&job_dstring);
   }

   DRETURN(ret);
}

bool 
reporting_create_job_log(lList **answer_list,
                         u_long32 event_time,
                         const job_log_t type,
                         const char *user,
                         const char *host,
                         const lListElem *job_report,
                         const lListElem *job, const lListElem *ja_task,
                         const lListElem *pe_task,
                         const char *message)
{
   bool ret = true;

   DENTER(TOP_LAYER, "reporting_create_job_log");

   if (mconf_get_do_reporting() && mconf_get_do_joblog() && job != NULL) {
      dstring job_dstring = DSTRING_INIT;

      u_long32 job_id = 0;
      int ja_task_id = -1;
      const char *pe_task_id = NONE_STR;
      u_long32 state_time = 0, jstate;
      const char *event;
      char state[20];
      u_long32 priority, submission_time;
      const char *job_name, *owner, *group, *project, *department, *account;

      job_id = lGetUlong(job, JB_job_number);

      /* set ja_task_id: 
       * -1, if we don't have a ja_task
       *  0, if we have a non array job
       *  task_number for array jobs
       */
      if (ja_task != NULL) {
         if (job_is_array(job)) {
            ja_task_id = (int)lGetUlong(ja_task, JAT_task_number);
         } else {
            ja_task_id = 0;
         }
      }
      if (pe_task != NULL) {
         pe_task_id = lGetStringNotNull(pe_task, PET_id);
      }

      /* JG: TODO: implement the whole job/task state mess */
      if (pe_task != NULL) {
         jstate = lGetUlong(pe_task, PET_status);
      } else if (ja_task != NULL) {
         jstate = lGetUlong(ja_task, JAT_status);
      } else {
         jstate = 0;
      }

      event = reporting_get_job_log_name(type);

      *state = '\0';
      job_get_state_string(state, jstate);
      if (message == NULL) {
         message = "";
      }

      priority          = lGetUlong(job, JB_priority);
      submission_time   = lGetUlong(job, JB_submission_time);
      job_name          = lGetStringNotNull(job, JB_job_name);
      owner             = lGetStringNotNull(job, JB_owner);
      group             = lGetStringNotNull(job, JB_group);
      project           = lGetStringNotNull(job, JB_project);
      department        = lGetStringNotNull(job, JB_department);
      account           = lGetStringNotNull(job, JB_account);

      sge_dstring_sprintf(&job_dstring, sge_U32CFormat"%c%s%c"sge_U32CFormat"%c%d%c%s%c%s%c%s%c%s%c"sge_U32CFormat"%c"sge_U32CFormat"%c"sge_U32CFormat"%c%s%c%s%c%s%c%s%c%s%c%s%c%s\n", 
                          sge_u32c(event_time), REPORTING_DELIMITER,
                          event, REPORTING_DELIMITER,
                          sge_u32c(job_id), REPORTING_DELIMITER,
                          ja_task_id, REPORTING_DELIMITER,
                          pe_task_id, REPORTING_DELIMITER,
                          state, REPORTING_DELIMITER,
                          user, REPORTING_DELIMITER,
                          host, REPORTING_DELIMITER,
                          sge_u32c(state_time), REPORTING_DELIMITER,
                          sge_u32c(priority), REPORTING_DELIMITER,
                          sge_u32c(submission_time), REPORTING_DELIMITER,
                          job_name, REPORTING_DELIMITER,
                          owner, REPORTING_DELIMITER,
                          group, REPORTING_DELIMITER,
                          project, REPORTING_DELIMITER,
                          department, REPORTING_DELIMITER,
                          account, REPORTING_DELIMITER,
                          message);
      /* write record to reporting buffer */
      DPRINTF(("%s", sge_dstring_get_string(&job_dstring)));
      ret = reporting_create_record(answer_list, "job_log", 
                                    sge_dstring_get_string(&job_dstring));
      sge_dstring_free(&job_dstring);
   }

   DRETURN(ret);
}

/****** qmaster/reporting/reporting_create_acct_record() *******************
*  NAME
*     reporting_create_acct_record() -- create an accounting record
*
*  SYNOPSIS
*     bool reporting_create_acct_record(lList **answer_list, 
*                                       lListElem *job_report, 
*                                       lListElem *job, lListElem *ja_task) 
*
*  FUNCTION
*     Create an accounting record.
*     Depending on the cluster configuration parameter reporting_params,
*     accounting is written to the accounting file and/or the reporting file.
*     
*     During the runtime of jobs, intermediate accounting records can be written
*     to the reporting file. This is usually done at midnight, to have correct
*     daily accounting information for long running jobs.
*
*  INPUTS
*     lList **answer_list   - used to report error messages
*     lListElem *job_report - job report from execd
*     lListElem *job        - job referenced in report
*     lListElem *ja_task    - array task that finished
*     bool intermediate     - is this an intermediate accounting record?
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: reporting_create_acct_record() is not MT safe as the
*              MT safety of called functions sge_build_job_category and
*              sge_write_rusage is not defined.
*******************************************************************************/
/* JG: TODO: we should also pass pe_task. It is known in the code pieces where
 * reporting_create_acct_record is called and we needn't search it from ja_task
 */
bool
reporting_create_acct_record(sge_gdi_ctx_class_t *ctx,
                       lList **answer_list, 
                       lListElem *job_report, 
                       lListElem *job, lListElem *ja_task, bool intermediate)
{
   bool ret = true;
   char category_buffer[MAX_STRING_SIZE];
   dstring category_dstring;
   dstring job_dstring = DSTRING_INIT;
   const char *category_string = NULL; 
   const char *job_string = NULL;
   bool do_reporting  = mconf_get_do_reporting();
   bool do_accounting = mconf_get_do_accounting();
   const char *acct_file = ctx->get_acct_file(ctx);

   DENTER(TOP_LAYER, "reporting_create_acct_record");

   /* anything to do at all? */
   if (do_reporting || do_accounting) {
   
      sge_dstring_init(&category_dstring, category_buffer, 
                       sizeof(category_buffer));

      sge_build_job_category_dstring(&category_dstring, job, 
                                     *(userset_list_get_master_list()), *object_type_get_master_list(SGE_TYPE_PROJECT), NULL,
                                     *object_type_get_master_list(SGE_TYPE_RQS));
      category_string = sge_dstring_get_string(&category_dstring);                                          

      /* accounting records will only be written at job end, not for intermediate
       * reports
       */
      if (do_accounting && !intermediate) {
         job_string = sge_write_rusage(&job_dstring, job_report, job, ja_task, 
                                       category_string, REPORTING_DELIMITER, 
                                       false);
         if (job_string == NULL) {
            ret = false;
         } else {
            /* write accounting file */
            rep_buf_t *buf = &reporting_buffer[ACCOUNTING_BUFFER];
            sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
            sge_dstring_append(&(buf->buffer), job_string);
            sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
         }
   
         /* If the flush internal is set to 0, flush the accounting buffer after
          * every write. */
         if (mconf_get_accounting_flush_time() == 0) {
            ret = (reporting_flush_accounting(acct_file, answer_list) != 0) ? true : false;
         }
      }

      /* reporting records will be written both for intermediate and final
       * job reports
       */
      if (ret && do_reporting) {
         /* job_dstring might have been filled with accounting record - this one
          * contains total accounting values.
          * If we have written intermediate accounting records earlier, or this
          * call will write intermediate accounting, we have to create our own 
          * accounting record.
          * Otherwise (final accounting record, no intermediate acct done before),
          * we can reuse the accounting record.
          */
         bool intermediate_written = intermediate_usage_written(job_report, ja_task);
         bool do_intermediate = (intermediate_written || intermediate) ? true : false;

         if (job_string == NULL || do_intermediate) {
            sge_dstring_clear(&job_dstring);
            job_string = sge_write_rusage(&job_dstring, job_report, job, ja_task, 
                                          category_string, REPORTING_DELIMITER,
                                          do_intermediate);
         }
         if (job_string == NULL) {
            ret = false;
         } else {
            ret = reporting_create_record(answer_list, "acct", job_string);
         }
      }
   }

   sge_dstring_free(&job_dstring);

   DRETURN(ret);
}

/****** qmaster/reporting/reporting_write_consumables() ********************
*  NAME
*     reporting_write_consumables() -- dump consumables to a buffer
*
*  SYNOPSIS
*     static bool 
*     reporting_write_consumables(lList **answer_list, dstring *buffer, 
*                                 const lList *actual, const lList *total
*                                 const lListElem *host, 
*                                 const lListElem *job) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     dstring *buffer       - target buffer
*     const lList *actual   - actual consumable values
*     const lList *total    - configured consumable values
*     const lListElem *host - host for which to output data
*     const lListElem *job  - optional: job which changes consumables
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: reporting_write_consumables() is MT safe
*******************************************************************************/
static bool
reporting_write_consumables(lList **answer_list, dstring *buffer,
                            const lList *actual, const lList *total,
                            const lListElem *host, const lListElem *job)
{
   bool ret = true;
   lListElem *cep; 
   
   DENTER(TOP_LAYER, "reporting_write_consumables");

   for_each (cep, actual) {
      const char *name = lGetString(cep, RUE_name);
      bool log_variable = true;

      /* 
       * if log_consumables == false, lookup if the consumable shall be logged
       * due to reporting_variables in global/local host
       */
      if (mconf_get_log_consumables() == false) {
         const lList *report_variables = lGetList(host, EH_merged_report_variables);
         if (lGetElemStr(report_variables, STU_name, name) == NULL) {
            log_variable = false;
         } else {
            /*
             * if we log consumables for a specific job, make sure to log only
             * consumables, which are requested by the job
             * slots is an implicit request - always log it if requested
             */
            if (strcmp(name, "slots") != 0 && job != NULL) {
               if (job_get_request(job, name) == NULL) {
                  log_variable = false;
               }
            }
         }
      }

      /* now do the logging, if requested */
      if (log_variable == true) {
         lListElem *tep = lGetElemStr(total, CE_name, name);
         if (tep != NULL) {
            sge_dstring_append(buffer, name);
            sge_dstring_append_char(buffer, '=');
            utilization_print_to_dstring(cep, buffer);
            sge_dstring_append_char(buffer, '=');
            centry_print_resource_to_dstring(tep, buffer);

            if (lNext(cep) != NULL) {
               sge_dstring_append_char(buffer, ','); 
            }
         }
      }
   }

   DRETURN(ret);
}

/****** qmaster/reporting/reporting_create_queue_record() *******************
*  NAME
*     reporting_create_queue_record() -- create queue reporting record
*
*  SYNOPSIS
*     bool 
*     reporting_create_queue_record(lList **answer_list, 
*                                  const lListElem *queue, 
*                                  u_long32 report_time) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     const lListElem *queue - the queue to output
*     u_long32 report_time  - time of the last load report
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: reporting_create_queue_record() is MT safe
*******************************************************************************/
bool
reporting_create_queue_record(lList **answer_list,
                             const lListElem *queue,
                             u_long32 report_time)
{
   bool ret = true;

   DENTER(TOP_LAYER, "reporting_create_queue_record");

   if (mconf_get_do_reporting() && queue != NULL) {
      dstring queue_dstring = DSTRING_INIT;

      sge_dstring_sprintf(&queue_dstring, "%s%c%s%c"sge_U32CFormat"%c", 
                          lGetString(queue, QU_qname),
                          REPORTING_DELIMITER,
                          lGetHost(queue, QU_qhostname),
                          REPORTING_DELIMITER,
                          sge_u32c(report_time),
                          REPORTING_DELIMITER);
      qinstance_state_append_to_dstring(queue, &queue_dstring);
      sge_dstring_append_char(&queue_dstring, '\n');

      /* write record to reporting buffer */
      ret = reporting_create_record(answer_list, "queue", 
                                    sge_dstring_get_string(&queue_dstring));

      sge_dstring_free(&queue_dstring);
   }

   DRETURN(ret);
}

/****** qmaster/reporting/reporting_create_queue_consumable_record() ********
*  NAME
*     reporting_create_queue_consumable_record() -- write queue consumables
*
*  SYNOPSIS
*     bool 
*     reporting_create_queue_consumable_record(lList **answer_list, 
*                                              const lListElem *host, 
*                                              const lListElem *queue, 
*                                              const lListElem *job, 
*                                              u_long32 report_time) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list    - used to return error messages
*     const lListElem *host  - host on which the qinstance is located
*     const lListElem *queue - queue instance to output
*     const lListElem *job   - optional: job which changes consumables
*     u_long32 report_time   - time when consumables changed
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: reporting_create_queue_consumable_record() is MT safe
*******************************************************************************/
bool
reporting_create_queue_consumable_record(lList **answer_list,
                                         const lListElem *host,
                                         const lListElem *queue,
                                         const lListElem *job,
                                         u_long32 report_time)
{
   bool ret = true;

   DENTER(TOP_LAYER, "reporting_create_queue_consumable_record");

   if (mconf_get_do_reporting() && host != NULL && queue != NULL) {
      dstring consumable_dstring = DSTRING_INIT;

      /* dump consumables */
      reporting_write_consumables(answer_list, &consumable_dstring, 
                                  lGetList(queue, QU_resource_utilization), 
                                  lGetList(queue, QU_consumable_config_list),
                                  host, job);

      if (sge_dstring_strlen(&consumable_dstring) > 0) {
         dstring queue_dstring = DSTRING_INIT;
         sge_dstring_sprintf(&queue_dstring, "%s%c%s%c"sge_U32CFormat"%c", 
                             lGetString(queue, QU_qname),
                             REPORTING_DELIMITER,
                             lGetHost(queue, QU_qhostname),
                             REPORTING_DELIMITER,
                             sge_u32c(report_time),
                             REPORTING_DELIMITER);
         qinstance_state_append_to_dstring(queue, &queue_dstring);
         sge_dstring_sprintf_append(&queue_dstring, "%c%s\n",
                                REPORTING_DELIMITER,
                                sge_dstring_get_string(&consumable_dstring));

         /* write record to reporting buffer */
         ret = reporting_create_record(answer_list, "queue_consumable", 
                                       sge_dstring_get_string(&queue_dstring));
         sge_dstring_free(&queue_dstring);
      }

      sge_dstring_free(&consumable_dstring);
   }


   DRETURN(ret);
}

/****** qmaster/reporting/reporting_create_host_record() *******************
*  NAME
*     reporting_create_host_record() -- create host reporting record
*
*  SYNOPSIS
*     bool 
*     reporting_create_host_record(lList **answer_list, 
*                                  const lListElem *host, 
*                                  u_long32 report_time) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     const lListElem *host - the host to output
*     u_long32 report_time  - time of the last load report
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: reporting_create_host_record() is MT safe
*******************************************************************************/
bool
reporting_create_host_record(lList **answer_list,
                             const lListElem *host,
                             u_long32 report_time)
{
   bool ret = true;

   DENTER(TOP_LAYER, "reporting_create_host_record");

   if (mconf_get_do_reporting() && host != NULL) {
      dstring load_dstring = DSTRING_INIT;

      reporting_write_load_values(answer_list, &load_dstring, 
                                  lGetList(host, EH_load_list), 
                                  lGetList(host, EH_merged_report_variables));

      /* As long as we have no host status information, dump host data only if we have
       * load values to report.
       */
      if (sge_dstring_strlen(&load_dstring) > 0) {
         dstring host_dstring = DSTRING_INIT;
         sge_dstring_sprintf(&host_dstring, "%s%c"sge_U32CFormat"%c%s%c%s\n", 
                             lGetHost(host, EH_name), REPORTING_DELIMITER,
                             sge_u32c(report_time), REPORTING_DELIMITER,
                             "X", REPORTING_DELIMITER,
                             sge_dstring_get_string(&load_dstring));
         /* write record to reporting buffer */
         ret = reporting_create_record(answer_list, "host", 
                                       sge_dstring_get_string(&host_dstring));

         sge_dstring_free(&host_dstring);
      }

      sge_dstring_free(&load_dstring);
   }

   DRETURN(ret);
}

/****** qmaster/reporting/reporting_create_host_consumable_record() ********
*  NAME
*     reporting_create_host_consumable_record() -- write host consumables
*
*  SYNOPSIS
*     bool 
*     reporting_create_host_consumable_record(lList **answer_list, 
*                                             const lListElem *host, 
*                                             const lListElem *job, 
*                                             u_long32 report_time) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     const lListElem *host - host to output
*     const lListElem *job  - optional: job which changes consumables
*     u_long32 report_time  - time when consumables changed
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: reporting_create_host_consumable_record() is MT safe
*******************************************************************************/
bool
reporting_create_host_consumable_record(lList **answer_list,
                                        const lListElem *host,
                                        const lListElem *job,
                                        u_long32 report_time)
{
   bool ret = true;

   DENTER(TOP_LAYER, "reporting_create_host_consumable_record");

   if (mconf_get_do_reporting() && host != NULL) {
      dstring consumable_dstring = DSTRING_INIT;

      /* dump consumables */
      reporting_write_consumables(answer_list, &consumable_dstring, 
                                  lGetList(host, EH_resource_utilization), 
                                  lGetList(host, EH_consumable_config_list),
                                  host, job);

      if (sge_dstring_strlen(&consumable_dstring) > 0) {
         dstring host_dstring = DSTRING_INIT;

         sge_dstring_sprintf(&host_dstring, "%s%c"sge_U32CFormat"%c%s%c%s\n", 
                             lGetHost(host, EH_name), REPORTING_DELIMITER,
                             sge_u32c(report_time), REPORTING_DELIMITER,
                             "X", REPORTING_DELIMITER,
                             sge_dstring_get_string(&consumable_dstring));


         /* write record to reporting buffer */
         ret = reporting_create_record(answer_list, "host_consumable", 
                                       sge_dstring_get_string(&host_dstring));
         sge_dstring_free(&host_dstring);
      }

      sge_dstring_free(&consumable_dstring);
   }

   DRETURN(ret);
}

/****** qmaster/reporting/reporting_create_sharelog_record() ***************
*  NAME
*     reporting_create_sharelog_record() -- dump sharetree usage
*
*  SYNOPSIS
*     bool 
*     reporting_create_sharelog_record(lList **answer_list) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList **answer_list   - used to return error messages
*     monitoring_t *monitor - monitors the use of the global lock
*
*  RESULT
*     bool -  true on success, false on error
*
*  NOTES
*     MT-NOTE: reporting_create_sharelog_record() is most probably MT safe
*              (depends on sge_sharetree_print with uncertain MT safety)
*******************************************************************************/
static bool
reporting_create_sharelog_record(lList **answer_list, monitoring_t *monitor)
{
   bool ret = true;
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "reporting_create_sharelog_record");

   if (mconf_get_do_reporting() && mconf_get_sharelog_time() > 0) {
      /* only create sharelog entries if we have a sharetree */
      if (lGetNumberOfElem(*object_base[SGE_TYPE_SHARETREE].list) > 0) {
         rep_buf_t *buf;
         dstring prefix_dstring = DSTRING_INIT;
         dstring data_dstring   = DSTRING_INIT;
         format_t format;
         char delim[2];
         delim[0] = REPORTING_DELIMITER;
         delim[1] = '\0';

         /* we need a prefix containing the reporting file std fields */
         sge_dstring_sprintf(&prefix_dstring, sge_U32CFormat"%csharelog%c",
                             sge_u32c(sge_get_gmt()),
                             REPORTING_DELIMITER, REPORTING_DELIMITER);

         /* define output format */
         format.name_format  = false;
         format.delim        = delim;
         format.line_delim   = "\n";
         format.rec_delim    = "";
         format.str_format   = "%s";
         format.field_names  = NULL;
         format.format_times = false;
         format.line_prefix  = sge_dstring_get_string(&prefix_dstring);

         /* dump the sharetree data */
         MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_READ), monitor);

         sge_sharetree_print(&data_dstring, *object_base[SGE_TYPE_SHARETREE].list, 
                             *object_base[SGE_TYPE_USER].list,
                             *object_base[SGE_TYPE_PROJECT].list,
                             *object_base[SGE_TYPE_USERSET].list,
                             true,
                             false,
                             NULL,
                             &format);

         SGE_UNLOCK(LOCK_GLOBAL, LOCK_READ);

         /* write data to reporting buffer */
         buf = &reporting_buffer[REPORTING_BUFFER];
         sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
         sge_dstring_append(&(buf->buffer),
                            sge_dstring_get_string(&data_dstring));
         sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));

         /* cleanup */
         sge_dstring_free(&prefix_dstring);
         sge_dstring_free(&data_dstring);
      }
   }

   DRETURN(ret);
}


/****** qmaster/reporting/reporting_is_intermediate_acct_required() ********
*  NAME
*     reporting_is_intermediate_acct_required() -- write intermed. acct record? 
*
*  SYNOPSIS
*     bool 
*     reporting_is_intermediate_acct_required(const lListElem *job, 
*                                             const lListElem *ja_task, 
*                                             const lListElem *pe_task) 
*
*  FUNCTION
*     Checks if it is necessary to write an intermediate accounting record
*     for the given ja_task or pe_task.
*
*     An intermediate accounting record is written
*        - reporting is activated at all.
*        - when the first usage record for a job is received after midnight.
*        - the job hasn't just started some seconds before midnight.
*          This is an optimization to limit the number of intermediate 
*          accounting records in troughput clusters with short job runtimes.
*          The minimum runtime required for an intermediate record to be written
*          is defined in INTERMEDIATE_MIN_RUNTIME.
*
*     A further optimization has been done: To reduce the overhead caused by
*     this function (called with every job report), the check will only be done
*     in a time window starting at midnight. The length of the time window is 
*     defined in INTERMEDIATE_ACCT_WINDOW.
*
*  INPUTS
*     const lListElem *job     - job
*     const lListElem *ja_task - array task
*     const lListElem *pe_task - optionally parallel task
*
*  RESULT
*     bool - true, if writing of an intermediate accounting record is required,
*            else false.
*
*  NOTES
*     MT-NOTE: reporting_is_intermediate_acct_required() is MT safe 
*
*  SEE ALSO
*     qmaster/reporting/reporting_create_acct_record()
*******************************************************************************/
bool
reporting_is_intermediate_acct_required(const lListElem *job, 
                                        const lListElem *ja_task, 
                                        const lListElem *pe_task)
{
   bool ret = false;
   time_t last_intermediate, now, start_time;
   struct tm tm_last_intermediate, tm_now;


   DENTER(TOP_LAYER, "reporting_is_intermediate_acct_required");

   /* if reporting isn't active, we needn't write intermediate usage */
   if (!mconf_get_do_reporting()) {
      DRETURN(false);
   }

   /* valid input data? */
   if (job == NULL || ja_task == NULL) {
      /* JG: TODO: i18N */
      WARNING((SGE_EVENT, "reporting_is_intermediate_acct_required: invalid input data"));
      DRETURN(false);
   }

   /* 
    * optimization: only do the following actions "shortly after midnight" 
    */
   now = (time_t)sge_get_gmt();
   localtime_r(&now, &tm_now);
#if 1
   if (tm_now.tm_hour != 0 || tm_now.tm_min > INTERMEDIATE_ACCT_WINDOW) {
      DRETURN(false);
   }
#endif

   /* 
    * optimization: do not write intermediate usage for jobs that just 
    * "started a short time before"
    */
   if (pe_task != NULL) {
      start_time = (time_t)lGetUlong(pe_task, PET_start_time);
   } else {
      start_time = (time_t)lGetUlong(ja_task, JAT_start_time);
   }

   if ((now - start_time) < (INTERMEDIATE_MIN_RUNTIME + tm_now.tm_min * 60 + 
                   tm_now.tm_sec)) {
      DRETURN(false);
   }

   /* 
    * try to read time of an earlier intermediate report
    * if no intermediate report has been written so far, use start time 
    */
   if (pe_task != NULL) {
      last_intermediate = (time_t)usage_list_get_ulong_usage(
                             lGetList(pe_task, PET_reported_usage), 
                             LAST_INTERMEDIATE, 0);
   } else {
      last_intermediate = (time_t)usage_list_get_ulong_usage(
                             lGetList(ja_task, JAT_reported_usage_list), 
                             LAST_INTERMEDIATE, 0);
   }

   if (last_intermediate == 0) {
      last_intermediate = start_time;
   }

   /* compare day portion of last_intermediate vs. current time 
    * if day changed, we have to write an intermediate report
    */
   localtime_r(&last_intermediate, &tm_last_intermediate);
   /* new day? */
   if (
#if 0 /* for development and debugging: write intermediate data every hour */
       tm_last_intermediate.tm_hour < tm_now.tm_hour ||
#endif
       tm_last_intermediate.tm_yday < tm_now.tm_yday ||
       tm_last_intermediate.tm_year < tm_now.tm_year
       ) {
       char buffer[100];
       char timebuffer[100];
       dstring buffer_dstring;
       sge_dstring_init(&buffer_dstring, buffer, sizeof(buffer));
       INFO((SGE_EVENT, MSG_REPORTING_INTERMEDIATE_SS, 
             job_get_key(lGetUlong(job, JB_job_number),
                         lGetUlong(ja_task, JAT_task_number),
                         pe_task != NULL ? lGetString(pe_task, PET_id) 
                                         : NULL,
                         &buffer_dstring),
           asctime_r(&tm_now, timebuffer)));
       ret = true;
   }

   DRETURN(ret);
}

/* ----- static functions ----- */

/*
* NOTES
*     MT-NOTE: reporting_write_load_values() is MT-safe
*/
static bool
reporting_write_load_values(lList **answer_list, dstring *buffer, 
                            const lList *load_list, const lList *variables)
{
   bool ret = true;
   bool first = true;
   const lListElem *variable;

   DENTER(TOP_LAYER, "reporting_write_load_values");

   for_each (variable, variables) {
      const char *name;
      const lListElem *load;

      name = lGetString(variable, STU_name);
      load = lGetElemStr(load_list, HL_name, name);
      if (load != NULL) {
         if (first) {
            first = false;
         } else {
            sge_dstring_append_char(buffer, ',');
         }
         sge_dstring_sprintf_append(buffer, "%s=%s", 
                                    name, lGetString(load, HL_value));
      }

   }

   DRETURN(ret);
}

/*
* NOTES
*     MT-NOTE: reporting_create_record() is MT-safe
*/
static bool 
reporting_create_record(lList **answer_list, 
                        const char *type,
                        const char *data)
{
   bool ret = true;
   rep_buf_t *buf = &reporting_buffer[REPORTING_BUFFER];

   DENTER(TOP_LAYER, "reporting_create_record");

   sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   sge_dstring_sprintf_append(&(buf->buffer), sge_U32CFormat"%c%s%c%s",
                              sge_u32c(sge_get_gmt()),
                              REPORTING_DELIMITER,
                              type,
                              REPORTING_DELIMITER,
                              data);
   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));

   DRETURN(ret);
}

/****** qmaster/reporting_flush_accounting() ***********************************
*  NAME
*     reporting_flush_accounting() -- flush the accounting info
*
*  SYNOPSIS
*     bool reporting_flush_accounting(lList **answer_list)
*
*  FUNCTION
*     Flush the information in the accounting buffer into the accounting file.
*
*  INPUTS
*     const char* acct_file  - accounting file name
*     lList **answer_list    - answer list
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_flush_accounting() is MT-safe
*******************************************************************************/
static bool reporting_flush_accounting(const char *acct_file, lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "sge_flush_accounting");

   /* write accounting data */
   ret = reporting_flush_report_file(answer_list, acct_file,
                                     &reporting_buffer[ACCOUNTING_BUFFER]);
   DRETURN(ret);
}

/****** qmaster/reporting_flush_reporting() ***********************************
*  NAME
*     reporting_flush_reporting() -- flush the reporting info
*
*  SYNOPSIS
*     bool reporting_flush_reporting(lList **answer_list)
*
*  FUNCTION
*     Flush the information in the reporting buffer into the reporting file.
*
*  INPUTS
*     lList **answer_list  - answer list
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_flush_reporting() is MT-safe
*******************************************************************************/
static bool reporting_flush_reporting(const char *reporting_file, lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "sge_flush_reporting");
   /* write reporting data */
   ret = reporting_flush_report_file(answer_list,
                                     reporting_file,
                                     &reporting_buffer[REPORTING_BUFFER]);
   DRETURN(ret);
}

/****** qmaster/reporting_flush_report_file() **********************************
*  NAME
*     reporting_flush_report_file() -- flush the buffer into the given file
*
*  SYNOPSIS
*     bool reporting_flush_report_file(lList **answer_list,
*                                      const char *filename, rep_buf_t *buf)
*
*  FUNCTION
*     Flush the information in the given buffer into the specified file.
*
*  INPUTS
*     lList **answer_list  - answer list
*     const char *filename - name of the destination file
*     rep_buf_t *buf       - target buffer
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_flush_report_file() is MT-safe
*******************************************************************************/
static bool reporting_flush_report_file(lList **answer_list,
                                        const char *filename, rep_buf_t *buf)
{
   bool ret = true;
   size_t size;
   char error_buffer[MAX_STRING_SIZE];
   dstring error_dstring;

   DENTER(TOP_LAYER, "reporting_flush_report_file");

   sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   size = sge_dstring_strlen(&(buf->buffer));
   sge_dstring_init(&error_dstring, error_buffer, sizeof(error_buffer));

   /* do we have anything to write? */ 
   if (size > 0) {
      FILE *fp;
      bool write_comment = false;
      SGE_STRUCT_STAT statbuf;


      /* if file doesn't exist: write a comment after creating it */
      if (SGE_STAT(filename, &statbuf)) {
         write_comment = true;
      }     

      /* open file for append */
      fp = fopen(filename, "a");
      if (fp == NULL) {
         if (answer_list == NULL) {
            ERROR((SGE_EVENT, MSG_ERROROPENINGFILEFORWRITING_SS, filename, 
                   sge_strerror(errno, &error_dstring)));
         } else {
            answer_list_add_sprintf(answer_list, STATUS_EDISK, 
                                    ANSWER_QUALITY_ERROR, 
                                    MSG_ERROROPENINGFILEFORWRITING_SS, filename, 
                                    sge_strerror(errno, &error_dstring));
         }

         ret = false;
      }

      /* write comment if necessary */
      if (ret) {
         if (write_comment) {
            int spool_ret;
            char version_buffer[MAX_STRING_SIZE];
            dstring version_dstring;
            const char *version_string;

            sge_dstring_init(&version_dstring, version_buffer, 
                             sizeof(version_buffer));
            version_string = feature_get_product_name(FS_VERSION, 
                                                      &version_dstring);

            spool_ret = sge_spoolmsg_write(fp, COMMENT_CHAR, version_string);
            if (spool_ret != 0) {
               if (answer_list == NULL) {
                  ERROR((SGE_EVENT, MSG_ERRORWRITINGFILE_SS, filename, 
                         sge_strerror(errno, &error_dstring)));
               } else {
                  answer_list_add_sprintf(answer_list, STATUS_EDISK, 
                                          ANSWER_QUALITY_ERROR, 
                                          MSG_ERRORWRITINGFILE_SS, filename, 
                                          sge_strerror(errno, &error_dstring));
               } 
               ret = false;
            }
         }
      }

      /* write data */
      if (ret) {
         if (fwrite(sge_dstring_get_string(&(buf->buffer)), size, 1, fp) != 1) {
            if (answer_list == NULL) {
               ERROR((SGE_EVENT, MSG_ERRORWRITINGFILE_SS, filename, 
                      sge_strerror(errno, &error_dstring)));
            } else {
               answer_list_add_sprintf(answer_list, STATUS_EDISK, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_ERRORWRITINGFILE_SS, filename, 
                                       sge_strerror(errno, &error_dstring));
            }

            ret = false;
         }
      }

      /* clear the buffer. We do this regardless of the result of
       * the writing command. Otherwise, if writing the report file failed
       * over a longer time period, the reporting buffer could grow endlessly.
       */
      sge_dstring_clear(&(buf->buffer));

      /* close file */
      if (fp != NULL) {
         FCLOSE(fp);
      }
   }

   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));

   DRETURN(ret);

FCLOSE_ERROR:
   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   if (answer_list == NULL) {
      ERROR((SGE_EVENT, MSG_ERRORCLOSINGFILE_SS, filename, 
             sge_strerror(errno, &error_dstring)));
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EDISK, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_ERRORCLOSINGFILE_SS, filename, 
                              sge_strerror(errno, &error_dstring));
   }

   DRETURN(false);
}

/****** qmaster/reporting_flush() **********************************************
*  NAME
*     reporting_flush() -- flush the buffers into files
*
*  SYNOPSIS
*     bool reporting_flush(lList **answer_list)
*
*  FUNCTION
*     Flush the information in the accounting and reporting buffers into the
*     into the appropriate files.
*
*  INPUTS
*     lList **answer_list  - answer list
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_flush() is MT-safe
*******************************************************************************/
static bool reporting_flush(const char *acct_file, const char *reporting_file, lList **answer_list)
{
   bool ret = true;
   bool reporting_ret;

   DENTER(TOP_LAYER, "reporting_flush");

   /* flush accounting data */
   reporting_ret = reporting_flush_accounting(acct_file, answer_list);
   if (!reporting_ret) {
      ret = false;
   }
     
   /* flush accounting data */
   reporting_ret = reporting_flush_reporting(reporting_file, answer_list);
   if (!reporting_ret) {
      ret = false;
   }
     
   DRETURN(ret);
}

/****** qmaster/reporting_create_new_ar_record() ******************************
*  NAME
*     reporting_create_new_ar_record() -- new ar record will be written
*
*  SYNOPSIS
*     bool
*     reporting_create_new_ar_record(lList **answer_list, 
*                                    const lListElem *ar,
*                                    u_long32 report_time) 
*
*  FUNCTION
*     Flushs the information that into the accounting file that a new
*     advance reservation has been created.
*
*  INPUTS
*     lList **answer_list  - answer list
*     const lListElem *ar  - the ar object which has been created
*     u_long32 report_time - the corresponding timestamp 
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_flush() is MT-safe
*******************************************************************************/
bool
reporting_create_new_ar_record(lList **answer_list, 
                               const lListElem *ar,
                               u_long32 report_time) 
{
   bool ret = true;
   rep_buf_t *buf = &reporting_buffer[REPORTING_BUFFER];
   const char *owner = lGetString(ar, AR_owner);

   DENTER(TOP_LAYER, "reporting_create_new_ar_record");

   /* if reporting isn't active, we needn't write intermediate usage */
   if (!mconf_get_do_reporting()) {
      DRETURN(false);
   }

   sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   sge_dstring_sprintf_append(&(buf->buffer), 
                              sge_U32CFormat"%c"
                              SFN"%c"
                              sge_U32CFormat"%c"
                              sge_U32CFormat"%c"
                              "%s\n",
                              sge_u32c(report_time), REPORTING_DELIMITER,
                              "new_ar", REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_submission_time)), REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_id)), REPORTING_DELIMITER,
                              (owner != NULL) ? owner : "");
   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));

   DRETURN(ret);
}

/****** qmaster/reporting_create_ar_attribute_record() ************************
*  NAME
*     reporting_create_ar_attribute_record() -- writes ar attributes  
*
*  SYNOPSIS
*     bool
*     reporting_create_ar_attribute_record(lList **answer_list,
*                                          const lListElem *ar,
*                                          u_long32 report_time)
*
*  FUNCTION
*     Writes advance reservation attributes into the reporting file.
*     This will be done whenever the ar settings change and when a new
*     ar object is created.
*
*  INPUTS
*     lList **answer_list  - answer list
*     const lListElem *ar  - the ar object which has been created
*     u_long32 report_time - the corresponding timestamp 
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_flush() is MT-safe
*******************************************************************************/
bool
reporting_create_ar_attribute_record(lList **answer_list,
                                     const lListElem *ar,
                                     u_long32 report_time)
{
   bool ret = true;
   rep_buf_t *buf = &reporting_buffer[REPORTING_BUFFER];
   const char *pe_name = NULL;
   const char *ar_name = NULL;
   const char *ar_account = NULL;
   dstring ar_granted_resources = DSTRING_INIT;

   DENTER(TOP_LAYER, "reporting_create_ar_attribute_record");

   /* if reporting isn't active, we needn't write intermediate usage */
   if (!mconf_get_do_reporting()) {
      DRETURN(false);
   }

   sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   pe_name = lGetString(ar, AR_pe);
   ar_name = lGetString(ar, AR_name);
   ar_account = lGetString(ar, AR_account);
   centry_list_append_to_dstring(lGetList(ar, AR_resource_list), &ar_granted_resources);
   sge_dstring_sprintf_append(&(buf->buffer), 
                              sge_U32CFormat"%c"
                              SFN"%c"
                              sge_U32CFormat"%c"   /* report_time */
                              sge_U32CFormat"%c"   /* AR_submission_time */
                              sge_U32CFormat"%c"   /* AR_id */
                              "%s%c"               /* AR_name */
                              "%s%c"               /* AR_account */
                              sge_U32CFormat"%c"   /* AR_start_time */
                              sge_U32CFormat"%c"   /* AR_end_time */
                              "%s%c"               /* AR_pe */
                              "%s\n",              /* granted resources */
                              sge_u32c(report_time), REPORTING_DELIMITER,
                              "ar_attr", REPORTING_DELIMITER,
                              sge_u32c(report_time), REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_submission_time)), REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_id)), REPORTING_DELIMITER,
                              (ar_name != NULL) ? ar_name : "", REPORTING_DELIMITER,
                              (ar_account != NULL) ? ar_account : "", REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_start_time)), REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_end_time)), REPORTING_DELIMITER,
                              (pe_name != NULL) ? pe_name : "", REPORTING_DELIMITER,
                              sge_dstring_get_string(&ar_granted_resources));
   sge_dstring_free(&ar_granted_resources);
   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));

   DRETURN(ret);
}

/****** qmaster/reporting_create_ar_log_record() ******************************
*  NAME
*     reporting_create_ar_log_record() -- writes status change info  
*
*  SYNOPSIS
*     bool
*     reporting_create_ar_log_record(lList **answer_list,
*                                    const lListElem *ar,
*                                    ar_state_event_t event,
*                                    const char *ar_description,
*                                    u_long32 report_time)
*
*  FUNCTION
*     Writes logging information into the reporting file whenever a status
*     change of an advance reservation occures
*
*  INPUTS
*     lList **answer_list  - answer list
*     const lListElem *ar  - the ar object which has been created
*     ar_state_event_t event  - the event if which caused the state change 
*     const char *ar_description  - a human readable description 
*     u_long32 report_time - the corresponding timestamp 
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_flush() is MT-safe
*******************************************************************************/
bool
reporting_create_ar_log_record(lList **answer_list,
                               const lListElem *ar,
                               ar_state_event_t event,
                               const char *ar_description,
                               u_long32 report_time)
{
   bool ret = true;
   rep_buf_t *buf = &reporting_buffer[REPORTING_BUFFER];
   dstring state_string = DSTRING_INIT;

   DENTER(TOP_LAYER, "reporting_create_ar_log_record");

   /* if reporting isn't active, we needn't write intermediate usage */
   if (!mconf_get_do_reporting()) {
      DRETURN(false);
   }

   ar_state2dstring((ar_state_t)lGetUlong(ar, AR_state), &state_string);
   sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   sge_dstring_sprintf_append(&(buf->buffer), 
                              sge_U32CFormat"%c"
                              SFN"%c"
                              sge_U32CFormat"%c"   /* report_time */
                              sge_U32CFormat"%c"   /* AR submission time */
                              sge_U32CFormat"%c"   /* AR_id */
                              "%s%c"               /* AR_state as string*/
                              "%s%c"               /* event as string*/
                              "%s\n",              /* message */
                              sge_u32c(report_time), REPORTING_DELIMITER,
                              "ar_log", REPORTING_DELIMITER,
                              sge_u32c(report_time), REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_submission_time)), REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_id)), REPORTING_DELIMITER,
                              sge_dstring_get_string(&state_string), REPORTING_DELIMITER,
                              ar_get_string_from_event(event), REPORTING_DELIMITER,
                              (ar_description != NULL) ? ar_description : "");
   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   sge_dstring_free(&state_string);
   DRETURN(ret);
}

/****** sge_reporting_qmaster/reporting_create_ar_acct_records() ***************
*  NAME
*     reporting_create_ar_acct_records() -- ar accounting records will be written
*
*  SYNOPSIS
*     bool reporting_create_ar_acct_records(lList **answer_list, const 
*     lListElem *ar, u_long32 report_time) 
*
*  FUNCTION
*     This records will be written for all reserved qinstance whenever an
*     advance reservation terminates.
*
*  INPUTS
*     lList **answer_list  - answer list
*     const lListElem *ar  - the ar object which has been created
*     u_long32 report_time - the corresponding timestamp
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_create_ar_acct_records() is MT safe 
*
*  SEE ALSO
*     qmaster/reporting_create_ar_acct_record()
*******************************************************************************/
bool reporting_create_ar_acct_records(lList **answer_list, const lListElem *ar, u_long32 report_time)
{
   lListElem *elem;
   bool ret = true;

   for_each(elem, lGetList(ar, AR_granted_slots)) {
      const char *queue_name = lGetString(elem, JG_qname);
      u_long32 slots = lGetUlong(elem, JG_slots);
      dstring cqueue_name = DSTRING_INIT;
      dstring host_or_hgroup = DSTRING_INIT;

      if (!cqueue_name_split(queue_name, &cqueue_name, &host_or_hgroup, NULL, 
                        NULL)) {
         ret = false;
         continue;
      }
     
      if (!reporting_create_ar_acct_record(NULL, ar,
                                      sge_dstring_get_string(&cqueue_name),
                                      sge_dstring_get_string(&host_or_hgroup),
                                      slots, report_time)) {
         ret = false;
      }

      sge_dstring_free(&cqueue_name);
      sge_dstring_free(&host_or_hgroup);
   } 

   return ret;
}

/****** qmaster/reporting_create_ar_acct_record() *****************************
*  NAME
*     reporting_create_ar_log_record() -- ar accounting record will be written 
*
*  SYNOPSIS
*     bool
*     reporting_create_ar_acct_record(lList **answer_list,
*                                     const lListElem *ar,
*                                     const char *cqueue_name,
*                                     const char *hostname,
*                                     u_long32 slots,
*                                     u_long32 report_time)
*
*  FUNCTION
*     This record will be written for every qinstance whenever an
*     advance reservation terminates.
*
*  INPUTS
*     lList **answer_list     - answer list
*     const lListElem *ar     - the ar object which has been created
*     const char *cqueue_name - cluster queue name
*     const char *hostname    - hostname of the qinstance
*     u_long32 slots          - number of reserved slots 
*     u_long32 report_time    - the corresponding timestamp 
*
*  RESULT
*     int - true  success
*           false failure
*
*  NOTES
*     MT-NOTE: reporting_flush() is MT-safe
*******************************************************************************/
static bool
reporting_create_ar_acct_record(lList **answer_list,
                                const lListElem *ar,
                                const char *cqueue_name,
                                const char *hostname,
                                u_long32 slots,
                                u_long32 report_time)
{
   bool ret = true;
   rep_buf_t *buf = &reporting_buffer[REPORTING_BUFFER];

   DENTER(TOP_LAYER, "reporting_create_ar_acct_record");

   /* if reporting isn't active, we needn't write intermediate usage */
   if (!mconf_get_do_reporting()) {
      DRETURN(false);
   }

   sge_mutex_lock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));
   sge_dstring_sprintf_append(&(buf->buffer), 
                              sge_U32CFormat"%c"
                              SFN"%c"
                              sge_U32CFormat"%c"   /* report_time */
                              sge_U32CFormat"%c"   /* AR_submission_time */
                              sge_U32CFormat"%c"   /* AR_id */
                              "%s%c"               /* cqueue */
                              "%s%c"               /* execution hostname */
                              sge_U32CFormat"\n",  /* number of slots */
                              sge_u32c(report_time), REPORTING_DELIMITER,
                              "ar_acct", REPORTING_DELIMITER,
                              sge_u32c(report_time), REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_submission_time)), REPORTING_DELIMITER,
                              sge_u32c(lGetUlong(ar, AR_id)), REPORTING_DELIMITER,
                              cqueue_name, REPORTING_DELIMITER,
                              hostname, REPORTING_DELIMITER,
                              slots);
   sge_mutex_unlock(buf->mtx_name, SGE_FUNC, __LINE__, &(buf->mtx));

   DRETURN(ret);
}

/*
* NOTES
*     MT-NOTE: reporting_get_job_log_name() is MT-safe
*/
static const char *
reporting_get_job_log_name(const job_log_t type)
{
   const char *ret;

   switch (type) {
      case JL_UNKNOWN:
         ret = MSG_JOBLOG_ACTION_UNKNOWN;
         break;
      case JL_PENDING:
         ret = MSG_JOBLOG_ACTION_PENDING;
         break;
      case JL_SENT:
         ret = MSG_JOBLOG_ACTION_SENT;
         break;
      case JL_RESENT:
         ret = MSG_JOBLOG_ACTION_RESENT;
         break;
      case JL_DELIVERED:
         ret = MSG_JOBLOG_ACTION_DELIVERED;
         break;
      case JL_RUNNING:
         ret = MSG_JOBLOG_ACTION_RUNNING;
         break;
      case JL_SUSPENDED:
         ret = MSG_JOBLOG_ACTION_SUSPENDED;
         break;
      case JL_UNSUSPENDED:
         ret = MSG_JOBLOG_ACTION_UNSUSPENDED;
         break;
      case JL_HELD:
         ret = MSG_JOBLOG_ACTION_HELD;
         break;
      case JL_RELEASED:
         ret = MSG_JOBLOG_ACTION_RELEASED;
         break;
      case JL_RESTART:
         ret = MSG_JOBLOG_ACTION_RESTART;
         break;
      case JL_MIGRATE:
         ret = MSG_JOBLOG_ACTION_MIGRATE;
         break;
      case JL_DELETED:
         ret = MSG_JOBLOG_ACTION_DELETED;
         break;
      case JL_FINISHED:
         ret = MSG_JOBLOG_ACTION_FINISHED;
         break;
      case JL_ERROR:
         ret = MSG_JOBLOG_ACTION_ERROR;
         break;
      default:
         ret = "!!!! unknown job state !!!!";
         break;
   }   

   return ret;
}

/*
* NOTES
*     MT-NOTE: MT-safety depends on lGetString (which has no MT-NOTE)
*              lGetStringNotNull() itself is MT-safe
*/
static const char *
lGetStringNotNull(const lListElem *ep, int nm)
{
   const char *ret = lGetString(ep, nm);
   if (ret == NULL) {
      ret = "";
   }
   return ret;
}

/*
* NOTES
*     MT-NOTE: config_sharelog() is MT-safe
*              Accessing the static boolean variable sharelog_running
*              is not breaking MT-safety.
*/
static void 
config_sharelog(void) {
   static bool sharelog_running = false;

   /* sharelog shall be written according to global config */
   if (mconf_get_sharelog_time() > 0) {
      /* if sharelog is not running: switch it on */
      if (!sharelog_running) {
         te_event_t ev = NULL;
         ev = te_new_event(time(NULL), TYPE_SHARELOG_TRIGGER , ONE_TIME_EVENT, 
                           1, 0, NULL);
         te_add_event(ev);
         te_free_event(&ev);
         sharelog_running = true;
      }
   } else {
      /* switch if off */
      if (sharelog_running) {
         te_delete_one_time_event(TYPE_SHARELOG_TRIGGER, 1, 0, NULL);
         sharelog_running = false;
      }
   }
}

/*
* NOTES
*     MT-NOTE: intermediate_usage_written() is MT-safe
*/
static bool 
intermediate_usage_written(const lListElem *job_report, 
                           const lListElem *ja_task) 
{
   bool ret = false;
   const char *pe_task_id;
   const lList *reported_usage = NULL;
  
   /* do we have a tightly integrated parallel task? */
   pe_task_id = lGetString(job_report, JR_pe_task_id_str);
   if (pe_task_id != NULL) {
      const lListElem *pe_task = lGetElemStr(lGetList(ja_task, JAT_task_list), 
                                             PET_id, pe_task_id);
      if (pe_task != NULL) {
         reported_usage = lGetList(pe_task, PET_reported_usage);
      }
   } else {
      reported_usage = lGetList(ja_task, JAT_reported_usage_list);
   }

   /* the reported usage list will be created when the first intermediate
    * acct record is written
    */
   if (reported_usage != NULL) {
      ret = true;
   }

   return ret;
}
