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
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "cull/cull.h"
#include "cull/cull_list.h"

#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_host.h"

#include "sge_pe_schedd.h"
#include "valid_queue_user.h"
#include "sge_range_schedd.h"
#include "schedd_monitor.h"
#include "sge_schedd_text.h"
#include "schedd_message.h"
#include "msg_schedd.h"

#ifdef WIN32NATIVE
#	define strcasecmp( a, b) stricmp( a, b)
#	define strncasecmp( a, b, n) strnicmp( a, b, n)
#endif

 /* -------------------------------------------------
   
   get number of slots per host from alloc rule 

   a return value of >0 allocate exactly this number at each host 
                      0 indicates an unknown allocation rule
                     -1 = ALLOC_RULE_FILLUP indicates that simply hosts should 
                         be filled up sequentially 
                     -2 = ALLOC_RULE_ROUNDROBIN indicates that a round robin 
                        algorithm with all available host is used 
*/
int sge_pe_slots_per_host(
const lListElem *pep,
int slots 
) {
   const char *alloc_rule;
   int ret = 0;

   DENTER(TOP_LAYER, "sge_pe_slots_per_host");

   if (!pep) { /* seq jobs */
      DEXIT;
      return 1;
   }

   alloc_rule = lGetString(pep, PE_allocation_rule);

   if (isdigit((int)alloc_rule[0])) {
      ret = atoi(alloc_rule);
      if (ret==0) {
         ERROR((SGE_EVENT, MSG_PE_XFAILEDPARSINGALLOCATIONRULEY_SS , 
            lGetString(pep, PE_name), alloc_rule));
      }
   
      /* can we divide */
      else if ( (slots % ret)!=0 ) {
         DPRINTF(("pe >%s<: cant distribute %d slots "
         "using \"%s\" as alloc rule\n", 
         lGetString(pep, PE_name), slots, alloc_rule)); 
         ret = 0; 
      }

      DEXIT;
      return ret;
   }

   if  (!strcasecmp(alloc_rule, "$pe_slots")) {
      DEXIT;
      return slots;
   }

   if  (!strcasecmp(alloc_rule, "$fill_up")) {
      DEXIT;
      return ALLOC_RULE_FILLUP;
   }
      
   if  (!strcasecmp(alloc_rule, "$round_robin")) {
      DEXIT;
      return ALLOC_RULE_ROUNDROBIN;
   }

   ERROR((SGE_EVENT, MSG_PE_XFAILEDPARSINGALLOCATIONRULEY_SS , 
      lGetString(pep, PE_name), alloc_rule));

   DEXIT;
   return 0;
}


/****** sge_pe_schedd/pe_match_static() ******************************************
*  NAME
*     pe_match_static() -- Why not job to PE?
*
*  SYNOPSIS
*     int pe_match_static(lListElem *job, lListElem *pe, lList *acl_list, bool 
*     only_static_checks) 
*
*  FUNCTION
*     Checks if PE is suited for the job.
*
*  INPUTS
*     lListElem *job          - ??? 
*     lListElem *pe           - ??? 
*     lList *acl_list         - ??? 
*     bool only_static_checks - ??? 
*
*  RESULT
*     dispatch_t - DISPATCH_OK        ok 
*                  DISPATCH_NEVER_CAT assignment will never be possible for all
*                                     jobs of that category
*
*  NOTES
*     MT-NOTE: pe_restricted() is not MT safe 
*******************************************************************************/
dispatch_t pe_match_static(const sge_assignment_t *a) 
{
   int total_slots;

   DENTER(TOP_LAYER, "pe_match_static");

   total_slots = (int)lGetUlong(a->pe, PE_slots);
   if (total_slots == 0) { 
      /* because there are not enough PE slots in total */
      DPRINTF(("total slots %d of PE \"%s\" not in range of job "sge_u32"\n",
            total_slots, a->pe_name, a->job_id));
         schedd_mes_add(a->monitor_alpp, a->monitor_next_run, a->job_id,
                        SCHEDD_INFO_TOTALPESLOTSNOTINRANGE_S, a->pe_name);
      DEXIT;
      return DISPATCH_NEVER_CAT;
   }

   if (!sge_has_access_(a->user, a->group, lGetList(a->pe, PE_user_list), 
            lGetList(a->pe, PE_xuser_list), a->acl_list)) {
      DPRINTF(("job %d has no access to parallel environment \"%s\"\n",
              (int)a->job_id, a->pe_name));
      schedd_mes_add(a->monitor_alpp, a->monitor_next_run, a->job_id,
                     SCHEDD_INFO_NOACCESSTOPE_S, a->pe_name);
      DRETURN(DISPATCH_NEVER_CAT);
   }

   DRETURN(DISPATCH_OK);
}
