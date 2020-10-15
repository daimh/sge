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
// Codine_Checkpoint_impl.cpp
// implementation for Checkpoint object

#include <pthread.h>

#include "Checkpoint_impl.h"
#include "Queue_impl.h"
#include "Complex_impl.h"
#include "Master_impl.h"
#include "qidl_common.h"

extern "C" {
#include "cod_api.h"
#include "cod_answerL.h"
#include "cod_ckptL.h"
#include "cod_queueL.h"
#include "cod_complexL.h"
#include "cod_peL.h"
#include "cod_m_event.h"
#include "codrmon.h"
#include "cod_log.h"
#include "qmaster.h"
#include "read_write_queue.h"
}

#ifdef HAVE_STD
using namespace std;
#endif

extern lList* Master_Ckpt_List;

Codine_Checkpoint_impl::Codine_Checkpoint_impl(const char* _name, CORBA_ORB_var o)
   : Codine_Checkpoint_implementation(_name, o) {
   QENTER("Codine_Checkpoint_impl::Codine_Checkpoint_impl");
   DPRINTF(("Name: %s\n", _name));
}

Codine_Checkpoint_impl::Codine_Checkpoint_impl(const char* _name, const time_t& tm, CORBA_ORB_var o)
   : Codine_Checkpoint_implementation(_name, tm, o) {
   QENTER("Codine_Checkpoint_impl::Codine_Checkpoint_impl(time)");
   DPRINTF(("Name: %s\n", _name));
   
   AUTO_LOCK_MASTER;

   //self = cull_read_in_qconf(NULL, NULL, 0, NULL, NULL);
   lSetString(self, CK_name, (char*)_name);
}
   
Codine_Checkpoint_impl::~Codine_Checkpoint_impl() {
   QENTER("Codine_Checkpoint_impl::~Codine_Checkpoint_impl");
   DPRINTF(("Name: %s\n", (const char*)key));

   if(creation != 0)
      lFreeElem(&self);
}

// inherited from Codine_Object
void Codine_Checkpoint_impl::destroy(CORBA_Context* ctx) {
   QENTER("Codine_Checkpoint_impl::destroy");

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

   lp = lCreateList("My Checkpoint List", CK_Type);
   lAppendElem(lp, lCopyElem(self));

   alp = cod_api(COD_CKPT_LIST, COD_API_DEL, &lp, NULL, NULL);
   throwErrors(alp);
}

lListElem* Codine_Checkpoint_impl::getSelf() {
   QENTER("Codine_Checkpoint_impl::getSelf");

   if(creation != 0)
      return self;

   // if newSelf is set, then use newSelf, because newSelf is
   // valid (if !NULL) and definitely newer than self
   if(newSelf)
      return self = newSelf;

   AUTO_LOCK_MASTER;

   lCondition* cp = lWhere("%T(%I==%s)", CK_Type, CK_name, (const char*)key);
   self = lFindFirst(Master_Ckpt_List, cp);
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


void Codine_Checkpoint_impl::add(CORBA_Context* ctx) {
   QENTER("Codine_Checkpoint_impl::add");

   if(creation == 0)
      return;
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lEnumeration* what;
   
   lp = lCreateList("My Checkpoint List", CK_Type);
   lAppendElem(lp, self);    // This cares for automatic deletion of self(!)

   what = lWhat("%T(ALL)", CK_Type);

   alp = cod_api(COD_CKPT_LIST, COD_API_ADD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);


   creation = 0;
}

Codine_cod_string Codine_Checkpoint_impl::get_name(CORBA_Context* ctx) {
   QENTER("Codine_Checkpoint_impl::get_name");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();
   const char* temp;
   Codine_cod_string foo = CORBA_string_dup((creation!=0)?((temp = lGetString(self, CK_name))?temp:""):(const char*)key);
   return foo;
}


Codine_QueueSeq* Codine_Checkpoint_impl::get_queue_list(CORBA_Context* ctx) {
   QENTER("Codine_Checkpoint_impl::get_queue_list");
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();
   
   lList* ql = lGetList(self, CK_queue_list);
   lListElem* lep;
   Codine_Queue_impl* q;

   Codine_QueueSeq* qs = new Codine_QueueSeq();
   for_each_cpp(lep, ql) {
      
      q = Codine_Master_impl::instance()->getQueueImpl(lGetString(lep, QR_name));
      if(q) 
         qs->append(Codine_Queue_impl::_duplicate(q));
      
   }
   
   return qs;
}


Codine_cod_ulong Codine_Checkpoint_impl::set_queue_list(const Codine_QueueSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_Checkpoint_impl::set_queue_list");

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

   Codine_ComplexSeq* cpx;
   lList*        cxl;
   lListElem*    cxel;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My Checkpoint List", CK_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }

   ltp = lCreateList("queue list", QU_Type);

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(QU_Type));
      lSetString(lep, QU_qname, val[i]->get_qname(ctx));
      /*lSetString(lep, QU_qhostname, val[i]->get_qhostname(ctx));
      lSetString(lep, QU_tmpdir, val[i]->get_tmpdir(ctx));
      lSetString(lep, QU_shell, val[i]->get_shell(ctx));
      lSetUlong(lep, QU_seq_no, val[i]->get_seq_no(ctx));
      lSetUlong(lep, QU_nsuspend, val[i]->get_nsuspend(ctx));
      lSetString(lep, QU_suspend_interval, val[i]->get_suspend_interval(ctx));
      lSetUlong(lep, QU_priority, val[i]->get_priority(ctx));
      lSetUlong(lep, QU_qtype, val[i]->get_qtype(ctx));
      lSetString(lep, QU_processors, val[i]->get_processors(ctx));
      lSetUlong(lep, QU_job_slots, val[i]->get_job_slots(ctx));
      lSetString(lep, QU_prolog, val[i]->get_prolog(ctx));
      lSetString(lep, QU_epilog, val[i]->get_epilog(ctx));
      lSetString(lep, QU_shell_start_mode, val[i]->get_shell_start_mode(ctx));
      lSetString(lep, QU_initial_state, val[i]->get_initial_state(ctx));
      lSetString(lep, QU_min_cpu_interval, val[i]->get_min_cpu_interval(ctx));
      lSetString(lep, QU_max_migr_time, val[i]->get_max_migr_time(ctx));
      lSetString(lep, QU_max_no_migr, val[i]->get_max_no_migr(ctx));
      lSetString(lep, QU_notify, val[i]->get_notify(ctx));
      
      //Calendar object
      lSetString(lep,QU_calendar, val[i]->get_calendar(ctx)->get_name(ctx));
      
      //Complex list
      cpx = val[i]->get_complex_list(ctx);
      cxl = lCreateList("complex list", CX_Type);
      
      for (CORBA_ULong j=0; j<cpx->length(); j++) {
         lAppendElem(cxl,cxel = lCreateElem(CX_Type));
         lSetString(cxel,CX_name, ((*cpx)[i])->get_name(ctx));   
      }
      lSetList(lep,QU_complex_list, cxl);

      //Complex entry lists
      lSetList(lep,QU_load_thresholds, ComplexEntrySeq2cull(*(val[i]->get_load_thresholds(ctx))));
      lSetList(lep,QU_suspend_thresholds, ComplexEntrySeq2cull(*(val[i]->get_suspend_thresholds(ctx))));
      lSetList(lep,QU_migr_load_thresholds, ComplexEntrySeq2cull(*(val[i]->get_migr_load_thresholds(ctx))));
      lSetList(lep,QU_consumable_config_list , ComplexEntrySeq2cull(*(val[i]->get_consumable_config_list(ctx))));*/

   }
   lSetList(ep, CK_queue_list, ltp);

   if(creation != 0)       // that's it for a local (= new) object
      return 0; 

   // otherwise send api request
   what = lWhat("%T(%I %I)", CK_Type, CK_name, CK_queue_list);
   
   alp = cod_api(COD_CKPT_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp); 

   // this one is set during the cod_api call
   return lastEvent;
}


void Codine_Checkpoint_impl::changed(lListElem* _newSelf) {
   QENTER("Codine_Checkpoint_impl::changed");

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
   ev.obj  = COD_CKPT_LIST;
   ev.name = CORBA_string_dup((creation!=0)?lGetString(newSelf, CK_name):(const char*)key);
   ev.id   = 0; 
   ev.ref  = Codine_Checkpoint_impl::_duplicate(this);
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
