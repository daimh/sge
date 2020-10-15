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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.PrintStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;
import junit.framework.TestCase;

/**
 * TODO: Add tests to testProcessMaps() for null values
 * TODO: Add tests for parse() and processMaps() for implicit results changes
 * TODO: Add tests for parse() and processMaps() for REJECT shunt
 */
public class JsvManagerTest extends TestCase {
    public JsvManagerTest() {
    }

    /**
     * Test of parse method, of class Jsv.
     */
    public void testParse() throws Exception {
        System.out.println("parse()");
        JsvManager instance = new JsvManager();
        JsvImpl listener = new JsvImpl();
        ByteArrayInputStream in = new ByteArrayInputStream("START".getBytes());
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bao);
        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        instance.setIn(in);
        instance.setOut(out);
        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);

        // START
        instance.parse(listener);

        assertTrue("onStart() method was not called for START command: ", listener.started);
        assertEquals("parse() method produced unexpected output for START command: ", "SEND ENV\nSTARTED\n", bao.toString());
        assertEquals("parse() method did not produce the correct log output: ", 3, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> START", handler.messages.get(0).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(1).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< SEND ENV", handler.messages.get(1).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< STARTED", handler.messages.get(2).getMessage());

        // ENV
        in = new ByteArrayInputStream("ENV test1 test2".getBytes());
        instance.setIn(in);
        bao.reset();
        handler.messages.clear();

        instance.parse(listener);

        Map<String,String> env = getPrivateField(instance, JsvManager.class, "environment");

        assertEquals("Environment variable was not set in master environment for ENV command: ", "test2", env.get("test1"));

        Map<String,String> envLog = getPrivateField(instance, JsvManager.class, "envLog");

        assertEquals("Environment variable was not added to environment log for ENV command: ", "test1", envLog.keySet().iterator().next());
        assertEquals("Environment variable was not added to environment log for ENV command: ", "test2", envLog.values().iterator().next());
        assertEquals("parse() method produced unexpected output for ENV command: ", "", bao.toString());
        assertEquals("parse() method did not produce the correct log output: ", 1, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> ENV test1 test2", handler.messages.get(0).getMessage());

        // PARAM
        in = new ByteArrayInputStream("PARAM CLIENT test".getBytes());
        instance.setIn(in);
        bao.reset();
        handler.messages.clear();

        instance.parse(listener);

        JobDescription params = getPrivateField(instance, JsvManager.class, "parameters");

        assertEquals("Parameter was not set in JsvParameters for PARAM command: ", "test", params.getClient());

        Map<String,String> paramLog = getPrivateField(instance, JsvManager.class, "paramLog");

        assertEquals("Environment variable was not added to parameter log for PARAM command: ", "CLIENT", paramLog.keySet().iterator().next());
        assertEquals("Environment variable was not added to parameter log for PARAM command: ", "test", paramLog.values().iterator().next());
        assertEquals("parse() method produced unexpected output for PARAM command: ", "", bao.toString());
        assertEquals("parse() method did not produce the correct log output: ", 1, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> PARAM CLIENT test", handler.messages.get(0).getMessage());

        // SHOW
        in = new ByteArrayInputStream("SHOW".getBytes());
        instance.setIn(in);
        bao.reset();
        handler.messages.clear();

        instance.parse(listener);

        assertEquals("parse() method produced incorrect output for SHOW command", "LOG INFO got param: CLIENT='test'\nLOG INFO got env: test1='test2'\n", bao.toString());
        assertEquals("parse() method did not produce the correct log output: ", 3, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> SHOW", handler.messages.get(0).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(1).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< LOG INFO got param: CLIENT='test'", handler.messages.get(1).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< LOG INFO got env: test1='test2'", handler.messages.get(2).getMessage());

        // BEGIN
        in = new ByteArrayInputStream("BEGIN".getBytes());
        instance.setIn(in);
        bao.reset();
        handler.messages.clear();

        instance.parse(listener);

        params = getPrivateField(instance, JsvManager.class, "parameters");

        assertTrue("onVerify() method was not called for BEGIN command: ", listener.verified);
        assertEquals("parse() method produced incorrect output for BEGIN command: ", "PARAM A test5\nENV ADD test3 test4\nRESULT STATE CORRECT modify\n", bao.toString());
        assertEquals("parse() method did not produce the correct log output: ", 4, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> BEGIN", handler.messages.get(0).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(1).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< PARAM A test5", handler.messages.get(1).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< ENV ADD test3 test4", handler.messages.get(2).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(3).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< RESULT STATE CORRECT modify", handler.messages.get(3).getMessage());

        // QUIT
        in = new ByteArrayInputStream("QUIT".getBytes());
        instance.setIn(in);
        bao.reset();
        handler.messages.clear();

        instance.parse(listener);
        assertEquals("parse() method did not produce the correct log output: ", 1, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> QUIT", handler.messages.get(0).getMessage());
        
        in = new ByteArrayInputStream("START\nBEGIN\nQUIT\n".getBytes());
        instance.setIn(in);
        bao.reset();
        handler.messages.clear();

        instance.parse(null);

        assertEquals("parse() method produced unexpected output: ", "STARTED\nRESULT STATE ACCEPT\n", bao.toString());
        assertEquals("parse() method did not produce the correct log output: ", 5, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> START", handler.messages.get(0).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(1).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< STARTED", handler.messages.get(1).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(2).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> BEGIN", handler.messages.get(2).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(3).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", "<<< RESULT STATE ACCEPT", handler.messages.get(3).getMessage());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(4).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> QUIT", handler.messages.get(4).getMessage());

        in = new ByteArrayInputStream("NONSENSE\n".getBytes());
        instance.setIn(in);
        bao.reset();
        handler.messages.clear();

        instance.parse(null);

        assertEquals("parse() sent output even though it recieved bad input: ", "", bao.toString());
        assertEquals("parse() did not log an appropriate log message: ", 2, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> NONSENSE", handler.messages.get(0).getMessage());
        assertEquals("parse() did not log an appropriate log message: ", Level.WARNING, handler.messages.get(1).getLevel());

        in = new ByteArrayInputStream("\n".getBytes());
        instance.setIn(in);
        bao.reset();
        handler.messages.clear();

        instance.parse(null);

        assertEquals("parse() sent output even though it recieved bad input: ", "", bao.toString());
        assertEquals("parse() did not log an appropriate log message: ", 2, handler.messages.size());
        assertEquals("parse() method did not produce the correct log output: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("parse() method did not produce the correct log output: ", ">>> ", handler.messages.get(0).getMessage());
        assertEquals("parse() did not log an appropriate log message: ", Level.WARNING, handler.messages.get(1).getLevel());
    }

    /**
     * Test of log method, of class Jsv.
     */
    public void testLog() {
        System.out.println("log()");
        JsvManager.LogLevel level = JsvManager.LogLevel.INFO;
        String expected = "LOG INFO test";
        String message = "test";
        JsvManager instance = new JsvManager();
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bao);

        instance.setOut(out);
        instance.log(level, message);
        assertEquals("The log() method did not send the expected value: ", expected + "\n", bao.toString());

        bao.reset();

        try {
            instance.log(null, "test");
            fail("The log() method allowed a null level value");
        } catch (IllegalArgumentException e) {
            assertEquals("log() sent output even though it threw an exception: ", "", bao.toString());
        }
    }

    /**
     * Test of log method, of class Jsv.
     */
    public void testSetIn() throws Exception {
        System.out.println("setIn()");
        JsvManager instance = new JsvManager();
        InputStream in = new ByteArrayInputStream(new byte[8]);
        Object obj = getPrivateField(instance, JsvManager.class, "in");

        assertSame("The input stream was did not default to System.in", System.in, obj);

        instance.setIn(in);
        obj = getPrivateField(instance, JsvManager.class, "in");
        assertSame("The setIn() method did not set the input stream", in, obj);

        instance.setIn(null);
        obj = getPrivateField(instance, JsvManager.class, "in");
        assertSame("The setIn() method did not set the input stream back to System.in", System.in, obj);
    }

    /**
     * Test of log method, of class Jsv.
     */
    public void testSetOut() throws Exception {
        System.out.println("setOut()");
        JsvManager instance = new JsvManager();
        PrintStream out = new PrintStream(new ByteArrayOutputStream());
        Object obj = getPrivateField(instance, JsvManager.class, "out");

        assertSame("The input stream was did not default to System.out", System.out, obj);

        instance.setOut(out);
        obj = getPrivateField(instance, JsvManager.class, "out");
        assertSame("The setOut() method did not set the output stream", out, obj);

        instance.setOut(null);
        obj = getPrivateField(instance, JsvManager.class, "out");
        assertSame("The setOut() method did not set the output stream back to System.out", System.out, obj);
    }

    /**
     * Test of sendCommand method, of class Jsv.
     */
    public void testSendCommand() throws Throwable {
        System.out.println("sendCommand()");
        String expected = "test";
        JsvManager instance = new JsvManager();
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bao);
        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        instance.setOut(out);
        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);
        callPrivateMethod(instance, JsvManager.class, "sendCommand", new Class[]{String.class}, new Object[]{expected});
        assertEquals("sendCommand() did not send the expected value: ", expected + "\n", bao.toString());
        assertEquals("sendCommand() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendCommand() did not log an appropriate log message: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("sendCommand() did not log an appropriate log message: ", "<<< " + expected, handler.messages.get(0).getMessage());

        bao.reset();
        handler.messages.clear();
        expected = "test1 test2 test3";

        callPrivateMethod(instance, JsvManager.class, "sendCommand", new Class[]{String.class}, new Object[]{"test1\ntest2\rtest3\n\r"});
        assertEquals("sendCommand() did not send the expected value: ", expected + "\n", bao.toString());
        assertEquals("sendCommand() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendCommand() did not log an appropriate log message: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("sendCommand() did not log an appropriate log message: ", "<<< " + expected, handler.messages.get(0).getMessage());

        bao.reset();
        handler.messages.clear();

        callPrivateMethod(instance, JsvManager.class, "sendCommand", new Class[]{String.class}, new Object[]{null});
        assertEquals("sendCommand() sent output even though it recieved a null command: ", "", bao.toString());
        assertEquals("sendCommand() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendCommand() did not log an appropriate log message: ", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of sendEnvironment method, of class Jsv.
     */
    public void testSendEnvironment() throws Throwable {
        System.out.println("sendEnvironment()");
        String expected = "ENV ADD test1 test2";
        JsvManager instance = new JsvManager();
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bao);
        Class opClass = Class.forName("com.sun.grid.jsv.JsvManager$Operation");
        Field addField = opClass.getDeclaredField("ADD");
        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        instance.setOut(out);
        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);

        callPrivateMethod(instance, JsvManager.class, "sendEnvironment", new Class[]{opClass, String.class, String.class}, new Object[]{addField.get(null), "test1", "test2"});
        assertEquals("sendEnvironment() did not send the expected value", expected + "\n", bao.toString());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", "<<< " + expected, handler.messages.get(0).getMessage());

        bao.reset();
        handler.messages.clear();

        callPrivateMethod(instance, JsvManager.class, "sendEnvironment", new Class[]{opClass, String.class, String.class}, new Object[]{addField.get(null), null, "test2"});
        assertEquals("sendEnvironment() sent output even though it received a null var: ", "", bao.toString());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", Level.WARNING, handler.messages.get(0).getLevel());

        bao.reset();
        handler.messages.clear();

        callPrivateMethod(instance, JsvManager.class, "sendEnvironment", new Class[]{opClass, String.class, String.class}, new Object[]{addField.get(null), "test2", null});
        assertEquals("sendEnvironment() sent output even though it received a null value: ", "", bao.toString());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", Level.WARNING, handler.messages.get(0).getLevel());

        bao.reset();
        handler.messages.clear();

        callPrivateMethod(instance, JsvManager.class, "sendEnvironment", new Class[]{opClass, String.class, String.class}, new Object[]{null, "test1", "test2"});
        assertEquals("sendEnvironment() sent output even though it received a null op: ", "", bao.toString());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendEnvironment() did not log an appropriate log message: ", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of sendParameter method, of class Jsv.
     */
    public void testSendParameter() throws Throwable {
        System.out.println("sendParameter()");
        String parameter = "test1";
        String value = "test2";
        String expected = "PARAM " + parameter + " " + value;
        JsvManager instance = new JsvManager();
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bao);
        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        instance.setOut(out);
        log.addHandler(handler);
        log.setLevel(Level.ALL);
        handler.setLevel(Level.ALL);
        callPrivateMethod(instance, JsvManager.class, "sendParameter", new Class[]{String.class, String.class}, new Object[]{parameter, value});
        assertEquals("sendParameter() did not send the expected value", expected + "\n", bao.toString());
        assertEquals("sendParameter() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendParameter() did not log an appropriate log message: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("sendParameter() did not log an appropriate log message: ", "<<< " + expected, handler.messages.get(0).getMessage());

        bao.reset();
        handler.messages.clear();

        callPrivateMethod(instance, JsvManager.class, "sendParameter", new Class[]{String.class, String.class}, new Object[]{null, value});
        assertEquals("sendParameter() sent output even though it received a null parameter: ", "", bao.toString());
        assertEquals("sendParameter() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendParameter() did not log an appropriate log message: ", Level.WARNING, handler.messages.get(0).getLevel());

        bao.reset();
        handler.messages.clear();
        expected = "PARAM " + parameter;

        callPrivateMethod(instance, JsvManager.class, "sendParameter", new Class[]{String.class, String.class}, new Object[]{parameter, null});
        assertEquals("sendParameter() did not send the expected value", expected + "\n", bao.toString());
        assertEquals("sendParameter() did not log an appropriate log message: ", 1, handler.messages.size());
        assertEquals("sendParameter() did not log an appropriate log message: ", Level.FINE, handler.messages.get(0).getLevel());
        assertEquals("sendParameter() did not log an appropriate log message: ", "<<< " + expected, handler.messages.get(0).getMessage());
    }

    /**
     * Test of prepareMaps method, of class Jsv.
     */
    public void testPrepareMaps() throws Exception {
        System.out.println("prepareMaps()");
        JsvManager instance = new JsvManager();
        JobDescription params = instance.getJobDescription();
        Map<String,String> env = getPrivateField(instance, JsvManager.class, "environment");

        params.setContext("test");
        env.put("test1", "test2");

        callPrivateMethod(instance, JsvManager.class, "prepareMaps", new Class[0], new Object[0]);
        
        JobDescription baseline = (JobDescription)getPrivateField(params, JobDescription.class, "baseline");
        env = baseline.getEnvironment();

        assertNotNull("The prepareMaps() method did not set the baseline", baseline);
        assertEquals("The prepareMaps() method did not set the baseline", "test", baseline.getContext());
        assertNotNull("The prepareMaps() method did not set the environment in the baseline", env);
        assertEquals("The prepareMaps() method did not set the environment in the baseline", "test2", env.get("test1"));
    }

    /**
     * Test of processMaps method, of class Jsv.
     */
    public void testProcessMaps() throws Exception {
        System.out.println("processMaps()");
        JsvManager instance = new JsvManager();
        JobDescription params = instance.getJobDescription();
        Map<String,String> env = new HashMap<String, String>();
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bao);
        String expected = "PARAM display test2\nENV ADD test7 test8\nENV MOD test3 test9\nENV DEL test5 test6\n";

        params.setDisplay("test1");
        env.put("test1", "test2");
        env.put("test3", "test4");
        env.put("test5", "test6");
        params.setEnvironment(env);

        params.setBaseline();

        params.setDisplay("test2");
        env.clear();
        env.put("test1", "test2");
        env.put("test3", "test9");
        env.put("test7", "test8");
        params.setEnvironment(env);

        instance.setOut(out);
        callPrivateMethod(instance, JsvManager.class, "processMaps", new Class[0], new Object[0]);
        assertEquals("processMaps() did not send the expected values", expected, bao.toString());
    }

    /**
     * Test of sendMaps method, of class Jsv.
     */
    public void testLogMaps() throws Exception {
        System.out.println("logMaps()");
        JsvManager instance = new JsvManager();
        Map<String,String> env = getPrivateField(instance, JsvManager.class, "envLog");
        Map<String,String> params = getPrivateField(instance, JsvManager.class, "paramLog");
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bao);
        String expected = "LOG INFO got param: display='test2'\nLOG INFO got env: test1='test2'\n";

        params.put("display", "test2");
        env.put("test1", "test2");

        instance.setOut(out);
        callPrivateMethod(instance, JsvManager.class, "logMaps", new Class[0], new Object[0]);
        assertEquals("logMaps() did not send the expected values", expected, bao.toString());
    }

    /**
     * Test of sendResult method, of class Jsv.
     */
    public void testSendResult() throws Exception {
        System.out.println("sendResult()");
        String accept = "RESULT STATE ACCEPT";
        String modify = "RESULT STATE CORRECT";
        String reject = "RESULT STATE REJECT";
        String wait = "RESULT STATE REJECT_WAIT";
        JsvManager instance = new JsvManager();
        ByteArrayOutputStream bao = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bao);

        instance.setOut(out);
        callPrivateMethod(instance, JsvManager.class, "sendResult", new Class[0], new Object[0]);

        assertEquals("The sendResult() method did not send the expected value: ", accept + "\n", bao.toString());

        bao.reset();
        instance.accept("test");
        callPrivateMethod(instance, JsvManager.class, "sendResult", new Class[0], new Object[0]);

        assertEquals("sendResult() did not send the expected value", accept + " test\n", bao.toString());

        bao.reset();
        instance.modify("test");
        callPrivateMethod(instance, JsvManager.class, "sendResult", new Class[0], new Object[0]);

        assertEquals("sendResult() did not send the expected value", modify + " test\n", bao.toString());

        bao.reset();
        instance.reject("test");
        callPrivateMethod(instance, JsvManager.class, "sendResult", new Class[0], new Object[0]);

        assertEquals("sendResult() did not send the expected value", reject + " test\n", bao.toString());

        bao.reset();
        instance.rejectWait("test");
        callPrivateMethod(instance, JsvManager.class, "sendResult", new Class[0], new Object[0]);

        assertEquals("sendResult() did not send the expected value", wait + " test\n", bao.toString());
    }

    /**
     * Test of ressetState method, of class Jsv.
     */
    public void testResetState() throws Exception {
        System.out.println("resetState()");

        JsvManager instance = new JsvManager();

        callPrivateMethod(instance, JsvManager.class, "resetState", new Class[0], new String[0]);
        checkClean(instance, "resetState", "");
        
        JobDescription params = null;
        Map<String,String> env = null;
        JsvManager.Result result = null;
        String msg = null;
        Boolean request = null;
        Map<String,String> envLog = null;
        Map<String,String> paramLog = null;

        try {
            params = getPrivateField(instance, JsvManager.class, "parameters");
            env = getPrivateField(instance, JsvManager.class, "environment");
            result = getPrivateField(instance, JsvManager.class, "result");
            msg = getPrivateField(instance, JsvManager.class, "resultMessage");
            request = getPrivateField(instance, JsvManager.class, "requestEnv");
            envLog = getPrivateField(instance, JsvManager.class, "envLog");
            paramLog = getPrivateField(instance, JsvManager.class, "paramLog");
        } catch (Throwable t) {
            fail(t.getClass().getName() + ": " + t.getMessage());
        }

        params.setClient("test");
        env.put("test1", "test2");
        result = JsvManager.Result.REJECT;
        msg = "test";
        request = true;
        envLog.put("test3", "test4");
        paramLog.put("test5", "test6");

        callPrivateMethod(instance, JsvManager.class, "resetState", new Class[0], new String[0]);
        checkClean(instance, "resetState", "");
    }

    /**
     * Test of sendResult method, of class Jsv.
     */
    public void testRequestEnvironment() throws Exception {
        System.out.println("requestEnvironment()");

        JsvManager instance = new JsvManager();
        Boolean request = null;

        instance.requestEnvironment(true);
        request = getPrivateField(instance, JsvManager.class, "requestEnv");

        assertTrue("requestEnvironment() method did not set the proper value: ", request);

        instance.requestEnvironment(false);
        request = getPrivateField(instance, JsvManager.class, "requestEnv");

        assertFalse("requestEnvironment() method did not set the proper value: ", request);
    }

    /**
     * Test of sendResult method, of class Jsv.
     */
    public void testGetParameters() throws Exception {
        System.out.println("getParameters()");

        JsvManager instance = new JsvManager();
        JobDescription expected = getPrivateField(instance, JsvManager.class, "parameters");
        JobDescription result = instance.getJobDescription();

        assertSame("The getEnvironment() method did not return the JSV parameters", expected, result);
    }

    /**
     * Test of result property, of class Jsv.
     */
    public void testResult() throws Exception {
        System.out.println("result");

        JsvManager instance = new JsvManager();
        JsvManager.Result expectedResult = JsvManager.Result.ACCEPT;
        JsvManager.Result result = instance.getResult();
        String message = null;

        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The default result was not ACCEPT: ", expectedResult, result);
        assertNull("The default message was not null: ", message);

        expectedResult = JsvManager.Result.MODIFY;
        instance.setResult(JsvManager.Result.MODIFY, "accept");
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by setResult(): ", expectedResult, result);
        assertEquals("The setResult() method did not store the correct message: ", "accept", message);

        expectedResult = JsvManager.Result.REJECT;
        instance.setResult(JsvManager.Result.REJECT, null);
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by setResult(): ", expectedResult, result);
        assertNull("The setResult() method did not store the correct message: ", message);

        try {
            instance.setResult(null, null);
            fail("The setResult() method allowed a null value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setResult() method set the result even though it threw an exception: ", expectedResult, result);
        }
    }

    /**
     * Test of accept method, of class Jsv.
     */
    public void testAccept() throws Exception {
        System.out.println("accept()");

        JsvManager instance = new JsvManager();
        JsvManager.Result expectedResult = JsvManager.Result.ACCEPT;
        JsvManager.Result result = null;
        String message = null;

        instance.accept("test");
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by accept(): ", expectedResult, result);
        assertEquals("The accept() method did not store the correct message: ", "test", message);

        instance.accept(null);
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by accept(): ", expectedResult, result);
        assertNull("The accept() method did not store the correct message: ", message);
    }

    /**
     * Test of accept method, of class Jsv.
     */
    public void testModify() throws Exception {
        System.out.println("modify()");

        JsvManager instance = new JsvManager();
        JsvManager.Result expectedResult = JsvManager.Result.MODIFY;
        JsvManager.Result result = null;
        String message = null;

        instance.modify("test");
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by modify(): ", expectedResult, result);
        assertEquals("The modify() method did not store the correct message: ", "test", message);

        instance.modify(null);
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by modify(): ", expectedResult, result);
        assertNull("The modify() method did not store the correct message: ", message);
    }

    /**
     * Test of accept method, of class Jsv.
     */
    public void testReject() throws Exception {
        System.out.println("reject()");

        JsvManager instance = new JsvManager();
        JsvManager.Result expectedResult = JsvManager.Result.REJECT;
        JsvManager.Result result = null;
        String message = null;

        instance.reject("test");
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by reject(): ", expectedResult, result);
        assertEquals("The reject() method did not store the correct message: ", "test", message);

        instance.reject(null);
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by reject(): ", expectedResult, result);
        assertNull("The reject() method did not store the correct message: ", message);
    }

    /**
     * Test of accept method, of class Jsv.
     */
    public void testRejectWait() throws Exception {
        System.out.println("rejectWait()");

        JsvManager instance = new JsvManager();
        JsvManager.Result expectedResult = JsvManager.Result.REJECT_WAIT;
        JsvManager.Result result = null;
        String message = null;

        instance.rejectWait("test");
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by rejectWait(): ", expectedResult, result);
        assertEquals("The rejectWait() method did not store the correct message: ", "test", message);

        instance.rejectWait(null);
        result = instance.getResult();
        message = getPrivateField(instance, JsvManager.class, "resultMessage");

        assertEquals("The getResult() method did not return the value set by rejectWait(): ", expectedResult, result);
        assertNull("The rejectWait() method did not store the correct message: ", message);
    }

    private static void checkClean(JsvManager instance, String method, String when) {
        JobDescription params = null;
        Map<String,String> env = null;
        JsvManager.Result result = null;
        String msg = null;
        Boolean request = null;
        Map<String,String> envLog = null;
        Map<String,String> paramLog = null;

        try {
            params = getPrivateField(instance, JsvManager.class, "parameters");
            env = getPrivateField(instance, JsvManager.class, "environment");
            result = getPrivateField(instance, JsvManager.class, "result");
            msg = getPrivateField(instance, JsvManager.class, "resultMessage");
            request = getPrivateField(instance, JsvManager.class, "requestEnv");
            envLog = getPrivateField(instance, JsvManager.class, "envLog");
            paramLog = getPrivateField(instance, JsvManager.class, "paramLog");
        } catch (Throwable t) {
            fail(t.getClass().getName() + ": " + t.getMessage());
        }

        assertEquals(method + "() method did not clear environment" + when + ": ", 0, env.size());
        assertEquals(method + "() method did not clear environment log" + when + ": ", 0, envLog.size());
        assertEquals(method + "() method did not clear parameter log" + when + ": ", 0, paramLog.size());
        assertEquals(method + "() method did not reset result" + when + ": ", JsvManager.Result.ACCEPT, result);
        assertNull(method + "() method did not reset result message" + when + ": ", msg);
        assertFalse(method + "() method did not reset environmentRequest" + when + ": ", request);

        // We have to do some cleverness to make this test work
        JobDescription baseline = new JobDescription();
        JobDescription compare = params.clone();

        try {
            Field f = JobDescription.class.getDeclaredField("baseline");

            f.setAccessible(true);
            f.set(compare, baseline);

            assertEquals(method + "() method did not clear parameters" + when + ": ", 0, compare.getDifference().size());
        } catch (Exception e) {
            fail(e.getClass().getName() + ": " + e.getMessage());
        }
    }

    public class JsvImpl implements Jsv {
        public boolean started = false;
        public boolean verified = false;

        public void onStart(JsvManager jsv) {
            checkClean(jsv, "parse", " before START command");
            jsv.requestEnvironment(true);
            started = true;
        }

        public void onVerify(JsvManager jsv) {
            Map<String,String> env = jsv.getJobDescription().getEnvironment();

            env.put("test3", "test4");
            jsv.getJobDescription().setEnvironment(env);
            jsv.getJobDescription().setAccount("test5");
            jsv.modify("modify");

            verified = true;
        }
    }

    private static Object callPrivateMethod(Object obj, Class clazz, String method, Class[] params, Object[] args) throws Exception {
        Object ret = null;
        Method m = clazz.getDeclaredMethod(method, params);

        m.setAccessible(true);
        ret = m.invoke(obj, args);

        return ret;
    }

    private static <T> T getPrivateField(Object obj, Class clazz, String field) throws Exception {
        T ret = null;
        Field f = clazz.getDeclaredField(field);

        f.setAccessible(true);
        ret = (T) f.get(obj);

        return ret;
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
