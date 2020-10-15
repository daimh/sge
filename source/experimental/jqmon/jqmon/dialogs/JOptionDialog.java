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
package jqmon.dialogs;

/* Swing 1.0.x 
import com.sun.java.swing.*;
import com.sun.java.swing.event.*;
*/

/* Swing 1.1 */
import javax.swing.*;
import javax.swing.event.*;

import java.util.*;
import java.awt.*;
import java.awt.event.*;

import jqmon.*;
import jqmon.util.*;
import jqmon.debug.*;
import jqmon.views.*;


public class JOptionDialog extends JDialog {

	ResourceBundle messages;

	/* Die ReturnStates */
	public static final int OK     = 1;
	public static final int CANCEL = 2;

	private int returnState;
	

	/* Das aktive JDebug-Objekt */
	private JDebug debug = null;
	/* Was soll in einem bestimmten Level debugged werden? */
	private boolean prints[][];

	/* Welcher Eintrag in der debugList wurde angewaehlt? */
	private int selected;

	/* Wieviele Debug-Zeilen sollen gebuffert werden? */
	private JTextField debugLines;

	/* In welches Logfile soll geloggt werden */
	private JTextField logFileName;

	/* In der Liste stehen alle DebugClasses */
	private JList debugList;

	/* Die Monitoring Level */
	private long monitoring_level[];
	
	/* Die Checkboxes */
	private JCheckBox trace;
	private JCheckBox info;
	private JCheckBox lgToFile;
	private JCheckBox lgToStdErr;
	
	/* Damit die Monitoring-Level richtig angezeigt werden.
		Nicht schoen, aber geht. */
	private boolean doIt;	

	/* Der LogFileName */
	private String logFilename = null;

	/* Das LogDirectory */
	private String logDirectory = null;
	
	public JOptionDialog(JFrame parent, ResourceBundle m, JDebug d) {
		/* Der Dialog soll modal sein */
		super(parent, "Options", true);
		
		messages = m;
		selected = 1;
		doIt     = false;
		debug    = d;

		monitoring_level = new long[8];
		for (int i = 0; i < 8; i++) {
			monitoring_level[i] = 0;
		}

		initGUI();

		/* Defaultwerte einsetzen */
		debugLines.setText("2000");
		logFilename = "debug.log";
		logDirectory = System.getProperty("user.home");
		logFileName.setText(logDirectory + "/" + logFilename);
		logFileName.setEnabled(false);
		lgToFile.setSelected(false);
		lgToStdErr.setSelected(false);	
		returnState = CANCEL;
		
		for (int i = 0; i < 8; i++) {
			monitoring_level[i] = 0;
		}

		selected = 0;
		debugList.setSelectedIndex(0);
		

		pack();

		/* Center the Dialog */
		Toolkit screen = getToolkit();
		Dimension screensize = screen.getScreenSize();
		int y = screensize.height/2 - getSize().height/2;
		int x = screensize.width/2  - getSize().width/2;
		setLocation(x,y);
	}

	protected void initGUI() {
		debug.DENTER("JOptionDialog.initGUI");

		JTabbedPane tab            = new JTabbedPane();

		/* Panels erzeugen */
		JGridBagLayout layout      = new JGridBagLayout();
		JPanel debugPanel          = new JPanel(layout);
		JPanel buttonPanel         = new JPanel(layout);


		
		/* debugPanel belegen */
		String data[] = { "TOP_LAYER",
				            "CULL_LAYER",
								"BASIS_LAYER",
								"GUI_LAYER",
								"SCHED_LAYER",
								"COMMD_LAYER",
								"API_LAYER",
								"PACK_LAYER" };
								
								
		debugList 		      		= new JList(data);
		JLabel label1         		= new JLabel(messages.getString("debugLabel1"));
		JLabel label2         		= new JLabel(messages.getString("debugLabel2"));
		trace 							= new JCheckBox("Trace");
		info  							= new JCheckBox("InfoPrint");
		lgToFile 						= new JCheckBox(messages.getString("logToFile"));
		lgToStdErr   					= new JCheckBox(messages.getString("logToStdErr"));
	   debugLines        	 		= new JTextField(10);
		logFileName						= new JTextField(30);
		JButton markAll            = new JButton(messages.getString("buttonMarkAll"));
		JButton demarkAll				= new JButton(messages.getString("buttonDemarkAll"));
		final JButton fileButton	= new JButton(messages.getString("buttonChooseFile"));
		
		trace.addItemListener(new ItemListener() {
			public void itemStateChanged(ItemEvent e) {
				if ( doIt ) {
					if ( trace.isSelected() ) {
						monitoring_level[selected] += 1;
					} else {
						monitoring_level[selected] -= 1;
					}
				}
			}
		} );
		
		info.addItemListener(new ItemListener() {
			public void itemStateChanged(ItemEvent e) {
				if ( doIt ) {
					if ( info.isSelected() ) {
						monitoring_level[selected] += 2;
					} else {
						monitoring_level[selected] -= 2;
					}
				}
			}
		} );

		lgToFile.addItemListener(new ItemListener() {
			public void itemStateChanged(ItemEvent e) {
				logFileName.setEnabled(lgToFile.isSelected());
				fileButton.setEnabled(lgToFile.isSelected());
			}
		} );
		
		debugList.addListSelectionListener(new ListSelectionListener() {
			public void valueChanged(ListSelectionEvent e) {
				try {
					selected = e.getFirstIndex();
					doIt = false;
					switch((int)monitoring_level[selected]) {
						case 0 : { trace.setSelected(false); info.setSelected(false); break; }
						case 1 : { trace.setSelected(true);  info.setSelected(false); break; }
						case 2 : { trace.setSelected(false); info.setSelected(true);  break; }
						case 3 : { trace.setSelected(true);  info.setSelected(true);  break; }
						default: { trace.setSelected(false); info.setSelected(false); break; }
					}
				} catch (IndexOutOfBoundsException ex) {}
				doIt = true;
			}
		} );

		markAll.addActionListener( new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				for (int i = 0; i < 8; i++) {
					int s = selected;
					monitoring_level[i] = 3;
					if ( s == 0 ) {
						debugList.setSelectedIndex(7);
					} else {
						debugList.setSelectedIndex(0);
					}
					selected = s;
					debugList.setSelectedIndex(selected);
					repaint();
				}
			}
		} );
		
		demarkAll.addActionListener( new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				for (int i = 0; i < 8; i++) {
					int s = selected;
					monitoring_level[i] = 0;
					if ( s == 0 ) {
						debugList.setSelectedIndex(7);
					} else {
						debugList.setSelectedIndex(0);
					}
					selected = s;
					debugList.setSelectedIndex(selected);
					repaint();
				}
			}
		} );
		fileButton.addActionListener( new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				FileDialog fd = new FileDialog(new Frame(), "File");
				fd.show();
				setLogDirectory(fd.getDirectory());
				setLogFileName(fd.getFile());
			}
		} );

		debugList.setSelectedIndex(0);
		debugList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		
		layout.constrain(debugPanel, label1,     1,1,1,1,  8,18,8,8);
		layout.constrain(debugPanel, debugList,  1,2,1,4,  8,18,18,8);
		layout.constrain(debugPanel, trace,      2,2,1,1,  8,8,8,8);
		layout.constrain(debugPanel, info,       2,3,1,1,  8,8,8,8);
		layout.constrain(debugPanel, markAll,    3,2,1,1,  8,8,8,8);
		layout.constrain(debugPanel, demarkAll,  3,3,1,1,  8,8,8,8);
		layout.constrain(debugPanel, label2,     1,10,1,1, 8,8,18,8);
		layout.constrain(debugPanel, debugLines, 2,10,1,1, 8,8,8,8);
		layout.constrain(debugPanel, lgToStdErr, 1,11,1,1, 8,8,8,8);
		layout.constrain(debugPanel, lgToFile,   1,12,1,1, 8,8,18,8);
		layout.constrain(debugPanel, logFileName,1,13,2,1, 0,8,18,8);
		layout.constrain(debugPanel, fileButton, 3,13,1,1, 0,8,8,8);

		/* buttonPanel erzeugen */
		JButton okButton     = new JButton(messages.getString("SOkay"));
		JButton cancelButton = new JButton(messages.getString("SCancel"));
		okButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				debug.DENTER("okButton.actionListener");
				returnState = OK;
				dispose();
				debug.DEXIT();
			}
		} );
		cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				debug.DENTER("cancelButton.actionListener");
				returnState = CANCEL;
				dispose();
				debug.DEXIT();
			}
		} );
		layout.constrain(buttonPanel, okButton,     1,1,1,1, 4,4,4,4);
		layout.constrain(buttonPanel, cancelButton, 2,1,1,1 ,4,4,4,4);

		JQueueGeneralPanel p = new JQueueGeneralPanel(debug);
		tab.addTab("Test", p);
		tab.addTab(messages.getString("OptionsTab1"), debugPanel);

		this.getContentPane().add("Center", tab);
		this.getContentPane().add("South",  buttonPanel);
		debug.DEXIT();
	}

	/*
	 * Die ganzen set und get Funktionen, um den Dialog vorzubereiten
	 * bzw. auszulesen
	 */

	/* MonitoringLevel */
	public void setMonitoringLevel(long m[]) {
		debug.DENTER("JOptionDialog.setMonitoringLevel");
		if ( m.length != 8 ) {
			debug.DPRINTF("Array length failure!!");
			debug.DEXIT();
			return;
		}

		for (int i = 0; i < 8; i++) {
			monitoring_level[i] = m[i];
		}
		debugList.setSelectedIndex(7);
		debugList.setSelectedIndex(0);
		selected=0;
		debug.DEXIT();
	}

	public long[] getMonitoringLevel() {
		debug.DENTER("JOptionDialog.getMonitoringLevel");
		debug.DEXIT();
		return monitoring_level;
	}

	/* LogToFile */
	public void setLogToFile(boolean log) {
		debug.DENTER("JOptionDialog.setLogToFile");
		lgToFile.setSelected(log);
		logFileName.setEnabled(log);
		debug.DEXIT();
	}

	public boolean getLogToFile() {
		debug.DENTER("JOptionDialog.getLogToFile");
		debug.DEXIT();
		return lgToFile.isSelected();
	}

	/* LogToStdErr */
	public void setLogToStdErr(boolean log) {
		debug.DENTER("JOptionDialog.setLogToStdErr");
		lgToStdErr.setSelected(log);
		debug.DEXIT();
	}

	public boolean getLogToStdErr() {
		debug.DENTER("JOptionDialog.getLogToStdErr");
		return lgToStdErr.isSelected();
	}

	/* LogFileName */
	public void setLogFileName(String s) {
		debug.DENTER("JOptionDialog.setLogFileName");
		logFilename = s;
		logFileName.setText(logDirectory + logFilename);
		debug.DEXIT();
	}

	public String getLogFileName() {
		debug.DENTER("JOptionDialog.getLogFileName");
		debug.DEXIT();
		return logFilename;
	}


	/* LogDirectory */
	public void setLogDirectory(String s) {
		debug.DENTER("JOptionDialog.setLogDirectory");
		logDirectory = s;
		logFileName.setText(logDirectory + logFilename);
		debug.DEXIT();
	}

	public String getLogDirectory() {
		debug.DENTER("JOptionDialog.getLogDirectory");
		debug.DEXIT();
		return logDirectory;
	}

	/* DebugLines */
	public void setDebugLine(int lines) {
		debug.DENTER("JOptionDialog.setDebugLines");
		debugLines.setText(Integer.toString(lines));
		debug.DEXIT();
	}

	public int getDebugLines() {
		// FIXME: Sollte eigentlich besser gehen...
		debug.DENTER("JOptionDialog.getDebugLines");
		int i = 1000;
		try {	
		   i = Integer.parseInt(debugLines.getText());
		} catch (NumberFormatException e) {
			debug.DPRINTF("Caught NumberFormatException: Debug Lines = " + i);
		}
		debug.DEXIT();
		return i;
	}

	/* ReturnState */
	public int getReturnState() {
		debug.DENTER("JOptionDialog.getReturnState");
		debug.DEXIT();
		return returnState;
	}
}
