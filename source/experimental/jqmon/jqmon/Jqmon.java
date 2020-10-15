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

/* Swing 1.0.x 
import com.sun.java.swing.*;
import com.sun.java.swing.event.*;
import com.sun.java.swing.border.*;
 */
/* Swing 1.1 */
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import codine.*;
import jqmon.*;
import jqmon.dialogs.*;
import jqmon.util.*;
import jqmon.debug.*;
//import jqmon.views.*;

/**
 * This class represents the Jqmon. It initializes the GUI and
 * the Workerthreads.
 *
 * @author    Michael Roehrl
 * @version   0.01
 *
 */
public class Jqmon extends JFrame {

	/*
    * Alle Eigenschaften dieser Klasse sind als
	 * ´static´ deklariert, da von dieser Klasse
	 * nur eine Instanz erzeugt wird.
	 *
	 */

	
	/* Menu-Eintraege */
	private static String SfileMenu;
	private static String SfileNew;
	private static String SfileOptions;
	private static String SfileExit;

	private static String SviewMenu;
	private static String SviewDebugWindow;
   private static String SviewToolbar;


	
	/* Haeufig benoetigte Strings */
	private static String SYes;
	private static String SNo;
	private static String SCancel;
	private static String SOkay;
  
	/* Konstanten fuer die Actions (Menus, Buttons, ...) */
	private final static int ACTIONS			  = 100;
	
	/* Konstanten fuer das FILE-Menu */
	private final static int FILEEXIT        = ACTIONS + 100;
	private final static int FILEOPTIONS     = ACTIONS + 101;
	private final static int FILENEW         = ACTIONS + 102;
	
	/* Konstanten fuer das VIEW-Menu */
	private final static int VIEWDEBUGWINDOW = ACTIONS + 200;
   private final static int VIEWTOOLBAR     = ACTIONS + 201;
   
   
	/* Menus, MenuItems, CheckBoxMenuItems, ... */
	protected static JMenuBar menuBar                  = null;	
	protected static JMenu fileMenu                    = null;
	protected static JMenu viewMenu                    = null;
	protected static JMenuItem fileNew						= null;
	protected static JMenuItem fileOptions             = null;
	protected static JMenuItem fileExit                = null;
   protected static JCheckBoxMenuItem viewDebugWindow = null;
   protected static JCheckBoxMenuItem viewToolbar     = null;
   
	/* Die Toolbar */
   protected static JToolBar toolbar = null;
   
	/* Das DebugFenster */
   protected static JDebugWindow debugWindow = null;	
   
	/* Das MDI-Panel */
   protected  static JLayeredPane desktop;
   
	/* Das Model fuer den Debug-String */
   protected static JDPrintStringModel dprint = null;

	/* Das Model fuer den QueueListUpdate */
	protected static JUpdateQueueList updateQueueList = null;
	
	/* Das Model fuer den HostListUpdate */
	protected static JUpdateHostList updateHostList = null;

	/* Der WorkerThread */
   protected static JWorkerThread wt          = null;
   
	/* Der Print-Thread fuer Debug-Ausgabenm */
	protected static JDPrintThread dt          = null;

	/* Die Debug-Klasse */
	protected static JDebug debug              = null;

   /* Icons & Buttons */
   protected static ImageIcon nullIcon    = null;
   protected static ImageIcon newIcon     = null;
   protected static JFlatButton newButton = null;

	/* Die Locale */
	protected static Locale locale           = null;
	protected static ResourceBundle messages = null;
   
	
	/* Die Properties */
	protected static Properties appProps     = null;


	/***************************************************************/
	/* Alle ueber die Properties-Datei einstellbaren Eigenschaften */
	/***************************************************************/

	/* Die Position des Hauptfensters */
	protected static int framePosX;
	protected static int framePosY;
	protected static int frameHeight;
	protected static int frameWidth;

	/** 
	 * Der Konstruktur erzeugt ein Jqmon-Objekt. Das heisst, er
	 *	initialisiert die GUI und faehrt die Threads hoch.
	 *
	 */
	public Jqmon() {
		super("JQMON");
	
		/* Properties laden */
		loadProps();
		
		locale   = Locale.getDefault();
		messages = ResourceBundle.getBundle("MessageBundle",locale);

		
		/* Haeufig benoetige Strings laden */
		SYes    = messages.getString("SYes");
		SNo     = messages.getString("SNo");
		SCancel = messages.getString("SCancel");
		SOkay   = messages.getString("SOkay");
		

		/* GUI initialisieren */
		initGUI();
		
		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
      dprint 				= new JDPrintStringModel();
		updateQueueList 	= new JUpdateQueueList();
		updateHostList		= new JUpdateHostList();

		JBuffer buffer 	= new JBuffer();
		
		/* Erzeugen der Threads */
		dt = new JDPrintThread(dprint,buffer);

		/* Properties fuer den DPrintThread holen und setzen */
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

		/* Threads starten */
		dt.start();

		/* Debug-Instanz erzeugen */
		debug = new JDebug(dt,buffer, appProps);


		/* MonitoringLevel laden */
		long ml[] = new long[JDebug.N_LAYER];
		for (int i = 0; i < JDebug.N_LAYER; i++ ) {
			ml[i] = 0;
		}
		
		/* TOP_LAYER - Einstellungen */
		s = appProps.getProperty("TOP_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.TOP_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("TOP_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.TOP_LAYER] += JDebug.INFOPRINT;
		
		/* CULL_LAYER - Einstellungen */
		s = appProps.getProperty("CULL_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.CULL_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("CULL_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.CULL_LAYER] += JDebug.INFOPRINT;

		/* BASIS_LAYER - Einstellungen */
		s = appProps.getProperty("BASIS_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.BASIS_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("BASIS_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.BASIS_LAYER] += JDebug.INFOPRINT;

		/* GUI_LAYER - Einstellungen */
		s = appProps.getProperty("GUI_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.GUI_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("GUI_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.GUI_LAYER] += JDebug.INFOPRINT;

		/* SCHED_LAYER - Einstellungen */
		s = appProps.getProperty("SCHED_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.SCHED_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("SCHED_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.SCHED_LAYER] += JDebug.INFOPRINT;

		/* COMMD_LAYER - Einstellungen */
		s = appProps.getProperty("COMMD_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.COMMD_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("COMMD_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.COMMD_LAYER] += JDebug.INFOPRINT;

		/* API_LAYER - Einstellungen */
		s = appProps.getProperty("API_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.API_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("API_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.API_LAYER] += JDebug.INFOPRINT;
		
		/* PACK_LAYER - Einstellungen */
		s = appProps.getProperty("PACK_LAYER.trace", "false");
		if ( s.equals("true") ) ml[JDebug.PACK_LAYER] += JDebug.TRACE;
		s = appProps.getProperty("PACK_LAYER.infoprint", "false");
		if ( s.equals("true") ) ml[JDebug.PACK_LAYER] += JDebug.INFOPRINT;


		debug.setMonitoringLevel(ml);

		JCodObj o = new JCodObj(debug);
      wt = new JWorkerThread(debug, updateQueueList, updateHostList);
		wt.setPriority(Thread.MIN_PRIORITY);
		wt.start();
		o.getID();
	}


	/** Laden der Properties */
	protected void loadProps() {
		try {
			/* DefaultProperties laden */
			appProps           = new Properties();
			FileInputStream in = new FileInputStream("Jqmon.properties");
			appProps.load(in);
			in.close();
		} catch (IOException e) {
			//FIXME
			System.err.println("Couldn´t load Jqmon.properties.");
			System.err.println("Using Defaults.");
		}

		
		/* Auslesen der benoetigten Properties */
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
			/* Die neuen Einstellungen in die Properties bringen */
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
			appProps.save(fout,"---- Properties for Jqmon ----");
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


	/** Inits the Graphical User Interface */
	protected void initGUI() {

		/* Das Look and Feel setzen */
		/*
		try {
			UIManager.setLookAndFeel("com.sun.java.swing.plaf.motif.MotifLookAndFeel");
		} catch (Exception e) {
			System.out.println("Look and Feel not found!");
		}
		*/
		/* Exit an das Fenster binden */
		WindowListener wl = new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				end();
			}
		
		};
		addWindowListener(wl);
		
		
		/* Die Menu-Strings aus dem ResourceBundle holen */
		SfileMenu          = messages.getString("fileMenuKey");
		SfileNew				 = messages.getString("fileNewKey");
		SfileOptions       = messages.getString("fileOptionsKey");
		SfileExit          = messages.getString("fileExitKey");

		SviewMenu          = messages.getString("viewMenuKey");
		SviewDebugWindow   = messages.getString("viewDebugWindowKey");
		SviewToolbar       = messages.getString("viewToolBarKey");

		
		/* Die Bilder laden und die Buttons draus erzeugen */
		
      nullIcon = new ImageIcon("images/blank-2ß.gif");      
      newIcon  = new ImageIcon("images/New.gif");
      newButton = new JFlatButton(newIcon);

		JFlatButton dummy = new JFlatButton(nullIcon);

      
      /* Menus erzeugen */
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

      /* Die CheckBox-Menueintraege auf den richtigen Status bringen */
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
		


      /* Die Toolbar erzeugen */ 
      toolbar = new JToolBar();
      toolbar.setBorderPainted(true);
      toolbar.setFloatable(true);
      toolbar.add(newButton);
	

      /* Alles auf den Bildschirm bringen */
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
	

	/** This method creates a MenuItem with a shortcut. */
	protected JMenuItem createMenuItem(String label, char shortcut, final int command) {
		JMenuItem item = new JMenuItem(label, (int) shortcut);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doCommand(command);
			}
		} );

		return item;
	}

	
	/** This method creates a MenuItem with a shortcut and an Icon and a Button. */
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
	
	
	/** This method creates a MenuItem with a shortcut AND an Icon. */
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


	/** This method creates a MenuItem without a shortcut. */
	protected JMenuItem createMenuItem(String label, final int com) {
		JMenuItem item = new JMenuItem(label);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doCommand(com);
			}
		} );
		return item;
	}


	/** This method creates a MenuItem without a shortcut. */
	protected JCheckBoxMenuItem createCheckBoxMenuItem(String label, final int com) {
		JCheckBoxMenuItem item = new JCheckBoxMenuItem(label);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doCommand(com);
			}
		} );
		return item;
	}


	/** Fuehrt die angegebene Aktion aus. */
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
	

	/**
	 * Beendet den Jqmon. Der Benutzer wird gefragt, ob er sich bewusst ist, was er
	 * macht. Sollte er wirklich wollen, so werden die Threads beendet und die globalen
	 * Variablen und der belegte Speicher auf der C-Seite freigegeben.
	 *
	 */
	protected void end() {
		
		int ret = JOptionPane.showConfirmDialog(this, messages.getString("SSure"), messages.getString("SExit"),
					                               JOptionPane.YES_NO_OPTION);
		
		if ( ret == JOptionPane.YES_OPTION) {
			/*
			 * Hier werden die Threads beendet und sonstige
			 * Aufraeumarbeiten geleistet.
			 *
			 * Der Garbage-Collector wird explizit aufgerufen,
			 * damit der Destruktor von JDebug auf alle Faelle
			 * aufgerufen wird.
			 *
			 */
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


	/**
	  * Erzeugt ein neues CodineView.
	  */
	protected void actionFileNew() {
		
		debug.DENTER("Jqmon.actionFileNew");
		JCodineWindow codine = new JCodineWindow(wt, debug, updateQueueList, 
															  updateHostList);
      codine.pack();
		codine.show();
		
		//desktop.add(codine, JLayeredPane.DEFAULT_LAYER);
		//codine.getDesktopPane().getDesktopManager().resizeFrame(codine,0,0,700,500);
		//codine.getDesktopPane().getDesktopManager().activateFrame(codine);
		
		debug.DEXIT();
	}
	
	/**
	  * Zeigt den OptionenDialog an. Zuerst werden die aktuellen Einstellungen
	  * an den Dialog geschickt, dieser dann angezeigt und beim Verlassen
	  * ueber den OK-Button werden die Einstellungen uebernommen.
	  */
	protected void actionFileOptions() {
		debug.DENTER("Jqmon.actionFileOptions");
		JOptionDialog d = new JOptionDialog(this,messages,debug);
		
		/* Setzen der Optionen */
		d.setLogFileName(dt.getLogFileName());
		d.setLogDirectory(dt.getLogDirectory());
		d.setLogToStdErr(dt.getLogToStdErr());
		d.setLogToFile(dt.getLogToFile());
		d.setMonitoringLevel(debug.getMonitoringLevel());
		d.show();
		/* Veraenderte Optionen nur holen, wenn der Dialog ueber OK verlassen wurde */
		if ( d.getReturnState() == JOptionDialog.OK ) {
			debug.setMonitoringLevel(d.getMonitoringLevel());
			dt.setLogToStdErr(d.getLogToStdErr());
			dt.setLogToFile(d.getLogToFile());
			dt.setLogFileName(d.getLogFileName());
			dt.setLogDirectory(d.getLogDirectory());
		}
		debug.DEXIT();
	}


	/** Show or hide the JDebugWindow */
	protected void actionViewDebugWindow() {
		debug.DENTER("Jqmon.actionViewDebugWindow");
		if (viewDebugWindow.getState() == true) {
			/* Das DebugWindow darstellen */
         debugWindow = new JDebugWindow(dprint);
         debugWindow.setSize(new Dimension(300,200));
               
         InternalFrameListener wl = new InternalFrameAdapter() {
            public void internalFrameClosing(InternalFrameEvent e) {
					debug.DENTER("Jqmon.InternalFrameListener." +
							       "internalFrameClosing");
					/* Dem DPrintThread mitteilen, dass kein Fenster da ist */
               dt.setJDebugWindow((JDebugWindow) null); 
               viewDebugWindow.setState(false);
					debug.DEXIT();
            }
         };
         debugWindow.addInternalFrameListener(wl);
         desktop.add(debugWindow,JLayeredPane.DEFAULT_LAYER);
         dt.setJDebugWindow(debugWindow);
		} else {
         /* Dem DPrintThread mitteilen, dass kein Fenster da ist. */
         dt.setJDebugWindow((JDebugWindow) null);
         debugWindow.dispose();
         debugWindow = null;
		}
		debug.DEXIT();
	}

	/** Show or hide the Toolbar */
	protected void actionViewToolbar() {
		debug.DENTER("Jqmon.actionViewToolBar");
		if (viewToolbar.getState() == true) {
			toolbar.setVisible(true);
		} else {
			toolbar.setVisible(false);
		}
		debug.DEXIT();
	}


   /* ************* */
   /* Hauptprogramm */
   /* ************* */
	public static void main(String args[]) {
   
		Jqmon jqmon = new Jqmon();
	}


   /* ************************************* */
   /* Laden der erforderlichen Bibliotheken */
   /* ************************************* */
   static {
		String s = "no one";
      try {
			s = "Jqmon";			
         System.loadLibrary("jqmon");
      }
      catch (UnsatisfiedLinkError e) {
         System.out.println("ERROR: Couldn't load library: " + s);
         System.exit(0);
      }
      catch (SecurityException e) {
         System.out.println("ERROR: You are not allowed to load library: " + s);
         System.exit(0);
      }
      
   }
}
