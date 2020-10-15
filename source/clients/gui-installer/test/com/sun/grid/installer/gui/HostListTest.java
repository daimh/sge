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

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;

public class HostListTest {
    /**
     * Gets emptied after every test
     */
    private static HostList hostList = null;

    public HostListTest() {
        hostList = new HostList();
    }

    @BeforeClass
    public static void setUpClass() throws Exception {
    }

    @AfterClass
    public static void tearDownClass() throws Exception {
        hostList.clear();
    }

    @Before
    public void setUp() {
    }

    @After
    public void tearDown() {
    }

    /**
     * Test of add method, of class HostList.
     */
    @Test
    public void testAdd() {
        Host host = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, true, true, "/execd/spool/dir", Host.State.NEW_UNKNOWN_HOST);
        assertTrue("Can not add host!", hostList.add(host));
        assertTrue("Host list remained empty!", hostList.size() == 1);
        assertSame("Loosing reference!", host, hostList.get(0));

        Host host2EqualsWithHost = new Host(Host.Type.HOSTNAME, "host", false, false, false, true, true, true, "/execd/spool/dir2", Host.State.NEW_UNKNOWN_HOST);
        assertTrue("Can not add host2!", hostList.add(host2EqualsWithHost));
        assertTrue("Duplicated host instance!", hostList.size() == 1);
        assertSame("Loosing reference!", host, hostList.get(0));
    }

    /**
     * Test of addHost method, of class HostList.
     */
    @Test
    public void testAddHost() {
        Host host = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, true, false, "/execd/spool/dir", Host.State.NEW_UNKNOWN_HOST);
        assertNotNull("Can not add host!", hostList.addHost(host));
        assertTrue("Host list remained empty!", hostList.size() == 1);
        assertSame("Loosing reference!", host, hostList.get(0));

        Host host2EqualsWithHost = new Host(Host.Type.HOSTNAME, "host", false, true, true, true, false, false, "/execd/spool/dir2", Host.State.NEW_UNKNOWN_HOST);

        assertNotNull("Can not add host2!", hostList.addHost(host2EqualsWithHost));
        assertTrue("Duplicated host instance!", hostList.size() == 1);
        assertSame("Loosing reference!", host, hostList.get(0));

        assertTrue("Host is not qmaster host!", hostList.get(0).isQmasterHost());
        assertTrue("Host is not bdb host!", hostList.get(0).isBdbHost());
        assertTrue("Host is not shadow host!", hostList.get(0).isShadowHost());
        assertTrue("Host is not execd host!", hostList.get(0).isExecutionHost());
        assertTrue("Host is not admin host!", hostList.get(0).isAdminHost());
        assertFalse("Host is submit host!", hostList.get(0).isSubmitHost());
    }

    /**
     * Test of indexOf method, of class HostList.
     */
    @Test
    public void testIndexOf() {
        Host host = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, true, true, "/execd/spool/dir", Host.State.NEW_UNKNOWN_HOST);
        Host host2 = new Host(Host.Type.HOSTNAME, "host2", false, false, false, true, true, false, "/execd/spool/dir2", Host.State.NEW_UNKNOWN_HOST);
        Host host3 = new Host(Host.Type.HOSTNAME, "host3", false, true, false, false, false, false, "/execd/spool/dir3", Host.State.NEW_UNKNOWN_HOST);
        Host host4EqualsWithHost3 = new Host(Host.Type.HOSTNAME, "host3", true, true, true, true, true, true, "/execd/spool/dir4", Host.State.NEW_UNKNOWN_HOST);

        hostList.add(host);
        hostList.add(host2);
        hostList.add(host3);

        assertEquals("Invalid index for host!", 0, hostList.indexOf(host));
        assertEquals("Invalid index for host2!", 1, hostList.indexOf(host2));
        assertEquals("Invalid index for host3!", 2, hostList.indexOf(host3));
        assertEquals("Invalid index for host4!", 2, hostList.indexOf(host4EqualsWithHost3));
    }

    /**
     * Test of get method, of class HostList.
     */
    @Test
    public void testGet_int() {
        Host host = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, true, true, "/execd/spool/dir", Host.State.NEW_UNKNOWN_HOST);
        Host host2 = new Host(Host.Type.HOSTNAME, "host2", false, false, false, true, true, false, "/execd/spool/dir2", Host.State.NEW_UNKNOWN_HOST);
        Host host3 = new Host(Host.Type.HOSTNAME, "host3", false, true, false, false, false, false, "/execd/spool/dir3", Host.State.NEW_UNKNOWN_HOST);

        hostList.add(host);
        hostList.add(host2);
        hostList.add(host3);

        assertSame("Failed to get host!", host, hostList.get(0));
        assertSame("Failed to get host2!", host2, hostList.get(1));
        assertSame("Failed to get host3!", host3, hostList.get(2));
    }

    /**
     * Test of get method, of class HostList.
     */
    @Test
    public void testGet_Host() {
        Host host = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, true, true, "/execd/spool/dir", Host.State.NEW_UNKNOWN_HOST);
        Host host2 = new Host(Host.Type.HOSTNAME, "host2", false, false, false, true, true, false, "/execd/spool/dir2", Host.State.NEW_UNKNOWN_HOST);
        Host host3 = new Host(Host.Type.HOSTNAME, "host3", false, true, false, false, false, false, "/execd/spool/dir3", Host.State.NEW_UNKNOWN_HOST);
        Host host4EqualsWithHost3 = new Host(Host.Type.HOSTNAME, "host3", true, true, true, true, true, true, "/execd/spool/dir4", Host.State.NEW_UNKNOWN_HOST);

        hostList.add(host);
        hostList.add(host2);
        hostList.add(host3);

        assertSame("Failed to get host!", host, hostList.get(host));
        assertSame("Failed to get host2!", host2, hostList.get(host2));
        assertSame("Failed to get host3!", host3, hostList.get(host3));
        assertEquals("Failed to get host4!", host3, hostList.get(host4EqualsWithHost3));
        
    }

    /**
     * Test of remove method, of class HostList.
     */
    @Test
    public void testRemove() {
        Host host = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, true, true, "/execd/spool/dir", Host.State.NEW_UNKNOWN_HOST);
        
        hostList.add(host);

        Host host2EqualsWithHost = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, false, true, "/execd/spool/dir2", Host.State.NEW_UNKNOWN_HOST);

        hostList.remove(host2EqualsWithHost);
        assertTrue("Host list is empty!", hostList.size() == 1);

        assertFalse("Host is qmaster host!", hostList.get(0).isQmasterHost());
        assertFalse("Host is bdb host!", hostList.get(0).isBdbHost());
        assertFalse("Host is shadow host!", hostList.get(0).isShadowHost());
        assertFalse("Host is execd host!", hostList.get(0).isExecutionHost());
        assertTrue("Host is not admin host!", hostList.get(0).isAdminHost());
        assertFalse("Host is submit host!", hostList.get(0).isSubmitHost());
    }

    /**
     * Test of addUnchecked method, of class HostList.
     */
    @Test
    public void testAddUnchecked() {
        Host host = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, true, true, "/execd/spool/dir", Host.State.NEW_UNKNOWN_HOST);
        hostList.add(host);

        Host host2EqualsWithHost = new Host(Host.Type.HOSTNAME, "host", false, false, false, true, false, true, "/execd/spool/dir2", Host.State.NEW_UNKNOWN_HOST);
        hostList.addUnchecked(host2EqualsWithHost);

        assertTrue("Could not add host2!", hostList.size() == 2);
    }

    /**
     * Test of removeUnchecked method, of class HostList.
     */
    @Test
    public void testRemoveUnchecked() {
        Host host = new Host(Host.Type.HOSTNAME, "host", true, false, false, true, true, true, "/execd/spool/dir", Host.State.NEW_UNKNOWN_HOST);
        hostList.add(host);

        Host host2EqualsWithHost = new Host(Host.Type.HOSTNAME, "host", false, false, false, true, false, true, "/execd/spool/dir2", Host.State.NEW_UNKNOWN_HOST);
        hostList.removeUnchecked(host2EqualsWithHost);

        assertTrue("Could not remove host!", hostList.size() == 0);
    }
}
