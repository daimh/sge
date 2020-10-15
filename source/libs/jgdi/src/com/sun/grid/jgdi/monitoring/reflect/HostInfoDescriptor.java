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
package com.sun.grid.jgdi.monitoring.reflect;

import com.sun.grid.jgdi.configuration.reflect.AbstractClassDescriptor;
import com.sun.grid.jgdi.monitoring.HostInfoImpl;

/**
 *
 */
public class HostInfoDescriptor extends AbstractClassDescriptor {
    
    /** Creates a new instance of HostInfoDescriptor */
    public HostInfoDescriptor() {
        super(HostInfoImpl.class, null);
        
        addSimple("hostname", String.class, null,-1, false, true, false);
        addSimple("arch", String.class, null, -1, false, true, false);
        addSimple("loadAvg", String.class, null, -1, false, true, false);
        addSimple("memTotal", String.class, null, -1, false, true, false);
        addSimple("memUsed", String.class, null, -1, false, true, false);
        addSimple("swapTotal", String.class, null, -1, false, true, false);
        addSimple("swapUsed", String.class, null, -1, false, true, false);
        
        addMap("hostValue", Object.class, null, String.class, -1, -1, -1, null, true, false);
    }
    
    public void validate(Object bean) throws com.sun.grid.jgdi.configuration.reflect.InvalidObjectException {
    }
    
}
