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

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import jqmon.util.*;
import jqmon.debug.*;
import jqmon.views.*;
import jqmon.events.*;
import jcodine.*;


// this class is responsible for the display of the Codine Window.
// it displays the TreeView to the right and the selected view to the left.
// 
// @author		Michael Roehrl
// modified by Hartmut Gilde
// 
// @version	1.0

public class JCodineWindow extends     JFrame 
									implements  JCalendarListListener,
                                       JCheckpointListListener,		
                                       JComplexListListener, 
                                       JHostListListener, 
                                       JQueueListListener 
{

	// connection to the workerthread 
	protected JWorkerThread wt       = null;
	protected JDebug        debug    = null;

	protected JSplitPane  splitPane  = null;
	protected JTreeView   treeView   = null;
	protected JCodineView codineView = null;
   
   protected JCalendarView    calendarView    = null;
   protected JCheckpointView  checkpointView  = null;
	protected JHostView        hostView        = null;
	protected JQueueView       queueView       = null;
	
   protected JCalendarsView   calendarsView  = null;
   protected JCheckpointsView checkpointsView= null;
	protected JComplexesView   complexesView  = null;
   protected JHostsView       hostsView      = null;
   protected JQueuesView      queuesView     = null;

   protected JUpdateCalendarList    updateCalendarList    = null;
   protected JUpdateCheckpointList  updateCheckpointList  = null;
	protected JUpdateComplexList     updateComplexList     = null;
	protected JUpdateQueueList       updateQueueList       = null;
	protected JUpdateHostList        updateHostList        = null;


	// create a JCodineWindow 
	public JCodineWindow(JWorkerThread w, JDebug d,
                        JUpdateCalendarList   cal, 
                        JUpdateCheckpointList ck,
                        JUpdateComplexList    cx, 
                        JUpdateHostList       eh, 
			               JUpdateQueueList      qu, 
								String title) {
		//super(title, true, true, true, true);
		super(title);
		wt    = w;
		debug = d;
      updateCalendarList   = cal;
      updateCheckpointList = ck;
		updateComplexList    = cx;
		updateHostList  	   = eh;
		updateQueueList 	   = qu;
      
		initGUI();
      updateCalendarList.addJCalendarListListener     (this);
      updateCheckpointList.addJCheckpointListListener (this);
		updateComplexList.addJComplexListListener       (this);
		updateHostList.addJHostListListener             (this);
		updateQueueList.addJQueueListListener           (this);

	}


	// creates a JCodineWindow 
	public JCodineWindow(JWorkerThread w, JDebug d, 
                        JUpdateCalendarList   cal, 
                        JUpdateCheckpointList ck,
                        JUpdateComplexList    cx, 
								JUpdateHostList       eh, 
                        JUpdateQueueList      qu) {
		this(w, d, cal, ck, cx, eh, qu, "Codine Window");
	}

   // ----------------------------------------------------------------------
	// initialize the GUI 
	protected void initGUI() {
		debug.DENTER("JCodineWindow.initGUI");
      
		JPanel p             = new JPanel();
		splitPane 				= new JSplitPane  (JSplitPane.HORIZONTAL_SPLIT);
		treeView					= new JTreeView   (debug,this);
		codineView           = new JCodineView (debug);
      
      calendarView         = new JCalendarView     (debug);
      checkpointView       = new JCheckpointView   (debug);
		hostView					= new JHostView         (debug);
		queueView				= new JQueueView        (debug);
      
      calendarsView        = new JCalendarsView    (debug);
      checkpointsView      = new JCheckpointsView  (debug);
		complexesView			= new JComplexesView    (debug);	
      hostsView            = new JHostsView        (debug);
		queuesView           = new JQueuesView       (debug);
		
		splitPane.setLeftComponent(treeView);
		setView(complexesView);
		splitPane.setContinuousLayout(true);
		p.add(splitPane, BorderLayout.CENTER);	
		//!!!!!!!!!!!!!!!!!!!!!!!11
		getContentPane().add(p, BorderLayout.CENTER);
		debug.DEXIT();
	}


   // ----------------------------------------------------------------------
   //    Calendar
   // ----------------------------------------------------------------------
	public void newJCalendarListAvailable(JCalendarListEvent e) {
		debug.DENTER("JCodineWindow.newJCalendarListAvailable");
      
		JAnswerList al = (JAnswerList)updateCalendarList.getAnswerList().clone();
		debug.DPRINTF("JAnswerList: " + al);
		try {
			JCalendarList l = (JCalendarList)updateCalendarList.getCalendarList().clone();
			treeView.newCalendarList(l);
		} catch (JAnswerListException ex) {
			debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
		}
		debug.DEXIT();
	}
   
   // ----------------------------------------------------------------------
   public void updateCalendarAvailable(JCalendarListEvent e) {
      debug.DPRINTF("JCodineWindow.updateCalendarAvailable");
      
      int index = e.getIndexUpdateCalendar();
      treeView.updateCalendar(index);
   }

   // ----------------------------------------------------------------------
   public void deleteCalendarAvailable(JCalendarListEvent e) { 
		debug.DENTER("JCodineWindow.deleteCalendarAvailable()");
      
      int index = e.getIndexUpdateCalendar();
		debug.DPRINTF((new Integer(index)).toString());
      treeView.deleteCalendar(index);
		debug.DEXIT();
   }
  
   // ----------------------------------------------------------------------
   public JPanel getCalendarView() {
		debug.DENTER("JCodineWindow.getCalendarView");
		Class cl    = getView().getClass();
		Class calv  = calendarView.getClass();

		if ( !cl.equals(calv) ) {
			//setView(calendarView);
			//debug.DPRINTF("changed view to CALENDARVIEW");
		}

		debug.DEXIT();
		return calendarView;
	}
   
   // ----------------------------------------------------------------------
   public JPanel getCalendarsView() {
      debug.DENTER("JCodineWindow.getCalendarsView");
      Class cl = getView().getClass();
      Class calsv = calendarsView.getClass();
 
      if ( !cl.equals(calsv) ) {
         //setView(queuesView);
         //debug.DPRINTF("changed view to CALENDARSVIEW");
      }
      debug.DEXIT();
      return calendarsView;
   }
  

   // ----------------------------------------------------------------------
   //    Checkpoint
   // ----------------------------------------------------------------------
	public void newJCheckpointListAvailable(JCheckpointListEvent e) {
		debug.DENTER("JCodineWindow.newJCheckpointListAvailable");
      
		JAnswerList al = (JAnswerList)updateCheckpointList.getAnswerList().clone();
		debug.DPRINTF("JAnswerList: " + al);
		try {
			JCheckpointList ckl = (JCheckpointList)updateCheckpointList.getCheckpointList().clone();
			treeView.newCheckpointList(ckl);
		} 
      catch (JAnswerListException ex) {
			debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
		}
		debug.DEXIT();
	}
   
   // ----------------------------------------------------------------------
   public void updateCheckpointAvailable(JCheckpointListEvent e) {
      debug.DPRINTF("JCodineWindow.updateCheckpointAvailable");
      
      int index = e.getIndexUpdateCheckpoint();
      treeView.updateCheckpoint(index);
   }

   // ----------------------------------------------------------------------
   public void deleteCheckpointAvailable(JCheckpointListEvent e) { 
		debug.DENTER("JCodineWindow.deleteCheckpointAvailable()");
      
      int index = e.getIndexUpdateCheckpoint();
		debug.DPRINTF((new Integer(index)).toString());
      treeView.deleteCheckpoint(index);
		debug.DEXIT();
   }

   // ----------------------------------------------------------------------
   public JPanel getCheckpointView() {
		debug.DENTER("JCodineWindow.getCheckpointView");
      
		Class cl  = getView().getClass();
		Class ckv = checkpointView.getClass();

		if ( !cl.equals(ckv) ) {
			//setView(checkpointView);
			//debug.DPRINTF("changed view to CHECKPOINTVIEW");
		}

		debug.DEXIT();
		return checkpointView;
	}
   
   // ----------------------------------------------------------------------
   public JPanel getCheckpointsView() {
      debug.DENTER("JCodineWindow.getCheckpointsView");
      
      Class cl  = getView().getClass();
      Class ckv = checkpointsView.getClass();
 
      if (!cl.equals(ckv) ) {
			//setView(checkpointsView);
         //debug.DPRINTF("changed view to CHECKPOINTSVIEW");
      }
      debug.DEXIT();
      return checkpointsView;
   }
  
   
   // ----------------------------------------------------------------------
	//    Complex
   // ----------------------------------------------------------------------
	public void newJComplexListAvailable(JComplexListEvent e) {
      debug.DENTER("JCodineWindow.newJComplexListAvailable");
      
      JAnswerList al = (JAnswerList)updateComplexList.getAnswerList().clone();
      debug.DPRINTF("JAnswerList: " + al);
      try {
         JComplexList l = (JComplexList)updateComplexList.getComplexList().clone();
         treeView.newComplexList(l);
      } 
      catch (JAnswerListException ex) {
         debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
      }
      debug.DEXIT();
   }
	
   // ----------------------------------------------------------------------
   public void updateComplexAvailable(JComplexListEvent e) {
      debug.DPRINTF("JCodineWindow.updateComplexAvailable");
      
/*    try {
         JQueueList l = (JQueueList)updateQueueList.getQueueList().clone();
         treeView.newQueueList(l);
      } 
      catch (JAnswerListException ex) {
         debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
      }
*/
      int index = e.getIndexUpdateComplex();
      treeView.updateComplex(index);
   }

   // ----------------------------------------------------------------------
   public void deleteComplexAvailable(JComplexListEvent e) { 
		debug.DENTER("JCodineWindow.deleteComplexAvailable()");
      
      int index = e.getIndexUpdateComplex();
		debug.DPRINTF((new Integer(index)).toString());
      treeView.deleteComplex(index);
		debug.DEXIT();
   }
   
   // ----------------------------------------------------------------------
	public JPanel getComplexesView() {
      debug.DENTER("JCodineWindow.getComplexesView");
      
      Class cl = getView().getClass();
      Class cxv = complexesView.getClass();
 
      if ( !cl.equals(cxv) ) {
         //setView(complexesView);
         //debug.DPRINTF("changed view to COMPLEXESVIEW");
      }
      debug.DEXIT();
      return complexesView;
   }
  
   
   // ----------------------------------------------------------------------
	//    Host 
   // ----------------------------------------------------------------------
	public void newJHostListAvailable(JHostListEvent e) {
		debug.DENTER("JCodineWindow.newJHostListAvailable");
      
		JAnswerList al = (JAnswerList)updateHostList.getAnswerList().clone();
		debug.DPRINTF("JAnswerList: " + al);
		try {
			JHostList l = (JHostList)updateHostList.getHostList().clone();
			treeView.newHostList(l);
		} 
      catch (JAnswerListException ex) {
			debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
		}
		debug.DEXIT();
	}
   
   // ----------------------------------------------------------------------
   public void updateHostAvailable(JHostListEvent e) {
      debug.DPRINTF("JCodineWindow.updateHostAvailable");
      
/*    try {
         JQueueList l = (JQueueList)updateQueueList.getQueueList().clone();
         treeView.newQueueList(l);
      } 
      catch (JAnswerListException ex) {
         debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
      }
*/
      int index = e.getIndexUpdateHost();
      treeView.updateHost(index);
   }
   
   // ----------------------------------------------------------------------
   public void deleteHostAvailable(JHostListEvent e) { 
		debug.DENTER("JCodineWindow.deleteHostAvailable()");
      
      int index = e.getIndexUpdateHost();
		debug.DPRINTF((new Integer(index)).toString());
      treeView.deleteHost(index);
		debug.DEXIT();
   }

   // ----------------------------------------------------------------------
	public JPanel getHostView() {
		debug.DENTER("JCodineWindow.getHostView");
		Class cl = getView().getClass();
		Class ehv = hostView.getClass();

		if ( !cl.equals(ehv) ) {
			//setView(hostView);
			//debug.DPRINTF("changed view to HOSTVIEW");
		}

		debug.DEXIT();
		return hostView;
	}
   
   // ----------------------------------------------------------------------
   public JPanel getHostsView() {
      debug.DENTER("JCodineWindow.getHostsView");
      Class cl = getView().getClass();
      Class ehv = hostsView.getClass();
      
      if ( !cl.equals(ehv) ) {
         //setView(hostsView);
         //debug.DPRINTF("changed view to HOSTSVIEW");
      }
      debug.DEXIT();
      return hostsView;
   }
  
   
   // ----------------------------------------------------------------------
	//    Queue 
   // ----------------------------------------------------------------------
	public void newJQueueListAvailable(JQueueListEvent e) {
		debug.DENTER("JCodineWindow.newJQueueListAvailable");
      
		/* According to the Java-Docu we now have a copy of the */
		/* JQueueList which stores itself copies of the original queues */
		/* Hope this is true!!!! */
		JAnswerList al = (JAnswerList)updateQueueList.getAnswerList().clone();
		debug.DPRINTF("JAnswerList: " + al);
		try {
			JQueueList l = (JQueueList)updateQueueList.getQueueList().clone();
			//JQueueList l = (JQueueList)updateQueueList.getQueueList();
			treeView.newQueueList(l);
		} 
      catch (JAnswerListException ex) {
			debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
		}
		debug.DEXIT();
	}
	
   // ----------------------------------------------------------------------
	public void updateQueueAvailable(JQueueListEvent e) {
/*		debug.DPRINTF("JCodineWindow.updateQueueAvailable");

		try {
         JQueueList l = (JQueueList)updateQueueList.getQueueList().clone();
         treeView.newQueueList(l);
      } 
      catch (JAnswerListException ex) {
         debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
      }
*/
		int index = e.getIndexUpdateQueue();
		treeView.updateQueue(index);
	}

   // ----------------------------------------------------------------------
   public void deleteQueueAvailable(JQueueListEvent e) { 
		debug.DENTER("JCodineWindow.deleteQueueAvailable()");
      
      int index = e.getIndexUpdateQueue();
		debug.DPRINTF((new Integer(index)).toString());
      treeView.deleteQueue(index);
		debug.DEXIT();
   } 

   // ----------------------------------------------------------------------
	public JPanel getQueueView() {
		debug.DENTER("JCodineWindow.getQueueView");
      
		Class cl = getView().getClass();
		Class quv = queueView.getClass();

		if ( !cl.equals(quv) ) {
			//setView(queueView);
			//debug.DPRINTF("changed view to QUEUEVIEW");
		}
		debug.DEXIT();
		return queueView;
	}

   // ----------------------------------------------------------------------
	public JPanel getQueuesView() {
      debug.DENTER("JCodineWindow.getQueuesView");
      
      Class cl = getView().getClass();
      Class quv = queuesView.getClass();
 
      if ( !cl.equals(quv) ) {
         //setView(queuesView);
         //debug.DPRINTF("changed view to QUEUESVIEW");
      }
      debug.DEXIT();
      return queuesView;
   }   
   
   
   // ----------------------------------------------------------------------
	public void setView(JPanel view) {
		debug.DENTER("JCodineWindow.setView");
		splitPane.setRightComponent(view);
		debug.DEXIT();
	}

   // ----------------------------------------------------------------------
	public JPanel getView() {
		return (JPanel)splitPane.getRightComponent();
	}
   
   // ----------------------------------------------------------------------
	// TreeView (only to be recieved!!!) 
	public JTreeView getTreeView() {
		debug.DENTER("JCodineWindow.getTreeView");
		debug.DEXIT();
		return treeView;
	}
}
