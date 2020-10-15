#ifndef __MASTER_IMPL_H
#define __MASTER_IMPL_H
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
// Master_impl.h
// Header file for the Sge_Master object
// this is a sigleton instance


#include <OB/CORBA.h>
#include <OB/CosNaming.h>
#include <OB/Reactor.h>
#include <CosEventChannelAdmin.h>
#include <CosEventComm_skel.h>

#include <list>
#include <deque>
#include <pthread.h>
#include "Master_skel.h"
#include "cull.h"

#ifdef HAVE_STD
using namespace std;
#endif

class Sge_Queue_impl;
class Sge_Complex_impl;
class Sge_Job_impl;
class Sge_Configuration_impl;
class Sge_Calendar_impl;
class Sge_Checkpoint_impl;
class Sge_ParallelEnvironment_impl;
class Sge_UserSet_impl;
class Sge_ExecHost_impl;
class Sge_User_impl;
class Sge_Project_impl;
class Sge_SchedConf_impl;
class Sge_ShareTreeNode_impl;


class Sge_Master_impl : virtual public OBEventHandler, 
                           virtual public Sge_Master_skel, 
                           virtual public CosEventComm_PushSupplier_skel {
   private:
      static Sge_Master_impl* master;

   protected:
      Sge_Master_impl(CORBA_ORB_ptr o, CORBA_BOA_ptr b);
      virtual ~Sge_Master_impl();

      // OBEventHandler stuff, sends event when notified
      virtual void handleEvent(OBMask mask);
      virtual void handleStop();

   public:
      // singleton retrieval
      static Sge_Master_impl* instance();
      const char*                getIOR();

      // idl methods
      virtual Sge_QueueSeq*               getQueues();
      virtual Sge_QueueSeq*               getQueues(CORBA_Context_ptr ctx);
      virtual Sge_JobSeq*                 getJobs();
      virtual Sge_JobSeq*                 getJobs(CORBA_Context_ptr ctx);
      virtual Sge_ComplexSeq*             getComplexes();
      virtual Sge_ComplexSeq*             getComplexes(CORBA_Context* ctx);
      virtual Sge_ConfigurationSeq*       getConfigurations();
      virtual Sge_ConfigurationSeq*       getConfigurations(CORBA_Context* ctx);
      virtual Sge_CalendarSeq*            getCalendars(CORBA_Context* ctx);
      virtual Sge_CalendarSeq*            getCalendars();
      virtual Sge_CheckpointSeq*          getCheckpoints(CORBA_Context* ctx);
      virtual Sge_CheckpointSeq*          getCheckpoints();
      virtual Sge_ParallelEnvironmentSeq* getParallelEnvironments(CORBA_Context* ctx);
      virtual Sge_ParallelEnvironmentSeq* getParallelEnvironments();
      virtual Sge_UserSetSeq*             getUserSets(CORBA_Context* ctx);
      virtual Sge_UserSetSeq*             getUserSets();
      virtual Sge_ExecHostSeq*            getExecHosts(CORBA_Context* ctx);
      virtual Sge_ExecHostSeq*            getExecHosts();
      virtual Sge_UserProjectSeq*         getUsers(CORBA_Context* ctx);
      virtual Sge_UserProjectSeq*         getUsers();
      virtual Sge_UserProjectSeq*         getProjects(CORBA_Context* ctx);
      virtual Sge_UserProjectSeq*         getProjects();
      virtual Sge_SchedConf*              getSchedConf();
      virtual Sge_SchedConf*              getSchedConf(CORBA_Context* ctx);
      virtual Sge_ShareTreeNode*          getShareTree();
      virtual Sge_ShareTreeNode*          getShareTree(CORBA_Context* ctx);
      
      
      // simple string objects
      virtual Sge_sge_stringSeq*          getAdminHosts(CORBA_Context* ctx);
      virtual Sge_sge_stringSeq*          getSubmitHosts(CORBA_Context* ctx);
      virtual Sge_sge_stringSeq*          getManagers(CORBA_Context* ctx);
      virtual Sge_sge_stringSeq*          getOperators(CORBA_Context* ctx);
      virtual Sge_sge_ulong               setAdminHosts(const Sge_sge_stringSeq& hosts, CORBA_Context* ctx);
      virtual Sge_sge_ulong               setSubmitHosts(const Sge_sge_stringSeq& hosts, CORBA_Context* ctx);
      virtual Sge_sge_ulong               setManagers(const Sge_sge_stringSeq& hosts, CORBA_Context* ctx);
      virtual Sge_sge_ulong               setOperators(const Sge_sge_stringSeq& hosts, CORBA_Context* ctx);
      
      
      virtual Sge_Queue*                  newQueue(const char* name, CORBA_Context* ctx);
      virtual Sge_Complex*                newComplex(const char* name, CORBA_Context* ctx);
      virtual Sge_Job*                    newJob(CORBA_Context* ctx);
      virtual Sge_Configuration*          newConfiguration(const char* name, CORBA_Context* ctx);
      virtual Sge_Calendar*               newCalendar(const char* name, CORBA_Context* ctx);
      virtual Sge_Checkpoint*             newCheckpoint(const char* name, CORBA_Context* ctx);
      virtual Sge_ParallelEnvironment*    newParallelEnvironment(const char* name, CORBA_Context* ctx);
      virtual Sge_UserSet*                newUserSet(const char* name, CORBA_Context* ctx);
      virtual Sge_ExecHost*               newExecHost(const char* name, CORBA_Context* ctx);
      virtual Sge_UserProject*            newUser(const char* name, CORBA_Context* ctx);
      virtual Sge_UserProject*            newProject(const char* name, CORBA_Context* ctx);
      virtual Sge_SchedConf*              newSchedConf(const char* name, CORBA_Context* ctx);
      virtual Sge_ShareTreeNode*          newShareTree(const char* name, CORBA_Context* ctx);
      

      // misc idl methods
      virtual CORBA_Boolean             sends_events() {return ev_flag;}
      virtual CORBA_Boolean             sends_events(CORBA_Context* ctx);
      virtual CORBA_Boolean             send_events(CORBA_Boolean sw, CORBA_Context* ctx);
      virtual CosEventChannelAdmin_ConsumerAdmin* getConsumerAdmin(CORBA_Context* ctx);
      virtual CORBA_Boolean             registerObject(class CORBA_Object *, const char* job_auth);
      virtual CORBA_Boolean             unregisterObject(class CORBA_Object *, const char* job_auth);

      virtual void                      disconnect_push_supplier();

      virtual void shutdown(CORBA_Context* ctx);      // make non-idl in final version !!!

      // static methods for creation and destruction
      static bool  initialize(CORBA_ORB_ptr o, CORBA_BOA_ptr b); 
                                    // returns false on failure
      static void  exit();
      
      // handling events
      void         updateShareTree(lListElem* ep);
      void         deleteObject(int type, const char* name);
      void         deleteObject(int type, Sge_sge_ulong id);
      void         addObject(int type, const char* name);
      void         addObject(int type, Sge_sge_ulong id);
      void         addEvent(const CORBA_Any& any);
      void         connectEventService();

      // finding objects
      list<Sge_Queue_impl*>::iterator         getQueue(const char* name);
      list<Sge_Complex_impl*>::iterator       getComplex(const char* name);
      list<Sge_Job_impl*>::iterator           getJob(Sge_sge_ulong name);
      list<Sge_Configuration_impl*>::iterator getConfiguration(const char* name);
      list<Sge_Calendar_impl*>::iterator      getCalendar(const char* name);
      list<Sge_Checkpoint_impl*>::iterator    getCheckpoint(const char* name);
      list<Sge_ParallelEnvironment_impl*>::iterator    getParallelEnvironment(const char* name);
      list<Sge_UserSet_impl*>::iterator       getUserSet(const char* name);
      list<Sge_ExecHost_impl*>::iterator      getExecHost(const char* name);
      list<Sge_User_impl*>::iterator          getUser(const char* name);
      list<Sge_Project_impl*>::iterator       getProject(const char* name);
      Sge_ShareTreeNode*                      getShareTreeNode(Sge_ShareTreeNode* node, const char* name);
      
      
      Sge_Queue_impl*                   getQueueImpl(const char* name);
      Sge_Complex_impl*                 getComplexImpl(const char* name);
      Sge_Job_impl*                     getJobImpl(Sge_sge_ulong name);
      Sge_Configuration_impl*           getConfigurationImpl(const char* name);
      Sge_Calendar_impl*                getCalendarImpl(const char* name);
      Sge_Checkpoint_impl*              getCheckpointImpl(const char* name);
      Sge_ParallelEnvironment_impl*     getParallelEnvironmentImpl(const char* name);
      Sge_UserSet_impl*                 getUserSetImpl(const char* name);
      Sge_ExecHost_impl*                getExecHostImpl(const char* name);
      Sge_User_impl*                    getUserImpl(const char* name);
      Sge_Project_impl*                 getProjectImpl(const char* name);
      Sge_SchedConf_impl*               getSchedConfImpl(const char* name);
      Sge_ShareTreeNode_impl*           getShareTreeImpl(const char* name);
      
      
      // misc
      CORBA_Context*                       getMasterContext();

   private:
      // all static since we are a singleton
      static CORBA_BOA_var                               boa;
      static CORBA_ORB_var                               orb;
      static list<Sge_Queue_impl*>                    queues;
      static list<Sge_Complex_impl*>                  complexes;
      static list<Sge_Job_impl*>                      jobs;
      static list<Sge_Configuration_impl*>            configurations;
      static list<Sge_Calendar_impl*>                 calendars;
      static list<Sge_Checkpoint_impl*>               checkpoints;
      static list<Sge_ParallelEnvironment_impl*>      parallelenvironments;
      static list<Sge_UserSet_impl*>                  usersets;
      static list<Sge_ExecHost_impl*>                 exechosts;
      static list<Sge_User_impl*>                     users;
      static list<Sge_Project_impl*>                  projects;
      static Sge_ShareTreeNode_impl*                  sharetree;
      static Sge_SchedConf_impl*                      schedconf;

      static bool                                        ev_flag;
      static deque<CORBA_Any>                            events;
      static pthread_mutex_t                             events_lock;
      static CosNaming_Name_var                          ns_name;
      static CosNaming_NamingContext_var                 ns;
      static CosEventChannelAdmin_EventChannel_var       es;
      static CosEventChannelAdmin_ProxyPushConsumer_var  ev_consumer;
      static int                                         obEventFd[2];
      static CORBA_Context_var                           context;
      static CORBA_String_var                            ior;

   friend class ShutdownHandler;
};

#endif /* __MASTER_IMPL_H */
