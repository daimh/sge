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
package jqmon.util;

     // File: JComponentCellEditor.java
     // Author: Zafir Anjum
     import java.awt.Component;
     import java.awt.event.*;
     import java.awt.AWTEvent;
     import java.lang.Boolean;
     import javax.swing.table.*;
     import javax.swing.event.*;
     import java.util.EventObject;
     import javax.swing.tree.*;
     import java.io.Serializable;
     import javax.swing.*;


public class JComponentCellEditor implements TableCellEditor, 
                                             TreeCellEditor,     
                                             Serializable {
             
   protected EventListenerList listenerList = new EventListenerList();
   transient protected ChangeEvent changeEvent = null;
             
   protected JComponent editorComponent = null;
   protected JComponent container = null;          // Can be tree or table
             
             
   public Component getComponent() {
      return editorComponent;
   }
             
   public Object getCellEditorValue() {
      return editorComponent;
   }
             
   public boolean isCellEditable(EventObject anEvent) {
      return true;
   }
             
   public boolean shouldSelectCell(EventObject anEvent) {
      if( editorComponent != null && anEvent instanceof MouseEvent && ((MouseEvent)anEvent).getID() == MouseEvent.MOUSE_PRESSED ) {
         Component dispatchComponent = SwingUtilities.getDeepestComponentAt(editorComponent, 3, 3 );
         MouseEvent e = (MouseEvent)anEvent;
         MouseEvent e2 = new MouseEvent( dispatchComponent, MouseEvent.MOUSE_RELEASED,
         e.getWhen() + 100000, e.getModifiers(), 3, 3, e.getClickCount(),
         e.isPopupTrigger() );
         dispatchComponent.dispatchEvent(e2); 
         e2 = new MouseEvent( dispatchComponent, MouseEvent.MOUSE_CLICKED,
         e.getWhen() + 100001, e.getModifiers(), 3, 3, 1,
         e.isPopupTrigger() );
         dispatchComponent.dispatchEvent(e2); 
      }
      return false;
   }
             
   public boolean stopCellEditing() {
      fireEditingStopped();
      return true;
   }
             
   public void cancelCellEditing() {
      fireEditingCanceled();
   }
             
   public void addCellEditorListener(CellEditorListener l) {
      listenerList.add(CellEditorListener.class, l);
   }
             
   public void removeCellEditorListener(CellEditorListener l) {
      listenerList.remove(CellEditorListener.class, l);
   }
             
   protected void fireEditingStopped() {
      Object[] listeners = listenerList.getListenerList();
      
      // process the listeners last to first, notifying
      // those that are interested in this event
      for (int i = listeners.length-2; i>=0; i-=2) {
         if (listeners[i]==CellEditorListener.class) {
            // lazily create the event:
            if (changeEvent == null)
               changeEvent = new ChangeEvent(this);
            ((CellEditorListener)listeners[i+1]).editingStopped(changeEvent);
         }              
      }
   }
             
   protected void fireEditingCanceled() {
      // guaranteed to return a non-null array
      Object[] listeners = listenerList.getListenerList();
      // process the listeners last to first, notifying
      // those that are interested in this event
      for (int i = listeners.length-2; i>=0; i-=2) {
         if (listeners[i]==CellEditorListener.class) {
            // lazily create the event:
            if (changeEvent == null)
               changeEvent = new ChangeEvent(this);
               ((CellEditorListener)listeners[i+1]).editingCanceled(changeEvent);
            }              
         }
      }
             
   // implements javax.swing.tree.TreeCellEditor
   public Component getTreeCellEditorComponent(JTree tree, Object value,
      boolean isSelected, boolean expanded, boolean leaf, int row) {
      String stringValue = tree.convertValueToText(value, isSelected, expanded, leaf, row, false);
      editorComponent = (JComponent)value;
      container = tree;
      return editorComponent;
   }
             
   // implements javax.swing.table.TableCellEditor
   public Component getTableCellEditorComponent(JTable table, Object value,
      boolean isSelected, int row, int column) {
         
      editorComponent = (JComponent)value;
      container = table;
      return editorComponent;
   }
             
} // end of class JComponentCellEditor


