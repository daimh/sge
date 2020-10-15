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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"
#include "uti/sge_profiling.h"
#include "uti/sge_prog.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_event.h"

#include "comm/commlib.h"

#include "mir/sge_mirror.h"

#include "spool/sge_spooling.h"
#include "spool/loader/sge_spooling_loader.h"
#include "spool/flatfile/sge_spooling_flatfile.h"

#include "sig_handlers.h"
#include "usage.h"
#include "msg_clients_common.h"

static lListElem* sge_get_configuration_for_host(const char* aName)
{
   lListElem *conf = NULL;
   char unique_name[CL_MAXHOSTLEN];
   int ret = -1;
   lList *cluster_config = *object_type_get_master_list(SGE_TYPE_CONFIG);

   DENTER(TOP_LAYER, "sge_get_configuration_for_host");

   SGE_ASSERT((NULL != aName));

   /*
    * Due to CR 6319231 IZ 1760:
    *    Try to resolve the hostname
    *    if it is not resolveable then
    *       ignore this and use the given hostname
    */
   ret = sge_resolve_hostname(aName, unique_name, EH_name, sizeof(unique_name));
   if (CL_RETVAL_OK != ret) {
      DPRINTF(("%s: error %s resolving host %s\n", SGE_FUNC,
              cl_get_error_text(ret), aName));
      strcpy(unique_name, aName);
   }

   conf = lCopyElem(lGetElemHost(cluster_config, CONF_name, unique_name));

   DRETURN(conf);
}

static int sge_read_configuration(sge_gdi_ctx_class_t *ctx, lListElem *aSpoolContext, lList *anAnswer)
{
   lListElem *local = NULL;
   lListElem *global = NULL;
   int ret = -1;
   const char *cell_root = ctx->get_cell_root(ctx);
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);
   u_long32 progid = ctx->get_who(ctx);
   lList *cluster_config = *object_type_get_master_list(SGE_TYPE_CONFIG);

   DENTER(TOP_LAYER, "sge_read_configuration");
   
   spool_read_list(&anAnswer, aSpoolContext, &cluster_config, SGE_TYPE_CONFIG);

   answer_list_output(&anAnswer);

   DPRINTF(("qualified_hostname: '%s'\n", qualified_hostname));
   local = sge_get_configuration_for_host(qualified_hostname);
         
   if ((global = sge_get_configuration_for_host(SGE_GLOBAL_NAME)) == NULL) {
      DRETURN(-1);
   }

   ret = merge_configuration(&anAnswer, progid, cell_root, global, local, NULL);
   answer_list_output(&anAnswer);

   lFreeElem(&local);
   lFreeElem(&global);
   
   if (0 != ret) {
      DRETURN(-1);
   }
   
   sge_show_conf();         

   DRETURN(0);
}

/* JG: TODO: This should be a public interface in libspool
 *           that can be used in all modules unspooling data.
 *           Therefore it will be necessary to extract some
 *           special handling and processing still contained
 *           in the reading functions (classic spooling) into
 *           other interfaces, e.g. use callbacks).
 *            
 *           Instead of hardcoding each list, we could loop over
 *           the sge_object_type enum. Problem is currently that
 *           a certain order of unspooling is required.
 *           This could be eliminated by splitting the read list 
 *           functions (from classic spooling) into reading and 
 *           post processing.
 *           
 *           If we do not need to spool/unspool all lists in a certain
 *           spooling client, we could even require that subscription
 *           has been done before calling this functions and call
 *           a function sge_mirror_is_subscribed function to check if we have
 *           to unspool a certain list or not.
 */
static bool read_spooled_data(sge_gdi_ctx_class_t *ctx)
{  
   lList *answer_list = NULL;
   lListElem *context;
   lList *master_list = NULL;
   lList **cluster_configuration = object_type_get_master_list(SGE_TYPE_CONFIG);

   DENTER(TOP_LAYER, "read_spooled_data");

   context = spool_get_default_context();

   /* cluster configuration */
   sge_read_configuration(ctx, context, answer_list);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Config_List\n", lGetNumberOfElem(*cluster_configuration)));

   /* cluster configuration */
   {
      lList *schedd_config = NULL;
      spool_read_list(&answer_list, context, &schedd_config, SGE_TYPE_SCHEDD_CONF);
      if (schedd_config)
         if (sconf_set_config(&schedd_config, &answer_list))
            lFreeList(&schedd_config);
      answer_list_output(&answer_list);
      DPRINTF(("read %d entries to Master_Sched_Config_List\n", lGetNumberOfElem(sconf_get_config_list())));
   }
   /* complexes */
   master_list = *object_type_get_master_list(SGE_TYPE_CENTRY);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_CENTRY);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_CEntry_List\n", lGetNumberOfElem(master_list)));

   /* hosts */
   master_list = *object_type_get_master_list(SGE_TYPE_EXECHOST);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_EXECHOST);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Exechost_List\n", lGetNumberOfElem(master_list)));

   master_list = *object_type_get_master_list(SGE_TYPE_ADMINHOST);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_ADMINHOST);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Adminhost_List\n", lGetNumberOfElem(master_list)));

   master_list = *object_type_get_master_list(SGE_TYPE_SUBMITHOST);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_SUBMITHOST);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Submithost_List\n", lGetNumberOfElem(master_list)));

   /* managers */
   master_list = *object_type_get_master_list(SGE_TYPE_MANAGER);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_MANAGER);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Manager_List\n", lGetNumberOfElem(master_list)));

   /* host groups */
   master_list = *object_type_get_master_list(SGE_TYPE_HGROUP);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_HGROUP);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Hostgroup_List\n", 
            lGetNumberOfElem(master_list)));

   /* operators */
   master_list = *object_type_get_master_list(SGE_TYPE_OPERATOR);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_OPERATOR);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Operator_List\n", lGetNumberOfElem(master_list)));

   /* usersets */
   master_list = *object_type_get_master_list(SGE_TYPE_USERSET);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_USERSET);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Userset_List\n", lGetNumberOfElem(master_list)));

   /* calendars */
   master_list = *object_type_get_master_list(SGE_TYPE_CALENDAR);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_CALENDAR);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Calendar_List\n", lGetNumberOfElem(master_list)));

#ifndef __SGE_NO_USERMAPPING__
   /* user mapping */
   master_list = *object_type_get_master_list(SGE_TYPE_CUSER);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_CUSER);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Cuser_List\n", lGetNumberOfElem(master_list)));
#endif

   /* queues */
   master_list = *object_type_get_master_list(SGE_TYPE_CQUEUE);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_CQUEUE);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_CQueue_List\n", lGetNumberOfElem(master_list)));

   /* pes */
   master_list = *object_type_get_master_list(SGE_TYPE_PE);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_PE);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Pe_List\n", lGetNumberOfElem(master_list)));

   /* ckpt */
   master_list = *object_type_get_master_list(SGE_TYPE_CKPT);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_CKPT);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Ckpt_List\n", lGetNumberOfElem(master_list)));

   /* jobs */
   master_list = *object_type_get_master_list(SGE_TYPE_JOB);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_JOB);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Job_List\n", lGetNumberOfElem(master_list)));

   /* user list */
   master_list = *object_type_get_master_list(SGE_TYPE_USER);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_USER);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_User_List\n", lGetNumberOfElem(master_list)));

   /* project list */
   master_list = *object_type_get_master_list(SGE_TYPE_PROJECT);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_PROJECT);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Project_List\n", lGetNumberOfElem(master_list)));

   /* sharetree */
   master_list = *object_type_get_master_list(SGE_TYPE_SHARETREE);
   spool_read_list(&answer_list, context, &master_list, SGE_TYPE_SHARETREE);
   answer_list_output(&answer_list);
   DPRINTF(("read %d entries to Master_Sharetree_List\n", lGetNumberOfElem(master_list)));

   DRETURN(true);
}

sge_callback_result spool_event_before(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, sge_event_action action, 
                       lListElem *event, void *clientdata)
{
   lList *answer_list = NULL;
   lListElem *context, *ep;
   lList **master_list, *new_list;
   int key_nm;
   dstring buffer = DSTRING_INIT;

   DENTER(TOP_LAYER, "spool_event_before");

   context = spool_get_default_context();
   
   master_list = object_type_get_master_list(type);
   key_nm      = object_type_get_key_nm(type);
   new_list    = lGetList(event, ET_new_version);

   if(action == SGE_EMA_LIST) {
      switch(type) {
         case SGE_TYPE_ADMINHOST:      
         case SGE_TYPE_EXECHOST:
         case SGE_TYPE_SUBMITHOST:
         case SGE_TYPE_CONFIG:
         case SGE_TYPE_HGROUP:
            for_each(ep, *master_list) {
               lListElem *new_ep;

               new_ep = lGetElemHost(new_list, key_nm, lGetHost(ep, key_nm));
               if(new_ep == NULL) {
                  /* object not contained in new list, delete it */
                  spool_delete_object(&answer_list, context, type, lGetHost(ep, key_nm), false);
                  answer_list_output(&answer_list);
               }
            }   

            for_each(ep, new_list) {
               lListElem *old_ep;
               const char *key = lGetHost(ep, key_nm);

               old_ep = lGetElemHost(*master_list, key_nm, key);

               /* check if spooling relevant attributes have changed,
                * if yes, spool the object.
                */
               if (old_ep == NULL || 
                  spool_compare_objects(&answer_list, context, type, ep, old_ep) != 0)  {
                  spool_write_object(&answer_list, context, ep, key, type, false);
                  answer_list_output(&answer_list);
               }
            }
            break;
         case SGE_TYPE_CALENDAR:
         case SGE_TYPE_CKPT:
         case SGE_TYPE_MANAGER:
         case SGE_TYPE_OPERATOR:
         case SGE_TYPE_PE:
         case SGE_TYPE_PROJECT:
         case SGE_TYPE_CQUEUE:
         case SGE_TYPE_USER:
         case SGE_TYPE_USERSET:
#ifndef __SGE_NO_USERMAPPING__
         case SGE_TYPE_CUSER:
#endif
            for_each(ep, *master_list) {
               lListElem *new_ep;

               new_ep = lGetElemStr(new_list, key_nm, lGetString(ep, key_nm));
               if(new_ep == NULL) {
                  /* object not contained in new list, delete it */
                  spool_delete_object(&answer_list, context, type, lGetString(ep, key_nm), false);
                  answer_list_output(&answer_list);
               }
            }

            for_each(ep, new_list) {
               lListElem *old_ep;
               const char *key = lGetString(ep, key_nm);

               old_ep = lGetElemStr(*master_list, key_nm, key);

               /* check if spooling relevant attributes have changed,
                * if yes, spool the object.
                */
               if(old_ep == NULL || 
                  spool_compare_objects(&answer_list, context, type, ep, old_ep))  {
                  spool_write_object(&answer_list, context, ep, key, type, false);
                  answer_list_output(&answer_list);
               }
            }
            break;

         case SGE_TYPE_JOB:
            for_each(ep, *master_list) {
               lListElem *new_ep;

               new_ep = lGetElemUlong(new_list, key_nm, lGetUlong(ep, key_nm));
               if(new_ep == NULL) {
                  const char *job_key;
                  job_key = job_get_key(lGetUlong(ep, key_nm), 0, NULL, &buffer);
                  /* object not contained in new list, delete it */
                  spool_delete_object(&answer_list, context, type, job_key, false);
                  answer_list_output(&answer_list);
               }
            }

            for_each(ep, new_list) {
               lListElem *old_ep;
               u_long32 key = lGetUlong(ep, key_nm);

               old_ep = lGetElemUlong(*master_list, key_nm, key);

               /* check if spooling relevant attributes have changed,
                * if yes, spool the object.
                */
               if(old_ep == NULL || 
                  spool_compare_objects(&answer_list, context, type, ep, old_ep))  {
                  const char *job_key;
                  job_key = job_get_key(lGetUlong(ep, key_nm), 0, NULL, &buffer);
                  spool_write_object(&answer_list, context, ep, job_key, type, false);
                  answer_list_output(&answer_list);
               }
            }
            break;

         case SGE_TYPE_SHARETREE:
            /* two pass algorithm:
             * 1. If we have an old sharetree: delete it
             * 2. If a new sharetree has been sent: write it (spool_event_after)
             * JG: TODO: we have to compare them: delete / write only, when
             *           sharetree no longer exists or has changed
             */
            ep = lFirst(*master_list);
            if(ep != NULL) {
               /* delete sharetree */
               spool_delete_object(&answer_list, context, type, SHARETREE_FILE, false);
               answer_list_output(&answer_list);
            }
            break;
         default:
            break;
      }
   }

   if(action == SGE_EMA_DEL) {
      switch(type) {
            case SGE_TYPE_JATASK:
            case SGE_TYPE_PETASK:
               {
                  u_long32 job_id, ja_task_id;
                  const char *pe_task_id;
                  const char *job_key;

                  job_id = lGetUlong(event, ET_intkey);
                  ja_task_id = lGetUlong(event, ET_intkey2);
                  pe_task_id = lGetString(event, ET_strkey);

                  job_key = job_get_key(job_id, ja_task_id, pe_task_id, &buffer);
                  spool_delete_object(&answer_list, context, type, job_key, false);
                  answer_list_output(&answer_list);
               }
               break;
            case SGE_TYPE_JOB:
               {
                  u_long32 job_id;
                  const char *job_key;

                  job_id = lGetUlong(event, ET_intkey);

                  job_key = job_get_key(job_id, 0, NULL, &buffer);
                  spool_delete_object(&answer_list, context, type, job_key, false);
                  answer_list_output(&answer_list);
               }
               break;

            default:
               break;
      }
   }

   sge_dstring_free(&buffer);
   DRETURN(SGE_EMA_OK);
}

sge_callback_result spool_event_after(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, sge_event_action action, 
                      lListElem *event, void *clientdata)
{
   sge_callback_result ret = SGE_EMA_OK;
   lList *answer_list = NULL;
   lListElem *context, *ep;
   lList **master_list;
   lList *master_job_list;
   int key_nm;
   const char *key;
   dstring buffer = DSTRING_INIT;

   DENTER(TOP_LAYER, "spool_event_after");

   context = spool_get_default_context();
   
   master_list = object_type_get_master_list(type);
   key_nm      = object_type_get_key_nm(type);
   master_job_list = *object_type_get_master_list(SGE_TYPE_JOB);

   switch(action) {
      case SGE_EMA_LIST:
         switch(type) {
            case SGE_TYPE_MANAGER:
            case SGE_TYPE_OPERATOR:
               /* The "classic" spooling functions always write all list entries
                * to the spool file, not individual ones.
                * Therefore we have to call the writing function once after
                * the master list has been updated by the mirroring
                */
               if(strcmp(lGetString(context, SPC_name), "classic spooling") == 0) {
                  ep = lFirst(*master_list);
                  if(ep != NULL) {
                     spool_write_object(&answer_list, context, ep, NULL, type, false);
                     answer_list_output(&answer_list);
                  }
               }   
               break;
            case SGE_TYPE_SHARETREE:
               ep = lFirst(*master_list);
               if(ep != NULL) {
                  /* spool sharetree */
                  spool_write_object(&answer_list, context, ep, SHARETREE_FILE, type, false);
                  answer_list_output(&answer_list);
               }
               break;
            default:
               break;
         }
         break;
   
      case SGE_EMA_DEL:
         switch(type) {
            case SGE_TYPE_ADMINHOST:
            case SGE_TYPE_EXECHOST:
            case SGE_TYPE_SUBMITHOST:
            case SGE_TYPE_CONFIG:
            case SGE_TYPE_CALENDAR:
            case SGE_TYPE_CKPT:
            case SGE_TYPE_MANAGER:
            case SGE_TYPE_OPERATOR:
            case SGE_TYPE_PE:
            case SGE_TYPE_PROJECT:
            case SGE_TYPE_CQUEUE:
            case SGE_TYPE_USER:
            case SGE_TYPE_USERSET:
#ifndef __SGE_NO_USERMAPPING__
            case SGE_TYPE_CUSER:
#endif   
            case SGE_TYPE_HGROUP:
               key = lGetString(event, ET_strkey);
               spool_delete_object(&answer_list, context, type, key, false);
               answer_list_output(&answer_list);

            default:
               break;
         }
         break;

      case SGE_EMA_ADD:
      case SGE_EMA_MOD:
         switch(type) {
            case SGE_TYPE_ADMINHOST:
            case SGE_TYPE_EXECHOST:
            case SGE_TYPE_SUBMITHOST:
            case SGE_TYPE_CONFIG:
               key = lGetString(event, ET_strkey);
               ep = lGetElemHost(*master_list, key_nm, lGetString(event, ET_strkey));
               if(ep == NULL) {
                  ERROR((SGE_EVENT, "%s element with id "SFQ" not found\n",
                         object_type_get_name(type), key));
                  ret = SGE_EMA_FAILURE;
               }
               if (ret == SGE_EMA_OK) {
                  spool_write_object(&answer_list, context, ep, key, type, false);
                  answer_list_output(&answer_list);
               }
               break;

            case SGE_TYPE_CALENDAR:
            case SGE_TYPE_CKPT:
            case SGE_TYPE_MANAGER:
            case SGE_TYPE_OPERATOR:
            case SGE_TYPE_PE:
            case SGE_TYPE_PROJECT:
            case SGE_TYPE_CQUEUE:
            case SGE_TYPE_USER:
            case SGE_TYPE_USERSET:
#ifndef __SGE_NO_USERMAPPING__
            case SGE_TYPE_CUSER:
#endif
            case SGE_TYPE_HGROUP:
               key = lGetString(event, ET_strkey);
               ep = lGetElemStr(*master_list, key_nm, lGetString(event, ET_strkey));
               if(ep == NULL) {
                  ERROR((SGE_EVENT, "%s element with id "SFQ" not found\n",
                         object_type_get_name(type), key));
                  ret = SGE_EMA_FAILURE;
               }
               
               if (ret == SGE_EMA_OK) {
                  spool_write_object(&answer_list, context, ep, key, type, false);
                  answer_list_output(&answer_list);
               }
               break;

            case SGE_TYPE_SCHEDD_CONF:
               ep = lFirst(*master_list);
               if(ep == NULL) {
                  ERROR((SGE_EVENT, "%s element not found\n",
                         object_type_get_name(type)));
                  ret = SGE_EMA_FAILURE;
               }
               if (ret == SGE_EMA_OK) {
                  spool_write_object(&answer_list, context, ep, "default", type, false);
                  answer_list_output(&answer_list);
               }
               break;
            case SGE_TYPE_JATASK:
            case SGE_TYPE_PETASK:
            case SGE_TYPE_JOB:
               {
                  u_long32 job_id, ja_task_id;
                  const char *pe_task_id;
                  const char *job_key;

                  job_id = lGetUlong(event, ET_intkey);
                  ja_task_id = lGetUlong(event, ET_intkey2);
                  pe_task_id = lGetString(event, ET_strkey);

                  ep = lGetElemUlong(master_job_list, JB_job_number, job_id);
                  job_key = job_get_key(job_id, ja_task_id, pe_task_id, &buffer);
                  spool_write_object(&answer_list, context, ep, job_key, type, false);
                  answer_list_output(&answer_list);
               }
               break;
            
            default:
               break;
         }
         break;
         
      default:
         break;
   }

   sge_dstring_free(&buffer);

   DRETURN(ret);
}

int main(int argc, char *argv[])
{
   lListElem *spooling_context;
   time_t next_prof_output = 0;
   lList *answer_list = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   sge_evc_class_t *evc = NULL;

   DENTER_MAIN(TOP_LAYER, "test_sge_spooling");

   /* parse commandline parameters */
   if(argc != 4) {
      ERROR((SGE_EVENT, "usage: test_sge_spooling <method> <shared lib> <arguments>\n"));
      SGE_EXIT((void**)&ctx, 1);
   }

   if (sge_gdi2_setup(&ctx, QEVENT, MAIN_THREAD, &answer_list) != AE_OK) {
      answer_list_output(&answer_list);
      SGE_EXIT((void**)&ctx, 1);
   }
   
   sge_setup_sig_handlers(QEVENT);

   if (ctx->reresolve_qualified_hostname(ctx) != CL_RETVAL_OK) {
      SGE_EXIT((void**)&ctx, 1);
   }

   if (false == sge_gdi2_evc_setup(&evc, ctx, EV_ID_SCHEDD, &answer_list, NULL)) {
      answer_list_output(&answer_list);
      SGE_EXIT((void**)&ctx, 1);
   }

#define defstring(str) #str

   /* initialize spooling */
   spooling_context = spool_create_dynamic_context(&answer_list, argv[1], argv[2], argv[3]); 
   answer_list_output(&answer_list);
   if(spooling_context == NULL) {
      SGE_EXIT((void**)&ctx, EXIT_FAILURE);
   }

   spool_set_default_context(spooling_context);

   if(!spool_startup_context(&answer_list, spooling_context, true)) {
      answer_list_output(&answer_list);
      SGE_EXIT((void**)&ctx, EXIT_FAILURE);
   }
   answer_list_output(&answer_list);
   
   /* read spooled data from disk */
   read_spooled_data(ctx);
   
   /* initialize mirroring */
   sge_mirror_initialize(evc, EV_ID_ANY, "test_sge_mirror", true, NULL, NULL, NULL, NULL, NULL);
   sge_mirror_subscribe(evc, SGE_TYPE_ALL, spool_event_before, spool_event_after, NULL, NULL, NULL);
   prof_start(SGE_PROF_ALL, NULL);

   while(!shut_me_down) {
      time_t now;
      
      sge_mirror_process_events(evc);

      now = time(0);
      if (now > next_prof_output) {
         prof_output_info(SGE_PROF_ALL, false, "test_sge_info:\n");
/*          INFO((SGE_EVENT, "\n%s", prof_get_info_string(SGE_PROF_ALL, false, NULL))); */
         next_prof_output = now + 60;
      }
   }

   sge_mirror_shutdown(evc);

   spool_shutdown_context(&answer_list, spooling_context);
   answer_list_output(&answer_list);

   DRETURN(EXIT_SUCCESS);
}
