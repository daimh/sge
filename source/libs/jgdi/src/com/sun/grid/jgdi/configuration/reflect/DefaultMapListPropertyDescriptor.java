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
import java.lang.reflect.Method;
import java.util.List;
import java.util.Set;

/**
 *
 */
public class DefaultMapListPropertyDescriptor extends MapListPropertyDescriptor {
    
    private Method addMethod;
    private Method addEmptyMethod;
    private Method getMethod;
    private Method getCountMethod;
    private Method getValuesMethod;
    private Method removeIndexMethod;
    private Method removeMethod;
    private Method removeAllAllMethod;
    private Method removeAllMethod;
    private Method setMethod;
    private Method keysMethod;
    
    /** Creates a new instance of DefaultMapListPropertyDescriptor */
    public DefaultMapListPropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType, Class keyType, String cullListType,
            int cullFieldName, int keyCullFieldName, int valueCullFieldName,
            Object defaultKey, boolean readOnly, boolean configurable ) {
        super(beanClass, propertyName, propertyType, cullType, keyType, cullListType,
                cullFieldName, keyCullFieldName, valueCullFieldName, defaultKey, readOnly, configurable);
        
        getMethod = findMethod("get", new Class [] { keyType, Integer.TYPE });
        getCountMethod = findMethod("get", "Count", new Class[] { keyType } );
        getValuesMethod = findMethod("get", "List", new Class[] { keyType });
        keysMethod = findMethod("get", "Keys", null);
        
        if (!readOnly) {
            addMethod = findMethod("add", new Class [] { keyType, propertyType } );
            addEmptyMethod = findMethod("addEmpty", new Class [] { keyType } );
            setMethod = findMethod("set", new Class [] { keyType, Integer.TYPE, propertyType });
            removeIndexMethod = findMethod("remove", "At", new Class [] { keyType, Integer.TYPE });
            removeMethod = findMethod("remove", new Class [] { keyType, propertyType });
            removeAllAllMethod = findMethod("removeAll", null );
            removeAllMethod = findMethod("removeAll", new Class [] { keyType } );
        }
    }
    
    public void add(Object bean, Object key, Object value) {
        invoke(addMethod, bean, new Object [] { key, value } );
    }
    
    public void addEmpty(Object bean, Object key) {
        invoke(addEmptyMethod, bean, new Object [] { key } );
    }
    
    public void clone(Object srcBean, Object targetBean) {
        throw new IllegalStateException("Yet not implemented");
    }
    
    public Object get(Object bean, Object key, int index) {
        return invoke(getMethod, bean, new Object [] { key, new Integer(index) } );
    }
    
    public int getCount(Object bean, Object key) {
        Integer ret = (Integer)invoke(getCountMethod, bean, new Object [] { key } );
        return ret.intValue();
    }
    
    public List getList(Object bean, Object key) {
        return (List)invoke(getValuesMethod, bean, new Object[] { key } );
    }
    
    
    public Set<String> getKeys(Object bean) {
        return (Set<String>)invoke(keysMethod, bean, null);
    }
    
    
    public boolean isBrowsable() {
        return false;
    }
    
    public Object remove(Object bean, Object key, int index) {
        return invoke(removeIndexMethod, bean, new Object[] { key, new Integer(index) });
    }
    
    public boolean remove(Object bean, Object key, Object value) {
        Boolean ret = (Boolean)invoke(removeIndexMethod, bean, new Object[] { key, value } );
        return ret.booleanValue();
    }
    
    public void removeAll(Object bean) {
        invoke(removeAllAllMethod, bean , null );
    }
    
    public void removeAll(Object bean, Object key) {
        invoke(removeAllMethod, bean , new Object [] { key } );
    }
    
    public void set(Object bean, Object key, int index, Object value) {
        invoke(setMethod, bean, new Object[] { key, new Integer(index), value } );
    }
    
}
