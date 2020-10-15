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




public class QueuesTableModel extends AbstractTableModel {
			
	final static public String[] columnNames = { "Status",
                                                "Name",
                                                "JobSlots",
                                                "Host",
                                                "Complex List"};
			
   protected JQueueList queueList = null;
	
	public QueuesTableModel(JQueueList qList) {
		queueList = qList;
	}

   public int getColumnCount() {
      return columnNames.length;
   }
                  
   public int getRowCount() {
      if (queueList!=null)
      {
         return queueList.size();
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
         JQueue queue = (JQueue)queueList.elementAt(row);
         switch (col) {
            case 0:
               return new Integer(0);
            case 1:
               return queue.getQName();
            case 2:
               //return new Long(queue.getJobSlots());
					return new Integer((int)queue.getJobSlots());
            case 3:
            	return queue.getQHostName();
				case 4:
					return queue.getComplexList();
         }
      } 
		catch (Exception e) {
      }
      
      return "";
   }

   public Class getColumnClass(int c) {
      return getValueAt(0, c).getClass();
   }

   // don't need to implement this method unless your table's
   // editable.
   public boolean isCellEditable(int row, int col) {
      // note that the data/cell address is constant,
      // no matter where the cell appears onscreen.
      if (col < 2) { 
         return false;
      } 
		else {
         return true;
      }
   }

   // don't need to implement this method unless your table's
   // data can change.
   public void setValueAt(Object value, int row, int col) {
      try {
         JQueue queue = (JQueue)queueList.elementAt(row);
         switch (col) {
            case 0:
					break;
            case 1:
               queue.setQName((String)value);
               break;
            case 2:
               //queue.setJobSlots( ((Long)value).longValue() );
               queue.setJobSlots( ((Integer)value).intValue() );
               break;
            case 3:
               queue.setQHostName((String)value);
               break;
				case 4: 
					queue.setComplexList((JComplexList)value);
					break;
         }
      }
      catch (Exception e) {
      }
   }

	public void setQueueList(JQueueList qList) {
		queueList = qList;
		fireTableDataChanged();
		System.out.println("QueuesTableModel.fireTableDataChanged()");
	}

	public void updateQueue(int index) {
		fireTableRowsUpdated(index, index);
	}
	
	public void deleteQueue(int index) {

		fireTableRowsDeleted(index, index);
		System.out.println("QueuesTableModel.fireTableRowsDeleted()");
   }
}


