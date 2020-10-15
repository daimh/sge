//*___INFO__MARK_BEGIN__*/
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

package com.sun.grid.jgdi.util.shell.editor;

import com.sun.grid.jgdi.BaseTestCase;
import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.configuration.ClusterQueue;
import com.sun.grid.jgdi.configuration.ClusterQueueImpl;
import com.sun.grid.jgdi.configuration.ParallelEnvironment;
import com.sun.grid.jgdi.configuration.ParallelEnvironmentImpl;
import java.util.List;

/**
 *
 */
public class JGDIDoesNotCrashQMasterTest extends BaseTestCase {
    
    private JGDI jgdi;
    private ClusterQueue cq;
    
    public JGDIDoesNotCrashQMasterTest(String testName) {
        super(testName);
    }
    
    protected synchronized void setUp() throws Exception {
        super.setUp();
        jgdi = createJGDI();
        
        cq = new ClusterQueueImpl(true);
        cq.setName("crashQueue");
        cq.addHostlist("@allhosts");
        ClusterQueue oldQueue;
        if ((oldQueue = jgdi.getClusterQueue(cq.getName())) != null) {
            jgdi.deleteClusterQueue(oldQueue);
        }
        jgdi.addClusterQueue(cq);
        
        ParallelEnvironment pe1 = new ParallelEnvironmentImpl(true);
        pe1.setName("one");
        ParallelEnvironment pe2 = new ParallelEnvironmentImpl(true);
        pe2.setName("two");
        ParallelEnvironment pe3 = new ParallelEnvironmentImpl(true);
        pe3.setName("three");
        ParallelEnvironment pe;
        if ((pe = jgdi.getParallelEnvironment("one")) != null) {
            jgdi.deleteParallelEnvironment(pe);
        }
        jgdi.addParallelEnvironment(pe1);
        if ((pe = jgdi.getParallelEnvironment("two")) != null) {
            jgdi.deleteParallelEnvironment(pe);
        }
        jgdi.addParallelEnvironment(pe2);
        if ((pe = jgdi.getParallelEnvironment("three")) != null) {
            jgdi.deleteParallelEnvironment(pe);
        }
        jgdi.addParallelEnvironment(pe3);
        List pes = jgdi.getParallelEnvironmentList();
    }
    
    protected void tearDown() throws Exception {
        jgdi.close();
    }
    
    public void testCrashQmaster() throws Exception {
        System.out.print("testCrashQmaster - ");
        cq.removeAllPe();
        cq.addPe("unknown", "two");
        cq.addPe("unknown", "three");
        try {
            jgdi.updateClusterQueue(cq);
            System.out.println("FAILED");
            fail("Should not have succeeded");
        } catch (Exception ex) {
            String msg = ex.getMessage();
            if (msg.equals("\"pe_list\" has no default value" + System.getProperty("line.separator"))) {
                System.out.println("OK");
                return;
            }
            System.out.println("Qmaster might have crashed. Message was: " + ex.getMessage());
            fail("Qmaster might have crashed. Message was: " + ex.getMessage());
        }
    }
    
    public void testCrashQmaster2() throws Exception {
        System.out.print("testCrashQmaster2 - ");
        cq.removeAllPe();
        cq.addPe("@/", "one");
        cq.addPe("unknown", "two");
        cq.addPe("unknown", "three");
        try {
            jgdi.updateClusterQueue(cq);
            System.out.println("OK");
        } catch (Exception ex) {
            System.out.println("FAILED: " + ex.getMessage());
            fail("Message was: " + ex.getMessage());
        }
    }
}
