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
import java.lang.reflect.*;

/**
 *
 */
public class DefaultListPropertyDescriptor extends ListPropertyDescriptor {
    
    private Method addMethod;
    private Method countMethod;
    private Method getMethod;
    private Method setMethod;
    private Method removeMethod;
    private Method removeValueMethod;
    private Method removeAllMethod;
    
    /** Creates a new instance of DefaultListPropertyDescriptor */
    public DefaultListPropertyDescriptor(Class beanClass, String propertyName,
            Class propertyType, String cullType,
            int cullFieldName, boolean browseable, boolean readOnly, boolean configurable) {
        super( beanClass, propertyName, propertyType, cullType, cullFieldName, browseable, readOnly, configurable);
        
        countMethod = findMethod( "get", "Count", null );
        getMethod = findMethod( "get", new Class[] { Integer.TYPE } );
//      if ( !readOnly ) {
        addMethod = findMethod( "add", new Class[] { propertyType } );
        setMethod = findMethod( "set", new Class[] { Integer.TYPE, propertyType } );
        removeMethod = findMethod( "remove", new Class[] { Integer.TYPE } );
        removeValueMethod = findMethod( "remove", new Class[] { propertyType } );
        removeAllMethod = findMethod( "removeAll", null );
//      }
    }
    
    
    public void add(Object bean, Object value) {
        invoke( addMethod, bean, new Object[] { value } );
    }
    
    public Object get(Object bean, int index) {
        return invoke( getMethod, bean, new Object[] { new Integer(index) } );
    }
    
    public int getCount(Object bean) {
        return ((Integer)invoke( countMethod, bean, null )).intValue();
    }
    
    public Object remove(Object bean, int index) {
        return invoke( removeMethod, bean, new Object[] { new Integer(index) } );
    }
    
    public boolean remove(Object bean, Object value) {
        return ((Boolean)invoke( removeValueMethod,  bean , new Object[] { value } )).booleanValue();
    }
    
    public void removeAll(Object bean) {
        invoke( removeAllMethod, bean, null );
    }
    
    public void set(Object bean, int index, Object value) {
        invoke( setMethod, bean, new Object[] { new Integer(index), value } );
    }
    
}
