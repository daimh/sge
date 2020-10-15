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


#include "uti/sge_rmon.h" 
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_hostname.h"

#include "comm/commlib.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "basis_types.h"
#include "msg_common.h"

#define HOSTREF_LAYER BASIS_LAYER

/****** sgeobj/href/href_list_add() *******************************************
*  NAME
*     href_list_add() -- Add host or hostgroup reference.
*
*  SYNOPSIS
*     bool 
*     href_list_add(lList **this_list, lList **answer_list, 
*                   const char *host_or_group) 
*
*  FUNCTION
*     Add a host or hostgroup given by 'host_or_group' into the list 
*     'this_list'. If the function is successfull then the function
*     returns 'true' otherwise it will add an entry into 'answer_list'
*     and return with 'false'. If 'this_list' does not exist than it
*     will be created.
*
*  INPUTS
*     lList **this_list         - HR_Type list 
*     lList **answer_list       - AN_Type list 
*     const char *host_or_group - host or group name  
*
*  RESULT
*     bool - error state
*        true - Success
*        false - Error
*******************************************************************************/
bool 
href_list_add(lList **this_list, lList **answer_list, const char *host_or_group)
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_add");
   if (this_list != NULL && host_or_group != NULL) {
      if (!href_list_has_member(*this_list, host_or_group)) {
         lListElem *h_or_g;   /* HR_Type */

         h_or_g = lAddElemHost(this_list, HR_name, host_or_group, HR_Type);
         if (h_or_g == NULL) {
            answer_list_add(answer_list, MSG_SGETEXT_NOMEM,
                            STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            ret = false;
         }
      }
   } else {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_INAVLID_PARAMETER_IN_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT, 
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      ret = false;
   }
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_has_member() ************************************
*  NAME
*     href_list_has_member() -- Is reference already in list 
*
*  SYNOPSIS
*     bool 
*     href_list_has_member(const lList *this_list, 
*                          const char *host_or_group) 
*
*  FUNCTION
*     Is the given host or hostgroup ('host_or_group') already
*     contained in the reference list?
*
*  INPUTS
*     const lList *this_list    - HR_Type list 
*     const char *host_or_group - hostname or hgroup 
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
href_list_has_member(const lList *this_list, const char *host_or_group)
{
   bool ret = false;

   DENTER(HOSTREF_LAYER, "href_list_has_member");
   if (this_list != NULL && host_or_group != NULL) {
      if (href_list_locate(this_list, host_or_group) != NULL) {
         ret = true;
      }
   } else {
      /*
       * If one of the argumets was not given by the callee we may be sure
       * that 'host_or_group' is not member of 'this_list'!
       */
      ;
   }
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_compare() ***************************************
*  NAME
*     href_list_compare() -- Finds additional entries in list 
*
*  SYNOPSIS
*     bool 
*     href_list_compare(const lList *this_list, lList **answer_list, 
*                       const lList *list, lList **add_hosts, 
*                       lList **add_groups, lList **equity_hosts,
*                       lList **equity_groups) 
*
*  FUNCTION
*     This function will find differences between two hostref lists
*     given by 'this_list' and 'list'. Hosts and hostgroups which are
*     only in 'this_list' can be found in 'add_hosts' and 'add_groups'.
*     References which are contained in both lists can be found in 
*     'equity_hosts' and 'equity_groups' after a call to this function.
*
*     If the calling function is not interested in one ore more of the
*     result lists than NULL should be used as parameter. The calling
*     function is responsible to free all result lists. 
*
*     If the callee is also interested in the references which are
*     only part of 'list' than this function can not be used.
*     href_list_find_diff() should be used in this case. 
*
*  INPUTS
*     const lList *this_list - HR_Type list to comapre
*     lList **answer_list    - AN_Type list 
*     const lList *list      - 2nd HR_Type list to be compared
*     lList **add_hosts      - HR_Type list 
*     lList **add_groups     - HR_Type list 
*     lList **equity_hosts   - HR_Type list
*     lList **equity_groups  - HR_Type list
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*
*  SEE ALSO
*     sgeobj/href/href_list_find_diff()
*******************************************************************************/
bool 
href_list_compare(const lList *this_list, lList **answer_list,
                  const lList *list, lList **add_hosts,
                  lList **add_groups, lList **equity_hosts, 
                  lList **equity_groups) 
{
   bool ret = true;
   lListElem *this_elem;   /* HR_Type */
  
   DENTER(HOSTREF_LAYER, "href_list_compare"); 

   for_each(this_elem, this_list) {
      const char *host_or_group = lGetHost(this_elem, HR_name);

      if (!href_list_has_member(list, host_or_group)) {
         if (is_hgroup_name(host_or_group)) {
            if (add_groups != NULL) {
               ret = href_list_add(add_groups, answer_list, host_or_group);
            }
         } else if (add_hosts != NULL) {
            ret = href_list_add(add_hosts, answer_list, host_or_group);
         }
      } else {
         if (is_hgroup_name(host_or_group)) {
            if (equity_groups != NULL) {
               ret = href_list_add(equity_groups, answer_list, host_or_group);
            }
         } else if (equity_hosts != NULL) {
            ret = href_list_add(equity_hosts, answer_list, host_or_group);
         }
      }
      if (!ret) {
         break;
      }
   }
   DRETURN(ret);
}

/****** sgeobj/href/href_list_find_diff() *************************************
*  NAME
*     href_list_find_diff() -- difference between two lists 
*
*  SYNOPSIS
*     bool 
*     href_list_find_diff(const lList *this_list, lList **answer_list, 
*                         const lList *list, lList **add_hosts, 
*                         lList **rem_hosts, lList **add_groups, 
*                         lList **rem_groups) 
*
*  FUNCTION
*     Will identify differences between 'this_list' and 'list'.
*     hosts which are only in 'this_list' will be copied into 'add_hosts'
*     hosts which are only in 'list' will be copied into 'rem_hosts'
*     groups which are only in 'this_list' will be copied to 'add_groups'
*     groups which are only in 'this' will be copied to 'rem_groups'
*
*     The calling context is responsible to free all result lists.
*     If the callee is not interested in one or more of the resultlist
*     than NULL should be used as parameter for this function.
*
*  INPUTS
*     const lList *this_list - HR_Type list to be compared
*     lList **answer_list    - AN_Type list 
*     const lList *list      - 2nd HR_Type list to be compared
*     lList **add_hosts      - HR_Type list 
*     lList **rem_hosts      - HR_Type list 
*     lList **add_groups     - HR_Type list 
*     lList **rem_groups     - HR_Type list 
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*
*  SEE ALSO
*     sgeobj/href/href_list_find_diff()
*******************************************************************************/
bool 
href_list_find_diff(const lList *this_list, lList **answer_list,
                    const lList *list, lList **add_hosts,
                    lList **rem_hosts, lList **add_groups, lList **rem_groups) 
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_find_diff");
   ret &= href_list_compare(this_list, answer_list, list,
                            add_hosts, add_groups, NULL, NULL);
   ret &= href_list_compare(list, answer_list, this_list,
                               rem_hosts, rem_groups, NULL, NULL);
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_find_effective_diff() ***************************
*  NAME
*     href_list_find_effective_diff() -- Resolves groups and creates diff
*
*  SYNOPSIS
*     bool href_list_find_effective_diff(lList **answer_list, 
*                                        const lList *add_groups, 
*                                        const lList *rem_groups, 
*                                        const lList *master_list, 
*                                        lList **add_hosts, lList **rem_hosts) 
*
*  FUNCTION
*     Resolves host names of all groups contained in "add_groups"
*     and "rem_groups". Hostnames not part of both resulting hostgroup
*     sets will be stored in "add_hosts" and "rem_hosts".
*
*  INPUTS
*     lList **answer_list      - AN_Type list 
*     const lList *add_groups  - HR_Type list1 (hgroups)
*     const lList *rem_groups  - HR_Type list2 (hgroups)
*     const lList *master_list - HGRP_Type list of all hgroups
*     lList **add_hosts        - resolved "add_groups" hosts not part
*                                of "rem_groups" 
*     lList **rem_hosts        - resolved "rem_groups" hosts not part
*                                of "add_hosts"
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
******************************************************************************/
bool
href_list_find_effective_diff(lList **answer_list, const lList *add_groups, 
                              const lList *rem_groups, const lList *master_list,
                              lList **add_hosts, lList **rem_hosts)
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_find_effective_diff");
   if (ret && add_groups != NULL) {
      ret &= href_list_find_all_references(add_groups, answer_list,
                                           master_list, add_hosts, NULL);
   }
   if (ret && rem_groups != NULL) {
      ret &= href_list_find_all_references(rem_groups, answer_list,
                                           master_list, rem_hosts, NULL);
   }
   if (ret && add_hosts != NULL && *add_hosts != NULL &&
       rem_hosts != NULL && *rem_hosts != NULL) {
      lList *tmp_rem_hosts = NULL;
      lList *tmp_add_hosts = NULL;

      ret &= href_list_find_diff(*add_hosts, answer_list,
                                 *rem_hosts, &tmp_add_hosts,
                                 &tmp_rem_hosts, NULL, NULL);
      if (ret) {
         lFreeList(add_hosts);
         lFreeList(rem_hosts);
         *add_hosts = tmp_add_hosts;
         *rem_hosts = tmp_rem_hosts;
         tmp_add_hosts = NULL;
         tmp_rem_hosts = NULL;
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_locate() ****************************************
*  NAME
*     href_list_locate() -- Find an entry in the reference list 
*
*  SYNOPSIS
*     lListElem* 
*     href_list_locate(const lList *this_list, const char *name) 
*
*  FUNCTION
*     Find an entry in the reference list. 
*
*  INPUTS
*     const lList *this_list - HR_Type 
*     const char *name       - host or groupname 
*
*  RESULT
*     lListElem* - Pointer to host or hostgroup element or NULL 
*******************************************************************************/
lListElem *
href_list_locate(const lList *this_list, const char *name) 
{
   lListElem *ret = NULL;  /* HR_Type */

   DENTER(HOSTREF_LAYER, "href_list_locate");
   if (this_list != NULL && name != NULL) {
      ret = lGetElemHost(this_list, HR_name, name);
   }
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_find_references() *******************************
*  NAME
*     href_list_find_references() -- Find referenced hosts and groups 
*
*  SYNOPSIS
*     bool 
*     href_list_find_references(const lList *this_list, 
*                               lList **answer_list, 
*                               const lList *master_list, 
*                               lList **referenced_hosts, 
*                               lList **referenced_groups) 
*
*  FUNCTION
*     Finds hosts and hostgroups, which are directy referenced
*     in the hostgroups mentioned in 'this_list'. 'master_list' is
*     the list of all existing hostgroups. Directly referenced hosts
*     and hostgroups will be added to 'used_hosts' and 'used_groups'.
*     In case of any errors 'answer_list' will be filled.
*
*  INPUTS
*     const lList *this_list   - HR_Type 
*     lList **answer_list      - AN_Type 
*     const lList *master_list - HGRP_Type
*     lList **used_hosts       - HR_Type 
*     lList **used_groups      - HR_Type 
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
href_list_find_references(const lList *this_list, lList **answer_list,
                          const lList *master_list, lList **used_hosts,
                          lList **used_groups)
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_find_references");
   if (this_list != NULL && master_list != NULL) { 
      lListElem *href;  /* HR_Type */

      /*
       * Handle each reference which was given by the calling context
       */
      for_each(href, this_list) {
         const char *name = lGetHost(href, HR_name);
         bool is_group = is_hgroup_name(name);
         lListElem *hgroup = NULL;  /* HGRP_name */

         /*
          * Try to locate the concerned hgroup object
          * or add host
          */
         if (is_group) {
            hgroup = hgroup_list_locate(master_list, name);
         } else {
            if (used_hosts != NULL) {
               href_list_add(used_hosts, answer_list, name);
            }
         }

         if (hgroup != NULL) {
            lList *href_list2 = lGetList(hgroup, HGRP_host_list);
            lListElem *href2;    /* HR_Type */

            /* 
             * Add each element contained in the sublist of the hostgroup
             * we found previously to one of the result lists.
             */
            for_each(href2, href_list2) {
               const char *name2 = lGetHost(href2, HR_name);

               if (is_hgroup_name(name2)) {
                  if (used_groups != NULL) {
                     href_list_add(used_groups, answer_list, name2); 
                  }
               } else {
                  if (used_hosts != NULL) {
                     href_list_add(used_hosts, answer_list, name2); 
                  }
               }   
            }
         }
      } 
   } 
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_find_all_references() ***************************
*  NAME
*     href_list_find_all_references() -- Find referenced hosts and hgroups 
*
*  SYNOPSIS
*     bool 
*     href_list_find_all_references(const lList *this_list, 
*                                   lList **answer_list, 
*                                   const lList *master_list, 
*                                   lList **used_hosts, 
*                                   lList **used_groups) 
*
*  FUNCTION
*     Finds all hosts and hostgroups which are directly and inderectly
*     referenced by the hostgroups mentioned in 'this_list'. 
*     'master_list' is the list of all existing hostgroups. Referenced 
*     hosts and hostgroups will be added to 'used_hosts' and 'used_groups'.
*     In case of any errors 'answer_list' will be filled.
*
*  INPUTS
*     const lList *this_list   - RN_Type 
*     lList **answer_list      - AN_Type 
*     const lList *master_list - HGRP_Type 
*     lList **used_hosts       - RN_Type 
*     lList **used_groups      - RN_Type 
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
href_list_find_all_references(const lList *this_list, lList **answer_list,
                              const lList *master_list, lList **used_hosts,
                              lList **used_groups)
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_find_all_references");
   if (this_list != NULL && master_list != NULL) {
      lList *tmp_used_groups = NULL;
      bool free_tmp_list = false;

      if (used_groups == NULL) {
         used_groups = &tmp_used_groups;
         free_tmp_list = true;
      }

      /*
       * Find all direct referenced hgroups and hosts
       */
      ret &= href_list_find_references(this_list, answer_list, master_list,
                                       used_hosts, used_groups);

      /* 
       * If there are subgroups then try to find their direct referenced
       * groups and hosts, subgroups ...
       *
       * Recursive!
       */
      if (ret && used_groups != NULL && *used_groups != NULL) {
         lList *used_sub_groups = NULL;
         lList *used_sub_hosts = NULL;

         ret &= href_list_find_all_references(*used_groups, answer_list,
                                              master_list, &used_sub_hosts,
                                              &used_sub_groups);
         if (ret) {
            if (used_hosts != NULL && used_sub_hosts != NULL) {
               if (*used_hosts != NULL) {
                  lAddList(*used_hosts, &used_sub_hosts);
               } else {
                  *used_hosts = used_sub_hosts;
                  used_sub_hosts = NULL;
               }
            }
            if (used_groups != NULL && used_sub_groups != NULL) {
               if (*used_groups != NULL) {
                  lAddList(*used_groups, &used_sub_groups);
               } else {
                  *used_groups = used_sub_groups;
                  used_sub_groups = NULL;
               }
            }
         } 
      }

      if (free_tmp_list) {
         lFreeList(&tmp_used_groups);
      }
   } 
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_find_referencees() ******************************
*  NAME
*     href_list_find_referencees() --  Find occupying hosts and hgroups
*
*  SYNOPSIS
*     bool 
*     href_list_find_referencees(const lList *this_list, 
*                                lList **answer_list, 
*                                const lList *master_list, 
*                                lList **occupant_groups) 
*
*  FUNCTION
*     Finds hostgroup references which directy occupy at least one of the
*     hostgroups mentioned in 'this_list'. 'master_list' is the list of 
*     all existing hostgroups. Directly occupying hostgroups will be 
*     added to 'occupant_groups'. In case of any errors 'answer_list' 
*     will be filled.
*
*  INPUTS
*     const lList *this_list   - HR_Type 
*     lList **answer_list      - AN_Type 
*     const lList *master_list - HGRP_Type 
*     lList **occupant_groups  - HR_Type 
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
href_list_find_referencees(const lList *this_list, lList **answer_list,
                           const lList *master_list, lList **occupant_groups)
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_find_referencees");
   if (this_list != NULL && occupant_groups != NULL) {
      lListElem *href;  /* HR_Type */

      for_each(href, this_list) {
         const char *name = lGetHost(href, HR_name);

         if (is_hgroup_name(name)) {
            lListElem *hgroup;   /* HGRP_Type */

            for_each(hgroup, master_list) {
               lList *href_list = lGetList(hgroup, HGRP_host_list);
               lListElem *href = href_list_locate(href_list, name);

               if (href != NULL) {
                  const char *name = lGetHost(hgroup, HGRP_name);

                  href_list_add(occupant_groups, answer_list, name);
               }
            }
         } 
      }
   } else {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_INAVLID_PARAMETER_IN_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT,
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      ret = false;
   }
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_find_all_referencees() **************************
*  NAME
*     href_list_find_all_referencees() -- Find occupying hosts and groups 
*
*  SYNOPSIS
*     bool 
*     href_list_find_all_referencees(const lList *this_list, 
*                                    lList **answer_list, 
*                                    const lList *master_list, 
*                                    lList **occupant_groups) 
*
*  FUNCTION
*     Finds recursivly all hostgroups which occupy the hostgroups mentioned in 
*     'this_list'. 'master_list' is the list of all
*     existing hostgroups. Occupying hostgroups will be
*     added to 'occupant_groups'. In case of any errors 'answer_list'
*     will be filled.
*
*  INPUTS
*     const lList *this_list   - RH_Type 
*     lList **answer_list      - AH_Type 
*     const lList *master_list - HGRP_Type 
*     lList **occupant_groups  - RH_Type 
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
href_list_find_all_referencees(const lList *this_list, lList **answer_list,
                               const lList *master_list, 
                               lList **occupant_groups)
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_find_all_referencees");
   if (this_list != NULL && occupant_groups != NULL) {

      /*
       * Find parents of all given hgroups
       */
      ret &= href_list_find_referencees(this_list, answer_list,
                                        master_list, occupant_groups);

      if (*occupant_groups != NULL && ret) {
         lList *occupant_sub_groups = NULL;  /* HR_Type */

         /*
          * Find grandparents, ...
          *
          * Recursive!
          */
         ret &= href_list_find_all_referencees(*occupant_groups, answer_list,
                                               master_list, 
                                               &occupant_sub_groups);

         if (occupant_sub_groups != NULL && ret) {
            lAddList(*occupant_groups, &occupant_sub_groups);
            occupant_sub_groups = NULL;
         } 
      }
   } else {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_INAVLID_PARAMETER_IN_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT,
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      ret = false;
   }
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_resolve_hostnames() *****************************
*  NAME
*     href_list_resolve_hostnames() -- resolve hostnames 
*
*  SYNOPSIS
*     bool 
*     href_list_resolve_hostnames(lList *this_list, 
*                                 lList **answer_list, bool ignore_errors
*
*  FUNCTION
*     Resolve hostnames contained in 'this_list'. Depending on the
*     'ignore_errors' parameter the function will either fail if a
*     host is not resolvable or this will be ignored.
*
*  INPUTS
*     lList *this_list    - HR_Type list 
*     lList **answer_list - AN_Type list 
*     bool ignore_errors  - ignore if a host is not resolveable
*
*  RESULT
*     bool - error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
href_list_resolve_hostnames(lList *this_list, lList **answer_list,
                            bool ignore_errors) 
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_resolve_hostnames");
   if (this_list != NULL) {
      lListElem *href = NULL;

      for_each(href, this_list) {
         const char *name = lGetHost(href, HR_name);

         if (!is_hgroup_name(name)) {
            char resolved_name[CL_MAXHOSTLEN];
            int back = getuniquehostname(name, resolved_name, 0);

            if (back == CL_RETVAL_OK) {
               lSetHost(href, HR_name, resolved_name);
            } else {
               if (!ignore_errors) {
                  INFO((SGE_EVENT, MSG_HGRP_UNKNOWNHOST, name));
                  answer_list_add(answer_list, SGE_EVENT, 
                                  STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                  ret = false;
               }
            }
         }
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_append_to_dstring() *****************************
*  NAME
*     href_list_append_to_dstring() -- Print href-list to dstring 
*
*  SYNOPSIS
*     bool 
*     href_list_append_to_dstring(const lList *this_list, 
*                                 dstring *string) 
*
*  FUNCTION
*     Print href-list to dstring 
*
*  INPUTS
*     const lList *this_list - HR_Type  
*     dstring *string        - dynamic string 
*
*  RESULT
*     bool - Error state
*        true  - Success
*        false - Error
*******************************************************************************/
bool 
href_list_append_to_dstring(const lList *this_list, dstring *string)
{
   const char *const delim = " ";
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_append_to_dstring");
   if (this_list != NULL && string != NULL) {
      lListElem *href;  /* HR_Type */
      bool is_first = true;

      for_each(href, this_list) {
         const char *name = lGetHost(href, HR_name);

         if (!is_first) {
            sge_dstring_append(string, delim);
         }
         sge_dstring_append(string, name);
         is_first = false; 
      }
   } else {
      ret = false;
   } 
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_remove_existing() *******************************
*  NAME
*     href_list_remove_existing() -- Removes entries from list 
*
*  SYNOPSIS
*     bool href_list_remove_existing(lList **this_list, 
*                                    lList **answer_list, 
*                                    lList *list) 
*
*  FUNCTION
*     Removes all entries contained in "list" will be removed from 
*     "this_list" if they exist.
*
*  INPUTS
*     lList **this_list   - HR_Type list 
*     lList **answer_list - AN_Type list 
*     lList *list         - HR_Type list 
*
*  RESULT
*     bool - Error state
*        true  - Success
*        false - Error
******************************************************************************/
bool
href_list_remove_existing(lList **this_list, lList **answer_list _UNUSED,
                          lList *list)
{
   bool ret = true;

   DENTER(HOSTREF_LAYER, "href_list_remove_existing");
   if (this_list != NULL && *this_list != NULL && list != NULL) {
      lListElem * href = NULL;

      for_each(href, list) {
         const char *hostname = lGetHost(href, HR_name);
         lListElem *existing_href = lGetElemHost(*this_list, HR_name, hostname);

         if (existing_href != NULL) {
            DTRACE;
            lRemoveElem(*this_list, &existing_href);
         }
      }
      if (lGetNumberOfElem(*this_list) == 0) {
         lFreeList(this_list);
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/href/href_list_debug_print() ***********************************
*  NAME
*     href_list_debug_print() -- Prints HR_Type list into TOP_LAYER 
*
*  SYNOPSIS
*     void href_list_debug_print(const lList *this_list, const char *prefix) 
*
*  FUNCTION
*     Prints prefix and HR_Type "this_list" into TOP_LAYER of debug 
*     output if this_list exists. 
*
*  INPUTS
*     const lList *this_list - HR_Type list 
*     const char *prefix     - prefix string 
*
*  RESULT
*     void -  None
******************************************************************************/
void
href_list_debug_print(const lList *this_list, const char *prefix) 
{
   lListElem *href = NULL;
   dstring message = DSTRING_INIT;
   bool is_first_hostname = true;
   DENTER(TOP_LAYER, "href_list_debug_print");

   for_each(href, this_list) {
      const char *hostname = lGetHost(href, HR_name);

      if (is_first_hostname) {
         sge_dstring_copy_string(&message, prefix);
         is_first_hostname = false;
      } else {
         sge_dstring_append(&message, ", ");
      }
      sge_dstring_append(&message, hostname);
   }
   if (!is_first_hostname) {
      sge_dstring_append(&message, "\n");
      DPRINTF((SFN, sge_dstring_get_string(&message)));
   }
   sge_dstring_free(&message);
   DEXIT;
}

/****** sgeobj/href/href_list_make_uniq() *************************************
*  NAME
*     href_list_make_uniq() -- remove duplicates 
*
*  SYNOPSIS
*     void href_list_make_uniq(lList *this_list, lList **answer_list) 
*
*  FUNCTION
*     Remove duplicates from "this_list" 
*
*  INPUTS
*     lList *this_list    - HR_Type list 
*     lList **answer_list - AN_Type list 
*
*  RESULT
*     void - none 
*******************************************************************************/
void
href_list_make_uniq(lList *this_list, lList **answer_list _UNUSED)
{
   lListElem *elem = NULL;
   lListElem *next_elem = NULL;

   DENTER(TOP_LAYER, "href_list_make_uniq");
   next_elem = lFirst(this_list);
   while((elem = next_elem) != NULL) {
      lListElem *elem2 = NULL;
      const void *iterator = NULL;

      next_elem = lNext(elem);
      elem2 = lGetElemHostFirst(this_list, HR_name, 
                                lGetHost(elem, HR_name), &iterator); 
      if (elem2 != NULL && elem != elem2) {
         lRemoveElem(this_list, &elem);
      }
   }
   DEXIT;
}

