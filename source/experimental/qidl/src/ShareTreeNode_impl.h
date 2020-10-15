#ifndef __SHARETREENODE_IMPL_H
#define __SHARETREENODE_IMPL_H
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
// ShareTreeNode_impl.h
// header for for ShareTreeNode implementation


#include "ShareTreeNode_implementation.h"
#include <list>

/*typedef enum _NodeType {
   DETERMINE_TYPE = 0,
   BRANCH_TYPE,
   LEAF_TYPE
} NodeType;*/

class Sge_ShareTreeNode_impl : public virtual Sge_ShareTreeNode_implementation {
   public:
      Sge_ShareTreeNode_impl(const char* _name, const time_t& tm, CORBA_ORB_var o, Sge_ShareTreeNode_impl* pnode = NULL);
      Sge_ShareTreeNode_impl(const char* _name, lListElem* root, CORBA_ORB_var o, Sge_ShareTreeNode_impl* pnode = NULL);
      virtual ~Sge_ShareTreeNode_impl();

      // idl
      virtual Sge_sge_string           get_name(CORBA_Context* ctx);
      virtual Sge_ShareTreeNodeSeq*    get_children(CORBA_Context* ctx);
      
      virtual Sge_ShareTreeNode* newNode(const char* name, CORBA_ULong shares, CORBA_Context_ptr);
      virtual Sge_ShareTreeNode* newLeaf(const char* name, CORBA_ULong shares, CORBA_Context_ptr);

      //needed for getSelf()
      void        storePath(list<char*>& pathlist);
      void        getPath(list<char*>& pathlist);
      lListElem*  findNodeByPath(lListElem* ep, list<char*>::iterator it);
      //lListElem*  findNodeByName(lListElem* ep, char* key);

      void     set_create_share(CORBA_ULong share=0);

      //remove specified child name from children list
      void removeChild(Sge_ShareTreeNode_impl*const & child);
      
      void addBranch          (lListElem* ep);
      void delShareTreeNodes  (lListElem* ep);
      void addShareTreeNodes  (lListElem* ep);

      virtual void      add(CORBA_Context* ctx);
      virtual void      destroy(CORBA_Context* ctx);

      // non-idl
      virtual void      changed(lListElem* _newSelf);
      
         
   private:
      virtual lListElem* getSelf();
      list<Sge_ShareTreeNode_impl*> children;
      Sge_ShareTreeNode_impl*       parent;
      list<char*>                      path;
      CORBA_ULong                      create_share;
   
      friend class Sge_Master_impl;
};

#endif /* __SHARETREENODE_IMPL_H */
