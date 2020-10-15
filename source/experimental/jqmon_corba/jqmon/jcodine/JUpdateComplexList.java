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

 
public class JUpdateComplexList extends Object {
 
   protected JAnswerList answerList          = null;
   protected JComplexList complexList          = null;
   protected EventListenerList listeners = null;
 
   public JUpdateComplexList() {
      listeners = new EventListenerList();
   }
 
   // the complex at index from updateComplexList is updated
   public void updateComplex(int index) {
      System.out.println("JUpdateComplexList.fireComplexUpdate()");
      fireComplexUpdate(index);
   }
 
   protected void fireComplexUpdate(int index) {
      System.out.println("ENTER JUpdateComplexList.fireComplexUpdate()");
 
      JComplexListEvent c = new JComplexListEvent(this, index);
      Object[] listener = listeners.getListenerList();
 
      System.out.println(listener.length-2);
      for (int i = listener.length-2; i>=0; i-=2) {
         System.out.println(listener[i].toString());
         System.out.println(JComplexListListener.class.toString());
         if (listener[i] == JComplexListListener.class) {
            JComplexListListener cl = (JComplexListListener)listener[i+1];
            cl.updateComplexAvailable(c);
            System.out.println("JUpdateComplexList.fireComplexUpdate(index)");
         }
      }
      System.out.println("EXIT");
 
   }
  
   public void newJComplexList(JAnswerList al, JComplexList cxl) {
      answerList = al;
      complexList  = cxl;
      fireChange();
   }
 
   public JComplexList getComplexList() throws JAnswerListException {
      Enumeration e = answerList.elements();
      while ( e.hasMoreElements() ) {
         JAnswerObject a = (JAnswerObject)e.nextElement();
         // if there is something wrong with the object an exception is thrown 
         if ( a.getStatus() != 1 ) {
            throw new JAnswerListException(a);
         }
      }
      return complexList;
   }
 
   public JAnswerList getAnswerList() {
      return answerList;
   }
 
   public void fireChange() {
      // create the event 
      JComplexListEvent c = new JComplexListEvent(this);
 
      // perform all listeners
      Object[] listener = listeners.getListenerList();
      
      for (int i = listener.length-2; i>=0; i-=2) {
         System.out.println(listener[i].toString());
         System.out.println(JComplexListListener.class.toString());
         if (listener[i] == JComplexListListener.class) {
            JComplexListListener cl = (JComplexListListener)listener[i+1];
            cl.newJComplexListAvailable(c);
         }
      }
   }
 
   public void addJComplexListListener(JComplexListListener x) {
      listeners.add(JComplexListListener.class, x);
      // just send it 
      x.newJComplexListAvailable(new JComplexListEvent(this));
   }
 
   public void removeChangeListener(ChangeListener x) {
      listeners.remove(JComplexListListener.class, x);
   }
 
}
 

