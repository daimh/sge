#ifndef __QIDL_COMMON_H
#define __QIDL_COMMON_H
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
// qidl_common.h
// common stuff for qidl


#include <pthread.h>
#include <iostream.h>
#include <map>
#include <set>

#include "Complex.h"
#include "MailRecipient.h"
#include "PathName.h"
#include "Variable.h"
#include "Range.h"
#include "ConfigEntry.h"
#include "UserEntry.h"
#include "HostLoad.h"
#include "LoadScaling.h"
#include "Usage.h"
#include "QueueingSystem.h"
#include "PathAlias.h"
#include "QSCommand.h"
#include "SubordinateQueue.h"
#include "Request.h"
#include "Task.h"
#include "GrantedQueue.h"
#include "Queue_impl.h"
#include "Job_impl.h"
#include "ParallelEnvironment_impl.h"

#ifdef HAVE_STD
using namespace std;
#endif

extern "C"
{
#include "sge_rmon.h"
#include "cull.h"
}

// stuff to auto lock and unlock the master
extern pthread_mutex_t master_lock;

// helper class to automatically lock and unlock a mutex
// allows recursive locking of mutexes in one thread
// make sure that PthreadLock::initialize() is called before the
// first usage of this class
// [for each thread, a <mutex, ref_count> map is stored. if a client tries
// to lock/unlock a given mutex, the corresponding ref_count is increased/
// decreased respectively. only if ref_count==0 is the mutex locked, and
// only if it is ref_count==1 it is unlocked.]
class PthreadLock {
   public:
      PthreadLock(pthread_mutex_t* m);
      ~PthreadLock();

      // avoid calling these unless you know what you're doing
      void     lock();
      void     unlock();
      
      static bool initialize();
   
   private:
      static bool init;

      static pthread_key_t key; // stores a map<pthread_mutex_t*, unsigned int>
      pthread_mutex_t*     mutex; // the PThreadLock object's mutex
      
      static void destroy(void* state) {delete (map<pthread_mutex_t*, unsigned int>*)state;}
};

#define AUTO_LOCK_MASTER PthreadLock lock(&master_lock);


// helper class to automatically free cull lists
class lListPtr {
   public:
      lListPtr(lList* l = NULL) : list(l), destroy(true) {}
      lListPtr(const lListPtr& l) : list(l.list), destroy(false) {}
      ~lListPtr() {if(list && destroy) lFreeList(list);}
      
      lListPtr& operator=(lList* l);
      lListPtr& operator=(lListPtr& l);
      lList**   operator &() {return &list;}
                operator lList*() {return list;}

   private:
      lList*   list;
      bool     destroy;
};


// helper class for managing method enter/leave
class Rmon {
   public:
      Rmon(const char* m);
      ~Rmon();
   private:
      const char* method;
#ifdef REENTRANCY_CHECK
      static set<const char*> entered;
      static pthread_mutex_t  set_lock;
#endif
};
#define QENTER(x) static const char SGE_FUNC[] = x; \
                  int LAYER = QIDL_LAYER; \
                  Rmon mon(SGE_FUNC);

// authentication
class CORBA_Context;
void qidl_authenticate(CORBA_Context* ctx);

// helper function to throw Errors from a given answer list
void throwErrors(lList* alp);

// converter functions
// to convert cull <-> struct

Sge_PathNameSeq*     cull2PathNameSeq(lList* ltp);
lList*                  PathNameSeq2cull(Sge_PathNameSeq& val);

Sge_MailRecipientSeq* cull2MailRecipientSeq(lList* ltp);
lList*                  MailRecipientSeq2cull(Sge_MailRecipientSeq& val);

Sge_RangeSeq*        cull2RangeSeq(lList* ltp);
lList*                  RangeSeq2cull(Sge_RangeSeq& val);

Sge_ComplexEntrySeq* cull2ComplexEntrySeq(lList* ltp);
lList*                  ComplexEntrySeq2cull(Sge_ComplexEntrySeq& val);

Sge_VariableSeq*     cull2VariableSeq(lList* ltp);
lList*                  VariableSeq2cull(Sge_VariableSeq& val);

Sge_ConfigEntrySeq*  cull2ConfigEntrySeq(lList* ltp);
lList*                  ConfigEntrySeq2cull(Sge_ConfigEntrySeq& val);

Sge_UserEntrySeq*    cull2UserEntrySeq(lList* ltp);
lList*                  UserEntrySeq2cull(Sge_UserEntrySeq& val);

Sge_HostLoadSeq*     cull2HostLoadSeq(lList* ltlp);
lList*                  HostLoadSeq2cull(Sge_HostLoadSeq& val);

Sge_LoadScalingSeq*  cull2LoadScalingSeq(lList* ltlp);
lList*                  LoadScalingSeq2cull(Sge_LoadScalingSeq& val);

Sge_UsageSeq*        cull2UsageSeq(lList* ltlp);
lList*                  UsageSeq2cull(Sge_UsageSeq& val);

Sge_QueueingSystemSeq*  cull2QueueingSystemSeq(lList* ltlp);
lList*                  QueueingSystemSeq2cull(Sge_QueueingSystemSeq& val);

Sge_SubordinateQueueSeq*  cull2SubordinateQueueSeq(lList* ltlp);
lList*                  SubordinateQueueSeq2cull(Sge_SubordinateQueueSeq& val);

Sge_PathAliasSeq*    cull2PathAliasSeq(lList* ltlp);
lList*                  PathAliasSeq2cull(Sge_PathAliasSeq& val);

Sge_RequestSeq*      cull2RequestSeq(lList* ltlp);
lList*                  RequestSeq2cull(Sge_RequestSeq& val);

Sge_QSCommandSeq*    cull2QSCommandSeq(lList* ltlp);
lList*                  QSCommandSeq2cull(Sge_QSCommandSeq& val);

Sge_sge_stringSeq*   cull2sge_stringSeq(lList* slp);
lList*                  sge_stringSeq2cull(Sge_sge_stringSeq& val);

Sge_TaskSeq*         cull2TaskSeq(lList* tlp);

Sge_GrantedQueueSeq* cull2GrantedQueueSeq(lList* gqlp);

Sge_QueueSeq*        queueReferenceCull2QueueSeq(lList* qrefp);
lList*                  QueueSeq2queueReferenceCull(Sge_QueueSeq& val);

Sge_JobSeq*          jobReferenceCull2JobSeq(lList* qrefp);
lList*                  JobSeq2jobReferenceCull(Sge_JobSeq& val);

#endif /* __QIDL_COMMON_H */
