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
import java.io.File;
import java.util.Properties;

/**
 * Thread to check settings remotely on the specified host.
 */
public class ValidateHostTask extends TestableTask {

    private Host host = null;
    private Properties variables = null;
    private Host.State prevState = null;
    private TaskHandler handler;

    /**
     * Constructor
     * @param host The host which has to be validated
     * @param handler The {@link TaskHandler} which hanldes the result
     * @param variables The variables have to be substituted in check_host script
     */
    public ValidateHostTask(Host host, TaskHandler handler, Properties variables) {
        setTaskName("CheckHostTask - " + host);

        this.host = host;
        this.prevState = host.getState();
        this.handler = handler;
        this.variables = new Properties();
        this.variables.putAll(variables);
    }

    public void run() {
        long start = System.currentTimeMillis(), end;
        Debug.trace("Begin: Validate host '"+host+"'.");
        int exitValue = getTestExitValue();
        Host.State newState = prevState;

        handler.setHostState(host, Host.State.VALIDATING);

        if (isIsTestMode()) {
            handler.setHostState(host, prevState);
            return;
        }

        try {
            // Check port usage in java only if the host is the localhost
            if (host.isLocalhost()) {
                if (host.isExecutionHost()) {
                    // Check execd port usage
                    if (!Util.isPortFree(host.getIp(), variables.getProperty(VAR_SGE_EXECD_PORT))) {
                        handler.setHostState(host, Host.State.USED_EXECD_PORT);
                        return;
                    }
                }

                if (host.isQmasterHost()) {
                    // Check jmx port usage
                    if (variables.getProperty(VAR_SGE_JMX).equalsIgnoreCase("true") &&
                            !Util.isPortFree(host.getIp(), variables.getProperty(VAR_SGE_JMX_PORT))) {
                        handler.setHostState(host, Host.State.USED_JMX_PORT);
                        return;
                    }

                    // Check qmaster port usage
                    if (!Util.isPortFree(host.getIp(), variables.getProperty(VAR_SGE_QMASTER_PORT))) {
                        handler.setHostState(host, Host.State.USED_QMASTER_PORT);
                        return;
                    }
                }

                if (host.isBdbHost()) {
                }
            }

            VariableSubstitutor vs = new VariableSubstitutor(variables);

            // Fill up cfg.exec.spool.dir.local
            if (host.isExecutionHost() && !host.getSpoolDir().equals(variables.getProperty(VAR_EXECD_SPOOL_DIR))) {
                   variables.setProperty(VAR_EXECD_SPOOL_DIR_LOCAL, host.getSpoolDir());
            } else {
                variables.setProperty(VAR_EXECD_SPOOL_DIR_LOCAL, "");
            }

            // Fill up template file
            String checkHostTempFile = vs.substituteMultiple(variables.getProperty(VAR_CHECK_HOST_TEMP_FILE), null);
            String checkHostFile = vs.substituteMultiple(variables.getProperty(VAR_CHECK_HOST_FILE), null);
            String remoteFile = "";

            checkHostFile = "/tmp/" + checkHostFile + "." + host.getHostname() + ".tmp";
            Debug.trace("Generating check_host file: '" + checkHostFile + "'.");

            variables.put("host.arch", host.getArchitecture());

            checkHostFile = Util.fillUpTemplate(checkHostTempFile, checkHostFile, variables);
            remoteFile = checkHostFile.substring(0, checkHostFile.length() - 4);
            new File(checkHostFile).deleteOnExit();           

            Debug.trace("Copy auto_conf file to '" + host.getHostname() + ":" + checkHostFile + "'.");
            CopyExecutableCommand copyCmd = new CopyExecutableCommand(host.getResolveTimeout(), host.getHostname(), host.getConnectUser(),
                    variables.getProperty(VAR_SHELL_NAME, ""), (Util.IS_MODE_WINDOWS && host.getArchitecture().startsWith("win")), checkHostFile, remoteFile);
            copyCmd.execute();
            exitValue = copyCmd.getExitValue();
            if (exitValue == EXIT_VAL_CMDEXEC_TERMINATED) {
                //Set the log content
                newState = Host.State.COPY_TIMEOUT_CHECK_HOST;
                Debug.error("Timeout while copying the " + checkHostFile + " script to host " + host.getHostname() + " via " + variables.getProperty(VAR_COPY_COMMAND) + " command!\nMaybe a password is expected. Try the command in the terminal first.");
            } else if (exitValue == EXIT_VAL_CMDEXEC_INTERRUPTED) {
                //Set the log content
                newState = Host.State.CANCELED;
            } else if (exitValue != 0) {
                newState = Host.State.COPY_FAILED_CHECK_HOST;
            } else {
                RemoteComponentScriptCommand checkCmd = new RemoteComponentScriptCommand((2 * host.getResolveTimeout()), host, host.getConnectUser(), 
                        variables.getProperty(VAR_SHELL_NAME, ""), (Util.IS_MODE_WINDOWS && host.getArchitecture().startsWith("win")), remoteFile);
                checkCmd.execute();
                exitValue = checkCmd.getExitValue();

                // Set the new state of the host depending on the return value of the script
                switch (exitValue) {
                    case EXIT_VAL_SUCCESS: newState = Host.State.REACHABLE; break;
                    case EXIT_VAL_QMASTER_SPOOL_DIR_PERM_DENIED: newState = Host.State.PERM_QMASTER_SPOOL_DIR; break;
                    case EXIT_VAL_EXECD_SPOOL_DIR_PERM_DENIED: newState = Host.State.PERM_EXECD_SPOOL_DIR; break;
                    case EXIT_VAL_JMX_KEYSTORE_PERM_DENIED: newState = Host.State.PERM_JMX_KEYSTORE; break;
                    case EXIT_VAL_JVM_LIB_DOES_NOT_EXIST_QMASTER: newState = Host.State.JVM_LIB_MISSING; break;
                    case EXIT_VAL_JVM_LIB_INVALID_QMASTER: newState = Host.State.JVM_LIB_INVALID; break;
                    case EXIT_VAL_BDB_SPOOL_DIR_EXISTS: newState = Host.State.BDB_SPOOL_DIR_EXISTS; break;
                    case EXIT_VAL_BDB_SPOOL_WRONG_FSTYPE: newState = Host.State.BDB_SPOOL_DIR_WRONG_FSTYPE; break;
                    case EXIT_VAL_BDB_SPOOL_DIR_PERM_DENIED: newState = Host.State.PERM_BDB_SPOOL_DIR; break;
                    case EXIT_VAL_JVM_LIB_DOES_NOT_EXIST_SHADOWD: newState = Host.State.JVM_LIB_MISSING; break;
                    case EXIT_VAL_JVM_LIB_INVALID_SHADOWD: newState = Host.State.JVM_LIB_INVALID; break;
                    case EXIT_VAL_EXECD_SPOOL_DIR_LOCAL_PERM_DENIED: newState = Host.State.PERM_EXECD_SPOOL_DIR; break;                    
                    case EXIT_VAL_BDB_SERVER_SPOOL_DIR_PERM_DENIED: newState = Host.State.PERM_BDB_SPOOL_DIR; break;
                    case EXIT_VAL_BDB_SERVER_SPOOL_DIR_EXISTS: newState = Host.State.BDB_SPOOL_DIR_EXISTS; break;
                    case EXIT_VAL_ADMIN_USER_NOT_KNOWN: newState = Host.State.ADMIN_USER_NOT_KNOWN; break;
                    case EXIT_VAL_CMDEXEC_INTERRUPTED: newState = Host.State.CANCELED; break;
                    case EXIT_VAL_CMDEXEC_TERMINATED: newState = Host.State.OPERATION_TIMEOUT; break;
                    case EXIT_VAL_CMDEXEC_MISSING_FILE: newState = Host.State.MISSING_FILE; break;
                    default: {
                        Debug.error("Unknown exit code:" + exitValue);
                        newState = Host.State.UNKNOWN_ERROR;
                        break;
                    }
                }
            }
        } catch (InterruptedException e) {
            newState = Host.State.CANCELED;
        } catch (Exception e) {
            Debug.error("Failed to check host '" + host + "'. " + e);
            newState = Host.State.UNKNOWN_ERROR;
        } finally {
            handler.setHostState(host, newState);

            end = System.currentTimeMillis();
            Debug.trace("End [" + (end - start) + "ms]: Validate host '"+host+"'.");
        }
    }
}
