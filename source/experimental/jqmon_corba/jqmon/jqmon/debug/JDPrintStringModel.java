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

/* Swing 1.0.x 
import com.sun.java.swing.*;
import com.sun.java.swing.event.*;
*/

/* Swing 1.1 */
import javax.swing.*;
import javax.swing.event.*;

import java.awt.*;
import java.awt.event.*;


public class JDPrintStringModel extends Object {
   
   protected String dprintString;
   protected EventListenerList changeListeners;
   
   public JDPrintStringModel() {
      dprintString = null;
      changeListeners = new EventListenerList();
   }

   
   synchronized public void addElement(String s) {
      dprintString = s;
      fireChange();
   }
   

   public String getJDPrintString() {
      return dprintString;
   }
   
   public void fireChange() {
      // create the event
      ChangeEvent c = new ChangeEvent(this);
      // get the listener list
      Object[] listeners = changeListeners.getListenerList();
      // process the listeners last to first
      // list is in pairs, class and instance
      for (int i = listeners.length-2; i >= 0; i -= 2) {
         if (listeners[i] == ChangeListener.class) {
            ChangeListener cl = (ChangeListener)listeners[i+1];
            cl.stateChanged(c);
         }
      }
   }

   
   public void addChangeListener(ChangeListener x) {
   
      changeListeners.add (ChangeListener.class, x);

      // bring it up to date with current state
      x.stateChanged(new ChangeEvent(this));
   }


   public void removeChangeListener(ChangeListener x) {
      changeListeners.remove (ChangeListener.class, x);
   }
   
}
