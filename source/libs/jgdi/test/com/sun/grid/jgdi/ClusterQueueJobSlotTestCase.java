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
package com.sun.grid.jgdi;

import com.sun.grid.jgdi.configuration.ClusterQueue;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import junit.framework.Test;
import junit.framework.TestSuite;

/**
 *
 */
public class ClusterQueueJobSlotTestCase extends BaseTestCase {
    
    private ClusterQueue testClusterQueue;
    
    /**
     * Creates a new instance of ClusterQueueJobSlotTestCase
     */
    public ClusterQueueJobSlotTestCase(String name) {
        super(name);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite( ClusterQueueJobSlotTestCase.class);
        return suite;
    }
    
    public void testUpdateJobSlots() throws Exception {
        
        JGDI jgdi = super.createJGDI();
        try {
            
            ClusterQueue testObj = jgdi.getClusterQueue("all.q");
            
            testObj.setName("new.q");
            
            jgdi.addClusterQueue(testObj);
            try {
                ClusterQueue retObj = jgdi.getClusterQueue(testObj.getName());
                
                String host = TestValueFactory.getNextHostname();
                retObj.putJobSlots(host, 10);
                
                jgdi.updateClusterQueue(retObj);
                
                ClusterQueue updatedObj = jgdi.getClusterQueue(testObj.getName());
                
                // TODO: problem with short/long hostnames as keys -> failure of test even for correct result
                //       workaround is correct hostname in Testvalues*.properties
                // System.out.println("retObj.getJobSlots(host) " + retObj.getJobSlots(host) + "(" + host + ")");
                // System.out.println("updateObj.getJobSlots(host) " + updatedObj.getJobSlots(host) + "(" + host + ")");
                // System.out.println("updateObj.getJobSlotsKeys() " + updatedObj.getJobSlotsKeys());
                assertEquals(retObj.getJobSlots(host), updatedObj.getJobSlots(host));
                
                retObj.removeJobSlots(host);
                Logger logger = Logger.getLogger("com.sun.grid.jgdi");
                Level orgLevel = logger.getLevel();
                logger.setLevel(Level.FINE);
                jgdi.updateClusterQueue(retObj);
                
                updatedObj = jgdi.getClusterQueue(testObj.getName());
                logger.setLevel(orgLevel);
                
                Set hostnames = updatedObj.getJobSlotsKeys();
                assertFalse(hostnames.contains(host));
                
            } finally {
                jgdi.deleteClusterQueue(testObj);
            }
            
        } finally {
            jgdi.close();
        }
        
        
    }
    
//   public void testWriteXML() throws Exception {
//      String[] clusterNames = getClusterNames();
//
//      for (int i=0; i<clusterNames.length;i++) {
//         JGDI gdi = createJGDI();
//         try {
//            for (ClusterQueue cq : gdi.getClusterQueueList()) {
//               logger.fine("ClusterQueue " + cq.getName() + "----------");
//               File file = File.createTempFile("cq_" + cq.getName(), ".xml" );
//               XMLUtil.write(cq, file);
//               logger.fine("cq " + cq.getName() + " -> " + file.getAbsolutePath());
//               ClusterQueue cq1 = (ClusterQueue)XMLUtil.read(file);
//               assertTrue("cq1 is not equals to cq2", cq.equalsCompletely(cq1));
//            }
//         } finally {
//            gdi.close();
//         }
//      }
//
//
//   }
//   public void testAdd() throws Exception {
//
//      JGDI gdi = createJGDI();
//      try {
//         ClassDescriptor cd = Util.getDescriptor(testClusterQueue.getClass());
//
//         cd.validate(testClusterQueue);
//
//         gdi.addClusterQueue(testClusterQueue);
//
//
//         gdi.deleteClusterQueue(testClusterQueue);
//      } finally {
//         gdi.close();
//      }
//   }
//
//   public void testAddProject() throws Exception {
//
//      JGDI gdi = createJGDI();
//      try {
//
//         Project p = new Project();
//         p.setName("testName");
//         gdi.addProject(p);
//
//         try {
//            ClusterQueue cq = gdi.getClusterQueue("all.q");
//
//
//            cq.addProjects("@allhosts", p );
//
//            List projectList = cq.getProjectsList("@allhosts");
//
//            boolean found = false;
//
//            for(int ii = 0; ii < projectList.size(); ii++) {
//
//               Project tmpPrj = (Project)projectList.get(ii);
//               logger.fine("project " + tmpPrj.getName());
//               if(tmpPrj.getName().equals(p.getName())) {
//                  found = true;
//                  break;
//               }
//            }
//
//            assertTrue("project " + p.getName() + " not found in cluster queue" + cq.getName(), found);
//
//         } finally {
//            gdi.deleteProject(p);
//         }
//      } finally {
//         gdi.close();
//      }
//
//   }
//
    
    
}
