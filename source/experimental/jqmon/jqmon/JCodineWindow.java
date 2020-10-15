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

/* Swing 1.0.x 
import com.sun.java.swing.*;
import com.sun.java.swing.event.*;
import com.sun.java.swing.border.*;
*/

/* Swing 1.1 */
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import jqmon.util.*;
import jqmon.debug.*;
import jqmon.views.*;
import jqmon.events.*;
import codine.*;

/** 
  * Diese Klasse stellt das Codine-Fenster dar. Sie enthaelt den
  * TreeView (rechts) und links die im TreeView ausgewaehlte View.
  *
  * @author		Michael Roehrl
  * @version	1.0
  */

public class JCodineWindow extends JFrame 
									implements JQueueListListener,
												  JHostListListener		
{

	/** Verbindung zum WorkerThread */
	protected JWorkerThread wt = null;

	/** Das JDebug Objekt */
	protected JDebug debug = null;

	/** Die SplitPane */
	protected JSplitPane splitPane = null;

	/** Das JTreeView */
	protected JTreeView treeView   = null;

	/** Das QueueView */
	protected JQueueView queueView = null;
	
	/** Das HostView */
	protected JHostView hostView = null;
	/** */
	protected JUpdateQueueList updateQueueList = null;

	protected JUpdateHostList updateHostList = null;
	

	/** Erzeugt ein JCodineWindow */
	public JCodineWindow(JWorkerThread w,    JDebug d,
			               JUpdateQueueList q, JUpdateHostList h,
								String title) {
		//super(title, true, true, true, true);
		super(title);
		wt    = w;
		debug = d;
		updateQueueList = q;
		updateHostList  = h;
		initGUI();
		updateQueueList.addJQueueListListener(this);
		updateHostList.addJHostListListener(this);
	}

	/** Erzeugt ein JCodineWindow */
	public JCodineWindow(JWorkerThread w, JDebug d, JUpdateQueueList q,
								JUpdateHostList h) {
		this(w, d, q, h, "Codine Window");
	}

	/** Initialisiere die GUI */
	protected void initGUI() {
		debug.DENTER("JCodineWindow.initGUI");
		JPanel p = new JPanel();

		splitPane 				= new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);

		queueView				= new JQueueView(debug);
		hostView					= new JHostView(debug);
		treeView					= new JTreeView(debug,this);
		
		splitPane.setLeftComponent(treeView);
		setView(queueView);
		splitPane.setContinuousLayout(true);
		p.add(splitPane, BorderLayout.CENTER);	
		//!!!!!!!!!!!!!!!!!!!!!!!11
		getContentPane().add(p, BorderLayout.CENTER);
		debug.DEXIT();
	}


	/** Der JQueueListListener */
	public void newJQueueListAvailable(JQueueListEvent e) {
		debug.DENTER("JCodineWindow.newJQueueListAvailable");
		/* Laut Java-Doku habe ich jetzt eine Kopie der */
		/* JQueueList in der Kopien der Originalein-    */
		/* traege sind. Hoffentlich ist dem auch so!!!! */
		JAnswerList al = (JAnswerList)updateQueueList.getAnswerList().clone();
		debug.DPRINTF("JAnswerList: " + al);
		try {
			JQueueList l = (JQueueList)updateQueueList.getQueueList().clone();
			treeView.newQueueList(l);
		} catch (JAnswerListException ex) {
			debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
		}
		debug.DEXIT();
	}

	
	/** Der JHostListListener */
	public void newJHostListAvailable(JHostListEvent e) {
		debug.DENTER("JCodineWindow.newJHostListAvailable");
		/* Laut Java-Doku habe ich jetzt eine Kopie der */
		/* JQueueList in der Kopien der Originalein-    */
		/* traege sind. Hoffentlich ist dem auch so!!!! */
		JAnswerList al = (JAnswerList)updateHostList.getAnswerList().clone();
		debug.DPRINTF("JAnswerList: " + al);
		try {
			JHostList l = (JHostList)updateHostList.getHostList().clone();
			treeView.newHostList(l);
		} catch (JAnswerListException ex) {
			debug.DPRINTF("AnswerList: " + ex.getAnswerObject());
		}
		debug.DEXIT();
	}
	/**
	  * set und get - Funktionen
	  */

	/** JQueueView */
	public void setView(JPanel view) {
		debug.DENTER("JCodineWindow.setView");
		splitPane.setRightComponent(view);
		debug.DEXIT();
	}

	public JPanel getView() {
		return (JPanel)splitPane.getRightComponent();
	}

	public JPanel getQueueView() {
		debug.DENTER("JCodineWindow.getQueueView");
		Class cl = getView().getClass();
		Class qv = queueView.getClass();

		if ( !cl.equals(qv) ) {
			setView(queueView);
			debug.DPRINTF("changed view to QUEUEVIEW");
		}
		debug.DEXIT();
		return queueView;
	}

	public JPanel getHostView() {
		debug.DENTER("JCodineWindow.getHostView");
		Class cl = getView().getClass();
		Class hv = hostView.getClass();

		if ( !cl.equals(hv) ) {
			setView(hostView);
			debug.DPRINTF("changed view to HOSTVIEW");
		}

		debug.DEXIT();
		return hostView;
	}

	/** TreeView (kann nur geholt werden!!! */
	public JTreeView getTreeView() {
		debug.DENTER("JCodineWindow.getTreeView");
		debug.DEXIT();
		return treeView;
	}
	
}
