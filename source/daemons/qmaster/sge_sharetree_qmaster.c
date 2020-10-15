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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
/*
   This is the module for handling the SGE sharetree
   We save the sharetree to <spool>/qmaster/sharetree
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/cull_parse_util.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_userprj.h"

#include "spool/sge_spooling.h"

#include "sge.h"
#include "sge_persistence_qmaster.h"
#include "sge_event_master.h"
#include "sge_sharetree_qmaster.h"
#include "sge_userprj_qmaster.h"

#include "msg_common.h"
#include "msg_qmaster.h"

/************************************************************
  sge_add_sharetree - Master code

  Add the sharetree
 ************************************************************/
int sge_add_sharetree(
sge_gdi_ctx_class_t *ctx,
lListElem *ep,
lList **lpp,    /* list to change */
lList **alpp,
char *ruser,
char *rhost 
) {
   int ret;

   DENTER(TOP_LAYER, "sge_add_sharetree");
   ret = sge_mod_sharetree(ctx, ep, lpp, alpp, ruser, rhost);
   DEXIT;
   return ret;
}

/************************************************************
  sge_mod_sharetree - Master code

  Modify a share tree
  lListElem *ep,   This is the new share tree 
  lList **lpp,     list to change 
 ************************************************************/
int sge_mod_sharetree(sge_gdi_ctx_class_t *ctx,
                      lListElem *ep, lList **lpp, lList **alpp, 
                      char *ruser, char *rhost ) 
{
   int ret;
   int prev_version;
   int adding; 
   lList *found = NULL;
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "sge_mod_sharetree");

   /* do some checks */
   if (!ep || !ruser || !rhost) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   ret = check_sharetree(alpp, ep, *object_base[SGE_TYPE_USER].list, 
                         *object_base[SGE_TYPE_PROJECT].list, 
                         NULL, &found);
   lFreeList(&found);
   if (ret) {
      /* alpp gets filled by check_sharetree() */
      DRETURN(STATUS_EUNKNOWN);
   }

   /* check for presence of sharetree list and create if neccesary */
   if (!*lpp) {
      prev_version = 0;
      *lpp = lCreateList("sharetree list", STN_Type);
      adding = 1;
   } else if (lGetNumberOfElem(*lpp) == 0) {
      prev_version = 0;
      adding = 1;
   } else {
      lListElem *first = lFirst(*lpp);
      /* real action: change user or project
         We simply replace the old element with the new one. If there is no
         old we simle add the new. */
     
      prev_version = lGetUlong(lFirst(*lpp), STN_version); 
      lRemoveElem(*lpp, &first);
      adding = 0;
   }

   lSetUlong(ep, STN_version, prev_version+1);

   id_sharetree(alpp, ep, 0, NULL);

   /* now insert new element */
   lAppendElem(*lpp, lCopyElem(ep));
  
   /* write sharetree to file */
   if (!sge_event_spool(ctx,
                        alpp, 0, sgeE_NEW_SHARETREE,
                        0, 0, NULL, NULL, NULL,
                        ep, NULL, NULL, true, true)) {

      /* answer list gets filled in sge_event_spool() */
      DEXIT;
      return ret;
   }

   if (adding) {
      INFO((SGE_EVENT, MSG_STREE_ADDSTREE_SSII, 
         ruser, rhost, lGetNumberOfNodes(ep, NULL, STN_children), lGetNumberOfLeafs(ep, NULL, STN_children)));
   } else {
      INFO((SGE_EVENT, MSG_STREE_MODSTREE_SSII, 
         ruser, rhost, lGetNumberOfNodes(ep, NULL, STN_children), lGetNumberOfLeafs(ep, NULL, STN_children)));
   }

   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);

   DEXIT;
   return STATUS_OK;
}

/************************************************************
  sge_del_sharetree - Master code

  Delete the sharetree
 ************************************************************/
int sge_del_sharetree(
sge_gdi_ctx_class_t *ctx,
lList **lpp,    /* list to change */
lList **alpp,
char *ruser,
char *rhost 
) {
   DENTER(TOP_LAYER, "sge_del_sharetree");

   if (!*lpp || !lFirst(*lpp)) {
      ERROR((SGE_EVENT, MSG_SGETEXT_DOESNOTEXIST_S, MSG_OBJ_SHARETREE));
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EEXIST;
   }

   sge_event_spool(ctx,
                   alpp, 0, sgeE_NEW_SHARETREE, 
                   0, 0, NULL, NULL, NULL,
                   NULL, NULL, NULL, true, true);

   lFreeList(lpp);

   INFO((SGE_EVENT, MSG_SGETEXT_REMOVEDLIST_SSS, ruser, rhost, MSG_OBJ_SHARETREE));
   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);

   DEXIT;
   return STATUS_OK;
}

/********************************************************
 ensure 
 - all nodes have a unique path in share tree
 - a project is not referenced more than once in share tree
 - a user appears only once in a project sub-tree
 - a user appears only once outside of a project sub-tree
 - a user does not appear as a non-leaf node
 - all leaf nodes in a project sub-tree reference a known
   user object or the reserved name "default"
 - there are no sub-projects within a project sub-tree
 - all leaf nodes not in a project sub-tree reference a known
   user or project object
 - all user leaf nodes in a project sub-tree have access
   to the project

 ********************************************************/
int check_sharetree(
lList **alpp,
lListElem *node,
lList *user_list,
lList *project_list,
lListElem *project,
lList **found  /* tmp list that contains one entry for each found u/p */
) {
   lList *children;
   lListElem *child, *remaining;
   lList *save_found = NULL;
   const char *name = lGetString(node, STN_name);
   lListElem *pep;

   DENTER(TOP_LAYER, "check_sharetree");

   /* Check for dangling or circular references. */
   if (name == NULL) {
      ERROR((SGE_EVENT, MSG_STREE_NOVALIDNODEREF_U,
             sge_u32c(lGetUlong(node, STN_id))));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }
   
   if ((children=lGetList(node, STN_children))) {

      /* not a leaf node */

      /* check if this is a project node */
      if ((pep=prj_list_locate(project_list, name))) {

         /* check for sub-projects (not allowed) */
         if (project) {
            ERROR((SGE_EVENT, MSG_STREE_PRJINPTJSUBTREE_SS, name, lGetString(project, PR_name)));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
         }

         /* check for projects appearing more than once */
         if (lGetElemStr(*found, STN_name, name)) {
            ERROR((SGE_EVENT, MSG_STREE_PRJTWICE_S, name));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
         }

         /* register that this project was found */
         lAddElemStr(found, STN_name, name, STN_Type);

         /* set up for project sub-tree recursion */
         project = pep;
         save_found = *found;
         *found = NULL;

         /* check for user appearing as non-leaf node */
      } else if (user_list_locate(user_list, name)) {
            ERROR((SGE_EVENT, MSG_STREE_USERNONLEAF_S, name));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
      }

      for_each(child, children) {
         /* ensure pathes are identically inside share tree */ 
         for (remaining=lNext(child); remaining; remaining=lNext(remaining)) {
            const char *cn = lGetString(child, STN_name);
            const char *rn = lGetString(remaining, STN_name);
            if (cn == NULL || rn == NULL) {
               ERROR((SGE_EVENT, MSG_GDI_KEYSTR_NULL_S, "STN_name"));
               answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               /* restore old found list */
               if (save_found) {
                  lFreeList(found);
                  *found = save_found;
               }
               DEXIT;
               return -1;
            }
            if (!strcmp(cn, rn)) {
               ERROR((SGE_EVENT, MSG_SGETEXT_FOUND_UP_TWICE_SS, 
                  cn, lGetString(node, STN_name)));
               answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               /* restore old found list */
               if (save_found) {
                  lFreeList(found);
                  *found = save_found;
               }
               DEXIT;
               return -1;
            }  
         }

         if (check_sharetree(alpp, child, user_list, project_list, project,
               found)) {
            /* restore old found list */
            if (save_found) {
               lFreeList(found);
               *found = save_found;
            }
            DEXIT;
            return -1;
         }
      }

      /* restore old found list */
      if (save_found) {
         lFreeList(found);
         *found = save_found;
      }
   } else {
      /* a leaf node */

      /* check if this is a project node */
      if (prj_list_locate(project_list, name)) {
         lSetUlong(node, STN_type, STT_PROJECT);
      }   

      if (project) {

         /* project set means this is a project sub-tree */
         
         /* check for sub-projects */
         if (prj_list_locate(project_list, name)) {
            ERROR((SGE_EVENT, MSG_STREE_PRJINPTJSUBTREE_SS, name, lGetString(project, PR_name)));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
         }

         /* leaf nodes of project sub-trees must be users */
         if (!user_list_locate(user_list, name) &&
             strcmp(name, "default")) {
            ERROR((SGE_EVENT, MSG_SGETEXT_UNKNOWN_SHARE_TREE_REF_TO_SS, MSG_OBJ_USER, name));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
         }

         /* make sure this user is in the project sub-tree once */
         if (lGetElemStr(*found, STN_name, name)) {
            ERROR((SGE_EVENT, MSG_STREE_USERTWICEINPRJSUBTREE_SS, name, lGetString(project, PR_name)));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
         }

#if 0  /* commented out because user may have access based on group ID */

         /* make sure this user has access to the project */

         if (strcmp(name, "default") &&
             !sge_has_access_(name, NULL, lGetList(project, PR_acl),
                  lGetList(project, PR_xacl), *object_type_get_master_list(SGE_TYPE_USERSET))) {
            ERROR((SGE_EVENT, MSG_STREE_USERTNOACCESS2PRJ_SS, name, lGetString(project, PR_name)));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
         }
#endif

      } else {
         const char *objname = MSG_OBJ_USER;

         /* non project sub-tree leaf nodes must be a user or a project */
         if (user_list_locate(user_list, name) == NULL &&
             strcmp(name, "default") != 0) {

             objname=MSG_JOB_PROJECT;

             if (prj_list_locate(project_list, name) == NULL) {
               ERROR((SGE_EVENT, MSG_SGETEXT_UNKNOWN_SHARE_TREE_REF_TO_SS, 
                        MSG_OBJ_USERPRJ, name));
               answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               DEXIT;
               return -1;
            }
         }

         /* make sure this user or project is in the non-project sub-tree
            portion of the share tree only once */
         if (lGetElemStr(*found, STN_name, name)) {
            ERROR((SGE_EVENT, MSG_STREE_USERPRJTWICE_SS, objname, name));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DEXIT;
            return -1;
         }

      }

      /* register that this was found */
      lAddElemStr(found, STN_name, name, STN_Type);
   }
   
   DEXIT;
   return 0;
}
 
/* ----------------------------------------------

   dst  is the share tree to be updated
   src  is a reduced share tree with at least

      STN_name
      STN_version
      STN_m_share
      STN_last_actual_proportion
      STN_adjusted_current_proportion

   Returns 0 on success, -1 on error.
*/
int update_sharetree(lList *dst, lList *src)
{
   static int depth = 0;
   lListElem *dnode, *snode;
#ifdef notdef
   const char *d_name;
#endif
   const char *s_name;

   DENTER(TOP_LAYER, "update_sharetree");

   dnode = lFirst(dst);
   snode = lFirst(src);
   if ((dnode!=NULL) != (snode!=NULL)) {
      ERROR((SGE_EVENT, MSG_STREE_QMASTERSORCETREE_SS,
            snode?"":MSG_STREE_NOPLUSSPACE, dnode?"":MSG_STREE_NOPLUSSPACE));
      depth = 0;
      DRETURN(-1);
   }

   /* root node ? */
   if (depth++ == 0) {
      int dv, sv;

      /* ensure STN_version of both root nodes are identically */

      if (dnode && (dv=lGetUlong(dnode, STN_version)) != (sv=lGetUlong(snode, STN_version))) {
         ERROR((SGE_EVENT, MSG_STREE_VERSIONMISMATCH_II, dv, sv));
         depth = 0;
         DRETURN(-1);
      }
   }

   /* seek matching node for each of the masters node on that level */
#ifdef notdef
   for_each (dnode, dst) {
      d_name = lGetString(dnode, STN_name);
      if (!(snode = lGetElemStr(src, STN_name, d_name))) {
#endif
   for_each (snode, src) {
      s_name = lGetString(snode, STN_name);
      if (!(dnode = lGetElemStr(dst, STN_name, s_name))) {
         ERROR((SGE_EVENT, MSG_STREE_MISSINGNODE_S, s_name));
         depth = 0;
         DRETURN(-1);
      }

      /* update fields */
      lSetDouble(dnode, STN_m_share, lGetDouble(snode, STN_m_share));
      lSetDouble(dnode, STN_last_actual_proportion,
         lGetDouble(snode, STN_last_actual_proportion));
      lSetDouble(dnode, STN_adjusted_current_proportion,
         lGetDouble(snode, STN_adjusted_current_proportion));
      lSetUlong(dnode, STN_job_ref_count, lGetUlong(snode, STN_job_ref_count));

      /* enter recursion */
      if (update_sharetree(lGetList(dnode, STN_children), lGetList(snode, STN_children)) != 0) {
         DRETURN(-1);
      }
   }

   depth--;
   DRETURN(0);
}

/* seek user/prj node (depends on node_type STT_USER|STT_PROJECT) in actual share tree */
lListElem *getNode(
lList *share_tree,
const char *name,
int node_type,
int recurse 
) {
   lListElem *tmp, *node;
  
   DENTER(TOP_LAYER, "getNode");

   if (!share_tree || !lFirst(share_tree)) {
      DEXIT;
      return NULL;
   }
   
#ifdef notdef
   if ( !recurse && lGetUlong(lFirst(share_tree), STN_type)!=node_type ) {
      DEXIT;
      return NULL;
   } 
#endif

   for_each (node, share_tree) {
      if (!strcmp(name, lGetString(node, STN_name))) {
         DEXIT;
         return node;
      }
      if ((tmp=getNode(lGetList(node, STN_children), name, node_type, 1))) {
         DEXIT;
         return tmp;
      }
   }

   DEXIT;
   return NULL;
}

