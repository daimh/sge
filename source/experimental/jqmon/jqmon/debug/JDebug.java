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
package jqmon.debug;

import java.util.*;

/** 
  Diese Klasse uebernimmt die Debugausgaben an den JDebugThread. Die Methoden werden auch
  von C heraus aufgerufen.
  @author Michael Roehrl
  @version 0.01
  */
public class JDebug {


	
	/* Die Layers fuer das Debugging */
	public static final int N_LAYER	   = 8; /* Anzahl der Layers */
	
	public static final int TOP_LAYER	= 0; /* t */
	public static final int CULL_LAYER	= 1; /* c */
	public static final int BASIS_LAYER	= 2; /* b */
	public static final int GUI_LAYER	= 3; /* g */
	public static final int SCHED_LAYER	= 4; /* s */
	public static final int COMMD_LAYER	= 5; /* h */
	public static final int API_LAYER	= 6; /* a */
	public static final int PACK_LAYER	= 7; /* p */

	
	/* Verschiedene Classes der Debugging-Messages */
	public static final long TRACE	   = 1; /* t */
	public static final long INFOPRINT 	= 2; /* i */

	/** Der an Jqmon angeschlossene JDPrintThread */
	protected JDPrintThread dt = null;

	/** Der Buffer, in den die DPrintMessages geschrieben werden */
	protected JBuffer buffer = null;
	
	/** Der Layerstack */
	protected Stack guiLayerStack;

	/** Die Monitoring Levels und deren Klassen */
	protected long monitoring_level[];

	/** Das Property-File */
	protected Properties appProps;

	/**
	  Dieser Konstruktor erzeugt ein JDebug-Objekt mit dem angegebenen
	  JDPrintThread.
	  @param d = Der an Jqmon angeschlossene JDPrintThread
	*/
	public JDebug(JDPrintThread d, JBuffer b, Properties props) {
		dt            = d;
		buffer        = b;
		appProps      = props;
		guiLayerStack = new Stack();
		guiLayerStack.push(new Integer(TOP_LAYER));
		init(dt);
		monitoring_level = new long[N_LAYER];
		for (int i = TOP_LAYER; i < N_LAYER; i++) {
			monitoring_level[i] = 0;
		}
	}


	public void setMonitoringLevel(long m[]) {
		if (m.length != 8) {
			System.out.println("JDebug.java: Laenge nicht 8");
			return;
		}

		for (int i = 0; i < 8; i++) {
			monitoring_level[i] = m[i];
			//System.out.println("Level: " + i + "    Class: " + monitoring_level[i]);
		}
	}

	public long[] getMonitoringLevel() {
		return monitoring_level;
	}


	/** Gibt dem DPrintThread eine Message weiter */
	public void PrintMsg(String s, int Layer, int Class) {
		//TODO: Layer und Class speichern fuer den Thread!!
		Print(s);
	}

	/** Gibt dem DPrintThread eine Message weiter */
	protected void PrintCMsg(String s, int Layer, int Class) {
		//TODO: Layer und Class speichern fuer den Thread!!
		
		/* Testen, ob fuer diesen diese Class eingeschaltet ist */
		for (int i = 0; i < N_LAYER; i++) {
			if ( Layer == i ) {
				if ( Class == TRACE ) {
					if ( (monitoring_level[Layer] == Class) ||
							monitoring_level[Layer] == 3) {
						Print(s);
					}
				} else if ( Class == INFOPRINT ) {
					if ( (monitoring_level[Layer] == Class) ||
							monitoring_level[Layer] == 3) {
						Print(s);
					}
				}  
			}
		}
	}
				
				
	protected void Print(String s) {
		buffer.put(s);
	}


	public void DENTER(String func) {
		guiLayerStack.push(func);
		PrintCMsg("Enter " + func, GUI_LAYER, (int)TRACE);
	}


	public void DEXIT() {
		String func = (String) guiLayerStack.pop();
		PrintCMsg("Exit " + func, GUI_LAYER, (int)TRACE);
	}


	public void DPRINTF(String s) {
		PrintCMsg(s, GUI_LAYER, (int)INFOPRINT);
	}

	/* ********************* */
	/* ** Native Methoden ** */
	/* ****************** ** */
	
	/** Diese Funktion muss vor allen anderen NativeFunktionen
	    ausgefuehrt werden. Sie stellt eine Art Konstruktor dar.
		 @param dt = Der an Jqmon angeschlossene JDPrintThread
	*/
	protected native void init(JDPrintThread d);
	
	/** Diese Funktion muss vor dem Zerstoeren der Klasse auf-
	    gerufen werden. Sie stellt eine Art Destruktor dar.
	*/
	protected native void exit();


	/** Holt die TOP_LAYER Konstante */
	protected native int getTOP_LAYER();

	/** Holt die N_LAYER Konstante */
	protected native int getN_LAYER();

	public native void Test();

	/** Mein erster Destruktor in Java!!!! */
	protected void finalize() { 
//		exit();
	}
}
