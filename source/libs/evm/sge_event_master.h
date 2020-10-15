#ifndef __SGE_M_EVENT_H
#define __SGE_M_EVENT_H
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

#include "basis_types.h"

#include "sgeobj/sge_event.h"

#include "gdi/sge_gdi_ctx.h"

#include "uti/sge_monitor.h"

/*
 * EVENT_MASTER_MIN_FREE_DESCRIPTORS
 * Event master assumes that every event client requires one file descriptor
 * for communication (in commlib).
 * This define is the number of file descriptors not to be used by
 * event master, but for use by the program containing event master
 * (sge_qmaster).
 */
#define EVENT_MASTER_MIN_FREE_DESCRIPTORS 25

/*
 ***** event_master_control_t definition ********************
 *
 * This struct contains all the control information needed
 * to have the event master running. It contains the references
 * to all lists, mutexes, and booleans.
 *
 ***********************************************************
 */

typedef struct {
   bool     is_transaction;                /* identifies, if a transaction is open, or not */
   lList    *transaction_requests;         /* a list storing all event add requests happening, while a transaction is open */
} event_master_transaction_t;
 
typedef struct {
   pthread_mutex_t  mutex;                 /* used for mutual exclusion. only use in public functions   */
   pthread_cond_t   cond_var;              /* used for waiting                                          */
   pthread_mutex_t  cond_mutex;            /* used for mutual exclusion. only use in internal functions */
   bool             delivery_signaled;     /* signals that an event delivery has been signaled          */
                                           /* protected by cond_mutex.                                  */

   u_long32         max_event_clients;     /* contains the max number of custom event clients, the      */
                                           /* scheduler is not accounted for. protected by mutex.       */

   bool             is_prepare_shutdown;   /* is set, when the qmaster is going down. Do not accept     */
                                           /* new event clients, when this is set to false. protected   */
                                           /* by mutex.                                                 */
   lList*           clients;               /* list of event master clients                              */
   lList*           client_ids;            /* range list holding free event client ids                  */
   lList*           requests;              /* event master requests (add/mod/del evc, add/ack event)    */
   pthread_mutex_t  request_mutex;         /* used to protect access to the request list                */

   pthread_key_t     transaction_key;      /* key to access thread local transaction storage            */
} event_master_control_t;

extern event_master_control_t Event_Master_Control;

void sge_event_master_process_requests(monitoring_t *monitor);
void sge_event_master_send_events(sge_gdi_ctx_class_t *ctx, lListElem *report, lList *report_list, monitoring_t *monitor);
void sge_event_master_wait_next(void);

int sge_add_event_client(lListElem *ev,
                         lList **alpp,
                         lList **eclpp,
                         char *ruser,
                         char *rhost,
                         event_client_update_func_t update_func,
                         monitoring_t *monitor);

int sge_mod_event_client(lListElem *clio, lList **alpp, char *ruser, char *rhost);
bool sge_has_event_client(u_long32 aClientID);
void sge_remove_event_client(u_long32 aClientID);
lList* sge_select_event_clients(const char *list_name, const lCondition *where, const lEnumeration *what);
int sge_shutdown_event_client(u_long32 aClientID, const char* anUser, uid_t anUID, lList **alpp, monitoring_t *monitor);
int sge_shutdown_dynamic_event_clients(const char *anUser, lList **alpp, monitoring_t *monitor);

bool sge_add_event(u_long32 timestamp,
                   ev_event type,
                   u_long32 intkey,
                   u_long32 intkey2,
                   const char *strkey,
                   const char *strkey2, 
                   const char *session,
                   lListElem *element);
                          
bool sge_add_event_for_client(u_long32 aClientID,
                              u_long32 aTimestamp,
                              ev_event type,
                              u_long32 anIntKey1, 
                              u_long32 anIntKey2,
                              const char *aStrKey1,
                              const char *aStrKey2, 
                              const char *aSession,
                              lListElem *element);
                                    
bool sge_add_list_event(u_long32 timestamp,
                        ev_event type, 
                        u_long32 intkey,
                        u_long32 intkey2,
                        const char *strkey, 
                        const char *strkey2,
                        const char *session,
                        lList *list);

bool sge_handle_event_ack(u_long32 event_client_id, u_long32 event_number);
void sge_deliver_events_immediately(u_long32 aClientID);

int sge_resync_schedd(monitoring_t *monitor);

u_long32 sge_set_max_dynamic_event_clients(u_long32 max);
u_long32 sge_get_max_dynamic_event_clients(void);

void sge_event_master_shutdown(void);
void sge_event_master_init(void);
bool sge_commit(void);
void sge_set_commit_required(void);

#endif /* __SGE_M_EVENT_H */
