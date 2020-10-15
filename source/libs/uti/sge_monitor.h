#ifndef _SGE_MONITOR_H
#define _SGE_MONITOR_H

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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <sys/time.h>

#include "basis_types.h"
#include "uti/sge_dstring.h"

/**
 * Monitoring functionality:
 * -------------------------
 *
 * - qping health monitoring
 *
 * - keeping statistics on what is done during a thread loop
 *
 * - outputting the statistics information via message file or
 *   qping
 *
 *
 * Monitoring Usage:
 * -----------------
 *
 * do a normal data definition, call init and free, when you are done.
 * You have to call MONITOR_IDLE_TIME and sge_monitor_output. After that
 * everything is up to you to design...
 *
 * -----start thread --------------
 *    monitoring_t monitor;
 *   
 *    sge_monitor_init(&monitor, "THREAD NAME", <EXTENSION>, <WARNING>, <ERROR>);
 *   
 *    <thread loop> {
 *       
 *       MONITOR_IDLE_TIME(<wait for something>,(&monitor), monitor_time, monitor_message_p);
 *   
 *      < do your stuff and monitoring >
 *   
 *       sge_monitor_output(&monitor);
 *    }
 *    sge_monitor_free(&monitor);
 * ------end thread----------------
 *
 * Important:
 * ----------
 *  The call to MONITOR_IDLE_TIME has to be the first one after the thread loop otherwise
 *  certain parts of the monitoring structure are not correctly initialized.
 *
 * General statistic methods:
 * --------------------------
 *
 * - MONITOR_IDLE_TIME    : counts idle time, very important, nothing works without it
 * - MONITOR_WAIT_TIME    : counts wait time (wait for a lock usually)
 * - MONITOR_MESSAGES     : counts how many times the thread loop is executed
 * - MONITOR_MESSAGES_OUT : counts how many messages are send
 *
 * GDI statistics methods:
 * -----------------------
 *
 * - MONITOR_GDI  : counts GDI requests
 * - MONITOR_ACK  : counts ACKs
 * - MONITOR_LOAD : counts reports
 */


/**
 * qping thread warning times in seconds
 */
typedef enum {
   NO_WARNING    = 0,
   EMT_WARNING   = 10,
   TET_WARNING   = 30,
   MT_WARNING    = 0,  /* no timeout for this thread */
   ST_WARNING    = 0,  /* no timeout for this thread */
   EXECD_WARNING = 10,
   SCT_WARNING   = 20
}thread_warning_t;

/* EB: TODO: ST: ??? */

/**
 * qping thread error times in seconds
 **/
#undef NO_ERROR /* defined on Cygwin by headers pulled in by hwloc.h */
typedef enum {
   NO_ERROR    = 0,
   EMT_ERROR   = 600,
   TET_ERROR   = 600,
   MT_ERROR    = 0,   /* no timeout for this thread */
   ST_ERROR    = 0,   /* no timeout for this thread */
   EXECD_ERROR = 600,
   SCT_ERROR   = 600   
}thread_error_t;

/**
 * This function definition is the prototype for the output function of a data
 * extension
 */
typedef void (*extension_output)(
   dstring *info_message,    /* target memory buffer*/
   void *monitor_extension,  /* contains the monitor extension structure */
   double time               /* length of the time inteval */
);

/**
 * This enum identifies all available extensions
 */
typedef enum {
   NONE_EXT = -1,
   GDI_EXT = 0,         /* GDI = request processing thread */
   EDT_EXT = 1,         /* EDT = event delivery thread */
   TET_EXT = 2,         /* TET = timed event thread */
   LIS_EXT = 3,         /* LIS = listener thread */
   SCH_EXT = 4          /* SCH = scheduler thread */
}extension_t;

/**
 * the monitoring data structure
 */
typedef struct {
   /*--- init data ------------*/
   const char *thread_name;
   u_long32    monitor_time;        /* stores the time interval for the mesuring run */
   bool        log_monitor_mes;     /* if true, it logs the monitoring info into the message file */
   /*--- output data ----------*/
   dstring *output_line1;
   dstring *output_line2;
   dstring *work_line;
   int  pos;                        /* position (line) in the qping output structure (kind of thread id) */
   /*--- work data ------------*/
   struct timeval now;              /* start time of mesurement */
   bool        output;              /* if true, triggers qping / message output */
   u_long32    message_in_count;
   u_long32    message_out_count;
   double      idle;                /* idle time*/
   double      wait;                /* wait time*/
   /*--- extension data -------*/
   extension_t       ext_type; 
   void             *ext_data;
   u_long32          ext_data_size;
   extension_output  ext_output; 
} monitoring_t;

void sge_monitor_init(monitoring_t *monitor, const char *thread_name, extension_t ext, 
                 thread_warning_t warning_timeout, thread_error_t error_timeout);
void sge_monitor_free(monitoring_t *monitor);
u_long32 sge_monitor_status(char **info_message, u_long32 monitor_time);
void sge_set_last_wait_time(monitoring_t *monitor, struct timeval after); 

void sge_monitor_output(monitoring_t *monitor); 
void sge_monitor_reset(monitoring_t *monitor);


/****************
 * MACRO section
 ****************/

#define MONITOR_IDLE_TIME(execute, monitor, output_time, is_log)    { \
                                 struct timeval before;  \
                                 gettimeofday(&before, NULL); \
                                 sge_set_last_wait_time((monitor), before); \
                                 if (output_time > 0) { \
                                    struct timeval before;  \
                                    struct timeval after; \
                                    double time; \
                                    \
                                    (monitor)->monitor_time = output_time; \
                                    (monitor)->log_monitor_mes = is_log; \
                                    gettimeofday(&before, NULL); \
                                    if ((monitor)->now.tv_sec == 0) { \
                                       (monitor)->now = before; \
                                    } \
                                    execute; \
                                    gettimeofday(&after, NULL);  \
                                    (monitor)->output = ((after.tv_sec-(monitor)->now.tv_sec) >= (monitor)->monitor_time)?true:false; \
                                    time = after.tv_usec - before.tv_usec; \
                                    time = after.tv_sec - before.tv_sec + (time/1000000); \
                                    (monitor)->idle += time; \
                                 } \
                                 else { \
                                    execute; \
                                 } \
                              } \

/**
 * This might pose a problem if it is called with another makro. 
 *
 * TODO: it should be customized for read/write locks.
 */
#define MONITOR_WAIT_TIME(execute, monitor)    if (((monitor) != NULL) && ((monitor)->monitor_time > 0)){ \
                                    struct timeval before;  \
                                    struct timeval after; \
                                    double time; \
                                    \
                                    gettimeofday(&before, NULL); \
                                    execute; \
                                    gettimeofday(&after, NULL);  \
                                    time = after.tv_usec - before.tv_usec; \
                                    time = after.tv_sec - before.tv_sec + (time/1000000); \
                                    (monitor)->wait += time; \
                                 } \
                                 else { \
                                    execute; \
                                 } \

#define MONITOR_MESSAGES(monitor) if (((monitor) != NULL) && ((monitor)->monitor_time > 0)) (monitor)->message_in_count++

#define MONITOR_MESSAGES_OUT(monitor) if (((monitor) != NULL) && ((monitor)->monitor_time > 0)) (monitor)->message_out_count++

/*--------------------------------*/
/*   EXTENSION SECTION            */
/*--------------------------------*/

/**
 * What you need to do to create a new extension:
 *
 * - create a new extension_t in the enum
 * - define a extension data structure
 * - modifiy the sge_monitor_init method to handle the new extension type
 *   Example:
 *     case GDI_EXT :
 *          monitor->ext_data_size = sizeof(m_gdi_t);
 *          monitor->ext_data = malloc(sizeof(m_gdi_t));
 *          monitor->ext_output = &ext_gdi_output;
 *       break;
 *
 * - write the extension output function
 * - write the measurement makros
 * - remember, that the entire extension structure is reset to 0 after the data is printed
 *
 **/


/* scheduler thread extensions */

typedef struct {
   u_long32    dummy;    /* unused */
} m_sch_t;

/* GDI message thread extensions */

typedef struct {
   u_long32    gdi_add_count;    /* counts the gdi add requests */
   u_long32    gdi_mod_count;    /* counts the gdi mod requests */
   u_long32    gdi_get_count;    /* counts the gdi get requests */
   u_long32    gdi_del_count;    /* counts teh gdi del requests */
   u_long32    gdi_cp_count;     /* counts the gdi cp requests */
   u_long32    gdi_trig_count;   /* counts the gdi trig requests */
   u_long32    gdi_perm_count;   /* counts the gdi perm requests */
   u_long32    gdi_replace_count;   /* counts the gdi perm requests */

   u_long32    eload_count; /* counts the execd load reports */
   u_long32    econf_count; /* counts the execd conf version requests */
   u_long32    ejob_count;  /* counts the execd job reports */
   u_long32    eproc_count; /* counts the execd processor reports */
   u_long32    eack_count;  /* counts the execd acks */

   u_long32    queue_length;     /* worker queue length */
} m_gdi_t;

#define MONITOR_GDI_ADD(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->gdi_add_count++
#define MONITOR_GDI_GET(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->gdi_get_count++
#define MONITOR_GDI_MOD(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->gdi_mod_count++
#define MONITOR_GDI_DEL(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->gdi_del_count++
#define MONITOR_GDI_CP(monitor)     if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->gdi_cp_count++
#define MONITOR_GDI_TRIG(monitor)   if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->gdi_trig_count++
#define MONITOR_GDI_PERM(monitor)   if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->gdi_perm_count++
#define MONITOR_GDI_REPLACE(monitor) if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->gdi_replace_count++

#define MONITOR_ACK(monitor)     if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->ack_count++

#define MONITOR_ELOAD(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->eload_count++
#define MONITOR_ECONF(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->econf_count++
#define MONITOR_EJOB(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->ejob_count++
#define MONITOR_EPROC(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->eproc_count++
#define MONITOR_EACK(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->eack_count++

#define MONITOR_SET_QLEN(monitor, qlen)    if ((monitor) != NULL && ((monitor)->monitor_time > 0) && ((monitor)->ext_type == GDI_EXT)) ((m_gdi_t*)((monitor)->ext_data))->queue_length = (qlen)

/* listener extension */
typedef struct {
   u_long32    inc_gdi; /* incoming GDI requests */
   u_long32    inc_ack; /* ack requests */
   u_long32    inc_ece; /* event client exits */
   u_long32    inc_rep; /* report request */
} m_lis_t;

#define MONITOR_INC_GDI(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == LIS_EXT)) ((m_lis_t*)((monitor)->ext_data))->inc_gdi++
#define MONITOR_INC_ACK(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == LIS_EXT)) ((m_lis_t*)((monitor)->ext_data))->inc_ack++
#define MONITOR_INC_ECE(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == LIS_EXT)) ((m_lis_t*)((monitor)->ext_data))->inc_ece++
#define MONITOR_INC_REP(monitor)    if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == LIS_EXT)) ((m_lis_t*)((monitor)->ext_data))->inc_rep++

/* event master thread extension */

typedef struct {
   u_long32   count;                /* counts the number of runs */
   u_long32   client_count;         /* connected event clients */
   u_long32   mod_client_count;     /* event client modifications */
   u_long32   ack_count;            /* nr of acknowledges */
   u_long32   new_event_count;      /* newly added events */
   u_long32   added_event_count;    /* nr of events added to the event clients */
   u_long32   skip_event_count;     /* nr of events ignored, no client has a subscription */
   u_long32   blocked_client_count; /* nr of event clients blocked during send */
   u_long32   busy_client_count;    /* nr of event clients busy during send */
}m_edt_t;

#define MONITOR_CLIENT_COUNT(monitor, inc)  if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                               ((m_edt_t*) ((monitor)->ext_data))->client_count += inc

#define MONITOR_EDT_COUNT(monitor) if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                    ((m_edt_t*) ((monitor)->ext_data))->count++

#define MONITOR_EDT_MOD(monitor) if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                    ((m_edt_t*) ((monitor)->ext_data))->mod_client_count++

#define MONITOR_EDT_ACK(monitor) if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                    ((m_edt_t*)((monitor)->ext_data))->ack_count++

#define MONITOR_EDT_NEW(monitor) if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                    ((m_edt_t*)((monitor)->ext_data))->new_event_count++

#define MONITOR_EDT_ADDED(monitor) if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                    ((m_edt_t*)((monitor)->ext_data))->added_event_count++

#define MONITOR_EDT_SKIP(monitor) if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                    ((m_edt_t*)((monitor)->ext_data))->skip_event_count++

#define MONITOR_EDT_BLOCKED(monitor)  if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                    ((m_edt_t*)((monitor)->ext_data))->blocked_client_count++

#define MONITOR_EDT_BUSY(monitor)  if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == EDT_EXT)) \
                                    ((m_edt_t*)((monitor)->ext_data))->busy_client_count++

/* timed event thread extension */

typedef struct {
   u_long32   count;         /* counts the number of runs */
   u_long32   event_count;   /* nr of pending events */
   u_long32   exec_count;    /* nr of executed events */
}m_tet_t;

#define MONITOR_TET_COUNT(monitor)  if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == TET_EXT)) \
                                    ((m_tet_t*)((monitor)->ext_data))->count++

#define MONITOR_TET_EVENT(monitor, inc)  if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == TET_EXT)) \
                                    ((m_tet_t*)((monitor)->ext_data))->event_count += inc

#define MONITOR_TET_EXEC(monitor)  if (((monitor)->monitor_time > 0) && ((monitor)->ext_type == TET_EXT)) \
                                    ((m_tet_t*)((monitor)->ext_data))->exec_count++


#endif /* _SGE_MONITIR_H */
