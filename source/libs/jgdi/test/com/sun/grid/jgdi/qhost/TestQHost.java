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
package com.sun.grid.jgdi.qhost;

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.monitoring.HostInfo;
import com.sun.grid.jgdi.monitoring.QHostOptions;
import com.sun.grid.jgdi.monitoring.QHostResult;
import com.sun.grid.jgdi.monitoring.filter.HostFilter;
import com.sun.grid.jgdi.monitoring.filter.ResourceAttributeFilter;
import com.sun.grid.jgdi.configuration.ClusterQueue;
import com.sun.grid.jgdi.configuration.QueueInstance;
import com.sun.grid.jgdi.configuration.ExecHost;
import com.sun.grid.jgdi.monitoring.QueueInfo;
import java.util.List;
import java.util.Set;
import junit.framework.Test;
import junit.framework.TestSuite;

/**
 *
 */
public class TestQHost extends com.sun.grid.jgdi.BaseTestCase {
    
    /** Creates a new instance of TestQHost */
    public TestQHost(String testName) {
        super(testName);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(TestQHost.class);
        return suite;
    }
    
    private void printResult(QHostResult res) {
        for (String hostname : res.getHostNames()) {
            HostInfo hostInfo = res.getHostInfo(hostname);
            logger.fine("Host " + hostname + " ---------------------");
            for (String hostValueNames : hostInfo.getHostValueKeys()) {
                logger.fine("HostValue: " + hostValueNames + " = " + hostInfo.getHostValue(hostValueNames));
            }
            
            for (QueueInfo qi : hostInfo.getQueueList()) {
                logger.fine("  Queue: " + qi.getQname());
                logger.fine("  Qtype: " + qi.getQtype());
                logger.fine("  State: " + qi.getState());
                logger.fine("  total: " + qi.getTotalSlots());
                logger.fine("  used:  " + qi.getUsedSlots());
            }
            
            for (String dom : hostInfo.getDominanceSet()) {
                for (String resourceValueName : hostInfo.getResourceValueNames(dom)) {
                    logger.fine("Resource: " + dom + ": " + resourceValueName + " = " + hostInfo.getResourceValue(dom, resourceValueName));
                }
            }
        }
    }
    
    public void testAllHostValues() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            QHostOptions qhostOptions = new QHostOptions();
            
            ResourceAttributeFilter resourceAttributeFilter = new ResourceAttributeFilter();
            
            qhostOptions.setResourceAttributeFilter(resourceAttributeFilter);
            
            QHostResult res = jgdi.execQHost(qhostOptions);
            
            printResult(res);
        } finally {
            jgdi.close();
        }
    }
    
    public void testQueueFilter() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            QHostOptions qhostOptions = new QHostOptions();
            List<ExecHost> ehList = jgdi.getRealExecHostList();
            List<ClusterQueue> cqList = jgdi.getClusterQueueList();
            qhostOptions.setIncludeQueue(true);
            for (ExecHost eh : ehList) {
                HostFilter hf = new HostFilter();
                hf.addHost(eh.getName());
                qhostOptions.setHostFilter(hf);
                QHostResult res = jgdi.execQHost(qhostOptions);
                printResult(res);
                HostInfo hi = res.getHostInfo(eh.getName());
                assertNotNull(hi);
                for (ClusterQueue cq : cqList) {
                    for (QueueInstance cqi : cq.getQinstancesList()) {
                        if (cqi.getQhostname().equals(eh.getName())) {
                            boolean foundq = false;
                            for (QueueInfo qi : hi.getQueueList()) {
                                String hqname = qi.getQname();
                                // System.out.println("++ comparing cq.getName() " + cq.getName() +
                                //                      " and hqname " + hqname);
                                if (cq.getName().equals(hqname)) {
                                    //   System.out.println("-- matching cq.getName() " + cq.getName() +
                                    //                      " and hqname " + hqname);
                                    foundq = true;
                                    break;
                                }
                            }
                            assertTrue("queue " + cq.getName() + " not included for host " + eh.getName(), foundq);
                        }
                    }
                }
            }
        } finally {
            jgdi.close();
        }
    }
    
    public void testHostValueFilter() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            QHostOptions qhostOptions = new QHostOptions();
            List<ExecHost> ehList = jgdi.getRealExecHostList();
            for (ExecHost eh : ehList) {
                for (String loadValueName : eh.getLoadKeys()) {
                    String loadValue = eh.getLoad(loadValueName);
                    ResourceAttributeFilter resourceAttributeFilter = new ResourceAttributeFilter();
                    resourceAttributeFilter.addValueName(loadValueName);
                    qhostOptions.setResourceAttributeFilter(resourceAttributeFilter);
                    QHostResult res = jgdi.execQHost(qhostOptions);
                    printResult(res);
                    HostInfo hi = res.getHostInfo(eh.getName());
                    assertNotNull(hi);
                    assertNotNull("Resource Value " + loadValueName + " not found", hi.getResourceValue("hl", loadValueName));
                }
            }
        } finally {
            jgdi.close();
        }
    }
    
    public void testHostFilter() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            QHostOptions qhostOptions = new QHostOptions();
            List<ExecHost> ehList = jgdi.getRealExecHostList();
            for (ExecHost eh : ehList) {
                HostFilter hostFilter = new HostFilter();
                hostFilter.addHost(eh.getName());
                qhostOptions.setHostFilter(hostFilter);
                QHostResult res = jgdi.execQHost(qhostOptions);
                printResult(res);
                Set<String> hostNames = res.getHostNames();
                assertTrue("host " + eh.getName() + " not found", hostNames.contains(eh.getName()));
                assertTrue("host global not found", hostNames.contains("global"));
            }
        } finally {
            jgdi.close();
        }
    }
}