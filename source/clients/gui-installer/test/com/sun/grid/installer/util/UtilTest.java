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
package com.sun.grid.installer.util;

import com.sun.grid.installer.gui.Host;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import junit.framework.JUnit4TestAdapter;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;

public class UtilTest {

    public UtilTest() {
    }

    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(UtilTest.class);
    }

    @BeforeClass
    public static void setUpClass() throws Exception {
    }

    @AfterClass
    public static void tearDownClass() throws Exception {
    }

    @Before
    public void setUp() {
    }

    @After
    public void tearDown() {
    }

    /**
     * Test of parseSinglePattern private method, of class Util.
     */
    @Test
    public void testParseSingleIpPattern1() {
        testParseSingleIpPattern("1[2 3 4 5]",Arrays.asList("12","13","14","15"));
    }

    @Test
    public void testParseSingleIpPattern2() {
        testParseSingleIpPattern("1[2-5]",Arrays.asList("12","13","14","15"));
    }

    @Test
    public void testParseSingleIpPattern3() {
        testParseSingleIpPattern("1[2-5]3",Arrays.asList("123","133","143","153"));
    }

    @Test
    public void testParseSingleIpPattern4() {
        testParseSingleIpPattern("1[2-5][3 8]",Arrays.asList("123","128","133","138","143","148","153","158"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseSingleIpPattern5() {
        List<String> exp = new ArrayList<String>();
        for (int i=12; i<=19; i++) exp.add(String.valueOf(i));
        for (int i=110; i<=158; i++) exp.add(String.valueOf(i));
        testParseSingleIpPattern("1[2-5[3 8]]",exp);
    }

    @Test
    public void testParseSingleIpPattern6() {
        List<String> exp = new ArrayList<String>();
        for (int i=12; i<=19; i++) exp.add(String.valueOf(i));
        for (int i=110; i<=158; i++) exp.add(String.valueOf(i));
        testParseSingleIpPattern("[12-19 110-158]",exp);
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseSingleIpPattern7() {
        List<String> exp = new ArrayList<String>();
        for (int i=25; i<=95; i++) exp.add(String.valueOf(i));
        for (int i=233; i<=255; i++) exp.add(String.valueOf(i));
        testParseSingleIpPattern("[2-9[5 3[3-5]]]",exp);
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseSingleIpPattern8() {
        testParseSingleIpPattern("[12 19[5 11[3-5]]]",Arrays.asList("12", "195", "19113", "19114", "19115"));
    }

    @Test
    public void testParseSingleIpPattern9() {
        List<String> exp = new ArrayList<String>();
        for (int i=12; i<=19; i++) exp.add(String.valueOf(i));
        for (int i=110; i<=158; i++) exp.add(String.valueOf(i));
        testParseSingleIpPattern("12-19 110-158",exp);
    }

    @Test
    public void testParseSingleIpPattern10() {
        testParseSingleIpPattern("1[2,3,4,5]",Arrays.asList("12","13","14","15"));
    }

    @Test
    public void testParseSingleIpPattern11() {
        testParseSingleIpPattern("1[2,3 4,5 6]",Arrays.asList("12","13","14","15","16"));
    }

    @Test
    public void testParseSingleIpPattern12() {
        testParseSingleIpPattern("1 [2-5]",Arrays.asList("1","2","3","4","5"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseSingleIpPattern13() {
        testParseSingleIpPattern("[1 [2-5]]",Arrays.asList("1","2","3","4","5"));
    }

    @Test
    public void testParseSingleIpPattern14() {
        testParseSingleIpPattern("1[2-5][3,8]",Arrays.asList("123","128","133","138","143","148","153","158"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseSingleIpPattern15() {
        List<String> exp = new ArrayList<String>();
        for (int i=12; i<=19; i++) exp.add(String.valueOf(i));
        for (int i=110; i<=158; i++) exp.add(String.valueOf(i));
        testParseSingleIpPattern("1[2-5[3,8]]",exp);
    }

    @Test
    public void testParseSingleIpPattern16() {
        List<String> exp = new ArrayList<String>();
        for (int i=12; i<=19; i++) exp.add(String.valueOf(i));
        for (int i=110; i<=158; i++) exp.add(String.valueOf(i));
        testParseSingleIpPattern("[12-19,110-158]",exp);
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseSingleIpPattern17() {
        testParseSingleIpPattern("[1,2,3 [4-5] 11,[15-19]]", Arrays.asList("1","2","3","4","5","11","15","16","17","18","19"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseSingleIpPattern18() {
        testParseSingleIpPattern("[12,19[5,11[3-5]]]", Arrays.asList("12", "195", "19113", "19114", "19115"));
    }

    @Test
    public void testParseSingleIpPattern19() {
        List<String> exp = new ArrayList<String>();
        for (int i=12; i<=19; i++) exp.add(String.valueOf(i));
        for (int i=110; i<=158; i++) exp.add(String.valueOf(i));
        testParseSingleIpPattern("12-19,110-158",exp);
    }

    /* Testing private method parseSinglePattern */
    private void testParseSingleIpPattern(String input, List<String> expResult) {
        System.out.println("parseSingleIpPattern: "+input);
        Util util = new Util();
        Object result = null;
        for (Method m : Util.class.getDeclaredMethods()) {
            if (m.getName().equals("parseSinglePattern")) {
                final Object params[] = {input, Host.Type.IP};
                m.setAccessible(true);
                try {
                    result = m.invoke(util, params);
                } catch (IllegalAccessException ex) {
                    Logger.getLogger(UtilTest.class.getName()).log(Level.SEVERE, null, ex);
                } catch (IllegalArgumentException ex) {
                    Logger.getLogger(UtilTest.class.getName()).log(Level.SEVERE, null, ex);
                } catch (InvocationTargetException ex) {
                    Throwable t = ex.getCause();
                    //parseSinglePattern will throw IllegalArgumentException as cause for this invocation when parsing error occurs
                    if (t.getClass().equals(IllegalArgumentException.class)) {
                        throw new IllegalArgumentException(t);
                    }
                    Logger.getLogger(UtilTest.class.getName()).log(Level.SEVERE, null, ex);
                }
                break;
            }
        }
        assertNotNull("result can't be null", result);
        assertEquals(expResult, (List<String>) result);
    }


    /**
     * Test of parseIpPattern public method, of class Util.
     */
    @Test
    public void testParseIpPattern1() {
        testParseIpPattern("192.168.0.1", Arrays.asList("192.168.0.1"));
    }

    @Test
    public void testParseIpPattern2() {
        List<String> exp = new ArrayList<String>();
        for (int i=0; i<=9; i++) exp.add("192.168."+String.valueOf(i)+".1");
        testParseIpPattern("192.168.0-9.1", exp);
    }

    @Test
    public void testParseIpPattern3() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("192.[168 12 53].0.1",  Arrays.asList("192.168.0.1", "192.12.0.1", "192.53.0.1"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseIpPattern4() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("10 11 12.[[2-5] 152 163].2-4.1",  Arrays.asList("10.2.2.1", "10.2.3.1", "10.2.4.1", "10.3.2.1", "10.3.3.1", "10.3.4.1", "10.4.2.1", "10.4.3.1", "10.4.4.1", "10.5.2.1", "10.5.3.1", "10.5.4.1", "10.152.2.1", "10.152.3.1", "10.152.4.1", "10.163.2.1", "10.163.3.1", "10.163.4.1", "11.2.2.1", "11.2.3.1", "11.2.4.1", "11.3.2.1", "11.3.3.1", "11.3.4.1", "11.4.2.1", "11.4.3.1", "11.4.4.1", "11.5.2.1", "11.5.3.1", "11.5.4.1", "11.152.2.1", "11.152.3.1", "11.152.4.1", "11.163.2.1", "11.163.3.1", "11.163.4.1", "12.2.2.1", "12.2.3.1", "12.2.4.1", "12.3.2.1", "12.3.3.1", "12.3.4.1", "12.4.2.1", "12.4.3.1", "12.4.4.1", "12.5.2.1", "12.5.3.1", "12.5.4.1", "12.152.2.1", "12.152.3.1", "12.152.4.1", "12.163.2.1", "12.163.3.1", "12.163.4.1"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseIpPattern5() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("10 11 568 12.[[2-5] 152 163].2-4.1",  Arrays.asList("10.2.2.1", "10.2.3.1", "10.2.4.1", "10.3.2.1", "10.3.3.1", "10.3.4.1", "10.4.2.1", "10.4.3.1", "10.4.4.1", "10.5.2.1", "10.5.3.1", "10.5.4.1", "10.152.2.1", "10.152.3.1", "10.152.4.1", "10.163.2.1", "10.163.3.1", "10.163.4.1", "11.2.2.1", "11.2.3.1", "11.2.4.1", "11.3.2.1", "11.3.3.1", "11.3.4.1", "11.4.2.1", "11.4.3.1", "11.4.4.1", "11.5.2.1", "11.5.3.1", "11.5.4.1", "11.152.2.1", "11.152.3.1", "11.152.4.1", "11.163.2.1", "11.163.3.1", "11.163.4.1", "255.2.2.1", "255.2.3.1", "255.2.4.1", "255.3.2.1", "255.3.3.1", "255.3.4.1", "255.4.2.1", "255.4.3.1", "255.4.4.1", "255.5.2.1", "255.5.3.1", "255.5.4.1", "255.152.2.1", "255.152.3.1", "255.152.4.1", "255.163.2.1", "255.163.3.1", "255.163.4.1", "12.2.2.1", "12.2.3.1", "12.2.4.1", "12.3.2.1", "12.3.3.1", "12.3.4.1", "12.4.2.1", "12.4.3.1", "12.4.4.1", "12.5.2.1", "12.5.3.1", "12.5.4.1", "12.152.2.1", "12.152.3.1", "12.152.4.1", "12.163.2.1", "12.163.3.1", "12.163.4.1"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseIpPattern6() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("10.18.a4.5",  Arrays.asList(""));
    }
    
    @Test (expected=IllegalArgumentException.class)
    public void testParseIpPattern7() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("10.18_4.5",  Arrays.asList(""));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseIpPattern8() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("10.11.12",  Arrays.asList(""));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseIpPattern9() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("10.11.12.13.14",  Arrays.asList(""));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseIpPattern10() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("10.11.12.13.14",  Arrays.asList(""));
    }

    @Test
    public void testParseIpPattern11() {
        List<String> exp = new ArrayList<String>();
        testParseIpPattern("10.11.12.13-14",  Arrays.asList("10.11.12.13", "10.11.12.14"));
    }

    private void testParseIpPattern(String input, List<String> expResult) {
        System.out.println("parseIpPattern: "+input);
        assertEquals(expResult, Util.parseIpPattern(input));
    }

    /**
     * Test of parseIpPattern public method, of class Util.
     */
    @Test
    public void testParseHostPattern1() {
        testParseHostPattern("GRID00", Arrays.asList("grid00"));
    }

    @Test
    public void testParseHostPattern2() {
        testParseHostPattern("grID[00-10]", Arrays.asList("grid00", "grid01", "grid02","grid03", "grid04", "grid05","grid06", "grid07", "grid08", "grid09", "grid10"));
    }

    @Test
    public void testParseHostPattern3() {
        testParseHostPattern("Gr[i o u]d",  Arrays.asList("grid", "grod", "grud"));
    }

    @Test
    public void testParseHostPattern4() {
        testParseHostPattern("gR[i o u]d[0-3]",  Arrays.asList("grid0", "grid1", "grid2", "grid3","grod0", "grod1", "grod2", "grod3","grud0", "grud1", "grud2", "grud3"));
    }

    @Test
    public void testParseHostPattern5() {
        testParseHostPattern("gR[i o u]d[0000-0013]",  Arrays.asList("grid0000", "grid0001", "grid0002", "grid0003","grid0004", "grid0005", "grid0006", "grid0007","grid0008", "grid0009", "grid0010", "grid0011","grid0012", "grid0013","grod0000", "grod0001", "grod0002", "grod0003","grod0004", "grod0005", "grod0006", "grod0007","grod0008", "grod0009", "grod0010", "grod0011","grod0012", "grod0013","grud0000", "grud0001", "grud0002", "grud0003","grud0004", "grud0005", "grud0006", "grud0007","grud0008", "grud0009", "grud0010", "grud0011","grud0012", "grud0013"));
    }

    @Test
    public void testParseHostPattern6() {
        //TODO: decide if this range should be invalid
        testParseHostPattern("gRId[0000-13]",  Arrays.asList("grid0000", "grid0001", "grid0002", "grid0003","grid0004", "grid0005", "grid0006", "grid0007","grid0008", "grid0009", "grid0010", "grid0011","grid0012", "grid0013"));
    }

    @Test
    public void testParseHostPattern7() {
        testParseHostPattern("gRId00-01",  Arrays.asList("grid00-01"));
    }

    @Test
    public void testParseHostPattern8() {
        testParseHostPattern("gRId00-01 h0003",  Arrays.asList("grid00-01","h0003"));
    }

    @Test
    public void testParseHostPattern9() {
        testParseHostPattern("gRId[00-01] h0003",  Arrays.asList("grid00","grid01","h0003"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseHostPattern10() {
        testParseHostPattern("[gRId[00-01] h0003]",  Arrays.asList("grid00","grid01","h0003"));
    }

    @Test
    public void testParseHostPattern11() {
        testParseHostPattern("  GRID00  , GRID01  ",  Arrays.asList("grid00","grid01"));
    }

    @Test
    public void testParseHostPattern12() {
        testParseHostPattern("  GRID[00-02]  , GRID05-6  ",  Arrays.asList("grid00","grid01","grid02","grid05-6"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseHostPattern13() {
        testParseHostPattern("GRID[00-02][ -h[00-02]]",  Arrays.asList("grid00","grid00-h00","grid00-h01","grid00-h02","grid01","grid01-h00","grid01-h01","grid01-h02","grid02","grid02-h00","grid02-h01","grid02-h02"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseHostPattern14() {
        testParseHostPattern("GRID[00-02][-h[00-02], ]",  Arrays.asList("grid00","grid00-h00","grid00-h01","grid00-h02","grid01","grid01-h00","grid01-h01","grid01-h02","grid02","grid02-h00","grid02-h01","grid02-h02"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseHostPattern15() {
        Util.parseHostPattern("grid[");
    }
    
    @Test (expected=IllegalArgumentException.class)
    public void testParseHostPattern16() {
        Util.parseHostPattern("]");
    }
    
    @Test (expected=IllegalArgumentException.class)
    public void testParseHostPattern17() {
        Util.parseHostPattern("[a]]");
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseHostPattern18() {
        Util.parseHostPattern("[]");
    }

    @Test
    public void testParseHostPattern19() {
        testParseHostPattern("abcd-10-15-20-20",Arrays.asList("abcd-10-15-20-20"));
    }

    @Test
    public void testParseHostPattern20() {
        testParseHostPattern("abcd-10-15-20-25 abcde-10-15-20-20",Arrays.asList("abcd-10-15-20-25", "abcde-10-15-20-20"));
    }

    @Test
    public void testParseHostPattern21() {
        testParseHostPattern("abcd-10-15-20-25,abcde-10-15-20-20",Arrays.asList("abcd-10-15-20-25", "abcde-10-15-20-20"));
    }

    @Test
    public void testParseHostPattern22() {
        testParseHostPattern("abcd-10-15-20-25  ,  abcde-10-15-20-20",Arrays.asList("abcd-10-15-20-25", "abcde-10-15-20-20"));
    }

    @Test
    public void testParseHostPattern23() {
        testParseHostPattern("abcd-10-[15-20]-25  ,  abcde-10-15-20-20",Arrays.asList("abcd-10-15-25", "abcd-10-16-25", "abcd-10-17-25", "abcd-10-18-25", "abcd-10-19-25", "abcd-10-20-25","abcde-10-15-20-20"));
    }

    @Test
    public void testParseHostPattern24() {
        testParseHostPattern("abcde-10-15-21-20 abcd-10-[15-20]-25  ,  abcde-10-15-20-20",Arrays.asList("abcde-10-15-21-20", "abcd-10-15-25", "abcd-10-16-25", "abcd-10-17-25", "abcd-10-18-25", "abcd-10-19-25", "abcd-10-20-25","abcde-10-15-20-20"));
    }

    @Test
    public void testParseHostPattern25() {
        testParseHostPattern("abcde-10-15-21-20 abcd-10-[14 15]-25 ,  abcde-10-15-20-20",Arrays.asList("abcde-10-15-21-20", "abcd-10-14-25", "abcd-10-15-25", "abcde-10-15-20-20"));
    }

    @Test
    public void testParseHostPattern26() {
        testParseHostPattern("abcde-10-15-21-20 abcd-10-[0 15-20]-25 ,  abcde-10-15-20-20",Arrays.asList("abcde-10-15-21-20", "abcd-10-0-25", "abcd-10-15-25", "abcd-10-16-25", "abcd-10-17-25", "abcd-10-18-25", "abcd-10-19-25", "abcd-10-20-25", "abcde-10-15-20-20"));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testParseHostPattern27() {
        Util.parseHostPattern("[a [ab");
    }

    @Test
    public void testParseHostPattern28() {
        testParseHostPattern("[a_b c_d]aa", Arrays.asList("a_baa", "c_daa"));
    }

    @Test
    public void testParseHostPattern29() {
        testParseHostPattern("[a_b c_d]_aa", Arrays.asList("a_b_aa", "c_d_aa"));
    }

    @Test
    public void testParseHostPattern30() {
        testParseHostPattern("[a_b c_d_]aa", Arrays.asList("a_baa", "c_d_aa"));
    }

    @Test
    public void testParseHostPattern31() {
        testParseHostPattern("[a_b_ c_d]aa", Arrays.asList("a_b_aa", "c_daa"));
    }

    @Test
    public void testParseHostPattern32() {
        testParseHostPattern("[a_b_ c_d]_aa", Arrays.asList("a_b__aa", "c_d_aa"));
    }

    @Test
    public void testParseHostPattern33() {
        testParseHostPattern("aaa.bbb.ccc", Arrays.asList("aaa.bbb.ccc"));
    }
    
    @Test  (expected=IllegalArgumentException.class)
    public void testParseHostPattern34() {
        testParseHostPattern("aaa0[-9]", Arrays.asList("aaa0[-9]"));
    }

    @Test  (expected=IllegalArgumentException.class)
    public void testParseHostPattern35() {
        testParseHostPattern("aaa[1-]9", Arrays.asList("aaa[1-]9"));
    }

    @Test  (expected=IllegalArgumentException.class)
    public void testParseHostPattern36() {
        testParseHostPattern("aaa[1-9-]", Arrays.asList("aaa[1-9-]"));
    }

    private void testParseHostPattern(String input, List<String> expResult) {
        System.out.println("parseHostPattern: "+input);
        assertEquals(expResult, Util.parseHostPattern(input));
    }

    @Test (expected=IllegalArgumentException.class)
    public void testValidateHostIDList1() {
        ArrayList<String> hostIDs = new ArrayList<String>();
        hostIDs.add("localhost");
        Util.validateHostIDList(hostIDs, Host.Type.HOSTNAME);
    }

    @Test (expected=IllegalArgumentException.class)
    public void testValidateHostIDList2() {
        ArrayList<String> hostIDs = new ArrayList<String>();
        hostIDs.add("127.0.0.0");
        Util.validateHostIDList(hostIDs, Host.Type.IP);
    }
}
