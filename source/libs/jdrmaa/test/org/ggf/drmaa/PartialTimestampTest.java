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
/*
 * PartialTimestampTest.java
 * JUnit based test
 *
 * Created on November 19, 2004, 1:40 PM
 */

package org.ggf.drmaa;

import java.util.*;
import junit.framework.*;

/**
 *
 * @author dan.templeton@sun.com
 */
public class PartialTimestampTest extends TestCase {
    private PartialTimestamp pt = null;
    
    public PartialTimestampTest(java.lang.String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(PartialTimestampTest.class);
        return suite;
    }
    
    public void setUp() {
        pt = new PartialTimestamp();
    }
    
    public void tearDown() {
        pt = null;
    }
    
    /** Test of isLeapYear method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testIsLeapYear() {
        System.out.println("testIsLeapYear");
        
        for (int year = 1970; year <= 2200; year++) {
            if (((year % 400) == 0) || (((year % 100) != 0) && ((year % 4) == 0))) {
                assertTrue(PartialTimestamp.isLeapYear(year));
            } else {
                assertFalse(PartialTimestamp.isLeapYear(year));
            }
        }
    }
    
    /** Test of getTotalDays method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testGetTotalDays() {
        System.out.println("testGetTotalDays");
        
        assertEquals(0, PartialTimestamp.getTotalDays(pt.JANUARY - 1, false));
        assertEquals(31, PartialTimestamp.getTotalDays(pt.JANUARY, false));
        assertEquals(59, PartialTimestamp.getTotalDays(pt.FEBRUARY, false));
        assertEquals(90, PartialTimestamp.getTotalDays(pt.MARCH, false));
        assertEquals(120, PartialTimestamp.getTotalDays(pt.APRIL, false));
        assertEquals(151, PartialTimestamp.getTotalDays(pt.MAY, false));
        assertEquals(181, PartialTimestamp.getTotalDays(pt.JUNE, false));
        assertEquals(212, PartialTimestamp.getTotalDays(pt.JULY, false));
        assertEquals(243, PartialTimestamp.getTotalDays(pt.AUGUST, false));
        assertEquals(273, PartialTimestamp.getTotalDays(pt.SEPTEMBER, false));
        assertEquals(304, PartialTimestamp.getTotalDays(pt.OCTOBER, false));
        assertEquals(334, PartialTimestamp.getTotalDays(pt.NOVEMBER, false));
        assertEquals(365, PartialTimestamp.getTotalDays(pt.DECEMBER, false));
        
        assertEquals(0, PartialTimestamp.getTotalDays(pt.JANUARY - 1, true));
        assertEquals(31, PartialTimestamp.getTotalDays(pt.JANUARY, true));
        assertEquals(60, PartialTimestamp.getTotalDays(pt.FEBRUARY, true));
        assertEquals(91, PartialTimestamp.getTotalDays(pt.MARCH, true));
        assertEquals(121, PartialTimestamp.getTotalDays(pt.APRIL, true));
        assertEquals(152, PartialTimestamp.getTotalDays(pt.MAY, true));
        assertEquals(182, PartialTimestamp.getTotalDays(pt.JUNE, true));
        assertEquals(213, PartialTimestamp.getTotalDays(pt.JULY, true));
        assertEquals(244, PartialTimestamp.getTotalDays(pt.AUGUST, true));
        assertEquals(274, PartialTimestamp.getTotalDays(pt.SEPTEMBER, true));
        assertEquals(305, PartialTimestamp.getTotalDays(pt.OCTOBER, true));
        assertEquals(335, PartialTimestamp.getTotalDays(pt.NOVEMBER, true));
        assertEquals(366, PartialTimestamp.getTotalDays(pt.DECEMBER, true));
    }
    
    /** Test of calculateMonth method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testCalculateMonth() {
        System.out.println("testCalculateMonth");
        int day = 1;
        
        for (int month = pt.JANUARY; month <= pt.DECEMBER; month++) {
            int end = PartialTimestamp.getTotalDays(month, false);
            
            for (; day <= end; day++) {
                assertEquals(month, PartialTimestamp.calculateMonth(day, false));
            }
        }
        
        assertEquals(365, day - 1);
        
        day = 1;
        
        for (int month = pt.JANUARY; month <= pt.DECEMBER; month++) {
            int end = PartialTimestamp.getTotalDays(month, true);
            
            for (; day <= end; day++) {
                assertEquals(month, PartialTimestamp.calculateMonth(day, true));
            }
        }
        
        assertEquals(366, day - 1);
    }
    
    /** Test of getLengthOfMonth method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testGetLengthOfMonth() {
        System.out.println("testGetLengthOfMonth");
        
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.JANUARY, false));
        assertEquals(28, PartialTimestamp.getLengthOfMonth(pt.FEBRUARY, false));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.MARCH, false));
        assertEquals(30, PartialTimestamp.getLengthOfMonth(pt.APRIL, false));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.MAY, false));
        assertEquals(30, PartialTimestamp.getLengthOfMonth(pt.JUNE, false));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.JULY, false));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.AUGUST, false));
        assertEquals(30, PartialTimestamp.getLengthOfMonth(pt.SEPTEMBER, false));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.OCTOBER, false));
        assertEquals(30, PartialTimestamp.getLengthOfMonth(pt.NOVEMBER, false));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.DECEMBER, false));
        
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.JANUARY, true));
        assertEquals(29, PartialTimestamp.getLengthOfMonth(pt.FEBRUARY, true));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.MARCH, true));
        assertEquals(30, PartialTimestamp.getLengthOfMonth(pt.APRIL, true));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.MAY, true));
        assertEquals(30, PartialTimestamp.getLengthOfMonth(pt.JUNE, true));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.JULY, true));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.AUGUST, true));
        assertEquals(30, PartialTimestamp.getLengthOfMonth(pt.SEPTEMBER, true));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.OCTOBER, true));
        assertEquals(30, PartialTimestamp.getLengthOfMonth(pt.NOVEMBER, true));
        assertEquals(31, PartialTimestamp.getLengthOfMonth(pt.DECEMBER, true));
    }
    
    /** Test of getLengthOfYear method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testGetLengthOfYear() {
        System.out.println("testGetLengthOfYear");
        
        for (int year = 1972; year <= 2200; year++) {
            if (PartialTimestamp.isLeapYear(year)) {
                assertEquals(366, PartialTimestamp.getLengthOfYear(year));
            } else {
                assertEquals(365, PartialTimestamp.getLengthOfYear(year));
            }
        }
    }
    
    /** Test of calculateDayOfWeek method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testCalculateDayOfWeek() {
        System.out.println("testCalculateDayOfWeek");
        
        /* 2000 started on a Saturday. */
        int dayOfWeek = pt.SATURDAY;
        
        for (int year = 2000; year <= 2200; year++) {
            for (int month = pt.JANUARY; month <= pt.DECEMBER; month++) {
                int length = PartialTimestamp.getLengthOfMonth(month, PartialTimestamp.isLeapYear(year));
                
                for (int day = 1; day <= length; day++) {
                    assertEquals(dayOfWeek, PartialTimestamp.calculateDayOfWeek(day, month, year));
                    
                    dayOfWeek++;
                    
                    if (dayOfWeek > pt.SATURDAY) {
                        dayOfWeek -= pt.SATURDAY;
                    }
                }
            }
        }
    }
    
    /** Test of g|setModifier method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testModifier() {
        System.out.println("testModifier");
        
        for (int field = pt.YEAR; field < pt.FIELD_COUNT; field++) {
            pt.setModifier(field, 1);
            assertEquals(1, pt.getModifier(field));
            pt.setModifier(field, 0);
            assertEquals(0, pt.getModifier(field));
            pt.setModifier(field, -1);
            assertEquals(-1, pt.getModifier(field));
        }
    }
    
    /** Test of set method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testSimpleSet() {
        System.out.println("testSimpleSet");
        
        /* Test in-range values */
        for (int field = pt.YEAR; field < pt.CENTURY; field++) {
            /* 1 is a valid value for every field except century. */
            pt.set(field, 1);
            assertEquals(1, pt.get(field));
            assertTrue(pt.isSet(field));
        }
        
        pt.set(pt.CENTURY, 21);
        assertEquals(21, pt.get(pt.CENTURY));
        assertTrue(pt.isSet(pt.CENTURY));
        
        /* Test out of range, non-resolveable values */
        for (int field = pt.YEAR; field < pt.CENTURY; field++) {
            /* 1 is a valid value for every field except century. */
            pt.set(field, 9999);
            assertEquals(9999, pt.get(field));
            assertTrue(pt.isSet(field));
        }
        
        for (int field = pt.YEAR; field < pt.CENTURY; field++) {
            /* 1 is a valid value for every field except century. */
            pt.set(field, -9999);
            assertEquals(-9999, pt.get(field));
            assertTrue(pt.isSet(field));
        }
    }
    
    /** Test of add method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testUnsetAdd() {
        System.out.println("testUnsetAdd");
        
        for (int field = pt.CENTURY; field < pt.CENTURY; field++) {
            pt.add(field, 1);
            assertEquals(1, pt.getModifier(field));
            assertFalse(pt.isSet(field));
        }
    }
    
    /** Test of add method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testSimpleAdd() {
        System.out.println("testSimpleAdd");
        
        pt.set(pt.CENTURY, 21);
        
        for (int field = pt.YEAR; field < pt.FIELD_COUNT; field++) {
            pt.set(field, 1);
        }
        
        pt.add(pt.CENTURY, 1);
        assertEquals(22, pt.get(pt.CENTURY));
        assertTrue(pt.isSet(pt.CENTURY));
        
        for (int field = pt.YEAR; field < pt.FIELD_COUNT; field++) {
            // Skip AM_PM -- handled in testComplexAdd()
            if (field == pt.AM_PM) {
                continue;
            }
            
            pt.add(field, 1);
            assertEquals(2, pt.get(field));
            assertTrue(pt.isSet(field));
        }
    }
    
    /** Test of get method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testComplexSet() {
        System.out.println("testComplexSet");
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        
        pt.set(pt.SECOND, 1);
        pt.set(pt.MILLISECOND, 1000);
        assertEquals(2, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MILLISECOND));
        
        pt.set(pt.SECOND, 10);
        pt.set(pt.MILLISECOND, -500);
        assertEquals(9, pt.get(pt.SECOND));
        assertEquals(500, pt.get(pt.MILLISECOND));
        
        pt.set(pt.MINUTE, 1);
        pt.set(pt.SECOND, 60);
        assertEquals(2, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        
        pt.set(pt.MINUTE, 10);
        pt.set(pt.SECOND, -10);
        assertEquals(9, pt.get(pt.MINUTE));
        assertEquals(50, pt.get(pt.SECOND));
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 60);
        assertEquals(2, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR_OF_DAY, 10);
        pt.set(pt.MINUTE, -10);
        assertEquals(9, pt.get(pt.HOUR_OF_DAY));
        assertEquals(50, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 1);
        pt.set(pt.MINUTE, 60);
        assertEquals(2, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 10);
        pt.set(pt.MINUTE, -10);
        assertEquals(9, pt.get(pt.HOUR));
        assertEquals(50, pt.get(pt.MINUTE));
        
        // Make sure that noon/midnight is handled correctly
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 12);
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 13);
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(1, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, -10);
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(2, pt.get(pt.HOUR));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.HOUR_OF_DAY, 24);
        assertEquals(2, pt.get(pt.DAY_OF_MONTH));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 10);
        pt.set(pt.HOUR_OF_DAY, -10);
        assertEquals(9, pt.get(pt.DAY_OF_MONTH));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.AM_PM, pt.PM + 1);
        assertEquals(2, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 10);
        pt.set(pt.AM_PM, -1);
        assertEquals(9, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.set(pt.HOUR_OF_DAY, 24);
        assertEquals(pt.MONDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.HOUR_OF_DAY, -10);
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.set(pt.AM_PM, pt.PM + 1);
        assertEquals(pt.MONDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.AM_PM, -1);
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.set(pt.HOUR_OF_DAY, 24);
        assertEquals(2, pt.get(pt.DAY_OF_YEAR));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.DAY_OF_YEAR, 10);
        pt.set(pt.HOUR_OF_DAY, -10);
        assertEquals(9, pt.get(pt.DAY_OF_YEAR));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.set(pt.AM_PM, pt.PM + 1);
        assertEquals(2, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_YEAR, 10);
        pt.set(pt.AM_PM, -1);
        assertEquals(9, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(2, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, -1);
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, -3);
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.WEDNESDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(2, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, -4);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(2, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, -2);
        pt.set(pt.DAY_OF_WEEK, -1);
        assertEquals(3, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.WEEK_OF_YEAR, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(2, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.WEEK_OF_YEAR, 10);
        pt.set(pt.DAY_OF_WEEK, -1);
        assertEquals(9, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 32);
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.APRIL);
        pt.set(pt.DAY_OF_MONTH, -1);
        assertEquals(pt.MARCH, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.DECEMBER + 1);
        assertEquals(5, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, -1);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(pt.DECEMBER, pt.get(pt.MONTH));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.DAY_OF_YEAR, 367);
        assertEquals(5, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.DAY_OF_YEAR));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.DAY_OF_YEAR, -1);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(364, pt.get(pt.DAY_OF_YEAR));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.WEEK_OF_YEAR, 54);
        pt.set(pt.DAY_OF_WEEK, pt.FRIDAY);
        assertEquals(5, pt.get(pt.YEAR));
      /* 2005 starts on a Saturday, so pretty much anything results in the
       * second week. */
        assertEquals(2, pt.get(pt.WEEK_OF_YEAR));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.WEEK_OF_YEAR, 0);
        pt.set(pt.DAY_OF_WEEK, pt.FRIDAY);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(52, pt.get(pt.WEEK_OF_YEAR));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 100);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        
        pt.set(pt.CENTURY, 21);
        pt.set(pt.YEAR, -10);
        assertEquals(20, pt.get(pt.CENTURY));
        assertEquals(90, pt.get(pt.YEAR));
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(2, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.WEEK_OF_MONTH, 5);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.WEEK_OF_MONTH, 5);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 5);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 5);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.DAY_OF_YEAR, 365);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.DAY_OF_YEAR));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.DAY_OF_YEAR, 365);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.WEEK_OF_YEAR, 53);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.WEEK_OF_YEAR, 53);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 0);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1000);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(pt.DECEMBER, pt.get(pt.MONTH));
        assertEquals(4, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, 21);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(3, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SATURDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 4);
        pt.set(pt.DAY_OF_WEEK, -20);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 3);
        pt.set(pt.DAY_OF_WEEK, -20);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(pt.DECEMBER, pt.get(pt.MONTH));
        assertEquals(4, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
    }
    
    /** Test of get method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testComplexSetNoYear() {
        System.out.println("testComplexSetNoYear");
        
        pt.set(pt.SECOND, 1);
        pt.set(pt.MILLISECOND, 1000);
        assertEquals(2, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MILLISECOND));
        
        pt.set(pt.SECOND, 10);
        pt.set(pt.MILLISECOND, -500);
        assertEquals(9, pt.get(pt.SECOND));
        assertEquals(500, pt.get(pt.MILLISECOND));
        
        pt.set(pt.MINUTE, 1);
        pt.set(pt.SECOND, 60);
        assertEquals(2, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        
        pt.set(pt.MINUTE, 10);
        pt.set(pt.SECOND, -10);
        assertEquals(9, pt.get(pt.MINUTE));
        assertEquals(50, pt.get(pt.SECOND));
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 60);
        assertEquals(2, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR_OF_DAY, 10);
        pt.set(pt.MINUTE, -10);
        assertEquals(9, pt.get(pt.HOUR_OF_DAY));
        assertEquals(50, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 1);
        pt.set(pt.MINUTE, 60);
        assertEquals(2, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 10);
        pt.set(pt.MINUTE, -10);
        assertEquals(9, pt.get(pt.HOUR));
        assertEquals(50, pt.get(pt.MINUTE));
        
        // Make sure that noon/midnight is handled correctly
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 12);
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 13);
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(1, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, -10);
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(2, pt.get(pt.HOUR));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.HOUR_OF_DAY, 24);
        assertEquals(2, pt.get(pt.DAY_OF_MONTH));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 10);
        pt.set(pt.HOUR_OF_DAY, -10);
        assertEquals(9, pt.get(pt.DAY_OF_MONTH));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.AM_PM, pt.PM + 1);
        assertEquals(2, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 10);
        pt.set(pt.AM_PM, -1);
        assertEquals(9, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.set(pt.HOUR_OF_DAY, 24);
        assertEquals(pt.MONDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.HOUR_OF_DAY, -10);
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.set(pt.AM_PM, pt.PM + 1);
        assertEquals(pt.MONDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.AM_PM, -1);
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.set(pt.HOUR_OF_DAY, 24);
        assertEquals(2, pt.get(pt.DAY_OF_YEAR));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.DAY_OF_YEAR, 10);
        pt.set(pt.HOUR_OF_DAY, -10);
        assertEquals(9, pt.get(pt.DAY_OF_YEAR));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.set(pt.AM_PM, pt.PM + 1);
        assertEquals(2, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_YEAR, 10);
        pt.set(pt.AM_PM, -1);
        assertEquals(9, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(2, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, -1);
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.WEEK_OF_YEAR, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(2, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.WEEK_OF_YEAR, 10);
        pt.set(pt.DAY_OF_WEEK, -1);
        assertEquals(9, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(2, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 1001);
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        /* From here on, none of these are resolvable because no year is set. */
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SATURDAY + 1, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, -3);
        assertEquals(2, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(-3, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SATURDAY + 1, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, -4);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY + 1);
        assertEquals(-4, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(pt.SATURDAY + 1, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, -2);
        pt.set(pt.DAY_OF_WEEK, -1);
        assertEquals(-2, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(-1, pt.get(pt.DAY_OF_WEEK));
        
      /* These last few are partially resolvable.  They do not set the other
       * date fields, but they do resolve themselves. */
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 32);
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 61);
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_MONTH, 32);
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_MONTH, 63);
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.APRIL);
        pt.set(pt.DAY_OF_MONTH, -1);
        assertEquals(pt.MARCH, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.MARCH);
        pt.set(pt.DAY_OF_MONTH, -5);
        assertEquals(pt.MARCH, pt.get(pt.MONTH));
        assertEquals(-5, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_MONTH, -1);
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, -31);
        assertEquals(pt.NOVEMBER, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        assertEquals(-1, pt.getModifier(pt.YEAR));
        
        pt.setModifier(pt.YEAR, 0);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, -310);
        assertEquals(pt.MARCH, pt.get(pt.MONTH));
        assertEquals(-4, pt.get(pt.DAY_OF_MONTH));
        assertEquals(-1, pt.getModifier(pt.YEAR));
    }
    
    /** Test of add method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testComplexAdd() {
        System.out.println("testComplexAdd");
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        
        pt.set(pt.SECOND, 1);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 1);
        assertEquals(2, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MILLISECOND));
        
        pt.set(pt.SECOND, 10);
        pt.set(pt.MILLISECOND, 0);
        pt.add(pt.MILLISECOND, -500);
        assertEquals(9, pt.get(pt.SECOND));
        assertEquals(500, pt.get(pt.MILLISECOND));
        
        pt.set(pt.MINUTE, 1);
        pt.set(pt.SECOND, 59);
        pt.add(pt.SECOND, 1);
        assertEquals(2, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        
        pt.set(pt.MINUTE, 10);
        pt.set(pt.SECOND, 0);
        pt.add(pt.SECOND, -10);
        assertEquals(9, pt.get(pt.MINUTE));
        assertEquals(50, pt.get(pt.SECOND));
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 59);
        pt.add(pt.MINUTE, 1);
        assertEquals(2, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR_OF_DAY, 10);
        pt.set(pt.MINUTE, 0);
        pt.add(pt.MINUTE, -10);
        assertEquals(9, pt.get(pt.HOUR_OF_DAY));
        assertEquals(50, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 1);
        pt.set(pt.MINUTE, 59);
        pt.add(pt.MINUTE, 1);
        assertEquals(2, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 10);
        pt.set(pt.MINUTE, 0);
        pt.add(pt.MINUTE, -10);
        assertEquals(9, pt.get(pt.HOUR));
        assertEquals(50, pt.get(pt.MINUTE));
        
        // Make sure that noon/midnight is handled correctly
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.add(pt.HOUR, 1);
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.add(pt.HOUR, 2);
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(1, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 1);
        pt.add(pt.HOUR, -11);
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(2, pt.get(pt.HOUR));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.add(pt.HOUR_OF_DAY, 1);
        assertEquals(2, pt.get(pt.DAY_OF_MONTH));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 10);
        pt.set(pt.HOUR_OF_DAY, 0);
        pt.add(pt.HOUR_OF_DAY, -10);
        assertEquals(9, pt.get(pt.DAY_OF_MONTH));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.AM_PM, pt.PM);
        pt.add(pt.AM_PM, 1);
        assertEquals(2, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 10);
        pt.set(pt.AM_PM, pt.AM);
        pt.add(pt.AM_PM, -1);
        assertEquals(9, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.add(pt.HOUR_OF_DAY, 1);
        assertEquals(pt.MONDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.add(pt.HOUR_OF_DAY, -11);
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.add(pt.AM_PM, 1);
        assertEquals(pt.MONDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.AM_PM, pt.AM);
        pt.add(pt.AM_PM, -1);
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.add(pt.HOUR_OF_DAY, 1);
        assertEquals(2, pt.get(pt.DAY_OF_YEAR));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.DAY_OF_YEAR, 10);
        pt.set(pt.HOUR_OF_DAY, 0);
        pt.add(pt.HOUR_OF_DAY, -10);
        assertEquals(9, pt.get(pt.DAY_OF_YEAR));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.set(pt.AM_PM, pt.PM);
        pt.add(pt.AM_PM, 1);
        assertEquals(2, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_YEAR, 10);
        pt.set(pt.AM_PM, pt.AM);
        pt.add(pt.AM_PM, -1);
        assertEquals(9, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(2, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -2);
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -4);
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.WEDNESDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(2, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, -4);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(2, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, -2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -2);
        assertEquals(3, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.WEEK_OF_YEAR, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(2, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.WEEK_OF_YEAR, 10);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -2);
        assertEquals(9, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.add(pt.DAY_OF_MONTH, 1);
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.APRIL);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.add(pt.DAY_OF_MONTH, -2);
        assertEquals(pt.MARCH, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.add(pt.MONTH, 1);
        assertEquals(5, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.add(pt.MONTH, -1);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(pt.DECEMBER, pt.get(pt.MONTH));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.DAY_OF_YEAR, 366);
        pt.add(pt.DAY_OF_YEAR, 1);
        assertEquals(5, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.DAY_OF_YEAR));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.add(pt.DAY_OF_YEAR, -2);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(364, pt.get(pt.DAY_OF_YEAR));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.DAY_OF_WEEK, pt.FRIDAY);
        pt.set(pt.WEEK_OF_YEAR, 53);
        pt.add(pt.WEEK_OF_YEAR, 1);
        assertEquals(5, pt.get(pt.YEAR));
      /* 2005 starts on a Saturday, so pretty much anything results in the
       * second week. */
        assertEquals(2, pt.get(pt.WEEK_OF_YEAR));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.DAY_OF_WEEK, pt.FRIDAY);
        pt.set(pt.WEEK_OF_YEAR, 1);
        pt.add(pt.WEEK_OF_YEAR, -1);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(52, pt.get(pt.WEEK_OF_YEAR));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.add(pt.YEAR, 1);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        
        pt.set(pt.CENTURY, 21);
        pt.set(pt.YEAR, 0);
        pt.add(pt.YEAR, -10);
        assertEquals(20, pt.get(pt.CENTURY));
        assertEquals(90, pt.get(pt.YEAR));
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(2, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.WEEK_OF_MONTH, 5);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.WEEK_OF_MONTH, 5);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 5);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 5);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.DAY_OF_YEAR, 365);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.DAY_OF_YEAR));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.DAY_OF_YEAR, 365);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.WEEK_OF_YEAR, 53);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 99);
        pt.set(pt.WEEK_OF_YEAR, 53);
        pt.set(pt.DAY_OF_WEEK, pt.THURSDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(21, pt.get(pt.CENTURY));
        assertEquals(0, pt.get(pt.YEAR));
        assertEquals(1, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 0);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 1);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(pt.DECEMBER, pt.get(pt.MONTH));
        assertEquals(4, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MILLISECOND));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 14);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(3, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SATURDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 4);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -21);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 3);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -21);
        assertEquals(3, pt.get(pt.YEAR));
        assertEquals(pt.DECEMBER, pt.get(pt.MONTH));
        assertEquals(4, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        /* Test day pinning. */
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.add(pt.MONTH, 1);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(29, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.MARCH);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.add(pt.MONTH, 1);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(pt.APRIL, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.DAY_OF_YEAR, 366);
        pt.add(pt.YEAR, 1);
        assertEquals(5, pt.get(pt.YEAR));
        assertEquals(365, pt.get(pt.DAY_OF_YEAR));
        
        /* Test day not pinning. */
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_MONTH, 29);
        pt.add(pt.MONTH, 1);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(pt.MARCH, pt.get(pt.MONTH));
        assertEquals(29, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        pt.set(pt.MONTH, pt.APRIL);
        pt.set(pt.DAY_OF_MONTH, 30);
        pt.add(pt.MONTH, 1);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(pt.MAY, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 3);
        pt.set(pt.DAY_OF_YEAR, 365);
        pt.add(pt.YEAR, 1);
        assertEquals(4, pt.get(pt.YEAR));
        assertEquals(365, pt.get(pt.DAY_OF_YEAR));
    }
    
    /** Test of add method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testComplexAddNoYear() {
        System.out.println("testComplexAddNoYear");
        
        pt.set(pt.SECOND, 1);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 1);
        assertEquals(2, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MILLISECOND));
        
        pt.set(pt.SECOND, 10);
        pt.set(pt.MILLISECOND, 0);
        pt.add(pt.MILLISECOND, -500);
        assertEquals(9, pt.get(pt.SECOND));
        assertEquals(500, pt.get(pt.MILLISECOND));
        
        pt.set(pt.MINUTE, 1);
        pt.set(pt.SECOND, 59);
        pt.add(pt.SECOND, 1);
        assertEquals(2, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        
        pt.set(pt.MINUTE, 10);
        pt.set(pt.SECOND, 0);
        pt.add(pt.SECOND, -10);
        assertEquals(9, pt.get(pt.MINUTE));
        assertEquals(50, pt.get(pt.SECOND));
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 59);
        pt.add(pt.MINUTE, 1);
        assertEquals(2, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR_OF_DAY, 10);
        pt.set(pt.MINUTE, 0);
        pt.add(pt.MINUTE, -10);
        assertEquals(9, pt.get(pt.HOUR_OF_DAY));
        assertEquals(50, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 1);
        pt.set(pt.MINUTE, 59);
        pt.add(pt.MINUTE, 1);
        assertEquals(2, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 10);
        pt.set(pt.MINUTE, 0);
        pt.add(pt.MINUTE, -10);
        assertEquals(9, pt.get(pt.HOUR));
        assertEquals(50, pt.get(pt.MINUTE));
        
        // Make sure that noon/midnight is handled correctly
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.add(pt.HOUR, 1);
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.add(pt.HOUR, 2);
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(1, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.PM);
        pt.set(pt.HOUR, 1);
        pt.add(pt.HOUR, -11);
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(2, pt.get(pt.HOUR));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.add(pt.HOUR_OF_DAY, 1);
        assertEquals(2, pt.get(pt.DAY_OF_MONTH));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 10);
        pt.set(pt.HOUR_OF_DAY, 0);
        pt.add(pt.HOUR_OF_DAY, -10);
        assertEquals(9, pt.get(pt.DAY_OF_MONTH));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.AM_PM, pt.PM);
        pt.add(pt.AM_PM, 1);
        assertEquals(2, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 10);
        pt.set(pt.AM_PM, pt.AM);
        pt.add(pt.AM_PM, -1);
        assertEquals(9, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.add(pt.HOUR_OF_DAY, 1);
        assertEquals(pt.MONDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.add(pt.HOUR_OF_DAY, -11);
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.set(pt.AM_PM, pt.PM);
        pt.add(pt.AM_PM, 1);
        assertEquals(pt.MONDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.set(pt.AM_PM, pt.AM);
        pt.add(pt.AM_PM, -1);
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.add(pt.HOUR_OF_DAY, 1);
        assertEquals(2, pt.get(pt.DAY_OF_YEAR));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.DAY_OF_YEAR, 10);
        pt.set(pt.HOUR_OF_DAY, 0);
        pt.add(pt.HOUR_OF_DAY, -10);
        assertEquals(9, pt.get(pt.DAY_OF_YEAR));
        assertEquals(14, pt.get(pt.HOUR_OF_DAY));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.set(pt.AM_PM, pt.PM);
        pt.add(pt.AM_PM, 1);
        assertEquals(2, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_YEAR, 10);
        pt.set(pt.AM_PM, pt.AM);
        pt.add(pt.AM_PM, -1);
        assertEquals(9, pt.get(pt.DAY_OF_YEAR));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(2, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -2);
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.WEEK_OF_YEAR, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(2, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.WEEK_OF_YEAR, 10);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -2);
        assertEquals(9, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(pt.FRIDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(2, pt.get(pt.HOUR_OF_DAY));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.set(pt.MINUTE, 59);
        pt.set(pt.SECOND, 59);
        pt.set(pt.MILLISECOND, 999);
        pt.add(pt.MILLISECOND, 2);
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(1, pt.get(pt.MILLISECOND));
        
        /* From here on, these are not resolvable. */
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SATURDAY + 1, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -4);
        assertEquals(2, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(-3, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, 1);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(1, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.SATURDAY + 1, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, -4);
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.add(pt.DAY_OF_WEEK, 1);
        assertEquals(-4, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(pt.SATURDAY + 1, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.FEBRUARY);
        pt.set(pt.DAY_OF_WEEK_IN_MONTH, -2);
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.add(pt.DAY_OF_WEEK, -2);
        assertEquals(-2, pt.get(pt.DAY_OF_WEEK_IN_MONTH));
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(-1, pt.get(pt.DAY_OF_WEEK));
        
      /* These are partially resolvable.  They can resolve the day of month, but
       * not the other date fields. */
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.add(pt.DAY_OF_MONTH, 1);
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.APRIL);
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.add(pt.DAY_OF_MONTH, -2);
        assertEquals(pt.MARCH, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        /* Test day pinning. */
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.add(pt.MONTH, 1);
        assertEquals(pt.FEBRUARY, pt.get(pt.MONTH));
        assertEquals(31, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.MONTH, pt.MARCH);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.add(pt.MONTH, 1);
        assertEquals(pt.APRIL, pt.get(pt.MONTH));
        assertEquals(30, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.YEAR, 4);
        pt.set(pt.DAY_OF_YEAR, 366);
        pt.add(pt.YEAR, 1);
        assertEquals(5, pt.get(pt.YEAR));
        assertEquals(366, pt.get(pt.DAY_OF_YEAR));
    }
    
    /** Test of roll method, of class org.ggf.drmaa.PartialTimestamp. */
    public void testRoll() {
        System.out.println("testRoll");
        
        pt.set(pt.CENTURY, 20);
        pt.set(pt.YEAR, 4);
        
        pt.set(pt.SECOND, 0);
        pt.set(pt.MILLISECOND, 999);
        pt.roll(pt.MILLISECOND, 1);
        assertEquals(0, pt.get(pt.MILLISECOND));
        assertEquals(0, pt.get(pt.SECOND));
        
        pt.set(pt.MILLISECOND, 0);
        pt.roll(pt.MILLISECOND, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(0, pt.get(pt.SECOND));
        
        pt.set(pt.MINUTE, 0);
        pt.set(pt.SECOND, 59);
        pt.roll(pt.SECOND, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(0, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.SECOND, 0);
        pt.roll(pt.SECOND, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MINUTE));
        
        pt.set(pt.HOUR, 1);
        pt.set(pt.MINUTE, 59);
        pt.roll(pt.MINUTE, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(0, pt.get(pt.MINUTE));
        assertEquals(1, pt.get(pt.HOUR));
        
        pt.set(pt.MINUTE, 0);
        pt.roll(pt.MINUTE, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(1, pt.get(pt.HOUR));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.set(pt.HOUR, 11);
        pt.roll(pt.HOUR, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.HOUR, 12);
        pt.roll(pt.HOUR, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(1, pt.get(pt.HOUR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.HOUR, 1);
        pt.roll(pt.HOUR, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.set(pt.AM_PM, pt.PM);
        pt.roll(pt.AM_PM, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(pt.AM, pt.get(pt.AM_PM));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.AM_PM, pt.AM);
        pt.roll(pt.AM_PM, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(12, pt.get(pt.HOUR));
        assertEquals(pt.PM, pt.get(pt.AM_PM));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.HOUR_OF_DAY, 23);
        pt.roll(pt.HOUR_OF_DAY, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(0, pt.get(pt.HOUR_OF_DAY));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.HOUR_OF_DAY, 0);
        pt.roll(pt.HOUR_OF_DAY, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        
        pt.set(pt.DAY_OF_WEEK, pt.SATURDAY);
        pt.roll(pt.DAY_OF_WEEK, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(pt.SUNDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.DAY_OF_WEEK, pt.SUNDAY);
        pt.roll(pt.DAY_OF_WEEK, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(pt.SATURDAY, pt.get(pt.DAY_OF_WEEK));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.DAY_OF_MONTH, 31);
        pt.roll(pt.DAY_OF_MONTH, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(1, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        
        pt.set(pt.DAY_OF_MONTH, 1);
        pt.roll(pt.DAY_OF_MONTH, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(31, pt.get(pt.DAY_OF_MONTH));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        
        pt.set(pt.DAY_OF_YEAR, 366);
        pt.roll(pt.DAY_OF_YEAR, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(1, pt.get(pt.DAY_OF_YEAR));
        assertEquals(4, pt.get(pt.YEAR));
        
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.roll(pt.DAY_OF_YEAR, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(366, pt.get(pt.DAY_OF_YEAR));
        assertEquals(4, pt.get(pt.YEAR));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.set(pt.WEEK_OF_MONTH, 5);
        pt.roll(pt.WEEK_OF_MONTH, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(1, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        
        pt.set(pt.WEEK_OF_MONTH, 1);
        pt.roll(pt.WEEK_OF_MONTH, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(5, pt.get(pt.WEEK_OF_MONTH));
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        
        pt.set(pt.WEEK_OF_YEAR, 53);
        pt.roll(pt.WEEK_OF_YEAR, 1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(1, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(4, pt.get(pt.YEAR));
        
        pt.set(pt.WEEK_OF_YEAR, 1);
        pt.roll(pt.WEEK_OF_YEAR, -1);
        assertEquals(999, pt.get(pt.MILLISECOND));
        assertEquals(59, pt.get(pt.SECOND));
        assertEquals(59, pt.get(pt.MINUTE));
        assertEquals(23, pt.get(pt.HOUR_OF_DAY));
        assertEquals(53, pt.get(pt.WEEK_OF_YEAR));
        assertEquals(4, pt.get(pt.YEAR));
        
        pt.set(pt.MONTH, pt.DECEMBER);
        pt.roll(pt.MONTH, 1);
        assertEquals(pt.JANUARY, pt.get(pt.MONTH));
        
        pt.set(pt.MONTH, pt.JANUARY);
        pt.roll(pt.MONTH, -1);
        assertEquals(pt.DECEMBER, pt.get(pt.MONTH));
        
        pt.set(pt.YEAR, 99);
      /* We set day of year to prevent any funny interactions with other date
       * fields. */
        pt.set(pt.DAY_OF_YEAR, 1);
        pt.roll(pt.YEAR, 1);
        assertEquals(0, pt.get(pt.YEAR));
        
        pt.set(pt.YEAR, 0);
        pt.roll(pt.YEAR, -1);
        assertEquals(99, pt.get(pt.YEAR));
        
        pt.set(pt.CENTURY, 20);
        
        try {
            pt.roll(pt.CENTURY, 1);
            fail("Allowed rolling of century");
        } catch (IllegalArgumentException e) {
            /* Don't care */
        }
        
        pt.set(pt.ZONE_OFFSET, 20);
        
        try {
            pt.roll(pt.ZONE_OFFSET, 1);
            fail("Allowed rolling of timezone offset");
        } catch (IllegalArgumentException e) {
            /* Don't care */
        }
    }
    
    public void testEquals() {
        System.out.println("testEquals");
        
        PartialTimestamp pt1 = new PartialTimestamp();
        PartialTimestamp pt2 = new PartialTimestamp();
        
        pt1.set(pt.HOUR_OF_DAY, 1);
        assertFalse(pt1.equals(pt2));
        assertFalse(pt2.equals(pt1));
        
        pt2.set(pt.HOUR_OF_DAY, 1);
        assertTrue(pt1.equals(pt2));
        assertTrue(pt2.equals(pt1));
        
        pt1.set(pt.CENTURY, 20);
        assertFalse(pt1.equals(pt2));
        assertFalse(pt2.equals(pt1));
        
        pt2.set(pt.CENTURY, 20);
        assertTrue(pt1.equals(pt2));
        assertTrue(pt2.equals(pt1));
        
        pt1.setModifier(pt.YEAR, 1);
        assertFalse(pt1.equals(pt2));
        assertFalse(pt2.equals(pt1));
        
        pt2.setModifier(pt.YEAR, 1);
        assertTrue(pt1.equals(pt2));
        assertTrue(pt2.equals(pt1));
        
        assertFalse(pt1.equals("20 01"));
        assertFalse(pt2.equals(Calendar.getInstance()));
    }
    
    public void testClone() {
        System.out.println("testClone");
        
        PartialTimestamp pt1 = new PartialTimestamp(4, 11, 24, 2, 25);
        PartialTimestamp pt2 = (PartialTimestamp)pt1.clone();
        
        assertEquals(pt1, pt2);
    }
    
    public void testHashCode() {
        System.out.println("testHashCode");
        
        PartialTimestamp pt1 = new PartialTimestamp(4, 11, 24, 2, 25);
        PartialTimestamp pt2 = new PartialTimestamp(4, 11, 24, 2, 25);
        
        assertEquals(pt1.hashCode(), pt2.hashCode());
    }
}
