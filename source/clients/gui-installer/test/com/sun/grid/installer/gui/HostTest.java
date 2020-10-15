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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.installer.gui;

import com.sun.grid.installer.gui.Host.State;
import com.sun.grid.installer.util.Util.SgeComponents;
import java.util.List;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;

public class HostTest {

    public HostTest() {
    }

    @BeforeClass
    public static void setUpClass() throws Exception {
    }

    @AfterClass
    public static void tearDownClass() throws Exception {
    }

    @Before
    public void setUp() {
    }

    @After
    public void tearDown() {
    }

    /**
     * Test of toStringInstance method, of class Host.
     */
    @Test
    public void testToStringInstance1() {
        Host instance = new Host(Host.Type.HOSTNAME, "grid", true, true, true, true, true, true, "/var/spooldir", State.NEW_UNKNOWN_HOST);
        String result = instance.toStringInstance();

        assertTrue("Host instance string does not start with name!", result.startsWith("grid"));
        assertTrue("Host instance string does not contain the spool directory!", result.indexOf("/var/spooldir") > -1);
        assertTrue("Host instance string does not contain 'qmaster' component!", result.indexOf(SgeComponents.qmaster.toString()) > -1);
        assertTrue("Host instance string does not contain 'execd' component!", result.indexOf(SgeComponents.execd.toString()) > -1);
        assertTrue("Host instance string does not contain 'shadow' component!", result.indexOf(SgeComponents.shadow.toString()) > -1);
        assertTrue("Host instance string does not contain 'bdb' component!", result.indexOf(SgeComponents.bdb.toString()) > -1);
        assertTrue("Host instance string does not contain 'admin' component!", result.indexOf(SgeComponents.admin.toString()) > -1);
        assertTrue("Host instance string does not contain 'submit' component!", result.indexOf(SgeComponents.submit.toString()) > -1);
    }

    @Test
    public void testToStringInstance2() {
        Host instance = new Host(Host.Type.IP, "1.1.1.1", true, false, true, false, true, true, "/var/spooldir", State.NEW_UNKNOWN_HOST);
        String result = instance.toStringInstance();

        assertTrue("Host instance string does not start with name!", result.startsWith("1.1.1.1"));
        assertTrue("Host instance string does not contain the spool directory!", result.indexOf("/var/spooldir") > -1);
        assertTrue("Host instance string does not contain 'qmaster' component!", result.indexOf(SgeComponents.qmaster.toString()) > -1);
        assertTrue("Host instance string contains 'execd' component!", result.indexOf(SgeComponents.execd.toString()) == -1);
        assertTrue("Host instance string does not contain 'shadow' component!", result.indexOf(SgeComponents.shadow.toString()) > -1);
        assertTrue("Host instance string contains 'bdb' component!", result.indexOf(SgeComponents.bdb.toString()) == -1);
        assertTrue("Host instance string does not contain 'admin' component!", result.indexOf(SgeComponents.admin.toString()) > -1);
        assertTrue("Host instance string does not contain 'submit' component!", result.indexOf(SgeComponents.submit.toString()) > -1);
    }

    /**
     * Test of fromStringInstance method, of class Host.
     */
    @Test
    public void testFromStringInstance1() {
        String instance = "grid[0-19]," + SgeComponents.shadow + "," + SgeComponents.qmaster + "," + SgeComponents.submit +",/var/spooldir";

        List<Host> result = Host.fromStringInstance(instance);

        assertTrue("Size of the result does not match!", result.size() == 20);

        for (Host host : result) {
            assertTrue("Host is not qmaster host!", host.isQmasterHost());
            assertTrue("Host is not shadow host!", host.isShadowHost());
            assertTrue("Host is not submit host!", host.isSubmitHost());

            assertTrue("Host is execd host!", !host.isExecutionHost());
            assertTrue("Host is bdb host!", !host.isBdbHost());
            assertTrue("Host is admin host!", !host.isAdminHost());
        }
    }

    @Test
    public void testFromStringInstance2() {
        String instance = "1.1.1.[0-19]";

        List<Host> result = Host.fromStringInstance(instance);

        assertTrue("Size of the result does not match!", result.size() == 20);

        for (Host host : result) {
            assertTrue("Host is qmaster host!", !host.isQmasterHost());
            assertTrue("Host is shadow host!", !host.isShadowHost());
            assertTrue("Host is submit host!", !host.isSubmitHost());
            assertTrue("Host is execd host!", !host.isExecutionHost());
            assertTrue("Host is bdb host!", !host.isBdbHost());
            assertTrue("Host is admin host!", !host.isAdminHost());
            assertTrue("Host's spool directory is not empty!", host.getSpoolDir().equals(""));
        }
    }

    public void testEquals() {
        Host host = new Host(Host.Type.HOSTNAME, "grid1", true, true, true, true, "spool/dir");
        host.setIp("1.1.1.1");
        Host host2 = new Host(Host.Type.HOSTNAME, "grid1", false, false, false, false, "spool/dir2");
        host2.setIp("1.1.1.1");
        Host host3 = new Host(Host.Type.HOSTNAME, "grid1", false, true, true, false, "spool/dir3");
        host2.setIp("1.1.1.1");

        assertTrue("Equals null!", !host.equals(null));
        assertTrue("Not reflexive!", host.equals(host));
        assertTrue("Not simmetric!", host.equals(host2) && host2.equals(host));
        assertTrue("Not transitive!", host.equals(host2) && host2.equals(host3) && host.equals(host3));
}

    public void testHashCode() {
        Host host = new Host(Host.Type.HOSTNAME, "grid1", true, true, true, true, "spool/dir");
        host.setIp("1.1.1.1");
        Host host2 = new Host(Host.Type.HOSTNAME, "grid1", false, false, false, false, "spool/dir2");
        host2.setIp("1.1.1.1");

        assertTrue("Not equals!", host.hashCode() == host2.hashCode());
    }
}
