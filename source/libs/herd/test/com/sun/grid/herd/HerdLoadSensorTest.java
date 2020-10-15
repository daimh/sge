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

import com.sun.grid.loadsensor.LoadSensor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import junit.framework.TestCase;

/**
 *
 */
public class HerdLoadSensorTest extends TestCase {
    
    public HerdLoadSensorTest(String testName) {
        super(testName);
    }

    /**
     * Test of getMeasurementInterval method, of class HerdLoadSensor.
     */
    public void testGetMeasurementInterval() {
        System.out.println("getMeasurementInterval()");
        HerdLoadSensor instance = new HerdLoadSensor();
        int expResult = LoadSensor.MEASURE_ON_DEMAND;
        int result = instance.getMeasurementInterval();

        assertEquals("The getMeasurementInterval() method did not return MEASURE_ON_DEMAND", expResult, result);
    }

    /**
     * Test of buildBlockStrings method, of class HerdLoadSensor.
     */
    public void testBuildBlockStrings() throws Exception {
        System.out.println("buildBlockStrings()");
        HerdLoadSensor instance = new HerdLoadSensor();
        List<Long> ids = new ArrayList<Long>(5);
        Map<String,String> expectedResult = new HashMap<String, String>();
        Map result = null;

        ids.add(0x1234567890abcdefL);
        ids.add(0x0123456789abcdefL);
        ids.add(0xfedcba0987654321L);
        ids.add(0xfedcba9876543210L);
        ids.add(0x0L);
        expectedResult.put("hdfs_blk12", "/34567890abcdef/");
        expectedResult.put("hdfs_blk01", "/23456789abcdef/");
        expectedResult.put("hdfs_blkfe", "/dcba0987654321/dcba9876543210/");
        expectedResult.put("hdfs_blk00", "/00000000000000/");

        result = (Map)callPrivateMethod(instance, "buildBlockStrings", new Class[]{List.class}, new Object[]{ids});
        assertEquals("The buildBlockStrings() method did not build the expected block strings: ", expectedResult, result);

        ids.clear();
        ids.add(0xfedcba0987654321L);
        ids.add(0xfedcba9876543210L);
        expectedResult.clear();
        expectedResult.put("hdfs_blkfe", "/dcba0987654321/dcba9876543210/");

        result = (Map)callPrivateMethod(instance, "buildBlockStrings", new Class[]{List.class}, new Object[]{ids});
        assertEquals("The buildBlockStrings() method did not build the expected block strings: ", expectedResult, result);

        ids.clear();
        ids.add(0xfedcba0987654321L);
        expectedResult.clear();
        expectedResult.put("hdfs_blkfe", "/dcba0987654321/");

        result = (Map)callPrivateMethod(instance, "buildBlockStrings", new Class[]{List.class}, new Object[]{ids});
        assertEquals("The buildBlockStrings() method did not build the expected block strings: ", expectedResult, result);

        ids.clear();
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildBlockStrings", new Class[]{List.class}, new Object[]{ids});
        assertEquals("The buildBlockStrings() method did not build the expected block strings: ", expectedResult, result);

        ids = null;
        expectedResult.clear();

        result = (Map)callPrivateMethod(instance, "buildBlockStrings", new Class[]{List.class}, new Object[]{ids});
        assertEquals("The buildBlockStrings() method did not build the expected block strings: ", expectedResult, result);
    }

    /**
     * Test of getLoadValues method, of class HerdLoadSensor.
     */
    public void testGetLoadValues() throws Exception {
        System.out.println("getLoadValues");
        HerdLoadSensor instance = new HerdLoadSensor();
        Map<String,String> blocks = new HashMap<String, String>();
        Map<String,Map<String,String>> expectedResult = new HashMap<String, Map<String, String>>();
        Map result = null;

        blocks.put("hdfs_blk12", "/34567890abcdef/");
        blocks.put("hdfs_blk01", "/23456789abcdef/");
        blocks.put("hdfs_blkfe", "/dcba0987654321/dcba9876543210/");
        blocks.put("hdfs_blk00", "/00000000000000/");

        setPrivateField(instance, "hostName", "myhost");
        setPrivateField(instance, "rackName", "/rack1");
        setPrivateField(instance, "blockStrings", blocks);

        Map<String,String> requests = new HashMap<String, String>(blocks);

        requests.put("hdfs_primary_rack", "/rack1");
        requests.put("hdfs_secondary_rack", "/rack1");
        expectedResult.put("myhost", requests);

        result = instance.getLoadValues();
        assertEquals("The getLoadValues() method did not return the expected load values: ", expectedResult, result);

        setPrivateField(instance, "hostName", null);

        requests = new HashMap<String, String>(blocks);

        requests.put("hdfs_primary_rack", "/rack1");
        requests.put("hdfs_secondary_rack", "/rack1");
        expectedResult.clear();
        expectedResult.put("localhost", requests);

        result = instance.getLoadValues();
        assertEquals("The getLoadValues() method did not return the expected load values: ", expectedResult, result);

        setPrivateField(instance, "hostName", "myhost");
        setPrivateField(instance, "rackName", null);

        requests = new HashMap<String, String>(blocks);

        requests.put("hdfs_primary_rack", "/default-rack");
        requests.put("hdfs_secondary_rack", "/default-rack");
        expectedResult.clear();
        expectedResult.put("myhost", requests);

        result = instance.getLoadValues();
        assertEquals("The getLoadValues() method did not return the expected load values: ", expectedResult, result);

        setPrivateField(instance, "hostName", "myhost");
        setPrivateField(instance, "rackName", "/rack1");
        setPrivateField(instance, "blockStrings", null);

        requests = new HashMap<String, String>();

        requests.put("hdfs_primary_rack", "/rack1");
        requests.put("hdfs_secondary_rack", "/rack1");
        expectedResult.clear();
        expectedResult.put("myhost", requests);

        result = instance.getLoadValues();
        assertEquals("The getLoadValues() method did not return the expected load values: ", expectedResult, result);
    }

    private static Object callPrivateMethod(Object obj, String method, Class[] params, Object[] args) throws Exception {
        Object ret = null;
        Method m = obj.getClass().getDeclaredMethod(method, params);

        m.setAccessible(true);
        ret = m.invoke(obj, args);

        return ret;
    }

    private static void setPrivateField(Object obj, String field, Object value) throws Exception {
        Field f = obj.getClass().getDeclaredField(field);

        f.setAccessible(true);
        f.set(obj, value);
    }
}
