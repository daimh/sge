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
// Master_impl.cpp
// Implementation of Codine_Master object

#include <time.h>
#include <list>
#include <algorithm>
#include <functional>
#include <unistd.h>
#include <OB/CORBA.h>
#include <OB/Util.h>

#include "Master_impl.h"
#include "Queue_impl.h"
#include "Complex_impl.h"
#include "Job_impl.h"
#include "Configuration_impl.h"
#include "Calendar_impl.h"
#include "Checkpoint_impl.h"
#include "ParallelEnvironment_impl.h"
#include "UserSet_impl.h"
#include "ExecHost_impl.h"
#include "UserProject_impl.h"
#include "SchedConf_impl.h"
#include "ShareTreeNode_impl.h"
#include "qidl_common.h"

extern "C" {
#include "cull.h"
#include "cod_api_intern.h"
#include "cod_complexL.h"
#include "cod_log.h"
#include "cod_me.h"
#include "cod_queueL.h"
#include "cod_jobL.h"
#include "cod_calendarL.h"
#include "cod_ckptL.h"
#include "cod_peL.h"
#include "cod_usersetL.h"
#include "cod_hostL.h"
#include "cod_userprjL.h"
#include "cod_schedconfL.h"
#include "cod_share_tree_nodeL.h"
#include "cod_manopL.h"
#include "codrmon.h"
#include "qmaster.h"
}

#ifdef HAVE_STD
using namespace std;
#endif

extern lList* Master_Queue_List;
extern lList* Master_Complex_List;
extern lList* Master_Job_List;
extern lList* Master_Config_List;
extern lList* Master_Calendar_List;
extern lList* Master_Ckpt_List;
extern lList* Master_Pe_List;
extern lList* Master_Userset_List;
extern lList* Master_Exechost_List;
extern lList* Master_User_List;
extern lList* Master_Project_List;
extern lList* Master_Sched_Config_List;
extern lList* Master_Sharetree_List;
extern lList* Master_Adminhost_List;
extern lList* Master_Submithost_List;
extern lList* Master_Manager_List;
extern lList* Master_Operator_List;

// static class members
Codine_Master_impl* Codine_Master_impl::master = NULL;
CORBA_BOA_var Codine_Master_impl::boa;
CORBA_ORB_var Codine_Master_impl::orb;
list<Codine_Queue_impl*> Codine_Master_impl::queues;
list<Codine_Complex_impl*> Codine_Master_impl::complexes;
list<Codine_Job_impl*> Codine_Master_impl::jobs;
list<Codine_Configuration_impl*> Codine_Master_impl::configurations;
list<Codine_Calendar_impl*> Codine_Master_impl::calendars;
list<Codine_Checkpoint_impl*> Codine_Master_impl::checkpoints;
list<Codine_ParallelEnvironment_impl*> Codine_Master_impl::parallelenvironments;
list<Codine_UserSet_impl*> Codine_Master_impl::usersets;
list<Codine_ExecHost_impl*> Codine_Master_impl::exechosts;
list<Codine_User_impl*> Codine_Master_impl::users;
list<Codine_Project_impl*> Codine_Master_impl::projects;
Codine_SchedConf_impl* Codine_Master_impl::schedconf;
Codine_ShareTreeNode_impl* Codine_Master_impl::sharetree;

bool Codine_Master_impl::ev_flag = true;
deque<CORBA_Any> Codine_Master_impl::events;
pthread_mutex_t Codine_Master_impl::events_lock = PTHREAD_MUTEX_INITIALIZER;
CosNaming_Name_var Codine_Master_impl::ns_name;
CosNaming_NamingContext_var Codine_Master_impl::ns;
CosEventChannelAdmin_EventChannel_var Codine_Master_impl::es;
CosEventChannelAdmin_ProxyPushConsumer_var Codine_Master_impl::ev_consumer;
int Codine_Master_impl::obEventFd[2] = {-1, -1};
CORBA_Context_var Codine_Master_impl::context;
CORBA_String_var Codine_Master_impl::ior;



Codine_Master_impl::Codine_Master_impl(CORBA_ORB_ptr o, CORBA_BOA_ptr b) {
   QENTER("Master::Master");

   if(!master) {
      boa = CORBA_BOA::_duplicate(b);
      orb = CORBA_ORB::_duplicate(o);
      orb->connect(this, "QIDL Server");
      master = (Codine_Master_impl*)this;

      // register with nameservice
      try {
         CORBA_Object_var ns_obj = orb->resolve_initial_references("NameService");
         if(CORBA_is_nil(ns_obj))
            throw 0;
         ns = CosNaming_NamingContext::_narrow(ns_obj);
         if(CORBA_is_nil(ns))
            throw 0;
         ns_name = new CosNaming_Name();
         //CosNaming_NameComponent_var nc = new CosNaming_NameComponent();
         //ns_name->append(nc);
         ns_name->length(1);
         ns_name[0].id = CORBA_string_dup(me.default_cell);
         ns_name[0].kind = CORBA_string_dup("");
         try {
            ns->bind_new_context(ns_name);
         }
         catch(...) {
            // this happens if already bound
         }
         ns_name->length(2);
         ns_name[1].id = CORBA_string_dup("cod_qidl");
         ns_name[1].kind = CORBA_string_dup("");
         ns->rebind(ns_name, this);
      }
      catch(...) {
         cerr << "naming service problems..." << endl;
      }

      // get event service
      try {
         connectEventService();

         // OBEventHandler stuff
         if(pipe(obEventFd) == -1)
            throw 0;
         OBReactor::instance()->registerHandler(this, OBEventRead, obEventFd[0]);
      }
      catch(...) {
         cerr << "event service problems..." << endl;
      }
   }
}

Codine_Master_impl::~Codine_Master_impl() {
   QENTER("Master::~Master");

   AUTO_LOCK_MASTER;

   try {
      // if(!master) delete all;
      // ...  why NOT master ???
      // that's because no-one ever calls this destructor directly
      // only when all CORBA references are released(), we come here.
      // and this can only happen, when someone calls this->exit();
      // and this->exit() sets master to NULL, thus signalling:
      // "Hey, it's OK to delete all data structures."
      if(!master) {
         // delete queues
         for(list<Codine_Queue_impl*>::iterator q=queues.begin(); q!=queues.end(); ++q) {
            orb->disconnect(*q);
            CORBA_release(*q);
         }
         
         // delete jobs
         for(list<Codine_Job_impl*>::iterator j=jobs.begin(); j!=jobs.end(); ++j) {
            orb->disconnect(*j);
            CORBA_release(*j);
         }
         
         // delete complexes
         for(list<Codine_Complex_impl*>::iterator c=complexes.begin(); c!=complexes.end(); ++c) {
            orb->disconnect(*c);
            CORBA_release(*c);
         }
         
         // delete configurations
         for(list<Codine_Configuration_impl*>::iterator conf=configurations.begin(); conf!=configurations.end(); ++conf) {
            orb->disconnect(*conf);
            CORBA_release(*conf);
         }
         
         // delete calendars
         for(list<Codine_Calendar_impl*>::iterator cal = calendars.begin();cal!=calendars.end();++cal) {
            orb->disconnect(*cal);
            CORBA_release(*cal);
         }
         
         // delete checkpoints
         for(list<Codine_Checkpoint_impl*>::iterator ckpt = checkpoints.begin();ckpt!=checkpoints.end();++ckpt) {
            orb->disconnect(*ckpt);
            CORBA_release(*ckpt);
         }
         
         // delete parallelenvironments
         for(list<Codine_ParallelEnvironment_impl*>::iterator pe = parallelenvironments.begin();pe!=parallelenvironments.end();++pe) {
            orb->disconnect(*pe);
            CORBA_release(*pe);
         }
         
         // delete usersets
         for(list<Codine_UserSet_impl*>::iterator us = usersets.begin();us!=usersets.end();++us) {
            orb->disconnect(*us);
            CORBA_release(*us);
         }
         
         // delete exechosts
         for(list<Codine_ExecHost_impl*>::iterator eh = exechosts.begin();eh!=exechosts.end();++eh) {
            orb->disconnect(*eh);
            CORBA_release(*eh);
         }
         
         // delete users
         for(list<Codine_User_impl*>::iterator user = users.begin();user!=users.end();++user) {
            orb->disconnect(*user);
            CORBA_release(*user);
         }
         
         // delete projects
         for(list<Codine_Project_impl*>::iterator prj = projects.begin();prj!=projects.end();++prj) {
            orb->disconnect(*prj);
            CORBA_release(*prj);
         }
         
         // delete sharetree
         //delete sharetree;
         if (sharetree) { 
            orb->disconnect(sharetree);
            CORBA_release(sharetree);
         }
         
         // delete schedconf
         if (schedconf) {
            orb->disconnect(schedconf);
            CORBA_release(schedconf);
         }
         
         // unregister from Reactor
         if(!CORBA_is_nil(es)) {
            close(obEventFd[0]);
            close(obEventFd[1]);
            OBReactor::instance()->unregisterHandler(this);
         }

         // unbind and go home
         if(!CORBA_is_nil(ns) && ns_name->length() == 2) 
            ns->unbind(ns_name);
         boa->deactivate_impl(NULL);
      }
   }
   catch (...) {
      DPRINTF(("exception when dying...\n"));
      // die gracefully...
   }
}

// shutdown
// shuts the server down
// sub objects are freed in the d'tor
void Codine_Master_impl::shutdown(CORBA_Context_ptr ctx) {
   QENTER("Master::shutdown");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   exit();
}

// exit
// same as shutdown. static function, non-idl
void Codine_Master_impl::exit() {
   QENTER("Master::exit");

   AUTO_LOCK_MASTER;

   if(!master)
      return;

   Codine_Master_impl* temp = master;
   master = NULL;
   orb->disconnect(temp);
   CORBA_release(temp);
}

// instance
// returns the singleton
// make sure that this is never called before initialize()
// or when initialize() failed, otherwise returns a NULL
// pointer...
Codine_Master_impl* Codine_Master_impl::instance() {
   return master;
}

// getIOR
// returns the stringified object reference of the singleton
const char* Codine_Master_impl::getIOR() {
   QENTER("Master::getIOR");

   if(!(const char*)ior)
      ior = CORBA_string_dup(orb->object_to_string(this));

   return (const char*)ior;
}

// initialize
// inits all data structures and sub-objects
bool Codine_Master_impl::initialize(CORBA_ORB_ptr o, CORBA_BOA_ptr b) {
   QENTER("Master::initialize");

   AUTO_LOCK_MASTER;

   try {
      master = new Codine_Master_impl(o, b);

      if(!master)
         return false;

      lListElem*     lep;
      
      // queues
      queues.clear();
      for_each_cpp(lep, Master_Queue_List) {
         Codine_Queue_impl* q = new Codine_Queue_impl(lGetString(lep, QU_qname), orb);
         queues.push_back(q);
         orb->connect(q);
      }

      // now for the complexes
      complexes.clear();
      for_each_cpp(lep, Master_Complex_List) {
         Codine_Complex_impl* c = new Codine_Complex_impl(lGetString(lep, CX_name), orb);
         complexes.push_back(c);
         orb->connect(c);
      }
      
      // now for the jobs
      jobs.clear();
      for_each_cpp(lep, Master_Job_List) {
         Codine_Job_impl* j = new Codine_Job_impl(lGetUlong(lep, JB_job_number), orb);
         jobs.push_back(j);
         orb->connect(j);
      }

      // now for the configs
      configurations.clear();
      for_each_cpp(lep, Master_Config_List) {
         Codine_Configuration_impl* c = new Codine_Configuration_impl(lGetString(lep, CONF_hname), orb);
         configurations.push_back(c);
         orb->connect(c);
      }
      
      // calendars
      calendars.clear();
      for_each_cpp(lep, Master_Calendar_List) {
         Codine_Calendar_impl* cal = new Codine_Calendar_impl(lGetString(lep, CAL_name), orb);
         calendars.push_back(cal);
         orb->connect(cal);
      }
      
      //checkpoints
      checkpoints.clear();
      for_each_cpp(lep, Master_Ckpt_List) {
         Codine_Checkpoint_impl* ckpt = new Codine_Checkpoint_impl(lGetString(lep, CK_name), orb);
         checkpoints.push_back(ckpt);
         orb->connect(ckpt);
      }
      
      //parallelenvironments
      parallelenvironments.clear();
      for_each_cpp(lep, Master_Pe_List) {
         Codine_ParallelEnvironment_impl* pe = new Codine_ParallelEnvironment_impl(lGetString(lep, PE_name), orb);
         parallelenvironments.push_back(pe);
         orb->connect(pe);
      }
      
      //usersets
      usersets.clear();
      for_each_cpp(lep, Master_Userset_List) {
         Codine_UserSet_impl* us = new Codine_UserSet_impl(lGetString(lep, US_name), orb);
         usersets.push_back(us);
         orb->connect(us);
      }
      
      //exechosts
      exechosts.clear();
      for_each_cpp(lep, Master_Exechost_List) {
         Codine_ExecHost_impl* eh = new Codine_ExecHost_impl(lGetString(lep, EH_name), orb);
         exechosts.push_back(eh);
         orb->connect(eh);
      }
      
      //users
      users.clear();
      for_each_cpp(lep, Master_User_List) {
         Codine_User_impl* up = new Codine_User_impl(lGetString(lep, UP_name), orb);
         users.push_back(up);
         orb->connect(up);
      }
      
      //projects
      projects.clear();
      for_each_cpp(lep, Master_Project_List) {
         Codine_Project_impl* up = new Codine_Project_impl(lGetString(lep, UP_name), orb);
         projects.push_back(up);
         orb->connect(up);
      }
      
      //sharetree
      lListElem* root = lFirst(Master_Sharetree_List);
      if(root) {
         sharetree = new Codine_ShareTreeNode_impl(lGetString(root, SN_name), root, orb);
         orb->connect(sharetree);
      }
      else
         sharetree = NULL;

      //schedconf
      if(lFirst(Master_Sched_Config_List)) {
         schedconf = new Codine_SchedConf_impl(lGetString(lFirst(Master_Sched_Config_List), SC_algorithm), orb);
         orb->connect(schedconf);
      }
      else
         schedconf = NULL;
   }
   catch(...) {
      // be sure to clean up properly...
      Codine_Master_impl::exit();
      
      return false;
   }

   // uh, done.
   return true;
}


// getQueues()
Codine_QueueSeq* Codine_Master_impl::getQueues() {
   QENTER("Master::getQueues");

   AUTO_LOCK_MASTER;

   Codine_QueueSeq* qs = new Codine_QueueSeq();
   list<Codine_Queue_impl*>::iterator it=queues.begin();
   for(; it != queues.end(); ++it)
      if((*it)->creation == 0) 
         qs->append(Codine_Queue_impl::_duplicate(*it)); 
   return qs;
}

Codine_QueueSeq* Codine_Master_impl::getQueues(CORBA_Context_ptr ctx) {
   QENTER("Master::getQueues(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getQueues();
}

// getComplexes()
Codine_ComplexSeq* Codine_Master_impl::getComplexes() {
   QENTER("Master::getComplexes");

   AUTO_LOCK_MASTER;

   Codine_ComplexSeq* cs = new Codine_ComplexSeq();
   for(list<Codine_Complex_impl*>::iterator it=complexes.begin(); it != complexes.end(); ++it)
      if((*it)->creation == 0)
         cs->append(Codine_Complex_impl::_duplicate(*it));

   return cs;
}

Codine_ComplexSeq* Codine_Master_impl::getComplexes(CORBA_Context* ctx) {
   QENTER("Master::getComplexes(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getComplexes();
}

// getJobs()
Codine_JobSeq* Codine_Master_impl::getJobs() {
   QENTER("Master::getJobs");

   AUTO_LOCK_MASTER;

   Codine_JobSeq* js = new Codine_JobSeq();
   for(list<Codine_Job_impl*>::iterator it=jobs.begin(); it != jobs.end(); ++it)
      if((*it)->creation == 0)
         js->append(Codine_Job_impl::_duplicate(*it));  
         
   return js;
}

Codine_JobSeq* Codine_Master_impl::getJobs(CORBA_Context* ctx) {
   QENTER("Master::getJobs(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getJobs();
}

// getConfigurations()
Codine_ConfigurationSeq* Codine_Master_impl::getConfigurations() {
   QENTER("Master::getConfiguration");

   AUTO_LOCK_MASTER;

   Codine_ConfigurationSeq* cs = new Codine_ConfigurationSeq();
   for(list<Codine_Configuration_impl*>::iterator it=configurations.begin(); it != configurations.end(); ++it)
      if((*it)->creation == 0)
         cs->append(Codine_Configuration_impl::_duplicate(*it));

   return cs;
}

Codine_ConfigurationSeq* Codine_Master_impl::getConfigurations(CORBA_Context* ctx) {
   QENTER("Master::getConfigurations(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getConfigurations();
}

// getCalendars()
Codine_CalendarSeq* Codine_Master_impl::getCalendars() {
   QENTER("Master::getCalendars");

   AUTO_LOCK_MASTER;

   Codine_CalendarSeq* cas = new Codine_CalendarSeq();
   for(list<Codine_Calendar_impl*>::iterator it=calendars.begin(); it != calendars.end(); ++it)
      if((*it)->creation == 0) 
         cas->append(Codine_Calendar_impl::_duplicate(*it));  // I don't know
                                                         // if the _duplicate
                                                         // is really neccessary
                                                         // have to check...
   return cas;
}

Codine_CalendarSeq* Codine_Master_impl::getCalendars(CORBA_Context* ctx) {
   QENTER("Master::getCalendars(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getCalendars();
}

// getCheckpoints()
Codine_CheckpointSeq* Codine_Master_impl::getCheckpoints() {
   QENTER("Master::getCheckpoints");

   AUTO_LOCK_MASTER;

   Codine_CheckpointSeq* cks = new Codine_CheckpointSeq();
   for(list<Codine_Checkpoint_impl*>::iterator it=checkpoints.begin(); it != checkpoints.end(); ++it)
      if((*it)->creation == 0) 
         cks->append(Codine_Checkpoint_impl::_duplicate(*it));                                      
                                      
   return cks;
}

Codine_CheckpointSeq* Codine_Master_impl::getCheckpoints(CORBA_Context* ctx) {
   QENTER("Master::getCheckpoints(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getCheckpoints();
}

// getParallelEnvironments()
Codine_ParallelEnvironmentSeq* Codine_Master_impl::getParallelEnvironments() {
   QENTER("Master::getParallelEnvironments");

   AUTO_LOCK_MASTER;

   Codine_ParallelEnvironmentSeq* pes = new Codine_ParallelEnvironmentSeq();
   for(list<Codine_ParallelEnvironment_impl*>::iterator it= parallelenvironments.begin(); it != parallelenvironments.end(); ++it)
      if((*it)->creation == 0) 
         pes->append(Codine_ParallelEnvironment_impl::_duplicate(*it));                                      
                                      
   return pes;
}

Codine_ParallelEnvironmentSeq* Codine_Master_impl::getParallelEnvironments(CORBA_Context* ctx) {
   QENTER("Master::getParallelEnvironments(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getParallelEnvironments();
}

// getUserSets()
Codine_UserSetSeq* Codine_Master_impl::getUserSets() {
   QENTER("Master::getUserSets");

   AUTO_LOCK_MASTER;

   Codine_UserSetSeq* uss = new Codine_UserSetSeq();
   for(list<Codine_UserSet_impl*>::iterator it=usersets.begin(); it != usersets.end(); ++it)
      if((*it)->creation == 0) 
         uss->append(Codine_UserSet_impl::_duplicate(*it));                                      
                                      
   return uss;
}

Codine_UserSetSeq* Codine_Master_impl::getUserSets(CORBA_Context* ctx) {
   QENTER("Master::getUserSets(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getUserSets();
}

// getExecHosts()
Codine_ExecHostSeq* Codine_Master_impl::getExecHosts() {
   QENTER("Master::getExecHosts");

   AUTO_LOCK_MASTER;

   Codine_ExecHostSeq* ehs = new Codine_ExecHostSeq();

   for(list<Codine_ExecHost_impl*>::iterator it=exechosts.begin(); it != exechosts.end(); ++it)
      if((*it)->creation == 0) 
         ehs->append(Codine_ExecHost_impl::_duplicate(*it));                                      
                                      
   return ehs;
}

Codine_ExecHostSeq* Codine_Master_impl::getExecHosts(CORBA_Context* ctx) {
   QENTER("Master::getExecHosts(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getExecHosts();
}

// getUsers()
Codine_UserProjectSeq* Codine_Master_impl::getUsers() {
   QENTER("Master::getUsers");

   AUTO_LOCK_MASTER;

   Codine_UserProjectSeq* ups = new Codine_UserProjectSeq();
   for(list<Codine_User_impl*>::iterator it=users.begin(); it != users.end(); ++it)
      if((*it)->creation == 0) 
         ups->append(Codine_User_impl::_duplicate(*it));                                      
   return ups;
}

Codine_UserProjectSeq* Codine_Master_impl::getUsers(CORBA_Context* ctx) {
   QENTER("Master::getUsers(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getUsers();
}

// getProjects()
Codine_UserProjectSeq* Codine_Master_impl::getProjects() {
   QENTER("Master::getProjects");

   AUTO_LOCK_MASTER;

   Codine_UserProjectSeq* ups = new Codine_UserProjectSeq();
   for(list<Codine_Project_impl*>::iterator it=projects.begin(); it != projects.end(); ++it)
      if((*it)->creation == 0) 
         ups->append(Codine_Project_impl::_duplicate(*it));                                      
   return ups;
}

Codine_UserProjectSeq* Codine_Master_impl::getProjects(CORBA_Context* ctx) {
   QENTER("Master::getUsers(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getProjects();
}

// getShareTree()
Codine_ShareTreeNode* Codine_Master_impl::getShareTree() {
   QENTER("Master::getShareTree");

   AUTO_LOCK_MASTER;

   if(sharetree && sharetree->creation == 0) 
      return Codine_ShareTreeNode_impl::_duplicate(sharetree);
   else
      return Codine_ShareTreeNode_impl::_nil();
}

Codine_ShareTreeNode* Codine_Master_impl::getShareTree(CORBA_Context* ctx) {
   QENTER("Master::getShareTree(CORBA_Context*)");
   qidl_authenticate(ctx);

   return getShareTree();
}

// getSchedConf()
Codine_SchedConf* Codine_Master_impl::getSchedConf() {
   QENTER("Master::getSchedConf");

   AUTO_LOCK_MASTER;

   if(schedconf && schedconf->creation == 0) 
      return Codine_SchedConf_impl::_duplicate(schedconf);
   else
      return Codine_SchedConf_impl::_nil();
}

Codine_SchedConf* Codine_Master_impl::getSchedConf(CORBA_Context* ctx) {
   qidl_authenticate(ctx);

   return getSchedConf();
}


// simple string objects
Codine_cod_stringSeq* Codine_Master_impl::getAdminHosts(CORBA_Context* ctx) {
   QENTER("Master::getAdminHosts");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   lListElem* lep;

   Codine_cod_stringSeq* ahs = new Codine_cod_stringSeq();
   
   for_each_cpp(lep, Master_Adminhost_List)
      ahs->append(CORBA_string_dup(lGetString(lep, AH_name)));

   return ahs;
}

Codine_cod_stringSeq* Codine_Master_impl::getSubmitHosts(CORBA_Context* ctx) {
   QENTER("Master::getSubmitHosts");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   lListElem* lep;

   Codine_cod_stringSeq* shs = new Codine_cod_stringSeq();
   
   for_each_cpp(lep, Master_Submithost_List)
      shs->append(CORBA_string_dup(lGetString(lep, SH_name)));

   return shs;
}

Codine_cod_stringSeq* Codine_Master_impl::getManagers(CORBA_Context* ctx) {
   QENTER("Master::getManagers");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   lListElem* lep;

   Codine_cod_stringSeq* ms = new Codine_cod_stringSeq();
   
   for_each_cpp(lep, Master_Manager_List)
      ms->append(CORBA_string_dup(lGetString(lep, MO_name)));

   return ms;
}

Codine_cod_stringSeq* Codine_Master_impl::getOperators(CORBA_Context* ctx) {
   QENTER("Master::getOperators");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   lListElem* lep;

   Codine_cod_stringSeq* os = new Codine_cod_stringSeq();
   
   for_each_cpp(lep, Master_Operator_List)
      os->append(CORBA_string_dup(lGetString(lep, MO_name)));

   return os;
}

Codine_cod_ulong Codine_Master_impl::setAdminHosts(const Codine_cod_stringSeq& hosts, CORBA_Context* ctx) {
   QENTER("Master::setAdminHosts");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   lListPtr     lp;
   lListPtr     alp;
   lListElem*   lep;
   lEnumeration* what;

   lp = cod_stringSeq2cull((Codine_cod_stringSeq&)hosts);
   what = lWhat("%T(ALL)", AH_Type);
   alp = cod_api(COD_ADMINHOST_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);
   throwErrors(alp);

   return 0;
}

Codine_cod_ulong Codine_Master_impl::setSubmitHosts(const Codine_cod_stringSeq& hosts, CORBA_Context* ctx) {
   QENTER("Master::setSubmitHosts");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   lListPtr     lp;
   lListPtr     alp;
   lListElem*   lep;
   lEnumeration* what;

   lp = cod_stringSeq2cull((Codine_cod_stringSeq&)hosts);
   what = lWhat("%T(ALL)", SH_Type);
   alp = cod_api(COD_SUBMITHOST_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);
   throwErrors(alp);

   return 0;
}

Codine_cod_ulong Codine_Master_impl::setManagers(const Codine_cod_stringSeq& mgr, CORBA_Context* ctx) {
   QENTER("Master::setManagers");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   lListPtr     lp;
   lListPtr     alp;
   lListElem*   lep;
   lEnumeration* what;

   lp = cod_stringSeq2cull((Codine_cod_stringSeq&)mgr);
   what = lWhat("%T(ALL)", MO_Type);
   alp = cod_api(COD_MANAGER_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);
   throwErrors(alp);

   return 0;
}

Codine_cod_ulong Codine_Master_impl::setOperators(const Codine_cod_stringSeq& op, CORBA_Context* ctx) {
   QENTER("Master::setOperators");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   lListPtr     lp;
   lListPtr     alp;
   lListElem*   lep;
   lEnumeration* what;

   lp = cod_stringSeq2cull((Codine_cod_stringSeq&)op);
   what = lWhat("%T(ALL)", MO_Type);
   alp = cod_api(COD_OPERATOR_LIST, COD_API_MOD, &lp, NULL, what);
   lFreeWhat(what);
   throwErrors(alp);

   return 0;
}


// newQueue()
Codine_Queue* Codine_Master_impl::newQueue(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newQueue");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   Codine_Queue_impl* q = new Codine_Queue_impl(name, time(NULL), orb);
   queues.push_back(q);
   orb->connect(q);
   return Codine_Queue_impl::_duplicate(q);
}

// newComplex()
Codine_Complex* Codine_Master_impl::newComplex(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newComplex");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);

   Codine_Complex_impl* c = new Codine_Complex_impl(name, time(NULL), orb);
   complexes.push_back(c);
   orb->connect(c);
   return Codine_Complex_impl::_duplicate(c);
}

// newJob()
Codine_Job* Codine_Master_impl::newJob(CORBA_Context_ptr ctx) {
   QENTER("Master::newJob");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_Job_impl* j = new Codine_Job_impl(0, time(NULL), orb);
   jobs.push_back(j);
   orb->connect(j);
   return Codine_Job_impl::_duplicate(j);
}

// newConfiguration()
Codine_Configuration* Codine_Master_impl::newConfiguration(const char* hname, CORBA_Context_ptr ctx) {
   QENTER("Master::newConfiguration");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_Configuration_impl* c = new Codine_Configuration_impl(hname, time(NULL), orb);
   configurations.push_back(c);
   orb->connect(c);
   return Codine_Configuration_impl::_duplicate(c);
}

// newCalendar
Codine_Calendar* Codine_Master_impl::newCalendar(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newCalendar");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_Calendar_impl* cal = new Codine_Calendar_impl(name, time(NULL), orb);
   calendars.push_back(cal);
   orb->connect(cal);
   return Codine_Calendar_impl::_duplicate(cal);
}

// newCheckpoint
Codine_Checkpoint* Codine_Master_impl::newCheckpoint(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newCheckpoint");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_Checkpoint_impl* ckpt = new Codine_Checkpoint_impl(name, time(NULL), orb);
   checkpoints.push_back(ckpt);
   orb->connect(ckpt);
   return Codine_Checkpoint_impl::_duplicate(ckpt);
}


Codine_ParallelEnvironment* Codine_Master_impl::newParallelEnvironment(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newParallelEnvironment");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_ParallelEnvironment_impl* pe = new Codine_ParallelEnvironment_impl(name, time(NULL), orb);
   parallelenvironments.push_back(pe);
   orb->connect(pe);
   return Codine_ParallelEnvironment_impl::_duplicate(pe);
}


Codine_UserSet* Codine_Master_impl::newUserSet(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newUserSet");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_UserSet_impl* us = new Codine_UserSet_impl(name, time(NULL), orb);
   usersets.push_back(us);
   orb->connect(us);
   return Codine_UserSet_impl::_duplicate(us);
}


Codine_ExecHost* Codine_Master_impl::newExecHost(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newExecHost");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_ExecHost_impl* eh = new Codine_ExecHost_impl(name, time(NULL), orb);
   exechosts.push_back(eh);
   orb->connect(eh);
   return Codine_ExecHost_impl::_duplicate(eh);
}

Codine_UserProject* Codine_Master_impl::newUser(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newUser");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_User_impl* up = new Codine_User_impl(name, time(NULL), orb);
   users.push_back(up);
   orb->connect(up);
   return Codine_User_impl::_duplicate(up);
}

Codine_UserProject* Codine_Master_impl::newProject(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newProject");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   Codine_Project_impl* up = new Codine_Project_impl(name, time(NULL), orb);
   projects.push_back(up);
   orb->connect(up);
   return Codine_Project_impl::_duplicate(up);
}

Codine_SchedConf* Codine_Master_impl::newSchedConf(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newSchedConf");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   
   if(schedconf)
      return Codine_SchedConf_impl::_duplicate(schedconf);  // TODO: throw exception here ?

   schedconf = new Codine_SchedConf_impl(name, time(NULL), orb);
   orb->connect(schedconf);
   return Codine_SchedConf_impl::_duplicate(schedconf);
}

Codine_ShareTreeNode* Codine_Master_impl::newShareTree(const char* name, CORBA_Context_ptr ctx) {
   QENTER("Master::newShareTree");

   AUTO_LOCK_MASTER;

   qidl_authenticate(ctx);
   if(sharetree)
      return Codine_ShareTreeNode_impl::_duplicate(sharetree);
      
   sharetree = new Codine_ShareTreeNode_impl(name, time(NULL), orb);
   orb->connect(sharetree);
   return Codine_ShareTreeNode_impl::_duplicate(sharetree);
}



CosEventChannelAdmin_ConsumerAdmin* Codine_Master_impl::getConsumerAdmin(CORBA_Context* ctx) {
   QENTER("Master::getConsumerAdmin");
   qidl_authenticate(ctx);
   
   if(!CORBA_is_nil(es))
      return es->for_consumers();
   else
      return CosEventChannelAdmin_ConsumerAdmin::_nil();
}


// deleteObject
// both functions delete the CORBA object representing the
// specified cull object
void Codine_Master_impl::deleteObject(int type, const char* name) {
   QENTER("Master::deleteObject(name)");

   AUTO_LOCK_MASTER;

   list<Codine_Queue_impl*>::iterator qelem;
   list<Codine_Complex_impl*>::iterator cplx;
   list<Codine_Calendar_impl*>::iterator calelem;
   list<Codine_Checkpoint_impl*>::iterator ckptelem;
   list<Codine_ParallelEnvironment_impl*>::iterator peelem;
   list<Codine_UserSet_impl*>::iterator uselem;
   list<Codine_ExecHost_impl*>::iterator ehelem;
   list<Codine_User_impl*>::iterator uelem;
   list<Codine_Project_impl*>::iterator pelem;
   Codine_ShareTreeNode_impl* sn;
   Codine_SchedConf_impl* sc;
   
   Codine_event ev;
   CORBA_Any any;
   // template event
   ev.type = Codine_ev_del;
   ev.name = CORBA_string_dup(name?name:"");
   ev.id   = 0;   // not needed
   ev.ref  = CORBA_Object::_nil();
   ev.count= 0;

   switch(type) {
      case COD_QUEUE_LIST:
         qelem = getQueue(name);
         if(qelem != queues.end()) {
            orb->disconnect(*qelem);
            CORBA_release(*qelem);
            qelem = queues.erase(qelem);
         
            // generate event
            ev.obj  = COD_QUEUE_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_COMPLEX_LIST:
         cplx = getComplex(name);
         if(cplx != complexes.end()) {
            orb->disconnect(*cplx);
            CORBA_release(*cplx);
            complexes.erase(cplx);
         
            // generate event
            ev.obj  = COD_COMPLEX_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_CALENDAR_LIST:
         calelem = getCalendar(name);
         if(calelem != calendars.end()) {
            orb->disconnect(*calelem);
            CORBA_release(*calelem);
            calendars.erase(calelem);
         
            // generate event
            ev.obj  = COD_CALENDAR_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_CKPT_LIST:
         ckptelem = getCheckpoint(name);
         if(ckptelem != checkpoints.end()) {
            orb->disconnect(*ckptelem);
            CORBA_release(*ckptelem);
            checkpoints.erase(ckptelem);
         
            // generate event
            ev.obj  = COD_CKPT_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_PE_LIST:
         peelem = getParallelEnvironment(name);
         if(peelem != parallelenvironments.end()) {
            orb->disconnect(*peelem);
            CORBA_release(*peelem);
            parallelenvironments.erase(peelem);
         
            // generate event
            ev.obj  = COD_PE_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_USERSET_LIST:
         uselem = getUserSet(name);
         if(uselem != usersets.end()) {
            orb->disconnect(*uselem);
            CORBA_release(*uselem);
            usersets.erase(uselem);
         
            // generate event
            ev.obj  = COD_USERSET_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_EXECHOST_LIST:
         ehelem = getExecHost(name);
         if(ehelem != exechosts.end()) {
            orb->disconnect(*ehelem);
            CORBA_release(*ehelem);
            exechosts.erase(ehelem);
         
            // generate event
            ev.obj  = COD_EXECHOST_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_USER_LIST:
         uelem = getUser(name);
         if(uelem != users.end()) {
            orb->disconnect(*uelem);
            CORBA_release(*uelem);
            users.erase(uelem);
         
            // generate event
            ev.obj  = COD_USER_LIST;
            any <<= ev;
            addEvent(any);
         }
         break; 
      case COD_PROJECT_LIST:
         pelem = getProject(name);
         if(pelem != projects.end()) {
            orb->disconnect(*pelem);
            CORBA_release(*pelem);
            projects.erase(pelem);
         
            // generate event
            ev.obj  = COD_PROJECT_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_SHARETREE_LIST:
         sn = getShareTreeImpl(name);
         if(sn) {
            orb->disconnect(sn);
            CORBA_release(sn);
            sharetree = NULL;
         
            // generate event
            ev.obj  = COD_SHARETREE_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      case COD_SC_LIST:
         sc = getSchedConfImpl(name);
         if(sc) {
            orb->disconnect(sc);
            CORBA_release(sc);
            schedconf = NULL;
         
            // generate event
            ev.obj  = COD_SC_LIST;
            any <<= ev;
            addEvent(any);
         }
         break;
      default:
         break;
   }

   // TODO: erase old objects now
}


void Codine_Master_impl::deleteObject(int type, Codine_cod_ulong id) {
   QENTER("Master::deleteObject(id)");

   AUTO_LOCK_MASTER;

   list<Codine_Job_impl*>::iterator jelem;
   Codine_event ev;
   CORBA_Any any;

   switch(type) {
      case COD_JOB_LIST:
         jelem = getJob(id);
         if(jelem != jobs.end()) {
            orb->disconnect(*jelem);
            CORBA_release(*jelem);
            jobs.erase(jelem);
         
            // generate event
            ev.type = Codine_ev_del;
            ev.obj  = COD_JOB_LIST;
            ev.name = CORBA_string_dup("");
            ev.id   = id;
            ev.ref  = Codine_Job_impl::_nil();
            ev.count= 0;
            any <<= ev;
            addEvent(any);
         }
         break;
      default:
         break;
   }
}


void Codine_Master_impl::updateShareTree(lListElem* ep) {
   QENTER("Master::updateShareTree");

   if (sharetree)
   {
      sharetree->addShareTreeNodes(ep);
      sharetree->delShareTreeNodes(ep);
   }
}


// addObject
// both functions add a new corba object representing
// specified cull object
// type: e.g. CODINE_QUEUE_LIST
// name,id: the identifier of the object
void Codine_Master_impl::addObject(int type, const char* name) {
   QENTER("Master::addObject(name)");

   AUTO_LOCK_MASTER;

   Codine_Queue_impl* q;
   Codine_Complex_impl* c;
   Codine_Calendar_impl* cal;
   Codine_Checkpoint_impl* ckpt;
   Codine_ParallelEnvironment_impl* pe;
   Codine_UserSet_impl* us;
   Codine_ExecHost_impl* eh;
   Codine_User_impl* user;
   Codine_Project_impl* prj;
   Codine_ShareTreeNode_impl* sn;
   Codine_SchedConf_impl* sc;
   
   Codine_event ev;
   Codine_contentSeq* ch;
   CORBA_Any any;
   
   // template event
   ev.type = Codine_ev_add;
   ev.name = CORBA_string_dup(name?name:"");
   ev.id   = 0;   // not needed
   ev.count= 0;
   
   switch(type) {
      case COD_QUEUE_LIST:
         // we must first check that no Q of that name already exists
         q = getQueueImpl(name);
         if(!q) {
            // there really is none, then add a new one
            q = new Codine_Queue_impl(name, orb);
            queues.push_back(q);
            orb->connect(q);
         
            // generate event
            ch = q->get_content(getMasterContext());
            ev.obj  = COD_QUEUE_LIST;
            ev.ref  = Codine_Queue_impl::_duplicate(q);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_COMPLEX_LIST:
         c = getComplexImpl(name);
         if(!c) {
            c = new Codine_Complex_impl(name, orb);
            complexes.push_back(c);
            orb->connect(c);
            
            // generate event
            ch = c->get_content(getMasterContext());
            ev.obj  = COD_COMPLEX_LIST;
            ev.ref  = Codine_Complex_impl::_duplicate(c);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch; 
         }
         break;
      case COD_CALENDAR_LIST:
         cal = getCalendarImpl(name);
         if(!cal) {
            cal = new Codine_Calendar_impl(name, orb);
            calendars.push_back(cal);
            orb->connect(cal);
            
            // generate event
            ch = cal->get_content(getMasterContext());
            ev.obj  = COD_CALENDAR_LIST;
            ev.ref  = Codine_Calendar_impl::_duplicate(cal);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_CKPT_LIST:
         ckpt = getCheckpointImpl(name);
         if(!ckpt) {
            ckpt = new Codine_Checkpoint_impl(name, orb);
            checkpoints.push_back(ckpt);
            orb->connect(ckpt);
            
            // generate event
            ch = ckpt->get_content(getMasterContext());
            ev.obj  = COD_CKPT_LIST;
            ev.ref  = Codine_Checkpoint_impl::_duplicate(ckpt);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_PE_LIST:
         pe = getParallelEnvironmentImpl(name);
         if(!pe) {
            pe = new Codine_ParallelEnvironment_impl(name, orb);
            parallelenvironments.push_back(pe);
            orb->connect(pe);
            
            // generate event
            ch = pe->get_content(getMasterContext());
            ev.obj  = COD_PE_LIST;
            ev.ref  = Codine_ParallelEnvironment_impl::_duplicate(pe);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_USERSET_LIST:
         us = getUserSetImpl(name);
         if(!us) {
            us = new Codine_UserSet_impl(name, orb);
            usersets.push_back(us);
            orb->connect(us);
            
            // generate event
            ch = us->get_content(getMasterContext());
            ev.obj  = COD_USERSET_LIST;
            ev.ref  = Codine_UserSet_impl::_duplicate(us);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_EXECHOST_LIST:
         eh = getExecHostImpl(name);
         if(!eh) {
            eh = new Codine_ExecHost_impl(name, orb);
            exechosts.push_back(eh);
            orb->connect(eh);
            
            // generate event
            ch = eh->get_content(getMasterContext());
            ev.obj  = COD_EXECHOST_LIST;
            ev.ref  = Codine_ExecHost_impl::_duplicate(eh);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_USER_LIST:
         user = getUserImpl(name);
         if(!user) {
            user = new Codine_User_impl(name, orb);
            users.push_back(user);
            orb->connect(user);
            
            // generate event
            ch = user->get_content(getMasterContext());
            ev.obj  = COD_USER_LIST;
            ev.ref  = Codine_User_impl::_duplicate(user);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_PROJECT_LIST:
         prj = getProjectImpl(name);
         if(!prj) {
            prj = new Codine_Project_impl(name, orb);
            projects.push_back(prj);
            orb->connect(prj);
            
            // generate event
            ch = prj->get_content(getMasterContext());
            ev.obj  = COD_PROJECT_LIST;
            ev.ref  = Codine_Project_impl::_duplicate(prj);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_SHARETREE_LIST:
         sn = getShareTreeImpl(name);
         if(!sn) {
            //sn = new Codine_ShareTreeNode_impl(name, orb);
            sharetree = sn;
            orb->connect(sn);
            
            // generate event
            ch = sn->get_content(getMasterContext());
            ev.obj  = COD_SHARETREE_LIST;
            ev.ref  = Codine_ShareTreeNode_impl::_duplicate(sn);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      case COD_SC_LIST:
         sc = getSchedConfImpl(name);
         if(!sc) {
            sc = new Codine_SchedConf_impl(name, orb);
            schedconf = sc;
            orb->connect(sc);
            
            // generate event
            ch = sc->get_content(getMasterContext());
            ev.obj  = COD_SC_LIST;
            ev.ref  = Codine_SchedConf_impl::_duplicate(sc);
            ev.changes = *ch;
            any <<= ev;
            addEvent(any);
            delete ch;
         }
         break;
      default:
         break;
   }
}

void Codine_Master_impl::addObject(int type, Codine_cod_ulong id) {
   QENTER("Master::addObject(id)");

   AUTO_LOCK_MASTER;

   Codine_Job_impl* j;
   Codine_event ev;
   Codine_contentSeq* ch;
   CORBA_Any any;
   
   switch(type) {
      case COD_JOB_LIST: 
         j = getJobImpl(0);  // MUST call with 0 here !!!!
         if (!j) {
            j = new Codine_Job_impl(id,orb);
            jobs.push_back(j);
            orb->connect(j);
         }
         // set key here
         j->key = id;

         // generate event
         ch = j->get_content(getMasterContext());
         ev.type = Codine_ev_add;
         ev.obj  = COD_JOB_LIST;
         ev.name = CORBA_string_dup("");
         ev.id   = id;
         ev.ref  = Codine_Job_impl::_duplicate(j);
         ev.count= 0;
         ev.changes = *ch;
         any <<= ev;
         addEvent(any);
         delete ch;
         break;
      default:
         break;
   }
}


CORBA_Boolean Codine_Master_impl::sends_events(CORBA_Context* ctx) {
   QENTER("Master::sends_events");

   qidl_authenticate(ctx);

   return sends_events();
}

CORBA_Boolean Codine_Master_impl::send_events(CORBA_Boolean sw, CORBA_Context* ctx) {
   QENTER("Master::send_events");

   qidl_authenticate(ctx);

   ev_flag = sw;
   
   try {
      connectEventService(); // this sets ev_flag to false in case of failure
   }
   catch (...) {
   }

   return ev_flag;
}

void Codine_Master_impl::connectEventService() {
   QENTER("Master::connectEventService");
   
   // already connected, or no need to ?
   if(!ev_flag || !CORBA_is_nil(es))
      return;

   try {
      // get it
      CORBA_Object_var es_obj = orb->resolve_initial_references("EventService");
      if(CORBA_is_nil(es_obj))
         throw 0;
      
      es = CosEventChannelAdmin_EventChannel::_narrow(es_obj);
      if(CORBA_is_nil(es))
         throw 0;
      CosEventChannelAdmin_SupplierAdmin_var supp_adm = es->for_suppliers();
      ev_consumer = supp_adm->obtain_push_consumer();
      ev_consumer->connect_push_supplier(this); // NO _duplicate !!!
   }
   catch(CosEventChannelAdmin_AlreadyConnected&) {
      // ok, leave me connected
   }
   catch (...) {
      es = CosEventChannelAdmin_EventChannel::_nil();
      ev_consumer = CosEventChannelAdmin_ProxyPushConsumer::_nil();
      ev_flag = false;
      throw;
   }
}

// event handling
// this is a bit tricky, because this function may be called
// from either the qmaster thread or the qidl thread
// but in both cases, the event must be sent from the qidl thread
// since the qmaster thread is not enrolled with CORBA.
// so we push the event into a queue and
// write a dummy byte into the pipe of an ORBacus event
// handler (running in the qidl thread), which will then send
// the event to the event service
// Note: Ourserlves (i.e. The Codine_Master_impl singleton) are the
// event handler, so see function handleEvent() for how the story goes on...
void Codine_Master_impl::addEvent(const CORBA_Any& any) {
   QENTER("Master::addEvent");

   AUTO_LOCK_MASTER;

   // forget it if there is no event service
   if(!ev_flag || CORBA_is_nil(es))
      return;

   PthreadLock foo(&events_lock);

   // push event into event queue
   events.push_back(any);

   // notify that there is an event
   char x = 'x';
   write(obEventFd[1], &x, sizeof(char));
}


// object retrieval functions
// used internally to find an object
list<Codine_Queue_impl*>::iterator Codine_Master_impl::getQueue(const char* name) {         
   QENTER("Master::getQueue");

   AUTO_LOCK_MASTER;

   if(!name)
      return queues.end();

   list<Codine_Queue_impl*>::iterator  qelem;
   for(qelem=queues.begin(); qelem != queues.end(); ++qelem)
      if(!strcmp((*qelem)->key, name)) 
         return qelem;

   return qelem;
}

list<Codine_Complex_impl*>::iterator Codine_Master_impl::getComplex(const char* name) {         
   QENTER("Master::getComplex");

   AUTO_LOCK_MASTER;

   if(!name)
      return complexes.end();

   list<Codine_Complex_impl*>::iterator  cplx;
   for(cplx=complexes.begin(); cplx != complexes.end(); ++cplx)
      if(!strcmp((*cplx)->key, name)) 
         return cplx;

   return cplx;
}

list<Codine_Job_impl*>::iterator Codine_Master_impl::getJob(Codine_cod_ulong key) { 
   QENTER("Master::getJob");

   AUTO_LOCK_MASTER;

   list<Codine_Job_impl*>::iterator  jelem;
   for(jelem=jobs.begin(); jelem != jobs.end(); ++jelem)
      if( (*jelem)->key == key)
         if(key==0 && (*jelem)->creation==-1) // special case for adding job
            return jelem;
         else if(key != 0)   // otherwise give out only public jobs
            return jelem;

   return jelem;
}

list<Codine_Configuration_impl*>::iterator Codine_Master_impl::getConfiguration(const char* key) { 
   QENTER("Master::getConfiguration");

   AUTO_LOCK_MASTER;

   list<Codine_Configuration_impl*>::iterator  celem;
   for(celem=configurations.begin(); celem != configurations.end(); ++celem)
      if(!strcmp((*celem)->key, key)) 
         return celem;

   return celem;
}

list<Codine_Calendar_impl*>::iterator Codine_Master_impl::getCalendar(const char* name) {         
   QENTER("Master::getCalendar");

   AUTO_LOCK_MASTER;

   if(!name)
      return calendars.end();

   list<Codine_Calendar_impl*>::iterator  cal;
   for(cal=calendars.begin(); cal != calendars.end(); ++cal)
      if(!strcmp((*cal)->key, name)) 
         return cal;

   return cal;
}

list<Codine_Checkpoint_impl*>::iterator Codine_Master_impl::getCheckpoint(const char* name) {         
   QENTER("Master::getCheckpoint");

   AUTO_LOCK_MASTER;

   if(!name)
      return checkpoints.end();

   list<Codine_Checkpoint_impl*>::iterator  ckpt;
   for(ckpt=checkpoints.begin(); ckpt != checkpoints.end(); ++ckpt)
      if(!strcmp((*ckpt)->key, name)) 
         return ckpt;

   return ckpt;
}

list<Codine_ParallelEnvironment_impl*>::iterator Codine_Master_impl::getParallelEnvironment(const char* name) {         
   QENTER("Master::getParallelEnvironment");

   AUTO_LOCK_MASTER;

   if(!name)
      return parallelenvironments.end();

   list<Codine_ParallelEnvironment_impl*>::iterator  pe;
   for(pe=parallelenvironments.begin(); pe != parallelenvironments.end(); ++pe)
      if(!strcmp((*pe)->key, name)) 
         return pe;

   return pe;
}

list<Codine_UserSet_impl*>::iterator Codine_Master_impl::getUserSet(const char* name) {         
   QENTER("Master::getUserSet");

   AUTO_LOCK_MASTER;

   if(!name)
      return usersets.end();

   list<Codine_UserSet_impl*>::iterator  us;
   for(us=usersets.begin(); us != usersets.end(); ++us)
      if(!strcmp((*us)->key, name)) 
         return us;

   return us;
}

list<Codine_ExecHost_impl*>::iterator Codine_Master_impl::getExecHost(const char* name) {         
   QENTER("Master::getExecHost");

   AUTO_LOCK_MASTER;

   if(!name)
      return exechosts.end();

   list<Codine_ExecHost_impl*>::iterator  eh;
   for(eh=exechosts.begin(); eh != exechosts.end(); ++eh)
      if(!strcmp((*eh)->key, name))
         return eh;

   return eh;
}

list<Codine_User_impl*>::iterator Codine_Master_impl::getUser(const char* name) {         
   QENTER("Master::getUser");

   AUTO_LOCK_MASTER;

   if(!name)
      return users.end();

   list<Codine_User_impl*>::iterator  up;
   for(up=users.begin(); up != users.end(); ++up)
      if(!strcmp((*up)->key, name)) 
         return up;

   return up;
}

list<Codine_Project_impl*>::iterator Codine_Master_impl::getProject(const char* name) {         
   QENTER("Master::getProject");

   AUTO_LOCK_MASTER;

   if(!name)
      return projects.end();

   list<Codine_Project_impl*>::iterator  up;
   for(up=projects.begin(); up != projects.end(); ++up)
      if(!strcmp((*up)->key, name)) 
         return up;

   return up;
}


Codine_ShareTreeNode* Codine_Master_impl::getShareTreeNode(Codine_ShareTreeNode* node,const char* name)
{
   Codine_ShareTreeNode* temp=NULL;
   if (!strcmp(node->get_name(context), name)) {
      //cout<<"found node: "<<node->get_name(ctx)<< "@" << (void*)node << endl;
      return node;
   }
   
   Codine_ShareTreeNodeSeq_var chs = node->get_children(context);
   
   if (chs->length() > 0) {
      for (int i=0; i < chs->length(); i++) {
         temp = getShareTreeNode(chs[i], name);
         if (temp) 
            return temp;
      }
   }

   return NULL;
}


Codine_Queue_impl* Codine_Master_impl::getQueueImpl(const char* name) {
   QENTER("Master::getQueueImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_Queue_impl*>::iterator  qelem = getQueue(name);
   if(qelem != queues.end())
      return *qelem;
   else
      return NULL;
}

Codine_Complex_impl* Codine_Master_impl::getComplexImpl(const char* name) {
   QENTER("Master::getComplexImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_Complex_impl*>::iterator  cplx = getComplex(name);
   if(cplx != complexes.end())
      return *cplx;
   else
      return NULL;
}

Codine_Job_impl* Codine_Master_impl::getJobImpl(Codine_cod_ulong name) {
   QENTER("Master::getJobImpl");

   AUTO_LOCK_MASTER;

   list<Codine_Job_impl*>::iterator  jelem = getJob(name);
   if(jelem != jobs.end())
      return *jelem;
   else
      return NULL;
}

Codine_Configuration_impl* Codine_Master_impl::getConfigurationImpl(const char* name) {
   QENTER("Master::getConfigurationImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_Configuration_impl*>::iterator  conf = getConfiguration(name);
   if(conf != configurations.end())
      return *conf;
   else
      return NULL;
}


Codine_Calendar_impl* Codine_Master_impl::getCalendarImpl(const char* name) {
   QENTER("Master::getCalendarImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_Calendar_impl*>::iterator  cal = getCalendar(name);
   if(cal != calendars.end())
      return *cal;
   else
      return NULL;
}

Codine_Checkpoint_impl* Codine_Master_impl::getCheckpointImpl(const char* name) {
      QENTER("Master::getCheckpointImpl");

      AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

      list<Codine_Checkpoint_impl*>::iterator  ckpt = getCheckpoint(name);
      if(ckpt != checkpoints.end())
         return *ckpt;
      else
         return NULL;
}

Codine_ParallelEnvironment_impl* Codine_Master_impl::getParallelEnvironmentImpl(const char* name) {
   QENTER("Master::getParallelEnvironmentImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_ParallelEnvironment_impl*>::iterator  pe = getParallelEnvironment(name);
   if(pe != parallelenvironments.end())
      return *pe;
   else
      return NULL;
}

Codine_UserSet_impl* Codine_Master_impl::getUserSetImpl(const char* name) {
   QENTER("Master::getUserSetImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_UserSet_impl*>::iterator  us = getUserSet(name);
   if(us != usersets.end())
      return *us;
   else
      return NULL;
}

Codine_ExecHost_impl* Codine_Master_impl::getExecHostImpl(const char* name) {
   QENTER("Master::getExecHostImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_ExecHost_impl*>::iterator  eh = getExecHost(name);
   if(eh != exechosts.end())
      return *eh;
   else
      return NULL;
}

Codine_User_impl* Codine_Master_impl::getUserImpl(const char* name) {
   QENTER("Master::getUserImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_User_impl*>::iterator  up = getUser(name);
   if(up != users.end())
      return *up;
   else
      return NULL;
}

Codine_Project_impl* Codine_Master_impl::getProjectImpl(const char* name) {
   QENTER("Master::getProjectImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   list<Codine_Project_impl*>::iterator  up = getProject(name);
   if(up != projects.end())
      return *up;
   else
      return NULL;
}

Codine_ShareTreeNode_impl* Codine_Master_impl::getShareTreeImpl(const char* name) {
   QENTER("Master::getShareTreeImpl");

   AUTO_LOCK_MASTER;
   
   if (!name)
      return NULL;

   Codine_ShareTreeNode* stn = getShareTreeNode(sharetree,name);
   if (stn)
      return dynamic_cast<Codine_ShareTreeNode_impl*>(stn);
   else
      return NULL;
   /*if(sharetree && !strcmp(sharetree->key, name))
      return sharetree;
   else
      return NULL;*/
}

Codine_SchedConf_impl* Codine_Master_impl::getSchedConfImpl(const char* name) {
   QENTER("Master::getSchedConfImpl");

   AUTO_LOCK_MASTER;

   if (!name)
      return NULL;

   if(schedconf && !strcmp(schedconf->key, name))
      return schedconf;
   else
      return NULL;
}

// handleEvent
// this is the ORBacus event handler function which will be activated
// when someone write something in the obEventFd pipe (usually this
// is the addEvent() function). This function is always executed
// in the qidl thread (ensured by program logic) and can thus safely
// invoke a CORBA request (push())
void Codine_Master_impl::handleEvent(OBMask mask) {
   QENTER("Master::handleEvent");

   AUTO_LOCK_MASTER;

   PthreadLock foo(&events_lock);

   // 'clear' event fd
   char x;
   read(obEventFd[0], &x, sizeof(char));

   CORBA_Any any = events.front();

   try {
      if(!CORBA_is_nil(ev_consumer)) {
         ev_consumer->push(any);
      }
   }
   catch(CORBA_SystemException& x) {
      es = CosEventChannelAdmin_EventChannel::_nil();
      ev_consumer = CosEventChannelAdmin_ProxyPushConsumer::_nil();
      ev_flag = false;
      OBPrintException(x);
   }
   catch(...) {
      ev_flag =false;
   }
   events.pop_front();
}

void Codine_Master_impl::handleStop() {
   QENTER("Master::handleStop");
}

// getMasterContext
// this function returns a context that authenticates itself as root :-)
// used to invoke CORBA functions internally from the qidl thread, when
// no user called a function
CORBA_Context* Codine_Master_impl::getMasterContext() {
   QENTER("Master::getMasterContext");

   AUTO_LOCK_MASTER;

   if(!CORBA_is_nil(context))
      return context;

   CORBA_Any any;
   char temp[1000];
   strcpy(temp, "0:0:0:0:"); // be root :-)
   gethostname(&temp[strlen(temp)], 100);
   any <<= temp;
   orb->get_default_context(context);
   context->set_one_value("cod_auth", any);

   return CORBA_Context::_duplicate(context);
}


// (un)registerObject
// top level function (some sort of FACADE pattern) to provide easy CORBA-
// job registration to application programmers. This function effectively
// adds(removes) an (IOR, ...) pair to(from) the given job's context.
CORBA_Boolean Codine_Master_impl::registerObject(CORBA_Object* obj, const char* job_auth) {
   QENTER("Master::registerObject");

   AUTO_LOCK_MASTER;

   CORBA_ULong jid = atol(job_auth);   // TODO: decode string here
   
   Codine_Job_impl* job = getJobImpl(jid);
   
   if(!job)
      return false;

   try {
      // append the IOR to the job's context
      // don't be confused about job context and CORBA context !!!
      Codine_VariableSeq_var context = job->get_context(getMasterContext());
      CORBA_ULong le;
      // first search if the IOR is set already
      for(le=0; le<context->length(); le++)
         if(!strcmp(context[le].variable, CONTEXT_IOR))
            return false;
      
      // now append it
      context->length((le = context->length())+1);
      context[le].variable = CORBA_string_dup(CONTEXT_IOR);
      context[le].value = CORBA_string_dup(orb->object_to_string(obj));
      job->set_context(context, getMasterContext());
   }
   catch(...) {
      return false;
   }
   
   return true;
}

CORBA_Boolean Codine_Master_impl::unregisterObject(CORBA_Object * obj, const char* job_auth) {
   QENTER("Master::unregisterObject");

   AUTO_LOCK_MASTER;

   CORBA_ULong jid = atol(job_auth);   // TODO: decode string here
 
   Codine_Job_impl* job = getJobImpl(jid);

   if(!job)
      return false;

   try {
      // get the IOR from the job's context
      // don't be confused about job context and CORBA context !!!
      Codine_VariableSeq_var context = job->get_context(getMasterContext());
      CORBA_ULong i;
      for(i=0; i<context->length(); i++)
         if(!strcmp(context[i].variable, CONTEXT_IOR) &&
            !strcmp(context[i].value, orb->object_to_string(obj))) {
            context->remove(i);
            job->set_context(context, getMasterContext());
            return true;
         }
   }
   catch(...) {
      return false;
   }
   
   return false;
}

void Codine_Master_impl::disconnect_push_supplier() {
   QENTER("Master::disconnect_push_supplier");
   
}
