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

public class JUpdateHostList extends Object {

	protected JAnswerList answerList 	  = null;
	protected JHostList hostList	 		  = null;
	protected EventListenerList listeners = null;
	
	public JUpdateHostList() {
		listeners = new EventListenerList();
	}

	public void newJHostList(JAnswerList al, JHostList q) {
		answerList = al;
		hostList  = q;
		fireChange();
	}
		
	public JHostList getHostList() throws JAnswerListException {
		Enumeration e = answerList.elements();
		while ( e.hasMoreElements() ) {
			JAnswerObject a = (JAnswerObject)e.nextElement();
			/* Sollte das Objekt nicht OK sein, wird eine Exception geworfen. */
			if ( a.getStatus() != 1 ) {
				throw new JAnswerListException(a);
			}
		}
		return hostList;
	}

	public JAnswerList getAnswerList() {
		return answerList;
	}

	public void fireChange() {
		/* Das Event erzeugen */
		JHostListEvent c = new JHostListEvent(this);

		/* Alle Listener bearbeiten*/
		Object[] listener = listeners.getListenerList();
		
		for (int i = listener.length-2; i>=0; i-=2) {
			if (listener[i] == JHostListListener.class) {
				JHostListListener cl = (JHostListListener)listener[i+1];
				cl.newJHostListAvailable(c);
			}
		}
	}

	public void addJHostListListener(JHostListListener x) {
		listeners.add(x.getClass(), x);
		/* Gleich mal benachrichtigen */
		x.newJHostListAvailable(new JHostListEvent(this));
	}
	
	public void removeChangeListener(ChangeListener x) {
		listeners.remove(JHostListListener.class, x);
	}
				
}
