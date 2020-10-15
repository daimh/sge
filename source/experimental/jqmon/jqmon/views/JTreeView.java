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

/* Swing 1.0.x 
import com.sun.java.swing.*;
import com.sun.java.swing.event.*;
import com.sun.java.swing.border.*;
import com.sun.java.swing.tree.*;
*/

/* Swing 1.1 */
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;
import javax.swing.tree.*;

import jqmon.*;
import jqmon.util.*;
import jqmon.debug.*;
import codine.*;
/**
  * Diese Klasse stellt das TreeView im Codine-Fenster dar.
  *
  */

public class JTreeView extends JPanel {

	protected JTree     tree			= null;
	protected Dimension dimension		= null;

	protected JDebug debug = null;
	
	/** Die absolute root */
	protected DefaultMutableTreeNode root = null;
	
	/** Der Knoten, dem die Queues untergeordnet werden */
	protected DefaultMutableTreeNode queuesRoot = null;

	/** Der Knoten, dem die Jobs untergeordnet werden */
	protected DefaultMutableTreeNode jobsRoot = null;

	/** Der Knoten, dem die Hosts untergeordnet werden */
	protected DefaultMutableTreeNode hostsRoot = null;

	protected JCodineWindow parent = null;

	protected JQueueSet queues = null;
	protected JHostSet  hosts  = null;

	public JTreeView(JDebug d, JCodineWindow w) {
		this(d, w, new Dimension(150,200));
	}

	
	public JTreeView(JDebug de, JCodineWindow w, Dimension d) {
		super();
		dimension = d;
		parent = w;
		debug = de;
		queues = new JQueueSet();
		hosts  = new JHostSet();
		initGUI();
	}
	
	
	public void initGUI() {
		debug.DENTER("JTreeView.initGUI");
		root       = new DefaultMutableTreeNode("Codine");
		queuesRoot = new DefaultMutableTreeNode("Queues");
		jobsRoot   = new DefaultMutableTreeNode("Jobs");
		hostsRoot  = new DefaultMutableTreeNode("Hosts");
		
		JGridBagLayout layout = new JGridBagLayout();
		JScrollPane		pane	 = new JScrollPane();
		
		root.add(queuesRoot);
		root.add(jobsRoot);
		root.add(hostsRoot);

		tree = new JTree(root);

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
				
				JQueue queue = new JQueue(debug);
				JHost  host  = new JHost(debug);
				
				TreePath pfad = e.getPath();
				DefaultMutableTreeNode sel = 
							(DefaultMutableTreeNode)pfad.getLastPathComponent();
				//**********************
				Object object = sel.getUserObject();
				Class cl = object.getClass();
				
				/* Eine Queue wurde markiert */
				if ( cl.equals(queue.getClass()) ) {
					queue = (JQueue)object;
					queues.clear();
					queues.add(queue);
					JQueueView v = (JQueueView)parent.getQueueView();
					v.setJQueue(queues.getFirst());
				} else if (cl.equals(host.getClass()) ) {
					/* Ein Host wurde markiert */
					host = (JHost)object;
					hosts.clear();
					hosts.add(host);
					JHostView v = (JHostView)parent.getHostView();
					v.setJHost(hosts.getFirst());
				}
			
				debug.DPRINTF("Klasse: " + sel.getClass());
				debug.DPRINTF("QUEUES: " + queues.getFirst().toString());
				debug.DEXIT();
			}
		} );

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


	/** Wird aufgerufen, wenn eine neue QueueListe da ist */
	public void newQueueList(JQueueList l) {
		debug.DENTER("JTreeView.newQueueList");
		Enumeration e = l.elements();
		while (e.hasMoreElements()) {
			JQueue queue = (JQueue)e.nextElement();
			DefaultMutableTreeNode node = new DefaultMutableTreeNode(queue);
			queuesRoot.add(node);
			JQueueView p = (JQueueView)parent.getQueueView();
			p.setJQueue(queue);
		}
		expandTree();
		debug.DEXIT();
	}

	
	/** Wird aufgerufen, wenn eine neue HostListe da ist */
	public void newHostList(JHostList l) {
		debug.DENTER("JTreeView.newHostList");
		Enumeration e = l.elements();
		while (e.hasMoreElements()) {
			JHost host = (JHost)e.nextElement();
			DefaultMutableTreeNode node = new DefaultMutableTreeNode(host);
			hostsRoot.add(node);
			JHostView p = (JHostView)parent.getHostView();
			p.setJHost(host);
		}
		expandTree();
		debug.DEXIT();
	}


	/** Liefert den Wurzeleintrag im Tree zurueck */
	public DefaultMutableTreeNode getRoot() {
		debug.DENTER("JTreeView.getRoot");
		debug.DEXIT();
		return root;
	}


	/** Expandiert den kompletten Tree */
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
