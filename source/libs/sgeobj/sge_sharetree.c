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

#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"

#include "cull/cull_list.h"

#include "sched/sge_support.h"

#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_answer.h"

#include "msg_common.h"


/************************************************************************
   id_sharetree - set the sharetree node id
************************************************************************/
bool id_sharetree(lList **alpp, lListElem *ep, int id, int *ret_id)
{
   lListElem *cep = NULL;
   int my_id = id;

   DENTER(TOP_LAYER, "id_sharetree");

   if (ep == NULL) {
      answer_list_add(alpp, MSG_OBJ_NOSTREEELEM, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(false);
   }
   
   lSetUlong(ep, STN_id, my_id++);

   /* handle the children */
   for_each(cep, lGetList(ep, STN_children)) {     
      if (false == id_sharetree(NULL, cep, my_id, &my_id)) {
         DRETURN(false);
      }
   }

   if (ret_id) {
      *ret_id = my_id;
   }   

   DRETURN(true);
}  

/************************************************************************
  show_sharetree

  display a tree representation of sharetree 

 ************************************************************************/
int show_sharetree(
lListElem *ep,
char *indent 
) {
   lListElem *cep;
   FILE *fp = stdout;
   static int level = 0;
   int i;

   DENTER(TOP_LAYER, "show_sharetree");

   if (!ep) {
      DEXIT;
      return -1;
   }

   for (i=0;i<level;i++)
      fprintf(fp, "%s", indent ? indent : "");
   fprintf(fp, "%s="sge_u32"\n", lGetString(ep, STN_name), 
            lGetUlong(ep, STN_shares));
   for_each(cep, lGetList(ep, STN_children)) {
      level++;
      show_sharetree(cep, "   ");
      level--;
   }   

   DEXIT;
   return 0;
}

/************************************************************************
  show_sharetree_path

  display a path representation of sharetree 

 ************************************************************************/
int show_sharetree_path(
lListElem *root,
const char *path 
) {
   lListElem *cep;
   lListElem *node;
   FILE *fp = stdout;
   ancestors_t ancestors;
   int i;
   dstring sb = DSTRING_INIT;
 
   DENTER(TOP_LAYER, "show_sharetree_path");
 
   if (!root) {
      DEXIT;
      return 1;
   }
 
   memset(&ancestors, 0, sizeof(ancestors));
   if ( !strcmp(path, "/") || !strcasecmp(path, "Root") ) {
      node = root;
   } else {
      node = search_named_node_path(root, path, &ancestors);
   }
 
   if (node) {
      for(i=0; i<ancestors.depth; i++)
         fprintf(fp, "/%s", lGetString(ancestors.nodes[i], STN_name));
      if (!strcmp(path, "/") || !strcasecmp(path, "Root") )
         fprintf(fp, "/="sge_u32"\n", lGetUlong(node, STN_shares));
      else
         fprintf(fp, "="sge_u32"\n", lGetUlong(node, STN_shares));
      free_ancestors(&ancestors);
      for_each(cep, lGetList(node, STN_children)) {

         if (!strcmp(path, "/") || !strcasecmp(path, "Root") )
            sge_dstring_sprintf(&sb, "/%s", lGetString(cep, STN_name));
         else
            sge_dstring_sprintf(&sb, "%s/%s", path,
                                 lGetString(cep, STN_name));
         show_sharetree_path(root, sge_dstring_get_string(&sb));
      }
   }
   else {
      fprintf(stderr, MSG_TREE_UNABLETOLACATEXINSHARETREE_S, path);
      fprintf(stderr, "\n");
      return 1;
   }
 
   sge_dstring_free(&sb);
   DEXIT;
   return 0;
}                                                                               

/***************************************************
 Generate a Template for a sharetreenode
 ***************************************************/
lListElem *getSNTemplate(void)
{
   lListElem *ep;

   DENTER(TOP_LAYER, "getSNTemplate");

   ep = lCreateElem(STN_Type);
   lSetString(ep, STN_name, "template");
   lSetUlong(ep, STN_type, 0);
   lSetUlong(ep, STN_id, 0);
   lSetUlong(ep, STN_shares, 0);
   lSetList(ep, STN_children, NULL);

   DEXIT;
   return ep;
}

/********************************************************
 Search for a share tree node with a given name in a
 share tree
 ********************************************************/
lListElem *search_named_node( lListElem *ep,  /* root of the tree */
                              const char *name )
{
   lListElem *cep, *fep;
   static int sn_children_pos = -1;
   static int sn_name_pos = -1;

   DENTER(TOP_LAYER, "search_named_node");

   if (!ep || !name) {
      DEXIT;
      return NULL;
   }

   if (sn_name_pos == -1) {
      sn_children_pos = lGetPosViaElem(ep, STN_children, SGE_NO_ABORT);
      sn_name_pos = lGetPosViaElem(ep, STN_name, SGE_NO_ABORT);
   }

   if (strcmp(lGetPosString(ep, sn_name_pos), name) == 0) {
      DEXIT;
      return ep;
   }

   for_each(cep, lGetPosList(ep, sn_children_pos)) {
      if ((fep = search_named_node(cep, name))) {
         DEXIT;
         return fep;
      }
   }
      
   DEXIT;
   return NULL;
}


/********************************************************
 Free internals of ancestors structure
 ********************************************************/
void free_ancestors( ancestors_t *ancestors )
{
   if (ancestors && ancestors->nodes) {
      sge_free(&(ancestors->nodes));
   }
}


/********************************************************
 Search for a share tree node with a given path in a
 share tree
 ********************************************************/

static lListElem *
search_by_path( lListElem *ep,  /* root of the [sub]tree */
                const char *name,
                const char *path,
                int delim,
                ancestors_t *ancestors,
                int depth )
{
   lList *children;
   lListElem *ret = NULL, *child;
   char *buf=NULL, *bufp;

   if (name == NULL)
      delim = '.';

   if (name == NULL || !strcmp(name, "*") ||
       !strcmp(name, lGetString(ep, STN_name))) {
      if (*path == 0) {
         if (name) {
            ret = ep;
            if (ancestors && depth > 0) {
               ancestors->depth = depth;
               ancestors->nodes =
                     (lListElem **)malloc(depth * sizeof(lListElem *));
               ancestors->nodes[depth-1] = ep;
            }
         }
         return ret;
      }

      /* get next component from path */

      bufp = buf = (char *)malloc(strlen(path)+1);
      if (*path == '.' || *path == '/')
         delim = *path++;
      while (*path && *path != '.' && *path != '/')
         *bufp++ = *path++;
      *bufp = 0;
      name = buf;
   } else if (delim == '/')
      return NULL;

   if ((children = lGetList(ep, STN_children)))
      for(child=lFirst(children); child && !ret; child = child->next)
         ret = search_by_path(child, name, path, delim, ancestors, depth+1);

   if (ret && ancestors && ancestors->nodes && depth > 0)
      ancestors->nodes[depth-1] = ep;
   if (buf) {
      sge_free(&buf);
   }
   return ret;
}


/********************************************************
 Search for a share tree node with a given path in a
 share tree
 ********************************************************/
lListElem *
search_named_node_path( lListElem *ep,  /* root of the tree */
                        const char *path,
                        ancestors_t *ancestors )
{
   return search_by_path(ep, NULL, path, 0, ancestors, 0);
}


/********************************************************
 Search for a share tree node with a given name in a
 share tree returning an array of ancestor nodes. The
 array is contained in the ancestors_t structure which
 consist of the depth and a dynamically allocated array
 of lListElem pointers for each node.  The nodes are
 ordered from the root node to the found node. The 
 caller is reponsible for freeing the nodes array.
 ********************************************************/

#ifdef notdef

lListElem *search_ancestor_list( lListElem *ep,  /* root of the tree */
                                 char *name,
                                 ancestors_t *ancestors )
{
   if (ancestors)
      return search_ancestors(ep, name, ancestors, 1);
   else
      return search_named_node(ep, name);
}

#endif

lListElem *
search_ancestors( lListElem *ep,
                  char *name,
                  ancestors_t *ancestors,
                  int depth )
{
   lListElem *cep, *fep;
   static int sn_children_pos = -1;
   static int sn_name_pos = -1;

   DENTER(TOP_LAYER, "search_named_node");

   if (!ep || !name) {
      DEXIT;
      return NULL;
   }

   if (sn_name_pos == -1) {
      sn_children_pos = lGetPosViaElem(ep, STN_children, SGE_NO_ABORT);
      sn_name_pos = lGetPosViaElem(ep, STN_name, SGE_NO_ABORT);
   }
   if (strcmp(lGetPosString(ep, sn_name_pos), name) == 0) {
      ancestors->depth = depth;
      ancestors->nodes = (lListElem **)malloc(depth * sizeof(lListElem *));
      ancestors->nodes[depth-1] = ep;
      DEXIT;
      return ep;
   }

   for_each(cep, lGetPosList(ep, sn_children_pos)) {
      if ((fep = search_ancestors(cep, name, ancestors, depth+1))) {
         ancestors->nodes[depth-1] = ep;
         DEXIT;
         return fep;
      }
   }
      
   DEXIT;
   return NULL;
}

/****** sge_search_unspecified_node() ******************************************
*  NAME
*     sge_search_unspecified_node() -- search for a node which is not specified
*
*  SYNOPSIS
*     static lListElem *sge_search_unspecified_node(lListElem *ep)
*
*
*  FUNCTION
*     The function walks through the sharetree looking for the first node which
*     has no name.  A node with no name means that it was created as a result of
*     a dangling or circular child reference.
*
*  INPUTS
*     ep - root of the tree
*
*  RESULT
*     the first node which has no name or NULL if all nodes have names
******************************************************************************/
lListElem *sge_search_unspecified_node(lListElem *ep)
{
   lListElem *cep = NULL, *ret = NULL;

   DENTER(TOP_LAYER, "sge_search_unspecified_node");

   if (ep == NULL) {
      DEXIT;
      return NULL;
   }

   for_each(cep, lGetList(ep, STN_children)) {
      if ((ret = sge_search_unspecified_node(cep))) {
         DEXIT;
         return ret;
      }   
   }

   if (lGetString(ep, STN_name) == NULL) {
      DEXIT;
      return ep;         /* no name filled in -> unspecified */
   }
   
   DEXIT;
   return NULL;
}
