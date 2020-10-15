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
package com.sun.grid.jgdi.event;

import com.sun.grid.jgdi.BaseTestCase;
import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.EventClient;
import com.sun.grid.jgdi.JobSubmitter;
import java.util.HashMap;
import java.util.Map;
import junit.framework.Test;
import junit.framework.TestSuite;

/**
 *
 */
public class JobTaskEventTestCase extends BaseTestCase {
    
    private JGDI jgdi;
    private EventClient evc;
    
    /** Creates a new instance of SpecialEventTestCase */
    public JobTaskEventTestCase(String testName) {
        super(testName);
    }
    
    protected void setUp() throws Exception {
        
        jgdi = createJGDI();
        evc = createEventClient(0);
        super.setUp();
        logger.fine("SetUp done");
    }
    
    protected void tearDown() throws Exception {
        try {
            evc.close();
        } finally {
            jgdi.close();
            evc.close();
        }
    }
    
    public void testJobTaskEvents() throws Exception {
        
        
        jgdi.disableQueues(new String[]{"*"}, false);
        
        int numberOfTasks = 10;
        
        int jobid = JobSubmitter.submitJob(getCurrentCluster(), new String[]{"-t", "1-" + numberOfTasks, "$SGE_ROOT/examples/jobs/sleeper.sh", "1"});
        
        Map<EventTypeEnum,Integer> map = new HashMap<EventTypeEnum,Integer>();
        map.put(EventTypeEnum.JobTaskAdd, 1);
        map.put(EventTypeEnum.JobTaskDel, 1);
        map.put(EventTypeEnum.JobDel, 1);
        
        evc.subscribe(map.keySet());
        evc.setFlush(map);
        
        JobTaskEventListener lis = new JobTaskEventListener(jobid);
        evc.addEventListener(lis);
        
        evc.commit();
        
        Thread.sleep(2);
        
        jgdi.enableQueues(new String[]{"*"}, false);
        jgdi.triggerSchedulerMonitoring();
        
        int timeout = 300;
        assertTrue("timeout while waiting for job finish event", lis.waitForJobFinish(timeout));
        assertEquals("too few job task add events", numberOfTasks, lis.getAddEventCount());
        assertEquals("too few job task del events", numberOfTasks - 1, lis.getDelEventCount());
    }
    
    
    class JobTaskEventListener implements EventListener {
        
        private final Object syncObj = new Object();
        private int addEventCount = 0;
        private int delEventCount = 0;
        private boolean finished = false;
        private int jobid;
        
        public JobTaskEventListener(int jobid) {
            this.jobid = jobid;
        }
        
        public void eventOccured(Event evt) {
            
            if (evt instanceof JobTaskAddEvent) {
                JobTaskAddEvent jte = (JobTaskAddEvent) evt;
                if (jte.getJobId() == jobid) {
                    addEventCount++;
                    logger.info("task " + jte.getJobId() + "." + jte.getTaskNumber() + " started");
                }
            } else if (evt instanceof JobTaskDelEvent) {
                JobTaskDelEvent tde = (JobTaskDelEvent) evt;
                if (tde.getJobId() == jobid) {
                    delEventCount++;
                    logger.info("task " + tde.getJobId() + "." + tde.getTaskNumber() + " deleted");
                }
            } else if (evt instanceof JobDelEvent) {
                JobDelEvent jde = (JobDelEvent) evt;
                if (jde.getJobNumber() == jobid) {
                    logger.info("got job delete event: " + jde);
                    synchronized (syncObj) {
                        finished = true;
                        syncObj.notifyAll();
                    }
                }
            }
        }
        
        public boolean waitForJobFinish(int timeout) throws InterruptedException {
            long end = System.currentTimeMillis() + (timeout * 1000);
            synchronized (syncObj) {
                while (!finished) {
                    long waittime = end - System.currentTimeMillis();
                    if (waittime < 0) {
                        logger.info("timeout while waiting for final usage event");
                        return false;
                    }
                    syncObj.wait(waittime);
                }
            }
            return true;
        }
        
        public int getAddEventCount() {
            return addEventCount;
        }
        
        public int getDelEventCount() {
            return delEventCount;
        }
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(JobTaskEventTestCase.class);
        return suite;
    }
}
