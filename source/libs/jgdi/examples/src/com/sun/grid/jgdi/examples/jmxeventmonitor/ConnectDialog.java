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
package com.sun.grid.jgdi.examples.jmxeventmonitor;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.prefs.Preferences;
import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.SpinnerNumberModel;

/**
 *
 */
public class ConnectDialog extends JDialog {

    private final JTextField hostTextField = new JTextField();
    private final SpinnerNumberModel portSpinnerModel = new SpinnerNumberModel(1, 1, 65536, 1);
    private final JSpinner portSpinner = new JSpinner(portSpinnerModel);
    private final JTextField usernameTextField = new JTextField();
    private final JPasswordField passwordTextField = new JPasswordField();
    private final JToggleButton useSSLButton = new JToggleButton(new ToggleAction());
    private final JTextField caTopTextField = new JTextField();
    private final JTextField keyStoreTextField = new JTextField();
    private final JPasswordField keyStorePasswordTextField = new JPasswordField();
    private boolean canceled;
    private boolean useSSL = false;

    public ConnectDialog(JFrame frame) {
        super(frame, true);

        setTitle("connect");
        JPanel panel = new JPanel(new GridBagLayout());
        getContentPane().add(panel, BorderLayout.CENTER);

        portSpinner.setEditor(new JSpinner.NumberEditor(portSpinner, "0"));
        addLabelAndComponent(panel, 0, "Host:", hostTextField);
        addLabelAndComponent(panel, 1, "Port:", portSpinner);
        addLabelAndComponent(panel, 2, "Username:", usernameTextField);
        addLabelAndComponent(panel, 3, "Password:", passwordTextField);

        addLabelAndComponent(panel, 4, "", useSSLButton);

        addLabelAndComponent(panel, 5, "CA Top Path:", caTopTextField);
        addLabelAndComponent(panel, 6, "Keystore Path:", keyStoreTextField);
        addLabelAndComponent(panel, 7, "Keystore Password:", keyStorePasswordTextField);

        keyStoreTextField.setEnabled(useSSL);
        keyStorePasswordTextField.setEnabled(useSSL);
        caTopTextField.setEnabled(useSSL);

        panel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.CENTER));

        buttonPanel.add(new JButton(new OKAction()));
        buttonPanel.add(new JButton(new CancelAction()));

        getContentPane().add(buttonPanel, BorderLayout.SOUTH);

        setDefaultCloseOperation(HIDE_ON_CLOSE);

        initFromPrefs();


        addWindowFocusListener(new MyWindowListener());
        pack();
    }

    private static void addLabelAndComponent(JPanel panel, int y, String text, JComponent comp) {
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.gridx = 0;
        gbc.gridy = y;
        gbc.gridwidth = 1;
        gbc.gridheight = 1;
        gbc.anchor = GridBagConstraints.EAST;
        gbc.fill = GridBagConstraints.NONE;
        gbc.insets = new Insets(3, 3, 3, 10);
        panel.add(new JLabel(text), gbc);

        gbc = new GridBagConstraints();
        gbc.gridx = 1;
        gbc.gridy = y;
        gbc.gridwidth = 1;
        gbc.gridheight = 1;
        gbc.anchor = GridBagConstraints.WEST;
        gbc.fill = GridBagConstraints.HORIZONTAL;
        gbc.insets = new Insets(3, 10, 3, 3);
        panel.add(comp, gbc);
    }

    public boolean isCanceled() {
        return canceled;
    }

    public String getHost() {
        return hostTextField.getText();
    }

    public int getPort() {
        return portSpinnerModel.getNumber().intValue();
    }

    public String getUsername() {
        return usernameTextField.getText();
    }

    public char[] getPassword() {
        return passwordTextField.getPassword();
    }

    public String getCaTop() {
        if (!useSSL) {
            return null;
        }
        return caTopTextField.getText();
    }

    public String getKeystore() {
        if (!useSSL) {
            return null;
        }
        return keyStoreTextField.getText();
    }

    public char[] getKeyStorePassword() {
        if (!useSSL) {
            return null;
        }
        return keyStorePasswordTextField.getPassword();
    }
    
    public boolean useSSL() {
        return useSSL;
    }

    private class OKAction extends AbstractAction {

        public OKAction() {
            super("OK");
        }

        public void actionPerformed(ActionEvent e) {
            canceled = false;
            setVisible(false);
        }
    }

    private class ToggleAction extends AbstractAction {

        public ToggleAction() {
            super("SSL is disabled");
        }

        public void actionPerformed(ActionEvent e) {
            useSSL = !useSSL;
            if (useSSL) {
                useSSLButton.setText("SSL is enabled");
            } else {
                useSSLButton.setText("SSL is disabled");
            }
            keyStoreTextField.setEnabled(useSSL);
            keyStorePasswordTextField.setEnabled(useSSL);
            caTopTextField.setEnabled(useSSL);
        }
        
     }

    private class CancelAction extends AbstractAction {

        public CancelAction() {
            super("Cancel");
        }

        public void actionPerformed(ActionEvent e) {
            setVisible(false);
        }
    }

    private class MyWindowListener extends WindowAdapter {

        @Override
        public void windowGainedFocus(WindowEvent e) {
            if (hostTextField.getText().length() > 0) {
                if (usernameTextField.getText().length() > 0) {
                    passwordTextField.requestFocusInWindow();
                } else {
                    usernameTextField.requestFocusInWindow();
                }
            } else {
                hostTextField.requestFocusInWindow();
            }
        }
    }

    private void initFromPrefs() {

        Preferences prefs = Preferences.userNodeForPackage(ConnectDialog.class);

        usernameTextField.setText(prefs.get("username", System.getProperty("user.name")));
        portSpinnerModel.setValue(prefs.getInt("port", 1025));
        hostTextField.setText(prefs.get("host", ""));

        caTopTextField.setText(prefs.get("caTop", System.getProperty("com.sun.grid.jgdi.caTop")));
        keyStoreTextField.setText(prefs.get("keyStore", System.getProperty("com.sun.grid.jgdi.keyStore")));
    }

    public void saveInPrefs() {

        Preferences prefs = Preferences.userNodeForPackage(ConnectDialog.class);

        prefs.put("username", getUsername());
        prefs.putInt("port", getPort());
        prefs.put("host", getHost());

        if (useSSL) {
            prefs.put("caTop", getCaTop());
            prefs.put("keyStore", getKeystore());
        }
    }

    @Override
    public void setVisible(boolean visible) {
        if (visible) {
            canceled = true;
            passwordTextField.setText("");
        }
        super.setVisible(visible);
    }
}
