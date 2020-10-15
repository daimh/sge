#ifndef __SGE_USERPRJ_PR_L_H
#define __SGE_USERPRJ_PR_L_H

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


/*
 * This is the list type we use to hold the 
 * information for project. This objects are targets of throwing
 * tickets to them and as usage accumulators.
 */
enum {
   PR_name = PR_LOWERBOUND,
   PR_oticket,
   PR_fshare,
   PR_job_cnt,
   PR_pending_job_cnt,
   PR_usage,
   PR_usage_time_stamp,
   PR_usage_seqno,
   PR_long_term_usage,
   PR_project,
   PR_acl,
   PR_xacl,
   PR_debited_job_usage,
   PR_version,
   PR_consider_with_categories
};

enum {
   PR_name_POS = 0,
   PR_oticket_POS,
   PR_fshare_POS,
   PR_job_cnt_POS,
   PR_pending_job_cnt_POS,
   PR_usage_POS,
   PR_usage_time_stamp_POS,
   PR_usage_seqno_POS,
   PR_long_term_usage_POS,
   PR_project_POS,
   PR_acl_POS,
   PR_xacl_POS,
   PR_debited_job_usage_POS,
   PR_version_POS,
   PR_consider_with_categories_POS
};

LISTDEF(PR_Type)
   JGDI_ROOT_OBJ(Project, SGE_PR_LIST, ADD | MODIFY | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_PROJECT_ADD) | MODIFY(sgeE_PROJECT_MOD) | DELETE(sgeE_PROJECT_DEL) | GET_LIST(sgeE_PROJECT_LIST))

   /* configured project name spooled */
   SGE_STRING_D(PR_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_SUBLIST | CULL_JGDI_CONF, "template")

   /* configured override tickets (set by Qmon, used by schedd) spooled */
   SGE_ULONG(PR_oticket, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   /* configured functional shares (set by Qmon, used by schedd) spooled */
   SGE_ULONG(PR_fshare, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   /* job count (set and used by schedd, not spooled) schedd local, not stored to qmaster */
   SGE_ULONG(PR_job_cnt, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   /* job count (set and used by schedd, not spooled) schedd local, not stored to qmaster */
   SGE_ULONG(PR_pending_job_cnt, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   /* UA_Type; decayed usage set and used by schedd stored to qmaster; spooled */
   SGE_MAP(PR_usage, UA_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* time stamp of last decay set when * PR_usage changes; set and used by schedd stored to qmaster; spooled */
   SGE_ULONG(PR_usage_time_stamp, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* usage sequence number set and used by SGE schedd, not stored to qmaster; not spooled */
   SGE_ULONG(PR_usage_seqno, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   /* UA_Type; long term accumulated non-decayed i usage; set by schedd stored to qmaster; spooled */
   SGE_MAP(PR_long_term_usage, UA_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* UPP_Type; usage on a project basis set and used by schedd stored to qmaster; spooled Only used by projects */
   SGE_LIST(PR_project, UPP_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* US_Type but only names are filled configured excluded user access list used by schedd; spooled */
   SGE_LIST(PR_acl, US_Type, CULL_DEFAULT | CULL_SPOOL_PROJECT | CULL_JGDI_CONF)

   /* US_Type but only names are filled configured excluded user access list used by schedd; spooled */
   SGE_LIST(PR_xacl, US_Type, CULL_DEFAULT | CULL_SPOOL_PROJECT | CULL_JGDI_CONF)

   /* UPU_Type (see below) still debited usage per job (set and used by schedd) */
   SGE_LIST(PR_debited_job_usage, UPU_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* user/project version, increments when usage is updated, stored to qmaster, not spooled */
   SGE_ULONG(PR_version, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   /* true, if project plays role with categories */
   SGE_BOOL(PR_consider_with_categories, CULL_DEFAULT | CULL_JGDI_HIDDEN)
LISTEND 

NAMEDEF(PRN)
   NAME("PR_name")
   NAME("PR_oticket")
   NAME("PR_fshare")
   NAME("PR_job_cnt")
   NAME("PR_pending_job_cnt")
   NAME("PR_usage")
   NAME("PR_usage_time_stamp")
   NAME("PR_usage_seqno")
   NAME("PR_long_term_usage")
   NAME("PR_project")
   NAME("PR_acl")
   NAME("PR_xacl")
   NAME("PR_debited_job_usage")
   NAME("PR_version")
   NAME("PR_consider_with_categories")
NAMEEND

#define PRS sizeof(PRN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_USERPRJL_H */
