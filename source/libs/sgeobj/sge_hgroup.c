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
/*
  This module is used for group building
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fnmatch.h>

#include "uti/sge_rmon.h" 
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_hostname.h"

#include "comm/commlib.h"

#include "sgeobj/sge_str.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_eval_expression.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "basis_types.h"
#include "sge.h"
#include "msg_common.h"


#define HGROUP_LAYER TOP_LAYER

/****** sgeobj/hgroup/hgroup_check_name() *************************************
*  NAME
*    hgroup_check_name() -- determine if the name is a valid hgroup name
*
*  SYNOPSIS
*     void check_hgroup_name(lList **answer_list, const char* name) 
*
*  FUNCTION
*     Determine if the given name is a valid hostgroup name. If not
*     add an approbiate error to the answer_list
*
*  INPUTS
*     lList **answer_list - answer list where errors are stored 
*     const char* name    - name of the hostgroup 
*
*  RESULT
*     bool - result 
*        true  -  name contains a valid name for a hostgroup
*        false - name is not a valid name for a hostrgroup
*
*  NOTES
*     MT-NOTE: check_hgroup_name() is not MT safe 
*
*  SEE ALSO
*     sgeobj/hgroup/is_hgroup_name
*******************************************************************************/
bool hgroup_check_name(lList **answer_list, const char* name)
{
   if (!is_hgroup_name(name)) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_HGRP_INVALIDHOSTGROUPNAME_S, name);
      return false;
   }
   if (verify_str_key(
      answer_list,&name[1], MAX_VERIFY_STRING,
      "hostgroup", KEY_TABLE) != STATUS_OK) {
      return false;
   }
   return true;
}

/****** sgeobj/hgroup/hgroup_list_get_master_list() ***************************
*  NAME
*     hgroup_list_get_master_list() -- Returns master list 
*
*  SYNOPSIS
*     lList **hgroup_list_get_master_list(void) 
*
*  FUNCTION
*     Returns master list containing all existing hostgroup objects. 
*
*  INPUTS
*     void - none 
*
*  RESULT
*     lList** - HGRP_Type list 
*******************************************************************************/
lList **
hgroup_list_get_master_list(void) 
{
    /* depending on the setting, we want to return the local thread setting and
       not the global master list. The object_type_get_master_list knows, which
       one to get */
    return object_type_get_master_list(SGE_TYPE_HGROUP);
}

/****** sgeobj/hgroup/hgroup_list_locate() ************************************
*  NAME
*     hgroup_list_locate() -- Find a group by name 
*
*  SYNOPSIS
*     lListElem* hgroup_list_locate(const lList *this_list, 
*                                      const char *group) 
*
*  FUNCTION
*     Find a 'group' in 'this_list'. 
*
*  INPUTS
*     const lList *this_list - HGRP_Type list 
*     const char *group      - group name 
*
*  RESULT
*     lListElem* - found element or NULL 
******************************************************************************/
lListElem *
hgroup_list_locate(const lList *this_list, const char *group) 
{
   lListElem *ret = NULL;
   
   DENTER(HGROUP_LAYER, "hgroup_list_locate");
   ret = lGetElemHost(this_list, HGRP_name, group);
   DEXIT;
   return ret;
}


/****** sgeobj/hgroup/hgroup_create() *****************************************
*  NAME
*     hgroup_create() -- Create a new hgroup. 
*
*  SYNOPSIS
*     lListElem* 
*     hgroup_create(lList **answer_list, const char *name, 
*                   lList *href_or_groupref) 
*
*  FUNCTION
*     Create a new hostgroup.
*
*  INPUTS
*     lList **answer_list     - AN_Type list 
*     const char *name        - name 
*     lList *href_or_groupref - list of hosts for this hgroup 
*     bool is_name_validate   - if true, the hgrp name is validated. Should be done all the time,
*                               there is only one case in qconf in that the name has to be ignored.
*
*  RESULT
*     lListElem* - new element or NULL 
*******************************************************************************/
lListElem *
hgroup_create(lList **answer_list, const char *name, lList *href_or_groupref, bool is_name_validate)
{
   lListElem *ret = NULL;  /* HGRP_Type */

   DENTER(HGROUP_LAYER, "hgroup_create");
   if (name != NULL) {
      if(!is_name_validate || hgroup_check_name(answer_list, name) ) {
         ret = lCreateElem(HGRP_Type);
         if (ret != NULL) {
            lSetHost(ret, HGRP_name, name);
            lSetList(ret, HGRP_host_list, href_or_groupref);
         } else {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, 
                                   MSG_MEM_MEMORYALLOCFAILED_S, SGE_FUNC));
            answer_list_add(answer_list, SGE_EVENT, 
                            STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         }
     }
   } else {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_INAVLID_PARAMETER_IN_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT,
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   }
   DEXIT;
   return ret; 
}

/****** sgeobj/hgroup/hgroup_add_references() *********************************
*  NAME
*     hgroup_add_references() -- Add a host or group reference 
*
*  SYNOPSIS
*     bool hgroup_add_references(lListElem *this_elem, 
*                                lList **answer_list, 
*                                const lList *href_or_groupref) 
*
*  FUNCTION
*     Add a host or group reference. 
*
*  INPUTS
*     lListElem *this_elem          - HGRP_Type elem
*     lList **answer_list           - AN_Type list 
*     const lList *href_or_groupref - HR_Type list
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error 
******************************************************************************/
bool 
hgroup_add_references(lListElem *this_elem, lList **answer_list, 
                      const lList *href_or_groupref) 
{
   bool ret = true;

   DENTER(HGROUP_LAYER, "hgroup_add_references");
   if (this_elem != NULL && href_or_groupref != NULL) {
      lList *href_list = NULL;   /* HR_Type */
      lListElem *href;           /* HR_Type */

      lXchgList(this_elem, HGRP_host_list, &href_list);
      for_each(href, href_or_groupref) {
         const char *name = lGetHost(href, HR_name);
   
         ret &= href_list_add(&href_list, answer_list, name);
         if (!ret) {
            break;
         }
      } 
      lXchgList(this_elem, HGRP_host_list, &href_list);
   } else {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_INAVLID_PARAMETER_IN_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT,
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      ret = false;
   }
   DRETURN(ret);
}

/****** sgeobj/hgroup/hgroup_find_all_references() ****************************
*  NAME
*     hgroup_find_all_references() -- Find referenced host and groups 
*
*  SYNOPSIS
*     bool 
*     hgroup_find_all_references(const lListElem *this_elem, 
*                                lList **answer_list, lList *master_list, 
*                                lList **used_hosts, lList **used_groups) 
*
*  FUNCTION
*     Find directly or indirectly referenced hgroup names. 
*     'master_list' has to be the list of all existing hgroups.
*     'used_hosts' and 'used_groups' will contain the names of
*     hosts and groups referenced by 'this_elem'.
*
*  INPUTS
*     const lListElem *this_elem - HGRP_Type 
*     lList **answer_list        - AN_Type list 
*     const lList *master_list   - HGRP_Type list 
*     lList **used_hosts         - HR_Type list 
*     lList **used_groups        - HR_Type list 
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*
* BUGS
*     Extremely poor performance. Try not to use this function.
******************************************************************************/
bool 
hgroup_find_all_references(const lListElem *this_elem, lList **answer_list,
                           const lList *master_list, lList **used_hosts,
                           lList **used_groups)
{
   bool ret = true;

   DENTER(HGROUP_LAYER, "hgroup_find_all_references");
   if (this_elem != NULL && master_list != NULL) {
      lList *href_list = NULL;   /* HR_Type */
      const char *name;

      name = lGetHost(this_elem, HGRP_name);
      ret &= href_list_add(&href_list, answer_list, name);

      if (ret) {
         DTRACE;
         ret &= href_list_find_all_references(href_list, answer_list, 
                                              master_list, used_hosts, 
                                              used_groups);
      }
      lFreeList(&href_list);
   }
   DRETURN(ret);
}

/****** sgeobj/hgroup/hgroup_find_references() ********************************
*  NAME
*     hgroup_find_references() -- find directly referenced hosts and groups 
*
*  SYNOPSIS
*     bool 
*     hgroup_find_references(const lListElem *this_elem, 
*                            lList **answer_list, 
*                            lList *master_list, 
*                            lList **used_hosts, 
*                            lList **used_groups) 
*
*  FUNCTION
*     Find all hgroups which are directly referenced by 'this_elem'
*     'master_list' has to be the list of all existing hgroups.
*     'used_hosts' and 'used_groups' will contain the names of
*     hosts and groups after a call to this function.
*
*  INPUTS
*     const lListElem *this_elem - HGRP_Type 
*     lList **answer_list        - AN_Type 
*     const lList *master_list   - HGRP_Type 
*     lList **used_hosts         - HR_Type 
*     lList **used_groups        - HR_Type 
*
*  RESULT
*     bool - Error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
hgroup_find_references(const lListElem *this_elem, lList **answer_list,
                       const lList *master_list, lList **used_hosts,
                       lList **used_groups)
{
   bool ret = true;

   DENTER(HGROUP_LAYER, "hgroup_find_all_references");
   if (this_elem != NULL && master_list != NULL) {
      const char *name = lGetHost(this_elem, HGRP_name);
      lList *href_list = NULL;   /* HR_Type */

      ret &= href_list_add(&href_list, answer_list, name);

      if (ret) {
         ret &= href_list_find_references(href_list, answer_list, master_list, 
                                          used_hosts, used_groups);
      }
      lFreeList(&href_list);
   }
   DRETURN(ret);
}

/****** sgeobj/hgroup/hgroup_find_all_referencees() ***************************
*  NAME
*     hgroup_find_all_referencees() -- find groups refering to this group 
*
*  SYNOPSIS
*     bool 
*     hgroup_find_all_referencees(const lListElem *this_elem, 
*                                 lList **answer_list, 
*                                 lList *master_list, 
*                                 lList **occupants_groups) 
*
*  FUNCTION
*     Find all hostgroups from 'master_list' which reference the
*     hostgroup 'this_elem'. The name of these hostgroups will be
*     returned in the hreference list 'occupants_groups'.
*     'answer_list' will contain error messages if the function is
*     not successfull
*      
*
*  INPUTS
*     const lListElem *this_elem - HGRP_Type element 
*     lList **answer_list        - AN_Type list 
*     const lList *master_list   - list of all existing HGRP_Type elements 
*     lList **occupants_groups   - HR_Type list 
*
*  RESULT
*     bool - exit state
*        true  - Success
*        false - Error 
*******************************************************************************/
bool 
hgroup_find_all_referencees(const lListElem *this_elem, 
                            lList **answer_list, const lList *master_list, 
                            lList **occupants_groups)
{
   bool ret = true;

   DENTER(HGROUP_LAYER, "hgroup_find_all_referencees");
   if (this_elem != NULL && occupants_groups != NULL) {
      lList *href_list = NULL;
      const char *name;

      name = lGetHost(this_elem, HGRP_name);
      ret &= href_list_add(&href_list, answer_list, name);

      if (ret) {
         ret &= href_list_find_all_referencees(href_list, answer_list,
                                             master_list, occupants_groups);
      }
      lFreeList(&href_list);
   }
   DEXIT;
   return ret;
}

/****** sgeobj/hgroup/hgroup_find_referencees() *********************************
*  NAME
*     hgroup_find_referencees() -- Find groups refering to this group 
*
*  SYNOPSIS
*     bool 
*     hgroup_find_referencees(const lListElem *this_elem, 
*                             lList **answer_list, 
*                             lList *master_list, 
*                             lList **occupants_groups
*                             lList **occupants_queues)
*
*  FUNCTION
*     Find all hostgroups from 'master_list' which reference the
*     hostgroup 'this_elem'. The name of these hostgroups will be
*     returned in the hreference list 'occupants_groups'.
*     'answer_list' will contain error messages if the function is
*     not successfull 
*
*  INPUTS
*     const lListElem *this_elem        - HGRP_Type 
*     lList **answer_list               - AN_Type 
*     const lList *master_hgroup_list   - HGRP_Type master list 
*     const lList *master_cqueue_list   - CQ_Type
*     lList **occupants_groups          - HR_Type 
*     lList **occupants_queues          - ST_Type
*
*  RESULT
*     bool - Error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
hgroup_find_referencees(const lListElem *this_elem, 
                        lList **answer_list,
                        const lList *master_hgroup_list, 
                        const lList *master_cqueue_list,
                        lList **occupants_groups,
                        lList **occupants_queues)
{
   bool ret = true;

   DENTER(HGROUP_LAYER, "hgroup_find_all_referencees");
   if (this_elem != NULL) {
      if (occupants_groups != NULL) {
         const char *name = lGetHost(this_elem, HGRP_name);
         lList *href_list = NULL;

         ret &= href_list_add(&href_list, answer_list, name);
         if (ret) {
            ret &= href_list_find_referencees(href_list, answer_list,
                                              master_hgroup_list, 
                                              occupants_groups);
         }
         lFreeList(&href_list);
      }
      if (ret && occupants_queues != NULL) {
         ret &= cqueue_list_find_hgroup_references(master_cqueue_list, 
                                                   answer_list,
                                                   this_elem, 
                                                   occupants_queues);
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/hgroup/hgroup_list_exists() *************************************
*  NAME
*     hgroup_list_exists() -- Do hostgroups really exist. 
*
*  SYNOPSIS
*     bool 
*     hgroup_list_exists(const lList *this_list, 
*                        lList **answer_list, 
*                        const lList *href_list) 
*
*  FUNCTION
*     This functions returns true if all hostgroups given by the parameter
*     "href_list" exist in "this_list". If one or more objects are missing
*     a corresponding error message will be returned via "answer_list".
*
*  INPUTS
*     const lList *this_list - HGRP_Type
*     lList **answer_list    - AN_Type 
*     const lList *href_list - HR_Type 
*
*  RESULT
*     bool - true or false
*******************************************************************************/
bool
hgroup_list_exists(const lList *this_list, lList **answer_list,
                   const lList *href_list)
{
   bool ret = true;

   DENTER(HGROUP_LAYER, "hgroup_list_exists");
   if (href_list != NULL && this_list != NULL) {
      lListElem *href;

      for_each(href, href_list) {
         const char *name = lGetHost(href, HR_name);

         if (is_hgroup_name(name)) {
            lListElem *hgroup = hgroup_list_locate(this_list, name);
         
            if (hgroup == NULL) {
               ret = false;
               SGE_ADD_MSG_ID(sprintf(SGE_EVENT, 
                              MSG_SGETEXT_DOESNOTEXIST_SS, "host group", name));
               answer_list_add(answer_list, SGE_EVENT,
                               STATUS_EEXIST, ANSWER_QUALITY_ERROR);
               break;      
            }
         }
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/hgroup/hgroup_list_find_matching_and_resolve() *****************
*  NAME
*     hgroup_list_find_matching_and_resolve() -- Finds hostnames 
*
*  SYNOPSIS
*     bool 
*     hgroup_list_find_matching_and_resolve(const lList *this_list, 
*                                           const char *hgroup_pattern, 
*                                           lList **used_hosts) 
*
*  FUNCTION
*    Selects all hostgroups of "this_list" which match the pattern 
*    "hgroup_pattern". All hostnames which are directly or indirectly
*     referenced will be added to "used_hosts"
*
*  INPUTS
*     const lList *this_list     - HGRP_Type 
*     const char *hgroup_pattern - fnmatch pattern 
*     lList **used_hosts         - HR_Type 
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool
hgroup_list_find_matching_and_resolve(const lList *this_list,
                                      const char *hgroup_pattern,
                                      lList **used_hosts) 
{
   bool ret = true;

   DENTER(HGROUP_LAYER, "hgroup_list_find_matching_and_resolve");
   if (this_list != NULL && hgroup_pattern != NULL) {
      lListElem *hgroup;

      for_each(hgroup, this_list) {
         const char *hgroup_name = lGetHost(hgroup, HGRP_name);
         
         /* use hostgroup expression */
         if (!sge_eval_expression(TYPE_HOST,hgroup_pattern, hgroup_name, NULL)) {
            lList *tmp_used_hosts = NULL;
            lListElem *tmp_href = NULL;

            ret = hgroup_find_all_references(hgroup, NULL, this_list,
                                             &tmp_used_hosts, NULL);
            for_each(tmp_href, tmp_used_hosts) {
               if (used_hosts != NULL) {
                  const char *hostname = lGetHost(tmp_href, HR_name);

                  lAddElemHost(used_hosts, HR_name, hostname, HR_Type);
               }
            }
            lFreeList(&tmp_used_hosts);
         }
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/hgroup/hgroup_list_find_matching() *****************************
*  NAME
*     hgroup_list_find_matching() -- Find hgroups which match pattern 
*
*  SYNOPSIS
*     bool 
*     hgroup_list_find_matching(const lList *this_list, 
*                               const char *hgroup_pattern, 
*                               lList **href_list) 
*
*  FUNCTION
*    Selects all hostgroups of "this_list" which match the pattern 
*    "hgroup_pattern". All matching hostgroup names will be added to
*    "href_list"
*
*  INPUTS
*     const lList *this_list     - HGRP_Type list 
*     const char *hgroup_pattern - hostgroup pattern 
*     lList **used_hosts         - HR_Type list  
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*******************************************************************************/
bool
hgroup_list_find_matching(const lList *this_list,
                          const char *hgroup_pattern, lList **href_list) 
{
   bool ret = true;

   DENTER(HGROUP_LAYER, "hgroup_list_find_matching");
   if (this_list != NULL && hgroup_pattern != NULL) {
      lListElem *hgroup;

      for_each(hgroup, this_list) {
         const char *hgroup_name = lGetHost(hgroup, HGRP_name);

   /* use hostgroup expression */
         if (!sge_eval_expression(TYPE_HOST,hgroup_pattern, hgroup_name, NULL)) {
            if (href_list != NULL) {
               lAddElemHost(href_list, HR_name, hgroup_name, HR_Type);
            }
         }
      }
   }
   DEXIT;
   return ret;
}
