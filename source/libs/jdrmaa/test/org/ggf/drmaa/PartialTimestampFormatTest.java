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
 * PartialTimestampFormatTest.java
 * JUnit based test
 *
 * Created on November 14, 2004, 12:49 AM
 */

package org.ggf.drmaa;

import java.text.*;

import junit.framework.*;

/**
 *
 * @author dan.templeton@sun.com
 */
public class PartialTimestampFormatTest extends TestCase {
    public PartialTimestampFormatTest(java.lang.String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(PartialTimestampFormatTest.class);
        return suite;
    }
    
    /** Test of format method, of class org.ggf.drmaa.PartialTimestampFormat. */
    public void testFormat() {
        System.out.println("testFormat");
        
        PartialTimestamp pt = new PartialTimestamp();
        PartialTimestampFormat ptf = new PartialTimestampFormat();
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 31);
        assertEquals("01:31", ptf.format(pt));
        
        pt.set(pt.DAY_OF_MONTH, 24);
        assertEquals("24 01:31", ptf.format(pt));
        
        pt.set(pt.MONTH, pt.NOVEMBER);
        assertEquals("11/24 01:31", ptf.format(pt));
        
        pt.set(pt.YEAR, 4);
        assertEquals("04/11/24 01:31", ptf.format(pt));
        
        pt.set(pt.CENTURY, 20);
        assertEquals("2004/11/24 01:31", ptf.format(pt));
        
        pt.set(pt.SECOND, 17);
        assertEquals("2004/11/24 01:31:17", ptf.format(pt));
        
        pt.set(pt.ZONE_OFFSET, -2 * 60 * 60 * 1000);
        assertEquals("2004/11/24 01:31:17 -02:00", ptf.format(pt));
        
        pt = new PartialTimestamp();
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 31);
        assertEquals("01:31", ptf.format(pt));
        
        pt.set(pt.SECOND, 17);
        assertEquals("01:31:17", ptf.format(pt));
        
        pt.set(pt.DAY_OF_MONTH, 24);
        assertEquals("24 01:31:17", ptf.format(pt));
        
        pt.set(pt.MONTH, pt.NOVEMBER);
        assertEquals("11/24 01:31:17", ptf.format(pt));
        
        pt.set(pt.YEAR, 4);
        assertEquals("04/11/24 01:31:17", ptf.format(pt));
        
        pt.set(pt.CENTURY, 20);
        assertEquals("2004/11/24 01:31:17", ptf.format(pt));
        
        pt.set(pt.ZONE_OFFSET, -2 * 60 * 60 * 1000);
        assertEquals("2004/11/24 01:31:17 -02:00", ptf.format(pt));
        
        pt = new PartialTimestamp();
        
        pt.set(pt.HOUR_OF_DAY, 1);
        pt.set(pt.MINUTE, 31);
        assertEquals("01:31", ptf.format(pt));
        
        pt.set(pt.ZONE_OFFSET, -2 * 60 * 60 * 1000);
        assertEquals("01:31 -02:00", ptf.format(pt));
        
        pt.set(pt.DAY_OF_MONTH, 24);
        assertEquals("24 01:31 -02:00", ptf.format(pt));
        
        pt.set(pt.MONTH, pt.NOVEMBER);
        assertEquals("11/24 01:31 -02:00", ptf.format(pt));
        
        pt.set(pt.YEAR, 4);
        assertEquals("04/11/24 01:31 -02:00", ptf.format(pt));
        
        pt.set(pt.CENTURY, 20);
        assertEquals("2004/11/24 01:31 -02:00", ptf.format(pt));
        
        pt.set(pt.SECOND, 17);
        assertEquals("2004/11/24 01:31:17 -02:00", ptf.format(pt));
    }
    
    /** Test of parse method, of class org.ggf.drmaa.PartialTimestampFormat. */
    public void testParse() throws ParseException {
        System.out.println("testParse");
        
        PartialTimestampFormat f = new PartialTimestampFormat();
        
        /* These should succeed. */
        PartialTimestamp pt = f.parse("2004/11/13 21:39:22 +01:00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 22, 60 * 60 * 1000);
        pt = f.parse("04/11/13 21:39:21 +01:00");
        this.checkPT(pt, pt.UNSET, 04, pt.NOVEMBER, 13, 21, 39, 21, 60 * 60 * 1000);
        pt = f.parse("11/13 21:39:21 +01:00");
        this.checkPT(pt, pt.UNSET, pt.UNSET, pt.NOVEMBER, 13, 21, 39, 21, 60 * 60 * 1000);
        pt = f.parse("13 21:39:21 +01:00");
        this.checkPT(pt, pt.UNSET, pt.UNSET, pt.UNSET, 13, 21, 39, 21, 60 * 60 * 1000);
        pt = f.parse("21:39:21 +01:00");
        this.checkPT(pt, pt.UNSET, pt.UNSET, pt.UNSET, pt.UNSET, 21, 39, 21, 60 * 60 * 1000);
        pt = f.parse("2004/11/13 21:39:21");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 21, pt.UNSET);
        pt = f.parse("2004/11/13 21:39 +01:00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, pt.UNSET, 60 * 60 * 1000);
        pt = f.parse("2004/11/13 21:39:21");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 21, pt.UNSET);
        pt = f.parse("21:39:21");
        this.checkPT(pt, pt.UNSET, pt.UNSET, pt.UNSET, pt.UNSET, 21, 39, 21, pt.UNSET);
        pt = f.parse("21:39 +01:00");
        this.checkPT(pt, pt.UNSET, pt.UNSET, pt.UNSET, pt.UNSET, 21, 39, pt.UNSET, 60 * 60 * 1000);
        pt = f.parse("21:39");
        this.checkPT(pt, pt.UNSET, pt.UNSET, pt.UNSET, pt.UNSET, 21, 39, pt.UNSET, pt.UNSET);
        pt = f.parse("     2004/11/13 21:39:21 +01:00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 21, 60 * 60 * 1000);
        pt = f.parse("2004/11/13 21:039:21 +01:00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 03, pt.UNSET, pt.UNSET);
        pt = f.parse("2004/11/13 21:39:201 +01:00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 20, pt.UNSET);
        pt = f.parse("2004/11/13 21:39:21 01:00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 21, pt.UNSET);
        pt = f.parse("2004/11/13 21:39:21 :00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 21, pt.UNSET);
        pt = f.parse("2004/11/13 21:39:21 00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 21, pt.UNSET);
        pt = f.parse("2004/11/13 21:39: +01:00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, pt.UNSET, pt.UNSET);
        pt = f.parse("2004/11/13 21:39:21 *01:00");
        this.checkPT(pt, 20, 04, pt.NOVEMBER, 13, 21, 39, 21, pt.UNSET);
        
        /* These should fail. */
        try {
            f.parse("12004/11/13 21:39:21 +01:00");
            fail("Allowed 12004/11/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/110/13 21:39:21 +01:00");
            fail("Allowed 2004/110/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/103 21:39:21 +01:00");
            fail("Allowed 2004/11/103 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 210:39:21 +01:00");
            fail("Allowed 2004/11/13 210:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:39:21 +001:00");
            fail("Allowed 2004/11/13 21:39:21 +001:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("204/11/13 21:39:21 +01:00");
            fail("Allowed 204/11/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/1/13 21:39:21 +01:00");
            fail("Allowed 2004/1/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/3 21:39:21 +01:00");
            fail("Allowed 2004/11/3 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 1:39:21 +01:00");
            fail("Allowed 2004/11/13 1:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:9:21 +01:00");
            fail("Allowed 2004/11/13 21:9:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:39:2 +01:00");
            fail("Allowed 2004/11/13 21:39:2 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:39:21 +1:00");
            fail("Allowed 2004/11/13 21:39:21 +1:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:39:21 +01:0");
            fail("Allowed 2004/11/13 21:39:21 +01:0");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:39:21 +01:");
            fail("Allowed 2004/11/13 21:39:21 +01:");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:39:21 +01");
            fail("Allowed 2004/11/13 21:39:21 +01");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:39:21 +0");
            fail("Allowed 2004/11/13 21:39:21 +0");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:39:21 +");
            fail("Allowed 2004/11/13 21:39:21 +");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21: +01:00");
            fail("Allowed 2004/11/13 21: +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 :21 +01:00");
            fail("Allowed 2004/11/13 :21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21 +01:00");
            fail("Allowed 2004/11/13 21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/ 21:39:21 +01:00");
            fail("Allowed 2004/11/ 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004// 21:39:21 +01:00");
            fail("Allowed 2004// 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("/11/13 21:39:21 +01:00");
            fail("Allowed /11/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004//13 21:39:21 +01:00");
            fail("Allowed 2004//13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("//13 21:39:21 +01:00");
            fail("Allowed //13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13/21:39:21 +01:00");
            fail("Allowed 2004/11/13/21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11:13 21:39:21 +01:00");
            fail("Allowed 2004/11:13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004:11/13 21:39:21 +01:00");
            fail("Allowed 2004:11/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004 11/13 21:39:21 +01:00");
            fail("Allowed 2004 11/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11 13 21:39:21 +01:00");
            fail("Allowed 2004/11 13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004|11/13 21:39:21 +01:00");
            fail("Allowed 2004|11/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21;39:21 +01:00");
            fail("Allowed 2004/11/13 21;39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("/2004/11/13 21:39:21 +01:00");
            fail("Allowed /2004/11/13 21:39:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
        try {
            f.parse("2004/11/13 21:3f:21 +01:00");
            fail("Allowed 2004/11/13 21:3f:21 +01:00");
        } catch (ParseException e) {
            /* Don't care. */
        }
    }
    
    private void checkPT(PartialTimestamp pt, int century, int year, int month,
            int date, int hour, int minute, int second,
            int offset) {
        assertEquals(century, pt.get(pt.CENTURY));
        assertEquals(year, pt.get(pt.YEAR));
        assertEquals(month, pt.get(pt.MONTH));
        assertEquals(date, pt.get(pt.DAY_OF_MONTH));
        assertEquals(hour, pt.get(pt.HOUR_OF_DAY));
        assertEquals(minute, pt.get(pt.MINUTE));
        assertEquals(second, pt.get(pt.SECOND));
        assertEquals(offset, pt.get(pt.ZONE_OFFSET));
    }
}
