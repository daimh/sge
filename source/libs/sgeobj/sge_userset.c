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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fnmatch.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_hostname.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "msg_common.h"

const char* userset_types[] = {
   "ACL",   /* US_ACL   */
   "DEPT",  /* US_DEPT  */
   NULL
};

lList **userset_list_get_master_list(void)
{
   return object_type_get_master_list(SGE_TYPE_USERSET);
}

/****** sgeobj/userset/userset_is_deadline_user() ******************************
*  NAME
*     userset_is_deadline_user() -- may user submit deadline jobs. 
*
*  SYNOPSIS
*     bool userset_is_deadline_user(lList *lp, const char *username) 
*
*  FUNCTION
*     Ask whether a given user is allowed to sumbit deadline jobs. 
*
*  INPUTS
*     lList *lp            - US_Type 
*     const char *username - user name
*
*  RESULT
*     bool - result 
*******************************************************************************/
bool userset_is_deadline_user(lList *lp, const char *username)
{
   lListElem *deadline_users;

   DENTER(TOP_LAYER, "userset_is_deadline_user");

   deadline_users = lGetElemStr(lp, US_name, DEADLINE_USERS);

   if (deadline_users && lGetSubStr(deadline_users, UE_name, username, 
         US_entries)) {
      DRETURN(true); /* found user in deadline user list */
   }

   DRETURN(false);
}

/****** sge_userset/userset_is_ar_user() ***************************************
*  NAME
*     userset_is_ar_user() -- may user request advance reservations
*
*  SYNOPSIS
*     bool userset_is_ar_user(lList *lp, const char *username) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lList *lp            - US_Type
*     const char *username - user name
*
*  RESULT
*     bool - true if user has permission
*            false if user has no permission
*  NOTES
*     MT-NOTE: userset_is_ar_user() is MT safe 
*******************************************************************************/
bool userset_is_ar_user(lList *lp, const char *username)
{
   lListElem *ar_users;

   DENTER(TOP_LAYER, "userset_is_ar_user");

   ar_users = lGetElemStr(lp, US_name, AR_USERS);

   if (ar_users && lGetSubStr(ar_users, UE_name, username, 
         US_entries)) {
      DRETURN(true); /* found user in ar user list */
   }

   DRETURN(false);
}

/****** sgeobj/userset/userset_list_locate() **********************************
*  NAME
*     userset_list_locate() -- Find user in list 
*
*  SYNOPSIS
*     lListElem* userset_list_locate(lList *lp, const char *name) 
*
*  FUNCTION
*     Find user in list. 
*
*  INPUTS
*     lList *lp        - US_Type list 
*     const char *name - name 
*
*  RESULT
*     lListElem* - NULL or element pointer
*******************************************************************************/
lListElem *userset_list_locate(lList *lp, const char *name) 
{
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "userset_list_locate");

   ep = lGetElemStr(lp, US_name, name);

   DRETURN(ep);
}

/****** sgeobj/userset/userset_list_validate_acl_list() ***********************
*  NAME
*     userset_list_validate_acl_list() -- validate an acl list 
*
*  SYNOPSIS
*     int 
*     userset_list_validate_acl_list(lList *acl_list, lList **alpp)
*
*  FUNCTION
*     Checks if all entries of an acl list (e.g. user list of a pe) 
*     are contained in the master userset list.
*
*  INPUTS
*     lList *acl_list       - the acl list to check
*     lList **alpp          - answer list pointer
*
*  RESULT
*     int - STATUS_OK, if everything is OK
*******************************************************************************/
int 
userset_list_validate_acl_list(lList *acl_list, lList **alpp)
{
   lListElem *usp;

   DENTER(TOP_LAYER, "userset_list_validate_acl_list");

   for_each (usp, acl_list) {
      if (!lGetElemStr(*object_type_get_master_list(SGE_TYPE_USERSET), US_name, lGetString(usp, US_name))) {
         ERROR((SGE_EVENT, MSG_CQUEUE_UNKNOWNUSERSET_S, 
                lGetString(usp, US_name) ? lGetString(usp, US_name) : "<NULL>"));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_EUNKNOWN);
      }
   }

   DRETURN(STATUS_OK);
}


/****** sge_userset/userset_list_validate_access() *****************************
*  NAME
*     userset_list_validate_access() -- all user sets names in list must exist  
*
*  SYNOPSIS
*     int userset_list_validate_access(lList *acl_list, int nm, lList **alpp) 
*
*  FUNCTION
*     All the user set names in the acl_list must be defined in the qmaster
*     user set lists. The user set is diferentiated from user names by @ sign
*
*  INPUTS
*     lList *acl_list - the acl list to check
*     int nm          - field name
*     lList **alpp    - answer list pointer
*
*  RESULT
*     int - STATUS_OK if no error,  STATUS_EUNKNOWN otherwise
*
*  NOTES
*     MT-NOTE: userset_list_validate_access() is not MT safe 
*
*******************************************************************************/
int userset_list_validate_access(lList *acl_list, int nm, lList **alpp)
{
   lListElem *usp;
   char *user;

   DENTER(TOP_LAYER, "userset_list_validate_access");

   for_each (usp, acl_list) {
      user = (char *) lGetString(usp, nm);
      if (is_hgroup_name(user) == true){
         user++;  /* jump ower the @ sign */
         if (!lGetElemStr(*object_type_get_master_list(SGE_TYPE_USERSET), US_name, user)) {
            ERROR((SGE_EVENT, MSG_CQUEUE_UNKNOWNUSERSET_S, user ? user : "<NULL>"));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(STATUS_EUNKNOWN);
         }
      }
   }

   DRETURN(STATUS_OK);
}
/****** sgeobj/userset/userset_validate_entries() *******************************
*  NAME
*     userset_validate_entries() -- verify entries of a user set
*
*  SYNOPSIS
*     int userset_validate_entries(lListElem *userset, lList **alpp, 
*                                  int start_up) 
*
*  FUNCTION
*     Validates all entries of a userset.
*
*  INPUTS
*     lListElem *userset - the userset to check
*     lList **alpp       - answer list pointer, if answer is expected. 
*                          In any case, errors are output using the 
*                          ERROR macro.
*     int start_up       - are we in the qmaster startup phase?
*
*  RESULT
*     int - STATUS_OK, if everything is OK
*******************************************************************************/
int userset_validate_entries(lListElem *userset, lList **alpp, int start_up)
{
   lListElem *ep;
   int name_pos;

   DENTER(TOP_LAYER, "userset_validate_entries");

   /*
      resolve cull names to positions
      for faster access in loop
   */

   name_pos = lGetPosInDescr(UE_Type, UE_name);

   for_each(ep, lGetList(userset, US_entries)) {
      if (!lGetPosString(ep, name_pos)) {
         ERROR((SGE_EVENT, SFNMAX, MSG_US_INVALIDUSERNAME));
         answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, 
                         ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_ESEMANTIC);
      }
   }

   DRETURN(STATUS_OK);
}

/****** sgeobj/userset/userset_get_type_string() **********************************
*  NAME
*     userset_get_type_string() -- get readable type definition
*
*  SYNOPSIS
*     const char* 
*     userset_get_type_string(const lListElem *userset, lList **answer_list,
*                           dstring *buffer) 
*
*  FUNCTION
*     Returns a readable string of the userset type bitfield.
*
*  INPUTS
*     const lListElem *userset - the userset containing the requested 
*                                information
*     dstring *buffer          - string buffer to hold the result string
*
*  RESULT
*     const char* - resulting string
*
*  SEE ALSO
*     sgeobj/userset/userset_set_type_string()
*******************************************************************************/
const char *
userset_get_type_string(const lListElem *userset, lList **answer_list, 
                        dstring *buffer)
{
   u_long32 type;
   int i;
   bool append = false;
   const char *ret;

   DENTER(TOP_LAYER, "userset_get_type_string");

   
   SGE_CHECK_POINTER_NULL(userset, answer_list);
   SGE_CHECK_POINTER_NULL(buffer, answer_list);

   type = lGetUlong(userset, US_type);
   sge_dstring_clear(buffer);

   for (i = 0; userset_types[i] != NULL; i++) {
      if ((type & (1 << i)) != 0) {
         if (append) {
            sge_dstring_append(buffer, " ");
         }
         sge_dstring_append(buffer, userset_types[i]);
         append = true;
      }
   }

   ret = sge_dstring_get_string(buffer);
   DRETURN(ret);
}

/****** sgeobj/userset/userset_set_type_string() ******************************
*  NAME
*     userset_set_type_string() -- set userset type from string 
*
*  SYNOPSIS
*     bool 
*     userset_set_type_string(lListElem *userset, lList **answer_list, 
*                           const char *value) 
*
*  FUNCTION
*     Takes a string representation for the userset type, 
*
*  INPUTS
*     lListElem *userset    - the userset to change
*     lList **answer_list - errors will be reported here
*     const char *value   - new value for userset type
*
*  RESULT
*     bool - true on success, 
*            false on error, error message will be in answer_list
*
*  SEE ALSO
*     sgeobj/userset/userset_get_type_string()
******************************************************************************/
bool 
userset_set_type_string(lListElem *userset, lList **answer_list, 
                        const char *value)
{
   bool ret = true;
   u_long32 type = 0;
 
   DENTER(TOP_LAYER, "userset_set_type_string");

   SGE_CHECK_POINTER_FALSE(userset, answer_list);

   if (value != NULL && *value != 0) {
      if (!sge_parse_bitfield_str(value, userset_types, &type, 
                                  "userset type", answer_list, false)) {
         ret = false;
      }
   }
   else { /* value == NULL || *value == 0 */
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_GDI_READCONFIGFILEEMPTYSPEC_S , "userset type"));
      answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      ret = false;
   }
 
   lSetUlong(userset, US_type, type);

   DRETURN(ret);
}

const char *
userset_list_append_to_dstring(const lList *this_list, dstring *string)
{
   const char *ret = NULL;

   DENTER(BASIS_LAYER, "userset_list_append_to_dstring");
   if (string != NULL) {
      lListElem *elem = NULL;
      bool printed = false;

      for_each(elem, this_list) {
         sge_dstring_append(string, lGetString(elem, US_name));
         if (lNext(elem)) {
            sge_dstring_append(string, " ");
         }
         printed = true;
      }
      if (!printed) {
         sge_dstring_append(string, "NONE");
      }
      ret = sge_dstring_get_string(string);
   }
   DRETURN(ret);
}



/* sge_contained_in_access_list() returns 
   1  yes it is contained in the acl
   0  no 

   user, group: may be NULL
*/   
int sge_contained_in_access_list(const char *user, const char *group, 
                                 const lListElem *acl, lList **alpp) 
{
   bool found = false;
   lList *user_list = lGetList(acl, US_entries);

   DENTER(TOP_LAYER, "sge_contained_in_access_list");
   if (group != NULL) {
      dstring group_entry = DSTRING_INIT;

      sge_dstring_sprintf(&group_entry, "@%s", group);
      if (lGetElemStr(user_list, UE_name, sge_dstring_get_string(&group_entry)) != NULL) {
         found = true;
      } else if (sge_is_pattern(group)) {
         lListElem *acl_entry; 
         const char *entry_name;
         for_each(acl_entry, user_list) {
            entry_name = lGetString(acl_entry, UE_name);
            if (entry_name != NULL && fnmatch(sge_dstring_get_string(&group_entry), entry_name, 0) == 0) {
               found = true;
               break;
            }
         }
      }
      sge_dstring_free(&group_entry);
   }
   if (found == false && user != NULL) {
      if (lGetElemStr(user_list, UE_name, user) != NULL) {
         found = true;            
      } else if (sge_is_pattern(user)) {
         lListElem *acl_entry; 
         const char *entry_name;
         for_each(acl_entry, user_list) {
            entry_name = lGetString(acl_entry, UE_name);
            if (entry_name != NULL && fnmatch(user, entry_name, 0) == 0) {
               found = true;
               break;
            }
         }
      }
   }

   if (found) {
      DRETURN(1);
   }

   DRETURN(0);
}
