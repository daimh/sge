#ifndef __SGE_JOB_JG_L_H
#define __SGE_JOB_JG_L_H
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

/* ---------------------------------------- 

   granted destination identifiers 

*/

enum {
   JG_qname = JG_LOWERBOUND,
   JG_qversion,
   JG_qhostname,
   JG_slots,
   JG_queue,
   JG_tag_slave_job, /* tag just used in qmaster for bookkeeping during job
                        delivery, finish, and cleanup */
   JG_task_id_range,
   JG_ticket,
   JG_oticket,
   JG_fticket,
   JG_sticket,
   JG_jcoticket,
   JG_jcfticket,
   JG_processors
};

LISTDEF(JG_Type)
   SGE_STRING(JG_qname, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_DEFAULT | CULL_SUBLIST)    /* the queue instance name                           */
   SGE_ULONG(JG_qversion, CULL_DEFAULT | CULL_JGDI_HIDDEN)  /* it's version                               */
   SGE_HOST(JG_qhostname, CULL_DEFAULT | CULL_HASH | CULL_SUBLIST)/* redundant qualified host name for caching  */
   SGE_ULONG(JG_slots, CULL_DEFAULT | CULL_SUBLIST)     /* from orders list                           */
   SGE_OBJECT(JG_queue, QU_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN | CULL_JGDI_RO) /* QU_Type - complete queue only in execd */
   SGE_ULONG(JG_tag_slave_job, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* whether slave execds job has arrived in 
                                 * case of pe's with sge controlled slaves */
   SGE_ULONG(JG_task_id_range, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* unused - please recycle */
   SGE_DOUBLE(JG_ticket, CULL_DEFAULT | CULL_JGDI_HIDDEN)    /* tickets assigned to slots              */
   SGE_DOUBLE(JG_oticket, CULL_DEFAULT | CULL_JGDI_HIDDEN)   /* override tickets assigned to slots     */
   SGE_DOUBLE(JG_fticket, CULL_DEFAULT | CULL_JGDI_HIDDEN)   /* functional tickets assigned to slots   */
   SGE_DOUBLE(JG_sticket, CULL_DEFAULT | CULL_JGDI_HIDDEN)   /* sharetree tickets assigned to slots    */
   SGE_DOUBLE(JG_jcoticket, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* job class override tickets             */
   SGE_DOUBLE(JG_jcfticket, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* job class functional tickets           */
   SGE_STRING(JG_processors, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* processor sets */
LISTEND

NAMEDEF( JGN )
   NAME( "JG_qname" )
   NAME( "JG_qversion" )
   NAME( "JG_qhostname" )
   NAME( "JG_slots" )
   NAME( "JG_queue" )
   NAME( "JG_tag_slave_job" )
   NAME( "JG_task_id_range" )
   NAME( "JG_ticket" )
   NAME( "JG_oticket" )
   NAME( "JG_fticket" )
   NAME( "JG_sticket" )
   NAME( "JG_jcoticket" )
   NAME( "JG_jcfticket" )
   NAME( "JG_processors" )
NAMEEND

#define JGS sizeof(JGN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_JOBL_H */
