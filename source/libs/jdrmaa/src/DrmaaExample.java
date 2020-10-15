import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import org.ggf.drmaa.DrmCommunicationException;
import org.ggf.drmaa.DrmaaException;
import org.ggf.drmaa.JobInfo;
import org.ggf.drmaa.JobTemplate;
import org.ggf.drmaa.Session;
import org.ggf.drmaa.SessionFactory;
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

public class DrmaaExample {
    private static int NBULKS = 3;
    private static int JOB_CHUNK = 8;
    private static Session session = null;

    public static void main(String[] args) throws Exception {
        String jobPath = args[0];

        SessionFactory factory = SessionFactory.getFactory();

        session = factory.getSession();
        session.init(null);

        JobTemplate jt = createJobTemplate(jobPath, 5, true);

        List allJobIds = new LinkedList();
        List jobIds = null;
        boolean retry = true;

        for (int count = 0; count < NBULKS; count++) {
            do {
                try {
                    jobIds = session.runBulkJobs(jt, 1, JOB_CHUNK, 1);
                    retry = false;
                } catch (DrmCommunicationException e) {
                    System.err.println("runBulkJobs() failed - retry: " + e.getMessage());

                    Thread.sleep(1000);
                }
            }
            while (retry);

            allJobIds.addAll(jobIds);

            System.out.println("submitted bulk job with jobids:");

            Iterator i = jobIds.iterator();

            while (i.hasNext()) {
                System.out.println("\t \"" + i.next() + "\"");
            }
        }

        session.deleteJobTemplate(jt);

        /* submit some sequential jobs */
        jt = createJobTemplate(jobPath, 5, false);

        String jobId = null;
        retry = true;

        for (int count = 0; count < JOB_CHUNK; count++) {
            while(retry) {
                try {
                    jobId = session.runJob(jt);
                    retry = false;
                } catch (DrmCommunicationException e) {
                    System.err.println("runBulkJobs() failed - retry: " + e.getMessage());

                    Thread.sleep(1000);
                }
            }

            System.out.println("\t \"" + jobId + "\"");
            allJobIds.add(jobId);
        }

        session.deleteJobTemplate(jt);

        /* synchronize with all jobs */
        session.synchronize(allJobIds, Session.TIMEOUT_WAIT_FOREVER, false);
        System.out.println("synchronized with all jobs");

        /* wait all those jobs */
        Iterator i = allJobIds.iterator();

        while (i.hasNext()) {
            JobInfo status = null;
            jobId = (String)i.next();

            status = session.wait(jobId, Session.TIMEOUT_WAIT_FOREVER);

            /* report how job finished */
            if (status.wasAborted()) {
                System.out.println("job \"" + jobId + "\" never ran");
            } else if (status.hasExited()) {
                System.out.println("job \"" + jobId + "\" finished regularly with exit status " + status.getExitStatus());
            } else if (status.hasSignaled()) {
                System.out.println("job \"" + jobId + "\" finished due to signal " + status.getTerminatingSignal());
            } else {
                System.out.println("job \"" + jobId + "\" finished with unclear conditions");
            }
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
}
