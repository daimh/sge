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

import com.izforge.izpack.installer.AutomatedInstallData;
import com.izforge.izpack.installer.InstallData;
import com.izforge.izpack.installer.PanelAutomation;
import com.izforge.izpack.util.Debug;
import com.sun.grid.installer.task.ThreadPoolObserver.ThreadPoolListener;
import com.sun.grid.installer.task.ThreadPoolObserver.ThreadPoolEvent;
import com.sun.grid.installer.util.Util;
import java.awt.Component;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import javax.swing.JComponent;
import javax.swing.JTable;
import net.n3.nanoxml.XMLElement;

/**
 * Functions to support automated usage of the HostPanel
 */
public class HostPanelAutomationHelper implements PanelAutomation {

    private static final String ELEMENT_HOSTS_NAME = "hosts";
    private static final String ELENENT_HOST_NAME = "host";
    private static final String ATTRIBUTE_HOST_ID = "id";
    private static final String ATTRIBUTE_HOTS_ARGS = "arguments";

    private static final Wait FINISHED = new Wait(false);
    private static final Wait STARTED = new Wait(false);
    private static final Wait UPDATED = new Wait(false);

    private Map<String, String> entries;

    public HostPanelAutomationHelper() {
        this.entries = null;
    }

    public HostPanelAutomationHelper(Map<String, String> entries) {
        this.entries = entries;
    }

    /**
     * Asks to make the XML panel data.
     *
     * @param idata     The installation data.
     * @param panelRoot The tree to put the data in.
     */
    public void makeXMLData(AutomatedInstallData idata, XMLElement panelRoot) {
        XMLElement hostsElement;
        XMLElement hostElement;

        // ----------------------------------------------------
        // add the item that combines all entries
        // ----------------------------------------------------
        hostsElement = new XMLElement(ELEMENT_HOSTS_NAME);
        panelRoot.addChild(hostsElement);

        // ----------------------------------------------------
        // add all entries
        // ----------------------------------------------------
        Iterator<String> keys = this.entries.keySet().iterator();
        while (keys.hasNext()) {
            String key = keys.next();
            String value = this.entries.get(key);

            hostElement = new XMLElement(ELENENT_HOST_NAME);
            hostElement.setAttribute(ATTRIBUTE_HOST_ID, key);
            hostElement.setAttribute(ATTRIBUTE_HOTS_ARGS, value);

            hostsElement.addChild(hostElement);
        }
    }

    /**
     * Asks to run in the automated mode.
     *
     * @param idata     The installation data.
     * @param panelRoot The XML tree to read the data from.
     * @return always true.
     */
    public boolean runAutomated(AutomatedInstallData idata, XMLElement panelRoot) {
        XMLElement hostsElement;
        XMLElement hostElement;
        String id;
        String arguments;

        // ----------------------------------------------------
        // get the section containing the user entries
        // ----------------------------------------------------
        hostsElement = panelRoot.getFirstChildNamed(ELEMENT_HOSTS_NAME);
        if (hostsElement == null) {
            return false;
        }

        Vector<XMLElement> hostElements = hostsElement.getChildrenNamed(ELENENT_HOST_NAME);
        if (hostElements == null) {
            return false;
        }

        // ----------------------------------------------------
        // retieve each entry and substitute the associated
        // variable
        // ----------------------------------------------------
        ArrayList<Host> allHosts = new ArrayList<Host>(hostElements.size());
        for (int i = 0; i < hostElements.size(); i++) {
            hostElement = hostElements.elementAt(i);

            id = hostElement.getAttribute(ATTRIBUTE_HOST_ID);
            arguments = hostElement.getAttribute(ATTRIBUTE_HOTS_ARGS);

            Debug.trace("Found host: '" + arguments + "'.");

            try {
                List<Host> hostsPart = Host.fromStringInstance(arguments);
                allHosts.addAll(hostsPart);
            } catch (IllegalArgumentException e) {
                Debug.error("Failed to load host instance from automated installation config file! " + e);
                Debug.trace("Terminating the installation!");
                return false;
            }
        }

        HostPanel hostPanel = new HostPanel(null, (InstallData) idata);
        hostPanel.addThreadPoolListener(new ThreadPoolEventNotifier());
            
        // Panel activate
        try {
            Method method = HostPanel.class.getMethod("panelActivate", (Class[]) null);
            method.invoke(hostPanel, (Object[]) null);
        } catch (Exception e) {
            Debug.error("Failed to call method 'panelActivate'! " + e);
            Debug.trace("Terminating the installation!");
            return false;
        }

        // Resolve hosts
        hostPanel.addHosts(allHosts);
        waitForTPFinished(allHosts.size() * Util.DEF_RESOLVE_TIMEOUT); // TODO diveded by the paralell thread numbers

        ArrayList<JTable> tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);
        HostList failedList = ((HostTable)tables.get(2)).getHostList();
        String failedHosts = "";
        for (Host host : failedList) {
            failedHosts += host.getHostname() + " ";
        }
        if (!failedHosts.equals("")) {
            Debug.error("Not all of the given hosts are reachable: " + failedHosts);
            Debug.trace("Terminating the installation!");
            return false;
        }
        failedHosts = "";

        // Validate
        try {
            Method method = HostPanel.class.getDeclaredMethod("validateHostsAndInstall", boolean.class, boolean.class);
            method.setAccessible(true);
            method.invoke(hostPanel, false, true);
        } catch (Exception e) {
            Debug.error("Failed to call method 'validateHostsAndInstall'! " + e);
            Debug.trace("Terminating the installation!");
            return false;
        }

        waitForTPFinished(allHosts.size() * Util.DEF_RESOLVE_TIMEOUT);
        
        HostList reachableHostList = ((HostTable)tables.get(1)).getHostList();
        // The switchToInstallModeAndInstall method expects List<Host> arg
        List<Host> reachableList = new ArrayList<Host>(reachableHostList.size());
        for (Host host : reachableHostList) {
            if (host.getState() != Host.State.REACHABLE) {
                Host.State state = host.getState();
                failedHosts += host.getHostname() + "(Warning: " + state + " - Tip: " + state.getTooltip() + ") ";
            } else {
                reachableList.add(host);
            }
        }
        if (!failedHosts.equals("")) {
            Debug.error("Some of the hosts failed during the validation: " + failedHosts);
            Debug.trace("Terminating the installation!");
            return false;
        }

        // Install
        try {
            Method method = HostPanel.class.getDeclaredMethod("switchToInstallModeAndInstall", List.class);
            method.setAccessible(true);
            method.invoke(hostPanel, reachableList);
        } catch (Exception e) {
            Debug.error("Failed to call method 'switchToInstallModeAndInstall'! " + e);
            Debug.trace("Terminating the installation!");
            return false;
        }
        waitForTPFinished(allHosts.size() * Util.DEF_INSTALL_TIMEOUT);

        tables = new ArrayList<JTable>(3);
        getJTables(hostPanel, tables);
        failedList = ((HostTable)tables.get(2)).getHostList();
        for (Host host : failedList) {
            failedHosts += host.getHostname() + "(" + host.getState() + ") ";
        }
        if (!failedHosts.equals("")) {
            Debug.error("Some of the hosts failed during the installation: " + failedHosts);
            Debug.trace("Installation failed!");
            return false;
        }

        Debug.trace("Installation succeeded!");
        return true;
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
                tables.add((JTable) comp);
            } else if (comp instanceof JComponent) {
                getJTables((JComponent) comp, tables);
            }
        }
    }

    /**
     * Wait till the thread pool gets started.
     */
    private void waitForTPStarted(long maxWaitTime) {
        waitFor(STARTED, maxWaitTime);
    }

    /**
     * Wait till the thread pool gets updated.
     */
    private void waitForTPUpdated(long maxWaitTime) {
        waitFor(UPDATED, maxWaitTime);
    }

    /**
     * Wait till the thread pool finishes.
     */
    private void waitForTPFinished(long maxWaitTime) {
        waitFor(FINISHED, maxWaitTime);
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
    private void waitFor(Wait waitForObject, long maxWaitTime) {
        synchronized (waitForObject) {
//            while (!waitForObject.ready) {
                try {
                    waitForObject.wait(maxWaitTime);
                } catch (InterruptedException ex) {}
//            }

            waitForObject.ready = false;
        }
    }

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
