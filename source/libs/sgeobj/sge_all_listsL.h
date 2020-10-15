#ifndef __SGE_ALL_LISTSL_H
#define __SGE_ALL_LISTSL_H

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

#include "basis_types.h"
#include "cull/cull.h"

/* Definition of new names */

#include "sgeobj/sge_access_tree_PGR_L.h"
#include "sgeobj/sge_access_tree_USR_L.h"
#include "sgeobj/sge_access_tree_JRL_L.h"
#include "sgeobj/sge_ack_ACK_L.h"
#include "sgeobj/sge_advance_reservation_AR_L.h"
#include "sgeobj/sge_advance_reservation_ARA_L.h"
#include "sgeobj/sge_answer_AN_L.h"
#include "sgeobj/sge_attr_ASTR_L.h"
#include "sgeobj/sge_attr_AULNG_L.h"
#include "sgeobj/sge_attr_ABOOL_L.h"
#include "sgeobj/sge_attr_ATIME_L.h"
#include "sgeobj/sge_attr_AMEM_L.h"
#include "sgeobj/sge_attr_AINTER_L.h"
#include "sgeobj/sge_attr_ASTRING_L.h"
#include "sgeobj/sge_attr_ASTRLIST_L.h"
#include "sgeobj/sge_attr_AUSRLIST_L.h"
#include "sgeobj/sge_attr_APRJLIST_L.h"
#include "sgeobj/sge_attr_ACELIST_L.h"
#include "sgeobj/sge_attr_ASOLIST_L.h"
#include "sgeobj/sge_attr_AQTLIST_L.h"
#include "sgeobj/sge_binding_BN_L.h"
#include "sgeobj/sge_calendar_CAL_L.h"
#include "sgeobj/sge_calendar_CA_L.h"
#include "sgeobj/sge_calendar_CQU_L.h"
#include "sgeobj/sge_calendar_TMR_L.h"
#include "sgeobj/sge_calendar_TM_L.h"
#include "sgeobj/sge_centry_CE_L.h"
#include "sgeobj/sge_ckpt_CK_L.h"
#include "sgeobj/sge_conf_CONF_L.h"
#include "sgeobj/sge_conf_CF_L.h"
#include "sgeobj/sge_cqueue_CQ_L.h"
#include "sgeobj/sge_ct_SCT_L.h"
#include "sgeobj/sge_ct_REF_L.h"
#include "sgeobj/sge_ct_CT_L.h"
#include "sgeobj/sge_ct_CCT_L.h"
#include "sgeobj/sge_ct_CTI_L.h"
#include "sgeobj/sge_cull_xml_XMLA_L.h"
#include "sgeobj/sge_cull_xml_XMLS_L.h"
#include "sgeobj/sge_cull_xml_XMLH_L.h"
#include "sgeobj/sge_cull_xml_XMLE_L.h"
#include "sgeobj/sge_cuser_CU_L.h"
#include "sgeobj/sge_eejob_FCAT_L.h"
#include "sgeobj/sge_eejob_SGEJ_L.h"
#include "sgeobj/sge_event_EV_L.h"
#include "sgeobj/sge_event_EVS_L.h"
#include "sgeobj/sge_event_ET_L.h"
#include "sgeobj/sge_event_request_EVR_L.h"
#include "sgeobj/sge_feature_FES_L.h"
#include "sgeobj/sge_helper_QAJ_L.h"
#include "sgeobj/sge_hgroup_HGRP_L.h"
#include "sgeobj/sge_host_EH_L.h"
#include "sgeobj/sge_host_RU_L.h"
#include "sgeobj/sge_host_AH_L.h"
#include "sgeobj/sge_host_SH_L.h"
#include "sgeobj/sge_host_HL_L.h"
#include "sgeobj/sge_host_HS_L.h"
#include "sgeobj/sge_href_HR_L.h"
#include "sgeobj/sge_id_ID_L.h"
#include "sgeobj/sge_japi_JJ_L.h"
#include "sgeobj/sge_japi_JJAT_L.h"
#include "sgeobj/sge_japi_NSV_L.h"
#include "sgeobj/sge_ja_task_JAT_L.h"
#include "sgeobj/sge_job_AT_L.h"
#include "sgeobj/sge_job_JB_L.h"
#include "sgeobj/sge_job_JG_L.h"
#include "sgeobj/sge_job_PN_L.h"
#include "sgeobj/sge_job_ref_JRE_L.h"
#include "sgeobj/sge_jsv_JSV_L.h"
#include "sgeobj/sge_krb_KRB_L.h"
#include "sgeobj/sge_krb_KTGT_L.h"
#include "sgeobj/sge_loadsensor_LS_L.h"
#include "sgeobj/sge_mailrec_MR_L.h"
#include "sgeobj/sge_manop_UM_L.h"
#include "sgeobj/sge_manop_UO_L.h"
#include "sgeobj/sge_mesobj_QIM_L.h"
#include "sgeobj/sge_message_SME_L.h"
#include "sgeobj/sge_message_MES_L.h"
#include "sgeobj/sge_multi_MA_L.h"
#include "sgeobj/sge_order_OR_L.h"
#include "sgeobj/sge_order_OQ_L.h"
#include "sgeobj/sge_order_RTIC_L.h"
#include "sgeobj/sge_pack_PACK_L.h"
#include "sgeobj/sge_parse_SPA_L.h"
#include "sgeobj/sge_path_alias_PA_L.h"
#include "sgeobj/sge_pe_PE_L.h"
#include "sgeobj/sge_pe_task_FPET_L.h"
#include "sgeobj/sge_pe_task_PET_L.h"
#include "sgeobj/sge_pe_task_PETR_L.h"
#include "sgeobj/sge_permission_PERM_L.h"
#include "sgeobj/sge_pref_PREF_L.h"
#include "sgeobj/sge_ptf_JL_L.h"
#include "sgeobj/sge_ptf_JO_L.h"
#include "sgeobj/sge_ptf_JP_L.h"
#include "sgeobj/sge_qeti_QETI_L.h"
#include "sgeobj/sge_qexec_RT_L.h"
#include "sgeobj/sge_qinstance_QU_L.h"
#include "sgeobj/sge_qref_QR_L.h"
#include "sgeobj/sge_range_RN_L.h"
#include "sgeobj/sge_report_REP_L.h"
#include "sgeobj/sge_report_JR_L.h"
#include "sgeobj/sge_report_LIC_L.h"
#include "sgeobj/sge_report_LR_L.h"
#include "sgeobj/sge_resource_quota_RQS_L.h"
#include "sgeobj/sge_resource_quota_RQR_L.h"
#include "sgeobj/sge_resource_quota_RQRF_L.h"
#include "sgeobj/sge_resource_quota_RQRL_L.h"
#include "sgeobj/sge_resource_quota_RQL_L.h"
#include "sgeobj/sge_resource_utilization_RDE_L.h"
#include "sgeobj/sge_resource_utilization_RUE_L.h"
#include "sgeobj/sge_schedd_conf_PARA_L.h"
#include "sgeobj/sge_schedd_conf_SC_L.h"
#include "sgeobj/sge_sec_SEC_L.h"
#include "sgeobj/sge_select_queue_LDR_L.h"
#include "sgeobj/sge_select_queue_QRL_L.h"
#include "sgeobj/sge_sharetree_STN_L.h"
#include "sgeobj/sge_spooling_SPC_L.h"
#include "sgeobj/sge_spooling_SPR_L.h"
#include "sgeobj/sge_spooling_SPT_L.h"
#include "sgeobj/sge_spooling_SPTR_L.h"
#include "sgeobj/sge_spooling_SPM_L.h"
#include "sgeobj/sge_str_ST_L.h"
#include "sgeobj/sge_str_STU_L.h"
#include "sgeobj/sge_subordinate_SO_L.h"
#include "sgeobj/sge_suser_SU_L.h"
#include "sgeobj/sge_time_event_TE_L.h"
#include "sgeobj/sge_ulong_ULNG_L.h"
#include "sgeobj/sge_usage_UA_L.h"
#include "sgeobj/sge_userprj_PR_L.h"
#include "sgeobj/sge_userprj_UU_L.h"
#include "sgeobj/sge_userprj_UPU_L.h"
#include "sgeobj/sge_userprj_UPP_L.h"
#include "sgeobj/sge_userset_US_L.h"
#include "sgeobj/sge_userset_UE_L.h"
#include "sgeobj/sge_userset_JC_L.h"
#include "sgeobj/sge_var_VA_L.h"
#include "sgeobj/sge_procL.h"

#if defined(__SGE_GDI_LIBRARY_HOME_OBJECT_FILE__)
#ifdef  __cplusplus
extern "C" {
#endif

   lNameSpace nmv[] = {

/*  
   1. unique keq of the first element in the descriptor 
   2. number of elements in the descriptor 
   3. array with names describing the fields of the descriptor
          1.        2.           3.
*/
      {JB_LOWERBOUND, JBS, JBN},        /* job */
      {QU_LOWERBOUND, QUS, QUN},        /* Queue Instance list */
      {EH_LOWERBOUND, EHS, EHN},        /* exec host */
      {AH_LOWERBOUND, AHS, AHN},        /* admin host */
      {SH_LOWERBOUND, SHS, SHN},        /* submit host */
      {AN_LOWERBOUND, ANS, ANN},        /* gdi acknowledge format */
      {HL_LOWERBOUND, HLS, HLN},        /* load of an exec host */
      {HS_LOWERBOUND, HSS, HSN},        /* scaling of and exec host */
      {EV_LOWERBOUND, EVS, EVN},        /* event client */
      {ET_LOWERBOUND, ETS, ETN},        /* event */
      {CE_LOWERBOUND, CES, CEN},        /* complex entity */
      {LR_LOWERBOUND, LRS, LRN},        /* load report */
      {OR_LOWERBOUND, ORS, ORN},        /* ?? */
      {OQ_LOWERBOUND, OQS, OQN},        /* ?? */
      {US_LOWERBOUND, USES, USEN},      /* user set */
      {UE_LOWERBOUND, UES, UEN},        /* user set entry */
      {RN_LOWERBOUND, RNS, RNN},        /* range list */
      {PN_LOWERBOUND, PNS, PNN},        /* path name list */
      {AT_LOWERBOUND, ATS, ATN},        /* account list */
      {VA_LOWERBOUND, VAS, VAN},        /* variable list */
      {MR_LOWERBOUND, MRS, MRN},        /* mail recipiants list */
      {UM_LOWERBOUND, UMS, UMN},        /* manager list */
      {UO_LOWERBOUND, UOS, UON},        /* operator list */
      {PE_LOWERBOUND, PES, PEN},        /* parallel environment object */
      {QR_LOWERBOUND, QRS, QRN},        /* queue reference used in PE object */
      {JC_LOWERBOUND, JCS, JCN},        /* job couter used in schedd */
      {CONF_LOWERBOUND, CONFS, CONFN},  /* config */
      {CF_LOWERBOUND, CFS, CFN},        /* config list */
      {ST_LOWERBOUND, STS, STN},        /* string list */
      {STU_LOWERBOUND, STUS, STUN},     /* unique string list */
      {JG_LOWERBOUND, JGS, JGN},        /* jobs sublist of granted destinatin 
                                         * identifiers */
      {SO_LOWERBOUND, SOS, SON},        /* subordinate configuration list */
      {QAJ_LOWERBOUND, QAJS, QAJN},     /* list for qacct special purpose */
      {SPA_LOWERBOUND, SPAS, SPAN},     /* option parse struct */
      {REP_LOWERBOUND, REPS, REPN},     /* report list */
      {UA_LOWERBOUND, UAS, UAN},        /* usage list */
      {PR_LOWERBOUND, PRS, PRN},        /* project */
      {UU_LOWERBOUND, UUS, UUN},        /* user */
      {STN_LOWERBOUND, STNS, STNN},     /* share tree node */
      {SC_LOWERBOUND, SCS, SCN},        /* scheduler config */
      {PET_LOWERBOUND, PETS, PETN},     /* PE Task object */
      {PETR_LOWERBOUND, PETRS, PETRN},  /* PE Task request object */
      {FPET_LOWERBOUND, FPETS, FPETN},  /* finished PE Task reference */
      {JR_LOWERBOUND, JRS, JRN},        /* Job report */
      {LIC_LOWERBOUND, LICS, LICN},     /* structure of license report */
      {CK_LOWERBOUND, CKS, CKN},        /* checkpointing object */
      {UPU_LOWERBOUND, UPUS, UPUN},     /* sublist of user/project for
                                         * storing jobs old usage */
      {KRB_LOWERBOUND, KRBS, KRBN},     /* Kerberos connection list */
      {PA_LOWERBOUND, PAS, PAN},        /* Path alias list */
      {JRE_LOWERBOUND, JRES, JREN},     /* job reference */
      {ID_LOWERBOUND, IDS, IDN},        /* id struct used for qmod requests */
      {MA_LOWERBOUND, MAS, MAN},        /* ma struct used for multi gdi
                                         * requests */

      {TE_LOWERBOUND, TES, TEN},        /* time event struct used for timer
                                         * in qmaster */
      {CAL_LOWERBOUND, CALS, CALN},     /* calendar week/year */
      {CA_LOWERBOUND, CAS, CAN},        /* calendar week/year */
      {TMR_LOWERBOUND, TMRS, TMRN},     /* time range */
      {TM_LOWERBOUND, TMS, TMN},        /* cullified struct tm */

      {RT_LOWERBOUND, RTS, RTN},        /* remote task (qrexec) */
      {UPP_LOWERBOUND, UPPS, UPPN},     /* sublist of user/project for
                                         * storing project usage */
      {KTGT_LOWERBOUND, KTGTS, KTGTN},  /* Kerberos TGT list */
      {SME_LOWERBOUND, SMES, SMEN},     /* scheduler message structure */
      {MES_LOWERBOUND, MESS, MESN},     /* scheduler job info */
      {JAT_LOWERBOUND, JATS, JATN},     /* JobArray task structure contains
                                         * the dynamic elements of a Job */
      {CT_LOWERBOUND, CTS, CTN},        /* scheduler job category */

      {PGR_LOWERBOUND, PGRS, PGRN},     /* scheduler access tree: priority
                                         * group */
      {USR_LOWERBOUND, USRS, USRN},     /* scheduler access tree: user */
      {JRL_LOWERBOUND, JRLS, JRLN},     /* scheduler access tree: job
                                         * reference */
      {JL_LOWERBOUND, JLS, JLN},        /* ptf job list */
      {JO_LOWERBOUND, JOS, JON},        /* ptf O.S. job list */

      {HGRP_LOWERBOUND, HGRPS, HGRPN},  /* hostgroup list */
      {HR_LOWERBOUND, HRS, HRN},        /* host/group reference list */
      {PERM_LOWERBOUND, PERMS, PERMN},  /* permission list */
      {CU_LOWERBOUND, CUS, CUN},        /* usermap entry list for
                                         * administrator mapping */
      {LS_LOWERBOUND, LSS, LSN},        /* load sensor list */

      {RU_LOWERBOUND, RUS, RUN},        /* user unknown list */
      {FES_LOWERBOUND, FESS, FESN},
      
      {SU_LOWERBOUND, SUS, SUN},        /* submit user */
      {SEC_LOWERBOUND, SECS, SECN},    /* Certificate security */

      {SPC_LOWERBOUND, SPCS, SPCN},     /* Spooling context */
      {SPR_LOWERBOUND, SPRS, SPRN},     /* Spooling rule */
      {SPT_LOWERBOUND, SPTS, SPTN},     /* Spooling object type */
      {SPTR_LOWERBOUND, SPTRS, SPTRN},  /* Spooling rules for object type */
      {SPM_LOWERBOUND, SPMS, SPMN},     /* Mapping object->id */

      {JJ_LOWERBOUND, JJS, JJN},        /* JAPI job */
      {JJAT_LOWERBOUND, JJATS, JJATN},  /* JAPI array task */

      {ASTR_LOWERBOUND, ASTRS, ASTRN},          /* CQ string sublist */
      {AULNG_LOWERBOUND, AULNGS, AULNGN},       /* CQ u_long32 sublist */
      {ABOOL_LOWERBOUND, ABOOLS, ABOOLN},       /* CQ bool sublist */
      {ATIME_LOWERBOUND, ATIMES, ATIMEN},       /* CQ time limit sublist */
      {AMEM_LOWERBOUND, AMEMS, AMEMN},          /* CQ memory limit sublist */
      {AINTER_LOWERBOUND, AINTERS, AINTERN},    /* CQ interval sublist */
      {ASTRING_LOWERBOUND, ASTRINGS, ASTRINGN},
      {ASTRLIST_LOWERBOUND, ASTRLISTS, ASTRLISTN}, /* CQ ST_Type-list sublist */
      {AUSRLIST_LOWERBOUND, AUSRLISTS, AUSRLISTN}, /* CQ US_Type-list sublist */
      {APRJLIST_LOWERBOUND, APRJLISTS, APRJLISTN}, /* CQ PR_Type-list sublist */
      {ACELIST_LOWERBOUND, ACELISTS, ACELISTN},    /* CQ CE_Type-list sublist */
      {ASOLIST_LOWERBOUND, ASOLISTS, ASOLISTN},    /* CQ SO_Type-list sublist */
      {AQTLIST_LOWERBOUND, AQTLISTS, AQTLISTN},    /* CQ qtype sublist */
      {CQ_LOWERBOUND, CQS, CQN},                /* Cluster Queue list */
      {QIM_LOWERBOUND, QIMS, QIMN},                /* Queue Instance Messege list */
      {FCAT_LOWERBOUND, FCATS, FCATN},          /* Functional category */
      {CTI_LOWERBOUND, CTIS, CTIN},             /* ignore host/queue list in a job category */
      {PARA_LOWERBOUND, PARAS, PARAN},          /* store the configuration "params" parameters in a list */
      {ULNG_LOWERBOUND, ULNGS, ULNGN},          /* ???? info-messages ??? */
      {EVS_LOWERBOUND, EVSS, EVSN},              /* subscribed event list */

/* this would generate a cycle in the dependencies between lib cull and lib obj. Therefor
   we ignore the names here and live with the fact, that lWriteList or lWriteElem will
   not print the CULL_names for the PACK structure. */
/*      {PACK_LOWERBOUND, PACKS, PACKN},   */       /* a cull version of the pack buffer */

      {XMLA_LOWERBOUND, XMLAS, XMLAN},          /* XML-Attribute */
      {XMLS_LOWERBOUND, XMLSS, XMLSN},          /* XML-Stype-Sheet */
      {XMLH_LOWERBOUND, XMLHS, XMLHN},          /* XML-Header*/
      {XMLE_LOWERBOUND, XMLES, XMLEN},          /* XML-Element*/

      {RDE_LOWERBOUND, RDES, RDEN},             /* resource diagram */
      {RUE_LOWERBOUND, RUES, RUEN},             /* resource utilization */
      {QETI_LOWERBOUND, QETIS, QETIN},          /* queue end time iterator (scheduler) */

      {LDR_LOWERBOUND, LDRS, LDRN},             /* queue consumables load alarm structure */
      {QRL_LOWERBOUND, QRL_S, QRL_N},           /* queue consumables load alarm structure */

      {CCT_LOWERBOUND, CCTS, CCTN},

      {CQU_LOWERBOUND, CQUS, CQUN},             /* queue state changes structure */
      
      {SCT_LOWERBOUND, SCTS, SCTN},             /* scheduler categories */

      {REF_LOWERBOUND, REFS, REFN},             /* a simple ref object */

      {RQS_LOWERBOUND, RQSS, RQSN},             /* resource quota set */
      {RQR_LOWERBOUND, RQRS, RQRN},             /* resource quota rule */
      {RQRF_LOWERBOUND, RQRFS, RQRFN},          /* resource quota rule filter */
      {RQRL_LOWERBOUND, RQRLS, RQRLN},          /* resource quota rule limit */
      {RQL_LOWERBOUND, RQLS, RQLN},             /* resource quota limit (scheduler) */
     
      {AR_LOWERBOUND, ARS, ARN},                /* advance reservation */ 
      {ARA_LOWERBOUND, ARAS, ARAN},             /* advance reservation acl*/ 
      
      {ACK_LOWERBOUND, ACKS, ACKN},             /* acknowledge */

      {EVR_LOWERBOUND, EVRS, EVRN},             /* event master requests */
      {JSV_LOWERBOUND, JSVS, JSVN},             /* job submission verifier */
      {RTIC_LOWERBOUND, RTICS, RTICN},          /* internal list for reprioritzie tickets to distribute */
      {PRO_LOWERBOUND, PROS, PRON},             /* list for all running processes under Linux */
      {GR_LOWERBOUND, GRS, GRN},                /* list of all process groups of Linux process */

      {BN_LOWERBOUND, BNS, BNN},                /* list of binding information */

      {0, 0, NULL}
   };

#ifdef  __cplusplus
}
#endif
#else
#ifdef __SGE_GDI_LIBRARY_SUBLIST_FILE__
#else
#ifdef  __cplusplus
extern "C" {
#endif

   extern lNameSpace nmv[];

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__ */
#endif                          /* __SGE_GDI_LIBRARY_SUBLIST_FILE__     */
#endif                          /* __SGE_ALL_LISTSL_H */
