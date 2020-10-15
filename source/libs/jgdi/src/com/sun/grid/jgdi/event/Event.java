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
package com.sun.grid.jgdi.event;
import java.text.DateFormat;
import java.util.Date;

/**
 *  Base class for all events
 */
public class Event implements java.io.Serializable {
    
    
    /** event type (SGE_EMA_LIST, SGE_EMA_ADD, SGE_EMA_MOD,
     *  SGE_EMA_DEL or SGE_EMA_TRIGGER )*/
    private EventType eventType;
    
    /** timestamp when the event occured */
    private long timestamp;
    
    /** unique event id */
    private int eventId;
    
    /**
     *  event type (add, del, mod)
     */
    public EventType getType() {
        return eventType;
    }
    
    
    /**
     *  Timestamp when the event occured
     */
    public long getTimestamp() {
        return timestamp;
    }
    
    /**
     *  get the event id
     */
    public int getEventId() {
        return eventId;
    }
    
    public String toString() {
        Date date = new Date(1000*getTimestamp());
        return "Id/Timestamp/Type/class: " + getEventId() + "/" + DateFormat.getTimeInstance().format(date) + "/" + getType() + "/" + getClass().getName();
    }
    
    /**
     * Create a new instance of event
     * @param eventType  event type (SGE_EMA_LIST, SGE_EMA_ADD, SGE_EMA_MOD,
     *                               SGE_EMA_DEL or SGE_EMA_TRIGGER )
     * @param timestamp  timestamp when the event occurect
     * @param eventId    unique event id
     */
    public Event(EventType eventType, long timestamp, int eventId) {
        this.eventType = eventType;
        this.timestamp = timestamp;
        this.eventId = eventId;
    }
    
}
