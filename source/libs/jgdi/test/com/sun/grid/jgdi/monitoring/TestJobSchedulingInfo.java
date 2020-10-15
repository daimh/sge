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
package com.sun.grid.jgdi.monitoring;

import junit.framework.Test;
import junit.framework.TestSuite;
import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.configuration.Job;
import com.sun.grid.jgdi.configuration.JobSchedulingInfo;
import com.sun.grid.jgdi.configuration.JobSchedulingMessage;
import com.sun.grid.jgdi.configuration.ULNG;
import java.io.PrintWriter;

/**
 *
 */
public class TestJobSchedulingInfo extends com.sun.grid.jgdi.BaseTestCase {
    
    /** Creates a new instance of TestQHost */
    public TestJobSchedulingInfo(String testName) {
        super(testName);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(TestJobSchedulingInfo.class);
        return suite;
    }
    
    public void testSimple() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            PrintWriter pw = new PrintWriter(System.out);
            pw.println("Global Message List");
            JobSchedulingInfo schedInfo = jgdi.getJobSchedulingInfo();
            for (JobSchedulingMessage mes : schedInfo.getGlobalMessageList()) {
                pw.println(mes.getMessage() + " (" + mes.getMessageNumber() + ")");
                for (int i = 0; i < mes.getJobNumberCount(); i++) {
                    pw.println("Job " + mes.getJobNumberList().get(i).toString());
                }
            }
            if (schedInfo.isSetMessage()) {
                pw.println("Message List");
                for (JobSchedulingMessage mes : schedInfo.getMessageList()) {
                    pw.println(mes.getMessage() + " (" + mes.getMessageNumber() + ")");
                    for (int i = 0; i < mes.getJobNumberCount(); i++) {
                        int jobId = ((ULNG) mes.getJobNumberList().get(i)).getValue();
                        pw.println("job_number: " + jobId);
                        Job job = jgdi.getJob(jobId);
                        pw.println("exec_file:  " + job.getExecFile());
                        pw.println("submission_time: " + job.getSubmissionTime());
                        pw.println("owner: " + job.getOwner());
                        pw.println("uid: " + job.getUid());
                        pw.println("group: " + job.getGroup());
                        pw.println("gid: " + job.getGid());
//                  pw.println("default env: " + job.getDefaultEnv());
//                  pw.println("sge_o_home: " + job.getEnv("SGE_O_HOME"));
//                  pw.println("sge_o_log_name: " + job.getEnv("SGE_O_LOG_NAME"));
//                  pw.println("sge_o_path: " + job.getEnv("SGE_O_PATH"));
                    }
                }
            }
            pw.flush();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            jgdi.close();
        }
    }
}
