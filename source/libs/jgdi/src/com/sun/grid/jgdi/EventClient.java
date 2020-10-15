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
package com.sun.grid.jgdi;

import com.sun.grid.jgdi.event.EventListener;
import com.sun.grid.jgdi.event.EventTypeEnum;
import java.util.Map;
import java.util.Set;


/**
 *
 */
public interface EventClient {
    
    /**
     * Get the id of this event client
     * @return the event client id
     */
    public int getId();
    
    /**
     *  Close this event client
     * @throws com.sun.grid.jgdi.JGDIException if the close operation failed
     * @throws java.lang.InterruptedException if closing the event client has been interrupted
     */
    public void close() throws JGDIException, InterruptedException;
    
    /**
     *  Determine if the event client is running
     *  @return <code>true</code> if the event client is running
     */
    public boolean isRunning();
    
    /**
     *  Determine if the event client has been closed
     *  @return <code>true</code> if the event client has been closed
     */
    public boolean isClosed();
    
    /**
     *  Subscribe all events for this event client
     *  @throws JGDIException if subscribe failed
     */
    public void subscribeAll() throws JGDIException;
    
    /**
     *  Unsubscribe all events for this event client
     *  @throws JGDIException if unsubscribe failed
     */
    public void unsubscribeAll() throws JGDIException;
    
    public void subscribe(EventTypeEnum type) throws JGDIException;
    public void unsubscribe(EventTypeEnum type) throws JGDIException;
    public void subscribe(Set<EventTypeEnum> types) throws JGDIException;
    public void unsubscribe(Set<EventTypeEnum> types) throws JGDIException;
    
    public void setSubscription(Set<EventTypeEnum> types) throws JGDIException;
    
    public void setFlush(Map<EventTypeEnum,Integer> map) throws JGDIException;
    public void setFlush(EventTypeEnum type, int time) throws JGDIException;
    public int getFlush(EventTypeEnum type) throws JGDIException;
    
    public Set<EventTypeEnum> getSubscription() throws JGDIException;
    /**
     * Add an event listener to this event client
     * @param lis the event listener
     */
    public void addEventListener(EventListener lis);
    
    /**
     * Remove an event listener from this event client
     * @param lis the event listener
     */
    public void removeEventListener(EventListener lis);
    
    /**
     * Commit the subscription
     * @throws JGDIException if the commit has failed
     */
    public void commit() throws JGDIException;
}