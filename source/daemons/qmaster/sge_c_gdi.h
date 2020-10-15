#ifndef __SGE_C_GDI_H
#define __SGE_C_GDI_H
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



#ifndef __SGE_GDI_INTERN_H
#   include "sge_gdiP.h"
#endif

#include "uti/sge_monitor.h"

#include "cull/cull.h"

#include "sgeobj/sge_object.h"

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi_packet.h"

typedef struct _gdi_object_t gdi_object_t;

typedef int (*modifier_func_t)(
   sge_gdi_ctx_class_t *ctx,
   lList **alpp,
   lListElem *new_cal,   /* destination */
   lListElem *cep,       /* reduced element */
   int add,              /* 1 for add/0 for mod */
   const char *ruser,
   const char *rhost,
   gdi_object_t *object, /* some kind of "this" */
   int sub_command,
   monitoring_t *monitor
);

typedef int (*writer_func_t)(
   sge_gdi_ctx_class_t *ctx,
   lList **alpp,
   lListElem *ep,      /* new modified element */
   gdi_object_t *this   /* some kind of "this" */
);

/* allows to retrieve a master list */
typedef lList ** (*getMasterList)(void);

typedef int (*on_success_func_t)(
   sge_gdi_ctx_class_t *ctx,
   lListElem *ep,       /* new modified and already spooled element */
   lListElem *old_ep,   /* old element is NULL in add case */
   gdi_object_t *this,  /* some kind of "this" */
   lList **ppList,       /* a list to pass back information for post processing */
   monitoring_t *monitor  /* monitoring structure */
);

struct _gdi_object_t {
   u_long32           target;          /* SGE_QUEUE_LIST */
   int                key_nm;          /* QU_qname */
   lDescr             *type;           /* QU_Type */
   char               *object_name;    /* "queue" */
   sge_object_type    list_type;       /* identifier to retrive the master list via object_type_get_master_list*/
   modifier_func_t    modifier;        /* responsible for validating each our attribute modifier */
   writer_func_t      writer;          /* function that spools our object */
   on_success_func_t  on_success;      /* do everything what has to be done on successful writing */
};

gdi_object_t *get_gdi_object(u_long32);

void
sge_c_gdi(sge_gdi_ctx_class_t *ctx, sge_gdi_packet_class_t *packet,
          sge_gdi_task_class_t *task, lList **answer_list, monitoring_t *monitor);

int
sge_gdi_add_mod_generic(sge_gdi_ctx_class_t *ctx,
                        lList **alpp, lListElem *instructions, int add,
                        gdi_object_t *object, const char *ruser,
                        const char *rhost, int sub_command, lList **ppList,
                        monitoring_t *monitor);

void sge_clean_lists(void);

/* EB: TODO: CLEANUP: should be replaced with sge_gdi_packet_verify_version() */
int verify_request_version(lList **alpp, u_long32 version, char *host,
                           char *commproc, int id);

int sge_chck_mod_perm_host(lList **alpp, u_long32 target, char *host,
                                  char *commproc, int mod, lListElem *ep,
                                  monitoring_t *monitor, object_description *object_base);

#endif /* __SGE_C_GDI_H */

