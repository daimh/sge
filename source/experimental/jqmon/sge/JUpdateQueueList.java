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
package codine;

/* Swing 1.0.x 
import com.sun.java.swing.event.*;
*/

/* Swing 1.1 */
import javax.swing.event.*;

import codine.*;
import jqmon.events.*;
import java.util.*;

public class JUpdateQueueList extends Object {

	protected JAnswerList answerList 	  = null;
	protected JQueueList queueList 		  = null;
	protected EventListenerList listeners = null;
	
	public JUpdateQueueList() {
		listeners = new EventListenerList();
	}

	public void newJQueueList(JAnswerList al, JQueueList q) {
		answerList = al;
		queueList  = q;
		fireChange();
	}
		
	public JQueueList getQueueList() throws JAnswerListException {
		Enumeration e = answerList.elements();
		while ( e.hasMoreElements() ) {
			JAnswerObject a = (JAnswerObject)e.nextElement();
			/* Sollte das Objekt nicht OK sein, wird eine Exception geworfen. */
			if ( a.getStatus() != 1 ) {
				throw new JAnswerListException(a);
			}
		}
		return queueList;
	}

	public JAnswerList getAnswerList() {
		return answerList;
	}

	public void fireChange() {
		/* Das Event erzeugen */
		JQueueListEvent c = new JQueueListEvent(this);

		/* Alle Listener bearbeiten*/
		Object[] listener = listeners.getListenerList();
		
		for (int i = listener.length-2; i>=0; i-=2) {
			if (listener[i] == JQueueListListener.class) {
				JQueueListListener cl = (JQueueListListener)listener[i+1];
				cl.newJQueueListAvailable(c);
			}
		}
	}

	public void addJQueueListListener(JQueueListListener x) {
		listeners.add(x.getClass(), x);
		/* Gleich mal benachrichtigen */
		x.newJQueueListAvailable(new JQueueListEvent(this));
	}
	
	public void removeChangeListener(ChangeListener x) {
		listeners.remove(JQueueListListener.class, x);
	}
				
}
