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

import com.izforge.izpack.installer.InstallData;
import com.izforge.izpack.installer.InstallerFrame;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.Config;
import java.awt.Color;
import java.awt.Font;
import java.text.MessageFormat;
import java.util.ArrayList;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

/**
 * Modal styled dialog window to configure host specific values.
 */
public class HostConfigFrame extends JFrame implements Config {

    private InstallerFrame parent = null;
    private InstallData idata = null;
    private HostList hosts = null;
    private static HostConfigFrame instance = null;
    // Indicate the installation setups
    private boolean isQmasterInst = true;
    private boolean isShadowdInst = true;
    private boolean isExecdInst = true;
    private boolean isBdbInst = true;
    private boolean isExpressInst = true;
    private static Font DEF_TEXTFIELD_FONT = null;
    private static Font MULTIPLEVALUES_TEXTFIELD_FONT = null;
    private static Color DEF_FOREGROUND_COLOR = null;
    private static Color DEF_BACKGROUND_COLOR = null;

    static {
        JTextField tmpTextField = new JTextField();

        DEF_TEXTFIELD_FONT = tmpTextField.getFont();
        MULTIPLEVALUES_TEXTFIELD_FONT = new Font("Arial", Font.ITALIC, DEF_TEXTFIELD_FONT.getSize());
        DEF_FOREGROUND_COLOR = tmpTextField.getForeground();
        DEF_BACKGROUND_COLOR = tmpTextField.getBackground();
    }
    private static String multipleValuesString = "";

    /**
     * Constructor.
     * @param parent The parent {@link InstallerFrame}
     * @param idata The {@link InstallData}
     */
    private HostConfigFrame(InstallerFrame parent, InstallData idata) {
        super("");

        setLocationRelativeTo(parent);
        if (parent != null) {
            setIconImage(parent.icons.getImageIcon("options").getImage());
        }

        this.parent = parent;
        this.idata = idata;

        multipleValuesString = getLabel("multiple.values.text");

        initComponents();
    }

    /**
     * Returns with the instance of the {@link HostConfigFrame}.
     * @param parent The parent {@link InstallerFrame}
     * @param idata The {@link InstallData}
     * 
     * @return The {@link HostConfigFrame} instance.
     */
    public static HostConfigFrame getInstance(InstallerFrame parent, InstallData idata) {
        if (instance == null) {
            instance = new HostConfigFrame(parent, idata);
        }

        return instance;
    }

    /**
     * Returns with the instance of the {@link HostConfigFrame}.
     * @return The {@link HostConfigFrame} instance.
     *
     * @throws IllegalStateException In case of the instance hasn't been initialized yet.
     *
     * @see HostConfigFrame#getInstance(com.izforge.izpack.installer.InstallerFrame, com.izforge.izpack.installer.InstallData)s
     */
    public static HostConfigFrame getInstance() {
        if (instance == null) {
            throw new IllegalStateException("HostConfigFrame instance is null!");
        }

        return instance;
    }

    /**
     * Shows the window.
     * @param hosts The list of hosts to configure
     */
    public void open(HostList hosts) {
        setTitle(MessageFormat.format(getLabel("host.config.frame.title"), hosts.size()));

        parent.blockGUI();

        setComponents();

        setHosts(hosts);
        setFields();

        setVisible(true);
    }

    /**
     * Closes the window.
     */
    public void close() {
        setVisible(false);

        hosts = null;

        parent.releaseGUI();
    }

    /**
     * Sets the components visibility and resets their states.
     */
    private void setComponents() {
        isQmasterInst = parent.getRules().isConditionTrue(COND_INSTALL_QMASTER);
        isShadowdInst = parent.getRules().isConditionTrue(COND_INSTALL_SHADOWD);
        isExecdInst = parent.getRules().isConditionTrue(COND_INSTALL_EXECD);
        isBdbInst = parent.getRules().isConditionTrue(COND_INSTALL_BDB) ||
                idata.getVariable(VAR_SPOOLING_METHOD).equals(idata.getVariable(VAR_SPOOLING_METHOD_BERKELEYDBSERVER));
        isExpressInst = parent.getRules().isConditionTrue(COND_EXPRESS_INSTALL);

        // Shell we show these fields in express mode?
        jvmLibPathTextField.setVisible(!isExpressInst);
        jvmAddArgsTextField.setVisible(!isExpressInst);
        jvmLibPathLabel.setVisible(!isExpressInst);
        jvmAddArgsLabel.setVisible(!isExpressInst);
    }

    /**
     * Sets the fields.
     */
    public void setFields() {
        ArrayList<String> spoolDirs = new ArrayList<String>();
        ArrayList<String> jvmLibPaths = new ArrayList<String>();
        ArrayList<String> jvmAddArgss = new ArrayList<String>();
        ArrayList<String> connectUsers = new ArrayList<String>();
        ArrayList<Long> resolveTimeouts = new ArrayList<Long>();
        ArrayList<Long> installTimeouts = new ArrayList<Long>();

        boolean isThereQmaster = false;
        boolean isThereShadow = false;
        boolean isThereExecd = false;

        for (Host h : hosts) {
            if (h.isQmasterHost()) {
                isThereQmaster = true;
            }
            if (h.isShadowHost()) {
                isThereShadow = true;
            }
            if (h.isExecutionHost()) {
                isThereExecd = true;
            }

            if (!spoolDirs.contains(h.getSpoolDir())) {
                spoolDirs.add(h.getSpoolDir());
            }
            if (!jvmLibPaths.contains(h.getJvmLibPath())) {
                jvmLibPaths.add(h.getJvmLibPath());
            }
            if (!jvmAddArgss.contains(h.getJvmAddArgs())) {
                jvmAddArgss.add(h.getJvmAddArgs());
            }
            if (!connectUsers.contains(h.getConnectUser())) {
                connectUsers.add(h.getConnectUser());
            }
            if (!resolveTimeouts.contains(h.getResolveTimeout() / 1000)) {
                resolveTimeouts.add(h.getResolveTimeout() / 1000);
            }
            if (!installTimeouts.contains(h.getInstallTimeout() / 1000)) {
                installTimeouts.add(h.getInstallTimeout() / 1000);
            }
        }

        spoolDirLabel.setVisible(isThereExecd);
        spoolDirTextField.setVisible(isThereExecd);
        jvmLibPathTextField.setVisible(isThereQmaster || isThereShadow);
        jvmLibPathLabel.setVisible(isThereQmaster || isThereShadow);
        jvmAddArgsLabel.setVisible(isThereQmaster || isThereShadow);
        jvmAddArgsTextField.setVisible(isThereQmaster || isThereShadow);

        setField(spoolDirs, spoolDirTextField);
        setField(jvmLibPaths, jvmLibPathTextField);
        setField(jvmAddArgss, jvmAddArgsTextField);
        setField(connectUsers, connectUserTextField);
        setField(resolveTimeouts, resolveTimeoutTextField);
        setField(installTimeouts, installTimeoutTextField);
    }

    /**
     * Set a field with the given values.
     * @param values The values to set. In case of multiple values the field gets distinguished.
     * @param textField The field to set.
     */
    private void setField(ArrayList values, JTextField textField) {
        if (values.size() > 1) {
            setMultipleValues(textField);
        } else {
            unsetMultipleValues(textField);
            textField.setText(String.valueOf(values.get(0)));
        }
    }

    /**
     * Validates the typed values.
     * @return boolean True if all of the fields has valid value, false otherwise.
     */
    private boolean validateFields() {
        String message = "";

        // Local execd spool dir field
        if (spoolDirTextField.isVisible() && spoolDirTextField.getText().equals("")) {
            message = getLabel("cfg.exec.spool.dir.local.notemptyvalidator");
        }

        // JVM library path field if it's empty means auto detected
//        if (jvmLibPathTextField.isVisible() && jvmLibPathTextField.getText().equals("")) {
//            message = getLabel("cfg.sge.jvm.lib.path.notemptyvalidator");
//        }

        // Connect user field
        if (connectUserTextField.isVisible() && connectUserTextField.getText().equals("")) {
            message = getLabel("connect.user.notemptyvalidator");
        }

        // Resolve timeout field
        if (resolveTimeoutTextField.isVisible()) {
            if (resolveTimeoutTextField.getText().equals("")) {
                message = getLabel("resolve.timeout.notemptyvalidator");
            } else if (!resolveTimeoutTextField.getText().equals(multipleValuesString)) {
                try {
                    Long value = Long.valueOf(resolveTimeoutTextField.getText());

                    if (value < 1L) {
                        throw new NumberFormatException("Value has to be equal or bigger then 1!");
                    }
                } catch (NumberFormatException e) {
                    message = getLabel("resolve.timeout.invalidnumber");
                }
            }
        }

        // Install timeout field
        if (installTimeoutTextField.isVisible()) {
            if (installTimeoutTextField.getText().equals("")) {
                message = getLabel("install.timeout.notemptyvalidator");
            } else if (!installTimeoutTextField.getText().equals(multipleValuesString)) {
                try {
                    Long value = Long.valueOf(installTimeoutTextField.getText());

                    if (value < 1L) {
                        throw new NumberFormatException("Value has to be equal or bigger then 1!");
                    }
                } catch (NumberFormatException e) {
                    message = getLabel("install.timeout.invalidnumber");
                }
            }
        }

        // if we have a message set, show it and return false
        if (!message.equals("")) {
            message = new VariableSubstitutor(idata.getVariables()).substituteMultiple(message, null);

            JOptionPane.showOptionDialog(this, message, getLabel("installer.error"),
                    JOptionPane.CANCEL_OPTION, JOptionPane.ERROR_MESSAGE, null,
                    new Object[]{getLabel("installer.cancel")}, getLabel("installer.cancel"));

            return false;
        } else {
            return true;
        }
    }

    /**
     * Reads the fields and sets the red values to every applied hosts.
     */
    private void readFields() {
        for (Host h : hosts) {
            if (spoolDirTextField.isVisible() &&
                    !spoolDirTextField.getText().equals(multipleValuesString)) {
                h.setSpoolDir(spoolDirTextField.getText());
            }
            if (jvmLibPathTextField.isVisible() &&
                    !jvmLibPathTextField.getText().equals(multipleValuesString)) {
                h.setJvmLibPath(jvmLibPathTextField.getText());
            }
            if (jvmAddArgsTextField.isVisible() &&
                    !jvmAddArgsTextField.getText().equals(multipleValuesString)) {
                h.setJvmAddArgs(jvmAddArgsTextField.getText());
            }
            if (connectUserTextField.isVisible() &&
                    !connectUserTextField.getText().equals(multipleValuesString)) {
                h.setConnectUser(connectUserTextField.getText());
            }
            if (resolveTimeoutTextField.isVisible() &&
                    !resolveTimeoutTextField.getText().equals(multipleValuesString)) {
                h.setResolveTimeout(Long.valueOf(resolveTimeoutTextField.getText()).longValue() * 1000);
            }
            if (installTimeoutTextField.isVisible() &&
                    !installTimeoutTextField.getText().equals(multipleValuesString)) {
                h.setInstallTimeout(Long.valueOf(installTimeoutTextField.getText()).longValue() * 1000);
            }
        }
    }

    /**
     * Set the list host to be configured
     * @param hosts The list of host to use.
     */
    public void setHosts(HostList hosts) {
        if (hosts == null || hosts.size() == 0) {
            throw new IllegalArgumentException("Empty host list!");
        }

        this.hosts = hosts;
    }

    /**
     * Set the host to be configured
     * @param host The host to use.
     */
    public void setHost(Host host) {
        if (host == null) {
            throw new IllegalArgumentException("Null value!");
        }

        this.hosts = new HostList();
        this.hosts.add(host);
    }

    /**
     * Returns with localized text for the given key
     * @param key The key which identifies the localized text
     * @return The localized text if there is any. Empty string otherwise.
     */
    public String getLabel(String key) {
        String label = "";

        if (parent != null) {
            label = parent.langpack.getString(key);
        }

        return label;
    }

    /**
     * Returns with localized tooltip for the given key
     * @param key The key which identifies the localized tooltip
     * @return The localized tooltip if there is any. Empty string otherwise.
     */
    public String getTooltip(String key) {
        if (idata == null) {
            return null;
        }

        if (!key.endsWith(TOOLTIP)) {
            key = key + "." + TOOLTIP;
        }

        String tooltip = getLabel(key);

        VariableSubstitutor vs = new VariableSubstitutor(idata.getVariables());
        tooltip = vs.substituteMultiple(tooltip, null);

        if (tooltip.equals(key)) {
            tooltip = null;
        }

        return tooltip;
    }

    /**
     * Sets the field's to distinguish multiple values
     * @param textField The field to set
     */
    private void setMultipleValues(JTextField textField) {
        textField.setBackground(Color.LIGHT_GRAY);
        textField.setFont(MULTIPLEVALUES_TEXTFIELD_FONT);
        textField.setForeground(Color.red);
        textField.setText(multipleValuesString);
    }

    /**
     * Sets the field's to look normal
     * @param textField The field to set
     */
    private void unsetMultipleValues(JTextField textField) {
        if (textField.getFont() == MULTIPLEVALUES_TEXTFIELD_FONT) {
            textField.setBackground(DEF_BACKGROUND_COLOR);
            textField.setFont(DEF_TEXTFIELD_FONT);
            textField.setForeground(DEF_FOREGROUND_COLOR);
            textField.setText("");
        }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        spoolDirLabel = new javax.swing.JLabel();
        spoolDirTextField = new javax.swing.JTextField();
        jvmLibPathLabel = new javax.swing.JLabel();
        jvmLibPathTextField = new javax.swing.JTextField();
        jvmAddArgsLabel = new javax.swing.JLabel();
        connectUserLabel = new javax.swing.JLabel();
        resolveTimeoutLabel = new javax.swing.JLabel();
        installTimeoutLabel = new javax.swing.JLabel();
        jvmAddArgsTextField = new javax.swing.JTextField();
        connectUserTextField = new javax.swing.JTextField();
        resolveTimeoutTextField = new javax.swing.JTextField();
        installTimeoutTextField = new javax.swing.JTextField();
        cancelButton = new javax.swing.JButton();
        okButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setAlwaysOnTop(true);
        setResizable(false);

        spoolDirLabel.setText(getLabel("cfg.exec.spool.dir.local.label"));

        spoolDirTextField.setToolTipText(getTooltip("cfg.exec.spool.dir.local.tooltip"));
        spoolDirTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                spoolDirTextFieldFocusGained(evt);
            }
        });

        jvmLibPathLabel.setText(getLabel("cfg.sge.jvm.lib.path.label"));

        jvmLibPathTextField.setToolTipText(getTooltip("cfg.sge.jvm.lib.path.label.tooltip"));
        jvmLibPathTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                jvmLibPathTextFieldFocusGained(evt);
            }
        });

        jvmAddArgsLabel.setText(getLabel("cfg.sge.additional.jvm.args.label"));

        connectUserLabel.setText(getLabel("connect.user.label"));

        resolveTimeoutLabel.setText(getLabel("resolve.timeout.label"));

        installTimeoutLabel.setText(getLabel("install.timeout.label"));

        jvmAddArgsTextField.setToolTipText(getTooltip("cfg.sge.additional.jvm.args.label.tooltip"));
        jvmAddArgsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                jvmAddArgsTextFieldFocusGained(evt);
            }
        });

        connectUserTextField.setToolTipText(getTooltip("connect.user.tooltip"));
        connectUserTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                connectUserTextFieldFocusGained(evt);
            }
        });

        resolveTimeoutTextField.setToolTipText(getTooltip("resolve.timeout.tooltip"));
        resolveTimeoutTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                resolveTimeoutTextFieldFocusGained(evt);
            }
        });

        installTimeoutTextField.setToolTipText(getTooltip("install.timeout.tooltip"));
        installTimeoutTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                installTimeoutTextFieldFocusGained(evt);
            }
        });

        cancelButton.setText(getLabel("installer.cancel"));
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });

        okButton.setText(getLabel("button.ok.label"));
        okButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                okButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(spoolDirLabel)
                            .add(jvmLibPathLabel)
                            .add(jvmAddArgsLabel)
                            .add(connectUserLabel)
                            .add(installTimeoutLabel)
                            .add(resolveTimeoutLabel))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(spoolDirTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 408, Short.MAX_VALUE)
                            .add(connectUserTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 408, Short.MAX_VALUE)
                            .add(jvmAddArgsTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 408, Short.MAX_VALUE)
                            .add(jvmLibPathTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 408, Short.MAX_VALUE)
                            .add(resolveTimeoutTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 408, Short.MAX_VALUE)
                            .add(installTimeoutTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 408, Short.MAX_VALUE)))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                        .add(okButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(cancelButton)))
                .addContainerGap())
        );

        layout.linkSize(new java.awt.Component[] {cancelButton, okButton}, org.jdesktop.layout.GroupLayout.HORIZONTAL);

        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(spoolDirLabel)
                    .add(spoolDirTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jvmLibPathLabel)
                    .add(jvmLibPathTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jvmAddArgsLabel)
                    .add(jvmAddArgsTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(connectUserLabel)
                    .add(connectUserTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(8, 8, 8)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(resolveTimeoutLabel)
                    .add(resolveTimeoutTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(6, 6, 6)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(installTimeoutTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(installTimeoutLabel))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, okButton)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, cancelButton))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void spoolDirTextFieldFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_spoolDirTextFieldFocusGained
        unsetMultipleValues(spoolDirTextField);
    }//GEN-LAST:event_spoolDirTextFieldFocusGained

    private void jvmLibPathTextFieldFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_jvmLibPathTextFieldFocusGained
        unsetMultipleValues(jvmLibPathTextField);
    }//GEN-LAST:event_jvmLibPathTextFieldFocusGained

    private void jvmAddArgsTextFieldFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_jvmAddArgsTextFieldFocusGained
        unsetMultipleValues(jvmAddArgsTextField);
    }//GEN-LAST:event_jvmAddArgsTextFieldFocusGained

    private void connectUserTextFieldFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_connectUserTextFieldFocusGained
        unsetMultipleValues(connectUserTextField);
    }//GEN-LAST:event_connectUserTextFieldFocusGained

    private void resolveTimeoutTextFieldFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_resolveTimeoutTextFieldFocusGained
        unsetMultipleValues(resolveTimeoutTextField);
    }//GEN-LAST:event_resolveTimeoutTextFieldFocusGained

    private void installTimeoutTextFieldFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_installTimeoutTextFieldFocusGained
        unsetMultipleValues(installTimeoutTextField);
    }//GEN-LAST:event_installTimeoutTextFieldFocusGained

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        close();
    }//GEN-LAST:event_cancelButtonActionPerformed

    private void okButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_okButtonActionPerformed
        if (validateFields()) {
            readFields();
            close();
        }
    }//GEN-LAST:event_okButtonActionPerformed
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton cancelButton;
    private javax.swing.JLabel connectUserLabel;
    private javax.swing.JTextField connectUserTextField;
    private javax.swing.JLabel installTimeoutLabel;
    private javax.swing.JTextField installTimeoutTextField;
    private javax.swing.JLabel jvmAddArgsLabel;
    private javax.swing.JTextField jvmAddArgsTextField;
    private javax.swing.JLabel jvmLibPathLabel;
    private javax.swing.JTextField jvmLibPathTextField;
    private javax.swing.JButton okButton;
    private javax.swing.JLabel resolveTimeoutLabel;
    private javax.swing.JTextField resolveTimeoutTextField;
    private javax.swing.JLabel spoolDirLabel;
    private javax.swing.JTextField spoolDirTextField;
    // End of variables declaration//GEN-END:variables
}
