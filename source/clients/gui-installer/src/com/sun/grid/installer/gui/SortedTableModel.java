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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.installer.gui;

import javax.swing.table.DefaultTableModel;

/**
 * Abstract class for extending DefaultTableModel with sorting functionality.
 *
 * @see DefaultTableModel
 */
public abstract class SortedTableModel extends DefaultTableModel {
    public int[] sortedIndexes = null;

    private int prevCol = -1;
    private boolean prevIsAscent = false;

    /**
     * Contructor
     */
    public SortedTableModel(Object[][] data, String[] columnHeaders) {
        super(data, columnHeaders);
    }

    /**
     * Calls sorting depending on the values of the given column then updates the table.
     * @param col The sorting based ont this column's values.
     * @param isAscent The order of the sorted rows.
     */
    public void sortByColumn(int col, boolean isAscent) {
        sort(col, isAscent);
        fireTableDataChanged();
    }

    /**
     * Returns with the new instance of sorted indexes depending on the row count.
     * @return a new sorted indexes instance
     */
    public int[] getSortedIndexes() {
        int rowCount = getRowCount();

        if (sortedIndexes != null) {
            if (sortedIndexes.length == rowCount) {
                return sortedIndexes;
            }
        }
        
        sortedIndexes = new int[rowCount];
        for (int i = 0; i < rowCount; i++) {
            sortedIndexes[i] = i;
        }
        return sortedIndexes;
    }

    /**
     * Sorts the rows depending on the values of the given column.
     * @param column The sorting based ont this column's values.
     * @param isAscent The order of the sorted rows.
     */
    private void sort(int column, boolean isAscent) {
        prevCol = column;
        prevIsAscent = isAscent;
        
        int n = getRowCount();
        int[] indexes = getSortedIndexes();

        for (int i = 0; i < n - 1; i++) {
            int k = i;
            for (int j = i + 1; j < n; j++) {
                if (isAscent) {
                    if (compare(column, j, k) < 0) {
                        k = j;
                    }
                } else {
                    if (compare(column, j, k) > 0) {
                        k = j;
                    }
                }
            }
            int tmp = indexes[i];
            indexes[i] = indexes[k];
            indexes[k] = tmp;
        }
    }

    /**
     * Compares two row values in the the specified column
     * @param col The column where the values should be compared
     * @param row1 The first row id
     * @param row2 The second row id
     * @return the result of the comparation.
     */
    private int compare(int col, int row1, int row2) {
        Object o1 = getValueAt(row1, col);
        Object o2 = getValueAt(row2, col);

        if (o1 == null && o2 == null) {
            return 0;
        } else if (o1 == null) {
            return -1;
        } else if (o2 == null) {
            return 1;
        } else if (o2 == o1) {
            return 0;
        } else {
            Class type = getColumnClass(col);

            if (type == String.class) {
                return ((String)o1).compareTo((String)o2);
            } else if (type == Boolean.class) {
                return ((Boolean)o1).compareTo((Boolean)o2);
            } else if (type == Host.State.class) {
                return o1.toString().compareTo(o2.toString());
            } else {
                return ((String)o1).compareTo((String)o2);
            }
        }
    }

    /**
     * Returns with the sorted index of the specified row
     * @param row The row id in the view.
     * @return The sorted index of the row in the model.
     *
     * @see getRowIndex
     */
    public int getSortedRowIndex(int row) {
        if (sortedIndexes != null && sortedIndexes.length > row) {
            row = sortedIndexes[row];
        }
        return row;
    }

    /**
     * Returns with the index of the sorted row index.
     * @param sortedRow The sorted row id in the model.
     * @return The index of the row in the view.
     *
     * @see getSortedRowIndex
     */
    public int getRowIndex(int sortedRow) {
        if (sortedIndexes != null) {
            for (int i = 0; i < sortedIndexes.length; i++) {
                if (sortedRow == sortedIndexes[i]) {
                    sortedRow = sortedIndexes[i];
                }
            }
        }

        return sortedRow;
    }

    /**
     * Re-sorts the rows based on the last column and direction selection.
     */
    public void reSort() {
        if (prevCol != -1) {
            sortedIndexes = null;
            sort(prevCol, prevIsAscent);
        }
    }
}
