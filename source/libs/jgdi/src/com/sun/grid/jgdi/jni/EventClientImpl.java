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
package com.sun.grid.jgdi.jni;

import com.sun.grid.jgdi.EventClient;
import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.event.Event;
import com.sun.grid.jgdi.event.Event;
import com.sun.grid.jgdi.event.EventListener;
import com.sun.grid.jgdi.event.EventTypeEnum;
import com.sun.grid.jgdi.event.EventTypeMapping;
import com.sun.grid.jgdi.event.QmasterGoesDownEvent;
import com.sun.grid.jgdi.event.ShutdownEvent;
import java.util.ArrayList;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * event client implementation
 *
 */
public class EventClientImpl implements EventClient {

    private static final Logger log = Logger.getLogger("com.sun.grid.jgdi.event");
    private static List<EventClientImpl> instances = new LinkedList<EventClientImpl>();
    private static volatile boolean shutdown = false;
    private Set<EventListener> eventListeners = Collections.<EventListener>emptySet();
    private final Lock listenerLock = new ReentrantLock();
    private final Lock executorLock = new ReentrantLock();
    private ExecutorService executor = null;
    private Future eventLoopFuture = null;
    private final EventLoopAction eventLoop;

    /**
     * Creates a new instance of EventClientImpl
     * @param url  JGDI connection url in the form
     *             <code>bootstrap://&lt;SGE_ROOT&gt;@&lt;SGE_CELL&gt;:&lt;SGE_QMASTER_PORT&gt;</code>
     * @param regId  Registration id for the event client (0 => qmaster assignes a
     *               event client id dynamically)
     */
    public EventClientImpl(String url, int regId) {
        shutdown = false;
        eventLoop = new EventLoopAction(url, regId);
        synchronized (instances) {
            instances.add(this);
        }
    }

    /**
     * Get the id of this event client
     * @return the event client id
     */
    public int getId() {
        log.entering(getClass().getName(), "getId");
        int ret = eventLoop.getId();
        log.exiting(getClass().getName(), "getId", ret);
        return ret;
    }

    public static void resetShutdown() {
        synchronized (instances) {
            shutdown = false;
        }
    }

    /**
     * Close all instances of the EventClientImpl
     */
    public static void closeAll() {
        log.entering(EventClientImpl.class.getName(), "closeAll");
        List<EventClientImpl> currentInstances = null;
        synchronized (instances) {
            currentInstances = new ArrayList<EventClientImpl>(instances);
            shutdown = true;
        }

        for (EventClientImpl evt : currentInstances) {
            try {
                evt.close();
            } catch (Exception ex) {
            // Ignore
            }
        }
        log.exiting(EventClientImpl.class.getName(), "closeAll");
    }

    /**
     *  Close this event client
     */
    public void close() {
        log.entering(getClass().getName(), "close");
        executorLock.lock();
        try {
            // Stop the event loop execution
            if (eventLoopFuture != null) {
                try {
                    if (!eventLoopFuture.isCancelled()) {
                        eventLoopFuture.cancel(true);
                        eventLoop.interrupt();
                    }
                    if (!eventLoopFuture.isDone()) {
                        eventLoopFuture.get();
                    }
                } catch (Exception ex) {
                    log.log(Level.WARNING, "Stopping of event loop failed", ex);
                } finally {
                    eventLoopFuture = null;
                }
            }
            if (executor != null && !executor.isShutdown()) {
                executor.shutdown();
                try {
                    executor.awaitTermination(60, TimeUnit.SECONDS);
                } catch (InterruptedException ex) {
                // Ingore
                } finally {
                    executor = null;
                }
            }
        } catch (Throwable ex) {
            log.log(Level.WARNING, "Close of event client failed", ex);
        } finally {
            executorLock.unlock();
            synchronized (instances) {
                instances.remove(this);
            }
        }
        log.exiting(getClass().getName(), "close");
    }

    /**
     *  Determine if the event client has been closed
     *  @return <code>true</code> if the event client has been closed
     */
    public boolean isClosed() {
        boolean ret = false;
        log.entering(getClass().getName(), "isClosed");
        ret = eventLoop.getState().equals(State.STOPPED);
        log.exiting(getClass().getName(), "isClosed", ret);
        return ret;
    }

    /**
     * Determine if the event client is running
     * @return <code>true</code> if the event client is running
     */
    public boolean isRunning() {
        boolean ret = false;
        log.entering(getClass().getName(), "isRunning");
        ret = eventLoop.getState().equals(State.RUNNING);
        log.exiting(getClass().getName(), "isRunning", ret);
        return ret;
    }

    protected void fireEventOccured(Event evt) {
        log.entering(getClass().getName(), "fireEventOccured", evt);
        log.log(Level.FINER, "fire event {0}", evt);
        Set<EventListener> tmp = null;
        listenerLock.lock();
        try {
            tmp = eventListeners;
        } finally {
            listenerLock.unlock();
        }

        for (EventListener lis : tmp) {
            lis.eventOccured(evt);
        }
        log.exiting(getClass().getName(), "fireEventOccured");
    }

    /**
     * Add an event listener to this event client
     * @param lis the event listener
     */
    public void addEventListener(EventListener lis) {
        log.entering(getClass().getName(), "addEventListener", lis);

        listenerLock.lock();
        try {
            Set<EventListener> tmp = eventListeners;
            eventListeners = new HashSet<EventListener>(tmp.size() + 1);
            eventListeners.addAll(tmp);
            eventListeners.add(lis);
        } finally {
            listenerLock.unlock();
        }
        log.exiting(getClass().getName(), "addEventListener");
    }

    /**
     * Remove an event listener from this event client
     * @param lis the event listener
     */
    public void removeEventListener(EventListener lis) {
        log.entering(getClass().getName(), "removeEventListener", lis);
        listenerLock.lock();
        try {
            eventListeners = new HashSet<EventListener>(eventListeners);
            eventListeners.remove(lis);
        } finally {
            listenerLock.unlock();
        }
        log.exiting(getClass().getName(), "removeEventListener");
    }

    /**
     *  Start the event client
     */
    private void start() throws JGDIException {
        log.entering(getClass().getName(), "start");
        boolean started = false;
        executorLock.lock();
        try {
            if (eventLoop.getState().equals(State.STOPPED)) {
                if (shutdown) {
                    throw new JGDIException("Can not start event client, shutdown is in progress");
                }
                // the close method is smart enough to do nothing
                // if the event client is already stopped
                // However it can happen that the executor thread is still
                // active => close it
                close();
                executor = Executors.newSingleThreadExecutor();
                eventLoopFuture = executor.submit(eventLoop);
                started = true;
            }
        } finally {
            executorLock.unlock();
        }
        if (started) {
            log.log(Level.FINE, "Waiting for worker thread startup");
            eventLoop.waitForStartup();
            log.log(Level.FINE, "worker thread started");
        }
        log.exiting(getClass().getName(), "start");
    }

    /**
     * Commit the changed subscription
     * @throws com.sun.grid.jgdi.JGDIException
     */
    public void commit() throws JGDIException {
        log.entering(getClass().getName(), "commit");
        // The first commit starts the event client automatically
        start();
        // Commit the changes
        eventLoop.commit();
        log.exiting(getClass().getName(), "commit");
    }

    /**
     * Subscribe an additional event.
     * @param type the event type
     */
    public void subscribe(EventTypeEnum type) {
        log.entering(getClass().getName(), "subscribe", type);
        eventLoop.subscribe(type);
        log.exiting(getClass().getName(), "subscribe");
    }

    /**
     * Add a set of events to the subscription
     * @param types set of event types
     */
    public void subscribe(Set<EventTypeEnum> types) {
        log.entering(getClass().getName(), "subscribe", types);
        eventLoop.subscribe(types);
        log.exiting(getClass().getName(), "subscribe");
    }

    /**
     * Remove a event from the subscription
     * @param type the event type
     */
    public void unsubscribe(EventTypeEnum type) {
        log.entering(getClass().getName(), "unsubscribe", type);
        eventLoop.unsubscribe(type);
        log.exiting(getClass().getName(), "unsubscribe");
    }

    /**
     * Remove a set of events from the subscription
     * @param types the set of events
     */
    public void unsubscribe(Set<EventTypeEnum> types) {
        log.entering(getClass().getName(), "unsubscribe", types);
        eventLoop.unsubscribe(types);
        log.exiting(getClass().getName(), "unsubscribe");
    }

    /**
     *  Subscribe all events for this event client
     */
    public void subscribeAll() {
        log.entering(getClass().getName(), "subscribeAll");
        eventLoop.subscribeAll();
        log.exiting(getClass().getName(), "subscribeAll");
    }

    /**
     *  Unsubscribe all events for this event client
     */
    public void unsubscribeAll() {
        log.entering(getClass().getName(), "unsubscribeAll");
        eventLoop.unsubscribeAll();
        log.exiting(getClass().getName(), "unsubscribeAll");
    }

    /**
     * Set the subscription of this event client
     * @param types set of event types
     */
    public void setSubscription(Set<EventTypeEnum> types) {
        log.entering(getClass().getName(), "setSubscription", types);
        eventLoop.setSubscription(types);
        log.exiting(getClass().getName(), "setSubscription");
    }

    /**
     * Get the current subscriptions
     * @return the current subscription
     */
    public Set<EventTypeEnum> getSubscription() {
        log.entering(getClass().getName(), "getSubscription");
        Set<EventTypeEnum> ret = eventLoop.getSubscription();
        log.exiting(getClass().getName(), "getSubscription", ret);
        return ret;
    }

    /**
     * Set the flush time for subscribed events
     * @param map   map with event type as key and flush time as value
     */
    public void setFlush(Map<EventTypeEnum, Integer> map) {
        log.entering(getClass().getName(), "setFlush", map);
        eventLoop.setFlush(map);
        log.exiting(getClass().getName(), "setFlush");
    }

    /**
     * Set the flush time for subscribed event
     * @param type the event type
     * @param time the flush time
     */
    public void setFlush(EventTypeEnum type, int time) {
        if (log.isLoggable(Level.FINER)) {
            log.entering(getClass().getName(), "setFlush", new Object[]{type, time});
        }
        eventLoop.setFlush(type, time);
        log.exiting(getClass().getName(), "setFlush");
    }

    /**
     * Get the flush time of a event
     * @param type  the event type
     * @return the flush time
     */
    public int getFlush(EventTypeEnum type) {
        log.entering(getClass().getName(), "getFlush", type);
        int ret = eventLoop.getFlush(type);
        log.exiting(getClass().getName(), "setFlush", ret);
        return ret;
    }

    private native void commitNative(int evcIndex) throws JGDIException;

    private native void setFlushNative(int evcIndex, int type, boolean flush, int time) throws JGDIException;

    private native int getFlushNative(int evcIndex, int type) throws JGDIException;

    private native void interruptNative(int evcIndex) throws JGDIException;

    private native void closeNative(int evcIndex) throws JGDIException;

    private native int initNative(JGDI jgdi, int regId) throws JGDIException;

    private native int registerNative(int evcIndex) throws JGDIException;

    private native void fillEvents(int evcIndex, List eventList) throws JGDIException;

    private native void subscribeNative(int evcIndex, int evenType, boolean subcribe) throws JGDIException;

    enum State {

        STARTING,
        RUNNING,
        STOPPING,
        STOPPED,
        ERROR
    }
    private static EventTypeEnum[] ALWAYS_SUBSCRIBED = {
        EventTypeEnum.QmasterGoesDown, EventTypeEnum.Shutdown
    };

    private class EventLoopAction implements Runnable {

        private State state = State.STOPPED;
        private final String url;
        private final int regId;
        private final Lock lock = new ReentrantLock();
        private final Condition stateChangedCondition = lock.newCondition();
        private final Condition commitCondition = lock.newCondition();
        private final Map<EventTypeEnum, Integer> subscription = new HashMap<EventTypeEnum, Integer>();
        private final Set<EventTypeEnum> nativeSubscription = new HashSet<EventTypeEnum>();
        private int evcId;
        private int evcIndex;
        private boolean commitFlag;
        private JGDIException error;

        public EventLoopAction(String url, int regId) {
            this.url = url;
            this.regId = regId;
            for (EventTypeEnum type : ALWAYS_SUBSCRIBED) {
                subscription.put(type, null);
                nativeSubscription.add(type);
            }
        }

        public int getId() {
            lock.lock();
            try {
                return evcId;
            } finally {
                lock.unlock();
            }
        }

        public void commit() throws JGDIException {
            lock.lock();
            try {
                if (!state.equals(State.RUNNING)) {
                    throw new JGDIException("Cannot commit, event client is not started");
                }
                commitFlag = true;
                interruptNative(evcIndex);
                commitCondition.await();
            } catch (InterruptedException ex) {
                // ignore
            } finally {
                lock.unlock();
            }
        }

        public void interrupt() throws JGDIException {
            lock.lock();
            try {
                if (evcIndex >= 0) {
                    interruptNative(evcIndex);
                }
            } finally {
                lock.unlock();
            }
        }

        private void setState(State state) {
            lock.lock();
            try {
                if (!this.state.equals(state)) {
                    this.state = state;
                    stateChangedCondition.signalAll();
                }
            } finally {
                lock.unlock();
            }
        }

        public State getState() {
            lock.lock();
            try {
                return state;
            } finally {
                lock.unlock();
            }
        }

        private void error(JGDIException ex) {
            lock.lock();
            try {
                this.error = ex;
                this.state = State.ERROR;
                stateChangedCondition.signalAll();
            } finally {
                lock.unlock();
            }
            log.log(Level.WARNING, "error in event loop", ex);
        }

        public void waitForStartup() throws JGDIException {
            lock.lock();
            try {
                while (true) {
                    switch (state) {
                        case RUNNING:
                            return;
                        case ERROR:
                            throw error;
                        case STOPPING:
                            throw new JGDIException("event client is going down");
                        default:
                            stateChangedCondition.await();
                    }

                }
            } catch (InterruptedException ex) {
                throw new JGDIException(ex, "Startup of event client has been interrupted");
            } finally {
                lock.unlock();
            }
        }

        public boolean isAlwaysSubscribed(EventTypeEnum type) {
            for (EventTypeEnum tmpType : ALWAYS_SUBSCRIBED) {
                if (tmpType.equals(type)) {
                    return true;
                }
            }
            return false;
        }

        private void doCommit(int evcIndex) throws JGDIException {
            log.entering(getClass().getName(), "doCommit");
            lock.lock();
            try {
                if (commitFlag) {
                    log.log(Level.FINE, "commiting subscription");
                    commitFlag = false;
                    Set<EventTypeEnum> rmSub = new HashSet<EventTypeEnum>(nativeSubscription);
                    Set<EventTypeEnum> newSub = new HashSet<EventTypeEnum>();
                    for (EventTypeEnum type : subscription.keySet()) {
                        if (!rmSub.remove(type)) {
                            newSub.add(type);
                        }
                    }
                    for (EventTypeEnum type : rmSub) {
                        if (!isAlwaysSubscribed(type)) {
                            log.log(Level.FINE, "unsubscribing event {0}", type);
                            subscribeNative(evcIndex, EventTypeMapping.getNativeEventType(type), false);
                            nativeSubscription.remove(type);
                        }
                    }
                    for (EventTypeEnum type : newSub) {
                        if (!isAlwaysSubscribed(type)) {
                            log.log(Level.FINE, "subscribing event {0}", type);
                            subscribeNative(evcIndex, EventTypeMapping.getNativeEventType(type), true);
                            nativeSubscription.add(type);
                        }
                    }

                    for (Map.Entry<EventTypeEnum, Integer> entry : subscription.entrySet()) {
                        if (entry.getValue() != null) {
                            if (log.isLoggable(Level.FINE)) {
                                log.log(Level.FINE, "setting flush for event {0} = {1}", new Object[]{entry.getKey(), entry.getValue()});
                            }
                            setFlushNative(evcIndex, EventTypeMapping.getNativeEventType(entry.getKey()), true, entry.getValue());
                        }
                    }
                    commitNative(evcIndex);
                    commitCondition.signalAll();
                }
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "doCommit", null);
        }

        public void run() {
            log.entering(getClass().getName(), "run");

            JGDI jgdi = null;
            try {
                setState(State.STARTING);
                jgdi = JGDIFactory.newInstance(url);

                log.log(Level.FINER, "calling initNative({0})", regId);
                evcIndex = initNative(jgdi, regId);

                log.log(Level.FINER, "calling registerNative({0})", evcIndex);
                int id = registerNative(evcIndex);
                log.log(Level.FINER, "event client registered (id={0})", id);
                lock.lock();
                try {
                    this.evcId = id;
                } finally {
                    lock.unlock();
                }
                setState(State.RUNNING);

                boolean gotShutdownEvent = false;
                List<Event> eventList = new ArrayList<Event>();

                while (true) {
                    if (Thread.currentThread().isInterrupted()) {
                        log.log(Level.FINE, "event loop has been interrupted");
                        break;
                    }
                    if (gotShutdownEvent) {
                        break;
                    }

                    doCommit(evcIndex);

                    log.log(Level.FINER, "calling native method fillEvents for event client {0}", evcId);
                    fillEvents(evcIndex, eventList);

                    if (log.isLoggable(Level.FINER)) {
                        log.log(Level.FINE, "got {0} events from qmaster for event client {1}",
                                new Object[]{eventList.size(), evcId});
                    }

                    if (Thread.currentThread().isInterrupted()) {
                        log.log(Level.FINE, "event loop has been interrupted");
                        break;
                    }
                    for (Event event : eventList) {
                        try {
                            fireEventOccured(event);
                            if (event instanceof QmasterGoesDownEvent || event instanceof ShutdownEvent) {
                                if (gotShutdownEvent == false) {
                                    log.log(Level.FINE, "got shutdown event");
                                    gotShutdownEvent = true;
                                }
                            }
                        } catch (Exception e) {
                            log.log(Level.WARNING, "error in fire event", e);
                        }
                    }
                    eventList.clear();
                }
            } catch (JGDIException ex) {
                error(ex);
            } catch (Throwable ex) {
                error(new JGDIException(ex, "Unknown error in event loop"));
            } finally {
                setState(State.STOPPING);
                if (evcIndex >= 0) {
                    try {
                        closeNative(evcIndex);
                    } catch (Exception ex) {
                    // Ignore
                    } finally {
                        evcIndex = -1;
                    }
                }
                if (jgdi != null) {
                    try {
                        jgdi.close();
                    } catch (Exception ex) {
                    // Igore
                    }
                }
                setState(State.STOPPED);
                subscription.clear();
                nativeSubscription.clear();
                for (EventTypeEnum type : ALWAYS_SUBSCRIBED) {
                    subscription.put(type, null);
                    nativeSubscription.add(type);
                }
                commitCondition.signalAll();
            }
            log.exiting(getClass().getName(), "run");
        }

        public void subscribe(EventTypeEnum type) {
            log.entering(getClass().getName(), "subscribe", type);
            lock.lock();
            try {
                subscription.put(type, null);
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "subscribe");
        }

        public void subscribe(Set<EventTypeEnum> types) {
            log.entering(getClass().getName(), "subscribe", types);
            lock.lock();
            try {
                for (EventTypeEnum type : types) {
                    subscription.put(type, null);
                }
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "subscribe");
        }

        public void unsubscribe(EventTypeEnum type) {
            log.entering(getClass().getName(), "unsubscribe", type);

            lock.lock();
            try {
                if (!isAlwaysSubscribed(type)) {
                    subscription.remove(type);
                }
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "unsubscribe");
        }

        public void unsubscribe(Set<EventTypeEnum> types) {
            log.entering(getClass().getName(), "unsubscribe", types);

            lock.lock();
            try {
                for (EventTypeEnum type : types) {
                    if (!isAlwaysSubscribed(type)) {
                        subscription.remove(type);
                    }
                }
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "unsubscribe");
        }

        /**
         *  Subscribe all events for this event client
         *  @throws JGDIException if subscribe failed
         */
        public void subscribeAll() {
            log.entering(getClass().getName(), "subscribeAll");
            lock.lock();
            try {
                subscription.clear();
                for (EventTypeEnum type : EventTypeEnum.values()) {
                    subscription.put(type, null);
                }
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "subscribeAll");
        }

        /**
         *  Unsubscribe all events for this event client
         *  @throws JGDIException if unsubscribe failed
         */
        public void unsubscribeAll() {
            log.entering(getClass().getName(), "unsubscribeAll");
            lock.lock();
            try {
                subscription.clear();
                for (EventTypeEnum type : ALWAYS_SUBSCRIBED) {
                    subscription.put(type, null);
                }
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "unsubscribeAll");
        }

        public void setSubscription(Set<EventTypeEnum> types) {
            log.entering(getClass().getName(), "setSubscription", types);
            lock.lock();
            try {
                subscription.clear();
                for (EventTypeEnum type : ALWAYS_SUBSCRIBED) {
                    subscription.put(type, null);
                }
                subscribe(types);
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "setSubscription");
        }

        public Set<EventTypeEnum> getSubscription() {
            log.entering(getClass().getName(), "getSubscription");
            Set<EventTypeEnum> ret = null;
            lock.lock();
            try {
                ret = new HashSet<EventTypeEnum>(subscription.keySet());
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "getSubscription", ret);
            return ret;
        }

        public void setFlush(Map<EventTypeEnum, Integer> map) {
            log.entering(getClass().getName(), "setFlush", map);
            lock.lock();
            try {
                for (Map.Entry<EventTypeEnum, Integer> entry : map.entrySet()) {
                    setFlush(entry.getKey(), entry.getValue());
                }
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "setFlush");
        }

        public void setFlush(EventTypeEnum type, int time) {
            if (log.isLoggable(Level.FINER)) {
                log.entering(getClass().getName(), "setFlush", new Object[]{type, time});
            }
            lock.lock();
            try {
                if (subscription.containsKey(type)) {
                    subscription.put(type, time);
                }
            } finally {
                lock.unlock();
            }
            log.exiting(getClass().getName(), "setFlush");
        }

        public int getFlush(EventTypeEnum type) {
            log.entering(getClass().getName(), "getFlush", type);
            Integer retObj = null;
            lock.lock();
            try {
                retObj = subscription.get(type);
            } finally {
                lock.unlock();
            }
            int ret = (retObj != null) ? retObj.intValue() : 0;
            log.exiting(getClass().getName(), "setFlush", ret);
            return ret;
        }
    }
}
