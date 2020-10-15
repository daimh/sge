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
 * Issues.java
 * JUnit based test
 *
 * Created on September 7, 2005, 3:50 PM
 */

package com.sun.grid.drmaa;

import java.io.*;
import java.util.*;
import junit.framework.*;
import org.ggf.drmaa.*;
import com.sun.grid.Settings;

/**
 *
 * @author dan.templeton@sun.com
 */
public class Issues extends TestCase {
    public Issues(String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(Issues.class);
        return suite;
    }
    
    public void setUp() throws DrmaaException {
    }
    
    public void tearDown() throws DrmaaException {
    }
    
    public void testIssue1770() throws DrmaaException {
        System.out.println("testIssue1770");
        
        String hostname = Settings.get(Settings.HOSTNAME);
        String pid = Settings.get(Settings.PID);
        String homeDir = Settings.get(Settings.HOME_DIR);
        String catname1 = hostname + pid + "cat1";
        String catname2 = hostname + pid + "cat2";
        String jobName = "TEST" + System.getProperty("user.name") + pid;
        File qtask = new File(homeDir, ".qtask");
        File script = new File("/tmp", "script" + hostname + pid + ".sh");
        File output1 = new File("/tmp", "1770" + hostname + pid);
        File output2 = null;
        Session s = SessionFactory.getFactory().getSession();
        
        try {
            try {
                // Add job categories to user's .qtask file
                FileWriter fout = new FileWriter(qtask);
                
                fout.write(catname1 + " -b n\n" + catname2 + " -b y\n");
                fout.close();
            } catch (IOException e) {
                fail("Unable to modify user qtask file");
                return;
            }
            
            try {
                // Create the job script
                FileWriter fout = new FileWriter(script);
                
                fout.write("#!/bin/sh\n");
                fout.write("#$ -S /bin/sh\n");
                fout.write("#$ -o " + output1.getAbsolutePath() + "\n");
                fout.write("/bin/echo $1");
                fout.close();
            } catch (IOException e) {
                fail("Unable to create job script");
                return;
            }
            
            Process p = null;
            
            try {
                p = Runtime.getRuntime().exec("chmod a+x " + script.getAbsolutePath());
                p.waitFor();
            } catch (IOException e) {
                fail("Unable to make script file executable");
                return;
            } catch (InterruptedException e) {
                System.err.println("Interrupted while waiting for chmod to finish");
            }
            
            // Initialize the session
            s.init("");
            
            LockBox lock = new LockBox();
            Thread1770 t1 = new Thread1770(s, script.getAbsolutePath(), catname1, hostname, jobName, lock);
            Thread1770 t2 = new Thread1770(s, script.getAbsolutePath(), catname2, hostname, jobName, lock);
            
            t1.start();
            t2.start();
            
            int got = lock.decrement(2, 60);
            
            if (got < 2) {
                fail("Timed out waiting for job submission, most likely because of Issue 1770");
                return;
            }
            
            if (t1.e != null) {
                throw t1.e;
            } else if (t2.e != null) {
                throw t2.e;
            }
            
            BufferedReader bin = null;
            String line = null;
            
            // Attempt to read the output from the location specified in the script
            try {
                bin = new BufferedReader(new FileReader(output1));
                line = bin.readLine();
                bin.close();
            } catch (IOException e) {
                fail("Unable to open output file for first category (" +
                        output1.getAbsolutePath() + ")");
                return;
            }
            
            assertEquals(catname1, line);
            
            output2 = new File("/tmp/" + jobName + ".o" + t2.jobId);
            
            // Atempt to read the output from the default output file
            try {
                bin = new BufferedReader(new FileReader(output2));
                line = bin.readLine();
                bin.close();
            } catch (IOException e) {
                fail("Unable to open output file for second category (" +
                        output2.getAbsolutePath() + ")");
                return;
            }
            assertEquals(catname2, line);
        } finally {
            // Clean up
            s.exit();
            qtask.delete();
            script.delete();
            output1.delete();
            
            if (output2 != null) {
                output2.delete();
            }
        }
    }
    
    private class Thread1770 extends Thread {
        private Session s = null;
        private String script = null;
        private String catname = null;
        private String hostname = null;
        private String jobname = null;
        String jobId = null;
        LockBox lock = null;
        DrmaaException e = null;
        
        Thread1770(Session s, String script, String catname, String hostname, String jobName, LockBox lock) {
            super("Thread1770");
            this.s = s;
            this.script = script;
            this.catname = catname;
            this.hostname = hostname;
            this.jobname = jobName;
            this.lock = lock;
            this.setDaemon(true);
        }
        
        public void run() {
            try {
                // Prep the job template
                JobTemplate jt = s.createJobTemplate();
                
                jt.setRemoteCommand(script);
            /* We're running the test on the local machine so that it's easier
             * to find the output files. */
                jt.setNativeSpecification("-N " + jobname + " -q all.q@" + hostname);
                jt.setJobCategory(catname);
                jt.setWorkingDirectory("/tmp");
                jt.setArgs(Collections.singletonList(catname));
                jt.setErrorPath(":/dev/null");
                
                // Run the job
                jobId = s.runJob(jt);
                
                JobInfo ji = s.wait(jobId, Session.TIMEOUT_WAIT_FOREVER);
                
                if (!ji.hasExited()) {
                    fail("Unable to run script job " + ji.getJobId());
                    return;
                }
                
                s.deleteJobTemplate(jt);
            } catch (DrmaaException e) {
                this.e = e;
            } finally {
                lock.increment();
            }
        }
    }
    
    private class LockBox extends Object implements Serializable {
        private int count = 0;
        
        LockBox() {
            this(0);
        }
        
        LockBox(int count) {
            this.count = count;
        }
        
        synchronized int increment() {
            return this.increment(1);
        }
        
        synchronized int decrement() {
            return this.decrement(1, 0);
        }
        
        synchronized int decrement(int decr) {
            return this.decrement(decr, 0);
        }
        
        synchronized int increment(int incr) {
            count += incr;
            this.notifyAll();
            return incr;
        }
        
        synchronized int decrement(int decr, int timeout) {
            int decrOrig = decr;
            long timeOut = 0L;
            boolean timedOut = false;
            
            if (timeout > 0) {
                timeOut = System.currentTimeMillis() + (timeout * 1000L);
            }
            
            while (count < decr) {
                long timeWait = 0L;
                
                if (timeout > 0) {
                    timeWait = timeOut - System.currentTimeMillis();
                    
                    if (timeWait <= 0) {
                        timedOut = true;
                        break;
                    }
                }
                
                if (count > 0) {
                    decr -= count;
                    count = 0;
                }
                
                try {
                    this.wait(timeWait);
                } catch (InterruptedException e) {
                    // Let the conditional sort it out.
                }
            }
            
            if (!timedOut) {
                count -= decr;
                return decrOrig;
            } else {
                return decrOrig - decr;
            }
        }
    }
}
