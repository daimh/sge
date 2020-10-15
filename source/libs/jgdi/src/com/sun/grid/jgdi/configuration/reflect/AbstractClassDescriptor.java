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

import java.util.*;
import com.sun.grid.jgdi.configuration.GEObject;
import com.sun.grid.jgdi.configuration.Util;


/**
 * Abstract Base class for <code>ClassDescriptor</code> subclasses
 *
 */
public abstract class AbstractClassDescriptor implements ClassDescriptor {
    
    private ArrayList<PropertyDescriptor> propertyList = new ArrayList<PropertyDescriptor>();
    private Map<String, PropertyDescriptor> propertyMap = new HashMap<String, PropertyDescriptor>();
    private Class beanClass;
    private Class implClass;
    private String cullName;
    
    protected AbstractClassDescriptor(Class beanClass, String cullName) {
        this.beanClass = beanClass;
        this.cullName = cullName;
    }
    
    public List getProperties() {
        return Collections.unmodifiableList(propertyList);
    }
    
    protected void add(PropertyDescriptor property) {
        propertyList.add(property);
        propertyMap.put(property.getPropertyName(), property);
    }
    
    protected SimplePropertyDescriptor addSimple(String name, Class type, String cullType, int cullFieldName, boolean required, boolean readOnly, boolean configurable) {
        SimplePropertyDescriptor prop = new SimplePropertyDescriptor(beanClass, name, type, cullType, cullFieldName, required, readOnly, configurable);
        add(prop);
        return prop;
    }
    
    protected ListPropertyDescriptor addList(String name, Class type, String cullType, int cullFieldName, boolean browsable, boolean readOnly, boolean configurable) {
        ListPropertyDescriptor prop = new DefaultListPropertyDescriptor(beanClass, name, type, cullType, cullFieldName, browsable, readOnly, configurable);
        add(prop);
        return prop;
    }
    
    protected MapPropertyDescriptor addMap(String name, Class type, String cullType, Class keyType, int cullFieldName, int keyCullFieldName, int valueCullFieldName, Object defaultKey, boolean readOnly, boolean configurable) {
        MapPropertyDescriptor prop = new DefaultMapPropertyDescriptor(beanClass, name, type, cullType, keyType, cullFieldName, keyCullFieldName, valueCullFieldName, defaultKey, readOnly, configurable);
        add(prop);
        return prop;
    }
    
    protected MapListPropertyDescriptor addMapList(String name, Class type, String cullType, Class keyType, String cullListType, int cullFieldName, int keyCullFieldName, int valueCullFieldName, Object defaultKey, boolean readOnly, boolean configurable) {
        MapListPropertyDescriptor prop = new DefaultMapListPropertyDescriptor(beanClass, name, type, cullType, keyType, cullListType, cullFieldName, keyCullFieldName, valueCullFieldName, defaultKey, readOnly, configurable);
        add(prop);
        return prop;
    }
    
    public PropertyDescriptor getProperty(String name) {
        return propertyMap.get(name);
    }
    
    public String[] getPropertyNames() {
        String[] ret = new String[propertyMap.size()];
        propertyMap.keySet().toArray(ret);
        return ret;
    }
    
    public int getPropertyCount() {
        return propertyList.size();
    }
    
    public PropertyDescriptor getProperty(int index) {
        return propertyList.get(index);
    }
    
    public PropertyDescriptor getPropertyByCullFieldName(int cullFieldName) {
        for(PropertyDescriptor pd: propertyList) {
            if (pd.getCullFieldName() == cullFieldName) {
                return pd;
            }
        }
        return null;
    }
    
    public Class getBeanClass() {
        return beanClass;
    }
    
    public Object newInstance() {
        try {
            if (implClass != null) {
                return implClass.newInstance();
            } else {
                return beanClass.newInstance();
            }
        } catch (IllegalAccessException ile) {
            IllegalStateException ilse = new IllegalStateException("Constructor" + beanClass.getName() + " is not accessible");
            ilse.initCause(ile);
            throw ilse;
        } catch (InstantiationException iae) {
            IllegalStateException ilse = new IllegalStateException("Can\'t create instanceof of " + beanClass.getName());
            ilse.initCause(iae);
            throw ilse;
        }
    }
    
    public Object clone(Object bean) {
        
        ClassDescriptor classDescr = Util.getDescriptor(bean.getClass());
        Object ret = classDescr.newInstance();
        PropertyDescriptor propDescr = null;
        
        for (int i = 0; i < classDescr.getPropertyCount(); i++) {
            propDescr = classDescr.getProperty(i);
            if (propDescr.getPropertyName().equals("parent")) {
                SimplePropertyDescriptor parentPropDescr = (SimplePropertyDescriptor) propDescr;
                parentPropDescr.setValue(ret, parentPropDescr.getValue(bean));
            } else {
                propDescr.clone(bean, ret);
            }
        }
        
        return ret;
    }
    
    public void registryObjects(Object root) {
        
        ClassDescriptor cd = Util.getDescriptor(root.getClass());
        int count = cd.getPropertyCount();
        PropertyDescriptor pd = null;
        
        for (int i = 0; i < count; i++) {
            pd = cd.getProperty(i);
            if (!pd.getPropertyName().equals("parent") && GEObject.class.isAssignableFrom(pd.getPropertyType())) {
                GEObject value = null;
                if (pd instanceof SimplePropertyDescriptor) {
                    value = (GEObject) ((SimplePropertyDescriptor)pd).getValue(root);
                } else if (pd instanceof ListPropertyDescriptor) {
                    ListPropertyDescriptor lpd = (ListPropertyDescriptor) pd;
                    int valueCount = lpd.getCount(root);
                    for (int ii = 0; ii < valueCount; ii++) {
                        value = (GEObject) lpd.get(root, ii);
                    }
                }
            }
        }
    }
    
    public String getCullName() {
        return cullName;
    }
    
    public String toString(Object bean) {
        return bean.toString();
    }
    
    public Class getImplClass() {
        return implClass;
    }
    
    public void setImplClass(Class implClass) {
        this.implClass = implClass;
    }
}
