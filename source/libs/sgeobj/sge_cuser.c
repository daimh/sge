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

#include <stdlib.h>

#include "uti/sge_rmon.h"
#include "uti/sge_hostname.h"
#include "cull/cull_list.h"
#include "sgeobj/sge_answer.h"


#define CUSER_LAYER TOP_LAYER

#ifndef __SGE_NO_USERMAPPING__

/* EB: ADOC: add comments */

lListElem *
cuser_create(lList **answer_list, const char *cluster_user, lList *remote_user)
{
   lListElem *ret = NULL;

   DENTER(CUSER_LAYER, "cuser_create");
   if (cluster_user != NULL) {
      ret = lCreateElem(CU_Type);

      if (ret != NULL) {
         lSetString(ret, CU_name, cluster_user);
         lSetList(ret, CU_ruser_list, remote_user);
      } else {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, 
                                MSG_MEM_MEMORYALLOCFAILED_S, SGE_FUNC));
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      }
   }
   DEXIT;
   return ret;
}

/*
const char **remote_user   pointer to NULL pointer
                           don't free this pointer
*/
bool
cuser_get_remote_user(const lListElem *this_elem, lList **answer_list,
                      const char *hostname, const char **remote_user) 
{
   bool ret = true;
   
   DENTER(CUSER_LAYER, "cuser_get_remote_user");

   if (this_elem != NULL && hostname != NULL && remote_user != NULL) {
      lList *attr_list = NULL; 
   
      attr_list = lGetList(this_elem, CU_ruser_list);
      if (attr_list != NULL) {
         bool is_ambiguous = false;
         const char *matching_host_or_group = NULL;
         const char *matching_group = NULL;

         ret &= str_attr_list_find_value(attr_list, answer_list,
                                         hostname, remote_user, 
                                         &matching_host_or_group,
                                         &matching_group,
                                         &is_ambiguous); 
      } else {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, 
                                MSG_CUSER_NOREMOTE_USER_S, "remote_user"));
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         ret = false;
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

bool
cuser_is_hgroup_referenced(const lListElem *this_elem,
                              const lListElem *hgroup)
{
   bool ret = false;
   
   DENTER(CUSER_LAYER, "cuser_is_hgroup_referenced");
   if (this_elem != NULL && hgroup != NULL) {
      const char *name = lGetHost(hgroup, HGRP_name);
      lList *attr_list = lGetList(this_elem, CU_ruser_list);
      lListElem *attr = str_attr_list_locate(attr_list, name);

      if (attr != NULL) {
         ret = true;
      }
   }
   DEXIT;
   return ret;
}

bool
cuser_list_find_hgroup_references(const lList *this_list,
                                     lList **answer_list,
                                     const lListElem *hgroup, 
                                     lList **string_list) 
{
   bool ret = true;
   lListElem *cuser;

   DENTER(CUSER_LAYER, "cuser_find_hgroup_references");
   if (this_list != NULL && hgroup != NULL && string_list != NULL) {
      for_each(cuser, this_list) {
         if (cuser_is_hgroup_referenced(cuser, hgroup)) {
            const char *name = lGetString(cuser, CU_name);

            lAddElemStr(string_list, ST_name, name, ST_Type);
         } 
      }
   }
   DEXIT;
   return ret;
}

lList **
cuser_list_get_master_list(void) 
{
    /* depending on the setting, we want to return the local thread setting and
       not the global master list. The object_type_get_master_list_mt knows, which
       one to get */
    return object_type_get_master_list(SGE_TYPE_CUSER);
}

lListElem *
cuser_list_locate(const lList *this_list, const char *cluster_user)
{
   lListElem *ret = NULL;

   DENTER(CUSER_LAYER, "cuser_list_locate");
   if (this_list != NULL && cluster_user != NULL) {
      ret = lGetElemStr(this_list, CU_name, cluster_user);
   }
   DEXIT;
   return ret;
}

bool
cuser_list_map_user(const lList *this_list, lList **answer_list,
                    const char *cluster_user, const char *hostname, 
                    const char **remote_user)
{
   bool ret = true;

   DENTER(CUSER_LAYER, "cuser_list_map_user");
   if (cluster_user != NULL && hostname != NULL && remote_user != NULL) {
      lListElem *cuser = cuser_list_locate(this_list, cluster_user); 

      if (cuser != NULL) {
         ret &= cuser_get_remote_user(cuser, answer_list, 
                                      hostname, remote_user); 
      } else {
         /* No user mapping entry defined for this object */
         *remote_user = cluster_user;
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

#endif
