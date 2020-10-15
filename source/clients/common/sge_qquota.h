#ifndef __SGE_QQUOTA_H
#define __SGE_QQUOTA_H
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

#include "cull/cull.h"
#include "gdi/sge_gdi_ctx.h"

typedef struct report_handler_str report_handler_t;

#define QQUOTA_SUCCESS 0
#define QQUOTA_ERROR   -1

struct report_handler_str {
   void* ctx;
   int (*report_started)(report_handler_t* handler, lList **alpp);
   int (*report_finished)(report_handler_t* handler, lList **alpp);
   int (*report_limit_rule_begin)(report_handler_t* handler, const char *limit_name, lList **alpp);
   int (*report_limit_string_value)(report_handler_t* handler, const char*name, const char *value, bool exclude, lList **alpp);
   int (*report_limit_rule_finished)(report_handler_t* handler, const char *limit_name, lList **alpp);

   int (*report_resource_value)(report_handler_t* handler, const char* resource, const char* limit, const char*value, lList **alpp);
   int (*destroy)(report_handler_t** handler, lList **alpp);
};

bool qquota_output(sge_gdi_ctx_class_t *ctx, lList *host_list, lList *resource_match_list, lList *user_list,
                lList *pe_list, lList *project_list, lList *cqueue_list, lList **alpp,
                report_handler_t* report_handler);

#ifdef  __cplusplus
}
#endif

#endif


