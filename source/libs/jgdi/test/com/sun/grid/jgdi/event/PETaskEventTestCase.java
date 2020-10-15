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
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JobSubmitter;
import com.sun.grid.jgdi.configuration.ClusterQueue;
import com.sun.grid.jgdi.configuration.ParallelEnvironment;
import com.sun.grid.jgdi.configuration.ParallelEnvironmentImpl;
import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import junit.framework.Test;
import junit.framework.TestSuite;

/**
 *
 */
public class PETaskEventTestCase extends BaseTestCase {
    
    private JGDI jgdi;
    private EventClient evc;
    
    /** Creates a new instance of PETaskEventTestCase */
    public PETaskEventTestCase(String testName) {
        super(testName);
    }
    
    private ParallelEnvironment pe;
    private ClusterQueue queue;
    
    protected void setUp() throws Exception {
        
        jgdi = createJGDI();
        evc = createEventClient(0);
        super.setUp();
        
        String peName = "pe" + System.currentTimeMillis();
        
        pe = new ParallelEnvironmentImpl();
        pe.setName(peName);
        pe.setSlots(999);
        pe.setStartProcArgs("NONE");
        pe.setStopProcArgs("NONE");
        pe.setAllocationRule("$pe_slots");
        pe.setControlSlaves(true);
        pe.setJobIsFirstTask(true);
        pe.setUrgencySlots("min");
        
        jgdi.addParallelEnvironment(pe);
        
        queue = jgdi.getClusterQueue("all.q");
        
        String queueName = peName + ".q";
        queue.setName(queueName);
        queue.putJobSlots("@/", 2);
        queue.removeAllPe();
        queue.addDefaultPe(peName);
        jgdi.addClusterQueue(queue);
        
        logger.fine("SetUp done");
    }
    
    protected void tearDown() throws Exception {
        if (queue != null) {
            try {
                jgdi.deleteClusterQueue(queue);
            } catch (JGDIException ex) {
                logger.log(Level.WARNING, ex.getLocalizedMessage(), ex);
            }
        }
        if (pe != null) {
            try {
                jgdi.deleteParallelEnvironment(pe);
            } catch (JGDIException ex) {
                logger.log(Level.WARNING, ex.getLocalizedMessage(), ex);
            }
        }
        try {
            evc.close();
        } finally {
            jgdi.close();
        }
    }
    
    public void testPETaskEvents() throws Exception {
        
        
        jgdi.disableQueues(new String[]{queue.getName()}, false);
        
        int numberOfTasks = 2;
        int taskRuntime = 10;
        
        File peJobFile = new File("util/scripts/pe_job.sh");
        File peTaskFile = new File("util/scripts/pe_task.sh");
        
        int jobid = JobSubmitter.submitJob(getCurrentCluster(), new String[]{"-pe", pe.getName(), Integer.toString(numberOfTasks), peJobFile.getAbsolutePath(), peTaskFile.getAbsolutePath(), Integer.toString(numberOfTasks), Integer.toString(taskRuntime)});
        
        Map<EventTypeEnum,Integer> map = new HashMap<EventTypeEnum,Integer>();
        map.put(EventTypeEnum.PETaskAdd, 1);
        map.put(EventTypeEnum.PETaskDel, 1);
        map.put(EventTypeEnum.JobDel, 1);
        
        evc.subscribe(map.keySet());
        evc.setFlush(map);
        
        PETaskEventListener lis = new PETaskEventListener(jobid);
        evc.addEventListener(lis);
        
        evc.commit();
        
        Thread.sleep(2);
        
        jgdi.enableQueues(new String[]{queue.getName()}, false);
        
        
        assertTrue("timeout while waiting for job finish event", lis.waitForJobFinish(taskRuntime * 10));
        assertEquals("Too few pe task add events", numberOfTasks, lis.getAddEventCount());
        assertEquals("Too few pe task del events", numberOfTasks - 1, lis.getDelEventCount());
    }
    
    
    class PETaskEventListener implements EventListener {
        
        private final Object finishSync = new Object();
        private int addEventCount = 0;
        private int delEventCount = 0;
        private int jobid;
        private boolean finished = false;
        
        public PETaskEventListener(int jobid) {
            this.jobid = jobid;
        }
        
        public void eventOccured(Event evt) {
            
            if (evt instanceof PETaskAddEvent) {
                PETaskAddEvent jte = (PETaskAddEvent) evt;
                if (jte.getJobId() == jobid) {
                    addEventCount++;
                    logger.info("pe task " + jte.getJobId() + "." + jte.getTaskNumber() + " started");
                }
            } else if (evt instanceof PETaskDelEvent) {
                PETaskDelEvent tde = (PETaskDelEvent) evt;
                if (tde.getJobId() == jobid) {
                    delEventCount++;
                    logger.info("pe task " + tde.getJobId() + "." + tde.getTaskNumber() + " deleted");
                }
            } else if (evt instanceof JobDelEvent) {
                JobDelEvent jde = (JobDelEvent) evt;
                if (jde.getJobNumber() == jobid) {
                    logger.info("got job delete event: " + jde);
                    synchronized (finishSync) {
                        finished = true;
                        finishSync.notifyAll();
                    }
                }
            }
        }
        
        public boolean waitForJobFinish(int timeout) throws InterruptedException {
            long end = System.currentTimeMillis() + (timeout * 1000);
            synchronized (finishSync) {
                while (!finished) {
                    long waittime = end - System.currentTimeMillis();
                    if (waittime < 0) {
                        logger.info("timeout while waiting for final usage event");
                        return false;
                    }
                    finishSync.wait(waittime);
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
        TestSuite suite = new TestSuite(PETaskEventTestCase.class);
        return suite;
    }
}
