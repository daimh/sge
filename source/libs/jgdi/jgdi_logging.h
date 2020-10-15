#ifndef JGDI_LOGGING_H
#define JGDI_LOGGING_H

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

#include "sge.h"                /* for __attribute__ */

typedef enum {
   SEVERE = 0,
   WARNING,
   INFO,
   CONFIG,
   FINE,
   FINER,
   FINEST,
   LOG_LEVEL_COUNT
} log_level_t;

#define JGDI_LOGGER        "com.sun.grid.jgdi.JGDI"
#define JGDI_QSTAT_LOGGER  "com.sun.grid.jgdi.monitoring.qstat"
#define JGDI_QHOST_LOGGER  "com.sun.grid.jgdi.monitoring.qhost"
#define JGDI_EVENT_LOGGER  "com.sun.grid.jgdi.event"

jobject jgdi_get_logger(JNIEnv *env, const char* logger);
jboolean jgdi_is_loggable(JNIEnv *env, jobject logger, log_level_t level);
void jgdi_log_printf(JNIEnv *env, const char* logger, log_level_t level, const char* fmt, ...) __attribute__ ((format (printf, 4, 5)));
void jgdi_log(JNIEnv *env, jobject logger, log_level_t level, const char* msg);
void jgdi_log_list(JNIEnv *env, const char* logger, log_level_t level, lList* list);
void jgdi_log_listelem(JNIEnv *env, const char* logger, log_level_t level, lListElem *elem);
void jgdi_log_answer_list(JNIEnv *env, const char* logger, lList *alp);

int jgdi_init_rmon_ctx(JNIEnv *env, const char* logger, rmon_ctx_t *ctx);
void jgdi_destroy_rmon_ctx(rmon_ctx_t *rmon_ctx);

#endif
