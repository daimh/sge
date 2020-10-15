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
 * SessionImplJobTest.java
 * JUnit based test
 *
 * Created on November 15, 2004, 10:41 AM
 */

package com.sun.grid.drmaa;

import java.util.*;
import junit.framework.*;
import org.ggf.drmaa.*;
import com.sun.grid.Settings;

/**
 *
 * @author dan.templeton@sun.com
 */
public class SessionImplJobTest extends TestCase {
    private static final String SLEEPER;
    private Session session = null;
    
    static {
        SLEEPER = Settings.get(Settings.SCRIPTS_DIR) + "/suspendable_sleeper.sh";
    }
    
    public SessionImplJobTest(java.lang.String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(SessionImplJobTest.class);
        return suite;
    }
    
    public void setUp() throws DrmaaException {
        session = SessionFactory.getFactory().getSession();
        session.init(null);
    }
    
    public void tearDown() throws DrmaaException {
        try {
            session.exit();
        } catch (NoActiveSessionException ex) {
            // Ignore
        }
        
        session = null;
    }
    
    /** Test of runJob method, of class com.sun.grid.drmaa.SessionImpl. */
    public void testBadJobTemplate() {
        System.out.println("testBadJobTemplate");
        
        JobTemplate jt = new BadJobTemplate();
        
        try {
            try {
                session.runJob(jt);
                fail("Allowed bad job template");
            } catch (InvalidJobTemplateException e) {
                /* Don't care */
            }
            
            try {
                session.runBulkJobs(jt, 1, 3, 1);
                fail("Allowed bad job template");
            } catch (InvalidJobTemplateException e) {
                /* Don't care */
            }
            
            try {
                session.deleteJobTemplate(jt);
                fail("Allowed bad job template");
            } catch (InvalidJobTemplateException e) {
                /* Don't care */
            }
        } catch (DrmaaException e) {
            fail("Exception while trying to run job: " + e.getMessage());
        }
    }
    
    /** Test of runJob method, of class com.sun.grid.drmaa.SessionImpl. */
    public void testBadRunJob() {
        System.out.println("testBadRunJob");
        
        try {
            session.runJob(null);
            fail("Allowed null job template");
        } catch (NullPointerException e) {
            /* Don't care */
        } catch (DrmaaException e) {
            fail("Exception while trying to run job: " + e.getMessage());
        }
    }
    
    public void testRunJob() {
        System.out.println("testRunJob");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            String jobId = session.runJob(jt);
            
            assertNotNull(jobId);
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to run a job: " + e.getMessage());
        }
    }
    
    /** Test of runBulkJobs method, of class com.sun.grid.drmaa.SessionImpl. */
    public void testBadRunBulkJobs() {
        System.out.println("testBadRunBulkJobs");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            try {
                session.runBulkJobs(null, 1, 2, 1);
                fail("Allowed null job template");
            } catch (NullPointerException e) {
                /* Don't care */
            }
            
            try {
                session.runBulkJobs(jt, -1, 2, 1);
                fail("Allowed invalid start id");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            try {
                session.runBulkJobs(jt, 1, -2, 1);
                fail("Allowed invalid end id");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            try {
                session.runBulkJobs(jt, 1, 2, -1);
                fail("Allowed negative step when end > start");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            try {
                session.runBulkJobs(jt, 3, 2, 1);
                fail("Allowed end < start");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to run jobs: " + e.getMessage());
        }
    }
    
    public void testRunMonoBulkJobs() {
        System.out.println("testRunMonoBulkJobs");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            List jobIds = session.runBulkJobs(jt, 1, 1, 1);
            
            assertNotNull(jobIds);
            assertEquals(1, jobIds.size());
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to run jobs: " + e.getMessage());
        }
    }
    
    public void testRunDualBulkJobs() {
        System.out.println("testRunDualBulkJobs");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            List jobIds = session.runBulkJobs(jt, 1, 2, 1);
            
            assertNotNull(jobIds);
            assertEquals(2, jobIds.size());
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to run jobs: " + e.getMessage());
        }
    }
    
    public void testRunBulkJobs() {
        System.out.println("testRunBulkJobs");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            List jobIds = session.runBulkJobs(jt, 2, 6, 2);
            
            assertNotNull(jobIds);
            assertEquals(3, jobIds.size());
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to run jobs: " + e.getMessage());
        }
    }
    
    /** Test of wait method, of class com.sun.grid.drmaa.SessionImpl. */
    public void testBadWait() {
        System.out.println("testBadWait");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            String jobId = session.runJob(jt);
            
            /* Test bad parameters */
            try {
                session.wait(null, 0L);
                fail("Allowed null job id");
            } catch (NullPointerException e) {
                /* Don't care */
            }
            
            try {
                session.wait("asdf", 0L);
                fail("Allowed invalid job id");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            try {
                session.wait(jobId, -3L);
                fail("Allowed negative timeout");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    public void testWaitForever() {
        System.out.println("testWaitForever");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            String jobId = session.runJob(jt);
            
            /* Test wait forever */
            JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            /* Make sure that the previous wait worked. */
            try {
                session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                fail("First wait didn't reap job exit info");
            } catch (InvalidJobException e) {
                /* Don't care */
            }
            
         /* Eventually I need to build a libtestjdrmaa.so with some utilities in
          * it, like sendSignal() and causeException(), so that I can test some
          * error cases. */
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    public void testNoWait() {
        System.out.println("testNoWait");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test no wait */
            String jobId = session.runJob(jt);
            long now = System.currentTimeMillis();
            JobInfo info = null;
            
            try {
                info = session.wait(jobId, session.TIMEOUT_NO_WAIT);
                fail("Waited for job to finish; ignored 0s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that 1 second is more than enough time to check the cache
          * and return. */
            assertTrue((later - now) < 1000L);
            
            /* Make sure that the previous wait didn't disrupt anything. */
            info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            /* Make sure that the previous wait worked. */
            try {
                session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                fail("Second wait didn't reap job exit info");
            } catch (InvalidJobException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    public void testWaitTimeout() {
        System.out.println("testWaitTimeout");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait */
            String jobId = session.runJob(jt);
            long now = System.currentTimeMillis();
         /* I'm assuming that 3 seconds is not long enough for the job to be
          * scheduled and run. */
            JobInfo info = null;
            
            try {
                info = session.wait(jobId, 3L);
                fail("Waited for job to finish; ignored 3s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I'm assuming that there's no more than 1 second overhead in making
          * the call. */
            assertTrue((later - now) < 4000L);
            
            /* Make sure that the previous wait didn't disrupt anything. */
            info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            /* Make sure that the previous wait worked. */
            try {
                session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                fail("Second wait didn't reap job exit info");
            } catch (InvalidJobException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    public void testWait() {
        System.out.println("testWait");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait */
            String jobId = session.runJob(jt);
            JobInfo info = session.wait(jobId, 30L);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            /* Make sure that the previous wait worked. */
            try {
                session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                fail("First wait didn't reap job exit info");
            } catch (InvalidJobException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    public void testWaitForeverAny() {
        System.out.println("testWaitForeverAny");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            String jobId = session.runJob(jt);
            
            /* Test wait forever */
            JobInfo info = session.wait(session.JOB_IDS_SESSION_ANY, session.TIMEOUT_WAIT_FOREVER);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            /* Make sure that the previous wait worked. */
            try {
                session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                fail("First wait didn't reap job exit info");
            } catch (InvalidJobException e) {
                /* Don't care */
            }
            
         /* Eventually I need to build a libtestjdrmaa.so with some utilities in
          * it, like sendSignal() and causeException(), so that I can test some
          * error cases. */
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    public void testNoWaitAny() {
        System.out.println("testNoWaitAny");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test no wait */
            String jobId = session.runJob(jt);
            long now = System.currentTimeMillis();
            JobInfo info = null;
            
            try {
                info = session.wait(session.JOB_IDS_SESSION_ANY, session.TIMEOUT_NO_WAIT);
                fail("Waited for job to finish; ignored 0s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that 1 second is more than enough time to check the cache
          * and return. */
            assertTrue((later - now) < 1000L);
            
            /* Make sure that the previous wait didn't disrupt anything. */
            info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            /* Make sure that the previous wait worked. */
            try {
                session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                fail("Second wait didn't reap job exit info");
            } catch (InvalidJobException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    public void testWaitTimeoutAny() {
        System.out.println("testWaitTimeoutAny");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait */
            String jobId = session.runJob(jt);
            long now = System.currentTimeMillis();
         /* I'm assuming that 3 seconds is not long enough for the job to be
          * scheduled and run. */
            JobInfo info = null;
            
            try {
                info = session.wait(session.JOB_IDS_SESSION_ANY, 3L);
                fail("Waited for job to finish; ignored 3s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I'm assuming that there's no more than 1 second overhead in making
          * the call. */
            assertTrue((later - now) < 4000L);
            
            /* Make sure that the previous wait didn't disrupt anything. */
            info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            /* Make sure that the previous wait worked. */
            try {
                session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                fail("Second wait didn't reap job exit info");
            } catch (InvalidJobException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    public void testWaitAny() {
        System.out.println("testWaitAny");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait */
            String jobId = session.runJob(jt);
            JobInfo info = session.wait(session.JOB_IDS_SESSION_ANY, 30L);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            /* Make sure that the previous wait worked. */
            try {
                session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                fail("First wait didn't reap job exit info");
            } catch (InvalidJobException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to wait for a job: " + e.getMessage());
        }
    }
    
    /** Test of synchronize method, of class com.sun.grid.drmaa.SessionImpl. */
    public void testBadSynchronize() {
        System.out.println("testBadSynchronize");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            List jobIds = session.runBulkJobs(jt, 1, 3, 1);
            
            /* Test bad parameters */
            try {
                session.synchronize(null, session.TIMEOUT_WAIT_FOREVER, true);
                fail("Allowed null job id");
            } catch (NullPointerException e) {
                /* Don't care */
            }
            
            List badJobId = Arrays.asList(new String[] {"asdf"});
            
            try {
                session.synchronize(badJobId, session.TIMEOUT_WAIT_FOREVER, true);
                fail("Allowed invalid job id");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            try {
                session.synchronize(jobIds, -3, true);
                fail("Allowed negative timeout");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeForeverDispose() {
        System.out.println("testSynchronizeForeverDispose");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            /* Test wait forever, dispose */
            session.synchronize(jobIds, session.TIMEOUT_WAIT_FOREVER, true);
            
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                try {
                    session.wait((String)i.next(), session.TIMEOUT_NO_WAIT);
                    fail("Synchronize didn't reap exit information");
                } catch (InvalidJobException e) {
                    /* Don't care */
                }
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeNoWaitDispose() {
        System.out.println("testSynchronizeNoWaitDispose");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test no wait, dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            long now = System.currentTimeMillis();
            
            try {
                session.synchronize(jobIds, session.TIMEOUT_NO_WAIT, true);
                fail("Waited for jobs to finish; ignored 0s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that 1 second is more than enough time to check the cache
          * and return. */
            assertTrue((later - now) < 1000L);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeTimeoutDispose() {
        System.out.println("testSynchronizeTimeoutDispose");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(15);
            
            /* Test timed wait (timeout), dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            long now = System.currentTimeMillis();
            
         /* I'm assuming that 3 seconds won't be enough time for the job to be
          * scheduled and run. */
            try {
                session.synchronize(jobIds, 3L, true);
                fail("Waited for jobs to finish; ignored 3s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that there is less than 1 second overhead in making the
          * call. */
            assertTrue((later - now) < 4000L);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeDispose() {
        System.out.println("testSynchronizeDispose");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait, dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            session.synchronize(jobIds, 600L, true);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                try {
                    session.wait((String)i.next(), session.TIMEOUT_NO_WAIT);
                    fail("Synchronize didn't reap exit information");
                } catch (InvalidJobException e) {
                    /* Don't care */
                }
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeForever() {
        System.out.println("testSynchronizeForever");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test wait forever, no dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            session.synchronize(jobIds, session.TIMEOUT_WAIT_FOREVER, false);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeNoWait() {
        System.out.println("testSynchronizeNoWait");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test no wait, no dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            long now = System.currentTimeMillis();
            
            try {
                session.synchronize(jobIds, session.TIMEOUT_NO_WAIT, false);
                fail("Waited for jobs to finish; ignored 0s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that there is less than 1 second overhead in making the
          * call. */
            assertTrue((later - now) < 1000L);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeTimeout() {
        System.out.println("testSynchronizeTimeout");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(15);
            
            /* Test timed wait (timeout), no dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            long now = System.currentTimeMillis();
            
         /* I'm assuming that 3 seconds won't be enough time for the job to be
          * scheduled and run. */
            try {
                session.synchronize(jobIds, 3L, false);
                fail("Waited for jobs to finish; ignored 3s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that there is less than 1 second overhead in making the
          * call. */
            assertTrue((later - now) < 4000L);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronize() {
        System.out.println("testSynchronize");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait (timeout), no dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            session.synchronize(jobIds, 600L, false);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeForeverDisposeAll() {
        System.out.println("testSynchronizeForeverDisposeAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            /* Test wait forever, dispose */
            session.synchronize(Collections.singletonList(session.JOB_IDS_SESSION_ALL), session.TIMEOUT_WAIT_FOREVER, true);
            
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                try {
                    session.wait((String)i.next(), session.TIMEOUT_NO_WAIT);
                    fail("Synchronize didn't reap exit information");
                } catch (InvalidJobException e) {
                    /* Don't care */
                }
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeNoWaitDisposeAll() {
        System.out.println("testSynchronizeNoWaitDisposeAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test no wait, dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            long now = System.currentTimeMillis();
            
            try {
                session.synchronize(Collections.singletonList(session.JOB_IDS_SESSION_ALL), session.TIMEOUT_NO_WAIT, true);
                fail("Waited for jobs to finish; ignored 0s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that 1 second is more than enough time to check the cache
          * and return. */
            assertTrue((later - now) < 1000L);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeTimeoutDisposeAll() {
        System.out.println("testSynchronizeTimeoutDisposeAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(15);
            
            /* Test timed wait (timeout), dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            long now = System.currentTimeMillis();
            
         /* I'm assuming that 3 seconds won't be enough time for the job to be
          * scheduled and run. */
            try {
                session.synchronize(Collections.singletonList(session.JOB_IDS_SESSION_ALL), 3L, true);
                fail("Waited for jobs to finish; ignored 3s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that there is less than 1 second overhead in making the
          * call. */
            assertTrue((later - now) < 4000L);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeDisposeAll() {
        System.out.println("testSynchronizeDisposeAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait, dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            session.synchronize(Collections.singletonList(session.JOB_IDS_SESSION_ALL), 600L, true);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                try {
                    JobInfo info = session.wait((String)i.next(), session.TIMEOUT_NO_WAIT);
                    fail("Synchronize didn't reap exit information for " + info.getJobId());
                } catch (InvalidJobException e) {
                    /* Don't care */
                }
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeForeverAll() {
        System.out.println("testSynchronizeForeverAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test wait forever, no dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            session.synchronize(Collections.singletonList(session.JOB_IDS_SESSION_ALL), session.TIMEOUT_WAIT_FOREVER, false);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeNoWaitAll() {
        System.out.println("testSynchronizeNoWaitAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test no wait, no dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            long now = System.currentTimeMillis();
            
            try {
                session.synchronize(Collections.singletonList(session.JOB_IDS_SESSION_ALL), session.TIMEOUT_NO_WAIT, false);
                fail("Waited for jobs to finish; ignored 0s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that there is less than 1 second overhead in making the
          * call. */
            assertTrue((later - now) < 1000L);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeTimeoutAll() {
        System.out.println("testSynchronizeTimeoutAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(15);
            
            /* Test timed wait (timeout), no dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            long now = System.currentTimeMillis();
            
         /* I'm assuming that 3 seconds won't be enough time for the job to be
          * scheduled and run. */
            try {
                session.synchronize(Collections.singletonList(session.JOB_IDS_SESSION_ALL), 3L, false);
                fail("Waited for jobs to finish; ignored 3s timeout");
            } catch (ExitTimeoutException e) {
                /* Don't care */
            }
            
            long later = System.currentTimeMillis();
            
         /* I assume that there is less than 1 second overhead in making the
          * call. */
            assertTrue((later - now) < 4000L);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeAll() {
        System.out.println("testSynchronizeAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait (timeout), no dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            session.synchronize(Collections.singletonList(session.JOB_IDS_SESSION_ALL), 600L, false);
            
            /* Make sure that the previous synchronize didn't disrupt anything. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                String jobId = (String)i.next();
                JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
                
                assertNotNull(info);
                assertEquals(jobId, info.getJobId());
                
                /* There's no reason that this job should exit prematurely. */
                assertTrue(info.hasExited());
                assertEquals(0, info.getExitStatus());
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeForeverDisposeMoreThanAll() {
        System.out.println("testSynchronizeDisposeMoreThanAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait, dispose */
            List jobIds = new LinkedList(session.runBulkJobs(jt, 1, 3, 1));
            
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            jobIds.addAll(session.runBulkJobs(jt, 1, 3, 1));
            
            List myJobIds = new ArrayList(2);
            
            myJobIds.add(jobIds.get(0));
            myJobIds.add(session.JOB_IDS_SESSION_ALL);
            session.synchronize(myJobIds, session.TIMEOUT_WAIT_FOREVER, true);
            
            /* Make sure that the previous synchronize reaped exit info. */
            Iterator i = jobIds.iterator();
            
            while (i.hasNext()) {
                try {
                    session.wait((String)i.next(), session.TIMEOUT_NO_WAIT);
                    fail("Synchronize didn't reap exit information");
                } catch (InvalidJobException e) {
                    /* Don't care */
                }
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    public void testSynchronizeNonexistant() {
        System.out.println("testSynchronizeNonexistant");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            /* Test timed wait (timeout), no dispose */
            String jobId = session.runJob(jt);
            /* Create a valid, unknown id. */
            String nextId = Integer.toString(Integer.parseInt(jobId) + 1);
            List jobIds = Collections.singletonList(nextId);
            
            try {
                session.synchronize(jobIds, session.TIMEOUT_WAIT_FOREVER, false);
                // Success!
            } catch (InvalidJobException e) {
                fail("Synchronize on non-existant job id failed");
            }
            
            /* Wait for the real job to end. */
            JobInfo info = session.wait(jobId, session.TIMEOUT_WAIT_FOREVER);
            
            assertNotNull(info);
            assertEquals(jobId, info.getJobId());
            
            /* There's no reason that this job should exit prematurely. */
            assertTrue(info.hasExited());
            assertEquals(0, info.getExitStatus());
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to synchronize jobs: " + e.getMessage());
        }
    }
    
    /** Test of getJobProgramStatus method, of class com.sun.grid.drmaa.SessionImpl. */
    public void testBadGetJobProgramStatus() {
        System.out.println("testBadGetJobProgramStatus");
        
        try {
            try {
                session.getJobProgramStatus(null);
                fail("Allowed null job id");
            } catch (NullPointerException e) {
                /* Don't care */
            }
            
            try {
                session.getJobProgramStatus("asdf");
                fail("Allowed invalid job id");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
        } catch (DrmaaException e) {
            fail("Exception while trying to get job status: " + e.getMessage());
        }
    }
    
    public void testGetJobProgramStatus() {
        System.out.println("testGetJobProgramStatus");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            String jobId = session.runJob(jt);
         /* Make sure it doesn't throw an exception.  We can't really be sure
          * what the state will be at this point. */
            session.getJobProgramStatus(jobId);
            /* We use synchronize so that we don't reap the job info. */
            session.synchronize(Collections.singletonList(jobId), session.TIMEOUT_WAIT_FOREVER, false);
            
            int status = session.getJobProgramStatus(jobId);
            
            /* No reason why this job should fail. */
            assertEquals(session.DONE, status);
            
            /* How do we test the other states??? */
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to get job status: " + e.getMessage());
        }
    }
    
    /** Test of control method, of class com.sun.grid.drmaa.SessionImpl. */
    public void testBadControl() {
        System.out.println("testBadControl");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(5);
            
            String jobId = session.runJob(jt);
            
            try {
                session.control(null, session.HOLD);
                fail("Allowed null job id");
            } catch (NullPointerException e) {
                /* Don't care */
            }
            
            try {
                session.control("asdf", session.HOLD);
                fail("Allowed invalid job id");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            try {
                session.control(jobId, -10);
                fail("Allowed invalid action");
            } catch (IllegalArgumentException e) {
                /* Don't care */
            }
            
            session.deleteJobTemplate(jt);
        } catch (DrmaaException e) {
            fail("Exception while trying to get job status: " + e.getMessage());
        }
    }
    
    public void testControl() {
        System.out.println("testControl");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(60000);
            
            String jobId = session.runJob(jt);
            
            session.deleteJobTemplate(jt);
            
            /* Take a nap so that we give the job time to get scheduled. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            int status = session.getJobProgramStatus(jobId);
            
            /* Make sure the job is running. */
            assertEquals(session.RUNNING, status);
            
            session.control(jobId, session.HOLD);
            
            /* Take a nap so that we give the job time to held. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            status = session.getJobProgramStatus(jobId);
            assertEquals(session.USER_ON_HOLD, status);
            
            session.control(jobId, session.RELEASE);
            
            /* Take a nap so that we give the job time to released. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            status = session.getJobProgramStatus(jobId);
            assertEquals(session.RUNNING, status);
            
            session.control(jobId, session.SUSPEND);
            
            /* Take a nap so that we give the job time to suspended. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            status = session.getJobProgramStatus(jobId);
            assertEquals(session.USER_SUSPENDED, status);
            
            session.control(jobId, session.RESUME);
            
            /* Take a nap so that we give the job time to resumed. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            status = session.getJobProgramStatus(jobId);
            assertEquals(session.RUNNING, status);
            
            session.control(jobId, session.TERMINATE);
            
            /* Take a nap so that we give the job time to killed. */
            try {
                Thread.sleep(60000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            status = session.getJobProgramStatus(jobId);
            assertEquals(session.FAILED, status);
        } catch (DrmaaException e) {
            fail("Exception while trying to get job status: " + e.getMessage());
        }
    }
    
    public void testControlAll() {
        System.out.println("testControlAll");
        
        try {
            JobTemplate jt = this.createSleeperTemplate(60000);
            
            String jobId1 = session.runJob(jt);
            String jobId2 = session.runJob(jt);
            String jobId3 = session.runJob(jt);
            
            session.deleteJobTemplate(jt);
            
            /* Take a nap so that we give the jobs time to get scheduled. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            /* Make sure the job is running. */
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId1));
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId2));
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId3));
            
            session.control(Session.JOB_IDS_SESSION_ALL, session.HOLD);
            
            /* Take a nap so that we give the jobs time to held. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            assertEquals(session.USER_ON_HOLD, session.getJobProgramStatus(jobId1));
            assertEquals(session.USER_ON_HOLD, session.getJobProgramStatus(jobId2));
            assertEquals(session.USER_ON_HOLD, session.getJobProgramStatus(jobId3));
            
            session.control(Session.JOB_IDS_SESSION_ALL, session.RELEASE);
            
            /* Take a nap so that we give the jobs time to released. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId1));
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId2));
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId3));
            
            session.control(Session.JOB_IDS_SESSION_ALL, session.SUSPEND);
            
            /* Take a nap so that we give the jobs time to suspended. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            assertEquals(session.USER_SUSPENDED, session.getJobProgramStatus(jobId1));
            assertEquals(session.USER_SUSPENDED, session.getJobProgramStatus(jobId2));
            assertEquals(session.USER_SUSPENDED, session.getJobProgramStatus(jobId3));
            
            session.control(Session.JOB_IDS_SESSION_ALL, session.RESUME);
            
            /* Take a nap so that we give the jobs time to resumed. */
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId1));
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId2));
            assertEquals(session.RUNNING, session.getJobProgramStatus(jobId3));
            
            session.control(Session.JOB_IDS_SESSION_ALL, session.TERMINATE);
            
            /* Take a nap so that we give the jobs time to killed. */
            try {
                Thread.sleep(60000);
            } catch (InterruptedException e) {
                fail("Sleep was interrupted");
            }
            
            assertEquals(session.FAILED, session.getJobProgramStatus(jobId1));
            assertEquals(session.FAILED, session.getJobProgramStatus(jobId2));
            assertEquals(session.FAILED, session.getJobProgramStatus(jobId3));
        } catch (DrmaaException e) {
            fail("Exception while trying to get job status: " + e.getMessage());
        }
    }
    
    public void testRecoverableSession() throws DrmaaException {
        System.out.println("testRecoverableSession");
        
        String contact = session.getContact();
        
        JobTemplate jt = this.createSleeperTemplate(120);
        
        String jobId1 = session.runJob(jt);
        
        session.deleteJobTemplate(jt);
        
        jt = this.createSleeperTemplate(10);
        
        String jobId2 = session.runJob(jt);
        
        session.deleteJobTemplate(jt);
        
        jt = this.createSleeperTemplate(10);
        jt.setJobSubmissionState(jt.HOLD_STATE);
        
        List jobIds34 = session.runBulkJobs(jt, 1, 2, 1);
        
        session.deleteJobTemplate(jt);
        session.control((String)jobIds34.get(0), session.RELEASE);
        
        session.exit();
        
        /* Take a nap so that we give the job time to get scheduled. */
        try {
            Thread.sleep(60000);
        } catch (InterruptedException e) {
            fail("Sleep was interrupted");
        }
        
        session.init(contact);
        
        JobInfo ji = session.wait(jobId1, session.TIMEOUT_WAIT_FOREVER);
        
        assertEquals(true, ji.hasExited());
        assertEquals(0, ji.getExitStatus());
        assertEquals(jobId1, ji.getJobId());
        
        try {
            ji = session.wait(jobId2, session.TIMEOUT_WAIT_FOREVER);

            assertNull(ji.getResourceUsage());
        } catch (InvalidJobException e) {
            // Supposed to happen
        }
        
        ji = session.wait((String)jobIds34.get(0), session.TIMEOUT_WAIT_FOREVER);
        assertNull(ji.getResourceUsage());
        
        try {
            session.wait((String)jobIds34.get(1), session.TIMEOUT_NO_WAIT);
            fail("Call to wait() did not time out as expect");
        } catch (ExitTimeoutException ex) {
            // Supposed to happen
        }
        
        session.control((String)jobIds34.get(1), session.TERMINATE);
    }
    
    private JobTemplate createSleeperTemplate(int sleep) throws DrmaaException {
        JobTemplate jt = session.createJobTemplate();
        
        jt.setRemoteCommand(SLEEPER);
        jt.setArgs(Collections.singletonList(Integer.toString(sleep)));
        jt.setOutputPath(":/dev/null");
        jt.setJoinFiles(true);
        
        return jt;
    }
    
    private class BadJobTemplate extends SimpleJobTemplate {
        public BadJobTemplate() {
        }
    }
}
