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

import java.util.*;

import javax.swing.table.*;

import jcodine.*;
import jqmon.*;
import jqmon.debug.*;
import jqmon.util.*;


public class JQueuesQueuesPanel extends JPanel {

	protected JDebug debug = null;
	protected static ResourceBundle messages = null;
	JGridBagLayout layout = null;
	QueuesTableModel queuesModel = null;
	JTable table = null;
	JScrollPane scrollPane = null;
	
	public JQueuesQueuesPanel(JDebug d, JQueueList qList){	
		super();
		layout = new JGridBagLayout();
		setLayout(layout);
		debug = d;
		Locale locale = Locale.getDefault();
		messages = ResourceBundle.getBundle("jqmon/views/MessageBundle", locale);
		initGUI(qList);
	}

	protected void initGUI(JQueueList qList) {
		debug.DENTER("JQueuesQueuesPanel.initGUI");
		
		queuesModel = new QueuesTableModel(qList);
                JTable table = new JTable(queuesModel);
/*		JTable table = new JTable(4, 1)
		{
                             public TableCellRenderer getCellRenderer(int row, int column) {
                                     TableColumn tableColumn = getColumnModel().getColumn(column);
                                     TableCellRenderer renderer = tableColumn.getCellRenderer();
                                     if (renderer == null) {
                                             Class c = getColumnClass(column);
                                             if( c.equals(Object.class) )
                                             {
                                                     Object o = getValueAt(row,column);
                                                     if( o != null )
                                                             c = getValueAt(row,column).getClass();
                                             }
                                             renderer = getDefaultRenderer(c);
                                     }
                                     return renderer;
                             }
                             
                             public TableCellEditor getCellEditor(int row, int column) {
                                     TableColumn tableColumn = getColumnModel().getColumn(column);
                                     TableCellEditor editor = tableColumn.getCellEditor();
                                     if (editor == null) {
                                             Class c = getColumnClass(column);
                                             if( c.equals(Object.class) )
                                             {
                                                     Object o = getValueAt(row,column);
                                                     if( o != null )
                                                             c = getValueAt(row,column).getClass();
                                             }
                                             editor = getDefaultEditor(c);
                                     }
                                     return editor;
                             }
                             
                };
		table.setDefaultRenderer( JComponent.class, new JComponentCellRenderer() );
                table.setDefaultEditor( JComponent.class, new JComponentCellEditor() );		
*/
                scrollPane = new JScrollPane(table);
              	
		// try to put CellEditor
/*		final JButton button = new JButton("");
		final TestEditor testEditor = new TestEditor(button, debug);
	        table.setDefaultEditor(Long.class, testEditor);
		final JTestDialog dialog = new JTestDialog(debug);
		button.addActionListener(new ActionListener() {
	            public void actionPerformed(ActionEvent e) {
        	        //button.setBackground(colorEditor.currentColor);
                	
			//dialog.setLong(testEditor.currentNomber);
                	
			//Without the following line, the dialog comes up
                	//in the middle of the screen.
                	//dialog.setLocationRelativeTo(button);
                	dialog.show();
			dialog.pack();
                   }
        	});		
*/
		
/*		for(int i=0; i < 4; i++) {
			//table.setValueAt(new JTextField("0", 5), i, 0);
			table.setValueAt(new JScrollBar(JScrollBar.VERTICAL), i , 0);
		}	
*/	
		// try to put LongEditor
		final JLongPanel  longPanel = new JLongPanel(0, 5, debug);
		final LongCellEditor longEditor = new LongCellEditor(longPanel);
		//table.setDefaultEditor(Long.class, longEditor);	
		table.setDefaultEditor(Integer.class, longEditor);
			
	  	table.setPreferredScrollableViewportSize(new Dimension(800, 800));
		layout.constrain(this, scrollPane, 1,1,1,1, GridBagConstraints.BOTH, GridBagConstraints.NORTH, 1.0, 1.0, 1,1,5,5 );

		debug.DEXIT();
	}
	
	public void setQueueList(JQueueList queueList) {
		queuesModel.setQueueList(queueList);
	}

	public void updateQueue(int index) {
		queuesModel.updateQueue(index);
	}	

	public void deleteQueue(int index) {
		debug.DENTER("JQueuesQueuesPanel.deleteQueue()"); 
      queuesModel.deleteQueue(index);
		debug.DEXIT();
   }

	// inner class
	class JComponentCellRenderer implements TableCellRenderer {
      public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
         return (JComponent)value;
      }
   }
}
