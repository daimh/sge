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
 
import org.omg.CORBA.*;
import CosEventComm.*;
import CosEventChannelAdmin.*;
import Codine.*;

import java.util.*;

import jqmon.debug.*;
import jcodine.*;



public class JEventClient_impl extends _PushConsumerImplBase {

   // allowed values for target field of cod_api_request
 
   final static int COD_ADMINHOST_LIST  = 1;
   final static int COD_SUBMITHOST_LIST = 2;
   final static int COD_EXECHOST_LIST   = 3;
   final static int COD_QUEUE_LIST      = 4;
   final static int COD_JOB_LIST        = 5;
   final static int COD_EVENT_LIST      = 6;
   final static int COD_COMPLEX_LIST    = 7;
   final static int COD_ORDER_LIST      = 8;
   final static int COD_MASTER_EVENT    = 9;
   final static int COD_CONFIG_LIST     = 10;
   final static int COD_MANAGER_LIST    = 11;
   final static int COD_OPERATOR_LIST   = 12;
   final static int COD_PE_LIST         = 13;
   final static int COD_SC_LIST         = 14;      // schedconf list
   final static int COD_USER_LIST       = 15;
   final static int COD_USERSET_LIST    = 16;
   final static int COD_PROJECT_LIST    = 17;
   final static int COD_SHARETREE_LIST  = 18;
   final static int COD_CALENDAR_LIST   = 20;
   final static int COD_CKPT_LIST       = 21;

   final static int BASIC_UNIT = 50;               // Don't touch 

   // -----------------------------------------------------------
 
	protected ORB     orb      = null;
	protected JDebug  debug    = null;
	protected Context context  = null;
	
   protected JUpdateCalendarList    updateCalendarList   = null;
   protected JUpdateCheckpointList  updateCheckpointList = null;
	protected JUpdateComplexList     updateComplexList    = null;
	protected JUpdateHostList        updateHostList       = null;
	protected JUpdateQueueList       updateQueueList      = null;

   // ----------------------------------------------------------
   
	public JEventClient_impl(  JUpdateCalendarList   cal,
                              JUpdateCheckpointList ck,
										JUpdateComplexList    cx, 
                              JUpdateHostList       eh, 
										JUpdateQueueList      qu,
										Master   master, 	
										Context  ctx, 
										ORB      o, 
										JDebug   d){
		orb = o;
		debug = d;
		context = ctx;
      
      updateCalendarList   = cal;
      updateCheckpointList = ck;
		updateComplexList    = cx;
		updateHostList       = eh;
		updateQueueList      = qu;
      
		try {
         ConsumerAdmin ca = master.getConsumerAdmin(ctx);
         ProxyPushSupplier pps = ca.obtain_push_supplier();
			pps.connect_push_consumer(this);
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
		catch(UserException e){
			// ..., AlreadyConnected and TypeError can be the reason for the exception
		}
	}
		
	public void push(Any any) {
		debug.DENTER("JEventClient_impl.push");
      
		LongHolder  u;
      int         index;
		event       ev = eventHelper.extract(any);
		String      s  = ev.name;
      
      // ---------------------------
      // modification event received
		if(ev.type.value() == event_type.ev_mod.value()) {
         debug.DPRINTF("modification event received");
         
         switch(ev.obj) {
				// a calendar has been modified
            case COD_CALENDAR_LIST:
               debug.DPRINTF("calendar " + s + " has been modified");
					index = findCalendar(s); 
					if(index >= 0) {
                  // update the calendar in the updateCalendarList
						for(int i = 0; i < ev.changes.length; i++){
							updateFieldCalendar(index, ev.changes[i].elem, ev.changes[i].value);
						}
						// send event for updating the calendars of the views
						updateCalendarList.updateCalendar(index);
					}
					else {
						// no such calendar
                  debug.DPRINTF("calendar " + s + " not found");
					} 
               break;
                  
				// a checkpoint has been modified
				case COD_CKPT_LIST:
               break; 
               
				// a complex has been modified
				case COD_COMPLEX_LIST:
               break; 
               
            // a host has been modified   
            case COD_EXECHOST_LIST:
					debug.DPRINTF("host " + s + " has been modified");
					index = findHost(s); 
					if(index >= 0) {
                  // update the host in the updateHostList
                  for(int i = 0; i < ev.changes.length; i++){
                     updateFieldHost(index, ev.changes[i].elem, ev.changes[i].value);
						}
						// send an event for updating the hosts of the views
						updateHostList.updateHost(index);
					}
					else {
						// no such host
                  debug.DPRINTF("host " + s + " not found");
					} 
               break;
               
				// a queue has been modified
            case COD_QUEUE_LIST:
               debug.DPRINTF("queue " + s + " has been modified");
               index = findQueue(s); 
               if(index >= 0) {
                  // update the queue in the updateQueueList
                  for(int i = 0; i < ev.changes.length; i++){
                     updateFieldQueue(index, ev.changes[i].elem, ev.changes[i].value);
                  }
                  // send an event to update the queues in the views
                  updateQueueList.updateQueue(index);
               }
               else {
                  // no such queue
                  debug.DPRINTF("queue " + s + " not found");
               } 
               break;
			}
      }
      // ---------------------
      // add event received
      else if (ev.type.value() == event_type.ev_add.value()) {
         debug.DPRINTF("add event received");

         switch(ev.obj) {
            // a calendar has been added
            case COD_CALENDAR_LIST:
               debug.DPRINTF("calendar "   + s + " has been added");
               break;
               
            // a checkpoint has been added
            case COD_CKPT_LIST:
               debug.DPRINTF("checkpoint " + s + " has been added");
               break;
               
            // a complex has been added 
            case COD_COMPLEX_LIST:
               debug.DPRINTF("complex "    + s + " has been added");
               break;
               
            // a host has been added 
            case COD_EXECHOST_LIST:
               debug.DPRINTF("host "       + s + " has been added");
               break;
               
            // a queue has been added 
            case COD_QUEUE_LIST:
               debug.DPRINTF("queue "      + s + " has been added");
               break;
         }
      }
      // ---------------------
      // delete event received
      else if (ev.type.value() == event_type.ev_del.value()) {
         debug.DPRINTF("delete event received");
         //debug.DPRINTF("event is of type " + ev.obj);
         
         switch(ev.obj) {
            // a calendar has been deleted   
            case COD_CALENDAR_LIST:
            debug.DPRINTF("calendar " + s + " has been deleted");
               index = findCalendar(s); // s = the name of the calendar which is unique
               if(index >= 0) {
                  // delete the calendar
/*				   	try {		
                     updateCalendarList.getCalendarList().removeElementAt(index);
                     // send an event to delete the queue
                     updateCalendarList.deleteCalendar(index); 
                  }
                  catch(JAnswerListException e) {
                  }
*/					
                  debug.DPRINTF("now trying to delete calendar" + s);
                  updateCalendarList.deleteCalendar(index);
               }
               else {
                  // no such calendar
                  debug.DPRINTF("calendar " + s + " not found");
               }  
               break;
               
            // a checkpoint has been deleted   
            case COD_CKPT_LIST:
               index = findCheckpoint(s);
               if (index >= 0) {
                  updateCheckpointList.deleteCheckpoint(index);
               }
               else {
                  // no such checkpoint
                  debug.DPRINTF("checkpoint " + s + " not found");
               }
               
               break;
               
            // a complex has been deleted   
            case COD_COMPLEX_LIST:
               break;
               
            // a host has been deleted   
            case COD_EXECHOST_LIST:
               break;
               
            // a queue has been deleted
            case COD_QUEUE_LIST:
               index = findQueue(s); // s = the name of the queue which is unique
               if(index >= 0) {
                  // delete the queue
/*					   try {		
                     updateQueueList.getQueueList().removeElementAt(index);
                     // send an event for delete the queue
                     updateQueueList.deleteQueue(index); 
                  }
                  catch(JAnswerListException e) {
                  }
*/					
               updateQueueList.deleteQueue(index);
               }
               else {
                  // no such queue
                  debug.DPRINTF("queue " + s + " not found");
               }  
               break;
               
         
         }
      }
      debug.DEXIT();
   }


   public void disconnect_push_consumer() {

   }


   
	private int findCalendar(String calendarName) {
		// the name of the calendar is unique
		try {
			JCalendarList calendarList = updateCalendarList.getCalendarList();
			Enumeration e = calendarList.elements();
			int count = 0;
         while ( e.hasMoreElements() ) {
            JCalendar calendar = (JCalendar)e.nextElement();
            if(calendarName.equals(calendar.getName())) {
               // the calendar is found
               return count;
            }			
            count++;
         }
		}
		catch(JAnswerListException e) {
			// isn't good
		}	
		// the calendar isn't found
		return -1;	
	}
	
   
	private void updateFieldCalendar(int index, int elem, Any value) {
		// update the calendar by index
		try {
			JCalendar calendar = (JCalendar) updateCalendarList.getCalendarList().elementAt(index);
			switch(elem) {
				// name
				case CAL_name.value:		   
               calendar.setName(value.extract_string());	
					break;
				// year_calendar
            case CAL_year_calendar.value: 
               calendar.setYearCalendar(value.extract_string());
					break;
				// week_calendar
				case CAL_week_calendar.value:	
               calendar.setWeekCalendar(value.extract_string());
					break;

			}
			updateCalendarList.getCalendarList().setElementAt(calendar, index);
		}
		catch(JAnswerListException e) {			
			// isn't good
		}
	}


	private int findCheckpoint(String checkpointName) {
		try {
			JCheckpointList checkpointList = updateCheckpointList.getCheckpointList();
			Enumeration e = checkpointList.elements();
			int count = 0;
         while ( e.hasMoreElements() ) {
            JCheckpoint checkpoint = (JCheckpoint)e.nextElement();
            if(checkpointName.equals(checkpoint.getName())) {
               // the calendar is found
               return count;
            }			
            count++;
         }
		}
		catch(JAnswerListException e) {
			// isn't good
		}	
		// the checkpoint wasn't found
		return -1;	
	}
	
   
	private void updateFieldCheckpoint(int index, int elem, Any value) {
		// update the checkpoint by index
		try {
			JCheckpoint checkpoint = (JCheckpoint) updateCheckpointList.getCheckpointList().elementAt(index);
			switch(elem) {
				// name
				case CK_name.value:		   
               checkpoint.setName(value.extract_string());	
					break;
				// interface
            case CK_interface.value: 
               checkpoint.setInterface(value.extract_string());
					break;
				// ckpt_command
				case CK_ckpt_command.value:	
               checkpoint.setCkptCommand(value.extract_string());
					break;
				// migr_command
				case CK_migr_command.value:	
               checkpoint.setMigrCommand(value.extract_string());
					break;
				// rest_command
				case CK_rest_command.value:	
               checkpoint.setRestCommand(value.extract_string());
					break;
				// ckpt_dir
				case CK_ckpt_dir.value:	
               checkpoint.setCkptDir(value.extract_string());
					break;

			}
			updateCalendarList.getCalendarList().setElementAt(checkpoint, index);
		}
		catch(JAnswerListException e) {			
			// isn't good
		}
	}
   private int findHost(String hostName) {
		try {
			JHostList hostList = updateHostList.getHostList();
			Enumeration e = hostList.elements();
			int count = 0;
         while ( e.hasMoreElements() ) {
            JHost host = (JHost)e.nextElement();
            if(hostName.equals(host.getName())) {
               return count;
            }			
            count++;
         }
		}
		catch(JAnswerListException e) {
			// isn't good
		}	
		return -1;	
	}
   
	private void updateFieldHost(int index, int elem, Any value) {
		try {
			JHost host = (JHost) updateHostList.getHostList().elementAt(index);
			switch(elem) {
				// name
				case EH_name.value:		
               host.setName(value.extract_string());	
					break;
				case EH_lt_heard_from.value:	
               host.setLtHeardFrom(value.extract_ulong());
					break; 
				// you must continue ...
			}
			updateHostList.getHostList().setElementAt(host, index);
		}
		catch(JAnswerListException e) {			
			// isn't good
		}
	}
   
   
   private int findQueue(String queueName) {
		// the name of the queue is unique
		try {
			JQueueList queueList = updateQueueList.getQueueList();
			Enumeration e = queueList.elements();
			int count = 0;
         while ( e.hasMoreElements() ) {
            JQueue queue = (JQueue)e.nextElement();
            if(queueName.equals(queue.getQName())) {
					// the queue is found
					return count;
				}			
				count++;
        	}
		}
		catch(JAnswerListException e) {
			// isn't good
		}	
		// the queue isn't found
		return -1;	
	}
	
	private void updateFieldQueue(int index, int elem, Any value) {
      // update the queue by index
		try {
			JQueue queue = (JQueue) updateQueueList.getQueueList().elementAt(index);
			switch(elem) {
				// name
				case QU_qname.value:		   
               queue.setQName(value.extract_string());	
               break;
				// hostname
				case QU_qhostname.value:	
               queue.setQHostName(value.extract_string());
					break;
				//
				case QU_job_slots.value:	
               queue.setJobSlots(value.extract_ulong());
					break;

				case QU_complex_list.value:	
               JComplexList complexList = queue.getComplexList();
					complexList.removeAllElements();
					Complex[] complexArray = ComplexSeqHelper.extract(value);
					// old version
					// complexList.init(debug, complexArray, context);
					JComplexList generalComplexList = updateComplexList.getComplexList();
					for(int i = 0; i < complexArray.length; i++) {
                  JComplex complex = generalComplexList.getComplex(complexArray[i]);
                  if(complex == null) {
                     // very very bad thing
                  	debug.DPRINTF("very very bad thing");
                  }
                  else {
                     // you take the reference from the complexList of updateComplexList
                     complexList.addElement(complex);
                  } 
				}
            
				queue.setComplexList(complexList);
				break; 
				// you must continue ...
			}
			updateQueueList.getQueueList().setElementAt(queue, index);
		}
		catch(JAnswerListException e) {			
			// isn't good
		}
	}
   
}
