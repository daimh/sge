#ifndef __SGE_BOUNDARIES_H
#define __SGE_BOUNDARIES_H
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

#include "cull/cull_list.h"

/*----------------------------------------------------
 *
 * 1. sge_boundaries.h
 * 
 * 	is included by every file of group 2 
 * 	defines boundaries of all lists
 * 
 * 2. sge_jobL.h ...
 * 
 * 	is used to define a specific list like:
 * 	- job list
 * 	- ... 
 * 
 *--------------------------------------------------*/

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * BASIC_UNIT and MAX_DESCR_SIZE is defined in cull/cull_list.h
 */

enum NameSpaceBoundaries {

   /* cull version of sge_list_type */
   /* is finally removed, first 50 are free now */
      
   /* job list */
   JB_LOWERBOUND = 1*BASIC_UNIT,
   JB_UPPERBOUND = JB_LOWERBOUND + MAX_DESCR_SIZE - 1,

   /* queue list */
   QU_LOWERBOUND = JB_UPPERBOUND + 1,
   QU_UPPERBOUND = QU_LOWERBOUND + MAX_DESCR_SIZE - 1,

   /* exec host list */
   EH_LOWERBOUND = QU_UPPERBOUND + 1,
   EH_UPPERBOUND = EH_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* admin host list */
   AH_LOWERBOUND = EH_UPPERBOUND + 1,
   AH_UPPERBOUND = AH_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* submit host list */
   SH_LOWERBOUND = AH_UPPERBOUND + 1,
   SH_UPPERBOUND = SH_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* answer list for gdi requests */
   AN_LOWERBOUND = SH_UPPERBOUND + 1,
   AN_UPPERBOUND = AN_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* host aliases list */
   HA_LOWERBOUND = AN_UPPERBOUND + 1,
   HA_UPPERBOUND = HA_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* host load list */
   HL_LOWERBOUND = HA_UPPERBOUND + 1,
   HL_UPPERBOUND = HL_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* host scaling list */
   HS_LOWERBOUND = HL_UPPERBOUND + 1,
   HS_UPPERBOUND = HS_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* host scaling list */
   HT_LOWERBOUND = HS_UPPERBOUND + 1,
   HT_UPPERBOUND = HT_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* clients getting events sent */
   EV_LOWERBOUND = HT_UPPERBOUND + 1,
   EV_UPPERBOUND = EV_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* a certain entry of the complex-list */
   CE_LOWERBOUND = EV_UPPERBOUND + 1,
   CE_UPPERBOUND = CE_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* order list */ 
   OR_LOWERBOUND = CE_UPPERBOUND + 1,
   OR_UPPERBOUND = OR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* entries for OR_queuelist-field */
   OQ_LOWERBOUND = OR_UPPERBOUND + 1,
   OQ_UPPERBOUND = OQ_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* a list for the to handle the range */
   RR_LOWERBOUND = OQ_UPPERBOUND + 1,
   RR_UPPERBOUND = RR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* structure of a load report */
   LR_LOWERBOUND = RR_UPPERBOUND + 1,
   LR_UPPERBOUND = LR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* (x)access-lists for the queues */
   US_LOWERBOUND = LR_UPPERBOUND + 1,
   US_UPPERBOUND = US_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* (x)access-lists entries for the queues */
   UE_LOWERBOUND = US_UPPERBOUND + 1,
   UE_UPPERBOUND = UE_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* range list */
   RN_LOWERBOUND = UE_UPPERBOUND + 1,
   RN_UPPERBOUND = RN_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* path name list */
   PN_LOWERBOUND = RN_UPPERBOUND + 1,
   PN_UPPERBOUND = PN_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* account list */
   AT_LOWERBOUND = PN_UPPERBOUND + 1,
   AT_UPPERBOUND = AT_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* variable list */
   VA_LOWERBOUND = AT_UPPERBOUND + 1,
   VA_UPPERBOUND = VA_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* mail list */
   MR_LOWERBOUND = VA_UPPERBOUND + 1,
   MR_UPPERBOUND = MR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* manager list */
   UM_LOWERBOUND = MR_UPPERBOUND + 1,
   UM_UPPERBOUND = UM_LOWERBOUND + 1*BASIC_UNIT - 1,
   
   /* manager and operator list */
   UO_LOWERBOUND = UM_UPPERBOUND + 1,
   UO_UPPERBOUND = UO_LOWERBOUND + 1*BASIC_UNIT - 1,
   
   /* the event itself */
   ET_LOWERBOUND = UO_UPPERBOUND + 1,
   ET_UPPERBOUND = ET_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* subscribed event list */
   EVS_LOWERBOUND = ET_UPPERBOUND + 1,
   EVS_UPPERBOUND = EVS_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* config list */
   CONF_LOWERBOUND = EVS_UPPERBOUND + 1,
   CONF_UPPERBOUND = CONF_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler configuration list */
   CFSCHED_LOWERBOUND = CONF_UPPERBOUND + 1,
   CFSCHED_UPPERBOUND = CFSCHED_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* the configuration itself */
   CF_LOWERBOUND = CFSCHED_UPPERBOUND + 1,
   CF_UPPERBOUND = CF_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* the parallel environmet */
   PE_LOWERBOUND = CF_UPPERBOUND + 1,
   PE_UPPERBOUND = PE_LOWERBOUND + 1*BASIC_UNIT - 1,
   
   /* queue references - a sublist containing qnames (used in the pe object) */
   QR_LOWERBOUND = PE_UPPERBOUND + 1,
   QR_UPPERBOUND = QR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* job counter used in schedd to keep number of jobs per user/group */
   JC_LOWERBOUND = QR_UPPERBOUND + 1,
   JC_UPPERBOUND = JC_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* string list */
   ST_LOWERBOUND = JC_UPPERBOUND + 1,
   ST_UPPERBOUND = ST_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* string list */
   STU_LOWERBOUND = ST_UPPERBOUND + 1,
   STU_UPPERBOUND = STU_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* jobs granted destination identifier list */
   JG_LOWERBOUND = STU_UPPERBOUND + 1,
   JG_UPPERBOUND = JG_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* history directory list */
   HD_LOWERBOUND = JG_UPPERBOUND + 1,
   HD_UPPERBOUND = HD_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* history file list */
   HF_LOWERBOUND = HD_UPPERBOUND + 1,
   HF_UPPERBOUND = HF_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* list for sorted output of job sums with QACCT */
   QAJ_LOWERBOUND = HF_UPPERBOUND + 1,
   QAJ_UPPERBOUND = QAJ_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* argument list for limited parser */
   SPA_LOWERBOUND = QAJ_UPPERBOUND + 1,
   SPA_UPPERBOUND = SPA_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* report list */
   REP_LOWERBOUND = SPA_UPPERBOUND + 1,
   REP_UPPERBOUND = REP_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* list of subordinated queues */
   SO_LOWERBOUND = REP_UPPERBOUND + 1,
   SO_UPPERBOUND = SO_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* usage list */
   UA_LOWERBOUND = SO_UPPERBOUND + 1,
   UA_UPPERBOUND = UA_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* list of projects */
   PR_LOWERBOUND = UA_UPPERBOUND + 1,
   PR_UPPERBOUND = PR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* list of users/projects */
   UU_LOWERBOUND = PR_UPPERBOUND + 1,
   UU_UPPERBOUND = UU_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* list of share tree nodes */
   STN_LOWERBOUND = UU_UPPERBOUND + 1,
   STN_UPPERBOUND = STN_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler config */
   SC_LOWERBOUND = STN_UPPERBOUND + 1,
   SC_UPPERBOUND = SC_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* pe task object */
   PET_LOWERBOUND = SC_UPPERBOUND + 1,
   PET_UPPERBOUND = PET_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* pe task request object */
   PETR_LOWERBOUND = PET_UPPERBOUND + 1,
   PETR_UPPERBOUND = PETR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* finished pe task reference */
   FPET_LOWERBOUND = PETR_UPPERBOUND + 1,
   FPET_UPPERBOUND = FPET_LOWERBOUND * 1*BASIC_UNIT - 1,

   /* job report */
   JR_LOWERBOUND = FPET_UPPERBOUND + 1,
   JR_UPPERBOUND = JR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* license report */
   LIC_LOWERBOUND = JR_UPPERBOUND + 1,
   LIC_UPPERBOUND = LIC_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* deadline structure */
   DL_LOWERBOUND = LIC_UPPERBOUND + 1,
   DL_UPPERBOUND = DL_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* ptf job list */
   JL_LOWERBOUND = DL_UPPERBOUND + 1,
   JL_UPPERBOUND = JL_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* ptf pid list */
   JP_LOWERBOUND = JL_UPPERBOUND + 1,
   JP_UPPERBOUND = JP_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* user/project sublist job usage */
   UPU_LOWERBOUND = JP_UPPERBOUND + 1,
   UPU_UPPERBOUND = UPU_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* checkpointing object */
   CK_LOWERBOUND = UPU_UPPERBOUND + 1,
   CK_UPPERBOUND = CK_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* kerberos connection list  */
   KRB_LOWERBOUND = CK_UPPERBOUND + 1,
   KRB_UPPERBOUND = KRB_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* path alias list */
   PA_LOWERBOUND = KRB_UPPERBOUND + 1,
   PA_UPPERBOUND = PA_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* job reference */
   JRE_LOWERBOUND = PA_UPPERBOUND + 1,
   JRE_UPPERBOUND = JRE_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* id struct used for qmod request */
   ID_LOWERBOUND = JRE_UPPERBOUND + 1,
   ID_UPPERBOUND = ID_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* multi gdi struct used for combined gdi requests */
   MA_LOWERBOUND = ID_UPPERBOUND + 1,
   MA_UPPERBOUND = MA_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* multi gdi struct used for combined gdi requests */
   TE_LOWERBOUND = MA_UPPERBOUND + 1,
   TE_UPPERBOUND = TE_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* CAL, CA, TMR and TM are for calendar management */
   /* SGE calendar object */
   CAL_LOWERBOUND = TE_UPPERBOUND + 1,
   CAL_UPPERBOUND = CAL_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* parsed calendar week or year */
   CA_LOWERBOUND = CAL_UPPERBOUND + 1,
   CA_UPPERBOUND = CA_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* time range */
   TMR_LOWERBOUND = CA_UPPERBOUND + 1,
   TMR_UPPERBOUND = TMR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* cullified struct tm */
   TM_LOWERBOUND = TMR_UPPERBOUND + 1,
   TM_UPPERBOUND = TM_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* remote tasks */
   RT_LOWERBOUND = TM_UPPERBOUND + 1,
   RT_UPPERBOUND = RT_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* user/project sublist project usage */
   UPP_LOWERBOUND = RT_UPPERBOUND + 1,
   UPP_UPPERBOUND = UPP_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* ptf osjobid sublist */
   JO_LOWERBOUND = UPP_UPPERBOUND + 1,
   JO_UPPERBOUND = JO_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* Kerberos TGT sublist */
   KTGT_LOWERBOUND = JO_UPPERBOUND + 1,
   KTGT_UPPERBOUND = KTGT_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler information messages */
   SME_LOWERBOUND = KTGT_UPPERBOUND + 1,
   SME_UPPERBOUND = SME_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler job information element */
   MES_LOWERBOUND = SME_UPPERBOUND + 1,
   MES_UPPERBOUND = MES_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* JobArray task structure */
   JAT_LOWERBOUND = MES_UPPERBOUND +1,
   JAT_UPPERBOUND = JAT_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler job category element */
   CT_LOWERBOUND = JAT_UPPERBOUND + 1,
   CT_UPPERBOUND = CT_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler access tree: priority group */
   PGR_LOWERBOUND = CT_UPPERBOUND + 1,
   PGR_UPPERBOUND = PGR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler access tree: user */
   USR_LOWERBOUND = PGR_UPPERBOUND + 1,
   USR_UPPERBOUND = USR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler access tree: job reference list */
   JRL_LOWERBOUND = USR_UPPERBOUND + 1,
   JRL_UPPERBOUND = JRL_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* scheduler sge job sort element */
   SGEJ_LOWERBOUND = JRL_UPPERBOUND + 1,
   SGEJ_UPPERBOUND = SGEJ_LOWERBOUND + 1*BASIC_UNIT - 1,

   ULNG_LOWERBOUND = SGEJ_UPPERBOUND + 1,
   ULNG_UPPERBOUND = ULNG_LOWERBOUND + 1*BASIC_UNIT - 1,

   PREF_LOWERBOUND = ULNG_UPPERBOUND + 1,
   PREF_UPPERBOUND = PREF_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* hostgroup */
   HGRP_LOWERBOUND = PREF_UPPERBOUND + 1,
   HGRP_UPPERBOUND = HGRP_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* host/hostgroup reference list */
   HR_LOWERBOUND = HGRP_UPPERBOUND + 1,
   HR_UPPERBOUND = HR_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* permissions list */
   PERM_LOWERBOUND = HR_UPPERBOUND + 1,
   PERM_UPPERBOUND = PERM_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* user mapping entry list */
   CU_LOWERBOUND = PERM_UPPERBOUND + 1,
   CU_UPPERBOUND = CU_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* load sensor list */
   LS_LOWERBOUND = CU_UPPERBOUND + 1,
   LS_UPPERBOUND = LS_LOWERBOUND + 1*BASIC_UNIT - 1,  

   /* user unknown list */
   RU_LOWERBOUND = LS_UPPERBOUND + 1,
   RU_UPPERBOUND = RU_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* feature sets */
   FES_LOWERBOUND = RU_UPPERBOUND + 1,
   FES_UPPERBOUND = FES_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* feature list */
   FE_LOWERBOUND = FES_UPPERBOUND + 1,
   FE_UPPERBOUND = FE_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* submit user */
   SU_LOWERBOUND = FE_UPPERBOUND + 1,
   SU_UPPERBOUND = SU_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* security connection list  */
   SEC_LOWERBOUND = SU_UPPERBOUND + 1,
   SEC_UPPERBOUND = SEC_LOWERBOUND + 1*BASIC_UNIT - 1,

   /* spooling framework */
   SPC_LOWERBOUND = SEC_UPPERBOUND + 1,
   SPC_UPPERBOUND = SPC_LOWERBOUND + 1 * BASIC_UNIT -1,

   SPR_LOWERBOUND = SPC_UPPERBOUND + 1,
   SPR_UPPERBOUND = SPR_LOWERBOUND + 1 * BASIC_UNIT -1,

   SPT_LOWERBOUND = SPR_UPPERBOUND + 1,
   SPT_UPPERBOUND = SPT_LOWERBOUND + 1 * BASIC_UNIT -1,

   SPTR_LOWERBOUND = SPT_UPPERBOUND + 1,
   SPTR_UPPERBOUND = SPTR_LOWERBOUND + 1 * BASIC_UNIT -1,

   SPM_LOWERBOUND = SPTR_UPPERBOUND + 1,
   SPM_UPPERBOUND = SPM_LOWERBOUND + 1 * BASIC_UNIT - 1,

   /* JAPI */
   JJ_LOWERBOUND = SPM_UPPERBOUND + 1,
   JJ_UPPERBOUND = JJ_LOWERBOUND + 1 * BASIC_UNIT -1,

   JJAT_LOWERBOUND = JJ_UPPERBOUND + 1,
   JJAT_UPPERBOUND = JJAT_LOWERBOUND + 1 * BASIC_UNIT -1,

   NSV_LOWERBOUND = JJAT_UPPERBOUND + 1,
   NSV_UPPERBOUND = NSV_LOWERBOUND + 1 * BASIC_UNIT -1,

   ASTR_LOWERBOUND = NSV_UPPERBOUND + 1,
   ASTR_UPPERBOUND = ASTR_LOWERBOUND + 1 * BASIC_UNIT -1,

   AULNG_LOWERBOUND = ASTR_UPPERBOUND + 1,
   AULNG_UPPERBOUND = AULNG_LOWERBOUND + 1 * BASIC_UNIT -1,

   ABOOL_LOWERBOUND = AULNG_UPPERBOUND + 1,
   ABOOL_UPPERBOUND = ABOOL_LOWERBOUND + 1 * BASIC_UNIT -1,

   ATIME_LOWERBOUND = ABOOL_UPPERBOUND + 1,
   ATIME_UPPERBOUND = ATIME_LOWERBOUND + 1 * BASIC_UNIT -1,

   AMEM_LOWERBOUND = ATIME_UPPERBOUND + 1,
   AMEM_UPPERBOUND = AMEM_LOWERBOUND + 1 * BASIC_UNIT -1,

   AINTER_LOWERBOUND = AMEM_UPPERBOUND + 1,
   AINTER_UPPERBOUND = AINTER_LOWERBOUND + 1 * BASIC_UNIT -1,

   ASTRING_LOWERBOUND = AINTER_UPPERBOUND + 1,
   ASTRING_UPPERBOUND = ASTRING_LOWERBOUND + 1 * BASIC_UNIT -1,

   ASTRLIST_LOWERBOUND = ASTRING_UPPERBOUND + 1,
   ASTRLIST_UPPERBOUND = ASTRLIST_LOWERBOUND + 1 * BASIC_UNIT -1,

   AUSRLIST_LOWERBOUND = ASTRLIST_UPPERBOUND + 1,
   AUSRLIST_UPPERBOUND = AUSRLIST_LOWERBOUND + 1 * BASIC_UNIT -1,

   APRJLIST_LOWERBOUND = AUSRLIST_UPPERBOUND + 1,
   APRJLIST_UPPERBOUND = APRJLIST_LOWERBOUND + 1 * BASIC_UNIT -1,
   
   ACELIST_LOWERBOUND = APRJLIST_UPPERBOUND + 1,
   ACELIST_UPPERBOUND = ACELIST_LOWERBOUND + 1 * BASIC_UNIT -1,

   ASOLIST_LOWERBOUND = ACELIST_UPPERBOUND + 1,
   ASOLIST_UPPERBOUND = ASOLIST_LOWERBOUND + 1 * BASIC_UNIT -1,

   AQTLIST_LOWERBOUND = ASOLIST_UPPERBOUND + 1,
   AQTLIST_UPPERBOUND = AQTLIST_LOWERBOUND + 1 * BASIC_UNIT -1,

   CQ_LOWERBOUND = AQTLIST_UPPERBOUND + 1,
   CQ_UPPERBOUND = CQ_LOWERBOUND + MAX_DESCR_SIZE -1,

   QIM_LOWERBOUND = CQ_UPPERBOUND + 1,
   QIM_UPPERBOUND = QIM_LOWERBOUND + MAX_DESCR_SIZE -1,

   FCAT_LOWERBOUND = QIM_UPPERBOUND + 1,
   FCAT_UPPERBOUND = FCAT_LOWERBOUND + 1 * BASIC_UNIT - 1,     

   CTI_LOWERBOUND = FCAT_UPPERBOUND + 1,
   CTI_UPPERBOUND = CTI_LOWERBOUND + 1 * BASIC_UNIT - 1,

   CTQV_LOWERBOUND = CTI_UPPERBOUND + 1,                    /* CTQV_LOWERBOUND is not used anymore */
   CTQV_UPPERBOUND = CTQV_LOWERBOUND + 1 * BASIC_UNIT - 1,

   PARA_LOWERBOUND = CTQV_UPPERBOUND + 1,
   PARA_UPPERBOUND = PARA_LOWERBOUND + 1 * BASIC_UNIT - 1,

   PACK_LOWERBOUND = PARA_UPPERBOUND + 1,
   PACK_UPPERBOUND = PACK_LOWERBOUND + 1 * BASIC_UNIT - 1,

   /* XML control structures */
   XMLA_LOWERBOUND= PACK_UPPERBOUND + 1,
   XMLA_UPPERBOUND = XMLA_LOWERBOUND + 1 * BASIC_UNIT - 1, 

   /* XML control structures */
   XMLH_LOWERBOUND= XMLA_UPPERBOUND + 1,
   XMLH_UPPERBOUND = XMLH_LOWERBOUND + 1 * BASIC_UNIT - 1, 

   /* XML control structures */
   XMLS_LOWERBOUND= XMLH_UPPERBOUND + 1,
   XMLS_UPPERBOUND = XMLS_LOWERBOUND + 1 * BASIC_UNIT - 1, 

   /* XML control structures */
   XMLE_LOWERBOUND= XMLS_UPPERBOUND + 1,
   XMLE_UPPERBOUND = XMLE_LOWERBOUND + 1 * BASIC_UNIT - 1,
             
   /* resource diagram */
   RDE_LOWERBOUND = XMLE_UPPERBOUND + 1,
   RDE_UPPERBOUND = RDE_LOWERBOUND  + 1 * BASIC_UNIT -1,

   /* resource utilization */
   RUE_LOWERBOUND = RDE_UPPERBOUND + 1,
   RUE_UPPERBOUND = RUE_LOWERBOUND + 1 * BASIC_UNIT -1,

   /* queue end time iterator (scheduler only) */
   QETI_LOWERBOUND = RUE_UPPERBOUND + 1,
   QETI_UPPERBOUND = QETI_LOWERBOUND + 1 * BASIC_UNIT -1,

   /* structure to compute consumable load alarms */
   LDR_LOWERBOUND = QETI_LOWERBOUND + 1,
   LDR_UPPERBOUND = LDR_LOWERBOUND  + 1 * BASIC_UNIT -1,

   /* structure to compute consumable load alarms */
   QRL_LOWERBOUND = LDR_UPPERBOUND + 1,
   QRL_UPPERBOUND = QRL_LOWERBOUND  + 1 * BASIC_UNIT -1,
      
   CCT_LOWERBOUND = QRL_UPPERBOUND + 1,
   CCT_UPPERBOUND = CCT_LOWERBOUND + 1 * BASIC_UNIT -1,

   CQU_LOWERBOUND = CCT_LOWERBOUND + 1,
   CQU_UPPERBOUND = CQU_LOWERBOUND + 1 * BASIC_UNIT - 1,

   SCT_LOWERBOUND = CQU_UPPERBOUND + 1,
   SCT_UPPERBOUND = SCT_LOWERBOUND + 1 * BASIC_UNIT - 1,

   REF_LOWERBOUND = SCT_UPPERBOUND + 1,
   REF_UPPERBOUND = REF_LOWERBOUND + 1 * BASIC_UNIT - 1,

   RQS_LOWERBOUND = REF_UPPERBOUND + 1,
   RQS_UPPERBOUND = RQS_LOWERBOUND + 1 * BASIC_UNIT - 1,

   RQR_LOWERBOUND = RQS_UPPERBOUND + 1,
   RQR_UPPERBOUND = RQR_LOWERBOUND + 1 * BASIC_UNIT - 1,

   RQRF_LOWERBOUND = RQR_UPPERBOUND + 1,
   RQRF_UPPERBOUND = RQRF_LOWERBOUND + 1 * BASIC_UNIT - 1,

   RQRL_LOWERBOUND = RQRF_UPPERBOUND + 1,
   RQRL_UPPERBOUND = RQRL_LOWERBOUND + 1 * BASIC_UNIT - 1,

   AR_LOWERBOUND = RQRL_UPPERBOUND + 1,
   AR_UPPERBOUND = AR_LOWERBOUND + 1 * BASIC_UNIT - 1,

   ARA_LOWERBOUND = AR_UPPERBOUND + 1,
   ARA_UPPERBOUND = ARA_LOWERBOUND + 1 * BASIC_UNIT - 1,
      
   RQL_LOWERBOUND = ARA_UPPERBOUND + 1,
   RQL_UPPERBOUND = RQL_LOWERBOUND + 1 * BASIC_UNIT - 1,

   ACK_LOWERBOUND = RQL_UPPERBOUND + 1,
   ACK_UPPERBOUND = ACK_LOWERBOUND + 1 * BASIC_UNIT - 1,

   EVR_LOWERBOUND = ACK_UPPERBOUND + 1,
   EVR_UPPERBOUND = EVR_LOWERBOUND + 1 * BASIC_UNIT - 1,

   JSV_LOWERBOUND = EVR_UPPERBOUND + 1,
   JSV_UPPERBOUND = JSV_LOWERBOUND + 1 * BASIC_UNIT - 1,

   RTIC_LOWERBOUND = JSV_UPPERBOUND + 1,
   RTIC_UPPERBOUND = RTIC_LOWERBOUND + 1 * BASIC_UNIT - 1,

   /* list for all running processes under Linux */
   PRO_LOWERBOUND = RTIC_UPPERBOUND + 1,
   PRO_UPPERBOUND = PRO_LOWERBOUND + 1 * BASIC_UNIT - 1,

   /* list for process groups */
   GR_LOWERBOUND = PRO_UPPERBOUND + 1,
   GR_UPPERBOUND = GR_LOWERBOUND + 1 * BASIC_UNIT - 1, 

   /* list with information about processor binding */ 
   BN_LOWERBOUND = GR_UPPERBOUND + 1, 
   BN_UPPERBOUND = BN_LOWERBOUND + 1 * BASIC_UNIT - 1

#  define LAST_UPPERBOUND BN_UPPERBOUND

};

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_BOUNDARIES_H */
