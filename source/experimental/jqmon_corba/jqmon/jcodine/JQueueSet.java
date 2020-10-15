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
package jcodine;


public class JQueueSet extends JCodSet {

	protected JQueueList queues;
	
	public JQueueSet() {
		super();
		queues = new JQueueList();
	}

	// returns the first queue
	public JQueue getFirst() {
		return (JQueue)queues.firstElement();
	}

	// add a queue
	public void add(JQueue q) {
		//queues.add(q);
		queues.addElement(q);
		RecalcAmbiguous();
	}

	// clear the complete set
	public void clear() {
		//queues.clear();
		queues.removeAllElements();
	}

	// check if set is empty
	public boolean isEmpty() {
		return queues.isEmpty();
	}


	// recalculates the ambiguoity among the fields
	protected void RecalcAmbiguous() {
		// TODO: implement the function
	}
}
