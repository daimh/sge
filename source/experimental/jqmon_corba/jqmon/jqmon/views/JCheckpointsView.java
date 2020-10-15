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

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import jqmon.*;
import jqmon.debug.*;
import jqmon.util.*;
import jcodine.*;
import java.util.*;



public class JCheckpointsView extends JPanel {

	// a copy of the checkpoint list 
	protected JCheckpointList checkpoints;
	protected JDebug debug = null;
	JCheckpointsCheckpointsPanel checkpointsPanel = null;

   
	public JCheckpointsView(JDebug d) {
		super();
		debug = d;
		initGUI();
		setMinimumSize   (new Dimension(300,200));
      setPreferredSize (new Dimension(800,800));
	}

	
	public JCheckpointsView(JDebug d, JCheckpointList ckptl) {
		this(d);
		setCheckpointList(ckptl);
	}

	// initialize the GUI
	public void initGUI() {
		debug.DENTER("JCheckpointsView.initGUI");

		JGridBagLayout layout = new JGridBagLayout();
		JTabbedPane tab 		 = new JTabbedPane();
		setLayout(layout);

		checkpointsPanel = new JCheckpointsCheckpointsPanel(debug, null);

		tab.addTab("Checkpoints", checkpointsPanel);
		tab.addTab("Dummy", new JPanel());
		
		layout.constrain(this, tab, 1,1,1,1, GridBagConstraints.BOTH, GridBagConstraints.NORTH, 1.0, 1.0, 1,1,5,5);
		debug.DEXIT();
	}


	public void setCheckpointList(JCheckpointList ckl) {
		debug.DENTER("JCheckpointsView.setCheckpointList");
		
		checkpointsPanel.setCheckpointList(ckl);		
		checkpoints = ckl;
		debug.DEXIT();
	}

	public void updateCheckpoint(int index) {
		checkpointsPanel.updateCheckpoint(index);
	}

	public void deleteCheckpoint(int index) {
      debug.DENTER("JCheckpointsView.deleteCheckpoint");
      checkpoints.removeElementAt(index);
		checkpointsPanel.deleteCheckpoint(index);
		debug.DEXIT();
   }


	public Object clone() {
		try {
			return super.clone();
		} catch (Exception e) {}
		return new JCheckpoint(debug);
	}
}
