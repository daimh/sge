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
#include <string.h>
#include <time.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_parse_num_par.h"

#include "sgeobj/sge_host.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_resource_utilization_RUE_L.h"

#include "schedd_monitor.h"
#include "load_correction.h"
#include "sge_complex_schedd.h"

int correct_load(lList *running_jobs, lList *queue_list, lList *host_list,
                  u_long32 decay_time, bool monitor_next_run) 
{
   lListElem *job = NULL;
   u_long32 now;
   lListElem *global_host = NULL;

   
   DENTER(TOP_LAYER, "correct_load");

   if (queue_list == NULL || host_list == NULL) {
      DEXIT;
      return 1;
   }

   global_host = host_list_locate(host_list, "global");
   now = sge_get_gmt();

   for_each (job, running_jobs) {   
      u_long32 job_id = lGetUlong(job, JB_job_number);
      lListElem *ja_task = NULL;
      double global_lcf = 0.0;

      for_each (ja_task, lGetList(job, JB_ja_tasks)) {  
         u_long32 ja_task_id = lGetUlong(ja_task, JAT_task_number); 
         u_long32 running_time = now - lGetUlong(ja_task, JAT_start_time);
         lListElem *granted_queue = NULL;
         lList *granted_list = NULL;
         double host_lcf = 0.0;

#if 1
         DPRINTF(("JOB "sge_u32"."sge_u32" start_time = "sge_u32" running_time "sge_u32
            " decay_time = "sge_u32"\n", job_id, ja_task_id, 
            lGetUlong(ja_task, JAT_start_time), running_time, 
            decay_time));
#endif
         if (running_time > decay_time) {
            continue;
         }
         granted_list = lGetList(ja_task, JAT_granted_destin_identifier_list);
         for_each (granted_queue, granted_list) {   
            const char *qnm = NULL;
            const char *hnm = NULL;
            lListElem *qep = NULL;
            lListElem *hep = NULL;
            u_long32 slots;
            
            qnm = lGetString(granted_queue, JG_qname);
            qep = qinstance_list_locate2(queue_list, qnm);
            if (qep == NULL) {
               DPRINTF(("Unable to find queue \"%s\" from gdil "
                        "list of job "sge_u32"."sge_u32"\n", qnm, job_id, ja_task_id));
               continue;
            }
           
            hnm=lGetHost(granted_queue, JG_qhostname); 
            hep = lGetElemHost(host_list, EH_name, hnm);
            if (hep == NULL) {
               DPRINTF(("Unable to find host \"%s\" from gdil "
                        "list of job "sge_u32"."sge_u32"\n", hnm, job_id, ja_task_id));
               continue;
            } 

            /* To implement load correction we add values between
               1 (just started) and 0 (load_adjustment_decay_time expired)
               for each job slot in the exec host field 
               EH_load_correction_factor. This field is used later on to:
               - sort hosts concerning load
               - decide about load thresholds of queues
               - resort hosts for each scheduled job          */ 
            
            /* use linear function for additional load correction factor 
                                         t
               correction(t) = 1 - ---------------- 
                                    decay_time
            */
            host_lcf = 1 - ((double) running_time / (double) decay_time);
            global_lcf += host_lcf;

            /* multiply it for each slot on this host */
            slots = lGetUlong(granted_queue, JG_slots);
            host_lcf *= slots;
            
            /* add this factor (multiplied with 100 for being able to use 
               u_long32) */
            lSetUlong(hep, EH_load_correction_factor, 
                      host_lcf * 100 + 
                      lGetUlong(hep, EH_load_correction_factor));

#if 1
            DPRINTF(("JOB "sge_u32"."sge_u32" ["sge_u32" slots] in queue %s increased lc of host "
                     "%s by "sge_u32" to "sge_u32"\n", job_id, ja_task_id, slots, qnm, hnm, 
                     (u_long32)(100*host_lcf), lGetUlong(hep, EH_load_correction_factor)));
#endif
            if (monitor_next_run) {
               char log_string[2048 + 1];
               snprintf(log_string, sizeof(log_string), "JOB "sge_u32"."sge_u32" ["sge_u32"] in queue "SFN
                          " increased absolute lc of host "SFN" by "sge_u32" to "
                          sge_u32"", job_id, ja_task_id, slots, qnm, hnm,
                          (u_long32)(host_lcf*100), lGetUlong(hep, EH_load_correction_factor));
               schedd_log(log_string, NULL, true);
            }
         }
      }
      lSetUlong(global_host, EH_load_correction_factor, 
                global_lcf * 100 + 
                lGetUlong(global_host, EH_load_correction_factor));
   }

   DEXIT;
   return 0;
}


/*
 * Do load scaling and capacity correction for all consumable 
 *  attributes where also load values are available
 *
 */
int 
correct_capacities(lList *host_list, lList *centry_list) 
{
   lListElem *hep, *ep, *cep; 
   lListElem *job_load, *scaling, *total, *inuse_rms;
   u_long32 type, relop;
   double dval, inuse_ext, full_capacity, sc_factor;
   double load_correction;
   lList* job_load_adj_list = NULL;

   DENTER(TOP_LAYER, "correct_capacities");
   job_load_adj_list = sconf_get_job_load_adjustments();
 
   for_each (hep, host_list) {   
      const char *host_name = lGetHost(hep, EH_name);

      for_each (ep, lGetList(hep, EH_load_list)) {  
         const char *attr_name = lGetString(ep, HL_name);
 
         /* seach for appropriate complex attribute */
         if (!(cep=centry_list_locate(centry_list, attr_name)))
            continue;

         type = lGetUlong(cep, CE_valtype);
         if (type != TYPE_INT &&
             type != TYPE_TIM &&
             type != TYPE_MEM &&  
             type != TYPE_BOO &&  
             type != TYPE_DOUBLE) {
            continue;
         }
        
         if (!parse_ulong_val(&dval, NULL, type, lGetString(ep, HL_value), NULL, 0))
            continue;

         /* do load scaling */
         if ((scaling=lGetSubStr(hep, HS_name, attr_name, EH_scaling_list))) {
            char sval[20];
            sc_factor = lGetDouble(scaling, HS_value);
            dval *= sc_factor;
            sprintf(sval, "%8.3f", dval);
            lSetString(ep, HL_value, sval);
         }

         if (lGetUlong(cep, CE_consumable) == CONSUMABLE_NO)
            continue;
         if (!(total=lGetSubStr(hep, CE_name, attr_name, EH_consumable_config_list)))
            continue;
         if (!(inuse_rms=lGetSubStr(hep, RUE_name, attr_name, EH_resource_utilization)))
            continue;

         relop = lGetUlong(cep, CE_relop);
         if (relop != CMPLXEQ_OP &&
             relop != CMPLXLT_OP &&
             relop != CMPLXLE_OP &&
             relop != CMPLXNE_OP)
            continue;

         /* do load correction */
         load_correction = 0;
         if ((job_load=lGetElemStr(job_load_adj_list, CE_name, attr_name))) {
            double lc_factor;
            const char *s = lGetString(job_load, CE_stringval);

            if (parse_ulong_val(&load_correction, NULL, type, s, NULL, 0)) {
               lc_factor = ((double)lGetUlong(hep, EH_load_correction_factor))/100.0;
               load_correction *= lc_factor;
               DPRINTF(("%s:%s %s %8.3f %8.3f\n", 
                  host_name, attr_name, s, load_correction, lc_factor));
               dval -= load_correction;
            }
         }

         /* use scaled load value to deduce the amount */
         full_capacity = lGetDouble(total, CE_doubleval);
         inuse_ext = full_capacity - lGetDouble(inuse_rms, RUE_utilized_now) - dval;

         if (inuse_ext > 0.0) {
            lSetDouble(total, CE_doubleval, full_capacity - inuse_ext);

            DPRINTF(("%s:%s %8.3f --> %8.3f (ext: %8.3f = all %8.3f - ubC %8.3f - load %8.3f) lc = %8.3f\n",
               host_name, attr_name, full_capacity, lGetDouble(total, CE_doubleval),
               inuse_ext, full_capacity, lGetDouble(inuse_rms, RUE_utilized_now), dval, load_correction));
         } else
            DPRINTF(("ext: %8.3f <= 0\n", inuse_ext));
      }
   }
   lFreeList(&job_load_adj_list);

   DEXIT;
   return 0;
}
