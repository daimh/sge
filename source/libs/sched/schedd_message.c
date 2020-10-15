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
#include <stdarg.h>
#include <stdio.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "cull/cull.h"

#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_schedd_conf.h"

#include "sge_message_SME_L.h"
#include "sge_message_MES_L.h"
#include "sge_ct_SCT_L.h"
#include "sge_ct_REF_L.h"
#include "sge_ct_CT_L.h"
#include "sge_ct_CCT_L.h"
#include "sge_ct_CTI_L.h"

#include "schedd_message.h"
#include "schedd_monitor.h"
#include "sge_schedd_text.h"
#include "msg_schedd.h"

static lList *schedd_mes_get_same_category_jids(lRef category,
                                                lList *job_list,
                                                int ignore_category);

static lRef schedd_mes_get_category(u_long32 job_id, lList *job_list);

static void schedd_mes_find_others(lListElem *tmp_sme, lList *job_list, int ignore_category)
{
   if (tmp_sme && job_list) {
      lListElem *message_elem = NULL;  /* MES_Type */
      lRef category = NULL;            /* Category pointer (void*) */
      lList *jid_cat_list = NULL;      /* ULNG */
      lList *message_list = lGetList(tmp_sme, SME_message_list);

      /*
       * Here we have a list of message elements where each 
       * MES_job_number_list contains only one id.
       * We have to find the other jobs (jids) which have the same category.
       */
      for_each(message_elem, message_list) {
         lList *jid_list = lGetList(message_elem, MES_job_number_list);
         u_long32 jid;
         lRef jid_category; 
         jid = lGetUlong(lFirst(jid_list), ULNG_value);
         jid_category = schedd_mes_get_category(jid, job_list);

         /*
          * Initialize jid_cat_list if not initialized
          * or if category differs from the last run
          */
         if (category != jid_category || ignore_category) {
            jid_cat_list = schedd_mes_get_same_category_jids(jid_category,
                                                             job_list,
                                                             ignore_category);
            category = jid_category;
            lSetList(message_elem, MES_job_number_list, jid_cat_list);
         } else {
            lSetList(message_elem, MES_job_number_list,
                     lCopyList("", jid_cat_list));
         }
      }
   }
}

static lRef schedd_mes_get_category(u_long32 job_id, lList *job_list)
{
   lListElem *job = NULL;  /* JB_Type */
   lRef ret = NULL;        /* Category pointer (void*) */

   DENTER(TOP_LAYER, "schedd_mes_get_category");
   job = lGetElemUlong(job_list, JB_job_number, job_id);
   if (job) {
      ret = lGetRef(job, JB_category);
   }
   DRETURN(ret);
}

static lList *schedd_mes_get_same_category_jids(lRef category,
                                                lList *job_list,
                                                int ignore_category)
{
   lList *ret = NULL;      /* ULNG */
   lListElem *job = NULL;  /* JB_Type */

   DENTER(TOP_LAYER, "schedd_mes_get_same_category_jids");
   if (job_list != NULL && category != NULL) {
      for_each(job, job_list) {
         if (ignore_category || lGetRef(job, JB_category) == category) {
            lAddElemUlong(&ret, ULNG_value, lGetUlong(job, JB_job_number), ULNG_Type);
         }
      }
   }
   DRETURN(ret);
}

/****** schedd/schedd_mes/schedd_mes_initialize() *****************************
*  NAME
*     schedd_mes_initialize() -- Initialize module variables 
*
*  SYNOPSIS
*     void schedd_mes_initialize(void) 
*
*  FUNCTION
*     Initialize module variables 
*******************************************************************************/
void schedd_mes_initialize(void)
{
   lListElem *sme = sconf_get_sme();
   lListElem *tmp_sme = sconf_get_tmp_sme();

   DENTER(TOP_LAYER, "schedd_mes_initialize");

   if (!sme) {
      lList *tmp_list;

      sme = lCreateElem(SME_Type);
      tmp_list = lCreateList("", MES_Type);
      lSetList(sme, SME_message_list, tmp_list);
      tmp_list = lCreateList("", MES_Type);
      lSetList(sme, SME_global_message_list, tmp_list);
      sconf_set_sme(sme);
   }

   /* prepare tmp_sme for collecting messages */
   if (!tmp_sme) {
      lList *tmp_list;

      tmp_sme = lCreateElem(SME_Type);
      tmp_list = lCreateList("", MES_Type);
      lSetList(tmp_sme, SME_message_list, tmp_list);
      sconf_set_tmp_sme(tmp_sme);
   }

   sconf_set_mes_schedd_info(true);
   schedd_mes_set_logging(1);

   DRETURN_VOID;
}

/****** schedd/schedd_mes/schedd_mes_commit() *********************************
*  NAME
*     schedd_mes_commit() -- Complete message elements and move them
*
*  SYNOPSIS
*     void schedd_mes_commit(lList *job_list, int ignore_category) 
*
*  FUNCTION
*     Each message contained in "tmp_sme" contains only
*     one job id. We have to find other jobs in "job_list" and
*     add the job ids to the list of ids contained in "tmp_sme"
*     message elements. After that we have to move all messages 
*     contained in "tmp_sme" into "sme".
*
*     If "ignore_category" is 1 than the job category will be ignored.
*     This means that all ids of "job_list" will be added to all
*     messages contained in "tmp_sme". 
*     
*     If no category is passed in and ignore_category is false, the messages
*     are only generated for the current job, meaning they are just copied.
*
*  INPUTS
*     lList *job_list     - JB_Type list 
*     int ignore_category - if set to true, the messages with be generated for all jobs
*                           in the list
*     lRef jid_category   - if not NULL, the function uses the category to ensure, that
*                           every message is only added per category once.
*******************************************************************************/
void schedd_mes_commit(lList *job_list, int ignore_category, lRef jid_category) {
   lListElem *sme = sconf_get_sme();
   lListElem *tmp_sme = sconf_get_tmp_sme();

   if (sme && tmp_sme) {
      lList *sme_mes_list = NULL;
      lList *tmp_sme_list = NULL;
      
      if (jid_category != NULL) {
         if (lGetBool(jid_category, CT_messages_added) ) {
            return;
         }
         lSetBool(jid_category, CT_messages_added, true);
      }

      /*
       * Try to find other jobs which apply also for created message
       */
      if (jid_category != NULL || ignore_category == 1) {
         schedd_mes_find_others(tmp_sme, job_list, ignore_category);
      }

      /*
       * Transfer all messages from tmp_sme to sme
       */
      sme_mes_list = lGetList(sme, SME_message_list);
      lXchgList(tmp_sme, SME_message_list, &tmp_sme_list);
      lAddList(sme_mes_list, &tmp_sme_list);
      tmp_sme_list = lCreateList("job info messages", MES_Type);
      lSetList(tmp_sme, SME_message_list, tmp_sme_list);
   }
}

/****** schedd/schedd_mes/schedd_mes_rollback() *******************************
*  NAME
*     schedd_mes_rollback() -- Free temporarily generated messages
*
*  SYNOPSIS
*     void schedd_mes_rollback(void) 
*
*  FUNCTION
*     Free temporarily generated messages contained in "tmp_sme".
******************************************************************************/
void schedd_mes_rollback(void)
{
   lListElem *tmp_sme = sconf_get_tmp_sme();
   
   if (tmp_sme) {
      lList *tmp_sme_list = NULL;

      tmp_sme_list = lCreateList("job info messages", MES_Type);
      lSetList(tmp_sme, SME_message_list, tmp_sme_list);
   }
}

/****** schedd/schedd_mes/schedd_mes_obtain_package() *************************
*  NAME
*     schedd_mes_obtain_package() -- Get message structure  
*
*  SYNOPSIS
*     lListElem *schedd_mes_obtain_packagevoid) 
*
*  FUNCTION
*     Returns message structure which contains all messages.
*
*  INPUTS
*     int *global_mes_count - out: returns number of global messages
*     int *job_mes_count    - out: returns number of job messages
*
*  NOTES
*     The calling function is responsible to free the returned
*     message structure if it is not needed anymore.
*
*  RESULT
*     lListElem* - SME_Type element
*******************************************************************************/
lListElem *schedd_mes_obtain_package(int *global_mes_count, int *job_mes_count)
{
   lListElem *ret;
   u_long32 schedd_job_info = sconf_get_schedd_job_info();
   lListElem *sme = sconf_get_sme();
   lListElem *tmp_sme = sconf_get_tmp_sme();

   DENTER(TOP_LAYER, "schedd_mes_obtain_package");

   if (schedd_job_info == SCHEDD_JOB_INFO_FALSE) {
      /*
       * Temporarily we enable schedd_job_info to add one
       * message which says that schedd_job_info is disabled. 
       */
      sconf_enable_schedd_job_info();
      schedd_mes_add_global(NULL, false, SCHEDD_INFO_TURNEDOFF);
      sconf_disable_schedd_job_info();
   } else if (schedd_job_info == SCHEDD_JOB_INFO_JOB_LIST) {
      schedd_mes_add_global(NULL, false, SCHEDD_INFO_JOBLIST);
   } else if (lGetNumberOfElem(lGetList(sme, SME_message_list))<1 &&
            lGetNumberOfElem(lGetList(sme, SME_global_message_list))<1) {
      schedd_mes_add_global(NULL, false, SCHEDD_INFO_NOMESSAGE);
   }

   if (global_mes_count != NULL) {
      *global_mes_count = lGetNumberOfElem(lGetList(sme, SME_global_message_list));
   }

   if (job_mes_count != NULL) {
      *job_mes_count = lGetNumberOfElem(lGetList(sme, SME_message_list));
   }

   ret = sme; /* calling function is responsible to free messages! */
   sconf_set_sme(NULL);
   lFreeElem(&tmp_sme);
   sconf_set_tmp_sme(NULL);

   sconf_set_mes_schedd_info(false);
   schedd_mes_set_logging(0);

   DRETURN(ret);
}

/****** schedd/schedd_mes/schedd_mes_add() ************************************
*  NAME
*     schedd_mes_add() -- Add one entry into the message structure.
*
*  SYNOPSIS
*     void schedd_mes_add(u_long32 job_number,
*                         u_long32 message_number,
*                         ...)
*
*  FUNCTION
*     During the time the scheduler tries to dispatch jobs it might
*     call this function to add messages into a temporary structure.
*     This function might be called several times. Each call
*     will add one element which contains one message describing
*     a reason why a job can't be dispatched and the jid concerned.
*
*     When it is clear if the job could be dispatched or not, one of
*     following functions has to be called:
*
*        schedd_mes_commit()
*        schedd_mes_rollback()
*
*  INPUTS
*     u_long32 job_number     - job id
*     u_long32 message_number - message number (sge_schedd_text.h)
*     ...                     - arguments for format string
*                               sge_schedd_text(message_number)
*
*  NOTES
*     MT-NOTE: schedd_mes_add() is MT safe
*
*  SEE ALSO
*     schedd/schedd_mes/schedd_mes_commit()
*     schedd/schedd_mes/schedd_mes_rollback()
*******************************************************************************/
void schedd_mes_add(lList **monitor_alpp, bool monitor_next_run, u_long32 job_id, u_long32 message_number, ...)
{
#ifndef WIN32NATIVE
   u_long32 schedd_job_info;

   DENTER(TOP_LAYER, "schedd_mes_add");

   schedd_job_info = sconf_get_schedd_job_info();

   if (monitor_alpp != NULL || monitor_next_run ||
       (job_id != 0 && schedd_job_info != SCHEDD_JOB_INFO_FALSE)) {
      va_list args;
      const char *fmt;

      char msg[MAXMSGLEN];
      dstring msg_ds;
      const char *msg_str;

      fmt = sge_schedd_text(message_number);
      va_start(args, message_number);
      sge_dstring_init(&msg_ds, msg, sizeof(msg));
      msg_str = sge_dstring_vsprintf(&msg_ds, fmt, args);
      va_end(args);

      if (monitor_alpp || monitor_next_run) {
         char msg_log[MAXMSGLEN];
         dstring msg_log_ds;
         const char *msg_log_str;

         sge_dstring_init(&msg_log_ds, msg_log, sizeof(msg_log));

         if (job_id) {
            msg_log_str = sge_dstring_sprintf(&msg_log_ds, "Job "sge_u32" %s", job_id, msg_str);
         } else {
            msg_log_str = sge_dstring_sprintf(&msg_log_ds, "Your job %s", msg_str);
         }
         schedd_log(msg_log_str, monitor_alpp, monitor_next_run);
      }

      if (!monitor_alpp) {
         if (job_id && (schedd_job_info != SCHEDD_JOB_INFO_FALSE)) {
            if (sconf_get_mes_schedd_info()) {
               lListElem *mes = NULL;
               lList *jobs_ulng = NULL;
               lListElem *jid_ulng;
               lListElem *tmp_sme = sconf_get_tmp_sme();

               if (schedd_job_info == SCHEDD_JOB_INFO_JOB_LIST) {
                  if (!range_list_is_id_within(sconf_get_schedd_job_info_range(),
                                               job_id)) {
                     DPRINTF(("Job "sge_u32" not in scheddconf.schedd_job_info_list\n", job_id));
                     DRETURN_VOID;
                  }
               }

               mes = lCreateElem(MES_Type);
               jobs_ulng = lCreateList("job ids", ULNG_Type);
               lSetList(mes, MES_job_number_list, jobs_ulng);
               lSetUlong(mes, MES_message_number, message_number);
               lSetString(mes, MES_message, msg_str);
               lAppendElem(lGetList(tmp_sme, SME_message_list), mes);

               jid_ulng = lCreateElem(ULNG_Type);
               lSetUlong(jid_ulng, ULNG_value, job_id);
               lAppendElem(jobs_ulng, jid_ulng);
            }
         }
      }
   }

   DRETURN_VOID;
#endif
}

/****** schedd_message/schedd_mes_add_join() ***********************************
*  NAME
*     schedd_mes_add_join() -- same as schedd_mes_add, but joins messages based
*                              on the message id.
*
*  SYNOPSIS
*     void schedd_mes_add_join(u_long32 job_number, u_long32 message_number,
*     ...)
*
*  FUNCTION
*     same as schedd_mes_add, but joins messages based
*     on the message id. But it only uses the temp message
*     list and not the global one.
*
*  INPUTS
*     u_long32 job_number     - job id
*     u_long32 message_number - message number (sge_schedd_text.h)
*     ...                     - arguments for format string
*                               sge_schedd_text(message_number)
*
*  NOTES
*     MT-NOTE: schedd_mes_add_join() is MT safe
*******************************************************************************/
void schedd_mes_add_join(bool monitor_next_run, u_long32 job_number, u_long32 message_number, ...)
{
#ifndef WIN32NATIVE
   u_long32 schedd_job_info;
   int do_logging;

   DENTER(TOP_LAYER, "schedd_mes_add_join");

   schedd_job_info = sconf_get_schedd_job_info();
   do_logging = schedd_mes_get_logging();

   if (do_logging || (job_number && (schedd_job_info != SCHEDD_JOB_INFO_FALSE))) {
      va_list args;
      const char *fmt;

      char msg[MAXMSGLEN];
      dstring msg_ds;
      const char *msg_str;

      char msg_log[MAXMSGLEN];
      dstring msg_log_ds;
      const char *msg_log_str;

      sge_dstring_init(&msg_ds, msg, sizeof(msg));
      sge_dstring_init(&msg_log_ds, msg_log, sizeof(msg_log));

      fmt = sge_schedd_text(message_number);
      va_start(args, message_number);
      msg_str = sge_dstring_vsprintf(&msg_ds, fmt, args);
      va_end(args);

      if (job_number && (schedd_job_info != SCHEDD_JOB_INFO_FALSE)) {
         if (sconf_get_mes_schedd_info()) {
            lListElem *tmp_sme = sconf_get_tmp_sme();
            lListElem *mes;
            lList *jobs_ulng = NULL;
            lListElem *jid_ulng;

            if (schedd_job_info == SCHEDD_JOB_INFO_JOB_LIST) {
               if (!sconf_is_id_in_schedd_job_info_range(job_number)) {
                  DPRINTF(("Job "sge_u32" not in scheddconf.schedd_job_info_list\n", job_number));
                  DRETURN_VOID;
               }
            }

            mes = lGetElemUlong(lGetList(tmp_sme, SME_message_list), MES_message_number, message_number);
            if (mes == NULL) {
               mes = lCreateElem(MES_Type);
               jobs_ulng = lCreateList("job ids", ULNG_Type);
               lSetList(mes, MES_job_number_list, jobs_ulng);
               lSetUlong(mes, MES_message_number, message_number);
               lSetString(mes, MES_message, msg_str);

               lAppendElem(lGetList(tmp_sme, SME_message_list), mes);
            } else {
               jobs_ulng = lGetList(mes, MES_job_number_list);
            }

            jid_ulng = lCreateElem(ULNG_Type);
            lSetUlong(jid_ulng, ULNG_value, job_number);
            lAppendElem(jobs_ulng, jid_ulng);
         }

         if (do_logging) {
            msg_log_str = sge_dstring_sprintf(&msg_log_ds, "Job "sge_u32" %s", job_number, msg_str);
            schedd_log(msg_log_str, NULL, monitor_next_run);
         }
      } else {
         if (do_logging) {
            if (job_number) {
               msg_log_str = sge_dstring_sprintf(&msg_log_ds, "Job "sge_u32" %s", job_number, msg_str);
            } else {
               msg_log_str = sge_dstring_sprintf(&msg_log_ds, "Your job %s", msg_str);
            }
            schedd_log(msg_log_str, NULL, monitor_next_run);
         }
      }
   }

   DRETURN_VOID;
#endif
}


/****** schedd/schedd_mes/schedd_mes_add_global() *************************
*  NAME
*     schedd_mes_add_global() -- add a global message
*
*  SYNOPSIS
*     void schedd_mes_add_global(u_long32 message_number, ...)
*
*  FUNCTION
*     Add a global message into a message structure.
*
*  INPUTS
*     u_long32 message_number - message number (sge_schedd_text.h) 
*     ...                     - arguments for format string
*                               sge_schedd_text(message_number) 
*
*  NOTES
*     MT-NOTE: schedd_mes_add_global() is MT safe
*******************************************************************************/
void schedd_mes_add_global(lList **monitor_alpp, bool monitor_next_run, u_long32 message_number, ...)
{

   DENTER(TOP_LAYER, "schedd_mes_add_global");

   if (monitor_alpp != NULL || monitor_next_run ||
       sconf_get_schedd_job_info() != SCHEDD_JOB_INFO_FALSE) {
      va_list args;
      const char *fmt;

      char msg[MAXMSGLEN];
      dstring msg_ds;
      const char *msg_str;

      /* Create error message */
      fmt = sge_schedd_text(message_number);
      va_start(args,message_number);

      sge_dstring_init(&msg_ds, msg, sizeof(msg));
      msg_str = sge_dstring_vsprintf(&msg_ds, fmt, args);

      if (!monitor_alpp && sconf_get_schedd_job_info() != SCHEDD_JOB_INFO_FALSE) {
         lListElem *sme = sconf_get_sme();

         if (sme != NULL) {
            lListElem *mes = lCreateElem(MES_Type);
            lSetUlong(mes, MES_message_number, message_number);
            lSetString(mes, MES_message, msg_str);
            lAppendElem(lGetList(sme, SME_global_message_list), mes);
         }
      }

      /* Write entry into log file */
      schedd_log(msg_str, monitor_alpp, monitor_next_run);
   }

   DRETURN_VOID;
}


/****** schedd_message/schedd_mes_set_tmp_list() *******************************
*  NAME
*     schedd_mes_set_tmp_list() -- sets the messages for a current job 
*
*  SYNOPSIS
*     void schedd_mes_set_tmp_list(lListElem *category, int name, int name, u_long32 job_number) 
*
*  FUNCTION
*     Takes a message list, changes the job number to the current job and stores
*     the list. 
*
*  INPUTS
*     lListElem *category - an object, which stores the list 
*     int name            - element id for the list
*     u_long32 job_number - job number 
*
*******************************************************************************/
void schedd_mes_set_tmp_list(lListElem *category, int name, u_long32 job_number)
{
   lListElem *tmp_sme = sconf_get_tmp_sme();
   lList *tmp_list = NULL;
   lListElem *tmp_elem;

   DENTER(TOP_LAYER, "schedd_mes_set_tmp_list");

   lXchgList(category, name, &tmp_list);

   for_each(tmp_elem, tmp_list)
      lAddSubUlong(tmp_elem, ULNG_value, job_number, MES_job_number_list, ULNG_Type);

   if (tmp_sme && tmp_list) {
      lList *prev = NULL;
      lXchgList(tmp_sme, SME_message_list, &prev);
      lAddList(tmp_list, &prev);
      lSetList(tmp_sme, SME_message_list, tmp_list);
   }

   DRETURN_VOID;
}
