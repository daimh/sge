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
 * Executor.java
 *
 * Created on March 29, 2005, 9:38 PM
 */

package org.ggf.drmaa.harness;

import java.util.HashMap;
import java.util.Map;
import org.ggf.drmaa.JobTemplate;

/**
 *
 * @author dan.templeton@sun.com
 */
abstract class DrmaaThread implements Runnable {
   static final int PENDING = 0;
   static final int RUNNING = 1;
   static final int DONE = 2;
   static final int FAILED = 3;
   static final int SUSPENDED = 4;
   static final int HOLD = 5;
   static final int ABORTED = 6;
   
   protected JobTemplate jt = null;
   protected String id = null;
   protected Object waitLock = null;
   protected int exitCode = 0;
   private int state = PENDING;
   private long timerStart = 0L;
   private Map map = null;
   
   /** Creates a new instance of Executor */
   DrmaaThread (String id, JobTemplate jt, Object waitLock) {
      this.jt = jt;
      this.id = id;
      this.waitLock = waitLock;
   }

   protected void notifyWaits () {
      synchronized (waitLock) {
         waitLock.notifyAll ();
      }
   }
   
   protected synchronized int getState () {
      return state;
   }
   
   protected synchronized void setState (int state) {
      if ((this.state != DONE) && (this.state != FAILED) &&
          (this.state != ABORTED)) {
         this.state = state;
      }
   }
   
   protected int waitForJob () {
      int state = this.getState ();
      
      while ((state != DONE) && (state != FAILED) && (state != ABORTED)) {
         synchronized (waitLock) {
            try {
               waitLock.wait ();
            }
            catch (InterruptedException e) {
               /* Ignore and go around again. */
            }
         }
         
         state = this.getState ();
      }

      return state;
   }
   
   protected void kill () {
      this.setState (ABORTED);
   }
   
   protected int getExitCode () {
      return exitCode;
   }
   
   protected String getId () {
      return id;
   }
   
   protected void startTimer () {
      timerStart = System.currentTimeMillis ();
   }
   
   protected void stopTimer () {
      map = new HashMap ();
      map.put ("h_cpu", Long.toString (System.currentTimeMillis () - timerStart));
   }
   
   protected Map getMap () {
      return map;
   }
}
