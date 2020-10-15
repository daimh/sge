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

#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_ja_task.h"

#include "dispatcher.h"
#include "execd_ticket.h"
#include "msg_execd.h"
#include "msg_common.h"

#ifdef COMPILE_DC
#  include "ptf.h"
#endif

/*************************************************************************
 EXECD function called by dispatcher

 get a list of jobid/tickets tuples and pass them to the PTF
 *************************************************************************/

int do_ticket(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg)
{
   u_long32 jobid, jataskid;
   double ticket;
   lListElem *job_ticket, *task_ticket;
   lList *ticket_modifier = NULL;

   DENTER(TOP_LAYER, "do_ticket");

   while (pb_unused(&(aMsg->buf))>0) {
      lList *jatasks = NULL;

      if (unpackint(&(aMsg->buf), &jobid) || unpackint(&(aMsg->buf), &jataskid)
          || unpackdouble(&(aMsg->buf), &ticket)) {
         ERROR((SGE_EVENT, SFNMAX, MSG_JOB_TICKETFORMAT));
         DRETURN(0);
      }
      DPRINTF(("got %lf new tickets for job "sge_u32"."sge_u32"\n", ticket, jobid, jataskid));
      job_ticket = lAddElemUlong(&ticket_modifier, JB_job_number, jobid, JB_Type);   
      if (job_ticket) {
         task_ticket = lAddElemUlong(&jatasks, JAT_task_number, jataskid, JAT_Type);
         if (task_ticket) {
            lSetDouble(task_ticket, JAT_tix, ticket);
         }
         lSetList(job_ticket, JB_ja_tasks, jatasks);
      }
   }
  
   DPRINTF(("got new tickets for %d jobs\n", lGetNumberOfElem(ticket_modifier)));

#ifdef COMPILE_DC
   { 
      int ptf_error;
      /* forward new tickets to ptf */
      if ((ptf_error=ptf_process_job_ticket_list(ticket_modifier))) {
         ERROR((SGE_EVENT, MSG_JOB_TICKETPASS2PTF_IS, 
            lGetNumberOfElem(ticket_modifier), 
            ptf_errstr(ptf_error)));
      }

      errno = 0;
      if (sge_switch2start_user() == 0) {
         DPRINTF(("ADJUST PRIORITIES\n"));
         ptf_adjust_job_priorities();
         sge_switch2admin_user();
      } else {
         CRITICAL((SGE_EVENT, MSG_SWITCH_USER_S, strerror(errno)));
      }
   }
#endif

   lFreeList(&ticket_modifier);

   DRETURN(0);
}


