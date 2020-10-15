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

public class MailSpecifierTest extends TestCase {
    public MailSpecifierTest() {
    }

    /**
     * Test of onBegin method, of class MailSpecifier.
     */
    public void testOnBegin() {
        System.out.println("onBegin()");

        MailSpecifier instance = new MailSpecifier();
        String expResult = "b";

        instance.onBegin(true);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onBegin()", expResult, result);

        expResult = "n";
        instance.onBegin(false);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onBegin()", expResult, result);
    }

    /**
     * Test of onEnd method, of class MailSpecifier.
     */
    public void testOnEnd() {
        System.out.println("onEnd()");

        MailSpecifier instance = new MailSpecifier();
        String expResult = "e";

        instance.onEnd(true);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onEnd()", expResult, result);

        expResult = "n";
        instance.onEnd(false);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onEnd()", expResult, result);
    }

    /**
     * Test of onAbort method, of class MailSpecifier.
     */
    public void testOnAbort() {
        System.out.println("onAbort()");

        MailSpecifier instance = new MailSpecifier();
        String expResult = "a";

        instance.onAbort(true);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onAbort()", expResult, result);

        expResult = "n";
        instance.onAbort(false);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onAbort()", expResult, result);
    }

    /**
     * Test of onSuspend method, of class MailSpecifier.
     */
    public void testOnSuspend() {
        System.out.println("onSuspend()");

        MailSpecifier instance = new MailSpecifier();
        String expResult = "s";

        instance.onSuspend(true);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onSuspend()", expResult, result);

        expResult = "n";
        instance.onSuspend(false);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with onSuspend()", expResult, result);
    }

    /**
     * Test of never method, of class MailSpecifier.
     */
    public void testNever() {
        System.out.println("never()");

        MailSpecifier instance = new MailSpecifier();
        String expResult = "n";

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the expected default value", expResult, result);

        instance.setOccasion("abes");
        instance.never();
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with never()", expResult, result);
    }

    /**
     * Test of getOccasion method, of class MailSpecifier.
     */
    public void testOccasion() {
        System.out.println("occasion");

        MailSpecifier instance = new MailSpecifier();
        byte expResult = 0x0F;

        instance.setOccasion(expResult);

        byte result = instance.getOccasion();

        assertEquals("The getOccasion() method did not return the value set with setOccasion()", expResult, result);

        try {
            instance.setOccasion((byte)0x80);
            fail("The setOccasion() method accepted an illegal value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setOccasion() method set the occasion value even though it through an exception", expResult, result);
        }
    }

    /**
     * Test of getOccasionString method, of class MailSpecifier.
     */
    public void testOccasionString() {
        System.out.println("occasionString");

        MailSpecifier instance = new MailSpecifier();
        String expResult = "abes";

        instance.setOccasion(expResult);

        String result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);

        instance.setOccasion("beas");
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);

        instance.setOccasion("seab");
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);

        expResult = "n";
        instance.setOccasion(expResult);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);

        instance.setOccasion("");
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);

        instance.setOccasion(null);
        result = instance.getOccasionString();

        assertEquals("The getOccasionString() method did not return the value set with setOccasion()", expResult, result);

        try {
          instance.setOccasion("q");
          fail("The setOccasion() method allowed an illegal occasion string");
        } catch (IllegalArgumentException e) {
          assertEquals("The setOccasion() method set the occasion string even though it threw an exception", "n", instance.getOccasionString());
        }

        try {
          instance.setOccasion("aqs");
          fail("The setOccasion() method allowed an illegal occasion string");
        } catch (IllegalArgumentException e) {
          assertEquals("The setOccasion() method set the occasion string even though it threw an exception", "n", instance.getOccasionString());
        }

        try {
          instance.setOccasion("ans");
          fail("The setOccasion() method allowed an illegal occasion string");
        } catch (IllegalArgumentException e) {
          assertEquals("The setOccasion() method set the occasion string even though it threw an exception", "n", instance.getOccasionString());
        }
    }

    /**
     * Test of equals method, of class MailSpecifier.
     */
    public void testEquals() {
        System.out.println("equals()");

        MailSpecifier instance1 = new MailSpecifier();
        MailSpecifier instance2 = new MailSpecifier();

        instance1.setOccasion("abe");
        instance2.setOccasion("abe");

        assertTrue("The equals() method reported two equal objects as unequal", instance1.equals(instance2));

        instance1.onSuspend(true);

        assertFalse("The equals() method reported two unequal objects as equal", instance1.equals(instance2));

        instance2.onSuspend(true);

        assertTrue("The equals() method reported two equal objects as unequal", instance2.equals(instance2));

        instance1.onBegin(false);

        assertFalse("The equals() method reported two unequal objects as equal", instance1.equals(instance2));
        assertFalse("The equals() method reported two unequal objects as equal", instance1.equals(new Object()));
    }

    /**
     * Test of hashCode method, of class MailSpecifier.
     */
    public void testHashCode() {
        System.out.println("hashCode()");

        MailSpecifier instance1 = new MailSpecifier();
        MailSpecifier instance2 = new MailSpecifier();

        instance1.setOccasion("abes");
        instance2.setOccasion("abes");

        assertEquals("The hashCode() method returned inconsistent results", instance1.hashCode(), instance2.hashCode());

        instance1.setOccasion("e");

        assertFalse("The hashCode() method returned inconsistent results", instance1.hashCode() == instance2.hashCode());
    }

    /**
     * Test of clone method, of class CheckpointSpecifier.
     */
    public void testClone() throws Exception {
        System.out.println("clone()");

        MailSpecifier instance1 = new MailSpecifier();

        instance1.setOccasion("abes");

        MailSpecifier instance2 = instance1.clone();

        assertNotSame("The clone() method did not return a clone: ", instance1, instance2);
        assertEquals("The clone() method returned inconsistent results: ", instance1, instance2);

        instance1.setOccasion("e");

        assertFalse("The clone() method returned inconsistent results: ", instance1 == instance2);
    }
}
