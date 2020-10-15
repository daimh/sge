#ifndef __SGE_QRSTAT_FILTER_H
#define __SGE_QRSTAT_FILTER_H
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
      
#ifdef  __cplusplus
extern "C" {
#endif

#include "basis_types.h"
#include "gdi/sge_gdi_ctx.h"

typedef struct qrstat_env_str qrstat_env_t;

struct qrstat_env_str {
   sge_gdi_ctx_class_t *ctx;

   /* input parameters */
   lList* user_list;  /* -u user_list */
   lList* ar_id_list; /* -ar ar_id */
   bool is_explain;   /* -explain */
   bool is_xml;       /* -xml */
   bool is_summary;   /* show summary of selected ar's or all details of one or multiple ar's */
   bool header_printed;

   /* needed lists */
   lList *ar_list;

   lEnumeration *what_AR_Type;
   lCondition *where_AR_Type;
};

void
qrstat_filter_init(qrstat_env_t *qrstat_env);

void
qrstat_filter_add_core_attributes(qrstat_env_t *qrstat_env);


void
qrstat_filter_add_ar_attributes(qrstat_env_t *qrstat_env);

void
qrstat_filter_add_xml_attributes(qrstat_env_t *qrstat_env);

void
qrstat_filter_add_explain_attributes(qrstat_env_t *qrstat_env);

void
qrstat_filter_add_u_where(qrstat_env_t *qrstat_env);

void
qrstat_filter_add_ar_where(qrstat_env_t *qrstat_env);

void
qrstat_filter_set_ctx(qrstat_env_t *qrstat_env, sge_gdi_ctx_class_t *ctx);

#ifdef  __cplusplus
}
#endif
#endif /* __SGE_QRSTAT_FILTER_H */
