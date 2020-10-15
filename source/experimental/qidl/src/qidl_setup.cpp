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
// qidl_setup.cpp
// methods to set up the qidl daemon

#include <unistd.h>
#include <fstream.h>
#include <exception>
#include <new>

#include <OB/CORBA.h>

#include "qidl_setup.h"
#include "qmaster.h"
#include "Master_impl.h"
#include "shutdownHandler.h"
#include "qidl_common.h"
#include "qidl_c_api.h"

extern "C" {
#include "codrmon.h"
#include "cod_log.h"
#include "commlib.h"
#include "setup_path.h"
#include "msg_utilib.h"
}

#include <OB/CORBA.h>

extern volatile int shut_me_down;

// THE master corba object
static Codine_Master_impl* master = NULL;

int qidl_init() {
   return PthreadLock::initialize();
}

void cleanup_server(void* arg) {
   leave_commd();
   Codine_Master_impl::exit();
}

// this is a dummy class for the new_handler. this must be
// 1) derived from Codine_InternalError to be compattible with CORBA
// 2) derived from bad_alloc to please the C++ standard.
class MyInternalError : public Codine_InternalError,
                        public bad_alloc {
   public:
      MyInternalError(const char* err) throw();
      virtual ~MyInternalError() throw();
      virtual const char* what() const throw();
};

MyInternalError::MyInternalError(const char* err) throw() : Codine_InternalError(err) {}

MyInternalError::~MyInternalError() throw() {}

const char* MyInternalError::what() const throw() {return "";}   

// this is my new_handler
static void myNewHandler() {
   throw MyInternalError("Out of Memory");
};

// this is the entry point for the CORBA thread
// its parameter is a structure that simply contains
// the command line arguments. i need this to pass
// them to the ORB and BOA init functions.
void* start_corba_thread(void* argsp) {
   DENTER(QIDL_LAYER, "start_corba_thread");

   CORBA_BOA_var  boa;
   CORBA_ORB_var  orb;
   struct arguments* args = (struct arguments*)argsp;

   // first thing to do is to set up a new_handler
   // it throws an "InternalError" exception. if this
   // happens inside a CORBA method execution, it is
   // propagated to the client.
   // if this happens locally (e.g. when an object's
   // changed() method is called by the master thread),
   // be sure to catch this in ANY case.
   std::set_new_handler(myNewHandler);

   try {
      orb = CORBA_ORB_init(args->argc, args->argv);
      DTRACE;
      boa = orb->BOA_init(args->argc, args->argv);
      DTRACE;
      //orb->conc_model(CORBA_ORB::ConcModelReactive);
      //boa->conc_model(CORBA_BOA::ConcModelReactive);
   }
   catch(...) {
      shut_me_down = 1;
      return 0;
   }
   
   if(!orb || !boa) {
      shut_me_down = 1;
      return 0;
   }

   lock_master();
   
   // Create CORBA object
   if(!Codine_Master_impl::initialize(orb, boa)) {
      ERROR((COD_EVENT, "CORBA initialisation failed"));
      shut_me_down = 1;
      unlock_master();
      return 0;
   }

   u_short id = 1;
   prepare_enroll("cod_qidl", id, NULL);
   if(enroll()) {
      ERROR((COD_EVENT, MSG_CODTEXT_COMMPROC_ALREADY_STARTED_S, "cod_qidl"));
      shut_me_down = 1;
      unlock_master();
      return 0;
   }

   // this ensures we disenroll from commd
   // and cleanup our data
   pthread_cleanup_push(&cleanup_server, NULL);
   
   // must be valid because otherwise initialize would have returned false
   master = Codine_Master_impl::instance();
  
   // ior logging
   CORBA_String_var s = orb -> object_to_string(master);
   ofstream ior(path.master_ior_file);
   ior << s << endl;
   ior.close();

   // the Master automatically binds to the nameservice when it is
   // created, so no extra ior logging is necessary

   unlock_master();

   ShutdownHandler   handler;    // to allow safe shutdown

   try {
      boa->impl_is_ready(CORBA_ImplementationDef::_nil());
   }
   catch(...) {
      DPRINTF(("Exception in QIDL thread !!!\n"));
   }

   // we only return from here if someone sent a shutdown request to
   // the event client or if the thread was canceled
   
   pthread_cleanup_pop(1);   // execute cleanup function, i.e. leave() commd

   DEXIT;
   
   return 0;
} 
