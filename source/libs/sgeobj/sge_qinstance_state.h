#ifndef __SGE_QINSTANCE_STATE_H
#define __SGE_QINSTANCE_STATE_H

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
#include "uti/sge_dstring.h"

/*
 * QI states
 */
#define QI_DEFAULT                     0x00000000 
#define QI_ALARM                       0x00000001
#define QI_SUSPEND_ALARM               0x00000002
#define QI_DISABLED                    0x00000004
#define QI_SUSPENDED                   0x00000100
#define QI_UNKNOWN                     0x00000400
#define QI_ERROR                       0x00004000
#define QI_SUSPENDED_ON_SUBORDINATE    0x00008000
#define QI_CAL_DISABLED                0x00020000
#define QI_CAL_SUSPENDED               0x00040000
#define QI_AMBIGUOUS                   0x00080000
#define QI_ORPHANED                    0x00100000
#define QI_FULL                        0x00200000

/*
 * QI state transition
 */
#define QI_DO_NOTHING                  0x00000000
#define QI_DO_DISABLE                  0x00000004
#define QI_DO_ENABLE                   0x00000008
#define QI_DO_UNSUSPEND                0x00000080
#define QI_DO_SUSPEND                  0x00000100
#define QI_DO_CLEARERROR               0x00004000
#define QI_DO_CLEAN                    0x00010000
#define QI_DO_RESCHEDULE               0x00080000
#define QI_DO_CAL_DISABLE              0x00020000
#define QI_DO_CAL_SUSPEND              0x00040000

#ifdef __SGE_QINSTANCE_STATE_DEBUG__
#  define QI_DO_SETERROR               0x00100000
#  define QI_DO_SETORPHANED            0x00200000
#  define QI_DO_CLEARORPHANED          0x00400000
#  define QI_DO_SETUNKNOWN             0x00800000
#  define QI_DO_CLEARUNKNOWN           0x01000000
#  define QI_DO_SETAMBIGUOUS           0x02000000
#  define QI_DO_CLEARAMBIGUOUS         0x04000000
#endif

/* job/queue state transition via job identifier */
#define JOB_DO_ACTION                  0x80000000
#define QUEUE_DO_ACTION                0x40000000

/*
 *
 */
#define QI_TRANSITION_NOTHING          0x00000000
#define QI_TRANSITION_OPTION           0x00000001

bool
transition_is_valid_for_qinstance(u_long32 transition, lList **answer_list);

bool
transition_option_is_valid_for_qinstance(u_long32 option, lList **answer_list);

#ifdef __SGE_QINSTANCE_STATE_DEBUG__
#  define QI_DO_SETERROR               0x00100000
#endif

/*
 *
 */
#define QI_TRANSITION_NOTHING          0x00000000
#define QI_TRANSITION_OPTION           0x00000001

bool
transition_is_valid_for_qinstance(u_long32 transition, lList **answer_list);

bool
transition_option_is_valid_for_qinstance(u_long32 option, lList **answer_list);

bool qinstance_has_state(const lListElem *this_elem, u_long32 bit); 

const char * qinstance_state_as_string(u_long32 bit);

u_long32 qinstance_state_from_string(const char* state, lList **answer_list, u_long32 filter);
/* */

bool
qinstance_state_set_alarm(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_suspend_alarm(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_manual_disabled(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_manual_suspended(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_unknown(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_error(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_susp_on_sub(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_cal_disabled(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_cal_suspended(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_full(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_orphaned(lListElem *this_elem, bool set_state);

bool
qinstance_state_set_ambiguous(lListElem *this_elem, bool set_state);

bool 
qinstance_state_is_alarm(const lListElem *this_elem);

bool 
qinstance_state_is_suspend_alarm(const lListElem *this_elem);

bool 
qinstance_state_is_manual_disabled(const lListElem *this_elem);

bool 
qinstance_state_is_manual_suspended(const lListElem *this_elem);

bool 
qinstance_state_is_unknown(const lListElem *this_elem);

bool 
qinstance_state_is_error(const lListElem *this_elem);

bool 
qinstance_state_is_susp_on_sub(const lListElem *this_elem);

bool 
qinstance_state_is_cal_disabled(const lListElem *this_elem);

bool 
qinstance_state_is_cal_suspended(const lListElem *this_elem);

bool
qinstance_state_is_orphaned(const lListElem *this_elem);

bool
qinstance_state_is_ambiguous(const lListElem *this_elem);

bool 
qinstance_state_is_full(const lListElem *this_elem);

bool 
qinstance_state_append_to_dstring(const lListElem *this_elem, dstring *string);

bool
qinstance_set_state(lListElem *this_elem, bool set_state, u_long32 bit);

#endif /* __SGE_QINSTANCE_STATE_H */
