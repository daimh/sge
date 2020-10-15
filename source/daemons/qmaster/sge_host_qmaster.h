#ifndef __SGE_HOST_QMASTER_H
#define __SGE_HOST_QMASTER_H
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

#include "uti/sge_monitor.h"

#include "gdi/sge_gdiP.h"
#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi_packet.h"

#include "sgeobj/sge_feature.h"

#include "sge_c_gdi.h"
#include "sge_qmaster_timed_event.h"

/* funtions called via gdi and inside the qmaster */
int sge_del_host(sge_gdi_ctx_class_t *context, lListElem *, lList **, char *, char *, u_long32, const lList* master_hGroup_List);

int host_spool(sge_gdi_ctx_class_t *context, lList **alpp, lListElem *ep, gdi_object_t *object);

int host_mod(sge_gdi_ctx_class_t *context,
             lList **alpp, lListElem *new_host, lListElem *ep, int add, 
             const char *ruser, const char *rhost, gdi_object_t *object, 
             int sub_command, monitoring_t *monitor);

int host_success(sge_gdi_ctx_class_t *context, lListElem *ep, lListElem *old_ep, gdi_object_t *object, lList **ppList, monitoring_t *monitor);

void sge_mark_unheard(lListElem *hep);

int sge_add_host_of_type(sge_gdi_ctx_class_t *context, const char *hostname, u_long32 target, monitoring_t *monitor);

void sge_gdi_kill_exechost(sge_gdi_ctx_class_t *ctx,                            
                           sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task);

void sge_update_load_values(sge_gdi_ctx_class_t *context, const char *rhost, lList *lp);

void sge_load_value_cleanup_handler(sge_gdi_ctx_class_t *context, te_event_t anEvent, monitoring_t *monitor);

int sge_execd_startedup(sge_gdi_ctx_class_t *context, 
                        lListElem *hep, lList **alpp, char *ruser, char *rhost, 
                        u_long32 target, monitoring_t *monitor, bool is_restart);

u_long32 load_report_interval(lListElem *hep); 

bool host_list_add_missing_href(sge_gdi_ctx_class_t *context,
                                lList *this_list, 
                                lList **answer_list, 
                                const lList *href_list, 
                                monitoring_t *monitor);

void host_diff_projects(const lListElem *new, const lListElem *old, lList **new_prj, lList **old_prj);
void host_diff_usersets(const lListElem *new, const lListElem *old, lList **new_acl, lList **old_acl);


void        
host_initalitze_timer(void);

#endif /* __SGE_HOST_QMASTER_H */

