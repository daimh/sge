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
// client.cpp
// simple test client for qidl server

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream.h>
#include <iostream.h>
#include <list>
#include <OB/CORBA.h>
#include <OB/CosNaming.h>
#include <OB/Util.h>

#include "elem_codes.h"
#include "Master.h"
#include "Queue.h"
#include "Complex.h"
#include "Job.h"
#include "Checkpoint.h"
#include "Calendar.h"
#include "ShareTreeNode.h"

#include "CosEventComm_skel.h"


extern "C" {
#include "cod_api.h"
#include "cod_all_listsL.h"
#include "cod_str_from_file.h"
}

#ifdef HAVE_STD
using namespace std;
#endif

CORBA_ORB_var     orb;
CORBA_BOA_var     boa;
CosNaming_NamingContext_var ns;
CosNaming_Name_var name;
CORBA_Context_ptr ctx;

// class EventClient
class EventClient_impl : virtual public CosEventComm_PushConsumer_skel {
   public:
      EventClient_impl(Codine_Master_var master, bool printCt = false);
      virtual ~EventClient_impl() {}

      virtual void push(const CORBA_Any& any);
      virtual void disconnect_push_consumer() {cerr << "disconnect!" << endl;}

   private:
      void printContent(Codine_contentSeq& content);
      bool pc;
};

EventClient_impl::EventClient_impl(Codine_Master_var master,
                                 bool printCt) : pc(printCt) {
   CosEventChannelAdmin_ConsumerAdmin_var ca = master->getConsumerAdmin(ctx);
   CosEventChannelAdmin_ProxyPushSupplier_var pps = ca->obtain_push_supplier();
   pps->connect_push_consumer(CosEventComm_PushConsumer::_duplicate(this));
   lInit(nmv);
   cout << "Event client initialized." << endl;
}

void EventClient_impl::printContent(Codine_contentSeq& content) {
   char* s;
   CORBA_ULong u;

   if(!pc)
      return;

   cout << "   Cont: " << endl;

   for(CORBA_ULong i=0; i<content.length(); i++) {
      cout << "      " << lNm2Str(content[i].elem) << ": ";
      switch(lGetType(QU_Type, content[i].elem)) {
         case lStringT:
            content[i].value >>= s;
            cout << s << endl;
            break;
         case lUlongT:
            content[i].value >>= u;
            cout << u << endl;
            break;
         default:
            cout << "unkown" << endl;
            break;
      }
   }
}

void EventClient_impl::push(const CORBA_Any& any) {
   try {
      Codine_event* ev;
      any >>= ev;

      if(!ev) {
         cout << "Receiving unkown event!" << endl;
         cout << "--------------------------------------------------" << endl;
         return;
      }

      cout << "Got event " << ev->count << endl;
      cout << "   Name: " << ev->name << endl;
      cout << "   ID  : " << ev->id << endl;
      cout << "   Obj : ";
      switch(ev->obj) {
         case COD_QUEUE_LIST:
            cout << "Queue" << endl;
            break;
         case COD_CKPT_LIST:
            cout << "Checkpoint" << endl;
            break;
         case COD_EXECHOST_LIST:
            cout << "Exechost" << endl;
            break;
         case COD_PE_LIST:
            cout << "PE" << endl;
            break;
         case COD_COMPLEX_LIST:
            cout << "Complex" << endl;
            break;
         case COD_JOB_LIST:
            cout << "Job" << endl;
            break;
         case COD_CALENDAR_LIST:
            cout << "Calendar" << endl;
            break;
         case COD_CONFIG_LIST:
            cout << "Configuration" << endl;
            break;
         case COD_SC_LIST:
            cout << "SchedConf" << endl;
            break;
         case COD_USERSET_LIST:
            cout << "UserSet" << endl;
            break;
         case COD_PROJECT_LIST:
            cout << "Project" << endl;
            break;
         case COD_SHARETREE_LIST:
            cout << "ShareTree" << endl;
         default:
            break;
      }
      cout << "   Being ";
      switch(ev->type) {
         case Codine_ev_add:
            cout << "added" << endl;
            break;
         case Codine_ev_mod:
            cout << "modified" << endl;
            break;
         case Codine_ev_del:
            cout << "deleted" << endl;
            break;
         default:
            break;
      }
      printContent(ev->changes);
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(CORBA_SystemException& x) {
      cout << "caught exception in push" << endl;
      cout << "Reason: " << x.reason() << endl;
      OBPrintException(x);
   }
   catch(...) {
      cout << "caught other x" << endl;
   }
   return;
}

typedef void (action_func)(Codine_Master_var master);

action_func readwrite;
action_func createQueue;
action_func displayQueues;
action_func removeQueues;
action_func shutdown;
action_func displayComplexes;
action_func qcplx;
action_func qcplxmod;
action_func submitJob;
action_func displayJobs;
action_func removeJobs;
action_func displayCheckpoints;
action_func displayExecHosts;
action_func displayCalendars;
action_func displayShareTree;
action_func createShareTreeNode;
action_func event;
action_func eventcont;
action_func unbind;
action_func doc_queue;
action_func doc_rel;
action_func doc_sub;
action_func toggle_ev;

struct action {
   const char*   name;
   action_func*  func;
   const char*   desc;
   action(const char* n, action_func f, const char* d) : name(n), func(f), desc(d) {}
} actions[] = {
     action("rw", readwrite, "reads and writes no. of slots of first q. endless loop"),
     action("createQueue", createQueue, "prompts the user and creates a new queue"),
     action("displayQueues", displayQueues, "displays queues in an endless loop"),
     action("removeQueues", removeQueues, "prompts the user and removes queues"),
     action("shutdown", shutdown, "shuts down the qidl server"),
     action("displayComplexes", displayComplexes, "displays all complexes once"),
     action("qcplx", qcplx, "displays all queues with their complexes once"),
     action("qcplxmod", qcplxmod, "lets the user modify a q's complexes"),
     action("displayCheckpoints", displayCheckpoints, "display checkpoints"),
     action("displayExecHosts",displayExecHosts,"display execution hosts"),
     action("displayCalendars",displayCalendars,"display calendars"),
     action("submitJob", submitJob, "prompts for a job script and submits the job"),
     action("displayJobs", displayJobs, "displays jobs"),
     action("removeJobs",removeJobs,"prompts the user and removes jobs"),
     action("displayShareTree", displayShareTree, "displays nodes of the sharetree"),
     action("createShareTreeNode", createShareTreeNode, "creates new node in the sharetree"),
     action("event", event, "acts as a simple event client. prints no contents"),
     action("eventcont", eventcont, "acts as a simple event client. prints contents"),
     action("unbind", unbind, "unbinds the master from the naming service"),
     action("doc_1", doc_queue, "the queue example as described in the user doc"),
     action("doc_2", doc_rel, "the object relations example from the user doc"),
     action("doc_3", doc_sub, "the job submit example from the user doc"),
     action("ev_toggle", toggle_ev, "turns events on/off")
  };


void displayQueues(Codine_Master_var master) {
   CORBA_ULong i,x;
   try {
   Codine_QueueSeq_var qs(master->getQueues(ctx));
   CORBA_String_var foo;
   while(1) {
   for(i=0; i<qs->length(); i++) {
      cout << "Queue " << i << endl;
      foo = qs[i]->get_qname(ctx);
      cout << "Name: " << foo << endl;
      foo = qs[i]->get_qhostname(ctx);
      cout << "host: " << foo << endl;
      cout << "prio: " << qs[i]->get_priority(ctx) << endl;
      cout << "type: " << qs[i]->get_qtype(ctx) << endl;
      cout << "slot: " << qs[i]->get_job_slots(ctx) << endl;
      cout << "load thresholds:" << endl;
      Codine_ComplexEntrySeq_var lts = qs[i]->get_load_thresholds(ctx);
      for(x=0; x<lts->length(); x++) {
         cout << "   (" << lts[x].name << "," << lts[x].shortcut;
         cout << "," << lts[x].valtype;
         cout << "," << lts[x].stringval;
         cout << "," << lts[x].relop;
         cout << "," << lts[x].request;
         cout << "," << lts[x].consumable;
         cout << "," << lts[x].forced;
         cout << ")" << endl;
      }

      cout << endl;
      cout << "Calendar: " << endl;
      Codine_Calendar* cal = qs[i]->get_calendar(ctx);
      
      if (cal) {
         cout << "   (" << endl;
         cout << "    " << cal->get_name(ctx) << endl;
         cout << "    " << cal->get_year_calendar(ctx) << endl;
         cout << "    " << cal->get_week_calendar(ctx) << endl;
         cout << "   )" << endl;
      }
      cout << endl;
      
   }
   }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(CORBA_SystemException& x) {
      cout << "caught exception in displayQueues" << endl;
      cout << "Reason: " << x.reason() << endl;
   }
   catch(...) {
      cout << "caught other x" << endl;
   }
   return;
}

void readwrite(Codine_Master_var master) {
   Codine_QueueSeq_var qs(master->getQueues(ctx));
   Codine_Queue_var q = qs[0];

   try {
      while(1) {
      for(int i=0; i<100; i++) {
         cout << "Old slots: " << q->get_job_slots(ctx) << endl;
         q->set_job_slots(i, ctx);
         cout << "New slots: " << q->get_job_slots(ctx) << endl;
      }
      }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in readwrite" << endl;
   }

   return;
}

void removeQueues(Codine_Master_var master) {
   Codine_QueueSeq_var qs(master->getQueues(ctx));

   try {
      char reply;
      for(CORBA_ULong i=0; i<qs->length(); i++) {
         cout << "ready to destroy ";
         CORBA_String_var foo(qs[i]->get_qname(ctx));
         cout << foo << "? ";
         cin >> reply;
         if(reply == 'y')
            qs[i]->destroy(ctx);
      }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in removeQueues" << endl;
   }
}

void createQueue(Codine_Master_var master) {
   try
   {
      char  name[100];
      char  host[100];
      CORBA_String_var str;

      cout << "Name of the q: ";
      cin >> name;
      str = CORBA_string_dup(name);
      Codine_Queue_var  q = master->newQueue(str, ctx);

      cout << "Host of the q: ";
      cin >> host;
      str = CORBA_string_dup(host);
      q->set_qhostname(str, ctx);

      cout << "Adding Queue" << endl;
      q->add(ctx);

      cout << "Now showing qs:" << endl;
      displayQueues(master);
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in createQueue" << endl;
   }
}

void displayComplexes(Codine_Master_var master) {
   try
   {
      Codine_ComplexSeq_var   cs = master->getComplexes(ctx);
      Codine_ComplexEntrySeq_var ces;
      CORBA_String_var foo;

      for(CORBA_ULong i=0; i<cs->length(); i++) {
         foo = cs[i]->get_name(ctx);
         cout << "complex " << foo << endl;
         ces = cs[i]->get_entries(ctx);
         for(CORBA_ULong x=0; x<ces->length(); x++) {
            cout << "   (" << ces[x].name;
            cout << "," << ces[x].shortcut;
            cout << "," << ces[x].valtype;
            cout << "," << ces[x].stringval;
            cout << "," << ces[x].relop;
            cout << "," << ces[x].request;
            cout << "," << ces[x].consumable;
            cout << "," << ces[x].forced;
            cout << ")" << endl;
         }
      }            
         
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in displayComplexes" << endl;
   }
}

void qcplx(Codine_Master_var master) {
   try
   {
      Codine_QueueSeq_var   qs = master->getQueues(ctx);
      Codine_ComplexSeq_var   cs;
      CORBA_String_var foo;
      CORBA_ULong i;
      CORBA_ULong y;
      CORBA_ULong x;

      for(i=0; i<qs->length(); i++) {
         foo = qs[i]->get_qname(ctx);
         cout << "Queue: " << foo << endl;
         cs = qs[i]->get_complex_list(ctx);
         for(y=0; y<cs->length(); y++) {
            foo = cs[y]->get_name(ctx);
            cout << "   Complex: " << foo << endl;
            Codine_ComplexEntrySeq_var ces = cs[y]->get_entries(ctx);
            for(x=0; x<ces->length(); x++) {
               cout << "      (" << ces[x].name;
               cout << "," << ces[x].shortcut;
               cout << "," << ces[x].valtype;
               cout << "," << ces[x].stringval;
               cout << "," << ces[x].relop;
               cout << "," << ces[x].request;
               cout << "," << ces[x].consumable;
               cout << "," << ces[x].forced;
               cout << ")" << endl;
            }
         }
      }            
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in qcplx" << endl;
   }
}

void qcplxmod(Codine_Master_var master) {
   try
   {
      Codine_QueueSeq_var   qs = master->getQueues(ctx);
      Codine_ComplexSeq_var   cs;
      Codine_ComplexSeq_var   cplxs;
      Codine_ComplexEntrySeq_var ces;
      CORBA_String_var foo;
      CORBA_ULong i;
      CORBA_ULong y;
      CORBA_ULong x;
      char choice;

      while(true) {
         for(i=0; i<qs->length(); i++) {
            foo = qs[i]->get_qname(ctx);
            cout << i << ": " << foo << endl;
         }
         cout << "Which q (" << i << " for exit): ";
         cin >> i;
         if(i == qs->length())
            break;

         cs = qs[i]->get_complex_list(ctx);
         for(y=0; y<cs->length(); y++) {
            foo = cs[y]->get_name(ctx);
            cout << "   " << y << ": " << foo << endl;
            ces = cs[y]->get_entries(ctx);
            for(x=0; x<ces->length(); x++) {
               cout << "      (" << ces[x].name;
               cout << "," << ces[x].shortcut;
               cout << "," << ces[x].valtype;
               cout << "," << ces[x].stringval;
               cout << "," << ces[x].relop;
               cout << "," << ces[x].request;
               cout << "," << ces[x].consumable;
               cout << "," << ces[x].forced;
               cout << ")" << endl;
            }
         }
         cout << "(A)dd, (R)emove, (Q)uit: ";
         cin >> choice;
         switch(choice) {
            case 'a':
            case 'A':
               cplxs = master->getComplexes(ctx);
               for(y=0; y<cplxs->length(); y++) {
                  foo = cplxs[y]->get_name(ctx);
                  cout << "   " << y << ": " << foo << endl;
                  ces = cplxs[y]->get_entries(ctx);
                  for(x=0; x<ces->length(); x++) {
                     cout << "      (" << ces[x].name;
                     cout << "," << ces[x].shortcut;
                     cout << "," << ces[x].valtype;
                     cout << "," << ces[x].stringval;
                     cout << "," << ces[x].relop;
                     cout << "," << ces[x].request;
                     cout << "," << ces[x].consumable;
                     cout << "," << ces[x].forced;
                     cout << ")" << endl;
                  }
               }
               cout << "which one: ";
               cin >> y;
               if(y < cplxs->length())
                  cs->append(Codine_Complex::_duplicate(cplxs[y]));
               break;
            case 'r':
            case 'R':
               cout << "which one: ";
               cin >> y;
               if(y < cs->length()) {
                  cs->remove(y);
                  qs[i]->set_complex_list(cs, ctx);
               }
               break;
            default:
               break;
         }
      }            
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in qcplx" << endl;
   }
}

void displayJobs(Codine_Master_var master) {
   try
   {
      
      Codine_JobSeq_var   js = master->getJobs(ctx);
      CORBA_ULong y;
      
      
      for (CORBA_ULong i=0; i < js->length(); i++) {
         
         cout << "job " << js[i]->get_job_number(ctx) << endl;
         cout <<         js[i]->get_owner(ctx);
         cout << ", " << js[i]->get_script_file(ctx);
         cout << ", " << js[i]->get_submission_time(ctx);
         cout << ", " << js[i]->get_end_time(ctx);
         cout << ", " << js[i]->get_account(ctx);
         cout << ", " << js[i]->get_cwd(ctx);
         cout << ", " << js[i]->get_merge_stderr(ctx);     
         cout << ", " << js[i]->get_mail_options(ctx);
         cout << ", " << js[i]->get_notify(ctx);
         cout << ", " << js[i]->get_restart(ctx);
         cout << endl;
         
         cout << "STDERR Paths:" << endl;
         Codine_PathNameSeq_var pn = js[i]->get_stderr_path_list(ctx);
         for (y=0;y<pn->length();y++)
         {
           cout << "  (" << pn[y].path << ", " << pn[y].host << ")" << endl;
         
         }
         cout << endl;
         cout << endl;
         
         cout << "Custom Attributes:" << endl;
         Codine_VariableSeq_var vs = js[i]->get_context(ctx);
         for(y=0; y<vs->length(); y++) 
            cout << "   (" << vs[y].variable << "," << vs[y].value << ")" << endl;
         cout << endl;
         cout << endl;

         cout << "STDOUT Paths:" << endl;
         pn = js[i]->get_stdout_path_list(ctx);
         for (y=0;y<pn->length();y++)
         {
           cout << "  (" << pn[y].path << ", " << pn[y].host << ")" << endl;
         
         }
         cout << endl;
         cout << endl;
         
         
         cout << "Mail Recipients:" << endl;
         Codine_MailRecipientSeq_var mr = js[i]->get_mail_list(ctx);
         for (y=0;y<mr->length();y++)
           cout << "  (" << mr[y].user << ", " << mr[y].host << ")" << endl;
         cout << endl;
         cout << endl;

               
         cout << "PE Range:" << endl;
         Codine_RangeSeq_var rn = js[i]->get_pe_range(ctx);
         for (y=0;y<rn->length();y++)
           cout << "  (" << rn[y].min << ", " << rn[y].max << ")" << endl;
         cout << endl;
         cout << endl;

         Codine_TaskSeq_var tasks = js[i]->get_ja_tasks(ctx);
         for(y=0; y<tasks->length(); y++) {
            cout << "Job State: " << flush;
            Codine_cod_ulong state = tasks[y].status;
            if(state & JIDLE) cout << "idle ";
            if(state & JTRANSITING) cout << "transiting ";
            if(state & JRUNNING) cout << "running ";
            if(state & JFINISHED) cout << "finished ";
            if(state & JHELD) cout << "held ";
            if(state & JSUSPENDED) cout << "suspended ";
            if(state & JSUSPENDED_ON_THRESHOLD) cout << "suspended_on_threshold ";
            if(state & JWAITING) cout << "waiting ";
            if(state & JQUEUED) cout << "queued ";
            if(state & JDELETED) cout << "deleted ";
            if(state & JEXITING) cout << "exiting ";
            if(state & JERROR) cout << "error ";

            cout << endl << "Running on Queue: ";
            if(CORBA_is_nil(tasks[y].master_queue))
               cout << "none";
            else
               cout << tasks[y].master_queue->get_qname(ctx);
            cout << endl;
         }
      }
         
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in displayJobs" << endl;
   }
}


void recurseTree(Codine_ShareTreeNode* node,int indent)
{
   for (int i=0; i < (indent*3); i++) {
      cout << " ";
   }
   cout << node->get_name(ctx) << " , "<< node->get_shares(ctx) << endl;
   
   Codine_ShareTreeNodeSeq_var chs = node->get_children(ctx);
   if (chs->length() > 0) {
      indent ++;
      for (int i=0; i < chs->length(); i++) {
         //cout << chs[i]->get_name(ctx) << endl;
         recurseTree(chs[i],indent);
      }
   }
}


void displayShareTree(Codine_Master_var master)
{
   
   try {
      CORBA_ULong y;
      Codine_ShareTreeNode* st = master->getShareTree(ctx);
      if (st) {
         recurseTree(st,0);
      }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in displayShareTree" << endl;
   }

}


/*Codine_ShareTreeNode* getNodeByName(Codine_ShareTreeNode* node,const char* name)
{
   Codine_ShareTreeNode* temp=NULL;
   if (!strcmp(node->get_name(ctx), name)) {
      return node;
   }
   
   Codine_ShareTreeNodeSeq_var chs = node->get_children(ctx);
   
   if (chs->length() > 0) {
      for (int i=0; i < chs->length(); i++) {
         temp = getNodeByName(chs[i], name);
         if (temp) 
            return Codine_ShareTreeNode::_duplicate(temp);
      }
   }

   return NULL;
}*/


void buildPath(list<char*>& path_list, char* path)
{
   char* nodename = new char[strlen(path)+1];
   nodename = strtok(path,"/");
   while (nodename) {
      path_list.push_back(nodename);
      nodename = strtok(NULL,"/");
   }
}


Codine_ShareTreeNode* findNodeByPath(Codine_ShareTreeNode* node, list<char*>::iterator it, list<char*>& path)
{
   Codine_ShareTreeNode* temp;
   Codine_ShareTreeNodeSeq_var children;
    
   if (!node)
      return NULL;
      
   if (it == path.end())
      return Codine_ShareTreeNode::_duplicate(node);

   children = node->get_children(ctx);
      
   for (int i=0; i<children->length(); i++) {
      if (!strcmp( children[i]->get_name(ctx), *it))
         return Codine_ShareTreeNode::_duplicate(findNodeByPath(children[i],(++it), path));
   }
   
   return NULL;
}


void createShareTreeNode(Codine_Master_var master)
{
   
   try {
      Codine_ShareTreeNode* pnode=NULL, *nnode=NULL;
      Codine_ShareTreeNode* st = master->getShareTree(ctx);
      list <char*> path_list;
      
      if (st) {
         string name,pname;
         CORBA_ULong shares;
         char* nodepath, *c, *lc = NULL;
                  
         cout << "path of new node: ";          
         cin >> pname;
         cout << "name of new node: ";
         cin >> name;
         cout << "shares of new node: ";
         cin >> shares;
         
         path_list.clear();
         buildPath(path_list, (char*)pname.c_str());
         
         list <char*>::iterator it = path_list.begin();
         
         pnode = findNodeByPath(st, it, path_list);
         
         if (!pnode) {
            cout << "Unable to locate parent node in sharetree" << endl;
            cout << "no modifications to sharetree" << endl;
         }
         else {
            nnode = pnode->newLeaf(name.c_str(), shares, ctx);
            if (!nnode) {
               cout << "Codine Error:" << endl;
               cout << "denied: found node \""<<name.c_str()<<"\" twice under node \""<<pnode->get_name(ctx)<< "\""<<endl;
            }
         }
      }
   }
   
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in createShareTreeNode" << endl;
   }

}


void displayCheckpoints(Codine_Master_var master)
{
   try
   {
      Codine_CheckpointSeq_var  ckpts = master->getCheckpoints(ctx);
      CORBA_ULong y;  

      for (CORBA_ULong i=0; i < ckpts->length(); i++) {
         
         cout << "Checkpoint:   " << ckpts[i]->get_name(ctx) << endl << endl;
         cout <<                  ckpts[i]->get_interface(ctx) << endl;
         cout <<                  ckpts[i]->get_ckpt_command(ctx) << endl;
         cout <<                  ckpts[i]->get_migr_command(ctx) << endl;
         cout <<                  ckpts[i]->get_rest_command(ctx) << endl;
         cout <<                  ckpts[i]->get_clean_command(ctx) << endl;
         cout <<                  ckpts[i]->get_ckpt_dir(ctx) << endl;
         cout << endl;

         cout << " Queues: " << endl;
         Codine_QueueSeq_var qs = ckpts[i]->get_queue_list(ctx);
         for (y=0; y<qs->length(); y++)
         {
           cout << "  (" << qs[y]->get_qname(ctx) << ")" << endl;
         
         }
         cout << endl;
         cout <<                 ckpts[i]->get_when(ctx) << endl;
         cout <<                 ckpts[i]->get_signal(ctx) << endl;
         cout << endl;
         cout << endl;
         
      }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in displayCheckpoints" << endl;
   }
}

void displayCalendars(Codine_Master_var master)
{
   try
   {
      Codine_CalendarSeq_var  cals = master->getCalendars(ctx);
      CORBA_ULong y;  

      for (CORBA_ULong i=0; i < cals->length(); i++) {
         
         cout << "calendar " << cals[i]->get_name(ctx) << endl << endl;
         cout <<                  cals[i]->get_week_calendar(ctx) << endl;
         cout <<                  cals[i]->get_year_calendar(ctx) << endl;
         cout << endl;
      }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in displayCalendars" << endl;
   }
}


void displayExecHosts(Codine_Master_var master)
{
   try
   {
      Codine_ExecHostSeq_var  ehs = master->getExecHosts(ctx);

      for (CORBA_ULong i=0; i < ehs->length(); i++) {
         cout << "host " << ehs[i]->get_name(ctx) << endl << endl;
         cout << endl;
      }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in displayExecHosts" << endl;
   }
}


void removeJobs(Codine_Master_var master) {
   Codine_JobSeq_var js(master->getJobs(ctx));

   try {
      char reply;
      for(CORBA_ULong i=0; i<js->length(); i++) {
         cout << "ready to destroy job ";
         cout << js[i]->get_job_number(ctx);
         cout << "? ";
         cin >> reply;
         if(reply == 'y')
            js[i]->destroy(ctx);
      }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in removeJobs" << endl;
   }
}


void submitJob(Codine_Master_var master) {
   try
   {
      Codine_Job_var   job = master->newJob(ctx);
      int len;
       
      string script, name;
      cout << "name: " << flush;
      cin >> name;
      cout << "Script: " << flush;
      cin >> script;

      //Codine_contentSeq_var state = job->get_content(ctx); 
      Codine_contentSeq state;
      state.length(5);
      state[0].elem = Codine_JB_job_name;
      state[0].value <<= CORBA_string_dup(name.c_str());
      state[1].elem = Codine_JB_script_file;
      state[1].value <<= CORBA_string_dup(script.c_str());
      state[2].elem = Codine_JB_script_ptr;
      state[2].value <<= CORBA_string_dup(str_from_file((char*)script.c_str(), &len));
      state[3].elem = Codine_JB_script_size;
      state[3].value <<= (Codine_cod_ulong)len;
      state[4].elem = Codine_JB_priority;
      state[4].value <<= (Codine_cod_ulong)BASE_PRIORITY;
      job->set_content(state, ctx);

      //job->set_job_name(name.c_str(), ctx);
      //job->set_script_file(script.c_str(), ctx);
      //job->set_script_ptr(str_from_file((char*)script.c_str(), &len), ctx);
      //job->set_script_size(len, ctx);
      //job->set_directive_prefix("#$", ctx);
      //job->set_priority(BASE_PRIORITY, ctx);
      
     
      Codine_PathNameSeq_var pns = new Codine_PathNameSeq;
      pns->length(3);
      pns[0].path = CORBA_string_dup("/bin/sh");
      pns[0].host = CORBA_string_dup("nori");
      pns[1].path = CORBA_string_dup("/bin/sh");
      pns[1].host = CORBA_string_dup("");
      pns[2].path = CORBA_string_dup("/bin/tcsh");
      pns[2].host = CORBA_string_dup("");
      job->set_shell_list(pns, ctx);

      job->submit(ctx);
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in submitJob" << endl;
   }
}

void doc_queue(Codine_Master_var master) {
   try
   {
      // local variables
      Codine_QueueSeq_var           qs;
      Codine_ComplexEntrySeq_var    lts;
      Codine_ComplexSeq_var         cs;
      Codine_Queue_var              q;
      CORBA_ULong                   i;
      
      // get the first queue from qmaster
      qs = master->getQueues(ctx);
      if(qs->length() < 1) {
         cerr << "need at least one queue." << endl;
         return;
      }
      q = qs[0];

      // print out simple attributes
      cout << "QueueName: " << q->get_qname(ctx) << endl;
      cout << "on host  : " << q->get_qhostname(ctx) << endl;
      cout << "slots    : " << q->get_job_slots(ctx) << endl;
      
      // print out compound attributes
      lts = q->get_load_thresholds(ctx);
      cout << "load thresholds:" << endl;
      for(i=0; i<lts->length(); i++) {
         cout << "   name: " << lts[i].name << endl;
         cout << "   valtype: " << lts[i].valtype << endl;
         cout << "   stringval: " << lts[i].stringval << endl;
         cout << "   ---------------------------------" << endl;
      }

      // print out relations to and contents of other objects
      cs = q->get_complex_list(ctx);
      cout << "attached complexes:" << endl;
      for(i=0; i<cs->length(); i++)
         cout << "   " << cs[i]->get_name(ctx) << endl;

      // modify slot number
      cout << endl << "Setting slots to 5...";
      q->set_job_slots(5, ctx);
      cout << "done." << endl;
      
      // delete load thresholds
      cout << "Removing any load thresholds...";
      lts->length(0);
      q->set_load_thresholds(lts, ctx);
      cout << "done." << endl;

      // checking modifications
      cout << endl << "Checking Modifications:" << endl;
      cout << "slots    : " << q->get_job_slots(ctx) << endl;
      lts = q->get_load_thresholds(ctx);
      if(lts->length() == 0)
         cout << "No load thresholds set anymore." << endl;
      else
         cout << "Load thresholds still set!" << endl;

      // append a load threshold again
      cout << endl << "Appending a load threshold...";
      lts->length(1);
      lts[0].name = CORBA_string_dup("load_avg");
      lts[0].shortcut = CORBA_string_dup("");
      lts[0].valtype = 0;
      lts[0].stringval = CORBA_string_dup("175");
      lts[0].relop = 0;
      lts[0].request = 0;
      lts[0].consumable = 0;
      lts[0].forced = 0;
      lts[0].default_val = CORBA_string_dup("0");
      q->set_load_thresholds(lts, ctx);
      cout << "done." << endl;

      // checking again
      cout << endl << "load thresholds now:" << endl;
      for(i=0; i<lts->length(); i++) {
         cout << "   name: " << lts[i].name << endl;
         cout << "   valtype: " << lts[i].valtype << endl;
         cout << "   stringval: " << lts[i].stringval << endl;
         cout << "   ---------------------------------" << endl;
      }
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in doc_queue" << endl;
   }
}

void doc_rel(Codine_Master_var master) {
   try
   {
      // local variables
      Codine_QueueSeq_var     qs;
      Codine_ComplexSeq_var   cs;
      Codine_ComplexSeq_var   cplxs;
      CORBA_ULong             q;
      CORBA_ULong             c;
      char                    choice;

      // get all queues and complexes
      qs = master->getQueues(ctx);
      cplxs = master->getComplexes(ctx);

      // prompt the user
      while(true) {
         // display queues and request one
         for(q=0; q<qs->length(); q++)
            cout << q << ": " << qs[q]->get_qname(ctx) << endl;
 
         cout << "Which queue (" << q << " for exit): ";
         cin >> q;
         if(q == qs->length())
            break;

         // get and display the queue's attached complexes
         cs = qs[q]->get_complex_list(ctx);
         for(c=0; c<cs->length(); c++)
            cout << "   " << c << ": " << cs[c]->get_name(ctx) << endl;

         // prompt for an action
         cout << "(A)dd, (R)emove, (Q)uit: ";
         cin >> choice;
         switch(choice) {
            // add a complex
            case 'a':
            case 'A':
               // display all avaiable complexes and request one
               for(c=0; c<cplxs->length(); c++)
                  cout << "   " << c << ": " << cplxs[c]->get_name(ctx) << endl;
               cout << "Which complex: ";
               cin >> c;
               
               // append the chosen complex and set the new complex list
               if(c < cplxs->length()) {
                  cs->append(Codine_Complex::_duplicate(cplxs[c]));
                  qs[q]->set_complex_list(cs, ctx);
               }
               break;
            // remove a complex
            case 'r':
            case 'R':
               // prompt for one and remove it from the list
               cout << "Which complex: ";
               cin >> c;
               if(c < cs->length()) {
                  cs->remove(c);   // ORBacus specific sequence function
                  qs[q]->set_complex_list(cs, ctx);
               }
               break;
            // quit
            default:
               break;
         }
      } 
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in doc_rel" << endl;
   }
}

void doc_sub(Codine_Master_var master) {
   try
   {
      // local variables
      Codine_Job_var   job;
      Codine_PathNameSeq_var pns;
      int len;
      string script, name;
       
      // request a new job from qidl
      // this is NOT publicly accessible
      job = master->newJob(ctx);
      
      // prompt for a name and a job script
      cout << "name: ";
      cin >> name;
      cout << "Script: ";
      cin >> script;

      // set the necessary values
      job->set_job_name(name.c_str(), ctx);
      job->set_script_file(script.c_str(), ctx);
      job->set_script_ptr(str_from_file((char*)script.c_str(), &len), ctx);
      job->set_script_size(len, ctx);
      
      // set the shell list
      pns = new Codine_PathNameSeq;
      pns->length(1);
      pns[0].path = CORBA_string_dup("/bin/sh");
      pns[0].host = CORBA_string_dup("");
      job->set_shell_list(pns, ctx);

      // do actual submit
      job->submit(ctx);
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(...) {
      cout << "caught exception in doc_sub" << endl;
   }
}

void event(Codine_Master_var master) {
   try
   {
      CosEventComm_PushConsumer_var ev = new EventClient_impl(master);
      boa->impl_is_ready(CORBA_ImplementationDef::_nil());
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(CORBA_SystemException& se) {
      OBPrintException(se);
   }
   catch(...) {
      cout << "caught exception in event" << endl;
   }
}

void toggle_ev(Codine_Master_var master) {
   try
   {
      int i=0;
      do {
         if(i==1)
            master->send_events(true, ctx);
         if(i==2)
            master->send_events(false, ctx);
         cout << "Currently the master sends ";
         if(!master->sends_events(ctx))
            cout << "no ";
         cout << "events." << endl << "Enter: (1) to turn events on" << endl;
         cout << "       (2) to turn events off" << endl;
         cout << "       (0) to exit" << endl;
         cout << "Choice: "; cin >> i;
      } while(i);
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(CORBA_SystemException& se) {
      OBPrintException(se);
   }
   catch(...) {
      cout << "caught exception in ev_toggle" << endl;
   }
}
void eventcont(Codine_Master_var master) {
   try
   {
      CosEventComm_PushConsumer_var ev = new EventClient_impl(master, true);
      boa->impl_is_ready(CORBA_ImplementationDef::_nil());
   }
   catch(Codine_ObjDestroyed& x) {
      cout << "Obj not exist" << endl;
   }
   catch(Codine_Error& x) {
      cout << "Codine Error: " << endl;;
      for(CORBA_ULong i=0; i<x.answer.length(); i++)
         cout << x.answer[i].text;
   }
   catch(CORBA_SystemException& se) {
      OBPrintException(se);
   }
   catch(...) {
      cout << "caught exception in eventcont" << endl;
   }
}

void unbind(Codine_Master_var master) {
   ns->unbind(name);
}

void shutdown(Codine_Master_var master) {
   master->shutdown(ctx);
}


void printActions() {
   for(unsigned int i=0; i<sizeof(actions)/sizeof(action); i++)
      cerr << " * " << actions[i].name << ": " << actions[i].desc << endl;
}

void makeContext(CORBA_Context* ctx) {
   CORBA_Any   any;

   char host[100];
   char temp[1000];
   gethostname(host, 100);
   sprintf(temp, "%d:%d:%d:%d:%s", getuid(),
                                   geteuid(),
                                   getgid(),
                                   getegid(),
                                   host);
   any <<= temp;
   ctx->set_one_value("cod_auth", any);
};


CORBA_Object_ptr getMasterViaEnv(CORBA_ORB_var& orb) {
   char *master_ior;

   if (!(master_ior = getenv("SGE_MASTER_IOR")))
      master_ior = getenv("SGE_MASTER_IOR");  

   if(master_ior)
      return orb->string_to_object(master_ior);
   else
      return CORBA_Object::_nil();
}

CORBA_Object_ptr getMasterViaNameService(CORBA_ORB_var& orb) {
   char *cell;
   CORBA_Object_ptr obj = CORBA_Object::_nil();

   cell = getenv("SGE_CELL");
   if (!cell)
      cell = "default";

   cout << "Resolving name service...";
   try {
      CORBA_Object_var ns_obj = orb->resolve_initial_references("NameService");
      if(CORBA_is_nil(ns_obj)) {
         cerr << "Sorry, no ns_obj" << endl;
         return obj;
      }
      cout << "done" << endl;
      ns = CosNaming_NamingContext::_narrow(ns_obj);
      if(CORBA_is_nil(ns)) {
         cerr << "Sorry, no ns" << endl;
         return obj;
      }
      name = new CosNaming_Name();
      name->length(2);
      name[0].id = CORBA_string_dup(cell);
      name[0].kind = CORBA_string_dup("");
      name[1].id = CORBA_string_dup("cod_qidl");
      name[1].kind = CORBA_string_dup("");
   
      cout << "resolving master object...";
      obj = ns->resolve(name);
      cout << "done" << endl;
   }
   catch(...) {
      cout << endl << "master not found via nameservice." << endl;
   }

   return obj;
}

CORBA_Object_ptr getMasterViaFile(CORBA_ORB_var& orb) {
   CORBA_Object_ptr obj = CORBA_Object::_nil();

   char ref[1024];
   char* croot;

   croot = getenv("SGE_ROOT");
   if (!croot) {
      cout << "no SGE_ROOT set, good-bye." << endl;
      return obj;
   }
   string filename = croot;
   filename += "/default/common/master.ior";
   cout << filename << endl;
   ifstream ref_file(filename.c_str());
   if(ref_file) 
      ref_file >> ref;
   ref_file.close();
   cout << "string to object..." << endl;
   obj = orb -> string_to_object(ref);

   return obj;
}

// cmd line syntax:
// client <action>
// <action>: one of ... (see the list at the top)

int main(int argc, char** argv) {
   try {
      if(argc < 2) {
         cerr << "usage: client <action>" << endl;
         cerr << "where <action> is one of:" << endl;
         printActions();
         return 1;
      }

      cout << "initing ORB...";
      orb = CORBA_ORB_init(argc, argv);
      if(!orb) {
         cerr << "could not init ORB." << endl;
         return 1;
      }
      cout << "done"<< endl;
      
      cout << "initing BOA...";
      boa = orb->BOA_init(argc, argv);
      if(!boa) {
         cerr << "could not init BOA." << endl;
         return 1;
      }
      cout << "done" << endl;

      // trying to get the master's object reference
      CORBA_Object_var obj;
      obj = getMasterViaEnv(orb);
      if(CORBA_is_nil(obj))
         obj = getMasterViaNameService(orb);
      if(CORBA_is_nil(obj))
         obj = getMasterViaFile(orb);
      if(CORBA_is_nil(obj)) {
         cerr << "could not get master's object reference." << endl;
         return 1;
      }

      Codine_Master_var master = Codine_Master::_narrow(obj);
      if(!master) {
         cerr << "could not narrow." << endl;
         return 1;
      }
      
      // make context
      orb->get_default_context(ctx);
      makeContext(ctx);

      unsigned int i;
      for(i=0; i<sizeof(actions)/sizeof(action); i++)
         if(!strcmp(argv[1], actions[i].name)) {
            actions[i].func(master);
            break;
         }

      // none found ?
      if(i == sizeof(actions)/sizeof(action)) {
         cerr << "Could not find action '" << argv[1] << endl;
         cerr << "Possible choices are: " << endl;
         printActions();
      }
   }
   catch(CORBA_SystemException& x) {
      OBPrintException(x);
   }
   catch(...) {
      cerr << "Caught exception." << endl;
      return 1;
   }

   return 0;
}
