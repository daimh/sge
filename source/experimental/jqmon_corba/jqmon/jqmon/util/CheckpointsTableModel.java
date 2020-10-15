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

import jcodine.*;
import javax.swing.table.*;
import java.util.Vector;



public class CheckpointsTableModel extends AbstractTableModel {
			
	final static public String[] columnNames = { "Name", 
                                                "Interface", 
                                                "Ckpt_Command", 
                                                "Migr_Command", 
                                                "Rest_Command", 
                                                "Ckpt_Dir", 
                                                "Queue_List", 
                                                "When", 
                                                "Signal", 
                                                "Clean_Command"};
			
   protected JCheckpointList checkpointList = null;


   
	public CheckpointsTableModel(JCheckpointList ckptList) {
		checkpointList = ckptList;
	}

   public int getColumnCount() {
      return columnNames.length;
   }
                  
   public int getRowCount() {
      if (checkpointList!=null)
      {
         return checkpointList.size();
      }
      else {
         return 0;
      }
   }

   public String getColumnName(int col) {
      return columnNames[col];
   }

   public Object getValueAt(int row, int col) {
      try {
         JCheckpoint checkpoint = (JCheckpoint)checkpointList.elementAt(row);
         switch (col) {
            case 0:
               return checkpoint.getName();
            case 1:
               return checkpoint.getInterface();
            case 2:
               return checkpoint.getCkptCommand();
            case 3:
               return checkpoint.getMigrCommand();
            case 4:
               return checkpoint.getRestCommand();
            case 5:
               return checkpoint.getCkptDir();
            case 6:
               return "";
               //return checkpoint.getQueueList();
            case 7:
               return checkpoint.getWhen();
            case 8:
               return checkpoint.getSignal();
            case 9:
               return checkpoint.getCleanCommand();
         }
     	} 
      catch (Exception e) {
      }
      
      return "";
   }

   public Class getColumnClass(int c) {
      return getValueAt(0, c).getClass();
   }

   //Don't need to implement this method unless your tables
   //editable.
   public boolean isCellEditable(int row, int col) {
      //Note that the data/cell address is constant,
      //no matter where the cell appears onscreen.
      if (col < 1) { 
         return false;
      } 
		else {
         return true;
      }
   }

   //Don't need to implement this method unless your tables
   //data can change.
   public void setValueAt(Object value, int row, int col) {
      try {
         JCheckpoint checkpoint = (JCheckpoint)checkpointList.elementAt(row);
         switch (col) {
            case 0:
               break;
            case 1:
               //checkpoint.setName((String)value);
               break;
         }
      }
      catch (Exception e) {
      }
   }

	public void setCheckpointList(JCheckpointList ckptList) {
		checkpointList = ckptList;
		fireTableDataChanged();
		System.out.println("CheckpointsTableModel.fireTableDataChanged()");
	}

	public void updateCheckpoint(int index) {
		fireTableRowsUpdated(index, index);
	}
	
	public void deleteCheckpoint(int index) {

      checkpointList.removeElementAt(index);
		fireTableRowsDeleted(index, index);
      
      System.out.println(index);
		System.out.println("CheckpointsTableModel.fireTableRowsDeleted()");
   }
}
