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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_stdio.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_log.h"
#include "uti/sge_signal.h"
#include "uti/sge_unistd.h"
#include "uti/sge_time.h"
#include "uti/sge_string.h"
#if defined(DARWIN)
#  include "uti/sge_uidgid.h"
#endif

#include "spool/classic/read_write_job.h"

#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_ack.h"
#include "sgeobj/sge_report.h"

#include "comm/commlib.h"

#include "symbols.h"
#include "job_report_execd.h"
#include "execd_signal_queue.h"
#include "sig_handlers.h"
#include "execd.h"
#include "dispatcher.h"
#include "reaper_execd.h"
#include "admin_mail.h"
#include "sge_report_execd.h"
#include "mail.h"
#include "get_path.h"
#include "sge.h"
#include "msg_execd.h"
#include "msg_daemons_common.h"

extern volatile int shut_me_down;

/**************************************************************************
 called from dispatcher

 counterpart in qmaster: c_qmod.c
 **************************************************************************/
int do_signal_queue(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg, sge_pack_buffer *apb)
{
   lListElem *jep;
   int found = 0;
   u_long32 jobid, signal, jataskid;
   char *qname = NULL;

   DENTER(TOP_LAYER, "do_signal_queue");

   if (unpackint(&(aMsg->buf), &jobid) != 0 ||
       unpackint(&(aMsg->buf), &jataskid) != 0 ||
       unpackstr(&(aMsg->buf), &qname) != 0 || /* mallocs qname !! */
       unpackint(&(aMsg->buf), &signal)) {     /* signal don't need to be packed Ü*/
      sge_free(&qname); 
      DRETURN(1);    
   }

   DPRINTF(("===>DELIVER_SIGNAL: %s >%s< Job(s) "sge_u32"."sge_u32" \n",
            sge_sig2str(signal), qname? qname: "<NULL>", jobid, jataskid));

   if (aMsg->tag == TAG_SIGJOB) { /* signal a job / task */
      pack_ack(apb, ACK_SIGJOB, jobid, jataskid, NULL);

      found = (signal_job(jobid, jataskid, signal)==0);
   } else {            /* signal a queue */
      pack_ack(apb, ACK_SIGQUEUE, jobid, jataskid, qname);

      for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
         lListElem *gdil_ep, *master_q, *jatep;
         const char *qnm;

         for_each (jatep, lGetList(jep, JB_ja_tasks)) {
            if (lGetUlong(jatep, JAT_status) == JSLAVE) {
               break;
            }   

            /* iterate through all queues of a parallel job -
               this is done to ensure that signal delivery is also
               forwarded to the job in case the master queue is still active */
            for_each (gdil_ep, lGetList(jatep, JAT_granted_destin_identifier_list)) {
               master_q = lGetObject(gdil_ep, JG_queue);
               if (master_q != NULL) {
                  qnm =  lGetString(master_q, QU_full_name);
                  if (strcmp(qname, qnm) == 0) {
                     char tmpstr[SGE_PATH_MAX];

                     /* job signaling triggered by a queue signal */
                     snprintf(tmpstr, sizeof(tmpstr), "%s (%s)", sge_sig2str(signal), qnm);
                     /* if the queue gets suspended and the job is already suspended
                        we do not deliver a signal */
                     if (signal == SGE_SIGSTOP) {
                        qinstance_state_set_manual_suspended(master_q, true);
                        if (!VALID(JSUSPENDED, lGetUlong(jatep, JAT_state))) {
                           if (lGetUlong(jep, JB_checkpoint_attr)& CHECKPOINT_SUSPEND) {
                              INFO((SGE_EVENT, MSG_JOB_INITMIGRSUSPQ_U, sge_u32c(lGetUlong(jep, JB_job_number))));
                              signal = SGE_MIGRATE;
                           }   
                           if (sge_execd_deliver_signal(signal, jep, jatep) == 0) {
                              sge_send_suspend_mail(signal, jep, jatep); 
                           }
                        }   
                     } else {
                        /* if the signal is a unsuspend and the job is suspended
                           we do not deliver a signal */
                        if (signal == SGE_SIGCONT) {
                           qinstance_state_set_manual_suspended(master_q, false);
                           if (!VALID(JSUSPENDED, lGetUlong(jatep, JAT_state))) {
                              if ( sge_execd_deliver_signal(signal, jep, jatep) == 0) {
                                 sge_send_suspend_mail(signal, jep, jatep);
                              }
                           }
                        } else {
                           sge_execd_deliver_signal(signal, jep, jatep);
                        }   
                     }
                     found = lGetUlong(jep, JB_job_number);

                     if (!mconf_get_simulate_jobs()) {
                        job_write_spool_file(jep, 
                           lGetUlong(lFirst(lGetList(jep, JB_ja_tasks)), 
                           JAT_task_number), NULL, SPOOL_WITHIN_EXECD);
                     }

                  }
               }
            }
         }
      }
      /*
      ** when a queue state has changed
      ** we release the block on sending admin mails
      */
      adm_mail_reset(BIT_ADM_QCHANGE);
   }

   /* If this is a queue signal 'found' now holds the number of a job
      running in this queue. */
   if (!found && aMsg->tag == TAG_SIGJOB) {
      lListElem *jr;
      jr = get_job_report(jobid, jataskid, NULL);
      remove_acked_job_exit(ctx, jobid, jataskid, NULL, jr);
      job_unknown(jobid, jataskid, qname);
   }

   sge_free(&qname);

   DRETURN(0);
}

/*************************************************************************
 execds function to deliver a signal to the job. This can't be done directly
 because there is the shepherd between.
 We do a signal mapping for the shepherd.

 sent to shepherd  |  signal to deliver to job
 ------------------|---------------------------
 SIGTTIN           | given in "signal"-file
 SIGUSR1           | SIGUSR1
 SIGUSR2           | SIGXCPU
 SIGCONT           | SIGCONT
 SIGWINCH          | SIGSTOP (not used anymore, use SIGTTIN to fwd SIGSTOP )
 SIGTSTP           | SIGKILL

 SIGTTIN forces the shepherd to look into the "signal" file. This file
 contains the number of the signal to send.

 The job will get a SIGUSR1 as notification for SIGSTOP and
                    SIGUSR2                     SIGKILL if
 The job has been submitted with -notify and the notification time of the
 queue is > 0.

 returns 
   0 on success
   1 if job is supposed to be not in a healthy state and thus
     should be removed by the calling context
 *************************************************************************/
int sge_execd_deliver_signal(u_long32 sig, lListElem *jep, lListElem *jatep)
{
   int queue_already_suspended;
   int getridofjob = 0;

   DENTER(TOP_LAYER, "sge_execd_deliver_signal");

   INFO((SGE_EVENT, MSG_JOB_SIGNALTASK_UUS,   
         sge_u32c(lGetUlong(jep, JB_job_number)), sge_u32c(lGetUlong(jatep, JAT_task_number)), 
         sge_sig2str(sig)));

   /* for simulated hosts do nothing */
   if (mconf_get_simulate_jobs()) {
      if (sig == SGE_SIGKILL) {
         lListElem *jr = NULL;
         u_long32 jobid, jataskid;
         u_long32 wallclock;

         jobid = lGetUlong(jep, JB_job_number);
         jataskid = lGetUlong(jatep, JAT_task_number);
         
         DPRINTF(("Simulated job "sge_u32"."sge_u32" is killed\n", jobid, jataskid));

         if ((jr=get_job_report(jobid, jataskid, NULL)) == NULL) {
            ERROR((SGE_EVENT, MSG_JOB_MISSINGJOBXYINJOBREPORTFOREXITINGJOBADDINGIT_UU, 
                   sge_u32c(jobid), sge_u32c(jataskid)));
            jr = add_job_report(jobid, jataskid, NULL, jep);
         }

         lSetUlong(jr, JR_state, JEXITING);
         lSetUlong(jatep, JAT_end_time, sge_get_gmt());
         add_usage(jr, "submission_time", NULL, lGetUlong(jep, JB_submission_time));
         add_usage(jr, "start_time", NULL, lGetUlong(jatep, JAT_start_time));
         add_usage(jr, "end_time", NULL, lGetUlong(jatep, JAT_end_time));
         wallclock = lGetUlong(jatep, JAT_end_time) - lGetUlong(jatep, JAT_start_time);
         add_usage(jr, "ru_wallclock", NULL, wallclock);
         add_usage(jr, USAGE_ATTR_CPU_ACCT, NULL, wallclock * 0.5);
         add_usage(jr, "ru_utime", NULL, wallclock * 0.4);
         add_usage(jr, "ru_stime", NULL, wallclock * 0.1);
         add_usage(jr, "exit_status", NULL, 137);
         add_usage(jr, "signal", NULL, sig);

         lSetUlong(jatep, JAT_status, JEXITING);

         flush_job_report(jr);
      }
      
      DRETURN(0);
   }

/*
   DPRINTF(("(sig==SGE_MIGRATE) = %d (ckpt on suspend) = %d %d\n", 
      (sig == SGE_MIGRATE), lGetUlong(jep, JB_checkpoint_attr)&CHECKPOINT_SUSPEND,
         lGetUlong(jep, JB_checkpoint_attr)));
*/
   /* Simply apply signal to all subtasks of the job 
      except in case of SGE_MIGRATE when there is a 
      ckpt env with "migrate on suspend" configured */
   /* See do_signal_queue concerning "queue" here.  */
   queue_already_suspended = (lGetUlong(jatep, JAT_state) & JSUSPENDED);
   DPRINTF(("queue_already_suspended = %d\n", queue_already_suspended));
   if (!(sig == SGE_MIGRATE
         && (lGetUlong(jep, JB_checkpoint_attr) & CHECKPOINT_SUSPEND))
         && !queue_already_suspended) {
      lListElem *petep;
      DPRINTF(("signaling each pe task with signal %d\n", (int) sig));
      /* signal each pe task */
      for_each (petep, lGetList(jatep, JAT_task_list)) {
         DPRINTF(("signaling pe task pid="sge_u32"\n", lGetUlong(petep, PET_pid)));
         if (sge_kill((int)lGetUlong(petep, PET_pid), sig, 
                      lGetUlong(jep, JB_job_number), lGetUlong(jatep, JAT_task_number), 
                      lGetString(petep, PET_id)) == -2) {
            getridofjob = 1;
         }
      }
   }

   DPRINTF(("lGetUlong(jatep, JAT_status) = "sge_u32", JSLAVE = %d\n",
            lGetUlong(jatep, JAT_status), JSLAVE));

   if (lGetUlong(jatep, JAT_status) != JSLAVE) {
      if (sge_kill((int)lGetUlong(jatep, JAT_pid), sig, lGetUlong(jep, JB_job_number), 
                        lGetUlong(jatep, JAT_task_number), NULL) == -2) {
         getridofjob = 1;
      }
   }

   DRETURN(getridofjob);
}

/****** execd_signal_queue/sge_send_suspend_mail() *****************************
*  NAME
*     sge_send_suspend_mail() -- send suspend / continue mail if enabled
*
*  SYNOPSIS
*     void sge_send_suspend_mail(u_long32 signal, lListElem *jep, lListElem *jatep) 
*
*  FUNCTION
*     This function will send the suspend/continue mail to the job owner 
*     or to the users defined with the -M option of qsub. The mail is 
*     only sent when the user has specified the -m s flag. 
*
*     The mail is sent when the execd is signaling the job with 
*     SGE_SIGSTOP(suspend) / SGE_SIGCONT(continue)
*
*  INPUTS
*     u_long32 signal     - type of signal (SGE_SIGSTOP/SGE_SIGCONT)
*     lListElem *jep      - pointer to JB_Type  cull list element 
*                           of job 
*     lListElem *jatep    - pointer to JAT_Type cull list element 
*                           of job
*******************************************************************************/
void sge_send_suspend_mail(u_long32 signal, lListElem *jep, lListElem *jatep) {

   u_long32 mail_options; 

   DENTER(TOP_LAYER, "sge_send_suspend_mail");

   mail_options = lGetUlong(jep, JB_mail_options); 

   /* only if mail at suspension is enabled */
   if (VALID(MAIL_AT_SUSPENSION, mail_options)) {

       lList *mail_users      = NULL; 

       u_long32 jobid         = 0;
       u_long32 taskid        = 0;
       u_long32 job_sub_time  = 0;
       u_long32 job_exec_time = 0; 

       char mail_subject[MAX_STRING_SIZE]; 
       char mail_body[MAX_STRING_SIZE];
       char job_sub_time_str[256];
       char job_exec_time_str[256];

       const char *job_name = NULL;
       const char *job_master_queue = NULL;
       const char *job_owner = NULL; 
       const char *mail_type = "unknown";

       dstring ds;
       char buffer[128];
      
       sge_dstring_init(&ds, buffer, sizeof(buffer));


       /* get values */       
       if (jep != NULL) {
          job_sub_time = lGetUlong(jep, JB_submission_time);
          jobid        = lGetUlong(jep, JB_job_number);
          mail_users   = lGetList(jep, JB_mail_list);
          job_name     = lGetString(jep, JB_job_name);
          job_owner    = lGetString(jep, JB_owner);
        }

       if (jatep != NULL) {
          job_exec_time    = lGetUlong(jatep, JAT_start_time );
          taskid           = lGetUlong(jatep, JAT_task_number );
          job_master_queue = lGetString(jatep, JAT_master_queue);
       }

       /* check strings */
       if (job_name == NULL) {
           job_name = MSG_MAIL_UNKNOWN_NAME;
       }
       if (job_master_queue == NULL) {
           job_master_queue = MSG_MAIL_UNKNOWN_NAME;
       }
       if (job_owner == NULL) {
           job_owner = MSG_MAIL_UNKNOWN_NAME;
       }


       /* make human readable time format */
       snprintf(job_sub_time_str, sizeof(job_sub_time_str), "%s",
                sge_ctime((time_t)job_sub_time, &ds));
       snprintf(job_exec_time_str, sizeof(job_exec_time_str), "%s",
                sge_ctime((time_t)job_exec_time, &ds));

       if (signal == SGE_SIGSTOP) {
          /* suspended */
          if (job_is_array(jep)) {
             snprintf(mail_subject, sizeof(mail_subject),
                      MSG_MAIL_SUBJECT_JA_TASK_SUSP_UUS,
                      sge_u32c(jobid), 
                      sge_u32c(taskid), 
                      job_name);
          } else {
             snprintf(mail_subject, sizeof(mail_subject),
                      MSG_MAIL_SUBJECT_JOB_SUSP_US,
                      sge_u32c(jobid), 
                      job_name);
          }
          mail_type = MSG_MAIL_TYPE_SUSP;
       } else if (signal == SGE_SIGCONT ) {
          /* continued */
          if (job_is_array(jep)) {
             snprintf(mail_subject, sizeof(mail_subject),
                      MSG_MAIL_SUBJECT_JA_TASK_CONT_UUS,
                      sge_u32c(jobid), 
                      sge_u32c(taskid), 
                      job_name);
          } else {
             snprintf(mail_subject, sizeof(mail_subject),
                      MSG_MAIL_SUBJECT_JOB_CONT_US,
                      sge_u32c(jobid), 
                      job_name);
          }
          mail_type = MSG_MAIL_TYPE_CONT;
       } else {
          DPRINTF(("no suspend or continue signaling\n"));
          DEXIT;
          return;
       }

       /* create mail body */
       snprintf(mail_body, sizeof(mail_body),
               MSG_MAIL_BODY_SSSSS,
               mail_subject,
               job_master_queue,
               job_owner,
               job_sub_time_str,
               job_exec_time_str);
       sge_strlcpy(mail_body, "\n", sizeof(mail_body));
 
       cull_mail(EXECD, mail_users, mail_subject, mail_body, mail_type );
   } 
   DEXIT;
}

/***********************************************************************
   forward signal to shepherd

   returns
       0  on success
       -2 if shepherd process is not (or no longer?) here
       -1 in case of other problems
*/

int sge_kill(int pid, u_long32 sge_signal, u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id)
{
   int sig;
   int direct_signal;   /* deliver per signal or per file */
   char id_buffer[MAX_STRING_SIZE];
   dstring id_dstring;

   DENTER(TOP_LAYER, "sge_kill");

   sge_dstring_init(&id_dstring, id_buffer, MAX_STRING_SIZE);

   if (!pid) {
      DPRINTF(("sge_kill won't kill pid 0!\n"));
      DEXIT;
      return -1;
   }

   /* mapping from SGE_SIG... which is equal on all platforms to the platform
      specific SIG... */
   sig = sge_unmap_signal(sge_signal);

   /*  now do the mapping for the shepherd 

       sent to shepherd  |  shepherd signal to deliver to job
       ------------------|---------------------------
       SIGTTIN           | given in "signal"-file 
       SIGTTOU           | SIGTTOU (initiate migration  - shepherd knows the signal)
       SIGUSR1           | SIGUSR1
       SIGUSR2           | SIGXCPU
       SIGCONT           | SIGCONT
       SIGWINCH          | SIGSTOP (not used anymore, use SIGTTIN to fwd SIGSTOP )
       SIGTSTP           | SIGKILL
   */ 
   direct_signal = 1;

   switch (sig) {
   case SIGKILL:
      sig = SIGTSTP;
      break;
#if defined(SIGXCPU)
   case SIGXCPU:
      sig = SIGUSR2;
      break;
#endif
   case SIGTTOU:
   case SIGUSR1:
   case SIGCONT:
      break;
   /* We now fwd SIGSTOP using file(SIGTTIN) rather than SIGWINCH, refer CR6623174 */
   default:
      direct_signal = 0;        /* communication has to be done via file */
   }

   DPRINTF(("signalling job/task "SFN", pid "pid_t_fmt" with %d\n", 
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), pid, sig));
   if (!direct_signal) {
      dstring fname = DSTRING_INIT;
      FILE *fp;

      sge_get_active_job_file_path(&fname,
                                   job_id, ja_task_id, pe_task_id, "signal");
      if (!(fp = fopen(sge_dstring_get_string(&fname), "w"))) {
         ERROR((SGE_EVENT, MSG_EXECD_WRITESIGNALFILE_S, 
                sge_dstring_get_string(&fname)));
         sge_dstring_free(&fname);
         goto CheckShepherdStillRunning;
      } 

      fprintf(fp, "%d\n", sig);
      sge_dstring_free(&fname);
      FCLOSE(fp);
   }

#if defined(DARWIN)
   sge_switch2start_user();  /* fixme: check return */
   if (kill(pid, direct_signal?sig:SIGTTIN)) {
      sge_switch2admin_user();  /* fixme: check return */
#else
   if (kill(pid, direct_signal?sig:SIGTTIN)) {
#endif   
      if (errno == ESRCH)
         goto CheckShepherdStillRunning;
      DEXIT; 
      return -1;
   }
   
   DEXIT;
   return 0;

FCLOSE_ERROR:
CheckShepherdStillRunning:
   {
      dstring path = DSTRING_INIT;
      SGE_STRUCT_STAT statbuf;

      sge_get_active_job_file_path(&path,
                                   job_id, ja_task_id, pe_task_id, NULL);

      if (!SGE_STAT(sge_dstring_get_string(&path), &statbuf) && S_ISDIR(statbuf.st_mode)) {
         sge_sig_handler_dead_children = 1; /* may be we've lost a SIGCHLD */
         sge_dstring_free(&path);
         DEXIT;
         return 0;
      } else {
         WARNING((SGE_EVENT, MSG_JOB_DELIVERSIGNAL_ISSIS, sig, 
         job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), 
         sge_sig2str(sge_signal), pid, strerror(errno)));
         sge_dstring_free(&path);
         DEXIT;
         return -2;
      }
   }
}

/*------------------------------------------------------------ 

NAME

   signal_job()

DESCRIPTION

   Tries to signal the job. 

RETURN

   0 job was found and was signaled
   1 job was not found you better get rid of it to prevent 
     infinite pingpong effects
   ------------------------------------------------------------ */
int signal_job(u_long32 jobid, u_long32 jataskid, u_long32 signal)
{
   lListElem *jep;
   u_long32 state;
   lListElem *master_q;
   lListElem *jatep = NULL;
   int getridofjob = 0;

   int suspend_change = 0;
   int send_mail = 0;

   DENTER(TOP_LAYER, "signal_job");

   /* search appropriate array task and job */
   if (!execd_get_job_ja_task(jobid, jataskid, &jep, &jatep)) {
      DRETURN(1);
   }

   master_q = lGetObject(lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)), JG_queue);

   DPRINTF(("sending %s to job "sge_u32"."sge_u32"\n", sge_sig2str(signal), jobid, jataskid));
   if (signal == SGE_SIGCONT) {
      state = lGetUlong(jatep, JAT_state);
      if (ISSET(state, JSUSPENDED)) {
         suspend_change = 1; 
      }
      CLEARBIT(JSUSPENDED, state);
      SETBIT(JRUNNING, state);
      lSetUlong(jatep, JAT_state, state);

      /* If the one of the queues is suspended 
         and we unsuspend the job. 
         The Job should stay sleeping */

      if (!qinstance_state_is_manual_suspended(master_q)) {
         getridofjob = sge_execd_deliver_signal(signal, jep, jatep);
         if ((!getridofjob) && (suspend_change == 1) ) {
            send_mail = 1;
         }
      } else {
         DPRINTF(("Queue is suspended -> do nothing\n"));
      }
   } else {
      if ((signal == SGE_SIGSTOP) && (lGetUlong(jep, JB_checkpoint_attr) & CHECKPOINT_SUSPEND)) {
         INFO((SGE_EVENT, MSG_JOB_INITMIGRSUSPJ_UU, 
               sge_u32c(lGetUlong(jep, JB_job_number)), sge_u32c(lGetUlong(jatep, JAT_task_number))));
         signal = SGE_MIGRATE;
         getridofjob = sge_execd_deliver_signal(signal, jep, jatep);
      } else if (signal == SGE_SIGSTOP) {
         state = lGetUlong(jatep, JAT_state);
         if (!ISSET(state, JSUSPENDED)) {
            suspend_change = 1;
         } 
         SETBIT(JSUSPENDED, state);
         CLEARBIT(JRUNNING, state);
         lSetUlong(jatep, JAT_state, state);

         /* if this is a stop signal for a job 
            which is in at least ONE queue
            which is already stopped we 
            do not deliver the signal */

         getridofjob = sge_execd_deliver_signal(signal, jep, jatep);
         if ((!getridofjob) && (suspend_change == 1)) {
            if (!qinstance_state_is_manual_suspended(master_q)) {
               send_mail = 2;
            }
         }
      } else {
         if (signal == SGE_SIGKILL) {
            /*
             * At this point job termination is triggered and JDELETED state is set.
             * Based on that state it should be possible to provide better diagnosis 
             * information if the job dies due to a signal (Issue: #483). Possibly 
             * SGE_SIGXCPU should be treated similarly. 
             * ja_task_message_add(jatep, 1, err_str) could be used to intermediately
             * store job termination reason until it gets reaped later on.
             */
            state = lGetUlong(jatep, JAT_state);
            SETBIT(JDELETED, state);
            lSetUlong(jatep, JAT_state, state); 
            DPRINTF(("SIGKILL of job "sge_u32"\n", jobid)); 
         }
         getridofjob = sge_execd_deliver_signal(signal, jep, jatep);
      }
   }

   /* now save this job/queue so we are up to date on restart */
   if (!getridofjob) {
      if (!mconf_get_simulate_jobs()) {
         job_write_spool_file(jep, jataskid, NULL, SPOOL_WITHIN_EXECD);
      }
      /* write mail */
      if (send_mail == 1) {
         sge_send_suspend_mail(SGE_SIGCONT, jep, jatep); 
      }
      if (send_mail == 2) {
         sge_send_suspend_mail(SGE_SIGSTOP, jep, jatep);
      }

   } else {
      DPRINTF(("Job  "sge_u32"."sge_u32" is no longer running\n", jobid, jataskid));
   }
   DRETURN(getridofjob);
}
