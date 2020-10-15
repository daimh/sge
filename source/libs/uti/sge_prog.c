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

#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>  

#include "uti/sge_rmon.h"
#include "uti/sge_hostname.h"
#include "uti/sge_log.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"
#include "uti/sge_unistd.h"
#include "uti/sge_prog.h"
#include "uti/sge_error_class.h"
#include "uti/sge_uidgid.h"
#include "uti/msg_utilib.h"

#include "sgeobj/sge_answer.h"

#include "sge.h"

/* Must match Qxxx defines in sge_prog.h */
const char *prognames[] =
{
   "unknown"	,
   "qalter"		,       /* 1   */
   "qconf"		,       /* 2   */
   "qdel"		,       /* 3   */
   "qhold"		,       /* 4   */
   "qmaster"	,       /* 5   */
   "qmod"		,       /* 6   */
   "qresub"    ,       /* 7   */
   "qrls"		,       /* 8   */
   "qselect"	,       /* 9   */
   "qsh"		   ,       /* 10  */
   "qrsh"		,       /* 11  */
   "qlogin"		,       /* 12  */
   "qstat"		,       /* 13  */
   "qsub"		,       /* 14  */
   "execd"		,       /* 15  */
   "qevent"    ,       /* 16  */
   "qrsub"     ,       /* 17  */
   "qrdel"     ,       /* 18  */
   "qrstat"    ,       /* 19  */
   "unknown"	,       /* 20  */
   "unknown"	,       /* 21  */
   "qmon"		,       /* 22  */
   "schedd"		,       /* 23  */
   "qacct"		,       /* 24  */
   "shadowd"	,       /* 25  */
   "qhost"		,       /* 26  */
   "spoolinit" ,       /* 27  */
   "japi"      ,       /* 28  */
   "japi_ec"   ,       /* 29  */
   "drmaa"     ,       /* 30  */
   "qping"     ,       /* 31  */ 
   "sgepasswd" ,       /* 32  */
   "qquota"    ,       /* 33  */
   "jgdi"      ,       /* 34  */
   "qtcsh"     ,       /* 35  */
   "sge_share_mon"     /* 36  */
};

const char *threadnames[] = {
   "main",              /* 1 */
   "listener",          /* 2 */
   "event_master",      /* 3 */
   "timer",             /* 4 */
   "worker",            /* 5 */
   "signaler",          /* 6 */
   "jvm",               /* 7 */
   "scheduler",         /* 8 */
   "tester"             /* 9 */
};

 
typedef struct prog_state_str {
   char*           sge_formal_prog_name;  /* taken from prognames[] */
   char*           qualified_hostname;
   char*           unqualified_hostname;
   u_long32        who;                   /* Qxxx defines  QUSERDEFINED  */
   u_long32        uid;
   u_long32        gid;
   bool             daemonized;
   char*           user_name;
   /* TODO: remove from prog_state, saved in sge_env_state_t */
   char*           default_cell;
   sge_exit_func_t exit_func;
   bool            exit_on_error;
} prog_state_t;


static pthread_once_t prog_once = PTHREAD_ONCE_INIT;
static pthread_key_t  prog_state_key;

static void          prog_once_init(void);
static void          prog_state_destroy(void *theState);
static prog_state_t* prog_state_getspecific(pthread_key_t aKey);
static void          prog_state_init(prog_state_t *theState);

static void sge_show_me(void); 
static void uti_state_set_sge_formal_prog_name(const char *s);
static void uti_state_set_uid(u_long32 uid);
static void uti_state_set_gid(u_long32 gid);
static void uti_state_set_user_name(const char *s);
static void uti_state_set_default_cell(const char *s);



typedef struct prog_state_str sge_prog_state_t;

static bool sge_prog_state_setup(sge_prog_state_class_t *thiz, sge_env_state_class_t *sge_env, u_long32 program_number, sge_error_class_t *eh);
static void sge_prog_state_dprintf(sge_prog_state_class_t *thiz);
static const char* get_sge_formal_prog_name(sge_prog_state_class_t *thiz);
static const char* get_qualified_hostname(sge_prog_state_class_t *thiz);
static const char* get_unqualified_hostname(sge_prog_state_class_t *thiz);
static u_long32 get_who(sge_prog_state_class_t *thiz);
static u_long32 get_uid(sge_prog_state_class_t *thiz);
static u_long32 get_gid(sge_prog_state_class_t *thiz);
static bool get_daemonized(sge_prog_state_class_t *thiz);
static const char* get_user_name(sge_prog_state_class_t *thiz);
static const char* get_default_cell(sge_prog_state_class_t *thiz);
static bool get_exit_on_error(sge_prog_state_class_t *thiz);
static sge_exit_func_t get_exit_func(sge_prog_state_class_t *thiz);
static void set_sge_formal_prog_name(sge_prog_state_class_t *thiz, const char *prog_name);
static void set_qualified_hostname(sge_prog_state_class_t *thiz, const char *qualified_hostname);
static void set_unqualified_hostname(sge_prog_state_class_t *thiz, const char *unqualified_hostname);
static void set_who(sge_prog_state_class_t *thiz, u_long32 who);
static void set_uid(sge_prog_state_class_t *thiz, u_long32 uid);
static void set_gid(sge_prog_state_class_t *thiz, u_long32 gid);
static void set_daemonized(sge_prog_state_class_t *thiz, bool daemonized);
static void set_user_name(sge_prog_state_class_t *thiz, const char* user_name);
static void set_default_cell(sge_prog_state_class_t *thiz, const char* default_cell);
static void set_exit_on_error(sge_prog_state_class_t *thiz, bool exit_on_error);
static void set_exit_func(sge_prog_state_class_t *thiz, sge_exit_func_t exit_func);

/****** uti/prog/uti_state_get_????() ************************************
*  NAME
*     uti_state_get_????() - read access to utilib global variables
*
*  FUNCTION
*     Provides access to either global variable or per thread global variable.
******************************************************************************/
const char *uti_state_get_sge_formal_prog_name(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->sge_formal_prog_name;
}

const char *uti_state_get_qualified_hostname(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->qualified_hostname;
}

const char *uti_state_get_unqualified_hostname(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->unqualified_hostname;
}

u_long32 uti_state_get_mewho(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->who;
}

u_long32 uti_state_get_uid(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->uid;
}

u_long32 uti_state_get_gid(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->gid;
}

int uti_state_get_daemonized(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->daemonized;
}

const char *uti_state_get_user_name(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->user_name;
}

const char *uti_state_get_default_cell(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->default_cell;
}

bool uti_state_get_exit_on_error(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->exit_on_error;
}

/****** uti/prog/uti_state_set_????() ************************************
*  NAME
*     uti_state_set_????() - write access to utilib global variables
*
*  FUNCTION
*     Provides access to either global variable or per thread global variable.
******************************************************************************/
static void uti_state_set_sge_formal_prog_name(const char *s)
{
   prog_state_t *prog_state = NULL;

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->sge_formal_prog_name = sge_strdup(prog_state->sge_formal_prog_name, s);

   return;
}

void uti_state_set_qualified_hostname(const char *s)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->qualified_hostname = sge_strdup(prog_state->qualified_hostname, s);

   return;
}

void uti_state_set_unqualified_hostname(const char *s)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->unqualified_hostname = sge_strdup(prog_state->unqualified_hostname, s);

   return;
}

void uti_state_set_daemonized(int daemonized)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->daemonized = daemonized ? true : false;

   return;
}

void uti_state_set_mewho(u_long32 who)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->who = who;

   return;
}

static void uti_state_set_uid(u_long32 uid)
{
   prog_state_t *prog_state = NULL;

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->uid = uid;

   return;
}

static void uti_state_set_gid(u_long32 gid)
{
   prog_state_t *prog_state = NULL;

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->gid = gid;

   return;
}

static void uti_state_set_user_name(const char *s)
{
   prog_state_t *prog_state = NULL;

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->user_name = sge_strdup(prog_state->user_name, s);

   return;
}

static void uti_state_set_default_cell(const char *s)
{
   prog_state_t *prog_state = NULL;

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->default_cell = sge_strdup(prog_state->default_cell, s);

   return;
}

void uti_state_set_exit_on_error(bool i)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->exit_on_error = i;

   return;
}

/****** uti/prog/uti_state_get_exit_func() ************************************
*  NAME
*     uti_state_get_exit_func() -- Return installed exit funciton 
*
*  SYNOPSIS
*     sge_exit_func_t uti_state_get_exit_func(void)
*
*  FUNCTION
*     Returns installed exit funciton. Exit function
*     will be called be sge_exit()
*
*  RESULT
*     sge_exit_func_t - function pointer 
*
*  SEE ALSO
*     uti/unistd/sge_exit() 
******************************************************************************/
sge_exit_func_t uti_state_get_exit_func(void)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   return prog_state->exit_func;
}

/****** uti/prog/uti_state_set_exit_func() ************************************
*  NAME
*     uti_state_set_exit_func() -- Installs a new exit handler 
*
*  SYNOPSIS
*     void uti_state_set_exit_func(sge_exit_func_t f)
*
*  FUNCTION
*     Installs a new exit handler. Exit function will be called be sge_exit()
*
*  INPUTS
*     sge_exit_func_t f - new function pointer 
*
*  SEE ALSO
*     uti/unistd/sge_exit() 
******************************************************************************/
void uti_state_set_exit_func(sge_exit_func_t f)
{
   prog_state_t *prog_state = NULL;

   pthread_once(&prog_once, prog_once_init);

   prog_state = prog_state_getspecific(prog_state_key);

   prog_state->exit_func = f;

   return;
}

/****** uti/prog/sge_getme() *************************************************
*  NAME
*     sge_getme() -- Initialize me-struct
*
*  SYNOPSIS
*     void sge_getme(u_long32 program_number)
*
*  FUNCTION
*     Initialize me-struct according to 'program_number'
*
*  INPUTS
*     u_long32 program_number - uniq internal program number
*
*  NOTES
*     MT-NOTE: sge_getme() is MT safe
******************************************************************************/
void sge_getme(u_long32 program_number)
{
   char *s = NULL;
   stringT tmp_str;
   struct hostent *hent = NULL;
 
   DENTER(TOP_LAYER, "sge_getme");
 
   pthread_once(&prog_once, prog_once_init);

   /* we need to detect cases when sge_getme() is called multiple times
      for a thread. Due to sge_getme() doing an abort(2) upon user name
      resolution we can utilize the user name as an indication sge_getme()
      was already called and safe the effort of doing it again */
   if (uti_state_get_user_name()!=NULL) {
      DEXIT;
      return;
   }

   /* get program info */
   uti_state_set_mewho(program_number);
   uti_state_set_sge_formal_prog_name(prognames[program_number]);

   /* Fetch hostnames */
   SGE_ASSERT((gethostname(tmp_str, sizeof(tmp_str)) == 0));
   SGE_ASSERT(((hent = sge_gethostbyname(tmp_str,NULL)) != NULL));

   DTRACE;

   uti_state_set_qualified_hostname(hent->h_name);
   s = sge_dirname(hent->h_name, '.');
   uti_state_set_unqualified_hostname(s);
   sge_free(&s);

   DTRACE;
 
   /* Bad resolving in some networks leads to short qualified host names */
   if (!strcmp(uti_state_get_qualified_hostname(), uti_state_get_unqualified_hostname())) {
      char tmp_addr[8];
      struct hostent *hent2 = NULL;
      memcpy(tmp_addr, hent->h_addr, hent->h_length);
      DTRACE;
      SGE_ASSERT(((hent2 = sge_gethostbyaddr((const struct in_addr *)tmp_addr, NULL)) != NULL));
      DTRACE;

      uti_state_set_qualified_hostname(hent2->h_name);
      s = sge_dirname(hent2->h_name, '.');
      uti_state_set_unqualified_hostname(s);
      sge_free(&s);
      sge_free_hostent(&hent2);
   }

   sge_free_hostent(&hent);
   DTRACE;

   /* SETPGRP; */
   uti_state_set_uid(getuid());
   uti_state_set_gid(getgid());

   {
      struct passwd *paswd = NULL;
      char *buffer;
      int size;
      struct passwd pwentry;

      size = get_pw_buffer_size();
      buffer = sge_malloc(size);
      SGE_ASSERT(getpwuid_r((uid_t)uti_state_get_uid(), &pwentry, buffer, size, &paswd) == 0 && paswd)
      uti_state_set_user_name(paswd->pw_name);
      sge_free(&buffer);
   }

   uti_state_set_default_cell(sge_get_default_cell());
 
   sge_show_me();
 
   DEXIT;
   return;
}

/****** uti/prog/sge_show_me() ************************************************
*  NAME
*     sge_show_me() -- Show content of me structure
*
*  SYNOPSIS
*     static void sge_show_me()
*
*  FUNCTION
*     Show content of me structure in debug output
*
*  NOTES
*     MT-NOTE: sge_show_me() is MT safe
******************************************************************************/
static void sge_show_me(void)
{
   DENTER(TOP_LAYER, "sge_show_me");
 
#ifdef NO_SGE_COMPILE_DEBUG
   return;
#else
   if (!TRACEON) {
      DEXIT;
      return;
   }
#endif
 
   DPRINTF(("me.who                      >%d<\n", (int) uti_state_get_mewho()));
   DPRINTF(("me.sge_formal_prog_name     >%s<\n", uti_state_get_sge_formal_prog_name()));
   DPRINTF(("me.qualified_hostname       >%s<\n", uti_state_get_qualified_hostname()));
   DPRINTF(("me.unqualified_hostname     >%s<\n", uti_state_get_unqualified_hostname()));
   DPRINTF(("me.uid                      >%d<\n", (int) uti_state_get_uid()));
   DPRINTF(("me.gid                      >%d<\n", (int) uti_state_get_gid()));
   DPRINTF(("me.daemonized               >%d<\n", uti_state_get_daemonized()));
   DPRINTF(("me.user_name                >%s<\n", uti_state_get_user_name()));
   DPRINTF(("me.default_cell             >%s<\n", uti_state_get_default_cell()));
 
   DEXIT;
   return;
}     

/****** uti/prog/prog_once_init() *********************************************
*  NAME
*     prog_once_init() -- One-time executable state initialization.
*
*  SYNOPSIS
*     static prog_once_init(void) 
*
*  FUNCTION
*     Create access key for thread local storage. Register cleanup function.
*
*     This function must be called exactly once.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: prog_once_init() is MT safe. 
*
*******************************************************************************/
static void prog_once_init(void)
{
   pthread_key_create(&prog_state_key, &prog_state_destroy);
   return;
} /* prog_once_init() */

/****** uti/prog/prog_state_destroy() ******************************************
*  NAME
*     prog_state_destroy() -- Free thread local storage
*
*  SYNOPSIS
*     static void prog_state_destroy(void* theState) 
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
*     MT-NOTE: prog_state_destroy() is MT safe.
*
*******************************************************************************/
static void prog_state_destroy(void *theState)
{
   prog_state_t *s = (prog_state_t *)theState;

   sge_free(&(s->sge_formal_prog_name));
   sge_free(&(s->sge_formal_prog_name));
   sge_free(&(s->qualified_hostname));
   sge_free(&(s->unqualified_hostname));
   sge_free(&(s->user_name));
   sge_free(&(s->default_cell));
   sge_free(&s);
}

/****** uti/prog/prog_state_getspecific() **************************************
*  NAME
*     prog_state_getspecific() -- Get thread local prog state 
*
*  SYNOPSIS
*     static prog_state_t* prog_state_getspecific(pthread_key_t aKey) 
*
*  FUNCTION
*     Return thread local prog state. 
*
*     If a given thread does call this function for the first time, no thread
*     local prog state is available for this particular thread. In this case the
*     thread local prog state is allocated and set.
*
*  INPUTS
*     pthread_key_t aKey - Key for thread local prog state. 
*
*  RESULT
*     static prog_state_t* - Pointer to thread local prog state
*
*  NOTES
*     MT-NOTE: prog_state_getspecific() is MT safe 
*
*******************************************************************************/
static prog_state_t* prog_state_getspecific(pthread_key_t aKey)
{
   prog_state_t *prog_state = NULL;
   int res = EINVAL;

   if ((prog_state = pthread_getspecific(aKey)) != NULL) { return prog_state; }

   prog_state = (prog_state_t*)sge_malloc(sizeof(prog_state_t));

   prog_state_init(prog_state);

   res = pthread_setspecific(prog_state_key, (const void*)prog_state);

   if (0 != res) {
      fprintf(stderr, "pthread_set_specific(%s) failed: %s\n", "prog_state_getspecific", strerror(res));
      abort();
   }
   
   return prog_state;
} /* prog_state_getspecific() */

/****** uti/prog/prog_state_init() *******************************************
*  NAME
*     prog_state_init() -- Initialize executable state.
*
*  SYNOPSIS
*     static void prog_state_init(prog_state_t *theState) 
*
*  FUNCTION
*     Initialize executable state.
*
*  INPUTS
*     struct prog_state_t* theState - Pointer to executable state structure.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: prog_state_init() in MT safe. 
*
*******************************************************************************/
static void prog_state_init(prog_state_t *theState)
{
   theState->sge_formal_prog_name = NULL;
   theState->qualified_hostname = NULL;
   theState->unqualified_hostname = NULL;
   theState->who = QUSERDEFINED;
   theState->uid = 0;
   theState->gid = 0;
   theState->daemonized = false;
   theState->user_name = NULL;
   theState->default_cell = NULL;
   theState->exit_func = NULL;
   theState->exit_on_error = true;

   return;
}


/*-------------------------------------------------------------------------*/
sge_prog_state_class_t *
sge_prog_state_class_create(sge_env_state_class_t *sge_env, 
                            u_long32 program_number, sge_error_class_t *eh)
{
   sge_prog_state_class_t *ret = (sge_prog_state_class_t *)sge_malloc(sizeof(sge_prog_state_class_t));

   DENTER(TOP_LAYER, "sge_prog_state_class_create");

   if (!ret) {
      eh->error(eh, STATUS_EMALLOC, ANSWER_QUALITY_ERROR, "%s",
                MSG_MEMORY_MALLOCFAILED);
      DEXIT;
      return NULL;
   }   
   
   ret->dprintf = sge_prog_state_dprintf;
   
   ret->get_sge_formal_prog_name = get_sge_formal_prog_name;
   ret->get_qualified_hostname = get_qualified_hostname;
   ret->get_unqualified_hostname = get_unqualified_hostname;
   ret->get_who = get_who;
   ret->get_uid = get_uid;
   ret->get_gid = get_gid;
   ret->get_daemonized = get_daemonized;
   ret->get_user_name = get_user_name;
   ret->get_default_cell = get_default_cell;
   ret->get_exit_on_error = get_exit_on_error;
   ret->get_exit_func = get_exit_func;

   ret->set_sge_formal_prog_name = set_sge_formal_prog_name;
   ret->set_qualified_hostname = set_qualified_hostname;
   ret->set_unqualified_hostname = set_unqualified_hostname;
   ret->set_who = set_who;
   ret->set_uid = set_uid;
   ret->set_gid = set_gid;
   ret->set_daemonized = set_daemonized;
   ret->set_user_name = set_user_name;
   ret->set_default_cell = set_default_cell;
   ret->set_exit_on_error = set_exit_on_error;
   ret->set_exit_func = set_exit_func;

   ret->sge_prog_state_handle = sge_malloc(sizeof(sge_prog_state_t));
   if (ret->sge_prog_state_handle == NULL) {
      eh->error(eh, STATUS_EMALLOC, ANSWER_QUALITY_ERROR, "%s",
                MSG_MEMORY_MALLOCFAILED);
      sge_free(&ret);
      DEXIT;
      return NULL;
   }
   memset(ret->sge_prog_state_handle, 0, sizeof(sge_prog_state_t));
   
   if (!sge_prog_state_setup(ret, sge_env, program_number, eh)) {
      sge_prog_state_class_destroy(&ret);
      DEXIT;
      return NULL;
   }

   DEXIT;
   return ret;
}   

void sge_prog_state_class_destroy(sge_prog_state_class_t **pst)
{
   DENTER(TOP_LAYER, "sge_prog_state_class_destroy");
   if (!pst || !*pst) {
      DEXIT;
      return;
   }   
      
   prog_state_destroy((*pst)->sge_prog_state_handle);
   sge_free(pst);
   DEXIT;
}


static bool sge_prog_state_setup(sge_prog_state_class_t *thiz, sge_env_state_class_t *sge_env, u_long32 program_number, sge_error_class_t *eh)
{
   stringT tmp_str;
   bool ret = true;

   DENTER(TOP_LAYER, "sge_prog_state_setup");
 
   thiz->set_who(thiz, program_number);
   thiz->set_sge_formal_prog_name(thiz, prognames[program_number]);
   thiz->set_default_cell(thiz, sge_env->get_sge_cell(sge_env));

   if (gethostname(tmp_str, sizeof(tmp_str)) != 0) {
      eh->error(eh, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, "gethostname failed %s", tmp_str);
      ret = false;
   }
   if (ret) {
      char *unqualified_hostname = NULL;
      char *qualified_hostname = NULL;
      struct hostent *hent = NULL;
      if (!(hent = sge_gethostbyname(tmp_str, NULL))) {
         eh->error(eh, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, "sge_gethostbyname failed");
         ret = false;
      } else {
         unqualified_hostname = sge_dirname(hent->h_name, '.');
         if (!strcmp(hent->h_name, unqualified_hostname)) {
            char tmp_addr[8];
            struct hostent *hent2 = NULL;
            memcpy(tmp_addr, hent->h_addr, hent->h_length);
            if (!(hent2 = sge_gethostbyaddr((const struct in_addr *)&tmp_addr, NULL))) {
               eh->error(eh, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, "sge_gethostbyaddr failed");
               ret = false;
            } else {
               qualified_hostname = sge_strdup(NULL, hent2->h_name);
               sge_free(&unqualified_hostname);
               unqualified_hostname = sge_dirname(hent2->h_name, '.');
               sge_free_hostent(&hent2);
            }
         } else {
            qualified_hostname = sge_strdup(qualified_hostname, unqualified_hostname);
         }
         sge_free_hostent(&hent);
      }
      thiz->set_qualified_hostname(thiz, qualified_hostname);
      thiz->set_unqualified_hostname(thiz, unqualified_hostname);
      sge_free(&unqualified_hostname);
      sge_free(&qualified_hostname);
   }

   if (ret) {
      struct passwd *paswd = NULL;
      char *buffer;
      size_t size;
      struct passwd pwentry;

      thiz->set_uid(thiz, getuid());
      thiz->set_gid(thiz, getgid());

      size = get_pw_buffer_size();
      buffer = sge_malloc(size);
      if ((getpwuid_r((uid_t)getuid(), &pwentry, buffer, size, &paswd)) != 0
          && paswd) {
         eh->error(eh, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, "getpwuid failed");
         ret = false;
      } else {
         thiz->set_user_name(thiz, paswd->pw_name);
      }
      sge_free(&buffer);
   }

   /*
   if (ret) {
      thiz->dprintf(thiz);
   }*/

   DEXIT;
   return ret;
}

static void sge_prog_state_dprintf(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;

   DENTER(TOP_LAYER, "sge_prog_state_dprintf");

   DPRINTF(("who                      >"sge_u32"<\n", ps->who));
   DPRINTF(("sge_formal_prog_name     >%s<\n", ps->sge_formal_prog_name));
   DPRINTF(("qualified_hostname       >%s<\n", ps->qualified_hostname));
   DPRINTF(("unqualified_hostname     >%s<\n", ps->unqualified_hostname));
   DPRINTF(("uid                      >"sge_u32"<\n", ps->uid));
   DPRINTF(("gid                      >"sge_u32"<\n", ps->gid));
   DPRINTF(("daemonized               >%d<\n", ps->daemonized));
   DPRINTF(("user_name                >%s<\n", ps->user_name));
   DPRINTF(("default_cell             >%s<\n", ps->default_cell));

   DEXIT;
}   

static const char* get_sge_formal_prog_name(sge_prog_state_class_t *thiz) 
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->sge_formal_prog_name;
}

static const char* get_qualified_hostname(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->qualified_hostname;
}

static const char* get_unqualified_hostname(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->unqualified_hostname;
}

static u_long32 get_who(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->who;
}

static u_long32 get_uid(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->uid;
}

static u_long32 get_gid(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->gid;
}

static bool get_daemonized(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->daemonized;
}

static const char* get_user_name(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->user_name;
}

static const char* get_default_cell(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->default_cell;
}

static bool get_exit_on_error(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->exit_on_error;
}

static sge_exit_func_t get_exit_func(sge_prog_state_class_t *thiz)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   return ps->exit_func;
}


static void set_sge_formal_prog_name(sge_prog_state_class_t *thiz, const char *prog_name)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->sge_formal_prog_name = sge_strdup(ps->sge_formal_prog_name, prog_name);
}

static void set_qualified_hostname(sge_prog_state_class_t *thiz, const char *qualified_hostname)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->qualified_hostname = sge_strdup(ps->qualified_hostname, qualified_hostname);
}

static void set_unqualified_hostname(sge_prog_state_class_t *thiz, const char *unqualified_hostname)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->unqualified_hostname = sge_strdup(ps->unqualified_hostname, unqualified_hostname);
}

static void set_who(sge_prog_state_class_t *thiz, u_long32 who)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->who = who; 
}

static void set_uid(sge_prog_state_class_t *thiz, u_long32 uid)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->uid = uid; 
}

static void set_gid(sge_prog_state_class_t *thiz, u_long32 gid)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->gid = gid; 
}

static void set_daemonized(sge_prog_state_class_t *thiz, bool daemonized)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->daemonized = daemonized; 
}

static void set_user_name(sge_prog_state_class_t *thiz, const char* user_name)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->user_name = sge_strdup(ps->user_name, user_name);
}

static void set_default_cell(sge_prog_state_class_t *thiz, const char* default_cell)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->default_cell = sge_strdup(ps->default_cell, default_cell);
}

static void set_exit_on_error(sge_prog_state_class_t *thiz, bool exit_on_error)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->exit_on_error = exit_on_error; 
}

static void set_exit_func(sge_prog_state_class_t *thiz, sge_exit_func_t exit_func)
{
   sge_prog_state_t *ps = (sge_prog_state_t *) thiz->sge_prog_state_handle;
   ps->exit_func = exit_func; 
}

