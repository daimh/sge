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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_prog.h"
#include "uti/sge_profiling.h"
#include "uti/sge_time.h"
#include "uti/sge_mtutil.h"

#include "cull/cull.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_all_listsL.h"

#include "sge_qmaster_timed_event.h"
#include "setup_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"

#define EVENT_LAYER CULL_LAYER

#define EVENT_FRMT(x) SGE_FUNC, x->type, (u_long32) x->when, (u_long32) x->mode, x->str_key?x->str_key:MSG_SMALLNULL

event_control_t Event_Control = {
   PTHREAD_MUTEX_INITIALIZER, 
   PTHREAD_COND_INITIALIZER, 
   false, 
   false, 
   NULL, 
   NULL, 
   0, 
   0, 
   0
};

handler_tbl_t Handler_Tbl = {
   PTHREAD_MUTEX_INITIALIZER, 
   0, 
   0, 
   NULL
};

/****** qmaster/sge_qmaster_timed_event/te_delete_all_or_one_time_event() *************
*  NAME
*     te_delete_all_or_one_time_event() -- Delete one time events 
*
*  SYNOPSIS
*     int 
*     te_delete_all_or_one_time_event(te_type_t aType, u_long32 aKey1, 
*                                     u_long32 aKey2, const char* aStrKey,
*                                     bool ignore_keys) 
*
*  FUNCTION
*     Delete one or more one-time events. All one-time events which do EXACTLY
*     match the given arguments will be deleted from the event list if 
*     ignore_keys is false. Otherwise all one-time events of the given type
*     will be removed.
*
*     If a timed event is scheduled for delivery, it will NOT be deleted,
*     even if it does match the arguments. Such an event will be deleted after
*     event delivery has been finished.
*
*  INPUTS
*     te_type_t aType     - event type 
*     u_long32 aKey1      - first numeric key 
*     u_long32 aKey2      - second numeric key 
*     const char* aStrKey - alphanumeric key 
*     bool ignore_kezs    - boolean flag
*
*  RESULT
*     int - number of events deleted
*
*  NOTES
*     MT-NOTE: te_delete_all_or_one_time_event() is MT safe. 
*     MT-NOTE:
*     MT-NOTE: If a timed event has been deleted we need to signal the event
*     MT-NOTE: delivery thread. This is because the event delivery thread
*     MT-NOTE: maybe waiting until the just deleted event becomes due. Event
*     MT-NOTE: deletion is communicated by setting 'Event_Control.delete'
*     MT-NOTE: to 'true'.
*******************************************************************************/
static int 
te_delete_all_or_one_time_event(te_type_t aType, u_long32 aKey1, u_long32 aKey2, const char* strKey, bool ignore_keys)     
{
   int res, n = 0;
   lCondition *cond = NULL;

   DENTER(EVENT_LAYER, "te_delete_event");


   DPRINTF(("%s: (t:%d u1:"sge_u32" u2:"sge_u32" s:%s)\n", SGE_FUNC, aType, aKey1, aKey2, strKey?strKey:MSG_SMALLNULL));

   if (ignore_keys) {
      cond = lWhere("%T(%I != %u || %I != %u)", TE_Type, TE_type, aType, TE_mode, ONE_TIME_EVENT);
   } else {
      if (strKey != NULL) {
         cond = lWhere("%T(%I != %u || %I != %u || %I != %u || %I != %u || %I != %s)", TE_Type,
            TE_type, aType, TE_mode, ONE_TIME_EVENT, TE_uval0, aKey1, TE_uval1, aKey2, TE_sval, strKey);
      } else {
         cond = lWhere("%T(%I != %u || %I != %u || %I != %u || %I != %u)", TE_Type,
            TE_type, aType, TE_mode, ONE_TIME_EVENT, TE_uval0, aKey1, TE_uval1, aKey2);
      
      }
   }

   sge_mutex_lock("event_control_mutex", SGE_FUNC, __LINE__, &Event_Control.mutex);

   n = lGetNumberOfElem(Event_Control.list);

   if (0 == n)
   {
      DPRINTF(("%s: event list empty!\n", SGE_FUNC));

      sge_mutex_unlock("event_control_mutex", SGE_FUNC, __LINE__, &Event_Control.mutex);
   
      lFreeWhere(&cond);
   
      DEXIT;
      return 0;  
   }

   lSplit(&Event_Control.list, NULL, NULL, cond);

   if (NULL == Event_Control.list)
   {
      DPRINTF(("%s: event list has been freed --> recreate \n", SGE_FUNC));

      Event_Control.list = lCreateList("timed event list", TE_Type);
      res = n; /* all elements have been deleted */
   } else {
      res = n - lGetNumberOfElem(Event_Control.list);
   }

   if( res > 0)
   {
      Event_Control.delete = true;

      pthread_cond_signal(&Event_Control.cond_var);

      DPRINTF(("%s: did delete %d event!\n", SGE_FUNC, res));
   }

   sge_mutex_unlock("event_control_mutex", SGE_FUNC, __LINE__, &Event_Control.mutex);

   lFreeWhere(&cond);

   DEXIT;
   return res;
} 

/****** sge_qmaster_timed_event/te_wait_empty() ***********************
*  NAME
*     te_wait_empty() -- waits, if the event list is empty
*
*  SYNOPSIS
*     static void te_wait_empty(void) 
*
*  FUNCTION
*     waits, if the event list is empty
*
*  NOTES
*     MT-NOTE: te_wait_empty() is not MT safe 
*
*******************************************************************************/
void te_wait_empty(void) 
{

   DENTER(EVENT_LAYER, "te_wait_empty");
   
   while (lGetNumberOfElem((const lList*)Event_Control.list) == 0) {
         DPRINTF(("%s: event list empty --> will wait\n", SGE_FUNC));
         Event_Control.next = 0;
         pthread_cond_wait(&Event_Control.cond_var, &Event_Control.mutex);
   }

   DEXIT;
}

/****** sge_qmaster_timed_event/te_wait_next() ************************
*  NAME
*     te_wait_next() -- waits for the next event
*
*  SYNOPSIS
*     static void te_wait_next(te_event_t te, time_t now) 
*
*  FUNCTION
*    waits for the next event
*
*  INPUTS
*     te_event_t te - next event
*     time_t now    - current time
*
*  NOTES
*     MT-NOTE: te_wait_next() is not MT safe 
*
*******************************************************************************/
void te_wait_next(te_event_t te, time_t now) 
{
   struct timespec ts;
   DENTER(EVENT_LAYER, "te_wait_next");

   ts.tv_sec = te->when;
   ts.tv_nsec = 0;
   
   while(Event_Control.next == te->when)
   {
      int res = 0;

      DPRINTF(("%s: time:%ld next:%ld --> will wait\n",
               SGE_FUNC, (unsigned long) now, (unsigned long) Event_Control.next));

      res = pthread_cond_timedwait(&Event_Control.cond_var, &Event_Control.mutex, &ts);
      if (ETIMEDOUT == res) { break; }
   }

DEXIT;
}

/****** qmaster/sge_qmaster_timed_event/te_register_event_handler() ************
*  NAME
*     te_register_event_handler() -- Register event handler 
*
*  SYNOPSIS
*     void te_register_event_handler(te_handler_t aHandler, te_type_t aType) 
*
*  FUNCTION
*     Register an event handler. The registered handler will be invoked whenever
*     an event of type 'aType' is due.
*
*     If more than one event handler for a given event type will be registered,
*     only the FIRST event handler registered will be invoked. It is possible,
*     however, to register the same event handler for multiple event types.
*
*  INPUTS
*     te_handler_t aHandler - event handler 
*     te_type_t aType       - event type
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: te_register_event_handler() is MT safe. 
*
*******************************************************************************/
void te_register_event_handler(te_handler_t aHandler, te_type_t aType)
{
   DENTER(EVENT_LAYER, "te_add_event_handler");

   SGE_ASSERT(aHandler != NULL);

   sge_mutex_lock("handler_table_mutex", SGE_FUNC, __LINE__, &Handler_Tbl.mutex);

   if (Handler_Tbl.num >= Handler_Tbl.max) /* grow table */
   {
      int sz = (TBL_GROW_FACTOR * Handler_Tbl.max) * sizeof(struct tbl_elem);

      Handler_Tbl.list = (struct tbl_elem *)sge_realloc((char*)Handler_Tbl.list, sz, 1);
      Handler_Tbl.max *= TBL_GROW_FACTOR;

      DPRINTF(("%s: grow handler table to %d elements\n", SGE_FUNC, Handler_Tbl.max));
   }

   Handler_Tbl.list[Handler_Tbl.num].type    = aType;
   Handler_Tbl.list[Handler_Tbl.num].handler = aHandler;
   Handler_Tbl.num++;

   DPRINTF(("%s: handler #%d for event type %d\n", SGE_FUNC, (Handler_Tbl.num - 1), aType));

   sge_mutex_unlock("handler_table_mutex", SGE_FUNC, __LINE__, &Handler_Tbl.mutex);

   DEXIT;
   return;
} /* te_register_event_handler() */

/****** qmaster/sge_qmaster_timed_event/te_new_event() *************************
*  NAME
*     te_new_event() -- Allocate new timed event. 
*
*  SYNOPSIS
*     te_event_t te_new_event(time_t aTime, te_type_t aType, te_mode_t aMode, 
*     u_long32 aKey1, u_long32 aKey2, const char* aStrKey) 
*
*  FUNCTION
*     Allocate and initialize a new timed event. The new event will be 
*     initialized using the arguments given.
*
*     The caller of this function is responsible to free the timed event
*     returned, using 'te_free_event()'.
*
*     If event type is 'ONE_TIME_EVENT', 'aTime' does determine the ABSOLUTE
*     timed event due time in seconds since the Epoch. If event type is
*     'RECURRING_EVENT', 'aTime' does determine the timed event INTERVAL in
*     seconds.
*
*     If 'aStrKey' is not 'NULL', the new timed event will contain a copy.
*
*  INPUTS
*     time_t aTime        - event due time or interval 
*     te_type_t aType     - event type 
*     te_mode_t aMode     - event mode 
*     u_long32 aKey1      - first numeric key, '0' if not used 
*     u_long32 aKey2      - second numeric key, '0' if not used 
*     const char* aStrKey - alphanumeric key, 'NULL' if not used 
*
*  RESULT
*     te_event_t - new timed event
*
*  NOTES
*     MT-NOTE: te_new_event() is MT safe. 
*
*******************************************************************************/
te_event_t te_new_event(time_t aTime, te_type_t aType, te_mode_t aMode, u_long32 aKey1, u_long32 aKey2, const char* aStrKey)
{
   te_event_t ev = NULL;
  
   DENTER(EVENT_LAYER, "te_new_event");
   
   ev = (te_event_t)sge_malloc(sizeof(struct te_event));
   
   if (ONE_TIME_EVENT == aMode) {
      ev->when = aTime;
      ev->interval = 0;
   } else {
      ev->when = 0;
      ev->interval = aTime;
   }

   ev->type = aType;
   ev->mode = aMode;
   ev->ulong_key_1 = aKey1;
   ev->ulong_key_2 = aKey2;
   ev->str_key = (aStrKey != NULL) ? strdup(aStrKey) : NULL;
   ev->seq_no = 0;

   DEXIT;
   return ev;
} /* te_new_event() */

/****** qmaster/sge_qmaster_timed_event/te_free_event() ************************
*  NAME
*     te_free_event() -- Free timed event 
*
*  SYNOPSIS
*     void te_free_event(te_event_t anEvent) 
*
*  FUNCTION
*     Free timed event 'anEvent'. Upon return, 'anEvent' will be 'NULL'.
*
*  INPUTS
*     te_event_t anEvent - timed event, must NOT be 'NULL'. 
*
*  RESULT
*     void - none, 'anEvent' will be 'NULL'.
*
*  NOTES
*     MT-NOTE: te_free_event() is MT safe. 
*
*******************************************************************************/
void te_free_event(te_event_t *anEvent)
{

   DENTER(EVENT_LAYER, "te_free_event");

   SGE_ASSERT((anEvent != NULL));
   
   sge_free(&((*anEvent)->str_key));
   sge_free(anEvent);

   DRETURN_VOID;
} /* te_free_event() */

/****** qmaster/sge_qmaster_timed_event/te_add_event() *************************
*  NAME
*     te_add_event() -- Add timed event 
*
*  SYNOPSIS
*     void te_add_event(te_event_t anEvent) 
*
*  FUNCTION
*     Add timed event. An event handler which does match the event type of
*     'anEvent' must have been registered previously. Otherwise the timed
*     event 'anEvent' will not be delivered.
*
*     After event delivery an event with event mode 'ONE_TIME_EVENT' will be
*     removed. An event with event mode 'RECURRING_EVENT' will be delivered
*     repeatedly until being removed explicitly, using 'te_delete_event()'.
*
*     The event 'anEvent' could be freed safely, using 'te_free_event()' after
*     this function did return. 
*
*  INPUTS
*     te_event_t anEvent - timed event 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: te_add_event() is MT safe. 
*     MT-NOTE:
*     MT-NOTE: If the event list is empty, the event delivery thread will wait
*     MT-NOTE: for work. Hence, the event delivery thread is signaled if the
*     MT-NOTE: very first event is added.
*     MT-NOTE:
*     MT-NOTE: If no event is due, i.e. the due date of the next event does lie
*     MT-NOTE: ahead, the event delivery thread does wait until the next event
*     MT-NOTE: does become due. Hence, the event delivery thread is signaled if
*     MT-NOTE: an event is added which is due earlier. In addition the due
*     MT-NOTE: date of the next event is set to the due date of the event just
*     MT-NOTE: added
*
*******************************************************************************/
void te_add_event(te_event_t anEvent)
{
   time_t when = 0;
   lListElem *le;

   DENTER(EVENT_LAYER, "te_add_event");

   SGE_ASSERT((anEvent != NULL));

   when = (ONE_TIME_EVENT == anEvent->mode) ? anEvent->when : (time(NULL) + anEvent->interval);

   le = lCreateElem(TE_Type);
   lSetUlong(le,  TE_when,     when);
   lSetUlong(le,  TE_type,     anEvent->type);
   lSetUlong(le,  TE_mode,     anEvent->mode);
   lSetUlong(le,  TE_interval, anEvent->interval);
   lSetUlong(le,  TE_uval0,    anEvent->ulong_key_1);
   lSetUlong(le,  TE_uval1,    anEvent->ulong_key_2);
   lSetString(le, TE_sval,     anEvent->str_key);

   DPRINTF(("%s: (t:%d w:%ld m:%d s:%s)\n", SGE_FUNC,
            anEvent->type, (unsigned long) when, anEvent->mode,
            anEvent->str_key?anEvent->str_key:MSG_SMALLNULL));

   sge_mutex_lock("event_control_mutex", SGE_FUNC, __LINE__, &Event_Control.mutex);

   lSetUlong(le, TE_seqno, Event_Control.seq_no++);

   if (lInsertSorted(Event_Control.sort_order, le, Event_Control.list) != 0) {
      lFreeElem(&le);
   }

   if ((Event_Control.next == 0) || (when < Event_Control.next)) {
      Event_Control.next = when;

      pthread_cond_signal(&Event_Control.cond_var);

      DPRINTF(("%s: did signal delivery thread!\n", SGE_FUNC));
   }

   sge_mutex_unlock("event_control_mutex", SGE_FUNC, __LINE__, &Event_Control.mutex);

   DRETURN_VOID;
} /* te_add_event() */

/****** qmaster/sge_qmaster_timed_event/te_delete_one_time_event() *************
*  NAME
*     te_delete_one_time_event() -- Delete one time events 
*
*  SYNOPSIS
*     int te_delete_one_time_event(te_type_t aType, u_long32 aKey1, u_long32 
*     aKey2, const char* aStrKey) 
*
*  FUNCTION
*     Delete one or more one-time events. All one-time events which do EXACTLY
*     match the given arguments will be deleted from the event list.
*
*     If a timed event is scheduled for delivery, it will will NOT be deleted,
*     even if it does match the arguments. Such an event will be deleted after
*     event delivery has been finished.
*
*  INPUTS
*     te_type_t aType     - event type 
*     u_long32 aKey1      - first numeric key 
*     u_long32 aKey2      - second numeric key 
*     const char* aStrKey - alphanumeric key 
*
*  RESULT
*     int - number of events deleted
*
*  NOTES
*     MT-NOTE: te_delete_one_time_event() is MT safe. 
*     MT-NOTE:
*     MT-NOTE: If a timed event has been deleted we need to signal the event
*     MT-NOTE: delivery thread. This is because the event delivery thread
*     MT-NOTE: maybe waiting until the just deleted event becomes due. Event
*     MT-NOTE: deletion is communicated by setting 'Event_Control.delete'
*     MT-NOTE: to 'true'.
*******************************************************************************/
int te_delete_one_time_event(te_type_t aType, u_long32 aKey1, u_long32 aKey2, const char* strKey) 
{
   int ret;

   DENTER(EVENT_LAYER, "te_delete_one_time_event");
   ret = te_delete_all_or_one_time_event(aType, aKey1, aKey2, strKey, false);
   DRETURN(ret); 
}

/****** qmaster/sge_qmaster_timed_event/te_delete_all_one_time_event() *************
*  NAME
*     te_delete_all_one_time_event() -- Delete one time events 
*
*  SYNOPSIS
*     int te_delete_all_one_time_event(te_type_t aType);
*
*  FUNCTION
*     Delete all one-time events of the given type. 
*
*     If a timed event is scheduled for delivery, it will will NOT be deleted,
*     even if it does match the arguments. Such an event will be deleted after
*     event delivery has been finished.
*
*  INPUTS
*     te_type_t aType     - event type 
*
*  RESULT
*     int - number of events deleted
*
*  NOTES
*     MT-NOTE: te_delete_all_one_time_event() is MT safe. 
*     MT-NOTE:
*     MT-NOTE: If a timed event has been deleted we need to signal the event
*     MT-NOTE: delivery thread. This is because the event delivery thread
*     MT-NOTE: maybe waiting until the just deleted event becomes due. Event
*     MT-NOTE: deletion is communicated by setting 'Event_Control.delete'
*     MT-NOTE: to 'true'.
*******************************************************************************/
int te_delete_all_one_time_events(te_type_t aType) {
   int ret;

   DENTER(EVENT_LAYER, "te_delete_all_one_time_events");
   ret = te_delete_all_or_one_time_event(aType, 0, 0, NULL, true);
   DRETURN(ret);
}


/****** qmaster/sge_qmaster_timed_event/te_get_when() **************************
*  NAME
*     te_get_when() -- Return timed event due date 
*
*  SYNOPSIS
*     time_t te_get_when(te_event_t anEvent) 
*
*  FUNCTION
*     Return timed event due date 
*
*  INPUTS
*     te_event_t anEvent - timed event 
*
*  RESULT
*     time_t - due date
*
*  NOTES
*     MT-NOTE: 'te_get_when()' is MT safe. 
*
*******************************************************************************/
time_t te_get_when(te_event_t anEvent)
{
   time_t res = 0;

   DENTER(EVENT_LAYER, "te_get_when");

   SGE_ASSERT(NULL != anEvent);

   res = (time_t)anEvent->when;

   DEXIT;
   return res;
} /* te_get_when() */

/****** qmaster/sge_qmaster_timed_event/te_get_type() **************************
*  NAME
*     te_get_type() -- Return timed event type. 
*
*  SYNOPSIS
*     te_type_t te_get_type(te_event_t anEvent) 
*
*  FUNCTION
*     Return timed event type. 
*
*  INPUTS
*     te_event_t anEvent - timed event 
*
*  RESULT
*     te_type_t - timed event type
*
*  NOTES
*     MT-NOTE: 'te_get_type()' is MT safe. 
*
*******************************************************************************/
te_type_t te_get_type(te_event_t anEvent)
{
   te_type_t res;

   DENTER(EVENT_LAYER, "te_get_type");

   SGE_ASSERT(NULL != anEvent);

   res = anEvent->type;

   DEXIT;
   return res;
} /* te_get_type() */

/****** qmaster/sge_qmaster_timed_event/te_get_mode() **************************
*  NAME
*     te_get_mode() -- Return timed event mode 
*
*  SYNOPSIS
*     te_mode_t te_get_mode(te_event_t anEvent) 
*
*  FUNCTION
*     Return timed event mode. 
*
*  INPUTS
*     te_event_t - timed event 
*
*  RESULT
*     te_mode_t - timed event mode
*
*  NOTES
*     MT-NOTE: 'te_get_mode()' is MT safe. 
*
*******************************************************************************/
te_mode_t te_get_mode(te_event_t anEvent)
{
   te_mode_t res;

   DENTER(EVENT_LAYER, "te_get_mode");

   SGE_ASSERT(NULL != anEvent);

   res = anEvent->mode;

   DEXIT;
   return res;
} /* te_get_mode() */

/****** qmaster/sge_qmaster_timed_event/te_get_first_numeric_key() *************
*  NAME
*     te_get_first_numeric_key() -- Return timed event first numeric key. 
*
*  SYNOPSIS
*     u_long32 te_get_first_numeric_key(te_event_t anEvent) 
*
*  FUNCTION
*     Return timed event first numeric key. 
*
*  INPUTS
*     te_event_t - timed event 
*
*  RESULT
*     u_long32 - first numeric key
*
*  NOTES
*     MT-NOTE: 'te_get_first_numeric_key()' is MT safe. 
*
*******************************************************************************/
u_long32 te_get_first_numeric_key(te_event_t anEvent)
{
   u_long32 res = 0;

   DENTER(EVENT_LAYER, "te_get_first_numeric_key");

   SGE_ASSERT(NULL != anEvent);
   
   res = anEvent->ulong_key_1;

   DEXIT;
   return res;
} /* te_get_first_numeric_key() */

/****** qmaster/sge_qmaster_timed_event/te_get_second_numeric_key() ************
*  NAME
*     te_get_second_numeric_key() -- Return timed event second numeric key. 
*
*  SYNOPSIS
*     u_long32 te_get_second_numeric_key(te_event_t anEvent) 
*
*  FUNCTION
*     Return timed event second numeric key. 
*
*  INPUTS
*     te_event_t anEvent - timed event 
*
*  RESULT
*     u_long32 - second numeric key
*
*  NOTES
*     MT-NOTE: 'te_get_second_numeric_key()' is MT safe. 
*
*******************************************************************************/
u_long32 te_get_second_numeric_key(te_event_t anEvent)
{
   u_long32 res = 0;

   DENTER(EVENT_LAYER, "te_get_second_numeric_key");

   SGE_ASSERT(NULL != anEvent);
   
   res = anEvent->ulong_key_2;

   DEXIT;
   return res;
} /* te_get_second_numeric_key() */

/****** qmaster/sge_qmaster_timed_event/te_get_alphanumeric_key() **************
*  NAME
*     te_get_alphanumeric_key() -- Return timed event alphanumeric key. 
*
*  SYNOPSIS
*     char* te_get_alphanumeric_key(te_event_t anEvent) 
*
*  FUNCTION
*     Return timed event alphanumeric key. 
*
*     The caller of this function MUST free the string returned.
*
*  INPUTS
*     te_event_t anEvent - timed event 
*
*  RESULT
*     char* - alphanumeric key or MSG_SMALLNULL if no key is set.
*
*  NOTES
*     MT-NOTE: 'te_get_alphanumeric_key()' is MT safe. 
*
*******************************************************************************/
char* te_get_alphanumeric_key(te_event_t anEvent)
{
   char* res = NULL;

   DENTER(EVENT_LAYER, "te_get_alphanumeric_key");

   SGE_ASSERT(NULL != anEvent);

   res = (anEvent->str_key != NULL) ? strdup(anEvent->str_key) : NULL;

   DRETURN(res);
}

/****** qmaster/sge_qmaster_timed_event/te_get_sequence_number() ***************
*  NAME
*     te_get_sequence_number() -- Return timed event sequence number. 
*
*  SYNOPSIS
*     u_long32 te_get_sequence_number(te_event_t anEvent) 
*
*  FUNCTION
*     Return timed event sequence number. 
*
*  INPUTS
*     te_event_t anEvent - timed event 
*
*  RESULT
*     u_long32 - event sequence number
*
*  NOTES
*     MT-NOTE: 'te_get_sequence_number()' is MT safe. 
*
*******************************************************************************/
u_long32 te_get_sequence_number(te_event_t anEvent)
{
   u_long32 res = 0;

   DENTER(EVENT_LAYER, "te_get_sequence_number");

   SGE_ASSERT(NULL != anEvent);

   res = anEvent->seq_no;

   DEXIT;
   return res;
} /* te_get_sequence_number() */

/****** qmaster/sge_qmaster_timed_event/te_init() ****************
*  NAME
*     te_init() -- one-time initialization 
*
*  SYNOPSIS
*     static void te_init(void) 
*
*  FUNCTION
*     Create timed event list. Set list sort order to be ascending event due 
*     time. Create event handler table of initial size. 
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: te_init() is not MT safe
*     MT-NOTE:
*     MT-NOTE: This function must only be used as a one-time initialization
*     MT-NOTE: function.
*
*******************************************************************************/
void te_init(void)
{
   DENTER(EVENT_LAYER, "te_init");

   Event_Control.list = lCreateList("timed event list", TE_Type);
   Event_Control.sort_order = lParseSortOrderVarArg(TE_Type, "%I+", TE_when);

   Handler_Tbl.list = (struct tbl_elem *)sge_malloc(TBL_INIT_SIZE * sizeof(struct tbl_elem));
   Handler_Tbl.max = TBL_INIT_SIZE;
   Handler_Tbl.num = 0;

   DEXIT;
   return;
} /* te_init() */

/****** qmaster/sge_qmaster_timed_event/te_shutdown() **************************
*  NAME
*     te_shutdown() -- Shutdown event delivery thread. 
*
*  SYNOPSIS
*     void te_shutdown(void) 
*
*  FUNCTION
*     Shutdown event delivery thread. Set event control structure 'exit' flag.
*     Wait until event delivery thread did terminate.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: 'te_shutdown()' is MT safe. 
*     MT-NOTE:
*     MT-NOTE: 'pthread_once()' is called for symmetry reasons. This module
*     MT-NOTE: will be initialized on demand, i.e. each function may be
*     MT-NOTE: invoked without any prerequisite.
*
*******************************************************************************/
void te_shutdown(void)
{
   DENTER(EVENT_LAYER, "te_shutdown");

   sge_free(&(Handler_Tbl.list));

   DEXIT;
   return;
} /* te_shutdown() */

/****** qmaster/sge_qmaster_timed_event/te_check_time() ***************************
*  NAME
*     te_check_time() -- check time
*
*  SYNOPSIS
*     void te_check_time(time_t aTime) 
*
*  FUNCTION
*     Check if 'aTime' is a point in time BEFORE the last timed event has been
*     delivered. If so, adjust all pending timed events and set the time of 
*     last event delivery to 'aTime'. In addition adjust due date of the next
*     event.
*
*  INPUTS
*     time_t aTime - time value to check 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: te_check_time() is NOT MT safe!
*     MT-NOTE:
*     MT-NOTE: It may only be called with 'Event_Control.mutex' locked!
*
*******************************************************************************/
void te_check_time(time_t aTime)
{
   lListElem* le;

   DENTER(EVENT_LAYER, "te_check_time");

   if (Event_Control.last > aTime)
   {
      time_t delta = Event_Control.last - aTime;

      WARNING((SGE_EVENT, MSG_SYSTEM_SYSTEMHASBEENMODIFIEDXSECONDS_I, (int)delta));

      for_each (le, Event_Control.list) {
         lSetUlong(le, TE_when, (lGetUlong(le, TE_when) - delta));
      }

      Event_Control.last = aTime;
      Event_Control.next -= delta;
   }

   DEXIT;
   return;
} /* te_check_time() */

/****** qmaster/sge_qmaster_timed_event/te_event_from_list_elem() *****************
*  NAME
*     te_event_from_list_elem() -- Allocate new timed event.
*
*  SYNOPSIS
*     te_event_t te_event_from_list_elem(lListElem* aListElem) 
*
*  FUNCTION
*     Allocate and initialize a new timed event. The new event will be
*     initialized using the given list element. 
*
*     The caller of this function is responsible to free the timed event
*     returned, using 'te_free_event()'.
*
*  INPUTS
*     lListElem* aListElem - list element 
*
*  RESULT
*     te_event_t - new timed event 
*
*  NOTES
*     MT-NOTE: te_event_from_list_elem() is MT safe. 
*
*******************************************************************************/
te_event_t te_event_from_list_elem(lListElem* aListElem)
{
   te_event_t ev = NULL;
   const char* str = NULL;

   DENTER(EVENT_LAYER, "te_event_from_list_elem");

   ev = (te_event_t)sge_malloc(sizeof(struct te_event));
   
   ev->when        = (time_t)lGetUlong(aListElem, TE_when);
   ev->interval    = (time_t)lGetUlong(aListElem, TE_interval);
   ev->type        = (te_type_t)lGetUlong(aListElem, TE_type);
   ev->mode        = (te_mode_t)lGetUlong(aListElem, TE_mode);
   ev->ulong_key_1 = lGetUlong(aListElem, TE_uval0);
   ev->ulong_key_2 = lGetUlong(aListElem, TE_uval1);
   ev->seq_no      = lGetUlong(aListElem, TE_seqno);

   str = lGetString(aListElem, TE_sval);
   ev->str_key = ((str != NULL) ? strdup(str) : NULL);

   DEXIT;
   return ev;
} /* te_event_from_list_elem() */

/****** qmaster/sge_qmaster_timed_event/te_scan_table_and_deliver() ***************
*  NAME
*     te_scan_table_and_deliver() -- Scan event handler table and deliver event. 
*
*  SYNOPSIS
*     static void te_scan_table_and_deliver(te_event_t anEvent) 
*
*  FUNCTION
*     Scan event handler table for an event handler which does match the event
*     type of 'anEvent'. If 'anEvent' is of mode 'RECURRING_EVENT' it will be
*     added to the event list again after delivery, with its due date adjusted.
*
*  INPUTS
*     te_event_t anEvent - event to deliver 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: te_scan_table_and_deliver() is MT safe. 
*     MT-NOTE:
*     MT-NOTE: Do NOT invoke this function with 'Event_Control.mutex' locked!
*     MT-NOTE: Otherwise a deadlock may occur due to recursive mutex locking.
*
*******************************************************************************/
void te_scan_table_and_deliver(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   int i = 0;
   te_handler_t handler = NULL;

   DENTER(EVENT_LAYER, "te_scan_table_and_deliver");

   DPRINTF(("%s: event (t:%d w:"sge_u32" m:"sge_u32" s:%s)\n",
            EVENT_FRMT(anEvent)));

   sge_mutex_lock("handler_table_mutex", SGE_FUNC, __LINE__, &Handler_Tbl.mutex);

   for(i = 0; i < Handler_Tbl.num; i++)
   {
      te_type_t type = Handler_Tbl.list[i].type;

      if (type == anEvent->type) {
         handler = Handler_Tbl.list[i].handler;
         break;
      }
   }

   sge_mutex_unlock("handler_table_mutex", SGE_FUNC, __LINE__, &Handler_Tbl.mutex);

   if (handler != NULL) {
      handler(ctx, anEvent, monitor);
   } else {
      WARNING((SGE_EVENT, MSG_SYSTEM_RECEIVEDUNKNOWNEVENT_I, anEvent->type ));
   }

   if (RECURRING_EVENT == anEvent->mode)
   {
      anEvent->when = time(NULL) + anEvent->interval;

      DPRINTF(("%s: reccuring event (t:%d w:"sge_u32" m:"sge_u32" s:%s)\n",
               EVENT_FRMT(anEvent)));

      te_add_event(anEvent);
   }

   DEXIT;
   return;
} /* te_scan_table_and_deliver */
