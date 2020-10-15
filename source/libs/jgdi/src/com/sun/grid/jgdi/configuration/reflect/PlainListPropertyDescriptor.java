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

/**
 *
 */
public class PlainListPropertyDescriptor extends ListPropertyDescriptor {
    
    private Method getListMethod;
    /** Creates a new instance of PlainListPropertyDescriptor */
    public PlainListPropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType,
            int cullFieldName, boolean browseable, boolean readOnly, boolean configurable) {
        super( beanClass, propertyName, propertyType, cullType, cullFieldName, browseable, readOnly, configurable );
        
        getListMethod = findMethod("get", "List", null);
    }
    
    public void clone(Object srcBean, Object targetBean) {
        throw new IllegalStateException("Yet not implemented");
    }
    
    private List getList(Object bean) {
        return (List)invoke(getListMethod, bean, null);
    }
    
    public void add(Object bean, Object value) {
        getList(bean).add(value);
    }
    
    public Object get(Object bean, int index) {
        return getList(bean).get(index);
    }
    
    public int getCount(Object bean) {
        return getList(bean).size();
    }
    
    public Object remove(Object bean, int index) {
        return getList(bean).remove(index);
    }
    
    public boolean remove(Object bean, Object value) {
        return getList(bean).remove(value);
    }
    
    public void removeAll(Object bean) {
        getList(bean).clear();
    }
    
    public void set(Object bean, int index, Object value) {
        getList(bean).set(index,value);
    }
    
}
