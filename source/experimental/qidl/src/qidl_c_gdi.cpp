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
// qidl_c_api.cpp
// implements c-style interface functions for qmaster

#include <unistd.h>

#include "Master_impl.h"
#include "Queue_impl.h"
#include "Job_impl.h"
#include "Complex_impl.h"
#include "Configuration_impl.h"
#include "ParallelEnvironment_impl.h"
#include "Checkpoint_impl.h"
#include "Calendar_impl.h"
#include "ExecHost_impl.h"
#include "UserSet_impl.h"
#include "UserProject_impl.h"
#include "SchedConf_impl.h"

#include "qidl_c_api.h"
#include "qidl_common.h"
#include "shutdownHandler.h"

extern "C" {
#include "cull.h"
#include "cod_all_listsL.h"
#include "cod_log.h"
#include "codrmon.h"
}


// all of the following functions provide a gateway for the C core
// code to call functions of the C++ objects
// this would otherwise be impossible due to different linkage
// all of them have exception handlers because the calling C code
// cannot cope with these otherwise and would crash badly.

// locking functions
pthread_mutex_t  master_lock = PTHREAD_MUTEX_INITIALIZER;

void lock_master() {
   QENTER("lock_master");
   PthreadLock lock(&master_lock);
   lock.lock();
}

void unlock_master() {
   QENTER("unlock_master");
   PthreadLock lock(&master_lock);
   lock.unlock();
}

// object creation/deletion
void deleteObjectByName(int type, const char* name) {
   QENTER("deleteObjectByName");

   try {
      if(Codine_Master_impl::instance())
         Codine_Master_impl::instance()->deleteObject(type, name);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void deleteObjectByID(int type, lUlong id) {
   QENTER("deleteObjectByID");

   try {
      if(Codine_Master_impl::instance())
         Codine_Master_impl::instance()->deleteObject(type, id);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void addObjectByName(int type, const char* name) {
   QENTER("addObjectByName");

   try {
      if(Codine_Master_impl::instance())
         Codine_Master_impl::instance()->addObject(type, name);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void addObjectByID(int type, lUlong id) {
   QENTER("addObjectByID");

   try {
      if(Codine_Master_impl::instance())
         Codine_Master_impl::instance()->addObject(type, id);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

// misc
void shutdownQIDL() {
   QENTER("shutdownQIDL");

   char c;

   if(ShutdownHandler::fd[1] > 1)
      write(ShutdownHandler::fd[1], &c, 1);
}

const char* get_master_ior() {
   QENTER("get_master_ior");

   if(Codine_Master_impl::instance())
      return Codine_Master_impl::instance()->getIOR();
   else
      return NULL;
}

// authentication
extern cod_auth qidl_me;
struct cod_auth* get_qidl_me() {
   QENTER("get_qidl_me");

   return &qidl_me;
}

// event handling
void queue_changed(lListElem* ep) {
   QENTER("queue_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_Queue_impl* q = Codine_Master_impl::instance()->getQueueImpl(lGetString(ep, QU_qname));

      if(q)
         q->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void job_changed(lListElem* ep) {
   QENTER("job_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_Job_impl* j = Codine_Master_impl::instance()->getJobImpl(lGetUlong(ep, JB_job_number));

      if(j)
         j->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void complex_changed(lListElem* ep) {
   QENTER("complex_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_Complex_impl* c = Codine_Master_impl::instance()->getComplexImpl(lGetString(ep, CX_name));

      if(c)
         c->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void configuration_changed(lListElem* ep) {
   QENTER("configuration_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_Configuration_impl* c = Codine_Master_impl::instance()->getConfigurationImpl(lGetString(ep, CONF_hname));

      if(c)
         c->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void calendar_changed(lListElem* ep) {
   QENTER("calendar_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_Calendar_impl* cal = Codine_Master_impl::instance()->getCalendarImpl(lGetString(ep, CAL_name));

      if(cal)
         cal->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void checkpoint_changed(lListElem* ep) {
   QENTER("checkpoint_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_Checkpoint_impl* ck = Codine_Master_impl::instance()->getCheckpointImpl(lGetString(ep, CK_name));

      if(ck)
         ck->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void parallelenvironment_changed(lListElem* ep) {
   QENTER("parallelenvironment_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_ParallelEnvironment_impl* pe = Codine_Master_impl::instance()->getParallelEnvironmentImpl(lGetString(ep, PE_name));

      if(pe)
         pe->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void exechost_changed(lListElem* ep) {
   QENTER("exechost_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_ExecHost_impl* eh = Codine_Master_impl::instance()->getExecHostImpl(lGetString(ep, EH_name));

      if(eh)
         eh->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void schedconf_changed(lListElem* ep) {
   QENTER("schedconf_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

     Codine_SchedConf_impl* sc = Codine_Master_impl::instance()->getSchedConfImpl(lGetString(ep, SC_algorithm));

      if(sc)
         sc->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}


void user_changed(lListElem* ep) {
   QENTER("user_changed");
   
   try {
      if(!Codine_Master_impl::instance())
         return;

     Codine_User_impl* up = Codine_Master_impl::instance()->getUserImpl(lGetString(ep, UP_name));

      if(up)
         up->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void project_changed(lListElem* ep) {
   QENTER("project_changed");
   
   try {
      if(!Codine_Master_impl::instance())
         return;

     Codine_Project_impl* up = Codine_Master_impl::instance()->getProjectImpl(lGetString(ep, UP_name));

      if(up)
         up->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void sharetree_changed(lListElem* ep)
{
   QENTER("sharetree_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_Master_impl::instance()->updateShareTree(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}

void userset_changed(lListElem* ep) {
   QENTER("userset_changed");

   try {
      if(!Codine_Master_impl::instance())
         return;

      Codine_UserSet_impl* us = Codine_Master_impl::instance()->getUserSetImpl(lGetString(ep, US_name));

      if(us)
         us->changed(ep);
   }
   catch(...) {
      //ERROR((COD_EVENT, "Caught unhandled exception"));
   }
}
