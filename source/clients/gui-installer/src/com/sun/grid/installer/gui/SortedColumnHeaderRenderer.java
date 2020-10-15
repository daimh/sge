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

import java.awt.Component;
import java.awt.Insets;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JTable;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableCellRenderer;

/**
 * Class for rendering sorted column header
 */
public class SortedColumnHeaderRenderer extends JButton implements TableCellRenderer {
    public static final int STATE_NONE = 0;
    public static final int STATE_UP   = 1;
    public static final int STATE_DOWN = 3;
    
    private int columnState = STATE_NONE;
    private int pressedColumn = -1;
    private int selectedColumn = pressedColumn;

    private JTableHeader header = null;
    
    private Icon upArrowIcon = null;
    private Icon downArrowIcon = null;

    /**
     * Constructor
     * @param header The table's header object
     * @param upArrowIcon Icon for showing ascent state
     * @param downArrowIcon Icon for showing descent state
     */
    public SortedColumnHeaderRenderer(JTableHeader header, Icon upArrowIcon, Icon downArrowIcon) {
        super();
        this.header = header;
        this.upArrowIcon = upArrowIcon;
        this.downArrowIcon = downArrowIcon;

        setMargin(new Insets(0, 0, 0, 0));
        setHorizontalTextPosition(LEFT);
    }

    public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
        setText(value == null ? "" : value.toString());

        // set pressed state
        boolean isPressed = (pressedColumn == column);
        getModel().setPressed(isPressed);
        getModel().setArmed(isPressed);

        // set icon
        if (selectedColumn == column) {
            switch (columnState) {
                case STATE_UP: setIcon(upArrowIcon); break;
                case STATE_DOWN: setIcon(downArrowIcon); break;
                default: setIcon(null);
            }
        } else {
            setIcon(null);
        }

        return this;
    }

    /**
     * Sets the pressed column index.
     * @param pressedColumn the index of the pressed column
     *
     * @see setSelectedColumn
     */
    public void setPressedColumn(int pressedColumn) {
        this.pressedColumn = pressedColumn;
    }

    /**
     * Set selected column index
     * @param selectedColumn Selected column index
     * @param state State of the arrow button. States: STATE_NONE, STATE_UP, STATE_DOWN
     *
     * @see setPressedColumn
     */
    public void setSelectedColumn(int selectedColumn, int state) {
        this.selectedColumn = selectedColumn;
        this.columnState = state;
    }

    /**
     * Returns with the selected column's state.
     * @return the selected column's state. States: STATE_NONE, STATE_UP, STATE_DOWN
     */
    public int getSelectedColumnState() {
        return columnState;
    }
}
