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
package com.sun.grid.drmaa;

import org.ggf.drmaa.Session;
import org.ggf.drmaa.SessionFactory;

/**
 * This class is used to create a SessionImpl instance.  In order to use the
 * Grid Engine binding, the $SGE_ROOT environment variable must be set, the
 * $SGE_ROOT/lib/drmaa.jar file must in included in the CLASSPATH environment
 * variable, and the $SGE_ROOT/lib/$ARCH directory must be included in the
 * library path, e.g. LD_LIBRARY_PATH.
 * @see org.ggf.drmaa.SessionFactory
 * @author dan.templeton@sun.com
 * @since 0.5
 * @version 1.0
 */
public class SessionFactoryImpl extends SessionFactory {
    private Session thisSession = null;
    
    /**
     * Creates a new instance of SessionFactoryImpl.
     */
    public SessionFactoryImpl() {
    }
    
    public Session getSession() {
        synchronized (this) {
            if (thisSession == null) {
                thisSession = new SessionImpl();
            }
        }
        
        return thisSession;
    }
}
