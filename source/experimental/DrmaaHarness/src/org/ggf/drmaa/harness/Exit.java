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
 * Sleeper.java
 *
 * Created on April 24, 2005, 11:32 PM
 */

package org.ggf.drmaa.harness;

import org.ggf.drmaa.InvalidArgumentException;
import org.ggf.drmaa.JobTemplate;

/**
 *
 * @author dan.templeton@sun.com
 */
public class Exit extends DrmaaThread {
   private boolean done = false;
   private Thread myThread = null;
   
   /** Creates a new instance of Sleeper */
   public Exit (String id, JobTemplate jt, Object waitLock) {
      super (id, jt, waitLock);
   }

   public void kill () {
      super.kill ();
   }

   public void run () {
      String[] args = jt.getArgs ();
      int exit = 0;
      
      if ((args != null) && (args.length > 0)) {
         try {
            exit = Integer.parseInt (args[0]);
         }
         catch (NumberFormatException e) {
            /* Ignore and assume default. */
         }
      }
      
      this.startTimer ();
      this.setState (RUNNING);
      exitCode = exit;
      this.setState (DONE);
      this.stopTimer ();
      this.notifyWaits ();
   }
}
