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
// Codine_UserProject_impl.cpp
// implementation for UserProject object

#include <pthread.h>

#include "UserProject_impl.h"
#include "UserSet_impl.h"
#include "Master_impl.h"
#include "qidl_common.h"

extern "C" {
#include "cod_api.h"
#include "cod_answerL.h"
#include "cod_userprjL.h"
#include "cod_usersetL.h"
#include "cod_m_event.h"
#include "codrmon.h"
#include "cod_log.h"
#include "qmaster.h"
#include "read_write_queue.h"
}

#ifdef HAVE_STD
using namespace std;
#endif

// implements TWO classes (User and Project)
// only difference is setting other apiListType values

extern lList* Master_User_List;
extern lList* Master_Project_List;


// User

Codine_User_impl::Codine_User_impl(const char* _name, CORBA_ORB_var o)
   : Codine_UserProject_implementation(_name, o) {
   QENTER("Codine_User_impl::Codine_User_impl");
   // do this AFTER calling base class constructor
   apiListType = COD_USER_LIST;
}

Codine_User_impl::Codine_User_impl(const char* _name, const time_t& tm, CORBA_ORB_var o)
   : Codine_UserProject_implementation(_name, tm, o) {
   QENTER("Codine_User_impl::Codine_User_impl (time)");
   AUTO_LOCK_MASTER;

   lSetString(self, UP_name, (char*)_name);
   apiListType = COD_USER_LIST;
}
   
Codine_User_impl::~Codine_User_impl() {
   QENTER("Codine_User_impl::~Codine_User_impl");
   if(creation != 0)
      lFreeElem(&self);
}

// inherited from Codine_Object
void Codine_User_impl::destroy(CORBA_Context* ctx) {
   QENTER("Codine_User_impl::destroy");
   
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

   lp = lCreateList("My User List", UP_Type);
   lAppendElem(lp, lCopyElem(self));

   alp = cod_api(apiListType, COD_API_DEL, &lp, NULL, NULL);
   throwErrors(alp);
}

lListElem* Codine_User_impl::getSelf() {
   QENTER("Codine_User_impl::getSelf");

   if(creation != 0)
      return self;

   // if newSelf is set, then use newSelf, because newSelf is
   // valid (if !NULL) and definitely newer than self
   if(newSelf)
      return self = newSelf;

   AUTO_LOCK_MASTER;

   lCondition* cp = lWhere("%T(%I==%s)", UP_Type, UP_name, (const char*)key);
   self = lFindFirst(Master_User_List, cp);
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


void Codine_User_impl::add(CORBA_Context* ctx) {
   QENTER("Codine_User_impl::add");

   if(creation == 0)
      return;
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lEnumeration* what;
   
   lp = lCreateList("My User List", UP_Type);
   lAppendElem(lp, self);    // This cares for automatic deletion of self(!)

   what = lWhat("%T(ALL)", UP_Type);

   alp = cod_api(apiListType, COD_API_ADD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);


   creation = 0;
}

Codine_cod_string Codine_User_impl::get_name(CORBA_Context* ctx) {
   QENTER("Codine_User_impl::get_name");
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();
   const char* temp;
   Codine_cod_string foo = CORBA_string_dup((creation!=0)?((temp = lGetString(self, UP_name))?temp:""):(const char*)key);
   return foo;
}

Codine_UserSetSeq* Codine_User_impl::get_acl(CORBA_Context* ctx) {
   QENTER("Codine_User_impl::get_acl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // now build the list of object refs
   lList* usl = lGetList(self, UP_acl);
   lListElem* lep;
   Codine_UserSet_impl* us;

   Codine_UserSetSeq* uss = new Codine_UserSetSeq();
   for_each_cpp(lep, usl) {
      us = Codine_Master_impl::instance()->getUserSetImpl(lGetString(lep, US_name));
      if(us)
         uss->append(Codine_UserSet_impl::_duplicate(us));
   }
   
   return uss;
}


Codine_cod_ulong Codine_User_impl::set_acl(const Codine_UserSetSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_User_impl::set_acl");
   
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
      lp = lCreateList("My User List", UP_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("userset list", US_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(US_Type));
      lSetString(lep, US_name, val[i]->get_name(ctx));
      /*lSetUlong(lep, US_fshare, val[i]->get_fshare(ctx));
      lSetUlong(lep, US_oticket, val[i]->get_oticket(ctx));
      lSetList(lep, US_entries, UserEntrySeq2cull(*(val[i]->get_entries(ctx))));*/
      
   }
   lSetList(ep, UP_acl, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", UP_Type, UP_name, UP_acl);
   
   alp = cod_api(apiListType, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}

Codine_UserSetSeq* Codine_User_impl::get_xacl(CORBA_Context* ctx) {
   QENTER("Codine_User_impl::get_xacl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // now build the list of object refs
   lList* usl = lGetList(self, UP_xacl);
   lListElem* lep;
   Codine_UserSet_impl* us;

   Codine_UserSetSeq* uss = new Codine_UserSetSeq();
   for_each_cpp(lep, usl) {
      us = Codine_Master_impl::instance()->getUserSetImpl(lGetString(lep, US_name));
      if(us)
         uss->append(Codine_UserSet_impl::_duplicate(us));
   }
   
   return uss;
}


Codine_cod_ulong Codine_User_impl::set_xacl(const Codine_UserSetSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_User_impl::set_xacl");

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
      lp = lCreateList("My User List", UP_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("userset list", US_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(US_Type));
      lSetString(lep, US_name, val[i]->get_name(ctx));
      
   }
   lSetList(ep, UP_acl, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", UP_Type, UP_name, UP_xacl);
   
   alp = cod_api(apiListType, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}


void Codine_User_impl::changed(lListElem* _newSelf) {
   QENTER("Codine_User_impl::changed");

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
   ev.obj  = apiListType;
   ev.name = CORBA_string_dup((creation!=0)?lGetString(newSelf, UP_name):(const char*)key);
   ev.id   = 0; 
   ev.ref  = Codine_User_impl::_duplicate(this);
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

// Project

Codine_Project_impl::Codine_Project_impl(const char* _name, CORBA_ORB_var o)
   : Codine_UserProject_implementation(_name, o) {
   QENTER("Codine_Project_impl::Codine_Project_impl");
   // do this AFTER calling base class constructor
   apiListType = COD_PROJECT_LIST;
}

Codine_Project_impl::Codine_Project_impl(const char* _name, const time_t& tm, CORBA_ORB_var o)
   : Codine_UserProject_implementation(_name, tm, o) {
   QENTER("Codine_Project_impl::Codine_Project_impl (time)");
   
   AUTO_LOCK_MASTER;

   lSetString(self, UP_name, (char*)_name);
   apiListType = COD_PROJECT_LIST;
}
   
Codine_Project_impl::~Codine_Project_impl() {
   QENTER("Codine_Project_impl::~Codine_Project_impl");
   if(creation != 0)
      lFreeElem(&self);
}

// inherited from Codine_Object
void Codine_Project_impl::destroy(CORBA_Context* ctx) {
   QENTER("Codine_Project_impl::destroy");

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

   lp = lCreateList("My Project List", UP_Type);
   lAppendElem(lp, lCopyElem(self));

   alp = cod_api(apiListType, COD_API_DEL, &lp, NULL, NULL);
   throwErrors(alp);
}

lListElem* Codine_Project_impl::getSelf() {
   QENTER("Codine_Project_impl::getSelf");

   if(creation != 0)
      return self;

   // if newSelf is set, then use newSelf, because newSelf is
   // valid (if !NULL) and definitely newer than self
   if(newSelf)
      return self = newSelf;

   AUTO_LOCK_MASTER;

   lCondition* cp = lWhere("%T(%I==%s)", UP_Type, UP_name, (const char*)key);
   self = lFindFirst(Master_Project_List, cp);
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


void Codine_Project_impl::add(CORBA_Context* ctx) {
   QENTER("Codine_Project_impl::add");

   if(creation == 0)
      return;
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lEnumeration* what;
   
   lp = lCreateList("My Project List", UP_Type);
   lAppendElem(lp, self);    // This cares for automatic deletion of self(!)

   what = lWhat("%T(ALL)", UP_Type);

   alp = cod_api(apiListType, COD_API_ADD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);


   creation = 0;
}

Codine_cod_string Codine_Project_impl::get_name(CORBA_Context* ctx) {
   QENTER("Codine_Project_impl::get_name");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();
   const char* temp;
   Codine_cod_string foo = CORBA_string_dup((creation!=0)?((temp = lGetString(self, UP_name))?temp:""):(const char*)key);
   return foo;
}

Codine_UserSetSeq* Codine_Project_impl::get_acl(CORBA_Context* ctx) {
   QENTER("Codine_Project_impl::get_acl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // now build the list of object refs
   lList* usl = lGetList(self, UP_acl);
   lListElem* lep;
   Codine_UserSet_impl* us;

   Codine_UserSetSeq* uss = new Codine_UserSetSeq();
   for_each_cpp(lep, usl) {
      us = Codine_Master_impl::instance()->getUserSetImpl(lGetString(lep, US_name));
      if(us)
         uss->append(Codine_UserSet_impl::_duplicate(us));
   }
   
   return uss;
}


Codine_cod_ulong Codine_Project_impl::set_acl(const Codine_UserSetSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_Project_impl::set_acl");

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
      lp = lCreateList("My Project List", UP_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("userset list", US_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(US_Type));
      lSetString(lep, US_name, val[i]->get_name(ctx));
      /*lSetUlong(lep, US_fshare, val[i]->get_fshare(ctx));
      lSetUlong(lep, US_oticket, val[i]->get_oticket(ctx));
      lSetList(lep, US_entries, UserEntrySeq2cull(*(val[i]->get_entries(ctx))));*/
      
   }
   lSetList(ep, UP_acl, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", UP_Type, UP_name, UP_acl);
   
   alp = cod_api(apiListType, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}

Codine_UserSetSeq* Codine_Project_impl::get_xacl(CORBA_Context* ctx) {
   QENTER("Codine_Project_impl::get_xacl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // now build the list of object refs
   lList* usl = lGetList(self, UP_xacl);
   lListElem* lep;
   Codine_UserSet_impl* us;

   Codine_UserSetSeq* uss = new Codine_UserSetSeq();
   for_each_cpp(lep, usl) {
      us = Codine_Master_impl::instance()->getUserSetImpl(lGetString(lep, US_name));
      if(us)
         uss->append(Codine_UserSet_impl::_duplicate(us));
   }
   
   return uss;
}


Codine_cod_ulong Codine_Project_impl::set_xacl(const Codine_UserSetSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_Project_impl::set_xacl");

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
      lp = lCreateList("My Project List", UP_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("userset list", US_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(US_Type));
      lSetString(lep, US_name, val[i]->get_name(ctx));
      
   }
   lSetList(ep, UP_acl, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", UP_Type, UP_name, UP_xacl);
   
   alp = cod_api(apiListType, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}


void Codine_Project_impl::changed(lListElem* _newSelf) {
   QENTER("Codine_Project_impl::changed");

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
   ev.obj  = apiListType;
   ev.name = CORBA_string_dup((creation!=0)?lGetString(newSelf, UP_name):(const char*)key);
   ev.id   = 0; 
   ev.ref  = Codine_Project_impl::_duplicate(this);
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
