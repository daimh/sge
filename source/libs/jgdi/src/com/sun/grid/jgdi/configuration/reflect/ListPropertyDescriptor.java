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
import com.sun.grid.jgdi.configuration.*;
import com.sun.grid.jgdi.configuration.reflect.*;

/**
 *
 */
public abstract class ListPropertyDescriptor extends PropertyDescriptor {
    
    boolean browsable;
    
    /** Creates a new instance of ListPropertyDescriptor */
    public ListPropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType,
            int cullFieldName, boolean browseable, boolean readOnly, boolean configurable ) {
        super( beanClass, propertyName, propertyType, cullType, cullFieldName, readOnly, configurable);
        setBrowsable(browseable);
    }
    
    
    
    public abstract int getCount( Object bean );
    public abstract Object get( Object bean, int index );
    public abstract void add( Object bean, Object value );
    public abstract void set( Object bean, int index, Object value );
    public abstract Object remove( Object bean, int index );
    public abstract boolean remove( Object bean, Object value );
    public abstract void removeAll( Object boean );
    
    public void clone(Object srcBean, Object targetBean) {
        
        int count = getCount( srcBean );
        Object srcValue = null;
        
        for( int i = 0; i < count; i++ ) {
            srcValue = get( srcBean, i );
            if( propertyType.isPrimitive() ||
                    propertyType.equals( String.class ) ) {
                add( targetBean, srcValue );
            } else {
                
                ClassDescriptor cd = Util.getDescriptor( propertyType );
                if( cd == null ) {
                    throw new IllegalArgumentException( "Can't clone property of type " + propertyType.getName() );
                }
                add( targetBean, cd.clone( srcValue ) );
            }
            
        }
    }
    
    public String toString(Object obj, int index) {
        Object value = get(obj,index);
        if(value == null) {
            return "null";
        } else {
            return value.toString();
        }
    }
    
}
