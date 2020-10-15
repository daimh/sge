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


// this class passes the debug output to the debug thread.
// the methods can also be invoked from C-side

public class JDebug {


	
	// the layers for the debugging 
	public static final int N_LAYER	   = 8; // number of Layers 	
	public static final int TOP_LAYER	= 0; // t 
	public static final int CULL_LAYER	= 1; // c 
	public static final int BASIS_LAYER	= 2; // b 
	public static final int GUI_LAYER	= 3; // g 
	public static final int SCHED_LAYER	= 4; // s 
	public static final int COMMD_LAYER	= 5; // h 
	public static final int API_LAYER	= 6; // a 
	public static final int PACK_LAYER	= 7; // p 

	
	// various classes for the debug messaging
	public static final long TRACE	   = 1; // t
	public static final long INFOPRINT 	= 2; // i 

	// the JDPrintThread attached to Jqmon
	protected JDPrintThread dt = null;

	// the buffer, the DPrintMessages are written to
	protected JBuffer buffer = null;
	
	// the layerstack 
	protected Stack guiLayerStack;

	//  the monitoring levels and their classes
	protected long monitoring_level[];

	// the property File
	protected Properties appProps;

	// this constructor creates an instance of JDebug with the specified JDPrintThread.
	// param d = the JDPrintThread attached to Jqmon
	public JDebug(JDPrintThread d, JBuffer b, Properties props) {
		dt            = d;
		buffer        = b;
		appProps      = props;
		guiLayerStack = new Stack();
		guiLayerStack.push(new Integer(TOP_LAYER));
		
		// WE DON'T USE JNI
		// init(dt);
		

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


	// Forwards a message to DPrintThread 
	public void PrintMsg(String s, int Layer, int Class) {
		//TODO: save layer and class for the thread!!
		Print(s);
	}

	// Forwards a message to DPrintThread 
	protected void PrintCMsg(String s, int Layer, int Class) {
		//TODO: save layer and class for the thread!!
		
		// check, what class is active 
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

	// ********************* 
	// ** Native Methods  ** 
	// ********************* 
	
	// this function has to be called before processing the 
   // other native methods
   // it represents a sort of constructor
	protected native void init(JDPrintThread d);
	
	// this function has to be called before the destruction of the class
   // it represents a kind of destructor
	protected native void exit();


	// get the TOP_LAYER constant
	protected native int getTOP_LAYER();

	// get the N_LAYER constant
	protected native int getN_LAYER();

	public native void Test();

	// destructor
	protected void finalize() { 
      //exit();
	}
}
