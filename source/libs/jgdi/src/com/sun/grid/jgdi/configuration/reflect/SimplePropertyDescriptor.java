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

import com.sun.grid.jgdi.configuration.Util;
import com.sun.grid.jgdi.configuration.reflect.*;
import java.lang.reflect.*;
import com.sun.grid.jgdi.configuration.*;
/**
 *
 */
public class SimplePropertyDescriptor extends PropertyDescriptor {
    
    private Method getter;
    private Method setter;
    boolean required;
    
    /**
     * Creates a new instance of SimplePropertyDescriptor
     * @param beanClass      the bean class
     * @param propertyName   name of the property
     * @param propertyType   class of the property
     * @param cullType       cull type of the property
     * @param required       is this property required
     * @param readOnly       is this property readonly
     * @param configurable   is this property configurable
     */
    public SimplePropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType,
            int cullFieldName, boolean required, boolean readOnly, boolean configurable) {
        super(beanClass, propertyName, propertyType, cullType, cullFieldName, readOnly, configurable);
        this.required = required;
        
        String methodName = "get";
        if( propertyType.equals( Boolean.TYPE )
        || propertyType.equals( Boolean.class ) ) {
            methodName = "is";
        }
        getter = findMethod( methodName, null );
        if( !readOnly ) {
            setter = findMethod( "set", new Class[] { propertyType } );
        }
    }
    
    /**
     * Getter for property required.
     * @return Value of property required.
     */
    public boolean isRequired() {
        return required;
    }
    
    /**
     * set the value of this property in a bean
     * @param bean   the bean
     * @param value  the value
     */
    public void setValue(Object bean, Object value) {
        if (isReadOnly()) {
            throw new IllegalStateException("Can in invoke setValue on readonly property "
                    + getPropertyName() + " of class " + getBeanClass().getName() );
        }
        try {
            setter.invoke( bean, new Object[] { value } );
        } catch( IllegalAccessException ila ) {
            IllegalStateException ilse = new IllegalStateException("setter of "
                    + getBeanClass().getName() + "." + getPropertyName()
                    + " is not accessible");
            ilse.initCause( ila );
            throw ilse;
        } catch( InvocationTargetException iva ) {
            IllegalStateException ilse = new IllegalStateException(
                    "error in setter of "
                    + getBeanClass().getName() + "." + getPropertyName() );
            ilse.initCause( iva );
            throw ilse;
        }
    }
    
    /**
     * get the value of this property from a bean
     * @param bean   the bean
     * @return the value
     */
    public Object getValue( Object bean ) {
        try {
            return getter.invoke( bean, (java.lang.Object[])null );
        } catch( IllegalAccessException ila ) {
            IllegalStateException ilse = new IllegalStateException("getter of "
                    + getBeanClass().getName() + "." + getPropertyName()
                    + " is not accessible");
            ilse.initCause( ila );
            throw ilse;
        } catch( InvocationTargetException iva ) {
            IllegalStateException ilse = new IllegalStateException(
                    "error in getter of "
                    + getBeanClass().getName() + "." + getPropertyName() );
            ilse.initCause( iva );
            throw ilse;
        }
    }
    
    /**
     * A simple property is never browsable
     * @return false
     */
    public boolean isBrowsable() {
        return false;
    }
    
    /**
     * clone this property of <code>srcBean</code> into
     * <code>targetBean</code>.
     * @param srcBean      the src bean
     * @param targetBean   the target bean
     */
    public void clone(Object srcBean, Object targetBean) {
        Class propertyType = getPropertyType();
        Object srcValue = getValue( srcBean );
        
        if( propertyType.isPrimitive() ||
                propertyType.equals( String.class ) ) {
            setValue( targetBean, srcValue );
        } else {
            
            ClassDescriptor cd = Util.getDescriptor( propertyType );
            if( cd == null ) {
                throw new IllegalArgumentException( "Can't clone property of type " + propertyType.getName() );
            }
            setValue( targetBean, cd.clone( srcValue ) );
        }
    }
    
    public String toString(Object bean) {
        Object value = getValue(bean);
        if(value == null) {
            return "null";
        } else {
            return value.toString();
        }
    }
}
