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

import codine.*;
import jqmon.events.*;
import jqmon.debug.*;

/**
 * Diese Klasse repreasentiert das CodineDokument.
 *
 * @author Michael Roehrl
 */
public class JCodDocument {

		/** Der WorkerThread */
		private JWorkerThread wt = null;

		/** Debug */
		private JDebug debug = null;

		/** Das Model fuer den QueueListUpdate */
		private JUpdateQueueList updateQueueList = null;
		
		/** Das Model fuer den HostListUpdate */
		private JUpdateHostList updateHostList = null;
		
		/** Eine neue *List verfuegbar? */
		private boolean newQueueList = false;
		private boolean newHostList  = false;
		private boolean newJobList   = false;
		
		/**
		  * Constructor
		  */
		public JCodDocument(JDebug d, JWorkerThread w) { 
			super();
			wt = w;
			debug = d;
			updateQueueList = new JUpdateQueueList();
			updateHostList = new JUpdateHostList();
			wt.setJUpdateQueueList(updateQueueList);
			wt.setJUpdateHostList(updateHostList);
		}

		/**
		 * Liefert die aktuelle JUpdateQueueList zurueck
		 */
		public JUpdateQueueList getJUpdateQueueList() {
			return updateQueueList;
		}

		
		/**
		 * Liefert die aktuelle JUpdateHostList zurueck
		 */
		public JUpdateHostList getJUpdateHostList() {
			return updateHostList;
		}
		

		/**
		 * Holt die alle aktuellen Daten vom Codinesystem 
		 */
		public void update() {
			updateQueues();
			updateHosts();
			updateJobs();
		}


		/**
		 * Holt die neuen Queues vom Codinesystem 
		 */
		public void updateQueues() {
			wt.updateQueues();
			newQueueList = true;
		}

		
		/** 
		 * Holt die neuen Hosts vom Codinesystem 
		 */
		public void updateHosts() {
			wt.updateHosts();
			newHostList = true;
		}

		
		/**
		 * Holt die neuen Jobs vom Codinesystem 
		 */
		public void updateJobs() {
		//	wt.updateJobs();
		//	newJobList = true;
		}
	


		/**
		 * Neue QueueList verfuegbar? 
		 */
		public boolean isNewQueueListAvailable() {
			return newQueueList;
		}

		
		/** 
	    * Neue HostList verfuegbar? 
		 */
		public boolean isNewHostListAvailable() {
			return newHostList;
		}
		
		
		/** 
		 * Neue JobList verfuegbar? 
		 */
		public boolean isNewJobListAvailable() {
			return newJobList;
		}

}
