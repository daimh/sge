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
// Codine_ExecHost_impl.cpp
// implementation for exechost object

#include <pthread.h>

#include "ExecHost_impl.h"
#include "UserProject_impl.h"
#include "UserSet_impl.h"
#include "Complex_impl.h"
#include "Master_impl.h"
#include "qidl_common.h"

extern "C" {
#include "cod_api.h"
#include "cod_answerL.h"
#include "cod_m_event.h"
#include "cod_hostL.h"
#include "cod_userprjL.h"
#include "cod_usersetL.h"
#include "cod_complexL.h"
#include "codrmon.h"
#include "cod_log.h"
#include "qmaster.h"
#include "read_write_queue.h"
}

#ifdef HAVE_STD
using namespace std;
#endif

extern lList* Master_Exechost_List;

Codine_ExecHost_impl::Codine_ExecHost_impl(const char* _name, CORBA_ORB_var o)
   : Codine_ExecHost_implementation(_name, o) {
   QENTER("Codine_ExecHost_impl::Codine_ExecHost_impl");
   DPRINTF(("Name: %s\n", _name));
}

Codine_ExecHost_impl::Codine_ExecHost_impl(const char* _name, const time_t& tm, CORBA_ORB_var o)
   : Codine_ExecHost_implementation(_name, tm, o) {
   QENTER("Codine_ExecHost_impl::Codine_ExecHost_impl(time)");
   DPRINTF(("Name: %s\n", _name));
   
   AUTO_LOCK_MASTER;

   
   self = lCreateElem(EH_Type);
   lSetString(self, EH_name, (char*)_name);
}
   
Codine_ExecHost_impl::~Codine_ExecHost_impl() {
   QENTER("Codine_ExecHost_impl::~Codine_ExecHost_impl");
   DPRINTF(("Name: %s\n", (const char*)key));

   if(creation != 0)
      lFreeElem(&self);
}

// inherited from Codine_Object
void Codine_ExecHost_impl::destroy(CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::destroy");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

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

   lp = lCreateList("My ExecHost List", EH_Type);
   lAppendElem(lp, lCopyElem(self));

   alp = cod_api(COD_EXECHOST_LIST, COD_API_DEL, &lp, NULL, NULL);
   throwErrors(alp);
}

lListElem* Codine_ExecHost_impl::getSelf() {
   QENTER("Codine_ExecHost_impl::getSelf");
   
   if(creation != 0)
      return self;

   // if newSelf is set, then use newSelf, because newSelf is
   // valid (if !NULL) and definitely newer than self
   if(newSelf)
      return self = newSelf;

   AUTO_LOCK_MASTER;

   lCondition* cp = lWhere("%T(%I==%s)", EH_Type, EH_name, (const char*)key);
   self = lFindFirst(Master_Exechost_List, cp);
   lFreeWhere(&cp);
    
   if(!self) {  
      // we must not destroy ourselves here because the other thread
      // might also have done so already. if the object is still
      // alive at this point and will NOT be destroyed automatically
      // (by OB runtime) after this exception, then there is some logical
      // error in the code: The codine kernel did not notify the qidl
      // layer of the death of the object
      throw Codine_ObjDestroyed();
   }

   return self;
}


void Codine_ExecHost_impl::add(CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::add");

   // this ExecHost has been added already ?
   if(creation == 0)
      return;
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lEnumeration* what;
   
   lp = lCreateList("My ExecHost List", EH_Type);
   lAppendElem(lp, self);    // This cares for automatic deletion of self(!)

   what = lWhat("%T(ALL)", EH_Type);

   alp = cod_api(COD_EXECHOST_LIST, COD_API_ADD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   // if we're here, everything worked fine and we can set
   // creation to 0, thus saying that we are a REAL queue
   creation = 0;
}

Codine_cod_string Codine_ExecHost_impl::get_name(CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::get_name");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();
   const char* temp;
   Codine_cod_string foo = CORBA_string_dup((creation!=0)?((temp = lGetString(self,EH_name))?temp:""):(const char*)key);
   return foo;
}


Codine_ComplexSeq* Codine_ExecHost_impl::get_complex_list(CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::get_complex_list");
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   if(!getSelf()) {
      orb->disconnect(this);
      CORBA_release(this);
      throw Codine_ObjDestroyed();
   }

   // now build the list of object refs
   lList* cpxl = lGetList(self, EH_complex_list);
   lListElem* lep;
   Codine_Complex_impl* c;

   Codine_ComplexSeq* cpx = new Codine_ComplexSeq();
   for_each_cpp(lep, cpxl) {
      c = Codine_Master_impl::instance()->getComplexImpl(lGetString(lep, CX_name));
      if(c)
         cpx->append(Codine_Complex_impl::_duplicate(c));
   }
   
   return cpx;
}


Codine_cod_ulong Codine_ExecHost_impl::set_complex_list(const Codine_ComplexSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::set_complex_list");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lList*        ltp;
   lListElem*    ep;
   lListElem*    lep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My ExecHost List", EH_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("cplx list", CX_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(CX_Type));
      lSetString(lep, CX_name, val[i]->get_name(ctx));
      // we don't need the other fields for EH_complex_list
   }
   lSetList(ep, EH_complex_list, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", EH_Type, EH_name, EH_complex_list);
   
   alp = cod_api(COD_EXECHOST_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}

Codine_UserSetSeq* Codine_ExecHost_impl::get_acl(CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::get_acl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make REAL UserSetSeq out of dummy UserSet List
   Codine_UserSetSeq*   uss = new Codine_UserSetSeq(lGetNumberOfElem(
                                                   lGetList(self, EH_acl)));

   lListElem* lep;
   for_each_cpp(lep, lGetList(self, EH_acl)) {
      Codine_UserSet_impl* us = Codine_Master_impl::instance()->getUserSetImpl(
                                          lGetString(lep, US_name));
      if(us)
         uss->append(Codine_UserSet_impl::_duplicate(us));
   }

   return uss;
}

Codine_cod_ulong Codine_ExecHost_impl::set_acl(const Codine_UserSetSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::set_acl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lList*        ltp;
   lListElem*    ep;
   lListElem*    lep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My ExecHost List", EH_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("acl list", US_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(US_Type));
      lSetString(lep, US_name, val[i]->get_name(ctx));
      // we don't need the other fields for EH_acl
   }
   lSetList(ep, EH_acl, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", EH_Type, EH_name, EH_acl);
   
   alp = cod_api(COD_EXECHOST_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}

Codine_UserSetSeq* Codine_ExecHost_impl::get_xacl(CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::get_xacl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make REAL UserSetSeq out of dummy UserSet List
   Codine_UserSetSeq*   uss = new Codine_UserSetSeq(lGetNumberOfElem(
                                                   lGetList(self, EH_xacl)));

   lListElem* lep;
   for_each_cpp(lep, lGetList(self, EH_xacl)) {
      Codine_UserSet_impl* us = Codine_Master_impl::instance()->getUserSetImpl(
                                          lGetString(lep, US_name));
      if(us)
         uss->append(Codine_UserSet_impl::_duplicate(us));
   }

   return uss;
}

Codine_cod_ulong Codine_ExecHost_impl::set_xacl(const Codine_UserSetSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::set_xacl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lList*        ltp;
   lListElem*    ep;
   lListElem*    lep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My ExecHost List", EH_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("xacl list", US_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(US_Type));
      lSetString(lep, US_name, val[i]->get_name(ctx));
      // we don't need the other fields for EH_xacl
   }
   lSetList(ep, EH_xacl, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", EH_Type, EH_name, EH_xacl);
   
   alp = cod_api(COD_EXECHOST_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}

Codine_UserProjectSeq* Codine_ExecHost_impl::get_prj(CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::get_prj");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make REAL UserSetSeq out of dummy UserSet List
   Codine_UserProjectSeq*   ups = new Codine_UserProjectSeq(lGetNumberOfElem(
                                                   lGetList(self, EH_prj)));

   lListElem* lep;
   for_each_cpp(lep, lGetList(self, EH_prj)) {
      Codine_Project_impl* up = Codine_Master_impl::instance()->getProjectImpl( lGetString(lep, US_name));
      if(up)
         ups->append(Codine_Project_impl::_duplicate(up));
   }

   return ups;
}

Codine_cod_ulong Codine_ExecHost_impl::set_prj(const Codine_UserProjectSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::set_prj");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lList*        ltp;
   lListElem*    ep;
   lListElem*    lep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My ExecHost List", EH_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("prj list", UP_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(UP_Type));
      lSetString(lep, UP_name, val[i]->get_name(ctx));
      // we don't need the other fields for EH_prj
   }
   lSetList(ep, EH_prj, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", EH_Type, EH_name, EH_prj);
   
   alp = cod_api(COD_EXECHOST_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}

Codine_UserProjectSeq* Codine_ExecHost_impl::get_xprj(CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::get_xprj");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make REAL UserSetSeq out of dummy UserSet List
   Codine_UserProjectSeq*   ups = new Codine_UserProjectSeq(lGetNumberOfElem(
                                                   lGetList(self, EH_xprj)));

   lListElem* lep;
   for_each_cpp(lep, lGetList(self, EH_xprj)) {
      Codine_Project_impl* up = Codine_Master_impl::instance()->getProjectImpl( lGetString(lep, US_name));
      if(up)
         ups->append(Codine_Project_impl::_duplicate(up));
   }

   return ups;
}

Codine_cod_ulong Codine_ExecHost_impl::set_xprj(const Codine_UserProjectSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_ExecHost_impl::set_xprj");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lList*        ltp;
   lListElem*    ep;
   lListElem*    lep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My ExecHost List", EH_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("xprj list", UP_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(UP_Type));
      lSetString(lep, UP_name, val[i]->get_name(ctx));
      // we don't need the other fields for EH_xprj
   }
   lSetList(ep, EH_xprj, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", EH_Type, EH_name, EH_xprj);
   
   alp = cod_api(COD_EXECHOST_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}


void Codine_ExecHost_impl::changed(lListElem* _newSelf) {
   QENTER("Codine_ExecHost_impl::changed");

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
   ev.obj  = COD_EXECHOST_LIST;
   ev.name = CORBA_string_dup((creation!=0)?lGetString(newSelf, EH_name):(const char*)key);
   ev.id   = 0;
   ev.ref = Codine_ExecHost_impl::_duplicate(this);
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
