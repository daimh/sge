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

import com.sun.grid.jgdi.configuration.reflect.*;
import java.util.Set;

/**
 *
 */
public abstract class MapPropertyDescriptor extends PropertyDescriptor {
    
    private int keyCullFieldName;
    private int valueCullFieldName;
    private Class keyType;
    private Object defaultKey;
    
    protected MapPropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType, Class keyType, int cullFieldName,
            int keyCullFieldName, int valueCullFieldName,
            Object defaultKey, boolean readOnly, boolean configurable ) {
        super(beanClass, propertyName, propertyType, cullType, cullFieldName, readOnly, configurable);
        this.keyCullFieldName = keyCullFieldName;
        this.valueCullFieldName = valueCullFieldName;
        this.keyType = keyType;
        this.defaultKey = defaultKey;
    }
    
    public abstract Object get(Object bean, Object key);
    public abstract void put(Object bean, Object key, Object value);
    public abstract Object remove(Object bean, Object key);
    public abstract void removeAll(Object bean);
    public abstract Set getKeys(Object bean);
    public abstract int getCount(Object bean);
    
    public Object getDefaultKey() {
        return defaultKey;
    }
    
    public int getKeyCullFieldName() {
        return keyCullFieldName;
    }
    
    public int getValueCullFieldName() {
        return valueCullFieldName;
    }
    
    public Class getKeyType() {
        return keyType;
    }
    
    public String toString(Object obj, Object key) {
        Object value = get(obj,key);
        if(value == null) {
            return "null";
        } else {
            return value.toString();
        }
    }
    
}
