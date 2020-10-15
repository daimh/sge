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
package com.sun.grid.jgdi.rmi;

import com.sun.grid.jgdi.JGDIException;
import java.rmi.RemoteException;
import java.util.logging.Logger;

/**
 *
 */
public class JGDIRemoteFactoryImpl implements JGDIRemoteFactory {
    
    private Logger logger = Logger.getLogger("com.sun.grid.jgdi.rmi");
    
    private String url;
    /** Creates a new instance of JGDIRemoteFactoryImpl */
    public JGDIRemoteFactoryImpl(String url) {
        this.url = url;
    }
    
    public JGDIRemote newInstance() throws java.rmi.RemoteException {
        try {
            return new JGDIRemoteImpl(url);
        } catch( JGDIException jgdie ) {
            logger.throwing("JGDIRemoteFactoryImpl", "newInstance", jgdie );
            throw new RemoteException("Can not create jgdi remote instance", jgdie);
        }
    }
    
}
