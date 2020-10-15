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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_hostname.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_cuser.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_href.h"

#include "spool/sge_spooling.h"

#include "sge.h"
#include "sge_c_gdi.h"
#include "sge_hgroup_qmaster.h"
#include "sge_event_master.h"
#include "sge_cqueue_qmaster.h"
#include "sge_host_qmaster.h"
#include "sge_utility_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"


/* EB: ADOC: add comments */

static bool
hgroup_mod_hostlist(lListElem *hgroup, lList **answer_list,
                    lListElem *reduced_elem, int sub_command,
                    lList **add_hosts, lList **rem_hosts,
                    lList **occupant_groups);

static void 
hgroup_commit(sge_gdi_ctx_class_t *ctx, lListElem *hgroup);

static void 
hgroup_rollback(lListElem *this_elem);

static bool
hgroup_mod_hostlist(lListElem *hgroup, lList **answer_list,
                    lListElem *reduced_elem, int sub_command,
                    lList **add_hosts, lList **rem_hosts,
                    lList **occupant_groups)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_mod_hostlist");
   if (hgroup != NULL && reduced_elem != NULL) {
      int pos = lGetPosViaElem(reduced_elem, HGRP_host_list, SGE_NO_ABORT);

      if (pos >= 0) {
         lList *list = lGetPosList(reduced_elem, pos);
         lList *old_href_list = lCopyList("", lGetList(hgroup, HGRP_host_list));
         lList *master_list = *(hgroup_list_get_master_list());
         lList *href_list = NULL;
         lList *add_groups = NULL;
         lList *rem_groups = NULL;

         if (ret) {
            ret &= href_list_resolve_hostnames(list, answer_list, true);
         }
         if (ret) {
            attr_mod_sub_list(answer_list, hgroup, HGRP_host_list, HR_name,
                              reduced_elem, sub_command, SGE_ATTR_HOSTLIST,
                              SGE_OBJ_HGROUP, 0, NULL);
            href_list = lGetList(hgroup, HGRP_host_list);
         }
         if (ret) {
            ret &= href_list_find_diff(href_list, answer_list, old_href_list,
                                       add_hosts, rem_hosts, &add_groups,
                                       &rem_groups);
         }
         if (ret && add_groups != NULL) {
            ret &= hgroup_list_exists(master_list, answer_list, add_groups);
         }
         if (ret) {
            ret &= href_list_find_effective_diff(answer_list, add_groups,
                                                 rem_groups, master_list,
                                                 add_hosts, rem_hosts);
         }
         if (ret) {
            ret &= href_list_resolve_hostnames(*add_hosts, answer_list, false);
         }

         /*
          * Try to find cycles in the definition
          */
         if (ret) {
            ret &= hgroup_find_all_referencees(hgroup, answer_list,
                                               master_list, occupant_groups);
            ret &= href_list_add(occupant_groups, answer_list,
                                 lGetHost(hgroup, HGRP_name));
            if (ret) {
               if (*occupant_groups != NULL && add_groups != NULL) {
                  lListElem *add_group = NULL;

                  for_each(add_group, add_groups) {
                     const char *name = lGetHost(add_group, HR_name);

                     if (href_list_has_member(*occupant_groups, name)) {
                        break;
                     }
                  }
                  if (add_group == NULL) {
                     /*
                      * No cycle found => success
                      */
                     ;
                  } else {
                     SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_HGROUP_CYCLEINDEF_SS,
                                            lGetHost(add_group, HR_name),
                                            lGetHost(hgroup, HGRP_name)));
                     answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX,
                                     ANSWER_QUALITY_ERROR);
                     ret = false;
                  }
               }
            }
         }

         /*
          * Make sure that:
          *   - added hosts where not already part the old hostlist
          *   - removed hosts are not part of the new hostlist
          */
         if (ret) {
            lList *tmp_hosts = NULL;

            ret &= href_list_find_all_references(old_href_list, answer_list,
                                                 master_list, &tmp_hosts, NULL);
            ret &= href_list_remove_existing(add_hosts, answer_list, tmp_hosts);
            lFreeList(&tmp_hosts);

            ret &= href_list_find_all_references(href_list, answer_list,
                                                 master_list, &tmp_hosts, NULL);
            ret &= href_list_remove_existing(rem_hosts, answer_list, tmp_hosts);
            lFreeList(&tmp_hosts);
         }

#if 1 /* debug */
         if (ret) {
            href_list_debug_print(*add_hosts, "add_hosts: ");
            href_list_debug_print(*rem_hosts, "rem_hosts: ");
         }
#endif

         /*
          * Cleanup
          */
         lFreeList(&old_href_list);
         lFreeList(&add_groups);
         lFreeList(&rem_groups);
      }
   }
   DEXIT;
   return ret;
}

static void 
hgroup_commit(sge_gdi_ctx_class_t *ctx, lListElem *hgroup) 
{
   lList *cqueue_master_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
   lList *cqueue_list = lGetList(hgroup, HGRP_cqueue_list);
   lListElem *next_cqueue = NULL;
   lListElem *cqueue = NULL;

   DENTER(TOP_LAYER, "hgroup_commit");
   next_cqueue = lFirst(cqueue_list);
   while ((cqueue = next_cqueue)) {
      const char *name = lGetString(cqueue, CQ_name);
      lListElem *org_queue = lGetElemStr(cqueue_master_list, CQ_name, name);

      next_cqueue = lNext(cqueue);
      cqueue_commit(ctx, cqueue);
      lDechainElem(cqueue_list, cqueue);
      lRemoveElem(cqueue_master_list, &org_queue);
      lAppendElem(cqueue_master_list, cqueue);
   }
   lSetList(hgroup, HGRP_cqueue_list, NULL);
   DEXIT;
}

static void 
hgroup_rollback(lListElem *this_elem) 
{
   DENTER(TOP_LAYER, "hgroup_rollback");
   lSetList(this_elem, HGRP_cqueue_list, NULL);
   DEXIT;
}

int 
hgroup_mod(sge_gdi_ctx_class_t *ctx,
           lList **answer_list, lListElem *hgroup, lListElem *reduced_elem,
           int add, const char *remote_user, const char *remote_host, 
           gdi_object_t *object, int sub_command, monitoring_t *monitor) 
{
   bool ret = true;
   int pos;

   DENTER(TOP_LAYER, "hgroup_mod");

   /* Did we get a hostgroupname?  */
   pos = lGetPosViaElem(reduced_elem, HGRP_name, SGE_NO_ABORT);
   if (pos >= 0) {
      const char *name = lGetPosHost(reduced_elem, pos);

      if (add) {
         /* Check groupname for new hostgroups */
         if (hgroup_check_name(answer_list, name)) {
            lSetHost(hgroup, HGRP_name, name); 
         } else {
            lListElem *aep;
            for_each(aep, *answer_list) {
               ERROR((SGE_EVENT, SFNMAX, lGetString(aep, AN_text)));
            }
            ret = false;
         }

      } else {
         const char *old_name = lGetHost(hgroup, HGRP_name);

         /* Reject modify requests which try to change the groupname */
         if (sge_hostcmp(old_name, name)) {
            ERROR((SGE_EVENT, SFNMAX, MSG_HGRP_NONAMECHANGE));
            answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX,
                            ANSWER_QUALITY_ERROR);
            ret = false;
         }
      }
   } else {
      ERROR((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS, 
             lNm2Str(HGRP_name), SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, 
                      ANSWER_QUALITY_ERROR);
      ret = false;
   }

   /*
    * Is there a list of host references
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, HGRP_host_list, SGE_NO_ABORT);
      if (pos >= 0) {
         lList *add_hosts = NULL;
         lList *rem_hosts = NULL;
         lList *occupant_groups = NULL;
   
         DPRINTF(("got new HGRP_host_list\n")); 

         if (ret) {
            ret &= hgroup_mod_hostlist(hgroup, answer_list, reduced_elem, 
                                       sub_command, &add_hosts, &rem_hosts, 
                                       &occupant_groups);
         }
         if (ret) {
            lList *cqueue_master_list = 
                               *(object_type_get_master_list(SGE_TYPE_CQUEUE));
            lListElem *cqueue;

            for_each (cqueue, cqueue_master_list) {
               if (cqueue_is_a_href_referenced(cqueue, occupant_groups, true)) {
                  lListElem *new_cqueue = NULL;
                  lList *real_add_hosts = NULL;
                  lList *real_rem_hosts = NULL;
                  lList *before_mod_list = NULL;
                  lList *after_mod_list = NULL;
                  const lList *href_list = NULL;
                  const char *name = NULL;
                  lListElem *org_hgroup = NULL;
                  lList *master_list = NULL;

                  /*
                   * Find CQs lists of referenced hosts before and after
                   * the hgroup modification
                   */

                  href_list = lGetList(cqueue, CQ_hostlist);
                  name = lGetHost(hgroup, HGRP_name);
                  master_list = *(hgroup_list_get_master_list());
                  org_hgroup = lGetElemHost(master_list, HGRP_name, name);

                  ret &= href_list_find_all_references(href_list, 
                                                       answer_list,
                                                       master_list, 
                                                       &before_mod_list,
                                                       NULL);

                  /*
                   * !!! Modify master list temorarily 
                   *     (find rollback below)
                   */
                  if (org_hgroup != NULL) {
                     lDechainElem(master_list, org_hgroup);
                  }
                  lAppendElem(master_list, hgroup);
                  ret &= href_list_find_all_references(href_list, 
                                                       answer_list,
                                                       master_list, 
                                                       &after_mod_list,
                                                       NULL);

                  /*
                   * Find the real set of hosts to be added/removed from
                   * the list of QIs of the CQ
                   */
                  if (ret) {
                     ret &= href_list_compare(rem_hosts, answer_list,
                                              after_mod_list, 
                                              &real_rem_hosts,
                                              NULL, NULL, NULL);
                     ret &= href_list_compare(add_hosts, answer_list,
                                              before_mod_list, 
                                              &real_add_hosts,
                                              NULL, NULL, NULL);
                  }

                  /*
                   * Make a copy of CQ
                   */
                  if (ret) {
                     lList *cqueue_list = lGetList(hgroup, HGRP_cqueue_list);

                     if (cqueue_list == NULL) {
                        cqueue_list = lCreateList("", CQ_Type);
                        lSetList(hgroup, HGRP_cqueue_list, cqueue_list);
                     }
                     new_cqueue = lCopyElem(cqueue);
                     if (new_cqueue != NULL && cqueue_list != NULL) {
                        lAppendElem(cqueue_list, new_cqueue);
                     } else {
                        ret = false;
                     }
                  }
   
                  /*
                   * Mopdify QIs of CQ
                   */
                  if (ret) {
                     bool refresh_all_values = ((add_hosts != NULL) || (rem_hosts != NULL)) ? true : false;

                     ret &= cqueue_handle_qinstances(ctx,
                                                     new_cqueue, answer_list, 
                                                     reduced_elem,
                                                     real_add_hosts, 
                                                     real_rem_hosts,
                                                     refresh_all_values, monitor);
                  }

                  /*
                   * Free all temorarily allocated memory
                   */
                  lFreeList(&after_mod_list);
                  lFreeList(&before_mod_list);
                  lFreeList(&real_add_hosts);
                  lFreeList(&real_rem_hosts);

                  /*
                   * !!! Rollback of masterlist modification
                   */
                  lDechainElem(master_list, hgroup);
                  if (org_hgroup != NULL) {
                     lAppendElem(master_list, org_hgroup);
                  }

                  /*
                   * Skip other CQs if this failed
                   */
                  if (!ret) {
                     break;
                  }
               }
            }
            if (!ret) { 
               hgroup_rollback(hgroup);
            }
         } 

         /*
          * Client and scheduler code expects existing EH_Type elements
          * for all hosts used in CQ_hostlist. Therefore it is neccessary
          * to create all not existing EH_Type elements.
          */
         if (ret) {
            lList *list = *(object_type_get_master_list(SGE_TYPE_EXECHOST));

            ret &= host_list_add_missing_href(ctx, list, answer_list, add_hosts, monitor);
         }

         lFreeList(&add_hosts);
         lFreeList(&rem_hosts);
         lFreeList(&occupant_groups);
      }  
   } 

   DEXIT;
   if (ret) {
      return 0;
   } else {
      return STATUS_EUNKNOWN;
   }
}

int 
hgroup_del(sge_gdi_ctx_class_t *ctx,
           lListElem *this_elem, lList **answer_list, 
           char *remote_user, char *remote_host) 
{
   int ret = true;

   DENTER(TOP_LAYER, "hgroup_del");
   /*
    * Check all incoming parameter
    */
   if (this_elem != NULL && remote_user != NULL && remote_host != NULL) {
      const char* name = lGetHost(this_elem, HGRP_name);

      /*
       * What is the name ob the hostgroup which should be removed?
       */
      if (name != NULL) {
         lList *master_hgroup_list = *(hgroup_list_get_master_list());
         lList *master_cqueue_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
#ifndef __SGE_NO_USERMAPPING__
         lList *master_cuser_list = *(cuser_list_get_master_list());
#endif
         lListElem *hgroup;

         /*
          * Does this hostgroup exist?
          */
         hgroup = hgroup_list_locate(master_hgroup_list, name);
         if (hgroup != NULL) {
            lList *href_list = NULL;
            lList *qref_list = NULL;
#ifndef __SGE_NO_USERMAPPING__
            lList *string_list = NULL;
#endif

            /*
             * Is it still referenced in another hostgroup or cqueue?
             */
            ret &= hgroup_find_referencees(hgroup, answer_list, 
                                           master_hgroup_list, 
                                           master_cqueue_list,
                                           &href_list,
                                           &qref_list);
            if (ret) {
               if (href_list != NULL) {
                  dstring string = DSTRING_INIT;

                  href_list_append_to_dstring(href_list, &string);
                  ERROR((SGE_EVENT, MSG_HGROUP_REFINHGOUP_SS, name,
                         sge_dstring_get_string(&string)));
                  answer_list_add(answer_list, SGE_EVENT, STATUS_EEXIST,
                                  ANSWER_QUALITY_ERROR);
                  sge_dstring_free(&string);
                  ret = false;
               }
               if (qref_list != NULL) {
                  dstring string = DSTRING_INIT;

                  str_list_append_to_dstring(qref_list, &string, ' ');
                  ERROR((SGE_EVENT, MSG_CQUEUE_REFINHGOUP_SS, name,
                         sge_dstring_get_string(&string)));
                  answer_list_add(answer_list, SGE_EVENT, STATUS_EEXIST,
                                  ANSWER_QUALITY_ERROR);
                  sge_dstring_free(&string);
                  ret = false;
               }
            }
            lFreeList(&href_list);
            lFreeList(&qref_list);


#ifndef __SGE_NO_USERMAPPING__
            /*
             * Is it still referenced in a cluster user object (user mapping)
             */ 
            ret &= cuser_list_find_hgroup_references(master_cuser_list,
                                                    answer_list, hgroup,
                                                    &string_list);
            if (ret) {
               if (string_list != NULL) {
                  dstring string = DSTRING_INIT;

                  str_list_append_to_dstring(string_list, &string, ','); 
                  ERROR((SGE_EVENT, MSG_HGROUP_REFINCUSER_SS, name,
                         sge_dstring_get_string(&string)));
                  answer_list_add(answer_list, SGE_EVENT, STATUS_EEXIST,
                                  ANSWER_QUALITY_ERROR);
                  sge_dstring_free(&string);
                  ret = false;
               }
            }
            lFreeList(&string_list);
#endif
            /*
             * Try to unlink the concerned spoolfile
             */
            if (ret) {
               if (sge_event_spool(ctx, answer_list, 0, sgeE_HGROUP_DEL, 
                                   0, 0, name, NULL, NULL,
                                   NULL, NULL, NULL, true, true)) {
                  /*
                   * Let's remove the object => Success!
                   */

                  lRemoveElem(*object_type_get_master_list(SGE_TYPE_HGROUP), &hgroup);

                  INFO((SGE_EVENT, MSG_SGETEXT_REMOVEDFROMLIST_SSSS, 
                        remote_user, remote_host, name , "host group entry"));
                  answer_list_add(answer_list, SGE_EVENT, STATUS_OK, 
                                  ANSWER_QUALITY_INFO);
               } else {
                  ERROR((SGE_EVENT, MSG_CANTSPOOL_SS,"host group entry",
                         name ));
                  answer_list_add(answer_list, SGE_EVENT, STATUS_EEXIST, 
                                  ANSWER_QUALITY_ERROR);
                  ret = false;
               }
            }
         } else {
            ERROR((SGE_EVENT, MSG_SGETEXT_DOESNOTEXIST_SS, 
                   "host group", name));
            answer_list_add(answer_list, SGE_EVENT, STATUS_EEXIST, 
                            ANSWER_QUALITY_ERROR);
            ret = false;
         }
      } else {
         ERROR((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS, 
                lNm2Str(HGRP_name), SGE_FUNC));
         answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, 
                         ANSWER_QUALITY_ERROR);
         ret = false;
      } 
   } else {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, 
                      ANSWER_QUALITY_ERROR);
      ret = false;        
   }

   DEXIT;
   if (ret) {
      return STATUS_OK;
   } else {
      return STATUS_EUNKNOWN;
   }
}

int 
hgroup_success(sge_gdi_ctx_class_t *ctx, lListElem *hgroup, lListElem *old_hgroup, gdi_object_t *object, lList **ppList, monitoring_t *monitor) 
{
   const char *name = lGetHost(hgroup, HGRP_name);
   lList *cqueue_list = NULL;

   DENTER(TOP_LAYER, "hgroup_success");

   /* we will have the cqueue_list in the final event */
   lXchgList(hgroup, HGRP_cqueue_list, &cqueue_list);
   /*
    * HGRP modify or add event
    */
   sge_add_event(0, old_hgroup?sgeE_HGROUP_MOD:sgeE_HGROUP_ADD, 0, 0, 
                 name, NULL, NULL, hgroup);
   lListElem_clear_changed_info(hgroup);

   lXchgList(hgroup, HGRP_cqueue_list, &cqueue_list);

   /*
    * QI add or delete events. Finalize operation.
    */
   hgroup_commit(ctx, hgroup);
    
   DRETURN(0);
}


int 
hgroup_spool(sge_gdi_ctx_class_t *ctx, lList **answer_list, lListElem *this_elem, gdi_object_t *object) 
{
   bool tmp_ret = true;
   bool dbret;
   const char *name = lGetHost(this_elem, HGRP_name);
   lList *cqueue_list = lGetList(this_elem, HGRP_cqueue_list);
   lListElem *cqueue = NULL;
   dstring key_dstring = DSTRING_INIT;
   lList *spool_answer_list = NULL;
   bool job_spooling = ctx->get_job_spooling(ctx);

   DENTER(TOP_LAYER, "hgroup_spool");

   /* start a transaction for spooling of all affected objects */
   dbret = spool_transaction(&spool_answer_list, spool_get_default_context(),
                             STC_begin);
   answer_list_output(&spool_answer_list);
   if (!dbret) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              "%s", MSG_PERSISTENCE_OPENTRANSACTION_FAILED);
      tmp_ret = false;
   }
  
   if (tmp_ret) {
      for_each (cqueue, cqueue_list) {
         lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
         lListElem *qinstance = NULL;
         const char *cqname = lGetString(cqueue, CQ_name);

         for_each(qinstance, qinstance_list) {
            u_long32 tag = lGetUlong(qinstance, QU_tag);

            if (tag == SGE_QI_TAG_ADD || tag == SGE_QI_TAG_MOD) {
               const char *key = sge_dstring_sprintf(&key_dstring, "%s/%s",
                                                cqname,
                                                lGetHost(qinstance, QU_qhostname));
               dbret = spool_write_object(&spool_answer_list, spool_get_default_context(), 
                                          qinstance, key, SGE_TYPE_QINSTANCE,
                                          job_spooling);
               answer_list_output(&spool_answer_list);

               if (!dbret) {
                  answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                          ANSWER_QUALITY_ERROR, 
                                          MSG_PERSISTENCE_WRITE_FAILED_S,
                                          key);
                  tmp_ret = false;
                  break;
               }
            }
         }
      }
   }

   sge_dstring_free(&key_dstring);

   if (tmp_ret) {
      dbret = spool_write_object(&spool_answer_list, spool_get_default_context(), 
                                 this_elem, name, SGE_TYPE_HGROUP,
                                 job_spooling);
      answer_list_output(&spool_answer_list);
      if (!dbret) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_PERSISTENCE_WRITE_FAILED_S,
                                 name);
         tmp_ret = false;
      }
   }

   /* commit or rollback database transaction */
   dbret = spool_transaction(&spool_answer_list, spool_get_default_context(),
                             tmp_ret ? STC_commit : STC_rollback);
   answer_list_output(&spool_answer_list);
   if (!dbret) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              "%s", MSG_PERSISTENCE_CLOSINGTRANSACTION_FAILED);
      tmp_ret = false;
   }
 
   /* commit or rollback hostgroup action */
   if (!tmp_ret) {
      hgroup_rollback(this_elem);
   }

   DEXIT;
   return tmp_ret ? 0 : 1;
}
