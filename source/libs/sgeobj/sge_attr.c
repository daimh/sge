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

#include <math.h>
#include <string.h>
#include <sys/types.h>

#include "uti/sge_rmon.h" 
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_hostname.h"
#include "uti/sge_stdlib.h"

#include "comm/commlib.h"

#include "sgeobj/sge_str.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "basis_types.h"
#include "msg_common.h"

#define HOSTATTR_LAYER BASIS_LAYER

#define TEMPLATE_ATTR_IMPL(PREFIX, TYPE, INTERNAL_TYPE,                       \
                           DESCRIPTOR, HREF_NM, VALUE_NM)                     \
                                                                              \
lListElem *                                                                   \
PREFIX##_create(lList **answer_list, const char *href, TYPE value)            \
{                                                                             \
   return attr_create(answer_list, href, &value,                              \
                      DESCRIPTOR, HREF_NM, VALUE_NM);                         \
}                                                                             \
                                                                              \
lListElem *                                                                   \
PREFIX##_list_find(const lList *this_list, const char *href)                  \
{                                                                             \
   return lGetElemHost(this_list, HREF_NM, href);                             \
}                                                                             \
                                                                              \
bool                                                                          \
PREFIX##_list_add(lList **this_list, lList **answer_list, lListElem **attr,   \
                  int flags, lList **ambiguous_href_list)                     \
{                                                                             \
   return attr_list_add(this_list, answer_list, attr, flags,                  \
                        ambiguous_href_list,                                  \
                        DESCRIPTOR, HREF_NM, VALUE_NM);                       \
}                                                                             \
                                                                              \
bool                                                                          \
PREFIX##_list_add_set_del(lList **this_list, lList **answer_list,             \
                         const char *hostname, void *value, bool remove)      \
{                                                                             \
   return attr_list_add_set_del(this_list, answer_list, hostname,             \
                        value, remove, DESCRIPTOR, HREF_NM, VALUE_NM);        \
}                                                                             \
                                                                              \
bool                                                                          \
PREFIX##_list_find_value(const lList *this_list, lList **answer_list,         \
                         const char *hostname, INTERNAL_TYPE *value,          \
                         const char **mastching_host_or_group,                \
                         const char **matching_group,                         \
                         bool *is_ambiguous)                                  \
{                                                                             \
   return attr_list_find_value(this_list, answer_list, hostname,              \
                               value, mastching_host_or_group,                \
                               matching_group, is_ambiguous,                  \
                               HREF_NM, VALUE_NM);                            \
}                                                                             \
                                                                              \
bool                                                                          \
PREFIX##_list_find_value_href(const lList *this_list, lList **answer_list,    \
                         const char *hostname, INTERNAL_TYPE *value,          \
                         bool *found)                                         \
{                                                                             \
   return attr_list_find_value_href(this_list, answer_list, hostname,         \
                               value, found, HREF_NM,                         \
                               VALUE_NM);                                     \
}                                                                             \
                                                                              \
bool                                                                          \
PREFIX##_list_append_to_dstring(const lList *this_list, dstring *string)      \
{                                                                             \
   return attr_list_append_to_dstring(this_list, string,                      \
                                      HREF_NM, VALUE_NM);                     \
}                                                                             \
                                                                              \
bool                                                                          \
PREFIX##_list_parse_from_string(lList **this_list, lList **answer_list,       \
                                const char *string, int flags)                \
{                                                                             \
   return attr_list_parse_from_string(this_list, answer_list, string,         \
                                      flags, DESCRIPTOR, HREF_NM, VALUE_NM);  \
}                                                                             \
                                                                              \
lListElem *                                                                   \
PREFIX##_list_locate(const lList *this_list, const char *host_or_group)       \
{                                                                             \
   return attr_list_locate(this_list, host_or_group, HREF_NM);                \
}                                                                             
 
static lListElem *
attr_create(lList **answer_list, const char *href, void *value,
            const lDescr *descriptor, int href_nm, int value_nm);

static bool 
attr_list_add(lList **this_list, lList **answer_list, lListElem **attr, 
              int flags, lList **ambiguous_href_list,
              const lDescr *descriptor, int href_nm, int value_nm);

static bool 
attr_list_add_set_del(lList **this_list, lList **answer_list, 
              const char *hostname, void *value_buffer, bool remove,
              const lDescr *descriptor, int href_nm, int value_nm);

static bool
attr_list_find_value(const lList *this_list, lList **answer_list, 
                     const char *hostname, void *value_buffer, 
                     const char **matching_host_or_group,
                     const char **matching_group,
                     bool *is_ambiguous, int href_nm, int value_nm);

static bool
attr_list_find_value_href(const lList *this_list, lList **answer_list, 
                     const char *hostname, void *value_buffer, 
                     bool *found, int href_nm, int value_nm);

static bool
attr_list_parse_from_string(lList **this_list, lList **answer_list,
                            const char *string, int flags,
                            const lDescr *descriptor, int href_nm, 
                            int value_nm);

static lListElem *
attr_list_locate(const lList *this_list, const char *host_or_group, 
                 int href_nm);

/****** sgeobj/attr/attr_create() *********************************************
*  NAME
*     attr_create() -- Returns a new attribute element 
*
*  SYNOPSIS
*     static lListElem *
*     attr_create(lList **answer_list, const char *href, void *value, 
*                 const lDescr *descriptor, int href_nm, int value_nm) 
*
*  FUNCTION
*     If an error occures "answer_list" will be filled with an error
*     message. "href" is the hostname or hgroupname of the new element.
*     "value" is a pointer to the new value for that attribute. "descriptor"
*     is the CULL descriptor wich will be used to create the new element.
*     "href_nm" is the CULL name of the field where the "href" name will
*     be stored and "value_nm" defines the value of the field which 
*     will be filled with the "value".
*
*  INPUTS
*     lList **answer_list      - AN_Type list 
*     const char *href         - host oder hgroupname 
*     void *value              - pointer to the attributes value 
*     const lDescr *descriptor - CULL descriptor 
*     int href_nm              - CULL field name host or hgroupname
*     int value_nm             - CULL field name for the value 
*
*  RESULT
*     lListElem * - new CULL element or NULL in case on an error
*
*  NOTES
*     There are typesafe versions of this function. Have a look into 
*     the headerfile and look for TEMPLATE_ATTR_PROTO. These macro
*     creates the typesafe versions. E.g.
*
*        str_attr_create()
*        ulng_attr_create()
*        bool_attr_create()
*        ...
*        strlist_attr_create()
*
*     MT-NOTE: attr_create() is MT safe 
******************************************************************************/
static lListElem *
attr_create(lList **answer_list, const char *href, void *value,
            const lDescr *descriptor, int href_nm, int value_nm)
{
   lListElem *ret = NULL;

   DENTER(HOSTATTR_LAYER, "attr_create");
   if (href != NULL) {
      lListElem *new_attr = lCreateElem(descriptor);

      if (new_attr != NULL) {
         lSetHost(new_attr, href_nm, href);
         if (value != NULL) {
            object_set_any_type(new_attr, value_nm, value);
         }
         ret = new_attr;
      } else {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_ATTR_NONEWATTRSETTING_S, href));
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
   } else {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_INAVLID_PARAMETER_IN_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT, 
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   }
   DEXIT;
   return ret;
}

/****** sgeobj/attr/attr_list_add() *******************************************
*  NAME
*     attr_list_add() -- Add a new attribute entry to a list 
*
*  SYNOPSIS
*     static bool 
*     attr_list_add(lList **this_list, lList **answer_list, lListElem **attr, 
*                   int flags, lList **ambiguous_href_list, 
*                   const lDescr *descriptor, int href_nm, int value_nm) 
*
*  FUNCTION
*     "this_list" and "attr" must have the same "descriptor". "href_nm"
*     defines one cull field within "attr" which containes a hostname 
*     or hgroup-name. "value_nm" containes the value of that concerned 
*     attribute.
*     "answer_list" will be filled in case of an error.
*     "flags" can be used to influence the behaviour of this function
*     in case of duplicates or other ambiguities within the resulting
*     list.
*     "ambiguous_href_list" might be used as output parameter for this
*     function. Find more detailes in the description of the flags
*     parameter below.
*
*  INPUTS
*     lList **this_list           - attribute list 
*     lList **answer_list         - AN_Type list 
*     lListElem **attr            - attribute pointer 
*     int flags                   - behaviour bitmask
*
*        HOSTATTR_OVERWRITE - If there is already an element in "this_list"
*           which has the same hostname or hgroup, then the value of this
*           element will be overwritten if this flag is set. 
*           If this flag is not given and the function should add a
*           duplicate, then this will be counted as function error.
*
*        HOSTATTR_ALLOW_AMBIGUITY - If the resulting "this_list" would 
*           result in an ambigous configuration for a ceratin host then 
*           this is allowed if the flag is given. Otherwise it will
*           be rejected. In that case "ambiguous_href_list" will be
*           filled with the conflicting hostnames.
*
*     lList **ambiguous_href_list - HR_Type list 
*     const lDescr *descriptor    - CULL descriptor 
*     int href_nm                 - CULL field name 
*     int value_nm                - CULL value name 
*
*  RESULT
*     static bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: attr_list_add() is not MT safe 
*******************************************************************************/
static bool 
attr_list_add(lList **this_list, lList **answer_list, lListElem **attr, 
              int flags, lList **ambiguous_href_list,
              const lDescr *descriptor, int href_nm, int value_nm)
{
   bool ret = false;

   DENTER(HOSTATTR_LAYER, "attr_list_add");

   if (this_list != NULL && attr != NULL && *attr != NULL) {
      lListElem *attr_elem = NULL; 
      const char *href = NULL;
      bool is_hgroup = false; 
      bool created_list = false;

      href = lGetHost(*attr, href_nm);
      is_hgroup = is_hgroup_name(href);

      if (*this_list == NULL) {
         *this_list = lCreateList("", descriptor);
         created_list = true;
      } else {
         attr_elem = attr_list_locate(*this_list, href, href_nm);
      }

      /*
       * HOSTREF_DEFAULT and host reference values can be added/changed
       * Hostgroup entries already contained in the list might be changed.
`      *
       * New hostgroup references might cause a conflict. We have to
       * make additional checks.
       */
      if (!strcmp(href, HOSTREF_DEFAULT) || 
          !is_hgroup ||
          (is_hgroup && attr_elem != NULL)) {
         const char *value = NULL; 

         object_get_any_type(*attr, value_nm, &value);
         if (attr_elem != NULL) {
            if (flags & HOSTATTR_OVERWRITE) {
               object_set_any_type(attr_elem, value_nm, &value);
               lFreeElem(attr);
               *attr = attr_elem;
               ret = true;
            } else {
               SGE_ADD_MSG_ID(sprintf(SGE_EVENT, 
                              MSG_ATTR_VALUEMULDEFINED_S, href));
               answer_list_add(answer_list, SGE_EVENT,
                               STATUS_ERROR1, ANSWER_QUALITY_ERROR);
            }
         } else {
            lAppendElem(*this_list, *attr);
            ret = true;
         }
      } else {
         lList *href_list = NULL;
         lList *host_list = NULL;
         lList *new_host_list = NULL;
         bool lret = true;
       
         /*
          * Create list of ambiguous hosts only 
          *  - if ambiguious entries are not allowed or
          *  - if the the caller of this functions expects this
          *    function to return it
          */ 
         if (!(flags & HOSTATTR_ALLOW_AMBIGUITY) && 
             ambiguous_href_list != NULL) {

            /*
             * Create host reference list of all used hostgroups
             * (except HOSTREF_DEFAULT, and host entries)
             */
            if (lret) {
               for_each(attr_elem, *this_list) {
                  const char *href = lGetHost(attr_elem, ASTR_href); 

                  if (strcmp(href, HOSTREF_DEFAULT) && 
                      is_hgroup_name(href)) {
                     lret &= href_list_add(&href_list, NULL, href);
                  }
               }
            }

            /*
             * Find all directly or indirectly referenced hosts for all 
             * hostgroups
             */
            if (lret && href_list != NULL) {
               lList *master_list = *(hgroup_list_get_master_list());
               lList *tmp_href_list = NULL; 

               lret &= href_list_find_all_references(href_list, NULL, 
                                                     master_list, &host_list, 
                                                     NULL); 
               lret &= href_list_add(&tmp_href_list, NULL, href);
               lret &= href_list_find_all_references(tmp_href_list, NULL,
                                                     master_list, 
                                                     &new_host_list, NULL);
               lFreeList(&tmp_href_list);
            }

            /*
             * Find all host references which already have a value attached.
             * For all these hosts the new value in ambiguious.
             */
            if (lret && ambiguous_href_list != NULL && host_list != NULL &&
                new_host_list != NULL) { 
               lret = href_list_compare(new_host_list, NULL, host_list,
                                           NULL, NULL, ambiguous_href_list,
                                           NULL);
            }
         } 

         if (ambiguous_href_list != NULL &&
               lGetNumberOfElem(*ambiguous_href_list) >= 1 &&
               !(flags & HOSTATTR_ALLOW_AMBIGUITY)) {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_ATTR_RESULTAMBIGUOUS));
            answer_list_add(answer_list, SGE_EVENT,
                            STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         } else {
            lAppendElem(*this_list, *attr);
            ret = true;
         }
      }

      if (created_list == true && ret == false) {
         lFreeList(this_list);
      }
   } 
   DRETURN(ret);
}

/****** sgeobj/attr/attr_list_add_set_del() ***********************************
*  NAME
*     attr_list_add_set_del() -- add/replace/delete an attribute entry 
*
*  SYNOPSIS
*     static bool 
*     attr_list_add_set_del(lList **this_list, lList **answer_list, 
*                           const char *hostname, void *value, 
*                           bool remove, const lDescr *descriptor, 
*                           int href_nm, int value_nm) 
*
*  FUNCTION
*     This function can be used to remove an entry from "this_list"
*     or it can add a new entry or replace an existing one. Find a
*     more detailed description in sgeobj/attr/attr_create()
*
*  INPUTS
*     lList **this_list        - cull list of type "descriptor" 
*     lList **answer_list      - AN_Type list 
*     const char *hostname     - hostname or hgroup name 
*     void *value              - pointer to value 
*     bool remove              - true -> remove the element 
*     const lDescr *descriptor - CULL descriptor 
*     int href_nm              - CULL field name 
*     int value_nm             - CULL value name 
*
*  RESULT
*     static bool - error status
*        true  - success
*        false - error 
*
*  NOTES
*     MT-NOTE: attr_list_add_set_del() is MT safe 
*
*  SEE ALSO
*     sgeobj/attr/attr_create()
*******************************************************************************/
static bool 
attr_list_add_set_del(lList **this_list, lList **answer_list, 
              const char *hostname, void *value, bool remove,
              const lDescr *descriptor, 
              int href_nm, int value_nm)
{
   bool ret = true;
   lListElem *attr = NULL;

   if (this_list && *this_list) {
      if (remove) {
         attr = attr_list_locate(*this_list, hostname, href_nm);
         lRemoveElem(*this_list, &attr);
      } else {
         attr = attr_create(answer_list, hostname, value, descriptor, 
                            href_nm, value_nm);
         ret = attr_list_add(this_list, answer_list,
                             &attr, HOSTATTR_OVERWRITE, NULL,
                             descriptor, href_nm, value_nm);
      }
   }
   return ret;
}

/*
descriptor        ASTR_Type
href_nm           ASTR_href
value_nm          ASTR_value
*/
static bool
attr_list_find_value(const lList *this_list, lList **answer_list, 
                     const char *hostname, void *value_buffer,
                     const char **matching_host_or_group,
                     const char **matching_group,
                     bool *is_ambiguous, int href_nm, int value_nm)
{
   bool ret = false;

   DENTER(HOSTATTR_LAYER, "attr_list_find_value");

   if (this_list != NULL && hostname != NULL) {
      lListElem *href = NULL;
   
      /*
       * Try to find a value for the concerned host
       */ 
      href = attr_list_locate(this_list, hostname, href_nm);
      if (href != NULL) {  
         object_get_any_type(href, value_nm, value_buffer);
         DPRINTF(("Found value for host "SFQ"\n", hostname));
         ret = true;
      } else {
         bool already_found = false;

         *is_ambiguous = false;
         /*
          * Try to find a value for all hgroup definitions
          * if there was no host related value
          *
          * Exit the loop as soon as possible! This will be the case if
          * an ambiguous value for the concerned host will be found.
          */
         for_each(href, this_list) {
            const char *href_name = lGetHost(href, href_nm);
            lList *master_list = *(hgroup_list_get_master_list());
            bool lret = true;

            if (strcmp(href_name, HOSTREF_DEFAULT) && 
                is_hgroup_name(href_name)) {
               lList *tmp_href_list = NULL;
               lListElem *tmp_href = NULL;
               lList *host_list = NULL;

               href_list_add(&tmp_href_list, NULL, href_name);
               lret &= href_list_find_all_references(tmp_href_list, NULL,
                                                     master_list, &host_list,
                                                     NULL); 
               tmp_href = href_list_locate(host_list, hostname);
               if (tmp_href != NULL) {
                  if (already_found == false) {
                     already_found = true;
                     object_get_any_type(href, value_nm, value_buffer);
                     *matching_host_or_group = href_name;
                     DPRINTF(("Found value for domain "SFQ"\n", href_name));
                     ret = true;
                  } else {
                     *is_ambiguous = true;
                     *matching_group = href_name;
                     DPRINTF(("Found ambiguous value in domain "SFQ"\n", 
                               href_name));
                     ret = false;
                     lFreeList(&host_list);
                     lFreeList(&tmp_href_list);
                     break; /* leave for_each loop */
                  }
               }
               lFreeList(&host_list);
               lFreeList(&tmp_href_list);
            }
         }
         if (ret == false) {
            lListElem *tmp_href = NULL;

            /*
             * Use the default value
             */
            tmp_href = attr_list_locate(this_list, HOSTREF_DEFAULT, href_nm);
            if (tmp_href != NULL) {
               DPRINTF(("Using default value\n"));
               object_get_any_type(tmp_href, value_nm, value_buffer);
               ret = true;
            } else {
               /*
                * Should never happen.
                */
               SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_ATTR_NOCONFVALUE));
               answer_list_add(answer_list, SGE_EVENT,
                               STATUS_ERROR1, ANSWER_QUALITY_ERROR);            
            }
         }
      }
      if (ret) {
         DTRACE;
      }
   }
   DEXIT;
   return ret;
}

/*
descriptor        ASTR_Type
href_nm           ASTR_href
value_nm          ASTR_value
*/
static bool
attr_list_find_value_href(const lList *this_list, lList **answer_list, 
                     const char *hostname, void *value_buffer,
                     bool *found, int href_nm, int value_nm)
{
   bool ret = false;

   DENTER(HOSTATTR_LAYER, "attr_list_find_value_href");

   if (this_list != NULL && hostname != NULL) {
      lListElem *href = NULL;
   
      /*
       * Try to find a value for the concerned host
       */ 
      href = attr_list_locate(this_list, hostname, href_nm);
      if (href != NULL) {  
         object_get_any_type(href, value_nm, value_buffer);
         *found = true;
         DTRACE;
         ret = true;
      } else {
         lListElem *tmp_href = NULL;

         /*
          * Use the default value
          */
         tmp_href = attr_list_locate(this_list, HOSTREF_DEFAULT, href_nm);
         if (tmp_href != NULL) {
            object_get_any_type(tmp_href, value_nm, value_buffer);
            *found = false;
            DTRACE;
            ret = true;
         } else {
            /*
             * Should never happen.
             */
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_ATTR_NOCONFVALUE));
            answer_list_add(answer_list, SGE_EVENT,
                            STATUS_ERROR1, ANSWER_QUALITY_ERROR);            
         }
      }
   }
   DRETURN(ret);
}

/*
descriptor        ASTR_Type
href_nm           ASTR_href
value_nm          ASTR_value
*/
bool
attr_list_append_to_dstring(const lList *this_list, dstring *string,
                            int href_nm, int value_nm)
{
   bool found_default = false;
   bool found_group = false;
   bool found_host = false;
   lListElem *attr = NULL;
   dstring host_string = DSTRING_INIT;

   DENTER(HOSTATTR_LAYER, "attr_list_append_to_dstring");

   if ((attr = attr_list_locate(this_list, HOSTREF_DEFAULT, href_nm)) != NULL) {
      found_default = true;
      object_append_field_to_dstring(attr, NULL, string, value_nm, '\0');
   }
   
   for_each(attr, this_list) {
      const char *href;

      href = lGetHost(attr, href_nm);

      if (href == NULL || (found_default && !strcmp(href, HOSTREF_DEFAULT))) {
         continue;
      } else {
         dstring *ds; /* will be reference to the corresponding dstring container */
        
         if (is_hgroup_name(href)) {
            ds = string;
            if (found_group || found_default) {
               sge_dstring_append_char(ds, ',');
            }
            found_group = true;
         } else {
            ds = &host_string;
            if (found_host) {
               sge_dstring_append_char(ds, ',');
            }
            found_host = true;
         }

         sge_dstring_append_char(ds, '[');
         sge_dstring_append(ds, href);
         sge_dstring_append_char(ds, '=');
         object_append_field_to_dstring(attr, NULL, ds, value_nm, 
                                        '\0');
         sge_dstring_append_char(ds, ']');
      }
   }
   if (found_host) {
      if (found_default || found_group) {
         sge_dstring_append_char(string, ',');
      }
      sge_dstring_append_dstring(string, &host_string);
   }
   if (!found_default && !found_group && !found_host) {
      sge_dstring_append(string, "NONE");
   }
   sge_dstring_free(&host_string);
   DRETURN(true);
}

/*
descriptor        ASTR_Type
href_nm           ASTR_href
value_nm          ASTR_value
*/
static bool
attr_list_parse_from_string(lList **this_list, lList **answer_list,
                            const char *string, int flags,
                            const lDescr *descriptor, int href_nm, 
                            int value_nm)
{
   bool ret = true;
   DENTER(TOP_LAYER, "attr_list_parse_from_string");
  
   if (this_list != NULL && string != NULL) { 
      struct saved_vars_s *strtok_context = NULL;
      char *token = NULL;
      char *next_token = NULL;
      bool is_first_token = true;
      bool is_last_token = false;

      /* 
       * start         := value {',' group_value} .
       * group_value   := '[' group_or_host '=' value ']' . 
       * group_or_host := ['@'] name .
       * value         := <depends on listtype>
       * 
       * example: lic=5,fast=1,[@group=lic=4,fast=0],[b=lic=0] 
       *
       * lic=5,fast=1, 
       * @group=lic=4,fast=0], 
       * b=lic=0]
       * 
       * lic=5,fast=1
       * @group=lic=4,fast=0
       * b=lic=0
       *
       * default  lic=5,fast=1
       * @group   lic=4,fast=0
       * b        lic=0
       * 
       */

      next_token = sge_strtok_r(string, "[", &strtok_context);
      while (ret && (token = next_token)) {
         size_t length; 

         next_token = sge_strtok_r(NULL, "[", &strtok_context);
         if (next_token == NULL) {
            is_last_token = true;
         }

         /*
          * There might be white space at the end of each token.
          */
         sge_strip_white_space_at_eol(token);
         length = strlen(token);

         if (length >= 1) {
            const char *href_name = NULL;
            char *value = NULL;
            bool first_is_default = true;
  
            /* 
             * All except the last token has to conatin a ',' as last
             * character in the string. This ',' has to be removed.
             */
            if (ret && !is_last_token) {
               if (token[length - 1] == ',') {
                  token[length - 1] = '\0';
                  length--;
               } else {
                  SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_ATTR_MISSINGCOMMA_S,
                                         string));
                  answer_list_add(answer_list, SGE_EVENT,
                                  STATUS_ERROR1, ANSWER_QUALITY_ERROR);
                  ret = false;
               }
            }
            else if (ret && is_last_token && (token[length - 1] == ',')) {
               SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_ATTR_TRAILINGCOMMA_S,
                                      string));
               answer_list_add(answer_list, SGE_EVENT,
                               STATUS_ERROR1, ANSWER_QUALITY_ERROR);
               ret = false;
            }

            /*
             * There might be space after a closing brace ']'
             */
            sge_strip_white_space_at_eol(token);
            length = strlen(token);

            /* 
             * All except the first token has to end with a ']'. Also
             * this charcter has to be removed.
             */
            if (ret && !is_first_token) {
               if (token[length - 1] == ']') {
                  token[length - 1] = '\0';
                  length--;
               } else {
                  SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_ATTR_MISSINGBRACKET_S,
                                         string));
                  answer_list_add(answer_list, SGE_EVENT,
                                  STATUS_ERROR1, ANSWER_QUALITY_ERROR);
                  ret = false;
               }
            }

            /*
             * If the first token containes a ']' as last charcter than
             * a default value is missing! This is not a error but
             * we have to parse a group or host additionally
             */ 
            if (ret && is_first_token) {
               if (token[length - 1] == ']') {
                  token[length - 1] = '\0';
                  length--;
                  first_is_default = false;
               } 
            }

            /*
             * All but the first token has to contain a hostgroup
             * or a host reference in the beginning of the string 
             */
            if (ret) {
               if (!is_first_token || !first_is_default) {
                  value = strchr(token, '=');
                  href_name = token;

                  if (value != NULL) {
                     value[0] = '\0';
                     value++;
                  } else {
                     SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX,
                                            MSG_ATTR_EQUALSIGNEXPRECTED));
                     answer_list_add(answer_list, SGE_EVENT,
                                     STATUS_ERROR1, ANSWER_QUALITY_ERROR);
                     ret = false;
                  }
               } else {
                  href_name = HOSTREF_DEFAULT;
                  value = token;
               }
            }

            /*
             * Parsing the token was successfull. We can create a new 
             * element.
             */
            if (ret) {
               lListElem *attr_elem = NULL;
     
               attr_elem = attr_create(answer_list, href_name, NULL,
                                       descriptor, href_nm, value_nm);
               if (attr_elem != NULL) {
                  ret &= object_parse_field_from_string(attr_elem, 
                                                        answer_list,
                                                        value_nm, value);
                  if (ret) {
                     ret &= attr_list_add(this_list, answer_list,
                                          &attr_elem, flags, NULL,
                                          descriptor, href_nm, value_nm);
                  } else {
                     SGE_ADD_MSG_ID(sprintf(SGE_EVENT, 
                                    MSG_ATTR_PARSINGERROR_S, value));
                     answer_list_add(answer_list, SGE_EVENT,
                                     STATUS_ERROR1, ANSWER_QUALITY_ERROR);
                  }
                  if (!ret) {
                     lFreeElem(&attr_elem);
                  }
               } else {
                  ret = false;
               }
            }
         } else {
            SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_ATTR_NOVALUEGIVEN));
            answer_list_add(answer_list, SGE_EVENT,
                            STATUS_ERROR1, ANSWER_QUALITY_ERROR);
            ret = false;
         }
         is_first_token = false;
      } 
      sge_free_saved_vars(strtok_context);
      strtok_context = NULL;
   } else {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_INAVLID_PARAMETER_IN_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT, 
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      ret = false;
   } 

   DEXIT;
   return ret;
}

/*
href_nm           ASTR_href
*/
static lListElem *
attr_list_locate(const lList *this_list, const char *host_or_group, int href_nm)
{
   lListElem *ret = NULL;

   DENTER(HOSTATTR_LAYER, "attr_list_locate");
   if (this_list != NULL && host_or_group != NULL) {
      ret = lGetElemHost(this_list, href_nm, host_or_group);
   }
   DEXIT;
   return ret; 
}

TEMPLATE_ATTR_IMPL(str_attr, const char *, const char *, ASTR_Type, ASTR_href, ASTR_value) 

TEMPLATE_ATTR_IMPL(ulng_attr, u_long32, u_long32, AULNG_Type, AULNG_href, AULNG_value) 

TEMPLATE_ATTR_IMPL(bool_attr, bool, bool, ABOOL_Type, ABOOL_href, ABOOL_value) 

TEMPLATE_ATTR_IMPL(time_attr, const char *, const char *, ATIME_Type, ATIME_href, ATIME_value) 

TEMPLATE_ATTR_IMPL(mem_attr, const char *, const char *, AMEM_Type, AMEM_href, AMEM_value) 

TEMPLATE_ATTR_IMPL(inter_attr, const char *, const char *, AINTER_Type, AINTER_href, AINTER_value) 

TEMPLATE_ATTR_IMPL(qtlist_attr, u_long32, u_long32, AQTLIST_Type, AQTLIST_href, AQTLIST_value) 


TEMPLATE_ATTR_IMPL(strlist_attr, const char *, lList *, ASTRLIST_Type, ASTRLIST_href, ASTRLIST_value) 

TEMPLATE_ATTR_IMPL(usrlist_attr, const char *, lList *, AUSRLIST_Type, AUSRLIST_href, AUSRLIST_value) 

TEMPLATE_ATTR_IMPL(prjlist_attr, const char *, lList *, APRJLIST_Type, APRJLIST_href, APRJLIST_value) 

TEMPLATE_ATTR_IMPL(celist_attr, const char *, lList *, ACELIST_Type, ACELIST_href, ACELIST_value) 

TEMPLATE_ATTR_IMPL(solist_attr, const char *, lList *, ASOLIST_Type, ASOLIST_href, ASOLIST_value) 


