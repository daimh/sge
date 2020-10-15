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
// Codine_ShareTreeNode_impl.cpp
// implementation for ShareTreeNode object

#include <pthread.h>


#include "ShareTreeNode_impl.h"
#include "Master_impl.h"
#include "qidl_common.h"

extern "C" {
#include "cod_api.h"
#include "cod_answerL.h"
#include "cod_m_event.h"
#include "cod_share_tree_nodeL.h"
#include "codrmon.h"
#include "cod_log.h"
#include "qmaster.h"
#include "read_write_queue.h"
#include "cod_sharetree.h"
}

#ifdef HAVE_STD
using namespace std;
#endif

extern lList* Master_Sharetree_List;


Codine_ShareTreeNode_impl::Codine_ShareTreeNode_impl(const char* _name, const time_t& tm, CORBA_ORB_var o, Codine_ShareTreeNode_impl* pnode)
   : Codine_ShareTreeNode_implementation(_name, tm, o), parent(pnode), create_share(0) {
   QENTER("Codine_ShareTreeNode_impl::Codine_ShareTreeNode_impl(time)");
   DPRINTF(("Name: %s\n", _name));
   
   AUTO_LOCK_MASTER;
 
   lSetString(self, SN_name, (char*)_name);
}

Codine_ShareTreeNode_impl::Codine_ShareTreeNode_impl(const char* _name, lListElem* ep, CORBA_ORB_var o, Codine_ShareTreeNode_impl* pnode)
   : Codine_ShareTreeNode_implementation(_name, o), parent(pnode), create_share(0) {
   QENTER("Codine_ShareTreeNode_impl::Codine_ShareTreeNode_impl(lListElem*)");
   DPRINTF(("Name: %s\n", _name));
   
   AUTO_LOCK_MASTER;
 
   Codine_ShareTreeNode_impl* sn;
   lListElem* tep;
   lList* cullchildren = lGetList(ep, SN_children);
      
   for_each_cpp(tep,cullchildren) {
      sn = new Codine_ShareTreeNode_impl(lGetString(tep, SN_name), tep, orb, this);
      if (sn) {
         children.push_back(sn);
         orb->connect(sn);
      }
   }
}

   
Codine_ShareTreeNode_impl::~Codine_ShareTreeNode_impl() {
   QENTER("Codine_ShareTreeNode_impl::~Codine_ShareTreeNode_impl");
   DPRINTF(("Name: %s\n", (char*) key));
                  
   list<Codine_ShareTreeNode_impl*>::iterator  it;

   for(it=children.begin(); it != children.end(); ++it) {
      orb->disconnect(*it);
      CORBA_release(*it);
   }
   
   if(creation != 0)
      lFreeElem(&self);
}


void Codine_ShareTreeNode_impl::removeChild(Codine_ShareTreeNode_impl*const & child) {
   QENTER("Codine_ShareTreeNode_impl::removeChild");
   
   children.remove(child);
}


void Codine_ShareTreeNode_impl::set_create_share(CORBA_ULong share) {
   QENTER("Codine_ShareTreeNode_impl::set_create_share");
   create_share = share;
}


void Codine_ShareTreeNode_impl::delShareTreeNodes(lListElem* ep) {
   QENTER("Codine_ShareTreeNode_impl::delShareTreeNodes");
   
   lListElem* elem;
   int found = 0;
   
   lList* children = lGetList(ep, SN_children);
   Codine_ShareTreeNodeSeq_var chs = get_children(Codine_Master_impl::instance()->getMasterContext());

   for (int i=0; i < chs->length(); i++) {
      for_each_cpp(elem, children) {
         if(!strcmp(dynamic_cast<Codine_ShareTreeNode_impl*>((Codine_ShareTreeNode*)chs[i])->key,lGetString(elem,SN_name))){
            found = 1;
            dynamic_cast<Codine_ShareTreeNode_impl*>((Codine_ShareTreeNode*)chs[i])->delShareTreeNodes(elem);
         }
      }
               
      if (!found) {  //node has been deleted -> delete node
            orb->disconnect(chs[i]);
            CORBA_release(chs[i]);
            if (dynamic_cast<Codine_ShareTreeNode_impl*>((Codine_ShareTreeNode*)chs[i])->parent)
               dynamic_cast<Codine_ShareTreeNode_impl*>((Codine_ShareTreeNode*)chs[i])->parent->removeChild(dynamic_cast<Codine_ShareTreeNode_impl*>((Codine_ShareTreeNode*)chs[i])); 
      }
      found = 0;
   }
}


void Codine_ShareTreeNode_impl::addShareTreeNodes(lListElem* ep) {
   QENTER("Codine_ShareTreeNode::addShareTreeNodes"); 
   
   lListElem* elem;
   int found = 0;
   
   Codine_ShareTreeNodeSeq_var chs = get_children(Codine_Master_impl::instance()->getMasterContext());
   lList* children = lGetList(ep, SN_children);
   
   for_each_cpp(elem, children) {
      for (int i=0; i < chs->length(); i++) {
         if(!strcmp(dynamic_cast<Codine_ShareTreeNode_impl*>((Codine_ShareTreeNode*)chs[i])->key,lGetString(elem,SN_name))){
            found = 1;
            dynamic_cast<Codine_ShareTreeNode_impl*>((Codine_ShareTreeNode*)chs[i])->addShareTreeNodes(elem);
         }
      }
      
      if (!found)   //node has been added -> add new node
         addBranch(elem);
      
      found = 0;
   }
}


void Codine_ShareTreeNode_impl::addBranch(lListElem* ep) {
   QENTER("Codine_ShareTreeNode_impl::addBranch");
        
   lListElem* tep;
   lList* cullchildren = lGetList(ep, SN_children);
   
   Codine_ShareTreeNode_impl* sn = new Codine_ShareTreeNode_impl(lGetString(ep,SN_name), (time_t)0, orb, this);
       
   if (sn) {
      children.push_back(sn);
      orb->connect(sn);
      cout << lGetString(ep,SN_name) << " added to " << key << "'s children." << endl;
   }
   
   for_each_cpp(tep, cullchildren) {
      sn->addBranch(tep);
   }
}


// inherited from Codine_Object
void Codine_ShareTreeNode_impl::destroy(CORBA_Context* ctx) {
   QENTER("Codine_ShareTreeNode_impl::destroy");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   if(!getSelf()) {
      orb->disconnect(this);
      CORBA_release(this);
      throw Codine_ObjDestroyed();
   }

   // CORBA object only ?
   if(creation != 0) {
      orb->disconnect(this);
      // The CORBA object itself will be destroyed by
      // the master, some time later
      return;
   }

   // make api request
   lListPtr      lp;
   lListPtr      alp;

   lp = lCreateList("My ShareTreeNode List", SN_Type);
   lAppendElem(lp, lCopyElem(self));

   alp = cod_api(COD_SHARETREE_LIST, COD_API_DEL, &lp, NULL, NULL);
   throwErrors(alp);
}


lListElem* Codine_ShareTreeNode_impl::getSelf() {
   QENTER("Codine_ShareTreeNode_impl::getSelf");
   
   if(creation != 0)
      return self;

   if(newSelf)
      return self = newSelf;

   AUTO_LOCK_MASTER;
   
   getPath(path);
   list<char*>::iterator it = path.begin();
   self = findNodeByPath(lFirst(Master_Sharetree_List), it);
   
   if(!self) {  
      throw Codine_ObjDestroyed();
   }
    
   return self;
}


void Codine_ShareTreeNode_impl::getPath(list<char*>& pathlist) {
   QENTER("Codine_ShareTreeNode_impl::getPath");
   
   path.clear();
   storePath(pathlist);
}


void Codine_ShareTreeNode_impl::storePath(list<char*>& pathlist) {
   QENTER("Codine_ShareTreeNode_impl::storePath");
  
   if (parent)
   {
      parent->storePath(pathlist);
      pathlist.push_back(key);
   }
}


lListElem* Codine_ShareTreeNode_impl::findNodeByPath(lListElem* ep, list<char*>::iterator it)
{
   QENTER("Codine_ShareTreeNode_impl::findNodeByPath");
   
   lListElem *cep;
   lCondition* cp;
   
   if (!ep)
      return NULL;
      
   if (it == path.end())
      return ep;
      
   cp = lWhere("%T(%I==%s)", SN_Type, SN_name, (const char*)(*it));
   if (cep = lFindFirst(lGetList(ep, SN_children), cp)) {
      return findNodeByPath(cep, (++it));
   }
   
   return NULL;
}


/*lListElem* Codine_ShareTreeNode_impl::findNodeByName(lListElem* ep, char* key)
{
   QENTER("Codine_ShareTreeNode_impl::findNodeByName");
   
   lListElem *cep, *fep;

   if (!ep)
      return NULL;
   
   if (!strcmp(lGetString(ep, SN_name), key)) 
      return ep;
   
   for_each_cpp(cep, lGetList(ep, SN_children)) {
      if ((fep = findNodeByName(cep, key))) {
         return fep;
      }
   }
    
   return NULL;
}*/


void Codine_ShareTreeNode_impl::add(CORBA_Context* ctx) {
   QENTER("Codine_ShareTreeNode_impl::add");

   if(creation == 0)
      return;
   
   AUTO_LOCK_MASTER;
   qidl_authenticate(ctx);
   //-------------------------------------------------------------------------------------------------------------------------------
   lEnumeration *what;
   lList *lp=NULL, *alp=NULL, *erralp=NULL;
   lListElem *pnode, *ep, *nnode;
   
   /* get the sharetree .. */
   what = lWhat("%T(ALL)", SN_Type);
   alp = cod_api(COD_SHARETREE_LIST, COD_API_GET, &lp, NULL, what);
   what = lFreeWhat(what);
   alp = lFreeList(alp);
 
   ep = lFirst(lp);
   if (!ep) {
      ep = lAddElemStr(&lp, SN_name, "Root", SN_Type);
      if (ep) lSetUlong(ep, SN_shares, 1);
   }
         
   parent->getPath(parent->path);
   list<char*>::iterator it = parent->path.begin();
   pnode = parent->findNodeByPath(ep, it);

   if (pnode) {
      lList *childs = lGetList(pnode, SN_children);
      nnode = lAddElemStr(&childs, SN_name, key, SN_Type);
      if (nnode)
         lSetUlong(nnode, SN_shares, create_share);

      if (nnode && !lGetList(pnode, SN_children)) {
         lSetList(pnode, SN_children, childs);
      }
   }
   
   what = lWhat("%T(ALL)", SN_Type);
   alp = cod_api(COD_SHARETREE_LIST, COD_API_MOD, &lp, NULL, what);
   
   if (alp) {
      what = lWhat("%T(ALL)", SN_Type);
      erralp = cod_api(COD_SHARETREE_LIST, COD_API_GET, &lp, NULL, what);
      lFreeList(&erralp);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
      ep = lFirst(lp);
      Codine_Master_impl::instance()->updateShareTree(ep);
   }
      
   what = lFreeWhat(what);
   lp = lFreeList(lp);
   throwErrors(alp);

   creation = 0;
}


Codine_cod_string Codine_ShareTreeNode_impl::get_name(CORBA_Context* ctx) {
   QENTER("Codine_ShareTreeNode_impl::get_name");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   getSelf();
   const char* temp;
   Codine_cod_string foo = CORBA_string_dup((creation!=0)?((temp = lGetString(self, SN_name))?temp:""):(const char*)key);
   return foo;
}


Codine_ShareTreeNode* Codine_ShareTreeNode_impl::newLeaf(const char* name, CORBA_ULong shares, CORBA_Context* ctx) {
   QENTER("Codine_ShareTreeNode_impl::newLeaf");
   
   AUTO_LOCK_MASTER;
   qidl_authenticate(ctx);
   
   list<Codine_ShareTreeNode_impl*>::iterator it = children.begin();
   for (it ; it != children.end();++it) 
      if (!strcmp((*it)->get_name(ctx), name)) {
         return NULL;
      }
      
   Codine_ShareTreeNode_impl* sn;
   
   sn = new Codine_ShareTreeNode_impl(name, time(NULL), orb, this);
       
   if (sn) {
      children.push_back(sn);
      orb->connect(sn);
      sn->set_create_share(shares);
      sn->add(ctx);
      cout << name << " added to " << key << "'s children." << endl;
   }

   
   return Codine_ShareTreeNode::_duplicate(sn);
}


Codine_ShareTreeNode* Codine_ShareTreeNode_impl::newNode(const char* name, CORBA_ULong shares, CORBA_Context* ctx) {
   QENTER("Codine_ShareTreeNode_impl::newNode");
   
   AUTO_LOCK_MASTER;
   qidl_authenticate(ctx);
   
   Codine_ShareTreeNode_impl* sn;
   sn = new Codine_ShareTreeNode_impl(name,time(NULL), orb, this);
   
   if (sn) {
      children.push_back(sn);
      orb->connect(sn);
   }
   return sn;
}


Codine_ShareTreeNodeSeq* Codine_ShareTreeNode_impl::get_children(CORBA_Context* ctx) {
   QENTER("Codine_ShareTreeNode_impl::get_children");
   AUTO_LOCK_MASTER;
   qidl_authenticate(ctx);
   
   list<Codine_ShareTreeNode_impl*>::iterator it=children.begin();

   Codine_ShareTreeNodeSeq* stns = new Codine_ShareTreeNodeSeq();

   for(it=children.begin(); it != children.end(); it++){
      stns->append(Codine_ShareTreeNode_impl::_duplicate(*it)); 
   }

   return stns;
}


void Codine_ShareTreeNode_impl::changed(lListElem* _newSelf) {
   QENTER("Codine_ShareTreeNode_impl::changed");

   AUTO_LOCK_MASTER;
   // set self variable for now
   self = newSelf = _newSelf;

   // store event counter locally, to pass it back to the client later
   // This works as long as there is only one CORBA thread, so there
   // can be only one request dispatched at one time.
   // If more client requests were able to execute simultanously,
   // they would overwrite this local variable => !&%$&/�$�%
   lastEvent = qidl_event_count;

   // build header
   Codine_event ev;
   ev.type = Codine_ev_mod;
   ev.obj  = COD_SHARETREE_LIST;
   ev.name = CORBA_string_dup((creation!=0)?lGetString(newSelf, SN_name):(const char*)key);
   ev.id   = 0; 
   ev.ref  = Codine_ShareTreeNode_impl::_duplicate(this);
   ev.count= qidl_event_count;
   
   // get state
   Codine_contentSeq* cs = get_content(Codine_Master_impl::instance()->getMasterContext());
   ev.changes = *cs;
   delete cs;

   CORBA_Any any;
   any <<= ev;

   Codine_Master_impl::instance()->addEvent(any);

   // now we're ourselves again :-(
   newSelf = 0;
}
