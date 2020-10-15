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
 * JobInfoImplTest.java
 * JUnit based test
 *
 * Created on November 15, 2004, 10:41 AM
 */

package com.sun.grid.drmaa;

import junit.framework.*;
import org.ggf.drmaa.*;

/**
 *
 * @author dan.templeton@sun.com
 */
public class JobInfoImplTest extends TestCase {
   private static final String SIG = "SIGTERM";
   private JobInfo ji_ok = null;
   private JobInfo ji_sig = null;
   private JobInfo ji_core = null;
   private JobInfo ji_abort = null;
   
   public JobInfoImplTest (java.lang.String testName) {
      super (testName);
   }
   
   public static Test suite () {
      TestSuite suite = new TestSuite (JobInfoImplTest.class);
      return suite;
   }
   
   public void setUp () {
      ji_ok = new JobInfoImpl ("12345", 0xf1,
                               new String[] {"cpu_time=1000","mem_usage=1024"},
                               null);
      ji_sig = new JobInfoImpl ("12346", 0x02,
                                new String[] {"cpu_time=100","mem_usage=1024"},
                                SIG);
      ji_core = new JobInfoImpl ("12348", 0x04,
                                 new String[] {"cpu_time=100","mem_usage=2048"},
                                 null);
      ji_abort = new JobInfoImpl ("12347", 0x08, new String[] {}, null);
   }
   
   public void tearDown () {
      ji_ok = null;
      ji_sig = null;
      ji_core = null;
      ji_abort = null;
   }
   
   /** Test of getExitStatus method, of class com.sun.grid.drmaa.JobInfoImpl. */
   public void testGetExitStatus () throws DrmaaException {
      System.out.println ("testGetExitStatus");

      assertEquals (15, ji_ok.getExitStatus ());
   }
   
   /** Test of getTerminatingSignal method, of class com.sun.grid.drmaa.JobInfoImpl. */
   public void testGetTerminatingSignal () throws DrmaaException {
      System.out.println ("testGetTerminatingSignal");

      assertEquals (SIG, ji_sig.getTerminatingSignal ());
   }
   
   /** Test of hasCoreDump method, of class com.sun.grid.drmaa.JobInfoImpl. */
   public void testHasCoreDump () throws DrmaaException {
      System.out.println ("testHasCoreDump");

      assertFalse (ji_ok.hasCoreDump ());
      assertFalse (ji_sig.hasCoreDump ());
      assertTrue (ji_core.hasCoreDump ());
      assertFalse (ji_abort.hasCoreDump ());      
   }
   
   /** Test of hasExited method, of class com.sun.grid.drmaa.JobInfoImpl. */
   public void testHasExited () throws DrmaaException {
      System.out.println ("testHasExited");
      
      assertTrue (ji_ok.hasExited ());
      assertFalse (ji_sig.hasExited ());
      assertFalse (ji_core.hasExited ());
      assertFalse (ji_abort.hasExited ());
   }
   
   /** Test of hasSignaled method, of class com.sun.grid.drmaa.JobInfoImpl. */
   public void testHasSignaled () throws DrmaaException {
      System.out.println ("testHasSignaled");

      assertFalse (ji_ok.hasSignaled ());
      assertTrue (ji_sig.hasSignaled ());
      assertFalse (ji_core.hasSignaled ());
      assertFalse (ji_abort.hasSignaled ());
   }
   
   /** Test of wasAborted method, of class com.sun.grid.drmaa.JobInfoImpl. */
   public void testWasAborted () throws DrmaaException {
      System.out.println ("testWasAborted");
      
      assertFalse (ji_ok.wasAborted ());
      assertFalse (ji_sig.wasAborted ());
      assertFalse (ji_core.wasAborted ());
      assertTrue (ji_abort.wasAborted ());
   }
}
