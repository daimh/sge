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
#include "execution_states.h"


int shepherd_state = SSTATE_BEFORE_PROLOG;
pid_t coshepherd_pid = -999;
/*
** NAME
**   get_sstate_description
** PARAMETER
**   sstate    -    shepherd exit states
** RETURN
**   char *    -    description string
**                  "invalid execution state" if exit state is unknown
** EXTERNAL
**
** DESCRIPTION
**   string description of execution exit states
*/
char *get_sstate_description(int sstate) {
   int i;
   static struct _state_message {
      char *message;
      int state;
   } state_message[] = {
      {"", 0},
      {"assumedly before job", SSTATE_FAILURE_BEFORE_JOB},
      {"on executing shepherd", ESSTATE_NO_SHEPHERD}, /* unused */
      {"before writing config", ESSTATE_NO_CONFIG},
      {"before writing pid", ESSTATE_NO_PID},
      {"on reading config file", SSTATE_READ_CONFIG}, /* never set */
      {"setting processor set", SSTATE_PROCSET_NOTSET},
      {"before prolog", SSTATE_BEFORE_PROLOG},
      {"in prolog", SSTATE_PROLOG_FAILED},
      {"before pestart", SSTATE_BEFORE_PESTART},
      {"in pestart", SSTATE_PESTART_FAILED},
      {"before job", SSTATE_BEFORE_JOB},
      {"before pestop", SSTATE_BEFORE_PESTOP},
      {"in pestop", SSTATE_PESTOP_FAILED},
      {"before epilog", SSTATE_BEFORE_EPILOG},
      {"in epilog", SSTATE_EPILOG_FAILED},
      {"releasing processor set", SSTATE_PROCSET_NOTFREED},
      {"through signal", ESSTATE_DIED_THRU_SIGNAL},
      {"shepherd returned error", ESSTATE_SHEPHERD_EXIT},
      {"before writing exit_status", ESSTATE_NO_EXITSTATUS},
      {"found unexpected error file", ESSTATE_UNEXP_ERRORFILE},
      {"in recognizing job", ESSTATE_UNKNOWN_JOB},
      {"removed manually", ESSTATE_EXECD_LOST_RUNNING}, /* unused */
      /* ESSTATE_PTF_CANT_GET_PIDS only in execd */
      {"migrating", SSTATE_MIGRATE},
      {"rescheduling", SSTATE_AGAIN},
      {"opening input/output file", SSTATE_OPEN_OUTPUT},
      {"searching requested shell", SSTATE_NO_SHELL},
      {"changing into working directory", SSTATE_NO_CWD},
      {"AFS setup", SSTATE_AFS_PROBLEM},
      {"application error returned", SSTATE_APPERROR},
      {"accessing sgepasswd file", SSTATE_PASSWD_FILE_ERROR},
      {"entry is missing in sgepasswd file", SSTATE_PASSWD_MISSING},
      {"wrong password", SSTATE_PASSWD_WRONG},
      {"communicating with Grid Engine Helper Service", SSTATE_HELPER_SERVICE_ERROR},
      {"before job in Grid Engine Helper Service", SSTATE_HELPER_SERVICE_BEFORE_JOB},
      {"checking configured daemons", SSTATE_CHECK_DAEMON_CONFIG},
      {"qmaster enforced h_rt, h_cpu, or h_vmem limit",
       SSTATE_QMASTER_ENFORCED_LIMIT},
      {"adding supplementary group", SSTATE_ADD_GRP_SET_ERROR},
      {"assumedly after job", SSTATE_FAILURE_AFTER_JOB}
   };

   for (i=0; i<sizeof(state_message)/sizeof(struct _state_message); i++) {
      if (state_message[i].state == sstate)
         return state_message[i].message;
   }

   return "invalid execution state";
}

