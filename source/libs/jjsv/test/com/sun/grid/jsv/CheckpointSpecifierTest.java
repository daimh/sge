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
package com.sun.grid.jsv;

import junit.framework.TestCase;

public class CheckpointSpecifierTest extends TestCase {
    public CheckpointSpecifierTest() {
    }

    /**
     * Test of getName method, of class CheckpointSpecifier.
     */
    public void testName() throws NoSuchFieldException, IllegalAccessException {
        System.out.println("name");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        String expResult = "test";

        instance.setName(expResult);

        String result = instance.getName();

        assertEquals("The getName() method did not return the value set with setName()", expResult, result);
    }

    /**
     * Test of onShutdown method, of class CheckpointSpecifier.
     */
    public void testOnShutdown() {
        System.out.println("onShutdown()");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        String expResult = "s";

        instance.onShutdown(true);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onShutdown()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        expResult = "n";
        instance.onShutdown(false);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onShutdown()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);
    }

    /**
     * Test of onMinCpuInterval method, of class CheckpointSpecifier.
     */
    public void testOnMinCpuInterval() {
        System.out.println("onMinCpuInterval()");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        String expResult = "m";

        instance.onMinCpuInterval(true);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onMinCpuInterval()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        expResult = "n";
        instance.onMinCpuInterval(false);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onMinCpuInterval()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);
    }

    /**
     * Test of onSuspend method, of class CheckpointSpecifier.
     */
    public void testOnSuspend() {
        System.out.println("onSuspend()");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        String expResult = "x";

        instance.onSuspend(true);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onSuspend()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        expResult = "n";
        instance.onSuspend(false);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onSuspend()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);
    }

    /**
     * Test of never method, of class CheckpointSpecifier.
     */
    public void testNever() {
        System.out.println("never()");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        String expResult = "n";

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the expected default value", expResult, result);

        instance.setOccasion("smx");
        instance.never();
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with never()", expResult, result);

        long expResult2 = 1234L;
        instance.setInterval(expResult2);
        instance.never();
        long result2 = instance.getInterval();

        assertEquals("The getOccasionString() method did not return the value set with never()", expResult, result);
        assertEquals("The getInterval() method did not return the value set with setInterval()", expResult2, result2);
    }

    /**
     * Test of getInterval method, of class CheckpointSpecifier.
     */
    public void testInterval() {
        System.out.println("interval");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        long expResult = 123456L;

        instance.onShutdown(true);
        instance.setInterval(expResult);

        long result = instance.getInterval();
        assertEquals("The getInterval() method did not return the value set with setInterval()", expResult, result);
        assertEquals("The getOccasion() method did not return 0", 0, instance.getOccasion());

        instance.onShutdown(true);
        instance.setInterval(0, 0, (int)expResult);

        result = instance.getInterval();
        assertEquals("The getInterval() method did not return the value set with setInterval()", expResult, result);
        assertEquals("The getOccasion() method did not return 0", 0, instance.getOccasion());

        instance.onShutdown(true);
        instance.setInterval((int)(expResult / 3600), (int)((expResult % 3600) / 60), (int)(expResult % 60));

        result = instance.getInterval();
        assertEquals("The getInterval() method did not return the value set with setInterval()", expResult, result);
        assertEquals("The getOccasion() method did not return 0", 0, instance.getOccasion());

        byte expResult2 = CheckpointSpecifier.ON_SHUTDOWN;
        instance.onShutdown(true);
        expResult = 0L;
        instance.setInterval(expResult);
        result = instance.getInterval();
        byte result2 = instance.getOccasion();

        assertEquals("The getInterval() method did not return the value set with setInterval()", expResult, result);
        assertEquals("The getOccasion() method did not return the value set with onShutdown()", CheckpointSpecifier.ON_SHUTDOWN, instance.getOccasion());

        try {
          instance.setInterval(-1L);
          fail("The setInterval(long) method allowed a negative argument");
        } catch (IllegalArgumentException e) {
          assertEquals("The setInterval() method set the interval even though it threw an exception", 0L, instance.getInterval());
        }

        try {
          instance.setInterval(-1, 1, 1);
          fail("The setInterval(int,int,int) method allowed a negative hours argument");
        } catch (IllegalArgumentException e) {
          assertEquals("The setInterval() method set the interval even though it threw an exception", 0L, instance.getInterval());
        }

        try {
          instance.setInterval(1, -1, 1);
          fail("The setInterval(int,int,int) method allowed a negative minutes argument");
        } catch (IllegalArgumentException e) {
          assertEquals("The setInterval() method set the interval even though it threw an exception", 0L, instance.getInterval());
        }

        try {
          instance.setInterval(1, 1, -1);
          fail("The setInterval(int,int,int) method allowed a negative seconds argument");
        } catch (IllegalArgumentException e) {
          assertEquals("The setInterval() method set the interval even though it threw an exception", 0L, instance.getInterval());
        }
    }

    /**
     * Test of getOccasion method, of class CheckpointSpecifier.
     */
    public void testOccasion() {
        System.out.println("occasion");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        byte expResult = 7;

        instance.setOccasion(expResult);

        byte result = instance.getOccasion();

        assertEquals("The getOccasion() method did not return the value set with setOccasion()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        try {
            instance.setOccasion((byte)0x08);
            fail("The setOccasion() method allowed an illegal value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setOccasion() method set the occasion even though it threw an exception", 7, instance.getOccasion());
        }

        try {
            instance.setOccasion((byte)0x0F);
            fail("The setOccasion() method allowed an illegal value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setOccasion() method set the occasion even though it threw an exception", 7, instance.getOccasion());
        }
    }

    /**
     * Test of intercation of getOccasion() and getInterval() methods, of class CheckpointSpecifier.
     */
    public void testOccasionPlusInterval() {
        System.out.println("occasion");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        byte expOccasion = 7;
        long expInterval = 7;

        instance.setInterval(expInterval);
        instance.setOccasion(expOccasion);

        byte resultOccasion = instance.getOccasion();

        assertEquals("The getOccasion() method did not return the value set with setOccasion()", expOccasion, resultOccasion);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        instance.setInterval(expInterval);

        long resultInterval = instance.getInterval();

        assertEquals("The getInterval() method did not return the value set with setInterval()", expInterval, resultInterval);
        assertEquals("The getOccasion() method did not return 0", instance.getOccasion(), 0);

        instance.setOccasion(expOccasion);

        resultOccasion = instance.getOccasion();

        assertEquals("The getOccasion() method did not return the value set with setOccasion()", expOccasion, resultOccasion);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);
    }

    /**
     * Test of getOccasion method, of class CheckpointSpecifier.
     */
    public void testOccasionString() {
        System.out.println("occasionString");

        CheckpointSpecifier instance = new CheckpointSpecifier();
        String expResult = "msx";

        instance.setOccasion(expResult);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        instance.setOccasion("xms");
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        instance.setOccasion("mxs");
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        expResult = "n";
        instance.setOccasion(expResult);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        instance.setOccasion("");
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        instance.setOccasion(null);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);
        assertEquals("The getInterval() method did not return 0", instance.getInterval(), 0L);

        try {
          instance.setOccasion("q");
          fail("The setOccasion() method allowed an illegal occasion string");
        } catch (IllegalArgumentException e) {
          assertEquals("The setOccasion() method set the occasion even though it threw an exception", "n", instance.getOccasionString());
        }

        try {
          instance.setOccasion("mqx");
          fail("The setOccasion() method allowed an illegal occasion string");
        } catch (IllegalArgumentException e) {
          assertEquals("The setOccasion() method set the occasion even though it threw an exception", "n", instance.getOccasionString());
        }

        try {
          instance.setOccasion("mnx");
          fail("The setOccasion() method allowed an illegal occasion string");
        } catch (IllegalArgumentException e) {
          assertEquals("The setOccasion() method set the occasion even though it threw an exception", "n", instance.getOccasionString());
        }
    }

    /**
     * Test of equals method, of class CheckpointSpecifier.
     */
    public void testEquals() {
        System.out.println("equals()");

        CheckpointSpecifier instance1 = new CheckpointSpecifier();
        CheckpointSpecifier instance2 = new CheckpointSpecifier();

        instance1.setName("test1");
        instance1.setInterval(1234L);
        instance2.setName("test1");
        instance2.setInterval(1234L);

        assertTrue("The equals() method reported two equal objects as unequal", instance1.equals(instance2));

        instance1.setOccasion("m");

        assertFalse("The equals() method reported two unequal objects as equal", instance1.equals(instance2));

        instance1.setInterval(1234L);
        instance1.setName("test2");

        assertFalse("The equals() method reported two unequal objects as equal", instance1.equals(instance2));

        instance1.setName("test1");
        instance1.onSuspend(true);

        assertFalse("The equals() method reported two unequal objects as equal", instance1.equals(instance2));
        assertFalse("The equals() method reported two unequal objects as equal", instance1.equals(new Object()));
    }

    /**
     * Test of hashCode method, of class CheckpointSpecifier.
     */
    public void testHashCode() {
        System.out.println("hashCode()");

        CheckpointSpecifier instance1 = new CheckpointSpecifier();
        CheckpointSpecifier instance2 = new CheckpointSpecifier();

        instance1.setName("test1");
        instance2.setName("test1");

        assertEquals("The hashCode() method returned inconsistent results: ", instance1.hashCode(), instance2.hashCode());

        instance1.setName("test2");

        assertFalse("The hashCode() method returned inconsistent results: ", instance1.hashCode() == instance2.hashCode());
    }

    /**
     * Test of clone method, of class CheckpointSpecifier.
     */
    public void testClone() throws Exception {
        System.out.println("clone()");

        CheckpointSpecifier instance1 = new CheckpointSpecifier();

        instance1.setName("test1");

        CheckpointSpecifier instance2 = instance1.clone();

        assertNotSame("The clone() method did not return a clone: ", instance1, instance2);
        assertEquals("The clone() method returned inconsistent results: ", instance1, instance2);

        instance1.setName("test2");

        assertFalse("The clone() method returned inconsistent results: ", instance1 == instance2);
    }
}
