#ifndef __SGERMON_H
#define __SGERMON_H
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

#ifndef NO_SGE_COMPILE_DEBUG

#include <string.h>

#include "uti/rmon.h"

#if defined(SOLARIS)
#  include <note.h>
#endif

/* EB: TODO: ST: add adoc comments */

#define DENTER_MAIN(layer, program)                                          \
   static const char SGE_FUNC[] = program;                                   \
   static const int xaybzc = layer;                                          \
                                                                             \
   rmon_mopen(&argc,argv,program);                                           \
   if (rmon_condition(xaybzc, TRACE)) {                                      \
      cl_thread_settings_t* ___thread_config = cl_thread_get_thread_config();\
      if (___thread_config != NULL) {                                        \
         rmon_menter (SGE_FUNC, ___thread_config->thread_name);              \
      } else {                                                               \
         rmon_menter (SGE_FUNC, NULL);                                       \
      }                                                                      \
   }
 
#define DENTER(layer, function)                                              \
   static const char SGE_FUNC[] = function;                                  \
   static const int xaybzc = layer;                                          \
                                                                             \
   if (rmon_condition(xaybzc, TRACE)) {                                      \
      cl_thread_settings_t* ___thread_config = cl_thread_get_thread_config();\
      if (___thread_config != NULL) {                                        \
         rmon_menter (SGE_FUNC, ___thread_config->thread_name);              \
      } else {                                                               \
         rmon_menter (SGE_FUNC, NULL);                                       \
      }                                                                      \
   } 

#define DENTER_(layer, function)                                             \
   static const char SGE_FUNC[] = function;                                  \
   static const int xaybzc = layer;                                          \
                                                                             \
   if (rmon_condition(xaybzc, TRACE)) {                                      \
      rmon_menter (SGE_FUNC, NULL);                                          \
   }        

/* fixme: these give "end of loop not reached" warnings with Solaris compiler  */
#define DRETURN(ret)                            \
  do {                                          \
    DEXIT;                                      \
    return ret;                                 \
  } while (0)

#define DRETURN_(ret)                                                   \
   do {                                                                 \
      DEXIT_;                                                           \
      return ret;                                                       \
   } while (0)

#define DRETURN_VOID                            \
  do {                                          \
    DEXIT;                                      \
    return;                                     \
  } while (0)

#define DEXIT                                                           \
   if (rmon_condition(xaybzc, TRACE)) {                                 \
      cl_thread_settings_t* ___thread_config = cl_thread_get_thread_config(); \
      if (___thread_config != NULL) {                                   \
         rmon_mexit(SGE_FUNC, __FILE__, __LINE__, ___thread_config->thread_name); \
      } else {                                                          \
         rmon_mexit(SGE_FUNC, __FILE__, __LINE__, NULL);                \
      }                                                                 \
   }

#define DEXIT_                                                          \
   if (rmon_condition(xaybzc, TRACE)) {                                 \
      rmon_mexit(SGE_FUNC, __FILE__, __LINE__, NULL);                   \
   }                                                                    \

#define DRETURN_VOID_                                                   \
   DEXIT_                                                               \
   return 

#define DTRACE                                                          \
   if (rmon_condition(xaybzc, TRACE)) {                                 \
      cl_thread_settings_t* ___thread_config = cl_thread_get_thread_config(); \
      if (___thread_config != NULL) {                                   \
         rmon_mtrace(SGE_FUNC, __FILE__, __LINE__, ___thread_config->thread_name); \
      } else {                                                          \
         rmon_mtrace(SGE_FUNC, __FILE__, __LINE__, NULL);               \
      }                                                                 \
   }

#define DTRACE_                                                         \
   if (rmon_condition(xaybzc, TRACE)) {                                 \
      rmon_mtrace(SGE_FUNC, __FILE__, __LINE__, NULL);                  \
   }

#define DLOCKPRINTF(msg)                                                \
   if (rmon_condition(xaybzc, LOCK)) {                                  \
      rmon_helper_t *helper = rmon_get_helper();                        \
      if (helper != NULL) {                                             \
         cl_thread_settings_t* ___thread_config = cl_thread_get_thread_config(); \
         if (___thread_config != NULL) {                                \
            strcpy(helper->thread_name, ___thread_config->thread_name); \
         }                                                              \
      }                                                                 \
      rmon_mprintf_lock msg ;                                           \
      if (helper != NULL) {                                             \
         helper->thread_name[0] = '\0';                                 \
      }                                                                 \
   }

#define DLOCKPRINTF_(msg)                       \
   if (rmon_condition(xaybzc, LOCK)) {          \
      rmon_mprintf_lock msg ;                   \
   }


#define DPRINTF(msg)                                                    \
   if (rmon_condition(xaybzc, INFOPRINT)) {                             \
      rmon_helper_t *helper = rmon_get_helper();                        \
      if (helper != NULL) {                                             \
         cl_thread_settings_t* ___thread_config = cl_thread_get_thread_config(); \
         if (___thread_config != NULL) {                                \
            strcpy(helper->thread_name, ___thread_config->thread_name); \
         }                                                              \
      }                                                                 \
      rmon_mprintf_info msg ;                                           \
      if (helper != NULL) {                                             \
         helper->thread_name[0] = '\0';                                 \
      }                                                                 \
   }

#define DPRINTF_(msg)                           \
   if (rmon_condition(xaybzc, INFOPRINT)) {     \
      rmon_mprintf_info msg ;                   \
   }


#define DTIMEPRINTF(msg)                                                \
   if (rmon_condition(xaybzc, TIMING))                                  \
      rmon_mprintf_timing msg

#define DSPECIALPRINTF(msg)                                             \
   if (rmon_condition(xaybzc, SPECIAL))                                 \
      rmon_mprintf_special msg

#define ISTRACE (rmon_condition(xaybzc, TRACE))

#define TRACEON  (rmon_is_enabled() && !rmon_mliszero(&RMON_DEBUG_ON))

#else /* NO_SGE_COMPILE_DEBUG */

#define DENTER_MAIN( layer, program )
#define DENTER( layer, function)
#define DEXIT
#define DRETURN(x) return x
#define DRETURN_VOID return
#define DTRACE
#define DLOCKPRINTF(x)
#define DPRINTF(x)
#define DTIMEPRINTF(x)
#define DSPECIALPRINTF(x)
#define TRACEON
#define ISTRACE

#endif /* NO_SGE_COMPILE_DEBUG */

#define DCLOSE
#define SGE_EXIT(x, y) sge_exit((x), (y))

#endif /* __SGERMON_H */
