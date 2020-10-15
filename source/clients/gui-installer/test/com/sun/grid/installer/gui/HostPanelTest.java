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

import com.izforge.izpack.installer.AutomatedInstaller;
import com.sun.grid.installer.task.ThreadPoolObserver.ThreadPoolEvent;
import com.sun.grid.installer.task.ThreadPoolObserver.ThreadPoolListener;
import com.sun.grid.installer.util.Config;
import com.sun.grid.installer.util.cmd.TestBedManager;
import java.awt.Component;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JComponent;
import javax.swing.JTable;
import junit.framework.JUnit4TestAdapter;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Before;
import org.junit.After;
import org.junit.Test;
import org.junit.Assert;
import org.junit.Ignore;

public class HostPanelTest implements Config {
    
    private static AutomatedInstaller automatedInstaller = null;
    private static HostPanel hostPanel = null;

    private static final Wait FINISHED = new Wait(false);
    private static final Wait STARTED = new Wait(false);
    private static final Wait UPDATED = new Wait(false);

    private int maxWaitTime = 10000;

    public HostPanelTest() {}

    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(HostPanelTest.class);
    }

    @BeforeClass
    public static void setUpClass() throws Exception {
       automatedInstaller = new AutomatedInstaller("");

       TestBedManager.getInstance().setRunMode(TestBedManager.RunMode.FAST);
    }

    @AfterClass
    public static void cleanUpClass() throws Exception {
    }

    @Before
    public void setUp() {
    }

    @After
    public void cleanUp() {
        FINISHED.ready = false;
        STARTED.ready = false;
        UPDATED.ready = false;

        maxWaitTime = 10000;

        TestBedManager.getInstance().setGenerationMode(TestBedManager.GenerationMode.ALWAYS_NEW);
    }

    /**
     * -------------------------------------------------------------------------
     * Tests
     * -------------------------------------------------------------------------
     */

    @Ignore
    public void testDefaultQmaster() throws Exception {
        setQmasterHost(automatedInstaller, "qmaster");
        hostPanel = getHostPanelInstance(automatedInstaller);
        hostPanel.addThreadPoolListener(new ThreadPoolEventNotifier());

        callMethod_PanelActivate(hostPanel);

        waitForTPFinished();

        ArrayList<JTable> tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);

        Assert.assertFalse("Qmaster is not in the 'all' table!", tables.get(0).getRowCount() == 0);
        Assert.assertFalse("More then 1 host in 'all' table!", tables.get(0).getRowCount() > 1);

        HostList hostList = ((HostTableModel)tables.get(0).getModel()).getHostList();
        Host qmaster = hostList.get(0);
        Assert.assertTrue("The host is not Qmaster host!", qmaster.isQmasterHost());
        Assert.assertFalse("The host is BDB host!", qmaster.isBdbHost());
        Assert.assertFalse("The host is Execution host!", qmaster.isExecutionHost());
        Assert.assertFalse("The host is Shadow host!", qmaster.isShadowHost());
        Assert.assertTrue("The host is not Admin  host!", qmaster.isAdminHost());
        Assert.assertTrue("The host is not Submit host!", qmaster.isSubmitHost());
        Assert.assertFalse("The host is First task!", qmaster.isFirstTask());
        Assert.assertFalse("The host is Last task!", qmaster.isLastTask());
    }

    @Ignore
    public void testDefaultBDB() throws Exception {
        setQmasterHost(automatedInstaller, "");
        setBDBHost(automatedInstaller, "bdbserver");
        setComponentSelection(automatedInstaller, false, false, false, true, false);
        hostPanel = getHostPanelInstance(automatedInstaller);
        hostPanel.addThreadPoolListener(new ThreadPoolEventNotifier());

        callMethod_PanelActivate(hostPanel);

        waitForTPFinished();

        ArrayList<JTable> tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);

        Assert.assertFalse("Bdb host is not in the 'all' table!", tables.get(0).getRowCount() == 0);
        Assert.assertFalse("More then 1 host in 'all' table!", tables.get(0).getRowCount() > 1);

        HostList hostList = ((HostTableModel)tables.get(0).getModel()).getHostList();
        Host dbdhost = hostList.get(0);
        Assert.assertFalse("The host is Qmaster host!", dbdhost.isQmasterHost());
        Assert.assertTrue("The host is not BDB host!", dbdhost.isBdbHost());
        Assert.assertFalse("The host is Execution host!", dbdhost.isExecutionHost());
        Assert.assertFalse("The host is Shadow host!", dbdhost.isShadowHost());
        Assert.assertFalse("The host is Admin  host!", dbdhost.isAdminHost());
        Assert.assertFalse("The host is Submit host!", dbdhost.isSubmitHost());
        Assert.assertFalse("The host is First task!", dbdhost.isFirstTask());
        Assert.assertFalse("The host is Last task!", dbdhost.isLastTask());
    }

    @Ignore
    public void testResolveTableConsistency() throws Exception {
//        setQmasterHost(automatedInstaller, "");
//        setBDBHost(automatedInstaller, "");
        automatedInstaller = new AutomatedInstaller("");
        setComponentSelection(automatedInstaller, false, true, false, false, false);
        hostPanel = getHostPanelInstance(automatedInstaller);
        hostPanel.addThreadPoolListener(new ThreadPoolEventNotifier());

        callMethod_PanelActivate(hostPanel);

        ArrayList<String> hostNames = generateHostNames(50, "grid");
        callMethod_ResolveHosts(hostPanel, Host.Type.HOSTNAME, hostNames, false, false, true, true, true, true, "execd_spool_dir");

        maxWaitTime = hostNames.size() * 2000;
        waitForTPFinished();

        ArrayList<JTable> tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);

        testResolveTableConsistency((HostTable)tables.get(0), (HostTable)tables.get(1), (HostTable)tables.get(2));
        testAddedAndRecievedEquivalency((HostTable)tables.get(0), (HostTable)tables.get(1), (HostTable)tables.get(2), hostNames.size());
    }

    @Ignore
    public void testValidateTableConsistency() throws Exception {
        ArrayList<JTable> tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);

        callMethod_ValidateHostsAndInstall(hostPanel, false);

        maxWaitTime = tables.get(1).getRowCount() * 2000;
        waitForTPFinished();

        tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);

        testResolveTableConsistency((HostTable)tables.get(0), (HostTable)tables.get(1), (HostTable)tables.get(2));
    }

    @Ignore
    public void testInstallTableConsistency() throws Exception {
        ArrayList<JTable> tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);
        HostList successHostList = ((HostTable)tables.get(1)).getHostList();

        maxWaitTime = successHostList.size() * 8000;
        callMethod_SwitchToInstallModeAndInstall(hostPanel, successHostList);

        waitForTPFinished();

        tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);

        testInstallTableConsistency((HostTable)tables.get(0), (HostTable)tables.get(1), (HostTable)tables.get(2));
//        testAddedAndRecievedEquivalency((HostTable)tables.get(0), (HostTable)tables.get(1), (HostTable)tables.get(2), hostList.size());
    }

    @Ignore
    public void testShadowOnQmaster() throws Exception {
        TestBedManager.getInstance().setGenerationMode(TestBedManager.GenerationMode.ALWAYS_SUCCEEDS);
        automatedInstaller = getAutomatedInstallerInstance("");
        setQmasterHost(automatedInstaller, "qmaster");
        setComponentSelection(automatedInstaller, true, true, true, false, true);
        hostPanel = getHostPanelInstance(automatedInstaller);
        hostPanel.addThreadPoolListener(new ThreadPoolEventNotifier());

        callMethod_PanelActivate(hostPanel);
        ArrayList<String> hosts = new ArrayList<String>();
        hosts.add("execd");
        callMethod_ResolveHosts(hostPanel, Host.Type.HOSTNAME, hosts, false, false, false, true, false, true, "execd_spool_dir");

        maxWaitTime = 5000;
        waitForTPFinished();

        ArrayList<JTable> tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);
        //System.out.println("Begin");
        debugTablesContent(tables);

        callMethod_PanelDeactivate(hostPanel);

        setComponentSelection(automatedInstaller, true, true, true, false, false);

        callMethod_PanelActivate(hostPanel);

        //System.out.println("Middle");
        debugTablesContent(tables);

        callMethod_ValidateHostsAndInstall(hostPanel, true);

        maxWaitTime = 10000;
        waitForTPFinished();
        waitForTPFinished();

//        ArrayList<JTable> tables = new ArrayList<JTable>(3);
//        getJTables(hostPanel, tables);
//        HostList successHostList = ((HostTable)tables.get(1)).getHostList();
//
//        List<Host> hostList = new ArrayList<Host>(successHostList.size());
//        for(Host host : successHostList) {
//            hostList.add(host);
//        }
//
//        maxWaitTime = hostList.size() * 8000;
//        callMethod_SwitchToInstallModeAndInstall(hostPanel, hostList);
//
//        waitForTPFinished();
//        try {
//            Thread.sleep(10000);
//        } catch (Exception e) {
//        }

        tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);

        //System.out.println("End:");
        debugTablesContent(tables);
        
        HostList result = ((HostTableModel)tables.get(1).getModel()).getHostList();
        for (Host host : result) {
            if (host.isShadowHost()) {
                return;
            }
        }
        
        Assert.fail("Qmaster host - missing shadow deamon.");
    }

    @Test
    public void testShadowOnQmasterBasic() throws Exception {
        TestBedManager.getInstance().setGenerationMode(TestBedManager.GenerationMode.ALWAYS_SUCCEEDS);
        automatedInstaller = getAutomatedInstallerInstance("");
        setQmasterHost(automatedInstaller, "qmaster2");
        setComponentSelection(automatedInstaller, true, true, true, false, true);
        hostPanel = getHostPanelInstance(automatedInstaller);
        hostPanel.addThreadPoolListener(new ThreadPoolEventNotifier());

        callMethod_PanelActivate(hostPanel);
//        ArrayList<String> hosts = new ArrayList<String>();
//        hosts.add("execd");
//        callMethod_ResolveHosts(hostPanel, Host.Type.HOSTNAME, hosts, false, false, false, true, false, true, "execd_spool_dir");

        maxWaitTime = 5000;
        waitForTPFinished();

        //System.out.println("Begin");
        ArrayList<JTable> tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);
        debugTablesContent(tables);

        callMethod_PanelDeactivate(hostPanel);

        setComponentSelection(automatedInstaller, true, true, true, false, false);

        callMethod_PanelActivate(hostPanel);

        //System.out.println("Middle");
        tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);
        debugTablesContent(tables);

        callMethod_ValidateHostsAndInstall(hostPanel, true);

        maxWaitTime = 10000;
        waitForTPFinished();
        waitForTPFinished();

//        ArrayList<JTable> tables = new ArrayList<JTable>(3);
//        getJTables(hostPanel, tables);
//        HostList successHostList = ((HostTable)tables.get(1)).getHostList();
//
//        List<Host> hostList = new ArrayList<Host>(successHostList.size());
//        for(Host host : successHostList) {
//            hostList.add(host);
//        }
//
//        maxWaitTime = hostList.size() * 8000;
//        callMethod_SwitchToInstallModeAndInstall(hostPanel, hostList);
//
//        waitForTPFinished();

        //System.out.println("End");
        tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);
        debugTablesContent(tables);

        HostList result = ((HostTableModel)tables.get(1).getModel()).getHostList();
        for (Host host : result) {
            if (host.isShadowHost()) {
                return;
            }
        }

        Assert.fail("Qmaster host - missing shadow deamon.");
    }

    private void testAddedAndRecievedEquivalency(HostTable allTable, HostTable successTable, HostTable failedTable, int mumOfAdded) {
        Assert.assertEquals("Tables does not conatin all added hosts!", mumOfAdded, successTable.getRowCount() + failedTable.getRowCount());
    }

    private void testResolveTableConsistency(HostTable allTable, HostTable successTable, HostTable failedTable) {
        Assert.assertTrue("'All' table row count does not equal with 'success' plus 'failed' tables row count!", (allTable.getRowCount() == (successTable.getRowCount() + failedTable.getRowCount())));

        HostList allList     = ((HostTableModel)allTable.getModel()).getHostList();
        HostList successList = ((HostTableModel)successTable.getModel()).getHostList();
        HostList failedList  = ((HostTableModel)failedTable.getModel()).getHostList();

        if (!allList.containsAll(successList)) {
            Assert.fail("'All' table does not contain all values from table 'success'!");
        }

        if (!allList.containsAll(failedList)) {
            Assert.fail("'All' table does not contain all values from table 'failed'!");
        }

        for (Host host : successList) {
            Host pair = allList.get(host);

            Assert.assertSame("Host '" + host + "' in 'success' table != with it's pair in 'all' table!", host, pair);
            Assert.assertSame("Inconsistent state in 'success' table at '" + host + "': " + host.getState() + "!=" + pair.getState(), host.getState(), pair.getState());

            switch (host.getState()) {
                case ADMIN_USER_NOT_KNOWN:
                case BDB_SPOOL_DIR_EXISTS:
                case BDB_SPOOL_DIR_WRONG_FSTYPE:
                case CANCELED:
                case COPY_FAILED_CHECK_HOST:
                case COPY_TIMEOUT_CHECK_HOST:
                case MISSING_FILE:
                case OPERATION_TIMEOUT:
                case PERM_BDB_SPOOL_DIR:
                case PERM_EXECD_SPOOL_DIR:
                case PERM_JMX_KEYSTORE:
                case PERM_QMASTER_SPOOL_DIR:
                case USED_EXECD_PORT:
                case USED_JMX_PORT:
                case USED_QMASTER_PORT:
                case JVM_LIB_MISSING:
                //case UNKNOWN_ERROR: let warn in case of unknown error
                case REACHABLE: break;
                default: {
                    if ((Integer)TestBedManager.getInstance().getValidationMap().get(host.getHostname()) != TestBedManager.EXIT_VAL_SOMETHING) {
                        Assert.fail("Unknown 'success' state:" + host.getState());
                    }
                }
            }

            Assert.assertEquals("Inconsistent component configuration in 'success' tableat '" + host + "':" + host.getComponentString() + "!=" + pair.getComponentString(), host.getComponentString(), pair.getComponentString());
            Assert.assertEquals("Inconsistent architecture in 'success' table at '" + host + "':" + host.getArchitecture() + "!=" + pair.getArchitecture(), host.getArchitecture(), pair.getArchitecture());
            Assert.assertEquals("Inconsistent spoolin directory in 'success' table at '" + host + "':" + host.getSpoolDir() + "!=" + pair.getSpoolDir(), host.getSpoolDir(), pair.getSpoolDir());

        }

        for (Host host : failedList) {
            Host pair = allList.get(host);

            Assert.assertSame("Host '" + host + "' in 'failed' table != with it's pair in 'all' table!", host, pair);
            Assert.assertSame("Inconsistent state in 'failed' table at '" + host + "': " + host.getState() + "!=" + pair.getState(), host.getState(), pair.getState());

            switch (host.getState()) {
                case CANCELED:
                case UNKNOWN_HOST:
                case UNREACHABLE:
                case RESOLVABLE: break;
                default: {
                    Assert.fail("Unknown 'failed' state:" + host.getState().name());
                }
            }

            Assert.assertEquals("Inconsistent component configuration in 'failed' tableat '" + host + "':" + host.getComponentString() + "!=" + pair.getComponentString(), host.getComponentString(), pair.getComponentString());
            Assert.assertEquals("Inconsistent architecture in 'failed' table at '" + host + "':" + host.getArchitecture() + "!=" + pair.getArchitecture(), host.getArchitecture(), pair.getArchitecture());
            Assert.assertEquals("Inconsistent spoolin directory in 'failed' table at '" + host + "':" + host.getSpoolDir() + "!=" + pair.getSpoolDir(), host.getSpoolDir(), pair.getSpoolDir());
        }
    }

    private void testInstallTableConsistency(HostTable allTable, HostTable successTable, HostTable failedTable) {
        HostList allList     = ((HostTableModel)allTable.getModel()).getHostList();
        HostList successList = ((HostTableModel)successTable.getModel()).getHostList();
        HostList failedList  = ((HostTableModel)failedTable.getModel()).getHostList();

        Assert.assertTrue("'All' table still contains rows!", allTable.getRowCount() == 0);

        for (Host host : successList) {

            switch (host.getState()) {
                case SUCCESS: break;
                default: {
                    Assert.fail("Unknown 'success' state:" + host.getState().name());
                }
            }

            Assert.assertTrue("Inconsistent table states! Host '" + host + "' is in both 'success' and 'failed' tables", failedList.indexOf(host) == -1);
        }

        for (Host host : failedList) {

            switch (host.getState()) {
                case FAILED:
                case CANCELED:
                case COPY_FAILED_INSTALL_COMPONENT:
                case COPY_TIMEOUT_INSTALL_COMPONENT:
                case OPERATION_TIMEOUT:
                case FAILED_ALREADY_INSTALLED_COMPONENT:
                case FAILED_DEPENDENT_ON_PREVIOUS: break;
                default: {
                    Assert.fail("Unknown 'failed' state:" + host.getState().name());
                }
            }
        }
    }

    //TODO Test cancel action and states after
    //TODO Test validation and states after
    //TODO Test installation and states after
    //TODO Test thread pool
    //TODO Test resolving and states after
    //TODO Test log content
    //TODO Test ip address
    //TODO Test architecture
    //TODO Test generated file content (check_host, install_component, auto_conf, readme)

    /**
     * -------------------------------------------------------------------------
     * Helper methods
     * -------------------------------------------------------------------------
     */

    /**
     * Creates a {@link HostPanel} instance
     * @param automatedInstaller The {@link AutomatedInstaller) which holds the necessary instalaltion data
     * @return The created {@link HostPanel} instance
     * @throws java.lang.Exception
     */
    public static HostPanel getHostPanelInstance(AutomatedInstaller automatedInstaller) throws Exception {
        return new HostPanel(null, automatedInstaller.getInstallData());
    }

    /**
     * Creates an {@link AutomatedInstaller) instance
     * @param inputFile The file which holds the inital installation data. Can be empty.
     * @return The created {@link AutomatedInstaller) instance
     * @throws java.lang.Exception
     */
    public static AutomatedInstaller getAutomatedInstallerInstance(String inputFile) throws Exception {
        return new AutomatedInstaller(inputFile);
    }

    /**
     * Sets the component selection on the given {@link AutomatedInstaller}'s install data
     * @param automatedInstaller The {@link AutomatedInstaller} which stores the values
     * @param installQmaster Set whether the qmaster installation is selected
     * @param installExecd Set whether the execd installation is selected
     * @param installShadow Set whether the shadow installation is selected
     * @param installBDB Set whether the berkeley DB installation is selected
     * @param isExpressMode Set whether the express installation mode is selected
     */
    public static void setComponentSelection(AutomatedInstaller automatedInstaller,
            boolean installQmaster, boolean installExecd, boolean installShadow, boolean installBDB, boolean isExpressMode) {
        automatedInstaller.getInstallData().setVariable(VAR_INSTALL_QMASTER, Boolean.toString(installQmaster));
        automatedInstaller.getInstallData().setVariable(VAR_INSTALL_SHADOW, Boolean.toString(installShadow));
        automatedInstaller.getInstallData().setVariable(VAR_INSTALL_EXECD, Boolean.toString(installExecd));
        automatedInstaller.getInstallData().setVariable(VAR_INSTALL_BDB, Boolean.toString(installBDB));
        automatedInstaller.getInstallData().setVariable(VAR_INSTALL_MODE, (isExpressMode ? VAR_INSTALL_MODE_EXPRESS : VAR_INSTALL_MODE_CUSTOM));
    }

    /**
     * Sets the qmaster host on the given {@link AutomatedInstaller}'s install data
     * @param automatedInstaller The {@link AutomatedInstaller} which stores the value
     * @param host The qmaster host to set
     *
     * @see Config#VAR_QMASTER_HOST
     */
    public static void setQmasterHost(AutomatedInstaller automatedInstaller, String host) {
        automatedInstaller.getInstallData().setVariable(VAR_QMASTER_HOST, host);
    }

    /**
     * Sets the bdb host on the given {@link AutomatedInstaller}'s install data
     * @param automatedInstaller The {@link AutomatedInstaller} which stores the value
     * @param host The bdb host to set
     *
     * @see Config#VAR_DB_SPOOLING_SERVER
     */
    public static void setBDBHost(AutomatedInstaller automatedInstaller, String host) {
        automatedInstaller.getInstallData().setVariable(VAR_DB_SPOOLING_SERVER, host);
    }

    /**
     * Returns with the <@link JTable> tables available in the specified <@link JComponent> component
     * @param main The top component where the search start from
     * @param tables An initialized array which will contain all of the tables have been found
     */
    public static void getJTables(JComponent main, ArrayList<JTable> tables) {
        Component[] comps = main.getComponents();

        for (Component comp : comps) {
            if (comp instanceof JTable) {
                tables.add((JTable)comp);
            } else if (comp instanceof JComponent) {
                getJTables((JComponent)comp, tables);
            }
        }
    }

    /**
     * Generates static host names
     * @param num The number of the names should be generated
     * @param prefix The prefix of the name which will be followed by the ordinal number
     * @return The list of generated names
     */
    public static ArrayList<String> generateHostNames(int num, String prefix) {
        ArrayList<String> hostNames = new ArrayList<String>();

        for (int i = 0; i < num; i++) {
            hostNames.add(prefix + String.valueOf(i));
        }

        return hostNames;
    }

    /**
     * Prints out the hosts stored in the tables in the given list.
     * @param tables The tables which implement {@link HostTableModel} interface
     */
    public static void debugTablesContent(ArrayList<JTable> tables) {
        int index = 0;
        for (JTable table : tables) {
            HostList hostList = ((HostTableModel)table.getModel()).getHostList();
            for (Host host : hostList) {
                System.out.println(index + ". table: " + host);
            }

            index++;
        }
    }

    /**
     * Wait till the thread pool gets started.
     */
    private void waitForTPStarted() {
        waitFor(STARTED);
    }

    /**
     * Wait till the thread pool gets updated.
     */
    private void waitForTPUpdated() {
        waitFor(UPDATED);
    }

    /**
     * Wait till the thread pool finishes.
     */
    private void waitForTPFinished() {
        waitFor(FINISHED);
    }

    /**
     * Waits for the given {@link Wait} object to be notified. Uses the {@link HostPanelTest#maxWaitTime} to
     * specify timeout.
     * @param waitForObject The {@link Wait} object to wait on.
     *
     * @see HostPanelTest#waitForTPStarted()
     * @see HostPanelTest#waitForTPUpdated()
     * @see HostPanelTest#waitForTPFinished()
     */
    private void waitFor(Wait waitForObject) {
        synchronized (waitForObject) {
//            while (!waitForObject.ready) {
            try {
                waitForObject.wait(maxWaitTime);
                //System.out.println("!!!!AWAKE");
            } catch (InterruptedException ex) {
            }
//            }

            waitForObject.ready = false;
        }
    }

    /**
     * Calls the {@link HostPanel#panelActivate()} method on the given {@link HostPanel} instance
     * @param hostPanelInstance The {@link HostPanel} instance
     * @throws java.lang.Exception
     */
    private void callMethod_PanelActivate(HostPanel hostPanelInstance) throws Exception {
        Method method = HostPanel.class.getMethod("panelActivate", (Class[])null);
        method.invoke(hostPanelInstance, (Object[])null);
    }

    /**
     * Calls the {@link HostPanel#panelActivate()} method on the given {@link HostPanel} instance
     * @param hostPanelInstance The {@link HostPanel} instance
     * @throws java.lang.Exception
     */
    private void callMethod_PanelDeactivate(HostPanel hostPanelInstance) throws Exception {
        Method method = HostPanel.class.getMethod("panelDeactivate", (Class[])null);
        method.invoke(hostPanelInstance, (Object[])null);
    }

    /**
     * Calls the {@link HostPanel#resolveHosts()} method on the given {@link HostPanel} instance
     * @param hostPanelInstance The {@link HostPanel} instance
     * @throws java.lang.Exception
     */
    private void callMethod_ResolveHosts(HostPanel hostPanelInstance,
            Host.Type type, List<String> list, boolean isQmasterHost, boolean isBDBHost, boolean isShadowHost, boolean isExecutionHost, boolean isAdminHost, boolean isSubmitHost, String execdSpoolDir) throws Exception {
        Method method = HostPanel.class.getMethod("resolveHosts", Host.Type.class, List.class, boolean.class, boolean.class, boolean.class, boolean.class, boolean.class, boolean.class, String.class);
        method.invoke(hostPanelInstance, type, list, isQmasterHost, isBDBHost, isShadowHost, isExecutionHost, isAdminHost, isSubmitHost, execdSpoolDir);
    }

    /**
     * Calls the {@link HostPanel#validateHostsAndInstall()} method on the given {@link HostPanel} instance
     * @param hostPanelInstance The {@link HostPanel} instance
     * @throws java.lang.Exception
     */
    private void callMethod_ValidateHostsAndInstall(HostPanel hostPanelInstance, boolean install) throws Exception {
        Method method = HostPanel.class.getDeclaredMethod("validateHostsAndInstall", boolean.class, boolean.class);
        method.setAccessible(true);
        method.invoke(hostPanelInstance, install, true);
    }

    /**
     * Calls the {@link HostPanel#switchToInstallModeAndInstall()} method on the given {@link HostPanel} instance
     * @param hostPanelInstance The {@link HostPanel} instance
     * @param hostList List of hosts to install
     * @throws java.lang.Exception
     */
    private void callMethod_SwitchToInstallModeAndInstall(HostPanel hostPanelInstance, HostList hostList) throws Exception {
        List<Host> hostListList = new ArrayList<Host>(hostList.size());
        for(Host host : hostList) {
            hostList.add(host);
        }

        callMethod_SwitchToInstallModeAndInstall(hostPanelInstance, hostListList);
    }

    /**
     * Calls the {@link HostPanel#switchToInstallModeAndInstall()} method on the given {@link HostPanel} instance
     * @param hostPanelInstance The {@link HostPanel} instance
     * @param hostList List of hosts to install
     * @throws java.lang.Exception
     */
    private void callMethod_SwitchToInstallModeAndInstall(HostPanel hostPanelInstance, List<Host> hostList) throws Exception {
        Method method = HostPanel.class.getDeclaredMethod("switchToInstallModeAndInstall", List.class);
        method.setAccessible(true);
        method.invoke(hostPanelInstance, hostList);
    }

    /**
     * -------------------------------------------------------------------------
     * Helper classes
     * -------------------------------------------------------------------------
     */

    /**
     * Class to listen thread pool events. Notifies the appropiate {@link Wait} objects
     * in case of recieved events.
     */
    private class ThreadPoolEventNotifier implements ThreadPoolListener {
        public void threadPoolActionPerformed(ThreadPoolEvent threadPoolEvent) {
            Wait waitForObject = null;
            
            if (threadPoolEvent.getType() == ThreadPoolEvent.EVENT_THREAD_POOL_STARTED) {
                waitForObject = STARTED;
                //System.out.println("!!!!STARTED");
            } else if (threadPoolEvent.getType() == ThreadPoolEvent.EVENT_THREAD_POOL_UPDATED) {
                waitForObject = UPDATED;
                //System.out.println("!!!!UPDATED");
            } else if (threadPoolEvent.getType() == ThreadPoolEvent.EVENT_THREAD_POOL_FINISHED) {
                waitForObject = FINISHED;
                //System.out.println("!!!!FINISHED");
            }

            // Notify the appropriate object
            synchronized (waitForObject) {
                waitForObject.ready = true;
                waitForObject.notifyAll();
            }
        }
    }

    /**
     * Object to store state for a thread.
     */
    private static class Wait {
        public boolean ready = false;

        public Wait(boolean ready) {
            this.ready = ready;
        }
    }
}

