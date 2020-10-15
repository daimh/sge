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
 * FileTransferModeTest.java
 * JUnit based test
 *
 * Created on November 14, 2004, 12:23 AM
 */

package org.ggf.drmaa;

import junit.framework.*;

/**
 *
 * @author dan.templeton@sun.com
 */
public class FileTransferModeTest extends TestCase {
    
    public FileTransferModeTest(java.lang.String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(FileTransferModeTest.class);
        return suite;
    }
    
    protected void setUp() {
    }
    
    protected void teadDown() {
    }
    
    /** Test of g|setErrorStream method, of class org.ggf.drmaa.FileTransferMode. */
    public void testErrorStream() {
        System.out.println("testErrorStream");
        
        FileTransferMode mode = new FileTransferMode();
        
        mode.setErrorStream(true);
        assertTrue(mode.getErrorStream());
        mode.setErrorStream(false);
        assertFalse(mode.getErrorStream());
    }
    
    /** Test of g|setInputStream method, of class org.ggf.drmaa.FileTransferMode. */
    public void testInputStream() {
        System.out.println("testInputStream");
        
        FileTransferMode mode = new FileTransferMode();
        
        mode.setInputStream(true);
        assertTrue(mode.getInputStream());
        mode.setInputStream(false);
        assertFalse(mode.getInputStream());
    }
    
    /** Test of g|setOutputStream method, of class org.ggf.drmaa.FileTransferMode. */
    public void testOutputStream() {
        System.out.println("testOutputStream");
        
        FileTransferMode mode = new FileTransferMode();
        
        mode.setOutputStream(true);
        assertTrue(mode.getOutputStream());
        mode.setOutputStream(false);
        assertFalse(mode.getOutputStream());
    }
    
    /** Test of equals method, of class org.ggf.drmaa.FileTransferMode. */
    public void testEquals() {
        System.out.println("testEquals");
        
        FileTransferMode mode1 = new FileTransferMode(false, false, false);
        FileTransferMode mode2 = new FileTransferMode(false, false, false);
        FileTransferMode mode3 = new FileTransferMode(true, false, false);
        FileTransferMode mode4 = new FileTransferMode(false, true, true);
        
        assertTrue(!mode1.equals(null));
        assertTrue(!mode1.equals("DRMAA"));
        assertEquals(mode1, mode2);
        assertTrue(!mode1.equals(mode3));
        assertTrue(!mode1.equals(mode4));
    }
    
    /** Test of clone method, of class org.ggf.drmaa.FileTransferMode. */
    public void testClone() {
        System.out.println("testClone");
        
        FileTransferMode mode1 = new FileTransferMode(false, true, false);
        FileTransferMode mode2 = (FileTransferMode)mode1.clone();
        
        assertEquals(mode1, mode2);
        
        mode1.setInputStream(true);
        mode2 = (FileTransferMode)mode1.clone();
        
        assertEquals(mode1, mode2);
    }
    
    /** Test of constructor, of class org.ggf.drmaa.FileTransferMode. */
    public void testConstructor() {
        System.out.println("testConstructor");
        
        FileTransferMode mode = new FileTransferMode(true, false, true);
        
        assertTrue(mode.getInputStream());
        assertTrue(!mode.getOutputStream());
        assertTrue(mode.getErrorStream());
        
        mode = new FileTransferMode(false, true, false);
        
        assertTrue(!mode.getInputStream());
        assertTrue(mode.getOutputStream());
        assertTrue(!mode.getErrorStream());
    }
    
    /** Test of hashCode method, of class org.ggf.drmaa.FileTransferMode. */
    public void testHashCode() {
        System.out.println("testHashCode");
        
        FileTransferMode mode1 = new FileTransferMode(true, false, true);
        FileTransferMode mode2 = new FileTransferMode(true, false, true);
        
        assertEquals(mode1.hashCode(), mode2.hashCode());
    }
    
    /** Test of toString method, of class org.ggf.drmaa.FileTransferMode. */
    public void testToString() {
        System.out.println("testToString");
        
        assertEquals("", new FileTransferMode(false, false, false).toString());
        assertEquals("input", new FileTransferMode(true, false, false).toString());
        assertEquals("input, output", new FileTransferMode(true, true, false).toString());
        assertEquals("input, error", new FileTransferMode(true, false, true).toString());
        assertEquals("input, output, error", new FileTransferMode(true, true, true).toString());
        assertEquals("output", new FileTransferMode(false, true, false).toString());
        assertEquals("output, error", new FileTransferMode(false, true, true).toString());
        assertEquals("error", new FileTransferMode(false, false, true).toString());
    }
}
