#ifndef __SGE_USERPRJ_UU_L_H
#define __SGE_USERPRJ_UU_L_H

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
 * information for user. This objects are targets of throwing
 * tickets to them and as usage accumulators.
 */
enum {
   UU_name = UU_LOWERBOUND,
   UU_oticket,
   UU_fshare,
   UU_delete_time,
   UU_job_cnt,
   UU_pending_job_cnt,
   UU_usage,
   UU_usage_time_stamp,
   UU_usage_seqno,
   UU_long_term_usage,
   UU_project,
   UU_debited_job_usage,
   UU_default_project,
   UU_version,
   UU_consider_with_categories
};

enum {
   UU_name_POS = 0,
   UU_oticket_POS,
   UU_fshare_POS,
   UU_delete_time_POS,
   UU_job_cnt_POS,
   UU_pending_job_cnt_POS,
   UU_usage_POS,
   UU_usage_time_stamp_POS,
   UU_usage_seqno_POS,
   UU_long_term_usage_POS,
   UU_project_POS,
   UU_debited_job_usage_POS,
   UU_default_project_POS,
   UU_version_POS,
   UU_consider_with_categories_POS
};

LISTDEF(UU_Type)
   JGDI_ROOT_OBJ(User, SGE_UU_LIST, ADD | MODIFY | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_USER_ADD) | MODIFY(sgeE_USER_MOD) | DELETE(sgeE_USER_DEL) | GET_LIST(sgeE_USER_LIST))

   /* configured user name spooled */
   SGE_STRING_D(UU_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_SUBLIST | CULL_JGDI_CONF, "template")

   /* configured override tickets (set by Qmon, used by schedd) spooled */
   SGE_ULONG(UU_oticket, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   
   /* configured functional shares (set by Qmon, used by schedd) spooled */
   SGE_ULONG(UU_fshare, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   /* delete time for automatic users, (set by qmaster, * used by qmaster) spooled */
   SGE_ULONG(UU_delete_time, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   /* job count (set and used by schedd, not spooled) schedd local, not stored to qmaster */
   SGE_ULONG(UU_job_cnt, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   /* job count (set and used by schedd, not spooled) schedd local, not stored to qmaster */
   SGE_ULONG(UU_pending_job_cnt, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   /* UA_Type; decayed usage set and used by schedd stored to qmaster; spooled */
   SGE_MAP(UU_usage, UA_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* time stamp of last decay set when UU_usage changes; set and used by schedd stored to qmaster; spooled */
   SGE_ULONG(UU_usage_time_stamp, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* usage sequence number set and used by SGE schedd, not stored to qmaster; not spooled */
   SGE_ULONG(UU_usage_seqno, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   /* UA_Type; long term accumulated * non-decayed i usage; set by schedd stored to qmaster; spooled */
   SGE_MAP(UU_long_term_usage, UA_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* UPP_Type; usage on a project basis set and used by schedd stored to qmaster; spooled Only used by projects */
   SGE_LIST(UU_project, UPP_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* UPU_Type (see below) still debited usage per job (set and used by schedd) */
   SGE_LIST(UU_debited_job_usage, UPU_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)

   /* default project for user */
   SGE_STRING(UU_default_project, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)

   /* user/project version, increments when usage is updated, stored to qmaster, not spooled */
   SGE_ULONG(UU_version, CULL_DEFAULT | CULL_JGDI_HIDDEN)

   /* true, if project plays role with categories */
   SGE_BOOL(UU_consider_with_categories, CULL_DEFAULT | CULL_JGDI_HIDDEN)
LISTEND 

NAMEDEF(UUN)
   NAME("UU_name")
   NAME("UU_oticket")
   NAME("UU_fshare")
   NAME("UU_delete_time")
   NAME("UU_job_cnt")
   NAME("UU_pending_job_cnt")
   NAME("UU_usage")
   NAME("UU_usage_time_stamp")
   NAME("UU_usage_seqno")
   NAME("UU_long_term_usage")
   NAME("UU_project")
   NAME("UU_debited_job_usage")
   NAME("UU_default_project")
   NAME("UU_version")
   NAME("UU_consider_with_categories")
NAMEEND

#define UUS sizeof(UUN)/sizeof(char*)


/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif

#endif                         
