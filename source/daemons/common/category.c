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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_prog.h"
#include "uti/sge_dstring.h"

#include "sgeobj/sge_event.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_qref.h"

#include "sched/schedd_monitor.h"
#include "sched/sge_select_queue.h"
#include "sched/sge_resource_quota_schedd.h"


#if 0 /* TODO: EB: ST: should this be enabled again? */

/* struct containing the cull field position of the job target structures
   and the reduced order elements */
typedef struct {
   int JB_hard_queue_list_pos;
   int JB_master_hard_queue_list_pos;
   int JB_hard_resource_list_pos;
   int JB_soft_resource_list_pos;
   int JB_checkpoint_name_pos;
   int JB_type_pos;
   int JB_owner_pos;
   int JB_group_pos;
   int JB_project_pos;
   int JB_pe_pos;
   int JB_range_pos;
   int JB_ar_pos;
} order_pos_t;

typedef struct {
   pthread_mutex_t cull_order_mutex; /* gards the last_update access */
   order_pos_t cull_order_pos;        /* stores cull positions in the job, ja-task, and order structure */
} sge_category_t;

static sge_category_t Category_Control = {PTHREAD_MUTEX_INITIALIZER, {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

#endif

/*-------------------------------------------------------------------------*/
/* build the category string                                               */
/* the category string includes now the soft requests                      */
/*-------------------------------------------------------------------------*/
/****** category/sge_build_job_category_dstring() ******************************
*  NAME
*     sge_build_job_category_dstring() -- build the category string   
*
*  SYNOPSIS
*     void sge_build_job_category_dstring(dstring *category_str, lListElem 
*     *job, lList *acl_list) 
*
*  FUNCTION
*     The following parameters are put into the category:
*        hard_queue_list
*        master_hard_queue_list
*        hard_resource_list
*        soft_resource_list
*        checkpoint_name
*        type
*
*        owner/group: -U user_lists 
*           Omitted if user_lists/xuser_lists were not used in
*           host_conf(5), sge_pe(5) and queue_conf(5). In sge_conf(5) 
*           user_lists/xuser_lists still can be used, as it causes
*           jobs already to be rejected at submit time.
*
*        project: -P user_lists 
*           Omitted if projects/xprojects were not used in
*           host_conf(5), sge_pe(5) and queue_conf(5). In sge_conf(5) 
*           projects/xprojects still can be used, as it cuases
*           jobs already to be rejected at submit time.
*
*        pe
*
*        ar
*
*  INPUTS
*     dstring *category_str - target string, contains the category or nothing
*     lListElem *job        - the job for the category creating
*     lList *acl_list       - global access list
*
*  NOTES
*     MT-NOTE: sge_build_job_category_dstring() is MT safe as long as the caller is
*
*******************************************************************************/
void sge_build_job_category_dstring(dstring *category_str, lListElem *job, lList *acl_list, const lList *prj_list, bool *did_project, const lList *rqs_list) 
{

   const char *owner = NULL;
   const char *group = NULL;

   DENTER(TOP_LAYER, "sge_build_job_category_dstring");

   DTRACE;

#if 0
   sge_mutex_lock("cull_order_mutex", SGE_FUNC, __LINE__, &Category_Control.cull_order_mutex);
   if (Category_Control.cull_order_pos.JB_hard_queue_list_pos == -1) {
      Category_Control.cull_order_pos.JB_checkpoint_name_pos = lGetPosViaElem(job, JB_checkpoint_name, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_soft_resource_list_pos = lGetPosViaElem(job, JB_soft_resource_list, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_master_hard_queue_list_pos = lGetPosViaElem(job, JB_master_hard_queue_list, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_hard_queue_list_pos = lGetPosViaElem(job, JB_hard_queue_list, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_owner_pos = lGetPosViaElem(job, JB_owner, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_group_pos = lGetPosViaElem(job, JB_group, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_hard_resource_list_pos = lGetPosViaElem(job, JB_hard_resource_list, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_type_pos = lGetPosViaElem(job, JB_type, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_project_pos = lGetPosViaElem(job, JB_project, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_ar_pos = lGetPosViaElem(job, JB_ar, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_pe_pos = lGetPosViaElem(job, JB_pe, SGE_NO_ABORT);
      Category_Control.cull_order_pos.JB_range_pos = lGetPosViaElem(job, JB_pe_range, SGE_NO_ABORT);
   }
   sge_mutex_unlock("cull_order_mutex", SGE_FUNC, __LINE__, &Category_Control.cull_order_mutex);
#endif

   DTRACE;

   /*
   ** owner -> acl
   */
   owner = lGetPosString(job, lGetPosViaElem(job, JB_owner, SGE_NO_ABORT));
   
   DTRACE;

   group = lGetPosString(job, lGetPosViaElem(job, JB_group, SGE_NO_ABORT));

   DTRACE;

   sge_unparse_acl_dstring(category_str, owner, group, acl_list, "-U");

   DTRACE;
 
   /* 
   ** -u if referenced in resource quota sets
   */

   /* RD TODO: A possible performance enhancement is to split user and group inside category.
      Some users are only referenced by the unix group. Their jobs could be grouped
      together by referencing only the group in the category string
   */
   if (sge_user_is_referenced_in_rqs(rqs_list, lGetString(job, JB_owner), lGetString(job, JB_group), acl_list)) {
      if (sge_dstring_strlen(category_str) > 0) {
         sge_dstring_append(category_str, " ");
      }
      sge_dstring_append(category_str, "-u ");
      sge_dstring_append(category_str, lGetString(job, JB_owner));
   }

   DTRACE;

   /*
   ** -hard -q qlist
   */
   sge_unparse_queue_list_dstring(category_str, job, lGetPosViaElem(job, JB_hard_queue_list, SGE_NO_ABORT), "-q");  

   DTRACE;

   /*
   ** -masterq qlist
   */
   sge_unparse_queue_list_dstring(category_str, job, lGetPosViaElem(job, JB_master_hard_queue_list, SGE_NO_ABORT), "-masterq");

   DTRACE;

   /*
   ** -l rlist (hard resource list)
   */
   sge_unparse_resource_list_dstring(category_str, job, lGetPosViaElem(job, JB_hard_resource_list, SGE_NO_ABORT), "-l");

   DTRACE;
   
   /*
   ** -soft -l rlist
   */
   sge_unparse_resource_list_dstring(category_str, job, lGetPosViaElem(job, JB_soft_resource_list, SGE_NO_ABORT), "-soft -l");

   DTRACE;

   /*
   ** -pe pe_name pe_range
   */
   sge_unparse_pe_dstring(category_str, job, lGetPosViaElem(job, JB_pe, SGE_NO_ABORT), 
                          lGetPosViaElem(job, JB_pe_range, SGE_NO_ABORT), "-pe");

   DTRACE;

   /*
   ** -ckpt ckpt_name 
   */
   sge_unparse_string_option_dstring(category_str, job, lGetPosViaElem(job, JB_checkpoint_name, SGE_NO_ABORT), "-ckpt");

   DTRACE;

   /*
   ** interactive jobs
   */
   if (JOB_TYPE_IS_IMMEDIATE(lGetPosUlong(job, lGetPosViaElem(job, JB_type, SGE_NO_ABORT)))) {
      if (sge_dstring_strlen(category_str) > 0) {
         sge_dstring_append(category_str, " -I y");
      }
      else {
         sge_dstring_append(category_str, "-I y");
      }
   }

   DTRACE;
      
   /*
   ** project
   */
   {
      const char *project = lGetPosString(job, lGetPosViaElem(job, JB_project, SGE_NO_ABORT));

      const lListElem *prj;
      if (project && (prj=lGetElemStr(prj_list, PR_name, project)) && lGetBool(prj, PR_consider_with_categories)) {
         if (did_project)
            *did_project = true;
         sge_unparse_string_option_dstring(category_str, job, lGetPosViaElem(job, JB_project, SGE_NO_ABORT), "-P");
      } else
         if (did_project)
            *did_project = false;
   }

   DTRACE;

   /*
   ** -ar ar_id
   */
   sge_unparse_ulong_option_dstring(category_str, job, lGetPosViaElem(job, JB_ar, SGE_NO_ABORT), "-ar");  

   DRETURN_VOID;
}

/****** category/sge_build_job_cs_category() ***********************************
*  NAME
*     sge_build_job_cs_category() -- generates the category string for the sc
*
*  SYNOPSIS
*     const char* sge_build_job_cs_category(dstring *category_str, lListElem 
*     *job, lListElem *cat_obj) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     dstring *category_str - storage forthe c string
*     lListElem *job        - job for which to create a category
*     lListElem *cat_obj    - regular category for the job.
*
*  RESULT
*     const char* - 
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     MT-NOTE: sge_build_job_cs_category() is MT safe 
*
*******************************************************************************/
const char* 
sge_build_job_cs_category(dstring *category_str, lListElem *job, lListElem *cat_obj, bool did_project) 
{
   const char *p;

   DENTER(TOP_LAYER, "sge_build_job_cs_category");

   /* 
    *  deadline
    */
   sge_dstring_sprintf_append(category_str, "-dl "sge_u32, lGetUlong(job, JB_deadline));
   /*
    * priority
    */
   sge_dstring_sprintf_append(category_str, "-p "sge_u32, lGetUlong(job, JB_priority));
   sge_dstring_sprintf_append(category_str, "-ot "sge_u32, lGetUlong(job, JB_override_tickets));
   sge_dstring_sprintf_append(category_str, "-js "sge_u32, lGetUlong(job, JB_jobshare));
   sge_dstring_sprintf_append(category_str, "-u %s", lGetString(job, JB_owner));

   /*
    * ticket assignment can depend on a jobs project.
    * If the project was not added to the resource categories
    * it gets added directly to the cs category string 
    */
   if ((p=lGetString(job, JB_project)) && !did_project)
      sge_dstring_sprintf_append(category_str, " -P %s", p);

   /*
    * id for the resource category
    * I use the address of the resource category as a hash value for its string.
    */
   sge_dstring_sprintf_append(category_str, " %lx", (unsigned long) cat_obj);

   DEXIT;
   return sge_dstring_get_string(category_str);
}
