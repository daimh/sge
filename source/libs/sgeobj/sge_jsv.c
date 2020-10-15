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
 *   Copyright: 2008 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/                                   

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>

#include "sge.h"

#include "uti/sge_mtutil.h"
#include "uti/sge_lock.h"
#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h"
#include "uti/sge_stdio.h"
#include "uti/sge_string.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_time.h"
#include "uti/sge_unistd.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_jsv.h"
#include "sgeobj/sge_jsv_script.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/msg_sgeobjlib.h"

/*
 * defines the timeout between the soft shutdown signal and the kill
 */
#define JSV_QUIT_TIMEOUT (5)

/*
 * This mutex is used to secure the jsv_list below.
 */
static pthread_mutex_t jsv_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Each element in this list contains data which shows the state
 * of the corresponding JSV. The order of the elements in the list 
 * is important because the JSVs are executed in that order.
 */
static lList *jsv_list = NULL;   /* JSV_Type */

/****** sgeobj/jsv/jsv_create() ***************************************************
*  NAME
*     jsv_create() -- creates a new JSV object and initializes its attributes 
*
*  SYNOPSIS
*     static lListElem * 
*     jsv_create(const char *name, const char *context, lList **answer_list, 
*                const char *jsv_url, const char *type, const char *user, 
*                const char *scriptfile) 
*
*  FUNCTION
*     Returns a new JSV instance. 
*
*  INPUTS
*     const char *name       - Name of the new JSV element
*     const char *context    - JSV_CONTEXT_CLIENT or thread name
*     lList **answer_list    - AN_Type list 
*     const char *jsv_url    - JSV URL 
*     const char *type       - type part of 'jsv_url' 
*     const char *user       - user part of 'jsv_url'
*     const char *scriptfile - path part of 'jsv_url'
*
*  RESULT
*     static lListElem * - JSV_Type element
*
*  NOTES
*     MT-NOTE: jsv_create() is MT safe 
*******************************************************************************/
static lListElem *
jsv_create(const char *name, const char *context, lList **answer_list, const char *jsv_url,
           const char *type, const char *user, const char *scriptfile)
{
   lListElem *new_jsv = NULL;

   DENTER(TOP_LAYER, "jsv_create");
   if (name != NULL && scriptfile != NULL) {
      new_jsv = lCreateElem(JSV_Type);

      if (new_jsv != NULL) {
         SGE_STRUCT_STAT st;

         if (SGE_STAT(scriptfile, &st) == 0) {
            char pid_buffer[256];

            snprintf(pid_buffer, sizeof(pid_buffer), pid_t_fmt, (pid_t)-1);
            lSetString(new_jsv, JSV_name, name);
            lSetString(new_jsv, JSV_context, context);
            lSetString(new_jsv, JSV_url, jsv_url);
            lSetString(new_jsv, JSV_type, type);
            lSetString(new_jsv, JSV_user, user);
            lSetString(new_jsv, JSV_command, scriptfile);
            lSetString(new_jsv, JSV_pid, pid_buffer);
            lSetBool(new_jsv, JSV_send_env, false);
            lSetRef(new_jsv, JSV_in, NULL);
            lSetRef(new_jsv, JSV_out, NULL);
            lSetRef(new_jsv, JSV_err, NULL);
            lSetBool(new_jsv, JSV_has_to_restart, false);
            lSetUlong(new_jsv, JSV_last_mod, st.st_mtime);
            lSetBool(new_jsv, JSV_test, false);

            sge_mutex_lock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);

            if (jsv_list == NULL) {
               jsv_list = lCreateList("", JSV_Type);
            }
            if (jsv_list != NULL) {
               lInsertElem(jsv_list, NULL, new_jsv);
            } else {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                       MSG_JSV_INSTANCIATE_S, scriptfile);
            }

            sge_mutex_unlock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);

         } else {
            lFreeElem(&new_jsv);
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                    MSG_JSV_EXISTS_S, scriptfile);
         }
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 MSG_JSV_INSTANCIATE_S, scriptfile);
      } 
   }
   DRETURN(new_jsv);
}

static pid_t
jsv_get_pid(lListElem *jsv) {
   pid_t pid = -1;
   const char *pid_string = NULL;

   DENTER(TOP_LAYER, "jsv_get_pid");
   pid_string = lGetString(jsv, JSV_pid);
   if (pid_string != NULL) {
      sscanf(pid_string, pid_t_fmt, &pid);
   }
   DRETURN(pid);
}

static void 
jsv_set_pid(lListElem *jsv, pid_t pid)
{
   char pid_buffer[256]; /* it is sure that pid is smaller than 256 characters */

   DENTER(TOP_LAYER, "jsv_set_pid");
   snprintf(pid_buffer, sizeof(pid_buffer), pid_t_fmt, pid);
   lSetString(jsv, JSV_pid, pid_buffer);
   DRETURN_VOID;
}


static bool
jsv_is_started(lListElem *jsv) 
{
   return (jsv_get_pid(jsv) != -1) ? true : false; 
}

static bool
jsv_is_send_ready(lListElem *jsv) {
   bool ret = false;
   const int timeout = 5;
   int fd;
   int lret;

   DENTER(TOP_LAYER, "jsv_is_send_ready");
   
   fd = fileno((FILE *) lGetRef(jsv, JSV_in));

   {
      struct pollfd pfds;
      memset(&pfds, 0, sizeof(struct pollfd));
      pfds.fd = fd;
      pfds.events |= POLLOUT;
      lret = poll(&pfds, 1, timeout * 1000);
      if (lret != -1 && lret != 0) {
         if (pfds.revents & POLLOUT) {
            ret = true;
         }
      }
   }

   if (ret == true) {
      DPRINTF(("JSV - fd is ready. Data can be sent\n"));
   } else {
      DPRINTF(("JSV - fd is NOT ready\n"));
   }
   DRETURN(ret);
}

static bool
jsv_send_data(lListElem *jsv, lList **answer_list, const char *buffer, size_t size) {
   bool ret = true;

   DENTER(TOP_LAYER, "jsv_send_data");
   if (jsv_is_send_ready(jsv)) {
      size_t lret;

      lret = fprintf(lGetRef(jsv, JSV_in), "%s", buffer);
      fflush(lGetRef(jsv, JSV_in));
      if (lret != size) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_JSV_SEND);
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "%s", MSG_JSV_SEND_READY);
      ret = false;
   } 
   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_start() **********************************************
*  NAME
*     jsv_start() -- Start a JSV instance
*
*  SYNOPSIS
*     bool jsv_start(lListElem *jsv, lList **answer_list) 
*
*  FUNCTION
*     A call to this function starts a new JSV instance if it is 
*     currently not running. Handles to pipe ends will be stored
*     internally so that it is possible to communicate with the
*     process which was started by this function.
*
*  INPUTS
*     lListElem *jsv      - JSV_Type data structure 
*     lList **answer_list - AN_Type list where error messages are stored. 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: jsv_start() is not MT safe 
*
*  SEE ALSO
*     sgeobj/jsv/jsv_start 
*******************************************************************************/
bool 
jsv_start(lListElem *jsv, lList **answer_list) 
{
   bool ret = true;

   DENTER(TOP_LAYER, "jsv_start");
   if (jsv != NULL && jsv_is_started(jsv) == false) {
      const char *scriptfile = lGetString(jsv, JSV_command);
      const char *user = lGetString(jsv, JSV_user);
      pid_t pid = -1;
      FILE *fp_in = NULL;
      FILE *fp_out = NULL;
      FILE *fp_err = NULL;
      SGE_STRUCT_STAT st;

      if (SGE_STAT(scriptfile, &st) == 0) {
         /* set the last modification time of the script */
         lSetUlong(jsv, JSV_last_mod, st.st_mtime);

         pid = sge_peopen_r("/bin/sh", 0, scriptfile, 
                            (user != NULL ? user : get_admin_user_name()), NULL,
                            &fp_in, &fp_out, &fp_err, false);
         if (pid == -1) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                    MSG_JSV_START_S, scriptfile);
            ret = false;
         } else if (pid == -2) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                    "%s", MSG_JSV_STARTPERMISSION);
            ret = false;
         } else {
            jsv_set_pid(jsv, pid);
            lSetRef(jsv, JSV_in, fp_in);
            lSetRef(jsv, JSV_out, fp_out);
            lSetRef(jsv, JSV_err, fp_err);

            /* we need it non blocking */
            fcntl(fileno(fp_out), F_SETFL, O_NONBLOCK);
            fcntl(fileno(fp_err), F_SETFL, O_NONBLOCK);

            INFO((SGE_EVENT, MSG_JSV_STARTED_S, scriptfile));
         }
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 MSG_JSV_EXISTS_S, scriptfile);
         ret = false;
      }
   }
   DRETURN(ret);
}

/****** sge_jsv/jsv_stop() *****************************************************
*  NAME
*     jsv_stop() -- Stop a JSV instance which was previously started 
*
*  SYNOPSIS
*     bool jsv_stop(lListElem *jsv, lList **answer_list, bool try_soft_quit) 
*
*  FUNCTION
*     Stop a running JSV instance which was previously started. If the
*     variable 'try_soft_quit' is set to 'true' then this function tries
*     to communicate with the process. It will then send a "QUIT" string
*     to pipe end which is connected with the stdin of the JSV process.
*     If the script does then not terminate within JSV_QUIT_TIMEOUT 
*     seconds then it will be terminated hard via kill signal.
*
*  INPUTS
*     lListElem *jsv      - JSV_Type element 
*     lList **answer_list - AN_Type element 
*     bool try_soft_quit  - "true" if the JSV should terminate itself 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: jsv_stop() is MT safe 
*
*  SEE ALSO
*     sgeobj/jsv/jsv_start() 
*******************************************************************************/
bool 
jsv_stop(lListElem *jsv, lList **answer_list, bool try_soft_quit) {
   bool ret = true;
   pid_t pid = -1;

   DENTER(TOP_LAYER, "jsv_stop");

   /* stop is only possible if it was started before */
   pid = jsv_get_pid(jsv);
   if (pid != -1) {
      const char *scriptfile = lGetString(jsv, JSV_command);
      struct timeval t;

      /* 
       * send a quit command to stop the JSV softly.
       * give it then JSV_QUIT_TIMEOUT seconds before
       * the kill is triggered (sge_peclose)
       */
      if (try_soft_quit) {
         jsv_send_command(jsv, answer_list, "QUIT");
         t.tv_sec = JSV_QUIT_TIMEOUT;
      } else {
         t.tv_sec = 0;
      }
  
      /*
       * kill the JSV process
       */ 
      sge_peclose(pid, lGetRef(jsv, JSV_in), 
                  lGetRef(jsv, JSV_out), lGetRef(jsv, JSV_err), &t);

      INFO((SGE_EVENT, MSG_JSV_STOPPED_S, scriptfile));

      jsv_set_pid(jsv, -1);
   }   
   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_url_parse() ************************************************
*  NAME
*     jsv_url_parse() -- parses a JSV URL 
*
*  SYNOPSIS
*     bool 
*     jsv_url_parse(dstring *jsv_url, lList **answer_list, dstring *type, 
*                   dstring *user, dstring *path, bool in_client) 
*
*  FUNCTION
*     This function parses the given 'jsv_url' and fills the dstring 'type', 
*     'user' and 'path' with the parsed elements. If one of the elements was
*     not specified with the JSV URL then the corresponding variable will 
*     not be changed. 
*
*     A JSV URL has following format:
*
*        jsv_url := client_jsv_url | server_jsv_url ;
*        server_jsv_url := [ type ":" ] [ user "@" ] path ;
*        client_jsv_url := [ type ":" ] path ;
*
*     Within client JSVs it is not allowed to specify a user. This function
*     has to be used with value 'true' for the parameter 'in_client'. 
*     If an error happens during parsing then this function will fill
*     'answer_list' with a corresponding message and the function will
*     return with value 'false' otherwise 'true' will be returned.
*
*  INPUTS
*     dstring *jsv_url    - dstring containing a JSV url 
*     lList **answer_list - answer_list
*     dstring *type       - "script" 
*     dstring *user       - specified username or NULL 
*     dstring *path       - absolute path to a script or binary 
*     bool in_client      - true in command-line clients
*                           false within qmaster 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: jsv_url_parse() is MT safe 
*******************************************************************************/
bool jsv_url_parse(dstring *jsv_url, lList **answer_list, dstring *type, 
                   dstring *user, dstring *path, bool in_client)
{
   bool success = true;

   DENTER(TOP_LAYER, "jsv_url_parse");
   if (jsv_url != NULL) {
      dstring tmp = DSTRING_INIT;
      const char *t, *u, *p;

      /*
       * string till the first ':' is the type. then the user name follows
       * till the '@' character and after that we have the path name.
       * strip also white space at the and of each token silently.
       */ 
      sge_dstring_split(jsv_url, ':', type, &tmp);
      sge_dstring_split(&tmp, '@', user, path);
      sge_dstring_free(&tmp);
      sge_dstring_strip_white_space_at_eol(type);
      sge_dstring_strip_white_space_at_eol(user);
      sge_dstring_strip_white_space_at_eol(path);
      t = sge_dstring_get_string(type);
      u = sge_dstring_get_string(user);
      p = sge_dstring_get_string(path);

      DPRINTF(("type = %s\n", t != NULL? t : "<null>"));
      DPRINTF(("u = %s\n", u != NULL ? u : "<null>"));
      DPRINTF(("p = %s\n", p != NULL ? p : "<null>"));

      /*
       * either the type is not specified (the type "script" is used)
       * or "script" has to be specified. In future we might support 
       * "sharedlib" or others 
       */
      if ((t == NULL) ||
          (t != NULL && strcmp(t, "script") == 0)) {

         if (in_client && u != NULL) {
            answer_list_add_sprintf(answer_list, STATUS_EEXIST, 
                                    ANSWER_QUALITY_ERROR, "%s", MSG_JSV_USER);
         } else {
            if (p != NULL) {
               if ((sge_is_file(p) && sge_is_executable(p)) || strcasecmp("none", p) == 0) {
                  if (u != NULL) {
                     struct passwd *pw;
                     struct passwd pw_struct;
                     char *buffer;
                     int size;

                     size = get_pw_buffer_size();
                     buffer = sge_malloc(size);
                     pw = sge_getpwnam_r(u, &pw_struct, buffer, size);
                     sge_free(&buffer);
                     if (pw == NULL) {
                        answer_list_add_sprintf(answer_list, STATUS_EEXIST, ANSWER_QUALITY_ERROR,  
                                                 MSG_JSV_USER_EXIST_S, u);
                        success = false;
                     }
                  } else {
                     /* we have only type and a path this is ok because user is optional */
                     success = true;
                  }
               } else {
                  answer_list_add_sprintf(answer_list, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_FILE_EXEC_S, p);
                  success = false;
               }
            } else {
               answer_list_add_sprintf(answer_list, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                       MSG_JSV_URL_S, sge_dstring_get_string(jsv_url));
               success = false;
            }
         }
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EEXIST, ANSWER_QUALITY_ERROR, 
                                 MSG_JSV_URL_TYPE_S, t);
         success = false;
      }
   }
   DRETURN(success);
}

/****** sgeobj/jsv/jsv_send_command() *********************************************
*  NAME
*     jsv_send_command() -- sends a command to a JSV script 
*
*  SYNOPSIS
*     bool 
*     jsv_send_command(lListElem *jsv, lList **answer_list, const char *message) 
*
*  FUNCTION
*     Sends the 'message' to the 'jsv' script. If this fails then 
*     'answer_list' will be filled.
*
*  INPUTS
*     lListElem *jsv      - JSV element 
*     lList **answer_list - answer list 
*     const char *message - null terminated string 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - failed
*
*  NOTES
*     MT-NOTE: jsv_send_command() is MT safe 
*******************************************************************************/
bool 
jsv_send_command(lListElem *jsv, lList **answer_list, const char *message) 
{
   bool ret = true;
   dstring buffer = DSTRING_INIT;
   const char *new_message = NULL;

   DENTER(TOP_LAYER, "jsv_send_command");
   sge_dstring_sprintf(&buffer, "%s\n", message);
   new_message = sge_dstring_get_string(&buffer);
   DPRINTF(("JSV(%s) >> %s\n", lGetString(jsv, JSV_context), message));
   ret = jsv_send_data(jsv, answer_list, new_message, strlen(new_message));
   sge_dstring_free(&buffer);
   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_list_add() *************************************************
*  NAME
*     jsv_list_add() -- adds a new JSV  
*
*  SYNOPSIS
*     bool 
*     jsv_list_add(const char *name, const char *context, 
*                  lList **answer_list, const char *jsv_url) 
*
*  FUNCTION
*     By calling this function a new JSV element will be registered internally.
*     The JSV will be initialized with the values 'name", 'context', and
*     'jsv_url'. The 'name' and 'context' will be used to identify the JSV.
*     In command line clients the 'context' string JSV_CONTEXT_CLIENT should
*     be passed to this function. Within qmaster the name of the thread
*     which calls this function should be used as 'context' string.
*
*     The JSV data structures will be initialized but the JSV is not started
*     when this function returns. 
*
*     In case of any errors the 'answer_list' will be filled and the function
*     will return with the value 'false' instead of 'true'.
*
*
*  INPUTS
*     const char *name    - JSV name (used for error messages) 
*     const char *context - JSV context name (to identify this JSV)
*     lList **answer_list - AN_Type answer list
*     const char *jsv_url - JSV URL 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - false 
*
*  NOTES
*     MT-NOTE: jsv_list_add() is MT safe 
*
*******************************************************************************/
bool 
jsv_list_add(const char *name, const char *context, lList **answer_list, const char *jsv_url)
{
   bool ret = true;
   DENTER(TOP_LAYER, "jsv_list_add");

   if (strcasecmp("none", jsv_url) != 0) {
      lListElem *new_jsv = NULL;
      dstring input = DSTRING_INIT;
      dstring type = DSTRING_INIT;
      dstring user = DSTRING_INIT;
      dstring path = DSTRING_INIT; 
      bool in_client = false;

      sge_dstring_append(&input, jsv_url);
      in_client = (strcmp(context, JSV_CONTEXT_CLIENT) == 0) ? true : false;
      jsv_url_parse(&input, answer_list, &type, &user, &path, in_client);

      new_jsv = jsv_create(name, context, answer_list, jsv_url, 
                           sge_dstring_get_string(&type),
                           sge_dstring_get_string(&user),
                           sge_dstring_get_string(&path));
      if (new_jsv == NULL) {
         ret = false;
      }   

      /* cleanup */
      sge_dstring_free(&input);
      sge_dstring_free(&type);
      sge_dstring_free(&user);
      sge_dstring_free(&path);
   }

   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_list_remove() *******************************************
*  NAME
*     jsv_list_remove() -- remove a JSV script 
*
*  SYNOPSIS
*     bool jsv_list_remove(const char *name, const char *context) 
*
*  FUNCTION
*     Remove all JSV scripts were 'name' and 'context' matches. 
*
*  INPUTS
*     const char *name    - name of a JSV script 
*     const char *context - JSV_CLIENT_CONTEXT or thread name 
*
*  RESULT
*     bool - error status
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: jsv_list_remove() is MT safe 
*******************************************************************************/
bool 
jsv_list_remove(const char *name, const char *context)
{
   bool ret = true;

   DENTER(TOP_LAYER, "jsv_list_remove");
   if (name != NULL && context != NULL) {
      const void *iterator = NULL;
      lListElem *jsv_next;
      lListElem *jsv;

      sge_mutex_lock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
      jsv_next = lGetElemStrFirst(jsv_list, JSV_context, context, &iterator);
      while ((jsv = jsv_next) != NULL) {
         jsv_next = lGetElemStrNext(jsv_list, JSV_context, context, &iterator);

         if ((strcmp(lGetString(jsv, JSV_name), name) == 0) &&
             (strcmp(lGetString(jsv, JSV_context), context) == 0)) {
            lRemoveElem(jsv_list, &jsv);
         }
      }
      sge_mutex_unlock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
   }
   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_is_enabled() ********************************************
*  NAME
*     jsv_is_enabled() -- is JSV enabled in the given context 
*
*  SYNOPSIS
*     bool jsv_is_enabled(void) 
*
*  FUNCTION
*     Returns if there are active JSV instances which have to be triggered. 
*     As a side effect the function updates the JSV (restart) if the
*     configuration setting changed in master.
*
*  INPUTS
*     void - NONE
*
*  RESULT
*     bool - is jsv active
*        true  - there are active JSVs
*        false - JSV is not configured
*
*  NOTES
*     MT-NOTE: jsv_is_enabled() is MT safe 
*******************************************************************************/
bool 
jsv_is_enabled(const char *context) {
   bool ret = true;
   const char *jsv_url;

   DENTER(TOP_LAYER, "jsv_is_enabled");

   jsv_url = mconf_get_jsv_url();
   jsv_list_update("jsv", context, NULL, jsv_url);
   sge_free(&jsv_url);
   sge_mutex_lock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
   ret = (lGetNumberOfElem(jsv_list) > 0) ? true : false;
   sge_mutex_unlock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_list_remove_all() ***************************************
*  NAME
*     jsv_list_remove_all() -- Remove all JSV elements 
*
*  SYNOPSIS
*     bool jsv_list_remove_all(void) 
*
*  FUNCTION
*     Remove all JSV elements from the global 'jsv_list' 
*
*  INPUTS
*     void - None 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - failed
*
*  NOTES
*     MT-NOTE: jsv_list_remove_all() is MT safe 
*******************************************************************************/
bool 
jsv_list_remove_all(void)
{
   bool ret = true;
   lListElem *jsv;
   lListElem *jsv_next;

   DENTER(TOP_LAYER, "jsv_list_remove_all");
   sge_mutex_lock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
   jsv_next = lFirst(jsv_list);
   while ((jsv = jsv_next) != NULL) {
      jsv_next = lNext(jsv);
      jsv_stop(jsv, NULL, true);
      lRemoveElem(jsv_list, &jsv);
   }
   sge_mutex_unlock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_list_update() **********************************************
*  NAME
*     jsv_list_update() -- update configuration and state of a JSV 
*
*  SYNOPSIS
*     bool 
*     jsv_list_update(const char *name, const char *context, 
*                     lList **answer_list, const char *jsv_url) 
*
*  FUNCTION
*     A call to this function either updates the configuration and/or
*     state of a existing JSV or it creates a new one. 'name' and
*     'context' are used to identify if the JSV already exists. If
*     it does not exist then it will be created. 
*
*     If the JSV exists then it is tested if the provided 'jsv_url'
*     is different from the jsv_url stored in the JSV element. In
*     that case the provided 'jsv_url' will be used as new
*     configuration value. If the JSV process is already running
*     then it will be stopped. The process will also be stopped
*     if the script file which was used to create the JSV process
*     has been changed.
*
*  INPUTS
*     const char *name    - name of the JSV element 
*     const char *context - context string which identifies JSVs
*     lList **answer_list - AN_Type answer list 
*     const char *jsv_url - JSV URL string 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: jsv_list_update() is not MT safe 
*******************************************************************************/
bool 
jsv_list_update(const char *name, const char *context,
                lList **answer_list, const char *jsv_url) 
{
   bool ret = false;

   DENTER(TOP_LAYER, "jsv_list_update");

   if (name != NULL && context != NULL) {
      bool already_exists = false;

      /*
       * Update the information stored in a JSV element. If the URL did
       * not change then it is not necessary do anything. But if something changed
       * then update the information and set a flag (JSV_has_to_restart)
       * that indicates that a JSV process which might be already running, is 
       * restarted before the next job verification is done. If the setting
       * is changed to none then stop the process and remove data structures.
       */
      {
         const void *iterator = NULL;
         lListElem *jsv = NULL;
         lListElem *jsv_next = NULL;
         bool not_parsed = true;
         bool use_old_url = (jsv_url == NULL) ? true : false;

         sge_mutex_lock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);

         ret = true;
         jsv_next = lGetElemStrFirst(jsv_list, JSV_context, context, &iterator);
         while ((ret == true) && ((jsv = jsv_next) != NULL)) {
            dstring input = DSTRING_INIT;
            dstring type = DSTRING_INIT;
            dstring user = DSTRING_INIT;
            dstring path = DSTRING_INIT; 
            const char *old_jsv_url = NULL; 
            bool in_client = false;

            /* position pointer to the next element */
            jsv_next = lGetElemStrNext(jsv_list, JSV_context, context, &iterator);

            old_jsv_url = lGetString(jsv, JSV_url);
            if (use_old_url) {
               jsv_url = old_jsv_url;
            }

            /* 
             * stop the process if the configuration changed or if the file was modified 
             */
            if (strcmp(old_jsv_url, jsv_url) != 0) {
               DTRACE;

               /* 
                * did we get a JSV URL or "none" as new setting? 
                * depending on that either change the jsv settings and trigger a restart
                * or trigger termination of jsv process and remove the jsv cull structure
                */
               if (strcasecmp(jsv_url, "none") != 0) {
                  if (not_parsed) {
                     sge_dstring_append(&input, jsv_url);
                     in_client = (strcmp(context, JSV_CONTEXT_CLIENT) == 0) ? true : false;
                     jsv_url_parse(&input, answer_list, &type, &user, &path, in_client);
                     not_parsed = false;
                  }

                  lSetString(jsv, JSV_type, sge_dstring_get_string(&type));
                  lSetString(jsv, JSV_user, sge_dstring_get_string(&user));
                  lSetString(jsv, JSV_command, sge_dstring_get_string(&path));
                  lSetString(jsv, JSV_url, jsv_url);
                  INFO((SGE_EVENT, MSG_JSV_SETTING_S, context));
                  jsv_stop(jsv, answer_list, true);
               } else {
                  jsv_stop(jsv, answer_list, true);
                  lRemoveElem(jsv_list, &jsv);
                  INFO((SGE_EVENT, MSG_JSV_STOP_S, context));
               }
            } else {
               SGE_STRUCT_STAT st;

               DTRACE;
   
               if (SGE_STAT(lGetString(jsv, JSV_command), &st) == 0 && 
                   lGetUlong(jsv, JSV_last_mod) != st.st_mtime) {
                  INFO((SGE_EVENT, MSG_JSV_TIME_S, context));
                  jsv_stop(jsv, answer_list, true);    
               }
            }

            DTRACE;

            /*
             * independent if the jsv was modified or deleted - don't create it below
             */
            already_exists = true;
   
            /* cleanup */
            sge_dstring_free(&input);
            sge_dstring_free(&type);
            sge_dstring_free(&user);
            sge_dstring_free(&path);
         }

         sge_mutex_unlock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
      }

      /*
       * create a new JSV element if no one exists
       */
      if (ret && !already_exists && jsv_url != NULL && strcasecmp(jsv_url, "none") != 0) {
         ret = jsv_list_add(name, context, answer_list, jsv_url);
      }
   }
   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_do_verify() *********************************************
*  NAME
*     jsv_do_verify() -- verify a job using JSVs
*
*  SYNOPSIS
*     bool 
*     jsv_do_verify(sge_gdi_ctx_class_t* ctx, const char *context, 
*                   lListElem **job, lList **answer_list, bool holding_lock) 
*
*  FUNCTION
*     This function verifies a job specification using one or multiple 
*     JSVs. The 'context' string will be used to identify which JSVs
*     should be executed. 
*
*     In command-line clients the string "JSV_CONTEXT_CLIENT" has to be
*     passed to this function. In qmaster context the name of the thread
*     which calls this function has to be provided.
*
*     If multiple JSVs should be executed then the job specification
*     of 'job' will be passed to the first JSV. This specification might
*     be changed during the verification process. The result will be 
*     passed to the second JSV process which might also adjust job
*     specification parameters. This will either continue until the
*     last JSV returns successfully or until a JSV rejects the job.
*
*     'job' is input and output parameter. It might either be unchanged,
*     partially changed or completely removed. Therefore 'job' might not be
*     element in a list when the function is called.
*
*     'holding_lock' notifies the function that the calling function
*     is holding the global lock. In that case the global lock will be
*     released during the verification process. After that the global
*     lock will be acquired again.
*
*     This function will return 'true' if the verification process
*     was successful. In that case 'job' contains the job specification
*     of the verified job with all adjustments which might have been 
*     requested during the verification process of all JSVs.
*
*     If the job was rejected by one of the JSVs then the return value
*     of this function will be 'false' and the answer list will either 
*     contain the error reason which was provided by the rejecting JSV 
*     script/binary or it will contain a uniform message that the
*     job was rejected due to JSV verification. If the job is accepted
*     or accepted with modifications the returned value will be 'true'.
*
*  INPUTS
*     sge_gdi_ctx_class_t* ctx - context object 
*     const char *context      - JSV_CONTEXT_CLIENT or thread name
*     lListElem **job          - pointer or a job (JB_Type) 
*     lList **answer_list      - answer_list for messages 
*     bool holding_lock        - is the calling thread holding the
*                                global lock 
*
*  RESULT
*     bool - error state
*        true  - success
*                job is accepted or accepted with modifications
*        false - error
*                job is rejected
*
*  NOTES
*     MT-NOTE: jsv_do_verify() is MT safe 
*******************************************************************************/
bool 
jsv_do_verify(sge_gdi_ctx_class_t* ctx, const char *context, lListElem **job, 
              lList **answer_list, bool holding_lock) 
{
   bool ret = true;
   lListElem *jsv;
   lListElem *jsv_next;
   const void *iterator = NULL;
   
   DENTER(TOP_LAYER, "jsv_do_verify");

   if (context != NULL && job != NULL) {
      const char *jsv_url = NULL;
      bool holding_mutex = false;

      /*
       * Depending on the context either provide a NULL pointer to 
       * jsv_list_update or the current setting of the configuration.
       * In the client context this means that the registered jsv_url
       * will be used which was provided at command start via -jsv
       * in the server context the current setting of the jsv_url
       * will be used which is part of the global configuration.
       */
      if (strcmp(context, JSV_CONTEXT_CLIENT) == 0) {
         jsv_url = NULL;
         DPRINTF(("JSV client context\n"));
      } else {
         jsv_url = mconf_get_jsv_url();
         DPRINTF(("JSV server context\n"));
      }

      /*
       * update the list of JSV scripts for the current thread
       */
      jsv_list_update("jsv", context, answer_list, jsv_url);
      DPRINTF(("JSV list for current thread updated\n"));

      sge_mutex_lock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
      holding_mutex = true;

      /*
       * execute JSV scripts for this thread
       */
      jsv_next = lGetElemStrFirst(jsv_list, JSV_context, context, &iterator);
      while ((ret == true) && ((jsv = jsv_next) != NULL)) {
         jsv_next = lGetElemStrNext(jsv_list, JSV_context, context, &iterator);

         if (jsv_is_started(jsv) == false) {
            DPRINTF(("JSV is not started\n"));
            ret &= jsv_start(jsv, answer_list);
         }
         if (ret) {
            lListElem *old_job = *job;
            lListElem *new_job = lCopyElem(*job);

            lSetBool(jsv, JSV_restart, false);
            lSetBool(jsv, JSV_accept, false);
            lSetBool(jsv, JSV_done, false);
  
            lSetRef(jsv, JSV_old_job, old_job);
            lSetRef(jsv, JSV_new_job, new_job);

            DPRINTF(("JSVs local variables initialized for verification run\n"));

            /* 
             * A) If we came here and if the JSV which is currently handled is a server JSV
             *    then the current state is this:
             *   which was hold before communication with JSV 
             *    - holding_lock is true because this code is then executed within the 
             *      master as part of a GDI JOB ADD request. The lock was acquired outside
             *    - jsv_mutex is currently hold because it was acquired above
             *    - jsv_next will always be NULL because there is only one server JSV 
             *      instance which has to be executed,
             *
             * B) If we came here and this code is executed due to a configured client JSV then
             *
             *    - holding_lock is false. There is no global lock in the client code.
             *    - jsv_mutex is currently hold because it was acquired above.
             *    - jsv_next might be != NULL when there are other client JSVs which have
             *      to be executed after the one which is currently handled.
             *
             * We can distinguish between case A and B by looking at holding_lock. In case A
             * we can:
             *
             *    - release the mutex_lock immediately because we can be sure that this
             *      loop (while loop where we are currently in) is not executed again because 
             *      there are no other JSVs. It is also true that the JSV will not be removed by 
             *      some other thread, because for Server JSVs, always the worker thread
             *      which executes a JSV, is responsible to stop and remove a JSV. Only exception
             *      is the shutdown phase were this is done during shutdown but during shutdown
             *      phase this is only done when the worker threads have finished all requests.
             *    - unlock the global lock. So some other thread can have it. 
             *    - do the verify (communication with JSV instance)
             *    - acquire the global lock again to finish the GDI JOB ADD request when
             *      this function returns.
             */
            if (holding_lock) {
               DPRINTF(("JSV releases global lock for verification process\n"));
               sge_mutex_unlock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
               holding_mutex = false;
               SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE)
               DPRINTF(("Client/master will start communication with JSV\n"));
               ret &= jsv_do_communication(ctx, jsv, answer_list);
               DPRINTF(("JSV acquires global lock which was hold before communication with JSV\n"));
               SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE)
            } else {
               DPRINTF(("Client/master will start communication with JSV\n"));
               ret &= jsv_do_communication(ctx, jsv, answer_list);
            }

            lSetRef(jsv, JSV_old_job, NULL); 
            lSetRef(jsv, JSV_new_job, NULL); 

            if (lGetBool(jsv, JSV_accept)) {
               DPRINTF(("JSV accepts job"));
               lFreeElem(job);
               *job = new_job;
               new_job = NULL;
               ret = true;
            } else {
               u_long32 jid = lGetUlong(new_job, JB_job_number);

               DPRINTF(("JSV rejects job\n"));
               if (jid == 0) {
                  INFO((SGE_EVENT, MSG_JSV_REJECTED_S, context));
               } else {
                  INFO((SGE_EVENT, MSG_JSV_REJECTED_SU, context, jid));
               }
               ret = false;
               lFreeElem(&new_job);
            }
            if (lGetBool(jsv, JSV_restart)) {
               bool soft_shutdown = lGetBool(jsv, JSV_soft_shutdown) ? true : false;

               DPRINTF(("JSV has to be restarted\n"));
               INFO((SGE_EVENT, MSG_JSV_RESTART_S, context));
               ret &= jsv_stop(jsv, answer_list, soft_shutdown);
            }
         }
         if (strcmp(context, JSV_CONTEXT_CLIENT) == 0) {
            ret &= jsv_stop(jsv, answer_list, true);
         } 
      }
      if (holding_mutex) {
         sge_mutex_unlock("jsv_list", SGE_FUNC, __LINE__, &jsv_mutex);
      }
      sge_free(&jsv_url);
   }
   DRETURN(ret);
}
