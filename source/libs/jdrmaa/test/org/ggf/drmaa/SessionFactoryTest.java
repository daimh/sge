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
/*
 * SessionFactoryTest.java
 * JUnit based test
 *
 * Created on November 14, 2004, 12:50 AM
 */

package org.ggf.drmaa;

import java.io.*;
import java.util.Properties;
import junit.framework.*;

/**
 *
 * @author dan.templeton@sun.com
 */
public class SessionFactoryTest extends TestCase {
    public SessionFactoryTest(java.lang.String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(SessionFactoryTest.class);
        return suite;
    }
    
    /** Test of getFactory method, of class org.ggf.drmaa.SessionFactory. */
    public void testGetFactory() {
        System.out.println("testGetFactory");
        
        SessionFactory factory = SessionFactory.getFactory();
        assertTrue(factory instanceof com.sun.grid.drmaa.SessionFactoryImpl);
    }
}
