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

import com.sun.grid.jsv.JobDescription.Verification;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.logging.LogRecord;

import java.util.List;
import java.util.Map;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import junit.framework.TestCase;

/**
 * The read-only properties can only be set until the baseline is set.
 * The read/write properties can be always be set.
 * TODO: Add test for clone()
 */
public class JobDescriptionTest extends TestCase {
    public JobDescriptionTest() {
    }

    /**
     * Test of getAccount method, of class JsvParameters.
     */
    public void testAccount() {
        System.out.println("account");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getAccount();

        instance.setAccount(expResult);

        String result = instance.getAccount();

        assertEquals("The getAccount() method did not return the value set with setAccount(): ", expResult, result);

        instance.setAccount(null);
        result = instance.getAccount();
        assertNull("The getAccount() method did not return the value set with setAccount(): ", result);

        instance.setAccount(expResult);
        instance.setBaseline();

        instance.setAccount(null);
        result = instance.getAccount();
        assertNull("The getAccount() method did not return the value set with setAccount(): ", result);
    }

    /**
     * Test of getAdvanceReservationId method, of class JsvParameters.
     */
    public void testAdvanceReservationId() {
        System.out.println("advanceReservationId");

        JobDescription instance = new JobDescription();

        // Test that default value doesn't cause an exception
        instance.getAdvanceReservationId();

        Integer expResult = 7;

        instance.setAdvanceReservationId(expResult);

        Integer result = instance.getAdvanceReservationId();

        assertEquals("The getAdvanceReservationId() method did not return the value set with setAdvanceReservationId(): ", expResult, result);

        try {
            instance.setAdvanceReservationId(-1);
            fail("The setAdvanceReservationId() method accepted a negative id value");
        } catch (IllegalArgumentException e) {
            assertEquals("The getAdvanceReservationId() method set the id value even though it threw an exception: ", expResult, instance.getAdvanceReservationId());
        }

        instance.setAdvanceReservationId(null);
        result = instance.getAdvanceReservationId();
        assertNull("The getAdvanceReservationId() method did not return the value set with setAdvanceReservationId(): ", result);

        instance.setAdvanceReservationId(expResult);
        instance.setBaseline();

        instance.setAdvanceReservationId(null);
        result = instance.getAdvanceReservationId();
        assertNull("The getAdvanceReservationId() method did not return the value set with setAdvanceReservationId(): ", result);
    }

    /**
     * Test of getCheckpointSpecifier method, of class JsvParameters.
     */
    
    public void testCheckpointSpecifier() {
        System.out.println("checkpointSpecifier");

        JobDescription instance = new JobDescription();

        CheckpointSpecifier expResult = new CheckpointSpecifier();

        expResult.setInterval(100L);

        try {
            instance.setCheckpointSpecifier(expResult);
            fail("The setCheckpointSpecifier() method accepted an invalid CheckpointSpecifier object");
        } catch (IllegalArgumentException e) {
            assertEquals("The setCheckpointSpecifier() method set the checkpoint specifier even though it threw an exception: ", null, instance.getCheckpointSpecifier());
        }

        expResult.setName("ckpt");
        instance.setCheckpointSpecifier(expResult);

        CheckpointSpecifier result = instance.getCheckpointSpecifier();

        assertEquals("The getCheckpointSpecifier() method did not return the value set with setCheckpointSpecifier(): ", expResult, result);

        expResult.setInterval(9L);
        instance.setCheckpointSpecifier(expResult);
        result = instance.getCheckpointSpecifier();
        assertEquals("The getCheckpointSpecifier() method did not return the value set with setCheckpointSpecifier(): ", expResult, result);

        instance.setCheckpointSpecifier(null);
        result = instance.getCheckpointSpecifier();
        assertEquals("The getCheckpointSpecifier() method did not return the value set with setCheckpointSpecifier(): ", null, result);

        instance.setCheckpointSpecifier(expResult);
        instance.setBaseline();

        instance.setCheckpointSpecifier(null);
        result = instance.getCheckpointSpecifier();
        assertEquals("The getCheckpointSpecifier() method did not return the value set with setCheckpointSpecifier(): ", null, result);
    }

    /**
     * Test of getClient method, of class JsvParameters.
     */
    
    public void testClient() {
        System.out.println("client");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getClient();

        instance.setClient(expResult);

        String result = instance.getClient();

        assertEquals("The getClient() method did not return the value set with setClient(): ", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        instance.setClient(null);
        result = instance.getClient();
        
        assertEquals("The getClient() method did not ignore the null value: ", expResult, result);
        assertEquals("The getClient() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The getClient() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        instance.setClient(expResult);
        instance.setBaseline();

        try {
            instance.setClient("test2");
            fail("The setClient() method allowed the client to modified after the baseline was set");
        } catch (IllegalStateException e) {
            assertEquals("The setClient() method set the client even though it threw an exception: ", expResult, instance.getClient());
        }
    }

    /**
     * Test of getCommmandArgs method, of class JsvParameters.
     */
    
    public void testCommmandArgs() {
        System.out.println("commmandArgs");

        JobDescription instance = new JobDescription();
        String[] expResult = new String[] { "test1", "test2" };

        // Test that default value doesn't cause an exception
        instance.getCommandArgs();
        instance.setCommandArgs(expResult);

        String[] result = instance.getCommandArgs();

        assertTrue("The getCommandArgs() method did not return the value set with setCommandArgs(): ", Arrays.deepEquals(expResult, result));

        result[0] = "test3";
        result = instance.getCommandArgs();

        assertTrue("The getCommandArgs() method did not return a copy of the arg list", Arrays.deepEquals(expResult, result));

        instance.setCommandArgs(new String[0]);
        result = instance.getCommandArgs();

        assertEquals("The getCommandArgs() method did not return the value set with setCommandArgs()", 0, result.length);

        instance.setCommandArgs(null);
        result = instance.getCommandArgs();

        assertEquals("The getCommandArgs() method did not return the value set with setCommandArgs()", 0, result.length);

        instance.setCommandArgs(expResult);

        try {
            instance.setCommandArgs(new String[] { null });
            fail("The setCommandArgs() method allowed an arg array with a null entry");
        } catch (IllegalArgumentException e) {
            assertTrue("The setCommandArgs() method set the arg array even though it threw an exception", Arrays.deepEquals(expResult, instance.getCommandArgs()));
        }

        try {
            instance.setCommandArgs(new String[] { "test1", null });
            fail("The setCommandArgs() method allowed an arg array with a null entry");
        } catch (IllegalArgumentException e) {
            assertTrue("The setCommandArgs() method set the arg array even though it threw an exception", Arrays.deepEquals(expResult, instance.getCommandArgs()));
        }

        try {
            instance.setCommandArgs(new String[] { null, "test2" });
            fail("The setCommandArgs() method allowed an arg array with a null entry");
        } catch (IllegalArgumentException e) {
            assertTrue("The setCommandArgs() method set the arg array even though it threw an exception", Arrays.deepEquals(expResult, instance.getCommandArgs()));
        }

        try {
            instance.setCommandArgs(new String[] { "test1", null, "test2" });
            fail("The setCommandArgs() method allowed an arg array with a null entry");
        } catch (IllegalArgumentException e) {
            assertTrue("The setCommandArgs() method set the arg array even though it threw an exception", Arrays.deepEquals(expResult, instance.getCommandArgs()));
        }

        // Test with a larger array
        expResult = new String[] { "test1", "test2", "test3" };
        instance.setCommandArgs(expResult);
        result = instance.getCommandArgs();

        assertTrue("The getCommandArgs() method did not return the value set with setCommandArgs(): ", Arrays.deepEquals(expResult, result));

        // Test with a smaller array
        expResult = new String[] { "test1" };
        instance.setCommandArgs(expResult);
        result = instance.getCommandArgs();

        assertTrue("The getCommandArgs() method did not return the value set with setCommandArgs(): ", Arrays.deepEquals(expResult, result));
    }

    /**
     * Test of getCommandName method, of class JsvParameters.
     */
    
    public void testCommandName() {
        System.out.println("commandName");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getCommandName();

        instance.setCommandName(expResult);

        String result = instance.getCommandName();

        assertEquals("The getCommandName() method did not return the value set with setCommandName(): ", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        instance.setCommandName(null);
        result = instance.getCommandName();

        assertEquals("The getCommandName() method did not ignore the null value: ", expResult, result);
        assertEquals("The getCommandName() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The getCommandName() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        instance.setCommandName(expResult);
        instance.setBaseline();

        try {
            instance.setCommandName("test2");
            fail("The setCommandName() method allowed the command name to be modified after the baseline was set");
        } catch (IllegalStateException e) {
            assertEquals("The setCommandName() method set the command name even though it threw an exception: ", expResult, instance.getCommandName());
        }
    }

    /**
     * Test of getContext method, of class JsvParameters.
     */
    
    public void testContext() {
        System.out.println("context");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getContext();

        instance.setContext(expResult);

        String result = instance.getContext();

        assertEquals("The getContext() method did not return the value set with setContext(): ", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        instance.setContext(null);
        result = instance.getContext();

        assertEquals("The setContext() method did not ignore the null value: ", expResult, result);
        assertEquals("The setContext() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The setContext() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        instance.setBaseline();

        try {
            instance.setContext("test2");
            fail("The setContext() method allowed the context to modified after the baseline was set");
        } catch (IllegalStateException e) {
            assertEquals("The setContext() method set the context even though it threw an exception: ", expResult, instance.getContext());
        }
    }

    /**
     * Test of getWorkingDirectory method, of class JsvParameters.
     */
    
    public void testWorkingDirectory() {
        System.out.println("workingDirectory");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getWorkingDirectory();

        instance.setWorkingDirectory(expResult);

        String result = instance.getWorkingDirectory();

        assertEquals("The getWorkingDirectory() method did not return the value set with setWorkingDirectory(): ", expResult, result);

        instance.setWorkingDirectory(null);
        result = instance.getWorkingDirectory();
        assertNull("The getWorkingDirectory() method did not return the value set with setWorkingDirectory(): ", result);

        instance.setWorkingDirectory(expResult);
        instance.setBaseline();

        instance.setWorkingDirectory(null);
        result = instance.getWorkingDirectory();
        assertNull("The getWorkingDirectory() method did not return the value set with setWorkingDirectory(): ", result);
    }

    /**
     * Test of getDeadline method, of class JsvParameters.
     */
    
    public void testDeadline() {
        System.out.println("deadline");

        JobDescription instance = new JobDescription();
        Calendar expResult = Calendar.getInstance();

        // Test that default value doesn't cause an exception
        instance.getDeadline();

        expResult.set(Calendar.YEAR, 2101);

        instance.setDeadline(expResult);

        Calendar result = instance.getDeadline();

        assertEquals("The getDeadline() method did not return the value set with setDeadline(): ", expResult, result);

        result.set(Calendar.YEAR, 2010);
        result = instance.getDeadline();
        assertEquals("The getDeadline() method did not return a copy of the deadline object: ", expResult, result);

        instance.setDeadline(null);
        result = instance.getDeadline();
        assertNull("The getDeadline() method did not return the value set with setDeadline(): ", result);

        instance.setDeadline(expResult);
        instance.setBaseline();

        instance.setDeadline(null);
        result = instance.getDeadline();
        assertNull("The getDeadline() method did not return the value set with setDeadline(): ", result);
    }

    /**
     * Test of getDisplay method, of class JsvParameters.
     */
    
    public void testDisplay() {
        System.out.println("display");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getDisplay();

        instance.setDisplay(expResult);

        String result = instance.getDisplay();

        assertEquals("The getDisplay() method did not return the value set with setDisplay(): ", expResult, result);

        instance.setDisplay(null);
        result = instance.getDisplay();
        assertNull("The getDisplay() method did not return the value set with setDisplay(): ", result);

        instance.setDisplay(expResult);
        instance.setBaseline();

        instance.setDisplay(null);
        result = instance.getDisplay();
        assertNull("The getDisplay() method did not return the value set with setDisplay(): ", result);
    }

    /**
     * Test of getErrorPath method, of class JsvParameters.
     */
    
    public void testErrorPath() {
        System.out.println("errorPath()");

        JobDescription instance = new JobDescription();
        Map<String, String> expResult = new HashMap<String, String>();

        // Test that default value doesn't cause an exception
        instance.getErrorPath();

        expResult.put(null, "test3");
        expResult.put("test1", "test2");

        instance.setErrorPath(expResult);

        Map<String, String> result = instance.getErrorPath();

        assertEquals("The getErrorPath() method did not return the value set with setErrorPath(): ", expResult, result);

        result.put("test1", "test3");
        result = instance.getErrorPath();
        assertEquals("The getErrorPath() method did not return a copy of the Collection: ", expResult, instance.getErrorPath());

        instance.setErrorPath(new HashMap<String, String>());
        result = instance.getErrorPath();
        assertNull("The getErrorPath() method did not return the value set with setErrorPath()", result);

        instance.setErrorPath(null);
        result = instance.getErrorPath();
        assertNull("The getErrorPath() method did not return the value set with setErrorPath(): ", result);

        Map<String, String> bad = new HashMap<String, String>();

        bad.put("test", null);
        instance.setErrorPath(expResult);

        try {
            instance.setErrorPath(bad);
            fail("The setErrorPath() method allowed a path list with a null entry value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setErrorPath() method set the path list even though it threw an exception: ", expResult, instance.getErrorPath());
        }

        bad.clear();
        bad.put(null, null);

        try {
            instance.setErrorPath(bad);
            fail("The setErrorPath() method allowed a path list with a null entry value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setErrorPath() method set the path list even though it threw an exception: ", expResult, instance.getErrorPath());
        }

        instance.setErrorPath(expResult);
        instance.setBaseline();

        instance.setErrorPath(null);
        result = instance.getErrorPath();
        assertNull("The getErrorPath() method did not return the value set with setErrorPath(): ", result);
    }

    /**
     * Test of getGroup method, of class JsvParameters.
     */
    
    public void testGroup() {
        System.out.println("group");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getGroup();

        instance.setGroup(expResult);

        String result = instance.getGroup();

        assertEquals("The getGroup() method did not return the value set with setGroup(): ", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        instance.setGroup(null);
        result = instance.getGroup();

        assertEquals("The setGroup() method did not ignore the null value: ", expResult, result);
        assertEquals("The setGroup() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The setGroup() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        instance.setBaseline();

        try {
            instance.setGroup("test2");
            fail("The setGroup() method allowed the group to modified after the baseline was set");
        } catch (IllegalStateException e) {
            assertEquals("The setGroup() method set the group even though it threw an exception: ", expResult, instance.getGroup());
        }
    }

    /**
     * Test of getHardQueue method, of class JsvParameters.
     */
    
    public void testHardQueue() {
        System.out.println("hardQueue");

        JobDescription instance = new JobDescription();
        List<String> expResult = new ArrayList<String>(1);

        // Test that default value doesn't cause an exception
        instance.getHardQueue();

        expResult.add("test1");

        instance.setHardQueue(expResult);

        List<String> result = instance.getHardQueue();

        assertEquals("The getHardQueue() method did not return the value set with setHardQueue(): ", "test1", result.get(0));

        result.add("test3");
        result = instance.getHardQueue();
        assertEquals("The getHardQueue() method did not return a copy of the Collection: ", expResult, result);

        instance.setHardQueue(Arrays.asList(new String[0]));
        result = instance.getHardQueue();
        assertNull("The getHardQueue() method did not return the value set with setHardQueue()", instance.getHardQueue());

        instance.setHardQueue(null);
        result = instance.getHardQueue();
        assertNull("The getHardQueue() method did not return the value set with setHardQueue(): ", result);

        instance.setHardQueue(expResult);

        try {
            instance.setHardQueue(Arrays.asList(new String[] { null }));
            fail("The setHardQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHardQueue() method set the queue list even though it threw an exception: ", expResult, instance.getHardQueue());
        }

        try {
            instance.setHardQueue(Arrays.asList(new String[] { "x", null }));
            fail("The setHardQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHardQueue() method set the queue list even though it threw an exception: ", expResult, instance.getHardQueue());
        }

        try {
            instance.setHardQueue(Arrays.asList(new String[] { null, "x" }));
            fail("The setHardQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHardQueue() method set the queue list even though it threw an exception: ", expResult, instance.getHardQueue());
        }

        try {
            instance.setHardQueue(Arrays.asList(new String[] { "x", null, "x" }));
            fail("The setHardQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHardQueue() method set the queue list even though it threw an exception: ", expResult, instance.getHardQueue());
        }

        instance.setHardQueue(expResult);
        instance.setBaseline();

        instance.setHardQueue(null);
        result = instance.getHardQueue();
        assertNull("The getHardQueue() method did not return the value set with setHardQueue(): ", result);
    }

    /**
     * Test of getHardResourceRequirements method, of class JsvParameters.
     */
    
    public void testHardResourceRequirements() {
        System.out.println("hardResourceRequirements");

        JobDescription instance = new JobDescription();
        Map<String, String> expResult = new HashMap<String, String>();

        // Test that default value doesn't cause an exception
        instance.getHardResourceRequirements();

        expResult.put("test1", "test2");

        instance.setHardResourceRequirements(expResult);

        Map<String, String> result = instance.getHardResourceRequirements();

        assertEquals("The getHardResourceRequirements() method did not return the value set with setHardResourceRequirements(): ", expResult, result);

        result.put("test1", "test3");
        result = instance.getHardResourceRequirements();
        assertEquals("The getHardResourceRequirements() method did not return a copy of the Collection: ", expResult, result);

        instance.setHardResourceRequirements(new HashMap<String, String>());
        result = instance.getHardResourceRequirements();
        assertNull("The getHardResourceRequirements() method did not return the value set with setHardResourceRequirements(): ", result);

        instance.setHardResourceRequirements(null);
        result = instance.getHardResourceRequirements();
        assertNull("The getHardResourceRequirements() method did not return the value set with setHardResourceRequirements(): ", result);

        Map<String, String> bad = new HashMap<String, String>();

        instance.setHardResourceRequirements(expResult);
        bad.put("test", null);

        try {
            instance.setHardResourceRequirements(bad);
            fail("The setHardResourceRequirements() method allowed resource requirements with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The getHardResourceRequirements() method set the resource requirements even though it threw an exception: ", expResult, instance.getHardResourceRequirements());
        }

        bad.clear();
        bad.put(null, "test");

        try {
            instance.setHardResourceRequirements(bad);
            fail("The setHardResourceRequirements() method allowed resource requirements with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The getHardResourceRequirements() method set the resource requirements even though it threw an exception: ", expResult, instance.getHardResourceRequirements());
        }

        bad.clear();
        bad.put(null, null);

        try {
            instance.setHardResourceRequirements(bad);
            fail("The setHardResourceRequirements() method allowed resource requirements with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The getHardResourceRequirements() method set the resource requirements even though it threw an exception: ", expResult, instance.getHardResourceRequirements());
        }

        instance.setHardResourceRequirements(expResult);
        instance.setBaseline();

        instance.setHardResourceRequirements(null);
        result = instance.getHardResourceRequirements();
        assertNull("The getHardResourceRequirements() method did not return the value set with setHardResourceRequirements(): ", result);
    }

    /**
     * Test of getHoldArrayJobIds method, of class JsvParameters.
     */
    
    public void testHoldArrayJobIds() {
        System.out.println("holdArrayJobIds");

        JobDescription instance = new JobDescription();
        List<String> expResult = new ArrayList<String>(1);

        // Test that default value doesn't cause an exception
        instance.getHoldArrayJobIds();

        expResult.add("123");

        instance.setHoldArrayJobIds(expResult);

        List<String> result = instance.getHoldArrayJobIds();

        assertEquals("The getHoldArrayJobIds() method did not return the value set with setHoldArrayJobIds()", "123", result.get(0));

        result.add("456");
        result = instance.getHoldArrayJobIds();
        assertEquals("The getHoldArrayJobIds() method did not return a copy of the Collection: ", expResult, result);

        instance.setHoldArrayJobIds(new ArrayList<String>(0));
        result = instance.getHoldArrayJobIds();
        assertNull("The setHoldArrayJobIds() method did not return the value set with setHoldArrayJobIds()", instance.getHoldArrayJobIds());

        instance.setHoldArrayJobIds(null);
        result = instance.getHoldArrayJobIds();
        assertNull("The getHoldArrayJobIds() method did not return the value set with setHoldArrayJobIds()", result);

        instance.setHoldArrayJobIds(expResult);

        try {
            instance.setHoldArrayJobIds(Arrays.asList(new String[] { null }));
            fail("The setHoldArrayJobIds() method allowed a job list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHoldArrayJobIds() method set the job list even though it threw an exception: ", expResult, instance.getHoldArrayJobIds());
        }

        try {
            instance.setHoldArrayJobIds(Arrays.asList(new String[] { "456", null }));
            fail("The setHoldArrayJobIds() method allowed a job list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHoldArrayJobIds() method set the job list even though it threw an exception: ", expResult, instance.getHoldArrayJobIds());
        }

        try {
            instance.setHoldArrayJobIds(Arrays.asList(new String[] { null, "456" }));
            fail("The setHoldArrayJobIds() method allowed a job list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHoldArrayJobIds() method set the job list even though it threw an exception: ", expResult, instance.getHoldArrayJobIds());
        }

        try {
            instance.setHoldArrayJobIds(Arrays.asList(new String[] { "456", null, "789" }));
            fail("The setHoldArrayJobIds() method allowed a job list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHoldArrayJobIds() method set the job list even though it threw an exception: ", expResult, instance.getHoldArrayJobIds());
        }

        instance.setHoldArrayJobIds(expResult);
        instance.setBaseline();

        instance.setHoldArrayJobIds(null);
        result = instance.getHoldArrayJobIds();
        assertNull("The getHoldArrayJobIds() method did not return the value set with setHoldArrayJobIds()", result);
    }

    /**
     * Test of getHoldJobIds method, of class JsvParameters.
     */
    
    public void testHoldJobIds() {
        System.out.println("holdJobIds");

        JobDescription instance = new JobDescription();
        List<String> expResult = new ArrayList<String>(1);

        // Test that default value doesn't cause an exception
        instance.getHoldJobIds();

        expResult.add("123");

        instance.setHoldJobIds(expResult);

        List<String> result = instance.getHoldJobIds();

        assertEquals("The getHoldJobIds() method did not return the value set with setHoldJobIds()", "123", result.get(0));

        result.add("456");
        result = instance.getHoldJobIds();
        assertEquals("The getHoldJobIds() method did not return a copy of the Collection: ", expResult, result);

        instance.setHoldJobIds(new ArrayList<String>(0));
        result = instance.getHoldJobIds();
        assertNull("The getHoldJobIds() method did not return the value set with setHoldJobIds()", result);

        instance.setHoldJobIds(null);
        result = instance.getHoldJobIds();
        assertNull("The getHoldJobIds() method did not return the value set with setHoldJobIds()", result);

        instance.setHoldJobIds(expResult);

        try {
            instance.setHoldJobIds(Arrays.asList(new String[] { null }));
            fail("The setHoldJobIds() method allowed a job list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHoldJobIds() method set the job list even though it threw an exception: ", expResult, instance.getHoldJobIds());
        }

        try {
            instance.setHoldJobIds(Arrays.asList(new String[] { "456", null }));
            fail("The setHoldJobIds() method allowed a job list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHoldJobIds() method set the job list even though it threw an exception: ", expResult, instance.getHoldJobIds());
        }

        try {
            instance.setHoldJobIds(Arrays.asList(new String[] { null, "456" }));
            fail("The setHoldJobIds() method allowed a job list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHoldJobIds() method set the job list even though it threw an exception: ", expResult, instance.getHoldJobIds());
        }

        try {
            instance.setHoldJobIds(Arrays.asList(new String[] { "456", null, "789" }));
            fail("The setHoldJobIds() method allowed a job list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setHoldJobIds() method set the job list even though it threw an exception: ", expResult, instance.getHoldJobIds());
        }

        instance.setHoldJobIds(expResult);
        instance.setBaseline();

        instance.setHoldJobIds(null);
        result = instance.getHoldJobIds();
        assertNull("The getHoldJobIds() method did not return the value set with setHoldJobIds()", result);
    }

    /**
     * Test of getInputPath method, of class JsvParameters.
     */
    
    public void testInputPath() {
        System.out.println("inputPath()");

        JobDescription instance = new JobDescription();
        Map<String, String> expResult = new HashMap<String, String>();

        // Test that default value doesn't cause an exception
        instance.getInputPath();

        expResult.put(null, "test3");
        expResult.put("test1", "test2");

        instance.setInputPath(expResult);

        Map<String, String> result = instance.getInputPath();

        assertEquals("The getInputPath() method did not return the value set with setInputPath(): ", expResult, result);

        result.put("test1", "test3");
        result = instance.getInputPath();
        assertEquals("The getInputPath() method did not return a copy of the Collection: ", expResult, result);

        instance.setInputPath(new HashMap<String, String>());
        result = instance.getInputPath();
        assertNull("The getInputPath() method did not return the value set with setInputPath(): ", result);

        instance.setInputPath(null);
        result = instance.getInputPath();
        assertNull("The getInputPath() method did not return the value set with setInputPath(): ", result);

        instance.setInputPath(expResult);

        Map<String, String> bad = new HashMap<String, String>();

        bad.put("test", null);

        try {
            instance.setInputPath(bad);
            fail("The setInputPath() method allowed a path list with a null entry value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setInputPath() method set the path list even though it threw an exception: ", expResult, instance.getInputPath());
        }

        bad.clear();
        bad.put(null, null);

        try {
            instance.setInputPath(bad);
            fail("The setInputPath() method allowed a path list with a null entry value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setInputPath() method set the path list even though it threw an exception: ", expResult, instance.getInputPath());
        }

        instance.setInputPath(expResult);
        instance.setBaseline();

        instance.setInputPath(null);
        result = instance.getInputPath();
        assertNull("The getInputPath() method did not return the value set with setInputPath(): ", result);
    }

    /**
     * Test of getJobContext method, of class JsvParameters.
     */
    
    public void testJobContext() {
        System.out.println("jobContext");

        JobDescription instance = new JobDescription();
        Map<String, String> expResult = new HashMap<String, String>();

        // Test that default value doesn't cause an exception
        instance.getJobContext();

        expResult.put("test1", "test2");

        instance.setJobContext(expResult);

        Map<String, String> result = instance.getJobContext();

        assertEquals("The getJobContext() method did not return the value set with setJobContext()", expResult, result);

        result.put("test1", "test3");
        result = instance.getJobContext();
        assertEquals("The getJobContext() method did not return a copy of the Collection: ", expResult, result);

        instance.setJobContext(new HashMap<String, String>());
        result = instance.getJobContext();
        assertNull("The getJobContext() method did not return the value set with setJobContext()", result);

        instance.setJobContext(null);
        result = instance.getJobContext();
        assertNull("The getJobContext() method did not return the value set with setJobContext()", result);

        instance.setJobContext(expResult);

        Map<String, String> bad = new HashMap<String, String>();

        bad.put("test", null);

        try {
            instance.setJobContext(bad);
            fail("The setJobContext() method allowed job context with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setJobContext() method set the job context even though it threw an exception: ", expResult, instance.getJobContext());
        }

        bad.clear();
        bad.put(null, "test");

        try {
            instance.setJobContext(bad);
            fail("The setJobContext() method allowed job context with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setJobContext() method set the job context even though it threw an exception: ", expResult, instance.getJobContext());
        }

        bad.clear();
        bad.put(null, null);

        try {
            instance.setJobContext(bad);
            fail("The setJobContext() method allowed job context with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setJobContext() method set the job context even though it threw an exception: ", expResult, instance.getJobContext());
        }

        instance.setJobContext(expResult);
        instance.setBaseline();

        instance.setJobContext(null);
        result = instance.getJobContext();
        assertNull("The getJobContext() method did not return the value set with setJobContext()", result);
    }

    /**
     * Test of getJobId method, of class JsvParameters.
     */
    
    public void testJobId() {
        System.out.println("jobId");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getJobId();

        instance.setJobId(expResult);

        String result = instance.getJobId();

        assertEquals("The getJobId() method did not return the value set with setJobId()", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        instance.setJobId(null);
        result = instance.getJobId();

        assertEquals("The setJobId() method did not ignore the null value: ", expResult, result);
        assertEquals("The setJobId() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The setJobId() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        instance.setJobId(expResult);
        instance.setBaseline();

        try {
            instance.setJobId("test2");
            fail("The setJobId() method allowed the job id to modified after the baseline was set");
        } catch (IllegalStateException e) {
            assertEquals("The setJobId() method set the job id even though it threw an exception: ", expResult, instance.getJobId());
        }
    }

    /**
     * Test of getJobShare method, of class JsvParameters.
     */
    
    public void testJobShare() {
        System.out.println("jobShare");

        JobDescription instance = new JobDescription();
        Integer expResult = 7;

        // Test that default value doesn't cause an exception
        instance.getJobShare();

        instance.setJobShare(expResult);

        Integer result = instance.getJobShare();

        assertEquals("The getJobShare() method did not return the value set with setJobShare()", expResult, result);

        try {
            instance.setJobShare(-1);
            fail("The setJobShare() method accepted a negative id value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setJobShare() method set the share value even though it threw an exception: ", expResult, instance.getJobShare());
        }

        instance.setJobShare(null);
        result = instance.getJobShare();
        assertNull("The getJobShare() method did not return the value set with setJobShare()", result);

        instance.setJobShare(expResult);
        instance.setBaseline();

        instance.setJobShare(null);
        result = instance.getJobShare();
        assertNull("The getJobShare() method did not return the value set with setJobShare()", result);
    }

    /**
     * Test of getMailRecipients method, of class JsvParameters.
     */
    
    public void testMailRecipients() {
        System.out.println("mailRecipients");

        JobDescription instance = new JobDescription();
        List<String> expResult = new ArrayList<String>(1);

        expResult.add("test1");

        // Test that default value doesn't cause an exception
        instance.getMailRecipients();

        instance.setMailRecipients(expResult);

        List<String> result = instance.getMailRecipients();

        assertEquals("The getMailRecipients() method did not return the value set with setMailRecipients()", "test1", result.get(0));

        result.add("test3");
        result = instance.getMailRecipients();
        assertEquals("The getMailRecipients() method did not return a copy of the Collection: ", expResult, result);

        instance.setMailRecipients(null);
        result = instance.getMailRecipients();
        assertNull("The getMailRecipients() method did not return the value set with setMailRecipients()", result);

        instance.setMailRecipients(new ArrayList<String>(0));
        result = instance.getMailRecipients();
        assertNull("The getMailRecipients() method did not return the value set with setMailRecipients()", result);

        instance.setMailRecipients(expResult);

        try {
            instance.setMailRecipients(Arrays.asList(new String[] { null }));
            fail("The setMailRecipients() method allowed a mail list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMailRecipients() method set the mail list even though it threw an exception: ", expResult, instance.getMailRecipients());
        }

        try {
            instance.setMailRecipients(Arrays.asList(new String[] { "x", null }));
            fail("The setMailRecipients() method allowed a mail list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMailRecipients() method set the mail list even though it threw an exception: ", expResult, instance.getMailRecipients());
        }

        try {
            instance.setMailRecipients(Arrays.asList(new String[] { null, "x" }));
            fail("The setMailRecipients() method allowed a mail list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMailRecipients() method set the mail list even though it threw an exception: ", expResult, instance.getMailRecipients());
        }

        try {
            instance.setMailRecipients(Arrays.asList(new String[] { "x", null, "x" }));
            fail("The setMailRecipients() method allowed a mail list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMailRecipients() method set the mail list even though it threw an exception: ", expResult, instance.getMailRecipients());
        }

        instance.setMailRecipients(expResult);
        instance.setBaseline();

        instance.setMailRecipients(null);
        result = instance.getMailRecipients();
        assertNull("The getMailRecipients() method did not return the value set with setMailRecipients()", result);
    }

    /**
     * Test of getMailSpecifier method, of class JsvParameters.
     */
    
    public void testMailSpecifier() {
        System.out.println("mailSpecifier");

        JobDescription instance = new JobDescription();

        MailSpecifier expResult = new MailSpecifier();

        expResult.onAbort(true);
        expResult.onSuspend(true);
        instance.setMailSpecifier(expResult);

        MailSpecifier result = instance.getMailSpecifier();

        assertEquals("The getMailSpecifier() method did not return the value set with setMailSpecifier(): ", expResult, result);

        expResult.onEnd(true);
        instance.setMailSpecifier(expResult);
        result = instance.getMailSpecifier();
        assertEquals("The getMailSpecifier() method did not return the value set with setMailSpecifier(): ", expResult, result);

        instance.setMailSpecifier(null);
        result = instance.getMailSpecifier();
        assertEquals("The getMailSpecifier() method did not return the value set with setMailSpecifier(): ", null, result);

        instance.setMailSpecifier(expResult);
        instance.setBaseline();

        instance.setMailSpecifier(null);
        result = instance.getMailSpecifier();
        assertEquals("The getMailSpecifier() method did not return the value set with setMailSpecifier(): ", null, result);
    }

    /**
     * Test of getMasterQueue method, of class JsvParameters.
     */
    
    public void testMasterQueue() {
        System.out.println("masterQueue");

        JobDescription instance = new JobDescription();
        List<String> expResult = new ArrayList<String>(1);

        // Test that default value doesn't cause an exception
        instance.getMasterQueue();

        expResult.add("test1");

        instance.setMasterQueue(expResult);

        List<String> result = instance.getMasterQueue();

        assertEquals("The getMasterQueue() method did not return the value set with setMasterQueue()", "test1", result.get(0));

        result.add("test3");
        result = instance.getMasterQueue();
        assertEquals("The getMasterQueue() method did not return a copy of the Collection: ", expResult, result);

        instance.setMasterQueue(null);
        result = instance.getMasterQueue();
        assertNull("The getMasterQueue() method did not return the value set with setMasterQueue()", result);

        instance.setMasterQueue(new ArrayList<String>(0));
        result = instance.getMasterQueue();
        assertNull("The getMasterQueue() method did not return the value set with setMasterQueue()", result);

        instance.setMasterQueue(expResult);

        try {
            instance.setMasterQueue(Arrays.asList(new String[] { null }));
            fail("The setMasterQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMasterQueue() method set the queue list even though it threw an exception: ", expResult, instance.getMasterQueue());
        }

        try {
            instance.setMasterQueue(Arrays.asList(new String[] { "x", null }));
            fail("The setMasterQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMasterQueue() method set the queue list even though it threw an exception: ", expResult, instance.getMasterQueue());
        }

        try {
            instance.setMasterQueue(Arrays.asList(new String[] { null, "x" }));
            fail("The setMasterQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMasterQueue() method set the queue list even though it threw an exception: ", expResult, instance.getMasterQueue());
        }

        try {
            instance.setMasterQueue(Arrays.asList(new String[] { "x", null, "x" }));
            fail("The setMasterQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setMasterQueue() method set the queue list even though it threw an exception: ", expResult, instance.getMasterQueue());
        }

        instance.setMasterQueue(expResult);
        instance.setBaseline();

        instance.setMasterQueue(null);
        result = instance.getMasterQueue();
        assertNull("The getMasterQueue() method did not return the value set with setMasterQueue()", result);
    }

    /**
     * Test of getName method, of class JsvParameters.
     */
    
    public void testName() {
        System.out.println("name");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getName();

        instance.setName(expResult);

        String result = instance.getName();

        assertEquals("The getJobId() method did not return the value set with setJobId()", expResult, result);

        instance.setName(null);
        result = instance.getName();

        assertNull("The getJobId() method did not return the value set with setJobId()", result);

        instance.setName(expResult);
        instance.setBaseline();

        instance.setName(null);
        result = instance.getName();

        assertNull("The getJobId() method did not return the value set with setJobId()", result);
    }

    /**
     * Test of getOutputPath method, of class JsvParameters.
     */
    
    public void testOutputPath() {
        System.out.println("outputPath()");

        JobDescription instance = new JobDescription();
        Map<String, String> expResult = new HashMap<String, String>();

        // Test that default value doesn't cause an exception
        instance.getOutputPath();

        expResult.put(null, "test3");
        expResult.put("test1", "test2");

        instance.setOutputPath(expResult);

        Map<String, String> result = instance.getOutputPath();

        assertEquals("The getOutputPath() method did not return the value set with setOutputPath(): ", expResult, result);

        result.put("test1", "test3");
        result = instance.getOutputPath();
        assertEquals("The getOutputPath() method did not return a copy of the Collection: ", expResult, result);

        instance.setOutputPath(new HashMap<String, String>());
        result = instance.getOutputPath();
        assertNull("The getOutputPath() method did not return the value set with setOutputPath(): ", result);

        instance.setOutputPath(null);
        result = instance.getOutputPath();
        assertNull("The getOutputPath() method did not return the value set with setOutputPath(): ", result);

        instance.setOutputPath(expResult);

        Map<String, String> bad = new HashMap<String, String>();

        bad.put("test", null);

        try {
            instance.setOutputPath(bad);
            fail("The setOutputPath() method allowed a path list with a null entry value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setOutputPath() method set the path list even though it threw an exception: ", expResult, instance.getOutputPath());
        }

        bad.clear();
        bad.put(null, null);

        try {
            instance.setOutputPath(bad);
            fail("The setOutputPath() method allowed a path list with a null entry value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setOutputPath() method set the path list even though it threw an exception: ", expResult, instance.getOutputPath());
        }

        instance.setOutputPath(expResult);
        instance.setBaseline();

        instance.setOutputPath(null);
        result = instance.getOutputPath();
        assertNull("The getOutputPath() method did not return the value set with setOutputPath(): ", result);
    }

    /**
     * Test of getParallelEnvironment method, of class JsvParameters.
     */    
    public void testParallelEnvironment() {
        System.out.println("paralleEnvironment");

        JobDescription instance = new JobDescription();

        ParallelEnvironment expResult = new ParallelEnvironment();

        expResult.setRange(1, 8);

        try {
            instance.setParallelEnvironment(expResult);
            fail("The setParalleEnvironment() method accepted an invalid ParallelEnvironment object");
        } catch (IllegalArgumentException e) {
            assertEquals("The setParalleEnvironment() method set the parallel environment even though it threw an exception: ", null, instance.getCheckpointSpecifier());
        }

        expResult.setName("pe");
        instance.setParallelEnvironment(expResult);

        ParallelEnvironment result = instance.getParallelEnvironment();

        assertEquals("The getParalleEnvironment() method did not return the value set with setParalleEnvironment(): ", expResult, result);

        expResult.setRangeMin(4);
        instance.setParallelEnvironment(expResult);
        result = instance.getParallelEnvironment();
        assertEquals("The getParalleEnvironment() method did not return the value set with setParalleEnvironment(): ", expResult, result);

        instance.setParallelEnvironment(null);
        result = instance.getParallelEnvironment();
        assertEquals("The getParalleEnvironment() method did not return the value set with setParalleEnvironment(): ", null, result);

        instance.setParallelEnvironment(expResult);
        instance.setBaseline();

        instance.setParallelEnvironment(null);
        result = instance.getParallelEnvironment();
        assertEquals("The getParalleEnvironment() method did not return the value set with setParalleEnvironment(): ", null, result);
    }

    /**
     * Test of getBindingSpecifier method, of class JsvParameters.
     * TODO: add negative tests
     */
    public void testBindingSpecifier() {
        System.out.println("bindingSpecifier");

        JobDescription instance = new JobDescription();
        BindingSpecifier expResult = null;
        BindingSpecifier result = null;

        instance.setBindingSpecifier(expResult);
        result = instance.getBindingSpecifier();
        assertEquals("The getBindingSpecifier() method did not return the value set with setBindingSpecifier(): ", expResult, result);

        expResult = new BindingSpecifier();
        expResult.setLinearStrategy(3, 4, 5);
        instance.setBindingSpecifier(expResult);
        result = instance.getBindingSpecifier();
        assertEquals("The getBindingSpecifier() method did not return the value set with setBindingSpecifier(): ", expResult, result);

        expResult = new BindingSpecifier();
        expResult.setLinearStrategy(10);
        instance.setBindingSpecifier(expResult);
        result = instance.getBindingSpecifier();
        assertEquals("The getBindingSpecifier() method did not return the value set with setBindingSpecifier(): ", expResult, result);

        expResult = new BindingSpecifier();
        expResult.setStridingStrategy(7, 8, 9);
        instance.setBindingSpecifier(expResult);
        result = instance.getBindingSpecifier();
        assertEquals("The getBindingSpecifier() method did not return the value set with setBindingSpecifier(): ", expResult, result);

        expResult = new BindingSpecifier();
        expResult.setStridingStrategy(11);
        instance.setBindingSpecifier(expResult);
        result = instance.getBindingSpecifier();
        assertEquals("The getBindingSpecifier() method did not return the value set with setBindingSpecifier(): ", expResult, result);

        expResult = new BindingSpecifier();
        List<BindingSpecifier.CoreSpecifier> list = new LinkedList<BindingSpecifier.CoreSpecifier>();
        list.add(expResult.new CoreSpecifier(0, 0));
        list.add(expResult.new CoreSpecifier(1, 1));
        list.add(expResult.new CoreSpecifier(2, 2));
        expResult.setExplicitStrategy(list);
        instance.setBindingSpecifier(expResult);
        result = instance.getBindingSpecifier();
        assertEquals("The getBindingSpecifier() method did not return the value set with setBindingSpecifier(): ", expResult, result);
    }

    /**
     * Test of getPriority method, of class JsvParameters.
     */
    
    public void testPriority() {
        System.out.println("priority");

        JobDescription instance = new JobDescription();
        Integer expResult = 7;

        // Test that default value doesn't cause an exception
        instance.getPriority();

        instance.setPriority(expResult);

        Integer result = instance.getPriority();

        assertEquals("The getPriority() method did not return the value set with setPriority()", expResult, result);

        try {
            instance.setPriority(-1);
            fail("The setPriority() method accepted a negative id value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setPriority() method set the priority value even though it threw an exception: ", expResult, instance.getPriority());
        }

        instance.setPriority(null);
        result = instance.getPriority();
        assertNull("The getPriority() method did not return the value set with setPriority()", result);

        instance.setPriority(expResult);
        instance.setBaseline();

        instance.setPriority(null);
        result = instance.getPriority();
        assertNull("The getPriority() method did not return the value set with setPriority()", result);
    }

    /**
     * Test of getProject method, of class JsvParameters.
     */
    
    public void testProject() {
        System.out.println("project");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getProject();

        instance.setProject(expResult);

        String result = instance.getProject();

        assertEquals("The getProject() method did not return the value set with setProject()", expResult, result);

        instance.setProject(null);
        result = instance.getProject();
        assertNull("The getProject() method did not return the value set with setProject()", result);

        instance.setProject(expResult);
        instance.setBaseline();

        instance.setProject(null);
        result = instance.getProject();
        assertNull("The getProject() method did not return the value set with setProject()", result);
    }

    /**
     * Test of getShellPath method, of class JsvParameters.
     */
    
    public void testShellPath() {
        System.out.println("shellPath()");

        JobDescription instance = new JobDescription();
        Map<String, String> expResult = new HashMap<String, String>();

        // Test that default value doesn't cause an exception
        instance.getShellPath();

        expResult.put(null, "test3");
        expResult.put("test1", "test2");

        instance.setShellPath(expResult);

        Map<String, String> result = instance.getShellPath();

        assertEquals("The getShellPath() method did not return the value set with setShellPath(): ", expResult, result);

        result.put("test1", "test3");
        result = instance.getShellPath();
        assertEquals("The getShellPath() method did not return a copy of the Collection: ", expResult, result);

        instance.setShellPath(new HashMap<String, String>());
        result = instance.getShellPath();
        assertNull("The getShellPath() method did not return the value set with setShellPath(): ", result);

        instance.setShellPath(null);
        result = instance.getShellPath();
        assertNull("The getShellPath() method did not return the value set with setShellPath(): ", result);

        instance.setShellPath(expResult);

        Map<String, String> bad = new HashMap<String, String>();

        bad.put("test", null);

        try {
            instance.setShellPath(bad);
            fail("The setShellPath() method allowed a path list with a null entry value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setShellPath() method set the path list even though it threw an exception: ", expResult, instance.getShellPath());
        }

        bad.clear();
        bad.put(null, null);

        try {
            instance.setShellPath(bad);
            fail("The setShellPath() method allowed a path list with a null entry value");
        } catch (IllegalArgumentException e) {
            assertEquals("The setShellPath() method set the path list even though it threw an exception: ", expResult, instance.getShellPath());
        }

        instance.setShellPath(expResult);
        instance.setBaseline();

        instance.setShellPath(null);
        result = instance.getShellPath();
        assertNull("The getShellPath() method did not return the value set with setShellPath(): ", result);
    }

    /**
     * Test of getSoftQueue method, of class JsvParameters.
     */
    
    public void testSoftQueue() {
        System.out.println("softQueue");

        JobDescription instance = new JobDescription();
        List<String> expResult = new ArrayList<String>(1);

        // Test that default value doesn't cause an exception
        instance.getSoftQueue();

        expResult.add("test1");

        instance.setSoftQueue(expResult);

        List<String> result = instance.getSoftQueue();

        assertEquals("The getSoftQueue() method did not return the value set with setSoftQueue()", "test1", result.get(0));

        result.add("test3");
        result = instance.getSoftQueue();
        assertEquals("The getSoftQueue() method did not return a copy of the Collection: ", expResult, result);

        instance.setSoftQueue(new ArrayList<String>(0));
        result = instance.getSoftQueue();
        assertNull("The getSoftQueue() method did not return the value set with setSoftQueue()", result);

        instance.setSoftQueue(null);
        result = instance.getSoftQueue();
        assertNull("The getSoftQueue() method did not return the value set with setSoftQueue()", result);

        instance.setSoftQueue(expResult);

        try {
            instance.setSoftQueue(Arrays.asList(new String[] { null }));
            fail("The setSoftQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setSoftQueue() method set the queue list even though it threw an exception: ", expResult, instance.getSoftQueue());
        }

        try {
            instance.setSoftQueue(Arrays.asList(new String[] { "x", null }));
            fail("The setSoftQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setSoftQueue() method set the queue list even though it threw an exception: ", expResult, instance.getSoftQueue());
        }

        try {
            instance.setSoftQueue(Arrays.asList(new String[] { null, "x" }));
            fail("The setSoftQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setSoftQueue() method set the queue list even though it threw an exception: ", expResult, instance.getSoftQueue());
        }

        try {
            instance.setSoftQueue(Arrays.asList(new String[] { "x", null, "x" }));
            fail("The setSoftQueue() method allowed a queue list with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setSoftQueue() method set the queue list even though it threw an exception: ", expResult, instance.getSoftQueue());
        }

        instance.setSoftQueue(expResult);
        instance.setBaseline();

        instance.setSoftQueue(null);
        result = instance.getSoftQueue();
        assertNull("The getSoftQueue() method did not return the value set with setSoftQueue()", result);
    }

    /**
     * Test of getSoftResourceRequirements method, of class JsvParameters.
     */
    
    public void testSoftResourceRequirements() {
        System.out.println("softResourceRequirements");

        JobDescription instance = new JobDescription();
        Map<String, String> expResult = new HashMap<String, String>();

        // Test that default value doesn't cause an exception
        instance.getSoftResourceRequirements();

        expResult.put("test1", "test2");

        instance.setSoftResourceRequirements(expResult);

        Map<String, String> result = instance.getSoftResourceRequirements();

        assertEquals("The getSoftResourceRequirements() method did not return the value set with setSoftResourceRequirements()", expResult, result);

        result.put("test1", "test3");
        result = instance.getSoftResourceRequirements();
        assertEquals("The getSoftResourceRequirements() method did not return a copy of the Collection: ", expResult, result);

        instance.setSoftResourceRequirements(new HashMap<String, String>());
        result = instance.getSoftResourceRequirements();
        assertNull("The getSoftResourceRequirements() method did not return the value set with setSoftResourceRequirements()", result);

        instance.setSoftResourceRequirements(null);
        result = instance.getSoftResourceRequirements();
        assertNull("The getSoftResourceRequirements() method did not return the value set with setSoftResourceRequirements()", result);

        instance.setSoftResourceRequirements(expResult);

        Map<String, String> bad = new HashMap<String, String>();

        bad.put("test", null);

        try {
            instance.setSoftResourceRequirements(bad);
            fail("The setSoftResourceRequirements() method allowed resource requirements with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setSoftResourceRequirements() method set the resource requirements even though it threw an exception: ", expResult, instance.getSoftResourceRequirements());
        }

        bad.clear();
        bad.put(null, "test");

        try {
            instance.setSoftResourceRequirements(bad);
            fail("The setSoftResourceRequirements() method allowed resource requirements with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setSoftResourceRequirements() method set the resource requirements even though it threw an exception: ", expResult, instance.getSoftResourceRequirements());
        }

        bad.clear();
        bad.put(null, null);

        try {
            instance.setSoftResourceRequirements(bad);
            fail("The setSoftResourceRequirements() method allowed resource requirements with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setSoftResourceRequirements() method set the resource requirements even though it threw an exception: ", expResult, instance.getSoftResourceRequirements());
        }

        instance.setSoftResourceRequirements(expResult);
        instance.setBaseline();

        instance.setSoftResourceRequirements(null);
        result = instance.getSoftResourceRequirements();
        assertNull("The getSoftResourceRequirements() method did not return the value set with setSoftResourceRequirements()", result);
    }

    /**
     * Test of getStartTime method, of class JsvParameters.
     */
    
    public void testStartTime() {
        System.out.println("startTime");

        JobDescription instance = new JobDescription();
        Calendar expResult = Calendar.getInstance();

        // Test that default value doesn't cause an exception
        instance.getStartTime();

        expResult.set(Calendar.YEAR, 2020);
        instance.setStartTime(expResult);

        Calendar result = instance.getStartTime();

        assertEquals("The getStartTime() method did not return the value set with setStartTime()", expResult, result);

        result.set(Calendar.YEAR, 2010);
        result = instance.getStartTime();

        assertEquals("The getStartTime() method did not return a copy of the start time object: ", expResult, result);

        instance.setStartTime(null);
        result = instance.getStartTime();
        assertNull("The getStartTime() method did not return the value set with setStartTime()", result);

        instance.setStartTime(expResult);
        instance.setBaseline();

        instance.setStartTime(null);
        result = instance.getStartTime();
        assertNull("The getStartTime() method did not return the value set with setStartTime()", result);
    }

    /**
     * Test of getTaskSpecifier method, of class JsvParameters.
     */
    
    public void testTaskSpecifier() {
        System.out.println("taskSpecifier");

        JobDescription instance = new JobDescription();

        TaskSpecifier expResult = new TaskSpecifier();

        expResult.setRange(1, 8);
        instance.setTaskSpecifier(expResult);

        TaskSpecifier result = instance.getTaskSpecifier();

        assertEquals("The getTaskSpecifier() method did not return the value set with setTaskSpecifier(): ", expResult, result);

        expResult.setRange(4, 8, 2);
        instance.setTaskSpecifier(expResult);
        result = instance.getTaskSpecifier();
        assertEquals("The getTaskSpecifier() method did not return the value set with setTaskSpecifier(): ", expResult, result);

        instance.setTaskSpecifier(null);
        result = instance.getTaskSpecifier();
        assertEquals("The getTaskSpecifier() method did not return the value set with setTaskSpecifier(): ", null, result);

        instance.setTaskSpecifier(expResult);
        instance.setBaseline();

        instance.setTaskSpecifier(null);
        result = instance.getTaskSpecifier();
        assertEquals("The getTaskSpecifier() method did not return the value set with setTaskSpecifier(): ", null, result);
    }

    /**
     * Test of getUser method, of class JsvParameters.
     */
    
    public void testUser() {
        System.out.println("user");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getUser();

        instance.setUser(expResult);

        String result = instance.getUser();

        assertEquals("The getUser() method did not return the value set with setUser()", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        instance.setUser(null);
        result = instance.getUser();

        assertEquals("The setUser() method did not ignore the null value: ", expResult, result);
        assertEquals("The setUser() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The setUser() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        instance.setUser(expResult);
        instance.setBaseline();

        try {
            instance.setUser("test2");
            fail("The setUser() method allowed the user to modified after the baseline was set");
        } catch (IllegalStateException e) {
            assertEquals("The setUser() method set the user even though it threw an exception: ", expResult, instance.getUser());
        }
    }

    /**
     * Test of getVerification method, of class JsvParameters.
     */
    
    public void testVerification() {
        System.out.println("verification");

        JobDescription instance = new JobDescription();
        Verification expResult = Verification.VERIFY;

        // Test that default value doesn't cause an exception
        instance.getVerification();

        instance.setVerification(expResult);

        Verification result = instance.getVerification();

        assertEquals("The getVerification() method did not return the value set by setVerification()", expResult, result);

        instance.setVerification(null);
        result = instance.getVerification();
        assertNull("The getVerification() method did not return the value set by setVerification()", result);

        instance.setVerification(expResult);
        instance.setBaseline();

        instance.setVerification(null);
        result = instance.getVerification();
        assertNull("The getVerification() method did not return the value set by setVerification()", result);
    }

    /**
     * Test of getVersion method, of class JsvParameters.
     */
    
    public void testVersion() {
        System.out.println("version");

        JobDescription instance = new JobDescription();
        String expResult = "test1";

        // Test that default value doesn't cause an exception
        instance.getVersion();

        instance.setVersion(expResult);

        String result = instance.getVersion();

        assertEquals("The getVersion() method did not return the value set with setVersion()", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        instance.setVersion(null);
        result = instance.getVersion();

        assertEquals("The setVersion() method did not ignore the null value: ", expResult, result);
        assertEquals("The setVersion() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The setVersion() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        instance.setVersion(expResult);
        instance.setBaseline();

        try {
            instance.setVersion("test2");
            fail("The setVersion() method allowed the version to modified after the baseline was set");
        } catch (IllegalStateException e) {
            assertEquals("The setVersion() method set the version even though it threw an exception: ", expResult, instance.getVersion());
        }
    }

    /**
     * Test of useShell method, of class JsvParameters.
     */
    
    public void testShell() {
        System.out.println("shell");

        JobDescription instance = new JobDescription();
        Boolean expResult = false;

        // Test that default value doesn't cause an exception
        instance.useShell();

        instance.setShell(expResult);

        Boolean result = instance.useShell();

        assertEquals("The useShell() method did not return the value set with setShell(): ", expResult, result);

        instance.setShell(null);
        result = instance.useShell();

        assertNull("The useShell() method did not return the value set with setShell(): ", result);

        instance.setShell(expResult);
        instance.setBaseline();

        instance.setShell(null);
        result = instance.useShell();

        assertNull("The useShell() method did not return the value set with setShell(): ", result);
    }

    /**
     * Test of resourceReservation property, of class JsvParameters.
     */
    
    public void testReservation() {
        System.out.println("resourceReservation");

        JobDescription instance = new JobDescription();
        Boolean expResult = true;

        instance.setResourceReservation(expResult);

        Boolean result = instance.hasResourceReservation();

        assertEquals("The hasReservation() method did not return the value set with setResourceReservation()", expResult, result);

        instance.setResourceReservation(null);
        result = instance.hasResourceReservation();

        assertNull("The hasReservation() method did not return the value set with setResourceReservation(): ", result);

        instance.setResourceReservation(expResult);
        instance.setBaseline();

        instance.setResourceReservation(null);
        result = instance.hasResourceReservation();

        assertNull("The hasReservation() method did not return the value set with setResourceReservation(): ", result);
    }

    /**
     * Test of isRerunnable method, of class JsvParameters.
     */
    
    public void testRerunnable() {
        System.out.println("rerunnable");

        JobDescription instance = new JobDescription();
        Boolean expResult = true;

        // Test that default value doesn't cause an exception
        instance.isRerunnable();

        instance.setRerunnable(expResult);

        Boolean result = instance.isRerunnable();

        assertEquals("The isRerunnable() method did not return the value set with setRerunnable()", expResult, result);

        instance.setRerunnable(null);
        result = instance.isRerunnable();

        assertNull("The isRerunnable() method did not return the value set with setRerunnable(): ", result);

        instance.setRerunnable(expResult);
        instance.setBaseline();

        instance.setRerunnable(null);
        result = instance.isRerunnable();

        assertNull("The isRerunnable() method did not return the value set with setRerunnable(): ", result);
    }

    /**
     * Test of doNotify method, of class JsvParameters.
     */
    
    public void testNotify() {
        System.out.println("notify");

        JobDescription instance = new JobDescription();
        Boolean expResult = true;

        // Test that default value doesn't cause an exception
        instance.doNotify();

        instance.setNotify(expResult);

        Boolean result = instance.doNotify();

        assertEquals("The doNotify() method did not return the value set with setNotify()", expResult, result);

        instance.setNotify(null);
        result = instance.doNotify();

        assertNull("The doNotify() method did not return the value set with setNotify(): ", result);

        instance.setNotify(expResult);
        instance.setBaseline();

        instance.setNotify(null);
        result = instance.doNotify();

        assertNull("The doNotify() method did not return the value set with setNotify(): ", result);
    }

    /**
     * Test of mergeStreams method, of class JsvParameters.
     */
    
    public void testMergeStreams() {
        System.out.println("mergeStreams");

        JobDescription instance = new JobDescription();
        Boolean expResult = true;

        // Test that default value doesn't cause an exception
        instance.mergeStreams();

        instance.setMergeStreams(expResult);

        Boolean result = instance.mergeStreams();

        assertEquals("The mergeStreams() method did not return the value set with setMergeStreams()", expResult, result);

        instance.setMergeStreams(null);
        result = instance.mergeStreams();

        assertNull("The mergeStreams() method did not return the value set with setMergeStreams(): ", result);

        instance.setMergeStreams(expResult);
        instance.setBaseline();

        instance.setMergeStreams(null);
        result = instance.mergeStreams();

        assertNull("The mergeStreams() method did not return the value set with setMergeStreams(): ", result);
    }

    /**
     * Test of onHold method, of class JsvParameters.
     */
    
    public void testHold() {
        System.out.println("hold");

        JobDescription instance = new JobDescription();
        Boolean expResult = true;

        // Test that default value doesn't cause an exception
        instance.onHold();

        instance.setHold(expResult);

        Boolean result = instance.onHold();

        assertEquals("The onHold() method did not return the value set with setHold()", expResult, result);

        instance.setHold(null);
        result = instance.onHold();

        assertNull("The onHold() method did not return the value set with setHold(): ", result);

        instance.setHold(expResult);
        instance.setBaseline();

        instance.setHold(null);
        result = instance.onHold();

        assertNull("The onHold() method did not return the value set with setHold(): ", result);
    }

    /**
     * Test of isBinary method, of class JsvParameters.
     */
    
    public void testBinary() {
        System.out.println("binary");

        JobDescription instance = new JobDescription();
        Boolean expResult = true;

        // Test that default value doesn't cause an exception
        instance.isBinary();

        instance.setBinary(expResult);

        Boolean result = instance.isBinary();

        assertEquals("The isBinary() method did not return the value set with setBinary()", expResult, result);

        instance.setBinary(null);
        result = instance.isBinary();

        assertNull("The isBinary() method did not return the value set with setBinary(): ", result);

        instance.setBinary(expResult);
        instance.setBaseline();

        instance.setBinary(null);
        result = instance.isBinary();

        assertNull("The isBinary() method did not return the value set with setBinary(): ", result);
    }

    /**
     * Test of getJobContext method, of class JsvParameters.
     */
    
    public void testEnvironment() {
        System.out.println("environment");

        JobDescription instance = new JobDescription();
        Map<String, String> expResult = new HashMap<String, String>();

        // Test that default value doesn't cause an exception
        instance.getEnvironment();

        expResult.put("test1", "test2");

        instance.setEnvironment(expResult);

        Map<String, String> result = instance.getEnvironment();

        assertEquals("The getEnvironment() method did not return the value set with setEnvironment()", expResult, result);

        result.put("test1", "test3");
        result = instance.getEnvironment();
        assertEquals("The getEnvironment() method did not return a copy of the Collection: ", expResult, result);

        instance.setEnvironment(new HashMap<String, String>());
        result = instance.getEnvironment();

        assertEquals("The getEnvironment() method did not return the value set with setEnvironment()", new HashMap<String, String>(), result);

        instance.setEnvironment(null);
        result = instance.getEnvironment();
        assertNull("The getEnvironment() method did not return the value set with setEnvironment()", result);

        instance.setEnvironment(expResult);

        Map<String, String> bad = new HashMap<String, String>();

        bad.put("test", null);

        try {
            instance.setEnvironment(bad);
            fail("The setEnvironment() method allowed environment with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setEnvironment() method set the environment even though it threw an exception: ", expResult, instance.getEnvironment());
        }

        bad.clear();
        bad.put(null, "test");

        try {
            instance.setEnvironment(bad);
            fail("The setEnvironment() method allowed environment with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setEnvironment() method set the environment even though it threw an exception: ", expResult, instance.getEnvironment());
        }

        bad.clear();
        bad.put(null, null);

        try {
            instance.setEnvironment(bad);
            fail("The setEnvironment() method allowed environment with a null entry");
        } catch (IllegalArgumentException e) {
            assertEquals("The setEnvironment() method set the environment even though it threw an exception: ", expResult, instance.getEnvironment());
        }

        instance.setEnvironment(expResult);
        instance.setBaseline();

        instance.setEnvironment(null);
        result = instance.getEnvironment();
        assertNull("The getEnvironment() method did not return the value set with setEnvironment()", result);
    }

    /**
     * Test of getJobContext method, of class JsvParameters.
     */
    
    public void testGetEnvironmentDifference() {
        System.out.println("getEnvironmentDifference()");

        JobDescription instance = null;
        Map<String, String> env = new HashMap<String, String>();
        Map<String, String> expAdd = new HashMap<String, String>();
        Map<String, String> expMod = new HashMap<String, String>();
        Map<String, String> expDel = new HashMap<String, String>();
        Map<String, String> resAdd = new HashMap<String, String>();
        Map<String, String> resMod = new HashMap<String, String>();
        Map<String, String> resDel = new HashMap<String, String>();
        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        handler.setLevel(Level.ALL);
        log.addHandler(handler);
        log.setLevel(Level.ALL);

        env.clear();
        instance = new JobDescription();
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);
        assertEquals("The getEnvironmentDifference() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The getEnvironmentDifference() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        env.clear();
        instance = new JobDescription();
        instance.setBaseline();
        env.put("test1", "test2");
        instance.setEnvironment(env);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expAdd.put("test1", "test2");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);

        env.clear();
        instance = new JobDescription();
        instance.setEnvironment(env);
        instance.setBaseline();
        env.put("test1", "test2");
        instance.setEnvironment(env);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expAdd.put("test1", "test2");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);

        env.clear();
        instance = new JobDescription();
        env.put("test1", "test2");
        instance.setEnvironment(env);
        instance.setBaseline();
        env.put("test1", "test3");
        instance.setEnvironment(env);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expMod.put("test1", "test3");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);

        env.clear();
        instance = new JobDescription();
        env.put("test1", "test2");
        instance.setEnvironment(env);
        instance.setBaseline();
        env.clear();
        instance.setEnvironment(env);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expDel.put("test1", "test2");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);

        env.clear();
        instance = new JobDescription();
        env.put("test1", "test2");
        instance.setEnvironment(env);
        instance.setBaseline();
        env.put("test1", "test3");
        env.put("test3", "test4");
        instance.setEnvironment(env);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expAdd.put("test3", "test4");
        expMod.put("test1", "test3");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);

        env.clear();
        instance = new JobDescription();
        env.put("test1", "test2");
        env.put("test3", "test4");
        instance.setEnvironment(env);
        instance.setBaseline();
        env.put("test1", "test3");
        env.remove("test3");
        instance.setEnvironment(env);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expMod.put("test1", "test3");
        expDel.put("test3", "test4");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);

        env.clear();
        instance = new JobDescription();
        env.put("test1", "test2");
        env.put("test3", "test4");
        instance.setEnvironment(env);
        instance.setBaseline();
        env.put("test5", "test6");
        env.remove("test3");
        instance.setEnvironment(env);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expAdd.put("test5", "test6");
        expDel.put("test3", "test4");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);

        env.clear();
        instance = new JobDescription();
        env.put("test1", "test2");
        env.put("test3", "test4");
        instance.setEnvironment(env);
        instance.setBaseline();
        env.put("test1", "test3");
        env.put("test5", "test6");
        env.remove("test3");
        instance.setEnvironment(env);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expAdd.put("test5", "test6");
        expMod.put("test1", "test3");
        expDel.put("test3", "test4");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);

        env.clear();
        instance = new JobDescription();
        env.put("test1", "test2");
        env.put("test3", "test4");
        instance.setEnvironment(env);
        instance.setBaseline();
        instance.setEnvironment(null);
        expAdd.clear();
        expMod.clear();
        expDel.clear();
        expDel.put("test1", "test2");
        expDel.put("test3", "test4");
        resAdd.clear();
        resMod.clear();
        resDel.clear();
        instance.getEnvironmentDifference(resAdd, resMod, resDel);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the added env vars: ", expAdd, resAdd);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the modified env vars: ", expMod, resMod);
        assertEquals("The getEnvironmentDifference() method did not correctly calculate the deleted env vars: ", expDel, resDel);
    }

    /**
     * Test of setBaseline method, of class JsvParameters.
     */
    
    public void testSetBaseline() throws Exception {
        System.out.println("setBaseline");

        JobDescription instance = new JobDescription();
        String expectedString = "test1";
        String resultString;
        Map<String, String> expectedPath = new HashMap<String, String>();
        Map<String, String> resultPath;
        int expectedInt = 7;
        int resultInt;
        boolean resultBoolean;
        Calendar expectedTime = Calendar.getInstance();
        Calendar resultTime;
        Map<String, String> expectedContext = new HashMap<String, String>();
        Map<String, String> resultContext;
        List<String> expectedList = new ArrayList<String>(2);
        List<? extends Object> resultList;
        List<Integer> expectedIds = new ArrayList<Integer>(2);
        Map<String, String> expectedMap = new HashMap<String, String>();
        Map<String, String> resultMap;
        Verification expectedVerification = Verification.VERIFY;
        Verification resultVerification;
        CheckpointSpecifier expectedCkpt = new CheckpointSpecifier();
        CheckpointSpecifier resultCkpt = null;
        MailSpecifier expectedMail = new MailSpecifier();
        MailSpecifier resultMail = null;
        ParallelEnvironment expectedPe = new ParallelEnvironment();
        ParallelEnvironment resultPe = null;
        TaskSpecifier expectedTask = new TaskSpecifier();
        TaskSpecifier resultTask = null;

        expectedPath.put(null, "/a/b");
        expectedPath.put("h", "c/d");

        expectedTime.set(Calendar.YEAR, 2009);
        expectedTime.set(Calendar.MONTH, Calendar.MAY);
        expectedTime.set(Calendar.DAY_OF_MONTH, 22);
        expectedTime.set(Calendar.HOUR_OF_DAY, 20);
        expectedTime.set(Calendar.MINUTE, 55);
        expectedTime.set(Calendar.SECOND, 10);
        expectedTime.set(Calendar.MILLISECOND, 0);

        expectedContext.put("test1", "test2");

        expectedList.add("123");
        expectedList.add("456");

        expectedIds.add(123);
        expectedIds.add(456);

        expectedMap.put("test1", "test2");
        expectedMap.put("test3", "test4");

        expectedCkpt.setName(expectedString);
        expectedCkpt.setOccasion("sx");

        expectedMail.setOccasion("abe");

        expectedPe.setName(expectedString);
        expectedPe.setRangeMin(expectedInt);
        expectedPe.setRangeMax(expectedInt);

        expectedTask.setRange(expectedInt, expectedInt, expectedInt);

        instance.setClient(expectedString);
        instance.setCommandName(expectedString);
        instance.setContext(expectedString);
        instance.setGroup(expectedString);
        instance.setJobId(expectedString);
        instance.setUser(expectedString);
        instance.setVersion(expectedString);
        instance.setCommandArgs(new String[] { expectedString });
        instance.setBinary(true);
        instance.setMergeStreams(true);
        instance.setNotify(true);
        instance.setShell(false);
        instance.setRerunnable(true);
        instance.setResourceReservation(true);
        instance.setAccount(expectedString);
        instance.setCheckpointSpecifier(expectedCkpt);
        instance.setWorkingDirectory(expectedString);
        instance.setName(expectedString);
        instance.setProject(expectedString);
        instance.setInputPath(expectedPath);
        instance.setOutputPath(expectedPath);
        instance.setErrorPath(expectedPath);
        instance.setShellPath(expectedPath);
        instance.setStartTime(expectedTime);
        instance.setJobContext(expectedContext);
        instance.setAdvanceReservationId(expectedInt);
        instance.setDisplay(expectedString);
        instance.setDeadline(expectedTime);
        instance.setHold(true);
        instance.setHoldJobIds(expectedList);
        instance.setHoldArrayJobIds(expectedList);
        instance.setJobShare(expectedInt);
        instance.setHardResourceRequirements(expectedMap);
        instance.setSoftResourceRequirements(expectedMap);
        instance.setMailSpecifier(expectedMail);
        instance.setMasterQueue(expectedList);
        instance.setHardQueue(expectedList);
        instance.setSoftQueue(expectedList);
        instance.setMailRecipients(expectedList);
        instance.setPriority(expectedInt);
        instance.setParallelEnvironment(expectedPe);
        instance.setTaskSpecifier(expectedTask);
        instance.setVerification(expectedVerification);

        instance.setBaseline();

        JobDescription baseline = getPrivateField(instance, "baseline");

        resultString = baseline.getClient();

        assertEquals("The setBaseline() method did not set the client property appropriately", expectedString, resultString);

        resultString = baseline.getCommandName();

        assertEquals("The setBaseline() method did not set the command name property appropriately", expectedString, resultString);

        resultString = baseline.getContext();

        assertEquals("The setBaseline() method did not set the context property appropriately", expectedString, resultString);

        resultString = baseline.getGroup();

        assertEquals("The setBaseline() method did not set the group property appropriately", expectedString, resultString);

        resultString = baseline.getJobId();

        assertEquals("The setBaseline() method did not set the job id property appropriately", expectedString, resultString);

        resultString = baseline.getUser();

        assertEquals("The setBaseline() method did not set the user property appropriately", expectedString, resultString);

        resultString = baseline.getVersion();

        assertEquals("The setBaseline() method did not set the version property appropriately", expectedString, resultString);

        resultString = baseline.getCommandArgs()[0];

        assertEquals("The setBaseline() method did not set the command arguments property appropriately", expectedString, resultString);

        resultBoolean = baseline.isBinary();

        assertEquals("The setBaseline() method did not set the binary property appropriately", true, resultBoolean);

        resultBoolean = baseline.mergeStreams();

        assertEquals("The setBaseline() method did not set the merge streams property appropriately", true, resultBoolean);

        resultBoolean = baseline.doNotify();

        assertEquals("The setBaseline() method did not set the notify property appropriately", true, resultBoolean);

        resultBoolean = baseline.useShell();

        assertEquals("The setBaseline() method did not set the shell property appropriately", false, resultBoolean);

        resultBoolean = baseline.isRerunnable();

        assertEquals("The setBaseline() method did not set the rerunnable property appropriately", true, resultBoolean);

        resultBoolean = baseline.hasResourceReservation();

        assertEquals("The setBaseline() method did not set the reservation property appropriately", true, resultBoolean);

        resultString = baseline.getAccount();

        assertEquals("The setBaseline() method did not set the account property appropriately", expectedString, resultString);

        resultCkpt = baseline.getCheckpointSpecifier();

        assertEquals("The setBaseline() method did not set the checkpint specifier name property appropriately", expectedCkpt, resultCkpt);

        resultString = baseline.getWorkingDirectory();

        assertEquals("The setBaseline() method did not set the cwd property appropriately", expectedString, resultString);

        resultString = baseline.getName();

        assertEquals("The setBaseline() method did not set the name property appropriately", expectedString, resultString);

        resultString = baseline.getProject();

        assertEquals("The setBaseline() method did not set the project property appropriately", expectedString, resultString);

        resultPath = baseline.getInputPath();

        assertEquals("The setBaseline() method did not set the input path property appropriately", expectedPath, resultPath);

        resultPath = baseline.getOutputPath();

        assertEquals("The setBaseline() method did not set the output path property appropriately", expectedPath, resultPath);

        resultPath = baseline.getErrorPath();

        assertEquals("The setBaseline() method did not set the error path property appropriately", expectedPath, resultPath);

        resultPath = baseline.getShellPath();

        assertEquals("The setBaseline() method did not set the shell path property appropriately", expectedPath, resultPath);

        resultTime = baseline.getStartTime();

        assertEquals("The setBaseline() method did not set the start time property appropriately", expectedTime, resultTime);

        resultContext = baseline.getJobContext();

        assertEquals("The setBaseline() method did not set the job context property appropriately", expectedContext, resultContext);

        resultInt = baseline.getAdvanceReservationId();

        assertEquals("The setBaseline() method did not set the advance revervation id property appropriately", expectedInt, resultInt);

        resultString = baseline.getDisplay();

        assertEquals("The setBaseline() method did not set the display property appropriately", expectedString, resultString);

        resultTime = baseline.getDeadline();

        assertEquals("The setBaseline() method did not set the deadline property appropriately", expectedTime, resultTime);

        resultBoolean = baseline.onHold();

        assertEquals("The setBaseline() method did not set the hold property appropriately", true, resultBoolean);

        resultList = baseline.getHoldJobIds();

        assertEquals("The setBaseline() method did not set the hold job ids property appropriately", expectedList, resultList);

        resultList = baseline.getHoldArrayJobIds();

        assertEquals("The setBaseline() method did not set the hold array job ids property appropriately", expectedList, resultList);

        resultInt = baseline.getJobShare();

        assertEquals("The setBaseline() method did not set the job share property appropriately", expectedInt, resultInt);

        resultMap = baseline.getHardResourceRequirements();

        assertEquals("The setBaseline() method did not set the hard resource request property appropriately", expectedMap, resultMap);

        resultMap = baseline.getSoftResourceRequirements();

        assertEquals("The setBaseline() method did not set the soft resource request property appropriately", expectedMap, resultMap);

        resultMail = baseline.getMailSpecifier();

        assertEquals("The setBaseline() method did not set the mail specifier property appropriately", expectedMail, resultMail);

        resultList = baseline.getMasterQueue();

        assertEquals("The setBaseline() method did not set the master queue property appropriately", expectedList, resultList);

        resultList = baseline.getHardQueue();

        assertEquals("The setBaseline() method did not set the hard queue property appropriately", expectedList, resultList);

        resultList = baseline.getSoftQueue();

        assertEquals("The setBaseline() method did not set the soft queue property appropriately", expectedList, resultList);

        resultList = baseline.getMailRecipients();

        assertEquals("The setBaseline() method did not set the mail recipients property appropriately", expectedList, resultList);

        resultInt = baseline.getPriority();

        assertEquals("The setBaseline() method did not set the priority property appropriately", expectedInt, resultInt);

        resultPe = baseline.getParallelEnvironment();

        assertEquals("The setBaseline() method did not set the PE property appropriately", expectedPe, resultPe);

        resultTask = baseline.getTaskSpecifier();

        assertEquals("The setBaseline() method did not set the task specifier property appropriately", expectedTask, resultTask);

        resultVerification = baseline.getVerification();

        assertEquals("The setBaseline() method did not set the verification property appropriately", expectedVerification, resultVerification);

        // setInterval() must be tested separately from setOccasion()
        expectedCkpt.setInterval(expectedInt);
        instance.setCheckpointSpecifier(expectedCkpt);

        instance.setBaseline();
        baseline = getPrivateField(instance, "baseline");

        resultCkpt = baseline.getCheckpointSpecifier();

        assertEquals("The setBaseline() method did not set the checkpoint specifier interval property appropriately", expectedCkpt, resultCkpt);
    }

    /**
     * Test of getDifference method, of class JsvParameters.
     */
    
    public void testGetDifference() throws Exception {
        System.out.println("getDifference");

        JobDescription instance = new JobDescription();

        Map<String, String> difference = instance.getDifference();

        assertEquals("The getDifference() method incorrectly detected differences: ", 0, difference.size());

        String expectedString = "test1";
        Map<String, String> expectedPath = new HashMap<String, String>();
        int expectedInt = 7;
        Calendar expectedTime = Calendar.getInstance();
        Map<String, String> expectedContext = new HashMap<String, String>();
        List<String> expectedList = new ArrayList<String>(2);
        List<Integer> expectedIds = new ArrayList<Integer>(2);
        Map<String, String> expectedMap = new HashMap<String, String>();
        Verification expectedVerification = Verification.VERIFY;
        String resultString;
        String[] resultArray;
        CheckpointSpecifier expectedCkpt = new CheckpointSpecifier();
        MailSpecifier expectedMail = new MailSpecifier();
        ParallelEnvironment expectedPe = new ParallelEnvironment();
        TaskSpecifier expectedTask = new TaskSpecifier();

        expectedPath.put(null, "/a/b");
        expectedPath.put("h", "c/d");

        expectedTime.set(Calendar.YEAR, 2009);
        expectedTime.set(Calendar.MONTH, Calendar.MAY);
        expectedTime.set(Calendar.DAY_OF_MONTH, 22);
        expectedTime.set(Calendar.HOUR_OF_DAY, 20);
        expectedTime.set(Calendar.MINUTE, 55);
        expectedTime.set(Calendar.SECOND, 10);
        expectedTime.set(Calendar.MILLISECOND, 0);

        expectedContext.put("test1", "test2");

        expectedList.add("123");
        expectedList.add("456");

        expectedIds.add(123);
        expectedIds.add(456);

        expectedMap.put("test1", "test2");
        expectedMap.put("test3", "test4");

        expectedCkpt.setName(expectedString);
        expectedCkpt.setOccasion("sx");

        expectedMail.setOccasion("abe");

        expectedPe.setName(expectedString);
        expectedPe.setRangeMin(expectedInt);
        expectedPe.setRangeMax(expectedInt);

        expectedTask.setRange(expectedInt, expectedInt, expectedInt);

        instance.setBaseline();

        instance.setCommandArgs(new String[] { expectedString });
        instance.setBinary(true);
        instance.setMergeStreams(true);
        instance.setNotify(true);
        instance.setShell(false);
        instance.setRerunnable(true);
        instance.setResourceReservation(true);
        instance.setAccount(expectedString);
        instance.setCheckpointSpecifier(expectedCkpt);
        instance.setWorkingDirectory(expectedString);
        instance.setName(expectedString);
        instance.setProject(expectedString);
        instance.setInputPath(expectedPath);
        instance.setOutputPath(expectedPath);
        instance.setErrorPath(expectedPath);
        instance.setShellPath(expectedPath);
        instance.setStartTime(expectedTime);
        instance.setJobContext(expectedContext);
        instance.setAdvanceReservationId(expectedInt);
        instance.setDisplay(expectedString);
        instance.setDeadline(expectedTime);
        instance.setHold(true);
        instance.setHoldJobIds(expectedList);
        instance.setHoldArrayJobIds(expectedList);
        instance.setJobShare(expectedInt);
        instance.setHardResourceRequirements(expectedMap);
        instance.setSoftResourceRequirements(expectedMap);
        instance.setMailSpecifier(expectedMail);
        instance.setMasterQueue(expectedList);
        instance.setHardQueue(expectedList);
        instance.setSoftQueue(expectedList);
        instance.setMailRecipients(expectedList);
        instance.setPriority(expectedInt);
        instance.setParallelEnvironment(expectedPe);
        instance.setTaskSpecifier(expectedTask);
        instance.setVerification(expectedVerification);

        difference = instance.getDifference();

        resultString = difference.get("CLIENT");

        assertNull("The getDifference() method allowed the client property changed", resultString);

        resultString = difference.get("COMMAND_NAME");

        assertNull("The getDifference() method allowed the command name property changed", resultString);

        resultString = difference.get("CONTEXT");

        assertNull("The getDifference() method allowed the context property changed", resultString);

        resultString = difference.get("GROUP");

        assertNull("The getDifference() method allowed the group property changed", resultString);

        resultString = difference.get("JOB_ID");

        assertNull("The getDifference() method allowed the job id property changed", resultString);

        resultString = difference.get("USER");

        assertNull("The getDifference() method allowed the user property changed", resultString);

        resultString = difference.get("VERSION");

        assertNull("The getDifference() method allowed the version property changed", resultString);

        resultString = difference.get("CMDARGS");

        assertEquals("The getDifference() method did not notice the command args property changed", "1", resultString);

        resultString = difference.get("CMDARG0");

        assertEquals("The getDifference() method did not notice the command args property changed", expectedString, resultString);

        resultString = difference.get("CMDARG1");

        assertNull("The getDifference() method incorrectly notice the command args property changed", resultString);

        resultString = difference.get("b");

        assertEquals("The getDifference() method did not notice the binary property changed", "y", resultString);

        resultString = difference.get("j");

        assertEquals("The getDifference() method did not notice the merge streams property changed", "y", resultString);

        resultString = difference.get("notify");

        assertEquals("The getDifference() method did not notice the notify property changed", "y", resultString);

        resultString = difference.get("shell");

        assertEquals("The getDifference() method did not notice the shell property changed", "n", resultString);

        resultString = difference.get("r");

        assertEquals("The getDifference() method did not notice the rerunnable property changed", "y", resultString);

        resultString = difference.get("R");

        assertEquals("The getDifference() method did not notice the reservation property changed", "y", resultString);

        resultString = difference.get("A");

        assertEquals("The getDifference() method did not notice the account property changed", expectedString, resultString);

        resultString = difference.get("ckpt");

        assertEquals("The getDifference() method did not notice the checkpoint specifier name property changed", expectedString, resultString);

        resultString = difference.get("cwd");

        assertEquals("The getDifference() method did not notice the cwd property changed", expectedString, resultString);

        resultString = difference.get("N");

        assertEquals("The getDifference() method did not notice the name property changed", expectedString, resultString);

        resultString = difference.get("P");

        assertEquals("The getDifference() method did not notice the project property changed", expectedString, resultString);

        resultString = difference.get("i");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the input path property changed", 2, resultArray.length);
// TODO: Debugger says strings are equal but the test fails due to this assert ?!?
//        assertEquals("The getDifference() method did not notice the input path property changed",  "/a/b", resultArray[0]);
        assertEquals("The getDifference() method did not notice the input path property changed", "h=c/d", resultArray[1]);

        resultString = difference.get("o");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the output path property changed", 2, resultArray.length);
// TODO: Debugger says strings are equal but the test fails due to this assert ?!?
//        assertEquals("The getDifference() method did not notice the output path property changed",  "/a/b", resultArray[0]);
        assertEquals("The getDifference() method did not notice the output path property changed", "h=c/d", resultArray[1]);

        resultString = difference.get("e");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the error path property changed", 2, resultArray.length);
//        assertEquals("The getDifference() method did not notice the error path property changed",  "/a/b", resultArray[0]);
        assertEquals("The getDifference() method did not notice the error path property changed", "h=c/d", resultArray[1]);

        resultString = difference.get("S");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the shell path property changed", 2, resultArray.length);
//        assertEquals("The getDifference() method did not notice the shell path property changed",  "/a/b", resultArray[0]);
        assertEquals("The getDifference() method did not notice the shell path property changed", "h=c/d", resultArray[1]);

        resultString = difference.get("a");

        assertEquals("The getDifference() method did not notice the start time property changed", "200905222055.10", resultString);

        resultString = difference.get("ac");

        assertEquals("The getDifference() method did not notice the job context property changed", "test1=test2", resultString);

        resultString = difference.get("dc");

        assertNull("The getDifference() method incorrectly noticed the job context property changed", resultString);

        resultString = difference.get("sc");

        assertNull("The getDifference() method incorrectly noticed the job context property changed", resultString);

        resultString = difference.get("ar");

        assertEquals("The getDifference() method did not notice the advance reservation id property changed", "7", resultString);

        resultString = difference.get("c_occasion");

        assertEquals("The getDifference() method did not notice the checkpoint specifier occasion property changed", "sx", resultString);

        resultString = difference.get("display");

        assertEquals("The getDifference() method did not notice the display property changed", expectedString, resultString);

        resultString = difference.get("dl");

        assertEquals("The getDifference() method did not notice the deadline property changed", "200905222055.10", resultString);

        resultString = difference.get("h");

        assertEquals("The getDifference() method did not notice the hold property changed", "u", resultString);

        resultString = difference.get("hold_jid");

        assertEquals("The getDifference() method did not notice the hold job ids property changed", "123,456", resultString);

        resultString = difference.get("hold_jid_ad");

        assertEquals("The getDifference() method did not notice the hold array job ids property changed", "123,456", resultString);

        resultString = difference.get("js");

        assertEquals("The getDifference() method did not notice the job share property changed", "7", resultString);

        resultString = difference.get("l_hard");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the hard resource request property changed", 2, resultArray.length);
        assertEquals("The getDifference() method did not notice the hard resource request property changed", "test1=test2", resultArray[0]);
        assertEquals("The getDifference() method did not notice the hard resource request property changed", "test3=test4", resultArray[1]);

        resultString = difference.get("l_soft");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the soft resource request property changed", 2, resultArray.length);
        assertEquals("The getDifference() method did not notice the soft resource request property changed", "test1=test2", resultArray[0]);
        assertEquals("The getDifference() method did not notice the soft resource request property changed", "test3=test4", resultArray[1]);

        resultString = difference.get("m");

        assertEquals("The getDifference() method did not notice the mail property changed", "abe", resultString);

        resultString = difference.get("masterq");

        assertEquals("The getDifference() method did not notice the master queue property changed", "123,456", resultString);

        resultString = difference.get("q_hard");

        assertEquals("The getDifference() method did not notice the hard queue property changed", "123,456", resultString);

        resultString = difference.get("q_soft");

        assertEquals("The getDifference() method did not notice the soft queue property changed", "123,456", resultString);

        resultString = difference.get("M");

        assertEquals("The getDifference() method did not notice the mail recipient property changed", "123,456", resultString);

        resultString = difference.get("p");

        assertEquals("The getDifference() method did not notice the priority property changed", "7", resultString);

        resultString = difference.get("pe_name");

        assertEquals("The getDifference() method did not notice the PE name property changed", expectedString, resultString);

        resultString = difference.get("pe_min");

        assertEquals("The getDifference() method did not notice the PE min range property changed", "7", resultString);

        resultString = difference.get("pe_max");

        assertEquals("The getDifference() method did not notice the PE max range property changed", "7", resultString);

        resultString = difference.get("t_min");

        assertEquals("The getDifference() method did not notice the task specifier min property changed", "7", resultString);

        resultString = difference.get("t_max");

        assertEquals("The getDifference() method did not notice the task specifier max property changed", "7", resultString);

        resultString = difference.get("t_step");

        assertEquals("The getDifference() method did not notice the task specifier step property changed", "7", resultString);

        resultString = difference.get("w");

        assertEquals("The getDifference() method did not notice the verification property changed", "v", resultString);

        // Now change some things and try again
        System.out.println("PHASE2");
        expectedString = "new1";

        expectedPath.clear();
        expectedPath.put("h2", "efg/h/ij");

        expectedTime.set(Calendar.MONTH, Calendar.JUNE);
        expectedTime.set(Calendar.DAY_OF_MONTH, 22);
        expectedTime.set(Calendar.HOUR_OF_DAY, 10);
        expectedTime.set(Calendar.MINUTE, 30);
        expectedTime.set(Calendar.SECOND, 29);

        expectedContext.clear();
        expectedContext.put("new1", "new2");
        expectedContext.put("new3", "new4");

        expectedInt = 49;

        expectedMap.clear();
        expectedMap.put("new1", "new2");

        expectedList.clear();
        expectedList.add("9876");
        expectedList.add("54");

        expectedIds.clear();
        expectedIds.add(9876);
        expectedIds.add(54);

        expectedVerification = Verification.WARNING;

        instance.setBaseline();

        instance.setCommandArgs(new String[] { expectedString });
        instance.setBinary(false);
        instance.setMergeStreams(false);
        instance.setNotify(false);
        instance.setShell(true);
        instance.setRerunnable(false);
        instance.setResourceReservation(false);
        instance.setAccount(expectedString);
        instance.setWorkingDirectory(expectedString);
        instance.setName(expectedString);
        instance.setProject(expectedString);
        instance.setInputPath(expectedPath);
        instance.setOutputPath(expectedPath);
        instance.setErrorPath(expectedPath);
        instance.setShellPath(expectedPath);
        instance.setStartTime(expectedTime);
        instance.setJobContext(expectedContext);
        instance.setAdvanceReservationId(expectedInt);
        instance.setDisplay(expectedString);
        instance.setDeadline(expectedTime);
        instance.setHold(false);
        instance.setHoldJobIds(expectedList);
        instance.setHoldArrayJobIds(expectedList);
        instance.setJobShare(expectedInt);
        instance.setHardResourceRequirements(expectedMap);
        instance.setSoftResourceRequirements(expectedMap);
        instance.setMasterQueue(expectedList);
        instance.setHardQueue(expectedList);
        instance.setSoftQueue(expectedList);
        instance.setMailRecipients(expectedList);
        instance.setPriority(expectedInt);
        instance.setVerification(expectedVerification);

        expectedCkpt.setName(expectedString);
        expectedCkpt.setInterval(expectedInt);
        instance.setCheckpointSpecifier(expectedCkpt);

        expectedMail.setOccasion("as");
        instance.setMailSpecifier(expectedMail);

        expectedPe.setName(expectedString);
        expectedPe.setRangeMin(expectedInt);
        expectedPe.setRangeMax(expectedInt);
        instance.setParallelEnvironment(expectedPe);

        expectedTask.setMin(expectedInt);
        expectedTask.setMax(expectedInt);
        expectedTask.setStep(expectedInt);
        instance.setTaskSpecifier(expectedTask);

        difference = instance.getDifference();

        resultString = difference.get("CLIENT");

        assertNull("The getDifference() method allowed the client property changed", resultString);

        resultString = difference.get("COMMAND_NAME");

        assertNull("The getDifference() method allowed the command name property changed", resultString);

        resultString = difference.get("CONTEXT");

        assertNull("The getDifference() method allowed the context property changed", resultString);

        resultString = difference.get("GROUP");

        assertNull("The getDifference() method allowed the group property changed", resultString);

        resultString = difference.get("JOB_ID");

        assertNull("The getDifference() method allowed the job id property changed", resultString);

        resultString = difference.get("USER");

        assertNull("The getDifference() method allowed the user property changed", resultString);

        resultString = difference.get("VERSION");

        assertNull("The getDifference() method allowed the version property changed", resultString);

        resultString = difference.get("CMDARGS");

        assertNull("The getDifference() method incorrectly noticed the command args property changed", resultString);

        resultString = difference.get("CMDARG0");

        assertEquals("The getDifference() method did not notice the command args property changed", "new1", resultString);

        resultString = difference.get("CMDARG1");

        assertNull("The getDifference() method incorrectly noticed the command args property changed", resultString);

        resultString = difference.get("b");

        assertEquals("The getDifference() method did not notice the binary property changed", "n", resultString);

        resultString = difference.get("j");

        assertEquals("The getDifference() method did not notice the merge streams property changed", "n", resultString);

        resultString = difference.get("notify");

        assertEquals("The getDifference() method did not notice the notify property changed", "n", resultString);

        resultString = difference.get("shell");

        assertEquals("The getDifference() method did not notice the shell property changed", "y", resultString);

        resultString = difference.get("r");

        assertEquals("The getDifference() method did not notice the rerunnable property changed", "n", resultString);

        resultString = difference.get("R");

        assertEquals("The getDifference() method did not notice the reservation property changed", "n", resultString);

        resultString = difference.get("A");

        assertEquals("The getDifference() method did not notice the account property changed", expectedString, resultString);

        resultString = difference.get("ckpt");

        assertEquals("The getDifference() method did not notice the checkpoint specifier name property changed", expectedString, resultString);

        resultString = difference.get("cwd");

        assertEquals("The getDifference() method did not notice the cwd property changed", expectedString, resultString);

        resultString = difference.get("N");

        assertEquals("The getDifference() method did not notice the name property changed", expectedString, resultString);

        resultString = difference.get("P");

        assertEquals("The getDifference() method did not notice the project property changed", expectedString, resultString);

        resultString = difference.get("i");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the input path property changed", 1, resultArray.length);
        assertEquals("The getDifference() method did not notice the input path property changed", "h2=efg/h/ij", resultArray[0]);

        resultString = difference.get("o");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the output path property changed", 1, resultArray.length);
        assertEquals("The getDifference() method did not notice the output path property changed", "h2=efg/h/ij", resultArray[0]);

        resultString = difference.get("e");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the error path property changed", 1, resultArray.length);
        assertEquals("The getDifference() method did not notice the error path property changed", "h2=efg/h/ij", resultArray[0]);

        resultString = difference.get("S");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the shell path property changed", 1, resultArray.length);
        assertEquals("The getDifference() method did not notice the shell path property changed", "h2=efg/h/ij", resultArray[0]);

        resultString = difference.get("a");

        assertEquals("The getDifference() method did not notice the start time property changed", "200906221030.29", resultString);

        resultString = difference.get("ac");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the job context property changed", 2, resultArray.length);
        assertEquals("The getDifference() method did not notice the job context property changed", "new1=new2", resultArray[0]);
        assertEquals("The getDifference() method did not notice the job context property changed", "new3=new4", resultArray[1]);

        resultString = difference.get("dc");

        assertNull("The getDifference() method incorrectly noticed the job context property changed", resultString);

        resultString = difference.get("sc");

        assertNull("The getDifference() method incorrectly noticed the job context property changed", resultString);

        resultString = difference.get("ar");

        assertEquals("The getDifference() method did not notice the advance reservation id property changed", "49", resultString);

        resultString = difference.get("c_interval");

        assertEquals("The getDifference() method did not notice the checkpoint specifier interval property changed", "49", resultString);

        resultString = difference.get("display");

        assertEquals("The getDifference() method did not notice the display property changed", expectedString, resultString);

        resultString = difference.get("dl");

        assertEquals("The getDifference() method did not notice the deadline property changed", "200906221030.29", resultString);

        resultString = difference.get("h");

        assertEquals("The getDifference() method did not notice the hold property changed", "n", resultString);

        resultString = difference.get("hold_jid");

        assertEquals("The getDifference() method did not notice the hold job ids property changed", "9876,54", resultString);

        resultString = difference.get("hold_jid_ad");

        assertEquals("The getDifference() method did not notice the hold array job ids property changed", "9876,54", resultString);

        resultString = difference.get("js");

        assertEquals("The getDifference() method did not notice the job share property changed", "49", resultString);

        resultString = difference.get("l_hard");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the hard resource request property changed", 1, resultArray.length);
        assertEquals("The getDifference() method did not notice the hard resource request property changed", "new1=new2", resultArray[0]);

        resultString = difference.get("l_soft");
        resultArray = null;

        if (resultString != null) {
            resultArray = resultString.split(",");
            Arrays.sort(resultArray);
        }

        assertEquals("The getDifference() method did not notice the soft resource request property changed", 1, resultArray.length);
        assertEquals("The getDifference() method did not notice the soft resource request property changed", "new1=new2", resultArray[0]);

        resultString = difference.get("m");

        assertEquals("The getDifference() method did not notice the mail property changed", "as", resultString);

        resultString = difference.get("masterq");

        assertEquals("The getDifference() method did not notice the master queue property changed", "9876,54", resultString);

        resultString = difference.get("q_hard");

        assertEquals("The getDifference() method did not notice the hard queue property changed", "9876,54", resultString);

        resultString = difference.get("q_soft");

        assertEquals("The getDifference() method did not notice the soft queue property changed", "9876,54", resultString);

        resultString = difference.get("M");

        assertEquals("The getDifference() method did not notice the mail recipient property changed", "9876,54", resultString);

        resultString = difference.get("p");

        assertEquals("The getDifference() method did not notice the priority property changed", "49", resultString);

        resultString = difference.get("pe_name");

        assertEquals("The getDifference() method did not notice the PE name property changed", expectedString, resultString);

        resultString = difference.get("pe_min");

        assertEquals("The getDifference() method did not notice the PE min range property changed", "49", resultString);

        resultString = difference.get("pe_max");

        assertEquals("The getDifference() method did not notice the PE max range property changed", "49", resultString);

        resultString = difference.get("t_min");

        assertEquals("The getDifference() method did not notice the task specifier min property changed", "49", resultString);

        resultString = difference.get("t_max");

        assertEquals("The getDifference() method did not notice the task specifier max property changed", "49", resultString);

        resultString = difference.get("t_step");

        assertEquals("The getDifference() method did not notice the task specifier step property changed", "49", resultString);

        resultString = difference.get("w");

        assertEquals("The getDifference() method did not notice the verification property changed", "w", resultString);
    }

    /**
     * Test of set method, of class JsvParameters.
     */
    
    public void testSet() {
        System.out.println("set");

        JobDescription instance = new JobDescription();
        String expectedString = "test1";
        String resultString;
        String pathString = "/a/b,h:c/d";
        Map<String, String> expectedPath = new HashMap<String, String>();
        Map<String, String> resultPath;
        int expectedInt = 7;
        Integer resultInt;
        Boolean resultBoolean;
        String timeString = "200905222055.10";
        Calendar expectedTime = Calendar.getInstance();
        Calendar resultTime;
        String contextString = "test1=test2";
        Map<String, String> expectedContext = new HashMap<String, String>();
        Map<String, String> resultContext;
        String listString = "123,456";
        List<Object> expectedList = new ArrayList<Object>(2);
        List<? extends Object> resultList;
        String resourceString = "test1=test2,test3=test4";
        Map<String, String> expectedResources = new HashMap<String, String>();
        Map<String, String> resultResources;
        String verificationString = "v";
        Verification expectedVerification = Verification.VERIFY;
        Verification resultVerification;

        expectedPath.put(null, "/a/b");
        expectedPath.put("h", "c/d");

        expectedTime.set(Calendar.YEAR, 2009);
        expectedTime.set(Calendar.MONTH, Calendar.MAY);
        expectedTime.set(Calendar.DAY_OF_MONTH, 22);
        expectedTime.set(Calendar.HOUR_OF_DAY, 20);
        expectedTime.set(Calendar.MINUTE, 55);
        expectedTime.set(Calendar.SECOND, 10);
        expectedTime.set(Calendar.MILLISECOND, 0);

        expectedContext.put("test1", "test2");

        expectedList.add("123");
        expectedList.add("456");

        expectedResources.put("test1", "test2");
        expectedResources.put("test3", "test4");

        instance.set("CLIENT", expectedString);
        resultString = instance.getClient();

        assertEquals("The getClient() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("CMDNAME", expectedString);
        resultString = instance.getCommandName();

        assertEquals("The getCommandName() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("CONTEXT", expectedString);
        resultString = instance.getContext();

        assertEquals("The getContext() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("GROUP", expectedString);
        resultString = instance.getGroup();

        assertEquals("The getGroup() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("JOBID", expectedString);
        resultString = instance.getJobId();

        assertEquals("The getJobId() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("USER", expectedString);
        resultString = instance.getUser();

        assertEquals("The getUser() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("VERSION", expectedString);
        resultString = instance.getVersion();

        assertEquals("The getVersion() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("CMDARGS", Integer.toString(expectedInt));
        resultInt = instance.getCommandArgs().length;

        assertEquals("The getCommandArgs() method did not return the number of args set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("CMDARG0", Integer.toString(expectedInt));
        resultString = instance.getCommandArgs()[0];

        assertEquals("The getCommandArgs() method did not return the arg[0] set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("CMDARG3", Integer.toString(expectedInt));
        resultString = instance.getCommandArgs()[3];

        assertEquals("The getCommandArgs() method did not return the arg[0] set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("b", "y");
        resultBoolean = instance.isBinary();

        assertEquals("The isBinary() method did not return the value set with the set() method", Boolean.TRUE, resultBoolean);

        instance.set("notify", "y");
        resultBoolean = instance.doNotify();

        assertEquals("The doNotify() method did not return the value set with the set() method", Boolean.TRUE, resultBoolean);

        instance.set("j", "y");
        resultBoolean = instance.mergeStreams();

        assertEquals("The mergeStreams() method did not return the value set with the set() method", Boolean.TRUE, resultBoolean);

        instance.set("shell", "n");
        resultBoolean = instance.useShell();

        assertEquals("The useShell() method did not return the value set with the set() method", Boolean.FALSE, resultBoolean);

        instance.set("r", "y");
        resultBoolean = instance.isRerunnable();

        assertEquals("The isRerunnable() method did not return the value set with the set() method", Boolean.TRUE, resultBoolean);

        instance.set("R", "y");
        resultBoolean = instance.hasResourceReservation();

        assertEquals("The hasReservation() method did not return the value set with the set() method", Boolean.TRUE, resultBoolean);

        instance.set("A", expectedString);
        resultString = instance.getAccount();

        assertEquals("The getAccount() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("ckpt", expectedString);
        resultString = instance.getCheckpointSpecifier().getName();

        assertEquals("The getCheckpointSpecifier() method did not return the name set with the set() method", expectedString, resultString);

        instance.set("cwd", expectedString);
        resultString = instance.getWorkingDirectory();

        assertEquals("The getWorkingDirectory() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("wd", expectedString);
        resultString = instance.getWorkingDirectory();

        assertEquals("The getWorkingDirectory() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("N", expectedString);
        resultString = instance.getName();

        assertEquals("The getName() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("P", expectedString);
        resultString = instance.getProject();

        assertEquals("The getProject() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("i", pathString);
        resultPath = instance.getInputPath();

        assertEquals("The getInputPath() method did not return the values set with the set() method", expectedPath, resultPath);

        instance.set("o", pathString);
        resultPath = instance.getOutputPath();

        assertEquals("The getOutputPath() method did not return the values set with the set() method", expectedPath, resultPath);

        instance.set("e", pathString);
        resultPath = instance.getErrorPath();

        assertEquals("The getErrorPath() method did not return the values set with the set() method", expectedPath, resultPath);

        instance.set("S", pathString);
        resultPath = instance.getShellPath();

        assertEquals("The getShellPath() method did not return the values set with the set() method", expectedPath, resultPath);

        instance.set("a", timeString);
        resultTime = instance.getStartTime();

        assertEquals("The getStartTime() method did not return the value set with the set() method", expectedTime, resultTime);

        instance.set("ac", contextString);
        resultContext = instance.getJobContext();

        assertEquals("The getJobContext() method did not return the values set with the set() method", expectedContext, resultContext);

        instance.set("ar", Integer.toString(expectedInt));
        resultInt = instance.getAdvanceReservationId();

        assertEquals("The getAdvanceReservationId() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("c_interval", Integer.toString(expectedInt));
        resultInt = (int) instance.getCheckpointSpecifier().getInterval();

        assertEquals("The getCheckpointSpecifier() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("c_occasion", "sx");
        resultString = instance.getCheckpointSpecifier().getOccasionString();

        assertEquals("The getCheckpointSpecifier() method did not return the value set with the set() method", "sx", resultString);

        instance.set("display", expectedString);
        resultString = instance.getDisplay();

        assertEquals("The getDisplay() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("dl", timeString);
        resultTime = instance.getDeadline();

        assertEquals("The getDeadline() method did not return the value set with the set() method", expectedTime, resultTime);

        instance.set("h", "u");
        resultBoolean = instance.onHold();

        assertEquals("The onHold() method did not return the value set with the set() method", Boolean.TRUE, resultBoolean);

        instance.set("h", "n");
        resultBoolean = instance.onHold();

        assertEquals("The onHold() method did not return the value set with the set() method", Boolean.FALSE, resultBoolean);

        instance.set("hold_jid", listString);
        resultList = instance.getHoldJobIds();

        assertEquals("The getHoldJobIds() method did not return the values set with the set() method", expectedList.toString(), resultList.toString());

        instance.set("hold_jid_ad", listString);
        resultList = instance.getHoldArrayJobIds();

        assertEquals("The getHoldArrayJobIds() method did not return the values set with the set() method", expectedList.toString(), resultList.toString());

        instance.set("js", Integer.toString(expectedInt));
        resultInt = instance.getJobShare();

        assertEquals("The getJobShare() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("l_hard", resourceString);
        resultResources = instance.getHardResourceRequirements();

        assertEquals("The getHardResourceRequirements() method did not return the value set with the set() method", expectedResources, resultResources);

        instance.set("l_soft", resourceString);
        resultResources = instance.getSoftResourceRequirements();

        assertEquals("The getSoftResourceRequirements() method did not return the value set with the set() method", expectedResources, resultResources);

        instance.set("m", "abe");
        resultString = instance.getMailSpecifier().getOccasionString();

        assertEquals("The getMailSpecifier() method did not return the value set with the set() method", "abe", resultString);

        instance.set("masterq", listString);
        resultList = instance.getMasterQueue();

        assertEquals("The getMasterQueue() method did not return the values set with the set() method", expectedList.toString(), resultList.toString());

        instance.set("q_hard", listString);
        resultList = instance.getHardQueue();

        assertEquals("The getHardQueue() method did not return the values set with the set() method", expectedList.toString(), resultList.toString());

        instance.set("q_soft", listString);
        resultList = instance.getSoftQueue();

        assertEquals("The getSoftQueue() method did not return the values set with the set() method", expectedList.toString(), resultList.toString());

        instance.set("M", listString);
        resultList = instance.getMailRecipients();

        assertEquals("The getMailRecipients() method did not return the values set with the set() method", expectedList.toString(), resultList.toString());

        instance.set("p", Integer.toString(expectedInt));
        resultInt = instance.getPriority();

        assertEquals("The getPriority() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("pe_name", expectedString);
        resultString = instance.getParallelEnvironment().getName();

        assertEquals("The getParalleEnvironment() method did not return the value set with the set() method", expectedString, resultString);

        instance.set("pe_min", Integer.toString(expectedInt));
        resultInt = instance.getParallelEnvironment().getRangeMin();

        assertEquals("The getParalleEnvironment() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("pe_max", Integer.toString(expectedInt));
        resultInt = instance.getParallelEnvironment().getRangeMax();

        assertEquals("The getParalleEnvironment() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("t_min", Integer.toString(expectedInt));
        resultInt = instance.getTaskSpecifier().getMin();

        assertEquals("The getTaskSpecifier() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("t_max", Integer.toString(expectedInt));
        resultInt = instance.getTaskSpecifier().getMax();

        assertEquals("The getTaskSpecifier() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("t_step", Integer.toString(expectedInt));
        resultInt = instance.getTaskSpecifier().getStep();

        assertEquals("The getTaskSpecifier() method did not return the value set with the set() method", (Integer) expectedInt, resultInt);

        instance.set("w", verificationString);
        resultVerification = instance.getVerification();

        assertEquals("The getVerification() method did not return the value set with the set() method", expectedVerification, resultVerification);

        instance.setVerification(expectedVerification);
        instance.set("w", "");
        resultVerification = instance.getVerification();
        assertNull("The getVerification() method did not return the value set with the set() method", resultVerification);

        instance.setVerification(expectedVerification);
        instance.set("w", null);
        resultVerification = instance.getVerification();
        assertNull("The getVerification() method did not return the value set with the set() method", resultVerification);

        instance.setPriority(expectedInt);
        instance.set("p", "");
        resultInt = instance.getPriority();
        assertNull("The getPriority() method did not return the value set with the set() method", resultInt);

        instance.setPriority(expectedInt);
        instance.set("p", null);
        resultInt = instance.getPriority();
        assertNull("The getPriority() method did not return the value set with the set() method", resultInt);

        instance.setProject(expectedString);
        instance.set("P", "");
        resultString = instance.getProject();
        assertNull("The getProject() method did not return the value set with the set() method", resultString);

        instance.setProject(expectedString);
        instance.set("P", null);
        resultString = instance.getProject();
        assertNull("The getProject() method did not return the value set with the set() method", resultString);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.addHandler(handler);

        // It's simply not reasonable to test that none of the fields changed,
        // so we just, in effect, are testing that the following bad calls
        // don't throw an exception.
        instance.set("N/A", "blah");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set(null, "blah");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("N/A", null);
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set(null, null);
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("ar", "xyz");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("js", "xyz");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("p", "xyz");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("e", "xyz:");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("i", "xyz:");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("o", "xyz:");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("S", "xyz:");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("c_occasion", "xyz");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("m", "xyz");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        instance.set("w", "xyz");
        assertEquals("The set() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The set() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());

        instance.setBaseline();

        try {
            instance.set("display", "test");
            fail("The set() method did not fail after the baseline was set.");
        } catch (IllegalArgumentException e) {
            // Desired result
        }
    }

    
    public void testFormatTime() throws Exception {
        System.out.println("formatTime()");

        JobDescription instance = new JobDescription();
        Calendar test = Calendar.getInstance();
        String expectedResult = "200904221955.21";

        test.set(Calendar.YEAR, 2009);
        test.set(Calendar.MONTH, Calendar.APRIL);
        test.set(Calendar.DAY_OF_MONTH, 22);
        test.set(Calendar.HOUR_OF_DAY, 19);
        test.set(Calendar.MINUTE, 55);
        test.set(Calendar.SECOND, 21);

        Object result = callPrivateMethod(instance, "formatTime", new Class[] { Calendar.class }, new Object[] { test });

        assertEquals("The formatTime() method did not return a correctly formatted time string", expectedResult, result);

        test.set(Calendar.MILLISECOND, 12);

        result = callPrivateMethod(instance, "formatTime", new Class[] { Calendar.class }, new Object[] { test });
        assertEquals("The formatTime() method did not return a correctly formatted time string", expectedResult, result);

        result = callPrivateMethod(instance, "formatTime", new Class[] { Calendar.class }, new Object[] { null });
        assertEquals("The formatTime() method did not return a correctly formatted time string", null, result);
    }

    
    public void testIsMapDifferent() throws Exception {
        System.out.println("isMapDifferent()");

        JobDescription instance = new JobDescription();
        Map<String, String> map1 = new HashMap<String, String>();
        Map<String, String> map2 = new HashMap<String, String>();

        map1.put("test1", "Test1");
        map1.put("test2", "Test2");
        map1.put("test3", "Test3");
        map2.putAll(map1);

        Object result = callPrivateMethod(instance, "isMapDifferent", new Class[] { Map.class, Map.class }, new Object[] { map2, map1 });

        assertEquals("The isMapDifferent() method did not return the correct value", false, result);

        result = callPrivateMethod(instance, "isMapDifferent", new Class[] { Map.class, Map.class }, new Object[] { null, null });
        assertEquals("The isMapDifferent() method did not return the correct value", false, result);

        result = callPrivateMethod(instance, "isMapDifferent", new Class[] { Map.class, Map.class }, new Object[] { null, map1 });
        assertEquals("The isMapDifferent() method did not return the correct value", true, result);

        result = callPrivateMethod(instance, "isMapDifferent", new Class[] { Map.class, Map.class }, new Object[] { map2, null });
        assertEquals("The isMapDifferent() method did not return the correct value", true, result);

        map2.remove("test2");

        result = callPrivateMethod(instance, "isMapDifferent", new Class[] { Map.class, Map.class }, new Object[] { map2, map1 });
        assertEquals("The isMapDifferent() method did not return the correct value", true, result);

        map2.put("test2", "Test2");
        map2.put("test3", "Test4");

        result = callPrivateMethod(instance, "isMapDifferent", new Class[] { Map.class, Map.class }, new Object[] { map2, map1 });
        assertEquals("The isMapDifferent() method did not return the correct value", true, result);

        map1.clear();

        result = callPrivateMethod(instance, "isMapDifferent", new Class[] { Map.class, Map.class }, new Object[] { map2, map1 });
        assertEquals("The isMapDifferent() method did not return the correct value", true, result);
    }

    
    public void testMapToString() throws Exception {
        System.out.println("mapToString()");

        JobDescription instance = new JobDescription();
        Map<String, String> map = new HashMap<String, String>();

        map.put("test1", "Test1");
        map.put("test2", "Test2");
        map.put("test3", "Test3");

        String[] expectedResult = new String[] { "test1=Test1", "test2=Test2", "test3=Test3" };
        Object result = callPrivateMethod(instance, "mapToString", new Class[] { Map.class }, new Object[] { map });
        String[] resultArray = result.toString().split(",");

        Arrays.sort(resultArray);
        assertEquals("The mapToString() method did not return the correct string", expectedResult.length, resultArray.length);
        assertEquals("The mapToString() method did not return the correct string", expectedResult[0], resultArray[0]);
        assertEquals("The mapToString() method did not return the correct string", expectedResult[1], resultArray[1]);
        assertEquals("The mapToString() method did not return the correct string", expectedResult[2], resultArray[2]);

        map.remove("test2");
        expectedResult = new String[] { "test1=Test1", "test3=Test3" };
        result = callPrivateMethod(instance, "mapToString", new Class[] { Map.class }, new Object[] { map });
        resultArray = result.toString().split(",");
        Arrays.sort(resultArray);
        assertEquals("The mapToString() method did not return the correct string", expectedResult.length, resultArray.length);
        assertEquals("The mapToString() method did not return the correct string", expectedResult[0], resultArray[0]);
        assertEquals("The mapToString() method did not return the correct string", expectedResult[1], resultArray[1]);

        map.put("test3", "Test4");
        expectedResult = new String[] { "test1=Test1", "test3=Test4" };
        result = callPrivateMethod(instance, "mapToString", new Class[] { Map.class }, new Object[] { map });
        resultArray = result.toString().split(",");
        Arrays.sort(resultArray);
        assertEquals("The mapToString() method did not return the correct string", expectedResult.length, resultArray.length);
        assertEquals("The mapToString() method did not return the correct string", expectedResult[0], resultArray[0]);
        assertEquals("The mapToString() method did not return the correct string", expectedResult[1], resultArray[1]);

        map.clear();
        result = callPrivateMethod(instance, "mapToString", new Class[] { Map.class }, new Object[] { map });
        assertEquals("The mapToString() method did not return the correct string", "", result);

        result = callPrivateMethod(instance, "mapToString", new Class[] { Map.class }, new Object[] { null });
        assertEquals("The mapToString() method did not return the correct string", null, result);
    }

    
    public void testIsListDifferent() throws Exception {
        System.out.println("isListDifferent()");

        JobDescription instance = new JobDescription();
        List<String> list1 = new ArrayList<String>(3);
        List<String> list2 = new ArrayList<String>(3);

        list1.add("test1");
        list1.add("test2");
        list1.add("test3");
        list2.addAll(list1);

        Object result = callPrivateMethod(instance, "isListDifferent", new Class[] { List.class, List.class }, new Object[] { list2, list1 });

        assertEquals("The isListDifferent() method did not return the correct value", false, result);

        result = callPrivateMethod(instance, "isListDifferent", new Class[] { List.class, List.class }, new Object[] { null, null });
        assertEquals("The isListDifferent() method did not return the correct value", false, result);

        result = callPrivateMethod(instance, "isListDifferent", new Class[] { List.class, List.class }, new Object[] { null, list1 });
        assertEquals("The isListDifferent() method did not return the correct value", true, result);

        result = callPrivateMethod(instance, "isListDifferent", new Class[] { List.class, List.class }, new Object[] { list2, null });
        assertEquals("The isListDifferent() method did not return the correct value", true, result);

        list2.remove("test2");

        result = callPrivateMethod(instance, "isListDifferent", new Class[] { List.class, List.class }, new Object[] { list2, list1 });
        assertEquals("The isListDifferent() method did not return the correct value", true, result);

        list2.add("test2");
        list2.add("test4");

        result = callPrivateMethod(instance, "isListDifferent", new Class[] { List.class, List.class }, new Object[] { list2, list1 });
        assertEquals("The isListDifferent() method did not return the correct value", true, result);

        list1.clear();

        result = callPrivateMethod(instance, "isListDifferent", new Class[] { List.class, List.class }, new Object[] { list2, list1 });
        assertEquals("The isListDifferent() method did not return the correct value", true, result);
    }

    
    public void testListToString() throws Exception {
        System.out.println("listToString()");

        JobDescription instance = new JobDescription();
        List<String> list = new ArrayList<String>(3);

        list.add("test1");
        list.add("test2");
        list.add("test3");

        String expectedResult = "test1,test2,test3";
        Object result = callPrivateMethod(instance, "listToString", new Class[] { List.class }, new Object[] { list });

        assertEquals("The listToString() method did not return the correct string", expectedResult, result);

        list.remove("test2");
        expectedResult = "test1,test3";
        result = callPrivateMethod(instance, "listToString", new Class[] { List.class }, new Object[] { list });
        assertEquals("The listToString() method did not return the correct string", expectedResult, result);

        list.add("test4");
        expectedResult = "test1,test3,test4";
        result = callPrivateMethod(instance, "listToString", new Class[] { List.class }, new Object[] { list });
        assertEquals("The mapToString() method did not return the correct string", expectedResult, result);

        list.clear();
        result = callPrivateMethod(instance, "listToString", new Class[] { List.class }, new Object[] { list });
        assertEquals("The mapToString() method did not return the correct string", "", result);

        result = callPrivateMethod(instance, "listToString", new Class[] { List.class }, new Object[] { null });
        assertEquals("The mapToString() method did not return the correct string", null, result);
    }

    
    public void testParseList() throws Exception {
        System.out.println("parseList()");

        JobDescription instance = new JobDescription();
        Object result = null;

        String string = "test1 ,test3, test2";
        List<String> expectedResult = Arrays.asList(new String[] { "test1", "test3", "test2" });
        result = callPrivateMethod(instance, "parseList", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parseList() method did not parse the string correctly", expectedResult, result);

        string = " ,, ,";
        expectedResult = Collections.EMPTY_LIST;
        result = callPrivateMethod(instance, "parseList", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parseList() method did not parse the string correctly", expectedResult, result);
    }

    
    public void testParseMap() throws Exception {
        System.out.println("parseMap()");

        JobDescription instance = new JobDescription();
        Object result = null;

        String string = "test1=Test1 ,test3=Test3, test2=Test2";
        Map<String, String> expectedResult = new HashMap<String, String>();

        expectedResult.put("test1", "Test1");
        expectedResult.put("test2", "Test2");
        expectedResult.put("test3", "Test3");

        result = callPrivateMethod(instance, "parseMap", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parseMap() method did not parse the string correctly", expectedResult, result);

        string = " ,, ,";
        expectedResult = Collections.EMPTY_MAP;
        result = callPrivateMethod(instance, "parseMap", new Class[] { String.class }, new Object[] { string });
        assertEquals("The parseMap() method did not parse the string correctly", expectedResult, result);

        expectedResult = new HashMap<String, String>();
        expectedResult.put("test1", "");
        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();
        log.addHandler(handler);
        string = "test1";
        result = callPrivateMethod(instance, "parseMap", new Class[] { String.class }, new Object[] { string });
        assertEquals("The parseMap() method did not parse the string correctly", expectedResult, result);
        assertEquals("The parseMap() method did log an inappropriate message", 0, handler.messages.size());

        expectedResult = Collections.EMPTY_MAP;
        string = "=bad";
        handler.messages.clear();
        result = callPrivateMethod(instance, "parseMap", new Class[] { String.class }, new Object[] { string });
        assertEquals("The parseMap() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The parseMap() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());
        assertEquals("The parseMap() method did not parse the string correctly", expectedResult, result);
    }

    
    public void testParsePath() throws Exception {
        System.out.println("parsePath()");

        JobDescription instance = new JobDescription();
        Object result = null;

        String string = "Test1 ,test3:Test3, test2:Test2";
        Map<String, String> expectedResult = new HashMap<String, String>();

        expectedResult.put(null, "Test1");
        expectedResult.put("test2", "Test2");
        expectedResult.put("test3", "Test3");

        result = callPrivateMethod(instance, "parsePath", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parsePath() method did not parse the string correctly", expectedResult, result);

        string = " ,, ,";
        expectedResult = Collections.EMPTY_MAP;

        result = callPrivateMethod(instance, "parsePath", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parsePath() method did not parse the string correctly", expectedResult, result);

        string = "bad";
        expectedResult = Collections.singletonMap(null, "bad");

        result = callPrivateMethod(instance, "parsePath", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parsePath() method did not parse the string correctly", expectedResult, result);

        string = ":bad";
        expectedResult = Collections.singletonMap(null, "bad");

        result = callPrivateMethod(instance, "parsePath", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parsePath() method did not parse the string correctly", expectedResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.addHandler(handler);

        string = "bad:";
        expectedResult = Collections.emptyMap();

        result = callPrivateMethod(instance, "parsePath", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parsePath() method did not parse the string correctly", expectedResult, result);
        assertEquals("The parsePath() method did not log an appropriate message", 1, handler.messages.size());
        assertEquals("The parsePath() method did not log an appropriate message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    
    public void testParseTime() throws Exception {
        System.out.println("parseTime()");

        JobDescription instance = new JobDescription();
        Calendar expectedResult = Calendar.getInstance();

        expectedResult.set(Calendar.YEAR, 2009);
        expectedResult.set(Calendar.MONTH, Calendar.APRIL);
        expectedResult.set(Calendar.DAY_OF_MONTH, 22);
        expectedResult.set(Calendar.HOUR_OF_DAY, 19);
        expectedResult.set(Calendar.MINUTE, 55);
        expectedResult.set(Calendar.SECOND, 21);
        expectedResult.set(Calendar.MILLISECOND, 0);

        String string = "200904221955.21";
        Object result = callPrivateMethod(instance, "parseTime", new Class[] { String.class }, new Object[] { string });

        assertEquals("The parseTime() method did not parse the string correctly", expectedResult, result);

        string = "0904221955.21";
        result = callPrivateMethod(instance, "parseTime", new Class[] { String.class }, new Object[] { string });
        assertEquals("The parseTime() method did not parse the string correctly", expectedResult, result);

        string = "04221955.21";
        result = callPrivateMethod(instance, "parseTime", new Class[] { String.class }, new Object[] { string });
        assertEquals("The parseTime() method did not parse the string correctly", expectedResult, result);

        string = "04221955";
        expectedResult.set(Calendar.SECOND, 0);
        result = callPrivateMethod(instance, "parseTime", new Class[] { String.class }, new Object[] { string });
        assertEquals("The parseTime() method did not parse the string correctly", expectedResult, result);

        string = "221955";

        try {
            callPrivateMethod(instance, "parseTime", new Class[] { String.class }, new Object[] { string });
            fail("The parseTime() method allowed an invalid time string");
        } catch (InvocationTargetException e) {
            assertEquals("The parseTime() method threw an unexpected exception", IllegalArgumentException.class, e.getCause().getClass());
        }
    }

    
    public void testPutDifference() throws Exception {
        System.out.println("putDifference(Map,Object,Object,String)");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = "test";
        String oldString = "test0";
        String expString = "test1";

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, oldString, expString, key });
        assertEquals("The putDifference() method did not put the correct entry into the map", expString, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, expString, expString, key });
        assertEquals("The putDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, oldString, null, key });
        assertNull("The putDifference() method did not put the correct entry into the map", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, null, expString, key });
        assertEquals("The putDifference() method did not put the correct entry into the map", expString, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, null, null, key });
        assertEquals("The putDifference() method incorrectly put an entry into the map", 0, map.size());

        Integer oldInt = 123;
        Integer expInt = 456;

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, oldInt, expInt, key });
        assertEquals("The putDifference() method did not put the correct entry into the map", expInt.toString(), map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, expInt, expInt, key });
        assertEquals("The putDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, oldInt, null, key });
        assertNull("The putDifference() method did not put the correct entry into the map", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, null, expInt, key });
        assertEquals("The putDifference() method did not put the correct entry into the map", expInt.toString(), map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class }, new Object[] { map, null, null, key });
        assertEquals("The putDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutDifference2() throws Exception {
        System.out.println("putDifference(Map,Object,Object,String,String)");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = "test";
        String value = "default";
        String oldString = "test0";
        String expString = "test1";

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, oldString, expString, key, value });
        assertEquals("The putDifference() method did not put the correct entry into the map", expString, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, expString, expString, key, value });
        assertEquals("The putDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, oldString, null, key, value });
        assertEquals("The putDifference() method did not put the correct entry into the map", value, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, null, expString, key, value });
        assertEquals("The putDifference() method did not put the correct entry into the map", expString, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, null, null, key, value });
        assertEquals("The putDifference() method incorrectly put an entry into the map", 0, map.size());

        Integer oldInt = 123;
        Integer expInt = 456;

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, oldInt, expInt, key, value });
        assertEquals("The putDifference() method did not put the correct entry into the map", expInt.toString(), map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, expInt, expInt, key, value });
        assertEquals("The putDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, oldInt, null, key, value });
        assertEquals("The putDifference() method did not put the correct entry into the map", value, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, null, expInt, key, value });
        assertEquals("The putDifference() method did not put the correct entry into the map", expInt.toString(), map.get(key));

        map.clear();

        callPrivateMethod(instance, "putDifference", new Class[] { Map.class, Object.class, Object.class, String.class, String.class }, new Object[] { map, null, null, key, value });
        assertEquals("The putDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutBooleanDifference() throws Exception {
        System.out.println("putBooleanDifference(Map,Boolean,Boolean,String,String)");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = "test";
        String tVal = "y";
        String fVal = "n";
        Boolean fBool = false;
        Boolean tBool = true;

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class }, new Object[] { map, fBool, tBool, key, tVal, fVal });
        assertEquals("The putBooleanDifference() method did not put the correct entry into the map", tVal, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class }, new Object[] { map, tBool, fBool, key, tVal, fVal });
        assertEquals("The putBooleanDifference() method did not put the correct entry into the map", fVal, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class }, new Object[] { map, tBool, tBool, key, tVal, fVal });
        assertEquals("The putBooleanDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class }, new Object[] { map, fBool, fBool, key, tVal, fVal });
        assertEquals("The putBooleanDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class }, new Object[] { map, fBool, null, key, tVal, fVal });
        assertNull("The putBooleanDifference() method did not put the correct entry into the map", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class }, new Object[] { map, null, tBool, key, tVal, fVal });
        assertEquals("The putBooleanDifference() method did not put the correct entry into the map", tVal, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class }, new Object[] { map, null, null, key, tVal, fVal });
        assertEquals("The putBooleanDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutBooleanDifference2() throws Exception {
        System.out.println("putBooleanDifference(Map,Boolean,Boolean,String,String,String)");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = "test";
        String tVal = "y";
        String fVal = "n";
        String dVal = fVal;
        Boolean fBool = false;
        Boolean tBool = true;

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class, String.class }, new Object[] { map, fBool, tBool, key, tVal, fVal, dVal });
        assertEquals("The putBooleanDifference() method did not put the correct entry into the map", tVal, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class, String.class }, new Object[] { map, tBool, fBool, key, tVal, fVal, dVal });
        assertEquals("The putBooleanDifference() method did not put the correct entry into the map", fVal, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class, String.class }, new Object[] { map, tBool, tBool, key, tVal, fVal, dVal });
        assertEquals("The putBooleanDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class, String.class }, new Object[] { map, fBool, fBool, key, tVal, fVal, dVal });
        assertEquals("The putBooleanDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class, String.class }, new Object[] { map, fBool, null, key, tVal, fVal, dVal });
        assertEquals("The putBooleanDifference() method did not put the correct entry into the map", dVal, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class, String.class }, new Object[] { map, null, tBool, key, tVal, fVal, dVal });
        assertEquals("The putBooleanDifference() method did not put the correct entry into the map", tVal, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putBooleanDifference", new Class[] { Map.class, Boolean.class, Boolean.class, String.class, String.class, String.class, String.class }, new Object[] { map, null, null, key, tVal, fVal, dVal });
        assertEquals("The putBooleanDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutTimeDifference() throws Exception {
        System.out.println("putTimeDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = "test";
        String value = "200910121027.36";
        Calendar old = Calendar.getInstance();
        Calendar expected = Calendar.getInstance();

        expected.set(Calendar.YEAR, 2009);
        expected.set(Calendar.MONTH, Calendar.OCTOBER);
        expected.set(Calendar.DAY_OF_MONTH, 12);
        expected.set(Calendar.HOUR_OF_DAY, 10);
        expected.set(Calendar.MINUTE, 27);
        expected.set(Calendar.SECOND, 36);

        callPrivateMethod(instance, "putTimeDifference", new Class[] { Map.class, Calendar.class, Calendar.class, String.class }, new Object[] { map, old, expected, key });
        assertEquals("The putTimeDifference() method did not put the correct entry into the map", value, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putTimeDifference", new Class[] { Map.class, Calendar.class, Calendar.class, String.class }, new Object[] { map, expected, expected, key });
        assertEquals("The putTimeDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putTimeDifference", new Class[] { Map.class, Calendar.class, Calendar.class, String.class }, new Object[] { map, old, null, key });
        assertNull("The putTimeDifference() method did not put the correct entry into the map", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putTimeDifference", new Class[] { Map.class, Calendar.class, Calendar.class, String.class }, new Object[] { map, null, expected, key });
        assertEquals("The putTimeDifference() method did not put the correct entry into the map", value, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putTimeDifference", new Class[] { Map.class, Calendar.class, Calendar.class, String.class }, new Object[] { map, null, null, key });
        assertEquals("The putTimeDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutListDifference() throws Exception {
        System.out.println("putListDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = "test";
        String value = "test2,test3";
        List<String> old = new ArrayList<String>(2);
        List<String> expected = new ArrayList<String>(2);

        old.add("test1");
        old.add("test2");
        expected.add("test2");
        expected.add("test3");

        callPrivateMethod(instance, "putListDifference", new Class[] { Map.class, List.class, List.class, String.class }, new Object[] { map, old, expected, key });
        assertEquals("The putListDifference() method did not put the correct entry into the map", value, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putListDifference", new Class[] { Map.class, List.class, List.class, String.class }, new Object[] { map, expected, expected, key });
        assertEquals("The putListDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putListDifference", new Class[] { Map.class, List.class, List.class, String.class }, new Object[] { map, old, null, key });
        assertNull("The putListDifference() method did not put the correct entry into the map", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putListDifference", new Class[] { Map.class, List.class, List.class, String.class }, new Object[] { map, null, expected, key });
        assertEquals("The putListDifference() method did not put the correct entry into the map", value, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putListDifference", new Class[] { Map.class, List.class, List.class, String.class }, new Object[] { map, null, null, key });
        assertEquals("The putListDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutMapDifference() throws Exception {
        System.out.println("putMapDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = "test";
        String value = "test2=Test2,test3=Test3";
        Map<String,String> old = new HashMap<String,String>();
        Map<String,String> expected = new HashMap<String,String>();

        old.put("test1", "Test1");
        old.put("test2", "Test2");
        expected.put("test2", "Test2");
        expected.put("test3", "Test3");

        callPrivateMethod(instance, "putMapDifference", new Class[] { Map.class, Map.class, Map.class, String.class }, new Object[] { map, old, expected, key });
        assertEquals("The putMapDifference() method did not put the correct entry into the map", value, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putMapDifference", new Class[] { Map.class, Map.class, Map.class, String.class }, new Object[] { map, expected, expected, key });
        assertEquals("The putMapDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putMapDifference", new Class[] { Map.class, Map.class, Map.class, String.class }, new Object[] { map, old, null, key });
        assertNull("The putMapDifference() method did not put the correct entry into the map", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putMapDifference", new Class[] { Map.class, Map.class, Map.class, String.class }, new Object[] { map, null, expected, key });
        assertEquals("The putMapDifference() method did not put the correct entry into the map", value, map.get(key));

        map.clear();

        callPrivateMethod(instance, "putMapDifference", new Class[] { Map.class, Map.class, Map.class, String.class }, new Object[] { map, null, null, key });
        assertEquals("The putMapDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutCmdArgsDifference() throws Exception {
        System.out.println("putCmdArgsDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String args = getPrivateField(instance, "COMMAND_ARGS");
        String arg = getPrivateField(instance, "COMMAND_ARG");
        String[] old = new String[]{"test1", "test2"};
        String[] expected = new String[]{"test2", "test3", "test4"};

        callPrivateMethod(instance, "putCmdArgsDifference", new Class[] { Map.class, String[].class, String[].class }, new Object[] { map, old, expected });
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "3", map.get(args));
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "test2", map.get(arg + "0"));
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "test3", map.get(arg + "1"));
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "test4", map.get(arg + "2"));

        map.clear();
        expected = new String[]{"test5", "test6"};

        callPrivateMethod(instance, "putCmdArgsDifference", new Class[] { Map.class, String[].class, String[].class }, new Object[] { map, old, expected });
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "test5", map.get(arg + "0"));
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "test6", map.get(arg + "1"));

        map.clear();

        callPrivateMethod(instance, "putCmdArgsDifference", new Class[] { Map.class, String[].class, String[].class }, new Object[] { map, expected, expected });
        assertEquals("The putCmdArgsDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putCmdArgsDifference", new Class[] { Map.class, String[].class, String[].class }, new Object[] { map, old, null });
        assertEquals("The putCmdArgsDifference() method did not put the correct entry into the map", "0", map.get(args));

        map.clear();

        callPrivateMethod(instance, "putCmdArgsDifference", new Class[] { Map.class, String[].class, String[].class }, new Object[] { map, null, expected });
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "2", map.get(args));
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "test5", map.get(arg + "0"));
        assertEquals("The putCmdArgsDifference() method did not put the correct entries into the map", "test6", map.get(arg + "1"));

        map.clear();

        callPrivateMethod(instance, "putCmdArgsDifference", new Class[] { Map.class, String[].class, String[].class }, new Object[] { map, null, null });
        assertEquals("The putCmdArgsDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutCheckpointDifference() throws Exception {
        System.out.println("putCheckpointDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String name = getPrivateField(instance, "CHECKPOINT_NAME");
        String interval = getPrivateField(instance, "CHECKPOINT_INTERVAL");
        String occasion = getPrivateField(instance, "CHECKPOINT_OCCASION");
        CheckpointSpecifier old = new CheckpointSpecifier();
        CheckpointSpecifier expected = new CheckpointSpecifier();

        old.setName("test1");
        old.setInterval(123L);
        expected.setName("test2");
        expected.setOccasion("sm");

        callPrivateMethod(instance, "putCheckpointDifference", new Class[] { Map.class, CheckpointSpecifier.class, CheckpointSpecifier.class }, new Object[] { map, old, expected });
        assertEquals("The putCheckpointDifference() method did not put the correct entries into the map", "test2", map.get(name));
        assertEquals("The putCheckpointDifference() method did not put the correct entries into the map", "ms", map.get(occasion));

        map.clear();

        callPrivateMethod(instance, "putCheckpointDifference", new Class[] { Map.class, CheckpointSpecifier.class, CheckpointSpecifier.class }, new Object[] { map, expected, old });
        assertEquals("The putCheckpointDifference() method did not put the correct entries into the map", "test1", map.get(name));
        assertEquals("The putCheckpointDifference() method did not put the correct entries into the map", "123", map.get(interval));

        map.clear();

        callPrivateMethod(instance, "putCheckpointDifference", new Class[] { Map.class, CheckpointSpecifier.class, CheckpointSpecifier.class }, new Object[] { map, expected, expected });
        assertEquals("The putCheckpointDifference() method incorrectly put an entries into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putCheckpointDifference", new Class[] { Map.class, CheckpointSpecifier.class, CheckpointSpecifier.class }, new Object[] { map, old, null });
        assertNull("The putCheckpointDifference() method did not put the correct entries into the map", map.get(name));

        map.clear();

        callPrivateMethod(instance, "putCheckpointDifference", new Class[] { Map.class, CheckpointSpecifier.class, CheckpointSpecifier.class }, new Object[] { map, null, expected });
        assertEquals("The putCheckpointDifference() method did not put the correct entries into the map", "test2", map.get(name));
        assertEquals("The putCheckpointDifference() method did not put the correct entries into the map", "ms", map.get(occasion));

        map.clear();

        callPrivateMethod(instance, "putCheckpointDifference", new Class[] { Map.class, CheckpointSpecifier.class, CheckpointSpecifier.class }, new Object[] { map, null, null });
        assertEquals("The putCheckpointDifference() method incorrectly put an entries into the map", 0, map.size());
    }

    
    public void testPutMailDifference() throws Exception {
        System.out.println("putMailDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String mail = getPrivateField(instance, "MAIL");
        MailSpecifier old = new MailSpecifier();
        MailSpecifier expected = new MailSpecifier();

        old.setOccasion("ae");
        expected.setOccasion("bs");

        callPrivateMethod(instance, "putMailDifference", new Class[] { Map.class, MailSpecifier.class, MailSpecifier.class }, new Object[] { map, old, expected });
        assertEquals("The putMailDifference() method did not put the correct entry into the map", "bs", map.get(mail));

        map.clear();

        callPrivateMethod(instance, "putMailDifference", new Class[] { Map.class, MailSpecifier.class, MailSpecifier.class }, new Object[] { map, expected, expected });
        assertEquals("The putMailDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putMailDifference", new Class[] { Map.class, MailSpecifier.class, MailSpecifier.class }, new Object[] { map, old, null });
        assertNull("The putMailDifference() method did not put the correct entry into the map", map.get(mail));

        map.clear();

        callPrivateMethod(instance, "putMailDifference", new Class[] { Map.class, MailSpecifier.class, MailSpecifier.class }, new Object[] { map, null, expected });
        assertEquals("The putMailDifference() method did not put the correct entry into the map", "bs", map.get(mail));

        map.clear();

        callPrivateMethod(instance, "putMailDifference", new Class[] { Map.class, MailSpecifier.class, MailSpecifier.class }, new Object[] { map, null, null });
        assertEquals("The putMailDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutPeDifference() throws Exception {
        System.out.println("putPeDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String name = getPrivateField(instance, "PE_NAME");
        String min = getPrivateField(instance, "PE_MIN");
        String max = getPrivateField(instance, "PE_MAX");
        ParallelEnvironment old = new ParallelEnvironment();
        ParallelEnvironment expected = new ParallelEnvironment();

        old.setName("test1");
        old.setRange(1, 2);
        expected.setName("test2");
        expected.setRange(3);

        callPrivateMethod(instance, "putPeDifference", new Class[] { Map.class, ParallelEnvironment.class, ParallelEnvironment.class }, new Object[] { map, old, expected });
        assertEquals("The putPeDifference() method did not put the correct entries into the map", "test2", map.get(name));
        assertEquals("The putPeDifference() method did not put the correct entries into the map", "3", map.get(min));
        assertEquals("The putPeDifference() method did not put the correct entries into the map", "3", map.get(max));

        map.clear();

        callPrivateMethod(instance, "putPeDifference", new Class[] { Map.class, ParallelEnvironment.class, ParallelEnvironment.class }, new Object[] { map, expected, expected });
        assertEquals("The putPeDifference() method incorrectly put an entries into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putPeDifference", new Class[] { Map.class, ParallelEnvironment.class, ParallelEnvironment.class }, new Object[] { map, old, null });
        assertNull("The putPeDifference() method did not put the correct entries into the map", map.get(name));

        map.clear();

        callPrivateMethod(instance, "putPeDifference", new Class[] { Map.class, ParallelEnvironment.class, ParallelEnvironment.class }, new Object[] { map, null, expected });
        assertEquals("The putPeDifference() method did not put the correct entries into the map", "test2", map.get(name));
        assertEquals("The putPeDifference() method did not put the correct entries into the map", "3", map.get(min));
        assertEquals("The putPeDifference() method did not put the correct entries into the map", "3", map.get(max));

        map.clear();

        callPrivateMethod(instance, "putPeDifference", new Class[] { Map.class, ParallelEnvironment.class, ParallelEnvironment.class }, new Object[] { map, null, null });
        assertEquals("The putPeDifference() method incorrectly put an entries into the map", 0, map.size());
    }

    
    public void testPutTaskDifference() throws Exception {
        System.out.println("putTaskDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String min = getPrivateField(instance, "TASK_MIN");
        String max = getPrivateField(instance, "TASK_MAX");
        String step = getPrivateField(instance, "TASK_STEP");
        TaskSpecifier old = new TaskSpecifier();
        TaskSpecifier expected = new TaskSpecifier();

        old.setRange(1,2,3);
        expected.setRange(4, 5);

        callPrivateMethod(instance, "putTaskDifference", new Class[] { Map.class, TaskSpecifier.class, TaskSpecifier.class }, new Object[] { map, old, expected });
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "4", map.get(min));
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "5", map.get(max));
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "1", map.get(step));

        map.clear();

        callPrivateMethod(instance, "putTaskDifference", new Class[] { Map.class, TaskSpecifier.class, TaskSpecifier.class }, new Object[] { map, expected, expected });
        assertEquals("The putTaskDifference() method incorrectly put an entries into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putTaskDifference", new Class[] { Map.class, TaskSpecifier.class, TaskSpecifier.class }, new Object[] { map, old, null });
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "1", map.get(min));
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "1", map.get(max));
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "1", map.get(step));

        map.clear();

        callPrivateMethod(instance, "putTaskDifference", new Class[] { Map.class, TaskSpecifier.class, TaskSpecifier.class }, new Object[] { map, null, expected });
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "4", map.get(min));
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "5", map.get(max));
        assertEquals("The putTaskDifference() method did not put the correct entries into the map", "1", map.get(step));

        map.clear();

        callPrivateMethod(instance, "putTaskDifference", new Class[] { Map.class, TaskSpecifier.class, TaskSpecifier.class }, new Object[] { map, null, null });
        assertEquals("The putTaskDifference() method incorrectly put an entries into the map", 0, map.size());
    }

    
    public void testPutVerificationDifference() throws Exception {
        System.out.println("putVerificationDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = getPrivateField(instance, "VERIFICATION");
        Verification old = Verification.NONE;
        Verification expected = Verification.VERIFY;

        callPrivateMethod(instance, "putVerificationDifference", new Class[] { Map.class, Verification.class, Verification.class }, new Object[] { map, old, expected });
        assertEquals("The putVerificationDifference() method did not put the correct entry into the map", "v", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putVerificationDifference", new Class[] { Map.class, Verification.class, Verification.class }, new Object[] { map, expected, expected });
        assertEquals("The putVerificationDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putVerificationDifference", new Class[] { Map.class, Verification.class, Verification.class }, new Object[] { map, old, null });
        assertNull("The putVerificationDifference() method did not put the correct entry into the map", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putVerificationDifference", new Class[] { Map.class, Verification.class, Verification.class }, new Object[] { map, null, expected });
        assertEquals("The putVerificationDifference() method did not put the correct entry into the map", "v", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putVerificationDifference", new Class[] { Map.class, Verification.class, Verification.class }, new Object[] { map, null, null });
        assertEquals("The putVerificationDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testPutNameDifference() throws Exception {
        System.out.println("putNameDifference()");

        JobDescription instance = new JobDescription();
        Map<String,String> map = new HashMap<String, String>();
        String key = getPrivateField(instance, "NAME");
        String cmd = "path/to/cmd";
        String old = "old";
        String expected = "new";

        callPrivateMethod(instance, "putNameDifference", new Class[] { Map.class, String.class, String.class, String.class }, new Object[] { map, old, expected, cmd });
        assertEquals("The putNameDifference() method did not put the correct entry into the map", "new", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putNameDifference", new Class[] { Map.class, String.class, String.class, String.class }, new Object[] { map, expected, expected, cmd });
        assertEquals("The putNameDifference() method incorrectly put an entry into the map", 0, map.size());

        map.clear();

        callPrivateMethod(instance, "putNameDifference", new Class[] { Map.class, String.class, String.class, String.class }, new Object[] { map, old, null, cmd });
        assertEquals("The putNameDifference() method did not put the correct entry into the map", "cmd", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putNameDifference", new Class[] { Map.class, String.class, String.class, String.class }, new Object[] { map, old, null, null });
        assertEquals("The putNameDifference() method did not put the correct entry into the map", "NO_NAME", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putNameDifference", new Class[] { Map.class, String.class, String.class, String.class }, new Object[] { map, null, expected, cmd });
        assertEquals("The putNameDifference() method did not put the correct entry into the map", "new", map.get(key));

        map.clear();

        callPrivateMethod(instance, "putNameDifference", new Class[] { Map.class, String.class, String.class, String.class }, new Object[] { map, null, null, cmd });
        assertEquals("The putNameDifference() method incorrectly put an entry into the map", 0, map.size());
    }

    
    public void testAppendComma() throws Exception {
        System.out.println("appendComma()");

        JobDescription instance = new JobDescription();
        StringBuilder sb = new StringBuilder();

        callPrivateMethod(instance, "appendComma", new Class[]{ StringBuilder.class }, new Object[]{ sb });
        assertEquals("The appendComma() method appended a comma to an empty string", "", sb.toString());

        sb.append("a");
        callPrivateMethod(instance, "appendComma", new Class[]{ StringBuilder.class }, new Object[]{ sb });
        assertEquals("The appendComma() method appended a comma to an empty string", "a,", sb.toString());

        callPrivateMethod(instance, "appendComma", new Class[]{ StringBuilder.class }, new Object[]{ sb });
        assertEquals("The appendComma() method appended a comma to an empty string", "a,,", sb.toString());
    }

    
    public void testExtractName() throws Exception {
        System.out.println("extractName()");

        JobDescription instance = new JobDescription();
        String name = "something";
        String path = "/path/to/" + name;
        Object result = null;

        result = callPrivateMethod(instance, "extractName", new Class[]{ String.class }, new Object[]{ path });
        assertEquals("The extractName() method did not produce the correct result", name, result);

        name = "something.sh";
        path = "/path/to/" + name;

        result = callPrivateMethod(instance, "extractName", new Class[]{ String.class }, new Object[]{ path });
        assertEquals("The extractName() method did not produce the correct result", name, result);

        name = "something.sh";
        path = name;

        result = callPrivateMethod(instance, "extractName", new Class[]{ String.class }, new Object[]{ path });
        assertEquals("The extractName() method did not produce the correct result", name, result);

        name = "this\tis\ta\ttabbed\tname";
        path = "/path/to/" + name;

        result = callPrivateMethod(instance, "extractName", new Class[]{ String.class }, new Object[]{ path });
        assertEquals("The extractName() method did not produce the correct result", name.replace('\t', '_'), result);

        name = "name\\with\\back\\slashes";
        path = "/path/to/" + name;

        result = callPrivateMethod(instance, "extractName", new Class[]{ String.class }, new Object[]{ path });
        assertEquals("The extractName() method did not produce the correct result", name.replace('\\', '_'), result);

        path = "/path/to/";

        result = callPrivateMethod(instance, "extractName", new Class[]{ String.class }, new Object[]{ path });
        assertEquals("The extractName() method did not produce the correct result", "NO_NAME", result);

        result = callPrivateMethod(instance, "extractName", new Class[]{ String.class }, new Object[]{ null });
        assertEquals("The extractName() method did not produce the correct result", "NO_NAME", result);

        result = callPrivateMethod(instance, "extractName", new Class[]{ String.class }, new Object[]{ "" });
        assertEquals("The extractName() method did not produce the correct result", "NO_NAME", result);
    }

    
    public void testVerificationToString() throws Exception {
        System.out.println("verificationToString()");

        JobDescription instance = new JobDescription();

        Object result = callPrivateMethod(instance, "verificationToString", new Class[]{ Verification.class }, new Object[]{ Verification.ERROR });

        assertEquals("The verificationToString() method returned the wrong string", "e", result);

        result = callPrivateMethod(instance, "verificationToString", new Class[]{ Verification.class }, new Object[]{ Verification.NONE });
        assertEquals("The verificationToString() method returned the wrong string", "n", result);

        result = callPrivateMethod(instance, "verificationToString", new Class[]{ Verification.class }, new Object[]{ Verification.POKE });
        assertEquals("The verificationToString() method returned the wrong string", "p", result);

        result = callPrivateMethod(instance, "verificationToString", new Class[]{ Verification.class }, new Object[]{ Verification.VERIFY });
        assertEquals("The verificationToString() method returned the wrong string", "v", result);

        result = callPrivateMethod(instance, "verificationToString", new Class[]{ Verification.class }, new Object[]{ Verification.WARNING });
        assertEquals("The verificationToString() method returned the wrong string", "w", result);

        result = callPrivateMethod(instance, "verificationToString", new Class[]{ Verification.class }, new Object[]{ null });
        assertNull("The verificationToString() method returned the wrong string", result);
    }

    /**
     * Test of parseCmdArgs method, of class JobDescription.
     */
    
    public void testParseCmdArgs() throws Exception {
        System.out.println("parseCmdArgs()");

        JobDescription instance = new JobDescription();
        String value = "3";
        String[] expResult = new String[3];

        String[] result = (String[])callPrivateMethod(instance, "parseCmdArgs", new Class[]{ String.class }, new Object[]{ value });

        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult[0], result[0]);
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult[1], result[1]);
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult[2], result[2]);

        setPrivateField(instance, "commandArgs", new String[]{ "test1", "test2", "test3" });
        value = "1";
        expResult = new String[]{ "test1" };
        result = (String[])callPrivateMethod(instance, "parseCmdArgs", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult[0], result[0]);

        setPrivateField(instance, "commandArgs", new String[]{ "test1" });
        value = "2";
        expResult = new String[]{ "test1", null };
        result = (String[])callPrivateMethod(instance, "parseCmdArgs", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult[0], result[0]);
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult[1], result[1]);

        setPrivateField(instance, "commandArgs", null);
        value = null;
        result = (String[])callPrivateMethod(instance, "parseCmdArgs", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseCmdArgs() method did not parse the value correctly", result);

        setPrivateField(instance, "commandArgs", new String[]{ "test1", null });
        value = null;
        result = (String[])callPrivateMethod(instance, "parseCmdArgs", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult[0], result[0]);
        assertEquals("The parseCmdArgs() method did not parse the value correctly", expResult[1], result[1]);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "invalid";
        result = (String[])callPrivateMethod(instance, "parseCmdArgs", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseCmdArgs() method changed the command args on an invalid value", expResult.length, result.length);
        assertEquals("The parseCmdArgs() method changed the command args on an invalid value", expResult[0], result[0]);
        assertEquals("The parseCmdArgs() method changed the command args on an invalid value", expResult[1], result[1]);
        assertEquals("The parseCmdArgs() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseCmdArgs() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseCmdArg method, of class JobDescription.
     */
    
    public void testParseCmdArg() throws Exception {
        System.out.println("parseCmdArg()");

        JobDescription instance = new JobDescription();
        String name = getPrivateField(instance, "COMMAND_ARG");
        String parameter = name + "0";
        String value = "test0";
        String[] expResult = new String[]{ "test0" };

        String[] result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });

        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[0], result[0]);

        setPrivateField(instance, "commandArgs", new String[]{ "test0" });
        parameter = name + "2";
        value = "test2";
        expResult = new String[]{ "test0", null, "test2" };
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[0], result[0]);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[1], result[1]);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[2], result[2]);

        setPrivateField(instance, "commandArgs", new String[]{ "test0", null, "test2" });
        parameter = name + "1";
        value = "test1";
        expResult = new String[]{ "test0", "test1", "test2" };
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[0], result[0]);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[1], result[1]);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[2], result[2]);

        setPrivateField(instance, "commandArgs", new String[]{ "test0", "test1", "test2" });
        parameter = name + "0";
        value = null;
        expResult = new String[]{ "test1", "test2" };
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[0], result[0]);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[1], result[1]);

        setPrivateField(instance, "commandArgs", new String[]{ "test0", "test1", "test2" });
        parameter = name + "1";
        value = null;
        expResult = new String[]{ "test0", "test2" };
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[0], result[0]);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[1], result[1]);

        setPrivateField(instance, "commandArgs", new String[]{ "test0", "test1", "test2" });
        parameter = name + "2";
        value = null;
        expResult = new String[]{ "test0", "test1" };
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult.length, result.length);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[0], result[0]);
        assertEquals("The parseCmdArg() method did not parse the value correctly", expResult[1], result[1]);

        setPrivateField(instance, "commandArgs", null);
        parameter = name + "2";
        value = null;
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertNull("The parseCmdArg() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        setPrivateField(instance, "commandArgs", new String[]{ "test0", "test1" });
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        parameter = "invalid";
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult.length, result.length);
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult[0], result[0]);
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult[1], result[1]);
        assertEquals("The parseCmdArg() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseCmdArg() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        parameter = name + "-1";
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult.length, result.length);
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult[0], result[0]);
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult[1], result[1]);
        assertEquals("The parseCmdArg() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseCmdArg() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        parameter = null;
        result = (String[])callPrivateMethod(instance, "parseCmdArg", new Class[]{ String.class, String.class }, new Object[]{ parameter, value });
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult.length, result.length);
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult[0], result[0]);
        assertEquals("The parseCmdArg() method changed the command args on an invalid value", expResult[1], result[1]);
        assertEquals("The parseCmdArg() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseCmdArg() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseBoolean method, of class JobDescription.
     */
    
    public void testParseBoolean() throws Exception {
        System.out.println("parseBoolean()");

        JobDescription instance = new JobDescription();
        String value = "y";
        Boolean expResult = true;

        Object result = callPrivateMethod(instance, "parseBoolean", new Class[]{ String.class }, new Object[]{ value });

        assertEquals("The parseBoolean() method did not parse the value correctly", expResult, result);

        value = "n";
        expResult = false;
        result = callPrivateMethod(instance, "parseBoolean", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseBoolean() method did not parse the value correctly", expResult, result);

        value = null;
        expResult = null;
        result = callPrivateMethod(instance, "parseBoolean", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseBoolean() method did not parse the value correctly", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        setPrivateField(instance, "commandArgs", new String[]{ "test0", "test1" });
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        expResult = null;
        result = callPrivateMethod(instance, "parseBoolean", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseBoolean() method did not parse the value correctly", expResult, result);
        assertEquals("The parseBoolean() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseBoolean() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        expResult = null;
        result = callPrivateMethod(instance, "parseBoolean", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseBoolean() method did not parse the value correctly", expResult, result);
        assertEquals("The parseBoolean() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseBoolean() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseInt method, of class JobDescription.
     */
    
    public void testParseInt() throws Exception {
        System.out.println("parseInt()");

        JobDescription instance = new JobDescription();
        String value = "1";
        Integer expResult = 1;

        Object result = callPrivateMethod(instance, "parseInt", new Class[]{ String.class }, new Object[]{ value });

        assertEquals("The parseInt() method did not parse the value correctly", expResult, result);

        value = "-1";
        expResult = -1;
        result = callPrivateMethod(instance, "parseInt", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseInt() method did not parse the value correctly", expResult, result);

        value = null;
        expResult = null;
        result = callPrivateMethod(instance, "parseInt", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseInt() method did not parse the value correctly", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        setPrivateField(instance, "commandArgs", new String[]{ "test0", "test1" });
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        expResult = null;
        result = callPrivateMethod(instance, "parseInt", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseInt() method did not parse the value correctly", expResult, result);
        assertEquals("The parseInt() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseInt() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        expResult = null;
        result = callPrivateMethod(instance, "parseInt", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseInt() method did not parse the value correctly", expResult, result);
        assertEquals("The parseInt() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseInt() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseCheckpointName method, of class JobDescription.
     */
    
    public void testParseCheckpointName() throws Exception {
        System.out.println("parseCheckpointName()");

        JobDescription instance = new JobDescription();
        String value = "test";
        Object result = callPrivateMethod(instance, "parseCheckpointName", new Class[]{ String.class }, new Object[]{ value });
        CheckpointSpecifier expResult = new CheckpointSpecifier();

        expResult.setName(value);
        assertEquals("The parseCheckpointName() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        value = "test2";
        result = callPrivateMethod(instance, "parseCheckpointName", new Class[]{ String.class }, new Object[]{ value });
        expResult.setName(value);
        assertEquals("The parseCheckpointName() method did not parse the value correctly", expResult, result);

        value = "";
        result = callPrivateMethod(instance, "parseCheckpointName", new Class[]{ String.class }, new Object[]{ value });
        expResult.setName(value);
        assertEquals("The parseCheckpointName() method did not parse the value correctly", expResult, result);

        expResult.setName("test");
        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parseCheckpointName", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseCheckpointName() method did not parse the value correctly", result);

        setPrivateField(instance, "checkpointSpecifier", null);
        value = null;
        result = callPrivateMethod(instance, "parseCheckpointName", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseCheckpointName() method did not parse the value correctly", result);
    }

    /**
     * Test of parseCheckpointOccasion method, of class JobDescription.
     */
    
    public void testParseCheckpointOccasion() throws Exception {
        System.out.println("parseCheckpointOccasion()");

        JobDescription instance = new JobDescription();
        String value = CheckpointSpecifier.ON_MIN_CPU_INTERVAL_STR;
        Object result = callPrivateMethod(instance, "parseCheckpointOccasion", new Class[]{ String.class }, new Object[]{ value });
        CheckpointSpecifier expResult = new CheckpointSpecifier();

        expResult.setOccasion(CheckpointSpecifier.ON_MIN_CPU_INTERVAL);
        assertEquals("The parseCheckpointOccasion() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        value = CheckpointSpecifier.NEVER_STR;
        result = callPrivateMethod(instance, "parseCheckpointOccasion", new Class[]{ String.class }, new Object[]{ value });
        expResult.setOccasion(0x0);
        assertEquals("The parseCheckpointOccasion() method did not parse the value correctly", expResult, result);

        expResult.setOccasion(CheckpointSpecifier.ON_MIN_CPU_INTERVAL);
        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        value = "";
        result = callPrivateMethod(instance, "parseCheckpointOccasion", new Class[]{ String.class }, new Object[]{ value });
        expResult.setOccasion(0x0);
        assertEquals("The parseCheckpointOccasion() method did not parse the value correctly", expResult, result);

        expResult.setOccasion(CheckpointSpecifier.ON_MIN_CPU_INTERVAL);
        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parseCheckpointOccasion", new Class[]{ String.class }, new Object[]{ value });
        expResult.setOccasion(0x0);
        assertEquals("The parseCheckpointOccasion() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "checkpointSpecifier", null);
        value = null;
        result = callPrivateMethod(instance, "parseCheckpointOccasion", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseCheckpointOccasion() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        expResult.setOccasion(CheckpointSpecifier.ON_MIN_CPU_INTERVAL);
        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parseCheckpointOccasion", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseCheckpointOccasion() method did not parse the value correctly", expResult, result);
        assertEquals("The parseCheckpointOccasion() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseCheckpointOccasion() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseCheckpointInterval method, of class JobDescription.
     */
    
    public void testParseCheckpointInterval() throws Exception {
        System.out.println("parseCheckpointInterval()");

        JobDescription instance = new JobDescription();
        String value = "1";
        Object result = callPrivateMethod(instance, "parseCheckpointInterval", new Class[]{ String.class }, new Object[]{ value });
        CheckpointSpecifier expResult = new CheckpointSpecifier();

        expResult.setInterval(1L);
        assertEquals("The parseCheckpointInterval() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        value = "10";
        result = callPrivateMethod(instance, "parseCheckpointInterval", new Class[]{ String.class }, new Object[]{ value });
        expResult.setInterval(10L);
        assertEquals("The parseCheckpointInterval() method did not parse the value correctly", expResult, result);

        value = "0";
        result = callPrivateMethod(instance, "parseCheckpointInterval", new Class[]{ String.class }, new Object[]{ value });
        expResult.setInterval(0L);
        assertEquals("The parseCheckpointInterval() method did not parse the value correctly", expResult, result);

        expResult.setInterval(10L);
        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parseCheckpointInterval", new Class[]{ String.class }, new Object[]{ value });
        expResult.setInterval(0L);
        assertEquals("The parseCheckpointInterval() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "checkpointSpecifier", null);
        value = null;
        result = callPrivateMethod(instance, "parseCheckpointInterval", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseCheckpointInterval() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        expResult.setInterval(10L);
        setPrivateField(instance, "checkpointSpecifier", expResult.clone());
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parseCheckpointInterval", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseCheckpointInterval() method did not parse the value correctly", expResult, result);
        assertEquals("The parseCheckpointInterval() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseCheckpointInterval() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        result = callPrivateMethod(instance, "parseCheckpointInterval", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseCheckpointInterval() method did not parse the value correctly", expResult, result);
        assertEquals("The parseCheckpointInterval() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseCheckpointInterval() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseHold method, of class JobDescription.
     */
    
    public void testParseHold() throws Exception {
        System.out.println("parseHold()");

        JobDescription instance = new JobDescription();
        String value = "u";
        Boolean expResult = true;

        Object result = callPrivateMethod(instance, "parseHold", new Class[]{ String.class }, new Object[]{ value });

        assertEquals("The parseHold() method did not parse the value correctly", expResult, result);

        value = "n";
        expResult = false;
        result = callPrivateMethod(instance, "parseHold", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseHold() method did not parse the value correctly", expResult, result);

        value = null;
        expResult = null;
        result = callPrivateMethod(instance, "parseHold", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseHold() method did not parse the value correctly", expResult, result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        expResult = null;
        result = callPrivateMethod(instance, "parseHold", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseHold() method did not parse the value correctly", expResult, result);
        assertEquals("The parseHold() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseHold() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        expResult = null;
        result = callPrivateMethod(instance, "parseHold", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseHold() method did not parse the value correctly", expResult, result);
        assertEquals("The parseHold() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseHold() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseMail method, of class JobDescription.
     */
    
    public void testParseMail() throws Exception {
        System.out.println("parseMail()");

        JobDescription instance = new JobDescription();
        String value = MailSpecifier.ON_BEGIN_STR;
        Object result = callPrivateMethod(instance, "parseMail", new Class[]{ String.class }, new Object[]{ value });
        MailSpecifier expResult = new MailSpecifier();

        expResult.setOccasion(MailSpecifier.ON_BEGIN);
        assertEquals("The parseMail() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "mailSpecifier", expResult.clone());
        value = MailSpecifier.NEVER_STR;
        result = callPrivateMethod(instance, "parseMail", new Class[]{ String.class }, new Object[]{ value });
        expResult.setOccasion(0x0);
        assertEquals("The parseMail() method did not parse the value correctly", expResult, result);

        expResult.setOccasion(MailSpecifier.ON_END);
        setPrivateField(instance, "mailSpecifier", expResult.clone());
        value = "";
        result = callPrivateMethod(instance, "parseMail", new Class[]{ String.class }, new Object[]{ value });
        expResult.setOccasion(0x0);
        assertEquals("The parseMail() method did not parse the value correctly", expResult, result);

        expResult.setOccasion(MailSpecifier.ON_END);
        setPrivateField(instance, "mailSpecifier", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parseMail", new Class[]{ String.class }, new Object[]{ value });
        expResult.setOccasion(0x0);
        assertEquals("The parseMail() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "mailSpecifier", null);
        value = null;
        result = callPrivateMethod(instance, "parseMail", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseMail() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        expResult.setOccasion(MailSpecifier.ON_END);
        setPrivateField(instance, "mailSpecifier", expResult.clone());
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parseMail", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseMail() method did not parse the value correctly", expResult, result);
        assertEquals("The parseMail() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseMail() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseTaskMin method, of class JobDescription.
     */
    
    public void testParseTaskMin() throws Exception {
        System.out.println("parseTaskMin()");

        JobDescription instance = new JobDescription();
        String value = "1";
        Object result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        TaskSpecifier expResult = new TaskSpecifier();

        expResult.setMin(1);
        assertEquals("The parseTaskMin() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = "10";
        result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        expResult.setMin(10);
        assertEquals("The parseTaskMin() method did not parse the value correctly", expResult, result);

        expResult.setMin(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = "0";
        result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMin() method did not parse the value correctly", result);

        setPrivateField(instance, "taskSpecifier", null);
        value = "0";
        result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMin() method did not parse the value correctly", result);

        expResult.setMin(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMin() method did not parse the value correctly", result);

        setPrivateField(instance, "taskSpecifier", null);
        value = null;
        result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMin() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        expResult.setMin(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseTaskMin() method did not parse the value correctly", expResult, result);
        assertEquals("The parseTaskMin() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskMin() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseTaskMin() method did not parse the value correctly", expResult, result);
        assertEquals("The parseTaskMin() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskMin() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        setPrivateField(instance, "taskSpecifier", null);
        value = "";
        result = callPrivateMethod(instance, "parseTaskMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMin() method did not parse the value correctly", result);
        assertEquals("The parseTaskMin() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskMin() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseTaskMax method, of class JobDescription.
     */
    
    public void testParseTaskMax() throws Exception {
        System.out.println("parseTaskMax()");

        JobDescription instance = new JobDescription();
        String value = "1";
        Object result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        TaskSpecifier expResult = new TaskSpecifier();

        expResult.setMax(1);
        assertEquals("The parseTaskMax() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = "10";
        result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        expResult.setMax(10);
        assertEquals("The parseTaskMax() method did not parse the value correctly", expResult, result);

        expResult.setMax(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = "0";
        result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMax() method did not parse the value correctly", result);

        setPrivateField(instance, "taskSpecifier", null);
        value = "0";
        result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMax() method did not parse the value correctly", result);

        expResult.setMax(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMax() method did not parse the value correctly", result);

        setPrivateField(instance, "taskSpecifier", null);
        value = null;
        result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMax() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        expResult.setMax(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseTaskMax() method did not parse the value correctly", expResult, result);
        assertEquals("The parseTaskMax() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskMax() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseTaskMax() method did not parse the value correctly", expResult, result);
        assertEquals("The parseTaskMax() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskMax() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        setPrivateField(instance, "taskSpecifier", null);
        value = "";
        result = callPrivateMethod(instance, "parseTaskMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskMax() method did not parse the value correctly", result);
        assertEquals("The parseTaskMax() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskMax() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseTaskStep method, of class JobDescription.
     */
    
    public void testParseTaskStep() throws Exception {
        System.out.println("parseTaskStep()");

        JobDescription instance = new JobDescription();
        String value = "1";
        Object result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        TaskSpecifier expResult = new TaskSpecifier();

        expResult.setStep(1);
        assertEquals("The parseTaskStep() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = "10";
        result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        expResult.setStep(10);
        assertEquals("The parseTaskStep() method did not parse the value correctly", expResult, result);

        expResult.setStep(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = "0";
        result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskStep() method did not parse the value correctly", result);

        setPrivateField(instance, "taskSpecifier", null);
        value = "0";
        result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskStep() method did not parse the value correctly", result);

        expResult.setStep(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskStep() method did not parse the value correctly", result);

        setPrivateField(instance, "taskSpecifier", null);
        value = null;
        result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskStep() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        expResult.setStep(10);
        setPrivateField(instance, "taskSpecifier", expResult.clone());
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseTaskStep() method did not parse the value correctly", expResult, result);
        assertEquals("The parseTaskStep() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskStep() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parseTaskStep() method did not parse the value correctly", expResult, result);
        assertEquals("The parseTaskStep() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskStep() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        setPrivateField(instance, "taskSpecifier", null);
        value = "";
        result = callPrivateMethod(instance, "parseTaskStep", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseTaskStep() method did not parse the value correctly", result);
        assertEquals("The parseTaskStep() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseTaskStep() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parsePeName method, of class JobDescription.
     */
    
    public void testParsePeName() throws Exception {
        System.out.println("parsePeName()");

        JobDescription instance = new JobDescription();
        String value = "test";
        Object result = callPrivateMethod(instance, "parsePeName", new Class[]{ String.class }, new Object[]{ value });
        ParallelEnvironment expResult = new ParallelEnvironment();

        expResult.setName(value);
        assertEquals("The parsePeName() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "pe", expResult.clone());
        value = "test2";
        result = callPrivateMethod(instance, "parsePeName", new Class[]{ String.class }, new Object[]{ value });
        expResult.setName(value);
        assertEquals("The parsePeName() method did not parse the value correctly", expResult, result);

        value = "";
        result = callPrivateMethod(instance, "parsePeName", new Class[]{ String.class }, new Object[]{ value });
        expResult.setName(value);
        assertEquals("The parsePeName() method did not parse the value correctly", expResult, result);

        expResult.setName("test");
        setPrivateField(instance, "pe", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parsePeName", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeName() method did not parse the value correctly", result);

        setPrivateField(instance, "pe", null);
        value = null;
        result = callPrivateMethod(instance, "parsePeName", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeName() method did not parse the value correctly", result);
    }

    /**
     * Test of parsePeMin method, of class JobDescription.
     */
    
    public void testParsePeMin() throws Exception {
        System.out.println("parsePeMin()");

        JobDescription instance = new JobDescription();
        String value = "1";
        Object result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        ParallelEnvironment expResult = new ParallelEnvironment();

        expResult.setRangeMin(1);
        assertEquals("The parsePeMin() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "pe", expResult.clone());
        value = "10";
        result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        expResult.setRangeMin(10);
        assertEquals("The parsePeMin() method did not parse the value correctly", expResult, result);

        expResult.setRangeMin(10);
        setPrivateField(instance, "pe", expResult.clone());
        value = "0";
        result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMin() method did not parse the value correctly", result);

        setPrivateField(instance, "pe", null);
        value = "0";
        result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMin() method did not parse the value correctly", result);

        expResult.setRangeMin(10);
        setPrivateField(instance, "pe", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMin() method did not parse the value correctly", result);

        setPrivateField(instance, "pe", null);
        value = null;
        result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMin() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        expResult.setRangeMin(10);
        setPrivateField(instance, "pe", expResult.clone());
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parsePeMin() method did not parse the value correctly", expResult, result);
        assertEquals("The parsePeMin() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parsePeMin() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parsePeMin() method did not parse the value correctly", expResult, result);
        assertEquals("The parsePeMin() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parsePeMin() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        setPrivateField(instance, "pe", null);
        value = "";
        result = callPrivateMethod(instance, "parsePeMin", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMin() method did not parse the value correctly", result);
        assertEquals("The parsePeMin() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parsePeMin() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parsePeMax method, of class JobDescription.
     */
    
    public void testParsePeMax() throws Exception {
        System.out.println("parsePeMax()");

        JobDescription instance = new JobDescription();
        String value = "1";
        Object result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        ParallelEnvironment expResult = new ParallelEnvironment();

        expResult.setRangeMax(1);
        assertEquals("The parsePeMax() method did not parse the value correctly", expResult, result);

        setPrivateField(instance, "pe", expResult.clone());
        value = "10";
        result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        expResult.setRangeMax(10);
        assertEquals("The parsePeMax() method did not parse the value correctly", expResult, result);

        expResult.setRangeMax(10);
        setPrivateField(instance, "pe", expResult.clone());
        value = "0";
        result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMax() method did not parse the value correctly", result);

        setPrivateField(instance, "pe", null);
        value = "0";
        result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMax() method did not parse the value correctly", result);

        expResult.setRangeMax(10);
        setPrivateField(instance, "pe", expResult.clone());
        value = null;
        result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMax() method did not parse the value correctly", result);

        setPrivateField(instance, "pe", null);
        value = null;
        result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMax() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        expResult.setRangeMax(10);
        setPrivateField(instance, "pe", expResult.clone());
        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parsePeMax() method did not parse the value correctly", expResult, result);
        assertEquals("The parsePeMax() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parsePeMax() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        assertEquals("The parsePeMax() method did not parse the value correctly", expResult, result);
        assertEquals("The parsePeMax() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parsePeMax() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        setPrivateField(instance, "pe", null);
        value = "";
        result = callPrivateMethod(instance, "parsePeMax", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parsePeMax() method did not parse the value correctly", result);
        assertEquals("The parsePeMax() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parsePeMax() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
    }

    /**
     * Test of parseVerification method, of class JobDescription.
     */
    
    public void testParseVerification() throws Exception {
        System.out.println("parseVerification()");

        JobDescription instance = new JobDescription();
        String value = "e";
        Object result = callPrivateMethod(instance, "parseVerification", new Class[]{ String.class }, new Object[]{ value });
        Verification expResult = Verification.ERROR;

        assertEquals("The parseVerification() method did not parse the value correctly", expResult, result);

        value = null;
        result = callPrivateMethod(instance, "parseVerification", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseVerification() method did not parse the value correctly", result);

        Logger log = Logger.getLogger("com.sun.grid.Jsv");
        TestHandler handler = new TestHandler();

        log.setLevel(Level.ALL);
        log.addHandler(handler);
        handler.setLevel(Level.ALL);
        value = "xyz";
        result = callPrivateMethod(instance, "parseVerification", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseVerification() method did not parse the value correctly", result);
        assertEquals("The parseVerification() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseVerification() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "";
        result = callPrivateMethod(instance, "parseVerification", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseVerification() method did not parse the value correctly", result);
        assertEquals("The parseVerification() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseVerification() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());

        handler.messages.clear();
        value = "evw";
        result = callPrivateMethod(instance, "parseVerification", new Class[]{ String.class }, new Object[]{ value });
        assertNull("The parseVerification() method did not parse the value correctly", result);
        assertEquals("The parseVerification() method did log an appropriate error message", 1, handler.messages.size());
        assertEquals("The parseVerification() method did log an appropriate error message", Level.WARNING, handler.messages.get(0).getLevel());
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
