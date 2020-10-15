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
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_prog.h"
#include "uti/sge_profiling.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_error_class.h"
#include "uti/sge_mtutil.h"

#include "comm/commlib.h"
#include "comm/commproc.h"

#include "gdi/qm_name.h"
#include "gdi/sge_gdi2.h"
#include "gdi/msg_gdilib.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_ack.h"

#include "sgeobj/sge_event.h"

#include "evc/sge_event_client.h"
#include "evc/msg_evclib.h"

#include "msg_common.h"

#define EVC_LAYER TOP_LAYER

/****** Eventclient/--Event_Client_Interface **********************************
*  NAME
*     Evenclient -- The Grid Engine Event Client Interface
*
*  FUNCTION
*     The Grid Engine Event Client Interface provides a means to connect 
*     to the Grid Engine qmaster and receive information about objects 
*     (actual object properties and changes).
*
*     It provides a subscribe/unsubscribe mechanism allowing fine grained
*     selection of objects per object types (e.g. jobs, queues, hosts)
*     and event types (e.g. add, modify, delete).
*
*     Flushing triggered by individual events can be set.
*
*     Policies can be set how to handle busy event clients.
*     Clients that receive large numbers of events or possibly take some
*     time for processing the events (e.g. depending on other components
*     like databases), should in any case make use of the busy handling.
*
*     The event client interface should have much less impact on qmaster 
*     performance than polling the same data in regular intervals.
*
*  SEE ALSO
*     Eventclient/--EV_Type
*     Eventclient/-Subscription
*     Eventclient/-Flushing
*     Eventclient/-Busy-state
*     Eventclient/Client/--Event_Client
*     Eventclient/Server/--Event_Client_Server
*******************************************************************************/
/****** Eventclient/-ID-numbers ***************************************
*
*  NAME
*     ID-numbers -- id numbers for registration
*
*  SYNOPSIS
*     #include <sge_eventL.h>
*
*  FUNCTION
*     Each event client registered at qmaster has a unique client id.
*     The request for registering at qmaster contains either a concrete
*     id the client wants to occupy, or it asks the qmaster to assign
*     an id.
*     The client sets the id to use at registration with the 
*     ec_prepare_registration function call.
*     
*     The following id's can be used:
*        EV_ID_ANY    - qmaster will assign a unique id
*        EV_ID_SCHEDD - register at qmaster as scheduler
*
*  NOTES
*     As long as an event client does not expect any special handling 
*     within qmaster, it should let qmaster assign an id.
*
*     If a client expects special handling, a new id in the range 
*     [2;10] has to be created and the special handling has to be
*     implemented in qmaster (probably in sge_event_master.c).
*
*  SEE ALSO
*     Eventclient/Client/ec_prepare_registration()
*     Eventclient/Server/sge_add_event_client()
****************************************************************************
*/
/****** Eventclient/-Subscription ***************************************
*
*  NAME
*     Subscription -- Subscription interface for event clients
*
*  FUNCTION
*     An event client is notified, if certain event conditions are 
*     raised in qmaster.
*
*     Which events an event client is interested in can be set through
*     the subscription interface.
*
*     Events have a unique event identification that classifies them, 
*     e.g. "a job has been submitted" or "a queue has been disabled".
*
*     Delivery of events can be switched on/off for each event id
*     individually.
*
*     Subscription can be changed dynamically at runtime through the
*     ec_subscribe/ec_unsubscribe functions.
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/-Events
*     Eventclient/Client/ec_subscribe()
*     Eventclient/Client/ec_subscribe_all()
*     Eventclient/Client/ec_unsubscribe()
*     Eventclient/Client/ec_unsubscribe_all()
*     Eventclient/Client/ec_subscribe_flush()
****************************************************************************
*/

/****** Eventclient/-Flushing ***************************************
*
*  NAME
*     Flushing -- Configuration of event flushing
*
*  FUNCTION
*     In the standard configuration, qmaster will deliver events
*     in certain intervals to event clients.
*     These intervals can be configured using the ec_set_edtime()
*     function call.
*
*     In certain cases, an event client will want to be immediately 
*     notified, if certain events occur. A scheduler for example
*     could be notified immediately, if resources in a cluster become
*     available to allow instant refilling of the cluster.
*
*     The event client interface allows to configure flushing 
*     for each individual event id.
*
*     Flushing can either be switched off (default) for an event, or
*     a delivery time can be configured. 
*     If a delivery time is configured, events for the event client
*     will be delivered by qmaster at latest after this delivery time.
*     A delivery time of 0 means instant delivery.
*
*     Flushing can be changed dynamically at runtime through the
*     ec_set_flush/ec_unset_flush functions.
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/Client/ec_set_edtime()
*     Eventclient/Client/ec_set_flush()
*     Eventclient/Client/ec_unset_flush()
*     Eventclient/Client/ec_subscribe_flush()
****************************************************************************
*/

/****** Eventclient/-List filtering***************************************
*
*  NAME
*     List filtering -- Configuration of the list filtering 
*
*  FUNCTION
*    The data sent with an event can be filtered on the master side.
*    Therefore one can set a where and what condition. If
*    all client data is removed via where condition, no event will
*    be send.
*
*    The method expects a lListElem representation of the lCondition
*    and lEnumeration. The two methods: "lWhatToElem" and 
*    "lWhereToElem" will convert the structures.
*
*  NOTES
*    One has to be carefull reducing the elements via what condition.
*    One has to ensure, that all elements have a custom descriptor
*    and that elements with different descriptors are not mixed in
*    the same list.
*
*    The master and client can benefit (in speed and memory consumption)
*    a lot by requesting only the data, the client actually needs.
*
*    All registered events for the same cull data structure need to have
*    the same what and where filter.
*
*    The JAT_Type list is handled special, because it is subscribable as
*    an event, and it is also a sub-structure in the JB_Type list. When
*    a JAT_Type filter is set, the JAT-Lists in the JB-List are filtered
*    as well.
*
*  SEE ALSO
*     Eventclient/Client/ec_mod_subscription_where()
*     cull/cull_what/lWhatToElem()
*     cull/cull_what/lWhatFromElem()
*     cull/cull_where/lWhereToElem()
*     cull/cull_where/lWhereFromElem()
****************************************************************************
*/

/****** Eventclient/-Busy-state ***************************************
*
*  NAME
*     Busy-state -- Handling of busy event clients
*
*  FUNCTION
*     An event client may have time periods where it is busy doing some
*     processing and can not accept new events.
*     
*     In this case, qmaster should not send out new events but buffer them,
*     as otherwise timeouts would occur waiting for acknowledges.
*
*     Therefore an event client can set a policy that defines how busy
*     states are set and unset.
*
*     Which policy to use is set with the ev_set_busy_handling() function 
*     call. The policy can be changed dynamically at runtime.
*
*     The following policies are implemented at present:
*        EV_BUSY_NO_HANDLING    - busy state is not handled automatically
*                                 by the event client interface
*        EV_BUSY_UNTIL_ACK      - when delivering events qmaster will set
*                                 the eventclient to busy and will unset 
*                                 the busy state when the event client
*                                 acknowledges receipt of the events.
*        EV_BUSY_UNTIL_RELEASED - when delivering events qmaster will set
*                                 the eventclient to busy.
*                                 It will stay in the busy state until it
*                                 is explicitly released by the client 
*                                 (calling ec_set_busy(0)) 
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/Client/ec_set_busy_handling()
*     Eventclient/Client/ec_set_busy()
****************************************************************************
*/
/****** Eventclient/-Session filtering *************************************
*
*  NAME
*     Session filtering -- Filtering of events using a session key.
*
*  FUNCTION
*     For JAPI event clients event subscription is not sufficient to
*     track only those events that are related to a JAPI session.
*     
*     When session filtering is used the following rules are applied 
*     to decide whether subscribed events are sent or not:
*
*     - Events that are associated with a specific session are not
*       sent when session filtering is used but the session does not 
*       match. 
*     - Events that are not associated with a specific session are 
*       not sent. The only exception are total update events. If subscribed
*       these events are always sent and a more fine grained filtering is 
*       applied.
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/Client/ec_set_session()
*     Eventclient/Client/ec_get_session()
****************************************************************************
*/
/****** Eventclient/--EV_Type ***************************************
*
*  NAME
*     EV_Type -- event client object
*
*  ELEMENTS
*     SGE_ULONG(EV_id)
*        event client id (see Eventclient/-ID-numbers)
*
*     SGE_STRING(EV_name)
*        event client name (any string characterizing the client)
*     
*     SGE_HOST(EV_host)
*        host on which the event client resides
*
*     SGE_STRING(EV_commproc)
*        addressing information
*
*     SGE_ULONG(EV_commid)
*        unique id of the event client in commd
*
*     SGE_ULONG(EV_uid)
*        user id under which the event client is running
*     
*     SGE_ULONG(EV_d_time)
*        event delivery time (interval used by qmaster to deliver events)
*
*     SGE_List(EV_subscribed)
*        information about subscription and flushing
*        (see Eventclient/-Subscription and Eventclient/-Flushing)
*
*     SGE_ULONG(EV_busy_handling)
*        information about handling of busy states
*        (see Eventclient/-Busy-state)
*     
*     SGE_ULONG(EV_last_heard_from)
*        time when qmaster heard from the event client for the last time
*
*     SGE_ULONG(EV_last_send_time)
*        time of the last delivery of events
*
*     SGE_ULONG(EV_next_send_time)
*        next time scheduled for the delivery of events
*
*     SGE_ULONG(EV_next_number)
*        next sequential number for events (each event gets a unique number)
*
*     SGE_ULONG(EV_busy)
*        is the event client busy? (0 = false, 1 = true)
*        no events will be delivered to a busy client
*
*     SGE_LIST(EV_events)
*        list of events - they will be spooled until the event client 
*        acknowledges receipt
*  
*  FUNCTION
*     An event client creates and initializes such an object and passes
*     it to qmaster for registration.
*     Qmaster will fill some of the fields (e.g. the EV_id) and send
*     the object back to the event client on successful registration.
*     Whenever the event client wants to change some configuration
*     parameters, it changes the object and sends it to qmaster
*     with a modification request.
*  
*     Qmaster uses the object internally to track sequential numbers, 
*     delivery times, timeouts etc.
*
*  SEE ALSO
*     Eventclient/--Event_Client_Interface
*     Eventclient/-ID-numbers
*     Eventclient/-Subscription
*     Eventclient/-Flushing
*     Eventclient/-Busy-state
****************************************************************************
*/
/****** Eventclient/-Events ***************************************
*
*  NAME
*     Events -- events available from qmaster
*
*  FUNCTION
*     The following events can be raised in qmaster and be subscribed
*     by an event client:
*
*        sgeE_ALL_EVENTS                  
*     
*        sgeE_ADMINHOST_LIST              send admin host list at registration
*        sgeE_ADMINHOST_ADD               event add admin host
*        sgeE_ADMINHOST_DEL               event delete admin host
*        sgeE_ADMINHOST_MOD               event modify admin host
*     
*        sgeE_CALENDAR_LIST               send calendar list at registration
*        sgeE_CALENDAR_ADD                event add calendar
*        sgeE_CALENDAR_DEL                event delete calendar
*        sgeE_CALENDAR_MOD                event modify calendar
*     
*        sgeE_CKPT_LIST                   send ckpt list at registration
*        sgeE_CKPT_ADD                    event add ckpt
*        sgeE_CKPT_DEL                    event delete ckpt
*        sgeE_CKPT_MOD                    event modify ckpt
*     
*        sgeE_CENTRY_LIST                 send complex entry list at reg.
*        sgeE_CENTRY_ADD                  event add complex entry
*        sgeE_CENTRY_DEL                  event delete complex entry
*        sgeE_CENTRY_MOD                  event modify complex entry
*     
*        sgeE_CONFIG_LIST                 send config list at registration
*        sgeE_CONFIG_ADD                  event add config
*        sgeE_CONFIG_DEL                  event delete config
*        sgeE_CONFIG_MOD                  event modify config
*     
*        sgeE_EXECHOST_LIST               send exec host list at registration
*        sgeE_EXECHOST_ADD                event add exec host
*        sgeE_EXECHOST_DEL                event delete exec host
*        sgeE_EXECHOST_MOD                event modify exec host
*     
*        sgeE_GLOBAL_CONFIG               global config changed, replace by sgeE_CONFIG_MOD
*     
*        sgeE_JATASK_ADD                  event add array job task
*        sgeE_JATASK_DEL                  event delete array job task
*        sgeE_JATASK_MOD                  event modify array job task
*     
*        sgeE_PETASK_ADD,                 event add a new pe task
*        sgeE_PETASK_DEL,                 event delete a pe task
*
*        sgeE_JOB_LIST                    send job list at registration
*        sgeE_JOB_ADD                     event job add (new job)
*        sgeE_JOB_DEL                     event job delete
*        sgeE_JOB_MOD                     event job modify
*        sgeE_JOB_MOD_SCHED_PRIORITY      event job modify priority
*        sgeE_JOB_USAGE                   event job online usage
*        sgeE_JOB_FINAL_USAGE             event job final usage report after job end
*        sgeE_JOB_FINISH                  job finally finished or aborted (user view) 
*        sgeE_JOB_SCHEDD_INFO_LIST        send job schedd info list at registration
*        sgeE_JOB_SCHEDD_INFO_ADD         event jobs schedd info added
*        sgeE_JOB_SCHEDD_INFO_DEL         event jobs schedd info deleted
*        sgeE_JOB_SCHEDD_INFO_MOD         event jobs schedd info modified
*     
*        sgeE_MANAGER_LIST                send manager list at registration
*        sgeE_MANAGER_ADD                 event add manager
*        sgeE_MANAGER_DEL                 event delete manager
*        sgeE_MANAGER_MOD                 event modify manager
*     
*        sgeE_OPERATOR_LIST               send operator list at registration
*        sgeE_OPERATOR_ADD                event add operator
*        sgeE_OPERATOR_DEL                event delete operator
*        sgeE_OPERATOR_MOD                event modify operator
*     
*        sgeE_NEW_SHARETREE               replace possibly existing share tree
*     
*        sgeE_PE_LIST                     send pe list at registration
*        sgeE_PE_ADD                      event pe add
*        sgeE_PE_DEL                      event pe delete
*        sgeE_PE_MOD                      event pe modify
*     
*        sgeE_PROJECT_LIST                send project list at registration
*        sgeE_PROJECT_ADD                 event project add
*        sgeE_PROJECT_DEL                 event project delete
*        sgeE_PROJECT_MOD                 event project modify
*     
*        sgeE_QMASTER_GOES_DOWN           qmaster notifies all event clients, before
*                                         it exits
*     
*        sgeE_QUEUE_LIST                  send queue list at registration
*        sgeE_QUEUE_ADD                   event queue add
*        sgeE_QUEUE_DEL                   event queue delete
*        sgeE_QUEUE_MOD                   event queue modify
*        sgeE_QUEUE_SUSPEND_ON_SUB        queue is suspended by subordinate mechanism
*        sgeE_QUEUE_UNSUSPEND_ON_SUB      queue is unsuspended by subordinate mechanism
*     
*        sgeE_SCHED_CONF                  replace existing (sge) scheduler configuration
*     
*        sgeE_SCHEDDMONITOR               trigger scheduling run
*     
*        sgeE_SHUTDOWN                    request shutdown of an event client
*     
*        sgeE_SUBMITHOST_LIST             send submit host list at registration
*        sgeE_SUBMITHOST_ADD              event add submit host
*        sgeE_SUBMITHOST_DEL              event delete submit hostsource/libs/evc/sge_event_client.c
*        sgeE_SUBMITHOST_MOD              event modify submit host
*     
*        sgeE_USER_LIST                   send user list at registration
*        sgeE_USER_ADD                    event user add
*        sgeE_USER_DEL                    event user delete
*        sgeE_USER_MOD                    event user modify
*     
*        sgeE_USERSET_LIST                send userset list at registration
*        sgeE_USERSET_ADD                 event userset add
*        sgeE_USERSET_DEL                 event userset delete
*        sgeE_USERSET_MOD                 event userset modify
*  
*     If user mapping is enabled (compile time option), the following
*     additional events can be subscribed:
*
*        sgeE_CUSER_ENTRY_LIST            send list of user mappings
*        sgeE_CUSER_ENTRY_ADD             a new user mapping was added
*        sgeE_CUSER_ENTRY_DEL             a user mapping was deleted
*        sgeE_CUSER_ENTRY_MOD             a user mapping entry was changed
*     
*        sgeE_HGROUP_LIST                 send list of host groups
*        sgeE_HGROUP_ADD                  a host group was added
*        sgeE_HGROUP_DEL                  a host group was deleted
*        sgeE_HGROUP_MOD                  a host group was changed
*
*  NOTES
*     This list of events will increase as further event situations
*     are identified and interfaced.
*     
*     IF YOU ADD EVENTS HERE, ALSO UPDATE sge_mirror!
*
*  SEE ALSO
*     Eventclient/-Subscription
****************************************************************************
*/

/****** Eventclient/Client/--Event_Client **********************************
*  NAME
*     Event Client Interface -- Client Functionality 
*
*  FUNCTION
*     The client side of the event client interface provides functions
*     to register and deregister (before registering, you have to call
*     ec_prepare_registration to set an id and client name).
*  
*     The subscribe / unsubscribe mechanism allows to select the data
*     (object types and events) an event client shall be sent.
*
*     It is possible to set the interval in which qmaster will send
*     new events to an event client.
*
*  EXAMPLE
*     clients/qevent/qevent.c can serve as a simple example.
*     The scheduler (daemons/qmaster/sge_thread_scheduler.c) 
*     is also implemented as (local) event client and uses all 
*     mechanisms of the event client interface.
*
*  SEE ALSO
*     Eventclient/--Event_Client_Interface
*     Eventclient/Client/ec_prepare_registration()
*     Eventclient/Client/ec_register()
*     Eventclient/Client/ec_deregister()
*     Eventclient/Client/ec_subscribe()
*     Eventclient/Client/ec_subscribe_all()
*     Eventclient/Client/ec_unsubscribe()
*     Eventclient/Client/ec_unsubscribe_all()
*     Eventclient/Client/ec_get_flush()
*     Eventclient/Client/ec_set_flush()
*     Eventclient/Client/ec_unset_flush()
*     Eventclient/Client/ec_subscribe_flush()
*     Eventclient/Client/ec_set_edtime()
*     Eventclient/Client/ec_get_edtime()
*     Eventclient/Client/ec_set_busy_handling()
*     Eventclient/Client/ec_get_busy_handling()
*     Eventclient/Client/ec_commit()
*     Eventclient/Client/ec_get()
*     Eventclient/Client/ec_mark4registration()
*     Eventclient/Client/ec_need_new_registration()
*******************************************************************************/

/****** Eventclient/Client/-Event_Client_Global_Variables *********************
*  NAME
*     Global_Variables -- global variables in the client 
*
*  SYNOPSIS
*     static bool config_changed = false;
*     static bool need_register  = true;
*     static lListElem *ec      = NULL;
*     static u_long32 ec_reg_id = 0;
*     static u_long32 next_event = 1;
*
*  FUNCTION
*     config_changed - the configuration changed (subscription or event 
*                      interval)
*     need_register  - the client is not registered at the server
*     ec             - event client object holding configuration data
*     ec_reg_id      - the id used to register at the qmaster
*     next_event     - the sequence number of the next event we are waiting for
*
*  NOTES
*     These global variables should be part of the event client object.
*     The only remaining global variable would be a list of event client objects,
*     allowing one client to connect to multiple event client servers.
*
*******************************************************************************/

#define EC_TIMEOUT_S 10
#define EC_TIMEOUT_N 0

typedef struct {
   pthread_mutex_t mutex;      /* used for mutual exclusion                         */
   pthread_cond_t  cond_var;   /* used for waiting                                  */
   bool            exit;       /* true -> exit event delivery                       */
   bool            triggered;  /* new events addded, a scheduling run is triggered  */
   lList           *new_events; /* the storage for new events                       */
   bool            rebuild_categories;
   bool            new_global_conf;
} ec_control_t;

typedef struct {
   bool need_register;
   lListElem *ec;
   u_long32 ec_reg_id;
   u_long32 next_event;
   sge_gdi_ctx_class_t *sge_gdi_ctx;
   ec_control_t event_control;
} sge_evc_t;


static bool ck_event_number(lList *lp, u_long32 *waiting_for, u_long32 *wrong_number);
static void ec2_add_subscriptionElement(sge_evc_class_t *thiz, ev_event event, bool flush, int interval); 
static void ec2_remove_subscriptionElement(sge_evc_class_t *thiz, ev_event event); 
static void ec2_mod_subscription_flush(sge_evc_class_t *thiz, ev_event event, bool flush, int intervall);
static void ec2_config_changed(sge_evc_class_t *thiz);
static bool sge_evc_setup(sge_evc_class_t *thiz, 
                          sge_gdi_ctx_class_t *sge_gdi_ctx,
                          ev_registration_id id, const char *ec_name);
static void sge_evc_destroy(sge_evc_t **sge_evc);
static bool ec2_is_initialized(sge_evc_class_t *thiz); 
static lListElem* ec2_get_event_client(sge_evc_class_t *thiz); 
static void ec2_mark4registration(sge_evc_class_t *thiz);
static bool ec2_need_new_registration(sge_evc_class_t *thiz);
static int ec2_set_edtime(sge_evc_class_t *thiz, int interval);
static int ec2_get_edtime(sge_evc_class_t *thiz);
static bool ec2_set_flush_delay(sge_evc_class_t *thiz, int flush_delay);
static int ec2_get_flush_delay(sge_evc_class_t *thiz);
static bool ec2_set_busy_handling(sge_evc_class_t *thiz, ev_busy_handling handling);
static ev_busy_handling ec2_get_busy_handling(sge_evc_class_t *thiz);
static bool ec2_register(sge_evc_class_t *thiz, bool exit_on_qmaster_down, lList** alpp, monitoring_t *monitor);
static bool ec2_deregister(sge_evc_class_t *thiz);
static bool ec2_deregister_local(sge_evc_class_t *thiz);
static bool ec2_register_local(sge_evc_class_t *thiz, bool exit_on_qmaster_down, lList** alpp, monitoring_t *monitor);
static bool ec2_subscribe(sge_evc_class_t *thiz, ev_event event);
static void ec2_add_subscriptionElement(sge_evc_class_t *thiz, ev_event event, bool flush, int interval);
static void ec2_mod_subscription_flush(sge_evc_class_t *thiz, ev_event event, bool flush, int intervall);
static bool ec2_mod_subscription_where(sge_evc_class_t *thiz, ev_event event, const lListElem *what, const lListElem *where);
static void ec2_remove_subscriptionElement(sge_evc_class_t *thiz, ev_event event);
static bool ec2_subscribe_all(sge_evc_class_t *thiz);
static bool ec2_unsubscribe(sge_evc_class_t *thiz, ev_event event);
static bool ec2_unsubscribe_all(sge_evc_class_t *thiz);
static int ec2_get_flush(sge_evc_class_t *thiz, ev_event event);
static bool ec2_set_flush(sge_evc_class_t *thiz, ev_event event, bool flush, int interval);
static bool ec2_unset_flush(sge_evc_class_t *thiz, ev_event event);
static bool ec2_subscribe_flush(sge_evc_class_t *thiz, ev_event event, int flush);
static bool ec2_set_busy(sge_evc_class_t *thiz, int busy);
static bool ec2_get_busy(sge_evc_class_t *thiz);
static bool ec2_set_session(sge_evc_class_t *thiz, const char *session);
static const char *ec2_get_session(sge_evc_class_t *thiz);
static void ec2_config_changed(sge_evc_class_t *thiz);
static bool ec2_commit(sge_evc_class_t *thiz, lList **alpp);                        
static bool ec2_commit_local(sge_evc_class_t *thiz, lList **alpp); 
static bool ec2_commit_multi(sge_evc_class_t *thiz, lList **malpp, state_gdi_multi *state);
static bool ec2_ack(sge_evc_class_t *thiz);
static bool ec2_get(sge_evc_class_t *thiz, lList **event_list, bool exit_on_qmaster_down);
static bool get_event_list(sge_evc_class_t *thiz, int sync, lList **report_list, int *commlib_error);
static ev_registration_id ec2_get_id(sge_evc_class_t *thiz);
static sge_gdi_ctx_class_t *get_gdi_ctx(sge_evc_class_t *thiz);
static bool ec2_get_local(sge_evc_class_t *thiz, lList **event_list, bool exit_on_qmaster_down);
static void ec2_wait_local(sge_evc_class_t *thiz);
static int ec2_signal_local(sge_evc_class_t *thiz, lList **alpp, lList *event_list);
static void ec2_wait(sge_evc_class_t *thiz);
static int ec2_signal(sge_evc_class_t *thiz, lList **alpp, lList *event_list);
static ec_control_t *ec2_get_event_control(sge_evc_class_t *thiz);
static bool ec2_evco_triggered(sge_evc_class_t *thiz);
static bool ec2_evco_exit(sge_evc_class_t *thiz);

sge_evc_class_t *
sge_evc_class_create(sge_gdi_ctx_class_t *sge_gdi_ctx, ev_registration_id reg_id, 
                     lList **alpp, const char *name)
{
   sge_evc_class_t *ret = (sge_evc_class_t *)sge_malloc(sizeof(sge_evc_class_t));
   sge_evc_t *sge_evc = NULL;
   bool is_qmaster_internal = false;

   DENTER(EVC_LAYER, "sge_evc_class_create");

   if (!ret) {
      answer_list_add_sprintf(alpp, STATUS_EMALLOC, ANSWER_QUALITY_ERROR,
                              "%s", MSG_MEMORY_MALLOCFAILED);
      DRETURN(NULL);
   }

   /*
   ** get type of connection internal/external
   */
   is_qmaster_internal = sge_gdi_ctx->is_qmaster_internal_client(sge_gdi_ctx);
   
   DPRINTF(("creating %s event client context\n", is_qmaster_internal ? "internal" : "external"));

   if (is_qmaster_internal == true) {
      ret->ec_register = ec2_register_local;
      ret->ec_deregister = ec2_deregister_local;
      ret->ec_commit = ec2_commit_local;
      ret->ec_get = ec2_get_local;
      ret->ec_signal = ec2_signal_local;
      ret->ec_wait = ec2_wait_local;
   } else {
      ret->ec_register = ec2_register;
      ret->ec_deregister = ec2_deregister;
      ret->ec_commit = ec2_commit;
      ret->ec_get = ec2_get;
      ret->ec_signal = ec2_signal;
      ret->ec_wait = ec2_wait;
   }

   ret->get_gdi_ctx = get_gdi_ctx;
   ret->ec_is_initialized = ec2_is_initialized;
   ret->ec_get_event_client = ec2_get_event_client;
   ret->ec_subscribe = ec2_subscribe;
   ret->ec_subscribe_all = ec2_subscribe_all;
   ret->ec_unsubscribe = ec2_unsubscribe;
   ret->ec_unsubscribe_all = ec2_unsubscribe_all;
   ret->ec_get_flush = ec2_get_flush;
   ret->ec_set_flush = ec2_set_flush;
   ret->ec_unset_flush = ec2_unset_flush;
   ret->ec_subscribe_flush = ec2_subscribe_flush;
   ret->ec_mod_subscription_where = ec2_mod_subscription_where;
   ret->ec_set_edtime = ec2_set_edtime;
   ret->ec_get_edtime = ec2_get_edtime;
   ret->ec_set_busy_handling = ec2_set_busy_handling;
   ret->ec_get_busy_handling = ec2_get_busy_handling;
   ret->ec_set_flush_delay = ec2_set_flush_delay;
   ret->ec_get_flush_delay = ec2_get_flush_delay;
   ret->ec_set_busy = ec2_set_busy;
   ret->ec_get_busy = ec2_get_busy;
   ret->ec_set_session = ec2_set_session;
   ret->ec_get_session = ec2_get_session;
   ret->ec_get_id = ec2_get_id;
   ret->ec_commit_multi = ec2_commit_multi;
   ret->ec_mark4registration = ec2_mark4registration;
   ret->ec_need_new_registration = ec2_need_new_registration;
   ret->ec_ack = ec2_ack;
   ret->ec_evco_triggered = ec2_evco_triggered;
   ret->ec_evco_exit = ec2_evco_exit;

   ret->sge_evc_handle = NULL;

   sge_evc = (sge_evc_t*)sge_malloc(sizeof(sge_evc_t));
   if (!sge_evc) {
      answer_list_add_sprintf(alpp, STATUS_EMALLOC, ANSWER_QUALITY_ERROR,
                              "%s", MSG_MEMORY_MALLOCFAILED);
      sge_evc_class_destroy(&ret);
      DRETURN(NULL);
   }
   sge_evc->need_register = true;
   sge_evc->ec = NULL;
   sge_evc->ec_reg_id = 0;
   sge_evc->next_event = 1;

   ret->sge_evc_handle = sge_evc;

   if (!sge_evc_setup(ret, sge_gdi_ctx, reg_id, name)) {
      sge_evc_class_destroy(&ret);
      DRETURN(NULL);
   }

   DRETURN(ret);
}   

void sge_evc_class_destroy(sge_evc_class_t **pst)
{
   DENTER(EVC_LAYER, "sge_evc_class_destroy");

   if (pst == NULL || *pst == NULL) {
      DRETURN_VOID;
   }   
      
   sge_evc_destroy((sge_evc_t **)&((*pst)->sge_evc_handle));
   sge_free(pst);
   DRETURN_VOID;
}

static void sge_evc_destroy(sge_evc_t **sge_evc)
{
   DENTER(EVC_LAYER, "sge_evc_destroy");
   
   if (sge_evc == NULL || *sge_evc == NULL) {
      DRETURN_VOID;
   }   

   /*
   ** signal all threads waiting on condition before destroy
   */
   pthread_mutex_lock(&((*sge_evc)->event_control.mutex));
   pthread_cond_broadcast(&((*sge_evc)->event_control.cond_var));
   pthread_mutex_unlock(&((*sge_evc)->event_control.mutex));
                                                                                                  
   pthread_cond_destroy(&((*sge_evc)->event_control.cond_var));
   pthread_mutex_destroy(&((*sge_evc)->event_control.mutex));
   lFreeList(&((*sge_evc)->event_control.new_events));

   lFreeElem(&((*sge_evc)->ec));
   sge_free(sge_evc);
   
   DRETURN_VOID;
}

/****** Eventclient/Client/ec_prepare_registration() ***************************
*  NAME
*     ec_prepare_registration() -- prepare registration at server
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_prepare_registration(ev_registration_id id, const char *name) 
*
*  FUNCTION
*     Initializes necessary data structures and sets the data needed for
*     registration at an event server.
*     The events sgeE_QMASTER_GOES_DOWN and sgeE_SHUTDOWN are subscribed.
*
*     For valid id's see Eventclient/-ID-numbers.
*
*     The name is informational data that will be included in messages
*     (errors, warnings, infos) and will be shown in the command line tool
*     qconf -sec.
*
*  INPUTS
*     ev_registration_id id  - id used to register
*     const char *name       - name of the event client
*
*  RESULT
*     bool - true, if the function succeeded, else false 
*
*  SEE ALSO
*     Eventclient/--EV_Type
*     Eventclient/-ID-numbers
*     Eventclient/-Subscription
*     qconf manpage
*     list of events
*******************************************************************************/
static bool sge_evc_setup(sge_evc_class_t *thiz, 
                          sge_gdi_ctx_class_t *sge_gdi_ctx,
                          ev_registration_id id,
                          const char *ec_name)
{
   bool ret = false;
   const char *name = NULL;
   sge_evc_t *sge_evc = (sge_evc_t*)thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "sge_evc_setup");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   sge_evc->sge_gdi_ctx = sge_gdi_ctx;

   /*
   ** event_control setup for internal event clients
   */
   pthread_mutex_init(&(sge_evc->event_control.mutex), NULL);
   pthread_cond_init(&(sge_evc->event_control.cond_var), NULL);
   sge_evc->event_control.exit = false;
   sge_evc->event_control.triggered = false;
   sge_evc->event_control.new_events = NULL;
   sge_evc->event_control.rebuild_categories = true;
   sge_evc->event_control.new_global_conf = false;

   if (ec_name != NULL) {
      name = ec_name;
   } else {
      name = sge_gdi_ctx->get_progname(sge_gdi_ctx);
   }

   if (id >= EV_ID_FIRST_DYNAMIC || name == NULL || *name == 0) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGAL_ID_OR_NAME_US, 
               sge_u32c(id), name != NULL ? name : "NULL" ));
   } else {
      sge_evc->ec = lCreateElem(EV_Type);

      if (sge_evc->ec != NULL) {
         stringT tmp_string;

         /* remember registration id for subsequent registrations */
         sge_evc->ec_reg_id = id;

         /* initialize event client object */
         lSetString(sge_evc->ec, EV_name, name);
         if (gethostname(tmp_string, sizeof(tmp_string)) == 0) {
            lSetHost(sge_evc->ec, EV_host, tmp_string); 
         }
         /*
         ** for internal clients we reuse the data of the gdi context
         */
         lSetString(sge_evc->ec, EV_commproc, sge_gdi_ctx->get_component_name(sge_gdi_ctx));
         lSetUlong(sge_evc->ec, EV_commid, 0);
         lSetUlong(sge_evc->ec, EV_d_time, DEFAULT_EVENT_DELIVERY_INTERVAL);

         /* always subscribe this three events */
         ec2_subscribe_flush(thiz, sgeE_QMASTER_GOES_DOWN, 0);
         ec2_subscribe_flush(thiz, sgeE_SHUTDOWN, 0);
         ec2_subscribe_flush(thiz, sgeE_ACK_TIMEOUT, 0);

         ec2_set_busy_handling(thiz, EV_BUSY_UNTIL_ACK);
         lSetUlong(sge_evc->ec, EV_busy, 0);
         ec2_config_changed(thiz);
         ret = true;
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}


static sge_gdi_ctx_class_t *get_gdi_ctx(sge_evc_class_t *thiz)
{
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   return sge_evc->sge_gdi_ctx;
}


/****** Eventclient/Client/ec_is_initialized() ********************************
*  NAME
*     ec_is_initialized() -- has the client been initialized
*
*  SYNOPSIS
*     int 
*     ec_is_initialized(void) 
*
*  FUNCTION
*     Checks if the event client mechanism has been initialized
*     (if ec_prepare_registration has been called).
*
*  RESULT
*     int - true, if the event client interface has been initialized,
*           else false.
*
*  SEE ALSO
*     Eventclient/Client/ec_prepare_registration()
*******************************************************************************/
static bool ec2_is_initialized(sge_evc_class_t *thiz) 
{
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   
   if (sge_evc == NULL || sge_evc->ec == NULL) {
      return false;
   } else {
      return true;
   }
}

/****** Eventclient/Client/ec_get_event_client() ********************************
*  NAME
*     ec_get_event_client() -- return lList *event_client cull list
*
*  SYNOPSIS
*     lList * 
*     ec_get_event_client(sge_evc_class_t *thiz) 
*
*  FUNCTION
*     return lList *event_client cull list
*
*  RESULT
*     lList *event_client - NULL otherwise
*
*  SEE ALSO
*     Eventclient/Client/ec_prepare_registration()
*******************************************************************************/
static lListElem* ec2_get_event_client(sge_evc_class_t *thiz) 
{
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   
   return sge_evc->ec;
}

/****** Eventclient/Client/ec_mark4registration() *****************************
*  NAME
*     ec_mark4registration() -- new registration is required
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     void 
*     ec_mark4registration(void) 
*
*  FUNCTION
*     Tells the event client mechanism, that the connection to the server
*     is broken and it has to reregister.
*
*  NOTES
*     Should be no external interface. The event client mechanism should itself
*     detect such situations and react accordingly.
*
*  SEE ALSO
*     Eventclient/Client/ec_need_new_registration()
*******************************************************************************/
static void ec2_mark4registration(sge_evc_class_t *thiz)
{
   cl_com_handle_t* handle = NULL;
   sge_evc_t *sge_evc = (sge_evc_t*)thiz->sge_evc_handle;
   sge_gdi_ctx_class_t *sge_gdi_ctx = thiz->get_gdi_ctx(thiz);
   const char *mastername = sge_gdi_ctx->get_master(sge_gdi_ctx, true);

   DENTER(EVC_LAYER, "ec2_mark4registration");

   handle = sge_gdi_ctx->get_com_handle(sge_gdi_ctx);
   if (handle != NULL) {
      cl_commlib_close_connection(handle, (char*)mastername, (char*)prognames[QMASTER], 1, false);
      DPRINTF(("closed old connection to qmaster\n"));
   }
   sge_evc->need_register = true;
   DPRINTF(("*** Need new registration at qmaster ***\n"));
   lSetBool(sge_evc->ec, EV_changed, true);
   DRETURN_VOID;
}

/****** Eventclient/Client/ec_need_new_registration() *************************
*  NAME
*     ec_need_new_registration() -- is a reregistration neccessary?
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool ec_need_new_registration(void) 
*
*  FUNCTION
*     Function to check, if a new registration at the server is neccessary.
*
*  RESULT
*     bool - true, if the client has to (re)register, else false 
*
*******************************************************************************/
static bool ec2_need_new_registration(sge_evc_class_t *thiz)
{
   sge_evc_t *sge_evc = NULL;

   DENTER(EVC_LAYER, "ec2_need_new_registration");
   sge_evc = (sge_evc_t*) thiz->sge_evc_handle;
   DRETURN(sge_evc->need_register);
}

/****** Eventclient/Client/ec_set_edtime() ************************************
*  NAME
*     ec_set_edtime() -- set the event delivery interval
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     int 
*     ec_set_edtime(int interval) 
*
*  FUNCTION
*     Set the interval qmaster will use to send events to the client.
*     Any number > 0 is a valid interval in seconds. However the interval 
*     my not be larger than the commd commproc timeout. Otherwise the event 
*     client encounters receive timeouts and this is returned as an error 
*     by ec_get().
*
*  INPUTS
*     int interval - interval [s]
*
*  RESULT
*     int - 1, if the value was changed, else 0
*
*  NOTES
*     The maximum interval is limited to the commd commproc timeout.
*     The maximum interval should be limited also by the application. 
*     A too big interval makes qmaster spool lots of events and consume 
*     a lot of memory.
*
*  SEE ALSO
*     Eventclient/Client/ec_get_edtime()
*******************************************************************************/
static int ec2_set_edtime(sge_evc_class_t *thiz, int interval) 
{
   int ret = 0;
   sge_evc_t *sge_evc = (sge_evc_t *)thiz->sge_evc_handle; 

   DENTER(EVC_LAYER, "ec2_set_edtime");
   
   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      ret = (lGetUlong(sge_evc->ec, EV_d_time) != interval);
      if (ret > 0) {
         lSetUlong(sge_evc->ec, EV_d_time, MIN(interval, CL_DEFINE_CLIENT_CONNECTION_LIFETIME-5));
         ec2_config_changed(thiz);
      }
   }

   DRETURN(ret);
}

/****** Eventclient/Client/ec_get_edtime() ************************************
*  NAME
*     ec_get_edtime() -- get the current event delivery interval
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     int 
*     ec_get_edtime(void) 
*
*  FUNCTION
*     Get the interval qmaster will use to send events to the client.
*
*  RESULT
*     int - the interval in seconds
*
*  SEE ALSO
*     Eventclient/Client/ec_set_edtime()
*******************************************************************************/
static int ec2_get_edtime(sge_evc_class_t *thiz) 
{
   int interval = 0;
   sge_evc_t *sge_evc = (sge_evc_t *)thiz->sge_evc_handle; 

   DENTER(EVC_LAYER, "ec2_get_edtime");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      interval = lGetUlong(sge_evc->ec, EV_d_time);
   }

   DRETURN(interval);
}

/****** Eventclient/Client/ec_set_flush_delay() *****************************
*  NAME
*     ec_set_flush_delay() -- set flush delay parameter
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_set_flush_delay(u_long32 flush_delay) 
*
*  FUNCTION
*
*  INPUTS
*     int flush_delay - flush delay parameter
*
*  RESULT
*     bool - true, if the value was changed, else false
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/Client/ec_get_flush_delay()
*     Eventclient/-Busy-state
*     Eventclient/Client/ec_commit()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_set_flush_delay(sge_evc_class_t *thiz, int flush_delay) 
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *)thiz->sge_evc_handle; 

   DENTER(EVC_LAYER, "ec2_set_flush_delay");
   
   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      ret = (lGetUlong(sge_evc->ec, EV_flush_delay) != flush_delay) ? true : false;

      if (ret) {
         lSetUlong(sge_evc->ec, EV_flush_delay, flush_delay);
         ec2_config_changed(thiz);
      }
   }

   DRETURN(ret);
}

/****** Eventclient/Client/ec_get_flush_delay() *****************************
*  NAME
*     ec_get_flush_delay() -- get configured flush delay paramter
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     int 
*     ec_get_flush_delay(void) 
*
*  FUNCTION
*     Returns the policy currently configured.
*
*  RESULT
*     flush_delay - current flush delay parameter setting
*
*  SEE ALSO
*     Eventclient/Client/ec_set_flush_delay()
*     Eventclient/-Busy-state
*******************************************************************************/
static int ec2_get_flush_delay(sge_evc_class_t *thiz) 
{
   int flush_delay = 0;
   sge_evc_t *sge_evc = (sge_evc_t *)thiz->sge_evc_handle; 

   DENTER(EVC_LAYER, "ec2_get_flush_delay");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      flush_delay = lGetUlong(sge_evc->ec, EV_flush_delay);
   }

   DRETURN(flush_delay);
}


/****** Eventclient/Client/ec_set_busy_handling() *****************************
*  NAME
*     ec_set_edtime() -- set the event client busy handling
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_set_busy_handling(ev_busy_handling handling) 
*
*  FUNCTION
*     The event client interface has a mechanism to handle situations in which
*     an event client is busy and will not accept any new events. 
*     The policy to use can be configured using this function.
*     For valid policies see ...
*     This parameter can be changed during runtime and will take effect
*     after a call to ec_commit or ec_get.
*
*  INPUTS
*     ev_busy_handling handling - the policy to use
*
*  RESULT
*     bool - true, if the value was changed, else false
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/Client/ec_get_busy_handling()
*     Eventclient/-Busy-state
*     Eventclient/Client/ec_commit()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_set_busy_handling(sge_evc_class_t *thiz, ev_busy_handling handling) 
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_set_busy_handling");
   
   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      DPRINTF(("EVC: change event client to "sge_U32CFormat"\n", sge_u32c(handling)));

      ret = (lGetUlong(sge_evc->ec, EV_busy_handling) != handling) ? true : false;

      if (ret) {
         lSetUlong(sge_evc->ec, EV_busy_handling, handling);
         ec2_config_changed(thiz);
      }
   }

   DRETURN(ret);
}

/****** Eventclient/Client/ec_get_busy_handling() *****************************
*  NAME
*     ec_get_busy_handling() -- get configured busy handling policy
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     ev_busy_handling 
*     ec_get_busy_handling(void) 
*
*  FUNCTION
*     Returns the policy currently configured.
*
*  RESULT
*     ev_busy_handling - the current policy
*
*  SEE ALSO
*     Eventclient/Client/ec_set_edtime()
*     Eventclient/-Busy-state
*******************************************************************************/
static ev_busy_handling ec2_get_busy_handling(sge_evc_class_t *thiz) 
{
   ev_busy_handling handling = EV_BUSY_NO_HANDLING;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_get_busy_handling");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      handling = (ev_busy_handling)lGetUlong(sge_evc->ec, EV_busy_handling);
   }

   DRETURN(handling);
}

static bool ec2_deregister_local(sge_evc_class_t *thiz)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_deregister_local");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   /* not yet initialized? Nothing to shutdown */
   if (sge_evc == NULL || sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      local_t *evc_local = &(thiz->ec_local);
      u_long32 id = sge_evc->ec_reg_id;
      ec_control_t *evco = NULL;

      DPRINTF(("ec2_deregister_local sge_evc->ec_reg_id "sge_u32"\n", sge_evc->ec_reg_id));

      /*
      ** signal thread when in ec2_get_local
      */
      evco = ec2_get_event_control(thiz);
      if (evco == NULL) {
         DPRINTF(("ec2_deregister_local evco IS NULL\n"));
         DRETURN(false);
      }

      sge_mutex_lock("event_control_mutex", SGE_FUNC, __LINE__, &(evco->mutex));  
      
      evco->exit = true;

      DPRINTF(("----> evco->exit = true\n"));

      pthread_cond_signal(&(evco->cond_var));
#ifdef EVC_DEBUG      
      {
      dstring dsbuf;
      char buf[1024];
      sge_dstring_init(&dsbuf, buf, sizeof(buf));
      printf("EVENT_CLIENT %d has been signaled at %s\n", thiz->ec_get_id(thiz), sge_ctime(0, &dsbuf));
      }
#endif
      sge_mutex_unlock("event_control_mutex", SGE_FUNC, __LINE__, &(evco->mutex));

      if (id != 0 && evc_local && evc_local->remove_func) {
         evc_local->remove_func(id);
      }

      /* clear state of this event client instance */
      lFreeElem(&(sge_evc->ec));
      sge_evc->need_register = true;
      sge_evc->ec_reg_id = 0;
      sge_evc->next_event = 1; 

      ret = true;
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}


static bool
ec2_register_local(sge_evc_class_t *thiz, bool exit_on_qmaster_down, lList** alpp, monitoring_t *monitor)
{
   bool ret = true;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_register_local");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   if (!thiz->ec_need_new_registration(thiz)) {
      DRETURN(ret);
   }

   sge_evc->next_event = 1;

   DPRINTF(("trying to register as internal client with preset %d (0 means EV_ID_ANY)\n", (int)sge_evc->ec_reg_id));

   if (sge_evc->ec == NULL) {
      WARNING((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
      ret = false;
   } else {
      lList *alp = NULL;
      lListElem *aep = NULL;
      local_t *evc_local = &(thiz->ec_local); 

      lSetUlong(sge_evc->ec, EV_id, sge_evc->ec_reg_id);

      /* initialize, we could do a re-registration */
      lSetUlong(sge_evc->ec, EV_last_heard_from, 0);
      lSetUlong(sge_evc->ec, EV_last_send_time, 0);
      lSetUlong(sge_evc->ec, EV_next_send_time, 0);
      lSetUlong(sge_evc->ec, EV_next_number, 0);

      /*
       *  to add may also mean to modify
       *  - if this event client is already enrolled at qmaster
       */

      if (evc_local && evc_local->add_func) {
         lList *eclp = NULL;
         const char *ruser = NULL;
         const char *rhost = NULL;
         sge_gdi_ctx_class_t *gdi_ctx = thiz->get_gdi_ctx(thiz); 
         if (gdi_ctx != NULL) {
            ruser = gdi_ctx->get_admin_user(gdi_ctx);
            rhost = gdi_ctx->get_master(gdi_ctx, false);
         }
         /*
         ** set busy handling, sets EV_changed to true if it is really changed
         */
         thiz->ec_set_busy_handling(thiz, EV_BUSY_UNTIL_RELEASED);
         evc_local->add_func(sge_evc->ec, &alp, &eclp, (char*)ruser, (char*)rhost, evc_local->update_func, monitor);
         if (eclp) {
            sge_evc->ec_reg_id = lGetUlong(lFirst(eclp), EV_id);
            lFreeList(&eclp);
         }
      }

      if (alp != NULL) {
         aep = lFirst(alp);
         ret = ((lGetUlong(aep, AN_status) == STATUS_OK) ? true : false);
      }

      if (!ret) {
         if (lGetUlong(aep, AN_quality) == ANSWER_QUALITY_ERROR) {
            ERROR((SGE_EVENT, "%s", lGetString(aep, AN_text)));
            answer_list_add(alpp, lGetString(aep, AN_text),
                  lGetUlong(aep, AN_status), (answer_quality_t)lGetUlong(aep, AN_quality));

            ret = false;
         }
      } else {
         lSetBool(sge_evc->ec, EV_changed, false);
         sge_evc->need_register = false;
         DPRINTF(("registered local event client with id "sge_u32"\n", sge_evc->ec_reg_id));
      }

      lFreeList(&alp);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);
   DRETURN(ret);
}


/****** Eventclient/Client/ec_register() ***************************************
*  NAME
*     ec_register() -- register at the event server
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_register(void) 
*
*  FUNCTION
*     Registers the event client at the event server (usually the qmaster). 
*     This function can be called explicitly in the event client at startup
*     or when the connection to qmaster is down.
*
*     It will be called implicitly by ec_get, whenever it detects the neccessity
*     to (re)register the client.
*
*  RESULT
*     bool - true, if the registration succeeded, else false
*
*  SEE ALSO
*     Eventclient/Client/ec_deregister()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_register(sge_evc_class_t *thiz, bool exit_on_qmaster_down, lList** alpp, monitoring_t *monitor)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   sge_gdi_ctx_class_t * sge_gdi_ctx = thiz->get_gdi_ctx(thiz);

   DENTER(EVC_LAYER, "ec2_register");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   if (sge_evc->ec == NULL) {
      WARNING((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      lList *lp, *alp;
      lListElem *aep;
      /*
       *   EV_host, EV_commproc and EV_commid get filled
       *  at qmaster side with more secure commd    
       *  informations 
       *
       *  EV_uid gets filled with gdi_request
       *  informations
       */

      lSetUlong(sge_evc->ec, EV_id, sge_evc->ec_reg_id);

      /* initialize, we could do a re-registration */
      lSetUlong(sge_evc->ec, EV_last_heard_from, 0);
      lSetUlong(sge_evc->ec, EV_last_send_time, 0);
      lSetUlong(sge_evc->ec, EV_next_send_time, 0);
      lSetUlong(sge_evc->ec, EV_next_number, 0);

      lp = lCreateList("registration", EV_Type);
      lAppendElem(lp, lCopyElem(sge_evc->ec));


#if 0
   cl_com_handle_t* com_handle = NULL;
   const char* progname = NULL;
   const char* mastername = NULL;

      progname = sge_gdi_ctx->get_progname(sge_gdi_ctx);
      mastername = sge_gdi_ctx->get_master(sge_gdi_ctx);

      /* TODO: is this code section really necessary */
      /* closing actual connection to qmaster and reopen new connection. This will delete all
         buffered messages  - CR */
      com_handle = sge_gdi_ctx->get_com_handle(sge_gdi_ctx);
      if (com_handle != NULL) {
         int ngc_error;
         ngc_error = cl_commlib_close_connection(com_handle, (char*)mastername, (char*)prognames[QMASTER], 1, false);
         if (ngc_error == CL_RETVAL_OK) {
            DPRINTF(("closed old connection to qmaster\n"));
         } else {
            INFO((SGE_EVENT, "error closing old connection to qmaster: "SFQ"\n", cl_get_error_text(ngc_error)));
         }
         ngc_error = cl_commlib_open_connection(com_handle, (char*)mastername, (char*)prognames[QMASTER], 1);
         if (ngc_error == CL_RETVAL_OK) {
            DPRINTF(("opened new connection to qmaster\n"));
         } else {
            ERROR((SGE_EVENT, "error opening new connection to qmaster: "SFQ"\n", cl_get_error_text(ngc_error)));
         }
      }
#endif      

      /*
       *  to add may also means to modify
       *  - if this event client is already enrolled at qmaster
       */
      alp = sge_gdi_ctx->gdi(sge_gdi_ctx, SGE_EV_LIST, SGE_GDI_ADD | SGE_GDI_RETURN_NEW_VERSION, &lp, NULL, NULL, false);
    
      aep = lFirst(alp);
    
      ret = (lGetUlong(aep, AN_status) == STATUS_OK) ? true : false;

      if (ret) { 
         lListElem *new_ec;
         u_long32 new_id = 0;

         new_ec = lFirst(lp);
         if(new_ec != NULL) {
            new_id = lGetUlong(new_ec, EV_id);
         }

         if (new_id != 0) {
            lSetUlong(sge_evc->ec, EV_id, new_id);
            DPRINTF(("REGISTERED with id "sge_U32CFormat"\n", sge_u32c(new_id)));
            lSetBool(sge_evc->ec, EV_changed, false);
            sge_evc->need_register = false;
            
         }
      } else {
         if (lGetUlong(aep, AN_quality) == ANSWER_QUALITY_ERROR) {
            ERROR((SGE_EVENT, "%s", lGetString(aep, AN_text)));
            answer_list_add(alpp, lGetString(aep, AN_text), 
                  lGetUlong(aep, AN_status), 
                  (answer_quality_t)lGetUlong(aep, AN_quality));
            lFreeList(&lp); 
            lFreeList(&alp);
            /* TODO: remove exit_on_qmaster_down and move to calling code by delivering
                     better return values */
            if (exit_on_qmaster_down) {
               DPRINTF(("exiting in ec2_register()\n"));
               SGE_EXIT((void**)&sge_gdi_ctx, 1);
            } else {
               /*
                * Trigger commlib in case of errors. This is to prevent 100% CPU usage
                * when client does not handle errors and perform a wait before retry
                * in an endless while loop.
                */
               cl_com_handle_t* com_handle = NULL;
               com_handle = sge_gdi_ctx->get_com_handle(sge_gdi_ctx);
               if (com_handle != NULL) {
                  cl_commlib_trigger(com_handle, 1);
               } else {
                  /* We have no commlib handle, do a sleep() */
                  sleep(1);
               }
               DRETURN(false);
            }
         }   
      }

      lFreeList(&lp); 
      lFreeList(&alp);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

/****** Eventclient/Client/ec_deregister() ************************************
*  NAME
*     ec_deregister() -- deregister from the event server
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     int 
*     ec_deregister(void) 
*
*  FUNCTION
*     Deregister from the event server (usually the qmaster).
*     This function should be called when an event client exits. 
*
*     If an event client does not deregister, qmaster will spool events for this
*     client until it times out (it did not acknowledge events sent by qmaster).
*     After the timeout, it will be deleted.
*
*  RESULT
*     int - true, if the deregistration succeeded, else false
*
*  SEE ALSO
*     Eventclient/Client/ec_register()
*******************************************************************************/
static bool ec2_deregister(sge_evc_class_t *thiz)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   sge_gdi_ctx_class_t *sge_gdi_ctx = thiz->get_gdi_ctx(thiz);

   DENTER(EVC_LAYER, "ec2_deregister");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   /* not yet initialized? Nothing to shutdown */
   if (sge_evc->ec != NULL) {
      sge_pack_buffer pb;

      if (init_packbuffer(&pb, sizeof(u_long32), 0) == PACK_SUCCESS) {
         /* error message is output from init_packbuffer */
         int send_ret; 
         lList *alp = NULL;
         /* TODO: to master only !!!!! */
         const char* commproc = prognames[QMASTER];
         const char* rhost = sge_gdi_ctx->get_master(sge_gdi_ctx, false);
         int         commid   = 1;


         packint(&pb, lGetUlong(sge_evc->ec, EV_id));

         send_ret = sge_gdi2_send_any_request(sge_gdi_ctx, 0, NULL, rhost, commproc, commid, &pb, TAG_EVENT_CLIENT_EXIT, 0, &alp);
         
         clear_packbuffer(&pb);
         answer_list_output (&alp);

         if (send_ret == CL_RETVAL_OK) {
            /* error message is output from sge_send_any_request */
            /* clear state of this event client instance */
            lFreeElem(&(sge_evc->ec));
            sge_evc->need_register = true;
            sge_evc->ec_reg_id = 0;
            sge_evc->next_event = 1;

            ret = true;
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

/****** Eventclient/Client/ec_subscribe() *************************************
*  NAME
*     ec_subscribe() -- Subscribe an event
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_subscribe(ev_event event) 
*
*  FUNCTION
*     Subscribe a certain event.
*     See Eventclient/-Events for a list of all events.
*     The subscription will be in effect after calling ec_commit or ec_get.
*     It is possible / sensible to subscribe all events wanted and then call
*     ec_commit.
*
*  INPUTS
*     ev_event event - the event number
*
*  RESULT
*     bool - true on success, else false
*
*  SEE ALSO
*     Eventclient/-Events
*     Eventclient/Client/ec_subscribe_all()
*     Eventclient/Client/ec_unsubscribe()
*     Eventclient/Client/ec_unsubscribe_all()
*     Eventclient/Client/ec_commit()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_subscribe(sge_evc_class_t *thiz, ev_event event)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   
   DENTER(EVC_LAYER, "ec2_subscribe");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event < sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event));
   } else {
      if (event == sgeE_ALL_EVENTS) {
         ev_event i;
         for(i = sgeE_ALL_EVENTS; i < sgeE_EVENTSIZE; i++) {
            ec2_add_subscriptionElement(thiz, i, EV_NOT_FLUSHED, -1);
         }
      } else {
         ec2_add_subscriptionElement(thiz, event, EV_NOT_FLUSHED, -1);
      }
      
      if (lGetBool(sge_evc->ec, EV_changed)) {
         ret = true;
      }
   }
   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

static void ec2_add_subscriptionElement(sge_evc_class_t *thiz, ev_event event, bool flush, int interval) 
{

   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_add_subscriptionElement");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event < sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event));
   } else {
      lListElem *sub_el = NULL;
      lList *subscribed = lGetList(sge_evc->ec, EV_subscribed);
      if (event != sgeE_ALL_EVENTS){
         if (!subscribed) {
            subscribed = lCreateList("subscription list", EVS_Type);
            lSetList(sge_evc->ec, EV_subscribed, subscribed);
         } else {
            sub_el = lGetElemUlong(subscribed, EVS_id, event);
         }
         
         if (!sub_el) {
            sub_el =  lCreateElem(EVS_Type);
            lAppendElem(subscribed, sub_el);
            
            lSetUlong(sub_el, EVS_id, event);
            lSetBool(sub_el, EVS_flush, flush);
            lSetUlong(sub_el, EVS_interval, interval);

            lSetBool(sge_evc->ec, EV_changed, true);
         }
      }
   }
   DRETURN_VOID;
}

static void ec2_mod_subscription_flush(sge_evc_class_t *thiz, ev_event event, bool flush, int intervall) 
{
   lList *subscribed = NULL;
   lListElem *sub_el = NULL; 
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_mod_subscription_flush");
  
   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event < sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event));
   } else {
      subscribed = lGetList(sge_evc->ec, EV_subscribed);
      if (event != sgeE_ALL_EVENTS){
         if (subscribed) {
            sub_el = lGetElemUlong(subscribed, EVS_id, event);
            if (sub_el) {
               lSetBool(sub_el, EVS_flush, flush);
               lSetUlong(sub_el, EVS_interval, intervall);
               lSetBool(sge_evc->ec, EV_changed, true);
            }
         }
      }
   }

   DRETURN_VOID;
}

/****** sge_event_client/ec_mod_subscription_where() ***************************
*  NAME
*     ec_mod_subscription_where() -- adds an element filter to the event 
*
*  SYNOPSIS
*     bool ec_mod_subscription_where(ev_event event, const lListElem *what, 
*     const lListElem *where) 
*
*  FUNCTION
*     Allows to filter the event date on the master side to reduce the
*     date, which is send to the clients.
*
*  INPUTS
*     ev_event event         - event type 
*     const lListElem *what  - what condition 
*     const lListElem *where - where condition 
*
*  RESULT
*     bool - true, if everything went fine 
*
*  SEE ALSO
*     cull/cull_what/lWhatToElem()
*     cull/cull_what/lWhatFromElem()
*     cull/cull_where/lWhereToElem()
*     cull/cull_where/lWhereFromElem()
*******************************************************************************/
static bool ec2_mod_subscription_where(sge_evc_class_t *thiz, ev_event event, const lListElem *what, const lListElem *where) {
   lList *subscribed = NULL;
   lListElem *sub_el = NULL;
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_mod_subscription_where");
 
   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event <= sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event));
   } else {
      subscribed = lGetList(sge_evc->ec, EV_subscribed);
      if (event != sgeE_ALL_EVENTS){
         if (subscribed) {
            sub_el = lGetElemUlong(subscribed, EVS_id, event);
            if (sub_el) {
               lSetObject(sub_el, EVS_what, lCopyElem(what));
               lSetObject(sub_el, EVS_where, lCopyElem(where));
               lSetBool(sge_evc->ec, EV_changed, true);
               ret = true;
            }
         }
      }
   }

   DRETURN(ret);
}

static void ec2_remove_subscriptionElement(sge_evc_class_t *thiz, ev_event event) 
{

   lList *subscribed =NULL; 
   lListElem *sub_el = NULL;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   
   DENTER(EVC_LAYER, "ec2_remove_subscriptionElement");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event < sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event));
   } else {
      subscribed = lGetList(sge_evc->ec, EV_subscribed);
      if (event != sgeE_ALL_EVENTS){
         if (subscribed) {
            sub_el = lGetElemUlong(subscribed, EVS_id, event);
            if (sub_el) {
               if (lRemoveElem(subscribed, &sub_el) == 0) {
                  lSetBool(sge_evc->ec, EV_changed, true);
               }
            }
         }
      }
   }
   DRETURN_VOID;
}
/****** Eventclient/Client/ec_subscribe_all() *********************************
*  NAME
*     ec_subscribe_all() -- subscribe all events
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_subscribe_all(void) 
*
*  FUNCTION
*     Subscribe all possible event.
*     The subscription will be in effect after calling ec_commit or ec_get.
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     Subscribing all events can cause a lot of traffic and may 
*     decrease performance of qmaster.
*     Only subscribe all events, if you really need them.
*
*  SEE ALSO
*     Eventclient/Client/ec_subscribe()
*     Eventclient/Client/ec_unsubscribe()
*     Eventclient/Client/ec_unsubscribe_all()
*     Eventclient/Client/ec_commit()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_subscribe_all(sge_evc_class_t *thiz)
{
   return ec2_subscribe(thiz, sgeE_ALL_EVENTS);
}

/****** Eventclient/Client/ec_unsubscribe() ***********************************
*  NAME
*     ec_unsubscribe() -- unsubscribe an event
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_unsubscribe(ev_event event) 
*
*  FUNCTION
*     Unsubscribe a certain event.
*     See ... for a list of all events.
*     The change will be in effect after calling ec_commit or ec_get.
*     It is possible / sensible to unsubscribe all events 
*     no longer needed and then call ec_commit.
*
*  INPUTS
*     ev_event event - the event to unsubscribe
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     The events sgeE_QMASTER_GOES_DOWN and sgeE_SHUTDOWN cannot
*     be unsubscribed. ec_unsubscribe will output an error message if
*     you try to unsubscribe sgeE_QMASTER_GOES_DOWN or sgeE_SHUTDOWN
*     and return 0.
*
*  SEE ALSO
*     Eventclient/-Events
*     Eventclient/Client/ec_subscribe()
*     Eventclient/Client/ec_subscribe_all()
*     Eventclient/Client/ec_unsubscribe_all()
*     Eventclient/Client/ec_commit()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_unsubscribe(sge_evc_class_t *thiz, ev_event event)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   
   DENTER(EVC_LAYER, "ec2_unsubscribe");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event < sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event ));
   } else {
      if (event == sgeE_ALL_EVENTS) {
         ev_event i;
         for (i = sgeE_ALL_EVENTS; i < sgeE_EVENTSIZE; i++) {
            ec2_remove_subscriptionElement(thiz, i);
         }
         ec2_add_subscriptionElement(thiz, sgeE_QMASTER_GOES_DOWN, EV_FLUSHED, 0);
         ec2_add_subscriptionElement(thiz, sgeE_ACK_TIMEOUT, EV_FLUSHED, 0);
         ec2_add_subscriptionElement(thiz, sgeE_SHUTDOWN, EV_FLUSHED, 0);

      } else {
         if (event == sgeE_QMASTER_GOES_DOWN || event == sgeE_SHUTDOWN || event == sgeE_ACK_TIMEOUT) {
            ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_HAVETOHANDLEEVENTS));
         } else {
            ec2_remove_subscriptionElement(thiz, event);
         }
      }

      if (lGetBool(sge_evc->ec, EV_changed)) {
         ret = true;
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

/****** Eventclient/Client/ec_unsubscribe_all() *******************************
*  NAME
*     ec_unsubscribe_all() -- unsubscribe all events
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_unsubscribe_all(void) 
*
*  FUNCTION
*     Unsubscribe all possible event.
*     The change will be in effect after calling ec_commit or ec_get.
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     The events sgeE_QMASTER_GOES_DOWN and sgeE_SHUTDOWN will not be 
*     unsubscribed.
*
*  SEE ALSO
*     Eventclient/Client/ec_subscribe()
*     Eventclient/Client/ec_subscribe_all()
*     Eventclient/Client/ec_unsubscribe()
*     Eventclient/Client/ec_commit()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_unsubscribe_all(sge_evc_class_t *thiz)
{
   return ec2_unsubscribe(thiz, sgeE_ALL_EVENTS);
}

/****** Eventclient/Client/ec_get_flush() *************************************
*  NAME
*     ec_get_flush() -- get flushing information for an event
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     int 
*     ec_get_flush(ev_event event) 
*
*  FUNCTION
*     An event client can request flushing of events from qmaster 
*     for any number of the events subscribed.
*     This function returns the flushing information for an 
*     individual event.
*
*  INPUTS
*     ev_event event - the event id to query 
*
*  RESULT
*     int - EV_NO_FLUSH or the number of seconds used for flushing
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/-Flushing
*     Eventclient/Client/ec_set_flush()
*******************************************************************************/
static int ec2_get_flush(sge_evc_class_t *thiz, ev_event event)
{
   int ret = EV_NO_FLUSH;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   
   DENTER(EVC_LAYER, "ec2_get_flush");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event < sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event ));
   } else {
      lListElem *sub_event = lGetElemUlong(lGetList(sge_evc->ec, EV_subscribed), EVS_id, event);

      if (sub_event == NULL) {
         ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
      } else if (lGetBool(sub_event, EVS_flush)) {
         ret = lGetUlong(sub_event, EVS_interval);
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

/****** Eventclient/Client/ec_set_flush() *************************************
*  NAME
*     ec_set_flush() -- set flushing information for an event
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_set_flush(ev_event event, int flush) 
*
*  FUNCTION
*     An event client can request flushing of events from qmaster 
*     for any number of the events subscribed.
*     This function sets the flushing information for an individual 
*     event.
*
*  INPUTS
*     ev_event event - id of the event to configure
*     bool flush     - true for flushing
*     int interval   - flush interval in sec.
*                      
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/-Flushing
*     Eventclient/Client/ec_get_flush()
*     Eventclient/Client/ec_unset_flush()
*     Eventclient/Client/ec_subscribe_flush()
*******************************************************************************/
static bool ec2_set_flush(sge_evc_class_t *thiz, ev_event event, bool flush, int interval)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   
   DENTER(EVC_LAYER, "ec2_set_flush");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event < sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event ));
   } else {
      if (!flush ) {
         PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);
         ret = ec2_unset_flush(thiz, event);
         ec2_mod_subscription_flush(thiz, event, EV_NOT_FLUSHED, EV_NO_FLUSH);
         PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);
/*      } else if (interval < 0 || interval > EV_MAX_FLUSH) {
         WARNING((SGE_EVENT, MSG_EVENT_ILLEGALFLUSHTIME_I, interval)); */
      } else {
         lListElem *sub_event = lGetElemUlong(lGetList(sge_evc->ec, EV_subscribed), EVS_id, event);

         if (sub_event == NULL) {
            ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
         } else {
            ec2_mod_subscription_flush(thiz, event, EV_FLUSHED, interval);
         }
         if (lGetBool(sge_evc->ec, EV_changed)) {
            ret = true;
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

/****** Eventclient/Client/ec_unset_flush() ***********************************
*  NAME
*     ec_unset_flush() -- unset flushing information
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_unset_flush(ev_event event) 
*
*  FUNCTION
*     Switch of flushing of an individual event.
*
*  INPUTS
*     ev_event event - if of the event to configure
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/-Flushing
*     Eventclient/Client/ec_set_flush()
*     Eventclient/Client/ec_get_flush()
*     Eventclient/Client/ec_subscribe_flush()
******************************************************************************/
static bool ec2_unset_flush(sge_evc_class_t *thiz, ev_event event)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   
   DENTER(EVC_LAYER, "ec2_unset_flush");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else if (event < sgeE_ALL_EVENTS || event >= sgeE_EVENTSIZE) {
      WARNING((SGE_EVENT, MSG_EVENT_ILLEGALEVENTID_I, event));
   } else {
      lListElem *sub_event = lGetElemUlong(lGetList(sge_evc->ec, EV_subscribed), EVS_id, event);

      if (sub_event == NULL) {
         ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
      } else {
         ec2_mod_subscription_flush(thiz, event, EV_NOT_FLUSHED, EV_NO_FLUSH);
      }

      if (lGetBool(sge_evc->ec, EV_changed)) {
         ret = true;
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

/****** Eventclient/Client/ec_subscribe_flush() *******************************
*  NAME
*     ec_subscribe_flush() -- subscribe an event and set flushing
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_subscribe_flush(ev_event event, int flush) 
*
*  FUNCTION
*     Subscribes and event and configures flushing for this event.
*
*  INPUTS
*     ev_event event - id of the event to subscribe and flush
*     int flush      - number of seconds between event creation 
*                      and flushing of events
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/-Subscription
*     Eventclient/-Flushing
*     Eventclient/Client/ec_subscribe()
*     Eventclient/Client/ec_set_flush()
******************************************************************************/
static bool ec2_subscribe_flush(sge_evc_class_t *thiz, ev_event event, int flush) 
{
   bool ret; 
   
   ret = ec2_subscribe(thiz, event);
   if (ret) {
      if (flush >= 0)
         ret = ec2_set_flush(thiz, event, true, flush);
      else
         ret = ec2_set_flush(thiz, event, false, flush);
   }

   return ret;
}

/****** Eventclient/Client/ec_set_busy() **************************************
*  NAME
*     ec_set_busy() -- set the busy state
*
*  SYNOPSIS
*     bool 
*     ec_set_busy(bool busy) 
*
*  FUNCTION
*     Sets the busy state of the client. This has to be done if 
*     the busy policy has been set to EV_BUSY_UNTIL_RELEASED.
*     An event client can set or unset the busy state at any time.
*     While it is marked busy at the qmaster, qmaster will not 
*     deliver events to this client.
*     The changed busy state will be communicated to qmaster with 
*     the next call to ec_commit (implicitly called by the next 
*     ec_get).
*
*  INPUTS
*     bool busy - true = event client busy, true = event client idle
*
*  RESULT
*     bool - true = success, false = failed
*
*  NOTES
*
*  SEE ALSO
*     Eventclient/-Busy-state
*     Eventclient/Client/ec_set_busy_handling()
*     Eventclient/Client/ec_get_busy()
*******************************************************************************/
static bool ec2_set_busy(sge_evc_class_t *thiz, int busy)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_set_busy");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      lSetUlong(sge_evc->ec, EV_busy, busy);
      ret = true;
   }

   DRETURN(ret);
}

/****** Eventclient/Client/ec_get_busy() **************************************
*  NAME
*     ec_get_busy() -- get the busy state
*
*  SYNOPSIS
*     bool 
*     ec_get_busy(void) 
*
*  FUNCTION
*     Reads the busy state of the event client.
*
*  RESULT
*     bool - true: the client is busy, false: the client is idle 
*
*  NOTES
*     The function only returns the local busy state in the event 
*     client itself. If this state changes, it will be reported to 
*     qmaster with the next communication, but not back from 
*     qmaster to the client.
*
*  SEE ALSO
*     Eventclient/-Busy-state
*     Eventclient/Client/ec_set_busy_handling()
*     Eventclient/Client/ec_set_busy()
*******************************************************************************/
static bool ec2_get_busy(sge_evc_class_t *thiz)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_get_busy");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      /* JG: TODO: EV_busy should be boolean datatype */
      ret = (lGetUlong(sge_evc->ec, EV_busy) > 0) ? true : false;
   }

   DRETURN(ret);
}

/****** sge_event_client/ec_set_session() **************************************
*  NAME
*     ec_set_session() -- Specify session key for event filtering.
*
*  SYNOPSIS
*     bool ec_set_session(const char *session) 
*
*  FUNCTION
*     Specifies a session that is used in event master for event
*     filtering.
*
*  INPUTS
*     const char *session - the session key
*
*  RESULT
*     bool - true = success, false = failed
*
*  SEE ALSO
*     Eventclient/-Session filtering
*     Eventclient/Client/ec_get_session()
*******************************************************************************/
static bool ec2_set_session(sge_evc_class_t *thiz, const char *session)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_set_session");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      lSetString(sge_evc->ec, EV_session, session);

      /* force communication to qmaster - we may be out of sync */
      ec2_config_changed(thiz);
      ret = true;
   }

   DRETURN(ret);
}

/****** sge_event_client/ec_get_session() **************************************
*  NAME
*     ec_get_session() -- Get session key used for event filtering.
*
*  SYNOPSIS
*     const char* ec_get_session(void) 
*
*  FUNCTION
*     Returns session key that is used in event master for event
*     filtering.
*
*  RESULT
*     const char* - the session key
*
*  SEE ALSO
*     Eventclient/-Session filtering
*     Eventclient/Client/ec_set_session()
*******************************************************************************/
static const char *ec2_get_session(sge_evc_class_t *thiz)
{
   const char *ret = NULL;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_get_session");

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
   } else {
      ret = lGetString(sge_evc->ec, EV_session);
   }

   DRETURN(ret);
}

/****** sge_event_client/ec_get_id() *******************************************
*  NAME
*     ec_get_id() -- Return event client id.
*
*  SYNOPSIS
*     ev_registration_id ec_get_id(void) 
*
*  FUNCTION
*     Return event client id.
*
*  RESULT
*     ev_registration_id - the event client id
*******************************************************************************/
static ev_registration_id ec2_get_id(sge_evc_class_t *thiz)
{
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_get_id");
   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
      DRETURN(EV_ID_INVALID);
   }
   
   DRETURN((ev_registration_id)lGetUlong(sge_evc->ec, EV_id));
}

/****** Eventclient/Client/ec_config_changed() ********************************
*  NAME
*     ec_config_changed() -- tell system the config has changed
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     static void 
*     ec_config_changed(void) 
*
*  FUNCTION
*     Checkes whether the configuration has changes.
*     Configuration changes can either be changes in the subscription
*     or change of the event delivery interval.
*
*  SEE ALSO
*     Eventclient/Client/ec_subscribe()
*     Eventclient/Client/ec_subscribe_all()
*     Eventclient/Client/ec_unsubscribe()
*     Eventclient/Client/ec_unsubscribe_all()
*     Eventclient/Client/ec_set_edtime()
******************************************************************************/
static void ec2_config_changed(sge_evc_class_t *thiz) 
{
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   if (sge_evc != NULL && sge_evc->ec != NULL) {
      lSetBool(sge_evc->ec, EV_changed, true);
   }   
}

static bool ec2_commit_local(sge_evc_class_t *thiz, lList **alpp) 
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_commit_local");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   /* not yet initialized? Cannot send modification to qmaster! */
   if (sge_evc->ec == NULL) {
      DPRINTF((SGE_EVENT, MSG_EVENT_UNINITIALIZED_EC));
   } else if (thiz->ec_need_new_registration(thiz)) {
      /* not (yet) registered? Cannot send modification to qmaster! */
      DPRINTF(("%s", MSG_EVENT_NOTREGISTERED));
   } else {
      const char *ruser = NULL;
      const char *rhost = NULL;
      local_t *evc_local = &(thiz->ec_local);
      sge_gdi_ctx_class_t *gdi_ctx = thiz->get_gdi_ctx(thiz); 
      if (gdi_ctx != NULL) {
         ruser = gdi_ctx->get_admin_user(gdi_ctx);
         rhost = gdi_ctx->get_master(gdi_ctx, false);
      }
      lSetRef(sge_evc->ec, EV_update_function, (void*) evc_local->update_func);

      /*
       *  to add may also means to modify
       *  - if this event client is already enrolled at qmaster
       */
      ret = ((evc_local->mod_func(sge_evc->ec, alpp, (char*)ruser, (char*)rhost) == STATUS_OK) ? true : false);

      if (ret) {
         lSetBool(sge_evc->ec, EV_changed, false);
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);
   DEXIT;
   return ret;
}

static bool ec2_ack(sge_evc_class_t *thiz) 
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;

   DENTER(EVC_LAYER, "ec2_ack");

   /* not yet initialized? Cannot send modification to qmaster! */
   if (sge_evc->ec == NULL) {
      DPRINTF(("%s", MSG_EVENT_UNINITIALIZED_EC));
   } else if (thiz->ec_need_new_registration(thiz)) {
      /* not (yet) registered? Cannot send modification to qmaster! */
      DPRINTF(("%s", MSG_EVENT_NOTREGISTERED));
   } else {
      local_t *evc_local = &(thiz->ec_local);
      if (evc_local && evc_local->ack_func) {
         ret = evc_local->ack_func(sge_evc->ec_reg_id, (ev_event) (sge_evc->next_event-1));
      }   
   }
   DRETURN(ret);
}

/****** Eventclient/Client/ec_commit() ****************************************
*  NAME
*     ec_commit() -- commit configuration changes
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_commit(void) 
*
*  FUNCTION
*     Configuration changes (subscription and/or event delivery 
*     time) will be sent to the event server.
*     The function should be called after (multiple) configuration 
*     changes have been made.
*     If it is not explicitly called by the event client program, 
*     the next call of ec_get will commit configuration changes 
*     before looking for new events.
*
*  RESULT
*     bool - true on success, else false
*
*  SEE ALSO
*     Eventclient/Client/ec_commit_multi()
*     Eventclient/Client/ec_config_changed()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_commit(sge_evc_class_t *thiz, lList **alpp)
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   sge_gdi_ctx_class_t * sge_gdi_ctx = thiz->get_gdi_ctx(thiz);

   DENTER(EVC_LAYER, "ec2_commit");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   /* not yet initialized? Cannot send modification to qmaster! */
   if (sge_evc->ec == NULL) {
      DPRINTF(("%s", MSG_EVENT_UNINITIALIZED_EC));
      answer_list_add(alpp, MSG_EVENT_UNINITIALIZED_EC, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
   } else if (thiz->ec_need_new_registration(thiz)) {
      /* not (yet) registered? Cannot send modification to qmaster! */
      DPRINTF(("%s", MSG_EVENT_NOTREGISTERED));
      answer_list_add(alpp, MSG_EVENT_NOTREGISTERED, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
   } else {
      lList *lp, *alp;

      lp = lCreateList("change configuration", EV_Type);
      lAppendElem(lp, lCopyElem(sge_evc->ec));
      if (!lGetBool(sge_evc->ec, EV_changed)) {
         lSetList(lFirst(lp), EV_subscribed, NULL);
      }

      /*
       *  to add may also means to modify
       *  - if this event client is already enrolled at qmaster
       */
      alp = sge_gdi_ctx->gdi(sge_gdi_ctx, SGE_EV_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
      lFreeList(&lp); 

      if (lGetUlong(lFirst(alp), AN_status) == STATUS_OK) {
         lFreeList(&alp);
         ret = true;
      } else {
         if (alpp) {
            *alpp = alp;
         } else {
            lFreeList(&alp);
         }
         ret = false;
      }
      
      if (ret) {
         lSetBool(sge_evc->ec, EV_changed, false);
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);
   DRETURN(ret);
}

/****** Eventclient/Client/ec_commit_multi() ****************************
*  NAME
*     ec_commit() -- commit configuration changes via gdi multi request
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     int 
*     ec_commit_multi(lList **malpp) 
*
*  FUNCTION
*     Similar to ec_commit configuration changes will be sent to qmaster.
*     But unless ec_commit which uses sge_gdi to send the change request, 
*     ec_commit_multi uses a sge_gdi_multi call to send the configuration
*     change along with other gdi requests.
*     The ec_commit_multi call has to be the last request of the multi 
*     request and will trigger the communication of the requests.
*
*  INPUTS
*     malpp - answer list for the whole gdi multi request
*
*  RESULT
*     int - true on success, else false
*
*  SEE ALSO
*     Eventclient/Client/ec_commit()
*     Eventclient/Client/ec_config_changed()
*     Eventclient/Client/ec_get()
*******************************************************************************/
static bool ec2_commit_multi(sge_evc_class_t *thiz, lList **malpp, state_gdi_multi *state) 
{
   bool ret = false;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   sge_gdi_ctx_class_t * sge_gdi_ctx = thiz->get_gdi_ctx(thiz);

   DENTER(EVC_LAYER, "ec2_commit_multi");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   /* not yet initialized? Cannot send modification to qmaster! */
   if (sge_evc->ec == NULL) {
      DPRINTF(("%s", MSG_EVENT_UNINITIALIZED_EC));
   } else if (thiz->ec_need_new_registration(thiz)) {
      /* not (yet) registered? Cannot send modification to qmaster! */
      DPRINTF(("%s", MSG_EVENT_NOTREGISTERED));
   } else {
      int commit_id, gdi_ret;
      lList *lp, *alp = NULL;

      /* do not check, if anything has changed.
       * we have to send the request in any case to finish the 
       * gdi multi request
       */
      lp = lCreateList("change configuration", EV_Type);
      lAppendElem(lp, lCopyElem(sge_evc->ec));
      if (!lGetBool(sge_evc->ec, EV_changed)) {
         lSetList(lFirst(lp), EV_subscribed, NULL);
      }

      /*
       * TODO: extend sge_gdi_ctx_class_t to support sge_gdi_multi()
       *  to add may also means to modify
       *  - if this event client is already enrolled at qmaster
       */
      commit_id = sge_gdi2_multi(sge_gdi_ctx, &alp, SGE_GDI_SEND, SGE_EV_LIST, SGE_GDI_MOD,
                                &lp, NULL, NULL, false, state, false);
      sge_gdi2_wait(sge_gdi_ctx, &alp, malpp, state);
      if (lp != NULL) {                                 
         lFreeList(&lp);
      }

      if (alp != NULL) {
         answer_list_handle_request_answer_list(&alp, stderr);
      } else {
         sge_gdi_extract_answer(&alp, SGE_GDI_ADD, SGE_ORDER_LIST, commit_id, 
                                      *malpp, NULL);

         gdi_ret = answer_list_handle_request_answer_list(&alp, stderr);

         if (gdi_ret == STATUS_OK) {
            lSetBool(sge_evc->ec, EV_changed, false);  /* TODO: call changed method */
            ret = true;
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

/****** Eventclient/Client/ec_get() ******************************************
*  NAME
*     ec_get() -- look for new events
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     bool 
*     ec_get(lList **event_list) 
*
*  FUNCTION
*     ec_get looks for new events. 
*     If new events have arrived, they are passed back to the 
*     caller in the parameter event_list.
*     
*     If the event client is not yet registered at the event server, 
*     the registration will be done before looking for events.
*
*     If the configuration changed since the last call of ec_get 
*     and has not been committed, ec_commit will be called before 
*     looking for events.
*
*  INPUTS
*     lList **event_list - pointer to an event list to hold arriving 
*                          events
*
*  RESULT
*     bool - true, if events or an empty event list was received, or
*                  if no data was received within a certain time period
*            false, if an error occured
*
*  NOTES
*     The event_list has to be freed by the calling function.
*
*  SEE ALSO
*     Eventclient/Client/ec_register()
*     Eventclient/Client/ec_commit()
*******************************************************************************/
static bool ec2_get(sge_evc_class_t *thiz, lList **event_list, bool exit_on_qmaster_down) 
{
   bool ret = true;
   lList *report_list = NULL;
   u_long32 wrong_number;
   lList *alp = NULL;
   sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
   sge_gdi_ctx_class_t *sge_gdi_ctx = thiz->get_gdi_ctx(thiz);

   DENTER(EVC_LAYER, "ec2_get");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   if (sge_evc->ec == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_UNINITIALIZED_EC));
      ret = false;
   } else if (thiz->ec_need_new_registration(thiz)) {
      sge_evc->next_event = 1;
      ret = thiz->ec_register(thiz, exit_on_qmaster_down, NULL, NULL);
   }
  
   if (ret) {
      if (lGetBool(sge_evc->ec, EV_changed)) {
         ret = thiz->ec_commit(thiz, NULL);
      }
   }

   /* receive event message(s) 
    * The following problems exists here:
    * - there might be multiple event reports at commd - so fetching only one
    *   is not sufficient
    * - if we fetch reports until fetching fails, we can run into an endless
    *   loop, if qmaster sends lots of event reports (e.g. due to flushing)
    * - so what number of events shall we fetch?
    *   Let's assume that qmaster will send a maximum of 1 event message per
    *   second. Then our maximum number of messages to fetch is the number of
    *   seconds passed since last fetch.
    *   For the first fetch after registration, we can take the event delivery
    *   interval.
    * - To make sure this algorithm works in all cases, we could restrict both
    *   event delivery time and flush time to intervals greater than 1 second.
    */
   if (ret) {
      static time_t last_fetch_time = 0;
      static time_t last_fetch_ok_time = 0;
      int commlib_error = CL_RETVAL_UNKNOWN;
      time_t now;

      bool done = false;
      bool fetch_ok = false;
      int max_fetch;
      int sync = 1;

      now = (time_t)sge_get_gmt();

      /* initialize last_fetch_ok_time */
      if (last_fetch_ok_time == 0) {
         last_fetch_ok_time = thiz->ec_get_edtime(thiz);
      }

      /* initialize the maximum number of fetches */
      if (last_fetch_time == 0) {
         max_fetch = thiz->ec_get_edtime(thiz);
      } else {
         max_fetch = now - last_fetch_time;
      }

      last_fetch_time = now;

      DPRINTF(("ec2_get retrieving events - will do max %d fetches\n", 
               max_fetch));

      /* fetch data until nothing left or maximum reached */
      while (!done) {
         DPRINTF(("doing %s fetch for messages, %d still to do\n", 
                  sync ? "sync" : "async", max_fetch));
         if (thiz->ec_need_new_registration(thiz)) {
            ret = false;
            done = true;
            continue;
         }
         if ((fetch_ok = get_event_list(thiz, sync, &report_list, &commlib_error))) {
            lList *new_events = NULL;
            lXchgList(lFirst(report_list), REP_list, &new_events);
            lFreeList(&report_list);
            if (!ck_event_number(new_events, &(sge_evc->next_event), &wrong_number)) {
               /*
                *  may be we got an old event, that was sent before
                *  reregistration at qmaster
                */
               lFreeList(event_list);
               lFreeList(&new_events);
               thiz->ec_mark4registration(thiz);
               ret = false;
               done = true;
               continue;
            }

            DPRINTF(("got %d events till "sge_u32"\n", 
                     lGetNumberOfElem(new_events), sge_evc->next_event - 1));

            if (*event_list != NULL) {
               lAddList(*event_list, &new_events);
            } else {
               *event_list = new_events;
            }

         } else {
            /* get_event_list failed - we are through */
            done = true;
            continue; 
         }

         sync = 0;

         if (--max_fetch <= 0) {
            /* maximum number of fetches reached - stop fetching reports */
            done = true;
         }
      }

      /* if first synchronous get_event_list failed, return error */
      if (sync && !fetch_ok) {
         u_long32 timeout = thiz->ec_get_edtime(thiz) * 10;

         DPRINTF(("first syncronous get_event_list failed\n"));

         /* we return false when we have reached a timeout or 
            on communication error, otherwise we return true */
         ret = true;

         /* check timeout */
         if (last_fetch_ok_time + timeout < now) {
            /* we have a  SGE_EM_TIMEOUT */
            DPRINTF(("SGE_EM_TIMEOUT reached\n"));
            ret = false;
         } else {
            DPRINTF(("SGE_EM_TIMEOUT in "sge_U32CFormat" seconds\n", sge_u32c(last_fetch_ok_time + timeout - now) ));
         }

         /* check for communicaton error */
         if (commlib_error != CL_RETVAL_OK) { 
            switch (commlib_error) {
               case CL_RETVAL_NO_MESSAGE:
               case CL_RETVAL_SYNC_RECEIVE_TIMEOUT:
                  break;
               default:
                  DPRINTF(("COMMUNICATION ERROR: %s\n", cl_get_error_text(commlib_error)));
                  ret = false;
                  break;
            }
         }
      } else {
         /* set last_fetch_ok_time, because we had success */
         last_fetch_ok_time = (time_t)sge_get_gmt();

         /* send an ack to the qmaster for all received events */
         if (sge_send_ack_to_qmaster(sge_gdi_ctx, ACK_EVENT_DELIVERY, sge_evc->next_event - 1,
                                     lGetUlong(sge_evc->ec, EV_id), NULL, &alp)
                                    != CL_RETVAL_OK) {
            answer_list_output(&alp);
            WARNING((SGE_EVENT, SFNMAX, MSG_COMMD_FAILEDTOSENDACKEVENTDELIVERY));
         } else {
            DPRINTF(("Sent ack for all events lower or equal "sge_u32"\n", (sge_evc->next_event - 1)));
         }
      }
   }

   if (*event_list != NULL) {
      DPRINTF(("ec2_get - received %d events\n", lGetNumberOfElem(*event_list)));
   }

   /* check if we got a QMASTER_GOES_DOWN or sgeE_ACK_TIMEOUT event. 
    * if yes, reregister with next event fetch
    */
   if (lGetNumberOfElem(*event_list) > 0) {
      const lListElem *event;
      lUlong tmp_type;

      for_each(event, *event_list) {
         tmp_type = lGetUlong(event, ET_type);
         if (tmp_type == sgeE_QMASTER_GOES_DOWN || 
             tmp_type == sgeE_ACK_TIMEOUT) {
            ec2_mark4registration(thiz);
            break;
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}


/****** Eventclient/Client/ck_event_number() **********************************
*  NAME
*     ck_event_number() -- test event numbers
*
*  SYNOPSIS
*     #include "evc/sge_event_client.h"
*
*     static bool 
*     ck_event_number(lList *lp, u_long32 *waiting_for, 
*                     u_long32 *wrong_number) 
*
*  FUNCTION
*     Tests list of events if it contains right numbered events.
*
*     Events with numbers lower than expected get trashed.
*
*     In cases the master has added no new events to the event list 
*     and the acknowledge we sent was lost also a list with events lower 
*     than "waiting_for" is correct. 
*     But the number of the last event must be at least "waiting_for"-1.
*
*     On success *waiting_for will contain the next number we wait for.
*
*     On failure *waiting_for gets not changed and if wrong_number is not
*     NULL *wrong_number contains the wrong number we got.
*
*  INPUTS
*     lList *lp              - event list to check
*     u_long32 *waiting_for  - next number to wait for
*     u_long32 *wrong_number - event number that causes a failure
*
*  RESULT
*     static bool - true on success, else false
*
*******************************************************************************/
static bool ck_event_number(lList *lp, u_long32 *waiting_for, u_long32 *wrong_number)
{
   bool ret = true;
   lListElem *ep, *tmp;
   u_long32 i, j;
   int skipped;
  
   DENTER(EVC_LAYER, "gdi2_ck_event_number");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   i = *waiting_for;

   if (!lp || !lGetNumberOfElem(lp)) {
      /* got a dummy event list for alive protocol */
      DPRINTF(("received empty event list\n"));
   } else {
      DPRINTF(("Checking %d events (" sge_u32"-"sge_u32 ") while waiting for #"sge_u32"\n",
            lGetNumberOfElem(lp), 
            lGetUlong(lFirst(lp), ET_number),
            lGetUlong(lLast(lp), ET_number),
            i));
    
      /* ensure number of last event is "waiting_for"-1 or higher */ 
      if ((j=lGetUlong(lLast(lp), ET_number)) < i-1) {
         /* error in event numbers */
         /* could happen if the order of two event messages was exchanged */
         if (wrong_number)
            *wrong_number = j;

         ERROR((SGE_EVENT, MSG_EVENT_HIGHESTEVENTISXWHILEWAITINGFORY_UU , 
                     sge_u32c(j), sge_u32c(i)));
      }

      /* ensure number of first event is lower or equal "waiting_for" */
      if ((j=lGetUlong(lFirst(lp), ET_number)) > i) {
         /* error in event numbers */
         if (wrong_number) {
            *wrong_number = j;
         }   
         ERROR((SGE_EVENT, MSG_EVENT_SMALLESTEVENTXISGRTHYWAITFOR_UU,
                  sge_u32c(j), sge_u32c(i)));
         ret = false;
      } else {

         /* skip leading events till event number is "waiting_for"
            or there are no more events */ 
         skipped = 0;
         ep = lFirst(lp);
         while (ep && lGetUlong(ep, ET_number) < i) {
            tmp = lNext(ep);
            lRemoveElem(lp, &ep);
            ep = tmp;
            skipped++;
         }

         if (skipped) {
            DPRINTF(("Skipped %d events, still %d in list\n", skipped, 
                     lGetNumberOfElem(lp)));
         }

         /* ensure number of events increase */
         for_each (ep, lp) {
            if ((j=lGetUlong(ep, ET_number)) != i++) {
               /* 
                  do not change waiting_for because 
                  we still wait for this number 
               */
               ERROR((SGE_EVENT, SFNMAX, MSG_EVENT_EVENTSWITHNOINCREASINGNUMBERS));
               if (wrong_number) {
                  *wrong_number = j;
               }   
               ret = false;
               break;
            }
         }
         
         if (ret) {
            /* that's the new number we wait for */
            *waiting_for = i;
            DPRINTF(("check complete, %d events in list\n", 
                     lGetNumberOfElem(lp)));
         }
      }
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

/***** Eventclient/Client/get_event_list() ***********************************
*  NAME
*     get_event_list() -- get event list via gdi call
*
*  SYNOPSIS
*     static bool 
*     get_event_list(int sync, lList **report_list, int *commlib_error) 
*
*  FUNCTION
*     Tries to retrieve the event list.
*     Returns the incoming data and the commlib status/error code.
*     This function is used by ec_get.
*
*  INPUTS
*     int sync            - synchronous transfer
*     lList **report_list - pointer to returned list
*     int *commlib_error  - pointer to integer to return communication error
*
*  RESULT
*     static bool - true on success, else false
*
*  SEE ALSO
*     Eventclient/Client/ec_get()
******************************************************************************/
static bool get_event_list(sge_evc_class_t *thiz, int sync, lList **report_list, int *commlib_error )
{
   int tag;
   bool ret = true;
   sge_pack_buffer pb;
   int help;
   sge_gdi_ctx_class_t * sge_gdi_ctx = thiz->get_gdi_ctx(thiz);
   char rhost[CL_MAXHOSTLEN];
   char commproc[CL_MAXHOSTLEN];
   u_short id = 0;

   DENTER(EVC_LAYER, "get_event_list");

   PROF_START_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   /* TODO: check if all the functionality of get_event_list has been mapped */
   
   tag = TAG_REPORT_REQUEST;
   id = 1;

   DPRINTF(("try to get request from %s, id %d\n",(char*)prognames[QMASTER], id ));
   if ( (help=sge_gdi2_get_any_request(sge_gdi_ctx, rhost, commproc, &id, &pb, &tag, sync,0,NULL)) != CL_RETVAL_OK) {
      if (help == CL_RETVAL_NO_MESSAGE || help == CL_RETVAL_SYNC_RECEIVE_TIMEOUT) {
         DEBUG((SGE_EVENT, "commlib returns %s\n", cl_get_error_text(help)));
      } else {
         WARNING((SGE_EVENT, "commlib returns %s", cl_get_error_text(help)));
      }
      ret = false;
   } else {
      if (cull_unpack_list(&pb, report_list)) {
         ERROR((SGE_EVENT, SFNMAX, MSG_LIST_FAILEDINCULLUNPACKREPORT));
         ret = false;
      }
      clear_packbuffer(&pb);
   }
   if ( commlib_error != NULL) {
      *commlib_error = help;
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_EVENTCLIENT);

   DRETURN(ret);
}

bool 
sge_gdi2_evc_setup(sge_evc_class_t **evc_ref, sge_gdi_ctx_class_t *sge_gdi_ctx, 
                   ev_registration_id reg_id, lList **alpp, const char * name)
{
   sge_evc_class_t *evc = NULL;

   DENTER(EVC_LAYER, "sge_gdi2_evc_setup");

   if (evc_ref == NULL) {
      answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                              "%s", MSG_NULLPOINTER);
      DRETURN(false);
   }
   
   evc = sge_evc_class_create(sge_gdi_ctx, reg_id, alpp, name); 
   if (evc == NULL) {
      DRETURN(false);
   }

   *evc_ref = evc;
   
   DRETURN(true);
}

static ec_control_t *ec2_get_event_control(sge_evc_class_t *thiz) {
   ec_control_t *event_control = NULL;

   DENTER(EVC_LAYER, "ec2_get_event_control");
   if (thiz && thiz->ec_is_initialized(thiz)) {
      sge_gdi_ctx_class_t *gdi_ctx = thiz->get_gdi_ctx(thiz);
      if (gdi_ctx && gdi_ctx->is_qmaster_internal_client(gdi_ctx)) {
         sge_evc_t *sge_evc = (sge_evc_t*)thiz->sge_evc_handle;
         event_control = &(sge_evc->event_control);
      }
   }
   DRETURN(event_control);
}

static bool ec2_get_local(sge_evc_class_t *thiz, lList **elist, bool exit_on_qmaster_down) {
   lList *event_list = NULL;
   ec_control_t * evco = NULL;
   u_long32 current_time = 0; 
   struct timespec ts;
   dstring dsbuf;
   char buffer[1024];

   DENTER(EVC_LAYER, "ec2_get_local");

   sge_dstring_init(&dsbuf, buffer, sizeof(buffer));

   if (thiz == NULL) {
      DRETURN(false);
   }
   evco = ec2_get_event_control(thiz);
   if (evco == NULL) {
      DRETURN(false);
   }

   if (thiz->ec_need_new_registration(thiz)) {
      sge_evc_t *sge_evc = (sge_evc_t *) thiz->sge_evc_handle;
      sge_evc->next_event = 1;
      thiz->ec_register(thiz, exit_on_qmaster_down, NULL, NULL);
   }

   sge_mutex_lock("evco_event_thread_cond_mutex", SGE_FUNC, __LINE__, 
                  &(evco->mutex));

   current_time = sge_get_gmt();
   while (!evco->triggered && !evco->exit &&
          ((sge_get_gmt() - current_time) < EC_TIMEOUT_S)){
      ts.tv_sec = (long) current_time + EC_TIMEOUT_S;
      ts.tv_nsec = EC_TIMEOUT_N;
#ifdef EVC_DEBUG      
printf("EVENT_CLIENT %d beginning to wait at %s\n", thiz->ec_get_id(thiz), sge_ctime(0, &dsbuf));
#endif
      pthread_cond_timedwait(&(evco->cond_var),
                             &(evco->mutex), &ts);
#ifdef EVC_DEBUG      
printf("EVENT_CLIENT %d ends to wait at %s\n", thiz->ec_get_id(thiz), sge_ctime(0, &dsbuf));
#endif
   }

   /* taking out the new events */
   event_list = evco->new_events;
   evco->new_events = NULL;
   evco->triggered = false;

   DPRINTF(("EVENT_CLIENT id=%d TAKES FROM EVENT QUEUE at %s\n", thiz->ec_get_id(thiz), sge_ctime(0, &dsbuf)));

   sge_mutex_unlock("evco_event_thread_cond_mutex", SGE_FUNC, __LINE__, 
                    &(evco->mutex));

   thiz->ec_ack(thiz);
   thiz->ec_set_busy(thiz, 1);
   thiz->ec_commit(thiz, NULL);

   *elist = event_list;

   if (lGetElemUlong(event_list, ET_type, sgeE_ACK_TIMEOUT) != NULL) {
      ec2_mark4registration(thiz);
   }
   DRETURN(true);
}

static void ec2_wait_local(sge_evc_class_t *thiz) {

   DENTER(EVC_LAYER, "ec2_wait_local");
   /*
   ** reset busy, important otherwise no new events
   */
   thiz->ec_set_busy(thiz, 0);
   thiz->ec_commit(thiz, NULL);

   DRETURN_VOID;
}

static int ec2_signal_local(sge_evc_class_t *thiz, lList **alpp, lList *event_list) {

   ec_control_t *evco = NULL;
   int num_events = 0;

   DENTER(EVC_LAYER, "ec2_signal_local");

   if (thiz == NULL) {
      DPRINTF(("EVENT UPDATE FUNCTION thiz IS NULL\n"));
      DRETURN(-1); 
   }
   evco = ec2_get_event_control(thiz);
   if (evco == NULL) {
      DPRINTF(("EVENT UPDATE FUNCTION evco IS NULL\n"));
      DRETURN(-1);
   }

   if ((num_events = lGetNumberOfElem(lGetList(lFirst(event_list), REP_list))) > 0) {
      sge_mutex_lock("event_control_mutex", SGE_FUNC, __LINE__, &(evco->mutex));  
      if (evco->new_events != NULL) {
         lList *events = NULL;
         lXchgList(lFirst(event_list), REP_list, &(events));
         lAddList(evco->new_events, &events);
         events = NULL;
      } else {
         lXchgList(lFirst(event_list), REP_list, &(evco->new_events));
      }   
      
      evco->triggered = true;
      DPRINTF(("EVENT UPDATE FUNCTION jgdi_event_update_func() HAS BEEN TRIGGERED\n"));

      pthread_cond_broadcast(&(evco->cond_var));
#ifdef EVC_DEBUG      
{
dstring dsbuf;
char buf[1024];
sge_dstring_init(&dsbuf, buf, sizeof(buf));
printf("EVENT_CLIENT %d has been signaled at %s\n", thiz->ec_get_id(thiz), sge_ctime(0, &dsbuf));
}
#endif
      sge_mutex_unlock("event_control_mutex", SGE_FUNC, __LINE__, &(evco->mutex));
   }

   DRETURN(num_events);
}

static void ec2_wait(sge_evc_class_t *thiz) {
   /* do nothing */
}

static int ec2_signal(sge_evc_class_t *thiz, lList **alpp, lList *event_list) {
   /* do nothing */
   return 1;
}

static bool ec2_evco_triggered(sge_evc_class_t *thiz) {
   bool ret = false;
   ec_control_t *evco = NULL;

   DENTER(EVC_LAYER, "ec2_evco_triggered");
   if (thiz == NULL) {
      DRETURN(false); 
   }
   evco = ec2_get_event_control(thiz);
   if (evco == NULL) {
      DRETURN(false);
   }
   sge_mutex_lock("event_control_mutex", SGE_FUNC, __LINE__, &(evco->mutex));  
   ret = evco->triggered;
   sge_mutex_unlock("event_control_mutex", SGE_FUNC, __LINE__, &(evco->mutex));

   DRETURN(ret);
}

static bool ec2_evco_exit(sge_evc_class_t *thiz) {
   bool ret = false;
   ec_control_t *evco = NULL;

   DENTER(EVC_LAYER, "ec2_evco_exit");
   if (thiz == NULL) {
      DRETURN(false); 
   }
   evco = ec2_get_event_control(thiz);
   if (evco == NULL) {
      DRETURN(false);
   }
   sge_mutex_lock("event_control_mutex", SGE_FUNC, __LINE__, &(evco->mutex));  
   ret = evco->exit;
   sge_mutex_unlock("event_control_mutex", SGE_FUNC, __LINE__, &(evco->mutex));

   DRETURN(ret);
}

/****** Eventclient/event_text() **********************************************
*  NAME
*     event_text() -- deliver event description
*
*  SYNOPSIS
*     const char* event_text(const lListElem *event) 
*
*  FUNCTION
*     Deliveres a short description of an event object.
*
*  INPUTS
*     const lListElem *event - the event to describe
*
*  RESULT
*     const char* - pointer to the descriptive string.
*
*  NOTES
*     The result points to a static buffer. Subsequent calls to
*     event_text will overwrite previous results.
*******************************************************************************/
/* function see libs/sgeobj/sge_event.c */

