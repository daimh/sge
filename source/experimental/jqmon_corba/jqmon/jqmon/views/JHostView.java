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

/* Swing 1.0.x 
import com.sun.java.swing.*;
import com.sun.java.swing.event.*;
import com.sun.java.swing.border.*;
*/

/* Swing 1.1 */
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import jqmon.*;
import jqmon.debug.*;
import jqmon.util.*;
import jcodine.*;



public class JHostView extends JPanel {

	protected JDebug debug = null;

	// a copy of the JHost
	protected JHost host;
	
	JHostGeneralPanel generalPanel = null;
	
	public JHostView(JDebug d) {
		super();
		debug = d;
		initGUI();
      setMinimumSize  (new Dimension(300,800));
      setPreferredSize(new Dimension(800,800));
	}

	
	public JHostView(JDebug d, JHost q) {
		this(d);
		setJHost(q);
	}

	// initialize the GUI
	public void initGUI() {
		debug.DENTER("JHostView.initGUI");

		JGridBagLayout layout = new JGridBagLayout();
		JTabbedPane tab 		 = new JTabbedPane();
		setLayout(layout);

		generalPanel = new JHostGeneralPanel(debug);

		tab.addTab("General", generalPanel);
		tab.addTab("Dummy", new JPanel());
		
		layout.constrain(this, tab, 1,1,1,1, GridBagConstraints.BOTH, GridBagConstraints.NORTH, 1.0, 1.0, 1,1,5,5);
		debug.DEXIT();
	}


	public void setJHost(JHost q) {
		debug.DENTER("JHostView.setJHost");

		// supply the general panel with data
		host = q;
		
		try {
			generalPanel.setName(host.getName());
			generalPanel.setLtHeardFrom(host.getLtHeardFrom());
		} catch (Exception e) {
			debug.DPRINTF("Fehler: " + e);
		}
		debug.DEXIT();
	}

	public Object clone() {
		try {
			return super.clone();
		} catch (Exception e) {}
		return new JHost(debug);
	}
}
