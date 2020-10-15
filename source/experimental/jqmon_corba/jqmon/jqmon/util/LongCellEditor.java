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
 
import java.awt.Component;
import java.awt.event.*;
import java.awt.AWTEvent;
import java.lang.Boolean;
import javax.swing.table.*;
import javax.swing.event.*;
import java.util.EventObject;
import javax.swing.tree.*;
import java.io.Serializable;

import java.awt.*;
import javax.swing.*; 
import jqmon.views.*;
 
public class LongCellEditor implements TableCellEditor, 
                                       TreeCellEditor,
                                       Serializable {
// Instance Variables
 
   // event listeners 
   protected EventListenerList listenerList = new EventListenerList();
   transient protected ChangeEvent changeEvent = null;
 
   protected JComponent editorComponent;
   protected EditorDelegate delegate;
   protected int clickCountToStart = 1;
 
//  Constructors
 
   // constructs a DefaultCellEditor that uses a text field.
   public LongCellEditor(JLongPanel x) {
      this.editorComponent = x;
      this.clickCountToStart = 1;
      this.delegate = new EditorDelegate() {
         public void setValue(Object x) {
            super.setValue(x);
            if (x instanceof Integer)
               ((JLongPanel)editorComponent).setValue(((Integer)x).intValue());
            else
               ((JLongPanel)editorComponent).setValue(0);
         }
         public Object getCellEditorValue() {
            return new Integer(((JLongPanel)editorComponent).getValue());
         }
 
         public boolean startCellEditing(EventObject anEvent) {
            if(anEvent == null)
               editorComponent.requestFocus();
            return true;
         }
 
         public boolean stopCellEditing() {
            return true;
         }
      };
      ((JLongPanel)editorComponent).addActionListener(delegate);
   }
 
   // returns the a reference to the editor component.
   // return the editor Component
   public Component getComponent() {
        return editorComponent;
    }
 
//  Modifying
 
    
   // specifies the number of clicks needed to start editing.
   
   // @param count  an int specifying the number of clicks needed to start editing
   // @see #getClickCountToStart
   public void setClickCountToStart(int count) {
      clickCountToStart = count;
   }
 
   // clickCountToStart controls the number of clicks required to start
   // editing if the event passed to isCellEditable() or startCellEditing() is
   // a MouseEvent.  For example, by default the clickCountToStart for
   // a JTextField is set to 2, so in a JTable the user will need to
   // double click to begin editing a cell.
   public int getClickCountToStart() {
      return clickCountToStart;
   }
 
// Implementing the CellEditor Interface
 
   // implements javax.swing.CellEditor
   public Object getCellEditorValue() {
      return delegate.getCellEditorValue();
   }
 
   // implements javax.swing.CellEditor
   public boolean isCellEditable(EventObject anEvent) {
      if (anEvent instanceof MouseEvent) {
         if (((MouseEvent)anEvent).getClickCount() < clickCountToStart)
            return false;
      }
      return delegate.isCellEditable(anEvent);
   }
 
   // implements javax.swing.CellEditor
   public boolean shouldSelectCell(EventObject anEvent) {
      boolean retValue = true;
 
      if (this.isCellEditable(anEvent)) {
         if (anEvent == null || ((MouseEvent)anEvent).getClickCount() >= clickCountToStart)
            retValue = delegate.startCellEditing(anEvent);
      }
 
      // By default we want the cell the be selected so
      // we return true
      return retValue;
   }
 
   // implements javax.swing.CellEditor
   public boolean stopCellEditing() {
      boolean stopped = delegate.stopCellEditing();
 
      if (stopped) {
         fireEditingStopped();
      }
 
        return stopped;
    }
 
   // implements javax.swing.CellEditor
   public void cancelCellEditing() {
      delegate.cancelCellEditing();
      fireEditingCanceled();
   }

 
//  Handle the event listener bookkeeping

   // implements javax.swing.CellEditor
   public void addCellEditorListener(CellEditorListener l) {
      listenerList.add(CellEditorListener.class, l);
   }
 
   // implements javax.swing.CellEditor
   public void removeCellEditorListener(CellEditorListener l) {
      listenerList.remove(CellEditorListener.class, l);
   }
 
   // notify all listeners that have registered interest for
   // notification on this event type.  The event instance
   // is lazily created using the parameters passed into
   // the fire method.
   // @see EventListenerList
   
   protected void fireEditingStopped() {
      // guaranteed to return a non-null array
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
 
   // notify all listeners that have registered interest for
   // notification on this event type.  The event instance
   // is lazily created using the parameters passed into
   // the fire method.
   // @see EventListenerList
     
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

 
//  Implementing the TreeCellEditor Interface
 
 
 
   // implements javax.swing.tree.TreeCellEditor
   public Component getTreeCellEditorComponent(JTree tree, Object value,
                                               boolean isSelected,
                                               boolean expanded,
                                               boolean leaf, int row) {
      String stringValue = tree.convertValueToText(value, isSelected, expanded, leaf, row, false);
 
      delegate.setValue(stringValue);
      return editorComponent;
   }
 

//  Implementing the CellEditor Interface

 
   // implements javax.swing.table.TableCellEditor
   public Component getTableCellEditorComponent(JTable table, Object value,
                                                boolean isSelected,
                                                int row, int column) {
 
      // modify component colors to reflect selection state
      // PENDING(alan)
      /*if (isSelected) {
         component.setBackground(selectedBackgroundColor);
         component.setForeground(selectedForegroundColor);
      }
      else {
         component.setBackground(backgroundColor);
         component.setForeground(foregroundColor);
      }*/
	
      //my try
      //table.resizeAndRepaint(); 
      //table.revalidate();
      //table.repaint(); 
      //Rectangle rect = table.getCellRect(row, column, true);
      //table.setRowHeight((int)rect.height);
	
      //table.setRowHeight(editorComponent.getSize().height);
      //table.repaint();       

      Rectangle rect = table.getCellRect(row, column, true);
      editorComponent.setMaximumSize(new Dimension((int)rect.width, (int)rect.height)); 

      delegate.setValue(value);
      return editorComponent;
   }
 
 

//  Protected EditorDelegate class

 
   protected class EditorDelegate implements ActionListener, ItemListener, Serializable {
 
      protected Object value;
 
      public Object getCellEditorValue() {
         return value;
      }
 
      public void setValue(Object x) {
         this.value = x;
      }
 
      public boolean isCellEditable(EventObject anEvent) {
         return true;
      }
 
      public boolean startCellEditing(EventObject anEvent) {
         return true;
      }
 
      public boolean stopCellEditing() {
         return true;
      }
 
      public void cancelCellEditing() {
      }
 
      // implementing ActionListener interface
      public void actionPerformed(ActionEvent e) {
         fireEditingStopped();
      }
 
      // implementing ItemListener interface
      public void itemStateChanged(ItemEvent e) {
         fireEditingStopped();
      }
   }
} // end of class JCellEditor

