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

import java.lang.reflect.*;

/**
 *
 */
public abstract class PropertyDescriptor {
    
    private Class  beanClass;
    private String propertyName;
    Class  propertyType;
    private int    cullFieldName;
    private String cullType;
    
    private boolean readOnly;
    private int hashCode;
    private boolean browsable;
    private boolean configurable;
    private Method isSetMethod;
    
    /** Some cull type are mapped to a primitive java class
     *  e.g HR_Type -> String
     *  For all these type the <code>hasCullWrapper</code> flag
     *  is set to <code>true</code>.
     */
    private boolean hasCullWrapper;
    
    /**
     *  If a cull type is mapped to a primitive java class the
     *  <code>cullContentField</code> specifies the cull attribute
     *  which contains the content.
     */
    private int     cullContentField;
    
    
    
    /** Creates a new instance of PropertyDescriptor */
    protected PropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType,
            int cullFieldName, boolean readOnly,
            boolean configurable) {
        this.beanClass = beanClass;
        this.propertyName = propertyName;
        this.propertyType = propertyType;
        this.cullType = cullType;
        this.cullFieldName = cullFieldName;
        this.readOnly = readOnly;
        this.configurable = configurable;
        this.hashCode = beanClass.hashCode() * 31 + propertyName.hashCode();
        
        try {
            this.isSetMethod = findMethod("isSet", null);
        } catch (IllegalArgumentException e) {
            // ignore error if isSet* is not present for bean class -> webqmon
        }
    }
    
    public boolean equals(Object obj) {
        return obj instanceof PropertyDescriptor &&
                beanClass.equals(((PropertyDescriptor)obj).beanClass) &&
                propertyName.equals(((PropertyDescriptor)obj).propertyName);
    }
    
    
    public int hashCode() {
        return hashCode;
    }
    
    protected Method findMethod(String prefix, String suffix, Class [] argTypes) {
        
        StringBuilder buf = new StringBuilder();
        if (prefix != null) {
            buf.append(prefix);
        }
        buf.append(Character.toUpperCase(propertyName.charAt(0)));
        buf.append(propertyName.substring(1));
        if (suffix != null) {
            buf.append(suffix);
        }
        String methodName = buf.toString();
        
        try {
            return beanClass.getMethod(methodName, argTypes);
        } catch (NoSuchMethodException nse) {
            StringBuilder buffer = new StringBuilder();
            if (argTypes != null) {
                for (int i=0; i<argTypes.length; i++) {
                    if (i>0) {
                        buffer.append(", ");
                    }
                    buffer.append(argTypes[i].getName());
                }
            } else {
                buffer.append("");
            }
            IllegalArgumentException ilse = new IllegalArgumentException(
                    "Method " + methodName + "(" + buffer + ")"
                    + " not found in class " + beanClass );
            ilse.initCause(nse);
            throw ilse;
        }
    }
    
    protected Method findMethod(String prefix, Class [] argTypes) {
        return findMethod(prefix, null, argTypes);
    }
    
    protected Object invoke(Method method, Object bean, Object[] args) {
        try {
            return method.invoke(bean, args);
        } catch (IllegalAccessException iae) {
            IllegalStateException ilse = new IllegalStateException( method.getName()  + " of "
                    + getBeanClass().getName() + "." + getPropertyName()
                    + " is not accessible");
            ilse.initCause( iae );
            throw ilse;
        } catch( InvocationTargetException ite ) {
            IllegalStateException ilse = new IllegalStateException(
                    "error in method " + method.getName() + " of "
                    + getBeanClass().getName() + "." + getPropertyName() );
            ilse.initCause( ite );
            throw ilse;
        }
    }
    
    
    /**
     * Getter for property propertyType.
     * @return Value of property propertyType.
     */
    public java.lang.Class getPropertyType() {
        return propertyType;
    }
    
    public String getCullType() {
        return cullType;
    }
    
    public String getJNIPropertyType() {
        
        String name = propertyType.getName();
        if( !propertyType.isPrimitive() ) {
            name = "L" + name.replace('.','/') + ";";
        }
        return name;
    }
    
    /**
     * Getter for property propertyName.
     * @return Value of property propertyName.
     */
    public java.lang.String getPropertyName() {
        return propertyName;
    }
    
    public int getCullFieldName() {
        return this.cullFieldName;
    }
    /**
     * Getter for property beanClass.
     * @return Value of property beanClass.
     */
    public java.lang.Class getBeanClass() {
        return beanClass;
    }
    
    public abstract void clone( Object srcBean, Object targetBean );
    
    public boolean isReadOnly() {
        return readOnly;
    }
    
    public void setReadOnly(boolean readOnly) {
        this.readOnly = readOnly;
    }
    
    public boolean isConfigurable() {
        return configurable;
    }
    
    public void setConfigurable(boolean configurable) {
        this.configurable = configurable;
    }
    
    
    /**
     * Getter for property browsable.
     * @return Value of property browsable.
     */
    public boolean isBrowsable() {
        return browsable;
    }
    
    /**
     * Setter for property browsable.
     * @param browsable New value of property browsable.
     */
    public void setBrowsable(boolean browsable) {
        this.browsable = browsable;
    }
    
    public boolean hasCullWrapper() {
        return hasCullWrapper;
    }
    
    public void setHasCullWrapper(boolean hasCullWrapper) {
        this.hasCullWrapper = hasCullWrapper;
    }
    
    public int getCullContentField() {
        return cullContentField;
    }
    
    public void setCullContentField(int cullContentField) {
        this.cullContentField = cullContentField;
    }
    
    public boolean isSet(Object bean) {
        return ((Boolean)invoke(isSetMethod,  bean , null)).booleanValue();
    }
    
}
