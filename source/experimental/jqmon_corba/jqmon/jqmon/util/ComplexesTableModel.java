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
 
 
 
public class ComplexesTableModel extends AbstractTableModel {
 
   final static public String[] columnNames = { "Name",
                                                "Complex List"};
 
   protected JComplexList complexList = null;
 
   public ComplexesTableModel(JComplexList cList) {
      complexList = cList;
   }
 
   public int getColumnCount() {
      return columnNames.length;
   }
 
   public int getRowCount() {
      if (complexList!=null)
      {
         return complexList.size();
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
         JComplex complex = (JComplex)complexList.elementAt(row);
         switch (col) {
            case 0:
               return complex.getName();
            case 1:
               return complex.getComplexEntryList();
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
      if (col < 1) {
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
         JComplex complex = (JComplex)complexList.elementAt(row);
         switch (col) {
            case 0:
               complex.setName((String)value);
               break;
            case 4:
               complex.setComplexEntryList((JComplexEntryList)value);
               break;
         }
      }
      catch (Exception e) {
      }
   }
 
   public void setComplexList(JComplexList cList) {
      complexList = cList;
      fireTableDataChanged();
      System.out.println("ComplexesTableModel.fireTableDataChanged()");
   }
 
   public void updateComplex(int index) {
      fireTableRowsUpdated(index, index);
   }
        
	public void deleteComplex(int index) {
      complexList.removeElementAt(index);
		fireTableRowsDeleted(index, index+1);
      System.out.println(index);
		System.out.println("ComplexesTableModel.fireTableRowsDeleted()");
   }
}

