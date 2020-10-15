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
#include "uti/sge_log.h"
#include "uti/sge_string.h"

#include "cull/cull_list.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/msg_sgeobjlib.h"

/****** sgeobj/userprj/prj_list_locate() **********************************
*  NAME
*     prj_list_locate() -- Find project in list 
*
*  SYNOPSIS
*     lListElem* prj_list_locate(lList *lp, const char *name) 
*
*  FUNCTION
*     Find project in list. 
*
*  INPUTS
*     lList *lp        - PR_Type list 
*     const char *name - project name 
*
*  RESULT
*     lListElem* - NULL or element pointer
*******************************************************************************/
lListElem *prj_list_locate(const lList *lp, const char *name) 
{
   lListElem *ep = NULL;

   DENTER(BASIS_LAYER, "prj_list_locate");

   ep = lGetElemStr(lp, PR_name, name);

   DRETURN(ep);
}

/****** sgeobj/userprj/user_list_locate() **********************************
*  NAME
*     user_list_locate() -- Find user in list 
*
*  SYNOPSIS
*     lListElem* user_list_locate(lList *lp, const char *name) 
*
*  FUNCTION
*     Find user in list. 
*
*  INPUTS
*     lList *lp        - UU_Type list 
*     const char *name - user name 
*
*  RESULT
*     lListElem* - NULL or element pointer
*******************************************************************************/
lListElem *user_list_locate(const lList *lp, const char *name) 
{
   lListElem *ep = NULL;

   DENTER(BASIS_LAYER, "user_list_locate");

   ep = lGetElemStr(lp, UU_name, name);

   DRETURN(ep);
}


/****** sgeobj/userprj/prj_list_append_to_dstring() **********************************
*  NAME
*     prj_list_append_to_dstring() -- append prj from list to dstring
*
*  SYNOPSIS
*     const char* prj_list_append_to_dstring(lList *lp, dstring *string) 
*
*  FUNCTION
*     Append all projects in list lp to dstring string.
*
*  INPUTS
*     lList *lp        - PR_Type list 
*     dstring *string  - dstring to append to
*
*  RESULT
*     const char* - NULL or resulting string of dstring 
*******************************************************************************/
const char *prj_list_append_to_dstring(const lList *this_list, dstring *string)
{
   const char *ret = NULL;

   DENTER(BASIS_LAYER, "prj_list_append_to_dstring");
   if (string != NULL) {
      lListElem *elem = NULL;
      bool printed = false;

      for_each(elem, this_list) {
         sge_dstring_append(string, lGetString(elem, PR_name));
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
   DEXIT;
   return ret;
}

bool
prj_list_do_all_exist(const lList *this_list, lList **answer_list,
                      const lList *prj_list)
{
   bool ret = true;
   lListElem *prj = NULL;

   DENTER(TOP_LAYER, "prj_list_do_all_exist");
   for_each(prj, prj_list) {
      const char *name = lGetString(prj, PR_name);

      if (prj_list_locate(this_list, name) == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EEXIST,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_CQUEUE_UNKNOWNPROJECT_S, name);
         DTRACE;
         ret = false;
         break;
      }
   }
   DEXIT;
   return ret;
}

/***************************************************
 Generate a Template for a user
 ***************************************************/
lListElem *getUserTemplate()
{
   lListElem *ep;

   DENTER(TOP_LAYER, "getUserTemplate");

   ep = lCreateElem(UU_Type);
   lSetString(ep, UU_name, "template");
   lSetString(ep, UU_default_project, NULL);
   lSetUlong(ep, UU_oticket, 0);
   lSetUlong(ep, UU_fshare, 0);
   lSetUlong(ep, UU_job_cnt, 0);
   lSetList(ep, UU_project, NULL);
   lSetList(ep, UU_usage, NULL);
   lSetList(ep, UU_long_term_usage, NULL);

   DEXIT;
   return ep;
}

/***************************************************
 Generate a Template for a user or project
 ***************************************************/
lListElem *getPrjTemplate()
{
   lListElem *ep;

   DENTER(TOP_LAYER, "getPrjTemplate");

   ep = lCreateElem(PR_Type);
   lSetString(ep, PR_name, "template");
   lSetUlong(ep, PR_oticket, 0);
   lSetUlong(ep, PR_fshare, 0);
   lSetUlong(ep, PR_job_cnt, 0);
   lSetList(ep, PR_project, NULL);
   lSetList(ep, PR_usage, NULL);
   lSetList(ep, PR_long_term_usage, NULL);
   lSetList(ep, PR_acl, NULL);
   lSetList(ep, PR_xacl, NULL);

   DEXIT;
   return ep;
}

