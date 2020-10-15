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
package com.sun.grid.jgdi.management;

import com.sun.grid.jgdi.jni.EventClientImpl;
import com.sun.grid.jgdi.jni.JGDIBaseImpl;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;
import java.util.concurrent.locks.ReentrantLock;
import java.util.logging.Level;
import javax.management.Notification;
import javax.management.ObjectName;
import javax.management.MBeanServer;

import com.sun.grid.jgdi.management.mbeans.JGDIJMX;
import com.sun.grid.jgdi.security.JGDIPrincipal;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.security.AccessController;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.logging.LogRecord;
import java.util.logging.Logger;
import javax.management.NotificationListener;
import javax.management.remote.JMXConnectionNotification;
import javax.management.remote.JMXConnectorServer;
import javax.security.auth.Subject;

/**
 * JGDI JMX agent class.
 */
public class JGDIAgent implements Runnable {

    enum State {
        STOPPED,
        START,
        RUNNING,
        SHUTDOWN
    }
    private JMXConnectorServer mbeanServerConnector;
    // Platform MBeanServer used to register your MBeans
    private MBeanServer mbs;
    private static File sgeRoot;
    private static File caTop;
    private final static Logger log = Logger.getLogger(JGDIAgent.class.getName());
    private static final JGDIAgent agent = new JGDIAgent();
    // JGDI url string
    private static String url;
    private static File jmxDir;
    private final Lock stateLock = new ReentrantLock();
    private final Condition stateChangedCondition = stateLock.newCondition();
    private State state = State.STOPPED;

    /**
     * JGDIAgent can not be instantiate from outside
     */
    private JGDIAgent() {
    }

    private boolean setState(State state) {
        boolean ret = true;
        log.entering("JGDIAgent", "setState", state);
        stateLock.lock();
        try {
            // prevent overriding of State.SHUTDOWN if it has been set before
            // startup completed
            if (state == State.RUNNING && this.state == State.SHUTDOWN) {
                log.log(Level.FINE, "shutdown called before State.RUNNING");
                ret = false;
                return ret;
            } else {
                log.log(Level.FINE, "old state: " + this.state + " new state: " + state);
            }
            this.state = state;
            stateChangedCondition.signalAll();
        } finally {
            stateLock.unlock();
            log.exiting("JGDIAgent", "setState", ret);
        }
        return ret;
    }

    public void startMBeanServer() throws Exception {

        File managementFile = new File(getJmxDir(), "management.properties");

        log.log(Level.FINE, "loading mbean server configuration from {0}", managementFile);

        final Properties props = new Properties();
        props.load(new FileInputStream(managementFile));

        final String portStr = props.getProperty(ConnectorBootstrap.PropertyNames.PORT);

        mbeanServerConnector = ConnectorBootstrap.initialize(portStr, props);
        mbeanServerConnector.addNotificationListener(new MyNotificationListener(), null, null);
        mbs = mbeanServerConnector.getMBeanServer();
        log.log(Level.FINE, "starting mbean server");
        mbeanServerConnector.start();
        log.log(Level.INFO, "mbean server started (port={0})", portStr);
    }

    private void stopMBeanServer() {
        if (mbeanServerConnector != null) {
            try {
                log.log(Level.FINE, "stopping mbean server");
                mbeanServerConnector.stop();
            } catch (Exception ex) {
                log.log(Level.WARNING, "cannot stop mbean server", ex);
            }
        }
    }

    public static String getUrl() {
        if (url == null) {
            throw new IllegalStateException("JGDIAgent.url is not initialized");
        }
        return url;
    }

    public static File getCaTop() {
        if (caTop == null) {
            String str = System.getProperty("com.sun.grid.jgdi.caTop");
            if (str == null) {
                throw new IllegalStateException("system properties com.sun.grid.jgdi.caTop not found");
            }
            caTop = new File(str);
        }
        return caTop;
    }

    public static File getSgeRoot() {
        if (sgeRoot == null) {
            String sgeRootStr = System.getProperty("com.sun.grid.jgdi.sgeRoot");
            if (sgeRootStr == null) {
                throw new IllegalStateException("system properties com.sun.grid.jgdi.sgeRoot not found");
            }
            sgeRoot = new File(sgeRootStr);
        }
        return sgeRoot;
    }

    public static String getSgeCell() {
        String ret = System.getProperty("com.sun.grid.jgdi.sgeCell");
        if (ret == null) {
            throw new IllegalStateException("system properties com.sun.grid.jgdi.sgeCell not found");
        }
        return ret;
    }

    public static File getJmxDir() {
        if (jmxDir == null) {
            jmxDir = new File(getSgeRoot(), getSgeCell() + File.separatorChar + "common" + File.separatorChar + "jmx");
        }
        return jmxDir;
    }

    private MBeanServer getMBeanServer() throws IOException {
        if (mbs == null) {
            throw new IllegalStateException("mbean server is not started");
        }
        return mbs;
    }

    public void run() {

        EventClientImpl.resetShutdown();
        JGDIBaseImpl.resetShutdown();
        // only start MBean server if state could be set to RUNNING
        if (setState(State.RUNNING)) {
            try {
                log.log(Level.FINE, "starting mbean server");
                startMBeanServer();
            } catch (Exception ex) {
                log.log(Level.SEVERE, "startup of mbean server failed", ex);
                return;
            }
        }
        try {
            // The following code blocks until the stop or shutdown methods are called
            log.log(Level.FINE, "waitForStop");
            waitForStop();
        } catch (InterruptedException ex) {
            log.log(Level.FINE, "JGDIAgent has been interrupted");
        } finally {
            log.log(Level.INFO, "JGDIAgent is going down");
            try {
                JGDISession.closeAllSessions();
                EventClientImpl.closeAll();
                JGDIBaseImpl.closeAllConnections();
            } finally {
                log.log(Level.FINE, "stopping mbean server");
                stopMBeanServer();
                setState(State.STOPPED);
            }
        }
    }

    public static void main(String[] args) {
        try {
            if (args.length != 1) {
                log.log(Level.SEVERE, "invalid arguments for JGDIAgent: JGDIAgent <jgdi connect url>");
                return;
            }
            url = args[0];

            try {
                FileOutputStream stdout = new FileOutputStream("jgdi.stdout", true);
                System.setOut(new PrintStream(stdout, true));
                log.fine("stdout redirected to jgdi.stdout");
            } catch (Exception ex) {
                log.log(Level.WARNING, "cannot redirect stdout to file jgdi.stdout", ex);
            }
            try {
                FileOutputStream stderr = new FileOutputStream("jgdi.stderr", true);
                System.setErr(new PrintStream(stderr, true));
                log.fine("stderr redirected to jgdi.stderr");
            } catch (Exception ex) {
                log.log(Level.WARNING, "cannot redirect stderr to file jgdi.stderr", ex);
            }

            Thread t = new Thread(agent);
            t.setContextClassLoader(agent.getClass().getClassLoader());
            t.start();
            t.join();
        } catch (Throwable ex) {
            log.log(Level.SEVERE, "unexpected error in JGDIAgent", ex);
        } finally {
            log.log(Level.INFO, "JGDIAgent is down");
        // LogManager.getLogManager().reset();
        }
    }

    private void waitForStop() throws InterruptedException {
        log.entering("JGDIAgent", "waitForStop");
        stateLock.lock();
        try {
            while (state == State.RUNNING) {
                stateChangedCondition.await();
            }
        } finally {
            stateLock.unlock();
        }
        log.exiting("JGDIAgent", "waitForStop");
    }

    /**
     * Shutdown the JGDIAgent
     */
    public static void shutdown() {
        log.entering("JGDIAgent", "shutdown");
        agent.setState(State.SHUTDOWN);
        log.exiting("JGDIAgent", "shutdown");
    }

    public static void start() {
        log.entering("JGDIAgent", "start");
        agent.setState(State.START);
        log.exiting("JGDIAgent", "start");
    }

    public static void stop() {
        log.entering("JGDIAgent", "stop");
        agent.setState(State.STOPPED);
        log.exiting("JGDIAgent", "stop");
    }

    /**
     * Get the name of the session mbean
     * @param connectionId the id of the jmx connection
     * @return name of the session mbean or <code>null</code> if the connection id
     *         does not contain a session id
     */
    public static ObjectName getObjectNameFromConnectionId(String connectionId) {
        log.entering("JGDIAgent", "getObjectNameFromConnectionId", connectionId);
        long sessionId = getSessionIdFromConnectionId(connectionId);
        ObjectName ret = null;
        if (sessionId >= 0) {
            ret = getObjectNameForSessionMBean(sessionId);
        }
        log.exiting("JGDIAgent", "getObjectNameFromConnectionId", ret);
        return ret;
    }

    /**
     * Get the name of the session mbean
     * @param sessionId the session id
     * @return the name of the session mbean
     */
    public static ObjectName getObjectNameForSessionMBean(long sessionId) {
        log.entering("JGDIAgent", "getObjectNameForSessionMBean", sessionId);
        ObjectName ret = null;
        try {
            ret = new ObjectName(String.format("gridengine:type=JGDI,sessionId=%d", sessionId));
        } catch (Exception ex) {
            IllegalStateException ilse = new IllegalStateException("Invalid object name", ex);
            log.throwing("JGDIAgent", "getObjectNameForSessionMBean", ilse);
            throw ilse;
        }
        log.exiting("JGDIAgent", "getObjectNameForSessionMBean", ret);
        return ret;
    }

    /**
     * Get the session id out of the connection id
     * @param connectionId the connection id
     * @return the session id of <code>-1</code> if the connection id does not contain a session id
     */
    public static long getSessionIdFromConnectionId(String connectionId) {
        log.entering("JGDIAgent", "getSessionIdFromConnectionId", connectionId);
        long ret = -1;
        int startIndex = connectionId.indexOf(JGDIPrincipal.SESSION_ID_PREFIX);
        if (startIndex >= 0) {
            startIndex += JGDIPrincipal.SESSION_ID_PREFIX.length();
            int endIndex = connectionId.indexOf(JGDIPrincipal.SESSION_ID_SUFFIX, startIndex);
            if (endIndex > 0) {
                String sessionIdStr = connectionId.substring(startIndex, endIndex);
                try {
                    log.log(Level.FINE, "sessionIdStr = {0}", sessionIdStr);
                    ret = Long.parseLong(sessionIdStr);
                } catch (NumberFormatException ex) {
                    log.log(Level.WARNING, "Got invalid sessionId ({0})", sessionIdStr);
                }
            } else {
                log.log(Level.WARNING, "end of sessionId not found in connectionId ({0})", connectionId);
            }
        } else {
            log.log(Level.WARNING, "sessionId not found in connectionId ({0})", connectionId);
        }
        if (ret < -1) {
            log.log(Level.WARNING, "jmx connection id does not contain a jgdi session id: {0}", connectionId);
        }
        log.exiting("JGDIAgent", "getSessionIdFromConnectionId", ret);
        return ret;
    }

    private void registerSessionMBean(JGDISession session) {
        log.entering("JGDIAgent", "registerSessionMBean", session);
        try {
            JGDIJMX mbean = new JGDIJMX(session);
            ObjectName mbeanName = getObjectNameForSessionMBean(session.getId());
            getMBeanServer().registerMBean(mbean, mbeanName);
            log.log(Level.FINE, "mbean for session {0} registered", session.getId());
        } catch (Exception ex) {
            LogRecord lr = new LogRecord(Level.WARNING, "Can not register mbean for session {0}");
            lr.setParameters(new Object[]{session.getId()});
            lr.setThrown(ex);
            log.log(lr);
        }
        log.exiting("JGDIAgent", "registerSessionMBean");
    }

    private void unregisterSessionMBean(JGDISession session) {
        log.entering("JGDIAgent", "unregisterSessionMBean", session);
        try {
            ObjectName mbeanName = getObjectNameForSessionMBean(session.getId());
            getMBeanServer().unregisterMBean(mbeanName);
            log.log(Level.FINE, "mbean for session {0} unregistered", session.getId());
        } catch (Exception ex) {
            LogRecord lr = new LogRecord(Level.WARNING, "Can not unregister mbean for session {0}");
            lr.setParameters(new Object[]{session.getId()});
            lr.setThrown(ex);
            log.log(lr);
        }
        log.exiting("JGDIAgent", "unregisterSessionMBean");
    }

    class MyNotificationListener implements NotificationListener {

        public void handleNotification(Notification notification, Object handback) {

            if (notification instanceof JMXConnectionNotification) {

                JMXConnectionNotification jn = (JMXConnectionNotification) notification;

                if (log.isLoggable(Level.FINE)) {
                    Subject sub = Subject.getSubject(AccessController.getContext());
                    log.log(Level.FINE, "Got notification from client {0}, subject = {1}", new Object[]{jn.getConnectionId(), sub});
                }

                long sessionId = getSessionIdFromConnectionId(jn.getConnectionId());
                if (sessionId >= 0) {
                    if (JMXConnectionNotification.CLOSED.equals(jn.getType())) {
                        log.log(Level.FINE, "client connection {0} closed", jn.getConnectionId());
                        JGDISession session = JGDISession.closeSession(sessionId);
                        if (session != null) {
                            unregisterSessionMBean(session);
                        }
                    } else if (JMXConnectionNotification.FAILED.equals(jn.getType())) {
                        log.log(Level.FINE, "client connection {0} failed", jn.getConnectionId());
                        JGDISession session = JGDISession.closeSession(sessionId);
                        if (session != null) {
                            unregisterSessionMBean(session);
                        }
                    } else if (JMXConnectionNotification.NOTIFS_LOST.equals(jn.getType())) {
                        log.log(Level.WARNING, "client connection {0} losts notification", jn.getConnectionId());
                    } else if (JMXConnectionNotification.OPENED.equals(jn.getType())) {
                        if (log.isLoggable(Level.FINE)) {
                            log.log(Level.FINE, "client connection {0} opened", new Object[]{jn.getConnectionId()});
                        }
                        JGDISession session = JGDISession.createNewSession(sessionId, url);
                        registerSessionMBean(session);
                    }
                } else {
                    log.log(Level.WARNING, "Got a jmx connection without a session id: {0}", jn.getConnectionId());
                }
            }
        }
    }
}

