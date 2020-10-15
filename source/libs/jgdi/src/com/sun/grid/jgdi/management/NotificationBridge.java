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

import com.sun.grid.jgdi.EventClient;
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.event.ConnectionClosedEvent;
import com.sun.grid.jgdi.event.Event;
import com.sun.grid.jgdi.event.EventListener;
import com.sun.grid.jgdi.event.EventTypeEnum;
import com.sun.grid.jgdi.event.ShutdownEvent;
import com.sun.grid.jgdi.jni.EventClientImpl;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicLong;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.management.ListenerNotFoundException;
import javax.management.MBeanNotificationInfo;
import javax.management.Notification;
import javax.management.NotificationFilter;
import javax.management.NotificationListener;

/**
 *
 */
public class NotificationBridge implements EventListener {

    private static final Logger log = Logger.getLogger(NotificationBridge.class.getName());
    private final AtomicLong seqNumber = new AtomicLong();
    private MBeanNotificationInfo[] notificationInfos;
    private final EventClient eventClient;
    private final List<NotificationListener> listeners = new LinkedList<NotificationListener>();
    private final Map<NotificationListener, Object> handbackMap = new HashMap<NotificationListener, Object>();

    public NotificationBridge(String url) {
        eventClient = new EventClientImpl(url, 0);
        eventClient.addEventListener(this);
    }

    public void eventOccured(Event evt) {
        log.entering("NotificationBridge", "eventOccured", evt);
        Notification n = null;
        List<NotificationListener> tmpLis = null;
        synchronized (this) {
            n = NotificationBridgeFactory.createNotification(evt, seqNumber.incrementAndGet());
            if (n != null) {
                tmpLis = new ArrayList<NotificationListener>(listeners);
            } else {
                log.log(Level.WARNING, "Received unknown event {0}", evt);
            }
        }
        if (n != null) {
            for (NotificationListener lis : tmpLis) {
                log.log(Level.FINE, "send notification to {0}", lis);
                lis.handleNotification(n, handbackMap.get(lis));
            }
        }
        log.exiting("NotificationBridge", "eventOccured");
    }

    public synchronized MBeanNotificationInfo[] getMBeanNotificationInfo() {
        log.entering("NotificationBridge", "getMBeanNotificationInfo");
        if (notificationInfos == null) {
            notificationInfos = NotificationBridgeFactory.createMBeanNotificationInfo();
        }
        log.exiting("NotificationBridge", "getMBeanNotificationInfo", notificationInfos);
        return notificationInfos;
    }

    public synchronized void removeNotificationListener(NotificationListener listener) throws JGDIException, ListenerNotFoundException {
        log.entering("NotificationBridge", "removeNotificationListener", listener);
        listeners.remove(listener);
        handbackMap.remove(listener);
        log.exiting("NotificationBridge", "removeNotificationListener");
    }

    public synchronized void addNotificationListener(NotificationListener listener, NotificationFilter filter, Object handback) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("NotificationBridge", "addNotificationListener", new Object[]{listener, filter, handback});
        }
        listeners.add(listener);
        handbackMap.put(listener, handback);
        log.exiting("NotificationBridge", "addNotificationListener");
    }

    public synchronized Set<EventTypeEnum> getSubscription() throws JGDIException {
        log.entering("NotificationBridge", "getSubscription");
        Set<EventTypeEnum> ret = null;
        if(eventClient != null ) {
            ret = eventClient.getSubscription();
        } else {
            ret = Collections.<EventTypeEnum>emptySet();
        }
        log.exiting("NotificationBridge", "getSubscription", ret);
        return ret;
    }

    public synchronized void setSubscription(Set<EventTypeEnum> types) throws JGDIException {
        log.entering("NotificationBridge", "setSubscription", types);
        eventClient.setSubscription(types);
        eventClient.commit();
        log.exiting("NotificationBridge", "setSubscription");
    }

    public synchronized void subscribe(EventTypeEnum type) throws JGDIException {
        log.entering("NotificationBridge", "subscribe", type);
        eventClient.subscribe(type);
        eventClient.commit();
        log.exiting("NotificationBridge", "subscribe");
    }

    public synchronized void unsubscribe(EventTypeEnum type) throws JGDIException {
        log.entering("NotificationBridge", "unsubscribe", type);
        eventClient.unsubscribe(type);
        eventClient.commit();
        log.exiting("NotificationBridge", "unsubscribe");
    }

    public synchronized void subscribe(Set<EventTypeEnum> types) throws JGDIException {
        log.entering("NotificationBridge", "subscribe", types);
        eventClient.subscribe(types);
        eventClient.commit();
        log.exiting("NotificationBridge", "subscribe");
    }

    public synchronized void unsubscribe(Set<EventTypeEnum> types) throws JGDIException {
        log.entering("NotificationBridge", "unsubscribe", types);
        eventClient.unsubscribe(types);
        eventClient.commit();
        log.exiting("NotificationBridge", "unsubscribe");
    }

    /**
     *  Close the event notification bridge
     *
     *  The JGDI event client will be stopped.
     */
    public synchronized void close() {
        log.entering("NotificationBridge", "close");
        if(eventClient != null) {
            try {
                log.log(Level.FINE, "closing event client [{0}]", eventClient.getId());
                eventClient.close();
            } catch (Exception ex) {
            }
        }
        log.exiting("NotificationBridge", "close");
    }
}
