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

public class ParallelEnvironmentTest extends TestCase {
    public ParallelEnvironmentTest() {
    }

    /**
     * Test of setName method, of class ParallelEnvironment.
     */
    public void testName() {
        System.out.println("name");

        ParallelEnvironment instance = new ParallelEnvironment();
        String expResult = "test";

        instance.setName(expResult);

        String result = instance.getName();

        assertEquals("The getName() method did not return the value set with setName()",
                expResult, result);
    }

    /**
     * Test of getRangeMin method, of class ParallelEnvironment.
     */
    public void testRangeMin() {
        System.out.println("rangeMin");

        ParallelEnvironment instance = new ParallelEnvironment();
        int expResult = 100;

        instance.setRangeMin(expResult);

        int result = instance.getRangeMin();

        assertEquals("The getRangeMin() method did not return the value set with setRangeMin()",
                expResult, result);

        try {
          instance.setRangeMin(0);
          fail("The setRangeMin() method allowed a zero value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setRangeMin() method set the min value even though it threw an exception", expResult, instance.getRangeMin());
        }

        try {
          instance.setRangeMin(-1);
          fail("The setRangeMin() method allowed a negative value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setRangeMin() method set the min value even though it threw an exception", expResult, instance.getRangeMin());
        }
    }

    /**
     * Test of getRangeMax method, of class ParallelEnvironment.
     */
    public void testRangeMax() {
        System.out.println("rangeMax");

        ParallelEnvironment instance = new ParallelEnvironment();
        int expResult = 100;

        instance.setRangeMax(expResult);

        int result = instance.getRangeMax();

        assertEquals("The getRangeMax() method did not return the value set with setRangeMax()",
                expResult, result);

        try {
          instance.setRangeMax(0);
          fail("The setRangeMax() method allowed a zero value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setRangeMax() method set the min value even though it threw an exception", expResult, instance.getRangeMax());
        }

        try {
          instance.setRangeMax(-1);
          fail("The setRangeMax() method allowed a negative value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setRangeMax() method set the min value even though it threw an exception", expResult, instance.getRangeMax());
        }
    }

    /**
     * Test of setRange method, of class ParallelEnvironment.
     */
    public void testSetRange() {
        System.out.println("setRange(int,int)");

        int min = 10;
        int max = 100;
        ParallelEnvironment instance = new ParallelEnvironment();

        instance.setRange(min, max);

        assertEquals("The getRangeMin() method did not return the value set with setRange(int,int)",
                min, instance.getRangeMin());
        assertEquals("The getRangeMax() method did not return the value set with setRange(int,int)",
                max, instance.getRangeMax());

        try {
          instance.setRange(0, 1);
          fail("The setRange(int,int) method allowed a zero min value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setRange() method set the min value even though it threw an exception", 10, instance.getRangeMin());
          assertEquals("The setRange() method set the min value even though it threw an exception", 100, instance.getRangeMax());
        }

        try {
          instance.setRange(1, 0);
          fail("The setRange(int,int) method allowed a zero max value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setRange() method set the min value even though it threw an exception", 10, instance.getRangeMin());
          assertEquals("The setRange() method set the min value even though it threw an exception", 100, instance.getRangeMax());
        }

        try {
          instance.setRange(-1, 1);
          fail("The setRange(int,int) method allowed a negative min value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setRange() method set the min value even though it threw an exception", 10, instance.getRangeMin());
          assertEquals("The setRange() method set the min value even though it threw an exception", 100, instance.getRangeMax());
        }

        try {
          instance.setRange(1, -1);
          fail("The setRange(int,int) method allowed a negative max value");
        } catch (IllegalArgumentException e) {
          assertEquals("The setRange() method set the min value even though it threw an exception", 10, instance.getRangeMin());
          assertEquals("The setRange() method set the min value even though it threw an exception", 100, instance.getRangeMax());
        }

        try {
            instance.setRange(100, 10);
            fail("The setRange() method accepted a min value that was higher than the max value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set the min value even though it threw an exception",
                    min, instance.getRangeMin());
            assertEquals("The setRange() method set the max value even though it threw an exception",
                    max, instance.getRangeMax());
        }
    }

    /**
     * Test of setRange method, of class ParallelEnvironment.
     */
    public void testSetRangeSingle() {
        System.out.println("setRange(int)");

        int val = 100;
        ParallelEnvironment instance = new ParallelEnvironment();
        instance.setRange(val);

        assertEquals("The getRangeMin() method did not return the value set with setRange(int)",
                instance.getRangeMin(), val);
        assertEquals("The getRangeMax() method did not return the value set with setRange(int)",
                instance.getRangeMax(), val);

        try {
          instance.setRange(0);
          fail("The setRange() method allowed a zero value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set the min value even though it threw an exception",
                    val, instance.getRangeMin());
            assertEquals("The setRange() method set the max value even though it threw an exception",
                    val, instance.getRangeMax());
        }

        try {
          instance.setRange(-1);
          fail("The setRange() method allowed a negative value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set the min value even though it threw an exception",
                    val, instance.getRangeMin());
            assertEquals("The setRange() method set the max value even though it threw an exception",
                    val, instance.getRangeMax());
        }
    }

    /**
     * Test of equals method, of class ParallelEnvironment.
     */
    public void testEquals() {
        System.out.println("equals()");

        ParallelEnvironment instance1 = new ParallelEnvironment();
        ParallelEnvironment instance2 = new ParallelEnvironment();

        instance1.setName("test1");
        instance1.setRange(1, 100);
        instance2.setName("test1");
        instance2.setRange(1, 100);

        assertTrue("The equals() method reported two equal objects as unequal",
                instance1.equals(instance2));

        instance2.setName("test2");

        assertFalse("The equals() method reported two unequal objects as equal",
                instance1.equals(instance2));

        instance1.setName("test2");

        assertTrue("The equals() method reported two equal objects as unequal",
                instance1.equals(instance2));

        instance2.setRange(10, 100);

        assertFalse("The equals() method reported two unequal objects as equal",
                instance1.equals(instance2));

        instance1.setRange(10, 100);

        assertTrue("The equals() method reported two equal objects as unequal",
                instance1.equals(instance2));

        instance2.setRange(10, 1000);

        assertFalse("The equals() method reported two unequal objects as equal",
                instance1.equals(instance2));

        instance1.setRange(10, 1000);

        assertTrue("The equals() method reported two equal objects as unequal",
                instance1.equals(instance2));
        assertFalse("The equals() method reported two unequal objects as equal",
                instance1.equals(new Object()));
    }

    /**
     * Test of hashCode method, of class ParallelEnvironment.
     */
    public void testHashCode() {
        System.out.println("hashCode()");

        ParallelEnvironment instance1 = new ParallelEnvironment();
        ParallelEnvironment instance2 = new ParallelEnvironment();

        instance1.setName("test1");
        instance1.setRange(1, 100);
        instance2.setName("test1");
        instance2.setRange(10, 50);

        assertEquals("The hashCode() method returned inconsistent results",
                instance1.hashCode(), instance2.hashCode());

        instance2.setName("test2");
        instance2.setRange(1, 100);

        assertFalse("The hashCode() method returned inconsistent results",
                instance1.hashCode() == instance2.hashCode());
    }

    /**
     * Test of clone method, of class CheckpointSpecifier.
     */
    public void testClone() throws Exception {
        System.out.println("clone()");

        ParallelEnvironment instance1 = new ParallelEnvironment();

        instance1.setName("test1");

        ParallelEnvironment instance2 = instance1.clone();

        assertNotSame("The clone() method did not return a clone: ", instance1, instance2);
        assertEquals("The clone() method returned inconsistent results: ", instance1, instance2);

        instance1.setName("test2");

        assertFalse("The clone() method returned inconsistent results: ", instance1 == instance2);
    }
}
