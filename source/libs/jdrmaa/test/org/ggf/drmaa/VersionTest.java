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
 * VersionTest.java
 * JUnit based test
 *
 * Created on November 14, 2004, 12:36 AM
 */

package org.ggf.drmaa;

import junit.framework.*;

/**
 *
 * @author dan.templeton@sun.com
 */
public class VersionTest extends TestCase {
    
    public VersionTest(java.lang.String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(VersionTest.class);
        return suite;
    }
    
    /** Test of getMajor|Minor method, of class org.ggf.drmaa.Version. */
    public void testGetMajorMinor() {
        System.out.println("testGetMajorMinor");
        
        Version v = new Version(3, 2);
        
        assertEquals(3, v.getMajor());
        assertEquals(2, v.getMinor());
    }
    
    /** Test of toString method, of class org.ggf.drmaa.Version. */
    public void testToString() {
        System.out.println("testToString");
        
        Version v = new Version(4, 2);
        
        assertEquals("4.2", v.toString());
    }
    
    /** Test of equals method, of class org.ggf.drmaa.Version. */
    public void testEquals() {
        System.out.println("testEquals");
        
        Version v1 = new Version(1, 1);
        Version v2 = new Version(1, 1);
        Version v3 = new Version(1, 2);
        Version v4 = new Version(2, 1);
        
        assertTrue(!v1.equals(null));
        assertTrue(!v1.equals("DRMAA"));
        assertEquals(v1, v2);
        assertTrue(!v1.equals(v3));
        assertTrue(!v1.equals(v4));
    }
    
    /** Test of hashCode method, of class org.ggf.drmaa.Version. */
    public void testHashCode() {
        System.out.println("testHashCode");
        
        Version v1 = new Version(3, 5);
        Version v2 = new Version(3, 5);
        
        assertEquals(v1.hashCode(), v2.hashCode());
    }
    
    /** Test of clone method, of class org.ggf.drmaa.Version. */
    public void testClone() {
        System.out.println("testClone");
        
        Version v1 = new Version(3, 5);
        Version v2 = (Version)v1.clone();
        
        assertEquals(v1, v2);
        assertNotSame(v1, v2);
    }
}
