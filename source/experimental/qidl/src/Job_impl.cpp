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
// Codine_Job_impl.cpp
// implementation for Job object

#include <pthread.h>

#include "Job_impl.h"
#include "Master_impl.h"
#include "qidl_common.h"
#include "Checkpoint_impl.h"
#include "ParallelEnvironment_impl.h"
#include "Queue_impl.h"
#include "UserProject_impl.h"
#include "UserSet_impl.h"

extern "C" {
#include "cod_api.h"
#include "cod_answerL.h"
#include "cod_queueL.h"
#include "cod_jobL.h"
#include "cod_all_listsL.h"
#include "cod_m_event.h"
#include "codrmon.h"
#include "cod_log.h"
#include "qmaster.h"
#include "read_write_queue.h"
#include "api_qmod.h"
}

#ifdef HAVE_STD
using namespace std;
#endif

extern lList* Master_Job_List;

Codine_Job_impl::Codine_Job_impl(Codine_cod_ulong _key, CORBA_ORB_var o)
   : Codine_Job_implementation(_key, o), hold_new_job(0) {
   QENTER("Codine_Job_impl::Codine_Job_impl");
   DPRINTF(("ID: %ld\n", key));
}

Codine_Job_impl::Codine_Job_impl(Codine_cod_ulong _key, const time_t& tm, CORBA_ORB_var o)
   : Codine_Job_implementation(_key, tm, o), hold_new_job(0) {
   QENTER("Codine_Job_impl::Codine_Job_impl(id)");
   DPRINTF(("ID: %ld\n", key));
   
   AUTO_LOCK_MASTER;

   self = lCreateElem(JB_Type);
   lSetUlong(self, JB_priority, BASE_PRIORITY);
}
   
Codine_Job_impl::~Codine_Job_impl() {
   QENTER("Codine_Job_impl::~Codine_Job_impl");
   DPRINTF(("ID: %ld\n", key));
   
   if(creation != 0)
      lFreeElem(&self);
}

// inherited from Codine_Object
void Codine_Job_impl::destroy(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::destroy");
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // CORBA object only ?
   if(creation != 0) {
      orb->disconnect(this);
      lFreeElem(&self);
      // The CORBA object itself will be destroyed by
      // the master, some time later
      return;
   }

   // make api request
#if 0
   lListElem *jep, *jrep;
   lListPtr jlp, alp;

   jlp = lCreateList("My Job List", JB_Type);
   jep = lCreateElem(JB_Type);

   lSetList(jep, JB_job_identifier_list, lCreateList("job_identifier_list", JRE_Type));

   jrep = lCreateElem(JRE_Type);
   lSetUlong(jrep, JRE_job_number, key);
   lAppendElem(lGetList(jep, JB_job_identifier_list), jrep);
   lAppendElem(jlp,jep);
      
   alp = cod_api(COD_JOB_LIST, COD_API_DEL, &jlp, NULL, NULL);
#else
   lListElem *idep;
   lListPtr idlp, alp;
   char id_string[256];

   sprintf(id_string, u32, key);

   idlp = lCreateList("My Job Id List", ID_Type);
   lAddElemStr(&idlp, ID_str, id_string, ID_Type);
   idep = lCreateElem(ID_Type);

   alp = cod_api(COD_JOB_LIST, COD_API_DEL, &idlp, NULL, NULL);
#endif
   throwErrors(alp);
      
   cod_api_shutdown();
}

lListElem* Codine_Job_impl::getSelf() {
   QENTER("Codine_Job_impl::getSelf");
   
   if(creation != 0)
      return self;

   // if newSelf is set, then use newSelf, because newSelf is
   // valid (if !NULL) and definitely newer than self
   if(newSelf)
      return self = newSelf;

   AUTO_LOCK_MASTER;

   lCondition* cp = lWhere("%T(%I==%u)", JB_Type, JB_job_number, key);
   self = lFindFirst(Master_Job_List, cp);
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

// submits a single-task job
void Codine_Job_impl::add(CORBA_Context* ctx) {
   submit(ctx);
}

// submits a job
// if called by the user, this submits a single-task job
// it is also called by submit_array() to submit a multi-task
// job. the ja_tasks list is then already set. we only have to
// provide the correct "hold" status of the tasks
void Codine_Job_impl::submit(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::submit");
   
   // this Job has been added already ?
   if(creation == 0)
      return;
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   // create ja_tasks if necessary
   if(hold_new_job && !lGetList(self, JB_ja_tasks))
      lSetList(self, JB_ja_tasks, lCreateList("task list", JAT_Type));

   // set hold flag for each task
   lListElem* lep;
   for_each_cpp(lep, lGetList(self, JB_ja_tasks))
      lSetUlong(lep, JAT_hold, hold_new_job);

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lEnumeration* what;
   
   lp = lCreateList("My Job List", JB_Type);
   lAppendElem(lp, self);    // This cares for automatic deletion of self(!)

   what = lWhat("%T(ALL)", JB_Type);

   // this marks the job as "the one currently added"
   creation = -1;

   // this sets this->key
   alp = cod_api(COD_JOB_LIST, COD_API_ADD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   // if we're here, everything worked fine and we can set
   // creation to 0, thus saying that we are a REAL job
   creation = 0;
}


// submits an array of tasks
void Codine_Job_impl::submit_array(Codine_cod_ulong min,
                                   Codine_cod_ulong max,
                                   Codine_cod_ulong step,
                                   CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::submit_array");
   
   // this Job has been added already ?
   if(creation == 0)
      return;
   
   // we only have to build up the task-sequence and call submit()
   // if this fails, we must clear up again and propagate the
   // exception
   lList*      lp = NULL;

   // first the ja_tasks
   for(Codine_cod_ulong i=min; i<=max; i+=step)
      lAddElemUlong(&lp, JAT_task_number, i, JAT_Type);
   lSetList(self, JB_ja_tasks, lp);

   // then the ja_structure. in my eyes, this information is
   // ABSOLUTELY redundant and unnecessary. but noone asked me...
   // ... i asked someone now: with this information, it is still
   // possible to see the ORIGINAL task structure even if some
   // tasks have finished.
   // still, at the point of job submission, this info is redundant...
   lp = lCreateList("task id range", RN_Type);
   lListElem* lep = lCreateElem(RN_Type);
   lSetUlong(lep, RN_min, min);
   lSetUlong(lep, RN_max, max);
   lSetUlong(lep, RN_step, step);
   lAppendElem(lp, lep);
   lSetList(self, JB_ja_structure, lp);

   // now submit the job
   try {
      submit(ctx);
   }
   catch(...) {
      // be sure to restore old status
      lSetList(self, JB_ja_tasks, NULL);
      lSetList(self, JB_ja_structure, NULL);
      throw;
   }

   // hey, we're back :) everything went fine.
   // note that the List created above are deleted together with
   // the "self" list in the submit() function
   // so that leaves for us to do:    ** NOTHING ** :-)
}

// hold the job
// if the optional start,end,rage arguments are not given, then the
// whole job is held. ( => that's how the CORBA hold() method invokes
// this function)
// hold_task() and hold_task_array() call this function with the appropriate
// parameters
void Codine_Job_impl::hold_impl(CORBA_Context* ctx,
                                lUlong start /* = 0 */,
                                lUlong end /* = 0 */,
                                lUlong step /* = 0*/) {
   QENTER("Codine_Job_impl::hold_impl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // if (creation != 0) then store locally and take effect only at submit()
   if(creation) {
      hold_new_job |= MINUS_H_TGT_USER;
      return;
   }

   // this is the normal case: modify a running/pending job
   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lListElem*    lep;
   lListElem*    tep;
   lEnumeration* what;
   
   lp = lCreateList("My Job List", JB_Type);
   lAppendElem(lp, lep = lCopyElem(self));

   // set all tasks if start == 0
   if(start == 0)
      for_each_cpp(tep, lGetList(lep, JB_ja_tasks))
         lSetUlong(tep, JAT_hold, lGetUlong(tep, JAT_hold) | MINUS_H_TGT_USER);
   // set only one task if end == 0
   else if(end == 0) {
      tep = lGetElemUlong(lGetList(lep, JB_ja_tasks), JAT_task_number, start);
      if(tep)
         lSetUlong(tep, JAT_hold, lGetUlong(tep, JAT_hold) | MINUS_H_TGT_USER);
   }
   // otherwise use range
   else if(start<end && step != 0)
      for(lUlong taskid=start; taskid<=end; taskid+=step) {
         tep = lGetElemUlong(lGetList(lep, JB_ja_tasks), JAT_task_number, taskid);
         if(tep)
            lSetUlong(tep, JAT_hold, lGetUlong(tep, JAT_hold) | MINUS_H_TGT_USER);
      }

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_ja_tasks);

   alp = cod_api(COD_JOB_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);
}

void Codine_Job_impl::hold(CORBA_Context* ctx) {
   hold_impl(ctx);
}

void Codine_Job_impl::hold_task(Codine_cod_ulong task_id, CORBA_Context* ctx) {
   hold_impl(ctx, task_id);
}

void Codine_Job_impl::hold_range(Codine_cod_ulong start,
                                 Codine_cod_ulong end,
                                 Codine_cod_ulong step,
                                 CORBA_Context* ctx) {
   hold_impl(ctx, start, end, step);
}

// release the whole job
// parameters as in hold_impl()
void Codine_Job_impl::release_impl(CORBA_Context* ctx,
                                   lUlong start /* = 0 */,
                                   lUlong end /* = 0 */,
                                   lUlong step /* = 0*/) {
   QENTER("Codine_Job_impl::release_impl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // if (creation != 0) then store locally and take effect only at submit()
   if(creation) {
      hold_new_job &= ~MINUS_H_TGT_USER;
      return;
   }

   // this is the normal case: modify a running/pending job
   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lListElem*    lep;
   lListElem*    tep;
   lEnumeration* what;
   
   lp = lCreateList("My Job List", JB_Type);
   lAppendElem(lp, lep = lCopyElem(self));
   
   // set all tasks if start == 0
   if(start == 0)
      for_each_cpp(tep, lGetList(lep, JB_ja_tasks))
         lSetUlong(tep, JAT_hold, lGetUlong(tep, JAT_hold) & ~MINUS_H_TGT_USER);
   // set only one task if end == 0
   else if(end == 0) {
      tep = lGetElemUlong(lGetList(lep, JB_ja_tasks), JAT_task_number, start);
      if(tep)
         lSetUlong(tep, JAT_hold, lGetUlong(tep, JAT_hold) & ~MINUS_H_TGT_USER);
   }
   // otherwise use range
   else if(start<end && step != 0)
      for(lUlong taskid=start; taskid<=end; taskid+=step) {
         tep = lGetElemUlong(lGetList(lep, JB_ja_tasks), JAT_task_number, taskid);
         if(tep)
            lSetUlong(tep, JAT_hold, lGetUlong(tep, JAT_hold) & ~MINUS_H_TGT_USER);
      }

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_ja_tasks);

   alp = cod_api(COD_JOB_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);
}

void Codine_Job_impl::release(CORBA_Context* ctx) {
   release_impl(ctx);
}

void Codine_Job_impl::release_task(Codine_cod_ulong task_id,
                                   CORBA_Context* ctx) {
   release_impl(ctx, task_id);
}

void Codine_Job_impl::release_range(Codine_cod_ulong start,
                                 Codine_cod_ulong end,
                                 Codine_cod_ulong step,
                                 CORBA_Context* ctx) {
   release_impl(ctx, start, end, step);
}

// suspend the job
// depending on the optional arguments, suspends one, many or all tasks
void Codine_Job_impl::suspend_impl(CORBA_Context* ctx,
                                   CORBA_Boolean force,
                                   lUlong start /* = 0 */,
                                   lUlong end /* = 0 */,
                                   lUlong step /* = 0 */) {
   QENTER("Codine_Job_impl::suspend_impl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr me;
   lListPtr alp;
   lListElem* lep;
   char     str_id[20];

   // whole job if start == 0
   if(start == 0)
      sprintf(str_id, u32, key);
   // single task if end == 0
   else if(end == 0)
      sprintf(str_id, u32"."u32, key, start);
   // otherwise use range
   else
      sprintf(str_id, u32"."u32"-"u32":"u32, key, start, end, step);

   me = lCreateList("suspend me", ST_Type);
   lAppendElem(me, lep = lCreateElem(ST_Type));
   lSetString(lep, STR, str_id);
   alp = api_qmod(me, force, QSUSPENDED);
   throwErrors(alp);
}

void Codine_Job_impl::suspend(CORBA_Boolean force,
                              CORBA_Context* ctx) {
   suspend_impl(ctx, force);
}

void Codine_Job_impl::suspend_task(Codine_cod_ulong task_id,
                                   CORBA_Boolean force,
                                   CORBA_Context* ctx) {
   suspend_impl(ctx, force, task_id);
}

void Codine_Job_impl::suspend_range(Codine_cod_ulong start,
                                    Codine_cod_ulong end,
                                    Codine_cod_ulong step,
                                    CORBA_Boolean force,
                                    CORBA_Context* ctx) {
   suspend_impl(ctx, force, start, end, step);
}

// unsuspend the job
void Codine_Job_impl::unsuspend_impl(CORBA_Context* ctx,
                                     CORBA_Boolean force,
                                     lUlong start /* = 0 */,
                                     lUlong end /* = 0 */,
                                     lUlong step /* = 0 */) {
   QENTER("Codine_Job_impl::unsuspend_impl");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr me;
   lListPtr alp;
   lListElem* lep;
   char     str_id[20];
   
   // whole job if start == 0
   if(start == 0)
      sprintf(str_id, u32, key);
   // single task if end == 0
   else if(end == 0)
      sprintf(str_id, u32"."u32, key, start);
   // otherwise use range
   else
      sprintf(str_id, u32"."u32"-"u32":"u32, key, start, end, step);

   me = lCreateList("suspend me", ST_Type);
   lAppendElem(me, lep = lCreateElem(ST_Type));
   lSetString(lep, STR, str_id);
   alp = api_qmod(me, force, QRUNNING);
   throwErrors(alp);
}

void Codine_Job_impl::unsuspend(CORBA_Boolean force,
                                CORBA_Context* ctx) {
   unsuspend_impl(ctx, force);
}

void Codine_Job_impl::unsuspend_task(Codine_cod_ulong task_id,
                                     CORBA_Boolean force,
                                     CORBA_Context* ctx) {
   unsuspend_impl(ctx, force, task_id);
}

void Codine_Job_impl::unsuspend_range(Codine_cod_ulong start,
                                      Codine_cod_ulong end,
                                      Codine_cod_ulong step,
                                      CORBA_Boolean force,
                                      CORBA_Context* ctx) {
   unsuspend_impl(ctx, force, start, end, step);
}

Codine_cod_ulong Codine_Job_impl::get_job_number(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::get_job_number");
   
   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();
   
   Codine_cod_ulong foo = key;
   
   return foo;
}

Codine_ParallelEnvironment* Codine_Job_impl::get_pe_object(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::get_pe_object");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   Codine_ParallelEnvironment_impl* pe = Codine_Master_impl::instance()->getParallelEnvironmentImpl(lGetString(self, JB_pe));

   return pe?(Codine_ParallelEnvironment_impl::_duplicate(pe)):NULL;
}
    
    
Codine_cod_ulong Codine_Job_impl::set_pe_object(Codine_ParallelEnvironment* val, CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::set_pe_object");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lListElem*    ep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My Job List", JB_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }
      
   lSetString(ep, JB_pe, val->get_name(ctx));

   if(creation != 0)        // that's it for a local (= new) object
      return 0;

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_pe);

   alp = cod_api(COD_JOB_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   return lastEvent;
}

Codine_QueueSeq* Codine_Job_impl::get_hard_queue_list(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::get_hard_queue_list");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   return queueReferenceCull2QueueSeq(lGetList(self, JB_hard_queue_list));
}
    
Codine_cod_ulong Codine_Job_impl::set_hard_queue_list(const Codine_QueueSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::set_hard_queue_list");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lListElem*    ep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My Job List", JB_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }
      
   lSetList(ep, JB_hard_queue_list, QueueSeq2queueReferenceCull((Codine_QueueSeq&)val));

   if(creation != 0)        // that's it for a local (= new) object
      return 0;

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_pe);

   alp = cod_api(COD_JOB_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   return lastEvent;
}

Codine_QueueSeq* Codine_Job_impl::get_soft_queue_list(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::get_soft_queue_list");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   return queueReferenceCull2QueueSeq(lGetList(self, JB_soft_queue_list));
}
    
Codine_cod_ulong Codine_Job_impl::set_soft_queue_list(const Codine_QueueSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::set_soft_queue_list");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lListElem*    ep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My Job List", JB_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }
      
   lSetList(ep, JB_soft_queue_list, QueueSeq2queueReferenceCull((Codine_QueueSeq&)val));

   if(creation != 0)        // that's it for a local (= new) object
      return 0;

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_pe);

   alp = cod_api(COD_JOB_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   return lastEvent;
}

Codine_Checkpoint* Codine_Job_impl::get_checkpoint_object(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::get_checkpoint_object");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   Codine_Checkpoint_impl* ckpt = Codine_Master_impl::instance()->getCheckpointImpl(lGetString(self, JB_checkpoint_object));

   return ckpt?(Codine_Checkpoint_impl::_duplicate(ckpt)):NULL;
}

Codine_cod_ulong Codine_Job_impl::set_checkpoint_object(Codine_Checkpoint* val, CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::set_checkpoint_object");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lListElem*    ep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My Job List", JB_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }
      
   lSetString(ep, JB_checkpoint_object, val->get_name(ctx));

   if(creation != 0)        // that's it for a local (= new) object
      return 0;

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_checkpoint_object);

   alp = cod_api(COD_JOB_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   return lastEvent;
}     


Codine_JobSeq* Codine_Job_impl::get_jid_predecessor_list(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::get_jid_predecessor_list");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   return jobReferenceCull2JobSeq(lGetList(self, JB_jid_predecessor_list));
}

Codine_cod_ulong Codine_Job_impl::set_jid_predecessor_list(const Codine_JobSeq& val, CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::set_jid_predecessor_list");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lListElem*    ep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My Job List", JB_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }
      
   lSetList(ep, JB_jid_predecessor_list, JobSeq2jobReferenceCull((Codine_JobSeq&)val));

   if(creation != 0)        // that's it for a local (= new) object
      return 0;

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_project);

   alp = cod_api(COD_JOB_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   return lastEvent;
}     

Codine_UserProject* Codine_Job_impl::get_project(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::get_project");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   Codine_Project_impl* proj = Codine_Master_impl::instance()->getProjectImpl(lGetString(self, JB_project));

   return proj?(Codine_Project_impl::_duplicate(proj)):NULL;
}

Codine_cod_ulong Codine_Job_impl::set_project(Codine_UserProject* val, CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::set_project");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   // make api request
   lListPtr      lp;
   lListPtr      alp;
   lListElem*    ep;
   lEnumeration* what;
   
   if(creation != 0)
      ep = self;
   else {
      lp = lCreateList("My Job List", JB_Type);
      lAppendElem(lp, ep = lCopyElem(self));
   }
      
   lSetString(ep, JB_project, val->get_name(ctx));

   if(creation != 0)        // that's it for a local (= new) object
      return 0;

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_project);

   alp = cod_api(COD_JOB_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);

   throwErrors(alp);

   return lastEvent;
}     
 
Codine_UserSet* Codine_Job_impl::get_department(CORBA_Context* ctx) {
   QENTER("Codine_Job_impl::get_departement");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   getSelf();

   Codine_UserSet_impl* dep = Codine_Master_impl::instance()->getUserSetImpl(lGetString(self, JB_department));

   return dep?(Codine_UserSet_impl::_duplicate(dep)):NULL;
}

void Codine_Job_impl::changed(lListElem* _newSelf) {
   QENTER("Codine_Job_impl::changed");
   AUTO_LOCK_MASTER;

   // set self variable for now
   // setting newself forces getSelf() NOT to query for myself in
   // the master list
   self = newSelf = _newSelf;

   // store event counter locally, to pass it back to the client later
   // This works as long as there is only one CORBA thread, so there
   // can be only one request dispatched at one time.
   // If more client requests were able to execute simultanously,
   // they would overwrite this local variable => !&%$&/�$�%
   lastEvent = qidl_event_count;

   // get out new job id
   key = lGetUlong(self, JB_job_number);

   // build header
   Codine_event ev;
   ev.type = Codine_ev_mod;
   ev.obj  = COD_JOB_LIST;
   ev.name = CORBA_string_dup(""); // not needed for Job
   ev.id   = key;
   ev.ref  = Codine_Job_impl::_duplicate(this);
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
