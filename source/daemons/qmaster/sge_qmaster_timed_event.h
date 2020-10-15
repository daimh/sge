#ifndef __SGE_QMASTER_TIMED_EVENT_H__
#define __SGE_QMASTER_TIMED_EVENT_H__
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <time.h>
#include <sys/types.h>

#include "basis_types.h"
#include "uti/sge_monitor.h"
#include "gdi/sge_gdi_ctx.h"

enum { TBL_GROW_FACTOR = 2, TBL_INIT_SIZE = 10 };

typedef enum {
   TYPE_CALENDAR_EVENT = 4,  /**/
   TYPE_SIGNAL_RESEND_EVENT,  /**/
   TYPE_JOB_RESEND_EVENT, /**/
   TYPE_RESCHEDULE_UNKNOWN_EVENT,/**/
   TYPE_SPOOLING_TRIGGER,/**/
   TYPE_REPORTING_TRIGGER,/**/
   TYPE_SHARELOG_TRIGGER,/**/
   TYPE_HEARTBEAT_EVENT,/**/
   TYPE_LOAD_VALUE_CLEANUP_EVENT,/**/
   TYPE_ZOMBIE_JOB_CLEANUP_EVENT,/**/
   TYPE_AUTOMATIC_USER_CLEANUP_EVENT, /**/
   TYPE_REMOTE_EVENT_DELIVERY_EVENT, /* not used */
   TYPE_SECURITY_EVENT, /**/
   TYPE_JOB_NUMBER_EVENT,/**/
   TYPE_ACCOUNTING_TRIGGER,/**/
   TYPE_AR_ID_EVENT,/**/
   TYPE_AR_EVENT, /**/
   TYPE_ENFORCE_LIMIT_EVENT, /**/

   /* EB: TODO: ST: can be removed after full scheduler is part of master process */
   TYPE_ORDER_PROCESSING_EVENT
} te_type_t;

typedef enum {
   ONE_TIME_EVENT = 1,
   RECURRING_EVENT
} te_mode_t;

struct te_event {
   time_t      when;        /* event delivery time                */
   time_t      interval;    /* event interval, if recurring event */
   te_type_t   type;        /* event type                         */
   te_mode_t   mode;        /* event mode                         */
   u_long32    ulong_key_1; /* 1st numeric key                    */
   u_long32    ulong_key_2; /* 2nd numeric key                    */
   const char* str_key;     /* alphanumeric key                   */
   u_long32    seq_no;      /* event sequence number              */
};

typedef struct {
   pthread_mutex_t mutex;      /* used for mutual exclusion                         */
   pthread_cond_t  cond_var;   /* used for waiting                                  */
   bool            exit;       /* true -> exit event delivery                       */
   bool            delete;     /* true -> at least one event has been deleted       */
   lList*          list;       /* timed event list                                  */
   lSortOrder*     sort_order; /* list sort order                                   */
   u_long32        seq_no;     /* last added timed event sequence number            */
   time_t          last;       /* last time, event delivery has been checked        */
   time_t          next;       /* due date for next event, 0 -> event list is empty */
} event_control_t;

typedef struct te_event* te_event_t;

typedef void (*te_handler_t)(sge_gdi_ctx_class_t *context, te_event_t, monitoring_t *monitor);

struct tbl_elem {
   te_type_t    type;    /* event type    */
   te_handler_t handler; /* event handler */
};

typedef struct {
   pthread_mutex_t  mutex; /* used for mutual exclusion            */
   int              num;   /* number of event handler in table     */
   int              max;   /* max number of handler, before resize */
   struct tbl_elem* list;  /* event handler list                   */
} handler_tbl_t;

extern handler_tbl_t Handler_Tbl;

extern event_control_t Event_Control;

/* internal interface functions */

void       te_check_time(time_t time);
void       te_wait_empty(void);
te_event_t te_event_from_list_elem(lListElem* aListElem);
void       te_wait_next(te_event_t te, time_t now);
void       te_scan_table_and_deliver(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor);
void       te_init(void);

/* user interface */

extern void       te_register_event_handler(te_handler_t, te_type_t);
extern te_event_t te_new_event(time_t, te_type_t, te_mode_t, u_long32, u_long32, const char*);
extern void       te_free_event(te_event_t*);
extern void       te_add_event(te_event_t);
extern int        te_delete_one_time_event(te_type_t, u_long32, u_long32, const char*);     
extern int        te_delete_all_one_time_events(te_type_t aType);
extern void       te_shutdown(void);

extern time_t      te_get_when(te_event_t);
extern te_type_t   te_get_type(te_event_t);
extern te_mode_t   te_get_mode(te_event_t);
extern u_long32    te_get_first_numeric_key(te_event_t);
extern u_long32    te_get_second_numeric_key(te_event_t);
extern char*       te_get_alphanumeric_key(te_event_t);
extern u_long32    te_get_sequence_number(te_event_t);

#endif /* __SGE_QMASTER_TIMED_EVENT_H__ */

