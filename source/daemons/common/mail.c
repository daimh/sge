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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_prog.h"
#include "uti/sge_os.h"
#include "uti/sge_stdio.h"
#include "uti/config_file.h"
#include "uti/sge_uidgid.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_mailrec.h"

#include "mail.h"
#include "sig_handlers.h"
#include "msg_common.h"
#include "msg_daemons_common.h"

#if defined(SOLARIS)
#   include "sge_smf.h"
#endif

static void sge_send_mail(u_long32 progid,
                          const char *mailer, int mailer_has_subj_line, 
                          const char *user, const char *host, const char *subj,
                          const char *buf);

/*
** NAME
**   cull_mail
** PARAMETER
**   user_list     -   mail recipients list (MR_Type)
**   subj          -   subject line
**   buf           -   message contents
** RETURN
**   none
** EXTERNAL
**   sge_send_mail
** DESCRIPTION
**   sends a mail to each of the recipients in the list
*/
void cull_mail(u_long32 progid, lList *user_list, const char *subj, const char *buf, const char *mail_type) {
   char *mailer;
   int mailer_has_subj_line;
   lListElem *ep;
   const char *user, *host;

   DENTER(TOP_LAYER, "cull_mail");

   mailer = mconf_get_mailer();
   mailer_has_subj_line = 1;

   if (!buf) {
      buf = subj;
   }

   if (user_list) {
      for_each(ep, user_list) {
         user = lGetString(ep, MR_user);
         host = lGetHost(ep, MR_host);
         if (!user && !host) {
            ERROR((SGE_EVENT, SFNMAX, MSG_MAIL_EMPTYUSERHOST));
            sge_free(&mailer);
            DRETURN_VOID;
         } else if (!host) {
            INFO((SGE_EVENT, MSG_MAIL_MAILUSER_SSSS, 
                  mail_type, user, mailer, subj ? subj : MSG_MAIL_NOSUBJ));
         } else {
            INFO((SGE_EVENT, MSG_MAIL_MAILUSERHOST_SSSSS, 
                  mail_type, user, host, mailer, 
                  subj ? subj : MSG_MAIL_NOSUBJ));
         }
         sge_send_mail(progid, mailer, mailer_has_subj_line, user, host, subj, buf);
      }
   } 

   sge_free(&mailer);
   DRETURN_VOID;
}

/************************************************************/

static void sge_send_mail(u_long32 progid, const char *mailer_in,
                          int mailer_has_subj_line, const char *user,
                          const char *host, const char *subj, const char *buf)
{
   int pid;
   int pid2;
   int i;
   int exit_status;
   int pipefds[2];
   FILE *fp;
   stringT user_str;
   bool done;
   char *mailer;

#if !defined(INTERIX)
   struct rusage rusage;
#endif

#if defined(SVR3) || defined(_BSD)
   union wait status;
#else
   int status;
#endif

   DENTER(TOP_LAYER, "sge_send_mail");

   if (!user) {
      DEXIT;
      return;
   }

   alarm(0);

#if defined(SOLARIS)
   char err_str[256];
   i = sge_smf_contract_fork(err_str, 256);
   /* 
    * -2 and lower is SMF contract failure, 
    * -1 is fork() failure and will be handled later 
    */
   if (i < -1){
      ERROR((SGE_EVENT, MSG_SMF_MAIL_FORK_FAILED_S, err_str));
   }
#else
   i = fork();
#endif
   if (i>0) {
      DPRINTF(("PARENT RETURNS\n"));
      DEXIT;
      return;
   }
   /* log fork() failure */
   else if (i == -1) { /* still in parent */
      ERROR((SGE_EVENT, MSG_MAIL_NOFORK_S, strerror(errno)));
      DEXIT;
      return;
   } /* else in child */

   DPRINTF(("CHILD CONTINUES\n"));
   SETPGRP;

   sge_close_all_fds(NULL, 0);

   /* 
      may never call SGE_EXIT() here because
      leave_commd() gets called by a the mailer child
      and leave_commd() unregisters the commproc
   */
   if (pipe(pipefds) < 0) {
      ERROR((SGE_EVENT, MSG_MAIL_NOPIPE_S, strerror(errno)));
      exit(1);
   }
   /* Don't need to start in new contract on Solaris - already in new one */
   if ((pid = fork()) < 0) {
      ERROR((SGE_EVENT, MSG_MAIL_NOFORK_S, strerror(errno)));
      exit(1);
   }
   if (!pid) {
      if (host)
         snprintf(user_str, sizeof(user_str), "%s@%s", user, host);
      else
         snprintf(user_str, sizeof(user_str), "%s", user);

      if (dup2(pipefds[0], 0) < 0) {
         CRITICAL((SGE_EVENT, MSG_MAIL_NODUP_S, strerror(errno)));
         exit(1);
      }

      close(pipefds[1]);

      mailer = strdup(mailer_in);
      /* switch user */
      {
        char *user = parse_script_params(&mailer);
        if (user != NULL) {
           char err_str[256];
           if (sge_set_uid_gid_addgrp(user, NULL, 0, 0, 0, err_str,
                                      sizeof(err_str), 0, 0, false)) {
             ERROR((SGE_EVENT, MSG_SYSTEM_SWITCHTOUSERFAILED_SS, user,
                    strerror(errno)));
             sge_free(&mailer);
             DEXIT;
             return;
           }
        }
      }

      if (mailer_has_subj_line) {
         DPRINTF(("%s mail -s %s %s", mailer, subj, user_str));  
         execl(mailer, "mail", "-s", subj, user_str, (char *) NULL);
      }
      else {
         DPRINTF(("%s mail %s", mailer, user_str));  
         execl(mailer, "mail", user_str, (char *) NULL);
      }
      CRITICAL((SGE_EVENT, MSG_MAIL_NOEXEC_SS, mailer, strerror(errno)));
      exit(1);
   }

   close(pipefds[0]);
   fp = fdopen(pipefds[1], "w");
   fprintf(fp, "%s\n", buf);
   FCLOSE(fp);

   sge_setup_sig_handlers(progid);

   done = false;
   while (!done) {
      alarm(60);                /* max time to allow for mail */
      sigprocmask(SIG_SETMASK, &io_mask, &omask);
      sigaction(SIGALRM, &sigalrm_vec, &sigalrm_ovec);

#if defined(INTERIX)
      pid2 = waitpid(pid, &status, 0);
#else
      pid2 = wait3(&status, 0, &rusage);
#endif

      alarm(0);
      if (pid2 == 0) {          /* how could this happen? */
         kill(pid, SIGKILL);
         ERROR((SGE_EVENT, SFNMAX, MSG_MAIL_NOMAIL1));
         exit(1);
      }

      if (pid2 == -1) {         /* alarm must have went off */
         kill(pid, SIGKILL);
         ERROR((SGE_EVENT, SFNMAX, MSG_MAIL_NOMAIL2));
         exit(1);
      }

      if (WIFSTOPPED(status)) { /* how could this happen? */
         kill(pid, SIGKILL);
         ERROR((SGE_EVENT, MSG_MAIL_NOMAIL3_I, WSTOPSIG(status)));
         exit(1);
      }

#if defined(SVR3) || defined(_BSD)
      exit_status = status.w_retcode;
#else
      exit_status = status;
#endif
      DPRINTF(("mailer exited with exit status %d\n", exit_status));
      exit(exit_status);
   }
FCLOSE_ERROR:
   CRITICAL((SGE_EVENT, MSG_FILE_NOCLOSE_SS, "<pipefds>", strerror(errno)));
   exit(1);
}
