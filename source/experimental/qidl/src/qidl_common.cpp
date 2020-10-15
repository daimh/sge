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
// qidl_common.cpp
// common stuff, helper classes, functions, etc.


#include <OB/CORBA.h>

#include "qidl_common.h"
#include "basic_types.h"
#include "qidl_c_api.h"
#include "elem_codes.h"

extern "C"
{
#include "cod_all_listsL.h"
#include "cod_uidgid.h"
#include "cod_max_nis_retries.h"
#include "commlib.h"
#include "codrmon.h"
}


// PthreadLock class
pthread_key_t PthreadLock::key;
bool          PthreadLock::init = false;

PthreadLock::PthreadLock(pthread_mutex_t* m) : mutex(m) {
   //QENTER("PthreadLock::PthreadLock");
   lock();
}

PthreadLock::~PthreadLock() {
   //QENTER("PthreadLock::~PthreadLock");
   unlock();
}

bool PthreadLock::initialize() {
   //QENTER("PthreadLock::initialize");
   return (init = !pthread_key_create(&key, &destroy));
}

void PthreadLock::lock() {
   //QENTER("PthreadLock::lock");

   if(!init)
      return;

   map<pthread_mutex_t*, unsigned int>* mutexes = (map<pthread_mutex_t*, unsigned int>*)pthread_getspecific(key);
   if(!mutexes) // there's no map, create one
      pthread_setspecific(key, mutexes = new map<pthread_mutex_t*, unsigned int>());
   
   if(!mutexes) // oh, oh...
      return;

   if((*mutexes)[mutex]++ == 0) {
      //cout << "thread: " << getpid() << " locking mutex " << (void*)mutex << endl;
      pthread_mutex_lock(mutex);
      //cout << "thread: " << getpid() << " succeeded." << endl;
   }
   //cout << "thread: " << getpid() << "leaving lock." << endl;
}

void PthreadLock::unlock() {
   //QENTER("PthreadLock::unlock");

   if(!init)
      return;

   map<pthread_mutex_t*, unsigned int>* mutexes = (map<pthread_mutex_t*, unsigned int>*)pthread_getspecific(key);
   
   if(!mutexes) // Oh oh, this should not happen...
      return;

   if((*mutexes)[mutex]-- == 1) {
      //cout << "thread: " << getpid() << " unlocking mutex " << (void*)mutex << endl;
      pthread_mutex_unlock(mutex);
      //cout << "thread: " << getpid() << " succeded." << endl;
   }
   //cout << "thread: " << getpid() << "leaving unlock." << endl;
}

// lListPtr class
lListPtr& lListPtr::operator=(lList* l) {
   QENTER("lListPtr::operator=(lList*)");

   if(list && destroy)
      lFreeList(list);
   list = l; destroy = true;
   return *this;
}

lListPtr& lListPtr::operator=(lListPtr& l) {
   QENTER("lListPtr::operator=(lListPtr&)");

   if(list && destroy)
      lFreeList(list);
   list = l.list; destroy = false;
   return *this;
}

void throwErrors(lList* alp) {
   QENTER("throwErrors");

   Codine_Error err;
   lListElem* aep;

   for_each_cpp(aep, alp) 
      if(lGetUlong(aep, AN_quality) == NUM_AN_ERROR) {
         CORBA_ULong i = err.answer.length();
         err.answer.length(i+1);
         char* text = lGetString(aep, AN_text);
         err.answer[i].status = lGetUlong(aep, AN_status);
         err.answer[i].text = CORBA_string_dup(text?text:"");
      }

   if(err.answer.length())
      throw err;
}


// Rmon class
Rmon::Rmon(const char* m) : method(m) {
   if(rmon_condition(QIDL_LAYER, TRACE))
      rmon_menter(m);
#ifdef REENTRANCY_CHECK
   if(entered.find(m) != entered.end())
      rmon_mprintf("Reentering method %s!!!!!!\n", m);
   entered.insert(method);
#endif
}

Rmon::~Rmon() {
   if(rmon_condition(QIDL_LAYER, TRACE))
      rmon_mexit(method, "unknown", 0);
#ifdef REENTRANCY_CHECK
   entered.erase(method);
#endif
}
#ifdef REENTRANCY_CHECK
set<const char*> Rmon::entered;
pthread_mutex_t  Rmon::set_lock = PTHREAD_MUTEX_INTIALIZER;
#endif

// authentication
// assume an already acquired Master Lock
cod_auth qidl_me;
void qidl_authenticate(CORBA_Context* ctx) {
   QENTER("qidl_authenticate");

   static CORBA_Context* last_ctx = 0;

   // this function is often called repeatedly with the same ctx
   // argument (e.g. the master context). the following check saves
   // a LOT of work
   if(last_ctx == ctx)
      return;
   else
      last_ctx = ctx;

   CORBA_NVList_var nv;
   ctx->get_values("",0, "cod_auth", nv);

   if(nv && nv->count()) {
      char  *temp;
      *nv->item(0)->value() >>= temp;
      
      // now extract auth info
      char* tok = strtok(temp, ":");
      qidl_me.uid = atoi(tok);
      tok = strtok(NULL, ":");
      qidl_me.euid = atoi(tok);
      tok = strtok(NULL, ":");
      qidl_me.gid = atoi(tok);
      tok = strtok(NULL, ":");
      qidl_me.egid = atoi(tok);
      tok = strtok(NULL, ":");
      
      //strcpy(qidl_me.host, tok);
      getuniquehostname(tok, qidl_me.host, 0);
      if(cod_uid2user(qidl_me.uid, qidl_me.user, sizeof(qidl_me.user), MAX_NIS_RETRIES))
         throw Codine_Authentication();
      if(cod_gid2group(qidl_me.gid, qidl_me.group, sizeof(qidl_me.group), MAX_NIS_RETRIES))
         throw Codine_Authentication();
   
   }
   else
      throw Codine_Authentication();
}

// converter functions
Codine_ComplexEntrySeq* cull2ComplexEntrySeq(lList* ltp) {
   QENTER("cull2ComplexEntrySeq");
   
   lListElem* lep;

   Codine_ComplexEntrySeq* lts = new Codine_ComplexEntrySeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_ComplexEntry ce;
      const char* dummy;
      dummy = lGetString(lep, CE_name);
      ce.name = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep, CE_shortcut);
      ce.shortcut = CORBA_string_dup(dummy?dummy:"");
      ce.valtype = lGetUlong(lep, CE_valtype);
      dummy = lGetString(lep, CE_stringval);
      ce.stringval = CORBA_string_dup(dummy?dummy:"");
      ce.relop = lGetUlong(lep, CE_relop);
      ce.request = lGetUlong(lep, CE_request);
      ce.consumable = lGetUlong(lep, CE_consumable);
      ce.forced = lGetUlong(lep, CE_forced);
      dummy = lGetString(lep, CE_default);
      ce.default_val = CORBA_string_dup(dummy?dummy:"");
      lts->append(ce);
   }

   return lts;
}

lList* ComplexEntrySeq2cull(Codine_ComplexEntrySeq& val) {
   QENTER("ComplexEntrySeq2cull");
   
   lList* ltp = lCreateList("cmplx entries", CE_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(CE_Type));
      lSetString(lep, CE_name, val[i].name);
      lSetString(lep, CE_shortcut, val[i].shortcut);
      lSetUlong(lep, CE_valtype, val[i].valtype);
      lSetString(lep, CE_stringval, val[i].stringval);
      lSetUlong(lep, CE_relop, val[i].relop);
      lSetUlong(lep, CE_request, val[i].request);
      lSetUlong(lep, CE_consumable, val[i].consumable);
      lSetUlong(lep, CE_forced, val[i].forced);
      lSetString(lep, CE_default, val[i].default_val);
   }

   return ltp;
}


Codine_PathNameSeq* cull2PathNameSeq(lList* ltp) {
   QENTER("cull2PathNameSeq");
   
   lListElem* lep;

   Codine_PathNameSeq* lts = new Codine_PathNameSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_PathName pn;
      const char* dummy;
      dummy = lGetString(lep,PN_path);
      pn.path = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep, PN_host);
      pn.host = CORBA_string_dup(dummy?dummy:"");
      lts->append(pn);
   }

   return lts;
}


lList* PathNameSeq2cull(Codine_PathNameSeq& val) {
   QENTER("PathNameSeq2cull");
   
   lList* ltp = lCreateList("path names", PN_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(PN_Type));
      lSetString(lep, PN_path, val[i].path);
      lSetString(lep, PN_host, val[i].host);
   }

   return ltp;
}

Codine_VariableSeq* cull2VariableSeq(lList* ltp) {
   QENTER("cull2VariableSeq");
   
   lListElem* lep;

   Codine_VariableSeq* lts = new Codine_VariableSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_Variable var;
      const char* dummy;
      dummy = lGetString(lep, VA_variable);
      var.variable = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep, VA_value);
      var.value = CORBA_string_dup(dummy?dummy:"");
      lts->append(var);
   }

   return lts;
}


lList* VariableSeq2cull(Codine_VariableSeq& val) {
   QENTER("VariableSeq2cull");
   
   lList* ltp = lCreateList("variables", VA_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(VA_Type));
      lSetString(lep, VA_variable, val[i].variable);
      lSetString(lep, VA_value, val[i].value);
   }

   return ltp;
}


Codine_MailRecipientSeq* cull2MailRecipientSeq(lList* ltp) {
   QENTER("cull2MailRecipientSeq");
   
   lListElem* lep;

   Codine_MailRecipientSeq* lts = new Codine_MailRecipientSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_MailRecipient mr;
      const char* dummy;
      dummy = lGetString(lep,MR_user);
      mr.user = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep, MR_host);
      mr.host = CORBA_string_dup(dummy?dummy:"");
      lts->append(mr);
   }

   return lts;
}


lList* MailRecipientSeq2cull(Codine_MailRecipientSeq& val) {
   QENTER("MailRecipientSeq2cull");
   
   lList* ltp = lCreateList("mail recipients", MR_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(MR_Type));
      lSetString(lep, MR_user, val[i].user);
      lSetString(lep, PN_host, val[i].host);
   }

   return ltp;
}

Codine_ConfigEntrySeq* cull2ConfigEntrySeq(lList* lp) {
   QENTER("cull2ConfigEntrySeq");
   
   lListElem* lep;

   Codine_ConfigEntrySeq* ces = new Codine_ConfigEntrySeq(lGetNumberOfElem(lp));

   for_each_cpp(lep, lp) {
      Codine_ConfigEntry ce;
      const char* dummy;
      dummy = lGetString(lep, CF_name);
      ce.name = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep, CF_value);
      ce.value = CORBA_string_dup(dummy?dummy:"");
      ces->append(ce);
   }

   return ces;
}


lList* ConfigEntrySeq2cull(Codine_ConfigEntrySeq& val) {
   QENTER("ConfigEntrySeq2cull");
   
   lList* lp = lCreateList("config entries", CF_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(lp, lep = lCreateElem(CF_Type));
      lSetString(lep, CF_name, val[i].name);
      lSetString(lep, CF_value, val[i].value);
   }

   return lp;
}

Codine_cod_stringSeq* cull2cod_stringSeq(lList* ltp) {
   lListElem* lep;

   Codine_cod_stringSeq* lts = new Codine_cod_stringSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_cod_string st;
      const char* dummy;
      dummy = lGetString(lep,STR);
      st = CORBA_string_dup(dummy?dummy:"");
      lts->append(st);
   }

   return lts;
}


lList* cod_stringSeq2cull(Codine_cod_stringSeq& val) {
   lList* ltp = lCreateList("strings", ST_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(ST_Type));
      lSetString(lep, STR, val[i]);
   }

   return ltp;
}


Codine_RangeSeq* cull2RangeSeq(lList* ltp) {
   QENTER("cull2RangeSeq");
   
   lListElem* lep;

   Codine_RangeSeq* lts = new Codine_RangeSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_Range rn;
      rn.min = lGetUlong(lep,RN_min);
      rn.max = lGetUlong(lep,RN_max);
      rn.step = lGetUlong(lep, RN_step);
      lts->append(rn);
   }

   return lts;
}


lList* RangeSeq2cull(Codine_RangeSeq& val) {
   QENTER("RangeSeq2cull");
   
   lList* ltp = lCreateList("ranges", RN_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(RN_Type));
      lSetUlong(lep, RN_min, val[i].min);
      lSetUlong(lep, RN_max, val[i].max);
      lSetUlong(lep, RN_step, val[i].step);
   }

   return ltp;
}


Codine_UserEntrySeq* cull2UserEntrySeq(lList* ltp) {
   QENTER("cull2UserEntrySeq");
   
   lListElem* lep;

   Codine_UserEntrySeq* ues = new Codine_UserEntrySeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_UserEntry ue;
      const char* dummy;
      dummy = lGetString(lep,UE_name);
      ue.name = CORBA_string_dup(dummy?dummy:"");         
      ues->append(ue);
   }

   return ues;
}


lList* UserEntrySeq2cull(Codine_UserEntrySeq& val) {
   QENTER("UserEntrySeq2cull");
   
   lList* ltp = lCreateList("user entries", UE_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(UE_Type));
      lSetString(lep, PN_path, val[i].name);
   }

   return ltp;
}


Codine_HostLoadSeq* cull2HostLoadSeq(lList* ltp) {
   QENTER("cull2HostLoadSeq");
   
   lListElem* lep;

   Codine_HostLoadSeq* hls = new Codine_HostLoadSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_HostLoad hl;
      const char* dummy;
      dummy = lGetString(lep,HL_name);
      hl.name = CORBA_string_dup(dummy?dummy:"");   
      dummy = lGetString(lep,HL_value);
      hl.value = CORBA_string_dup(dummy?dummy:"");
      hl.last_update = lGetUlong(lep, HL_last_update);
      hls->append(hl);
   }

   return hls;
}


lList* HostLoadSeq2cull(Codine_HostLoadSeq& val) {
   QENTER("HostLoadSeq2cull");
   
   lList* ltp = lCreateList("host loads", HL_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(HL_Type));
      lSetString(lep, HL_name, val[i].name);
      lSetString(lep, HL_value,val[i].value);
   }

   return ltp;
}


Codine_LoadScalingSeq* cull2LoadScalingSeq(lList* ltp) {
   QENTER("cull2LoadScalingSeq");
   
   lListElem* lep;

   Codine_LoadScalingSeq* lss = new Codine_LoadScalingSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_LoadScaling ls;
      const char* dummy;
      dummy = lGetString(lep,HS_name);
      ls.name = CORBA_string_dup(dummy?dummy:"");
      ls.value = lGetDouble(lep,HS_value);
      lss->append(ls);
   }

   return lss;
}


lList* LoadScalingSeq2cull(Codine_LoadScalingSeq& val) {
   QENTER("LoadScalingSeq2cull");
   
   lList* ltp = lCreateList("load scalings", HS_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(HS_Type));
      lSetString(lep, HS_name, val[i].name);
      lSetDouble(lep,HS_value,val[i].value);
   }

   return ltp;
}


Codine_UsageSeq* cull2UsageSeq(lList* ltp) {
   QENTER("cull2UsageSeq");
   
   lListElem* lep;

   Codine_UsageSeq* us = new Codine_UsageSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_Usage u;
      const char* dummy;
      dummy = lGetString(lep,UA_name);
      u.name = CORBA_string_dup(dummy?dummy:"");
      u.value = lGetDouble(lep,UA_value);
      us->append(u);
   }

   return us;
}


lList* UsageSeq2cull(Codine_UsageSeq& val) {
   QENTER("UsageSeq2cull");
   
   lList* ltp = lCreateList("usages", UA_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(UA_Type));
      lSetString(lep, UA_name, val[i].name);
      lSetDouble(lep, UA_value, val[i].value);
   }

   return ltp;
}


Codine_QueueingSystemSeq* cull2QueueingSystemSeq(lList* ltp) {
   QENTER("cull2QueueingSystemSeq");
   
   lListElem* lep;

   Codine_QSCommandSeq* temp_qsc;
   Codine_QueueingSystemSeq* css = new Codine_QueueingSystemSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_QueueingSystem cs;
      const char* dummy;
      dummy = lGetString(lep,CS_qs_name);
      cs.qs_name = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep,CS_transfer_queue);
      cs.transfer_queue = CORBA_string_dup(dummy?dummy:"");
      temp_qsc = cull2QSCommandSeq(lGetList(lep, CS_commands));
      cs.commands = *temp_qsc;
      delete temp_qsc;
      css->append(cs);
   }

   return css;
}


lList* QueueingSystemSeq2cull(Codine_QueueingSystemSeq& val) {
   QENTER("QueueingSystemSeq2cull");
   
   lList* ltp = lCreateList("queueing systems", CS_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(CS_Type));
      lSetString(lep, CS_qs_name, val[i].qs_name);
      lSetString(lep, CS_transfer_queue, val[i].transfer_queue);
      lSetList(lep, CS_commands, QSCommandSeq2cull(val[i].commands));
   }

   return ltp;
}

      
Codine_PathAliasSeq* cull2PathAliasSeq(lList* ltp) {
   QENTER("cull2PathAliasSeq");
   
   lListElem* lep;

   Codine_PathAliasSeq* pas = new Codine_PathAliasSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_PathAlias pa;
      const char* dummy;
      dummy = lGetString(lep,PA_origin);
      pa.origin = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep,PA_submit_host);
      pa.submit_host = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep,PA_exec_host);
      pa.exec_host = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep,PA_translation);
      pa.translation = CORBA_string_dup(dummy?dummy:"");
      pas->append(pa);
   }

   return pas;
}


lList* PathAliasSeq2cull(Codine_PathAliasSeq& val) {
   QENTER("PathAliasSeq2cull");
   
   lList* ltp = lCreateList("path aliases", PA_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(PA_Type));
      lSetString(lep, PA_origin, val[i].origin);
      lSetString(lep, PA_submit_host, val[i].submit_host);
      lSetString(lep, PA_exec_host, val[i].exec_host);
      lSetString(lep, PA_translation, val[i].translation);
   }

   return ltp;
}


Codine_QSCommandSeq* cull2QSCommandSeq(lList* ltp) {
   QENTER("cull2QSCommandSeq");
   
   lListElem* lep;

   Codine_QSCommandSeq* cos = new Codine_QSCommandSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_QSCommand co;
      const char* dummy;
      dummy = lGetString(lep,CO_name);
      co.name = CORBA_string_dup(dummy?dummy:"");
      dummy = lGetString(lep,CO_command);
      co.command = CORBA_string_dup(dummy?dummy:"");
      cos->append(co);
   }

   return cos;
}


lList* QSCommandSeq2cull(Codine_QSCommandSeq& val) {
   QENTER("QSCommandSeq2cull");
   
   lList* ltp = lCreateList("qscommands", CO_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(CO_Type));
      lSetString(lep, CO_name, val[i].name);
      lSetString(lep, CO_command, val[i].command);
   }

   return ltp;
}

Codine_RequestSeq* cull2RequestSeq(lList* ltp) {
   lListElem* lep;

   Codine_RequestSeq* rqs = new Codine_RequestSeq(lGetNumberOfElem(ltp));
   Codine_RangeSeq*   temp_range;
   Codine_ComplexEntrySeq* temp_ce;

   for_each_cpp(lep, ltp) {
      Codine_Request rq;
      temp_range = cull2RangeSeq(lGetList(lep, RE_ranges));
      rq.ranges = *temp_range;
      temp_ce = cull2ComplexEntrySeq(lGetList(lep, RE_entries));
      rq.entries = *temp_ce;
      rqs->append(rq);
      delete temp_range;
      delete temp_ce;
   }

   return rqs;
}


lList* RequestSeq2cull(Codine_RequestSeq& val) {
   lList* ltp = lCreateList("requests", RE_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(RE_Type));
      lSetList(lep, RE_ranges, RangeSeq2cull(val[i].ranges));
      lSetList(lep, RE_entries, ComplexEntrySeq2cull(val[i].entries));
   }

   return ltp;
}


Codine_SubordinateQueueSeq* cull2SubordinateQueueSeq(lList* ltp) {
   QENTER("cull2SubordinateQueueSeq");
   
   lListElem* lep;

   Codine_SubordinateQueueSeq* sos = new Codine_SubordinateQueueSeq(lGetNumberOfElem(ltp));

   for_each_cpp(lep, ltp) {
      Codine_SubordinateQueue so;
      //Codine_Queue_impl* q = Codine_Master_impl::instance()->getQueueImpl(lGetString(lep,SO_qname));
      //so.queue = q?Codine_Queue_impl::_duplicate(q):NULL;
      const char* dummy = lGetString(lep, SO_qname);
      so.qname = CORBA_string_dup(dummy?dummy:"");
      so.threshold = lGetUlong(lep,SO_threshold);
      sos->append(so);
   }

   return sos;
}


lList* SubordinateQueueSeq2cull(Codine_SubordinateQueueSeq& val) {
   QENTER("SubordinateQueueSeq2cull");
   
   lList* ltp = lCreateList("subordinate queues", SO_Type);
   lListElem* lep;

   for(CORBA_ULong i=0; i<val.length(); i++) {
      lAppendElem(ltp, lep = lCreateElem(SO_Type));
      //if(!CORBA_is_nil(val[i].queue))
      //   lSetString(lep, SO_qname, val[i].queue->get_qname(Codine_Master_impl::instance()->getMasterContext()));
      //else
      //   lSetString(lep, SO_qname, NULL);
      lSetString(lep, SO_qname, val[i].qname);
      lSetUlong(lep, SO_threshold, val[i].threshold);
   }

   return ltp;
}

Codine_TaskSeq* cull2TaskSeq(lList* tlp) {
   QENTER("cull2TaskSeq");

   lListElem*        lep;
   Codine_Queue*     tempqueue;
   Codine_UsageSeq*  tempusage;
   Codine_ParallelEnvironment* temppe;
   Codine_GrantedQueueSeq*  tempgrantedqueues;
   Codine_TaskSeq*   ts = new Codine_TaskSeq(lGetNumberOfElem(tlp));

   for_each_cpp(lep, tlp) {
      Codine_Task task;
      task.task_number = lGetUlong(lep, JAT_task_number);
      task.status = lGetUlong(lep, JAT_status) | lGetUlong(lep, JAT_state);
      task.start_time = lGetUlong(lep, JAT_start_time);
      task.hold = lGetUlong(lep, JAT_hold);
      task.job_restarted = (lGetUlong(lep, JAT_job_restarted) != 0);
      task.fshare = lGetUlong(lep, JAT_fshare);
      task.ticket = lGetUlong(lep, JAT_ticket);
      task.oticket = lGetUlong(lep, JAT_oticket);
      task.dticket = lGetUlong(lep, JAT_dticket);
      task.fticket = lGetUlong(lep, JAT_fticket);
      task.sticket = lGetUlong(lep, JAT_sticket);
      task.share = lGetDouble(lep, JAT_share);

      // need to copy the VALUE of the usage lists
      tempusage = cull2UsageSeq(lGetList(lep, JAT_usage_list));
      task.usage_list = *tempusage;
      delete tempusage;
      tempusage = cull2UsageSeq(lGetList(lep, JAT_scaled_usage_list));
      task.scaled_usage_list = *tempusage;
      delete tempusage;

      // need to copy the VALUE of the granted queue list
      tempgrantedqueues = cull2GrantedQueueSeq(lGetList(lep, JAT_granted_destin_identifier_list));
      task.granted_destin_identifier_list = *tempgrantedqueues;
      delete tempgrantedqueues;

      // find the queue from the master
      tempqueue = Codine_Master_impl::instance()->getQueueImpl(lGetString(lep, JAT_master_queue));
      if(tempqueue)
         task.master_queue = Codine_Queue_impl::_duplicate(tempqueue);
      else
         task.master_queue = Codine_Queue::_nil();

      // find pe from master
      temppe = Codine_Master_impl::instance()->getParallelEnvironmentImpl(lGetString(lep, JAT_granted_pe));
      if(temppe)
         task.granted_pe = Codine_ParallelEnvironment_impl::_duplicate(temppe);
      else
         task.granted_pe = Codine_ParallelEnvironment_impl::_nil();
         
      ts->append(task);
   }
   
   return ts;
}

Codine_GrantedQueueSeq* cull2GrantedQueueSeq(lList* gqlp) {
   QENTER("cull2GrantedQueueSeq");

   Codine_GrantedQueueSeq* gqs = new Codine_GrantedQueueSeq(lGetNumberOfElem(gqlp));
   lListElem* lep;
   lListElem* qep;
   char* dummy;
   Codine_Queue* tempqueue;

   for_each_cpp(lep, gqlp) {
      Codine_GrantedQueue gq;
      dummy = lGetString(lep, JG_qname);
      gq.qname = CORBA_string_dup(dummy?dummy:"");
      qep = lFirst(lGetList(lep, JG_queue));
      tempqueue = qep?Codine_Master_impl::instance()->getQueueImpl(lGetString(qep, QU_qname)):NULL;
      if(tempqueue)
         gq.queue = Codine_Queue_impl::_duplicate(tempqueue);
      else
         gq.queue = Codine_Queue::_nil();
      gq.slots = lGetUlong(lep, JG_slots);
      gq.ticket = lGetUlong(lep, JG_ticket);
      gq.oticket = lGetUlong(lep, JG_oticket);
      gq.fticket = lGetUlong(lep, JG_fticket);
      gq.dticket = lGetUlong(lep, JG_dticket);
      gq.sticket = lGetUlong(lep, JG_sticket);
   }

   return gqs;
}

Codine_QueueSeq* queueReferenceCull2QueueSeq(lList* qrefp) {
   QENTER("queueReferenceCull2QueueSeq");

   Codine_QueueSeq* qs = new Codine_QueueSeq;
   lListElem* ep;
   Codine_Queue_impl* temp;
   
   for_each_cpp(ep, qrefp)
      if(temp = Codine_Master_impl::instance()->getQueueImpl(lGetString(ep, QR_name)))
         qs->append(Codine_Queue_impl::_duplicate(temp));

   return qs;
}

lList* QueueSeq2queueReferenceCull(Codine_QueueSeq& val) {
   QENTER("QueueSeq2queueReferenceCull");

   lList* lp;
   lListElem* ep;

   if(!val.length())
      return NULL;

   lp = lCreateList("My Queue Reference List", QR_Type);
   for(int i=0; i<val.length(); i++) {
      ep = lCreateElem(QR_Type);
      lSetString(ep, QR_name, val[i]->get_qname(Codine_Master_impl::instance()->getMasterContext()));
      lAppendElem(lp, ep);
   }

   return lp;
}

Codine_JobSeq* jobReferenceCull2JobSeq(lList* jrefp) {
   QENTER("JobSeq2jobReferenceCull");

   Codine_JobSeq* js = new Codine_JobSeq;
   lListElem* ep;
   Codine_Job_impl* temp;
   
   for_each_cpp(ep, jrefp)
      if(temp = Codine_Master_impl::instance()->getJobImpl(lGetUlong(ep, JRE_job_number)))
         js->append(Codine_Job_impl::_duplicate(temp));

   return js;
}

lList* JobSeq2jobReferenceCull(Codine_JobSeq& val) {
   QENTER("JobSeq2jobReferenceCull");

   lList* lp;
   lListElem* ep;

   if(!val.length())
      return NULL;

   lp = lCreateList("My Job Reference List", JRE_Type);
   for(int i=0; i<val.length(); i++) {
      ep = lCreateElem(JRE_Type);
      lSetUlong(ep, JRE_job_number, val[i]->get_job_number(Codine_Master_impl::instance()->getMasterContext()));
      lAppendElem(lp, ep);
   }

   return lp;
}
