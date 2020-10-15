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

public class TaskSpecifierTest extends TestCase {
    public TaskSpecifierTest() {
    }

    /**
     * Test of getMin method, of class TaskSpecifier.
     */
    public void testMin() {
        System.out.println("min");

        TaskSpecifier instance = new TaskSpecifier();
        int expResult = 100;

        instance.setMin(expResult);

        int result = instance.getMin();

        assertEquals("The getMin() method did not return the value set with setMin()",
                expResult, result);

        try {
            instance.setMin(0);
            fail("The setMin() method allowed a zero value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMin() method set the min value even though it threw an exception",
                    expResult, result);
        }

        try {
            instance.setMin(-1);
            fail("The setMin() method allowed a negative value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMin() method set the min value even though it threw an exception",
                    expResult, result);
        }
    }

    /**
     * Test of getMax method, of class TaskSpecifier.
     */
    public void testMax() {
        System.out.println("max");

        TaskSpecifier instance = new TaskSpecifier();
        int expResult = 100;

        instance.setMax(expResult);

        int result = instance.getMax();

        assertEquals("The getMax() method did not return the value set with setMax()",
                expResult, result);

        try {
            instance.setMax(0);
            fail("The setMax() method allowed a zero value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMax() method set the min value even though it threw an exception",
                    expResult, result);
        }

        try {
            instance.setMax(-1);
            fail("The setMax() method allowed a negative value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMax() method set the min value even though it threw an exception",
                    expResult, result);
        }
    }

    /**
     * Test of getStep method, of class TaskSpecifier.
     */
    public void testStep() {
        System.out.println("step");

        TaskSpecifier instance = new TaskSpecifier();
        int expResult = 100;

        instance.setStep(expResult);

        int result = instance.getStep();

        assertEquals("The getStep() method did not return the value set with setStep()",
                expResult, result);

        try {
            instance.setStep(0);
            fail("The setStep() method allowed a zero value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setStep() method set the min value even though it threw an exception",
                    expResult, result);
        }

        try {
            instance.setStep(-1);
            fail("The setStep() method allowed a negative value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setStep() method set the min value even though it threw an exception",
                    expResult, result);
        }
    }

    /**
     * Test of setRange method, of class TaskSpecifier.
     */
    public void testSetRange() {
        System.out.println("setRange(int,int)");

        int min = 10;
        int max = 100;
        TaskSpecifier instance = new TaskSpecifier();

        instance.setRange(min, max);

        assertEquals("The getMin() method did not return the value set with setRange(int,int)",
                instance.getMin(), min);
        assertEquals("The getMax() method did not return the value set with setRange(int,int)",
                instance.getMax(), max);

        try {
            instance.setRange(-1, 1);
            fail("The setRange(int,int) method allowed a negative min value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set min even though it threw an exception", 10, instance.getMin());
            assertEquals("The setRange() method set max even though it threw an exception", 100, instance.getMax());
        }

        try {
            instance.setRange(1, -1);
            fail("The setRange(int,int) method allowed a negative max value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set min even though it threw an exception", 10, instance.getMin());
            assertEquals("The setRange() method set max even though it threw an exception", 100, instance.getMax());
        }

        try {
            instance.setRange(100, 10);
            fail("The setRange() method allowed max to be set less than min");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set min even though it threw an exception", 10, instance.getMin());
            assertEquals("The setRange() method set max even though it threw an exception", 100, instance.getMax());
        }
    }

    /**
     * Test of setRange method, of class TaskSpecifier.
     */
    public void testSetRangeFull() {
        System.out.println("setRange(int,int,int)");

        int min = 10;
        int max = 100;
        int step = 2;

        TaskSpecifier instance = new TaskSpecifier();

        instance.setRange(min, max, step);

        assertEquals("The getMin() method did not return the value set with setRange(int,int,int)",
                instance.getMin(), min);
        assertEquals("The getMax() method did not return the value set with setRange(int,int,int)",
                instance.getMax(), max);
        assertEquals("The getStep() method did not return the value set with setRange(int,int,int)",
                instance.getStep(), step);

        try {
            instance.setRange(-1, 1, 1);
            fail("The setRange(int,int,int) method allowed a negative min value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set min even though it threw an exception", 10, instance.getMin());
            assertEquals("The setRange() method set max even though it threw an exception", 100, instance.getMax());
            assertEquals("The setRange() method set step even though it threw an exception", 2, instance.getStep());
        }

        try {
            instance.setRange(1, -1, 1);
            fail("The setRange(int,int,int) method allowed a negative max value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set min even though it threw an exception", 10, instance.getMin());
            assertEquals("The setRange() method set max even though it threw an exception", 100, instance.getMax());
            assertEquals("The setRange() method set step even though it threw an exception", 2, instance.getStep());
        }

        try {
            instance.setRange(1, 1, -1);
            fail("The setRange(int,int,int) method allowed a negative step value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set min even though it threw an exception", 10, instance.getMin());
            assertEquals("The setRange() method set max even though it threw an exception", 100, instance.getMax());
            assertEquals("The setRange() method set step even though it threw an exception", 2, instance.getStep());
        }

        try {
            instance.setRange(100, 10, 1);
            fail("The setRange() method allowed max to be set less than min");
        } catch (IllegalArgumentException e) {
            assertEquals("The setRange() method set min even though it threw an exception", 10, instance.getMin());
            assertEquals("The setRange() method set max even though it threw an exception", 100, instance.getMax());
            assertEquals("The setRange() method set step even though it threw an exception", 2, instance.getStep());
        }
    }

    /**
     * Test of equals method, of class TaskSpecifier.
     */
    public void testEquals() {
        System.out.println("equals()");

        TaskSpecifier instance1 = new TaskSpecifier();
        TaskSpecifier instance2 = new TaskSpecifier();

        instance1.setRange(1, 100, 2);
        instance2.setRange(1, 100, 2);

        assertTrue("The equals() method reported two equal objects as unequal",
                instance1.equals(instance2));

        instance2.setMin(2);

        assertFalse("The equals() method reported two unequal objects as equal",
                instance1.equals(instance2));

        instance1.setMin(2);

        assertTrue("The equals() method reported two equal objects as unequal",
                instance1.equals(instance2));

        instance2.setMax(200);

        assertFalse("The equals() method reported two unequal objects as equal",
                instance1.equals(instance2));

        instance1.setMax(200);

        assertTrue("The equals() method reported two equal objects as unequal",
                instance1.equals(instance2));

        instance2.setStep(4);

        assertFalse("The equals() method reported two unequal objects as equal",
                instance1.equals(instance2));

        instance1.setStep(4);

        assertTrue("The equals() method reported two equal objects as unequal",
                instance1.equals(instance2));
    }

    /**
     * Test of hashCode method, of class TaskSpecifier.
     */
    public void testHashCode() {
        System.out.println("hashCode()");

        TaskSpecifier instance1 = new TaskSpecifier();
        TaskSpecifier instance2 = new TaskSpecifier();

        instance1.setRange(1, 100, 2);
        instance2.setRange(1, 100, 2);

        assertEquals("The hashCode() method returned inconsistent results",
                instance1.hashCode(), instance2.hashCode());

        instance2.setMin(2);

        assertFalse("The hashCode() method returned inconsistent results",
                instance1.hashCode() == instance2.hashCode());

        instance1.setMin(2);

        assertEquals("The hashCode() method returned inconsistent results",
                instance1.hashCode(), instance2.hashCode());

        instance2.setMax(200);

        assertFalse("The hashCode() method returned inconsistent results",
                instance1.hashCode() == instance2.hashCode());

        instance1.setMax(200);

        assertEquals("The hashCode() method returned inconsistent results",
                instance1.hashCode(), instance2.hashCode());

        instance2.setStep(4);

        assertFalse("The hashCode() method returned inconsistent results",
                instance1.hashCode() == instance2.hashCode());

        instance1.setStep(4);

        assertEquals("The hashCode() method returned inconsistent results",
                instance1.hashCode(), instance2.hashCode());
    }

    /**
     * Test of clone method, of class CheckpointSpecifier.
     */
    public void testClone() throws Exception {
        System.out.println("clone()");

        TaskSpecifier instance1 = new TaskSpecifier();

        instance1.setMax(10);

        TaskSpecifier instance2 = instance1.clone();

        assertNotSame("The clone() method did not return a clone: ", instance1, instance2);
        assertEquals("The clone() method returned inconsistent results: ", instance1, instance2);

        instance1.setMax(101);

        assertFalse("The clone() method returned inconsistent results: ", instance1 == instance2);
    }
}
