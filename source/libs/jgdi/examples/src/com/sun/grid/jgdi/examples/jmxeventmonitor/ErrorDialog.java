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
 *   Copyright: 2006 by Sun Microsystems, Inc
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jgdi.examples.jmxeventmonitor;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.tree.DefaultMutableTreeNode;

/**
 *
 */
public class ErrorDialog extends JDialog {

    private Throwable ex;
    private DefaultMutableTreeNode rootNode = new DefaultMutableTreeNode();
    private JTree exceptionTree = new JTree(rootNode);
    private JScrollPane exceptionScrollPane = new JScrollPane(exceptionTree);
    private JPanel buttonPanel = new JPanel(new FlowLayout());

    private ErrorDialog(JFrame f, String msg, int type, boolean modal) {

        super(f, modal);

        Icon icon = null;
        switch (type) {
            case JOptionPane.ERROR_MESSAGE:
                setTitle("Error");
                icon = UIManager.getIcon("OptionPane.errorIcon");
                break;
            case JOptionPane.INFORMATION_MESSAGE:
                setTitle("Info");
                icon = UIManager.getIcon("OptionPane.informationIcon");
                break;
            case JOptionPane.WARNING_MESSAGE:
                setTitle("Warning");
                icon = UIManager.getIcon("OptionPane.warningIcon");
                break;
        }

        setLayout(new BorderLayout());

        JTextArea textArea = new JTextArea();
        textArea.setColumns(30);
        textArea.setText(msg);
        textArea.setLineWrap(true);
        textArea.setWrapStyleWord(true);
        textArea.setEditable(false);
        textArea.setOpaque(false);

        textArea.setBorder(BorderFactory.createEmptyBorder(10, 20, 10, 20));


        if (icon != null) {
            JPanel textPanel = new JPanel(new BorderLayout());
            textPanel.add(textArea, BorderLayout.CENTER);
            JLabel label = new JLabel(icon);
            textPanel.add(label, BorderLayout.WEST);
            textPanel.setBorder(BorderFactory.createEmptyBorder(10, 20, 10, 20));
            add(textPanel, BorderLayout.NORTH);
        } else {
            add(textArea, BorderLayout.NORTH);
        }
        exceptionScrollPane.setVisible(false);
        add(exceptionScrollPane, BorderLayout.CENTER);

        JButton btc = new JButton("Close");

        buttonPanel.add(btc);


        btc.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                setVisible(false);
            }
        });


        add(buttonPanel, BorderLayout.SOUTH);
    }

    /** Creates a new instance of ErrorDialog */
    private ErrorDialog(JFrame f, Throwable ex) {
        this(f, ex.getLocalizedMessage(), ex);
    }

    private ErrorDialog(JFrame f, String msg, Throwable ex) {
        this(f, msg, JOptionPane.ERROR_MESSAGE, true);
        this.ex = ex;
        if (ex != null) {
            buttonPanel.add(new JButton(new ShowDetailsAction()));
        }
    }

    public static void showErrorDialog(JFrame frame, Throwable ex) {
        ErrorDialog dlg = new ErrorDialog(frame, ex);
        dlg.pack();
        if (frame != null) {
            dlg.setLocationRelativeTo(frame);
        }
        dlg.setVisible(true);
    }

    public static void showErrorDialog(JFrame frame, String msg, Throwable ex) {
        ErrorDialog dlg = new ErrorDialog(frame, msg, ex);
        dlg.pack();
        if (frame != null) {
            dlg.setLocationRelativeTo(frame);
        }
        dlg.setVisible(true);
    }

    public static void showErrorDialog(JFrame frame, String msg) {
        ErrorDialog dlg = new ErrorDialog(frame, msg, JOptionPane.ERROR_MESSAGE, true);
        dlg.pack();
        if (frame != null) {
            dlg.setLocationRelativeTo(frame);
        }
        dlg.setVisible(true);
    }

    public static void showInfoDialog(JFrame frame, String msg) {
        ErrorDialog dlg = new ErrorDialog(frame, msg, JOptionPane.INFORMATION_MESSAGE, true);
        dlg.pack();
        if (frame != null) {
            dlg.setLocationRelativeTo(frame);
        }
        dlg.setVisible(true);
    }

    public static void showWarnDialog(JFrame frame, String msg) {
        ErrorDialog dlg = new ErrorDialog(frame, msg, JOptionPane.WARNING_MESSAGE, true);
        dlg.pack();
        if (frame != null) {
            dlg.setLocationRelativeTo(frame);
        }
        dlg.setVisible(true);
    }

    public static void showWarnDialog(JFrame frame, String msg, Throwable ex) {
        ErrorDialog dlg = new ErrorDialog(frame, msg, JOptionPane.WARNING_MESSAGE, true);
        dlg.pack();
        if (frame != null) {
            dlg.setLocationRelativeTo(frame);
        }
        dlg.setVisible(true);
    }

    private class ShowDetailsAction extends AbstractAction {

        boolean visible = false;

        public ShowDetailsAction() {
            super("Details");
        }

        public void actionPerformed(ActionEvent e) {

            if (visible) {
                exceptionScrollPane.setVisible(false);
                visible = false;
            } else {
                if (rootNode.getChildCount() == 0) {
                    rootNode.add(new ExceptionNode(ErrorDialog.this.ex));

                    for (Throwable te = ErrorDialog.this.ex.getCause(); te != null; te = te.getCause()) {
                        rootNode.add(new ExceptionNode(te));
                    }
                    exceptionTree.expandRow(0);
                }
                exceptionScrollPane.setVisible(true);
                visible = true;
            }
            ErrorDialog.this.pack();
        }
    }

    private class ExceptionNode extends DefaultMutableTreeNode {

        public ExceptionNode(Throwable t) {
            super.setUserObject(String.format("%s: %s", t.getClass().getName(), t.getMessage()));

            StackTraceElement[] st = t.getStackTrace();
            for (StackTraceElement elem : st) {
                DefaultMutableTreeNode node = new DefaultMutableTreeNode();
                node.setUserObject(elem.toString());
                add(node);
            }
        }
    }

    public static void main(String[] args) {

        Exception ex = new Exception("Test exception");
        ex.initCause(new IllegalStateException("ex1"));
        ErrorDialog.showErrorDialog(null, ex);
        ErrorDialog.showInfoDialog(null, "blubber");
        System.exit(0);
    }
}
