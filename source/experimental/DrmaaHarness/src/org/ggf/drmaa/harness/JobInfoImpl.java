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
 * JobInfoImpl.java
 *
 * Created on April 24, 2005, 11:53 PM
 */

package org.ggf.drmaa.harness;

import java.util.Map;
import org.ggf.drmaa.JobInfo;

/**
 *
 * @author dan.templeton@sun.com
 */
public class JobInfoImpl extends JobInfo {
   private int status = -1;
   private int exitCode = -1;
   
   /** Creates a new instance of JobInfoImpl */
   public JobInfoImpl (String jobId, int status, int exitCode, Map resourceUsage) {
      super (jobId, 0, resourceUsage);
      this.status = status;
      this.exitCode = exitCode;
   }

   public int getExitStatus () {
      return exitCode;
   }

   public String getTerminatingSignal () {
      return null;
   }

   public boolean hasCoreDump () {
      return false;
   }

   public boolean hasExited () {
      return ((status == DrmaaThread.DONE) || (status == DrmaaThread.FAILED));
   }

   public boolean hasSignaled () {
      return false;
   }

   public boolean wasAborted () {
      return (exitCode == DrmaaThread.ABORTED);
   }
}
