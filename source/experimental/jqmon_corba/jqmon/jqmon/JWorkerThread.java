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
package jqmon;

import Codine.*;
import org.omg.CosNaming.*;
import CosEventComm.*;
import org.omg.CORBA.*;

import java.lang.*;
import java.util.*;
import java.io.*;

import jqmon.*;
import jqmon.util.*;
import jqmon.debug.*;
import jqmon.events.*;
import jcodine.*;


// this class represents the JWorkerThread.
// it sends DPrint-Messages to the attached JDPrintThread,
// if there exists one
//	
//	@@author     Michael Roehrl
// modified by Hartmut Gilde      
//	@@version 0,01
 
public class JWorkerThread extends Thread {

	// indicates if the thread should be killed
	protected boolean          end            = false;
	protected JDebug           debug          = null;
   protected JAnswerList      answerList     = null;
   
   protected JCalendarList    calendarList   = null;
   protected JCheckpointList  checkpointList = null;
	protected JComplexList     complexList    = null;
	protected JHostList        hostList       = null;
	protected JQueueList       queueList      = null;

   protected JUpdateCalendarList    updateCalendarList   = null;
   protected JUpdateCheckpointList  updateCheckpointList = null;
	protected JUpdateComplexList     updateComplexList    = null;
	protected JUpdateHostList        updateHostList       = null;
	protected JUpdateQueueList       updateQueueList      = null;


   //------------------------------------------------------
   // added by Laurentiu
   // arguments from application command line
	String[]          args  = null; 
	// properties of the application
	Properties        prop  = null;
	protected ORB     orb   = null;
	protected BOA     boa   = null;
	protected Master  master= null;
	protected Context ctx   = null;	
	
   // default constructor
   // instantiates a JWorkerThread without a JDPrintThread. 
   // normally a JDPrintThread should be defined.
   public JWorkerThread(JDebug                d, 
                        JUpdateCalendarList   cal,
                        JUpdateCheckpointList ck,
								JUpdateComplexList    cx,
								JUpdateHostList       eh, 
								JUpdateQueueList      qu, 
								String[] a,
								Properties p) {
      
      debug 	  		      = d;
      updateCalendarList   = cal;
      updateCheckpointList = ck;
		updateComplexList    = cx;
		updateHostList       = eh;
		updateQueueList      = qu;

		answerList 		 = new JAnswerList      (10,10);
      calendarList    = new JCalendarList    (10,10);
      checkpointList  = new JCheckpointList  (10,10);
		complexList		 = new JComplexList     (10,10);
		hostList	  		 = new JHostList        (10,10);
		queueList  		 = new JQueueList       (10,10);

		prop = p;
		args = a;
   }
   //--------------------------------------------------------------------------
   
   // keeps the thread running in an infinite loop 
   // can only be suspended with a 'stop'
   public void run() {
		contact();
      try {
         while( !end ) {
            work();
            sleep(1000);
         }
      } 
      catch (InterruptedException e) {
      }
      catch (ThreadDeath td) {
			cleanup();
         throw td; // has to be thrown again
      }

		cleanup();
	}


   // does the complete work. 
   // is invoked through run() , because only this way 
   // synchronized processing is guaranteed 
   protected synchronized void work() {
		
   }

   // set the JDPrintThread.
   // if it is Null no JDPrintStrings are processed
   public synchronized void endThread() {
		end = true;
	}

	protected void cleanup() {
	}


   protected synchronized void contact() {
      debug.DENTER("JWorkerThread.contact");

        try{
            // create and initialize the ORB
            orb = ORB.init(args, prop);
            
            //read ior-file
            /*FileInputStream fin = new FileInputStream("Env.properties");
            prop.load(fin);
            fin.close();*/             
            
            String refFile = new String(System.getProperty("grd.root", System.getProperty("codine.root")) + "/default/common/master.ior");
            
            String ref = null;
            try
            {
               java.io.BufferedReader in = new java.io.BufferedReader(new java.io.FileReader(refFile));
               ref = in.readLine();
            }
            catch(java.io.IOException ex)
            {
               debug.DPRINTF("Can't read from '" + ex.getMessage() + "`");
					System.exit(1);
            }
 
            // get the master reference
            org.omg.CORBA.Object objRef = orb.string_to_object(ref);
            master = MasterHelper.narrow(objRef);
 
            // get the root naming context
            //NamingContext ncRef = NamingContextHelper.narrow(objRef);
            //NameComponent nc = new NameComponent("Master", "");
            //NameComponent path[] = {nc};
            //Master masterRef = MasterHelper.narrow(ncRef.resolve(path));
 
            //initialize context
            ctx = orb.get_default_context();
            Any any = orb.create_any();

            //insert your uid, euid, gid, egid 
            any.insert_string("278:278:140:140:gloin");
            ctx.set_one_value("cod_auth", any);

               
            try {
               // get the calendars 
               debug.DPRINTF("Print the calendars");
               CalendarSeqHolder cals = new CalendarSeqHolder(master.getCalendars(ctx));
               for(int i = 0; i < cals.value.length; i++){
						JCalendar calendar = new JCalendar(debug, cals.value[i], ctx);
                  debug.DPRINTF("Calendar " + (cals.value[i].get_name(ctx)) );
                  calendar.setYearCalendar(cals.value[i].get_year_calendar(ctx));
                  calendar.setWeekCalendar(cals.value[i].get_week_calendar(ctx));
                  calendarList.addElement(calendar);
               }

               // get the checkpoints 
               debug.DPRINTF("Print the checkpoints");
               CheckpointSeqHolder ckpts = new CheckpointSeqHolder(master.getCheckpoints(ctx));
               for(int i = 0; i < ckpts.value.length; i++){
						JCheckpoint checkpoint = new JCheckpoint(debug, ckpts.value[i], ctx);
                  debug.DPRINTF("Checkpoint " + (ckpts.value[i].get_name(ctx)) );
                  checkpoint.setInterface(ckpts.value[i].get_interface(ctx));
                  checkpoint.setCkptCommand(ckpts.value[i].get_ckpt_command(ctx));
                  checkpoint.setMigrCommand(ckpts.value[i].get_migr_command(ctx));
                  checkpoint.setRestCommand(ckpts.value[i].get_rest_command(ctx));
                  checkpoint.setCkptDir(ckpts.value[i].get_ckpt_dir(ctx));
                  checkpoint.setWhen(ckpts.value[i].get_when(ctx));
                  checkpoint.setSignal(ckpts.value[i].get_signal(ctx));
                  checkpoint.setCleanCommand(ckpts.value[i].get_clean_command(ctx));
                  checkpointList.addElement(checkpoint);
               }
               
               // get the complexes 
               debug.DPRINTF("Print the complexes");
               ComplexSeqHolder cs = new ComplexSeqHolder(master.getComplexes(ctx));
               for(int i = 0; i < cs.value.length; i++){
/*                old version 
                  JComplex complex = new JComplex(debug);
                  init(complex, cs.value[i]);
*/
						JComplex complex = new JComplex(debug, cs.value[i], ctx);
                  debug.DPRINTF("Complex " + (new Integer(i)).toString() );
                  complexList.addElement(complex);
               }
               
               // get the hosts 
               debug.DPRINTF("Print the hosts");
               ExecHostSeqHolder hs = new ExecHostSeqHolder(master.getExecHosts(ctx));
               for(int i = 0; i < hs.value.length; i++){
						JHost host = new JHost(debug, hs.value[i], ctx);
                  debug.DPRINTF("Host " + (hs.value[i].get_name(ctx)) );
                  host.setLtHeardFrom(hs.value[i].get_lt_heard_from(ctx));
                  hostList.addElement(host);
               }
               
               // print all the queues with some of their information
					debug.DPRINTF("Print the queues");
               QueueSeqHolder qs = new QueueSeqHolder(master.getQueues(ctx));
               for(LongHolder i = new LongHolder(0); i.value < qs.value.length; i.value++){
                  JQueue queue = new JQueue(debug);
						//System.out.println("Queue " + i.value );
                  debug.DPRINTF("Queue " + i.value );
						String foo;
                  foo = qs.value[(int)i.value].get_qname(ctx);
						queue.setQName(foo);
                  //System.out.println("Name: " + foo );
                  debug.DPRINTF("Name: " + foo );
						foo = qs.value[(int)i.value].get_qhostname(ctx);
						queue.setQHostName(foo);
                  //System.out.println("host: " + foo );
                  debug.DPRINTF("host: " + foo );
						//System.out.println("prio: " + qs.value[(int)i.value].get_priority(ctx) );
                  //System.out.println("type: " + qs.value[(int)i.value].get_qtype(ctx) );
                  queue.setJobSlots( qs.value[(int)i.value].get_job_slots(ctx) );
						//debug.DPRINTF(qs.value[(int)i.value].get_job_slots(ctx));
						//System.out.println("slot: " + qs.value[(int)i.value].get_job_slots(ctx) );
                 
						// get the complex list
						debug.DPRINTF("Complexes: ");
						cs = new ComplexSeqHolder(qs.value[(int)i.value].get_complex_list(ctx));
						JComplexList	  cl = new JComplexList();
						for(int j = 0; j < cs.value.length; j++) {

/* 						old version
							JComplex complex = new JComplex(debug);
							complex.setName(cs.value[j].get_name(ctx));
							debug.DPRINTF(cs.value[j].get_name(ctx));
							cl.addElement(complex);
*/
							JComplex complex = complexList.getComplex(cs.value[j]);
							if(complex == null) {
								// very very bad thing
								debug.DPRINTF("very very bad thing");
							}						
							else {
								// you take the reference from the complexList
								cl.addElement(complex);
							}
						}						
						queue.setComplexList(cl);
 
						queueList.addElement((java.lang.Object)queue);
						
               };
 					
			      //create a queue               
               //...
            }
            catch(Codine.Error x){
               System.out.println("Codine Error");
            }
        } 
		  catch (Exception e) {
            //System.out.println("ERROR : " + e) ;
            debug.DPRINTF("ERROR : " + e );
				e.printStackTrace(System.out);
        }

		// WE DON'T USE JNI
		//contactC();

		updateCalendarList.newJCalendarList(answerList, calendarList);
      updateCheckpointList.newJCheckpointList(answerList, checkpointList);
		updateComplexList.newJComplexList(answerList, complexList);
		updateQueueList.newJQueueList(answerList, queueList);
		updateHostList.newJHostList(answerList, hostList);

		//register as an EventClient
		boa = orb.BOA_init(args, prop);
		PushConsumer ev = new JEventClient_impl(  updateCalendarList,
                                                updateCheckpointList,
                                                updateComplexList, 
                                                updateHostList, 
                                                updateQueueList, 
                                                master, 
                                                ctx, 
                                                orb, 
                                                debug);
      
		boa.impl_is_ready(null);

		debug.DEXIT();
	}


	private void init(JComplex complex, Complex c) {
      content[] cnt = null;
      try {
         cnt = c.get_content(ctx);    
      }
      catch(Codine.Error x){
         System.out.println("Codine Error");
      }
      catch(Codine.Authentication x){
         System.out.println("Codine Authentication Error");
      }
      catch(Codine.ObjDestroyed x){
         System.out.println("Codine Object Destroyed");
      }
      
      contentSeqHolder cs = new contentSeqHolder(cnt);
		for(int i = 0; i < cs.value.length; i++) {
			switch(cs.value[i].elem) {
				case CX_name.value:	
               complex.setName(cs.value[i].value.extract_string());
					break;

				case CX_entries.value: 
					ComplexEntry [] ceArray = ComplexEntrySeqHelper.extract(cs.value[i].value);
					JComplexEntryList cel = new JComplexEntryList();
					for(int j = 0; j < ceArray.length; j++) {
                  JComplexEntry ce = new JComplexEntry(debug);
                  ce.setName(ceArray[j].name);
						ce.setShortcut(ceArray[j].shortcut);
						ce.setStringval(ceArray[j].stringval);
						ce.setValtype(ceArray[j].valtype);
						ce.setRelop(ceArray[j].relop);
						ce.setRequest(ceArray[j].request);
						ce.setConsumable(ceArray[j].consumable);
						ce.setForced(ceArray[j].forced);
						cel.addElement(ce);
					}
               complex.setComplexEntryList(cel);
					break;
			}
		}

	}

	private native void contactC();
	private native void getQueue();
}
      
