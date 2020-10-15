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
// Codine_Queue_impl.cpp
// implementation for Queue object

#include <pthread.h>

#include "Queue_impl.h"
#include "Complex_impl.h"
#include "Calendar_impl.h"
#include "Master_impl.h"
#include "qidl_common.h"

extern "C" {
#include "cod_api.h"
#include "cod_answerL.h"
#include "cod_complexL.h"
#include "cod_m_event.h"
#include "cod_queueL.h"
#include "cod_calendarL.h"
#include "codrmon.h"
#include "cod_log.h"
#include "qmaster.h"
#include "read_write_queue.h"
}

#ifdef HAVE_STD
using namespace std;
#endif

extern lList* Master_Queue_List;

Codine_Queue_impl::Codine_Queue_impl(const char* _name, CORBA_ORB_var o)
   : Codine_Queue_implementation(_name, o) {
   QENTER("Codine_Queue_impl::Codine_Queue_impl");
   DPRINTF(("Name: %s\n", _name));
}

Codine_Queue_impl::Codine_Queue_impl(const char* _name, const time_t& tm, CORBA_ORB_var o)
   : Codine_Queue_implementation(_name, tm, o) {
   QENTER("Codine_Queue_impl::Codine_Queue_impl(time)");
   DPRINTF(("Name: %s\n", _name));
   
   AUTO_LOCK_MASTER;

   // copy the template queue from master
   self = cull_read_in_qconf(NULL, NULL, 0, 0, NULL, NULL);
   lSetString(self, QU_qname, (char*)_name);
}
   
Codine_Queue_impl::~Codine_Queue_impl() {
   QENTER("Codine_Queue_impl::~Codine_Queue_impl");
   DPRINTF(("Name: %s\n", (const char*)key));

   if(creation != 0)
      lFreeElem(&self);
}

// inherited from Codine_Object
void Codine_Queue_impl::destroy(CORBA_Context* ctx) {
   QENTER("Codine_Queue_impl::destroy");

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

   lp = lCreateList("My Queue List", QU_Type);
   lAppendElem(lp, lCopyElem(self));

   alp = cod_api(COD_QUEUE_LIST, COD_API_DEL, &lp, NULL, NULL);
   throwErrors(alp);
}

lListElem* Codine_Queue_impl::getSelf() {
   QENTER("Codine_Queue_impl::getSelf");
   
   if(creation != 0)
      return self;

   // if newSelf is set, then use newSelf, because newSelf is
   // valid (if !NULL) and definitely newer than self
   if(newSelf)
      return self = newSelf;

   AUTO_LOCK_MASTER;

   lCondition* cp = lWhere("%T(%I==%s)", QU_Type, QU_qname, (const char*)key);
   self = lFindFirst(Master_Queue_List, cp);
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


void Codine_Queue_impl::add(CORBA_Context* ctx) {
   QENTER("Codine_Queue_impl::add");

   // this Queue has been added already ?
   if(creation == 0)
      return;
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lEnumeration* what;
   
   lp = lCreateList("My Queue List", QU_Type);
   lAppendElem(lp, self);    // This cares for automatic deletion of self(!)

   what = lWhat("%T(ALL)", QU_Type);

   alp = cod_api(COD_QUEUE_LIST, COD_API_ADD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   // if we're here, everything worked fine and we can set
   // creation to 0, thus saying that we are a REAL queue
   creation = 0;
}

Codine_cod_string Codine_Queue_impl::get_qname(CORBA_Context* ctx) {
   QENTER("Codine_Queue_impl::get_qname");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();
   const char* temp;
   Codine_cod_string foo = CORBA_string_dup((creation!=0)?((temp = lGetString(self, QU_qname))?temp:""):(const char*)key);
   return foo;
}

Codine_ComplexSeq* Codine_Queue_impl::get_complex_list(CORBA_Context* ctx) {
   QENTER("Codine_Queue_impl::get_complex_list");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // now build the list of object refs
   lList* cpxl = lGetList(self, QU_complex_list);
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


Codine_cod_ulong Codine_Queue_impl::set_complex_list(const Codine_ComplexSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_Queue_impl::set_complex_list");

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
      lp = lCreateList("My Queue List", QU_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("cplx list", CX_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(CX_Type));
      lSetString(lep, CX_name, val[i]->get_name(ctx));
      // we don't need the other fields for QU_complex_list
   }
   lSetList(ep, QU_complex_list, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", QU_Type, QU_qname, QU_complex_list);
   
   alp = cod_api(COD_QUEUE_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}


Codine_Calendar* Codine_Queue_impl::get_calendar(CORBA_Context* ctx) {
   QENTER("Codine_Queue_impl::get_calendar");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // return the  object ref
   if(lGetString(self, QU_calendar))
      return Codine_Calendar_impl::_duplicate(Codine_Master_impl::instance()->getCalendarImpl(lGetString(self, QU_calendar)));
   else
      return 0;
}


Codine_cod_ulong Codine_Queue_impl::set_calendar(Codine_Calendar* val, CORBA_Context* ctx) {
   QENTER("Codine_Queue_impl::set_calendar");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   lListPtr      lp;
   lListPtr      alp;
   lListElem*    ep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My Queue List", QU_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   lSetString(ep,QU_calendar,val->get_name(ctx));

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", QU_Type, QU_qname, QU_calendar);
   
   alp = cod_api(COD_QUEUE_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}


void Codine_Queue_impl::changed(lListElem* _newSelf) {
   QENTER("Codine_Queue_impl::changed");

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
   ev.obj  = COD_QUEUE_LIST;
   ev.name = CORBA_string_dup((creation!=0)?lGetString(newSelf, QU_qname):(const char*)key);
   ev.id   = 0; // not needed for queue
   ev.ref  = Codine_Queue_impl::_duplicate(this);
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
