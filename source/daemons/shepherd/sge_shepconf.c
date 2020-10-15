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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <string.h>
#include <signal.h>

#include "uti/config_file.h"
#include "uti/sge_signal.h"
#include "uti/sge_string.h"

#include "signal_queue.h"
#include "sge_shepconf.h"
#include "err_trace.h"

/****** shepherd/shepconf/shepconf_has_userdef_method() ************************
*  NAME
*     shepconf_has_userdef_method() -- Do we have a user def. method?
*
*  SYNOPSIS
*     int shepconf_has_userdef_method(char *method_name, dstring *method) 
*
*  FUNCTION
*     Try to find the variable "method_name" in config-file. 
*     Return true and set "method" if it is an absolute path;
*     otherwise return false.
*      
*
*  INPUTS
*     char *method_name - "starter_method", "suspend_method", 
*                         "resume_method" or "terminate_method"
*     dstring *method   - Absolute filename of the method
*
*  RESULT
*     int - true or false 
*******************************************************************************/
int shepconf_has_userdef_method(const char *method_name, dstring *method)
{
   char *conf_val = search_nonone_conf_val(method_name);
   int ret = 0;

   if (conf_val != NULL && conf_val[0] == '/') {
      sge_dstring_copy_string(method, conf_val);
      ret = 1;
   }
   return ret;
}

/****** shepherd/shepconf/shepconf_has_userdef_signal() ***********************
*  NAME
*     shepconf_has_userdef_signal() -- Do we have a user def. signal? 
*
*  SYNOPSIS
*     int shepconf_has_userdef_signal(char *method_name, int *signal) 
*
*  FUNCTION
*     Try to find the variable "method_name" in config-file.
*     Return true and set "signal" if it is a signal name
*     otherwise return false. 
*
*  INPUTS
*     char *method_name - "starter_method", "suspend_method",
*                         "resume_method" or "terminate_method" 
*     int *signal       - signal id 
*
*  RESULT
*     int - true or false 
*******************************************************************************/
int shepconf_has_userdef_signal(const char *method_name, int *signal) 
{
   char *conf_val = search_nonone_conf_val(method_name);
   int ret = 0;

   if (conf_val != NULL && conf_val[0] != '/') {
      *signal = shepherd_sys_str2signal(conf_val);
      ret = 1;
   }
   return ret;
}

/****** shepherd/shepconf/shepconf_has_notify_signal() ************************
*  NAME
*     shepconf_has_notify_signal() -- Do we have a notification signal 
*
*  SYNOPSIS
*     int shepconf_has_notify_signal(char *notify_name, int *signal) 
*
*  FUNCTION
*     This function checks if the notification mechanism is enabled.
*     In this case the function will retuen 'true' and it will
*     return the default signal or the user defined signal for
*     the given "notify_name".
*
*  INPUTS
*     char *notify_name - "notify_susp" or "notify_kill" 
*     int *signal       - signal id 
*
*  RESULT
*     int - true or false
*******************************************************************************/
int shepconf_has_notify_signal(const char *notify_name, int *signal)
{
   const char *notify_array[] = {
      "notify_susp", "notify_kill", NULL
   };
   int signal_array[] = {
      SIGUSR1, SIGUSR2, 0
   };
   dstring param_name = DSTRING_INIT;
   char *conf_type = NULL;
   int conf_id;
   int ret = 0;

   /*
    * There are three possibilities:
    *    a) There is a user defined signal which should be used
    *    b) Default signal should be used
    *    c) Notification mechanism is disabled
    */
   sge_dstring_sprintf(&param_name, "%s%s", notify_name, "_type");
   conf_type = search_conf_val(sge_dstring_get_string(&param_name));
   sge_dstring_free(&param_name);
   if (conf_type != NULL) {
      conf_id = atol(conf_type);
   } else {
      conf_id = 1;   /* Default signal should be used */
   }

   if (conf_id == 0) {
      char *conf_signal = search_conf_val(notify_name);

      if (conf_signal != NULL) {
         *signal = sge_sys_str2signal(conf_signal);
         ret = 1;
      }
   } else if (conf_id == 1) {
      int i;

      for (i = 0; notify_array[i] != NULL; i++) {
         if (!strcmp(notify_array[i], notify_name)) {
            break;
         }
      }
      *signal = signal_array[i];
      ret = 1;
   } else {
      *signal = 0;
      ret = 0;
   }
   return ret;
}

/****** shepherd/shepconf/shepconf_has_to_notify_before_signal() **************
*  NAME
*     shepconf_has_to_notify_before_signal() -- Get notification time 
*
*  SYNOPSIS
*     int shepconf_has_to_notify_before_signal(int *seconds) 
*
*  FUNCTION
*     If the notification mechanism is enabled then this function
*     will return with "true" and "seconds" will be > 0. 
*
*  INPUTS
*     int *seconds - time to elapse between notification and final signal 
*
*  RESULT
*     int - true or false
*******************************************************************************/
int shepconf_has_to_notify_before_signal(int *seconds) 
{
   *seconds = atoi(get_conf_val("notify"));

   return (*seconds > 0);
} 

