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

import java.lang.*;
import jqmon.debug.*;
import jqmon.events.*;
import codine.*;

/**
   Diese Klasse repraesentiert den JWorkerThread, das Arbeitstier.
   Er schickt DPrint-Meldungen an den angeschlossenen JDPrintThread,
   falls einer angeschlossen ist.
	
	@author  Michael Roehrl
	@version 0,01

*/

public class JWorkerThread extends Thread {

	/** Soll der Thread beendet werden ? */
	protected boolean end = false;

	/** Das globale JDebug-Objekt */
	protected JDebug debug = null;

	/** Die JAnswerList */
	protected JAnswerList answerList = null;
	
	/** Die Queue-Liste */
	protected JQueueList queueList = null;

	/** Die Host-Liste */
	protected JHostList hostList = null;

	/** Das UpdateQueueList Model */
	protected JUpdateQueueList updateQueueList = null;
	
	/** Das UpdateHostList Model */
	protected JUpdateHostList updateHostList = null;
	
	
   /** Default-Konstruktor. Legt einen JWorkerThread ohne JDPrintThread an.
       Im Normalfall sollte ein JDPrintThread mitangegeben werden.
    */
   public JWorkerThread(JDebug d, JUpdateQueueList q, JUpdateHostList h) {
      debug 	  		 = d;
		updateQueueList = q;
		updateHostList  = h;
		answerList 		 = new JAnswerList(10,10);
		queueList  		 = new JQueueList(10,10);
		hostList	  = new JHostList(10,10);
   }
   

   /** Haelt den Thread am Laufen. Er befindet sich in einer Endlosschleife
       und kann nur doch "stop" angehalten werden.
    */
	public void run() {
		contact();
      try {
         while( !end ) {
            work();
            sleep(1000);
         }
      } 
      catch (InterruptedException e) {
      }
      catch (ThreadDeath td) {
			cleanup();
         throw td; // Muss unbedingt nochmal geworfen werden!!
      }

		cleanup();
	}


   /** Macht die ganze Arbeit. Wird von run() aufgerufen, weil
       sich nur so synchronisiert arbeiten laesst. 
    */
   protected synchronized void work() {
		
   }


   /** Setzen des JDPrintThread. Sollte dieser Null sein, so werden keine JDPrintStrings
       verarbeitet.
    */

	public synchronized void endThread() {
		end = true;
	}

	protected void cleanup() {
	}

	protected synchronized void contact() {
		debug.DENTER("JWorderThread.contact");
		contactC();
		updateQueueList.newJQueueList(answerList, queueList);
		updateHostList.newJHostList(answerList, hostList);
		debug.DEXIT();
	}

	private native void contactC();
	private native void getQueue();
}
