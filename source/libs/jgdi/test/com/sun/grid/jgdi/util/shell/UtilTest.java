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

package com.sun.grid.jgdi.util.shell;

import junit.framework.TestCase;

public class UtilTest  extends TestCase {
    
    public UtilTest() {
    }
    
    protected void setUp() throws Exception {
        super.setUp();
    }
    
    public void testGetDateAndTimeAsString() {
        System.out.println("getDateAndTimeAsString");
        assertEquals("01/01/1970 00:00:00", Util.getDateAndTimeAsString(0));
        assertEquals("01/01/1970 00:00:01", Util.getDateAndTimeAsString(1));
        assertEquals("01/01/1970 00:10:01", Util.getDateAndTimeAsString(601));
        assertEquals("01/01/1970 02:10:01", Util.getDateAndTimeAsString(7801));
        assertEquals("01/02/1970 02:10:01", Util.getDateAndTimeAsString(94201));
        assertEquals("08/03/2023 05:04:23", Util.getDateAndTimeAsString(1691035463));
        assertEquals("04/03/2007 05:04:23", Util.getDateAndTimeAsString(1175573063));
        assertEquals("06/05/2007 04:04:00", Util.getDateAndTimeAsString(1181012640));
    } /* Test of getDateAndTimeAsString method, of class Util. */
    
    public void testGetDateTimeAsInt() {
        System.out.println("getDateTimeAsInt");
        assertEquals(0, Util.getDateTimeAsInt("197001010000"));
        assertEquals(1, Util.getDateTimeAsInt("197001010000.01"));
        assertEquals(601, Util.getDateTimeAsInt("197001010010.01"));
        assertEquals(7801, Util.getDateTimeAsInt("197001010210.01"));
        assertEquals(94201, Util.getDateTimeAsInt("197001020210.01"));
        assertEquals(1691035463, Util.getDateTimeAsInt("202307340504.23"));
        // assertEquals(1175573063, Util.getDateTimeAsInt("03340504.23"));
        // assertEquals(1181016240, Util.getDateTimeAsInt("06050504"));
    } /* Test of getDateTimeAsInt method, of class Util. */
    
    public void testGetTimeAsString() {
        System.out.println("getTimeAsString");
        assertEquals("00:00:25", Util.getTimeAsString(25));
        assertEquals("00:05:30", Util.getTimeAsString(330));
        assertEquals("02:10:11", Util.getTimeAsString(7811));
        assertEquals("26:10:11", Util.getTimeAsString(94211));
        assertEquals("242:10:11", Util.getTimeAsString(871811));
    } /* Test of getTimeAsString method, of class Util. */
    
    public void testGetTimeAsInt() {
        System.out.println("getTimeAsInt");
        assertEquals(25, Util.getTimeAsInt("25"));
        assertEquals(330, Util.getTimeAsInt("330"));
        assertEquals(330, Util.getTimeAsInt(":330"));
        assertEquals(330, Util.getTimeAsInt("::330"));
        assertEquals(300, Util.getTimeAsInt(":5:"));
        assertEquals(300, Util.getTimeAsInt("5:"));
        assertEquals(300, Util.getTimeAsInt("5:0"));
        assertEquals(300, Util.getTimeAsInt("5:00"));
        assertEquals(300, Util.getTimeAsInt("5:000"));
        assertEquals(300, Util.getTimeAsInt("0:5:"));
        assertEquals(300, Util.getTimeAsInt("0000:5:"));
        assertEquals(300, Util.getTimeAsInt("0000:5:0"));
        assertEquals(330, Util.getTimeAsInt("5:30"));
        assertEquals(330, Util.getTimeAsInt(":5:30"));
        assertEquals(330, Util.getTimeAsInt(":5:30"));
        assertEquals(7811, Util.getTimeAsInt("7811"));
        assertEquals(7811, Util.getTimeAsInt(":7811"));
        assertEquals(7811, Util.getTimeAsInt("::7811"));
        assertEquals(7811, Util.getTimeAsInt("130:11"));
        assertEquals(7811, Util.getTimeAsInt(":130:11"));
        assertEquals(7811, Util.getTimeAsInt("2::611"));
        assertEquals(7811, Util.getTimeAsInt("2:10:11"));
        assertEquals(94211, Util.getTimeAsInt("94211"));
        assertEquals(94211, Util.getTimeAsInt(":94211"));
        assertEquals(94211, Util.getTimeAsInt("1570:11"));
        assertEquals(94200, Util.getTimeAsInt("1570:"));
        assertEquals(94200, Util.getTimeAsInt("1570:0"));
        assertEquals(94200, Util.getTimeAsInt(":1570:"));
        assertEquals(94200, Util.getTimeAsInt("0000:1570:000"));
        assertEquals(94211, Util.getTimeAsInt(":1570:11"));
        assertEquals(93600, Util.getTimeAsInt("26::"));
        assertEquals(93600, Util.getTimeAsInt("26:0:"));
        assertEquals(93600, Util.getTimeAsInt("26::0"));
        assertEquals(93600, Util.getTimeAsInt("26:00:"));
        assertEquals(93600, Util.getTimeAsInt("26::00"));
        assertEquals(93660, Util.getTimeAsInt("26:1:"));
        assertEquals(93602, Util.getTimeAsInt("26::2"));
        assertEquals(4442400, Util.getTimeAsInt("1234::"));
        assertEquals(4442400, Util.getTimeAsInt(":74040:"));
        assertEquals(4442400, Util.getTimeAsInt("74040:"));
        assertEquals(4442400, Util.getTimeAsInt("70000:242400"));
        assertEquals(4442400, Util.getTimeAsInt(":70000:242400"));
        assertEquals(4442400, Util.getTimeAsInt("600:34000:242400"));
    } /* Test of getTimeAsInt method, of class Util. */
}
