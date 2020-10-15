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
import com.izforge.izpack.util.Debug;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.ExtendedFile;
import com.sun.grid.installer.util.Util;
import com.sun.grid.installer.util.cmd.GetArchCommand;
import com.sun.grid.installer.util.cmd.GetJvmLibCommand;
import java.util.Enumeration;
import java.util.Properties;
import javax.swing.JOptionPane;

public class IntermediateActionPanel extends ActionPanel {

    public IntermediateActionPanel(InstallerFrame parent, InstallData idata) {
        super(parent, idata);

        setNumOfMaxExecution(Integer.MAX_VALUE);
    }

    @Override
    public void doAction() {
        initializeVariables(false);

    }

    protected boolean initializeVariables(boolean silentMode) {
        boolean isQmasterInst, isShadowdInst, isExecdInst, isBdbInst, isJmxEnabled;
        isQmasterInst = isValueEqualsTrue(idata, VAR_INSTALL_QMASTER);
        isShadowdInst = isValueEqualsTrue(idata, VAR_INSTALL_SHADOW);
        isExecdInst = isValueEqualsTrue(idata, VAR_INSTALL_EXECD);
        isJmxEnabled = isValueEqualsTrue(idata, VAR_SGE_JMX);
        isBdbInst = isValueEqualsTrue(idata, VAR_INSTALL_BDB);

        VariableSubstitutor vs = new VariableSubstitutor(idata.getVariables());

        // Only once...
        if (getNumOfExecution() == 0) {
            // Localhost arch
            Properties variables = idata.getVariables();
            GetArchCommand archCmd = new GetArchCommand(Host.localHostName, Util.DEF_CONNECT_USER, 
                    variables.getProperty(VAR_SHELL_NAME, ""), (Util.IS_MODE_WINDOWS && Host.localHostArch.startsWith("win")), variables.getProperty(VAR_SGE_ROOT, ""));
            archCmd.execute();
            if (archCmd.getExitValue() == 0 && archCmd.getOutput().size() > 0) {
                Host.localHostArch = archCmd.getOutput().get(0).trim();
                idata.setVariable(VAR_LOCALHOST_ARCH, Host.localHostArch);
                Debug.trace("localhost.arch='" + idata.getVariable(VAR_LOCALHOST_ARCH) + "'");
            }

            // Check user
            if (vs.substituteMultiple(idata.getVariable(VAR_CONNECT_USER), null).equals(vs.substituteMultiple(idata.getVariable(VAR_USER_NAME), null))) {
                String sgeRoot = vs.substitute(idata.getVariable(VAR_SGE_ROOT), null);
                String userName = vs.substitute(idata.getVariable(VAR_USER_NAME), null);

                ExtendedFile sgeRootDir = new ExtendedFile(sgeRoot);

                Debug.trace(sgeRootDir.getPermissions() + " " + sgeRootDir.getOwner() + " " + sgeRootDir.getGroup() + " " + sgeRoot);

                if (!userName.equals("root")) {
                    if (userName.equals(sgeRootDir.getOwner())) {
                        Debug.trace("You are not installing as user 'root'! This will allow you to run Grid Engine only under your user id for testing a limited functionality of Grid Engine!");
                        if (!silentMode && JOptionPane.NO_OPTION == JOptionPane.showOptionDialog(parent, vs.substituteMultiple(idata.langpack.getString(WARNING_USER_NOT_ROOT), null),
                                idata.langpack.getString("installer.warning"), JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE, null,
                                new Object[]{idata.langpack.getString("installer.yes"), idata.langpack.getString("installer.no")}, idata.langpack.getString("installer.yes"))) {
                            parent.exit(true);
                        }
                    } else {
                        Debug.error("You are not installing as user 'root' or as the owner of the Grid Engine root directory. You don't have enough permission to perform installation. The installation terminates!");
                        if (silentMode) {
                            return false;
                        } else {
                            JOptionPane.showOptionDialog(parent, vs.substituteMultiple(idata.langpack.getString(ERROR_USER_INVALID), null),
                                    idata.langpack.getString("installer.error"), JOptionPane.CANCEL_OPTION, JOptionPane.ERROR_MESSAGE, null,
                                    new Object[]{idata.langpack.getString("button.exit.label")}, idata.langpack.getString("button.exit.label"));

                            parent.exit(true);
                        }
                    }
                }
            }

            // Set SGE_JMX_PORT to SGE_QMASTER_PORT + 2
            idata.setVariable(VAR_SGE_JMX_PORT, Integer.toString(Integer.valueOf(idata.getVariable(VAR_SGE_QMASTER_PORT)) + 2));
        }

        // Set value for 'cond.qmaster.on.localhost' condition
        Host.IS_QMASTER_ON_LOCALHOST = Host.localHostName.equalsIgnoreCase(idata.getVariable(VAR_QMASTER_HOST)) ||
                Host.localHostIP.equalsIgnoreCase(idata.getVariable(VAR_QMASTER_HOST));

        // cfg.spooling.method
        if (isBdbInst) {
            idata.setVariable(VAR_SPOOLING_METHOD, vs.substituteMultiple(idata.getVariable(VAR_SPOOLING_METHOD_BERKELEYDBSERVER), null));
        } else if (getNumOfExecution() == 0) {
            idata.setVariable(VAR_SPOOLING_METHOD, vs.substituteMultiple(idata.getVariable(VAR_SPOOLING_METHOD_BERKELEYDB), null));
        }

        // Detect Jvm Library if on qmaster host
        if (isJmxEnabled && (isQmasterInst || isShadowdInst)) {
            String sgeRoot = idata.getVariable(VAR_SGE_ROOT);
            String remHost = (isQmasterInst) ? idata.getVariable(VAR_QMASTER_HOST) : Host.localHostName;
            String libjvm="";

            //TODO: Verify behavior of delay and when connection is not possible via rsh
            GetJvmLibCommand cmd = new GetJvmLibCommand(remHost, Util.DEF_CONNECT_USER, idata.getVariable(VAR_SHELL_NAME), Util.IS_MODE_WINDOWS, sgeRoot);
            cmd.execute();
            if (cmd.getOutput() != null && cmd.getOutput().size() > 0) {
                libjvm = cmd.getOutput().get(0);
            }
            idata.setVariable(VAR_JVM_LIB_PATH, libjvm);
        }

        // In case of execd and/or shadow host standalone installation source the settings.sh boostrap and act_qmaster files
        if (!isQmasterInst && (isExecdInst || isShadowdInst)) {
            /**
             * Source settings
             */
            try {
                Properties settingsProps = Util.sourceSGESettings(idata.getVariable(VAR_SGE_ROOT), idata.getVariable(VAR_SGE_CELL_NAME));
                
                String origKey = "";
                String newKey = "";
                for (Enumeration<Object> enumer = settingsProps.keys(); enumer.hasMoreElements();) {
                    origKey = (String) enumer.nextElement();
                    newKey = CONFIG_VAR_PREFIX + "." + origKey.toLowerCase().replace('_', '.');
                    idata.setVariable(newKey, settingsProps.getProperty(origKey));
                }
            } catch (Exception ex) {
                Debug.error("Can not source 'settings.sh' file! " + ex);
            }

            /**
             * Source bootstrap
             */
            boolean jmxEnabled = false;
            try {
                Properties bootstrapProps = Util.sourceSGEBootstrap(idata.getVariable(VAR_SGE_ROOT), idata.getVariable(VAR_SGE_CELL_NAME));

                idata.setVariable(VAR_ADMIN_USER, bootstrapProps.getProperty("admin_user", idata.getVariable(VAR_ADMIN_USER)));
                idata.setVariable(VAR_PRODUCT_MODE, bootstrapProps.getProperty("add.product.mode")); //To correctly set CSP mode in execd only installs
                idata.setVariable(VAR_QMASTER_SPOOL_DIR, bootstrapProps.getProperty("qmaster_spool_dir", idata.getVariable(VAR_QMASTER_SPOOL_DIR)));

                jmxEnabled = !bootstrapProps.getProperty("jvm_threads").equals("0");
            } catch (Exception ex) {
                Debug.error("Can not source 'bootstrap' file! " + ex);
            }

            /**
             * Discover qmaster host
             */
            try {
                String qmasterHost = Util.getQmasterHost(idata.getVariable(VAR_SGE_ROOT), idata.getVariable(VAR_SGE_CELL_NAME));
                
                idata.setVariable(VAR_QMASTER_HOST, qmasterHost);
            } catch (Exception ex) {
                Debug.error("Can not source 'act_qmaster' file! " + ex);
            }

//            if (isShadowdInst) {
//              /**
//                 * Read JMX specific settings
//                 */
//                For standalone shadowd installtion the install scripts sources
//                the management.properties file (permissions will be changed on the file)
//                idata.setVariable(VAR_SGE_JMX, (jmxEnabled ? "true" : "false"));
//                if (jmxEnabled) {
//                    try {
//                        Properties jmxProps = Util.sourceJMXSettings(idata.getVariable(VAR_SGE_ROOT), idata.getVariable(VAR_SGE_CELL_NAME));
//
//                        idata.setVariable(VAR_SGE_JMX_PORT, jmxProps.getProperty("com.sun.grid.jgdi.management.jmxremote.port"));
//                        idata.setVariable(VAR_JMX_SSL, jmxProps.getProperty("com.sun.grid.jgdi.management.jmxremote.ssl"));
//                        idata.setVariable(VAR_JMX_SSL_CLIENT, jmxProps.getProperty("com.sun.grid.jgdi.management.jmxremote.ssl.need.client.auth"));
//                        idata.setVariable(VAR_JMX_SSL_KEYSTORE, jmxProps.getProperty("com.sun.grid.jgdi.management.jmxremote.ssl.serverKeystore"));
//                        idata.setVariable(VAR_JMX_SSL_KEYSTORE_PWD, jmxProps.getProperty("com.sun.grid.jgdi.management.jmxremote.ssl.serverKeystorePassword"));
//                    } catch (Exception ex) {
//                        Debug.error("Can not source 'management.properties' file! " + ex);
//                    }
//                }
//            }
        }

        // set cfg.sge.enable.smf and cfg.remove.rc
        idata.setVariable(VAR_SGE_ENABLE_SMF, idata.getVariable(VAR_ADD_TO_RC));
        idata.setVariable(VAR_REMOVE_RC, idata.getVariable(VAR_ADD_TO_RC));

        return true;
    }

    public boolean isValueEqualsTrue(InstallData idata, String key) {
        return idata.getVariable(key).equalsIgnoreCase("true");
    }
}
