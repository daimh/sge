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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "sge_thread_jvm.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_bootstrap.h"
#include "uti/msg_utilib.h"  /* remove once 'sge_daemonize_qmaster' did become 'sge_daemonize' */
#include "uti/setup_path.h"
#include "uti/sge_os.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_answer.h"

#include "gdi/sge_security.h"
#include "gdi/sge_gdi2.h"

#include "comm/cl_commlib.h"

#include "basis_types.h"
#include "sge_qmaster_threads.h"
#include "sge_thread_main.h"
#include "sge_event_master.h"
#include "sge_reporting_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_host_qmaster.h"
#include "sge_advance_reservation_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"
#include "msg_daemons_common.h"
#include "setup_qmaster.h"

/****** qmaster/sge_qmaster_main/sge_gdi_kill_master() *************************
*  NAME
*     sge_gdi_kill_master() -- Shutdown qmaster via GDI
*
*  SYNOPSIS
*     void sge_gdi_kill_master(sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task);
*
*  FUNCTION
*     Shutdown qmaster by means of a GDI request. This operation is only
*     permitted for a user of type 'manager'.
*
*  INPUTS
*     sge_gdi_packet_class_t *packet - request packet
*     sge_gdi_task_class_t *task     - request task
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: sge_gdi_kill_master() is not MT safe.
*     MT-NOTE:
*     MT-NOTE: This is acceptable for now, because this function is currently
*     MT-NOTE: only invoked from the message thread.
*
*     TODO-AD: make this function thread safe. 'manop_is_manager()' is NOT MT
*     TODO-AD  safe.
*
*******************************************************************************/
void sge_gdi_kill_master(sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task)
{
   DENTER(GDI_LAYER, "sge_gdi_kill_master");

   DPRINTF(("uid/username = %d/%s, gid/groupname = %d/%s\n", (int) packet->uid, packet->user, (int) packet->gid, packet->group));

   if (!manop_is_manager(packet->user)) {
      ERROR((SGE_EVENT, SFNMAX, MSG_SHUTDOWN_SHUTTINGDOWNQMASTERREQUIRESMANAGERPRIVILEGES));
      answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ENOMGR, ANSWER_QUALITY_ERROR);
      DEXIT;
      return;
   }

   if (sge_qmaster_shutdown_via_signal_thread(0) == 0) {
      INFO((SGE_EVENT, MSG_SGETEXT_KILL_SSS, packet->user, packet->host, prognames[QMASTER]));
      answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
   } else {
      ERROR((SGE_EVENT, MSG_SGETEXT_KILL_FAILED_SSS, packet->user, packet->host, prognames[QMASTER]));
      answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   }

   DEXIT;
   return;
} /* sge_gdi_kill_master() */

/****** qmaster/sge_qmaster_main/sge_daemonize_qmaster() ***************************
*  NAME
*     sge_daemonize_qmaster() -- Turn qmaster into a daemon. 
*
*  SYNOPSIS
*     static void sge_daemonize_qmaster(void) 
*
*  FUNCTION
*     If the environment variable 'SGE_ND' is set, this function returns
*     immediately.
*
*     First, we call 'fork()'. If the process was started as a shell command in
*     the foreground, when the parent terminates, the shell thinks the command
*     is done. This automatically runs the child process in the background.
*     Also, the child inherits the process group ID from the parent but gets
*     its own process ID. This guarantees that the child is not a process group
*     leader. 
*
*     We call 'setsid()' to create a new session. The process becomes the
*     session leader of the new session, becomes the process group leader of a
*     new process group, and has no controlling terminal.
*
*     By calling 'fork()' a second time, we guarantee that the daemon (second
*     child) is no longer a session leader, so it cannot acquire a controlling
*     terminal. We must ignore 'SIGHUP' because when the session leader
*     terminates (the first child), all processes in the session (our second
*     child) receive the 'SIGHUP' signal.
*
*     We close any open descriptors that are inherited from the process that
*     executed 'sge_qmaster', normally a shell. We redirect 'stdin', 'stdout'
*     and 'stderr' to '/dev/null'. The reason for opening these descriptors
*     is so that any library function called by 'sge_qmaster' that assumes it
*     can read from standard input or write to either standard output or
*     standard error will not fail.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: sge_daemonize_qmaster() is not MT safe 
*
*******************************************************************************/
bool sge_daemonize_qmaster()
{
   pid_t pid = -1;
   int failed_fd;

   DENTER(TOP_LAYER, "sge_daemonize_qmaster");

   if (getenv("SGE_ND") != NULL) {
      DPRINTF(("sge_qmaster is not daemonized\n"));
      DEXIT;
      return false;
   }

   if((pid = fork()) != 0) {
      if (pid < 0) {
         CRITICAL((SGE_EVENT, MSG_PROC_FIRSTFORKFAILED_S , strerror(errno)));
      }
      exit(0); /* parent terminates */
   }

   setsid();

   signal(SIGHUP, SIG_IGN);

   if((pid = fork()) != 0) {
      if (pid < 0) {
         CRITICAL((SGE_EVENT, MSG_PROC_SECONDFORKFAILED_S , strerror(errno)));
      }
      exit(0); /* child 1 terminates */
   }

   sge_close_all_fds(NULL, 0);

   failed_fd = sge_occupy_first_three();
   if (failed_fd  != -1) {
      CRITICAL((SGE_EVENT, MSG_CANNOT_REDIRECT_STDINOUTERR_I, failed_fd));
      SGE_EXIT(NULL, 0);
   }

   DEXIT;
   return true;
} /* sge_daemonize_qmaster() */

/****** qmaster/sge_qmaster_main/sge_become_admin_user() ***************************
*  NAME
*     sge_become_admin_user() -- Become admin user. 
*
*  SYNOPSIS
*     static void sge_become_admin_user(void) 
*
*  FUNCTION
*     Get admin user from bootstrap configuration. Set admin user and change
*     the effective UID/GID to the admin user UID/GID. 
*
*     Note: The effective UID does determine file access permissions.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: sge_become_admin_user() is not MT safe 
*
*******************************************************************************/
void sge_become_admin_user(const char *admin_user)
{
   char str[MAX_STRING_SIZE];

   DENTER(TOP_LAYER, "sge_become_admin_user");

   if (sge_set_admin_username(admin_user, str, sizeof(str)) == -1) {
      CRITICAL((SGE_EVENT, SFNMAX, str));
      SGE_EXIT(NULL, 1);
   }

   if (sge_switch2admin_user()) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_ERROR_CANTSWITCHTOADMINUSER));
      SGE_EXIT(NULL, 1);
   }

   DEXIT;
   return;
} /* sge_become_admin_user() */

/****** qmaster/sge_qmaster_main/sge_exit_func() **********************************
*  NAME
*     sge_exit_func() -- qmaster exit function
*
*  SYNOPSIS
*     static void sge_exit_func(int anExitValue) 
*
*  FUNCTION
*     qmaster exit function. This function should be used BEFORE qmaster
*     did change its working directory to be the spool directory. This
*     exit function does NOT lock the qmaster lock file.
*
*  INPUTS
*     int anExitValue - exit value 
*
*  RESULT
*     void - none 
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     MT-NOTE: sge_exit_func() is MT safe.
*
*******************************************************************************/
void sge_exit_func(void **ctx_ref, int anExitValue)
{
   DENTER(TOP_LAYER, "sge_exit_func");
   sge_gdi2_shutdown(ctx_ref);

   DEXIT;
   return;
} /* sge_exit_func */

