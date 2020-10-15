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

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.event.ConnectionClosedEvent;
import com.sun.grid.jgdi.security.JGDIPrincipal;
import java.security.AccessController;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;
import javax.security.auth.Subject;

/**
 *  Session for a JMX connection to JGDI
 */
public class JGDISession {

    private final static Logger log = Logger.getLogger(JGDISession.class.getName());
    private final static Map<Long, JGDISession> sessionMap = new HashMap<Long, JGDISession>();
    private final static ReadWriteLock sessionLock = new ReentrantReadWriteLock();
    private final long id;
    private final Lock lock = new ReentrantLock();
    private final NotificationBridge notificationBridge;
    private final String url;
    private JGDI jgdi;
    private boolean closed;

    /**
     * Create a new JGDI session
     * @param id   id of the session
     * @param url  jgdi connection url
     */
    private JGDISession(long id, String url) {
        this.id = id;
        this.url = url;
        notificationBridge = NotificationBridgeFactory.newInstance(url);
    }

    /**
     * Get the jgdi object of this session
     * @return the jgdi object
     * @throws com.sun.grid.jgdi.JGDIException if the jgdi object could not be created
     */
    public JGDI getJGDI() throws JGDIException {
        log.entering("JGDISession", "getJGDI");
        JGDI ret = null;
        lock.lock();
        try {
            if (closed) {
                JGDIException ex = new JGDIException("session already closed");
                log.throwing("JGDISession", "getJGDI", ex);
                throw ex;
            }
            if (jgdi == null) {
                jgdi = JGDIFactory.newInstance(url);
                if (log.isLoggable(Level.FINE)) {
                    log.log(Level.FINE, "jgdi object {0} for session {1} created", new Object[]{jgdi, getId()});
                }
            }
            ret = jgdi;
        } finally {
            lock.unlock();
        }
        log.exiting("JGDISession", "getJGDI", ret);
        return ret;
    }

    /**
     * Get the notification bridge of this session
     * @return the notification brigde
     */
    public NotificationBridge getNotificationBridge() {
        log.entering("JGDISession", "getNotificationBridge");
        log.exiting("JGDISession", "getNotificationBridge", notificationBridge);
        return notificationBridge;
    }

    /**
     * Close this jgdi session
     */
    public void close() {
        log.entering("JGDISession", "close");
        lock.lock();
        try {
            if (!closed) {
                closed = true;
                if (jgdi != null) {
                    try {
                        jgdi.close();
                    } catch (Exception ex) {
                        LogRecord lr = new LogRecord(Level.WARNING, "Error while closing jgdi connection of session {0}");
                        lr.setParameters(new Object[]{getId()});
                        lr.setThrown(ex);
                        log.log(lr);
                    } finally {
                        jgdi = null;
                    }
                }
                try {
                    notificationBridge.close();
                } catch (Exception ex) {
                    LogRecord lr = new LogRecord(Level.WARNING, "Error while closing notification bridge of session {0}");
                    lr.setParameters(new Object[]{getId()});
                    lr.setThrown(ex);
                    log.log(lr);
                }
            }
            notificationBridge.eventOccured(new ConnectionClosedEvent(System.currentTimeMillis(), -1));
        } finally {
            lock.unlock();
        }
        log.exiting("JGDISession", "close");
    }

    private static long getSessionId() throws JGDIException {
        Subject sub = Subject.getSubject(AccessController.getContext());
        if (sub != null) {
            Set<JGDIPrincipal> ps = sub.getPrincipals(JGDIPrincipal.class);
            if (!ps.isEmpty()) {
                JGDIPrincipal p = ps.iterator().next();
                return p.getSessionId();
            }
        }
        throw new JGDIException("no active session found in subject " + sub);
    }

    /**
     * is this session already closed
     * @return <code>true</code> if this session is already closed
     */
    public boolean isClosed() {
        log.entering("JGDISession", "isClosed");
        boolean ret;
        lock.lock();
        try {
            ret = closed;
        } finally {
            lock.unlock();
        }
        log.entering("JGDISession", "isClosed", ret);
        return ret;
    }

    /**
     * Get the jgdi session from the security context
     * @return the current jgdi session
     * @throws com.sun.grid.jgdi.JGDIException new jgdi session has been found in the security context
     */
    public static JGDISession getCurrentSession() throws JGDIException {
        log.entering("JGDISession", "getCurrentSession");
        long sessionId = getSessionId();
        JGDISession ret = null;
        sessionLock.readLock().lock();
        try {
            ret = sessionMap.get(sessionId);
        } finally {
            sessionLock.readLock().unlock();
        }
        if (ret == null) {
            throw new JGDIException("no active session found");
        }
        log.exiting("JGDISession", "getCurrentSession", ret);
        return ret;
    }

    /**
     * Close a jgdi session.
     * 
     * @param sessionId id of the session
     * @return the closed session object or <code>null</code> if the session does
     *         not exist or if the session has been already closed
     */
    public static JGDISession closeSession(long sessionId) {
        log.entering("JGDISession", "closeSession", sessionId);
        JGDISession ret = null;
        sessionLock.writeLock().lock();
        try {
            JGDISession session = sessionMap.remove(sessionId);
            if (session != null) {
                session.close();
                ret = session;
            }
        } finally {
            sessionLock.writeLock().unlock();
        }
        log.exiting("JGDISession", "closeSession", ret);
        return ret;
    }

    /**
     * Create a new jgdi session
     * @param sessionId id of the session
     * @param url  jgdi connection url
     * @return the new session or <code>null</code> if a session with this id already exists
     */
    public static JGDISession createNewSession(long sessionId, String url) {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDISession", "createNewSession", new Object[]{sessionId, url});
        }
        JGDISession ret = null;
        sessionLock.writeLock().lock();
        try {
            if (!sessionMap.containsKey(sessionId)) {
                JGDISession session = new JGDISession(sessionId, url);
                sessionMap.put(sessionId, session);
                log.log(Level.FINE, "New jgdi jmx session with id {0} created", sessionId);
                ret = session;
            } else {
                log.log(Level.WARNING, "Do not create new session for session id {0}, this session already exists", sessionId);
            }
        } finally {
            sessionLock.writeLock().unlock();
        }
        log.exiting("JGDISession", "createNewSession", ret);
        return ret;
    }

    /**
     * Close all jgdi sessions
     */
    public static void closeAllSessions() {
        log.entering("JGDISession", "closeAllSessions");
        sessionLock.writeLock().lock();
        try {
            for (JGDISession session : sessionMap.values()) {
                session.close();
            }
            sessionMap.clear();
        } finally {
            sessionLock.writeLock().unlock();
        }
        log.exiting("JGDISession", "closeAllSessions");
    }

    /**
     * get the id of the session
     * @return the id of the session
     */
    public long getId() {
        return id;
    }
}
