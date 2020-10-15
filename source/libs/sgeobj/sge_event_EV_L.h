#ifndef __SGE_EVENT_EV_L_H
#define __SGE_EVENT_EV_L_H

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

#include "uti/sge_monitor.h"
#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */

/* documentation see libs/evc/sge_event_client.c */
enum {
   /* identification */
   EV_id = EV_LOWERBOUND,    /* unique id requested by client or given by qmaster */
   EV_name,                  /* name of event client (non unique) */

   /* addressing informations */
   EV_host,                  /* host to deliver the events */
   EV_commproc,              /* used to deliver events */
   EV_commid,                /* unique id delivered by the sge_commd */

   /* authentication informations */
   EV_uid,

   /* event request description */
   EV_d_time,                /* delivery interval for events */
   EV_flush_delay,           /* flush delay paramter */
   /* je kleiner EV_flush_delay eines Event client
      desto unmittelbarer das Drosseln des Event-flushens */
   EV_subscribed,            /* a list of subscribed events */
   
   EV_changed,               /* identifies changes in the subscription list */
   EV_busy_handling,         /* how to handle busy-states */
   EV_session,               /* session key to be used for filtering subscribed events */

   /* dynamic data */
   EV_last_heard_from,         /* used to trash unheard event clients */
   EV_last_send_time,        /* time when last event list has been sent */
   EV_next_send_time,        /* time when next list has to be sent */
   EV_next_number,           /* the number the next event will get */
   EV_busy,                  /* is the client busy? */
   EV_events,                /* used to hold the events that */
                             /* are not acknowledged */
   EV_sub_array,             /* contains an array of subscribed events, only used in qmaster */      
   
   EV_state,                 /* identifies the state the event client is in. Sofar we have: connected, closing, terminated */
   EV_update_function        /* stores an update function for process internal event clients (threads) */
};

LISTDEF(EV_Type)
   JGDI_ROOT_OBJ(EventClient, SGE_EV_LIST, GET_LIST)

/*    SGE_ULONG(EV_id, CULL_DEFAULT) */
   SGE_ULONG(EV_id, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_DEFAULT)
   SGE_STRING(EV_name, CULL_DEFAULT)
   
   SGE_HOST(EV_host, CULL_DEFAULT)
   SGE_STRING(EV_commproc, CULL_DEFAULT)
   SGE_ULONG(EV_commid, CULL_DEFAULT)
   
   SGE_ULONG(EV_uid, CULL_DEFAULT)
   
   SGE_ULONG(EV_d_time, CULL_DEFAULT)
   SGE_ULONG(EV_flush_delay, CULL_DEFAULT)
   SGE_LIST(EV_subscribed, EVS_Type, CULL_DEFAULT)
   SGE_BOOL(EV_changed, CULL_DEFAULT)
   SGE_ULONG(EV_busy_handling, CULL_DEFAULT)
   SGE_STRING(EV_session, CULL_DEFAULT)
   
   SGE_ULONG(EV_last_heard_from, CULL_DEFAULT)
   SGE_ULONG(EV_last_send_time, CULL_DEFAULT)
   SGE_ULONG(EV_next_send_time, CULL_DEFAULT)
   SGE_ULONG(EV_next_number, CULL_DEFAULT)
   SGE_ULONG(EV_busy, CULL_DEFAULT)
   SGE_LIST(EV_events, ET_Type, CULL_DEFAULT)
   SGE_REF(EV_sub_array, CULL_ANY_SUBTYPE, CULL_DEFAULT)

   SGE_ULONG(EV_state, CULL_DEFAULT)
   SGE_REF(EV_update_function, CULL_ANY_SUBTYPE, CULL_DEFAULT)
LISTEND 

NAMEDEF(EVN)
   NAME("EV_id")
   NAME("EV_name")

   NAME("EV_host")
   NAME("EV_commproc")
   NAME("EV_commid")

   NAME("EV_uid")

   NAME("EV_d_time")
   NAME("EV_flush_delay")
   NAME("EV_subscribed")
   NAME("EV_changed")
   NAME("EV_busy_handling")
   NAME("EV_session")

   NAME("EV_last_heard_from")
   NAME("EV_last_send_time")
   NAME("EV_next_send_time")
   NAME("EV_next_number")
   NAME("EV_busy")
   NAME("EV_events")
   NAME("EV_sub_array")

   NAME("EV_state")
   NAME("EV_update_function")
NAMEEND

#define EVS sizeof(EVN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_EVENTL_H */
