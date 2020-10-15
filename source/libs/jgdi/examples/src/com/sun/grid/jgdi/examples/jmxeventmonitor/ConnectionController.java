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
 *   Copyright: 2006 by Sun Microsystems, Inc
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jgdi.examples.jmxeventmonitor;

import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.event.ConnectionClosedEvent;
import com.sun.grid.jgdi.event.ConnectionFailedEvent;
import com.sun.grid.jgdi.event.Event;
import com.sun.grid.jgdi.event.EventListener;
import com.sun.grid.jgdi.event.EventTypeEnum;
import com.sun.grid.jgdi.event.QmasterGoesDownEvent;
import com.sun.grid.jgdi.event.ShutdownEvent;
import com.sun.grid.jgdi.management.JGDIProxy;
import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 */
public class ConnectionController {

    private final static Logger log = Logger.getLogger(ConnectionController.class.getName());
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private final Set<Listener> listeners = new HashSet<Listener>();
    private final Set<EventListener> eventListeners = new HashSet<EventListener>();
    private JGDIProxy jgdiProxy;
    private boolean useSSL = false;
    private File caTop;
    private String serverHostname;
    private int serverPort;

    private final EventListener shutdownListener = new EventListener() {

        public void eventOccured(Event evt) {
            if (evt instanceof QmasterGoesDownEvent || evt instanceof ConnectionClosedEvent || evt instanceof ConnectionFailedEvent) {
                disconnect();
            } else if (evt instanceof ShutdownEvent) {
                executor.submit(new ClearSubscriptionAction());
            }
        }
    };

    public ConnectionController() {
        eventListeners.add(shutdownListener);
    }

    public void connect(String host, int port, Object credentials, String caTop, String keyStore, char[] pw) {
        executor.submit(new ConnectAction(host, port, credentials, caTop, keyStore, pw));
    }

    public void disconnect() {
        executor.submit(new DisconnectAction());
    }

    public void setSubscription(Set<EventTypeEnum> types) {
        executor.submit(new SetSubscriptionAction(types));
    }

    public void subscribeAll() {
        EventTypeEnum[] values = EventTypeEnum.values();
        Set<EventTypeEnum> subs = new HashSet<EventTypeEnum>(values.length);
        for (EventTypeEnum value : values) {
            subs.add(value);
        }
        subscribe(subs);
    }

    public void unsubscribeAll() {
        executor.submit(new UnsubscribeAllAction());
    }

    public void subscribe(Set<EventTypeEnum> types) {
        executor.submit(new SubscribeAction(types, true));
    }

    public void unsubscribe(Set<EventTypeEnum> types) {
        executor.submit(new SubscribeAction(types, false));
    }

    public void addListener(Listener lis) {
        synchronized (listeners) {
            listeners.add(lis);
        }
    }

    public void removeListener(Listener lis) {
        synchronized (listeners) {
            listeners.remove(lis);
        }
    }

    private List<Listener> getListeners() {
        synchronized (listeners) {
            return new ArrayList<Listener>(listeners);
        }
    }

    public void addEventListener(EventListener lis) {
        eventListeners.add(lis);
    }

    public void removeEventListener(EventListener lis) {
        eventListeners.remove(lis);
    }

    private class UnsubscribeAllAction implements Runnable {

        public void run() {
            try {
                if (jgdiProxy != null) {
                    jgdiProxy.getProxy().setSubscription(Collections.<EventTypeEnum>emptySet());
                }
                for (Listener lis : getListeners()) {
                    lis.clearSubscription();
                }
            } catch (Exception ex) {
                for (Listener lis : getListeners()) {
                    lis.errorOccured(ex);
                }
            }
        }
    }

    private class ClearSubscriptionAction implements Runnable {

        public void run() {
            for (Listener lis : getListeners()) {
                lis.clearSubscription();
            }
        }
    }

    private class SetSubscriptionAction implements Runnable {

        private final Set<EventTypeEnum> types;

        public SetSubscriptionAction(Set<EventTypeEnum> types) {
            this.types = types;

        }

        public void run() {
            try {
                if (jgdiProxy != null) {
                    jgdiProxy.getProxy().setSubscription(types);
                    for (Listener lis : getListeners()) {
                        lis.subscriptionSet(types);
                    }
                }
            } catch (Exception ex) {
                for (Listener lis : getListeners()) {
                    lis.errorOccured(ex);
                }
            }
        }
    }

    private class SubscribeAction implements Runnable {

        private final Set<EventTypeEnum> types;
        private final boolean subscribe;

        public SubscribeAction(Set<EventTypeEnum> types, boolean subscribe) {
            this.types = types;
            this.subscribe = subscribe;
        }

        public void run() {

            try {
                if (jgdiProxy != null) {

                    if (subscribe) {
                        jgdiProxy.getProxy().subscribe(types);
                        for (Listener lis : getListeners()) {
                            lis.subscribed(types);
                        }
                    } else {
                        jgdiProxy.getProxy().unsubscribe(types);
                        for (Listener lis : getListeners()) {
                            lis.unsubscribed(types);
                        }
                    }
                }
            } catch (Exception ex) {
                for (Listener lis : getListeners()) {
                    lis.errorOccured(ex);
                }
            }
        }
    }

    private class DisconnectAction implements Runnable {

        public void run() {

            try {
                if (jgdiProxy != null) {
                    jgdiProxy.close();
                    if (useSSL) {
                        JGDIProxy.resetSSL(serverHostname, serverPort, caTop);
                    }
                }
            } catch (Exception ex) {
                log.log(Level.WARNING, "connector.close failed", ex);
            } finally {
                jgdiProxy = null;
                for (Listener lis : getListeners()) {
                    lis.disconnected();
                }
            }
        }
    }

    private class ConnectAction implements Runnable {

        private final Object credentials;
        private final File keyStore;
        private final char[] pw;

        public ConnectAction(String myhost, int myport, Object credentials, String caTopPath, String keyStorePath, char[] pw) {
            serverHostname = myhost;
            serverPort = myport;
            this.credentials = credentials;
            if (caTopPath != null) {
                useSSL = true;
                caTop = new File(caTopPath);
                this.keyStore = new File(keyStorePath);
                this.pw = pw;
            } else {
                useSSL = false;
                caTop = null;
                this.keyStore = null;
                this.pw = null;
            }
        }

        public void run() {
            try {
                if (useSSL) {
                    JGDIProxy.setupSSL(serverHostname, serverPort, caTop, keyStore, pw);
                }
                jgdiProxy = JGDIFactory.newJMXInstance(serverHostname, serverPort, credentials);
                for (EventListener lis : eventListeners) {
                    jgdiProxy.addEventListener(lis);
                }
                Set<EventTypeEnum> subscription = jgdiProxy.getProxy().getSubscription();
                for (Listener lis : getListeners()) {
                    lis.connected(serverHostname, serverPort, subscription);
                }
            } catch (Throwable ex) {
                for (Listener lis : getListeners()) {
                    lis.errorOccured(ex);
                }
            }
        }
    }

    public static interface Listener {

        public void connected(String host, int port, Set<EventTypeEnum> subscription);

        public void disconnected();

        public void subscriptionSet(Set<EventTypeEnum> types);

        public void subscribed(Set<EventTypeEnum> types);

        public void unsubscribed(Set<EventTypeEnum> typea);

        public void errorOccured(Throwable ex);

        public void clearSubscription();
    }
}
