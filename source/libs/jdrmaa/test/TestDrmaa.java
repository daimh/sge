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
import java.io.*;
import java.util.*;

import org.ggf.drmaa.*;

/**
 *
 * @author  dan.templeton@sun.com
 */
public class TestDrmaa {
    private static Session session = null;
    private static LinkedList jobIds = null;
    private static final Object lock = new Object();
    private static String script = null;

    /** Creates a new instance of TestDrmaa */
    public TestDrmaa() throws Exception {
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) throws Exception {
        String arg = null;
        int duration = 300;

        if ((args.length < 2) || (args.length > 3)) {
            printUsage();
            System.exit(1);
        } else if (args[0].equals("-h") || args[0].equals("-help")) {
            printUsage();
            System.exit(0);
        }

        script = args[0];
        arg = args[1];

        if (args.length > 2) {
            try {
                duration = Integer.parseInt(args[2]);
            } catch (NumberFormatException e) {
                printUsage();
                System.exit(1);
            }
        }

        jobIds = new LinkedList();

        SessionFactory factory = SessionFactory.getFactory();

        session = factory.getSession();
        Runtime.getRuntime().addShutdownHook(new ShutdownHook());
        session.init(null);

        System.out.println("DRMS: " + session.getDrmSystem());
        System.out.println("Contact: " + session.getContact());
        System.out.println("Implementation: " + session.getDrmaaImplementation());

        Version version = session.getVersion();

        System.out.println("Version: " + Integer.toString(version.getMajor()) + "." + Integer.toString(version.getMinor()));
        System.out.println("--------------------------------------------------");
        System.out.println("");

        log("0: Starting test threads...");

        startThreads(arg);

        sleep(5);

        synchronized (lock) {
            lock.notifyAll();
        }

        log("0: Sleeping");
        sleep(duration);

        System.exit(0);
    }

    private static void printUsage() {
        System.out.println("java TestDRMAA script codes [duration]");
        System.out.println("\tcodes: <number><code>[:codes]");
        System.out.println("\tnumber: 1-9,0=10 -- number of instances");
        System.out.println("\tcode: W|Y|D|H|P|s|b|w|y|d|h|p");
        System.out.println("\t\tW -- Submit and wait");
        System.out.println("\t\tY -- Submit bulk and synchronize");
        System.out.println("\t\tD -- Submit and delete");
        System.out.println("\t\tH -- Submit, hold, release");
        System.out.println("\t\tP -- Submit, suspend, resume");
        System.out.println("\t\ts -- Submit");
        System.out.println("\t\tb -- Submit bulk");
        System.out.println("\t\tw -- Wait any");
        System.out.println("\t\ty -- Synchronize all");
        System.out.println("\t\td -- Delete all");
        System.out.println("\t\th -- Hold all, release all");
        System.out.println("\t\tp -- Suspend all, resume all");
        System.out.println("Example:");
        System.out.println("\tjava TestDRMAA 1S:4H:3s:2w:1d");
    }

    private static void startThreads(String arg) throws DrmaaException {
        StringTokenizer tok = new StringTokenizer(arg, ":");
        int id = 1;

        while (tok.hasMoreTokens()) {
            String code = tok.nextToken();
            int number = -1;
            char type = code.charAt(1);

            try {
                number = Integer.parseInt(code.substring(0, 1));

                if (number == 0) {
                    number = 10;
                }
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException("Invalid argument: " + arg);
            }

            if (type == 'W') {
                for (int count = 0; count < number; count++) {
                    new Thread(new SubmitWaitTester(id++)).start();
                }
            } else if (type == 'Y') {
                for (int count = 0; count < number; count++) {
                    new Thread(new BulkSubmitSyncTester(id++)).start();
                }
            } else if (type == 'D') {
                for (int count = 0; count < number; count++) {
                    new Thread(new SubmitDeleteTester(id++)).start();
                }
            } else if (type == 'H') {
                for (int count = 0; count < number; count++) {
                    new Thread(new SubmitHoldReleaseTester(id++)).start();
                }
            } else if (type == 'P') {
                for (int count = 0; count < number; count++) {
                    new Thread(new SubmitSuspendResumeTester(id++)).start();
                }
            } else if (type == 's') {
                for (int count = 0; count < number; count++) {
                    new Thread(new SubmitTester(id++, 5)).start();
                }
            } else if (type == 'b') {
                for (int count = 0; count < number; count++) {
                    new Thread(new BulkSubmitTester(id++, 5)).start();
                }
            } else if (type == 'w') {
                for (int count = 0; count < number; count++) {
                    new Thread(new WaitTester(id++)).start();
                }
            } else if (type == 'y') {
                for (int count = 0; count < number; count++) {
                    new Thread(new SyncTester(id++)).start();
                }
            } else if (type == 'd') {
                for (int count = 0; count < number; count++) {
                    new Thread(new DeleteAllTester(id++)).start();
                }
            } else if (type == 'h') {
                for (int count = 0; count < number; count++) {
                    new Thread(new HoldReleaseAllTester(id++)).start();
                }
            } else if (type == 'p') {
                for (int count = 0; count < number; count++) {
                    new Thread(new SuspendResumeAllTester(id++)).start();
                }
            } else {
                throw new IllegalArgumentException("Invalid code: " + code);
            }
        }
    }

    static synchronized void log(String message) {
        System.out.println(message);
    }

    private static abstract class Tester implements Runnable {
        private int id = 0;
        protected JobTemplate jt = null;

        Tester(int id) {
            this.id = id;
        }

        void log(String message) {
            TestDrmaa.log(Integer.toString(id) + ": " + message);
        }

        void log(Throwable e) {
            StringWriter sout = new StringWriter();
            PrintWriter pout = new PrintWriter(sout);
            e.printStackTrace(pout);
            pout.close();
            TestDrmaa.log(Integer.toString(id) + ": " + sout.toString());
        }

        public void run() {
            try {
                synchronized (lock) {
                    lock.wait();
                }
            } catch (InterruptedException e) {
                log(e);
                return;
            }

            log("Starting up");

            while (true) {
                try {
                    test();
                } catch (NoActiveSessionException e) {
                    log("No active session. Shutting down.");
                    break;
                } catch (Exception e) {
                    log(e);
                }
            }
        }

        public abstract void test() throws DrmaaException;

        public void cleanup() {
            try {
                session.deleteJobTemplate(jt);
            } catch (DrmaaException e) {
                log(e);
            }
        }
    }

    private static class SubmitWaitTester extends Tester {
        SubmitWaitTester(int id) throws DrmaaException {
            super(id);

            log("SubmitWaitTester");

            jt = createJobTemplate(script, 5, false);
        }

        public void test() throws DrmaaException {
            LinkedList ids = new LinkedList();
            String jobId = null;

            for (int count = 0; count < 10; count++) {
                jobId = session.runJob(jt);

                log("Submitted job " + jobId);

                ids.add(jobId);
            }

            Iterator i = ids.iterator();

            while (i.hasNext()) {
                JobInfo status = null;
                jobId = (String)i.next();

                try {
                    status = session.wait(jobId, Session.TIMEOUT_WAIT_FOREVER);
                } catch (ExitTimeoutException e) {
                    throw new NoActiveSessionException();
                }

                /* report how job finished */
                if (status.wasAborted()) {
                    log("job \"" + status.getJobId() + "\" never ran");
                } else if (status.hasExited()) {
                    log("job \"" + status.getJobId() + "\" finished regularly with exit status " + status.getExitStatus());
                } else if (status.hasSignaled()) {
                    log("job \"" + status.getJobId() + "\" finished due to signal " + status.getTerminatingSignal());
                } else {
                    log("job \"" + status.getJobId() + "\" finished with unclear conditions");
                }
            }
        }
    }

    private static class BulkSubmitSyncTester extends Tester {
        BulkSubmitSyncTester(int id) throws DrmaaException {
            super(id);

            log("BulkSubmitSyncTester");

            jt = createJobTemplate(script, 5, true);
        }

        public void test() throws DrmaaException {
            List jobIds = session.runBulkJobs(jt, 1, 20, 1);

            log("Submitted " + jobIds.size() + " jobs");

            try {
                session.synchronize(jobIds, Session.TIMEOUT_WAIT_FOREVER, true);
            } catch (ExitTimeoutException e) {
                throw new NoActiveSessionException();
            }

            log("All jobs have finished");
        }
    }

    private static class BulkSubmitTester extends Tester {
        private int sleep = 0;

        BulkSubmitTester(int id, int sleep) throws DrmaaException {
            super(id);

            log("BulkSubmitTester");

            this.sleep = sleep;

            jt = createJobTemplate(script, sleep, true);
        }

        public void test() throws DrmaaException {
            List jobIds = session.runBulkJobs(jt, 1, 20, 1);

            log("Submitted " + jobIds.size() + " jobs");

            if (sleep > 0) {
                sleep(sleep);
            }
        }
    }

    private static class SubmitTester extends Tester {
        private int sleep = 0;

        SubmitTester(int id, int sleep) throws DrmaaException {
            super(id);

            log("SubmitTester");

            this.sleep = sleep;

            jt = createJobTemplate(script, sleep, false);
        }

        public void test() throws DrmaaException {
            String jobId = session.runJob(jt);

            log("Submitted job " + jobId);

            if (sleep > 0) {
                sleep(sleep);
            }
        }
    }

    private static class WaitTester extends Tester {
        WaitTester(int id) throws DrmaaException {
            super(id);

            log("WaitTester");
        }

        public void test() throws DrmaaException {
            JobInfo info = null;

            try {
                info = session.wait(Session.JOB_IDS_SESSION_ANY,
                        Session.TIMEOUT_WAIT_FOREVER);

                if (info.wasAborted()) {
                    log("job \"" + info.getJobId() + "\" never ran");
                } else if (info.hasExited()) {
                    log("job \"" + info.getJobId() + "\" finished regularly with exit status " + info.getExitStatus());
                } else if (info.hasSignaled()) {
                    log("job \"" + info.getJobId() + "\" finished due to signal " + info.getTerminatingSignal());
                } else {
                    log("job \"" + info.getJobId() + "\" finished with unclear conditions");
                }
            } catch (ExitTimeoutException e) {
                throw new NoActiveSessionException();
            }

            if (info.getResourceUsage() == null) {
                log("job \"" + info.getJobId() + "\" has already been reaped");
            }
        }
    }

    private static class SyncTester extends Tester {
        private List all = Collections.singletonList(Session.JOB_IDS_SESSION_ALL);

        SyncTester(int id) throws DrmaaException {
            super(id);

            log("SyncTester");
        }

        public void test() throws DrmaaException {
            try {
                session.synchronize(all, Session.TIMEOUT_WAIT_FOREVER, false);
            } catch (ExitTimeoutException e) {
                throw new NoActiveSessionException();
            }

            log("All jobs have finished");
        }
    }

    private static class SubmitDeleteTester extends Tester {
        SubmitDeleteTester(int id) throws DrmaaException {
            super(id);

            log("SubmitDeleteTester");

            jt = createJobTemplate(script, 600, false);
        }

        public void test() throws DrmaaException {
            String jobId = null;

            jobId = session.runJob(jt);

            log("Submitted job " + jobId);

            session.control(jobId, Session.TERMINATE);

            log("Deleted job " + jobId);
        }
    }

    private static class SubmitHoldReleaseTester extends Tester {
        SubmitHoldReleaseTester(int id) throws DrmaaException {
            super(id);

            log("SubmitHoldReleaseTester");

            jt = createJobTemplate(script, 60, false);
        }

        public void test() throws DrmaaException {
            String jobId = null;

            jobId = session.runJob(jt);

            log("Submitted job " + jobId);

            session.control(jobId, Session.HOLD);

            log("Job state is " + statusToString(session.getJobProgramStatus(jobId)));

            sleep(20);

            log("Job state is " + statusToString(session.getJobProgramStatus(jobId)));

            session.control(jobId, Session.RELEASE);

            log("Job state is " + statusToString(session.getJobProgramStatus(jobId)));

            sleep(20);

            log("Job state is " + statusToString(session.getJobProgramStatus(jobId)));
        }
    }

    private static class SubmitSuspendResumeTester extends Tester {
        SubmitSuspendResumeTester(int id) throws DrmaaException {
            super(id);

            log("SubmitSuspendResumeTester");

            jt = createJobTemplate(script, 60, false);
        }

        public void test() throws DrmaaException {
            String jobId = null;

            jobId = session.runJob(jt);

            log("Submitted job " + jobId);

            while (true) {
                try {
                    session.control(jobId, Session.SUSPEND);
                    break;
                } catch (InconsistentStateException e) {
                    // Just means that the job isn't running yet
                    log("Waiting for job to be scheduled...");
                    sleep(10);
                }
            }

            log("Job state is " + statusToString(session.getJobProgramStatus(jobId)));

            sleep(20);

            log("Job state is " + statusToString(session.getJobProgramStatus(jobId)));

            session.control(jobId, Session.RESUME);

            log("Job state is " + statusToString(session.getJobProgramStatus(jobId)));

            sleep(20);

            log("Job state is " + statusToString(session.getJobProgramStatus(jobId)));
        }
    }

    private static class SuspendResumeAllTester extends Tester {
        SuspendResumeAllTester(int id) throws DrmaaException {
            super(id);

            log("SuspendResumeAllTester");
        }

        public void test() throws DrmaaException {
            sleep(30);

            session.control(Session.JOB_IDS_SESSION_ALL,
                    Session.SUSPEND);

            sleep(20);

            session.control(Session.JOB_IDS_SESSION_ALL,
                    Session.RESUME);
        }
    }

    private static class HoldReleaseAllTester extends Tester {
        HoldReleaseAllTester(int id) throws DrmaaException {
            super(id);

            log("HoldReleaseAllTester");
        }

        public void test() throws DrmaaException {
            session.control(Session.JOB_IDS_SESSION_ALL,
                    Session.HOLD);

            sleep(20);

            session.control(Session.JOB_IDS_SESSION_ALL,
                    Session.RELEASE);
        }
    }

    private static class DeleteAllTester extends Tester {
        DeleteAllTester(int id) throws DrmaaException {
            super(id);

            log("DeleteAllTester");
        }

        public void test() throws DrmaaException {
            sleep(50);

            session.control(Session.JOB_IDS_SESSION_ALL,
                    Session.TERMINATE);
        }
    }

    private static JobTemplate createJobTemplate(String jobPath, int seconds, boolean isBulkJob) throws DrmaaException {
        JobTemplate jt = session.createJobTemplate();

        jt.setWorkingDirectory("$drmaa_hd_ph$");
        jt.setRemoteCommand(jobPath);
        jt.setArgs(Collections.singletonList(Integer.toString(seconds)));
        jt.setJoinFiles(true);

        if (!isBulkJob) {
            jt.setOutputPath(":$drmaa_hd_ph$/DRMAA_JOB");
        } else {
            jt.setOutputPath(":$drmaa_hd_ph$/DRMAA_JOB$drmaa_incr_ph$");
        }

        return jt;
    }

    static String statusToString(int status) {
        switch (status) {
            case Session.QUEUED_ACTIVE:
                return "Queued";
            case Session.SYSTEM_ON_HOLD:
                return "System Hold";
            case Session.USER_ON_HOLD:
                return "User Hold";
            case Session.USER_SYSTEM_ON_HOLD:
                return "User & System Hold";
            case Session.RUNNING:
                return "Running";
            case Session.SYSTEM_SUSPENDED:
                return "System Suspended";
            case Session.USER_SUSPENDED:
                return "User Suspended";
            case Session.DONE:
                return "Finished";
            case Session.FAILED:
                return "Failed";
            default:
                return "Undetermined";
        }
    }

    static void sleep(int time) {
        try {
            Thread.sleep(time * 1000);
        } catch (InterruptedException e) {
            // Don't care
        }
    }

    private static class ShutdownHook extends Thread {
        public void run() {
            try {
                System.out.println("0: Exiting");
                session.exit();
            } catch (DrmaaException e) {
                e.printStackTrace();
            }
        }
    }
}
