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

import com.izforge.izpack.util.Debug;
import com.sun.grid.installer.util.Config;
import com.sun.grid.installer.util.Util;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

public class Host implements Config {
    public static String localHostName;
    public static String localHostIP;
    public static String localHostArch;

    // Value for 'cond.qmaster.on.localhost' condition depends on the value of 'add.qmaster.host'
    public static boolean IS_QMASTER_ON_LOCALHOST = true;
    
    public static final String HOST_TYPE_QMASTER = "qmaster";
    public static final String HOST_TYPE_EXECD   = "execd";
    public static final String HOST_TYPE_SHADOWD = "shadow";
    public static final String HOST_TYPE_BDB     = "bdb";
    public static final String HOST_TYPE_SUBMIT  = "submit";
    public static final String HOST_TYPE_ADMIN   = "admin";
    public static final String HOST_TYPE_ALL     = "all hosts";

    public static final String SEPARATOR = ",";
    public static final String ARG_SPOOLDIR = "spooldir";
    public static final String ARG_CONNECT_USER = "connectuser";
    public static final String ARG_JVM_LIB_PATH = "jvmlibpath";
    public static final String ARG_JVM_ADD_ARGS = "jvmaddargs";

    static {
        try {
            localHostName = InetAddress.getLocalHost().getHostName();
            localHostIP = InetAddress.getLocalHost().getHostAddress();
        } catch (UnknownHostException ex) {
            Debug.error("Unable to discover localhost! " + ex);
            localHostName = "";
            localHostIP = "";
        }
    }

    public enum State {
        NEW_UNKNOWN_HOST,
        RESOLVING,
        RESOLVABLE,
        MISSING_FILE,
        UNKNOWN_HOST,
        CONTACTING,
        VALIDATING,
        REACHABLE,
        UNREACHABLE,
        OK,
        OPERATION_TIMEOUT,
        COPY_TIMEOUT_CHECK_HOST,
        COPY_FAILED_CHECK_HOST,
        READY_TO_INSTALL,
        COPY_TIMEOUT_INSTALL_COMPONENT,
        COPY_FAILED_INSTALL_COMPONENT,
        PROCESSING,
        SUCCESS,
        CANCELED,
        FAILED,
        FAILED_ALREADY_INSTALLED_COMPONENT, // not used
        FAILED_DEPENDENT_ON_PREVIOUS,
        PERM_QMASTER_SPOOL_DIR,
        PERM_EXECD_SPOOL_DIR,
        PERM_BDB_SPOOL_DIR,
        BDB_SPOOL_DIR_EXISTS,
        BDB_SPOOL_DIR_WRONG_FSTYPE,
        ADMIN_USER_NOT_KNOWN,
        PERM_JMX_KEYSTORE,
        JVM_LIB_MISSING,
        JVM_LIB_INVALID,
        USED_QMASTER_PORT,
        USED_EXECD_PORT,
        USED_JMX_PORT,
        UNKNOWN_ERROR;
        
        public static Properties localizedTexts = new Properties();

        @Override
        public String toString() {
            String text = (LANGID_PREFIX_STATE + "." + name()).toLowerCase();

            if (localizedTexts.containsKey(text)) {
                text = localizedTexts.getProperty(text);
            } else {
                text = name();
            }

            return text;
        }
        
        public String getTooltip() {
            String text = (LANGID_PREFIX_STATE + "." + name() + ".tooltip").toLowerCase();

            if (localizedTexts.containsKey(text)) {
                text = localizedTexts.getProperty(text);
            } else {
                text = toString();
            }

            return text;
        }
    }
    
    public enum Type { HOSTNAME, IP}    

    private String hostname = "";
    private String displayName = "";
    private String ip = "";
    private String architecture = "";
    private String spoolDir = "";
    private String connectUser = "";
    private String jvmLibPath = "";
    private String jvmAddArgs = "";
    private String log = "";
    private long   resolveTimeout = Util.DEF_RESOLVE_TIMEOUT;
    private long   installTimeout = Util.DEF_INSTALL_TIMEOUT;
    private State state = State.UNKNOWN_HOST;
    private boolean bdbHost, qmasterHost, shadowHost, executionHost, adminHost, submitHost, firstTask, lastTask;

    public Host(Host h) {
        hostname = new String(h.getHostname());
        ip = new String(h.getIp());
        architecture = new String(h.getArchitecture());
        spoolDir = new String(h.getSpoolDir());
        state = h.getState();
        shadowHost = h.isShadowHost();
        executionHost = h.isExecutionHost();
        adminHost = h.isAdminHost();
        submitHost = h.isSubmitHost();
        bdbHost = h.isBdbHost();
        qmasterHost = h.isQmasterHost();

        connectUser = h.getConnectUser();
        jvmLibPath  = h.getJvmLibPath();
        jvmAddArgs  = h.getJvmAddArgs();

        resolveTimeout = h.getResolveTimeout();
        installTimeout = h.getInstallTimeout();

        checkArchDependencies();
    }

    //Default constructor for selected hosts
    public Host (Host.Type type, String value, boolean isShadowHost, boolean isExecutionHost, boolean isAdminHost, boolean isSubmitHost, String execdSpoolDir) {
        this(type, value, false, false, isShadowHost, isExecutionHost, isAdminHost, isSubmitHost, false, false, execdSpoolDir, State.NEW_UNKNOWN_HOST);
    }

    public Host (Host.Type type, String value, String displayName, boolean isFirstTask, boolean isLastTask) {
        this(type, value, false, false, false, false, false, false, isFirstTask, isLastTask, "", State.READY_TO_INSTALL);
        this.displayName = displayName;
    }

    public Host (Host.Type type, String value, boolean isQmasterHost, boolean isBdbHost, boolean isShadowHost, boolean isExecutionHost, boolean isAdminHost, boolean isSubmitHost, String execdSpoolDir, Host.State state) {
        this(type, value, isQmasterHost, isBdbHost, isShadowHost, isExecutionHost, isAdminHost, isSubmitHost, false, false, execdSpoolDir, state);
    }

    private Host (Host.Type type, String value, boolean isQmasterHost, boolean isBdbHost, boolean isShadowHost, boolean isExecutionHost, boolean isAdminHost, boolean isSubmitHost, boolean isFirstTask, boolean isLastTask, String execdSpoolDir, Host.State state) {
        switch (type) {
            case HOSTNAME:
                this.hostname = value;
                this.ip = "";
                break;
            case IP:
                this.hostname = "";
                this.ip = value;
                break;
            default:
                throw new IllegalArgumentException("Unknown type: "+type.toString());
        }
        this.qmasterHost = isQmasterHost;
        this.bdbHost = isBdbHost;
        this.adminHost = isAdminHost;
        this.submitHost = isSubmitHost;
        this.shadowHost = isShadowHost;
        this.executionHost = isExecutionHost;
        this.firstTask = isFirstTask;
        this.lastTask = isLastTask;
        this.spoolDir = execdSpoolDir;
        this.state = state;

        checkArchDependencies();
    }
    
    public boolean[] getType() {
    	return new boolean[]{qmasterHost, executionHost, shadowHost, adminHost, submitHost, bdbHost};
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof Host) {
            Host h = (Host) o;
            //Special handling for special tasks
            if ((h.isFirstTask() && !this.isFirstTask()) || (!h.isFirstTask() && this.isFirstTask())) {
                return false;
            }
            if ((h.isLastTask() && !this.isLastTask()) || (!h.isLastTask() && this.isLastTask())) {
                return false;
            }
            //Compare hostnames if either host does not have a hostname
            if (this.getHostname().length() > 0 && h.getHostname().length() > 0) {
                return this.getHostname().equalsIgnoreCase(h.getHostname());
            //Else compare IPs - solves an issue when host is added first by IP, resoves to hostname and is added again by IP
            } else {
                return this.getIp().equals(h.getIp());
            }
        }
        
        return false;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 17 * hash + (this.hostname != null ? this.hostname.hashCode() : 0);
        hash = 17 * hash + (this.ip != null ? this.ip.hashCode() : 0);
        return hash;
    }

    @Override
    public String toString() {
        String string = hostname + "(" + ip + ") " + architecture + "-" + getComponentString();
        if (isExecutionHost() && !spoolDir.equals("")) {
            string += " ExecSpoolDir='" + spoolDir + "'";
        }
        if (log != null && !log.equals("")) {
            string += " Log='" + log + "'";
        }
        return string;
    }

    /**
     * Creates a string representation from this host instance.
     * @return The fingerprint of this instance.
     *
     * @see Host#SEPARATOR
     * @see Host#fromStringInstance(java.lang.String)
     */
    public String toStringInstance() {
        String instance = "";

        if (!hostname.equals("")) {
            instance += hostname + SEPARATOR;
        } else {
            instance += ip + SEPARATOR;
        }
        
        instance += ARG_SPOOLDIR + "=" + spoolDir + SEPARATOR;
        instance += ARG_CONNECT_MODE + "=" + connectUser + SEPARATOR;
        instance += ARG_JVM_LIB_PATH + "=" + jvmLibPath + SEPARATOR;
        instance += ARG_JVM_ADD_ARGS + "=" + jvmAddArgs + SEPARATOR;
        
        if (isQmasterHost()) {
            instance += Util.SgeComponents.qmaster + SEPARATOR;
        }
        if (isExecutionHost()) {
            instance += Util.SgeComponents.execd + SEPARATOR;
        }
        if (isShadowHost()) {
            instance += Util.SgeComponents.shadow + SEPARATOR;
        }
        if (isBdbHost()) {
            instance += Util.SgeComponents.bdb + SEPARATOR;
        }
        if (isAdminHost()) {
            instance += Util.SgeComponents.admin + SEPARATOR;
        }
        if (isSubmitHost()) {
            instance += Util.SgeComponents.submit + SEPARATOR;
        }

        // Remove last separator
        if (instance.endsWith(SEPARATOR)) {
            instance = instance.substring(0, instance.length() - 1);
        }

        return instance;
    }

    /**
     * Instantiates hosts from arguements. The record has to contain the host id (ip or name) at first place and may
     * contains list of components and value of execution spool directory.
     * @param instance the record which holds necessary arguments for the instantiation.
     *
     * @return list of hosts parsed from the record
     * 
     * @see Host#SEPARATOR
     * @see Host#toStringInstance()
     * @throws java.lang.IllegalArgumentException
     */
    public static List<Host> fromStringInstance(String instance) throws IllegalArgumentException {
        if (instance.equals("")) {
            throw new IllegalArgumentException("Missing fields!");
        }

        // Split up the arguments
        String[] args = instance.split(SEPARATOR);

        // The length of the argum list is unimportan as we are searching for specific
        // named arguments
//        if (args.length > 8) {
//            throw new IllegalArgumentException("Too many arguments: " + args.length);
//        }

        boolean isQmaster = false;
        boolean isExecd = false;
        boolean isShadow = false;
        boolean isBDB = false;
        boolean isAdmin = false;
        boolean isSubmit = false;
        String spoolDir = "";
        String connectUser = "";
        String jvmLibPath = "";
        String jvmAddArgs = "";

        // Create the list of ids
        Type type = Util.isIpPattern(args[0]) ? Type.IP : Type.HOSTNAME;
        List<String> ids = Util.parsePattern(args[0], type);

        Util.validateHostIDList(ids, type);

        // Read the arguments
        String arg;
        for (int i = 1; i < args.length; i++) {
            arg = args[i].trim();

            if (arg.equalsIgnoreCase(Util.SgeComponents.qmaster.toString())) {
                isQmaster = true;
            } else if (arg.equalsIgnoreCase(Util.SgeComponents.execd.toString())) {
                isExecd = true;
            } else if (arg.equalsIgnoreCase(Util.SgeComponents.shadow.toString())) {
                isShadow = true;
            } else if (arg.equalsIgnoreCase(Util.SgeComponents.bdb.toString())) {
                isBDB = true;
            } else if (arg.equalsIgnoreCase(Util.SgeComponents.submit.toString())) {
                isSubmit = true;
            } else if (arg.equalsIgnoreCase(Util.SgeComponents.admin.toString())) {
                isAdmin = true;
            } else if (arg.startsWith(ARG_SPOOLDIR)) {
                if (arg.indexOf("=") > -1 && arg.indexOf("=") < arg.length()) {
                    arg = arg.substring(arg.indexOf("=") + 1);
                    spoolDir = arg;
                }
            } else if (arg.startsWith(ARG_CONNECT_USER)) {
                if (arg.indexOf("=") > -1 && arg.indexOf("=") < arg.length()) {
                    arg = arg.substring(arg.indexOf("=") + 1);
                    connectUser = arg;
            }
            } else if (arg.startsWith(ARG_JVM_LIB_PATH)) {
                if (arg.indexOf("=") > -1 && arg.indexOf("=") < arg.length()) {
                    arg = arg.substring(arg.indexOf("=") + 1);
                    jvmLibPath = arg;
        }
            } else if (arg.startsWith(ARG_JVM_ADD_ARGS)) {
                if (arg.indexOf("=") > -1 && arg.indexOf("=") < arg.length()) {
                    arg = arg.substring(arg.indexOf("=") + 1);
                    jvmAddArgs = arg;
                }
            }
        }

        // Create host instances
        List<Host> hosts = new ArrayList<Host>(ids.size());
        for (String id : ids) {
            Host h = new Host(type, id, isQmaster, isBDB, isShadow, isExecd, isAdmin, isSubmit, spoolDir, State.NEW_UNKNOWN_HOST);
            h.setConnectUser(connectUser);
            h.setJvmLibPath(jvmLibPath);
            h.setJvmAddArgs(jvmAddArgs);
            hosts.add(h);
        }

        return hosts;
    }

    public String getSpoolDir() {
        return spoolDir;
    }

    public void setSpoolDir(String execdSpoolDir) {
        this.spoolDir = execdSpoolDir;
    }

    public String getArchitecture() {
        return architecture;
    }

    public void setArchitecture(String architecture) {
        this.architecture = architecture;

        checkArchDependencies();
    }

        /**
     * @return the bdbHost
     */
    public boolean isBdbHost() {
        return bdbHost;
    }

    /**
     * @param bdbHost the bdbHost to set
     */
    public void setBdbHost(boolean bdbHost) {
        this.bdbHost = bdbHost;

        checkArchDependencies();
    }

     /**
     * @return the qmasterHost
     */
    public boolean isQmasterHost() {
        return qmasterHost;
    }

    /**
     * @param qmasterHost the qmasterHost to set
     */
    public void setQmasterHost(boolean qmasterHost) {
        this.qmasterHost = qmasterHost;

        checkArchDependencies();
    }

    /**
     * @return the shadowHost
     */
    public boolean isShadowHost() {
        return shadowHost;
    }

    /**
     * @param shadowHost the shadowHost to set
     */
    public void setShadowHost(boolean shadowHost) {
        this.shadowHost = shadowHost;

        checkArchDependencies();
    }

    /**
     * @return the executionHost
     */
    public boolean isExecutionHost() {
        return executionHost;
    }

    /**
     * @param executionHost the executionHost to set
     */
    public void setExecutionHost(boolean executionHost) {
        this.executionHost = executionHost;
    }

    /**
     * @return the adminHost
     */
    public boolean isAdminHost() {
        return adminHost;
    }

    /**
     * @param adminHost the adminHost to set
     */
    public void setAdminHost(boolean adminHost) {
        this.adminHost = adminHost;
    }

    /**
     * @return the submitHost
     */
    public boolean isSubmitHost() {
        return submitHost;
    }

    /**
     * @param submitHost the submitHost to set
     */
    public void setSubmitHost(boolean submitHost) {
        this.submitHost = submitHost;
    }

    /**
     * @return the firstTask
     */
    public boolean isFirstTask() {
        return firstTask;
    }

    /**
     * @return the lastTask
     */
    public boolean isLastTask() {
        return lastTask;
    }

    public State getState() {
        return state;
    }

    public void setState(State state) {
        this.state = state;
    }
    
    public String getHostname() {
        return hostname;
    }

    public void setHostname(String hostname) {
        this.hostname = hostname;
    }

    public String getIp() {
        return ip;
    }

    public void setIp(String ip) {
        this.ip = ip;
    }

    public void setLogContent(String log) {
        this.log = log;
    }

    public String getLogContent() {
        return log;
    }

    public void setDisplayName(String displayName) {
        this.displayName = displayName;
    }

    public String getDisplayName() {
        if (isFirstTask() || isLastTask()) {
            if (displayName.trim().length()>0) {
                return displayName;
            }
        }
        return getHostname();
    }

    public String getConnectUser() {
        return connectUser;
    }

    public void setConnectUser(String connectUser) {
        this.connectUser = connectUser;
    }

    public String getJvmAddArgs() {
        return jvmAddArgs;
    }

    public void setJvmAddArgs(String jvmAddArgs) {
        this.jvmAddArgs = jvmAddArgs;
    }

    public String getJvmLibPath() {
        return jvmLibPath;
    }

    public void setJvmLibPath(String jvmLibPath) {
        this.jvmLibPath = jvmLibPath;
    }

    public long getInstallTimeout() {
        return installTimeout;
    }

    public void setInstallTimeout(long installTimeout) {
        this.installTimeout = installTimeout;
    }

    public long getResolveTimeout() {
        return resolveTimeout;
    }

    public void setResolveTimeout(long resolveTimeout) {
        this.resolveTimeout = resolveTimeout;
    }

    public boolean hasAnyComponent() {
        return isBdbHost() || isQmasterHost() || isShadowHost() || isExecutionHost() || isAdminHost() || isSubmitHost();
    }

    public String getComponentString() {
        String str = "";
        if (isFirstTask()) {
            return "prerequisites";
        }
        if (isLastTask()) {
            return "admin/submit";
        }
        if (isBdbHost()) {
            str += Util.SgeComponents.bdb.toString()+", ";
        }
        if (isQmasterHost()) {
            str += Util.SgeComponents.qmaster.toString()+", ";
        }
        if (isShadowHost()) {
            str += Util.SgeComponents.shadow.toString()+", ";
        }
        if (isExecutionHost()) {
            str += Util.SgeComponents.execd.toString()+", ";
        }
        return str.length()>1 ? str.substring(0, str.length()-2) : "";
    }
    
    public void setComponentVariables(Properties variables) {
    	variables.put("isQmaster", isQmasterHost());
    	variables.put("isExecd", isExecutionHost());
    	variables.put("isShadow", isShadowHost());
    	variables.put("isAdmin", isAdminHost());
    	variables.put("isSubmit", isSubmitHost());
    	
    	// Fill out all of the lists. The differentation between component type
        // will happen at the call of the install script
        variables.put(VAR_EXEC_HOST_LIST, getHostname());
        variables.put(VAR_SHADOW_HOST_LIST, getHostname());
        variables.put(VAR_ADMIN_HOST_LIST, getHostname());
        variables.put(VAR_SUBMIT_HOST_LIST, getHostname());
    }

    /**
     * Checks the architecture dependencies and sets the host component options respectively
     */
    public void checkArchDependencies() {
        if (architecture.equals("")) {
            return;
        }
        
        // Restrict qmaster and shadow components
        if (architecture.startsWith("win-") == true) {
            qmasterHost = false;
            shadowHost  = false;
        }
    }

    /**
     * Returns true if the host is the local host
     * @return true if the host's IP equals the localhost's IP, false otherwise.
     */
    public boolean isLocalhost() {
        return getIp().equals(localHostIP);
    }
}
