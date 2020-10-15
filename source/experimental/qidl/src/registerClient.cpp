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
// registerClient.cpp
// a simple client that must be run as a codine job
// registers with the qidl master process
// uses DII, so no #include "Master.h" is needed
// => independence of CODINE release.
// unfortunately, ORBacus is not quite CORBA compliant with DII...
// no sophisticated error checking is done in order to clarify code

#include <OB/CORBA.h>
#include <iostream.h>
#include <unistd.h>
#include <stdlib.h>

class Dummy : public CORBA_Object_skel {};

int main(int argc, char** argv) {
   CORBA_ORB_var orb = CORBA_ORB_init(argc, argv);
   CORBA_BOA_var boa = orb->BOA_init(argc, argv);
   
   cout << "Starting up..." << endl;
   
   // intialize corba server
   // just using an empty object here
   Dummy* obj = new Dummy;

   // get the qidl reference from the environment
   const char* qidl_ref = getenv("CODINE_MASTER_IOR");
   const char* job_auth = getenv("COD_JOB_AUTH");
   if(!qidl_ref) {
      cerr << "Sorry, cannot get the master's ior." << endl;
      return 1;
   }
   if(!job_auth) {
      cerr << "Sorry, cannot get the job authentication context." << endl;
      return 1;
   }
   cout << "Master IOR is:  " << qidl_ref << endl;
   cout << "Job Context is: " << job_auth << endl;

   CORBA_Object_var qidl = orb->string_to_object(qidl_ref);
   
   // fill the DII request
   CORBA_Request_var       request;
   CORBA_NamedValue_var    result;
   request = qidl->_request("registerObject");
   request->add_in_arg() <<= obj;
   request->add_in_arg() <<= job_auth;

   // send it
   request->invoke();

   // do server stuff here
   // e.g. impl_is_ready
   cout << "Sleeping now..." << endl;
   sleep(60);
   
   // now be sure to unregister
   request = qidl->_request("unregisterObject");
   request->add_in_arg() <<= (CORBA_ULong)atol(getenv("JOB_ID"));
   request->add_in_arg() <<= obj;
   request->invoke();

   delete obj;

   cout << "Done." << endl;

   return 0;
}
