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
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Java Wrapper class for the JOB_USAGE event.
 */
public class JobUsageEvent extends JobEvent implements java.io.Serializable {
    
    private Map<String, Double> usage;
    
    /** Creates a new instance of JobFinishEvent */
    public JobUsageEvent(long timestamp, int evtId) {
        super(timestamp, evtId);
    }
    
    public void addUsage(String name, double value) {
        if (usage == null) {
            usage = new HashMap<String, Double>();
        }
        usage.put(name, new Double(value));
    }
    
    public Set<String> getLoadValueNames() {
        if (usage == null) {
            return Collections.EMPTY_SET;
        } else {
            return usage.keySet();
        }
    }
    
    public Double getLoadValue(String name) {
        if (usage == null) {
            throw new IllegalStateException("Have no load values");
        }
        return usage.get(name);
    }
    
    public Map<String, Double> getUsage() {
        if (usage == null) {
            return Collections.EMPTY_MAP;
        }
        return Collections.unmodifiableMap(usage);
    }
}