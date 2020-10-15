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

import com.izforge.izpack.installer.InstallerFrame;
import com.sun.grid.installer.util.Util;
import java.awt.Component;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.DefaultCellEditor;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JTable;

/**
 * Editor for log button cells
 */
public class LogButtonCellEditor extends DefaultCellEditor {
    private JButton button = null;
    private static LogFrame logFrame = null;
    private String pressedText = "";

    /**
     * Constructor
     * @param pressedText Text to show when the cell has been pressed
     */
    public LogButtonCellEditor(String pressedText) {
        super(new JCheckBox());

        this.pressedText   = pressedText;

        button = new JButton();
        button.setMargin(new Insets(0, 0, 0, 0));
        button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                fireEditingStopped();
            }
        });
    }

    @Override
    public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
        // check wheter the host has log file
        String log = (String)table.getModel().getValueAt(row, column);
        if (log != null && !log.equals("")) {

            // set title
            String hostName = (String) table.getModel().getValueAt(row, 0);
            String hostIp = (String) table.getModel().getValueAt(row, 1);
            String component = (String) table.getModel().getValueAt(row, 2);

            String title = "Installation Log - " + hostName;
            if (hostIp != null && !hostIp.equals("")) {
                title += " (" + hostIp + ") ";
            }

            title += component;

            // show the Log window
            InstallerFrame installerFrame = (InstallerFrame)Util.findParentInstallerFrameComponent(table);
            LogFrame frame = getLogFrame(installerFrame, title);
            frame.setLogContent(log);
            frame.setVisible(true);
        }

        button.setText(pressedText);

        return button;
    }

    @Override
    protected void fireEditingStopped() {
        super.fireEditingStopped();
    }

    /**
     * Returns with the single instance of the Log window
     * @param title The title of the Log window
     * @return the single instance of the Log window
     */
    public static LogFrame getLogFrame(InstallerFrame parent, String title) {
        if (logFrame == null) {
            logFrame = new LogFrame(parent, title);
            logFrame.setLocationRelativeTo(parent);
        } else {
            logFrame.setTitle(title);
        }

        return logFrame;
    }
}
