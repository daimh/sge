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
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.configuration.UserSet;
import com.sun.grid.jgdi.configuration.UserSetImpl;
import java.util.Arrays;
import com.sun.grid.jgdi.configuration.*;

/**
 *
 */
public class JGDIGEObjectEditorTest extends BaseTestCase {
    private JGDI jgdi;
    
    private ClusterQueue cq1;
    private Project prj1, prj2, prj3, prj4;
    private UserSet us1, us2, us3;
    private ParallelEnvironmentImpl pe1, pe2;
    private HostgroupImpl hg1, hg2;
    
    public JGDIGEObjectEditorTest(String testName) {
        super(testName);
    }
    
    protected synchronized void setUp() throws Exception {
        super.setUp();
        jgdi = createJGDI();
        init();
    }
    
    private void init() throws JGDIException {
        ClusterQueue cq;
        cq1 = new ClusterQueueImpl(true);
        cq1.setName("testQueue_1");
        cq1.addHostlist("@allhosts");
        
        if ((cq = jgdi.getClusterQueue(cq1.getName())) != null) {
            jgdi.deleteClusterQueue(cq);
        }
        jgdi.addClusterQueue(cq1);
        
        Project prj;
        prj1 = new ProjectImpl("newProjectName");
        prj2 = new ProjectImpl("project2");
        prj3 = new ProjectImpl("project3");
        prj4 = new ProjectImpl("project4");
        if ((prj = jgdi.getProject("ProjectName")) != null) {
            jgdi.deleteProject(prj);
        }
        if ((prj = jgdi.getProject(prj1.getName())) != null) {
            jgdi.deleteProject(prj);
        }
        if ((prj = jgdi.getProject(prj2.getName())) != null) {
            jgdi.deleteProject(prj);
        }
        if ((prj = jgdi.getProject(prj3.getName())) != null) {
            jgdi.deleteProject(prj);
        }
        if ((prj = jgdi.getProject(prj4.getName())) != null) {
            jgdi.deleteProject(prj);
        }
        
        Hostgroup hg;
        hg1 = new HostgroupImpl("@hgroup1");
        if ((hg = jgdi.getHostgroup(hg1.getName())) != null) {
            jgdi.deleteHostgroup(hg);
        }
        
        UserSet us;
        us1 = new UserSetImpl("user1");
        us2 = new UserSetImpl("user2");
        us3 = new UserSetImpl("user3");
        if ((us = jgdi.getUserSet("user1")) != null) {
            jgdi.deleteUserSet(us);
        }
        if ((us = jgdi.getUserSet("user2")) != null) {
            jgdi.deleteUserSet(us);
        }
        if ((us = jgdi.getUserSet("user3")) != null) {
            jgdi.deleteUserSet(us);
        }
        
        ParallelEnvironment pe;
        pe1 = new ParallelEnvironmentImpl(true);
        pe1.setName("oldPe");
        pe2 = new ParallelEnvironmentImpl(true);
        pe2.setName("myPe");
        if ((pe = jgdi.getParallelEnvironment(pe1.getName())) != null) {
            jgdi.deleteParallelEnvironment(pe);
        }
        if ((pe = jgdi.getParallelEnvironment(pe2.getName())) != null) {
            jgdi.deleteParallelEnvironment(pe);
        }
        
        jgdi.addProject(prj1);
        jgdi.addProject(prj2);
        jgdi.addProject(prj3);
        jgdi.addProject(prj4);
        jgdi.addHostgroup(hg1);
        jgdi.addUserSet(us1);
        jgdi.addUserSet(us2);
        jgdi.addUserSet(us3);
        jgdi.addParallelEnvironment(pe1);
        jgdi.addParallelEnvironment(pe2);
    }
    
    protected void tearDown() throws Exception {
        jgdi.close();
    }
    
    public void testUpdateObjectWithText_SetProjectAcl() throws Exception {
        System.out.println("testUpdateObjectWithText_SetProjectAcl");
        GEObjectEditor.updateObjectWithText(jgdi, prj1, "acl user1 user2 user3");
        jgdi.updateProject(prj1);
        assertEquals(Arrays.asList(new Object[] {new UserSetImpl("user1"), new UserSetImpl("user2"), new UserSetImpl("user3")}), jgdi.getProject(prj1.getName()).getAclList());
    }
    
    public void testUpdateObjectWithText_SetProjectAll1() throws Exception {
        Project newProject = new ProjectImpl(true);
        System.out.println("testUpdateObjectWithText_SetProjectAll1");
        String text = "name ProjectName\n oticket 1566\n fshare 666 \n acl user1 user2 \n xacl user3";
        GEObjectEditor.updateObjectWithText(jgdi, newProject, text);
        jgdi.addProject(newProject);
        assertEquals("ProjectName", jgdi.getProject(newProject.getName()).getName());
        assertEquals(1566 ,jgdi.getProject(newProject.getName()).getOticket());
        assertEquals(666 ,jgdi.getProject(newProject.getName()).getFshare());
        assertEquals(2, jgdi.getProject(newProject.getName()).getAclCount());
        assertTrue(jgdi.getProject(newProject.getName()).getAcl(0).equalsCompletely(new UserImpl("user1")));
        assertTrue(jgdi.getProject(newProject.getName()).getAcl(1).equalsCompletely(new UserImpl("user2")));
        assertEquals(1, jgdi.getProject(newProject.getName()).getXaclCount());
        assertTrue(jgdi.getProject(newProject.getName()).getXacl(0).equalsCompletely(new UserImpl("user3")));
        assertEquals(Arrays.asList(new Object[] {new UserSetImpl("user1"), new UserSetImpl("user2")}), jgdi.getProject(newProject.getName()).getAclList());
        assertEquals(Arrays.asList(new Object[] {new UserSetImpl("user3")}), jgdi.getProject(newProject.getName()).getXaclList());
        jgdi.deleteProject(newProject);
    }
    
    public void testUpdateObjectWithText_FailUpdateClusterQueueName() {
        System.out.print("testUpdateObjectWithText_FailUpdateClusterQueueName");
        try {
            GEObjectEditor.updateObjectWithText(jgdi, cq1, "name newQueueName");
            jgdi.updateClusterQueue(cq1);
            fail();
        } catch (JGDIException ex) {
            System.out.print(" - " + ex.getMessage());
        }
    }
    
    //MapList with NONE
    public void testUpdateObjectWithText_SetClusterQueuePeWithNone() throws Exception {
        System.out.println("testUpdateObjectWithText_SetClusterQueuePeWithNone");
        GEObjectEditor.updateObjectWithText(jgdi, cq1, "pe_list  NONE,[@hgroup1=oldPe myPe]");
        jgdi.updateClusterQueue(cq1);
        
        assertEquals(0, jgdi.getClusterQueue(cq1.getName()).getPeCount("@/"));
        
        assertEquals(2, jgdi.getClusterQueue(cq1.getName()).getPeCount("@hgroup1"));
        assertEquals("oldPe", jgdi.getClusterQueue(cq1.getName()).getPe("@hgroup1",0));
        assertEquals("myPe", jgdi.getClusterQueue(cq1.getName()).getPe("@hgroup1",1));
    }
    //Map with NONE
    public void testUpdateObjectWithText_SetClusterQueuePrologWithNone() throws Exception {
        System.out.println("testUpdateObjectWithText_SetClusterQueuePrologWithNone");
        GEObjectEditor.updateObjectWithText(jgdi, cq1, "prolog                secret,[@hgroup1=/bin/sh],[host_to_be_added=NONE],[unknown1=/bin/tcsh],[new5=unknown]");
        jgdi.updateClusterQueue(cq1);
        assertEquals("secret", jgdi.getClusterQueue(cq1.getName()).getProlog("@/"));
        assertEquals("/bin/sh", jgdi.getClusterQueue(cq1.getName()).getProlog("@hgroup1"));
        assertEquals("NONE", jgdi.getClusterQueue(cq1.getName()).getProlog("host_to_be_added"));
        assertEquals("/bin/tcsh", jgdi.getClusterQueue(cq1.getName()).getProlog("unknown1"));
        assertEquals("unknown", jgdi.getClusterQueue(cq1.getName()).getProlog("new5"));
    }
    
    public void testUpdateObjectWithText_SetClusterQueueAll_1() throws Exception {
        System.out.println("testUpdateObjectWithText_SetClusterQueueAll_1");
        GEObjectEditor.updateObjectWithText(jgdi, cq1, "name "+cq1.getName()+"\n"+
                "projects "+prj1.getName()+" "+prj2.getName()+
                ",["+hg1.getName()+"="+prj1.getName()+" "+prj4.getName()+
                "],[host1="+prj3.getName()+" "+prj1.getName()+" "+prj4.getName()+"]\n" +
                " pe   make,["+hg1.getName()+"="+pe1.getName()+" "+pe2.getName()+"]\n"+
                " load_thresholds np_load_avg=1.75,qname="+cq1.getName()+",rerun=true,s_rss=3k,h_rss=NONE, \\\n" +"["+hg1.getName()+"=qname="+cq1.getName()+",swap_free=1G]\n");
        jgdi.updateClusterQueue(cq1);
        
        assertEquals("testQueue_1", jgdi.getClusterQueue(cq1.getName()).getName());
        //PROJECT
        assertEquals(2,jgdi.getClusterQueue(cq1.getName()).getProjectsCount("@/"));
        assertEquals(prj1.getName(), jgdi.getClusterQueue(cq1.getName()).getProjects("@/",0).getName());
        assertEquals(prj2.getName(), jgdi.getClusterQueue(cq1.getName()).getProjects("@/",1).getName());
        assertTrue(jgdi.getClusterQueue(cq1.getName()).getProjects("@/",0).equalsCompletely(new ProjectImpl(prj1.getName())));
        assertTrue(jgdi.getClusterQueue(cq1.getName()).getProjects("@/",1).equalsCompletely(new ProjectImpl(prj2.getName())));
        
        assertEquals(2,jgdi.getClusterQueue(cq1.getName()).getProjectsCount(hg1.getName()));
        assertEquals(prj1.getName(), jgdi.getClusterQueue(cq1.getName()).getProjects(hg1.getName(),0).getName());
        assertEquals(prj4.getName(), jgdi.getClusterQueue(cq1.getName()).getProjects(hg1.getName(),1).getName());
        assertTrue(jgdi.getClusterQueue(cq1.getName()).getProjects(hg1.getName(),0).equalsCompletely(new ProjectImpl(prj1.getName())));
        assertTrue(jgdi.getClusterQueue(cq1.getName()).getProjects(hg1.getName(),1).equalsCompletely(new ProjectImpl(prj4.getName())));
        
        assertEquals(3,jgdi.getClusterQueue(cq1.getName()).getProjectsCount("host1"));
        assertEquals(prj3.getName(), jgdi.getClusterQueue(cq1.getName()).getProjects("host1",0).getName());
        assertEquals(prj1.getName(), jgdi.getClusterQueue(cq1.getName()).getProjects("host1",1).getName());
        assertEquals(prj4.getName(), jgdi.getClusterQueue(cq1.getName()).getProjects("host1",2).getName());
        assertTrue(jgdi.getClusterQueue(cq1.getName()).getProjects("host1",0).equalsCompletely(new ProjectImpl(prj3.getName())));
        assertTrue(jgdi.getClusterQueue(cq1.getName()).getProjects("host1",1).equalsCompletely(new ProjectImpl(prj1.getName())));
        assertTrue(jgdi.getClusterQueue(cq1.getName()).getProjects("host1",2).equalsCompletely(new ProjectImpl(prj4.getName())));
        
        //PE
        assertEquals(1,jgdi.getClusterQueue(cq1.getName()).getPeCount("@/"));
        assertEquals("make", jgdi.getClusterQueue(cq1.getName()).getPe("@/",0));
        
        assertEquals(2,jgdi.getClusterQueue(cq1.getName()).getPeCount(hg1.getName()));
        assertEquals(pe1.getName(), jgdi.getClusterQueue(cq1.getName()).getPe(hg1.getName(),0));
        assertEquals(pe2.getName(), jgdi.getClusterQueue(cq1.getName()).getPe(hg1.getName(),1));
        
        //Load thresholds
        assertEquals(5, jgdi.getClusterQueue(cq1.getName()).getLoadThresholdsCount("@/"));
        assertEquals("np_load_avg",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",0).getName());
        assertEquals("1.75",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",0).getStringval());
        
        assertEquals("qname",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",1).getName());
        assertEquals(cq1.getName(),jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",1).getStringval());
        
        assertEquals("rerun",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",2).getName());
        assertEquals("true",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",2).getStringval());
        
        assertEquals("s_rss",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",3).getName());
        assertEquals("3k",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",3).getStringval());
        
        assertEquals("h_rss",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",4).getName());
        assertEquals("NONE",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds("@/",4).getStringval());
        
        assertEquals(2, jgdi.getClusterQueue(cq1.getName()).getLoadThresholdsCount(hg1.getName()));
        assertEquals("qname",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds(hg1.getName(),0).getName());
        assertEquals(cq1.getName(),jgdi.getClusterQueue(cq1.getName()).getLoadThresholds(hg1.getName(),0).getStringval());
        
        assertEquals("swap_free",jgdi.getClusterQueue(cq1.getName()).getLoadThresholds(hg1.getName(),1).getName());
        assertEquals("1G", jgdi.getClusterQueue(cq1.getName()).getLoadThresholds(hg1.getName(),1).getStringval());
    }
    
    public static void main(String[] args) throws Exception {
        GEObjectEditorTest g = new GEObjectEditorTest("testClusterQueue");
        g.setUp();
        g.tearDown();
    }
}
