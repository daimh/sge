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
 * Session.java
 *
 * Created on March 29, 2005, 9:30 PM
 */

package org.ggf.drmaa.harness;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import org.ggf.drmaa.*;

/**
 *
 * @author dan.templeton@sun.com
 */
public class SessionImpl implements Session {
   public static final String SLEEPER = "SLEEPER";
   public static final String EXIT = "EXIT";
   private static long nextId = 0L;
   private Object lock = new Object ();
   private boolean initialized = false;
   private Map threadMap = null;
   
   /** Creates a new instance of Session */
   public SessionImpl () {
      threadMap = new HashMap ();
   }

   public void control (String jobId, int action) throws DrmaaException {
      List ids = null;
      Iterator i = null;
      
      if (jobId.equals (Session.JOB_IDS_SESSION_ALL)) {
         ids = new ArrayList (threadMap.keySet ());
      }
      else {
         ids = Collections.singletonList (jobId);
      }
      
      i = ids.iterator ();

      while (i.hasNext ()) {
         DrmaaThread thread = null;
         String id = (String)i.next ();
         
         synchronized (lock) {
            thread = (DrmaaThread)threadMap.get (id);

            if (thread == null) {
               throw new InvalidJobException ();
            }
         }
                 
         switch (action) {
            case Session.TERMINATE:
               thread.kill ();
               break;
            case Session.SUSPEND:
               if (thread.getState () != DrmaaThread.RUNNING) {
                  throw new SuspendInconsistentStateException ();
               }
               
               thread.setState (DrmaaThread.SUSPENDED);
               break;
            case Session.RESUME:
               if (thread.getState () != DrmaaThread.SUSPENDED) {
                  throw new ResumeInconsistentStateException ();
               }
               
               thread.setState (DrmaaThread.RUNNING);
               break;
            case Session.HOLD:
               if (thread.getState () != DrmaaThread.RUNNING) {
                  throw new HoldInconsistentStateException ();
               }
               
               thread.setState (DrmaaThread.HOLD);
               break;
            case Session.RELEASE:
               if (thread.getState () != DrmaaThread.HOLD) {
                  throw new ReleaseInconsistentStateException ();
               }
               
               thread.setState (DrmaaThread.RUNNING);
               break;
            default:
               throw new InvalidArgumentException ();
         }
      }
   }

   public JobTemplate createJobTemplate () throws DrmaaException {
      return new JobTemplate ();
   }

   public void deleteJobTemplate (JobTemplate jobTemplate) throws DrmaaException {
      /* No worries */
   }

   public void exit () throws DrmaaException {
      synchronized (lock) {
         initialized = false;
      }
   }

   public String getContact () {
      return "";
   }

   public String getDrmSystem () {
      return this.getDrmaaImplementation ();
   }

   public String getDrmaaImplementation () {
      return "DRMAA Test Harness 0.1";
   }

   public int getJobProgramStatus (String jobId) throws DrmaaException {
      DrmaaThread exec = (DrmaaThread)threadMap.get (jobId);
      
      return translateState (exec.getState ());
   }

   public Version getVersion () {
      return new Version (0, 5);
   }

   public void init (String contact) throws DrmaaException {
      synchronized (lock) {
         initialized = true;
      }
   }

   public java.util.List runBulkJobs (JobTemplate jobTemplate, int start, int end, int step) throws DrmaaException {
      ArrayList idList = new ArrayList ((end - start) / step);
      
      for (int count = start; count < end; count += step) {
         String id = getNextJobId ();
         
         this.runJob (jobTemplate, id);
         idList.add (id);
      }
      
      return idList;
   }

   public String runJob (JobTemplate jobTemplate) throws DrmaaException {
      String id = getNextJobId ();
      
      this.runJob (jobTemplate, id);
      
      return id;
   }

   private void runJob (JobTemplate jobTemplate, String id) throws DrmaaException {
      DrmaaThread thread = null;
      
      if (jobTemplate.getRemoteCommand ().equals (SLEEPER)) {
         thread = new Sleeper (id, jobTemplate, lock);
      }
      else if (jobTemplate.getRemoteCommand ().equals (EXIT)) {
         thread = new Exit (id, jobTemplate, lock);
      }
      else {
         thread = new Executor (id, jobTemplate, lock);
      }
      
      Thread execThread = new Thread (thread);
      
      synchronized (lock) {
         threadMap.put (id, thread);
      }
      
      execThread.start ();
   }
   
   public void synchronize (java.util.List jobIds, long timeout, boolean dispose) throws DrmaaException {
      if (jobIds.contains (Session.JOB_IDS_SESSION_ALL)) {
         synchronized (lock) {
            jobIds = new ArrayList (threadMap.keySet ());
         }
      }
      
      Iterator i = jobIds.iterator ();
      
      while (i.hasNext ()) {
         String id = (String)i.next ();
         
         this.wait (id, timeout, dispose);
      }
   }

   public JobInfo wait (String jobId, long timeout) throws DrmaaException {
      return this.wait (jobId, timeout, true);
   }

   private JobInfo wait (String jobId, long timeout, boolean dispose) throws DrmaaException {
      DrmaaThread thread = null;
      
      if (jobId.equals (Session.JOB_IDS_SESSION_ANY)) {
         thread = this.waitAny (timeout);
      }
      else {
         synchronized (lock) {
            thread = (DrmaaThread)threadMap.remove (jobId);
         }

         if (thread != null) {
            thread.waitForJob ();
         }
         else {
            throw new InvalidJobException ();
         }
      }

      JobInfo info = null;
      
      if (thread != null) {
         if (!dispose) {
            synchronized (lock) {
               threadMap.put (thread.getId (), thread);
            }
         }
         
         info = new JobInfoImpl (thread.getId (), thread.getState (), thread.getExitCode (), thread.getMap ());
      }
      
      return info;
   }

   private DrmaaThread waitAny (long timeout) throws DrmaaException {
      while (true) {
         synchronized (lock) {
            if (threadMap.size () == 0) {
               throw new ExitTimeoutException ();
            }
            
            Iterator i = threadMap.keySet ().iterator ();

            while (i.hasNext ()) {
               Object id = i.next ();
               DrmaaThread thread = (DrmaaThread)threadMap.get (id);

               if ((thread.getState() == DrmaaThread.DONE) ||
                   (thread.getState() == DrmaaThread.FAILED)) {

                  i.remove ();
                  return thread;
               }
            }

            if (timeout == Session.TIMEOUT_NO_WAIT) {
               throw new ExitTimeoutException ();
            }
            else {
               long waitTime = timeout;
               
               if (waitTime == Session.TIMEOUT_WAIT_FOREVER) {
                  waitTime = 0L;
               }
               
               try {
                  lock.wait (waitTime);
               }
               catch (InterruptedException e) {
                  /* Ignore and go around again. */
               }
            }
         }
      }
   }
   
   private synchronized static String getNextJobId () {
      String id = Long.toString (nextId);
      
      nextId++;
      
      return id;
   }
   
   private static int translateState (int state) {
      switch (state) {
         case DrmaaThread.DONE:
            return Session.DONE;
         case DrmaaThread.FAILED:
            return Session.FAILED;
         case DrmaaThread.PENDING:
            return Session.QUEUED_ACTIVE;
         case DrmaaThread.RUNNING:
            return Session.RUNNING;
         default:
            return Session.UNDETERMINED;
      }
   }
}
