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
package com.sun.grid.installer.task;

import com.sun.grid.installer.gui.*;
import com.izforge.izpack.util.Debug;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.Util;
import com.sun.grid.installer.util.cmd.CopyExecutableCommand;
import com.sun.grid.installer.util.cmd.RemoteComponentScriptCommand;
import java.util.Properties;
import java.io.File;

/**
 * Installs the host with the specified components
 */
public class InstallTask extends TestableTask {

    private Host host;
    private Properties variables, msgs;
    private TaskHandler handler = null;

    /**
     * Constructor
     * @param host The host to install
     * @param handler The {@link TaskHandler} which hanldes the result
     * @param variables The variables have to be substituted in check_host script
     * @param msgs The localized massages
     */
    public InstallTask(Host host, TaskHandler handler, Properties variables, final Properties msgs) {
        setTaskName("InstallTask - " + host);

        this.host = host;
        this.handler = handler;
        this.variables = variables;
        this.variables.put("host.arch", host.getArchitecture());
        this.msgs = msgs;
    }

    public void run() {
        long start = System.currentTimeMillis(), end;
        Debug.trace("Begin: Install host '"+host+"'.");

        Host.State state = Host.State.UNKNOWN_ERROR;
        handler.setHostState(host, Host.State.PROCESSING);

        RemoteComponentScriptCommand installCmd = null;
        CopyExecutableCommand copyCmd = null;
        
        boolean isSpecialTask = Boolean.valueOf(variables.getProperty(VAR_FIRST_TASK, "false")) || Boolean.valueOf(variables.getProperty(VAR_LAST_TASK, "false"));
        VariableSubstitutor vs  = new VariableSubstitutor(variables);
        String log = "";

        try {
            String autoConfFile = null;
            String remoteFile = "";
            int exitValue;

            if (host.isBdbHost() || host.isQmasterHost() || host.isShadowHost() || host.isExecutionHost() || isSpecialTask) {
                if (!isSpecialTask) {
                    if (host.isQmasterHost()) {
                        // shadow host installation  is done in separate task
                        variables.put(VAR_SHADOW_HOST_LIST, "");
                    }

                    // Fill up cfg.exec.spool.dir.local
                    if (host.isExecutionHost() && !host.getSpoolDir().equals(variables.getProperty(VAR_EXECD_SPOOL_DIR))) {
                        variables.setProperty(VAR_EXECD_SPOOL_DIR_LOCAL, host.getSpoolDir());
                    } else {
                        variables.setProperty(VAR_EXECD_SPOOL_DIR_LOCAL, "");
                    }
                }

                String autoConfTempFile = vs.substituteMultiple(variables.getProperty(VAR_AUTO_INSTALL_COMPONENT_TEMP_FILE), null);
                autoConfFile = vs.substituteMultiple(variables.getProperty(VAR_AUTO_INSTALL_COMPONENT_FILE), null);

                //Appended CELL_NAME prevents a race in case of parallel multiple cell installations
                String taskName = isSpecialTask == true ? (Boolean.valueOf(variables.getProperty(VAR_FIRST_TASK, "false")).booleanValue() == true ? "first_task" : "last_task") : host.getComponentString();
                autoConfFile = "/tmp/" + autoConfFile + "." + host.getHostname() + "." + taskName + "." + variables.getProperty(VAR_SGE_CELL_NAME) + ".tmp";                
                Debug.trace("Generating auto_conf file: '" + autoConfFile + "'.");
                autoConfFile = Util.fillUpTemplate(autoConfTempFile, autoConfFile, variables);
                remoteFile = autoConfFile.substring(0, autoConfFile.length() - 4);
                new File(autoConfFile).deleteOnExit();
            }

            if (!isIsTestMode()) {
                /**
                 * Copy install_component script
                 */
                Debug.trace("Copy auto_conf file to '" + host.getHostname() + ":" + autoConfFile + "'.");
                copyCmd = new CopyExecutableCommand(host.getResolveTimeout(), host.getHostname(), host.getConnectUser(),
                        variables.getProperty(VAR_SHELL_NAME, ""), (Util.IS_MODE_WINDOWS && host.getArchitecture().startsWith("win")), autoConfFile, remoteFile);
                copyCmd.execute();
                exitValue = copyCmd.getExitValue();
            } else {
                log = Util.listToString(getTestOutput());
                exitValue = getTestExitValue();
            }

            if (exitValue == EXIT_VAL_CMDEXEC_TERMINATED) {
                state = Host.State.COPY_TIMEOUT_INSTALL_COMPONENT;
                if (copyCmd != null) {
                    copyCmd.setFirstLogMessage("Timeout while copying the " + autoConfFile + " script to host " + host.getHostname() + " via " + variables.getProperty(VAR_COPY_COMMAND) + " command!\nMaybe a password is expected. Try the command in the terminal first.");
                    log = copyCmd.generateLog(msgs);
                }
            } else if (exitValue == EXIT_VAL_CMDEXEC_INTERRUPTED) {
                state = Host.State.CANCELED;
                if (copyCmd != null) {
                    copyCmd.setFirstLogMessage("Cancelled copy action!");
                    log = copyCmd.generateLog(msgs);
                }
            } else if (exitValue != 0) {
                state = Host.State.COPY_FAILED_INSTALL_COMPONENT;
                if (copyCmd != null) {
                    copyCmd.setFirstLogMessage("Error when copying the file " + autoConfFile + "to host " + host.getHostname() + " via " + variables.getProperty(VAR_COPY_COMMAND) + " command!\n");
                    log = copyCmd.generateLog(msgs);
                }
            } else {
                /**
                 * Execute installation
                 */
                long timeout = host.getInstallTimeout();
                if (host.isFirstTask()) {
                    String cspHosts = variables.getProperty(VAR_ALL_CSPHOSTS);
                    int cspCount = (cspHosts == null) ? 0 : cspHosts.split(" ").length;
                    if (cspCount > 2) {
                        timeout += 30000 * cspCount; //We add 30sec to the timeout for every host we need to copy the CSP certificates to
                    }
                }

                if (!isIsTestMode()) {
                    installCmd = new RemoteComponentScriptCommand(timeout, host, host.getConnectUser(), 
                            variables.getProperty(VAR_SHELL_NAME, ""), (Util.IS_MODE_WINDOWS && host.getArchitecture().startsWith("win")), remoteFile);
                    installCmd.execute();
                    exitValue = installCmd.getExitValue();
                }

                if (exitValue == 0) {
                    state = Host.State.SUCCESS;
                } else if (exitValue == EXIT_VAL_CMDEXEC_INTERRUPTED) {
                    state = Host.State.CANCELED;
                    if (installCmd != null) {
                        installCmd.setFirstLogMessage("CANCELED: Task has been canceled by the user.");
                    }
                } else if (exitValue == EXIT_VAL_FAILED_ALREADY_INSTALLED_COMPONENT) {
                    state = Host.State.FAILED_ALREADY_INSTALLED_COMPONENT;
                } else if (exitValue == EXIT_VAL_CMDEXEC_TERMINATED) {
                    state = Host.State.OPERATION_TIMEOUT;
                } else {
                    state = Host.State.FAILED;
                }
            }
        } catch (InterruptedException e) {
            state = Host.State.CANCELED;
            if (installCmd != null) {
                installCmd.setFirstLogMessage("CANCELED: Task has been canceled by the user.");
            }
        } catch (Exception e) {
            state = Host.State.FAILED;
            if (installCmd != null) {
                installCmd.setFirstLogMessage("Message was " + e.getMessage());
            }
        } finally {
            if (installCmd != null) {
                log = installCmd.generateLog(msgs);
            }
            //Unset the special variables
            variables.remove(VAR_FIRST_TASK);
            variables.remove(VAR_LAST_TASK);
            variables.remove(VAR_ALL_ADMIN_HOSTS);
            variables.remove(VAR_ALL_SUBMIT_HOSTS);
            variables.remove(VAR_ALL_CSPHOSTS);

            handler.setHostState(host, state);
            handler.setHostLog(host, log);

            end = System.currentTimeMillis();
            Debug.trace("End [" + (end - start) + "ms]: Install host '"+host+"'.");
        }
    }
}
