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

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_prog.h"
#include "uti/sge_edit.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_hgroup.h"

#include "spool/flatfile/sge_flatfile.h"
#include "spool/flatfile/sge_flatfile_obj.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sge.h"
#include "sge_hgroup_qconf.h"
#include "msg_common.h"
#include "msg_clients_common.h"


static void 
hgroup_list_show_elem(lList *hgroup_list, const char *name, int indent);
static bool
hgroup_provide_modify_context(sge_gdi_ctx_class_t *ctx, lListElem **this_elem, lList **answer_list,
                                   bool ignore_unchanged_message);


static void 
hgroup_list_show_elem(lList *hgroup_list, const char *name, int indent)
{
   const char *const indent_string = "   ";
   lListElem *hgroup = NULL;
   int i;

   DENTER(TOP_LAYER, "hgroup_list_show_elem");

   for (i = 0; i < indent; i++) {
      printf("%s", indent_string);
   }
   printf("%s\n", name);

   hgroup = lGetElemHost(hgroup_list, HGRP_name, name);   
   if (hgroup != NULL) {
      lList *sub_list = lGetList(hgroup, HGRP_host_list);
      lListElem *href = NULL;

      for_each(href, sub_list) {
         const char *href_name = lGetHost(href, HR_name);

         hgroup_list_show_elem(hgroup_list, href_name, indent + 1); 
      } 
   } 
   DRETURN_VOID;
}

bool 
hgroup_add_del_mod_via_gdi(sge_gdi_ctx_class_t *ctx,
                           lListElem *this_elem, lList **answer_list,
                           u_long32 gdi_command)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_add_del_mod_via_gdi");
   if (this_elem != NULL) {
      lListElem *element = NULL;
      lList *hgroup_list = NULL;
      lList *gdi_answer_list = NULL;

      element = lCopyElem(this_elem);
      hgroup_list = lCreateList("", HGRP_Type);
      lAppendElem(hgroup_list, element);
      gdi_answer_list = ctx->gdi(ctx, SGE_HGRP_LIST, gdi_command,
                                &hgroup_list, NULL, NULL, false);
      answer_list_replace(answer_list, &gdi_answer_list);
      lFreeList(&hgroup_list);
   }
   DRETURN(ret);
}

lListElem *hgroup_get_via_gdi(sge_gdi_ctx_class_t *ctx,
                              lList **answer_list, const char *name) 
{
   lListElem *ret = NULL;

   DENTER(TOP_LAYER, "hgroup_get_via_gdi");
   if (name != NULL) {
      lList *gdi_answer_list = NULL;
      lEnumeration *what = NULL;
      lCondition *where = NULL;
      lList *hostgroup_list = NULL;

      what = lWhat("%T(ALL)", HGRP_Type);
      where = lWhere("%T(%I==%s)", HGRP_Type, HGRP_name, 
                     name);
      gdi_answer_list = ctx->gdi(ctx, SGE_HGRP_LIST, SGE_GDI_GET, 
                                &hostgroup_list, where, what, false);
      lFreeWhat(&what);
      lFreeWhere(&where);

      if (!answer_list_has_error(&gdi_answer_list)) {
         ret = lDechainElem(hostgroup_list, lFirst(hostgroup_list));
      } else {
         answer_list_replace(answer_list, &gdi_answer_list);
      }
      lFreeList(&hostgroup_list);
      lFreeList(&gdi_answer_list);
   } 
   DRETURN(ret);
}

static bool hgroup_provide_modify_context(sge_gdi_ctx_class_t *ctx, lListElem **this_elem, lList **answer_list,
                                   bool ignore_unchanged_message)
{
   bool ret = false;
   int status = 0;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;
   uid_t uid = ctx->get_uid(ctx);
   gid_t gid = ctx->get_gid(ctx);
   
   DENTER(TOP_LAYER, "hgroup_provide_modify_context");
   if (this_elem != NULL && *this_elem != NULL) {
      const char *filename = NULL;
      filename = spool_flatfile_write_object(answer_list, *this_elem,
                                                     false, HGRP_fields,
                                                     &qconf_sfi,
                                                     SP_DEST_TMP, SP_FORM_ASCII,
                                                     filename, false);
      if (answer_list_has_error(answer_list)) {
         if (filename != NULL) {
            unlink(filename);
            sge_free(&filename);
         }
         DRETURN(false);
      }
      
      status = sge_edit(filename, uid, gid);
      
      if (status >= 0) {
         lListElem *hgroup = NULL;

         fields_out[0] = NoName;
         hgroup = spool_flatfile_read_object(answer_list, HGRP_Type, NULL,
                                         HGRP_fields, fields_out, true, &qconf_sfi,
                                         SP_FORM_ASCII, NULL, filename);
            
         if (answer_list_output (answer_list)) {
            lFreeElem(&hgroup);
         }

         if (hgroup != NULL) {
            missing_field = spool_get_unprocessed_field(HGRP_fields, fields_out, answer_list);
         }

         if (missing_field != NoName) {
            lFreeElem(&hgroup);
            answer_list_output (answer_list);
         }      

         if (hgroup != NULL) {
            if (object_has_differences(*this_elem, answer_list,
                                       hgroup, false) || 
                ignore_unchanged_message) {
               lFreeElem(this_elem);
               *this_elem = hgroup;
               ret = true;
            } else {
               lFreeElem(&hgroup);
               answer_list_add(answer_list, MSG_FILE_NOTCHANGED,
                               STATUS_ERROR1, ANSWER_QUALITY_ERROR);
            }
         } else {
            answer_list_add(answer_list, MSG_FILE_ERRORREADINGINFILE,
                            STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         }
      } else {
         answer_list_add(answer_list, MSG_PARSE_EDITFAILED,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
      unlink(filename);
      sge_free(&filename);
   } 
   DRETURN(ret);
}

/****** sge_hgroup_qconf/hgroup_add() ******************************************
*  NAME
*     hgroup_add() -- creates a default hgroup object.
*
*  SYNOPSIS
*     bool hgroup_add(lList **answer_list, const char *name, bool 
*     is_name_validate) 
*
*  FUNCTION
*     To create a new hgrp, qconf needs a default object, that can be edited.
*
*  INPUTS
*     lList **answer_list   - any errors?
*     const char *name      - name of the hgrp
*     bool is_name_validate - should the name be validated? false, if one generates
*                             a template
*
*  RESULT
*     bool - true, if everything went fine
*
*  NOTES
*     MT-NOTE: hgroup_add() is MT safe 
*
*******************************************************************************/
bool hgroup_add(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name, bool is_name_validate ) 
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_add");
   if (name != NULL) {
      lListElem *hgroup = hgroup_create(answer_list, name, NULL, is_name_validate);

      if (hgroup == NULL) {
         ret = false;
      }
      if (ret) {
         ret = hgroup_provide_modify_context(ctx, &hgroup, answer_list, true);
      }
      if (ret) {
         ret = hgroup_add_del_mod_via_gdi(ctx, hgroup, answer_list, SGE_GDI_ADD);
      }

      lFreeElem(&hgroup);
   }
  
   DRETURN(ret); 
}

bool hgroup_add_from_file(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *filename) 
{
   bool ret = true;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;

   DENTER(TOP_LAYER, "hgroup_add");

   if (filename != NULL) {
      lListElem *hgroup;

      fields_out[0] = NoName;
      hgroup = spool_flatfile_read_object(answer_list, HGRP_Type, NULL,
                                      HGRP_fields, fields_out, true, &qconf_sfi,
                                      SP_FORM_ASCII, NULL, filename);

      if (answer_list_output (answer_list)) {
         lFreeElem(&hgroup);
      }

      if (hgroup != NULL) {
         missing_field = spool_get_unprocessed_field (HGRP_fields, fields_out, answer_list);
      }

      if (missing_field != NoName) {
         lFreeElem(&hgroup);
         answer_list_output (answer_list);
      }

      if (hgroup == NULL) {
         ret = false;
      }
      if (ret) {
         ret = hgroup_add_del_mod_via_gdi(ctx, hgroup, answer_list, SGE_GDI_ADD); 
      }
      lFreeElem(&hgroup);
   }

   DRETURN(ret);
}

bool hgroup_modify(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_modify");
   if (name != NULL) {
      lListElem *hgroup = hgroup_get_via_gdi(ctx, answer_list, name);

      if (hgroup == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_ERROR1,
                                 ANSWER_QUALITY_ERROR, MSG_HGROUP_NOTEXIST_S, name);
         ret = false;
      }
      if (ret == true) {
         ret = hgroup_provide_modify_context(ctx, &hgroup, answer_list, false);
      }
      if (ret == true) {
         ret = hgroup_add_del_mod_via_gdi(ctx, hgroup, answer_list, SGE_GDI_MOD);
      }
      lFreeElem(&hgroup);
   }

   DRETURN(ret);
}

bool hgroup_modify_from_file(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *filename)
{
   bool ret = true;
   int fields_out[MAX_NUM_FIELDS];
   int missing_field = NoName;

   DENTER(TOP_LAYER, "hgroup_modify");
   if (filename != NULL) {
      lListElem *hgroup;

      fields_out[0] = NoName;
      hgroup = spool_flatfile_read_object(answer_list, HGRP_Type, NULL,
                                      HGRP_fields, fields_out, true, &qconf_sfi,
                                      SP_FORM_ASCII, NULL, filename);
            
      if (answer_list_output(answer_list)) {
         lFreeElem(&hgroup);
      }

      if (hgroup != NULL) {
         missing_field = spool_get_unprocessed_field(HGRP_fields, fields_out, answer_list);
      }

      if (missing_field != NoName) {
         lFreeElem(&hgroup);
         answer_list_output (answer_list);
      }      

      if (hgroup == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_ERROR1,
                                 ANSWER_QUALITY_ERROR, MSG_HGROUP_FILEINCORRECT_S, filename);
         ret = false;
      }
      if (ret) {
         ret = hgroup_add_del_mod_via_gdi(ctx, hgroup, answer_list, SGE_GDI_MOD);
      }
      if (hgroup) {
         lFreeElem(&hgroup);
      }
   }

   DRETURN(ret);
}

bool hgroup_delete(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_delete");
   if (name != NULL) {
      lListElem *hgroup = hgroup_create(answer_list, name, NULL, true); 
   
      if (hgroup != NULL) {
         ret = hgroup_add_del_mod_via_gdi(ctx, hgroup, answer_list, SGE_GDI_DEL); 
      }
      lFreeElem(&hgroup);
   }
   DRETURN(ret);
}

bool hgroup_show(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_show");
   if (name != NULL) {
      lListElem *hgroup = hgroup_get_via_gdi(ctx, answer_list, name); 
   
      if (hgroup != NULL) {
         const char *filename;
         filename = spool_flatfile_write_object(answer_list, hgroup, false, HGRP_fields,
                                     &qconf_sfi, SP_DEST_STDOUT,
                                     SP_FORM_ASCII, NULL, false);
      
         sge_free(&filename);
         lFreeElem(&hgroup);

         if (answer_list_has_error(answer_list)) {
            DRETURN(false);
         }
      } else {
         answer_list_add_sprintf(answer_list, STATUS_ERROR1,
                                 ANSWER_QUALITY_ERROR, MSG_HGROUP_NOTEXIST_S, name); 
         ret = false;
      }
   }
   DRETURN(ret);
}

bool hgroup_show_structure(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name, 
                           bool show_tree)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_show_tree");
   if (name != NULL) {
      lList *hgroup_list = NULL;
      lListElem *hgroup = NULL;
      lEnumeration *what = NULL;
      lList *alp = NULL;
      lListElem *alep = NULL;

      what = lWhat("%T(ALL)", HGRP_Type);
      alp = ctx->gdi(ctx, SGE_HGRP_LIST, SGE_GDI_GET, &hgroup_list, NULL, what, false);
      lFreeWhat(&what);

      alep = lFirst(alp);
      answer_exit_if_not_recoverable(alep);
      if (answer_get_status(alep) != STATUS_OK) {
         fprintf(stderr, "%s\n", lGetString(alep, AN_text));
         lFreeList(&alp);
         DRETURN(false);
      }

      hgroup = lGetElemHost(hgroup_list, HGRP_name, name); 
      if (hgroup != NULL) {
         if (show_tree) {
            hgroup_list_show_elem(hgroup_list, name, 0);
         } else {
            dstring string = DSTRING_INIT;
            lList *sub_host_list = NULL;
            lList *sub_hgroup_list = NULL;

            hgroup_find_all_references(hgroup, answer_list, hgroup_list, 
                                       &sub_host_list, &sub_hgroup_list);
            href_list_make_uniq(sub_host_list, answer_list);
            href_list_append_to_dstring(sub_host_list, &string);
            if (sge_dstring_get_string(&string)) {
               printf("%s\n", sge_dstring_get_string(&string));
            }
            sge_dstring_free(&string);
            lFreeList(&sub_host_list);
            lFreeList(&sub_hgroup_list);
         }
      } else {
         answer_list_add_sprintf(answer_list, STATUS_ERROR1,
                                 ANSWER_QUALITY_ERROR, MSG_HGROUP_NOTEXIST_S, name); 
         ret = false;
      }

      lFreeList(&hgroup_list);
      lFreeList(&alp);
   }
   DRETURN(ret);
}
