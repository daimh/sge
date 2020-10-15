#ifndef __SGE_HOST_EH_L_H
#define __SGE_HOST_EH_L_H

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
 * exec host
 */
enum {
   EH_name = EH_LOWERBOUND,  /* host where queue instance resides  */
                             /* directly user controlled - spooled */
   EH_scaling_list,          /* HS_Type - directly user controlled exec
                              * host only Used to scale host load values.
                              * Contains pairs of load value names and
                              * doubles. spooled */
   EH_consumable_config_list, /* consumable resources of host
                               * CE_Type */
   EH_usage_scaling_list,    /* HS_Type - directly user controlled exec
                              * host only scaling of usage
                              * spooled */
   EH_load_list,             /* HL_Type exec host only list of load values 
                              * reported for exechost spooled (not
                              * everytime load comes in) */
   EH_lt_heard_from,
   EH_processors,            /* for license purposes exec host only
                              * spooled */
   EH_acl,                   /* US_Type - userset access list */
   EH_xacl,                  /* US_Type - userset access list */
   EH_prj,                   /* PR_Type - project access list */
   EH_xprj,                  /* PR_Type - project excluded access list */

   /* scheduling stuff */
   EH_sort_value,            /* combined load value for sorting only
                              * scheduler local not spooled */
   EH_reuse_me,              /* can be rused */
   EH_tagged,                /* used by qhost only */
   EH_load_correction_factor,        /* a value of 100 (stands for 1)
                                      * means the load values of this host 
                                      * * has to be increased fully by all
                                      * values from
                                      * conf.load_decay_adjustments only
                                      * scheduler local not spooled */
   EH_seq_no,                /* suitability of this host for a job only
                              * scheduler local not spooled */
   EH_real_name,             /* in case of pseudo host: real name spooled */
   EH_sge_load,              /* load calculated from load values
                              * scheduler local */
   EH_sge_ticket_pct,        /* percentage of total tickets
                              * scheduler local */
   EH_sge_load_pct,          /* percentage of total load
                              * scheduler local */

   EH_featureset_id,         /* supported feature-set id; not spooled */
   EH_scaled_usage_list,     /* scaled usage for jobs on a host - used by
                              * sge_host_mon */
   EH_scaled_usage_pct_list, /* scaled usage for jobs on a host - used by
                              * sge_host_mon */
   EH_num_running_jobs,      /* number of jobs running on a host - used by 
                              * sge_host_mon */
   EH_load_report_interval,  /* used for caching from global/local
                              * configuration */
   EH_resource_utilization,  /* RUE_Type contains per consumable information 
                              * about resource utilization for this host */
   EH_cached_complexes,      /* CE_Type used in scheduler for caching
                              * built attributes */
   EH_cache_version,         /* used to decide whether QU_cached_complexes 
                              * needs a refresh */
   EH_master_host,           /* no longer used */
   EH_reschedule_unknown,    /* used for caching from global/local conf;
                              * timout after which jobs will be
                              * rescheduled automatically */
   EH_reschedule_unknown_list,       /* after the rundown of
                                      * EH_reschedule_unknown this list
                                      * contains all jobs which will be
                                      * rescheduled automatically */
   EH_report_seqno,          /* sequence number of the last report
                              * (job/load/..) qmaster received from the
                              * execd. This seqno is used to detect old
                              * reports, because reports are send
                              * asynchronous and we have no guarantee that 
                              * they arrive in order at qmaster */
   EH_report_variables,      /* list of variables written to the report file */
   EH_merged_report_variables /* list of variables written to the report file,
                              * merged from global host and actual host
                              */
};

LISTDEF(EH_Type)
   JGDI_ROOT_OBJ(ExecHost, SGE_EH_LIST, ADD | MODIFY | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_EXECHOST_ADD) | MODIFY(sgeE_EXECHOST_MOD) | DELETE(sgeE_EXECHOST_DEL) | GET_LIST(sgeE_EXECHOST_LIST))
   SGE_HOST_D(EH_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_JGDI_CONF, "template")
   SGE_MAP(EH_scaling_list, HS_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_LIST(EH_consumable_config_list, CE_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_MAP(EH_usage_scaling_list, HS_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_MAP(EH_load_list, HL_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_ULONG(EH_lt_heard_from, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_processors, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_LIST(EH_acl, US_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_LIST(EH_xacl, US_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_LIST(EH_prj, PR_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_LIST(EH_xprj, PR_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   /* scheduling stuff */
   SGE_DOUBLE(EH_sort_value, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_reuse_me, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_tagged, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_load_correction_factor, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_seq_no, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   SGE_STRING(EH_real_name, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   SGE_ULONG(EH_sge_load, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_DOUBLE(EH_sge_ticket_pct, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_DOUBLE(EH_sge_load_pct, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   SGE_ULONG(EH_featureset_id, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   SGE_MAP(EH_scaled_usage_list, UA_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_MAP(EH_scaled_usage_pct_list, UA_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_num_running_jobs, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   SGE_ULONG(EH_load_report_interval, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_LIST(EH_resource_utilization, RUE_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_LIST(EH_cached_complexes, CE_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_cache_version, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_master_host, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_ULONG(EH_reschedule_unknown, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_LIST(EH_reschedule_unknown_list, RU_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* JG: TODO: shall it be spooled? Problem: composed primary key */
   SGE_ULONG(EH_report_seqno, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_LIST(EH_report_variables, STU_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_LIST(EH_merged_report_variables, STU_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN)
LISTEND 

NAMEDEF(EHN)
   NAME("EH_name")
   NAME("EH_scaling_list")
   NAME("EH_consumable_config_list")
   NAME("EH_usage_scaling_list")
   NAME("EH_load_list")
   NAME("EH_lt_heard_from")
   NAME("EH_processors")
   NAME("EH_acl")
   NAME("EH_xacl")
   NAME("EH_prj")
   NAME("EH_xprj")

   /* scheduling stuff */
   NAME("EH_sort_value")
   NAME("EH_reuse_me")
   NAME("EH_tagged")
   NAME("EH_load_correction_factor")
   NAME("EH_seq_no")

   NAME("EH_real_name")

   NAME("EH_sge_load")
   NAME("EH_sge_ticket_pct")
   NAME("EH_sge_load_pct")

   NAME("EH_featureset_id")

   NAME("EH_scaled_usage_list")
   NAME("EH_scaled_usage_pct_list")
   NAME("EH_num_running_jobs")

   NAME("EH_load_report_interval")
   NAME("EH_resource_utilization")
   NAME("EH_cached_complexes")
   NAME("EH_cache_version")
   NAME("EH_master_host")
   NAME("EH_reschedule_unknown")
   NAME("EH_reschedule_unknown_list")
   NAME("EH_report_seqno")
   NAME("EH_report_variables")
   NAME("EH_merged_report_variables")
NAMEEND

#define EHS sizeof(EHN)/sizeof(char*)
/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_HOSTL_H */
