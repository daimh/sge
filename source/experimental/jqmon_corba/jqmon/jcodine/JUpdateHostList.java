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

/* Swing 1.0.x 
import com.sun.java.swing.event.*;
*/

/* Swing 1.1 */
import javax.swing.event.*;

import jcodine.*;
import jqmon.events.*;
import java.util.*;

public class JUpdateHostList extends Object {

	protected JAnswerList answerList 	  = null;
	protected JHostList hostList	 		  = null;
	protected EventListenerList listeners = null;
	
	public JUpdateHostList() {
		listeners = new EventListenerList();
	}
   
   // the host at index from updateHostList is update
   public void updateHost(int index) {
      System.out.println("JUpdateHostList.fireHostUpdate()");
      fireHostUpdate(index);
   }
 
   protected void fireHostUpdate(int index) {
      System.out.println("ENTER JUpdateHostList.fireHostUpdate()");
 
      JHostListEvent h = new JHostListEvent(this, index);
 
      Object[] listener = listeners.getListenerList();
 
      System.out.println(listener.length-2);
      for (int i = listener.length-2; i>=0; i-=2) {
         System.out.println(listener[i].toString());
         System.out.println(JHostListListener.class.toString());
         if (listener[i] == JHostListListener.class) {
            JHostListListener hl = (JHostListListener)listener[i+1];
            hl.updateHostAvailable(h);
            System.out.println("JUpdateHostList.fireHostUpdate(index)");
         }
      }
      System.out.println("EXIT");
 
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
         // if there is something wrong with the object an exception is thrown 
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
      // create the event 
		JHostListEvent h = new JHostListEvent(this);

      // perform all listeners
		Object[] listener = listeners.getListenerList();
		
		for (int i = listener.length-2; i>=0; i-=2) {
			if (listener[i] == JHostListListener.class) {
				JHostListListener hl = (JHostListListener)listener[i+1];
				hl.newJHostListAvailable(h);
			}
		}
	}

	public void addJHostListListener(JHostListListener x) {
		listeners.add(x.getClass(), x);
      // just send it 
		x.newJHostListAvailable(new JHostListEvent(this));
	}
	
	public void removeChangeListener(ChangeListener x) {
		listeners.remove(JHostListListener.class, x);
	}
				
}
