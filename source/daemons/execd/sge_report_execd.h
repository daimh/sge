#ifndef _REPORT_H
#define _REPORT_H
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

#include "cull.h"
#include "gdi/sge_gdi_ctx.h"

typedef int (*report_func_type)(sge_gdi_ctx_class_t *ctx, lList *, u_long32 now, u_long32 *next_send);

typedef struct report_source {
  int type;
  report_func_type func;
  u_long32 next_send;
} report_source;

int sge_send_all_reports(sge_gdi_ctx_class_t *ctx, u_long32 now, int which, report_source *report_sources);

int sge_add_double2load_report(lList **lpp, char *name, double value, const char *host, char *units);
int sge_add_int2load_report(lList **lpp, const char *name, int value, const char *host);
int sge_add_str2load_report(lList **lpp, const char *name, const char *value, const char *host);

#endif /* _REPORT_H */

