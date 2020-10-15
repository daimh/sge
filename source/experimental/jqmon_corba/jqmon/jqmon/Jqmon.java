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
import java.util.*;
import java.io.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import jcodine.*;
import jqmon.*;
import jqmon.dialogs.*;
import jqmon.util.*;
import jqmon.debug.*;
//import jqmon.views.*;


// this class represents the Jqmon 
// it cares for the initialization of the GUI and the workerthread
//
// @author     Michael Roehrl
// modified by Hartmut Gilde
// @version   0.01
 
 
public class Jqmon extends JFrame {

	// all members of this class are declared 
	// as ³static³ , due to the fact, that there must
	// exist only one instance of this class

	// menu entries
	private static String SfileMenu;
	private static String SfileNew;
	private static String SfileOptions;
	private static String SfileExit;

	private static String SviewMenu;
	private static String SviewDebugWindow;
   private static String SviewToolbar;

	// frequently needed strings
	private static String SYes;
	private static String SNo;
	private static String SCancel;
	private static String SOkay;
  
	// constants for the actions (menus, buttons, ...)
	private final static int ACTIONS			  = 100;
	
	// constants for the 'File' menu 
	private final static int FILEEXIT        = ACTIONS + 100;
	private final static int FILEOPTIONS     = ACTIONS + 101;
	private final static int FILENEW         = ACTIONS + 102;
	
	// constants for the 'View' menu 
	private final static int VIEWDEBUGWINDOW = ACTIONS + 200;
   private final static int VIEWTOOLBAR     = ACTIONS + 201;
   
   
	// menus, menuItems, checkBoxMenuItems, ... 
	protected static JMenuBar           menuBar        = null;	
	protected static JMenu              fileMenu       = null;
	protected static JMenu              viewMenu       = null;
	protected static JMenuItem          fileNew		   = null;
	protected static JMenuItem          fileOptions    = null;
	protected static JMenuItem          fileExit       = null;
   protected static JCheckBoxMenuItem  viewDebugWindow= null;
   protected static JCheckBoxMenuItem  viewToolbar    = null;
   
	// the toolbar
   protected static JToolBar toolbar = null;
   
	// the debug window 
   protected static JDebugWindow debugWindow = null;	
   
	// the MDI-panel 
   protected  static JLayeredPane desktop;
   
	// the model for the debug string 
   protected static JDPrintStringModel dprint = null;

   protected static JUpdateCalendarList   updateCalendarList   = null;
   protected static JUpdateCheckpointList updateCheckpointList = null;
	protected static JUpdateComplexList    updateComplexList    = null;
	protected static JUpdateHostList       updateHostList       = null;
	protected static JUpdateQueueList      updateQueueList      = null;
   
	// the workerthread 
   protected static JWorkerThread wt          = null;
   
	// the printthread for debug output 
	protected static JDPrintThread dt          = null;

	// the debug class 
	protected static JDebug debug              = null;

   // icons and buttons
   protected static ImageIcon nullIcon    = null;
   protected static ImageIcon newIcon     = null;
   protected static JFlatButton newButton = null;

	// the locale 
	protected static Locale locale           = null;
	protected static ResourceBundle messages = null;
   
	
	// the properties
   protected static Properties appProps     = null;


	//**************************************************************
	// Attributes configured by the properties-file
	//**************************************************************

	// the position of the main frame 
	protected static int framePosX;
	protected static int framePosY;
	protected static int frameHeight;
	protected static int frameWidth;

	 
	// the constructor creates a 'Jqmon' object 
	// further it initializes the GUI and starts the threads
	public Jqmon(String[] arguments) {
		super("JQMON");
	
		// load properties 
		loadProps();
		
		locale   = Locale.getDefault();
		messages = ResourceBundle.getBundle("MessageBundle",locale);

		
		// load frequently needed strings
		SYes    = messages.getString("SYes");
		SNo     = messages.getString("SNo");
		SCancel = messages.getString("SCancel");
		SOkay   = messages.getString("SOkay");
		

		// initialize GUI 
		initGUI();
		
		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
      dprint 				= new JDPrintStringModel();
      
      updateCalendarList   = new JUpdateCalendarList();
      updateCheckpointList = new JUpdateCheckpointList();
		updateComplexList    = new JUpdateComplexList();
		updateHostList		   = new JUpdateHostList();
		updateQueueList   	= new JUpdateQueueList();

		JBuffer buffer 	= new JBuffer();
		
		// create threads 
		dt = new JDPrintThread(dprint,buffer);

		// get and set properties for DPrintThread 
		String s = appProps.getProperty("logFileName", "default.log");
		dt.setLogFileName(s);

		s = appProps.getProperty("logDirectory", "$HOME");
		if ( s.equals("$HOME") ) {
			dt.setLogDirectory(System.getProperty("user.home") + "/");
		} else {
			dt.setLogDirectory(s);
		}
		
		s = appProps.getProperty("logToStdErr", "false");
		if ( s.equals("true") ) {
			dt.setLogToStdErr(true);
		} else {
			dt.setLogToStdErr(false);
		}
		
		s = appProps.getProperty("logToFile", "false");
		if ( s.equals("true") ) {
			dt.setLogToFile(true);
		} else {
			dt.setLogToFile(false);
		}

		// start threads 
		dt.start();

		// create an instance of the debug object
      debug = new JDebug(dt,buffer, appProps);


		// load monitoring level
      long ml[] = new long[JDebug.N_LAYER];
		for (int i = 0; i < JDebug.N_LAYER; i++ ) {
			ml[i] = 0;
		}
		
		// TOP_LAYER - preferences 
		s = appProps.getProperty("TOP_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.TOP_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("TOP_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.TOP_LAYER] += JDebug.INFOPRINT;
		
		// CULL_LAYER - preferences 
		s = appProps.getProperty("CULL_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.CULL_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("CULL_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.CULL_LAYER] += JDebug.INFOPRINT;

		// BASIS_LAYER - preferences 
		s = appProps.getProperty("BASIS_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.BASIS_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("BASIS_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.BASIS_LAYER] += JDebug.INFOPRINT;

		// GUI_LAYER - preferencesn 
		s = appProps.getProperty("GUI_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.GUI_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("GUI_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.GUI_LAYER] += JDebug.INFOPRINT;

		// SCHED_LAYER - preferences 
		s = appProps.getProperty("SCHED_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.SCHED_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("SCHED_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.SCHED_LAYER] += JDebug.INFOPRINT;

		// COMMD_LAYER - preferences 
		s = appProps.getProperty("COMMD_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.COMMD_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("COMMD_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.COMMD_LAYER] += JDebug.INFOPRINT;

		// API_LAYER - preferences 
		s = appProps.getProperty("API_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.API_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("API_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.API_LAYER] += JDebug.INFOPRINT;
		
		// PACK_LAYER - preferences 
		s = appProps.getProperty("PACK_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.PACK_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("PACK_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.PACK_LAYER] += JDebug.INFOPRINT;


		debug.setMonitoringLevel(ml);

		JCodObj o = new JCodObj(debug);
      wt = new JWorkerThread(debug, updateCalendarList,
                                    updateCheckpointList, 
                                    updateComplexList, 
                                    updateHostList, 
                                    updateQueueList, 
                                    arguments, new java.util.Properties());
		wt.setPriority(Thread.MIN_PRIORITY);
		wt.start();
		o.getID();
	}


	// load the properties
	protected void loadProps() {
		try {
			// load default properties 
			appProps           = new Properties();
			FileInputStream in = new FileInputStream("Jqmon.properties");
			appProps.load(in);
			in.close();
		} catch (IOException e) {
			//FIXME
			System.err.println("Couldn³t load Jqmon.properties.");
			System.err.println("Using Defaults.");
		}

		
		// read the elementary properties 
		framePosX   = Integer.parseInt(appProps.getProperty("framePosX",   "25"));
		framePosY   = Integer.parseInt(appProps.getProperty("framePosY",   "25"));
		frameWidth  = Integer.parseInt(appProps.getProperty("frameWidth",  "900"));
		frameHeight = Integer.parseInt(appProps.getProperty("frameHeight", "600"));

		if (framePosX < 10) framePosX = 10;
		if (framePosY < 10) framePosY = 10;
	}


	public void saveProps() {
		debug.DENTER("Jqmon.saveProps");
		try {
			// save the recent properties
			appProps.put("framePosX",
								Integer.toString(getLocationOnScreen().x));
			appProps.put("framePosY",
								Integer.toString(getLocationOnScreen().y));
			appProps.put("frameWidth",
								Integer.toString(getSize().width));
			appProps.put("frameHeight",
								Integer.toString(getSize().height));
			appProps.put("logFileName",
								dt.getLogFileName());
			appProps.put("logDirectory", 
								dt.getLogDirectory());
			appProps.put("logToFile", 
								(new Boolean(dt.getLogToFile())).toString());
			appProps.put("logToStdErr", 
								(new Boolean(dt.getLogToStdErr())).toString());
			
			long mll[] = debug.getMonitoringLevel();
			int ml[] = new int[JDebug.N_LAYER];
			for (int i = 0; i < JDebug.N_LAYER; i++) {
				ml[i] = (int)mll[i];
				String s = null;
				switch(i) {
					case JDebug.TOP_LAYER:		{ s="TOP"; break; }
					case JDebug.CULL_LAYER:    { s="CULL"; break; }
					case JDebug.BASIS_LAYER:   { s="BASIS"; break; }
					case JDebug.GUI_LAYER:     { s="GUI"; break; }
					case JDebug.SCHED_LAYER:   { s="SCHED"; break; }
					case JDebug.COMMD_LAYER:   { s="COMMD"; break; }
					case JDebug.API_LAYER:     { s="API"; break; }
					case JDebug.PACK_LAYER:    { s="PACK"; break; }
					default: break;
				}
				
				String l = s + "_LAYER.";
				
				switch(ml[i]) {
					case (int)JDebug.TRACE: {
												appProps.put( l + "trace",
																	"true");
												appProps.put( l + "infoprint",
																	"false");
												break;
										}
					case (int)JDebug.INFOPRINT: {
												appProps.put( l + "infoprint",
																	"false");
												appProps.put( l + "infoprint",
																	"true");
												break;
										}
					case 3:			{
												appProps.put( l + "trace",
																	"true");
												appProps.put( l + "infoprint",
																	"true");
												break;
										}
					default: {
												appProps.put( l + "trace",
																	"false");
												appProps.put( l + "infoprint",
																	"false");
								}
				}	
			}
				
														

	
			
			FileOutputStream fout = new FileOutputStream("Jqmon.properties");
         //save is deprecated -> using store
			appProps.store(fout,"---- Properties for Jqmon ----");
			fout.flush();
			fout.close();
		}
		catch (IOException e) {
			debug.DPRINTF("IOException: Couldn't save Jqmon.properties.");
		}
		catch (NullPointerException e) {
			debug.DPRINTF("NullPointerException: Couldn't save Jqmon.properties.");
		}
		debug.DEXIT();
	}


	// initialize the GUI
   protected void initGUI() {

		// set the look & feel 
		/*
		try {
			UIManager.setLookAndFeel("com.sun.java.swing.plaf.motif.MotifLookAndFeel");
		} catch (Exception e) {
			System.out.println("Look and Feel not found!");
		}
		*/
		// attach 'Exit' to the window
		WindowListener wl = new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				end();
			}
		
		};
		addWindowListener(wl);
		
		
		// retrieve the menu strings from ResourceBundle
      SfileMenu          = messages.getString("fileMenuKey");
		SfileNew				 = messages.getString("fileNewKey");
		SfileOptions       = messages.getString("fileOptionsKey");
		SfileExit          = messages.getString("fileExitKey");

		SviewMenu          = messages.getString("viewMenuKey");
		SviewDebugWindow   = messages.getString("viewDebugWindowKey");
		SviewToolbar       = messages.getString("viewToolBarKey");

		
		// load the images and create the buttons
      nullIcon = new ImageIcon("images/blank-2ž.gif");      
      newIcon  = new ImageIcon("images/New.gif");
      newButton = new JFlatButton(newIcon);

		JFlatButton dummy = new JFlatButton(nullIcon);

      
      // create menus 
      menuBar = new JMenuBar();
		setJMenuBar(menuBar);
	
		fileMenu                     = new JMenu(SfileMenu);
		fileMenu.add(fileNew			  = createMenuItem( newIcon, SfileNew, FILENEW, newButton));
		fileMenu.add(fileOptions     = createMenuItem( nullIcon, SfileOptions, FILEOPTIONS, dummy));
		fileMenu.addSeparator();
		fileMenu.add(fileExit        = createMenuItem( nullIcon, SfileExit, FILEEXIT, dummy));
		
		viewMenu                     = new JMenu(SviewMenu);
		viewMenu.add(viewDebugWindow = createCheckBoxMenuItem( SviewDebugWindow, VIEWDEBUGWINDOW));
      viewMenu.add(viewToolbar     = createCheckBoxMenuItem( SviewToolbar, VIEWTOOLBAR));

      // set the right state for the CheckBox-Menu-entries 
      viewDebugWindow.setState(false);
      viewToolbar.setState(true);

		menuBar.add(fileMenu);
		menuBar.add(viewMenu);

		EtchedBorder etched = new EtchedBorder();
		fileMenu.setBorder(etched);
		EmptyBorder empty = new EmptyBorder(etched.getBorderInsets(fileMenu));
		fileMenu.setBorder(empty);
		MouseMonitor mm = new MouseMonitor(etched, empty);
		fileMenu.addMouseListener(mm);
		viewMenu.setBorder(empty);
		viewMenu.addMouseListener(mm);
		


      // create the toolbar 
      toolbar = new JToolBar();
      toolbar.setBorderPainted(true);
      toolbar.setFloatable(true);
      toolbar.add(newButton);
	

      // bring it to the screen
      Container container = getContentPane();
      container.add(toolbar, BorderLayout.NORTH);

		JScrollPane sc = new JScrollPane();
      desktop = new JDesktopPane();
      desktop.setOpaque(true);
		sc.getViewport().add(desktop);
      container.add(sc, BorderLayout.CENTER); 
		show();

      setSize(new Dimension(frameWidth, frameHeight));
		setLocation(new Point(framePosX, framePosY));
	}
	

	// this method creates a menu item with a shortcut. 
   protected JMenuItem createMenuItem(String label, char shortcut, final int command) {
		JMenuItem item = new JMenuItem(label, (int) shortcut);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doCommand(command);
			}
		} );

		return item;
	}

	
	// this method creates a menu item with a shortcut and an icon and a button 
	protected JMenuItem createMenuItem(ImageIcon icon,String label, 
                                       final int command, final JFlatButton button) {
		JMenuItem item = new JMenuItem(label, icon);
		item.setHorizontalTextPosition(JButton.RIGHT);
		ActionListener action = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doCommand(command);
			}
		};
		item.addActionListener(action);
		button.addActionListener(action);
		button.setText("");
		button.setToolTipText(label);
		return item;
	}
	
	
	// this method creates a menu item with a shortcut and an icon 
   protected JMenuItem createMenuItem(ImageIcon icon,String label, char shortcut, final int command) {
		JMenuItem item = new JMenuItem(label, icon);
		item.setHorizontalTextPosition(JButton.RIGHT);
		item.setMnemonic(shortcut);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doCommand(command);
			}
		} );

		return item;
	}


	// this method creates a menu item without a shortcut
	protected JMenuItem createMenuItem(String label, final int com) {
		JMenuItem item = new JMenuItem(label);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doCommand(com);
			}
		} );
		return item;
	}


	// this method creates a menu item without a shortcut 
	protected JCheckBoxMenuItem createCheckBoxMenuItem(String label, final int com) {
		JCheckBoxMenuItem item = new JCheckBoxMenuItem(label);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doCommand(com);
			}
		} );
		return item;
	}


	// processes the specified action  
	protected void doCommand(int command) {
		debug.DENTER("Jqmon.doCommand");
		switch (command) {
         case VIEWDEBUGWINDOW : { actionViewDebugWindow(); break; }
         case VIEWTOOLBAR :     { actionViewToolbar();     break; }
			case FILEOPTIONS :     { actionFileOptions();     break; }
			case FILENEW :			  { actionFileNew();         break; }
			case FILEEXIT :        { end();                   break; }
                         
			default : { debug.DPRINTF("Unknown Command."); break; }
		}
		debug.DEXIT();
	}
	

	
	// exits Jqmon
   // the user is asked if he really wants to exit
	// if he agrees the threads are suspended and the global variables and the 
	// memory on the C-side are released
	protected void end() {
		
		int ret = JOptionPane.showConfirmDialog(this, messages.getString("SSure"), messages.getString("SExit"),
					                               JOptionPane.YES_NO_OPTION);
		
		if ( ret == JOptionPane.YES_OPTION) {
			// here the threads are killed and 
			// other cleanup work is done
         
			// the Garbage-Collector is called explicitely 
			// to ensure that the destructor of JDebug is called 
			 
         try {
				saveProps();
				debug = null;
				appProps = null;
				System.gc(); 
            if ( dt != null )  dt.endThread();
            if ( wt != null )  wt.endThread();
				//FIXME
            //dt.join();
            //wt.join();
         } catch (Exception e) {
            System.out.println("FIXME" + e);
         }
			System.exit(0);		
		}
	}


	// create a new CodineView
	protected void actionFileNew() {
		
		debug.DENTER("Jqmon.actionFileNew");
		JCodineWindow codine = new JCodineWindow(wt, debug, updateCalendarList, 
                                                          updateCheckpointList,
                                                          updateComplexList,
                                                          updateHostList,
                                                          updateQueueList); 
      codine.pack();
		codine.setSize(980,850);
      codine.setLocation(100,100);
      codine.show();
		//desktop.add(codine, JLayeredPane.DEFAULT_LAYER);
		//codine.getDesktopPane().getDesktopManager().resizeFrame(codine,0,0,700,500);
		//codine.getDesktopPane().getDesktopManager().activateFrame(codine);
		
		debug.DEXIT();
	}
	
	// displays the options-dialog. 
   // first the latest settings are sent to the dialog.
   // if the program is exited through the OK-Button, the settings are adapted. 
	protected void actionFileOptions() {
		debug.DENTER("Jqmon.actionFileOptions");
		JOptionDialog d = new JOptionDialog(this,messages,debug);
		
		// set the options
		d.setLogFileName(dt.getLogFileName());
		d.setLogDirectory(dt.getLogDirectory());
		d.setLogToStdErr(dt.getLogToStdErr());
		d.setLogToFile(dt.getLogToFile());
		d.setMonitoringLevel(debug.getMonitoringLevel());
		d.show();
		// get the altered options only if the dialog has been exited through th OK-Button
		if ( d.getReturnState() == JOptionDialog.OK ) {
			debug.setMonitoringLevel(d.getMonitoringLevel());
			dt.setLogToStdErr(d.getLogToStdErr());
			dt.setLogToFile(d.getLogToFile());
			dt.setLogFileName(d.getLogFileName());
			dt.setLogDirectory(d.getLogDirectory());
		}
		debug.DEXIT();
	}


	// show or hide the JDebugWindow 
	protected void actionViewDebugWindow() {
		debug.DENTER("Jqmon.actionViewDebugWindow");
		if (viewDebugWindow.getState() == true) {
			// display the DebugWindow 
         debugWindow = new JDebugWindow(dprint);
         debugWindow.setSize(new Dimension(300,200));
               
         InternalFrameListener wl = new InternalFrameAdapter() {
            public void internalFrameClosing(InternalFrameEvent e) {
					debug.DENTER("Jqmon.InternalFrameListener." +
							       "internalFrameClosing");
               // tell the DPrintThread, that there is no window 
               dt.setJDebugWindow((JDebugWindow) null); 
               viewDebugWindow.setState(false);
					debug.DEXIT();
            }
         };
         debugWindow.addInternalFrameListener(wl);
         desktop.add(debugWindow,JLayeredPane.DEFAULT_LAYER);
         dt.setJDebugWindow(debugWindow);
		} else {
         // tell the DPrintThread, that there is no window 
         dt.setJDebugWindow((JDebugWindow) null);
         debugWindow.dispose();
         debugWindow = null;
		}
		debug.DEXIT();
	}

	// show or hide the toolbar
   protected void actionViewToolbar() {
		debug.DENTER("Jqmon.actionViewToolBar");
		if (viewToolbar.getState() == true) {
			toolbar.setVisible(true);
		} else {
			toolbar.setVisible(false);
		}
		debug.DEXIT();
	}


   // *************** 
   // Main program 
   // *************** 
	public static void main(String args[]) {
   
		Jqmon jqmon = new Jqmon(args);
      jqmon.setSize(800, 600);
	}


   //************************************* 
   //load the required libraries 
   //************************************* 
}
