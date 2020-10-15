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

import com.sun.grid.jgdi.monitoring.filter.QueueStateFilter;
import com.sun.grid.jgdi.monitoring.filter.ResourceFilter;
import java.util.List;
import junit.framework.Test;
import junit.framework.TestSuite;
import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.monitoring.filter.UserFilter;
import com.sun.grid.jgdi.configuration.ClusterQueue;
import com.sun.grid.jgdi.configuration.ConfigurationFactory;
import com.sun.grid.jgdi.configuration.UserSet;


/**
 *
 */
public class TestClusterQueueSummary extends com.sun.grid.jgdi.BaseTestCase {
    
    /** Creates a new instance of TestQHost */
    public TestClusterQueueSummary(String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(TestClusterQueueSummary.class);
        return suite;
    }
    
    private void printResult(List<ClusterQueueSummary> result) {
        
        logger.fine("CLUSTER QUEUE CQLOAD   USED  AVAIL  TOTAL");
        StringBuilder buf = new StringBuilder();
        for (ClusterQueueSummary cqs : result) {
            buf.setLength(0);
            buf.append(cqs.getName());
            buf.append(" ");
            buf.append(cqs.isLoadSet() ? Double.toString(cqs.getLoad()) : "-NA-");
            buf.append(" ");
            buf.append(cqs.getUsedSlots());
            buf.append(" ");
            buf.append(cqs.getAvailableSlots());
            buf.append(" ");
            buf.append(cqs.getTotalSlots());
            logger.fine(buf.toString());
        }
    }
    
    public void testSimple() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            ClusterQueueSummaryOptions options = new ClusterQueueSummaryOptions();
            List<ClusterQueueSummary> result = jgdi.getClusterQueueSummary(options);
            printResult(result);
        } finally {
            jgdi.close();
        }
    }
    
    public void testResourceFilter() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            ClusterQueueSummaryOptions options = new ClusterQueueSummaryOptions();
            ResourceFilter rsf = new ResourceFilter();
            rsf.addResource("NoAccessUsers", "lx-x86");
            options.setResourceFilter(rsf);
            List<ClusterQueueSummary> result = jgdi.getClusterQueueSummary(options);
            printResult(result);
        } finally {
            jgdi.close();
        }
    }
    
    public void testQueueStateFilter() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            ClusterQueueSummaryOptions options = new ClusterQueueSummaryOptions();
            QueueStateFilter qsf = new QueueStateFilter();
            qsf.setAlarm(true);
            options.setQueueStateFilter(qsf);
            logger.fine("queueStateFiter: " + qsf);
            List<ClusterQueueSummary> result = jgdi.getClusterQueueSummary(options);
            printResult(result);
        } finally {
            jgdi.close();
        }
    }
    
    public void testQueueUserFilter() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            ClusterQueueSummaryOptions options = new ClusterQueueSummaryOptions();
            UserSet userSet = ConfigurationFactory.createUserSet();
            userSet.setName("NoAccessUsers");
            userSet.addEntries("noaccess");
            jgdi.addUserSet(userSet);
            try {
                ClusterQueue cq = ConfigurationFactory.createClusterQueueWithDefaults();
                cq.setName("testQueueUserFilter");
                UserFilter uf = new UserFilter();
                uf.addUser("noaccess");
                jgdi.addClusterQueue(cq);
                
                try {
                    options.setQueueUserFilter(uf);
                    logger.fine("testQueueUserFilter for queue " + cq.getName());
                    List<ClusterQueueSummary> result = jgdi.getClusterQueueSummary(options);
                    printResult(result);
                } finally {
                    jgdi.deleteClusterQueue(cq);
                }
            } finally {
                jgdi.deleteUserSet(userSet);
            }
        } finally {
            jgdi.close();
        }
    }
}
