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

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_order.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"

#include "sge.h"

/****** sge_order/sge_free_cull_order_pos() ************************************
*  NAME
*     sge_free_cull_order_pos() -- frees a cull order struct
*
*  SYNOPSIS
*     void sge_free_cull_order_pos(order_pos_t **cull_order_pos) 
*
*  FUNCTION
*     frees a cull order struct
*
*  INPUTS
*     order_pos_t **cull_order_pos - a douple pointer to the struct. Will be
*                                    set to NULL
*
*  NOTES
*     MT-NOTE: sge_free_cull_order_pos() is MT safe 
*
*******************************************************************************/
void
sge_free_cull_order_pos(order_pos_t **cull_order_pos)
{
   sge_free(cull_order_pos);
}

/****** sge_order/sge_create_cull_order_pos() **********************************
*  NAME
*     sge_create_cull_order_pos() -- generates a cull order position struct
*
*  SYNOPSIS
*     void sge_create_cull_order_pos(order_pos_t **cull_order_pos, lListElem 
*     *jep, lListElem *jatp, lListElem *joker, lListElem *joker_task) 
*
*  FUNCTION
*     generates a cull order position struct
*
*  INPUTS
*     order_pos_t **cull_order_pos - struct to init. if not NULL, the old struct will be freed
*     lListElem *jep               - job structure
*     lListElem *jatp              - ja task structure
*     lListElem *joker             - job order structure
*     lListElem *joker_task        - ja task order structure
*
*  NOTES
*     MT-NOTE: sge_create_cull_order_pos() is MT safe 
*
*******************************************************************************/
void 
sge_create_cull_order_pos(order_pos_t **cull_order_pos, lListElem *jep, lListElem *jatp,
                      lListElem *joker, lListElem *joker_task) 
{
   ja_task_pos_t *ja_pos;
   ja_task_pos_t *order_ja_pos;   
   job_pos_t   *job_pos;
   job_pos_t   *order_job_pos; 

   if (*cull_order_pos != NULL) {
      sge_free(&cull_order_pos);
   }

   *cull_order_pos = malloc(sizeof(order_pos_t));

   ja_pos = &((*cull_order_pos)->ja_task);
   order_ja_pos = &((*cull_order_pos)->order_ja_task);
   job_pos = &((*cull_order_pos)->job);
   order_job_pos = &((*cull_order_pos)->order_job);   

   if (jep != NULL) {
      job_pos->JB_version_pos = lGetPosViaElem(jep,JB_version, SGE_NO_ABORT);
      job_pos->JB_nppri_pos = lGetPosViaElem(jep,JB_nppri, SGE_NO_ABORT);
      job_pos->JB_nurg_pos = lGetPosViaElem(jep,JB_nurg, SGE_NO_ABORT);
      job_pos->JB_urg_pos = lGetPosViaElem(jep,JB_urg, SGE_NO_ABORT);
      job_pos->JB_rrcontr_pos = lGetPosViaElem(jep,JB_rrcontr, SGE_NO_ABORT);
      job_pos->JB_dlcontr_pos = lGetPosViaElem(jep,JB_dlcontr, SGE_NO_ABORT);
      job_pos->JB_wtcontr_pos = lGetPosViaElem(jep,JB_wtcontr, SGE_NO_ABORT);  
/*      
DPRINTF(("job prio pos: %d %d %d %d %d %d %d\n", job_pos->JB_version_pos, job_pos->JB_nppri_pos,  job_pos->JB_nurg_pos,
                                  job_pos->JB_urg_pos, job_pos->JB_rrcontr_pos, job_pos->JB_dlcontr_pos,
                                  job_pos->JB_wtcontr_pos));#
*/                                  
   }

   if (jatp != NULL) {
      ja_pos->JAT_status_pos = lGetPosViaElem(jatp,JAT_status, SGE_NO_ABORT);
      ja_pos->JAT_tix_pos = lGetPosViaElem(jatp,JAT_tix, SGE_NO_ABORT);

      ja_pos->JAT_oticket_pos = lGetPosViaElem(jatp,JAT_oticket, SGE_NO_ABORT);
      ja_pos->JAT_fticket_pos = lGetPosViaElem(jatp,JAT_fticket, SGE_NO_ABORT);
      ja_pos->JAT_sticket_pos = lGetPosViaElem(jatp,JAT_sticket, SGE_NO_ABORT);
      ja_pos->JAT_share_pos = lGetPosViaElem(jatp,JAT_share, SGE_NO_ABORT);
      ja_pos->JAT_prio_pos = lGetPosViaElem(jatp,JAT_prio, SGE_NO_ABORT);
      ja_pos->JAT_ntix_pos = lGetPosViaElem(jatp,JAT_ntix, SGE_NO_ABORT);
/*
DPRINTF(("ja task prio pos: %d %d %d %d %d %d %d %d\n", ja_pos->JAT_status_pos, ja_pos->JAT_tix_pos, ja_pos->JAT_oticket_pos,
                                        ja_pos->JAT_fticket_pos, ja_pos->JAT_sticket_pos, 
                                        ja_pos->JAT_share_pos, ja_pos->JAT_prio_pos, ja_pos->JAT_ntix_pos)); 
*/                                        
   }

   if (joker != NULL) {
      order_job_pos->JB_version_pos = -1;
      order_job_pos->JB_nppri_pos = lGetPosViaElem(joker,JB_nppri, SGE_NO_ABORT);
      order_job_pos->JB_nurg_pos = lGetPosViaElem(joker,JB_nurg, SGE_NO_ABORT);
      order_job_pos->JB_urg_pos = lGetPosViaElem(joker,JB_urg, SGE_NO_ABORT);
      order_job_pos->JB_rrcontr_pos = lGetPosViaElem(joker,JB_rrcontr, SGE_NO_ABORT);
      order_job_pos->JB_dlcontr_pos = lGetPosViaElem(joker,JB_dlcontr, SGE_NO_ABORT);
      order_job_pos->JB_wtcontr_pos = lGetPosViaElem(joker,JB_wtcontr, SGE_NO_ABORT);
/*
      DPRINTF(("job order pos: %d %d %d %d %d %d %d\n", order_job_pos->JB_version_pos, order_job_pos->JB_nppri_pos,  order_job_pos->JB_nurg_pos,
                                  order_job_pos->JB_urg_pos, order_job_pos->JB_rrcontr_pos, order_job_pos->JB_dlcontr_pos,
                                  order_job_pos->JB_wtcontr_pos));
*/                                  
   }

   if (joker_task != NULL) {
      order_ja_pos->JAT_status_pos = -1;
      order_ja_pos->JAT_tix_pos = -1;

      order_ja_pos->JAT_oticket_pos = lGetPosViaElem(joker_task,JAT_oticket, SGE_NO_ABORT);
      order_ja_pos->JAT_fticket_pos = lGetPosViaElem(joker_task,JAT_fticket, SGE_NO_ABORT);
      order_ja_pos->JAT_sticket_pos = lGetPosViaElem(joker_task,JAT_sticket, SGE_NO_ABORT);
      order_ja_pos->JAT_share_pos = lGetPosViaElem(joker_task,JAT_share, SGE_NO_ABORT);
      order_ja_pos->JAT_prio_pos = lGetPosViaElem(joker_task,JAT_prio, SGE_NO_ABORT);
      order_ja_pos->JAT_ntix_pos = lGetPosViaElem(joker_task,JAT_ntix, SGE_NO_ABORT);  
/*
      DPRINTF(("ja task order pos: %d %d %d %d %d %d %d %d\n", order_ja_pos->JAT_status_pos, order_ja_pos->JAT_tix_pos, order_ja_pos->JAT_oticket_pos,
                                        order_ja_pos->JAT_fticket_pos, order_ja_pos->JAT_sticket_pos, 
                                        order_ja_pos->JAT_share_pos, order_ja_pos->JAT_prio_pos, order_ja_pos->JAT_ntix_pos));  
*/                                        
   }
}

