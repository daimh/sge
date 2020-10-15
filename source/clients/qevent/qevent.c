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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#if 0
#define QEVENT_SHOW_ALL
#endif

#include <sys/resource.h>
#include <sys/wait.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_unistd.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_profiling.h"
#include "uti/sge_time.h"
#include "uti/sge_spool.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_event.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_job.h"

#include "comm/commlib.h"

#include "mir/sge_mirror.h"

#include "gdi/sge_gdi_ctx.h"

#include "qevent.h"
#include "usage.h"
#include "sge_mt_init.h"
#include "sig_handlers.h"
#include "msg_clients_common.h"
#include "msg_common.h"


u_long Global_jobs_running = 0;
u_long Global_jobs_registered = 0;
qevent_options *Global_qevent_options;


static void qevent_show_usage(void);
static void qevent_testsuite_mode(sge_evc_class_t *evc);
static void qevent_subscribe_mode(sge_evc_class_t *evc);
static char* qevent_get_event_name(int event);
static void qevent_trigger_scripts(int qevent_event, qevent_options *option_struct, lListElem *event);
static void qevent_start_trigger_script(int qevent_event, const char* script_file, lListElem *event);
static qevent_options* qevent_get_option_struct(void);
static void qevent_set_option_struct(qevent_options *option_struct);


static void  qevent_set_option_struct(qevent_options *option_struct) {
   Global_qevent_options=option_struct;
}


static qevent_options* qevent_get_option_struct(void) {
   return Global_qevent_options;
}

static void qevent_dump_pid_file(void) {
   sge_write_pid("qevent.pid");
}

static sge_callback_result 
print_event(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
            sge_event_action action, lListElem *event, void *clientdata)
{
   char buffer[1024];
   dstring buffer_wrapper;

   DENTER(TOP_LAYER, "print_event");

   sge_dstring_init(&buffer_wrapper, buffer, sizeof(buffer));

   fprintf(stdout, "%s\n", event_text(event, &buffer_wrapper));
   fflush(stdout);
   /* create a callback error to test error handling */
   if(type == SGE_TYPE_GLOBAL_CONFIG) {
      DEXIT;
      return SGE_EMA_FAILURE;
   }
   
   DEXIT;
   return SGE_EMA_OK;
}

#ifndef QEVENT_SHOW_ALL
static sge_callback_result
print_jatask_event(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
                   sge_event_action action, lListElem *event, void *clientdata)
{
   char buffer[1024];
   dstring buffer_wrapper;

   DENTER(TOP_LAYER, "print_jatask_event");

   sge_dstring_init(&buffer_wrapper, buffer, sizeof(buffer));
   
   DPRINTF(("%s\n", event_text(event, &buffer_wrapper)));
   if (lGetPosViaElem(event, ET_type, SGE_NO_ABORT) >= 0) {
      u_long32 type = lGetUlong(event, ET_type);
      u_long32 timestamp = lGetUlong(event, ET_timestamp);
      
      if (type == sgeE_JATASK_MOD) { 
         lList *jat = lGetList(event,ET_new_version);
         u_long job_id  = lGetUlong(event, ET_intkey);
         u_long task_id = lGetUlong(event, ET_intkey2);
         lListElem *ep = lFirst(jat);
         u_long job_status = lGetUlong(ep, JAT_status);
         int task_running = (job_status==JRUNNING || job_status==JTRANSFERING);

         if (task_running) {
            fprintf(stdout,"JOB_START (%ld.%ld:ECL_TIME="sge_U32CFormat")\n", job_id ,task_id,sge_u32c(timestamp));
            fflush(stdout);  
            Global_jobs_running++;
         }
      }
   
      if (type == sgeE_JOB_FINAL_USAGE) { 
         /* lList *jat = lGetList(event,ET_new_version); */
         u_long job_id = lGetUlong(event, ET_intkey);
         u_long task_id = lGetUlong(event, ET_intkey2);
         /* lWriteElemTo(event, stdout); */
         fprintf(stdout,"JOB_FINISH (%ld.%ld:ECL_TIME="sge_U32CFormat")\n", job_id, task_id,sge_u32c(timestamp));
         Global_jobs_running--;
         fflush(stdout);  
      }
      if (type == sgeE_JOB_ADD) { 
         lList *jat = lGetList(event,ET_new_version);
         u_long job_id  = lGetUlong(event, ET_intkey);
         u_long task_id = lGetUlong(event, ET_intkey2);
         lListElem *ep = lFirst(jat);
         const char* job_project = lGetString(ep, JB_project);
         if (job_project == NULL) {
            job_project = "NONE";
         }
         fprintf(stdout,"JOB_ADD (%ld.%ld:ECL_TIME="sge_U32CFormat":project=%s)\n", job_id, task_id, sge_u32c(timestamp),job_project);
         Global_jobs_registered++;
         fflush(stdout);  
      }
      if (type == sgeE_JOB_DEL) { 
         u_long job_id  = lGetUlong(event, ET_intkey);
         u_long task_id = lGetUlong(event, ET_intkey2);
         fprintf(stdout,"JOB_DEL (%ld.%ld:ECL_TIME="sge_U32CFormat")\n", job_id, task_id,sge_u32c(timestamp));
         Global_jobs_registered--;
         fflush(stdout);  
      }

   }
   /* create a callback error to test error handling */
   if(type == SGE_TYPE_GLOBAL_CONFIG) {
      DEXIT;
      return SGE_EMA_FAILURE;
   }
   
   DEXIT;
   return SGE_EMA_OK;
}
#endif

static sge_callback_result
analyze_jatask_event(sge_evc_class_t *evc, object_description *object_base,sge_object_type type, 
                     sge_event_action action, lListElem *event, void *clientdata)
{
   char buffer[1024];
   dstring buffer_wrapper;

   sge_dstring_init(&buffer_wrapper, buffer, sizeof(buffer));
   
   if (lGetPosViaElem(event, ET_type, SGE_NO_ABORT) >= 0) {
      u_long32 type = lGetUlong(event, ET_type);

      if (type == sgeE_JATASK_MOD) { 
         lList *jat = lGetList(event,ET_new_version);
         lListElem *ep = lFirst(jat);
         u_long job_status = lGetUlong(ep, JAT_status);
         int task_running = (job_status==JRUNNING || job_status==JTRANSFERING);
         if (task_running) {
         }
      }

      if (type == sgeE_JOB_FINAL_USAGE) { 
      }

      if (type == sgeE_JOB_ADD) { 
         /* lList *jat = lGetList(event,ET_new_version);
         u_long job_id  = lGetUlong(event, ET_intkey);
         u_long task_id = lGetUlong(event, ET_intkey2);
         lListElem *ep = lFirst(jat); */
      }

      if (type == sgeE_JOB_DEL) { 
         qevent_trigger_scripts(QEVENT_JB_END, qevent_get_option_struct(), event);
      }

      if (type == sgeE_JATASK_DEL) { 
         qevent_trigger_scripts(QEVENT_JB_TASK_END,qevent_get_option_struct() , event);
      }


   }
   /* create a callback error to test error handling */
   if(type == SGE_TYPE_GLOBAL_CONFIG) {
      return SGE_EMA_FAILURE;
   }
   
   return SGE_EMA_OK;
}



static void qevent_trigger_scripts( int qevent_event, qevent_options *option_struct, lListElem *event) {

   int i=0;
   DENTER(TOP_LAYER, "qevent_trigger_scripts");
   if (option_struct->trigger_option_count > 0) {
      INFO((SGE_EVENT, "trigger for event "SFN"\n", qevent_get_event_name(qevent_event) ));
      for (i=0;i<option_struct->trigger_option_count;i++) {
         if ( (option_struct->trigger_option_events)[i] == qevent_event ) {
            qevent_start_trigger_script(qevent_event ,(option_struct->trigger_option_scripts)[i], event);
         }
      }
   }
   DEXIT;
}

static void qevent_start_trigger_script(int qevent_event, const char* script_file, lListElem *event ) {
   u_long jobid, taskid;
   const char* event_name;
   int pid;
   char buffer[MAX_STRING_SIZE];
   char buffer2[MAX_STRING_SIZE];

   DENTER(TOP_LAYER, "qevent_start_trigger_script");

   jobid  = lGetUlong(event, ET_intkey);
   taskid = lGetUlong(event, ET_intkey2);
   event_name = qevent_get_event_name(qevent_event);
   

   /* test if script is executable and valid file */
   if (!sge_is_file(script_file)) {
      ERROR((SGE_EVENT, "no script file: "SFQ"\n", script_file));
      DEXIT;
      return;
   }

   /* is file executable ? */
   if (!sge_is_executable(script_file)) {  
      ERROR((SGE_EVENT, "file not executable: "SFQ"\n", script_file));
      DEXIT;
      return;
   } 

   pid = fork();
   if (pid < 0) {
      ERROR((SGE_EVENT, "fork() error\n"));
      DEXIT;
      return;
   }

   if (pid > 0) {
      int exit_status;

#if !defined(INTERIX)
      struct rusage rusage;
#endif

#if defined(SVR3) || defined(_BSD)
      union wait status;
#else
      int status;
#endif
#if defined(INTERIX)
      waitpid(pid, &status, 0);
#else
      wait3(&status, 0, &rusage);
#endif
#if defined(SVR3) || defined(_BSD)
      exit_status = status.w_retcode;
#else
      exit_status = status;
#endif

      if ( WEXITSTATUS(exit_status) == 0 ) {
         INFO((SGE_EVENT,"exit status of script: "sge_U32CFormat"\n", sge_u32c(WEXITSTATUS(exit_status))));
      } else {
         ERROR((SGE_EVENT,"exit status of script: "sge_U32CFormat"\n", sge_u32c(WEXITSTATUS(exit_status))));
      }
      DEXIT;
      return;
   } else {
      const char *basename = sge_basename( script_file, '/' );
      /*      SETPGRP;  */
      /*      sge_close_all_fds(NULL); */
      sprintf(buffer  ,sge_U32CFormat,sge_u32c(jobid));
      sprintf(buffer2 ,sge_U32CFormat,sge_u32c(taskid)); 
      execlp(script_file, basename, event_name, buffer, buffer2, (char *)0);
   }
   exit(1);
}

static void qevent_show_usage(void) {
   dstring ds;
   char buffer[256];
   
   sge_dstring_init(&ds, buffer, sizeof(buffer));

   fprintf(stdout, "%s\n", feature_get_product_name(FS_SHORT_VERSION, &ds));
   fprintf(stdout, "%s\n", MSG_SRC_USAGE );

   /* fixme:  localize  */
   fprintf(stdout,"qevent [-h|-help]\n");
   fprintf(stdout,"qevent -ts|-testsuite\n");
   fprintf(stdout,"qevent -sm|-subscribe\n");
   fprintf(stdout,"qevent -trigger EVENT SCRIPT [ -trigger EVENT SCRIPT, ... ]\n\n");

   fprintf(stdout,"   -h,  -help             show usage\n");
   fprintf(stdout,"   -ts, -testsuite        run in testsuite mode\n");
   fprintf(stdout,"   -sm, -subscribe        run in subscribe mode\n");
   fprintf(stdout,"   -trigger EVENT SCRIPT  start SCRIPT (executable) when EVENT occurs\n");
   fprintf(stdout,"\n");
   fprintf(stdout,"SCRIPT - path to a executable shell script\n");
   fprintf(stdout,"         1. command line argument: event name\n");
   fprintf(stdout,"         2. command line argument: jobid\n");
   fprintf(stdout,"         3. command line argument: taskid\n");
   fprintf(stdout,"EVENT  - One of the following event category:\n");
   fprintf(stdout,"         %s      - job end event\n", qevent_get_event_name(QEVENT_JB_END));
   fprintf(stdout,"         %s - job task end event\n", qevent_get_event_name(QEVENT_JB_TASK_END));
}


static void qevent_parse_command_line(int argc, char **argv, qevent_options *option_struct) {

   
   DENTER(TOP_LAYER, "qevent_parse_command_line");

   option_struct->help_option = 0;
   option_struct->testsuite_option = 0;
   option_struct->subscribe_option = 0;
   option_struct->trigger_option_count =0;

   while (*(++argv)) {
      if (!strcmp("-h", *argv) || !strcmp("-help", *argv)) {
         option_struct->help_option = 1;
         continue;
      }
      if (!strcmp("-ts", *argv) || !strcmp("-testsuite", *argv)) {
         option_struct->testsuite_option = 1;
         continue;
      }
      if (!strcmp("-sm", *argv) || !strcmp("-subscribe", *argv)) {
         option_struct->subscribe_option = 1;
         continue;
      }
      if (!strcmp("-trigger", *argv)) {
         int ok = 0;
         if (option_struct->trigger_option_count >= MAX_TRIGGER_SCRIPTS ) {
            sge_dstring_sprintf(option_struct->error_message,
                                "option \"-trigger\": only "sge_U32CFormat" trigger arguments supported\n",
                                sge_u32c(MAX_TRIGGER_SCRIPTS) );
            break; 
         }

         ++argv;
         if (*argv) {
            /* get EVENT argument */
            if (strcmp(qevent_get_event_name(QEVENT_JB_END),*argv) == 0) {
               ok = 1;
               (option_struct->trigger_option_events)[option_struct->trigger_option_count] = QEVENT_JB_END;
            } 
            if (strcmp(qevent_get_event_name(QEVENT_JB_TASK_END),*argv) == 0) {
               ok = 1;
               (option_struct->trigger_option_events)[option_struct->trigger_option_count] = QEVENT_JB_TASK_END;
            } 

            if (!ok) {
               sge_dstring_append(option_struct->error_message,"option \"-trigger\": undefined EVENT type\n");
               break; 
            }
         } else {
            sge_dstring_append(option_struct->error_message,"option \"-trigger\": found no EVENT argument\n");
            break;
         }
         ++argv;
         if (*argv) {
            /* get SCRIPT argument */

            /* check for SCRIPT file */
            if (!sge_is_file(*argv)) {
               sge_dstring_sprintf(option_struct->error_message,
                                   "option \"-trigger\": SCRIPT file %s not found\n",
                                   (*argv));
               break;
            }

            /* is file executable ? */
            if (!sge_is_executable(*argv)) {  
               sge_dstring_sprintf(option_struct->error_message,
                                   "option \"-trigger\": SCRIPT file %s not executable\n",
                                   (*argv));
               break;

            } 
 
            (option_struct->trigger_option_scripts)[option_struct->trigger_option_count] = *argv;
            (option_struct->trigger_option_count)++;
         } else {
            sge_dstring_append(option_struct->error_message,"option \"-trigger\": found no SCRIPT argument\n");
            break;
         }
         continue;
      }


      /* unknown option */
      if ( *argv[0] == '-' ) {  
         sge_dstring_append(option_struct->error_message,"unknown option: ");
      } else {
         sge_dstring_append(option_struct->error_message,"unknown argument: ");
      }
      sge_dstring_append(option_struct->error_message,*argv);
      sge_dstring_append(option_struct->error_message,"\n");
   } 
   DEXIT;
}

int main(int argc, char *argv[])
{
   qevent_options enabled_options;
   dstring errors = DSTRING_INIT;
   int i, gdi_setup;
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL; 
   sge_evc_class_t *evc = NULL;

   DENTER_MAIN(TOP_LAYER, "qevent");

/*    sge_mt_init(); */

   /* dump pid to file */
   qevent_dump_pid_file();

   log_state_set_log_gui(1);
   sge_setup_sig_handlers(QEVENT);

   /* setup event client */
   gdi_setup = sge_gdi2_setup(&ctx, QEVENT, MAIN_THREAD, &alp);
   if (gdi_setup != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT((void**)&ctx, 1);
   }

   /* parse command line */
   enabled_options.error_message = &errors;
   qevent_set_option_struct(&enabled_options);
   qevent_parse_command_line(argc, argv, &enabled_options);

   

   /* check if help option is set */
   if (enabled_options.help_option) {
      qevent_show_usage();
      sge_dstring_free(enabled_options.error_message);
      SGE_EXIT((void**)&ctx, 0);
   }

   /* are there command line parsing errors ? */
   if (sge_dstring_get_string(enabled_options.error_message)) {
      ERROR((SGE_EVENT, "%s", sge_dstring_get_string(enabled_options.error_message) ));
      qevent_show_usage();
      sge_dstring_free(enabled_options.error_message);
      SGE_EXIT((void**)&ctx, 1);
   }

   /* TODO: how is the memory we allocate here released ???, SGE_EXIT doesn't */
   if (false == sge_gdi2_evc_setup(&evc, ctx, EV_ID_ANY, &alp, NULL)) {
      answer_list_output(&alp);
      sge_dstring_free(enabled_options.error_message);
      SGE_EXIT((void**)&ctx, 1);
   }

   /* ok, start over ... */
   /* check for testsuite option */
   
   if (enabled_options.testsuite_option) {
      /* only for testsuite */
      qevent_testsuite_mode(evc);
      sge_dstring_free(enabled_options.error_message);
      SGE_EXIT((void**)&ctx, 0);
   }

   /* check for subscribe option */
   if (enabled_options.subscribe_option) {
      /* only for testsuite */
      qevent_subscribe_mode(evc);
      sge_dstring_free(enabled_options.error_message);
      SGE_EXIT((void**)&ctx, 0);
   }

   if (enabled_options.trigger_option_count > 0) {
      lCondition *where =NULL;
      lEnumeration *what = NULL;

      sge_mirror_initialize(evc, EV_ID_ANY, "sge_mirror -trigger", true, 
                            NULL, NULL, NULL, NULL, NULL);
      evc->ec_set_busy_handling(evc, EV_BUSY_UNTIL_ACK);

      /* put out information about -trigger option */
      for (i=0;i<enabled_options.trigger_option_count;i++) {
         INFO((SGE_EVENT, "trigger script for %s events: %s\n",
                         qevent_get_event_name((enabled_options.trigger_option_events)[i]), 
                         (enabled_options.trigger_option_scripts)[i]));
         switch((enabled_options.trigger_option_events)[i]) {
            case QEVENT_JB_END:
                  
                  /* build mask for the job structure to contain only the needed elements */
                  where = NULL; 
                  what = lWhat("%T(%I %I %I %I %I %I %I %I)", JB_Type, JB_job_number, JB_ja_tasks, 
                                                              JB_ja_structure, JB_ja_n_h_ids, JB_ja_u_h_ids, 
                                                              JB_ja_s_h_ids,JB_ja_o_h_ids, JB_ja_template);
                  
                  /* register for job events */ 
                  sge_mirror_subscribe(evc, SGE_TYPE_JOB, analyze_jatask_event, NULL, NULL, where, what);
                  evc->ec_set_flush(evc, sgeE_JOB_DEL,true, 1);

                  /* the mirror interface registers more events, than we need,
                     thus we free the ones, we do not need */
                /*  evc->ec_unsubscribe(evc, sgeE_JOB_LIST); */
                  evc->ec_unsubscribe(evc, sgeE_JOB_MOD);
                  evc->ec_unsubscribe(evc, sgeE_JOB_MOD_SCHED_PRIORITY);
                  evc->ec_unsubscribe(evc, sgeE_JOB_USAGE);
                  evc->ec_unsubscribe(evc, sgeE_JOB_FINAL_USAGE);
               /*   evc->ec_unsubscribe(evc, sgeE_JOB_ADD); */

                  /* free the what and where mask */
                  lFreeWhere(&where);
                  lFreeWhat(&what);
               break;
            case QEVENT_JB_TASK_END:
            
                  /* build mask for the job structure to contain only the needed elements */
                  where = NULL; 
                  what = lWhat("%T(%I)", JAT_Type, JAT_status);
                  /* register for JAT events */ 
                  sge_mirror_subscribe(evc, SGE_TYPE_JATASK, analyze_jatask_event, NULL, NULL, where, what);
                  evc->ec_set_flush(evc, sgeE_JATASK_DEL,true, 1);
                  
                  /* the mirror interface registers more events, than we need,
                     thus we free the ones, we do not need */ 
                  evc->ec_unsubscribe(evc, sgeE_JATASK_ADD);
                  evc->ec_unsubscribe(evc, sgeE_JATASK_MOD);
                  /* free the what and where mask */
                  lFreeWhere(&where);
                  lFreeWhat(&what);
               break;
         }        
      }

      while(!shut_me_down) {
         sge_mirror_error error = sge_mirror_process_events(evc);
         if (error == SGE_EM_TIMEOUT && !shut_me_down ) {
            sleep(10);
            continue;
         }
      }

      sge_mirror_shutdown(evc);

      sge_dstring_free(enabled_options.error_message);
      sge_prof_cleanup();
      SGE_EXIT((void**)&ctx, 0);
      return 0;
   }


   ERROR((SGE_EVENT, "no option selected\n" ));
   qevent_show_usage();
   sge_dstring_free(enabled_options.error_message);
   sge_prof_cleanup();
   SGE_EXIT((void**)&ctx, 1);
   return 1;
}

static char* qevent_get_event_name(int event) {
  
   switch(event) {
      case QEVENT_JB_END:
         return "JB_END";
      case QEVENT_JB_TASK_END:
         return "JB_TASK_END";
   }
   return "unexpected event id";
}



static void qevent_testsuite_mode(sge_evc_class_t *evc) 
{
#ifndef QEVENT_SHOW_ALL
   u_long32 timestamp;
   lCondition *where =NULL;
   lEnumeration *what = NULL;
 
   const int job_nm[] = {       
         JB_job_number,
         JB_host,
         JB_category,            
         JB_project, 
         JB_ja_tasks,
         JB_ja_structure,
         JB_ja_n_h_ids,
         JB_ja_u_h_ids,
         JB_ja_s_h_ids,
         JB_ja_o_h_ids,   
         JB_ja_template,
         NoName
      };

   const int jat_nm[] = {     
      JAT_status, 
      JAT_task_number,
      NoName
   };  
#endif
   
   DENTER(TOP_LAYER, "qevent_testsuite_mode");

   sge_mirror_initialize(evc, EV_ID_ANY, "qevent", true,
                         NULL, NULL, NULL, NULL, NULL);

#ifdef QEVENT_SHOW_ALL
   sge_mirror_subscribe(evc, SGE_TYPE_ALL, print_event, NULL, NULL, NULL, NULL);
#else /* QEVENT_SHOW_ALL */
   where = NULL; 
   what =  lIntVector2What(JB_Type, job_nm); 
   sge_mirror_subscribe(evc, SGE_TYPE_JOB, print_jatask_event, NULL, NULL, where, what);
   lFreeWhere(&where);
   lFreeWhat(&what);
   
   where = NULL; 
   what = lIntVector2What(JAT_Type, jat_nm); 
   sge_mirror_subscribe(evc, SGE_TYPE_JATASK, print_jatask_event, NULL, NULL, where, what);
   lFreeWhere(&where);
   lFreeWhat(&what);
 
   /* we want a 5 second event delivery interval */
   evc->ec_set_edtime(evc, 5);

   /* and have our events flushed immediately */
   evc->ec_set_flush(evc, sgeE_JATASK_MOD, true, 1);
   evc->ec_set_flush(evc, sgeE_JOB_FINAL_USAGE, true, 1);
   evc->ec_set_flush(evc, sgeE_JOB_ADD, true, 1);
   evc->ec_set_flush(evc, sgeE_JOB_DEL, true, 1);

#endif /* QEVENT_SHOW_ALL */
   
   while (!shut_me_down) {
      sge_mirror_error error = sge_mirror_process_events(evc);
      if (error == SGE_EM_TIMEOUT && !shut_me_down) {
         sleep(10);
         continue;
      }

#ifndef QEVENT_SHOW_ALL
      timestamp = sge_get_gmt();
      fprintf(stdout,"ECL_STATE (jobs_running=%ld:jobs_registered=%ld:ECL_TIME="sge_U32CFormat")\n",
              Global_jobs_running,Global_jobs_registered,sge_u32c(timestamp));
      fflush(stdout);  
#endif
   }

   sge_mirror_shutdown(evc);

   DEXIT;
}

/****** qevent/qevent_subscribe_mode() *****************************************
*  NAME
*     qevent_subscribe_mode() -- ??? 
*
*  SYNOPSIS
*     static void qevent_subscribe_mode(sge_evc_class_t *evc) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     sge_evc_class_t *evc - ??? 
*
*  RESULT
*     static void - 
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     MT-NOTE: qevent_subscribe_mode() is not MT safe 
*
*  BUGS
*     ??? 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static void qevent_subscribe_mode(sge_evc_class_t *evc) 
{
   sge_object_type event_type = SGE_TYPE_ADMINHOST;
   
   DENTER(TOP_LAYER, "qevent_subscribe_mode");

   sge_mirror_initialize(evc, EV_ID_ANY, "qevent", true,
                         NULL, NULL, NULL, NULL, NULL);
   sge_mirror_subscribe(evc, SGE_TYPE_SHUTDOWN, print_event, NULL, NULL, NULL, NULL);
   sge_mirror_subscribe(evc, SGE_TYPE_ADMINHOST, print_event, NULL, NULL, NULL, NULL);

   while(!shut_me_down) {
      sge_mirror_error error = sge_mirror_process_events(evc);
      if (evc != NULL) {
         if (event_type < SGE_TYPE_NONE) {
            event_type++;
            printf("Subscribe event_type: %d\n", event_type);
            error = sge_mirror_subscribe(evc, event_type, print_event, NULL, NULL, NULL, NULL);
         } else {   
            event_type = SGE_TYPE_ADMINHOST;
            printf("Unsubscribe all event_types\n");
            error = sge_mirror_unsubscribe(evc, SGE_TYPE_ALL);
         }
      }   
      if (error == SGE_EM_TIMEOUT && !shut_me_down) {
         printf("error was SGE_EM_TIMEOUT\n");
         sleep(10);
         continue;
      }
   }

   sge_mirror_shutdown(evc);

   DEXIT;
}

