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
import java.util.List;
import java.util.Set;

/**
 *
 */
public abstract class MapListPropertyDescriptor extends PropertyDescriptor {
    
    private int keyCullFieldName;
    private int valueCullFieldName;
    private Class keyType;
    private String cullListType;
    private Object defaultKey;
    
    protected MapListPropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType, Class keyType, String cullListType, int cullFieldName,
            int keyCullFieldName, int valueCullFieldName,
            Object defaultKey, boolean readOnly, boolean configurable ) {
        super(beanClass, propertyName, propertyType, cullType, cullFieldName, readOnly, configurable);
        this.keyCullFieldName = keyCullFieldName;
        this.valueCullFieldName = valueCullFieldName;
        this.keyType = keyType;
        this.cullListType = cullListType;
        this.defaultKey = defaultKey;
    }
    
    public abstract Object get(Object bean, Object key, int index);
    public abstract void add(Object bean, Object key, Object value);
    public abstract void addEmpty(Object bean, Object key);
    public abstract void set(Object bean, Object key, int index, Object value);
    
    public abstract Object remove(Object bean, Object key, int index);
    public abstract boolean remove(Object bean, Object key, Object value);
    
    public abstract void removeAll(Object bean);
    public abstract void removeAll(Object bean, Object key);
    
    public abstract Set getKeys(Object bean);
    public abstract List getList(Object bean, Object key);
    public abstract int getCount(Object bean, Object key);
    
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
    
    public String getCullListType() {
        return cullListType;
    }
    
    public String toString(Object obj, Object key, int index) {
        Object value = get(obj,key, index);
        if(value == null) {
            return "null";
        } else {
            return value.toString();
        }
    }
    
}
