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

import javax.swing.event.*;

import jcodine.*;
import jqmon.events.*;
import java.util.*;


public class JUpdateCalendarList extends Object {

	protected JAnswerList answerList 	  = null;
	protected JCalendarList calendarList	 		  = null;
	protected EventListenerList listeners = null;
	
	public JUpdateCalendarList() {
		listeners = new EventListenerList();
	}
   

   public void deleteCalendar(int index) {
      System.out.println("JUpdateCalendarList.deleteCalendar()");
		fireCalendarDelete(index);
   }

	protected void fireCalendarDelete(int index) {
      System.out.println("ENTER JUpdateCalendarList.fireCalendarDelete()");

      JCalendarListEvent cale = new JCalendarListEvent(this, index);

      Object[] listener = listeners.getListenerList();

      System.out.println(listener.length-2);
      for (int i = listener.length-2; i>=0; i-=2) {
         System.out.println(listener[i].toString());
         System.out.println(JCalendarListListener.class.toString());
         //if (listener[i] == JCalendarListListener.class) {

            JCalendarListListener call = (JCalendarListListener)listener[i+1];
            call.deleteCalendarAvailable(cale);
            System.out.println("JUpdateCalendarList.fireCalendarDelete(index)");
         //}
      }
      System.out.println("EXIT");

   }  

   
   // the host at index from updateCalendarList is update
   public void updateCalendar(int index) {
      System.out.println("JUpdateCalendarList.fireCalendarUpdate()");
      fireCalendarUpdate(index);
   }
 
   protected void fireCalendarUpdate(int index) {
      System.out.println("ENTER JUpdateCalendarList.fireCalendarUpdate()");
 
      JCalendarListEvent cal = new JCalendarListEvent(this, index);
 
      Object[] listener = listeners.getListenerList();
 
      System.out.println(listener.length-2);
      for (int i = listener.length-2; i>=0; i-=2) {
         System.out.println(listener[i].toString());
         System.out.println(JCalendarListListener.class.toString());
         if (listener[i] == JCalendarListListener.class) {
            JCalendarListListener call = (JCalendarListListener)listener[i+1];
            call.updateCalendarAvailable(cal);
            System.out.println("JUpdateCalendarList.fireCalendarUpdate(index)");
                        }
                }
                System.out.println("EXIT");
 
        }
	public void newJCalendarList(JAnswerList al, JCalendarList call) {
		answerList = al;
		calendarList  = call;
		fireChange();
	}
		
	public JCalendarList getCalendarList() throws JAnswerListException {
		Enumeration e = answerList.elements();
		while ( e.hasMoreElements() ) {
			JAnswerObject a = (JAnswerObject)e.nextElement();
			// if there 's something wrong with the object throw an exception
			if ( a.getStatus() != 1 ) {
				throw new JAnswerListException(a);
			}
		}
		return calendarList;
	}

	public JAnswerList getAnswerList() {
		return answerList;
	}

	public void fireChange() {
		// build Event 
		JCalendarListEvent h = new JCalendarListEvent(this);

		// check all listeners
		Object[] listener = listeners.getListenerList();
		
		for (int i = listener.length-2; i>=0; i-=2) {
			if (listener[i] == JCalendarListListener.class) {
				JCalendarListListener hl = (JCalendarListListener)listener[i+1];
				hl.newJCalendarListAvailable(h);
			}
		}
	}

	public void addJCalendarListListener(JCalendarListListener x) {
		listeners.add(x.getClass(), x);
		x.newJCalendarListAvailable(new JCalendarListEvent(this));
	}
	
	public void removeChangeListener(ChangeListener x) {
		listeners.remove(JCalendarListListener.class, x);
	}
				
}
