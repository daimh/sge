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
#include <sys/types.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"

#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "spool/sge_spooling.h"

#include "sched/valid_queue_user.h"

#include "sge.h"
#include "sge_event_master.h"
#include "sge_userset_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "sge_utility_qmaster.h"
#include "sge_resource_quota_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"

static void sge_change_queue_version_acl(sge_gdi_ctx_class_t *ctx, const char *acl_name);
static lList* do_depts_conflict(lListElem *new, lListElem *old);
static int verify_userset_deletion(lList **alpp, const char *userset_name);
static int dept_is_valid_defaultdepartment(lListElem *dept, lList **answer_list);
static int acl_is_valid_acl(lListElem *acl, lList **answer_list);

/******************************************************************
   sge_del_userset() - Qmaster code

   deletes an userset list from the global userset_list
 ******************************************************************/
int sge_del_userset(
sge_gdi_ctx_class_t *ctx,
lListElem *ep,
lList **alpp,
lList **userset_list,
char *ruser,
char *rhost 
) {
   lListElem *found;
   int pos, ret;
   const char *userset_name;

   DENTER(TOP_LAYER, "sge_del_userset");

   if ( !ep || !ruser || !rhost ) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EUNKNOWN);
   }

   /* ep is no userset element, if ep has no US_name */
   if ((pos = lGetPosViaElem(ep, US_name, SGE_NO_ABORT)) < 0) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS,
            lNm2Str(US_name), SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EUNKNOWN);
   }

   userset_name = lGetPosString(ep, pos);
   if (!userset_name) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EUNKNOWN);
   }

   /* search for userset with this name and remove it from the list */
   if (!(found = userset_list_locate(*userset_list, userset_name))) {
      ERROR((SGE_EVENT, MSG_SGETEXT_DOESNOTEXIST_SS, MSG_OBJ_USERSET, userset_name));
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EEXIST);
   }

   /* ensure there are no (x)acl lists in 
      a queue/pe/project/.. refering to this userset */   
   if ((ret=verify_userset_deletion(alpp, userset_name))!=STATUS_OK) {
      /* answerlist gets filled by verify_userset_deletion() in case of errors */
      DRETURN(ret);
   }

   lRemoveElem(*userset_list, &found);

   sge_event_spool(ctx, alpp, 0, sgeE_USERSET_DEL, 
                   0, 0, userset_name, NULL, NULL,
                   NULL, NULL, NULL, true, true);

   INFO((SGE_EVENT, MSG_SGETEXT_REMOVEDFROMLIST_SSSS,
            ruser, rhost, userset_name, MSG_OBJ_USERSET));
   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
   DRETURN(STATUS_OK);
}

/*********************************************************************
   Qmaster code

   increase version of queues using given userset as access list (acl or xacl)

   having changed a complex we have to increase the versions
   of all queues containing this complex;
 **********************************************************************/
static void 
sge_change_queue_version_acl(sge_gdi_ctx_class_t *ctx, const char *acl_name) 
{
   lListElem *cqueue = NULL;

   DENTER(TOP_LAYER, "sge_change_queue_version_acl");

   for_each(cqueue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
      lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
      lListElem *qinstance = NULL;

      for_each(qinstance, qinstance_list) {
         lList *acl_list = lGetList(qinstance, QU_acl);
         lList *xacl_list = lGetList(qinstance, QU_xacl);
         lListElem *acl = lGetElemStr(acl_list, US_name, acl_name);
         lListElem *xacl = lGetElemStr(xacl_list, US_name, acl_name);
         bool is_used = ((acl != NULL) || (xacl != NULL)) ? true : false;

         if (is_used) {
            lList *answer_list = NULL;

            DPRINTF(("increasing version of queue "SFQ" because acl "SFQ
                     " changed\n", lGetString(qinstance, QU_full_name), 
                     acl_name));
            qinstance_increase_qversion(qinstance);
            sge_event_spool(ctx, &answer_list, 0, sgeE_QINSTANCE_MOD, 
                            0, 0, lGetString(qinstance, QU_qname), 
                            lGetHost(qinstance, QU_qhostname), NULL,
                            qinstance, NULL, NULL, true, false);
            answer_list_output(&answer_list);
         }
      }
   }

   DRETURN_VOID;
}

/******************************************************
   sge_verify_department_entries()
      resolves user set/department

   userset_list
      the current master user list (US_Type)
   new_userset
      the new userset element
   alpp
      may be NULL
      is used to build up an answer
      element in case of error

   returns
      STATUS_OK         - on success
      STATUS_ESEMANTIC  - on error
 ******************************************************/
int sge_verify_department_entries(
lList *userset_list,
lListElem *new_userset,
lList **alpp 
) {
   lListElem *up;
   lList *depts;
   lList *answers = NULL;
   lCondition *where = NULL;
   lEnumeration *what = NULL;

   DENTER(TOP_LAYER, "sge_verify_department_entries");

   /*
    * make tests for the defaultdepartment
    */
   if (!strcmp(lGetString(new_userset, US_name), DEFAULT_DEPARTMENT)) {
      if (!dept_is_valid_defaultdepartment(new_userset, alpp)) {
         DRETURN(STATUS_ESEMANTIC);
      }
   }

   if (!(lGetUlong(new_userset, US_type) & US_DEPT)) {
      DRETURN(STATUS_OK);
   }
   
   /*
   ** get the department usersets and only those that have a different
   ** name than the new one.
   */
   what = lWhat("%T(ALL)", US_Type);
   where = lWhere("%T(%I m= %u && %I != %s)", US_Type, US_type, US_DEPT,
                     US_name, lGetString(new_userset, US_name));
   depts = lSelect("Departments", userset_list, where, what);
   lFreeWhere(&where);
   lFreeWhat(&what);

   if (!depts) {
      DRETURN(STATUS_OK);
   }

   /*
   ** Loop over departments and check if a user in the new
   ** element is already contained in another department list.
   ** This requires expanding the group entries.
   */
   for_each(up, depts) {
      answers = do_depts_conflict(new_userset, up); 
      if (answers)
         break;
   }
   
   lFreeList(&depts);
   
   if (answers) {
      *alpp = answers;
      DRETURN(STATUS_ESEMANTIC);
   }

   DRETURN(STATUS_OK);
}

/****** qmaster/dept/dept_is_valid_defaultdepartment() ************************
*  NAME
*     dept_is_valid_defaultdepartment() -- is defaultdepartment correct 
*
*  SYNOPSIS
*     static int dept_is_valid_defaultdepartment(lListElem *dept, 
*                                                lList **answer_list) 
*
*  FUNCTION
*     Check if the given defaultdepartment "dept" is valid. 
*
*  INPUTS
*     lListElem *dept     - US_Type defaultdepartment 
*     lList **answer_list - AN_Type answer list 
*
*  RESULT
*     static int - 0 or 1
*******************************************************************************/
static int dept_is_valid_defaultdepartment(lListElem *dept,
                                           lList **answer_list)
{
   int ret = 1;
   DENTER(TOP_LAYER, "dept_is_valid_defaultdepartment");

   if (dept != NULL) {
      /* test 'type' */
      if (!(lGetUlong(dept, US_type) & US_DEPT)) {
         ERROR((SGE_EVENT, SFNMAX, MSG_QMASTER_DEPTFORDEFDEPARTMENT));
         answer_list_add(answer_list, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         ret = 0;
      }
      /* test user list */
      if (lGetNumberOfElem(lGetList(dept, US_entries)) > 0 ) {
         ERROR((SGE_EVENT, SFNMAX, MSG_QMASTER_AUTODEFDEPARTMENT));
         answer_list_add(answer_list, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         ret = 0;
      }
   }
   DRETURN(ret);
}

/****** qmaster/acl/acl_is_valid_acl() ****************************************
*  NAME
*     acl_is_valid_acl() -- is the acl correct 
*
*  SYNOPSIS
*     static int acl_is_valid_acl(lListElem *acl, lList **answer_list) 
*
*  FUNCTION
*     Check if the given "acl" is correct 
*
*  INPUTS
*     lListElem *acl      - US_Type acl 
*     lList **answer_list - AN_Type list 
*
*  RESULT
*     static int - 0 or 1
*******************************************************************************/
static int acl_is_valid_acl(lListElem *acl,
                            lList **answer_list)
{
   int ret = 1;
   DENTER(TOP_LAYER, "acl_is_valid_acl");

   if (acl != NULL) {
      if (!(lGetUlong(acl, US_type) & US_DEPT)) {
         if (lGetUlong(acl, US_fshare) > 0) {
            ERROR((SGE_EVENT, SFNMAX, MSG_QMASTER_ACLNOSHARE));
            answer_list_add(answer_list, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
            ret = 0;
         }
         if (lGetUlong(acl, US_oticket) > 0) {
            ERROR((SGE_EVENT, SFNMAX, MSG_QMASTER_ACLNOTICKET));
            answer_list_add(answer_list, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
            ret = 0;
         }
      }
   }
   DRETURN(ret);
}

static lList* do_depts_conflict(lListElem *new, lListElem *old)
{
   lList *new_users = NULL;
   lList *old_users = NULL; 
   lListElem *np;
   lList *alp = NULL;
   const char *nname;
   
   DENTER(TOP_LAYER, "do_depts_conflict");
   
   new_users = lGetList(new, US_entries);
   old_users = lGetList(old, US_entries);

   if (!old_users || !new_users) {
      DRETURN(NULL);
   }
   
   /*
   ** groups are encoded with the first letter @, e.g. @sge
   */
   for_each(np, new_users) {
      nname = lGetString(np, UE_name);
      if (nname && nname[0] == '@') { 
         if (sge_contained_in_access_list(NULL, &nname[1], old, &alp)) {
            DRETURN(alp);
         }
      } else {
         if (sge_contained_in_access_list(nname, NULL, old, &alp)) {
            DRETURN(alp);
         }
      }   
   }      

   DRETURN(NULL);
}

/* 

   return
      0   no matching department found
      1   set department
*/
int set_department(lList **alpp, lListElem *job, lList *userset_list)
{
   lListElem *dep;
   const char *owner, *group; 

   DENTER(TOP_LAYER, "set_department");

   /* first try to find a department searching the user name directly
      in a department */
   owner = lGetString(job, JB_owner);
   for_each (dep, userset_list) {
      /* use only departments */
      if (!(lGetUlong(dep, US_type) & US_DEPT)) 
         continue;

      if (sge_contained_in_access_list(owner, NULL, dep, NULL)) {
         lSetString(job, JB_department, lGetString(dep, US_name));   
         DPRINTF(("user %s got department "SFQ"\n", 
            owner, lGetString(dep, US_name)));

         DRETURN(1);
      }
   }

   /* the user does not appear in any department - now try to find
      our group in the department */
   group = lGetString(job, JB_group);
   for_each (dep, userset_list) {
      /* use only departments */ 
      if (!(lGetUlong(dep, US_type) & US_DEPT))
         continue;

      if (sge_contained_in_access_list(NULL, group, dep, NULL)) {
         lSetString(job, JB_department, lGetString(dep, US_name));   
         DPRINTF(("user %s got department \"%s\"\n", owner, lGetString(dep, US_name)));

         DRETURN(1);
      }
   }   
   
   /*
   ** attach default department if present
   ** if job has no department we reach this
   */
   if (lGetElemStr(userset_list, US_name, DEFAULT_DEPARTMENT)) {
      lSetString(job, JB_department, DEFAULT_DEPARTMENT);
      DPRINTF(("user %s got department "SFQ"\n", owner, DEFAULT_DEPARTMENT));
      DRETURN(1);
   }
   
   ERROR((SGE_EVENT, MSG_SGETEXT_NO_DEPARTMENT4USER_SS, owner, group));
   answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);

   DRETURN(0);
}


static int verify_userset_deletion(
lList **alpp,
const char *userset_name 
) {
   int ret = STATUS_OK;
   lListElem *ep = NULL;
   lListElem *cqueue = NULL;
   lList* user_lists = NULL;
   const lListElem *cl;
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "verify_userset_deletion");

   /*
    * fix for bug 6422335
    * check the cq configuration for userset references instead of qinstances
    */
   for_each (cqueue, *object_base[SGE_TYPE_CQUEUE].list) {
      for_each (cl, lGetList(cqueue, CQ_acl)) {
         if (lGetSubStr(cl, US_name, userset_name, AUSRLIST_value))  {
            ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, 
                   userset_name, MSG_OBJ_USERLIST, MSG_OBJ_QUEUE, 
                   lGetString(cqueue, CQ_name)));
            answer_list_add(alpp, SGE_EVENT, 
                            STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = STATUS_EUNKNOWN;
         }
      }
      for_each (cl, lGetList(cqueue, CQ_xacl)) {
         if (lGetSubStr(cl, US_name, userset_name, AUSRLIST_value))  {
            ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, 
                   userset_name, MSG_OBJ_XUSERLIST, MSG_OBJ_QUEUE, 
                   lGetString(cqueue, CQ_name)));
            answer_list_add(alpp, SGE_EVENT, 
                            STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = STATUS_EUNKNOWN;
         }
      }
   }

   for_each (ep, *object_base[SGE_TYPE_PE].list) {
      if (lGetElemStr(lGetList(ep, PE_user_list), US_name, userset_name)) {
         ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, userset_name, 
               MSG_OBJ_USERLIST, MSG_OBJ_PE, lGetString(ep, PE_name)));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      }
      if (lGetElemStr(lGetList(ep, PE_xuser_list), US_name, userset_name)) {
         ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, userset_name, 
               MSG_OBJ_XUSERLIST, MSG_OBJ_PE, lGetString(ep, PE_name)));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      }
   }

   for_each (ep, *object_base[SGE_TYPE_PROJECT].list) {
      if (lGetElemStr(lGetList(ep, PR_acl), US_name, userset_name)) {
         ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, userset_name, 
               MSG_OBJ_USERLIST, MSG_OBJ_PRJ, lGetString(ep, PR_name)));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      }
      if (lGetElemStr(lGetList(ep, PR_xacl), US_name, userset_name)) {
         ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, userset_name, 
               MSG_OBJ_XUSERLIST, MSG_OBJ_PRJ, lGetString(ep, PR_name)));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      }
   }

   /* hosts */
   for_each (ep, *object_base[SGE_TYPE_EXECHOST].list) {
      if (lGetElemStr(lGetList(ep, EH_acl), US_name, userset_name)) {
         ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, userset_name,
               MSG_OBJ_USERLIST, MSG_OBJ_EH, lGetHost(ep, EH_name)));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_EEXIST);
      }
      if (lGetElemStr(lGetList(ep, EH_xacl), US_name, userset_name)) {
         ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, userset_name,
               MSG_OBJ_XUSERLIST, MSG_OBJ_EH, lGetHost(ep, EH_name)));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_EEXIST);
      }
   }

   /* global configuration */
   user_lists = mconf_get_user_lists();
   if (lGetElemStr(user_lists, US_name, userset_name)) {
      ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, userset_name, 
            MSG_OBJ_USERLIST, MSG_OBJ_CONF, MSG_OBJ_GLOBAL));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      ret = STATUS_EUNKNOWN;
   }
   lFreeList(&user_lists);

   user_lists = mconf_get_xuser_lists();
   if (lGetElemStr(user_lists, US_name, userset_name)) {
      ERROR((SGE_EVENT, MSG_SGETEXT_USERSETSTILLREFERENCED_SSSS, userset_name, 
            MSG_OBJ_XUSERLIST, MSG_OBJ_CONF, MSG_OBJ_GLOBAL));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      ret = STATUS_EUNKNOWN;
   }
   lFreeList(&user_lists);

   DRETURN(ret);
}

/****** sge_userset_qmaster/userset_still_used() *******************************
*  NAME
*     userset_still_used() -- True, if userset still used 
*
*  SYNOPSIS
*     static bool userset_still_used(const char *u)
*
*  FUNCTION
*     Returns true, if userset is still used as ACL with host_conf(5),
*     queue_conf(5), or sge_pe(5).
* 
*     Use of usersets as ACLs in sge_conf(5) play no role here, 
*     since such ACLs are checked in qmaster and thus are not 
*     relevant for the scheduling algorithm.
*
*  INPUTS
*     const char *p - the userset
*
*  RESULT
*     static bool - True, if userset still used
*
*  NOTES
*     MT-NOTE: userset_still_used() is not MT safe
*******************************************************************************/
static bool userset_still_used(const char *u)
{
   const lListElem *qc, *cq, *hep, *rqs;
   dstring ds = DSTRING_INIT;

   sge_dstring_sprintf(&ds, "@%s", u);

   for_each (rqs, *object_type_get_master_list(SGE_TYPE_RQS)) {
      if (scope_is_referenced_rqs(rqs, RQR_filter_users, sge_dstring_get_string(&ds))) {
         sge_dstring_free(&ds);
         return true;
      }
   }
   sge_dstring_free(&ds);

   for_each (hep, *object_type_get_master_list(SGE_TYPE_PE)) 
      if (lGetSubStr(hep, US_name, u, PE_user_list) ||
          lGetSubStr(hep, US_name, u, PE_xuser_list))
         return true;

   for_each (hep, *object_type_get_master_list(SGE_TYPE_EXECHOST))
      if (lGetSubStr(hep, US_name, u, EH_acl) ||
          lGetSubStr(hep, US_name, u, EH_xacl))
         return true;

   for_each (cq, *object_type_get_master_list(SGE_TYPE_CQUEUE)) {
      for_each (qc, lGetList(cq, CQ_acl))
         if (lGetSubStr(qc, US_name, u, AUSRLIST_value))
            return true;
      for_each (qc, lGetList(cq, CQ_xacl))
         if (lGetSubStr(qc, US_name, u, AUSRLIST_value))
            return true;
   }

   return false;
}


/****** sge_userset_qmaster/userset_update_categories() ************************
*  NAME
*     userset_update_categories() -- Update all usersets wrts categories
*
*  SYNOPSIS
*     void userset_update_categories(const lList *added, const lList *removed)
*
*  FUNCTION
*     Each added/removed userset is verified whether it is used first
*     time/still as ACL for host_conf(5)/queue_conf(5)/sge_pe(5). If
*     so an event is sent.
*
*  INPUTS
*     const lList *added   - List of added userset references (US_Type)
*     const lList *removed - List of removed userset references (US_Type)
*
*  NOTES
*     MT-NOTE: userset_update_categories() is not MT safe
*******************************************************************************/
void userset_update_categories(const lList *added, const lList *removed)
{
   const lListElem *ep;
   const char *u;
   lListElem *acl;

   DENTER(TOP_LAYER, "userset_update_categories");

   for_each (ep, added) {
      u = lGetString(ep, US_name);
      DPRINTF(("added userset: \"%s\"\n", u));
      acl = lGetElemStr(*object_type_get_master_list(SGE_TYPE_USERSET), US_name, u);
      if (acl && lGetBool(acl, US_consider_with_categories)==false) {
         lSetBool(acl, US_consider_with_categories, true);
         sge_add_event(0, sgeE_USERSET_MOD, 0, 0, u, NULL, NULL, acl);
      }
   }

   for_each (ep, removed) {
      u = lGetString(ep, US_name);
      DPRINTF(("removed userset: \"%s\"\n", u));
      acl = lGetElemStr(*object_type_get_master_list(SGE_TYPE_USERSET), US_name, u);


      if (acl && !userset_still_used(u)) {
         lSetBool(acl, US_consider_with_categories, false);
         sge_add_event(0, sgeE_USERSET_MOD, 0, 0, u, NULL, NULL, acl);
      }
   }

   DRETURN_VOID;
}

/****** sge_userset_qmaster/userset_mod() **************************************
*  NAME
*     userset_mod() -- gdi callback function for adding/modifing a userset
*
*  SYNOPSIS
*     int userset_mod(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem 
*     *new_userset, lListElem *userset, int add, const char *ruser, const char 
*     *rhost, gdi_object_t *object, int sub_command, monitoring_t *monitor) 
*
*  FUNCTION
*     This function is called from the generic gdi framework when a userset is
*     added or modified.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - gdi context
*     lList **alpp             - answer list
*     lListElem *new_userset   - if add it's an empty userset that needs to
*                                be filled
*                                if mod it's the actual stored userset
*     lListElem *userset       - reduced userset object with new/modified values
*     int add                  - 1 for gdi add
*                                0 for gdi mod
*     const char *ruser        - user who invoked the gdi request
*     const char *rhost        - host where the gid request was invoked
*     gdi_object_t *object     - structure of the gdi framework
*     int sub_command          - requested sub_commands
*     monitoring_t *monitor    - monitoring structure
*
*  RESULT
*     int - 0 on success
*           STATUS_EUNKNOWN if an error occured
*  NOTES
*     MT-NOTE: userset_mod() is not MT safe, needs global lock 
*******************************************************************************/
int userset_mod(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *new_userset,
           lListElem *userset, int add, const char *ruser, 
           const char *rhost, gdi_object_t *object, int sub_command,
           monitoring_t *monitor)
{
   const char* userset_name;
   lList *master_userset_list = *object_type_get_master_list(SGE_TYPE_USERSET);
   
   DENTER(TOP_LAYER, "userset_mod");

   /* ---- US_name */
   if (add) {
      if (attr_mod_str(alpp, userset, new_userset, US_name, object->object_name)) {
         goto ERROR;
      }
   }
   userset_name = lGetString(new_userset, US_name);
   if (add && verify_str_key(
         alpp, userset_name, MAX_VERIFY_STRING, object->object_name, KEY_TABLE) != STATUS_OK) {
      goto ERROR;  
   }

   /* ---- US_fshare */
   attr_mod_ulong(userset, new_userset, US_fshare, object->object_name);

   /* ---- US_oticket */
   attr_mod_ulong(userset, new_userset, US_oticket, object->object_name);

   /* ---- US_type */
   attr_mod_ulong(userset, new_userset, US_type, object->object_name);

   /* ensure userset is at least an ACL (qconf -au ) */
   if (lGetUlong(new_userset, US_type) == 0) {
      lSetUlong(new_userset, US_type, US_ACL);
   }
   /* make sure acl is valid */
   if (!add) {
      if (acl_is_valid_acl(new_userset, alpp) != STATUS_OK) {
         goto ERROR;
      }
   }
   /* ---- US_entries */
   attr_mod_sub_list(alpp, new_userset, US_entries,
         UE_name, userset, sub_command, SGE_ATTR_USER_LISTS, object->object_name, 0, NULL);
   /* interpret user/group names */
   if (userset_validate_entries(new_userset, alpp, 0) != STATUS_OK) {
      goto ERROR;
   }

   /*
   ** check for users defined in more than one userset if they
   ** are used as departments
   */
   if (sge_verify_department_entries(master_userset_list, new_userset, alpp) != STATUS_OK) {
      goto ERROR;
   }

   /*
   ** check advance reservations
   */
   if (!add) {
      lListElem *cqueue;
      lList *new_master_userset_list = NULL;

      for_each(cqueue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
         lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
         lListElem *qinstance = NULL;

         for_each(qinstance, qinstance_list) {
            lListElem *ar;
            const char *queue_name = lGetString(qinstance, QU_full_name);

            if (qinstance_slots_reserved(qinstance) == 0) {
               /* queue not reserved, can not conflict with AR */
               continue;
            }

            if (lGetElemStr(lGetList(qinstance, QU_acl), US_name, userset_name) == NULL &&
                lGetElemStr(lGetList(qinstance, QU_xacl), US_name, userset_name) == NULL) {
               /* userset not referenced by queue instance */
               continue;
            }

            for_each(ar, *(object_type_get_master_list(SGE_TYPE_AR))) {
               if (lGetElemStr(lGetList(ar, AR_granted_slots), JG_qname, queue_name)) {
                  if (new_master_userset_list == NULL) {
                     lListElem *old_userset;
                     new_master_userset_list = lCopyList("", *(object_type_get_master_list(SGE_TYPE_USERSET)));
                     old_userset = lGetElemStr(new_master_userset_list, US_name, userset_name);
                     lRemoveElem(new_master_userset_list, &old_userset);
                     lAppendElem(new_master_userset_list, lCopyElem(new_userset));
                  }

                  if (!sge_ar_have_users_access(NULL, ar, lGetString(qinstance, QU_full_name),
                                                lGetList(qinstance, QU_acl),
                                                lGetList(qinstance, QU_xacl),
                                                new_master_userset_list)) {
                     answer_list_add_sprintf(alpp, STATUS_ESYNTAX,
                                             ANSWER_QUALITY_ERROR,
                                             MSG_PARSE_MOD3_REJECTED_DUE_TO_AR_SU,
                                             "entries", 
                                             sge_u32c(lGetUlong(ar, AR_id)));
                     lFreeList(&new_master_userset_list);
                     goto ERROR;
                  }
               }
            }
         }
      }
      lFreeList(&new_master_userset_list);
   }
   
   DRETURN(0);

ERROR:
   DRETURN(STATUS_EUNKNOWN);
}

/****** sge_userset_qmaster/userset_spool() ************************************
*  NAME
*     userset_spool() -- gdi callback funtion to spool a userset
*
*  SYNOPSIS
*     int userset_spool(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem 
*     *userset, gdi_object_t *object) 
*
*  FUNCTION
*     This function is called by the gdi framework after successfully adding or
*     modifing a userset.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - gdi context
*     lList **alpp             - answer list
*     lListElem *userset       - userset object to spool
*     gdi_object_t *object     - structure of the gdi framework
*
*  RESULT
*     [alpp] - error messages will be added to this list
*     0 - success
*     STATUS_EEXIST - an error occured
*
*  NOTES
*     MT-NOTE: userset_spool() is not MT safe 
*******************************************************************************/
int userset_spool(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *userset, gdi_object_t *object)
{
   lList *answer_list = NULL;
   bool dbret;
   bool job_spooling = ctx->get_job_spooling(ctx);

   DENTER(TOP_LAYER, "userset_spool");

   dbret = spool_write_object(&answer_list, spool_get_default_context(), userset, 
                              lGetString(userset, US_name), SGE_TYPE_USERSET,
                              job_spooling);
   answer_list_output(&answer_list);

   if (!dbret) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_PERSISTENCE_WRITE_FAILED_S,
                              lGetString(userset, US_name));
   }

   DRETURN(dbret ? 0 : 1);

}

/****** sge_userset_qmaster/userset_success() **********************************
*  NAME
*     userset_success() -- do something after a successfull add/mod
*
*  SYNOPSIS
*     int userset_success(sge_gdi_ctx_class_t *ctx, lListElem *ep, lListElem 
*     *old_ep, gdi_object_t *object, lList **ppList, monitoring_t *monitor) 
*
*  FUNCTION
*     This function is called from the framework after successfull add/mod and
*     spool.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - gdi context
*     lListElem *ep            - new added userset
*     lListElem *old_ep        - for mod the old userset
*     gdi_object_t *object     - structure of the gdi framework
*     lList **ppList           - additional list that is returned to the client
*     monitoring_t *monitor    - monitoring structure
*
*  RESULT
*     int - 0 on success
*
*  NOTES
*     MT-NOTE: userset_success() is not MT safe 
*******************************************************************************/
int userset_success(sge_gdi_ctx_class_t *ctx, lListElem *ep, lListElem *old_ep, gdi_object_t *object, lList **ppList, monitoring_t *monitor)
{
   const char *userset_name;
   dstring ds = DSTRING_INIT;
   lListElem *rqs;

   DENTER(TOP_LAYER, "userset_success");

   userset_name = lGetString(ep, US_name);

   /* set consider with categories */
   sge_dstring_sprintf(&ds, "@%s", userset_name);
   for_each(rqs, *(object_type_get_master_list(SGE_TYPE_RQS))) {
      if (scope_is_referenced_rqs(rqs, RQR_filter_users, sge_dstring_get_string(&ds))) {
         lSetBool(ep, US_consider_with_categories, true);
         break;
      }
   }

   if (old_ep != NULL) {
      /* change queue versions if userset was modified */
      sge_change_queue_version_acl(ctx, userset_name);
   }

   sge_add_event(0, old_ep?sgeE_USERSET_MOD:sgeE_USERSET_ADD, 0, 0, 
                 userset_name, NULL, NULL, ep);
   lListElem_clear_changed_info(ep);

   sge_dstring_free(&ds);
   DRETURN(0);
}
