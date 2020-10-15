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
#include "uti/sge_edit.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_cuser.h"
#include "sgeobj/sge_conf.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sge.h"
#include "sge_cuser_qconf.h"
#include "parse_qconf.h"
#include "msg_common.h"
#include "msg_qconf.h"


#ifndef __SGE_NO_USERMAPPING__

bool 
cuser_add_del_mod_via_gdi(sge_gdi_ctx_class_t *ctx,
                          lListElem *this_elem, lList **answer_list,
                          u_long32 gdi_command)
{
   bool ret = false;

   DENTER(TOP_LAYER, "cuser_add_del_mod_via_gdi");

   if (this_elem != NULL) {
      lList *cuser_list = NULL;
      lList *gdi_answer_list = NULL;

      cuser_list = lCreateList("", CU_Type);
      lAppendElem(cuser_list, this_elem);
      gdi_answer_list = ctx->gdi(ctx, SGE_USER_MAPPING_LIST, gdi_command,
                                &cuser_list, NULL, NULL);
      answer_list_replace(answer_list, &gdi_answer_list);
   }

   DRETURN(ret);
}

lListElem *cuser_get_via_gdi(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name) 
{
   lListElem *ret = NULL;

   DENTER(TOP_LAYER, "cuser_get_via_gdi");

   if (name != NULL) {
      lList *gdi_answer_list = NULL;
      lEnumeration *what = NULL;
      lCondition *where = NULL;
      lList *cuser_list = NULL;

      what = lWhat("%T(ALL)", CU_Type);
      where = lWhere("%T(%I==%s)", CU_Type, CU_name, name);
      gdi_answer_list = ctx->gdi(ctx, SGE_USER_MAPPING_LIST, SGE_GDI_GET, 
                                &cuser_list, where, what);
      lFreeWhat(&what);
      lFreeWhere(&where);

      if (!answer_list_has_error(&gdi_answer_list)) {
         ret = lFirst(cuser_list);
      } else {
         answer_list_replace(answer_list, &gdi_answer_list);
      }
   } 

   DRETURN(ret);
}

bool cuser_provide_modify_context(sge_gdi_ctx_class_t *ctx, lListElem **this_elem, lList **answer_list)
{
   bool ret = false;
   int status = 0;
   uid_t uid = ctx->get_uid(ctx);
   gid_t gid = ctx->get_gid(ctx);
   
   DENTER(TOP_LAYER, "cuser_provide_modify_context");

   if (this_elem != NULL && *this_elem) {
      char *filename = write_ume(2, 1, *this_elem); 
 
      status = sge_edit(filename, uid, gid);
      if (status >= 0) {
         lListElem *cuser;

         cuser = cull_read_in_ume(NULL, filename, 1, 0, 0, NULL);
         if (cuser != NULL) {
            lFreeElem(this_elem);
            *this_elem = cuser; 
            ret = true;
         } else {
            answer_list_add(answer_list, MSG_FILE_ERRORREADINGINFILE,
                            STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         }
      } else {
         answer_list_add(answer_list, MSG_PARSE_EDITFAILED,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
      unlink(filename);
   } 

   DRETURN(ret);
}

bool cuser_add(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name) 
{
   bool ret = true;

   DENTER(TOP_LAYER, "cuser_add");
   if (name != NULL) {
      lListElem *cuser = cuser_create(answer_list, name, NULL);

      if (cuser == NULL) {
         ret = false;
      }
      if (ret) {
         ret &= cuser_provide_modify_context(ctx, &cuser, answer_list);
      }
      if (ret) {
         ret &= cuser_add_del_mod_via_gdi(ctx, cuser, answer_list, SGE_GDI_ADD); 
      } 
   }  
  
   DRETURN(ret); 
}

bool cuser_add_from_file(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *filename) 
{
   bool ret = true;

   DENTER(TOP_LAYER, "cuser_add_from_file");
   if (filename != NULL) {
      lListElem *cuser;

      cuser = cull_read_in_ume(NULL, filename, 1, 0, 0, NULL); 
      if (cuser == NULL) {
         ret = false;
      }
      if (ret) {
         ret &= cuser_add_del_mod_via_gdi(ctx, cuser, answer_list, SGE_GDI_ADD); 
      } 
   }  
  
   DRETURN(ret); 
}

bool cuser_modify(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "cuser_modify");
   if (name != NULL) {
      lListElem *cuser = cuser_get_via_gdi(ctx, answer_list, name);

      if (cuser == NULL) {
         sprintf(SGE_EVENT, MSG_CUSER_DOESNOTEXIST_S, name);
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         ret = false;
      }
      if (ret) {
         ret &= cuser_provide_modify_context(ctx, &cuser, answer_list);
      }
      if (ret) {
         ret &= cuser_add_del_mod_via_gdi(ctx, cuser, answer_list, SGE_GDI_MOD);
      }
      if (cuser) {
         lFreeElem(&cuser);
      }
   }

   DRETURN(ret);
}

bool cuser_modify_from_file(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *filename)
{
   bool ret = true;

   DENTER(TOP_LAYER, "cuser_modify_from_file");

   if (filename != NULL) {
      lListElem *cuser;

      cuser = cull_read_in_ume(NULL, filename, 1, 0, 0, NULL); 
      if (cuser == NULL) {
         sprintf(SGE_EVENT, MSG_CUSER_FILENOTCORRECT_S, filename);
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         ret = false;
      }
      if (ret) {
         ret &= cuser_add_del_mod_via_gdi(ctx, cuser, answer_list, SGE_GDI_MOD);
      }
      if (cuser != NULL) {
         lFreeElem(&cuser);
      }
   }

   DRETURN(ret);
}

bool cuser_delete(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "cuser_delete");

   if (name != NULL) {
      lListElem *cuser = cuser_create(answer_list, name, NULL); 
   
      if (cuser != NULL) {
         ret &= cuser_add_del_mod_via_gdi(ctx, cuser, answer_list, SGE_GDI_DEL); 
      }
   }

   DRETURN(ret);
}

bool cuser_show(sge_gdi_ctx_class_t *ctx, lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "cuser_show");
   if (name != NULL) {
      lListElem *cuser = cuser_get_via_gdi(ctx, answer_list, name); 
   
      if (cuser != NULL) {
         write_ume(0, 0, cuser);
         lFreeElem(&cuser);
      } else {
         sprintf(SGE_EVENT, MSG_CUSER_DOESNOTEXIST_S, name);
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR); 
         ret = false;
      }
   }

   DRETURN(ret);
}

#endif
