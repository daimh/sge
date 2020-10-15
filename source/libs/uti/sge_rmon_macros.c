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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "uti/rmon.h"
#include "uti/msg_rmon.h"
#include "uti/sge_mtutil.h"

#define DEBUG RMON_LOCAL

#if 0
/*
 * It increases scheduler dispatch time by 60-70%.
 * Must be wrong.
 */
define RMON_USE_CTX
#endif 

/* ALPHA (osf4 and tru64) have f(un)lockfile, but prototype is missing */
#if defined (ALPHA)
extern void flockfile(FILE *);
extern void funlockfile(FILE *);
#endif

enum {
   RMON_NONE     = 0,   /* monitoring off */
   RMON_LOCAL    = 1,   /* monitoring on */
   RMON_BUF_SIZE = 5120  /* size of buffer used for monitoring messages */
};

monitoring_level RMON_DEBUG_ON = { {0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L} };
monitoring_level RMON_DEBUG_ON_STORAGE = { {0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L} };

static const char* empty = "    ";

static u_long mtype = RMON_NONE;
#ifdef DEBUG_CLIENT_SUPPORT
static u_long mtype_storage = RMON_NONE;
#endif
static FILE* rmon_fp;

static void mwrite(char *message, const char *thread_name);
static int set_debug_level_from_env(void);
static int set_debug_target_from_env(void);
static void rmon_mprintf_va(int debug_class, const char* fmt, va_list args);

#ifdef DEBUG_CLIENT_SUPPORT
static pthread_mutex_t rmon_print_callback_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t rmon_condition_mutex = PTHREAD_MUTEX_INITIALIZER;
static rmon_print_callback_func_t rmon_print_callback = NULL;
#define RMON_CALLBACK_FUNC_LOCK()      pthread_mutex_lock(&rmon_print_callback_mutex)
#define RMON_CALLBACK_FUNC_UNLOCK()    pthread_mutex_unlock(&rmon_print_callback_mutex)
#define RMON_CONDITION_LOCK()          pthread_mutex_lock(&rmon_condition_mutex)
#define RMON_CONDITION_UNLOCK()        pthread_mutex_unlock(&rmon_condition_mutex)
#endif


static pthread_key_t rmon_ctx_key;
static pthread_once_t rmon_ctx_key_once = PTHREAD_ONCE_INIT;
static void rmon_ctx_key_init(void);
static void rmon_ctx_key_destroy(void * ctx);

static pthread_key_t rmon_helper_key;
static pthread_once_t rmon_helper_key_once = PTHREAD_ONCE_INIT;
static void rmon_helper_key_init(void);
static void rmon_helper_key_destroy(void * ctx);


rmon_ctx_t* rmon_get_thread_ctx(void)
{
  pthread_once(&rmon_ctx_key_once, rmon_ctx_key_init);
  return (rmon_ctx_t*) pthread_getspecific(rmon_ctx_key);
}

void rmon_set_thread_ctx(rmon_ctx_t* ctx) {
   pthread_once(&rmon_ctx_key_once, rmon_ctx_key_init);
   pthread_setspecific(rmon_ctx_key, ctx);
}

/* Allocate the key */
static void rmon_ctx_key_init(void)
{
  pthread_key_create(&rmon_ctx_key, rmon_ctx_key_destroy);
}

/* Free the thread-specific buffer */
static void rmon_ctx_key_destroy(void * ctx)
{
  /* Nothing to destroy */
}

rmon_helper_t *rmon_get_helper(void)
{
   rmon_helper_t *helper = NULL;
   
   pthread_once(&rmon_helper_key_once, rmon_helper_key_init);

   helper = pthread_getspecific(rmon_helper_key);
   if (helper == NULL) {
      helper = (rmon_helper_t *)malloc(sizeof(rmon_helper_t));

      memset(helper, 0, sizeof(rmon_helper_t));
      pthread_setspecific(rmon_helper_key, helper);
   }
   return helper;
}

static void rmon_helper_key_init(void)
{
   pthread_key_create(&rmon_helper_key, rmon_helper_key_destroy);
}

static void rmon_helper_key_destroy(void * ctx)
{
   free(ctx);
}

/****** rmon/Introduction ******************************************************
*  NAME
*     RMON - Grid Engine Monitoring Interface
*
*  FUNCTION
*     The RMON library is a set of functions, which do allow monitoring of 
*     of application execution. The functions provided, however, should not
*     be used directly. Rather the RMON functions are utilized by a set of
*     monitoring macros, like 'DENTER' or 'DEXIT'.
*
*     If monitoring is active, the RMON functions do get called very frequently.
*     Hence, the overhead caused by monitoring needs to be minimal. For this
*     reason, access to external global and static global variables is NOT
*     synchronized through a mutex! Not using a lock of type 'pthread_mutex_t'
*     also means that the RMON functions are async-signal safe.
* 
*     To use RMON library in a multi threaded environment, some restrictions
*     must be followed strictly! It is of utmost importance, that the function
*     'rmon_mopen()' is ONLY invoked from exactly one thread. The thread which
*     is calling 'rmon_mopen()' must be the ONLY thread at this point in time.
*     'DENTER_MAIN' is the only macro from which 'rmon_mopen()' is called. The
*     macro 'DENTER_MAIN' is used at the beginning of a main function. At this
*     point in time, the so called main-thread is the only thread.
*
*     It is safe to call the remaining RMON functions, like 'rmon_menter()' or
*     'rmon_mexit()', from within multiple threads. 'rmon_mopen()' is the only
*     RMON function which does change the critical global variables ('mtype',
*     'rmon_fp' and 'RMON_DEBUG_ON'). 'rmon_menter()' and 'rmon_mexit()' are used by
*     the macro 'DENTER' and 'DEXIT', respectively.
*     
*******************************************************************************/

/****** rmon_macros/rmon_condition() *******************************************
*  NAME
*     rmon_condition() -- Check monitoring condition. 
*
*  SYNOPSIS
*     int rmon_condition(int layer, int class) 
*
*  FUNCTION
*     Check whether monitoring should be enabled for the given combination of
*     'layer' and 'class'. 
*
*  INPUTS
*     int layer - monitor layer 
*     int class - monitor class 
*
*  RESULT
*     1 - do monitor
*     0 - do not monitor
*
*  NOTES
*     MT-NOTE: 'rmon_condition()' is MT safe with exceptions. See introduction!
*
*******************************************************************************/
int rmon_condition(int layer, int class)
{
   int ret_val;
#define MLGETL(s, i) ((s)->ml[i]) /* for the sake of speed */
   /*
    * NOTE:
    * 
    * This is only a static u_long value used as flag for switching
    * debug printing on/off. Therefore we don't need a mutex lock
    * at this point.
    */ 
#ifdef DEBUG_CLIENT_SUPPORT
   if ( mtype == RMON_NONE ) {
      return 0;
   }
   
   /* if debug printing is on we use a lock for further layer checking */
   RMON_CONDITION_LOCK();
#endif
   ret_val = ((mtype != RMON_NONE) && (class & MLGETL(&RMON_DEBUG_ON, layer))) ? 1 : 0;
#ifdef DEBUG_CLIENT_SUPPORT
   RMON_CONDITION_UNLOCK();
#endif
   return ret_val;
#undef MLGETL
} /* rmon_condition() */


/****** rmon_macros/rmon_debug_client_callback() ****************************
*  NAME
*     rmon_debug_client_callback() -- callback for debug clients
*
*  SYNOPSIS
*     static void rmon_debug_client_callback(int dc_connected, int debug_level) 
*
*  FUNCTION
*     Is called when a debug client is connected/disconnected or on
*     debug level changes. Use cl_com_application_debug() to send debug
*     messages to the connected qping -dump client.
*
*  INPUTS
*     int dc_connected - 1 debug client is connected
*                        0 no debug client is connected
*     int debug_level  - debug level from 0 (off) to 5(DPRINTF)
*  NOTES
*     MT-NOTE: rmon_debug_client_callback() is MT safe 
*              (but is called from commlib thread, which means that no
*               qmaster thread specific setup is done), just use global
*               thread locking methods which are initialized when called, or
*               initialized at compile time)
*
*******************************************************************************/
void rmon_debug_client_callback(int dc_connected, int debug_level) {
#ifdef DEBUG_CLIENT_SUPPORT
   RMON_CONDITION_LOCK();

   /* we are saving the old value of the RMON_DEBUG_ON structure into RMON_DEBUG_ON_STORAGE */
   if (dc_connected) {
      /* TODO: support rmon debug levels with $SGE_DEBUG_LEVEL string ? 
       *       if so, the debug_level parameter should be a string value
       */
      (&RMON_DEBUG_ON)->ml[TOP_LAYER]   = 2; 
      if (debug_level > 1) {
         (&RMON_DEBUG_ON)->ml[TOP_LAYER]   = 3; 
      }
      mtype = RMON_LOCAL;
   } else {
      (&RMON_DEBUG_ON)->ml[TOP_LAYER]   = (&RMON_DEBUG_ON_STORAGE)->ml[TOP_LAYER]; 
      mtype = mtype_storage;
   }
   RMON_CONDITION_UNLOCK();
#else
   return;
#endif
}

void rmon_set_print_callback(rmon_print_callback_func_t function_p) {
#ifdef DEBUG_CLIENT_SUPPORT
   if (function_p != NULL) {
      RMON_CALLBACK_FUNC_LOCK();
      rmon_print_callback = *function_p;
      RMON_CALLBACK_FUNC_UNLOCK();
   }
#else
   return;
#endif
}



/****** rmon_macros/rmon_is_enabled() ******************************************
*  NAME
*     rmon_is_enabled() -- Check if monitoring is enabled. 
*
*  SYNOPSIS
*     int rmon_is_enabled(void) 
*
*  FUNCTION
*     Check if monitoring is enabled. Note that even if monitoring is enabled
*     no actual monitoring output may be generated. Generation of monitoring
*     output is controlled by 'rmon_condition()'.   
*
*  INPUTS
*     void - none 
*
*  RESULT
*     1 - monitoring enabled 
*     0 - monitoring disabled
*
*  NOTES
*     MT-NOTE: 'rmon_is_enabled()' is MT safe with exceptions. See introduction! 
*
*******************************************************************************/
int rmon_is_enabled(void)
{
   return ((mtype == RMON_LOCAL) ? 1 : 0);
} /* rmon_is_enabled() */

/****** rmon_macros/rmon_mopen() ***********************************************
*  NAME
*     rmon_mopen() -- Open, i.e. initialize monitoring. 
*
*  SYNOPSIS
*     void rmon_mopen(int *argc, char *argv[], char *programname) 
*
*  FUNCTION
*     Initialize monitoring. Clear all monitoring levels. Set monitoring levels
*     according to 'SGE_DEBUG_LEVEL' environment variable. Set monitoring
*     target (i.e. output stream) according to 'SGE_DEBUG_TARGET' environment
*     variable. Enable monitoring.  
*
*     NOTE: Even though 'argc' and 'argv' are not used, they do make sure that
*     'rmon_mopen()' is only used within a main function to a certain degree.
*
*  INPUTS
*     int *argc         - not used 
*     char *argv[]      - not used 
*     char *programname - not used 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: 'rmon_mopen()' is NOT MT safe. See introduction! 
*
*******************************************************************************/
void rmon_mopen(int *argc, char *argv[], char *programname)
{
   int ret = -1;

   rmon_mlclr(&RMON_DEBUG_ON);
   rmon_fp = stderr;

   ret = set_debug_level_from_env();
   ret = set_debug_target_from_env();

   if (ret != 0) {
      exit(-1);
   }

   mtype = RMON_LOCAL;
#ifdef DEBUG_CLIENT_SUPPORT
   mtype_storage = mtype;
#endif

   return;
} /* rmon_mopen */

/****** rmon_macros/rmon_menter() **********************************************
*  NAME
*     rmon_menter() -- Monitor function entry 
*
*  SYNOPSIS
*     void rmon_menter(const char *func) 
*
*  FUNCTION
*     Monitor function entry. Generate function entry message. 
*
*  INPUTS
*     const char *func - function name 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: 'rmon_menter()' is MT safe with exceptions. See introduction! 
*
*******************************************************************************/

void rmon_menter(const char *func, const char *thread_name)
{
   char msgbuf[RMON_BUF_SIZE];
#ifdef RMON_USE_CTX
   rmon_ctx_t *ctx = rmon_get_thread_ctx();
   if (ctx) {
      ctx->menter(ctx, func);
      return;
   }
#endif      
   sprintf(msgbuf, "--> %s() {\n", func);
   mwrite(msgbuf, thread_name);
} /* rmon_enter() */



/****** rmon_macros/rmon_mexit() ***********************************************
*  NAME
*     rmon_mexit() -- Monitor function exit 
*
*  SYNOPSIS
*     void rmon_mexit(const char *func, const char *file, int line) 
*
*  FUNCTION
*     Monitor function exit. Generate function exit message. 
*
*  INPUTS
*     const char *func - function name 
*     const char *file - source file in which function is defined 
*     int line         - number of invocation source line
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: 'rmon_mexit()' is MT safe with exceptions. See introduction! 
*
*******************************************************************************/
void rmon_mexit(const char *func, const char *file, int line, const char *thread_name)
{
   char msgbuf[RMON_BUF_SIZE];
#ifdef RMON_USE_CTX   
   rmon_ctx_t *ctx = rmon_get_thread_ctx();
   if (ctx) {
      ctx->mexit(ctx, func, file, line);
      return;
   }
#endif
   sprintf(msgbuf, "<-- %s() %s %d }\n", func, file, line);
   mwrite(msgbuf, thread_name);

} /* rmon_mexit() */

/****** rmon_macros/rmon_mtrace() **********************************************
*  NAME
*     rmon_mtrace() -- Monitor function progress 
*
*  SYNOPSIS
*     void rmon_mtrace(const char *func, const char *file, int line) 
*
*  FUNCTION
*     Monitor function progress. Generate function trace message. 
*
*  INPUTS
*     const char *func - function name 
*     const char *file - source file in which function is defined 
*     int line         - number of invocation source line
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: 'rmon_mtrace()' is MT safe with exceptions. See introduction! 
*
*******************************************************************************/
void rmon_mtrace(const char *func, const char *file, int line, const char *thread_name)
{
   char msgbuf[RMON_BUF_SIZE];
#ifdef RMON_USE_CTX   
   rmon_ctx_t *ctx = rmon_get_thread_ctx();
   if (ctx) {
      ctx->mtrace(ctx, func, file, line);
      return;
   }
#endif      
   strcpy(msgbuf, empty);
   sprintf(&msgbuf[4], "%s:%s:%d\n", func, file, line);
   mwrite(msgbuf, thread_name);
} /* rmon_mtrace() */

/****** rmon_macros/rmon_mprintf() *********************************************
*  NAME
*     rmon_mprintf() -- Print formatted monitoring message. 
*
*  SYNOPSIS
*     void rmon_mprintf(const char *fmt, ...) 
*
*  FUNCTION
*     Print formatted monitoring message. 
*
*  INPUTS
*     const char *fmt - format string 
*     ...             - variable argument list 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: 'rmon_mprintf()' is MT safe with exceptions. See introduction! 
*
*******************************************************************************/
void rmon_mprintf(int debug_class, const char *fmt,...)
{
   va_list args;
   
   va_start(args, fmt);
   rmon_mprintf_va(debug_class, fmt, args);
   va_end(args);
   
   return;
} /* rmon_mprintf() */


void rmon_mprintf_lock(const char* fmt, ...) {
   va_list args;
   va_start(args, fmt);
   rmon_mprintf_va(LOCK, fmt, args);
   va_end(args);
}

void rmon_mprintf_info(const char* fmt, ...) {
   va_list args;
   va_start(args, fmt);
   rmon_mprintf_va(INFOPRINT, fmt, args);
   va_end(args);
}

void rmon_mprintf_timing(const char* fmt, ...) {
   va_list args;
   va_start(args, fmt);
   rmon_mprintf_va(TIMING, fmt, args);
   va_end(args);
}

void rmon_mprintf_special(const char* fmt, ...) {
   va_list args;
   va_start(args, fmt);
   rmon_mprintf_va(SPECIAL, fmt, args);
   va_end(args);
}

static void rmon_mprintf_va(int debug_class, const char* fmt, va_list args) {
   char msgbuf[RMON_BUF_SIZE];
   rmon_helper_t *helper = NULL;
#ifdef RMON_USE_CTX   
   rmon_ctx_t *ctx = rmon_get_thread_ctx();
   if (ctx) {
      ctx->mprintf(ctx, debug_class, fmt, args);
      return;
   }
#endif
   helper = rmon_get_helper();
   strcpy(msgbuf, empty);
   vsnprintf(&msgbuf[4], (RMON_BUF_SIZE) - 10 , fmt, args);
   if ((helper != NULL) && (helper->thread_name != NULL) && (strlen(helper->thread_name) > 0)) {
      mwrite(msgbuf, helper->thread_name);
   } else {
      mwrite(msgbuf, NULL);
   }
}

/****** rmon_macros/mwrite() ***************************************************
*  NAME
*     mwrite() -- Write monitoring message
*
*  SYNOPSIS
*     static void mwrite(char *message) 
*
*  FUNCTION
*     Write monitoring message. The message is written to the output stream
*     associated with 'rmon_fp'. The output stream is flushed immediately. 
*
*     A prefix is added to 'message'. It does consist of a trace sequence number,
*     the PID and the thread ID of the calling thread.
*
*  INPUTS
*     char *message - monitoring message
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: 'mwrite()' is MT safe with exceptions. See introduction!
*     MT-NOTE:
*     MT-NOTE: It is guaranteed that the output of different threads is not
*     MT-NOTE: mingled.
*
*******************************************************************************/
static void mwrite(char *message, const char *thread_name)
{
   static u_long traceid = 0;
   unsigned long tmp_pid    = (unsigned long) getpid();
   unsigned long tmp_thread = (unsigned long) pthread_self();

#ifdef DEBUG_CLIENT_SUPPORT
   /* if there is a callback function, don't call standard function */
   RMON_CALLBACK_FUNC_LOCK();
   if (rmon_print_callback != NULL) {
      rmon_print_callback(message, traceid, tmp_pid, tmp_thread);
   }
   RMON_CALLBACK_FUNC_UNLOCK();
#endif

   if (thread_name != NULL) {
      fprintf(rmon_fp, "%6ld %6d %12.12s ", traceid, (int)tmp_pid, thread_name);
   } else {
      fprintf(rmon_fp, "%6ld %6d %ld ", traceid, (int)tmp_pid, (long int)tmp_thread);
   }
   fprintf(rmon_fp, "%s", message);
   fflush(rmon_fp);

   traceid++;

   return;
} /* mwrite() */

/****** rmon_macros/set_debug_level_from_env() *********************************
*  NAME
*     set_debug_level_from_env() -- Set debug level from environment variable.
*
*  SYNOPSIS
*     static int set_debug_level_from_env(void) 
*
*  FUNCTION
*     Set debug level. Read environment variable "SGE_DEBUG_LEVEL" and use it
*     to initialize debug levels.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     0 - successful
*     ENOENT - environment variable not set
*     EINVAL - unexpected format
*
*  NOTES
*     MT-NOTE: 'set_debug_level_from_env()' is MT safe with exceptions.
*     MT-NOTE:  See introduction!
*
*******************************************************************************/
static int set_debug_level_from_env(void)
{
   const char *env, *s = NULL;
   int i, l[N_LAYER];

   if ((env = getenv("SGE_DEBUG_LEVEL")) == NULL) {
      return ENOENT;
   }

   s = strdup(env);
   if ((i = sscanf(s, "%d%d%d%d%d%d%d%d", l, l+1, l+2, l+3, l+4, l+5, l+6, l+7)) != N_LAYER) {
      printf("%s\n", MSG_RMON_ILLEGALDBUGLEVELFORMAT);
      free((char *)s);
      return EINVAL;
   }

   for (i = 0; i < N_LAYER; i++) {
      rmon_mlputl(&RMON_DEBUG_ON, i, l[i]);
      rmon_mlputl(&RMON_DEBUG_ON_STORAGE, i, l[i]);
   }

   free((char *)s);
   return 0;
} /* set_debug_level_from_env() */

/****** rmon_macros/set_debug_target_from_env() *********************************
*  NAME
*     set_debug_target_from_env() -- Set debug target from environment variable.
*
*  SYNOPSIS
*     static int set_debug_target_from_env(void) 
*
*  FUNCTION
*     Set debug target. Read environment variable "SGE_DEBUG_TARGET" and use it
*     to initialize debug output target. 
*
*     'SGE_DEBUG_TARGET' may either be 'stdout', 'stderr' or a fully qualified
*     file name (that is file name and path). If a file name is given an 
*     already existing file with the same name will be overwritten.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     0 - successful
*     EACCES - file name is invalid or unable to open file
*
*  NOTES
*     MT-NOTE: 'set_debug_target_from_env()' is MT safe with exceptions.
*     MT-NOTE: See introduction!
*
*******************************************************************************/
static int set_debug_target_from_env(void)
{
   const char *env, *s = NULL;

   if ((env = getenv("SGE_DEBUG_TARGET")) == NULL) {
      return 0;
   }

   s = strdup(env);
   if (strcmp(s, "stdout") == 0) {
         rmon_fp = stdout;
   } else if (strcmp(s, "stderr") == 0) {
         rmon_fp = stderr;
   } else if ((rmon_fp = fopen(s, "w")) == NULL) {
      rmon_fp = stderr;
      fprintf(rmon_fp, MSG_RMON_UNABLETOOPENXFORWRITING_S, s);
      fprintf(rmon_fp, MSG_RMON_ERRNOXY_DS, errno, strerror(errno));
      free((char *)s);
      return EINVAL;
   } 

   free((char *)s);
   return 0;
} /* set_debug_target_from_env() */
