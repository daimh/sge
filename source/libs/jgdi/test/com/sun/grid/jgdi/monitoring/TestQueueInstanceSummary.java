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
package com.sun.grid.jgdi.monitoring;

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.configuration.QueueInstance;
import com.sun.grid.jgdi.monitoring.filter.QueueFilter;
import com.sun.grid.jgdi.monitoring.filter.QueueStateFilter;
import com.sun.grid.jgdi.monitoring.filter.ResourceAttributeFilter;
import java.io.PrintWriter;
import java.util.List;
import java.util.logging.Level;
import junit.framework.Test;
import junit.framework.TestSuite;

/**
 *
 */
public class TestQueueInstanceSummary extends com.sun.grid.jgdi.BaseTestCase {
    
    public TestQueueInstanceSummary(String testName) {
        super(testName);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(TestQueueInstanceSummary.class);
        return suite;
    }
    
    public void testFullOutput() throws Exception {
        JGDI jgdi = createJGDI();
        try {
            QueueInstanceSummaryOptions options = new QueueInstanceSummaryOptions();
            options.setShowFullOutput(true);
            QueueInstanceSummaryResult result = jgdi.getQueueInstanceSummary(options);
            if (logger.isLoggable(Level.FINE)) {
                PrintWriter pw = new PrintWriter(System.out);
                QueueInstanceSummaryPrinter.print(pw, result, options);
                pw.flush();
            }
        } finally {
            jgdi.close();
        }
    }
    
    public void testAllResourceAttributes() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            QueueInstanceSummaryOptions options = new QueueInstanceSummaryOptions();
            ResourceAttributeFilter raf = new ResourceAttributeFilter();
            options.setResourceAttributeFilter(raf);
            QueueInstanceSummaryResult result = jgdi.getQueueInstanceSummary(options);
            if (logger.isLoggable(Level.FINE)) {
                PrintWriter pw = new PrintWriter(System.out);
                QueueInstanceSummaryPrinter.print(pw, result, options);
                pw.flush();
            }
        } finally {
            jgdi.close();
        }
    }
    
    public void testQueueStateFilterSuspended() throws Exception {
        
        QueueInstanceSummaryOptions options = new QueueInstanceSummaryOptions();
        options.setShowFullOutput(true);
        JGDI jgdi = createJGDI();
        try {
            QueueInstanceSummaryResult result = jgdi.getQueueInstanceSummary(options);
            List<QueueInstanceSummary> qiList = result.getQueueInstanceSummary();
            QueueStateFilter qsf = new QueueStateFilter();
            qsf.setSuspended(true);
            options.setQueueStateFilter(qsf);
            options.setShowFullOutput(true);
            for (QueueInstanceSummary qis : qiList) {
                logger.fine("Suspend queue " + qis.getName());
                jgdi.suspendQueues(new String[]{qis.getName()}, false);
                result = jgdi.getQueueInstanceSummary(options);
                // if (logger.isLoggable(Level.FINE)) {
                PrintWriter pw = new PrintWriter(System.out);
                QueueInstanceSummaryPrinter.print(pw, result, options);
                pw.flush();
                // }
                boolean found = false;
                for (QueueInstanceSummary susQis : result.getQueueInstanceSummary()) {
                    pw.println("susQis.getName() = " + susQis.getName() + " state = " + susQis.getState());
                    assertTrue("expect state 's' for queue instance " + susQis.getName(), (susQis.getState().indexOf('s') >= 0));
                    if (qis.getName().equals(susQis.getName())) {
                        found = true;
                    }
                }
                assertTrue("suspended queue instance " + qis.getName() + " is not in result", found);
                logger.fine("Unsuspend queue " + qis.getName());
                jgdi.unsuspendQueues(new String[]{qis.getName()}, false);
            }
        } finally {
            jgdi.close();
        }
    }
    
    public void testQueueStateFilterDisabled() throws Exception {
        
        QueueInstanceSummaryOptions options = new QueueInstanceSummaryOptions();
        options.setShowFullOutput(true);
        QueueStateFilter qsf = new QueueStateFilter();
        qsf.setDisabled(true);
        options.setQueueStateFilter(qsf);
        JGDI jgdi = createJGDI();
        try {
            QueueInstanceSummaryResult result = jgdi.getQueueInstanceSummary(options);
            List<QueueInstanceSummary> qiList = result.getQueueInstanceSummary();
            // Disable all queues
            jgdi.disableQueues(new String[]{"*"}, false);
            result = jgdi.getQueueInstanceSummary(options);
            List<QueueInstanceSummary> newQiList = result.getQueueInstanceSummary();
            for (QueueInstanceSummary qis : qiList) {
                boolean found = false;
                for (QueueInstanceSummary qis1 : newQiList) {
                    if (qis.getName().equals(qis1.getName())) {
                        found = true;
                        break;
                    }
                }
                assertTrue("queue instance " + qis.getName() + " has not been found in disabled queue list", found);
            }
            jgdi.enableQueues(new String[]{"*"}, true);
            result = jgdi.getQueueInstanceSummary(options);
            newQiList = result.getQueueInstanceSummary();
            if (logger.isLoggable(Level.FINE)) {
                PrintWriter pw = new PrintWriter(System.out);
                QueueInstanceSummaryPrinter.print(pw, result, options);
                pw.flush();
            }
            assertEquals("got queue instances, but all queues are disabled", 0, newQiList.size());
        } finally {
            jgdi.close();
        }
    }
    
    public void testQueueFilter() throws Exception {
        
        QueueInstanceSummaryOptions options = new QueueInstanceSummaryOptions();
        options.setShowFullOutput(true);
        
        JGDI jgdi = createJGDI();
        
        try {
            QueueInstanceSummaryResult result = jgdi.getQueueInstanceSummary(options);
            List<QueueInstanceSummary> qiList = result.getQueueInstanceSummary();
            
            for (QueueInstanceSummary qis : qiList) {
                logger.fine("Filter for queue instance " + qis.getName());
                QueueFilter qf = new QueueFilter();
                qf.addQueue(qis.getName());
                options.setQueueFilter(qf);
                result = jgdi.getQueueInstanceSummary(options);
                if (logger.isLoggable(Level.FINE)) {
                    PrintWriter pw = new PrintWriter(System.out);
                    QueueInstanceSummaryPrinter.print(pw, result, options);
                    pw.flush();
                }
                
                List<QueueInstanceSummary> newQiList = result.getQueueInstanceSummary();
                assertTrue("Missing queue instance " + qis.getName(), !newQiList.isEmpty());
                for (QueueInstanceSummary newQis : newQiList) {
                    assertEquals("Queue instance " + newQis.getName() + " is not in queue filter", qis.getName(), newQis.getName());
                }
            }
        } finally {
            jgdi.close();
        }
    }
}