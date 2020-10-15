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
 *   Copyright: 2009 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

package com.sun.grid.herd;

import com.sun.grid.jsv.JsvManager;
import java.lang.reflect.Method;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import junit.framework.TestCase;
import org.apache.hadoop.hdfs.protocol.Block;
import org.apache.hadoop.hdfs.protocol.DatanodeID;
import org.apache.hadoop.hdfs.protocol.DatanodeInfo;
import org.apache.hadoop.hdfs.protocol.LocatedBlock;

/**
 *
 */
public class HerdJsvTest extends TestCase {
    public HerdJsvTest(String testName) {
        super(testName);
    }

    /**
     * Test of onStart method, of class HerdJsv.
     */
    public void testOnStart() {
        System.out.println("onStart()");
        JsvManager jsv = new JsvManager();
        HerdJsv instance = new HerdJsv();

        // This is really just a smoke test.  The onStart() method doesn't
        // actually do anything.
        instance.onStart(jsv);
    }

    /**
     * Test of buildBlockRequests() method, of class HerdJsv.
     */
    public void testBuildBlockRequests() throws Exception {
        System.out.println("buildBlockRequests()");
        HerdJsv instance = new HerdJsv();
        List<LocatedBlock> blocks = new ArrayList<LocatedBlock>(5);
        Map<String,String> expectedResult = new HashMap<String, String>();
        Map result = null;
        long id1 = 0x1234567890abcdefL;
        long id2 = 0x0123456789abcdefL;
        long id3 = 0xfedcba0987654321L;
        long id4 = 0xfedcba9876543210L;
        long id5 = 0x0L;

        blocks.add(new LocatedBlock(new Block(id1), null));
        blocks.add(new LocatedBlock(new Block(id2), null));
        blocks.add(new LocatedBlock(new Block(id3), null));
        blocks.add(new LocatedBlock(new Block(id4), null));
        blocks.add(new LocatedBlock(new Block(id5), null));
        expectedResult.put("hdfs_B12", "*34567890abcdef*");
        expectedResult.put("hdfs_B01", "*23456789abcdef*");
        expectedResult.put("hdfs_Bfe", "*dcba0987654321*dcba9876543210*");
        expectedResult.put("hdfs_B00", "*00000000000000*");

        result = (Map)callPrivateMethod(instance, "buildBlockRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildBlockRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(new Block(id3), null));
        blocks.add(new LocatedBlock(new Block(id4), null));
        expectedResult.clear();
        expectedResult.put("hdfs_Bfe", "*dcba0987654321*dcba9876543210*");

        result = (Map)callPrivateMethod(instance, "buildBlockRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildBlockRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(new Block(id3), null));
        expectedResult.clear();
        expectedResult.put("hdfs_Bfe", "*dcba0987654321*");

        result = (Map)callPrivateMethod(instance, "buildBlockRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildBlockRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, null));
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildBlockRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildBlockRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildBlockRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildBlockRequests() method did not build the expected requests: ", expectedResult, result);

        blocks = null;
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildBlockRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildBlockRequests() method did not build the expected requests: ", expectedResult, result);
    }

    /**
     * Test of toIdString() method, of class HerdJsv.
     */
    public void testToIdString() throws Exception {
        System.out.println("toIdString()");
        HerdJsv instance = new HerdJsv();
        String expectedResult = "1234567890abcdef";
        String result = null;

        result = (String)callPrivateMethod(instance, "toIdString", new Class[] {Long.TYPE}, new Object[] {Long.parseLong(expectedResult, 16)});
        assertEquals("The toIdString() method did not return the correct id", expectedResult, result);

        expectedResult = "fedcba9876543210";
        BigInteger interrum = new BigInteger(expectedResult, 16);
        interrum.subtract(new BigInteger("10000000000000000", 16));
        result = (String)callPrivateMethod(instance, "toIdString", new Class[] {Long.TYPE}, new Object[] {new Long(interrum.longValue())});
        assertEquals("The toIdString() method did not return the correct id", expectedResult, result);

        expectedResult = "0123456789abcdef";
        result = (String)callPrivateMethod(instance, "toIdString", new Class[] {Long.TYPE}, new Object[] {Long.parseLong(expectedResult, 16)});
        assertEquals("The toIdString() method did not return the correct id", expectedResult, result);

        expectedResult = "0000000000000001";
        result = (String)callPrivateMethod(instance, "toIdString", new Class[] {Long.TYPE}, new Object[] {Long.parseLong(expectedResult, 16)});
        assertEquals("The toIdString() method did not return the correct id", expectedResult, result);
    }

    /**
     * Test of buildRackRequests() method, of class HerdJsv.
     */
    public void testBuildRackRequests() throws Exception {
        System.out.println("buildRackRequests()");
        HerdJsv instance = new HerdJsv();
        List<LocatedBlock> blocks = new ArrayList<LocatedBlock>(5);
        Map<String,String> expectedResult = new HashMap<String, String>();
        Map result = null;
        DatanodeInfo node1 = new DatanodeInfo(new DatanodeID("node1"));
        DatanodeInfo node2 = new DatanodeInfo(new DatanodeID("node2a"));
        DatanodeInfo node3 = new DatanodeInfo(new DatanodeID("node2b"));
        DatanodeInfo node4 = new DatanodeInfo(new DatanodeID("node4"));
        DatanodeInfo node5 = new DatanodeInfo(new DatanodeID("node5"));

        node1.setNetworkLocation("/rack1");
        node2.setNetworkLocation("/rack2a");
        node3.setNetworkLocation("/rack2b");
        node4.setNetworkLocation("/rack4");
        node5.setNetworkLocation("/rack5");

        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node2, node5}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node2, node3}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node2, node3, node4}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node4, node5}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node3, node5}));
        expectedResult.put("hdfs_R", "/rack1");
        expectedResult.put("hdfs_r", "/rack1|/rack2a|/rack2b|/rack5|/rack4");

        result = (Map)callPrivateMethod(instance, "buildRackRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildRackRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node2, node5}));
        expectedResult.put("hdfs_R", "/rack1");
        expectedResult.put("hdfs_r", "/rack1|/rack2a|/rack5");

        result = (Map)callPrivateMethod(instance, "buildRackRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildRackRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node3}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node5}));
        expectedResult.put("hdfs_R", "/rack1");
        expectedResult.put("hdfs_r", "/rack1|/rack2b|/rack5");

        result = (Map)callPrivateMethod(instance, "buildRackRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildRackRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1}));
        expectedResult.put("hdfs_R", "/rack1");
        expectedResult.put("hdfs_r", "/rack1");

        result = (Map)callPrivateMethod(instance, "buildRackRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildRackRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {}));
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildRackRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildRackRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, null));
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildRackRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildRackRequests() method did not build the expected requests: ", expectedResult, result);

        blocks.clear();
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildRackRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildRackRequests() method did not build the expected requests: ", expectedResult, result);

        blocks = null;
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildRackRequests", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The buildRackRequests() method did not build the expected requests: ", expectedResult, result);
    }

    /**
     * Test of collateRacks() method, of class HerdJsv.
     */
    public void testCollateRacks() throws Exception {
        System.out.println("collateRacks()");
        HerdJsv instance = new HerdJsv();
        List<LocatedBlock> blocks = new ArrayList<LocatedBlock>(5);
        Map<String,Integer> expectedResult = new HashMap<String, Integer>();
        Map result = null;
        DatanodeInfo node1 = new DatanodeInfo(new DatanodeID("node1"));
        DatanodeInfo node2 = new DatanodeInfo(new DatanodeID("node2"));
        DatanodeInfo node3 = new DatanodeInfo(new DatanodeID("node3"));
        DatanodeInfo node4 = new DatanodeInfo(new DatanodeID("node4"));
        DatanodeInfo node5 = new DatanodeInfo(new DatanodeID("node5"));

        node1.setNetworkLocation("/rack1");
        node2.setNetworkLocation("/rack2");
        node3.setNetworkLocation("/rack3");
        node4.setNetworkLocation("/rack4");
        node5.setNetworkLocation("/rack5");

        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node2, node5}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node2, node3}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node2, node3, node4}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node4, node5}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node3, node5}));
        expectedResult.put("/rack1", 4);
        expectedResult.put("/rack2", 3);
        expectedResult.put("/rack3", 3);
        expectedResult.put("/rack4", 2);
        expectedResult.put("/rack5", 3);

        result = (Map)callPrivateMethod(instance, "collateRacks", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The collateRacks() method did not count the racks correctly", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1, node3, node5}));
        expectedResult.clear();
        expectedResult.put("/rack1", 1);
        expectedResult.put("/rack3", 1);
        expectedResult.put("/rack5", 1);

        result = (Map)callPrivateMethod(instance, "collateRacks", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The collateRacks() method did not count the racks correctly", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node3}));
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node5}));
        expectedResult.clear();
        expectedResult.put("/rack1", 1);
        expectedResult.put("/rack3", 1);
        expectedResult.put("/rack5", 1);

        result = (Map)callPrivateMethod(instance, "collateRacks", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The collateRacks() method did not count the racks correctly", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {node1}));
        expectedResult.clear();
        expectedResult.put("/rack1", 1);

        result = (Map)callPrivateMethod(instance, "collateRacks", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The collateRacks() method did not count the racks correctly", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, new DatanodeInfo[] {}));
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "collateRacks", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The collateRacks() method did not count the racks correctly", expectedResult, result);

        blocks.clear();
        blocks.add(new LocatedBlock(null, null));
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "collateRacks", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The collateRacks() method did not count the racks correctly", expectedResult, result);

        blocks.clear();
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "collateRacks", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The collateRacks() method did not count the racks correctly", expectedResult, result);

        blocks = null;
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "collateRacks", new Class[] {Collection.class}, new Object[] {blocks});
        assertEquals("The collateRacks() method did not count the racks correctly", expectedResult, result);
    }

    /**
     * Test of getSortedRacks() method, of class HerdJsv.
     */
    public void testGetSortedRacks() throws Exception {
        System.out.println("getSortedRacks()");
        HerdJsv instance = new HerdJsv();
        Map<String,Integer> racks = new HashMap<String, Integer>();
        List<String> expectedResult = new ArrayList<String>(5);
        List result = null;

        racks.put("rack4", 4);
        racks.put("rack2a", 2);
        racks.put("rack3", 3);
        racks.put("rack2b", 2);
        racks.put("rack1", 1);
        expectedResult.add("rack4");
        expectedResult.add("rack3");
        // Yes, these are intentionally in this order
        expectedResult.add("rack2a");
        expectedResult.add("rack2b");
        expectedResult.add("rack1");

        result = (List)callPrivateMethod(instance, "getSortedRacks", new Class[] {Map.class}, new Object[] {racks});
        assertEquals("The getSortedRacks() method did not sort the rack list correctly: ", expectedResult, result);

        racks.clear();
        racks.put("rack1", 1);
        expectedResult.clear();
        expectedResult.add("rack1");

        result = (List)callPrivateMethod(instance, "getSortedRacks", new Class[] {Map.class}, new Object[] {racks});
        assertEquals("The getSortedRacks() method did not handle a single-element rack list correctly: ", expectedResult, result);

        racks.clear();
        expectedResult.clear();

        result = (List)callPrivateMethod(instance, "getSortedRacks", new Class[] {Map.class}, new Object[] {racks});
        assertEquals("The getSortedRacks() method did not handle a no-element rack list correctly: ", expectedResult, result);

        racks = null;
        expectedResult.clear();

        result = (List)callPrivateMethod(instance, "getSortedRacks", new Class[] {Map.class}, new Object[] {racks});
        assertEquals("The getSortedRacks() method did not handle a no-element rack list correctly: ", expectedResult, result);
    }


    /**
     * Test of verifyPath() method, of class HerdJsv.
     */
    public void testVerifyPath() throws Exception {
        System.out.println("verifyPath()");
        HerdJsv instance = new HerdJsv();
        boolean result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {"/valid/path"});

        assertTrue("The verifyPath() method rejected a valid path", result);

        result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {"/also/valid/path/"});
        assertTrue("The verifyPath() method rejected a valid path", result);

        result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {"/shortpath"});
        assertTrue("The verifyPath() method rejected a valid path", result);

        result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {"/"});
        assertTrue("The verifyPath() method rejected a valid path", result);

        result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {"invalid/path"});
        assertFalse("The verifyPath() method rejected a valid path", result);

        result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {"./also/invalid/path/"});
        assertFalse("The verifyPath() method rejected a valid path", result);

        result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {"hdfs:///also/invalid/path"});
        assertFalse("The verifyPath() method rejected a valid path", result);

        result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {""});
        assertFalse("The verifyPath() method rejected a valid path", result);

        result = (Boolean)callPrivateMethod(instance, "verifyPath", new Class[] {String.class}, new Object[] {null});
        assertFalse("The verifyPath() method rejected a valid path", result);
    }

    private static Object callPrivateMethod(Object obj, String method, Class[] params, Object[] args) throws Exception {
        Object ret = null;
        Method m = obj.getClass().getDeclaredMethod(method, params);

        m.setAccessible(true);
        ret = m.invoke(obj, args);

        return ret;
    }
}
