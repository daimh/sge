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

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_dstring.h"
#include "uti/sge_prog.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_mtutil.h"

#include "gdi/sge_gdi_ctx.h"
#include "uti/sge_string.h"

#include "uti/msg_utilib.h"

#include "sge.h"

typedef struct {
   pthread_mutex_t  mutex;
   char*            log_file;
   u_long32         log_level;
   int              log_as_admin_user;
   int              verbose;
   int              gui_log;
} log_state_t;

/* Fixme:  Any log message must fit into this.  What guarantee do we
   have that it can't overflow?  */
typedef struct {
   char  log_buffer[4 * MAX_STRING_SIZE]; /* a.k.a. SGE_EVENT */
} log_buffer_t;

typedef struct {
   sge_gdi_ctx_class_t *context;
} log_context_t;


static log_state_t Log_State = {PTHREAD_MUTEX_INITIALIZER, "syslog.init", LOG_WARNING, 0, 1, 1};

static pthread_once_t log_buffer_once = PTHREAD_ONCE_INIT;
static pthread_key_t  log_buffer_key;

static pthread_once_t log_context_once = PTHREAD_ONCE_INIT;
static pthread_key_t  log_context_key;

static void log_buffer_once_init(void);
static void log_context_once_init(void);

static void log_buffer_destroy(void* theState);
static log_buffer_t* log_buffer_getspecific(void);

static void log_context_destroy(void* theState);
static log_context_t* log_context_getspecific(void);
static sge_gdi_ctx_class_t* log_state_get_log_context(void);


static void sge_do_log(u_long32 prog_number,
                       const char *progname,
                       const char *unqualified_hostname,
                       int aLevel, 
                       const char* aMessage,
                       int log_level);

static sge_gdi_ctx_class_t *log_get_log_context(void);
static void log_set_log_context(sge_gdi_ctx_class_t *theCtx);

/****** uti/log/log_get_log_buffer() ******************************************
*  NAME
*     log_get_log_buffer() -- Return a buffer that can be used to build logging 
*                                   strings
*
*  SYNOPSIS
*     char *log_get_log_buffer(void) 
*
*  FUNCTION
*     Return a buffer that can be used to build logging strings
*
*  RESULT
*     char * 
*
******************************************************************************/
char *log_get_log_buffer(void)
{
   log_buffer_t *buf = NULL;
   char *log_buffer = NULL;

   buf = log_buffer_getspecific();

   if (buf != NULL) {
      log_buffer = buf->log_buffer;
   }

   return log_buffer;
}

/****** uti/log/log_get_log_context() ******************************************
*  NAME
*     log_get_log_context() -- Return a context for the specific thread 
*
*  SYNOPSIS
*     sge_gdi_ctx_class_t* log_get_log_context(void) 
*
*  FUNCTION
*     Return a context for the specific thread
*
*  RESULT
*     sge_gdi_ctx_class_t* 
*
******************************************************************************/
static sge_gdi_ctx_class_t* log_get_log_context(void)
{
   log_context_t *log_ctx = NULL;
   sge_gdi_ctx_class_t *context = NULL;

   log_ctx = log_context_getspecific();

   if (log_ctx != NULL) {
      context = log_ctx->context;
   }   

   return context;
}

/****** uti/log/log_set_log_context() ******************************************
*  NAME
*     log_set_log_context() -- set a context for the specific thread 
*
*  SYNOPSIS
*     void log_set_log_context(void *newctx) 
*
*  FUNCTION
*     Return a context for the specific thread
*
*  INPUTS
*     void newctx - the new ctx
*
*  RESULT
*
*  NOTES
*     MT-NOTE: log_state_set_log_file() is not MT safe.
*     MT-NOTE:
*     MT-NOTE: It is safe, however, to call this function from within multiple
*     MT-NOTE: threads as long as no other thread calls log_state_set_log_file() 
*
******************************************************************************/
static void log_set_log_context(sge_gdi_ctx_class_t *newctx)
{
   log_context_t *log_ctx = NULL;

   log_ctx = log_context_getspecific();

   if (log_ctx != NULL) {
      log_ctx->context = newctx;
   } 
}

/****** uti/log/log_state_get_log_level() ******************************************
*  NAME
*     log_state_get_log_level() -- Return log level.
*
*  SYNOPSIS
*     u_long32 log_state_get_log_level(void) 
*
*  FUNCTION
*     Return log level
*
*  RESULT
*     u_long32
*
******************************************************************************/
u_long32 log_state_get_log_level(void)
{
   u_long32 level = 0;

   sge_mutex_lock("Log_State_Lock", "log_state_get_log_level", __LINE__, &Log_State.mutex);

   level = Log_State.log_level;

   sge_mutex_unlock("Log_State_Lock", "log_state_get_log_level", __LINE__, &Log_State.mutex);

   return level;
}

/****** uti/sge_log/log_state_get_log_file() ***********************************
*  NAME
*     log_state_get_log_file() -- get log file name
*
*  SYNOPSIS
*     const char* log_state_get_log_file(void) 
*
*  FUNCTION
*     Return name of current log file. The string returned may or may not 
*     contain a path.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     const char* - log file name (with relative or absolute path)
*
*  NOTES
*     MT-NOTE: log_state_get_log_file() is not MT safe.
*     MT-NOTE:
*     MT-NOTE: It is safe, however, to call this function from within multiple
*     MT-NOTE: threads as long as no other thread does change 'Log_File'.
*
*  BUGS
*     BUGBUG-AD: This function should use something like a barrier for
*     BUGBUG-AD: synchronization.
*
*******************************************************************************/
const char *log_state_get_log_file(void)
{
   char* file = NULL;

   sge_mutex_lock("Log_State_Lock", "log_state_get_log_file", __LINE__, &Log_State.mutex);

   file = Log_State.log_file;

   sge_mutex_unlock("Log_State_Lock", "log_state_get_log_file", __LINE__, &Log_State.mutex);

   return file;
}

/****** uti/log/log_state_get_log_verbose() ******************************************
*  NAME
*     log_state_get_log_verbose() -- Is verbose logging enabled?
*
*  SYNOPSIS
*     int log_state_get_log_verbose(void) 
*
*  FUNCTION
*     Is verbose logging enabled? 
*     With verbose logging enabled not only ERROR/CRITICAL messages are 
*     printed to stderr but also WARNING/INFO.
*
*  RESULT
*     int - 0 or 1 
*
*  SEE ALSO
*     uti/log/log_state_set_log_verbose()
******************************************************************************/
int log_state_get_log_verbose(void)
{
   int verbose = 0;

   sge_mutex_lock("Log_State_Lock", "log_state_get_log_verbose", __LINE__, &Log_State.mutex);
   
   verbose = Log_State.verbose;
   
   sge_mutex_unlock("Log_State_Lock", "log_state_get_log_verbose", __LINE__, &Log_State.mutex);

   return verbose;
}

/****** uti/log/log_state_get_log_gui() ******************************************
*  NAME
*     log_state_get_log_gui() -- Is GUI logging enabled?
*
*  SYNOPSIS
*     int log_state_get_log_gui(void) 
*
*  FUNCTION
*     Is GUI logging enabled? 
*     With GUI logging enabled messages are printed to stderr/stdout.  
*
*  RESULT
*     int - 0 or 1 
*
*  SEE ALSO
*     uti/log/log_state_set_log_gui()
******************************************************************************/
int log_state_get_log_gui(void)
{
   int gui_log = 0;

   sge_mutex_lock("Log_State_Lock", "log_state_get_log_gui", __LINE__, &Log_State.mutex);
   
   gui_log = Log_State.gui_log;

   sge_mutex_unlock("Log_State_Lock", "log_state_get_log_gui", __LINE__, &Log_State.mutex);

   return gui_log;
}

/****** uti/log/log_state_get_log_as_admin_user() ******************************************
*  NAME
*     log_state_get_log_as_admin_user() -- Needs sge_log() to change into admin user?
*
*  SYNOPSIS
*     trace_func_type log_state_get_log_as_admin_user(void) 
*
*  FUNCTION
*     Returns whether logging shall be done as admin user.
*
*  RESULT
*     int
*
*  SEE ALSO
*     uti/log/log_state_set_log_as_admin_user()
******************************************************************************/
int log_state_get_log_as_admin_user(void)
{
   int log_as_admin_user = 0;

   sge_mutex_lock("Log_State_Lock", "log_state_get_log_as_admin_user", __LINE__, &Log_State.mutex);
   
   log_as_admin_user = Log_State.log_as_admin_user;
   
   sge_mutex_unlock("Log_State_Lock", "log_state_get_log_as_admin_user", __LINE__, &Log_State.mutex);

   return log_as_admin_user;
}

/****** uti/log/log_state_set_log_level() *****************************************
*  NAME
*     log_state_set_log_level() -- Set log level to be used.
*
*  SYNOPSIS
*     void log_state_set_log_level(int i) 
*
*  FUNCTION
*     Set log level to be used.
*
*  INPUTS
*     u_long32 
*
*  SEE ALSO
*     uti/log/log_state_get_log_level() 
******************************************************************************/
void log_state_set_log_level(u_long32 theLevel)
{ 

   sge_mutex_lock("Log_State_Lock", "log_state_set_log_level", __LINE__, &Log_State.mutex);
   
   Log_State.log_level = theLevel;

   sge_mutex_unlock("Log_State_Lock", "log_state_set_log_level", __LINE__, &Log_State.mutex);

   return;
}

void log_state_set_log_file(char *theFile)
{
   sge_mutex_lock("Log_State_Lock", "log_state_set_log_file", __LINE__, &Log_State.mutex);

   /* Don't change it if it's been set to syslog in the execd or
      qmaster params.  */
   if (strcmp( Log_State.log_file, "syslog") != 0)
      Log_State.log_file = theFile;
   
   sge_mutex_unlock("Log_State_Lock", "log_state_set_log_file", __LINE__, &Log_State.mutex);

   return;
}

/****** uti/log/log_state_set_log_verbose() *****************************************
*  NAME
*     log_state_set_log_verbose() -- Enable/disable verbose logging 
*
*  SYNOPSIS
*     void log_state_set_log_verbose(int i) 
*
*  FUNCTION
*     Enable/disable verbose logging 
*
*  INPUTS
*     int i - 0 or 1  
*
*  SEE ALSO
*     uti/log/log_state_get_log_verbose() 
******************************************************************************/
void log_state_set_log_verbose(int i)
{
   sge_mutex_lock("Log_State_Lock", "log_state_set_log_verbose", __LINE__, &Log_State.mutex);
   
   Log_State.verbose = i;
   
   sge_mutex_unlock("Log_State_Lock", "log_state_set_log_verbose", __LINE__, &Log_State.mutex);

   return;
}

/****** uti/log/log_state_set_log_gui() ********************************************
*  NAME
*     log_state_set_log_gui() -- Enable/disable logging for GUIs 
*
*  SYNOPSIS
*     void log_state_set_log_gui(int i) 
*
*  FUNCTION
*     Enable/disable logging for GUIs 
*     With GUI logging enabled messages are printed to stderr/stdout.  
*
*  INPUTS
*     int i - 0 or 1 
******************************************************************************/
void log_state_set_log_gui(int i)
{

   sge_mutex_lock("Log_State_Lock", "log_state_set_log_gui", __LINE__, &Log_State.mutex);
   
   Log_State.gui_log = i;

   sge_mutex_unlock("Log_State_Lock", "log_state_set_log_gui", __LINE__, &Log_State.mutex);
   
   return;
}   

/****** uti/log/log_state_set_log_as_admin_user() *****************************
*  NAME
*     log_state_set_log_as_admin_user() -- Enable/Disable logging as admin user 
*
*  SYNOPSIS
*     void log_state_set_log_as_admin_user(int i)
*
*  FUNCTION
*     This function enables/disables logging as admin user. This 
*     means that the function/macros switches from start user to 
*     admin user before any messages will be written. After that 
*     they will switch back to 'start' user. 
*
*  INPUTS
*     int i - 0 or 1 
*  
*  SEE ALSO
*     uti/uidgid/sge_switch2admin_user
*     uti/uidgid/sge_switch2start_user
******************************************************************************/
void log_state_set_log_as_admin_user(int i)
{
   sge_mutex_lock("Log_State_Lock", "log_state_set_log_as_admin_user", __LINE__, &Log_State.mutex);
   
   Log_State.log_as_admin_user = i;

   sge_mutex_unlock("Log_State_Lock", "log_state_set_log_as_admin_user", __LINE__, &Log_State.mutex);
}


static sge_gdi_ctx_class_t* log_state_get_log_context(void)
{
   sge_gdi_ctx_class_t *log_context = NULL;

/*    sge_mutex_lock("Log_State_Lock", "log_state_get_log_context", __LINE__, &Log_State.mutex); */

   log_context = log_get_log_context();

/*    sge_mutex_unlock("Log_State_Lock", "log_state_get_log_context", __LINE__, &Log_State.mutex); */

   return log_context;
}

void log_state_set_log_context(void *theCtx)
{
   sge_mutex_lock("Log_State_Lock", "log_state_set_log_context", __LINE__, &Log_State.mutex);
   
   log_set_log_context((sge_gdi_ctx_class_t*)theCtx);

   sge_mutex_unlock("Log_State_Lock", "log_state_set_log_context", __LINE__, &Log_State.mutex);
}



/****** uti/log/sge_log() *****************************************************
*  NAME
*     sge_log() -- Low level logging function 
*
*  SYNOPSIS
*     int sge_log(int log_level, char *mesg, char *file__, 
*                 char *func__, int line__) 
*
*  FUNCTION
*     Low level logging function. Used by various macros.
*     Do not use this function directly. 
*
*  INPUTS
*     int log_level - Logging Level
*     char *mesg    - Message
*     char *file__  - Filename
*     char *func__  - Function Name
*     int line__    - Line within 'file__'
*
*  RESULT
*     int - 0
*
*  SEE ALSO
*     uti/log/CRITICAL
*     uti/log/ERROR
*     uti/log/WARNING
*     uti/log/NOTICE
*     uti/log/INFO
*     uti/log/DEBUG
*
*  NOTES
*     MT-NOTE: sge_log() is not MT safe due to sge_switch2admin_user()
*     MT-NOTE: sge_log() can be used in clients where no admin user switching 
*     MT-NOTE: takes place
*     MT-NOTE: sge_log() is not MT safe due rmon_condition()
*     MT-NOTE: sge_log() can be used if DENTER_MAIN() is called only by one 
*     MT-NOTE: thread
******************************************************************************/
int sge_log(int log_level, const char *mesg, const char *file__, const char *func__, int line__) 
{
   char buf[128*4];
   int levelchar;
   char levelstring[32*4];
   
   sge_gdi_ctx_class_t *ctx = NULL;
   /* TODO: this must be kept for qmaster and should be done in a different
            way (qmaster context) !!! */
   u_long32 me = 0;
   const char *threadname = NULL;
   const char *unqualified_hostname = NULL;
   int is_daemonized = 0; 

   DENTER_(BASIS_LAYER, "sge_log");
   
   ctx = log_state_get_log_context();
   
   if (ctx != NULL) {
      me = ctx->get_who(ctx);
      threadname = ctx->get_thread_name(ctx);
      unqualified_hostname = ctx->get_unqualified_hostname(ctx);
      is_daemonized = ctx->is_daemonized(ctx);
   } else {
      DPRINTF(("sge_log: ctx is NULL\n"));
   }   

   /* Make sure to have at least a one byte logging string */
   if (!mesg || mesg[0] == '\0') {
      snprintf(buf, sizeof(buf), MSG_LOG_CALLEDLOGGINGSTRING_S,
               mesg ? MSG_LOG_ZEROLENGTH : MSG_POINTER_NULL);
      mesg = buf;
   }

   DPRINTF(("%s %d %s\n", file__, line__, mesg));

   /* quick exit if nothing to log */
   if (log_level > MAX(log_state_get_log_level(), LOG_WARNING)) {
      DRETURN_(0);
   }

   if (!log_state_get_log_gui()) {
      DRETURN_(0);
   }

   switch(log_level) {
      case LOG_PROF:
         sge_strlcpy(levelstring, MSG_LOG_PROFILING, sizeof(levelstring));
         levelchar = 'P';
         break;
      case LOG_CRIT:
         sge_strlcpy(levelstring, MSG_LOG_CRITICALERROR, sizeof(levelstring));
         levelchar = 'C';
         break;
      case LOG_ERR:
         sge_strlcpy(levelstring, MSG_LOG_ERROR, sizeof(levelstring));
         levelchar = 'E';
         break;
      case LOG_WARNING:
         strcpy(levelstring, "");
         levelchar = 'W';
         break;
      case LOG_NOTICE:
         strcpy(levelstring, "");
         levelchar = 'N';
         break;
      case LOG_INFO:
         strcpy(levelstring, "");
         levelchar = 'I';
         break;
      case LOG_DEBUG:
         strcpy(levelstring, "");
         levelchar = 'D';
         break;
      default:
         strcpy(levelstring, "");
         levelchar = 'L';
         log_level = LOG_INFO;
         break;
   }

   /* avoid double output in debug mode */
   if (!is_daemonized && !rmon_condition(TOP_LAYER, INFOPRINT) && 
       (log_state_get_log_verbose() || log_level <= LOG_WARNING)) {
      fprintf(stderr, "%s%s\n", levelstring, mesg);
   }

   sge_do_log(me, threadname, unqualified_hostname, levelchar, mesg, log_level);

   DRETURN_(0);
} /* sge_log() */

/****** uti/sge_log/sge_do_log() ***********************************************
*  NAME
*     sge_do_log() -- Write message to log file (or syslog)
*
*  SYNOPSIS
*     static void sge_do_log(u_long32 me, const char* progname, const char* unqualified_hostname, int aLevel, const char *aMessage, int log_level)
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     u_long32 me          - process type (QMASTER etc.)
*     const char* progname - program name
*     const char* unqualified_hostname - name of host issuing message
*     int aLevel           - log level
*     const char *aMessage - log message
*     int log_level        - numeric level from which aLevel is derived
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: sge_do_log() is MT safe.
*
*******************************************************************************/
static void sge_do_log(u_long32 me, const char* progname, const char* unqualified_hostname,
                       int aLevel, const char *aMessage, int log_level)
{
   int fd;

   if (me == QMASTER || me == EXECD || me == SCHEDD || me == SHADOWD) {
      const char *logfile = log_state_get_log_file();
      char msg2log[4*MAX_STRING_SIZE];
      dstring msg;

      sge_dstring_init(&msg, msg2log, sizeof(msg2log));

      if (strncmp(logfile, "syslog", 6) == 0) {
         if (LOG_PROF == log_level)
            log_level = LOG_INFO;
         sge_dstring_sprintf_append(&msg, "%6.6s|%s|%c|%s\n",
                                    progname, unqualified_hostname,
                                    aLevel, aMessage);
         syslog(log_level | LOG_USER, "%s", msg2log);
      } else if ((fd = SGE_OPEN3(logfile, O_WRONLY | O_APPEND | O_CREAT,
                                 0666)) >= 0) {
         int len;

         append_time((time_t)sge_get_gmt(), &msg, false);

         sge_dstring_sprintf_append(&msg, "|%6.6s|%s|%c|%s\n",
                 progname,
                 unqualified_hostname,
                 aLevel,
                 aMessage);

         len = strlen(msg2log);
         if (write(fd, msg2log, len) != len) {
            /* we are in error logging here - the only chance to log this problem
             * might be to write it to stderr
             */
            fprintf(stderr, "can't log to file %s: %s\n", log_state_get_log_file(), sge_strerror(errno, &msg));
         }
         close(fd);
      }
   }

   return;
} /* sge_do_log() */

/****** uti/log/log_buffer_once_init() ********************************************
*  NAME
*     log_buffer_once_init() -- One-time logging initialization.
*
*  SYNOPSIS
*     static log_buffer_once_init(void) 
*
*  FUNCTION
*     Create access key for thread local storage. Register cleanup function.
*     This function must be called exactly once.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: log_buffer_once_init() is MT safe. 
*
*******************************************************************************/
static void log_buffer_once_init(void)
{
   pthread_key_create(&log_buffer_key, &log_buffer_destroy);
} /* log_once_init */

/****** uti/log/log_buffer_destroy() ****************************************
*  NAME
*     log_buffer_destroy() -- Free thread local storage
*
*  SYNOPSIS
*     static void log_buffer_destroy(void* theState) 
*
*  FUNCTION
*     Free thread local storage.
*
*  INPUTS
*     void* theState - Pointer to memroy which should be freed.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: log_buffer_destroy() is MT safe.
*
*******************************************************************************/
static void log_buffer_destroy(void* theBuffer)
{
   sge_free(&theBuffer);
}

/****** uti/log/log_buffer_getspecific() ****************************************
*  NAME
*     log_buffer_getspecific() -- Get thread local log state
*
*  SYNOPSIS
*     static log_buffer_t* log_buffer_getspecific() 
*
*  FUNCTION
*     Return thread local log state.
*
*     If a given thread does call this function for the first time, no thread
*     local log state is available for this particular thread. In this case the
*     thread local log state is allocated and set.
*
*  RESULT
*     static log_buffer_t* - Pointer to thread local log state.
*
*  NOTES
*     MT-NOTE: log_buffer_getspecific() is MT safe 
*
*******************************************************************************/
static log_buffer_t* log_buffer_getspecific(void)
{
   log_buffer_t *buf = NULL;
   int res = -1;

   pthread_once(&log_buffer_once, log_buffer_once_init);

   if ((buf = pthread_getspecific(log_buffer_key)) != NULL) {
      return buf;
   }

   buf = (log_buffer_t*)sge_malloc(sizeof(log_buffer_t));
   memset((void*)(buf), 0, sizeof(log_buffer_t));

   res = pthread_setspecific(log_buffer_key, (const void*)buf);

   if (0 != res) {
      fprintf(stderr, "pthread_set_specific(%s) failed: %s\n", "log_buffer_getspecific", strerror(res));
      abort();
   }
   
   return buf;
} /* log_buffer_getspecific() */


/****** uti/log/log_context_once_init() ********************************************
*  NAME
*     log_context_once_init() -- One-time logging initialization.
*
*  SYNOPSIS
*     static log_context_once_init(void) 
*
*  FUNCTION
*     Create access key for thread local storage. Register cleanup function.
*     This function must be called exactly once.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: log_context_once_init() is MT safe. 
*
*******************************************************************************/
static void log_context_once_init(void)
{
   pthread_key_create(&log_context_key, &log_context_destroy);
} /* log_once_init */

/****** uti/log/log_context_destroy() ****************************************
*  NAME
*     log_context_destroy() -- Free thread local storage
*
*  SYNOPSIS
*     static void log_context_destroy(void* theState) 
*
*  FUNCTION
*     Free thread local storage.
*
*  INPUTS
*     void* theState - Pointer to memroy which should be freed.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: log_context_destroy() is MT safe.
*
*******************************************************************************/
static void log_context_destroy(void* theContext)
{
   sge_free(&theContext);
}

/****** uti/log/log_context_getspecific() ****************************************
*  NAME
*     log_context_getspecific() -- Get thread local log context
*
*  SYNOPSIS
*     static log_context_t* log_context_getspecific() 
*
*  FUNCTION
*     Return thread local log context.
*
*     If a given thread does call this function for the first time, no thread
*     local log state is available for this particular thread. In this case the
*     thread local log state is allocated and set.
*
*  RESULT
*     static log_context_t* - Pointer to thread local log context.
*
*  NOTES
*     MT-NOTE: log_context_getspecific() is MT safe 
*
*******************************************************************************/
static log_context_t* log_context_getspecific(void)
{
   log_context_t *myctx = NULL;
   int res = -1;

   pthread_once(&log_context_once, log_context_once_init);

   if ((myctx = pthread_getspecific(log_context_key)) != NULL) {
      return myctx;
   }

   myctx = (log_context_t*)sge_malloc(sizeof(log_context_t));
   if (myctx != NULL) {
      myctx->context = NULL;
   }   
   res = pthread_setspecific(log_context_key, (const void*)myctx);

   if (0 != res) {
      fprintf(stderr, "pthread_set_specific(%s) failed: %s\n", "log_context_getspecific", strerror(res));
      abort();
   }
   
   return myctx;
} /* log_context_getspecific() */

