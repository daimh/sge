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
import com.sun.grid.installer.util.Util;
import com.sun.grid.installer.util.cmd.GetArchCommand;
import com.sun.grid.installer.util.cmd.ResolveHostCommand;
import java.net.UnknownHostException;
import java.util.Vector;
import java.util.concurrent.RejectedExecutionException;

/**
 * Determines the host's name and IP address and discovers the architecture.
 */
public class GetArchitectureTask extends TestableTask {
    private Host host;
    private TaskHandler handler;
    private String shell;
    private String sge_root;
    private String sge_qmaster_port;
    private String execd_spool_dir;

    /**
     * Constructor
     * @param host The host
     * @param handler The {@link TaskHandler} which hanldes the result
     * @param shell The shell to use during the connection
     * @param sge_root The SGE_ROOT value
     * @param sge_qmaster_port The SGE_QMASTER_PORT value
     * @param execd_spool_dir The EXECD_SPOOL_DIR value
     */
    public GetArchitectureTask(Host host, TaskHandler handler, String shell, String sge_root, String sge_qmaster_port, String execd_spool_dir) {
        setTaskName("GetArchTask - " + host);

        this.host = host;
        this.handler = handler;
        this.shell = shell;
        this.sge_root = sge_root;
        this.sge_qmaster_port = sge_qmaster_port;
        this.execd_spool_dir = execd_spool_dir;
    }

    public void run() {
        long start = System.currentTimeMillis(), end;
        Debug.trace("Begin: GetArchitecture host '"+host+"'.");
        String name, ip;
        handler.setHostState(host, Host.State.RESOLVING);
        String value = (host.getHostname().trim().length() == 0) ? host.getIp() : host.getHostname();
        Host.State state = Host.State.UNREACHABLE;
        
        try {
            int exitValue = getTestExitValue();
            Vector<String> output = getTestOutput();

            if (!isIsTestMode()) {
                ResolveHostCommand resolveHostCommand = new ResolveHostCommand(host.getResolveTimeout());
                resolveHostCommand.execute(value);
                exitValue = resolveHostCommand.getExitValue();

                name = resolveHostCommand.getHostName(); //If IP can't be resolved takes took long on windows
                ip = resolveHostCommand.getHostAddress();

                if (exitValue != EXIT_VAL_SUCCESS || ip.equals(name)) {
                    throw new UnknownHostException(ip);
                }

                //handler.setHostState(host, Host.State.RESOLVABLE);
                state = Host.State.RESOLVABLE;
                host.setHostname(name);
                host.setIp(ip);
            }

            handler.setHostState(host, Host.State.CONTACTING);

            if (!isIsTestMode()) {
                GetArchCommand cmd = new GetArchCommand(host.getResolveTimeout(), host.getHostname(), host.getConnectUser(),
                        shell, (Util.IS_MODE_WINDOWS && host.getArchitecture().startsWith("win")), sge_root);
                cmd.execute();
                exitValue = cmd.getExitValue();
                output = cmd.getOutput();
            }

            if (exitValue == 0 && output.size() > 0) {
                host.setArchitecture(output.get(0));
                state = Host.State.REACHABLE;
                //If windows host, need to set a LOCAL_SPOOL_DIR if empty!
                if (host.getArchitecture().startsWith("win") && host.getSpoolDir().equals(execd_spool_dir)) {
                    host.setSpoolDir(CONST_DEFAULT_WINDOWS_SPOOL_DIR + sge_qmaster_port);
                }
            } else if (exitValue == EXIT_VAL_CMDEXEC_MISSING_FILE) {
                state = Host.State.MISSING_FILE;
            } else if (exitValue == EXIT_VAL_CMDEXEC_INTERRUPTED) {
                state = Host.State.CANCELED;
            } else if (exitValue == EXIT_VAL_CMDEXEC_TERMINATED) {
                state = Host.State.OPERATION_TIMEOUT;
            }
            
        } catch (UnknownHostException e) {
            state = Host.State.UNKNOWN_HOST;
        } catch (RejectedExecutionException e) {
            state = Host.State.CANCELED;
        } finally {
            handler.setHostState(host, state);

            end = System.currentTimeMillis();
            Debug.trace("End [" + (end - start) + "ms]: GetArchitecture host '"+host+"'.");
        }
    }
}
