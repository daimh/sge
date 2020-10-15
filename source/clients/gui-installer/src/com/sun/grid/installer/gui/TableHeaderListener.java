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

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.table.JTableHeader;

/**
 * Listener class to catch mouse actions on table header
 *
 * @see MouseAdapter
 */
public class TableHeaderListener extends MouseAdapter {
    private JTableHeader header = null;
    private SortedColumnHeaderRenderer headerRenderer = null;
    private SortedTableModel model = null;
    private boolean isAscent = true;

    /**
     * Constructor
     * @param header The table header object
     * @param headerRenderer The sorted column header renderer
     *
     * @see JTableHeader
     * @see SortedColumnHeaderRenderer
     */
    public TableHeaderListener(JTableHeader header, SortedColumnHeaderRenderer headerRenderer) {
        this.header = header;
        this.headerRenderer = headerRenderer;
        this.model = (SortedTableModel)header.getTable().getModel();
    }

    @Override
    public void mousePressed(MouseEvent e) {
        int col = header.columnAtPoint(e.getPoint());
        int sortCol = header.getTable().convertColumnIndexToModel(col);

        if (header.getTable().isEditing()) {
            header.getTable().getCellEditor().stopCellEditing();
        }

        // flip the sorting order
        isAscent = !isAscent;

        // set the header view
        headerRenderer.setPressedColumn(col);
        headerRenderer.setSelectedColumn(col, (isAscent ? SortedColumnHeaderRenderer.STATE_UP : SortedColumnHeaderRenderer.STATE_DOWN));

        header.repaint();

        // sort
        model.sortByColumn(sortCol, isAscent);
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        // erase pressed button state after mouse has been released
        headerRenderer.setPressedColumn(-1);
        header.repaint();
    }
}
