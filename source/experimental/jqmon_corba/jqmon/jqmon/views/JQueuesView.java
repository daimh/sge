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
import java.util.*;



public class JQueuesView extends JPanel {

	protected JDebug debug = null;
	// a copy of the queue
	protected JQueueList queues;
	JQueuesQueuesPanel queuesPanel = null;
	//JQueueGeneralPanel generalPanel = null;
	
	public JQueuesView(JDebug d) {
		super();
		debug = d;
		initGUI();
		setMinimumSize   (new Dimension(300,200));
      setPreferredSize (new Dimension(800,800));
	}

	
	public JQueuesView(JDebug d, JQueueList qs) {

		this(d);
		setQueueList(qs);
	}

	// initialize the GUI
	public void initGUI() {
		debug.DENTER("JQueuesView.initGUI");

		JGridBagLayout layout = new JGridBagLayout();
		JTabbedPane tab 		 = new JTabbedPane();
		setLayout(layout);

		//generalPanel = new JQueueGeneralPanel(debug);
/*		Enumeration e = queues.elements();
		
		Object[][] data = new Object[queues.capacity()][4];
		for(int i = 0; i < queues.capacity(); i++){
			JQueue q = (JQueue)e.nextElement();
			for(int j = 0; j < 4; j++){
				data[i][j] = new Long(q.getID());
				data[i][j] = q.getQName();
				data[i][j] = new Long(q.getJobSlots());
				data[i][j] = q.getQHostName();
			}
		}
*/   
/*		String[] columnNames = {"Status", 
                                      "Name",
                                      "JobSlots",
                                      "Host"};
		Object[][] data = new Object[0][0];
		final JTable table = new JTable(data, columnNames);		
		JScrollPane scrollPane = new JScrollPane(table);
                table.setPreferredScrollableViewportSize(new Dimension(500, 70));

		JPanel jp = new JPanel();
		jp.setMinimumSize(new Dimension(400, 200));
		JGridBagLayout jgbl = new JGridBagLayout();
		jp.setLayout(jgbl);
		jgbl.constrain(jp, new Label("Nothing"), 1, 1, 1, 1, GridBagConstraints.BOTH,
				GridBagConstraints.NORTH, 1.0, 1.0, 1,1,5,5 );	
*/

		queuesPanel = new JQueuesQueuesPanel(debug, null);

		tab.addTab("Queues", queuesPanel);
		tab.addTab("Dummy", new JPanel());
		
		layout.constrain(this, tab, 1,1,1,1, GridBagConstraints.BOTH,
				           GridBagConstraints.NORTH, 1.0, 1.0, 1,1,5,5);
		debug.DEXIT();
	}


	public void setQueueList(JQueueList qs) {
		debug.DENTER("JQueuesView.setJQueueList");
		
		queuesPanel.setQueueList(qs);		

		// supply data for the general panel
		queues = qs;
/*		
		try {
			generalPanel.setQName(queue.getQName());
			generalPanel.setQHostName(queue.getQHostName());
			generalPanel.setTmpDir(queue.getTmpDir());
			generalPanel.setJobSlots(queue.getJobSlots());
		} catch (Exception e) {
			debug.DPRINTF("Fehler: " + e);
		}
*/
		debug.DEXIT();
	}

	public void updateQueue(int index) {
		queuesPanel.updateQueue(index);
	}

	public void deleteQueue(int index) {
      debug.DENTER("JQueuesView.deleteJQueue");
		queuesPanel.deleteQueue(index);
		debug.DEXIT();
   }


	public Object clone() {
		try {
			return super.clone();
		} catch (Exception e) {}
		return new JQueue(debug);
	}
}
