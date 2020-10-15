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

import com.sun.grid.installer.util.Util;
import java.awt.Color;
import java.awt.Component;
import java.util.Hashtable;
import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.border.EmptyBorder;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellRenderer;

/**
 * Class for rendering State syled cells
 *
 * @see Host.State
 */
public class StateCellRenderer implements TableCellRenderer {
    private JLabel label = null;
    private Hashtable<Integer, HostProgressBar> progressBars = null;

    private static final Color COLOR_GOOD = Color.GREEN;
    private static final Color COLOR_BAD  = Color.RED;
    private static final Color COLOR_WARNING  = new Color(255, 215, 0); // Yellow

    private StateCellColumnUpdater progressBarUpdater = null;

    /**
     * Constructor
     */
    public StateCellRenderer() {
        // create progress bar storage
        int initCapacity = Math.max(Util.INSTALL_THREAD_POOL_SIZE, Util.RESOLVE_THREAD_POOL_SIZE) + 1;
        progressBars = new Hashtable<Integer, HostProgressBar>(initCapacity, 1);

        label = new JLabel();
        label.setOpaque(true);
        label.setBorder(new EmptyBorder(0, 1, 0, 1));
    }

    public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
        Component comp = null;

        if (value != null && value instanceof Host.State) {
            Host.State state = (Host.State)value;
            
            Color backColor = table.getBackground();
            label.setText(state.toString());
            label.setToolTipText(state.getTooltip());

            // differenciate between states
            switch (state) {
                case RESOLVING:
                case PROCESSING:
                case CONTACTING:
                case VALIDATING: { // running states
                    if (!progressBars.containsKey(Integer.valueOf(row))) {
                        progressBars.put(Integer.valueOf(row), new HostProgressBar(state.toString()));
                        
                        startUpdate(table, column);
                    }
                    comp = progressBars.get(Integer.valueOf(row));
                    break;
                }
                case OK:
                case SUCCESS:
                case READY_TO_INSTALL:
                case REACHABLE: { // good states
                    if (progressBars.containsKey(Integer.valueOf(row))) {
                        progressBars.remove(Integer.valueOf(row));
                    }

                    comp = label;
                    backColor = COLOR_GOOD;
                    break;
                }
                case PERM_QMASTER_SPOOL_DIR:
                case PERM_EXECD_SPOOL_DIR:
                case PERM_BDB_SPOOL_DIR:
                case PERM_JMX_KEYSTORE:
                case BDB_SPOOL_DIR_EXISTS:
                case BDB_SPOOL_DIR_WRONG_FSTYPE:
                case USED_QMASTER_PORT:
                case USED_EXECD_PORT:
                case USED_JMX_PORT:
                case ADMIN_USER_NOT_KNOWN:
                case UNKNOWN_ERROR: { // warning states
                    if (progressBars.containsKey(Integer.valueOf(row))) {
                        progressBars.remove(Integer.valueOf(row));
                    }
                    comp = label;
                    backColor = COLOR_WARNING;
                    break;
                }
                default: { // bad states
                    if (progressBars.containsKey(Integer.valueOf(row))) {
                        progressBars.remove(Integer.valueOf(row));
                    }
                    comp = label;
                    backColor = COLOR_BAD; // RESOLVABLE, UNREACHABLE, NEW_UNKNOWN_HOST, UNKNOWN_HOST, FAILED, COPY_TIMEOUT, COPY_FAILED, ADMIN_USER_MISSING
                }
            }

            if (isSelected) {
                comp.setForeground(backColor);
                comp.setBackground(table.getSelectionBackground());
            } else {
                comp.setForeground(backColor);
                comp.setBackground(table.getBackground());
            }
        }

        return comp;
    }

    /**
     * Starts a thread if it hasn't been started yet which updates the column
     * associated with this cell renderer.
     * @param table The table contains the column
     * @param col The visible column index
     */
    private void startUpdate(JTable table, int col) {
        if (progressBarUpdater == null || !progressBarUpdater.isRunning()) {
            progressBarUpdater = new StateCellColumnUpdater((DefaultTableModel)table.getModel(), toModel(table, col));

            Thread t = new Thread(progressBarUpdater, "StateCellColumnUpdater");
            t.start();
        }
    }

    /**
     * Converts a visible column index to a column index in the model.
     * @param table The table which contains the column
     * @param vColIndex The visible column index
     * @return The model index of the column. -1 if the index does not exist.
     */
    public int toModel(JTable table, int vColIndex) {
        if (vColIndex >= table.getColumnCount()) {
            return -1;
        }
        return table.getColumnModel().getColumn(vColIndex).getModelIndex();
    }

    /**
     * Column updater to imitate moving progress bar
     */
    private class StateCellColumnUpdater implements Runnable {
        private DefaultTableModel tableModel = null;
        private int col = 0;
        private boolean isRunning = false;

        /**
         * Constructor
         * @param tableModel The model to be updated
         * @param col The model index of the column to be updated
         */
        public StateCellColumnUpdater(DefaultTableModel tableModel, int col) {
            this.tableModel = tableModel;
            this.col = col;

            if (col < 0 || tableModel.getColumnCount() < col) {
                throw new IllegalArgumentException("Invalid column index: " + col);
            }
        }

        public void run() {
            isRunning = true;
            
            boolean found = false;
            Object obj = null;
            Host.State state = null;
            
            do {
                found = false;
                
                try {
                    Thread.sleep(200);
                } catch (InterruptedException e) {}
                
                for (int row = 0; row < tableModel.getRowCount(); row++) {
                    try {
                        obj = tableModel.getValueAt(row, col);

                        if (obj instanceof Host.State) {
                            state = (Host.State) obj;

                            switch (state) {
                                case RESOLVING:
                                case PROCESSING:
                                case CONTACTING:
                                case VALIDATING:
                                case READY_TO_INSTALL:
                                case NEW_UNKNOWN_HOST:
                                    found = true;
                            }
                        }

                        // if the state turns from RESOLVING/INSTALLING/CONTACTING to a final state
                        // the cell does not get updated and the progress bar remains shown

                        // TODO update cell only if it's visible (view)
                        tableModel.fireTableCellUpdated(row, col);
                    } catch (IndexOutOfBoundsException e) {
                        // if a host gets deleted during the progress: IndexOutOfBoundsException

                        // try another run...
                        found = true;
                    }
                }
            } while (found);
            
            isRunning = false;
        }

        /**
         * Returns true only if the thread is running and hasn't been terminated.
         * @return true if the thread is still running, false otherwise.
         */
        public boolean isRunning() {
            return isRunning;
        }
    }
}
