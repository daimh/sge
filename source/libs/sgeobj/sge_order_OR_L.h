#ifndef __SGE_ORDER_OR_L_H
#define __SGE_ORDER_OR_L_H

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

enum {
   OR_type = OR_LOWERBOUND,
   OR_job_number,
   OR_ja_task_number,
   OR_job_version,
   OR_queuelist,
   OR_ticket,
   OR_joker,
   OR_pe,
   OR_ntix,
   OR_prio
};

LISTDEF(OR_Type)
   SGE_ULONG(OR_type, CULL_DEFAULT)         /* command */
   SGE_ULONG(OR_job_number, CULL_DEFAULT)   /* which job */
   SGE_ULONG(OR_ja_task_number, CULL_DEFAULT)       /* which JobArray task */
   SGE_ULONG(OR_job_version, CULL_DEFAULT)  /* which job version */
   SGE_LIST(OR_queuelist, OQ_Type, CULL_DEFAULT)     /* associated queue list */
   SGE_DOUBLE(OR_ticket, CULL_DEFAULT)      /* job tickets */
   SGE_LIST(OR_joker, CULL_ANY_SUBTYPE, CULL_DEFAULT)         
      /* 
       * Type of this sublist depends on OR_type!  
       *    ORT_start_job                        empty 
       *    ORT_remove_job                       empty  
       *    ORT_tickets                          reduced job element JB_Type
       *    ORT_update_*_usage                   reduced user/prj object UP_Type 
       *    ORT_share_tree                       reduced share tree root node STN_Type 
       *    ORT_remove_immediate_job             empty 
       *    ORT_job_schedd_info                  SME_Type 
       *    ORT_ptickets                         reduced job element JB_Type
       */
   SGE_STRING(OR_pe, CULL_DEFAULT)          /* which pe */
   SGE_DOUBLE(OR_ntix, CULL_DEFAULT)        /* normalized ticket amount sent with job start order */
   SGE_DOUBLE(OR_prio, CULL_DEFAULT)        /* priority sent with job start order */
LISTEND 

NAMEDEF(ORN)
     NAME("OR_type")
     NAME("OR_job_number")
     NAME("OR_ja_task_number")
     NAME("OR_job_version")
     NAME("OR_queuelist")
     NAME("OR_ticket")
     NAME("OR_joker")
     NAME("OR_pe")
     NAME("OR_ntix")
     NAME("OR_prio")
     NAMEEND

#define ORS sizeof(ORN)/sizeof(char*)

/* *INDENT-ON* */ 
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_ORDERL_H */
