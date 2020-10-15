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

#include <ctype.h>
#include <float.h>

#include "sge.h"

#include "uti/sge_rmon.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "cull/cull.h"

#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_pe.h"

#include "sge_ct_SCT_L.h"
#include "sge_ct_REF_L.h"
#include "sge_ct_CT_L.h"
#include "sge_ct_CCT_L.h"
#include "sge_ct_CTI_L.h"
#include "sge_eejob_SGEJ_L.h"

#include "sge_urgency.h"
#include "sge_sched_process_events.h"


static void sge_normalize_urgency(lList *job_list, double min_urgency, 
   double max_urgency);
static void sge_urgency(u_long32 now, double *min_urgency, double *max_urgency, 
               lList *job_list, const lList *centry_list, lList *pe_list);


/****** sge_urgency/sge_do_urgency() *****************************
*  NAME
*     sge_do_urgency() -- Compute normalized urgency
*
*  SYNOPSIS
*     void sge_do_urgency(u_long32 now, lList *running_jobs, lList 
*     *pending_jobs, sge_Sdescr_t *lists) 
*
*  FUNCTION
*     Determine normalized urgency for all job lists passed:
*     * for the pending jobs we need it for determine dispatch order 
*     * for the running jobs it is needed when running jobs priority must
*       be compared with pending jobs (preemption only)
*
*  INPUTS
*     u_long32 now        - Current time
*     lList *running_jobs - The running jobs list
*     lList *pending_jobs - The pending jobs list
*     sge_Sdescr_t *lists - Additional config information
*
*  NOTES
*******************************************************************************/
void sge_do_urgency(u_long32 now, lList *running_jobs, lList *pending_jobs, 
                    scheduler_all_data_t *lists)
{
   double min_urgency = DBL_MAX;
   double max_urgency = DBL_MIN;

   /* determine absolute static urgency and related min/max values */
   sge_urgency(now, &min_urgency, &max_urgency, pending_jobs, 
         lists->centry_list, lists->pe_list);
   sge_urgency(now, &min_urgency, &max_urgency, running_jobs, 
         lists->centry_list, lists->pe_list);

   /* use min/max value to normalize static urgency */
   if (pending_jobs) {
      sge_normalize_urgency(pending_jobs, min_urgency, max_urgency);
   }   
   if (running_jobs) {
      sge_normalize_urgency(running_jobs, min_urgency, max_urgency);
   }   
}

/****** sge_urgency/sge_urgency() ********************************
*  NAME
*     sge_urgency() -- Determine urgency value for a list of jobs
*
*  SYNOPSIS
*     static void sge_urgency(u_long32 now, double *min_urgency, 
*     double *max_urgency, lList *job_list, const lList *centry_list, 
*     const lList *pe_list) 
*
*  FUNCTION
*     The urgency value is determined for all jobs in job_list. The urgency 
*     value has two time dependent components (waiting time contribution and
*     deadline contribution) and a resource request dependent component. Only 
*     resource requests that apply to the job irrespective what resources it 
*     gets assigned finally are considered. Default requests specified for 
*     consumable resources are not considered as they are placement dependent.
*     For the same reason soft request do not contribute to the urgency value.
*     The urgency value range is tracked via min/max urgency. Category-based
*     caching is used for the resource request urgency contribution.
*
*  INPUTS
*     u_long32 now               - Current time
*     double *min_urgency - For tracking minimum urgency value
*     double *max_urgency - For tracking minimum urgency value
*     lList *job_list            - The jobs.
*     const lList *centry_list   - Needed for per resource urgency setting.
*     const lList *pe_list       - Needed to determine urgency slot setting.
*
*  NOTES
*******************************************************************************/
static void sge_urgency(u_long32 now, double *min_urgency, double *max_urgency, 
               lList *job_list, const lList *centry_list, lList *pe_list)
{
   lListElem *jep;
   double rrc, wtc, dtc, absolute_urgency;
   int slots;
   double weight_deadline = sconf_get_weight_deadline();
   double weight_waiting_time = sconf_get_weight_waiting_time();

   DENTER(TOP_LAYER, "sge_urgency");

   for_each (jep, job_list) {
      lListElem *cat;
      u_long32 deadline;

      rrc = dtc = 0.0;

      /* waiting time dependent contribution */
      wtc = weight_waiting_time * (now - lGetUlong(jep, JB_submission_time));

      /* job deadline dependent contribution */
      if ((deadline=lGetUlong(jep, JB_deadline))) {
          int time_left = deadline - now;
/*           DPRINTF(("free: %d now: "sge_u32" deadline: "sge_u32"\n", time_left, now, deadline)); */
          /* might be too late for this job anyways we're optimistic and treat it high prior */
          dtc = weight_deadline / MAX(time_left, 1);
      }

      /* we do category based caching when determining the resource request 
         dependent contribution */
      if ((cat = lGetRef(jep, JB_category)) && lGetBool(cat, CT_rc_valid)) {
         rrc = lGetDouble(cat, CT_resource_contribution);
/*         DPRINTF(("  resource contribution from category cache ---> %7f\n", rrc)); */
      } else {
         lListElem *centry, *rr;
         double contribution;

         slots = sge_job_slot_request(jep, pe_list);

         /* contribution for implicit slot request */ 
         if (!(centry = centry_list_locate(centry_list, SGE_ATTR_SLOTS))) {
            continue;
         }
         contribution = centry_urgency_contribution(slots, SGE_ATTR_SLOTS, 1.0, centry);
         rrc += contribution;

         /* contribution for all explicit requests */
         for_each (rr, lGetList(jep, JB_hard_resource_list)) {
            if (!(centry = centry_list_locate(centry_list, lGetString(rr, CE_name)))) {
               continue;
            } 
            contribution = centry_urgency_contribution(slots, lGetString(rr, CE_name), 
                  lGetDouble(rr, CE_doubleval), centry);
            rrc += contribution;
         }

         /* cache in category */
         if (cat) {
            lSetBool(cat, CT_rc_valid,              true);
            lSetDouble(cat, CT_resource_contribution, rrc);
         }
      }
      absolute_urgency = wtc + dtc + rrc; 

      /* store these values with the job */
      lSetDouble(jep, JB_dlcontr, dtc);
      lSetDouble(jep, JB_rrcontr, rrc);
      lSetDouble(jep, JB_wtcontr, wtc);
      lSetDouble(jep, JB_urg, absolute_urgency);

/*      DPRINTF(("--- job "sge_U32CFormat" (dtc %7f + wtc %7f + rrc %7f) = asu %7f\n", 
            lGetUlong(jep, JB_job_number), dtc, wtc, rrc, absolute_urgency));
*/            
      
      /* track min/max values */
      if (min_urgency) {
         *min_urgency = MIN(*min_urgency, absolute_urgency);
      }   
      if (max_urgency) {
         *max_urgency = MAX(*max_urgency, absolute_urgency);
      }   
   }

   DEXIT;
   return;
}

/****** sge_urgency/sge_normalize_urgency() **********************
*  NAME
*     sge_normalize_urgency() -- Computes normalized urgency for job list
*
*  SYNOPSIS
*     static void sge_normalize_urgency(lList *job_list, double 
*     min_urgency, double max_urgency) 
*
*  FUNCTION
*     The normalized urgency is determined for a list of jobs based on the
*     min/max urgency values passed and the JB_urg value of each job.
*
*  INPUTS
*     lList *job_list           - The job list
*     double min_urgency - minimum urgency value
*     double max_urgency - maximum urgency value
*
*  NOTES
*     MT-NOTES: sge_normalize_urgency() is MT safe
*******************************************************************************/
static void sge_normalize_urgency(lList *job_list, double min_urgency, 
   double max_urgency)
{
   double nsu;
   lListElem *jep;

   DENTER(TOP_LAYER, "sge_normalize_urgency");

   DPRINTF(("ASU min = %13.11f, ASU max = %13.11f\n", 
         min_urgency, max_urgency));
   for_each (jep, job_list) {
      double asu = lGetDouble(jep, JB_urg);
      nsu = sge_normalize_value(asu, min_urgency, max_urgency);
      lSetDouble(jep, JB_nurg, nsu);
/*    DPRINTF(("NSU(job " sge_u32 ") = %f from %f\n", lGetUlong(jep, JB_job_number), nsu, asu)); */
   }

   DEXIT;
}


/****** sge_job_schedd/sge_job_slot_request() **********************************
*  NAME
*     sge_job_slot_request() -- return static urgency jobs slot request
*
*  SYNOPSIS
*     int sge_job_slot_request(lListElem *job, lList *pe_list)
*
*  FUNCTION
*     For sequential jobs the static urgency job slot request is always 1.
*     For parallel jobs the static urgency job slot request depends on
*     static urgency slots as defined with sge_pe(5).
*
*  INPUTS
*     lListElem *job - the job (JB_Type)
*     lList *pe_list - the PE list (PE_Type)
*
*  RESULT
*     int - Number of slots
*
*  NOTES
*     In case of a wildcard parallel environment request the setting of the 
*     first matching is used. Behaviour is undefined if multiple parallel 
*     environments specify different settings!
*******************************************************************************/
int sge_job_slot_request(const lListElem *job, const lList *pe_list)
{
   const char *pe_name;
   const char *urgency_slot_setting; 
   lList* range_list;
   const lListElem *pep;
   int n;
  
   DENTER(TOP_LAYER, "sge_job_slot_request");

   /* sequential job */
   if (!(pe_name=lGetString(job, JB_pe))) {
      DEXIT;
      return 1;
   } 

   /* parallel job with fixed slot request */
   range_list = lGetList(job, JB_pe_range);
   if (range_list_get_number_of_ids(range_list)==1) { 
      DEXIT;
      return range_list_get_first_id(range_list, NULL);
   } 

   /* parallel job with slot range request */
   if (!sge_is_pattern(pe_name))
      pep = pe_list_locate(pe_list, pe_name);
   else {
      /* use the first matching pe */
      if ((pep=pe_list_find_matching(pe_list, pe_name))) {
         DPRINTF(("use %s as first matching pe for %s to verify schedulability\n", 
                  lGetString(pep, PE_name), pe_name));
      }
   }
   if (!pep) {
      ERROR((SGE_EVENT, "no matching parallel environment "
            "for job's PE request \"%s\"\n", pe_name));
      DEXIT;
      return 1;
   }

   urgency_slot_setting = lGetString(pep, PE_urgency_slots);

   n = pe_urgency_slots(pep, urgency_slot_setting, range_list); 

   {
      char pe_range_str[1024];
      dstring pe_range;
      sge_dstring_init(&pe_range, pe_range_str, sizeof(pe_range_str));
      range_list_print_to_string(range_list, &pe_range, true, false, false);
      DPRINTF(("slot request assumed for static urgency is %d for %s PE range due to "
          "PE's \"%s\" setting \"%s\"\n", n, pe_range_str, lGetString(pep, PE_name), 
          urgency_slot_setting));
   }

   DEXIT;
   return n;
}


/****** sge_urgency/sge_normalize_value() *******************************
*  NAME
*     sge_normalize_value() -- Returns normalized value with passed value range 
*
*  SYNOPSIS
*     double sge_normalize_value(double value, double range_min, double 
*     range_max) 
*
*  FUNCTION
*     The value passed is normalized and resulting value (0.0-1.0) is returned
*     The value range passed is assumed. In case there is no range because
*     min/max are (nearly) equal 0.5 is returned. 
*
*  INPUTS
*     double value     - Value to be normalized.
*     double range_min - Range minimum value.
*     double range_max - Range maximum value.
*
*  RESULT
*     double - Normalized value (0.0-1.0)
*
*  NOTES
*     MT-NOTE: sge_normalize_value() is MT safe
*******************************************************************************/
double sge_normalize_value(double value, double range_min, double range_max)
{
   double result;

   if (range_max - range_min < SGE_EPSILON)
      result = 0.5;
   else
      result = (value - range_min)/( range_max - range_min);
   return result;
}

