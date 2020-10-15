//*___INFO__MARK_BEGIN__*/
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

package com.sun.grid.jgdi.util.shell.editor;

import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import junit.framework.*;
import com.sun.grid.jgdi.configuration.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 *
 */
public class GEObjectEditorTest extends TestCase {
    private TestGEObject geObj;
    
    private static String HGRP_1  = "@hostgroupA";
    private static String HGRP_2  = "@hostgroupB";
    
    
    //String
    private String[] passStringOnlyValues =
    {
        ",fsd",
        "aaa,",
        "aaa,bbb",
    };
    
    private String[] passStringValues =
    {
        "val",
        "val*/.\\nb`3432'",
        " 12fds",
        "\"quote\""
    };
    private String[] failStringValues =
    {
        "value with spaces",    //" " is a separator =>  3 values
        "\"long quote\""        //" " is a separator =>  2 values
    };
    
    //Long
    private String[] passLongValues =
    {
        "1",
        "002",
        " 123",
        " 004 ",
        "123456789",
        String.valueOf(Long.MAX_VALUE),
        //TODO LP All negative values should probably fail?
        String.valueOf(Long.MIN_VALUE)
    };
    private String[] failLongValues =
    {
        "1.2",
        "1,2",
        "1 2",
        ".5",
        ",005 ",
        "005,,",
        "10000000000000000000000000000000.11",
        "-5.5"
    };
    
    //Double
    private String[] passDoubleValues =
    {
        "1.0",
        " 004.61656516516516514456564654564564654654654654465654564564 ",
        "123456789123456789.12345687",
        ".5",
        String.valueOf(Double.MAX_VALUE),
        //TODO LP All negative values should probably fail?
        String.valueOf(Double.MIN_VALUE)
    };
    private String[] failDoubleValues =
    {
        "1.2.0",
        "1,2,0",
        "1 2",
        "002,231.52",
        " 123,2.52",
        ",005 ",
        "005,,",
        ",005.14",
        "90000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.11,123321"
    };
    
    //StringList
    private static Map passStringListValues;
    static {
        passStringListValues = new HashMap();
        passStringListValues.put("*/.\\ nb`3432'", Arrays.asList(new Object[] {"*/.\\", "nb`3432'"}));
        passStringListValues.put(" 12 fds", Arrays.asList(new Object[] {"12", "fds"}));
        passStringListValues.put(" 12 fds", Arrays.asList(new Object[] {"12", "fds"}));
        passStringListValues.put(",aa aaa", Arrays.asList(new Object[] {"aa", "aaa"}));//TODO LP Or should this be ",aa" "aaa"?
        passStringListValues.put("aaa,bb b", Arrays.asList(new Object[] {"aaa", "bb", "b"}));
        passStringListValues.put("\"quote\"", Arrays.asList(new Object[] {"\"quote\""}));
        passStringListValues.put("value with spaces", Arrays.asList(new Object[] {"value", "with", "spaces"}));
        passStringListValues.put("\"long quote\"", Arrays.asList(new Object[] {"\"long", "quote\""}));
        passStringListValues.put("a=b c=d", Arrays.asList(new Object[] {"a=b", "c=d"}));
        passStringListValues.put("a=b=,c", Arrays.asList(new Object[] {"a=b=", "c"}));
        passStringListValues.put("a b \n\n", Arrays.asList(new Object[] {"a",  "b"}));
    };
    
    private static Map failStringListValues;
    static {
        failStringListValues = new HashMap();
        failStringListValues.put("a b \n c", "Expected at least 2 tokens name and value got: \"c\"");
    };
    
    //Map
    private static Map passStringMapValues;
    static {
        passStringMapValues = new HashMap();
        HashMap temp = new HashMap();
        temp.put("@/", "val");
        passStringMapValues.put("val", temp);
        passStringMapValues.put("val ", temp);
        passStringMapValues.put("[@/=val]", temp);
        passStringMapValues.put("[@/ = val  ]  ", temp);
        
        temp = new HashMap();
        temp.put("@/", "val");
        temp.put(HGRP_1, "hval");
        passStringMapValues.put("val,["+HGRP_1+"=hval]", temp);
        passStringMapValues.put("val ["+HGRP_1+" = hval]", temp);
        passStringMapValues.put("[@/ = val] , ["+HGRP_1+" = hval]", temp);
        passStringMapValues.put("[@/=val] ["+HGRP_1+"= hval]", temp);
        
        temp = new HashMap();
        temp.put("@/", "val");
        temp.put(HGRP_1, "hval");
        temp.put(HGRP_2, "hval2");
        passStringMapValues.put("val,["+HGRP_1+"= hval],["+HGRP_2+"= hval2]", temp);
        passStringMapValues.put("val ["+HGRP_1+"=hval] ["+HGRP_2+"=hval2]", temp);
        passStringMapValues.put("val ["+HGRP_1+"=hval],["+HGRP_2+"=hval2]", temp);
        passStringMapValues.put("val,["+HGRP_1+"=hval] ["+HGRP_2+"=hval2]", temp);
        passStringMapValues.put("[@/=val],["+HGRP_1+"=hval],["+HGRP_2+"=hval2]", temp);
        passStringMapValues.put("[@/ =val] ["+HGRP_1+"=hval] ["+HGRP_2+" = hval2]", temp);
        passStringMapValues.put("[@/= val] ["+HGRP_1+" = hval],["+HGRP_2+"=hval2]", temp);
        passStringMapValues.put("[@/ = val],["+HGRP_1+"=hval] ["+HGRP_2+"=hval2]", temp);
    };
    
    private static Map failStringMapValues;
    static {
        failStringMapValues = new HashMap();
        failStringMapValues.put("val val2", "Expected [key=val] elements got: [@/=val val2]");
        failStringMapValues.put("val,val2", "Expected [key=val] elements got: [@/=val val2]");
        failStringMapValues.put("val = val2", "Expected [key=val] elements got: [@/=val = val2]");
        failStringMapValues.put("[@/=val val2]", "Expected [key=val] elements got: [@/=val val2]");
        failStringMapValues.put("[@/=val,val2  ]  ", "Expected [key=val] elements got: [@/=val val2]");
        failStringMapValues.put("[@/=val = val2  ]  ", "Expected [key=val] elements got: [@/=val = val2]");
        
        failStringMapValues.put("val,["+HGRP_1+"=hval=cont]", "Expected [key=val] elements got: ["+HGRP_1+"=hval=cont]");
        failStringMapValues.put("val=cont ["+HGRP_1+"=hval]", "Expected [key=val] elements got: [@/=val=cont]");
        failStringMapValues.put("[@/=val],["+HGRP_1+"=hval=cont]", "Expected [key=val] elements got: ["+HGRP_1+"=hval=cont]");
        failStringMapValues.put("[@/=val=cont] ["+HGRP_1+" hval]", "Expected [key=val] elements got: [@/=val=cont]");
    };
    
    //MapList
    private static Map passStringMapListValues;
    static {
        passStringMapListValues = new HashMap();
        HashMap keyMap = new HashMap();
        HashMap valueMap = new HashMap();
        valueMap.put("key1", "val1");
        valueMap.put("key2", "val2");
        valueMap.put("key3", "val3");
        keyMap.put("@/", valueMap);
        passStringMapListValues.put("key1=val1 key2=val2 key3=val3", keyMap);
        passStringMapListValues.put("key1=val1,key2=val2,key3=val3", keyMap);
        passStringMapListValues.put("key1=val1 , key2=val2 ,key3=val3", keyMap);
        passStringMapListValues.put("key1=val1   key2=val2  ,  key3=val3", keyMap);
        passStringMapListValues.put("key1=val1 ,,,, key2=val2 key3=val3", keyMap);
        passStringMapListValues.put("[@/=key1=val1 key2=val2 key3=val3]", keyMap);
        passStringMapListValues.put(" [ @/ = key1=val1, key2=val2 key3=val3 ]", keyMap);
        passStringMapListValues.put("[@/ =key1=val1,  key2=val2 , key3=val3\t]", keyMap);
        passStringMapListValues.put("[\t@/\t=\tkey1=val1   key2=val2  ,  key3=val3\t]", keyMap);
        passStringMapListValues.put("[@/=key1=val1 ,,,,  key2=val2 , key3=val3]", keyMap);
    };
     /*          "key=val1 val2 val3,["+HGRP_1+"=key=hval1 hval2 hval3]",
               "key=val1,val2,val3,["+HGRP_1+"=key=hval1,hval2 hval3]",
               "key=val1 val2,val3,["+HGRP_1+"=key=hval1 hval2,hval3]",
               "key=val1 , val2, val3,["+HGRP_1+"=key=hval1 , hval2, hval3]",
               "key=val1 , val2, ,, , val3,["+HGRP_1+"=key=hval1 , hval2,  ,, , ,hval3]",
               "key=val1 , val2, val3 ["+HGRP_1+"=key=hval1 , hval2, hval3]",
      
               "[@/=key=val1 val2 val3]",
               "[@/=key=val1,val2,val3]",
               "[@/=key=val1 val2,val3]",
               "[@/=key=val1 , val2, val3]",
      
               "[@/=key=val1 val2 val3],["+HGRP_1+"=key=hval1 hval2 hval3]",
               "[@/=key=val1,val2,val3],["+HGRP_1+"=key=hval1,hval2 hval3]",
               "[@/=key=val1 val2,val3],["+HGRP_1+"=key=hval1 hval2,hval3]",
               "[@/=key=val1 , val2, val3],["+HGRP_1+"=key=hval1 , hval2, hval3]",
               "[@/=key=val1 , val2, val3,],,[,"+HGRP_1+"=key=hval1, ,, hval2, hval3],",
               "[@/=key=val1 , val2, val3] ["+HGRP_1+"=key=hval1 , hval2, hval3]",
            };*/
    private static Map failStringMapListValues;
    static {
        failStringMapListValues = new HashMap();
        //failStringMapListValues.put("key val1 val2 val3",""); //Should this really fail
        failStringMapListValues.put("key=val1,val2,val3=val4", "Expected all elements to be of a same kind. First elements were a Map current element \"val2\" is not.");
        failStringMapListValues.put("key=val1,val2,val3=val4", "Expected all elements to be of a same kind. First elements were a Map current element \"val2\" is not.");
        failStringMapListValues.put("key=val1 val2[,val3,", "Expected all elements to be of a same kind. First elements were a Map current element \"val2\" is not.");
        failStringMapListValues.put("key=val1 , [val2], val3", "Expected [key=val1 val2] elements got: [val2]");
        failStringMapListValues.put("key=val1 , [val2, val3]", "Expected [key=val1 val2] elements got: [val2 val3]");
    };
    
      /*         "key=val1=val2 val3,["+HGRP_1+"=key=hval1 hval2= hval3]",
               "key=val1,val2,val3,["+HGRP_1+"=key=hval1,hval2 hval3]",
               "key=val1 [val2,val3],["+HGRP_1+"=key=hval1 hval2,hval3]",
               "key=val1 , val2, val3,[["+HGRP_1+"=key=hval1 , hval2, hval3]]",
               "key=val1 , val2, val3 ["+HGRP_1+"=key=hval1 , [hval2=hval3]]",
       
               "[@/=key=val1 val2=val3]",
       
               "[@/=key=val1 val2 val3],["+HGRP_1+"=key=hval1 [hval2 hval3]]",
               "[@/=key=val1,val2,val3],["+HGRP_1+"=key=hval1,[hval2=hval3]]",
               "[@/=key=val1 val2,val3],["+HGRP_1+"=key=hval1 hval2,hval3]",
               "[[@/=key=val1 , val2, val3],["+HGRP_1+"=key=hval1 , hval2, hval3]]",
               "[[@/=key=val1 , val2, val3] ["+HGRP_1+"=key=hval1 , hval2, hval3]",
            };*/
    
    
    public GEObjectEditorTest(String testName) {
        super(testName);
    }
    
    protected synchronized void setUp() throws Exception {
        geObj = new TestGEObject();
        Util.addDescriptor(geObj.getClass(), new TestGEObjectDescriptor());
    }
    
    protected void tearDown() throws Exception {
        geObj = null;
    }
    
    
    private void updateSimpleValue(String testName, String[] values, String propertyName, boolean shouldFail) {
        for (int i=0; i< values.length; i++) {
            String str = values[i];
            System.out.print(testName+"(\""+str+"\")");
            try {
                GEObjectEditor.updateObjectWithText(geObj,propertyName+" "+str);
                if (shouldFail) {
                    System.out.println(" - FAILED");
                    fail("Expecting failure for "+testName+"(\""+str+"\"), but test passed.");
                }
                simpleAssert(propertyName, str, geObj);
                System.out.println(" - OK");
            } catch (Exception ex) {
                if (!shouldFail) {
                    System.out.println(" - FAILED");
                    fail("Got unexpected exception "+ex.getMessage());
                }
                System.out.println(" - OK "+ex.getMessage());
            }
        }
    }
    
    private void simpleAssert(String propertyName, String str, TestGEObject geObj) {
        if (propertyName.equals("name")) {
            assertEquals("Setting value \""+str+"\" failed.", str.trim(), geObj.getName());
        } else if (propertyName.equals("long")) {
            assertEquals("Setting value \""+str+"\" failed.", Long.parseLong(str.trim()), geObj.getLong());
        } else if (propertyName.equals("double")) {
            assertEquals("Setting value \""+str+"\" failed.", Double.parseDouble(str.trim()), geObj.getDouble(),0);
        } else if (propertyName.equals("string_list")) {
            assertEquals("Setting value \""+str+"\" failed.", 1, geObj.getStringList().size());
            assertEquals("Setting value \""+str+"\" failed.", str.trim(), (String)geObj.getStringList().get(0));
        } else if (propertyName.equals("string_map")) {
            assertEquals(1, geObj.getStringMapKeys().size());
            assertEquals("Setting value \""+str+"\" failed.", str.trim(), geObj.getStringMap("@/"));
        } else if (propertyName.equals("string_map_list")) {
            assertEquals(1, geObj.getStringMapListKeys().size());
            assertEquals(1, geObj.getStringMapListList("@/").size());
            assertEquals("Setting value \""+str+"\" failed.", str.trim(), (String)geObj.getStringMapListList("@/").get(0));
        } else {
            throw new IllegalArgumentException("simpleAssert: Unknown attribute \""+propertyName+"\"");
        }
    }
    
    //STRING
    public void testPassString() {
        updateSimpleValue("testPassString", passStringOnlyValues, "name", false);
        updateSimpleValue("testPassString", passStringValues, "name", false);
    }
    
    public void testPassStringWithLongValues() {
        updateSimpleValue("testPassStringWithLongValues", passLongValues, "name", false);
    }
    
    public void testPassStringWithDoubleValues() {
        updateSimpleValue("testPassStringWithDoubleValues", passDoubleValues, "name", false);
    }
    
    public void testFailString() {
        for (int i=0; i< failStringValues.length; i++) {
            String str = failStringValues[i];
            System.out.print("testFailString(\""+str+"\") - ");
            try {
                GEObjectEditor.updateObjectWithText(geObj,"name "+str);
                System.out.println("FAILED");
                fail("Setting value \""+str+"\" should have failed.");
            } catch (IllegalArgumentException ex) {
                System.out.println("OK "+ex.getMessage());
            }
        }
    }
    
    
    //LONG
    public void testPassLong() {
        updateSimpleValue("testPassLong", passLongValues, "long", false);
    }
    
    public void testFailLongWithDouble() {
        updateSimpleValue("testFailLongWithDouble", passDoubleValues, "long", true);
    }
    
    public void testFailLongWithString() {
        updateSimpleValue("testFailLongWithString", passStringValues, "long", true);
    }
    
    public void testFailLong() {
        for (int i=0; i< failLongValues.length; i++) {
            String str = failLongValues[i];
            System.out.print("testFailLong(\""+str+"\") - ");
            try {
                GEObjectEditor.updateObjectWithText(geObj,"long "+str);
                System.out.println("FAILED");
                fail("Setting value \""+str+"\" should have failed.");
            } catch (IllegalArgumentException ex) {
                System.out.println("OK got: "+ex.getMessage());
            }
        }
    }
    
    
    //DOUBLE
    public void testPassDouble() {
        updateSimpleValue("testPassDouble", passDoubleValues, "double", false);
    }
    
    public void testPassDoubleWithLong() {
        updateSimpleValue("testPassDoubleWithLong", passLongValues, "double", false);
    }
    
    public void testFailDoubleWithString() {
        updateSimpleValue("testPassDoubleWithLong", passStringValues, "double", true);
    }
    
    public void testFailDouble() {
        for (int i=0; i< failDoubleValues.length; i++) {
            String str = failDoubleValues[i];
            System.out.print("testFailDouble(\""+str+"\") - ");
            try {
                GEObjectEditor.updateObjectWithText(geObj,"double "+str);
                System.out.println("FAILED");
                fail("Setting value \""+str+"\" should have failed.");
            } catch (IllegalArgumentException ex) {
                System.out.println("OK got: "+ex.getMessage());
            }
        }
    }
    
    //STRING_LIST
    public void testPassStringList() {
        for (Iterator iter = passStringListValues.keySet().iterator(); iter.hasNext();) {
            String str = (String)iter.next();
            System.out.println("testPassStringList(\""+str+"\")");
            GEObjectEditor.updateObjectWithText(geObj,"string_list "+str);
            assertEquals("Setting value \""+str+"\" failed.", (List)passStringListValues.get(str), geObj.getStringList());
        }
    }
    
    public void testPassStringListWithLong() {
        updateSimpleValue("testPassStringListWithLong", passLongValues, "string_list", false);
    }
    
    public void testPassStringListWithDouble() {
        updateSimpleValue("testPassStringListWithDouble", passDoubleValues, "string_list", false);
    }
    
    public void testPassStringListWithString() {
        updateSimpleValue("testPassStringListWithString", passStringValues, "string_list", false);
    }
    
    public void testFailStringList() {
        for (Iterator iter = failStringListValues.keySet().iterator(); iter.hasNext();) {
            String str = (String)iter.next();
            System.out.print("testFailStringList(\""+str+"\") - ");
            try {
                GEObjectEditor.updateObjectWithText(geObj,"string_list "+str);
                System.out.println("FAILED");
                fail("Setting value \""+str+"\" should have failed.");
            } catch (IllegalArgumentException ex) {
                String msg = ex.getMessage();
                String exp = (String)failStringListValues.get(str);
                if (msg.equals(exp)) {
                    System.out.println("OK got: "+msg);
                } else {
                    System.out.println("FAILED");
                    fail("Expected exception with \""+exp+"\" text got: \""+msg+"\"");
                }
            }
        }
    }
    
    //STRING_MAP
    public void testPassStringMap() {
        for (Iterator iter = passStringMapValues.keySet().iterator(); iter.hasNext();) {
            String str = (String)iter.next();
            System.out.println("testPassStringMap(\""+str+"\")");
            GEObjectEditor.updateObjectWithText(geObj,"string_map "+str);
            Map valMap = (HashMap)passStringMapValues.get(str);
            assertEquals(valMap.keySet().size(), geObj.getStringMapKeys().size());
            for (Iterator iter2 = valMap.keySet().iterator(); iter2.hasNext();) {
                String key = (String)iter2.next();
                assertEquals("Setting value \""+str+"\" failed.", valMap.get(key), geObj.getStringMap(key));
            }
        }
    }
    
    public void testPassStringMapWithLong() {
        updateSimpleValue("testPassStringMapWithLong", passLongValues, "string_map", false);
    }
    
    public void testPassStringMapWithDouble() {
        updateSimpleValue("testPassStringMapWithDouble", passDoubleValues, "string_map", false);
    }
    
    public void testPassStringMapWithString() {
        updateSimpleValue("testPassStringMapWithString", passStringValues, "string_map", false);
    }
    
    public void testFailStringMap() {
        for (Iterator iter = failStringMapValues.keySet().iterator(); iter.hasNext();) {
            String str = (String)iter.next();
            System.out.print("testFailStringMap(\""+str+"\") - ");
            try {
                GEObjectEditor.updateObjectWithText(geObj,"string_map "+str);
                System.out.println("FAILED");
                fail("Setting value \""+str+"\" should have failed.");
            } catch (IllegalArgumentException ex) {
                String msg = ex.getMessage();
                String exp = (String)failStringMapValues.get(str);
                if (msg.equals(exp)) {
                    System.out.println("OK got: "+msg);
                } else {
                    System.out.println("FAILED");
                    fail("Expected exception with \""+exp+"\" text got: \""+msg+"\"");
                }
            }
        }
    }
    
    //STRING_MAP_LIST
    public void testPassStringMapList() {
        for (Iterator iter = passStringMapListValues.keySet().iterator(); iter.hasNext();) {
            String str = (String)iter.next();
            System.out.print("testPassStringMap(\""+str+"\")");
            GEObjectEditor.updateObjectWithText(geObj,"string_map_list "+str);
            Map valMap = (HashMap)passStringMapListValues.get(str);
            assertEquals(valMap.keySet().size(), geObj.getStringMapListKeys().size());
            for (Iterator iter2 = valMap.keySet().iterator(); iter2.hasNext();) {
                String key = (String)iter2.next();
                List list = new ArrayList();
                Map m = (HashMap)valMap.get(key);
                for (Iterator iter3 = m.keySet().iterator(); iter3.hasNext();) {
                    String k = (String)iter3.next();
                    list.add(k+"="+m.get(k));
                }
                Collections.sort(list, new Comparator() {
                    public int compare(Object o1, Object o2) {
                        if (o1 == null && o2 == null) {
                            return 0;
                        }
                        if (o1 ==null) {
                            return -1;
                        }
                        if (o2 == null) {
                            return 1;
                        }
                        return ((String)o1).compareTo((String)o2);
                    }
                });
                assertEquals("Setting value \""+str+"\" failed.", list, geObj.getStringMapListList(key));
            }
        }
    }
    
    public void testPassStringMapListWithLong() {
        updateSimpleValue("testFailStringMapListWithLong", passLongValues, "string_map_list", false);
    }
    
    public void testPassStringMapListWithDouble() {
        updateSimpleValue("testFailStringMapListWithDouble", passDoubleValues, "string_map_list", false);
    }
    
    public void testPassStringMapListWithString() {
        updateSimpleValue("testFailStringMapListWithString", passStringValues, "string_map_list", false);
    }
    
    public void testFailStringMapList() {
        for (Iterator iter = failStringMapListValues.keySet().iterator(); iter.hasNext();) {
            String str = (String)iter.next();
            System.out.print("testFailStringMapList(\""+str+"\") - ");
            try {
                GEObjectEditor.updateObjectWithText(geObj,"string_map_list "+str);
                System.out.println("FAILED");
                fail("Setting value \""+str+"\" should have failed.");
            } catch (IllegalArgumentException ex) {
                String msg = ex.getMessage();
                String exp = (String)failStringMapListValues.get(str);
                if (msg.equals(exp)) {
                    System.out.println("OK got: "+msg);
                } else {
                    System.out.println("FAILED");
                    fail("Expected exception with \""+exp+"\" text got: \""+msg+"\"");
                }
            }
        }
    }
}
