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

package com.sun.grid.loadsensor;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.PrintStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;
import junit.framework.TestCase;

/**
 *
 * @author dant
 */
public class LoadSensorManagerTest extends TestCase {
    
    public LoadSensorManagerTest(String testName) {
        super(testName);
    }

    /**
     * Test of main method, of class LoadSensorManager.
     */
    public void xtestMain() throws Exception {
        System.out.println("main()");
        final String[] args = new String[] {"com.sun.grid.loadsensor.LoadSensorManagerTest$TestLS"};
        ExecutorService es = Executors.newSingleThreadExecutor();

        // Set the security manager to prevent the main() method from calling
        // System.exit()
        TestLS.intervalTime = 0L;
        TestLS.measurementTime = 0L;

        // Mostly we're just testing that we don't get an exception.
        // We use a separate thread so that we can stop it after the test.
        es.submit(new Runnable() {
            public void run() {
                LoadSensorManager.main(args);
            }
        });

        // Give the method a chance to run
        Thread.sleep(1000L);

        assertFalse("The main() method did not measurement the load values", TestLS.measurementTime == 0);
        // There's no point in testing further, since we're really testing the
        // parse() method.  We'll do that in testParse().
        es.shutdownNow();

        // TODO: Need to figure out how to test this
//        try {
//            LoadSensorManager.main(new String[0]);
//            fail("The main() method did not fail on no args");
//        } catch (SecurityException e) {
//            assertEquals("The main() method did not exit with the correct exit status", "1", e.getMessage());
//        }
//
//        args[0] = "java.lang.Object";
//
//        try {
//            LoadSensorManager.main(args);
//            fail("The main() method did not fail on non-LoadSensor class");
//        } catch (SecurityException e) {
//            assertEquals("The main() method did not exit with the correct exit status", "2", e.getMessage());
//        }
//
//        args[0] = "this.class.do.not.Exist";
//
//        try {
//            LoadSensorManager.main(args);
//            fail("The main() method did not fail on nonexistant class");
//        } catch (SecurityException e) {
//            assertEquals("The main() method did not exit with the correct exit status", "3", e.getMessage());
//        }
    }

    /**
     * Test of parse method, of class LoadSensorManager.
     */
    public void testParse() throws Exception {
        System.out.println("parse()");

        Logger log = Logger.getLogger("com.sun.grid.LoadSensor");
        TestHandler handler = new TestHandler();

        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);

        doParseTest("\nQUIT\n", "begin\nhost:load:value\nend\n");
        assertEquals("The parse() method did log the expected output", 8, handler.messages.size());
        // Collecting load values
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(0).getLevel());
        // Adjusted measurement time
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(1).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> ", handler.messages.get(2).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(3).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< begin", handler.messages.get(3).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(4).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< host:load:value", handler.messages.get(4).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(5).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< end", handler.messages.get(5).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(6).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> QUIT", handler.messages.get(6).getMessage());
        // Exiting
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(7).getLevel());

        handler.messages.clear();
        doParseTest("\n\nQUIT\n", "begin\nhost:load:value\nend\nbegin\nhost:load:value\nend\n");
        assertEquals("The parse() method did log the expected output", 13, handler.messages.size());
        // Collecting load values
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(0).getLevel());
        // Adjusted measurement time
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(1).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> ", handler.messages.get(2).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(3).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< begin", handler.messages.get(3).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(4).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< host:load:value", handler.messages.get(4).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(5).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< end", handler.messages.get(5).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(6).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> ", handler.messages.get(6).getMessage());
        // Last load report interval
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(7).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(8).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< begin", handler.messages.get(8).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(9).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< host:load:value", handler.messages.get(9).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(10).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< end", handler.messages.get(10).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(11).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> QUIT", handler.messages.get(11).getMessage());
        // Exiting
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(12).getLevel());

        handler.messages.clear();
        doParseTest("\nQUIT\n\n", "begin\nhost:load:value\nend\n");
        assertEquals("The parse() method did log the expected output", 8, handler.messages.size());
        // Collecting load values
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(0).getLevel());
        // Adjusted measurement time
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(1).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> ", handler.messages.get(2).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(3).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< begin", handler.messages.get(3).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(4).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< host:load:value", handler.messages.get(4).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(5).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< end", handler.messages.get(5).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(6).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> QUIT", handler.messages.get(6).getMessage());
        // Exiting
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(7).getLevel());

        handler.messages.clear();
        doParseTest("QUIT\n", "");
        assertEquals("The parse() method did log the expected output", 4, handler.messages.size());
        // Collecting load values
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(0).getLevel());
        // Adjusted measurement time
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(1).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> QUIT", handler.messages.get(2).getMessage());
        // Exiting
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(3).getLevel());

        handler.messages.clear();
        doParseTest("QUIT\n\n", "");
        assertEquals("The parse() method did log the expected output", 4, handler.messages.size());
        // Collecting load values
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(0).getLevel());
        // Adjusted measurement time
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(1).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> QUIT", handler.messages.get(2).getMessage());
        // Exiting
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(3).getLevel());

        handler.messages.clear();
        doParseTest("NONSENSE\n", "");
        assertEquals("The parse() method did log the expected output", 4, handler.messages.size());
        // Collecting load values
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(0).getLevel());
        // Adjusted measurement time
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(1).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> NONSENSE", handler.messages.get(2).getMessage());
        // Bad input
        assertEquals("The parse() method did log the expected output", Level.WARNING, handler.messages.get(3).getLevel());

        handler.messages.clear();
        doParseTest("NONSENSE\n\nQUIT\n", "begin\nhost:load:value\nend\n");
        assertEquals("The parse() method did log the expected output", 10, handler.messages.size());
        // Collecting load values
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(0).getLevel());
        // Adjusted measurement time
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(1).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> NONSENSE", handler.messages.get(2).getMessage());
        // Bad input
        assertEquals("The parse() method did log the expected output", Level.WARNING, handler.messages.get(3).getLevel());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(4).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> ", handler.messages.get(4).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(5).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< begin", handler.messages.get(5).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(6).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< host:load:value", handler.messages.get(6).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(7).getLevel());
        assertEquals("The parse() method did log the expected output", "<<< end", handler.messages.get(7).getMessage());
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(8).getLevel());
        assertEquals("The parse() method did log the expected output", ">>> QUIT", handler.messages.get(8).getMessage());
        // Exiting
        assertEquals("The parse() method did log the expected output", Level.FINE, handler.messages.get(9).getLevel());

        handler.messages.clear();
        doParseTest("", "");
        assertEquals("The parse() method did log the expected output", 2, handler.messages.size());
        // Collecting load values
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(0).getLevel());
        // Adjusted measurement time
        assertEquals("The parse() method did log the expected output", Level.FINEST, handler.messages.get(1).getLevel());
}

    private void doParseTest(String input, String expectedResult) throws Exception {
        LoadSensorManager instance = new LoadSensorManager(new TestLS());
        ByteArrayInputStream in = new ByteArrayInputStream(input.getBytes());
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(buffer);

        setPrivateField(instance, "doMeasurement", Boolean.FALSE);
        instance.setIn(in);
        instance.setOut(out);
        instance.parse();

        assertEquals("The parse() method did not produce the expected output", expectedResult, buffer.toString());
    }

    /**
     * Test of log method, of class Jsv.
     */
    public void testSetIn() throws Exception {
        System.out.println("setIn()");
        LoadSensorManager instance = new LoadSensorManager(null);
        InputStream in = new ByteArrayInputStream(new byte[8]);
        Object obj = getPrivateField(instance, "in");

        assertSame("The input stream did not default to System.in", System.in, obj);

        instance.setIn(in);
        obj = getPrivateField(instance, "in");
        assertSame("The setIn() method did not set the input stream", in, obj);

        instance.setIn(null);
        obj = getPrivateField(instance, "in");
        assertSame("The setIn() method did not set the input stream back to System.in", System.in, obj);
    }

    /**
     * Test of log method, of class Jsv.
     */
    public void testSetOut() throws Exception {
        System.out.println("setOut()");
        LoadSensorManager instance = new LoadSensorManager(null);
        PrintStream out = new PrintStream(new ByteArrayOutputStream());
        Object obj = getPrivateField(instance, "out");

        assertSame("The input stream was did not default to System.out", System.out, obj);

        instance.setOut(out);
        obj = getPrivateField(instance, "out");
        assertSame("The setOut() method did not set the output stream", out, obj);

        instance.setOut(null);
        obj = getPrivateField(instance, "out");
        assertSame("The setOut() method did not set the output stream back to System.out", System.out, obj);
    }

    /**
     * Test of printUsage method, of class LoadSensorManager.
     */
    public void testPrintUsage() throws Exception {
        System.out.println("printUsage()");
        Logger log = Logger.getLogger("com.sun.grid.LoadSensor");
        TestHandler handler = new TestHandler();

        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);

        callPrivateMethod(new LoadSensorManager(null), "printUsage", new Class[0], new Object[0]);
        
        assertEquals("The parse() method did log the expected output", 1, handler.messages.size());
        assertEquals("The parse() method did log the expected output", Level.INFO, handler.messages.get(0).getLevel());
    }

    /**
     * Test of send method, of class LoadSensorManager.
     */
    public void testSend() throws Exception {
        System.out.println("send()");
        LoadSensorManager instance = new LoadSensorManager(new TestLS());
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(buffer);
        Logger log = Logger.getLogger("com.sun.grid.LoadSensor");
        TestHandler handler = new TestHandler();

        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);

        instance.setOut(out);

        String message = "test";
        callPrivateMethod(instance, "send", new Class[] {String.class}, new Object[] {message});
        assertEquals("The send() method did not send the correct output: ", message + "\n", buffer.toString());
        assertEquals("The send() method did log the expected output", 1, handler.messages.size());
        assertEquals("The send() method did log the expected output", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("The send() method did log the expected output", "<<< " + message, handler.messages.get(0).getMessage());

        handler.messages.clear();
        buffer.reset();
        message = "";
        callPrivateMethod(instance, "send", new Class[] {String.class}, new Object[] {message});

        assertEquals("The send() method did not send the correct output: ", message + "\n", buffer.toString());
        assertEquals("The send() method did log the expected output", 1, handler.messages.size());
        assertEquals("The send() method did log the expected output", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("The send() method did log the expected output", "<<< " + message, handler.messages.get(0).getMessage());

        handler.messages.clear();
        buffer.reset();
        message = "";
        callPrivateMethod(instance, "send", new Class[] {String.class}, new Object[] {null});

        assertEquals("The send() method did not send the correct output: ", message + "\n", buffer.toString());
        assertEquals("The send() method did log the expected output", 1, handler.messages.size());
        assertEquals("The send() method did log the expected output", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("The send() method did log the expected output", "<<< null", handler.messages.get(0).getMessage());
    }

    /**
     * Test of adjustTimeToMeasure method, of class LoadSensorManager.
     */
    public void testAdjustTimeToMeasure() throws Exception {
        System.out.println("adjustTimeToMeasure()");
        LoadSensorManager instance = new LoadSensorManager(new TestLS());
        int result = 0;
        int expectedResult = 10;
        Logger log = Logger.getLogger("com.sun.grid.LoadSensor");
        TestHandler handler = new TestHandler();

        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);

        setPrivateField(instance, "timeToMeasure", new Integer(0));
        callPrivateMethod(instance, "adjustTimeToMeasure", new Class[] { Integer.TYPE }, new Object[] {new Integer(10000)});
        result = (Integer)getPrivateField(instance, "timeToMeasure");
        assertEquals("The adjustTimeToMeasure() method did not store the expected value: ", expectedResult, result);
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());

        expectedResult = 10;

        handler.messages.clear();
        callPrivateMethod(instance, "adjustTimeToMeasure", new Class[] { Integer.TYPE }, new Object[] {new Integer(5000)});
        result = (Integer)getPrivateField(instance, "timeToMeasure");
        assertEquals("The adjustTimeToMeasure() method did not store the expected value: ", expectedResult, result);
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());

        expectedResult = 22;

        handler.messages.clear();
        callPrivateMethod(instance, "adjustTimeToMeasure", new Class[] { Integer.TYPE }, new Object[] {new Integer(100000)});
        result = (Integer)getPrivateField(instance, "timeToMeasure");
        assertEquals("The adjustTimeToMeasure() method did not store the expected value: ", expectedResult, result);
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());

        expectedResult = 20;

        handler.messages.clear();
        callPrivateMethod(instance, "adjustTimeToMeasure", new Class[] { Integer.TYPE }, new Object[] {new Integer(1000)});
        result = (Integer)getPrivateField(instance, "timeToMeasure");
        assertEquals("The adjustTimeToMeasure() method did not store the expected value: ", expectedResult, result);
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());

        expectedResult = 18;

        handler.messages.clear();
        callPrivateMethod(instance, "adjustTimeToMeasure", new Class[] { Integer.TYPE }, new Object[] {new Integer(0)});
        result = (Integer)getPrivateField(instance, "timeToMeasure");
        assertEquals("The adjustTimeToMeasure() method did not store the expected value: ", expectedResult, result);
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());

        expectedResult = 18;

        handler.messages.clear();
        callPrivateMethod(instance, "adjustTimeToMeasure", new Class[] { Integer.TYPE }, new Object[] {new Integer(-1000)});
        result = (Integer)getPrivateField(instance, "timeToMeasure");
        assertEquals("The adjustTimeToMeasure() method did not store the expected value: ", expectedResult, result);
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The adjustTimeToMeasure() method did not log the expected output: ", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of calculateTiming method, of class LoadSensorManager.
     */
    public void testCalculateTiming() throws Exception {
        System.out.println("calculateTiming()");
        LoadSensorManager instance = new LoadSensorManager(new TestLS());
        int interval = 0;
        Logger log = Logger.getLogger("com.sun.grid.LoadSensor");
        TestHandler handler = new TestHandler();

        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);

        callPrivateMethod(instance, "calculateTiming", null, null);
        interval = (Integer)getPrivateField(instance, "lastInterval");

        assertEquals("The calculateTiming() method did not calculate the interval correctly: ", 0, interval);
        assertEquals("The calculateTiming() method did not log the expected output: ", 0, handler.messages.size());

        Thread.sleep(2500L);

        handler.messages.clear();
        callPrivateMethod(instance, "calculateTiming", null, null);
        interval = (Integer)getPrivateField(instance, "lastInterval");

        assertEquals("The calculateTiming() method did not calculate the interval correctly: ", 3, interval);
        assertEquals("The calculateTiming() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The calculateTiming() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());

        Thread.sleep(1L);

        handler.messages.clear();
        callPrivateMethod(instance, "calculateTiming", null, null);
        interval = (Integer)getPrivateField(instance, "lastInterval");

        assertEquals("The calculateTiming() method did not calculate the interval correctly: ", 1, interval);
        assertEquals("The calculateTiming() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The calculateTiming() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());
    }

    /**
     * Test of doMeasurement method, of class LoadSensorManager.
     */
    public void testDoMeasurement() throws Exception {
        System.out.println("doMeasurement()");
        System.out.println("-- This one takes a while, so please be patient --");
        LoadSensorManager instance = new LoadSensorManager(new TestLS());
        Logger log = Logger.getLogger("com.sun.grid.LoadSensor");
        TestHandler handler = new TestHandler();

        // Test that interval < 0 and lastInterval == 0 takes measurement
        TestLS.measurementTime = 0L;
        TestLS.interval = -1;
        setPrivateField(instance, "lastInterval", new Integer(0));
        setPrivateField(instance, "doMeasurement", Boolean.TRUE);
        callPrivateMethod(instance, "doMeasurement", null, null);
        assertFalse("The doMeasurement() method did not take an immediate measurement", TestLS.measurementTime == 0);

        // Test that interval == 0 and lastInterval == 0 takes measurement
        TestLS.measurementTime = 0L;
        TestLS.interval = 0;
        setPrivateField(instance, "lastInterval", new Integer(0));
        setPrivateField(instance, "doMeasurement", Boolean.TRUE);
        callPrivateMethod(instance, "doMeasurement", null, null);
        assertFalse("The doMeasurement() method did not take an immediate measurement", TestLS.measurementTime == 0);

        // Test that interval < 0 and lastInterval > 0 takes delayedmeasurement
        TestLS.measurementTime = 0L;
        TestLS.interval = -1;
        setPrivateField(instance, "timeToMeasure", new Integer(1));
        setPrivateField(instance, "lastInterval", new Integer(3));
        setPrivateField(instance, "doMeasurement", Boolean.TRUE);
        callPrivateMethod(instance, "doMeasurement", null, null);
        for (LogRecord r:handler.messages) System.out.println("MSG: " + r.getMessage());
        assertTrue("The doMeasurement() method took an immediate measurement", TestLS.measurementTime == 0);
        
        Thread.sleep(1500L);
        assertFalse("The doMeasurement() method did not take a delayed measurement", TestLS.measurementTime == 0);

        // Test that interval == 0 and lastInterval > 0 takes delayed measurement
        TestLS.measurementTime = 0L;
        TestLS.interval = 0;
        setPrivateField(instance, "timeToMeasure", new Integer(1));
        setPrivateField(instance, "lastInterval", new Integer(3));
        setPrivateField(instance, "doMeasurement", Boolean.TRUE);
        callPrivateMethod(instance, "doMeasurement", null, null);
        assertTrue("The doMeasurement() method took an immediate measurement", TestLS.measurementTime == 0);

        Thread.sleep(1500L);
        assertFalse("The doMeasurement() method did not take a delayed measurement", TestLS.measurementTime == 0);

        // Test that interval > 0 takes periodic measurement
        TestLS.measurementTime = 0L;
        TestLS.interval = 2;
        setPrivateField(instance, "doMeasurement", Boolean.FALSE);
        callPrivateMethod(instance, "doMeasurement", null, null);
        assertTrue("The doMeasurement() method took an immediate measurement", TestLS.measurementTime == 0);

        Thread.sleep(2100L);
        long lastTime = TestLS.intervalTime;
        assertFalse("The doMeasurement() method did not take a delayed measurement", lastTime == 0);

        Thread.sleep(2100L);
        assertTrue("The doMeasurement() method did not take second delayed measurement", lastTime < TestLS.measurementTime);

        // Test that interval < 0 after interval > 0 takes no measurement
        TestLS.measurementTime = 0L;
        TestLS.interval = -1;
        setPrivateField(instance, "doMeasurement", Boolean.FALSE);
        callPrivateMethod(instance, "doMeasurement", null, null);
        assertTrue("The doMeasurement() method took an immediate measurement", TestLS.measurementTime == 0);

        // Allow for the last periodic measurement to die off
        Thread.sleep(3000L);
        TestLS.measurementTime = 0L;

        // Wait again
        Thread.sleep(3000L);
        assertTrue("The doMeasurement() method took a delayed measurement", TestLS.measurementTime == 0);

        // Test that interval == 0 after interval > 0 takes no measurement
        TestLS.measurementTime = 0L;
        TestLS.interval = 0;
        setPrivateField(instance, "doMeasurement", Boolean.FALSE);
        callPrivateMethod(instance, "doMeasurement", null, null);
        assertTrue("The doMeasurement() method took an immediate measurement", TestLS.measurementTime == 0);

        Thread.sleep(3000L);
        assertTrue("The doMeasurement() method took a delayed measurement", TestLS.measurementTime == 0);

        // Test with exceptions
        TestLS.measurementTime = 0L;
        TestLS.interval = 0;

        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);
        instance = new LoadSensorManager(new ExLS());

        setPrivateField(instance, "lastInterval", new Integer(0));
        setPrivateField(instance, "doMeasurement", Boolean.TRUE);
        callPrivateMethod(instance, "doMeasurement", null, null);
        assertEquals("The doMeasurement() method did not log the expected output: ", 5, handler.messages.size());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.WARNING, handler.messages.get(1).getLevel());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.WARNING, handler.messages.get(2).getLevel());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.FINEST, handler.messages.get(3).getLevel());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.WARNING, handler.messages.get(4).getLevel());

        handler.messages.clear();
        TestLS.measurementTime = 0L;
        TestLS.interval = 0;
        setPrivateField(instance, "timeToMeasure", new Integer(1));
        setPrivateField(instance, "lastInterval", new Integer(3));
        setPrivateField(instance, "doMeasurement", Boolean.TRUE);
        callPrivateMethod(instance, "doMeasurement", null, null);

        Thread.sleep(1500L);
        assertEquals("The doMeasurement() method did not log the expected output: ", 4, handler.messages.size());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.WARNING, handler.messages.get(0).getLevel());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.FINEST, handler.messages.get(1).getLevel());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.FINEST, handler.messages.get(2).getLevel());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.WARNING, handler.messages.get(3).getLevel());

        handler.messages.clear();
        TestLS.measurementTime = 0L;
        TestLS.interval = 2;
        setPrivateField(instance, "doMeasurement", Boolean.FALSE);
        callPrivateMethod(instance, "doMeasurement", null, null);

        Thread.sleep(3000L);
        for(LogRecord r:handler.messages)System.out.println("MSG: " + r.getMessage());
        assertEquals("The doMeasurement() method did not log the expected output: ", 1, handler.messages.size());
        assertEquals("The doMeasurement() method did not log the expected output: ", Level.WARNING, handler.messages.get(0).getLevel());

        // Use parse to make sure the timer dies
        callPrivateMethod(instance, "parse", null, null);
    }

    /**
     * Test of adjustedInterval method, of class LoadSensorManager.
     */
    public void testAdjustedInterval() throws Exception {
        System.out.println("adjustedInterval()");
        LoadSensorManager instance = new LoadSensorManager(new TestLS());

        int expResult = 0;
        int result = 0;

        setPrivateField(instance, "lastInterval", new Integer(-1));
        setPrivateField(instance, "timeToMeasure", new Integer(5));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 0;
        setPrivateField(instance, "lastInterval", new Integer(0));
        setPrivateField(instance, "timeToMeasure", new Integer(5));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 0;
        setPrivateField(instance, "lastInterval", new Integer(4));
        setPrivateField(instance, "timeToMeasure", new Integer(-1));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 2;
        setPrivateField(instance, "lastInterval", new Integer(4));
        setPrivateField(instance, "timeToMeasure", new Integer(0));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 0;
        setPrivateField(instance, "lastInterval", new Integer(4));
        setPrivateField(instance, "timeToMeasure", new Integer(5));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 0;
        setPrivateField(instance, "lastInterval", new Integer(5));
        setPrivateField(instance, "timeToMeasure", new Integer(5));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 0;
        setPrivateField(instance, "lastInterval", new Integer(6));
        setPrivateField(instance, "timeToMeasure", new Integer(5));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 0;
        setPrivateField(instance, "lastInterval", new Integer(10));
        setPrivateField(instance, "timeToMeasure", new Integer(5));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 1;
        setPrivateField(instance, "lastInterval", new Integer(11));
        setPrivateField(instance, "timeToMeasure", new Integer(5));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);

        expResult = 10;
        setPrivateField(instance, "lastInterval", new Integer(20));
        setPrivateField(instance, "timeToMeasure", new Integer(5));
        result = (Integer)callPrivateMethod(instance, "adjustedInterval", null, null);
        assertEquals("The adjustedInterval() method did not return the expected value: ", expResult, result);
    }

    /**
     * Test of doDelayedMeasurement method, of class LoadSensorManager.
     */
    public void testTakeMeasurement() throws Exception {
        System.out.println("takeMeasurement()");
        LoadSensorManager instance = new LoadSensorManager(new TestLS());
        Logger log = Logger.getLogger("com.sun.grid.LoadSensor");
        TestHandler handler = new TestHandler();

        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);

        TestLS.measurementTime = 0L;
        callPrivateMethod(instance, "takeMeasurement", null, null);
        assertFalse("The takeMeasurement() method did not cause a measurement to be taken", TestLS.measurementTime == 0);
        assertEquals("The takeMeasurement() method did not log the expected output: ", 2, handler.messages.size());
        assertEquals("The takeMeasurement() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());
        assertEquals("The takeMeasurement() method did not log the expected output: ", Level.FINEST, handler.messages.get(1).getLevel());

        instance = new LoadSensorManager(new ExLS());
        handler.messages.clear();
        callPrivateMethod(instance, "takeMeasurement", null, null);
        assertEquals("The takeMeasurement() method did not log the expected output: ", 2, handler.messages.size());
        assertEquals("The takeMeasurement() method did not log the expected output: ", Level.FINEST, handler.messages.get(0).getLevel());
        assertEquals("The takeMeasurement() method did not log the expected output: ", Level.WARNING, handler.messages.get(1).getLevel());
    }

    private static Object callPrivateMethod(Object obj, String method, Class[] params, Object[] args) throws Exception {
        Object ret = null;
        Method m = obj.getClass().getDeclaredMethod(method, params);

        m.setAccessible(true);
        ret = m.invoke(obj, args);

        return ret;
    }

    private static <T> T getPrivateField(Object obj, String field) throws Exception {
        T ret = null;
        Field f = obj.getClass().getDeclaredField(field);

        f.setAccessible(true);
        ret = (T) f.get(obj);

        return ret;
    }

    private static void setPrivateField(Object obj, String field, Object value) throws Exception {
        Field f = obj.getClass().getDeclaredField(field);

        f.setAccessible(true);
        f.set(obj, value);
    }

    public static class TestLS implements LoadSensor {
        public static int interval = MEASURE_ON_DEMAND;
        public static long intervalTime = 0L;
        public static long measurementTime = 0L;
        public static long loadTime = 0L;

        public int getMeasurementInterval() {
            intervalTime = System.currentTimeMillis();
            return interval;
        }

        public void doMeasurement() {
            measurementTime = System.currentTimeMillis();
        }

        public Map<String, Map<String, String>> getLoadValues() {
            loadTime = System.currentTimeMillis();
            return Collections.singletonMap("host", Collections.singletonMap("load", "value"));
        }
    }

    private static class ExLS implements LoadSensor {
        public int getMeasurementInterval() {
            throw new UnsupportedOperationException("Not supported yet.");
        }

        public void doMeasurement() {
            throw new UnsupportedOperationException("Not supported yet.");
        }

        public Map<String, Map<String, String>> getLoadValues() {
            throw new UnsupportedOperationException("Not supported yet.");
        }
    }

    private static class TestHandler extends Handler {
        List<LogRecord> messages = new ArrayList<LogRecord>(8);

        @Override
        public void publish(LogRecord record) {
            messages.add(record);
        }

        @Override
        public void close() throws SecurityException {
        }

        @Override
        public void flush() {
        }
    }
}
