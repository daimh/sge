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
 *   Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#define NO_SGE_COMPILE_DEBUG

#include <string.h>

#include "sge.h"

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"

#include "comm/commlib.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_utility.h"

#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_userprj.h"

#include "sched/sort_hosts.h"
#include "sched/sge_complex_schedd.h"
#include "sched/sge_select_queue.h"
#include "sched/debit.h"

#include "spool/sge_spooling.h"
#include "spool/sge_spooling_utilities.h"
#include "spool/msg_spoollib.h"
#include "spool/flatfile/msg_spoollib_flatfile.h"

#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_job.h"

#include "msg_common.h"

const spool_instr spool_config_subinstr = {
   CULL_SUBLIST,
   false,
   false,
   NULL,
   NULL
};

const spool_instr spool_config_instr = {
   CULL_SPOOL,
   true,
   true,
   &spool_config_subinstr,
   NULL
};

const spool_instr spool_complex_subinstr = {
   CULL_SPOOL,
   false,
   false,
   NULL,
   NULL
};

const spool_instr spool_complex_instr = {
   CULL_SPOOL,
   false,
   false,
   &spool_complex_subinstr,
   NULL
};

const spool_instr spool_userprj_subinstr = {
   CULL_SUBLIST,
   false,
   false,
   &spool_userprj_subinstr,
   NULL
};

const spool_instr spool_user_instr = {
   CULL_SPOOL | CULL_SPOOL_USER,
   true,
   true,
   &spool_userprj_subinstr,
   NULL
};

static spooling_field *
_spool_get_fields_to_spool(lList **answer_list, const lDescr *descr, 
                           const spool_instr *instr);

/****** spool/utilities/spool_get_fields_to_spool() *********************
*  NAME
*     spool_get_fields_to_spool() -- which fields are to be spooled
*
*  SYNOPSIS
*     spooling_field * 
*     spool_get_fields_to_spool(lList **answer_list, const lDescr *descr, 
*                               const spool_instr *instr) 
*
*  FUNCTION
*     Returns an array of field descriptions (sge_spooling_field).
*     Which fields have to be spooled is retrieved from the CULL object
*     definition given in <descr> and the spooling instruction <instr>.
*
*     For each attribute the function checks if the attribute information 
*     fulfils the selection given in <instr> (e.g. attribute property 
*     CULL_SPOOL). 
*
*     If <instr> contains an in struction for sublists, the function will
*     try to figure out the CULL object definition for the sublists
*     (by calling object_get_subtype) and call itself recursively.
*
*  INPUTS
*     lList **answer_list      - answer list to report errors
*     const lDescr *descr      - object type to analyze
*     const spool_instr *instr - spooing instructions to use
*
*  RESULT
*     spooling_field * - an array of type spooling_field, or 
*                        NULL, if an error occured, error messages are returned
*                        in answer_list
*
*  NOTES
*     The returned spooling_field array has to be freed by the caller of this
*     function using the function spool_free_spooling_fields().
*
*  SEE ALSO
*     gdi/object/object_get_subtype()
*     spool/utilities/spool_free_spooling_fields()
*******************************************************************************/
spooling_field * 
spool_get_fields_to_spool(lList **answer_list, const lDescr *descr, 
                          const spool_instr *instr)
{
   spooling_field *fields;

   DENTER(TOP_LAYER, "spool_get_fields_to_spool");

   SGE_CHECK_POINTER_NULL(descr, answer_list);

   fields = _spool_get_fields_to_spool(answer_list, descr, instr);

   DRETURN(fields);
}

static spooling_field *
_spool_get_fields_to_spool(lList **answer_list, const lDescr *descr, 
                           const spool_instr *instr)
{
   spooling_field *fields;
   int i, j, size;
   int strip = 0;

   DENTER(TOP_LAYER, "_spool_get_fields_to_spool");

   /* we don't check descr and instr, as we know they are ok
    * (it's a static function)
    */

   /* count fields to spool */
   for (i = 0, size = 0; mt_get_type(descr[i].mt) != lEndT; i++) {
      if ((descr[i].mt & instr->selection) != 0) {
         size++;
      }
   }

   /* allocate memory */
   fields = (spooling_field *)malloc((size + 1) * sizeof(spooling_field));
   if (fields == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_UNABLETOALLOCATEBYTES_DS, 
                              (size * 1) * sizeof(spooling_field), SGE_FUNC);
      DRETURN(NULL);
   }

   /* initialize fields */
   for (i = 0; i < size; i++) {
      fields[i].nm         = NoName;
      fields[i].width      = 0;
      fields[i].name       = NULL;
      fields[i].sub_fields = NULL;
      fields[i].clientdata = NULL;
      fields[i].read_func  = NULL;
      fields[i].write_func = NULL;
   }

   /* do we have to strip field prefixes, e.g. "QU_" from field names? */
   if (instr->copy_field_names && instr->strip_field_prefix) {
      dstring buffer = DSTRING_INIT;
      const char *prefix = object_get_name_prefix(descr, &buffer);
      strip = prefix ? strlen(prefix) : 0;
      sge_dstring_free(&buffer);
   }

   /* copy field info */
   for (i = 0, j = 0; mt_get_type(descr[i].mt) != lEndT; i++) {
      if ((descr[i].mt & instr->selection) != 0) {
         spooling_field *sub_fields = NULL;

         DPRINTF(("field "SFQ" will be spooled\n", lNm2Str(descr[i].nm)));

         fields[j].nm         = descr[i].nm;

         if (instr->copy_field_names) {
            const char *name;
            name = lNm2Str(descr[i].nm);
            if(name == NULL || strlen(name) <= strip) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR, 
                                       MSG_NONAMEFORATTRIBUTE_D, 
                                       descr[i].nm);
               fields = spool_free_spooling_fields(fields);
               DEXIT;
               return NULL;
            }
            fields[j].name = strdup(name + strip);
         }
         
         if (mt_get_type(descr[i].mt) == lListT) {
            const lDescr *sub_descr;

            if (instr->sub_instr == NULL) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR,
                                       MSG_DONTKNOWHOWTOSPOOLSUBLIST_SS,
                                       lNm2Str(descr[i].nm), SGE_FUNC);
               fields = spool_free_spooling_fields(fields);
               DEXIT;
               return NULL;
            }

            sub_descr = object_get_subtype(descr[i].nm);
            if (sub_descr == NULL) {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                       ANSWER_QUALITY_ERROR,
                                       MSG_UNKNOWNOBJECTTYPEFOR_SS,
                                       lNm2Str(descr[i].nm), SGE_FUNC);
               fields = spool_free_spooling_fields(fields);
               DEXIT;
               return NULL;
            }

            /* recursive spooling, e.g. sharetree */
            if (instr->sub_instr == instr && descr == sub_descr) {
               sub_fields = fields;
               DPRINTF(("recursive structure detected for field %s\n",
                        lNm2Str(descr[i].nm)));
            } else {
               sub_fields = _spool_get_fields_to_spool(answer_list, sub_descr, 
                                                       instr->sub_instr);
            }
         }

         fields[j++].sub_fields = sub_fields;
      }
   }

   /* end of field array */
   fields[j].nm = NoName;

   DEXIT;
   return fields;
}

/****** spool/utilities/spool_free_spooling_fields() ********************
*  NAME
*     spool_free_spooling_fields() -- free a spooling field array
*
*  SYNOPSIS
*     spooling_field * spool_free_spooling_fields(spooling_field *fields) 
*
*  FUNCTION
*     Frees an array of spooling_field with all sublists and contained strings.
*
*  INPUTS
*     spooling_field *fields - the field array to free
*
*  RESULT
*     spooling_field * - NULL
*
*  EXAMPLE
*     fields = spool_free_spooling_fields(fields);
*******************************************************************************/
spooling_field * 
spool_free_spooling_fields(spooling_field *fields)
{
   if (fields != NULL) {
      int i;
      for (i = 0; fields[i].nm >=0; i++) {
         if (fields[i].sub_fields != NULL && fields[i].sub_fields != fields) {
            fields[i].sub_fields = spool_free_spooling_fields(fields[i].sub_fields);
         }

         if (fields[i].name != NULL) {
            sge_free(&(fields[i].name));
         }
      }
      sge_free(&fields);
   }

   return NULL;
}

/****** spool/utilities/spool_default_validate_func() ****************
*  NAME
*     spool_default_validate_func() -- validate objects
*
*  SYNOPSIS
*     bool
*     spool_default_validate_func(lList **answer_list, 
*                               const lListElem *type, 
*                               const lListElem *rule, 
*                               const lListElem *object, 
*                               const char *key, 
*                               const sge_object_type object_type) 
*
*  FUNCTION
*     Verifies an object.
*
*  INPUTS
*     lList **answer_list - to return error messages
*     const lListElem *type           - object type description
*     const lListElem *rule           - rule to use
*     const lListElem *object         - object to validate
*     const sge_object_type object_type - object type
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     This function should not be called directly, it is called by the
*     spooling framework.
*
*  SEE ALSO
*******************************************************************************/
bool spool_default_validate_func(lList **answer_list, 
                          const lListElem *type, 
                          const lListElem *rule,
                          lListElem *object,
                          const sge_object_type object_type)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_default_validate_func");

   switch(object_type) {
      case SGE_TYPE_ADMINHOST:
      case SGE_TYPE_EXECHOST:
      case SGE_TYPE_SUBMITHOST:
         {
            int cl_ret;
            int key_nm = object_type_get_key_nm(object_type);
            char *old_name = strdup(lGetHost(object, key_nm));

            /* try hostname resolving */
            if (strcmp(old_name, SGE_GLOBAL_NAME) != 0) {
               cl_ret = sge_resolve_host(object, key_nm);

               /* if hostname resolving failed: create error */
               if (cl_ret != CL_RETVAL_OK) {
                  if (cl_ret != CL_RETVAL_GETHOSTNAME_ERROR) {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                             ANSWER_QUALITY_ERROR, 
                                             MSG_SPOOL_CANTRESOLVEHOSTNAME_SS, 
                                             old_name, cl_get_error_text(ret)); 
                     ret = false;
                  } else {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                             ANSWER_QUALITY_WARNING, 
                                             MSG_SPOOL_CANTRESOLVEHOSTNAME_SS, 
                                             old_name, cl_get_error_text(ret));
                  }
               } else {
                  /* if hostname resolving changed hostname: spool */
                  const char *new_name;
                  new_name = lGetHost(object, key_nm);
                  if (strcmp(old_name, new_name) != 0) {
                     spooling_write_func write_func = 
                             (spooling_write_func)lGetRef(rule, SPR_write_func);
                     spooling_delete_func delete_func = 
                             (spooling_delete_func)lGetRef(rule, SPR_delete_func);
                     write_func(answer_list, type, rule, object, new_name, 
                                object_type);
                     delete_func(answer_list, type, rule, old_name, object_type);
                  }
               }
            }

            sge_free(&old_name);

            if (object_type == SGE_TYPE_EXECHOST && ret) {
               lListElem *load_value;
               lList *master_centry_list = *object_type_get_master_list(SGE_TYPE_CENTRY);

               /* all spooled load values are static, therefore we tag them here */
               for_each(load_value, lGetList(object, EH_load_list)) {
                  lSetBool(load_value, HL_static, true);
               }

               /* necessary to init double values of consumable configuration */
               centry_list_fill_request(lGetList(object, EH_consumable_config_list), 
                     NULL, master_centry_list, true, false, true);
               /* necessary to setup actual list of exechost */
               debit_host_consumable(NULL, object, master_centry_list, 0, true, NULL);

               if (ensure_attrib_available(NULL, object, 
                                           EH_consumable_config_list)) {
                  ret = false;
               }
            }
         }
         break;
      case SGE_TYPE_QINSTANCE:
         ret = qinstance_validate(object, answer_list, *object_type_get_master_list(SGE_TYPE_EXECHOST));
         break;
      case SGE_TYPE_CQUEUE:
         ret = qinstance_list_validate(lGetList(object, CQ_qinstances), answer_list, *object_type_get_master_list(SGE_TYPE_EXECHOST));
         break;
      case SGE_TYPE_CONFIG:
         {
            int cl_ret;
            char *old_name = strdup(lGetHost(object, CONF_name));

            /* try hostname resolving */
            if (strcmp(old_name, SGE_GLOBAL_NAME) != 0) {
               cl_ret = sge_resolve_host(object, CONF_name);
               /* if hostname resolving failed: create error */
               if (cl_ret != CL_RETVAL_OK) {
                  if (cl_ret != CL_RETVAL_GETHOSTNAME_ERROR) {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                             ANSWER_QUALITY_ERROR, 
                                             MSG_SPOOL_CANTRESOLVEHOSTNAME_SS, 
                                             old_name, cl_get_error_text(ret)); 
                     ret = false;
                  } else {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, 
                                             ANSWER_QUALITY_WARNING, 
                                             MSG_SPOOL_CANTRESOLVEHOSTNAME_SS, 
                                             old_name, cl_get_error_text(ret));
                  }
               } else {
                  /* if hostname resolving changed hostname: spool */
                  const char *new_name = lGetHost(object, CONF_name);
                  if (strcmp(old_name, new_name) != 0) {
                     spooling_write_func write_func = 
                             (spooling_write_func)lGetRef(rule, SPR_write_func);
                     spooling_delete_func delete_func = 
                             (spooling_delete_func)lGetRef(rule, SPR_delete_func);
                     write_func(answer_list, type, rule, object, new_name, 
                                object_type);
                     delete_func(answer_list, type, rule, old_name, object_type);
                  }
               }
            }
            sge_free(&old_name);
         }
         break;
      case SGE_TYPE_USERSET:
         if (userset_validate_entries(object, answer_list, 1) != STATUS_OK) {
            ret = false;
         }
         break;
      case SGE_TYPE_CKPT:
         if (ckpt_validate(object, answer_list) != STATUS_OK) {
            ret = false;
         }
         break;
      case SGE_TYPE_PE:
         if (pe_validate(object, answer_list, 1) != STATUS_OK) {
            ret = false;
         }
         break;
      case SGE_TYPE_CENTRY:
         if (!centry_elem_validate(object, *object_type_get_master_list(SGE_TYPE_CENTRY), answer_list)) {
            ret = false;
         }
         break;
      case SGE_TYPE_RQS:
         if (!rqs_verify_attributes(object, answer_list, true)) {
            ret = false;
         }
         break;
      case SGE_TYPE_AR:
         if (!ar_validate(object, answer_list, true, true)) {
            ret = false;
         }
         break;
      case SGE_TYPE_USER:
         NULL_OUT_NONE(object, UU_default_project);
         break;
      case SGE_TYPE_MANAGER:
      case SGE_TYPE_OPERATOR:
      case SGE_TYPE_HGROUP:
#ifndef __SGE_NO_USERMAPPING__
      case SGE_TYPE_CUSER:
#endif
      case SGE_TYPE_CALENDAR:
      case SGE_TYPE_PROJECT:
      case SGE_TYPE_SHARETREE:
      case SGE_TYPE_SCHEDD_CONF:
      case SGE_TYPE_JOB:
      default:
         break;
   }

   DRETURN(ret);
}


bool
spool_default_validate_list_func(lList **answer_list, 
                          const lListElem *type, const lListElem *rule,
                          const sge_object_type object_type)
{
   bool ret = true;

   DENTER(TOP_LAYER, "spool_default_validate_list_func");

   switch(object_type) {
      case SGE_TYPE_ADMINHOST:
         break;
      case SGE_TYPE_EXECHOST:
         host_list_merge(*object_type_get_master_list(SGE_TYPE_EXECHOST));
         break;
      case SGE_TYPE_SUBMITHOST:
      case SGE_TYPE_CONFIG:
      case SGE_TYPE_USERSET:
      case SGE_TYPE_CKPT:
      case SGE_TYPE_PE:
         break;
      case SGE_TYPE_CENTRY:
         centry_list_sort(*object_type_get_master_list(SGE_TYPE_CENTRY));
         break;
      case SGE_TYPE_MANAGER:
      case SGE_TYPE_OPERATOR:
      case SGE_TYPE_HGROUP:
#ifndef __SGE_NO_USERMAPPING__
      case SGE_TYPE_CUSER:
#endif
      case SGE_TYPE_CALENDAR:
      case SGE_TYPE_PROJECT:
      case SGE_TYPE_USER:
      case SGE_TYPE_SHARETREE:
         break;
      case SGE_TYPE_SCHEDD_CONF:
         ret = sconf_validate_config_(answer_list);
         break;
      case SGE_TYPE_JOB:
         job_list_sort(*object_type_get_master_list(SGE_TYPE_JOB));
         break;
      case SGE_TYPE_AR:
         ar_list_sort(*object_type_get_master_list(SGE_TYPE_AR));
         break;
      default:
         break;
   }

   DRETURN(ret);
}

