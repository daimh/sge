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

/**
 * Base class of all events which signalizes a changes of
 * objects in the Sun [tm] Grid Engine.
 *
 */
public abstract class ChangedObjectEvent extends Event implements java.io.Serializable {
    
    /** the new version of the changed objects */
    private Object changedObject;
    private Class objectType;
    
    /**
     * Creates a new instance of ChangedObjectEvent
     * @param eventType   type of event
     * @param timestamp   timestamp when the event occured
     * @param eventId     id of the event
     * @param objectType  type of the changed objects
     */
    public ChangedObjectEvent(EventType eventType, long timestamp, int eventId, Class objectType) {
        super(eventType, timestamp, eventId);
        this.objectType = objectType;
    }
    
    /**
     *  Get the type of the changed objects
     *  @return type of the changed objects
     */
    public Class getObjectType() {
        return objectType;
    }
    
    /**
     *  Set the new version of the changed object
     *  @param changedObj  the changed object
     *  @throws IllegalArgumentException if <code>changedObj</code> is <code>null</code>
     *  @throws IllegalArgumentException if the class object type for the event is not
     *                                   assignable from the class of <code>changedObj</code>
     *  @see #getObjectType
     */
    public void setChangedObject(Object changedObj) {
        if (changedObj == null) {
            throw new IllegalArgumentException("changedObj must not be null");
        }
        if (!objectType.isAssignableFrom(changedObj.getClass())) {
            throw new IllegalArgumentException("changedObj must instanceof " + objectType.getName());
        }
        this.changedObject = changedObj;
    }
    
    /**
     *  Get the new version of the changed object which has been changed by this event
     *  @return list of changed objects
     */
    public Object getChangedObject() {
        return changedObject;
    }
    
    public String toString() {
        StringBuilder ret = new StringBuilder();
        ret.append("[");
        ret.append(super.toString());
        ret.append(", ");
        ret.append(changedObject);
        ret.append("]");
        return ret.toString();
    }
    
    /**
     *  Set the primary key information of the changed object into the event.
     *
     *  <p>This method is called from the the native part of jgdi to fill the
     *     primary key information into a del event</p>
     *
     *  @param  numKey1  first numerical primary key of the deleted object
     *  @param  numKey2  second numerical primary key of the deleted object
     *  @param  strKey1  first string primary key of the deleted object
     *  @param  strKey2  second string primary key of the deleted object
     */
    public abstract void setPKInfo(int numKey1, int numKey2, String strKey1, String strKey2);
}
