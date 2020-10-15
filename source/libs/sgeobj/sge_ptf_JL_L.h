#ifndef __SGE_PTF_JL_L_H
#define __SGE_PTF_JL_L_H

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
 * This is the list type we use to hold the jobs which
 * are running under the local execution daemon.
 */

enum {
   JL_job_ID = JL_LOWERBOUND,
   JL_OS_job_list,
   JL_state,
   JL_tickets,
   JL_share,
   JL_ticket_share,
   JL_timeslice,
   JL_usage,
   JL_old_usage_value,
   JL_adjusted_usage,
   JL_last_usage,
   JL_old_usage,
   JL_proportion,
   JL_adjusted_proportion,
   JL_adjusted_current_proportion,
   JL_actual_proportion,
   JL_diff_proportion,
   JL_last_proportion,
   JL_curr_pri,
   JL_pri,
   JL_procfd,
   JL_interactive
};

LISTDEF(JL_Type)
   SGE_ULONG(JL_job_ID, CULL_DEFAULT)       /* job identifier */
   SGE_LIST(JL_OS_job_list, JO_Type, CULL_DEFAULT)   /* O.S. job list */
   SGE_ULONG(JL_state, CULL_DEFAULT)        /* job state (JL_JOB_*) */
   SGE_ULONG(JL_tickets, CULL_DEFAULT)      /* job tickets */
   SGE_DOUBLE(JL_share, CULL_DEFAULT)       /* ptf interval share */
   SGE_DOUBLE(JL_ticket_share, CULL_DEFAULT) /* ptf job ticket share */
   SGE_DOUBLE(JL_timeslice, CULL_DEFAULT)   /* ptf calculated timeslice (SX) */
   SGE_DOUBLE(JL_usage, CULL_DEFAULT)       /* ptf interval combined usage */
   SGE_DOUBLE(JL_old_usage_value, CULL_DEFAULT)     /* ptf interval combined usage */
   SGE_DOUBLE(JL_adjusted_usage, CULL_DEFAULT)      /* ptf interval adjusted usage */
   SGE_DOUBLE(JL_last_usage, CULL_DEFAULT)  /* ptf last interval combined usage */
   SGE_DOUBLE(JL_old_usage, CULL_DEFAULT)   /* prev interval combined usage */
   SGE_DOUBLE(JL_proportion, CULL_DEFAULT)
   SGE_DOUBLE(JL_adjusted_proportion, CULL_DEFAULT)
   SGE_DOUBLE(JL_adjusted_current_proportion, CULL_DEFAULT)
   SGE_DOUBLE(JL_actual_proportion, CULL_DEFAULT)
   SGE_DOUBLE(JL_diff_proportion, CULL_DEFAULT)
   SGE_DOUBLE(JL_last_proportion, CULL_DEFAULT)
   SGE_DOUBLE(JL_curr_pri, CULL_DEFAULT)
   SGE_LONG(JL_pri, CULL_DEFAULT)
   SGE_ULONG(JL_procfd, CULL_DEFAULT)       /* /proc file descriptor */
   SGE_ULONG(JL_interactive, CULL_DEFAULT)  /* interactive flag */
LISTEND 

NAMEDEF(JLN)
   NAME("JL_job_ID")
   NAME("JL_OS_job_list")
   NAME("JL_state")
   NAME("JL_tickets")
   NAME("JL_share")
   NAME("JL_ticket_share")
   NAME("JL_timeslice")
   NAME("JL_usage")
   NAME("JL_old_usage_value")
   NAME("JL_adjusted_usage")
   NAME("JL_last_usage")
   NAME("JL_old_usage")
   NAME("JL_proportion")
   NAME("JL_adjusted_proportion")
   NAME("JL_adjusted_current_proportion")
   NAME("JL_actual_proportion")
   NAME("JL_diff_proportion")
   NAME("JL_last_proportion")
   NAME("JL_curr_pri")
   NAME("JL_pri")
   NAME("JL_procfd")
   NAME("JL_interactive")
NAMEEND

#define JLS sizeof(JLN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_PTFL_H */
