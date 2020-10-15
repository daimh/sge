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
 * SimpleJobTemplateTest.java
 * JUnit based test
 *
 * Created on November 13, 2004, 5:28 PM
 */

package org.ggf.drmaa;

import java.util.*;
import junit.framework.*;

/**
 *
 * @author dan.templeton@sun.com
 */
public class SimpleJobTemplateTest extends TestCase {
    private SimpleJobTemplate jt = null;

    public SimpleJobTemplateTest(java.lang.String testName) {
        super(testName);
    }

    public static Test suite() {
        TestSuite suite = new TestSuite(SimpleJobTemplateTest.class);
        return suite;
    }

    protected void setUp() {
        jt = new SimpleJobTemplate();
    }

    protected void tearDown() {
        jt = null;
    }

    /** Test of g|setRemoteCommand method, of class org.ggf.drmaa.JobTemplate. */
    public void testRemoteCommand() throws DrmaaException {
        System.out.println("testRemoteCommand");

        jt.setRemoteCommand("MyRemoteCommand");
        assertEquals("MyRemoteCommand", jt.getRemoteCommand());
    }

    /** Test of g|setArgs method, of class org.ggf.drmaa.JobTemplate. */
    public void testArgs() throws DrmaaException {
        System.out.println("testArgs");

        List args = Arrays.asList(new String[] {"arg1", "arg2", "arg3"});

        jt.setArgs(args);

        List retArgs = jt.getArgs();

        assertNotSame(args, retArgs);

        for (int count = 0; count < Math.min(args.size(), retArgs.size()); count++) {
            assertEquals(args.get(count), retArgs.get(count));
        }
    }

    /** Test of g|setJobSubmissionState method, of class org.ggf.drmaa.JobTemplate. */
    public void testJobSubmissionState() throws DrmaaException {
        System.out.println("testJobSubmissionState");

        jt.setJobSubmissionState(jt.HOLD_STATE);
        assertEquals(jt.HOLD_STATE, jt.getJobSubmissionState());
        jt.setJobSubmissionState(jt.ACTIVE_STATE);
        assertEquals(jt.ACTIVE_STATE, jt.getJobSubmissionState());
    }

    /** Test of g|setJobEnvironment method, of class org.ggf.drmaa.JobTemplate. */
    public void testJobEnvironment() throws DrmaaException {
        System.out.println("testJobEnvironment");

        Map env = new HashMap();
        env.put("PATH", "/usr/bin");
        env.put("LD_LIBRARY_PATH", "/usr/lib");

        jt.setJobEnvironment(env);

        Map retEnv = jt.getJobEnvironment();

        assertNotSame(env, retEnv);
        assertEquals(env, retEnv);
    }

    /** Test of g|setWorkingDirectory method, of class org.ggf.drmaa.JobTemplate. */
    public void testWorkingDirectory() throws DrmaaException {
        System.out.println("testWorkingDirectory");

        jt.setWorkingDirectory("/home/me");
        assertEquals("/home/me", jt.getWorkingDirectory());
    }

    /** Test of g|setJobCategory method, of class org.ggf.drmaa.JobTemplate. */
    public void testJobCategory() throws DrmaaException {
        System.out.println("testJobCategory");

        jt.setJobCategory("mycat");
        assertEquals("mycat", jt.getJobCategory());
    }

    /** Test of g|setNativeSpecification method, of class org.ggf.drmaa.JobTemplate. */
    public void testNativeSpecification() throws DrmaaException {
        System.out.println("testNativeSpecification");

        jt.setNativeSpecification("-shell yes");
        assertEquals("-shell yes", jt.getNativeSpecification());
    }

    /** Test of g|setEmail method, of class org.ggf.drmaa.JobTemplate. */
    public void testEmail() throws DrmaaException {
        System.out.println("testEmail");

        Set email = new HashSet(Arrays.asList(new String[] {"dant@germany", "admin"}));

        jt.setEmail(email);

        Set retEmail = jt.getEmail();

        assertNotSame(email, retEmail);
        assertEquals(email, retEmail);
    }

    /** Test of g|setBlockEmail method, of class org.ggf.drmaa.JobTemplate. */
    public void testBlockEmail() throws DrmaaException {
        System.out.println("testBlockEmail");

        jt.setBlockEmail(true);
        assertTrue(jt.getBlockEmail());
        jt.setBlockEmail(false);
        assertFalse(jt.getBlockEmail());
    }

    /** Test of g|setStartTime method, of class org.ggf.drmaa.JobTemplate. */
    public void testStartTime() throws DrmaaException {
        System.out.println("testStartTime");

        PartialTimestamp pt = new PartialTimestamp();
        Calendar cal = Calendar.getInstance();

        pt.set(pt.HOUR_OF_DAY, cal.get(cal.HOUR_OF_DAY) + 1);
        pt.set(pt.MINUTE, 0);

        jt.setStartTime(pt);

        PartialTimestamp retPt = jt.getStartTime();

        assertNotSame(pt, retPt);
        assertEquals(pt, retPt);

        pt.set(pt.HOUR_OF_DAY, cal.get(cal.HOUR_OF_DAY) - 1);

        jt.setStartTime(pt);

        retPt = jt.getStartTime();

        assertNotSame(pt, retPt);
        assertEquals(pt, retPt);

        pt.set(pt.CENTURY, 19);

        try {
            jt.setStartTime(pt);
            fail("Allowed start time in the past");
        } catch (IllegalArgumentException e) {
            /* Don't care. */
        }
    }

    /** Test of g|setJobName method, of class org.ggf.drmaa.JobTemplate. */
    public void testJobName() throws DrmaaException {
        System.out.println("testJobName");

        jt.setJobName("MyJob");
        assertEquals("MyJob", jt.getJobName());
    }

    /** Test of g|setInputPath method, of class org.ggf.drmaa.JobTemplate. */
    public void testInputPath() throws DrmaaException {
        System.out.println("testInputPath");

        jt.setInputPath("/tmp");
        assertEquals("/tmp", jt.getInputPath());
    }

    /** Test of g|setOutputPath method, of class org.ggf.drmaa.JobTemplate. */
    public void testOutputPath() throws DrmaaException {
        System.out.println("testOutputPath");

        jt.setOutputPath("/tmp");
        assertEquals("/tmp", jt.getOutputPath());
    }

    /** Test of g|setErrorPath method, of class org.ggf.drmaa.JobTemplate. */
    public void testErrorPath() throws DrmaaException {
        System.out.println("testErrorPath");

        jt.setErrorPath("/tmp");
        assertEquals("/tmp", jt.getErrorPath());
    }

    /** Test of g|setJoinFiles method, of class org.ggf.drmaa.JobTemplate. */
    public void testJoinFiles() throws DrmaaException {
        System.out.println("testJoinFiles");

        jt.setJoinFiles(true);
        assertTrue(jt.getJoinFiles());
        jt.setJoinFiles(false);
        assertFalse(jt.getJoinFiles());
    }

    /** Test of setTransferFiles method, of class org.ggf.drmaa.JobTemplate. */
    public void testTransferFiles() throws DrmaaException {
        System.out.println("testTransferFiles");

        FileTransferMode mode = new FileTransferMode(true, false, true);

        try {
            new SimpleJobTemplate().setTransferFiles(mode);
            fail("Allowed setting of unsupported transferFiles property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }

        try {
            new SimpleJobTemplate().getTransferFiles();
            fail("Allowed getting of unsupported transferFiles property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }
    }

    /** Test of g|setDeadlineTime method, of class org.ggf.drmaa.JobTemplate. */
    public void testDeadlineTime() throws DrmaaException {
        System.out.println("testDeadlineTime");

        PartialTimestamp pt = new PartialTimestamp();

        try {
            new SimpleJobTemplate().setDeadlineTime(pt);
            fail("Allowed setting of unsupported deadlineTime property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }

        try {
            new SimpleJobTemplate().getDeadlineTime();
            fail("Allowed getting of unsupported deadlineTime property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }
    }

    /** Test of g|setHardWallclockTimeLimit method, of class org.ggf.drmaa.JobTemplate. */
    public void testHardWallclockTimeLimit() throws DrmaaException {
        System.out.println("testHardWallclockTimeLimit");

        try {
            new SimpleJobTemplate().setHardWallclockTimeLimit(101L);
            fail("Allowed setting of unsupported hardWallclockTimeLimit property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }

        try {
            new SimpleJobTemplate().getHardWallclockTimeLimit();
            fail("Allowed getting of unsupported hardWallclockTimeLimit property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }
    }

    /** Test of g|setSoftWallclockTimeLimit method, of class org.ggf.drmaa.JobTemplate. */
    public void testSoftWallclockTimeLimit() throws DrmaaException {
        System.out.println("testSoftWallclockTimeLimit");

        try {
            new SimpleJobTemplate().setSoftWallclockTimeLimit(101L);
            fail("Allowed setting of unsupported softWallclockTimeLimit property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }

        try {
            new SimpleJobTemplate().getSoftWallclockTimeLimit();
            fail("Allowed getting of unsupported softWallclockTimeLimit property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }
    }

    /** Test of g|setHardRunDurationLimit method, of class org.ggf.drmaa.JobTemplate. */
    public void testHardRunDurationLimit() throws DrmaaException {
        System.out.println("testHardRunDurationLimit");

        try {
            new SimpleJobTemplate().setHardRunDurationLimit(101L);
            fail("Allowed setting of unsupported hardRunDurationLimit property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }

        try {
            new SimpleJobTemplate().getHardRunDurationLimit();
            fail("Allowed getting of unsupported hardRunDurationLimit property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }
    }

    /** Test of g|setSoftRunDurationLimit method, of class org.ggf.drmaa.JobTemplate. */
    public void testSoftRunDurationLimit() throws DrmaaException {
        System.out.println("testSoftRunDurationLimit");

        try {
            new SimpleJobTemplate().setSoftRunDurationLimit(101L);
            fail("Allowed setting of unsupported softRunDurationLimit property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }

        try {
            new SimpleJobTemplate().getSoftRunDurationLimit();
            fail("Allowed getting of unsupported softRunDurationLimit property");
        } catch (UnsupportedAttributeException e) {
            /* Don't care. */
        }
    }

    /** Test of getAttributeNames method, of class org.ggf.drmaa.JobTemplate. */
    public void testGetAttributeNames() throws DrmaaException {
        System.out.println("testGetAttributeNames");

        ArrayList names = new ArrayList(21);

        names.add("args");
        names.add("blockEmail");
        names.add("email");
        names.add("errorPath");
        names.add("inputPath");
        names.add("jobCategory");
        names.add("jobEnvironment");
        names.add("jobName");
        names.add("jobSubmissionState");
        names.add("joinFiles");
        names.add("nativeSpecification");
        names.add("outputPath");
        names.add("remoteCommand");
        names.add("startTime");
        names.add("workingDirectory");

        List retNames = new LinkedList(jt.getAttributeNames());

        Collections.sort(names);
        Collections.sort(retNames);

        assertEquals(names, retNames);
    }

    /** Test of getOptionalAttributeNames method, of class org.ggf.drmaa.JobTemplate. */
    public void testGetOptionalAttributeNames() {
        System.out.println("testGetOptionalAttributeNames");

        Set retNames = jt.getOptionalAttributeNames();

        assertEquals(Collections.EMPTY_SET, retNames);
    }

    /** Test of toString method, of class org.ggf.drmaa.JobTemplate. */
    public void testToString() throws DrmaaException {
        System.out.println("testToString");

        assertEquals("{blockEmail = false} {jobSubmissionState = ACTIVE_STATE} {joinFiles = false}",
                jt.toString());

        jt.setJobCategory("myCategory");

        assertEquals("{blockEmail = false} {jobCategory = myCategory} {jobSubmissionState = ACTIVE_STATE} {joinFiles = false}",
                jt.toString());

        jt.setArgs(Arrays.asList(new String[] {"arg1", "arg2"}));

        assertEquals("{args = \"arg1\", \"arg2\"} {blockEmail = false} {jobCategory = myCategory} {jobSubmissionState = ACTIVE_STATE} {joinFiles = false}",
                jt.toString());

        jt.setJoinFiles(true);

        assertEquals("{args = \"arg1\", \"arg2\"} {blockEmail = false} {jobCategory = myCategory} {jobSubmissionState = ACTIVE_STATE} {joinFiles = true}",
                jt.toString());

        jt.setJobSubmissionState(jt.HOLD_STATE);

        assertEquals("{args = \"arg1\", \"arg2\"} {blockEmail = false} {jobCategory = myCategory} {jobSubmissionState = HOLD_STATE} {joinFiles = true}",
                jt.toString());

        jt.setNativeSpecification("-l arch=sol-sparc64");

        assertEquals("{args = \"arg1\", \"arg2\"} {blockEmail = false} {jobCategory = myCategory} {jobSubmissionState = HOLD_STATE} {joinFiles = true} {nativeSpecification = \"-l arch=sol-sparc64\"}",
                jt.toString());

        Properties env = new Properties();
        env.setProperty("PATH", "/tmp:/usr/bin");
        env.setProperty("SHELL", "/usr/bin/csh");
        jt.setJobEnvironment(env);

        assertEquals("{args = \"arg1\", \"arg2\"} {blockEmail = false} {jobCategory = myCategory} {jobEnvironment = [\"SHELL\" = \"/usr/bin/csh\"], [\"PATH\" = \"/tmp:/usr/bin\"]} {jobSubmissionState = HOLD_STATE} {joinFiles = true} {nativeSpecification = \"-l arch=sol-sparc64\"}",
                jt.toString());

        jt.setStartTime(new PartialTimestamp(19, 10, 49));

        assertEquals("{args = \"arg1\", \"arg2\"} {blockEmail = false} {jobCategory = myCategory} {jobEnvironment = [\"SHELL\" = \"/usr/bin/csh\"], [\"PATH\" = \"/tmp:/usr/bin\"]} {jobSubmissionState = HOLD_STATE} {joinFiles = true} {nativeSpecification = \"-l arch=sol-sparc64\"} {startTime = \"19:10:49\"}",
                jt.toString());
    }
}
