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

#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "cull/cull_list.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "sge.h"

/****** sgeobj/qinstance_state/--State_Chart() ********************************
*
*         /---------------------------------------------------\
*         |                     exists                        |
*         |                                                   |
* o-----> |                                                   |------->X
*         |                               /-----------------\ |
*         |                               |   (suspended)   | |
*         |                               |                 | |
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !s   |            |    |   s   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |- - - - - - - - - - - - - - - -|- - - - - - - - -|-|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !S   |            |    |   S   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |- - - - - - - - - - - - - - - -|- - - - - - - - -|-|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !A   |            |    |   A   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |- - - - - - - - - - - - - - - -|- - - - - - - - -|-|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !C   |            |    |   C   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |                               \-----------------/ |
*         |- - - - - - - - - - - - - - - - - - - - - - - - - -|
*         |                               /-----------------\ |
*         |                               |   (disabled)    | |
*         |                               |                 | |
*         |         /--------\            |    /-------\    | |
*         | o-----> |        |---------------> |       |    | |
*         |         |   !u   |            |    |   u   |    | |
*         |         |        | <---------------|       |    | |
*         |         \--------/            |    \-------/    | | 
*         |- - - - - - - - - - - - - - - -|- - - - - - - - - -|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !a   |            |    |   a   |    | |
*         |         |        | <---------------|       |    | |
*         |         \--------/            |    \-------/    | | 
*         |- - - - - - - - - - - - - - - -|- - - - - - - - - -|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !d   |            |    |   d   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |- - - - - - - - - - - - - - - -|- - - - - - - - -|-|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !D   |            |    |   D   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |- - - - - - - - - - - - - - - -|- - - - - - - - -|-|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !E   |            |    |   E   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |- - - - - - - - - - - - - - - -|- - - - - - - - -|-|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !c   |            |    |   c   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |- - - - - - - - - - - - - - - -|- - - - - - - - -|-|
*         |         /--------\            |    /-------\    | |   
*         | o-----> |        |---------------> |       |    | |
*         |         |   !o   |            |    |   o   |    | |
*         |         |        | <---------------|       |    | | 
*         |         \--------/            |    \-------/    | |
*         |                               \-----------------/ |
*         \---------------------------------------------------/
*
*         u := qinstance-host is unknown
*         a := load alarm
*         s := manual suspended
*         A := suspended due to suspend_threshold
*         S := suspended due to subordinate
*         C := suspended due to calendar
*         d := manual disabled
*         D := disabled due to calendar
*         c := configuration ambiguous
*         o := orphaned
*         P := preempted
*
*******************************************************************************/

#define QINSTANCE_STATE_LAYER TOP_LAYER

/* EB: ADOC: add commets */

static const u_long32 states[] = {
      QI_ALARM,
      QI_SUSPEND_ALARM,
      QI_CAL_SUSPENDED,
      QI_CAL_DISABLED,
      QI_DISABLED,
      QI_UNKNOWN,
      QI_ERROR,
      QI_SUSPENDED_ON_SUBORDINATE,
      QI_SUSPENDED,
      QI_AMBIGUOUS,
      QI_ORPHANED, 
      0
   };
   
static const char letters[] = {
      'a',
      'A',
      'C',
      'D',
      'd',
      'u',
      'E',
      'S',
      's',
      'c',
      'o',
      '\0'
   };

bool
qinstance_set_state(lListElem *this_elem, bool set_state, u_long32 bit)
{
   bool ret = false;
   u_long32 old_state = lGetUlong(this_elem, QU_state);
   u_long32 new_state = old_state;

   if (set_state) {   
      new_state |= bit;
   } else {
      new_state &= ~bit;
   }

   if (old_state != new_state) {
      lSetUlong(this_elem, QU_state, new_state);
      ret = true;
   }

   return ret;
}

/****** sgeobj/qinstance_state/qinstance_has_state() **************************
*  NAME
*     qinstance_has_state() -- checks a qi for a given states 
*
*  SYNOPSIS
*     bool qinstance_has_state(const lListElem *this_elem, u_long32 bit) 
*
*  FUNCTION
*     Takes a state mask and a queue instance and checks wheather the queue
*     is in at least one of the states. If the state mask contains U_LONG32_MAX
*     the function will always return true.
*
*  INPUTS
*     const lListElem *this_elem - queue instance 
*     u_long32 bit               - state mask 
*
*  RESULT
*     bool - true, if the queue instance has one of the requested states.
*
*  NOTES
*     MT-NOTE: qinstance_has_state() is MT safe 
*
*******************************************************************************/
bool qinstance_has_state(const lListElem *this_elem, u_long32 bit) {
   bool ret = true;

   if (bit != U_LONG32_MAX) {
      ret = (lGetUlong(this_elem, QU_state) & bit) ? true : false;
   }
   return ret;
}

/****** sgeobj/qinstance_state/transition_is_valid_for_qinstance() ************
*  NAME
*     transition_is_valid_for_qinstance() -- is transition valid 
*
*  SYNOPSIS
*     bool 
*     transition_is_valid_for_qinstance(u_long32 transition, 
*                                       lList **answer_list) 
*
*  FUNCTION
*     Checks if the given transition is valid for a qinstance object.
*     If the transition is valid, than true will be returned by this function. 
*
*  INPUTS
*     u_long32 transition - transition id 
*     lList **answer_list - AN_Type list 
*
*  RESULT
*     bool - test result
*        true  - transition is valid
*        false - transition is invalid
*
*  NOTES
*     MT-NOTE: transition_is_valid_for_qinstance() is MT safe 
*******************************************************************************/
bool
transition_is_valid_for_qinstance(u_long32 transition, lList **answer_list)
{
   bool ret = false;
  
   transition = transition & (~JOB_DO_ACTION);
   transition = transition & (~QUEUE_DO_ACTION);
   
   if (transition == QI_DO_NOTHING ||
       transition == QI_DO_DISABLE ||
       transition == QI_DO_ENABLE ||
       transition == QI_DO_SUSPEND ||
       transition == QI_DO_UNSUSPEND ||
       transition == QI_DO_CLEARERROR ||
       transition == QI_DO_RESCHEDULE
#ifdef __SGE_QINSTANCE_STATE_DEBUG__
       || transition == QI_DO_SETERROR ||
       transition == QI_DO_SETORPHANED ||
       transition == QI_DO_CLEARORPHANED ||
       transition == QI_DO_SETUNKNOWN ||
       transition == QI_DO_CLEARUNKNOWN ||
       transition == QI_DO_SETAMBIGUOUS ||
       transition == QI_DO_CLEARAMBIGUOUS 
#endif
      ) {
      ret = true;
   } else {
      answer_list_add(answer_list, MSG_QINSTANCE_INVALIDACTION, 
                      STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
   }

   return ret;
}

/* EB: What is the purpose of this function? */
bool
transition_option_is_valid_for_qinstance(u_long32 option, lList **answer_list)
{
   bool ret = false;

   DENTER(QINSTANCE_STATE_LAYER, "transition_option_is_valid_for_qinstance");
   if (option == QI_TRANSITION_NOTHING ||
       option == QI_TRANSITION_OPTION) {
      ret = true;
   } else {
      answer_list_add(answer_list, MSG_QINSTANCE_INVALIDOPTION, 
                     STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
   }
   DEXIT;
   return ret;
}

const char *
qinstance_state_as_string(u_long32 bit) 
{
   static const u_long32 states[] = { 
      QI_ALARM,
      QI_SUSPEND_ALARM,
      QI_DISABLED,
      QI_SUSPENDED,
      QI_UNKNOWN,
      QI_ERROR,
      QI_SUSPENDED_ON_SUBORDINATE,
      QI_CAL_DISABLED,
      QI_CAL_SUSPENDED,
      QI_AMBIGUOUS,
      QI_ORPHANED,

      (u_long32)~QI_ALARM,
      (u_long32)~QI_SUSPEND_ALARM,
      (u_long32)~QI_DISABLED,
      (u_long32)~QI_SUSPENDED,
      (u_long32)~QI_UNKNOWN,
      (u_long32)~QI_ERROR,
      (u_long32)~QI_SUSPENDED_ON_SUBORDINATE,
      (u_long32)~QI_CAL_DISABLED,
      (u_long32)~QI_CAL_SUSPENDED,
      (u_long32)~QI_AMBIGUOUS,
      (u_long32)~QI_ORPHANED,

      /*
       * Don't forget to change the names-array, too
       * if something is changed here
       */

      0 
   };
   static const char *names[23] = { NULL }; 
   const char *ret = NULL;
   int i = 0;

   DENTER(TOP_LAYER, "qinstance_state_as_string");
   if (names[0] == NULL) {
      names[0] = MSG_QINSTANCE_ALARM;
      names[1] = MSG_QINSTANCE_SUSPALARM;
      names[2] = MSG_QINSTANCE_DISABLED;
      names[3] = MSG_QINSTANCE_SUSPENDED;
      names[4] = MSG_QINSTANCE_UNKNOWN;
      names[5] = MSG_QINSTANCE_ERROR;
      names[6] = MSG_QINSTANCE_SUSPOSUB;
      names[7] = MSG_QINSTANCE_CALDIS;
      names[8] = MSG_QINSTANCE_CALSUSP;
      names[9] = MSG_QINSTANCE_CONFAMB;
      names[10] = MSG_QINSTANCE_ORPHANED;
      names[11] = MSG_QINSTANCE_NALARM;
      names[12] = MSG_QINSTANCE_NSUSPALARM;
      names[13] = MSG_QINSTANCE_NDISABLED;
      names[14] = MSG_QINSTANCE_NSUSPENDED;
      names[15] = MSG_QINSTANCE_NUNKNOWN;
      names[16] = MSG_QINSTANCE_NERROR;
      names[17] = MSG_QINSTANCE_NSUSPOSUB;
      names[18] = MSG_QINSTANCE_NCALDIS;
      names[19] = MSG_QINSTANCE_NCALSUSP;
      names[20] = MSG_QINSTANCE_NCONFAMB;
      names[21] = MSG_QINSTANCE_NORPHANED;
      names[22] = NULL;
   }

   while (states[i] != 0) {
      if (states[i] == bit) {
         ret = names[i];
         break;
      }
      i++;
   }
   DEXIT;
   return ret;
}

/****** sgeobj/qinstance_state/qinstance_state_from_string() ******************
*  NAME
*     qinstance_state_from_string() -- takes a state string and returns an int 
*
*  SYNOPSIS
*     u_long32 qinstance_state_from_string(const char* sstate) 
*
*  FUNCTION
*     Takes a string with character representations of the different states and
*     generates a mask with the different states.
*
*  INPUTS
*     const char* sstate - each character one state
*     lList **answer_list - stores error messages
*     u_long32 filter  - a bit filter for allowed states
*
*  RESULT
*     u_long32 - new state or 0, if no state was set
*
*  NOTES
*     MT-NOTE: qinstance_state_from_string() is MT safe 
*******************************************************************************/
u_long32 
qinstance_state_from_string(const char* sstate, 
                            lList **answer_list, 
                            u_long32 filter){
   u_long32 ustate = 0;
   int i;
   int y;
   bool found = false;
   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_from_string");

   i=-1;
   while(sstate[++i]!='\0'){
      y=-1;
      found = false;
      while(letters[++y]!='\0'){
         if (letters[y] == sstate[i]) {
            found = true;
            ustate |=  states[y];
            break;
         }
      }

      if ((!found) || ((ustate & ~filter) != 0)){
         ERROR((SGE_EVENT, MSG_QSTATE_UNKNOWNCHAR_CS, sstate[i], sstate));
         answer_list_add(answer_list, SGE_EVENT, STATUS_ENOMGR, ANSWER_QUALITY_ERROR);
         DEXIT;
         return U_LONG32_MAX;
      }
   }

   if (!found) {
      ustate = U_LONG32_MAX;
   }

   DEXIT;
   return ustate;
}


bool 
qinstance_state_append_to_dstring(const lListElem *this_elem, dstring *string)
{
   bool ret = true;
   int i = 0;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_append_to_dstring");
   while (states[i] != 0) {
      if (qinstance_has_state(this_elem, states[i])) {
         sge_dstring_append_char(string, letters[i]);
      }
      i++;
   }
   sge_dstring_sprintf_append(string, "%c", '\0');

   DRETURN(ret);
}

bool
qinstance_state_set_orphaned(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_orphaned");
   changed = qinstance_set_state(this_elem, set_state, QI_ORPHANED);
   DRETURN(changed);
}

bool 
qinstance_state_is_orphaned(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_ORPHANED);
}

bool
qinstance_state_set_ambiguous(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_ambiguous");
   changed = qinstance_set_state(this_elem, set_state, QI_AMBIGUOUS);
   DRETURN(changed);
}

bool 
qinstance_state_is_ambiguous(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_AMBIGUOUS);
}

bool
qinstance_state_set_alarm(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_alarm");
   changed = qinstance_set_state(this_elem, set_state, QI_ALARM);
   DRETURN(changed);
}

bool 
qinstance_state_is_alarm(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_ALARM);
}

bool
qinstance_state_set_suspend_alarm(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_suspend_alarm");
   changed = qinstance_set_state(this_elem, set_state, QI_SUSPEND_ALARM);
   DRETURN(changed);
}

bool 
qinstance_state_is_suspend_alarm(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_SUSPEND_ALARM);
}

bool
qinstance_state_set_manual_disabled(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_manual_disabled");
   changed = qinstance_set_state(this_elem, set_state, QI_DISABLED);
   DRETURN(changed);
}

bool 
qinstance_state_is_manual_disabled(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_DISABLED);
}

bool
qinstance_state_set_manual_suspended(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_manual_suspended");
   changed = qinstance_set_state(this_elem, set_state, QI_SUSPENDED);
   DRETURN(changed);
}

bool 
qinstance_state_is_manual_suspended(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_SUSPENDED);
}

bool
qinstance_state_set_unknown(lListElem *this_elem, bool set_state)
{
   bool changed;
   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_unknown");
   if (mconf_get_simulate_execds())
      changed = qinstance_set_state(this_elem, false, QI_UNKNOWN);
   else
      changed = qinstance_set_state(this_elem, set_state, QI_UNKNOWN);
   DRETURN(changed);
}


bool 
qinstance_state_is_unknown(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_UNKNOWN);
}

bool
qinstance_state_set_error(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_error");
   changed = qinstance_set_state(this_elem, set_state, QI_ERROR);
   DRETURN(changed);
}

bool 
qinstance_state_is_error(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_ERROR);
}

bool
qinstance_state_set_susp_on_sub(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_susp_on_sub");
   changed = qinstance_set_state(this_elem, set_state, QI_SUSPENDED_ON_SUBORDINATE);
   DRETURN(changed);
}

bool 
qinstance_state_is_susp_on_sub(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_SUSPENDED_ON_SUBORDINATE);
}

bool
qinstance_state_set_cal_disabled(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_cal_disabled");
   changed = qinstance_set_state(this_elem, set_state, QI_CAL_DISABLED);
   DRETURN(changed);
}

bool 
qinstance_state_is_cal_disabled(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_CAL_DISABLED);
}

bool
qinstance_state_set_cal_suspended(lListElem *this_elem, bool set_state)
{
   bool changed;

   DENTER(QINSTANCE_STATE_LAYER, "qinstance_state_set_cal_suspended");
   changed = qinstance_set_state(this_elem, set_state, QI_CAL_SUSPENDED);
   DRETURN(changed);
}

bool 
qinstance_state_is_cal_suspended(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_CAL_SUSPENDED);
}

bool
qinstance_state_set_full(lListElem *this_elem, bool set_state)
{
   return qinstance_set_state(this_elem, set_state, QI_FULL);
}

bool 
qinstance_state_is_full(const lListElem *this_elem)
{
   return qinstance_has_state(this_elem, QI_FULL);
}
