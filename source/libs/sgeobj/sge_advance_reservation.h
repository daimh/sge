#ifndef __SGE_ADVANCE_RESERVATION_H
#define __SGE_ADVANCE_RESERVATION_H 
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

#include "sgeobj/sge_advance_reservation_AR_L.h"
#include "sgeobj/sge_advance_reservation_ARA_L.h"

/* values for AR_verify */
#define AR_OPTION_VERIFY_STR "ev"
enum {
   AR_ERROR_VERIFY = 0,
   AR_JUST_VERIFY
};

/* AR states for AR_state */
typedef enum {
   AR_UNKNOWN = 0,  /* should never be seen. only used for variable initialisation */

   AR_WAITING,      /* w   waiting - granted but start time not reached */
   AR_RUNNING,      /* r   running - start time reached */
   AR_EXITED,       /* x   exited - end time reached and doing cleanup */
   AR_DELETED,      /* d   deleted - manual deletion */
   AR_ERROR,        /* E   error - AR became invalid and start time is reached */
   AR_WARNING       /* W   error - AR became invalid but start time not reached */

   /* append new states below! otherwise update procedures for reporting an accounting 
    * have to be written. change ar_state2dstring if you change something here */

} ar_state_t;

/* AR event types which trigger state changes */

typedef enum {
   ARL_UNKNOWN = 0, /* should never be seen. only used for variable initialisation */

   ARL_CREATION,           /* incoming request to create a new ar object */
   ARL_STARTTIME_REACHED,  /* start time reached */
   ARL_ENDTIME_REACHED,    /* end time reached */
   ARL_UNSATISFIED,        /* resources are unsatisfied */
   ARL_OK,                 /* resources are OK after they were unsatisfied */
   ARL_TERMINATED,         /* ar object deleted */
   ARL_DELETED

   /* append new events below! otherwise update procedures for reporting an accounting 
    * have to be written*/

} ar_state_event_t;

lListElem *
ar_list_locate(lList *ar_list, u_long32 job_id);

bool 
ar_validate(lListElem *ar, lList **alpp, bool in_master, bool is_spool);

ar_state_event_t 
ar_get_event_from_string(const char *string);

const char *
ar_get_string_from_event(ar_state_event_t event);

void ar_state2dstring(ar_state_t state, dstring *state_as_string);

bool sge_ar_has_errors(lListElem *ar);

int ar_list_sort(lList *this_list);

#endif /* __SGE_ADVANCE_RESERVATION_H */
