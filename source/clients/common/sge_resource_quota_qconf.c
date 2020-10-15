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

#include <string.h>
 
#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_edit.h"
#include "uti/sge_prog.h"

#include "gdi/sge_gdi.h"

#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_answer.h"

#include "spool/flatfile/sge_flatfile.h"
#include "spool/flatfile/sge_flatfile_obj.h"

#include "sge_resource_quota_qconf.h"
#include "msg_common.h"
#include "msg_clients_common.h"

static bool rqs_provide_modify_context(sge_gdi_ctx_class_t *ctx,
                                  lList **rqs_list, lList **answer_list,
                                  bool ignore_unchanged_message);


/****** resource_quota_qconf/rqs_show() *********************************
*  NAME
*     rqs_show() -- show resource quota sets
*
*  SYNOPSIS
*     bool rqs_show(lList **answer_list, const char *name) 
*
*  FUNCTION
*     This funtion gets the selected resource quota sets from GDI and
*     writes they out on stdout
*
*  INPUTS
*     lList **answer_list - answer list
*     const char *name    - comma separated list of resource quota set names
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_show() is MT safe 
*
*******************************************************************************/
bool rqs_show(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   lList *rqs_list = NULL;
   bool ret = false;

   DENTER(TOP_LAYER, "rqs_show");

   if (name != NULL) {
      lList *rqsref_list = NULL;

      lString2List(name, &rqsref_list, RQS_Type, RQS_name, ", ");
      ret = rqs_get_via_gdi(ctx, answer_list, rqsref_list, &rqs_list);
      lFreeList(&rqsref_list);
   } else {
      ret = rqs_get_all_via_gdi(ctx, answer_list, &rqs_list);
   }

   if (ret && lGetNumberOfElem(rqs_list)) {
      const char* filename;
      filename = spool_flatfile_write_list(answer_list, rqs_list, RQS_fields, 
                                        &qconf_rqs_sfi,
                                        SP_DEST_STDOUT, SP_FORM_ASCII, NULL,
                                        false);
      sge_free(&filename);
   }
   if (lGetNumberOfElem(rqs_list) == 0) {
      answer_list_add(answer_list, MSG_NORQSFOUND, STATUS_EEXIST, ANSWER_QUALITY_WARNING);
      ret = false;
   }

   lFreeList(&rqs_list);

   DRETURN(ret);
}

/****** resource_quota_qconf/rqs_get_via_gdi() **************************
*  NAME
*     rqs_get_via_gdi() -- get resource quota sets from GDI
*
*  SYNOPSIS
*     bool rqs_get_via_gdi(lList **answer_list, const lList 
*     *rqsref_list, lList **rqs_list) 
*
*  FUNCTION
*     This function gets the selected resource quota sets from qmaster. The selection
*     is done in the string list rqsref_list.
*
*  INPUTS
*     lList **answer_list       - answer list
*     const lList *rqsref_list - resource quota sets selection
*     lList **rqs_list         - copy of the selected rule sets
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_get_via_gdi() is MT safe 
*
*******************************************************************************/
bool rqs_get_via_gdi(sge_gdi_ctx_class_t *ctx, lList **answer_list, const lList *rqsref_list,
                            lList **rqs_list)
{
   bool ret = false;

   DENTER(TOP_LAYER, "rqs_get_via_gdi");
   if (rqsref_list != NULL) {
      lListElem *rqsref = NULL;
      lCondition *where = NULL;
      lEnumeration *what = NULL;

      what = lWhat("%T(ALL)", RQS_Type);

      for_each(rqsref, rqsref_list) {
         lCondition *add_where = NULL;
         add_where = lWhere("%T(%I p= %s)", RQS_Type, RQS_name, lGetString(rqsref, RQS_name));
         if (where == NULL) {
            where = add_where;
         } else {
            where = lOrWhere(where, add_where);
         }
      }
      *answer_list = ctx->gdi(ctx, SGE_RQS_LIST, SGE_GDI_GET, rqs_list, where, what, false);
      if (!answer_list_has_error(answer_list)) {
         ret = true;
      }

      lFreeWhat(&what);
      lFreeWhere(&where);
   }

   DRETURN(ret);
}

/****** resource_quota_qconf/rqs_get_all_via_gdi() **********************
*  NAME
*     rqs_get_all_via_gdi() -- get all resource quota sets from GDI 
*
*  SYNOPSIS
*     bool rqs_get_all_via_gdi(lList **answer_list, lList **rqs_list) 
*
*  FUNCTION
*     This function gets all resource quota sets known by qmaster
*
*  INPUTS
*     lList **answer_list - answer list
*     lList **rqs_list   - copy of all resource quota sets
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_get_all_via_gdi() is MT safe 
*
*******************************************************************************/
bool rqs_get_all_via_gdi(sge_gdi_ctx_class_t *ctx, lList **answer_list, lList **rqs_list)
{
   bool ret = false;
   lEnumeration *what = lWhat("%T(ALL)", RQS_Type);

   DENTER(TOP_LAYER, "rqs_get_all_via_gdi");

   *answer_list = ctx->gdi(ctx, SGE_RQS_LIST, SGE_GDI_GET, rqs_list, NULL, what, false);
   if (!answer_list_has_error(answer_list)) {
      ret = true;
   }

   lFreeWhat(&what);

   DRETURN(ret);
}

/****** resource_quota_qconf/rqs_add() ******************************
*  NAME
*     rqs_add() -- add resource quota set list
*
*  SYNOPSIS
*     bool rqs_add(lList **answer_list, const char *name) 
*
*  FUNCTION
*     This function provide a modify context for qconf to add new resource
*     quota sets. If no name is given a template rule set is shown
*
*  INPUTS
*     lList **answer_list - answer list
*     const char *name    - comma seperated list of rule sets to add
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_add() is MT safe 
*
*******************************************************************************/
bool rqs_add(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   bool ret = false;

   DENTER(TOP_LAYER, "rqs_list_add");
   if (name != NULL) {
      lList *rqs_list = NULL;
      lListElem *rqs = NULL;

      lString2List(name, &rqs_list, RQS_Type, RQS_name, ", ");
      for_each (rqs, rqs_list) {
         rqs = rqs_set_defaults(rqs);
      }

      ret = rqs_provide_modify_context(ctx, &rqs_list, answer_list, true);

      if (ret) {
         ret = rqs_add_del_mod_via_gdi(ctx, rqs_list, answer_list, SGE_GDI_ADD | SGE_GDI_SET_ALL);
      }

      lFreeList(&rqs_list);
   }

   DRETURN(ret);
}

/****** resource_quota_qconf/rqs_modify() ***************************
*  NAME
*     rqs_modify() -- modify resource quota sets
*
*  SYNOPSIS
*     bool rqs_modify(lList **answer_list, const char *name) 
*
*  FUNCTION
*     This function provides a modify context for qconf to modify resource
*     quota sets.
*
*  INPUTS
*     lList **answer_list - answer list
*     const char *name    - comma seperated list of rule sets to modify
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_modify() is MT safe 
*
*******************************************************************************/
bool rqs_modify(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   bool ret = false;
   lList *rqs_list = NULL;
   u_long32 gdi_command = 0;

   DENTER(TOP_LAYER, "rqs_modify");

   if (name != NULL) {
      lList *rqsref_list = NULL;

      gdi_command = SGE_GDI_MOD | SGE_GDI_SET_ALL;

      lString2List(name, &rqsref_list, RQS_Type, RQS_name, ", ");
      ret = rqs_get_via_gdi(ctx, answer_list, rqsref_list, &rqs_list);
      lFreeList(&rqsref_list);
   } else {
      gdi_command = SGE_GDI_REPLACE;
      ret = rqs_get_all_via_gdi(ctx, answer_list, &rqs_list);
   }

   if (ret) {
      ret = rqs_provide_modify_context(ctx, &rqs_list, answer_list, false);
   }
   if (ret) {
      ret = rqs_add_del_mod_via_gdi(ctx, rqs_list, answer_list,
                                       gdi_command);
   }

   lFreeList(&rqs_list);

   DRETURN(ret);
}

/****** resource_quota_qconf/rqs_add_from_file() ********************
*  NAME
*     rqs_add_from_file() -- add resource quota set from file
*
*  SYNOPSIS
*     bool rqs_add_from_file(lList **answer_list, const char 
*     *filename) 
*
*  FUNCTION
*     This function add new resource quota sets from file.
*
*  INPUTS
*     lList **answer_list  - answer list
*     const char *filename - filename of new resource quota sets
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_add_from_file() is MT safe 
*
*******************************************************************************/
bool rqs_add_from_file(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *filename)
{
   bool ret = false;

   DENTER(TOP_LAYER, "rqs_add_from_file");
   if (filename != NULL) {
      lList *rqs_list = NULL;

      /* fields_out field does not work for rqs because of duplicate entry */
      rqs_list = spool_flatfile_read_list(answer_list, RQS_Type, RQS_fields,
                                          NULL, true, &qconf_rqs_sfi,
                                          SP_FORM_ASCII, NULL, filename);
      if (!answer_list_has_error(answer_list)) {
         ret = rqs_add_del_mod_via_gdi(ctx, rqs_list, answer_list, 
                                       SGE_GDI_ADD | SGE_GDI_SET_ALL); 
      }

      lFreeList(&rqs_list);
   }
   DRETURN(ret);
}

/****** resource_quota_qconf/rqs_provide_modify_context() ***********
*  NAME
*     rqs_provide_modify_context() -- provide qconf modify context
*
*  SYNOPSIS
*     bool rqs_provide_modify_context(lList **rqs_list, lList 
*     **answer_list, bool ignore_unchanged_message) 
*
*  FUNCTION
*     This function provides a editor session to edit the selected resource quota
*     sets interactively. 
*
*  INPUTS
*     lList **rqs_list             - resource quota sets to modify
*     lList **answer_list           - answer list
*     bool ignore_unchanged_message - ignore unchanged message
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_provide_modify_context() is MT safe 
*
*******************************************************************************/
static bool rqs_provide_modify_context(sge_gdi_ctx_class_t *ctx, lList **rqs_list, lList **answer_list,
                                           bool ignore_unchanged_message)
{
   bool ret = false;
   int status = 0;
   const char *filename = NULL;
   uid_t uid = ctx->get_uid(ctx);
   gid_t gid = ctx->get_gid(ctx);
   
   DENTER(TOP_LAYER, "rqs_provide_modify_context");

   if (rqs_list == NULL) {
      answer_list_add(answer_list, MSG_PARSE_NULLPOINTERRECEIVED, 
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      DRETURN(ret); 
   }

   if (*rqs_list == NULL) {
      *rqs_list = lCreateList("", RQS_Type);
   }

   filename = spool_flatfile_write_list(answer_list, *rqs_list, RQS_fields,
                                        &qconf_rqs_sfi, SP_DEST_TMP,
                                        SP_FORM_ASCII, filename, false);

   if (answer_list_has_error(answer_list)) {
      if (filename != NULL) {
         unlink(filename);
         sge_free(&filename);
      }
      DRETURN(ret);
   }

   status = sge_edit(filename, uid, gid);

   if (status == 0) {
      lList *new_rqs_list = NULL;

      /* fields_out field does not work for rqs because of duplicate entry */
      new_rqs_list = spool_flatfile_read_list(answer_list, RQS_Type, RQS_fields,
                                               NULL, true, &qconf_rqs_sfi,
                                               SP_FORM_ASCII, NULL, filename);
      if (answer_list_has_error(answer_list)) {
         lFreeList(&new_rqs_list);
      }
      if (new_rqs_list != NULL) {
         if (ignore_unchanged_message || object_list_has_differences(new_rqs_list, answer_list, *rqs_list, false)) {
            lFreeList(rqs_list);
            *rqs_list = new_rqs_list;
            ret = true;
         } else {
            lFreeList(&new_rqs_list);
            answer_list_add(answer_list, MSG_FILE_NOTCHANGED,
                            STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         }
      } else {
         answer_list_add(answer_list, MSG_FILE_ERRORREADINGINFILE,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
   } else if (status == 1) {
      answer_list_add(answer_list, MSG_FILE_FILEUNCHANGED,
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   } else {
      answer_list_add(answer_list, MSG_PARSE_EDITFAILED,
                      STATUS_ERROR1, ANSWER_QUALITY_ERROR);
   }

   unlink(filename);
   sge_free(&filename);
   DRETURN(ret);
}

/****** resource_quota_qconf/rqs_add_del_mod_via_gdi() **************
*  NAME
*    rqs_add_del_mod_via_gdi/rqs_add_del_mod_via_gdi() -- modfies qmaster resource quota sets
*
*  SYNOPSIS
*     bool rqs_add_del_mod_via_gdi(lList *rqs_list, lList 
*     **answer_list, u_long32 gdi_command) 
*
*  FUNCTION
*     This function modifies via GDI the qmaster copy of the resource quota sets.
*
*  INPUTS
*     lList *rqs_list     - resource quota sets to modify on qmaster
*     lList **answer_list  - answer list from qmaster
*     u_long32 gdi_command - commands what to do
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_add_del_mod_via_gdi() is MT safe 
*
*******************************************************************************/
bool rqs_add_del_mod_via_gdi(sge_gdi_ctx_class_t *ctx, lList *rqs_list, lList **answer_list,
                                        u_long32 gdi_command) 
{
   bool ret = false;
   
   DENTER(TOP_LAYER, "rqs_add_del_mod_via_gdi");

   if (rqs_list != NULL) {
      u_long32 operation = SGE_GDI_GET_OPERATION(gdi_command);
      bool do_verify = (operation == SGE_GDI_MOD) || (operation == SGE_GDI_ADD
                        || (operation == SGE_GDI_REPLACE)) ? true : false;

      if (do_verify) {
         ret = rqs_list_verify_attributes(rqs_list, answer_list, false);
      }
      if (ret) {
         lList *my_answer_list = ctx->gdi(ctx, SGE_RQS_LIST, gdi_command, &rqs_list, NULL, NULL, false);
         if (my_answer_list != NULL) {
            answer_list_append_list(answer_list, &my_answer_list);
         }
      }
   }
   DRETURN(ret);
}

/****** resource_quota_qconf/rqs_modify_from_file() *****************
*  NAME
*     rqs_modify_from_file() -- modifies resource quota sets from file
*
*  SYNOPSIS
*     bool rqs_modify_from_file(lList **answer_list, const char 
*     *filename, const char* name) 
*
*  FUNCTION
*     This function allows to modify one or all resource quota sets from a file
*
*  INPUTS
*     lList **answer_list  - answer list
*     const char *filename - filename with the resource quota sets to change
*     const char* name     - comma separated list of rule sets to change
*
*  RESULT
*     bool - true  on success
*            false on error
*
*  NOTES
*     MT-NOTE: rqs_modify_from_file() is MT safe 
*
*******************************************************************************/
bool rqs_modify_from_file(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *filename, const char* name) 
{
   bool ret = false;
   u_long32 gdi_command = 0;
   
   DENTER(TOP_LAYER, "rqs_modify_from_file");
   if (filename != NULL) {
      lList *rqs_list = NULL;

      /* fields_out field does not work for rqs because of duplicate entry */
      rqs_list = spool_flatfile_read_list(answer_list, RQS_Type, RQS_fields,
                                          NULL, true, &qconf_rqs_sfi,
                                          SP_FORM_ASCII, NULL, filename);
      if (rqs_list != NULL) {

         if (name != NULL && strlen(name) > 0 ) {
            lList *selected_rqs_list = NULL;
            lListElem *tmp_rqs = NULL;
            lList *found_rqs_list = lCreateList("rqs_list", RQS_Type);

            gdi_command = SGE_GDI_MOD | SGE_GDI_SET_ALL;

            lString2List(name, &selected_rqs_list, RQS_Type, RQS_name, ", ");
            for_each(tmp_rqs, selected_rqs_list) {
               lListElem *found = rqs_list_locate(rqs_list, lGetString(tmp_rqs, RQS_name));
               if (found != NULL) {
                  lAppendElem(found_rqs_list, lCopyElem(found));
               } else {
                  sprintf(SGE_EVENT, MSG_RQSNOTFOUNDINFILE_SS, lGetString(tmp_rqs, RQS_name), filename);
                  answer_list_add(answer_list, SGE_EVENT, STATUS_ERROR1, ANSWER_QUALITY_ERROR);
                  lFreeList(&found_rqs_list);
                  break;
               }
            }
            lFreeList(&selected_rqs_list);
            lFreeList(&rqs_list);
            rqs_list = found_rqs_list;
         } else {
            gdi_command = SGE_GDI_REPLACE;
         }

         if (rqs_list != NULL) {
            ret = rqs_add_del_mod_via_gdi(ctx, rqs_list, answer_list,
                                           gdi_command); 
         }
      }
      lFreeList(&rqs_list);
   }
   DRETURN(ret);
}
