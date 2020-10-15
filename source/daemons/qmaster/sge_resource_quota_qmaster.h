#ifndef __SGE_RESOURCE_QUOTA_QMASTER_H
#define __SGE_RESOURCE_QUOTA_QMASTER_H
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

#include "sge_c_gdi.h"
#include "uti/sge_monitor.h"
#include "gdi/sge_gdi_ctx.h"

/* funtions called from within gdi framework in qmaster */
int rqs_mod(sge_gdi_ctx_class_t *ctx,
            lList **alpp, lListElem *new_rqs, lListElem *rqs, int add, const char *ruser, 
            const char *rhost, gdi_object_t *object, int sub_command, monitoring_t *monitor);

int rqs_spool(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *pep, gdi_object_t *object);

int rqs_success(sge_gdi_ctx_class_t *ctx, lListElem *ep, lListElem *old_ep, gdi_object_t *object, lList **ppList, monitoring_t *monitor);

/* funtions called via gdi and inside the qmaster */
int rqs_del(sge_gdi_ctx_class_t *ctx, lListElem *ep, lList **alpp, lList **rqs_list, 
                char *ruser, char *rhost);

bool rqs_diff_usersets(const lListElem *new_rqs, const lListElem *old_rqs, lList **new_acl,
                       lList **old_acl, lList *master_userset_list);

bool rqs_diff_projects(const lListElem *new_rqs, const lListElem *old_rqs, lList **new_acl,
                       lList **old_acl, lList *master_project_list);

bool scope_is_referenced_rqs(const lListElem *rqs, int filter_type, const char *name);

#endif
