#ifndef __SGE_GDI_H
#define __SGE_GDI_H
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

/* may be this should be included by the gdi user */
#include "cull/cull.h"
#include "uti/sge_hostname.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* 
 * allowed values for command field of a gdi request 
 * (see sge_gdi_packet_class_t and sge_gdi_task_class_t
 */
enum { 
   /* OPERATION -------------- */
   SGE_GDI_GET = 1, 
   SGE_GDI_ADD, 
   SGE_GDI_DEL, 
   SGE_GDI_MOD,
   SGE_GDI_TRIGGER,
   SGE_GDI_PERMCHECK,
   SGE_GDI_SPECIAL,
   SGE_GDI_COPY,
   SGE_GDI_REPLACE,

   /* SUB COMMAND  ----------- */

   /* for SGE_JB_LIST => SGE_GDI_ADD */
   SGE_GDI_RETURN_NEW_VERSION = (1<<8),

   /* for SGE_JB_LIST => SGE_GDI_DEL, SGE_GDI_MOD */
   SGE_GDI_ALL_JOBS  = (1<<9),
   SGE_GDI_ALL_USERS = (1<<10),

   /* for SGE_QUEUE_LIST, SGE_EH_LIST => SGE_GDI_MOD */
   SGE_GDI_SET     = 0,        /* overwrite the sublist with given values */
   SGE_GDI_CHANGE  = (1<<11),  /* change the given elements */
   SGE_GDI_APPEND  = (1<<12),  /* add some elements into a sublist */
   SGE_GDI_REMOVE  = (1<<13),  /* remove some elements from a sublist */
   SGE_GDI_SET_ALL = (1<<14),  /* 
                                * overwrite the sublist with given values
                                * and erase all domain/host specific values
                                * not given with the current request
                                */
   SGE_GDI_EXECD_RESTART = (1<<15)
};

#define SGE_GDI_OPERATION (0xFF)
#define SGE_GDI_SUBCOMMAND (~SGE_GDI_OPERATION)

#define SGE_GDI_GET_OPERATION(x) ((x)&SGE_GDI_OPERATION)
#define SGE_GDI_GET_SUBCOMMAND(x) ((x)&SGE_GDI_SUBCOMMAND)
#define SGE_GDI_IS_SUBCOMMAND_SET(x,y) ((x)&(y))

/* 
 * allowed values for target field of GDI request 
 * (see sge_gdi_packet_class_t and sge_gdi_task_class_t)
 */
enum {
   SGE_AH_LIST = 1,
   SGE_SH_LIST,
   SGE_EH_LIST,
   SGE_CQ_LIST,
   SGE_JB_LIST,
   SGE_EV_LIST,
   SGE_CE_LIST,
   SGE_ORDER_LIST,
   SGE_MASTER_EVENT,
   SGE_CONF_LIST,
   SGE_UM_LIST,                 /* manager */
   SGE_UO_LIST,                 /* operator */
   SGE_PE_LIST,
   SGE_SC_LIST,                 /* schedconf list */
   SGE_UU_LIST,                 /* user */
   SGE_US_LIST,                 /* userset */
   SGE_PR_LIST,
   SGE_STN_LIST,                /* share tree */
   SGE_CK_LIST,
   SGE_CAL_LIST,
   SGE_SME_LIST,                /* scheduler message */
   SGE_ZOMBIE_LIST,
   SGE_USER_MAPPING_LIST,
   SGE_HGRP_LIST,
   SGE_RQS_LIST,
   SGE_AR_LIST,
   SGE_DUMMY_LIST
};

/*
** Special target numbers for ´complex requests´ which shall be handled
** directly at the master in order to reduce network traffic and replication
** of master lists. They are only useful for SGE_GDI_GET requests
** ATTENTION: the numbers below must not collide with the numbers in the enum
**            directly above
*/
enum {
   SGE_QSTAT = 1024,
   SGE_QHOST
};

/* sge_gdi_multi enums */
enum {
   SGE_GDI_RECORD,
   SGE_GDI_SEND,
   SGE_GDI_SHOW
};

/* preserves state between multiple calls to sge_gdi_multi() */
typedef struct _state_gdi_multi state_gdi_multi;

struct _state_gdi_multi {
   void *packet; /* TODO: EB: this is a sge_gdi_packet_class_t pointer - fix includes */
   lList **malpp;
   state_gdi_multi *next;
};

/* to be used for initializing state_gdi_multi */
#define STATE_GDI_MULTI_INIT { NULL, NULL, NULL}

bool sge_gdi_extract_answer(lList **alpp, u_long32 cmd, u_long32 target, int id, lList *mal, lList **olpp);

/* from gdi_checkpermissions.h */
#define MANAGER_CHECK     (1<<0)
#define OPERATOR_CHECK    (1<<1)
/*
#define USER_CHECK        (1<<2)
#define SGE_USER_CHECK    (1<<3)
*/

/* from gdi_setup.h */
/* these values are standarized gdi return values */
enum {
   AE_ERROR = -1,
   AE_OK = 0,
   AE_ALREADY_SETUP,
   AE_UNKNOWN_PARAM,
   AE_QMASTER_DOWN
};

/* from gdi_tsm.h */
#define MASTER_KILL       (1<<0)
#define SCHEDD_KILL       (1<<1)
#define EXECD_KILL        (1<<2)
#define JOB_KILL          (1<<3)
#define EVENTCLIENT_KILL  (1<<4)
#define THREAD_START      (1<<5)

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_GDI_H */
