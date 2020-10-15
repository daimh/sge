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
package com.sun.grid.jgdi.configuration.reflect;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 *
 */
public class InvalidObjectException extends java.lang.Exception {
    
    private Object obj;
    private Map<String, String> propertyErrorMap;
    
    /**
     * Creates a new instance of <code>InvalidObjectException</code> without detail message.
     */
    public InvalidObjectException(Object obj) {
        this.obj = obj;
    }
    
    /**
     * Constructs an instance of <code>InvalidObjectException</code> with the specified detail message.
     * @param msg the detail message.
     */
    public InvalidObjectException(Object obj, String msg) {
        super(msg);
    }
    
    
    public void addPropertyError(String propertyName, String error) {
        if (propertyErrorMap == null) {
            propertyErrorMap = new HashMap<String, String>();
        }
        propertyErrorMap.put(propertyName, error);
    }
    
    public Set getInvalidProperties() {
        if (propertyErrorMap == null) {
            return Collections.EMPTY_SET;
        } else {
            return Collections.unmodifiableSet(propertyErrorMap.keySet());
        }
    }
    
    public String getPropertyError(String propertyName) {
        return propertyErrorMap.get(propertyName);
    }
    
    public String toString() {
        StringBuilder ret = new StringBuilder();
        ret.append(getMessage());
        if (propertyErrorMap != null) {
            boolean first = true;
            ret.append('[');
            for (Map.Entry<String, String> entry: propertyErrorMap.entrySet()) {
                ret.append(entry.getKey());
                ret.append(": ");
                ret.append(entry.getValue());
                if (first) {
                    first = false;
                } else {
                    ret.append(", ");
                }
            }
            ret.append(']');
        }
        return ret.toString();
    }
}
