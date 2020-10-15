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

/**
 *
 */
public class DefaultMapPropertyDescriptor extends MapPropertyDescriptor {
    
    private Method getMethod;
    private Method countMethod;
    private Method putMethod;
    private Method removeMethod;
    private Method removeAllMethod;
    private Method keysMethod;
    
    /** Creates a new instance of DefaultMapPropertyDescriptor */
    protected DefaultMapPropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType, Class keyType, int cullFieldName,
            int keyCullFieldName, int valueCullFieldName,
            Object defaultKey, boolean readOnly, boolean configurable ) {
        super(beanClass, propertyName, propertyType, cullType, keyType,
                cullFieldName, keyCullFieldName, valueCullFieldName, defaultKey, readOnly, configurable);
        countMethod = findMethod( "get", "Count", null );
        getMethod = findMethod( "get", new Class[] { keyType } );
        keysMethod = findMethod("get", "Keys", null);
//      if (!readOnly) {
        putMethod = findMethod( "put", new Class[] { keyType, propertyType } );
        removeMethod = findMethod( "remove", new Class[] { keyType } );
        removeAllMethod = findMethod( "removeAll", null );
//      }
    }
    
    public void clone(Object srcBean, Object targetBean) {
        throw new IllegalStateException("Yet not implemented");
    }
    
    public Object get(Object bean, Object key) {
        return invoke( getMethod, bean, new Object[] { key } );
    }
    
    public int getCount(Object bean) {
        return ((Integer)invoke( countMethod, bean, null )).intValue();
    }
    
    public java.util.Set<String> getKeys(Object bean) {
        return (java.util.Set<String>)invoke(keysMethod, bean, null);
    }
    
    public boolean isBrowsable() {
        return false;
    }
    
    public Object remove(Object bean, Object key) {
        return invoke(removeMethod, bean, new Object[] { key } );
    }
    
    public void removeAll(Object bean) {
        invoke(removeAllMethod, bean, null );
    }
    
    public void put(Object bean, Object key, Object value) {
        invoke(putMethod, bean, new Object[] { key, value } );
    }
}
