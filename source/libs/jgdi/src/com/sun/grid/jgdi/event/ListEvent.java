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

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

/**
 * Abstract base class for list events
 */
public abstract class ListEvent extends Event implements java.io.Serializable {
    
    /** the list of object */
    private List objects;
    /** Type of the changed objects */
    private Class objectType;
    
    /** Creates a new instance of AbstractListEvent */
    public ListEvent(long timestamp, int eventId, Class objectType) {
        super(EventType.SGE_EMA_LIST, timestamp, eventId);
        this.objectType = objectType;
    }
    
    /**
     *  Add a object
     *
     *  @param obj the object
     */
    public void add(Object obj) {
        if (obj == null) {
            throw new IllegalArgumentException("obj must not be null");
        }
        if (!objectType.isAssignableFrom(obj.getClass())) {
            throw new IllegalArgumentException("changedObj must instanceof " + objectType.getName());
        }
        if (objects == null) {
            objects = new LinkedList();
        }
        objects.add(obj);
    }
    
    /**
     * Get the objects we belongs to this list event
     * @return list of objects
     */
    public List getObjects() {
        if (objects == null) {
            return Collections.EMPTY_LIST;
        } else {
            return Collections.unmodifiableList(objects);
        }
    }
}