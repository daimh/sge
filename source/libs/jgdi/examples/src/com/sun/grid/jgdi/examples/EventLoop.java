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
package com.sun.grid.jgdi.examples;

import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.event.Event;
import com.sun.grid.jgdi.event.EventListener;
import com.sun.grid.jgdi.event.EventTypeEnum;
import com.sun.grid.jgdi.management.JGDIProxy;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.logging.Logger;

/**
 *
 */
public class EventLoop implements EventListener {

    Logger log = Logger.getLogger(EventLoop.class.getName());
    
    public static void main(String[] args) {
        String master = args[0];
        int port = Integer.valueOf(args[1]);
        Object credentials = new String[]{System.getProperty("user.name"), "aa"};

        EventLoop loop = new EventLoop(master, port, credentials);
        try {
            while(true) {
                loop.run();
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
    private String master;
    private int port;
    private Object credentials;

    public EventLoop(String master, int port, Object credentials) {
        this.master = master;
        this.port = port;
        this.credentials = credentials;
    }

    public void run() throws Exception {

        log.info("connecting");
        
        JGDIProxy jgdiProxy = JGDIFactory.newJMXInstance(master, port, credentials);

        Set<EventTypeEnum> subscription = new HashSet<EventTypeEnum>(4);

        
        subscription.add(EventTypeEnum.ExecHostList);
        subscription.add(EventTypeEnum.ExecHostAdd);
        subscription.add(EventTypeEnum.ExecHostDel);
        subscription.add(EventTypeEnum.ExecHostMod);
        
        log.info("addEventListener");

        jgdiProxy.addEventListener(this);
        
        log.info("setSubscription");
        
        jgdiProxy.getProxy().setSubscription(subscription);

        log.info("connected");
        
        lock.lock();
        try {
            cond.await();
        } finally {
            lock.unlock();
        }
        log.info("disconnecting");
        jgdiProxy.close();
        log.info("disconnected");

    }
    
    private Lock lock = new ReentrantLock();
    private Condition cond = lock.newCondition();
    int eventCount = 0;

    public void eventOccured(Event evt) {
        log.info("eventOccured " + evt);
        lock.lock();
        try {
            eventCount++;
            if(eventCount > 3) {
               cond.signalAll(); 
            }
        } finally {
            lock.unlock();
        }
    }
}
