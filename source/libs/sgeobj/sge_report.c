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

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"

#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "execution_states.h"

#if 0
/****** sgeobj/report/job_report_print_usage() *******************************
*  NAME
*     job_report_print_usage() -- Print usage contained in job report 
*
*  SYNOPSIS
*     void job_report_print_usage(const lListElem *job_report, 
*                                 FILE *fp) 
*
*  FUNCTION
*     Print usage information conatines in "job_report". Print the 
*     information to the given file stream "fp" or as debug messages. 
*
*  INPUTS
*     const lListElem *job_report - JR_Type element 
*     FILE *fp                    - file stream or NULL 
******************************************************************************/
void job_report_print_usage(const lListElem *job_report, FILE *fp) 
{
   lListElem *uep;

   DENTER(TOP_LAYER, "job_report_print_usage");

   if (!job_report) {
      DEXIT;
      return;
   }

   for_each(uep, lGetList(job_report, JR_usage)) {
      if (fp) {
         fprintf(fp, "   \"%s\" =   %.99g\n", lGetString(uep, UA_name),
                 lGetDouble(uep, UA_value));
      } else {
         DPRINTF(("   \"%s\" =   %.99g\n", lGetString(uep, UA_name),
                  lGetDouble(uep, UA_value)));
      }
   }

   DEXIT;
   return;
}
#endif

/****** sgeobj/report/job_report_init_from_job() *****************************
*  NAME
*     job_report_init_from_job() -- initialize job report 
*
*  SYNOPSIS
*     void job_report_init_from_job(lListElem *job_report, 
*                                   const lListElem *job, 
*                                   const lListElem *ja_task, 
*                                   const lListElem *pe_task) 
*
*  FUNCTION
*     Initialize "job_report" from the attributes obtained from
*     "job", "ja_task" and "pe_task". 
*
*  INPUTS
*     lListElem *job_report    - JR_Type object
*     const lListElem *job     - JB_Type object
*     const lListElem *ja_task - JAT_Type object 
*     const lListElem *pe_task - PET_Type object 
******************************************************************************/
void job_report_init_from_job(lListElem *job_report, 
                              const lListElem *job, 
                              const lListElem *ja_task, 
                              const lListElem *pe_task) 
{
   u_long32 job_id = lGetUlong(job, JB_job_number);
   u_long32 ja_task_id = lGetUlong(ja_task, JAT_task_number);
   lListElem *queue = NULL;   /* QU_Type */

   DENTER(TOP_LAYER, "job_report_init_from_job");

   lSetUlong(job_report, JR_job_number, job_id);
   lSetUlong(job_report, JR_ja_task_number, ja_task_id);

   if (pe_task != NULL) {
      lSetString(job_report, JR_pe_task_id_str, lGetString(pe_task, PET_id));
      queue = lFirst(lGetList(pe_task, PET_granted_destin_identifier_list));
   } else {
      queue = lFirst(lGetList(ja_task, JAT_granted_destin_identifier_list));
   }

   if (lGetUlong(ja_task, JAT_status) == JSLAVE){
      if (pe_task == NULL) {
         lSetUlong(job_report, JR_state, JSLAVE);
      } else {
         lSetUlong(job_report, JR_state, JWRITTEN);
      }
   } else {
      lSetUlong(job_report, JR_state, JWRITTEN);
   }

   if (queue != NULL) {
      lSetString(job_report, JR_queue_name, lGetString(queue, JG_qname));
   }

   DEXIT;
}

void job_report_init_from_job_with_usage(lListElem *job_report,
                                         lListElem *job,
                                         lListElem *ja_task,
                                         lListElem *pe_task,
                                         u_long32 time_stamp)
{
   lListElem *ep;
   lListElem *obj;
   int nm;

   DENTER(TOP_LAYER, "job_report_init_from_job_with_usage");

   /*
    * initialize the job jeport like any other job report...
    */
   job_report_init_from_job(job_report, job, ja_task, pe_task);

   /*
    * ... and now add the online usage plus some fileds which are needed for the accounting
    */
   lSetUlong(job_report, JR_wait_status, SGE_SET_WEXITSTATUS(SGE_WEXITED_BIT, 0));
   lSetUlong(job_report, JR_failed, 0);

   if (pe_task == NULL) {
      nm = JAT_scaled_usage_list;
      obj = ja_task;
   } else {
      nm = PET_scaled_usage;
      obj = pe_task;

      lSetString(job_report, JR_pe_task_id_str, lGetString(pe_task, PET_id));
   }

   ep = lAddSubStr(obj, UA_name, "submission_time", nm, UA_Type);
   lSetDouble(ep, UA_value, lGetUlong(job, JB_submission_time));
   ep = lAddSubStr(obj, UA_name, "start_time", nm, UA_Type);
   lSetDouble(ep, UA_value, lGetUlong(ja_task, JAT_start_time)); 
   ep = lAddSubStr(obj, UA_name, "end_time", nm, UA_Type);
   lSetDouble(ep, UA_value, time_stamp);
   ep = lAddSubStr(obj, UA_name, "ru_wallclock", nm, UA_Type);
   lSetDouble(ep, UA_value, 0.0);

   lSetList(job_report, JR_usage, lCopyList("", lGetList(obj, nm)));
   DEXIT;
}

