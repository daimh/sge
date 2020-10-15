#ifndef __SGE_SHARETREE_H 
#define __SGE_SHARETREE_H 
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

#include "sgeobj/sge_sharetree_STN_L.h"

/*
 *  * This is the list type we use to hold the 
 *   * nodes of a share tree.
 *    */
#define STT_USER    0
#define STT_PROJECT 1

typedef struct {
   int depth;
   lListElem **nodes;
} ancestors_t;

/************************************************************************
   id_sharetree - set the sharetree node id
************************************************************************/
bool id_sharetree(lList **alpp, lListElem *ep, int id, int *ret_id);
int show_sharetree_path(lListElem *root, const char *path);
int show_sharetree(lListElem *ep, char *indent);
lListElem *getSNTemplate(void);
lListElem *search_named_node ( lListElem *ep, const char *name );
lListElem *search_named_node_path ( lListElem *ep, const char *path, ancestors_t *ancestors );
void free_ancestors( ancestors_t *ancestors);
lListElem *sge_search_unspecified_node(lListElem *ep);
#ifdef notdef
lListElem *search_ancestor_list ( lListElem *ep, char *name, ancestors_t *ancestors );
#endif
lListElem *search_ancestors(lListElem *ep, char *name,
                                   ancestors_t *ancestors, int depth);

#endif /* __SGE_SHARETREE_H */
