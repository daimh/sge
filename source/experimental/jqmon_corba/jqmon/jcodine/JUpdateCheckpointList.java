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


public class JUpdateCheckpointList extends Object {

	protected JAnswerList       answerList     = null;
	protected JCheckpointList   checkpointList = null;
	protected EventListenerList listeners      = null;
	
	public JUpdateCheckpointList() {
		listeners = new EventListenerList();
	}
   

   public void deleteCheckpoint(int index) {
      System.out.println("JUpdateCheckpointList.deleteCheckpoint()");
		fireCheckpointDelete(index);
   }

	protected void fireCheckpointDelete(int index) {
      System.out.println("ENTER JUpdateCheckpointList.fireCheckpointDelete()");

      JCheckpointListEvent ckpte = new JCheckpointListEvent(this, index);

      Object[] listener = listeners.getListenerList();

      System.out.println(listener.length-2);
      for (int i = listener.length-2; i>=0; i-=2) {
         System.out.println(listener[i].toString());
         System.out.println(JCheckpointListListener.class.toString());
         //if (listener[i] == JCheckpointListListener.class) {
         	JCheckpointListListener ckptl = (JCheckpointListListener)listener[i+1];
            ckptl.deleteCheckpointAvailable(ckpte);
            System.out.println("JUpdateCheckpointList.fireCheckpointDelete(index)");
         //}
      }
      System.out.println("EXIT");

   }  

   
   public void updateCheckpoint(int index) {
      System.out.println("JUpdateCheckpointList.fireCheckpointUpdate()");
      fireCheckpointUpdate(index);
   }
 
   protected void fireCheckpointUpdate(int index) {
      System.out.println("ENTER JUpdateCheckpointList.fireCheckpointUpdate()");
 
      JCheckpointListEvent ckpte = new JCheckpointListEvent(this, index);
 
      Object[] listener = listeners.getListenerList();
 
      System.out.println(listener.length-2);
      for (int i = listener.length-2; i>=0; i-=2) {
         System.out.println(listener[i].toString());
         System.out.println(JCheckpointListListener.class.toString());
         if (listener[i] == JCheckpointListListener.class) {
            JCheckpointListListener ckptl = (JCheckpointListListener)listener[i+1];
            ckptl.updateCheckpointAvailable(ckpte);
            System.out.println("JUpdateCheckpointList.fireCheckpointUpdate(index)");
                        }
                }
                System.out.println("EXIT");
 
        }
	public void newJCheckpointList(JAnswerList al, JCheckpointList ckptl) {
		answerList     = al;
		checkpointList = ckptl;
		fireChange();
	}
		
	public JCheckpointList getCheckpointList() throws JAnswerListException {
		Enumeration e = answerList.elements();
		while ( e.hasMoreElements() ) {
			JAnswerObject a = (JAnswerObject)e.nextElement();
			// if there 's something wrong with the object throw an exception 
			if ( a.getStatus() != 1 ) {
				throw new JAnswerListException(a);
			}
		}
		return checkpointList;
	}

	public JAnswerList getAnswerList() {
		return answerList;
	}

	public void fireChange() {
		// build Event 
		JCheckpointListEvent h = new JCheckpointListEvent(this);

		// check all listeners
		Object[] listener = listeners.getListenerList();
		
		for (int i = listener.length-2; i>=0; i-=2) {
			if (listener[i] == JCheckpointListListener.class) {
				JCheckpointListListener ckptl = (JCheckpointListListener)listener[i+1];
				ckptl.newJCheckpointListAvailable(h);
			}
		}
	}

	public void addJCheckpointListListener(JCheckpointListListener x) {
		listeners.add(x.getClass(), x);
		x.newJCheckpointListAvailable(new JCheckpointListEvent(this));
	}
	
	public void removeChangeListener(ChangeListener x) {
		listeners.remove(JCheckpointListListener.class, x);
	}
				
}
