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

package jqmon.views;

import java.awt.*;
import java.awt.event.*;
import java.util.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;
import javax.swing.tree.*;

import jqmon.*;
import jqmon.util.*;
import jqmon.debug.*;
import jcodine.*;


// this class represents the tree view in the codine window

public class JTreeView extends JPanel {

	protected JTree     tree			= null;
	protected Dimension dimension		= null;
   protected JDebug debug           = null;
	
	// the absolute root 
	protected DefaultMutableTreeNode root = null;
	
	// the parent nodes for the codine objects 
	protected DefaultMutableTreeNode queuesRoot        = null;
	protected DefaultMutableTreeNode jobsRoot          = null;
	protected DefaultMutableTreeNode hostsRoot         = null;
   protected DefaultMutableTreeNode complexesRoot     = null;
	protected DefaultMutableTreeNode calendarsRoot     = null;
   protected DefaultMutableTreeNode checkpointsRoot   = null;
   
	protected JCodineWindow parent         = null;

	protected JQueueSet queues             = null;
	protected JHostSet  hosts              = null;
   protected JCalendarSet calendars       = null;
   protected JCheckpointSet checkpoints   = null;


	public JTreeView(JDebug d, JCodineWindow w) {
		this(d, w, new Dimension(150,200));
	}

	
	public JTreeView(JDebug de, JCodineWindow w, Dimension d) {
		super();
		dimension   = d;
		parent      = w;
		debug       = de;
		queues      = new JQueueSet();
		hosts       = new JHostSet();
      calendars   = new JCalendarSet();
      checkpoints = new JCheckpointSet();
		initGUI();
	}
	
	
	public void initGUI() {
		debug.DENTER("JTreeView.initGUI");
      
		root           = new DefaultMutableTreeNode("Codine");
		queuesRoot     = new DefaultMutableTreeNode("Queues");
		jobsRoot       = new DefaultMutableTreeNode("Jobs");
		hostsRoot      = new DefaultMutableTreeNode("Hosts");
		complexesRoot  = new DefaultMutableTreeNode("Complexes");		
      calendarsRoot  = new DefaultMutableTreeNode("Calendars");	
      checkpointsRoot= new DefaultMutableTreeNode("Checkpoints");	
      
      
		JGridBagLayout layout = new JGridBagLayout();
		JScrollPane		pane	 = new JScrollPane();
		
		root.add(queuesRoot);
		root.add(jobsRoot);
		root.add(hostsRoot);
		root.add(complexesRoot);		
      root.add(calendarsRoot);
      root.add(checkpointsRoot);

		tree = new JTree(root);
		TreePath path = new TreePath(root.getUserObjectPath());
      tree.putClientProperty("JTree.lineStyle", "Angled");

		//tree.setExpandedState(path,false);
      //DefaultTreeCellRenderer renderer = new DefaultTreeCellRenderer();
      //renderer.setLeafIcon(new ImageIcon("/cod_home/hartmut/C4/jqmon_corba/images/NextArrow2.gif"));
      //tree.setCellRenderer(renderer);
      
      tree.setCellRenderer(new CodineTreeRenderer());
      System.out.println(tree.getSelectionModel().getSelectionMode());
		
		setLayout(layout);
		pane.getViewport().add(tree);
		pane.setPreferredSize(dimension);
		pane.setMinimumSize(dimension);
		pane.setBorder(new BevelBorder(BevelBorder.LOWERED));

		layout.constrain(this, pane, 1,1,1,1, GridBagConstraints.BOTH,
				           GridBagConstraints.NORTHWEST, 1.0, 1.0, 1,1,5,5);

		tree.addTreeSelectionListener(new TreeSelectionListener() {
			public void valueChanged(TreeSelectionEvent e) {
				debug.DENTER("JTreeView.tree.TreeSelectionListener.valueCanged");
            debug.DPRINTF("Pfad: " + (e.getPath()).toString());
            				
				JQueue queue            = new JQueue      (debug);
				JHost  host             = new JHost       (debug);
            JCalendar calendar      = new JCalendar   (debug);
            JCheckpoint checkpoint  = new JCheckpoint (debug);
				
				TreePath pfad = e.getPath();
				DefaultMutableTreeNode sel = (DefaultMutableTreeNode)pfad.getLastPathComponent();
            
				Object object  = sel.getUserObject();
				Class subcl    = object.getClass();
				Class cl       = sel.getClass();
				String str     = object.toString();
            
				debug.DPRINTF("Class: " + cl.toString());
            
				if(str.equals("Queues")) {
					JQueuesView quv = (JQueuesView)parent.getQueuesView();
					parent.setView(quv);	
				}	
				else if(str.equals("Complexes")) {
					JComplexesView cxv = (JComplexesView)parent.getComplexesView();
					parent.setView(cxv); 
				}
            else if(str.equals("Hosts")) {
               JHostsView ehv = (JHostsView)parent.getHostsView();
               parent.setView(ehv);
            }
            else if(str.equals("Calendars")) {
               JCalendarsView calv = (JCalendarsView)parent.getCalendarsView();
               parent.setView(calv);
            }
            else if(str.equals("Checkpoints")) {
               JCheckpointsView ckv = (JCheckpointsView)parent.getCheckpointsView();
               parent.setView(ckv);
            }
				// a queue has been selected 
				if ( subcl.equals(queue.getClass()) ) {
					queue = (JQueue)object;
					queues.clear();
					queues.add(queue);
					JQueueView quv = (JQueueView)parent.getQueueView();
               quv.setJQueue(queues.getFirst());
               parent.setView(quv);
				}
				// a host has been selected 
            else if (subcl.equals(host.getClass()) ) {
               host = (JHost)object;
					hosts.clear();
					hosts.add(host);
					JHostView ehv = (JHostView)parent.getHostView();
					ehv.setJHost(hosts.getFirst());
               parent.setView(ehv);
				}
            // a calendar has been selected
            else if (subcl.equals(calendar.getClass()) ) {
               calendar = (JCalendar)object;
               calendars.clear();
               calendars.add(calendar);
               JCalendarView calv = (JCalendarView)parent.getCalendarView();
               calv.setJCalendar(calendars.getFirst());
               parent.setView(calv);
            }
            // a checkpoint has been selected
            else if (subcl.equals(checkpoint.getClass()) ) {
               checkpoint = (JCheckpoint)object;
               checkpoints.clear();
               checkpoints.add(checkpoint);
               JCheckpointView ckv = (JCheckpointView)parent.getCheckpointView();
               ckv.setJCheckpoint(checkpoints.getFirst());
               parent.setView(ckv);
            }
            
				debug.DPRINTF("Klasse: " + sel.getClass());
				//if(queues!=null) 
				//	 debug.DPRINTF("QUEUES: " + queues.getFirst().toString());
				debug.DEXIT();
			}
		}); 

		tree.addTreeExpansionListener(new TreeExpansionListener() {
			public void treeCollapsed(TreeExpansionEvent e) {
				debug.DENTER("JTreeView.tree.TreeExpansionListener.treeCollapsed");
				debug.DPRINTF("Pfad: " + (e.getPath()).toString());
				debug.DEXIT();
			}
			
			public void treeExpanded(TreeExpansionEvent e) {
				debug.DENTER("JTreeView.tree.TreeExpansionListener.treeExpanded");
				debug.DPRINTF("Path: " + e.getPath());
				debug.DEXIT();
			}
		});

		setVisible(true);
		debug.DEXIT();
	}

   // update the Calendar by index
   public void updateCalendar(int index) {
      JCalendarsView calendarsView = (JCalendarsView)parent.getCalendarsView();
      calendarsView.updateCalendar(index);
   }

   // delete the Calendar by index
	public void deleteCalendar(int index) {
      JCalendarsView calendarsView = (JCalendarsView)parent.getCalendarsView();
      calendarsView.deleteCalendar(index);

   }
   
   // is called, when a new calendar list is available 
	public void newCalendarList(JCalendarList l) {
      debug.DENTER("JTreeView.newCalendarList");

		Enumeration e = l.elements();
      
		while (e.hasMoreElements()) {
			JCalendar calendar = (JCalendar)e.nextElement();
			DefaultMutableTreeNode node = new DefaultMutableTreeNode(calendar);
			calendarsRoot.add(node);
			JCalendarView calv = (JCalendarView)parent.getCalendarView();
			calv.setJCalendar(calendar);
		}
      
      JCalendarsView calendarsView = (JCalendarsView)parent.getCalendarsView();
      calendarsView.setCalendarList(l);
		expandTree();
		debug.DEXIT();
	}
  

   // update the Checkpoint by index
   public void updateCheckpoint(int index) {
      JCheckpointsView checkpointsView = (JCheckpointsView)parent.getCheckpointsView();
      checkpointsView.updateCheckpoint(index);
   }
   
   // delete the Checkpoint by index
	public void deleteCheckpoint(int index) {
      JCheckpointsView checkpointsView = (JCheckpointsView)parent.getCheckpointsView();
      checkpointsView.deleteCheckpoint(index);

   }
   
   // is called, when a new checkpoint list is available 
	public void newCheckpointList(JCheckpointList l) {
      debug.DENTER("JTreeView.newCheckpointList");

		Enumeration e = l.elements();
      
		while (e.hasMoreElements()) {
			JCheckpoint checkpoint = (JCheckpoint)e.nextElement();
			DefaultMutableTreeNode node = new DefaultMutableTreeNode(checkpoint);
			checkpointsRoot.add(node);
			JCheckpointView ckv = (JCheckpointView)parent.getCheckpointView();
			ckv.setJCheckpoint(checkpoint);
		}
      
      JCheckpointsView checkpointsView = (JCheckpointsView)parent.getCheckpointsView();
      checkpointsView.setCheckpointList(l);
		expandTree();
		debug.DEXIT();
	}


   // update the complex by index
   public void updateComplex(int index) {
      JComplexesView complexesView = (JComplexesView)parent.getComplexesView();
      complexesView.updateComplex(index);
   }
 
   // delete the Complex by index
	public void deleteComplex(int index) {
      JComplexesView complexesView = (JComplexesView)parent.getComplexesView();
      complexesView.deleteComplex(index);

   }
   
   // is called, when a new complex list is available 
   public void newComplexList(JComplexList l) {
      debug.DENTER("JTreeView.newComplexList");
   
      /*Enumeration e = l.elements();
   
      while (e.hasMoreElements()) {
         JComplex complexx = (JComplex)e.nextElement();
         DefaultMutableTreeNode node = new DefaultMutableTreeNode(complex);
         complexesRoot.add(node);
         JComplexView cxv = (JComplexView)parent.getComplexView();
         cxv.setJComplex(complex);
      }*/
   
      JComplexesView complexesView = (JComplexesView)parent.getComplexesView();
      complexesView.setComplexList(l);
 
      expandTree();
      debug.DEXIT();
   }
   
        
   // update the host by index
   public void updateHost(int index) {
      JHostsView hostsView = (JHostsView)parent.getHostsView();
      hostsView.updateHost(index);
   }
	
   // delete the Host by index
	public void deleteHost(int index) {
      JHostsView hostsView = (JHostsView)parent.getHostsView();
      hostsView.deleteHost(index);

   }
   
   // is called, when a new host list is available 
	public void newHostList(JHostList l) {
		debug.DENTER("JTreeView.newHostList");
      
		Enumeration e = l.elements();
      
		while (e.hasMoreElements()) {
			JHost host = (JHost)e.nextElement();
			DefaultMutableTreeNode node = new DefaultMutableTreeNode(host);
			hostsRoot.add(node);
			JHostView ehv = (JHostView)parent.getHostView();
			ehv.setJHost(host);
		}
      
      JHostsView hostsView = (JHostsView)parent.getHostsView();
      hostsView.setHostList(l);
		expandTree();
		debug.DEXIT();
	}
  
   
   // update the queue by index
	public void updateQueue(int index) {
		JQueuesView queuesView = (JQueuesView)parent.getQueuesView();
      queuesView.updateQueue(index);
	}

   // delete the queue by index
	public void deleteQueue(int index) {
      JQueuesView queuesView = (JQueuesView)parent.getQueuesView();
      queuesView.deleteQueue(index);

   }

   // is called, when a new queue list is available 
	public void newQueueList(JQueueList l) {
		debug.DENTER("JTreeView.newQueueList");
      
		Enumeration e = l.elements();
      
		while (e.hasMoreElements()) {
			JQueue queue = (JQueue)e.nextElement();
			DefaultMutableTreeNode node = new DefaultMutableTreeNode(queue);
			queuesRoot.add(node);
			JQueueView quv = (JQueueView)parent.getQueueView();
			quv.setJQueue(queue);
		}

		JQueuesView queuesView = (JQueuesView)parent.getQueuesView();
		queuesView.setQueueList(l);

		expandTree();
		debug.DEXIT();
	}


	// returns the tree view root 
	public DefaultMutableTreeNode getRoot() {
		debug.DENTER("JTreeView.getRoot");
		debug.DEXIT();
		return root;
	}

	// expand the tree completely 
	public void expandTree() {
		DefaultTreeModel dtm = (DefaultTreeModel)tree.getModel();
		TreePath tp = null;
		int i = 0;
		
		while ( i < tree.getRowCount()) {
			tp = tree.getPathForRow(i);
			tree.expandPath(tp);
			i++;
		}
		dtm.reload();
	}
}
