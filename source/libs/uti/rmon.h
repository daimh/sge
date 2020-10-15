#ifndef __RMON_H
#define __RMON_H
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

#include <stdarg.h>
#include <sys/types.h>

#include "uti/sge_rmon_monitoring_level.h"

#include "comm/cl_commlib.h"
#include "sge.h"                /* for __attribute__ */

typedef void              (*rmon_print_callback_func_t) (const char *message, unsigned long traceid, unsigned long pid, unsigned long thread_id);

extern monitoring_level RMON_DEBUG_ON;

#define PRATTR(idx, first) __attribute__ ((format (printf, idx, first)))

int  rmon_condition(int layer, int debug_class);
int  rmon_is_enabled(void);
void rmon_mopen(int *argc, char *argv[], char *programname);
void rmon_menter(const char *func, const char *thread_name);
void rmon_mtrace(const char *func, const char *file, int line, const char *thread_name);
void rmon_mprintf(int debug_class, const char *fmt, ...) PRATTR(2, 3);
void rmon_mexit(const char *func, const char *file, int line, const char *thread_name);
void rmon_debug_client_callback(int dc_connected, int debug_level);
void rmon_set_print_callback(rmon_print_callback_func_t function_p);

void rmon_mprintf_lock(const char* fmt, ...) PRATTR(1, 2);
void rmon_mprintf_info(const char* fmt, ...) PRATTR(1, 2);
void rmon_mprintf_timing(const char* fmt, ...) PRATTR(1, 2);
/* void rmon_mprintf_special(const char* fmt, ...); */

typedef struct rmon_ctx_str rmon_ctx_t;

struct rmon_ctx_str {
  void *ctx;
  
  int  (*is_loggable)(rmon_ctx_t *ctx, int layer, int debug_class);
  void (*menter)(rmon_ctx_t *ctx, const char* func);
  void (*mexit)(rmon_ctx_t *ctx, const char* func, const char *file, int line);
  void (*mtrace)(rmon_ctx_t *ctx, const char *func, const char *file, int line);
  void (*mprintf)(rmon_ctx_t *ctx, int debug_class, const char* fmt, va_list args);
};

typedef struct rmon_helper_str rmon_helper_t;

struct rmon_helper_str {
   char thread_name[32];
};

void rmon_set_thread_ctx(rmon_ctx_t* ctx);
rmon_ctx_t* rmon_get_thread_ctx(void);

rmon_helper_t *rmon_get_helper(void);

#define __CONDITION(x) rmon_condition(TOP_LAYER, x)

#endif /* __RMON_H */

