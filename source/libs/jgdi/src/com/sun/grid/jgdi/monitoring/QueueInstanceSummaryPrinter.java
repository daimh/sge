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

import com.sun.grid.jgdi.util.OutputTable;
import com.sun.grid.jgdi.util.OutputTable.Calc;
import java.beans.IntrospectionException;
import java.io.PrintWriter;
import java.text.DateFormat;
import java.text.DecimalFormat;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.List;

/**
 *
 */
public class QueueInstanceSummaryPrinter {

    private static final String HASHES = "##############################################################################################################";
    /* regular output */
    private static final String JHUL1 = "---------------------------------------------------------------------------------------------";
    /* -g t */
    private static final String JHUL2 = "-";
    /* -ext */
    private static final String JHUL3 = "-------------------------------------------------------------------------------";
    /* -t */
    private static final String JHUL4 = "-----------------------------------------------------";
    /* -urg */
    private static final String JHUL5 = "----------------------------------------------------------------";
    /* -pri */
    private static final String JHUL6 = "-----------------------------------";
    private boolean job_header_printed;
    private int last_job_id;
    private String lastQueue;

    public static OutputTable createJobSummaryTable(QueueInstanceSummaryOptions options) throws IntrospectionException {
        boolean sge_urg;
        boolean sge_pri;
        boolean sge_ext;
        boolean sge_time;
        boolean tsk_ext;
        boolean print_job_id;


        sge_ext = options.showAdditionalAttributes();
        tsk_ext = options.showExtendedSubTaskInfo();
        sge_urg = options.showJobUrgency();
        sge_pri = options.showJobPriorities();
        sge_time = !sge_ext;
        sge_time = sge_time | tsk_ext | sge_urg | sge_pri;

        OutputTable table = new OutputTable(JobSummary.class);

        table.addCol("id", "job-ID ", 7, OutputTable.Column.RIGHT);
        table.addCol("priority", "prior", 7);
        if (sge_pri || sge_urg) {
            table.addCol("normalizedUrgency", "nurg", 9);
        }
        if (sge_pri) {
            table.addCol("normalizedRequestedPriority", "npprior", 10);
        }
        if (sge_pri || sge_ext) {
            table.addCol("normalizedTickets", "ntckts", 6);
        }
        if (sge_urg) {
            table.addCol("urgency", "urg", 8);
            table.addCol("rrcontr", "rrcontr", 8);
            table.addCol("wtcontr", "wtcontr", 8);
            table.addCol("wtcontr", "dlcontr", 8);
        }
        if (sge_pri) {
            table.addCol("priority", "ppri", 7);
        }
        table.addCol("name", "name", 10);
        table.addCol("user", "user", 12);
        if (sge_ext) {
            table.addCol("project", "project", 15);
            table.addCol("department", "department", 15);
        }
        table.addCol("state", "state", 5);
        if (sge_time) {

            Calc timeCalc = new Calc() {

                public Object getValue(Object obj) {
                    JobSummary js = (JobSummary) obj;
                    if (js.isRunning()) {
                        return js.getStartTime();
                    } else {
                        return js.getSubmitTime();
                    }
                }
            };
            table.addCol("submitTime", "submit/start at     ", 19, DEFAULT_DATE_FORMAT, timeCalc);
        }
        if (sge_urg) {
            table.addCol("deadline", "deadline", 15);
        }
        if (sge_ext) {
            table.addCol("cpuUsage", "cpu", 8, new CpuUsageCalc());
            table.addCol("memUsage", "mem", 7, OutputTable.Column.RIGHT, new MemUsageCalc());
            table.addCol("ioUsage", "io", 7, OutputTable.Column.RIGHT, new IOUsageCalc());
            table.addCol("tickets", "tckts", 8, new TicketCalc(sge_ext) {

                public long getValue(JobSummary js) {
                    return js.getTickets();
                }
            });
            table.addCol("overrideTickets", "ovrts", 8, new TicketCalc(sge_ext) {

                public long getValue(JobSummary js) {
                    return js.getOverrideTickets();
                }
            });
            table.addCol("otickets", "otckt", 8, new TicketCalc(sge_ext) {

                public long getValue(JobSummary js) {
                    return js.getOtickets();
                }
            });
            table.addCol("ftickets", "ftckt", 8, new TicketCalc(sge_ext) {

                public long getValue(JobSummary js) {
                    return js.getFtickets();
                }
            });
            table.addCol("stickets", "stckt", 8, new TicketCalc(sge_ext) {

                public long getValue(JobSummary js) {
                    return js.getStickets();
                }
            });

            table.addCol("share", "share", 8, new ShareCalc(sge_ext));
        }
        //table.addCol("queueAssigned", "qs", 5);
        if (!options.showFullOutput()) {
            table.addCol("queue", "queue", 30);
        }
        // if (options.showPEJobs()) {
        if (options.showExtendedSubTaskInfo()) {
            table.addCol("masterQueue", "master", 8, OutputTable.Column.LEFT);
        } else {
            table.addCol("slots", "slots", 8, OutputTable.Column.RIGHT);
        }
        table.addCol("ja-taskId", "ja-task-ID", 10, new JaTaskIdCalc());


        if (tsk_ext) {
            Calc fixedValue = new Calc() {

                public Object getValue(Object obj) {
                    return "   NA ";
                }
            };
            table.addCol("taskId", "task-ID ", 7, fixedValue);
            table.addCol("state", "state", 6, new StatCalc());
            // TODO retrieve values from first task
            table.addCol("cpuUsage", "cpu", 8, fixedValue);
            table.addCol("memUsage", "mem", 7, OutputTable.Column.RIGHT, fixedValue);
            table.addCol("ioUsage", "io", 7, OutputTable.Column.RIGHT, fixedValue);
            table.addCol("stat", "stat", 5, fixedValue);
            table.addCol("failed", "failed", 7, fixedValue);
        }
        return table;
    }

    private static class StatCalc implements Calc {

        public Object getValue(Object obj) {
            JobSummary js = (JobSummary) obj;

            List tl = js.getTaskList();

            if (tl.isEmpty()) {
                return "";
            } else {
                TaskSummary ts = (TaskSummary) tl.get(0);
                return ts.getState();
            }
        }
    }

    private static class CpuUsageCalc implements Calc {

        public Object getValue(Object obj) {
            JobSummary js = (JobSummary) obj;
            if (!js.hasCpuUsage()) {
                if (js.isRunning()) {
                    return "NA";
                } else {
                    return "";
                }
            } else {
                int secs;
                int minutes;
                int hours;
                int days;

                secs = js.getCpuUsage();

                days = secs / (60 * 60 * 24);
                secs -= days * (60 * 60 * 24);

                hours = secs / (60 * 60);
                secs -= hours * (60 * 60);

                minutes = secs / 60;
                secs -= minutes * 60;

                return MessageFormat.format("{0,number}:{1,number,00}:{1,number,00}:{1,number,00}", new Object[]{new Integer(days), new Integer(hours), new Integer(minutes), new Integer(secs)});
            }
        }
    }

    private static abstract class UsageCalc implements Calc {

        private DecimalFormat format = new DecimalFormat("#.00000");

        protected abstract boolean hasValue(JobSummary js);

        protected abstract double getValue(JobSummary js);

        public Object getValue(Object obj) {
            JobSummary js = (JobSummary) obj;
            if (!hasValue(js)) {
                if (js.isRunning()) {
                    return "NA";
                } else {
                    return "";
                }
            } else {
                return format.format(getValue(js));
            }
        }
    }

    private static class MemUsageCalc extends UsageCalc {

        protected boolean hasValue(JobSummary js) {
            return js.hasMemUsage();
        }

        protected double getValue(JobSummary js) {
            return js.getMemUsage();
        }
    }

    private static class IOUsageCalc extends UsageCalc {

        protected boolean hasValue(JobSummary js) {
            return js.hasIoUsage();
        }

        protected double getValue(JobSummary js) {
            return js.getIoUsage();
        }
    }

    private static abstract class TicketCalc implements Calc {

        private DecimalFormat format = new DecimalFormat("####0");
        private boolean sge_ext;

        public TicketCalc(boolean sge_ext) {
            this.sge_ext = sge_ext;
        }

        protected abstract long getValue(JobSummary js);

        public Object getValue(Object obj) {
            JobSummary js = (JobSummary) obj;
            if (js.isZombie()) {
                return "NA ";
            } else {
                if (sge_ext || js.isQueueAssigned()) {
                    return format.format(getValue(js));
                } else {
                    return "";
                }
            }
        }
    }

    private static class ShareCalc implements Calc {

        private DecimalFormat format = new DecimalFormat("##0.00");
        private boolean sge_ext;

        public ShareCalc(boolean sge_ext) {
            this.sge_ext = sge_ext;
        }

        public Object getValue(Object obj) {
            JobSummary js = (JobSummary) obj;
            if (js.isZombie()) {
                return "NA ";
            } else {
                if (sge_ext || js.isQueueAssigned()) {
                    return format.format(js.getShare());
                } else {
                    return "";
                }
            }
        }
    }

    private static class JaTaskIdCalc implements Calc {

        private DecimalFormat format = new DecimalFormat("###.00");

        public Object getValue(Object obj) {
            JobSummary js = (JobSummary) obj;
            if (js.isArray() && js.getTaskId() != null) {
                return js.getTaskId();
            } else {
                return "";
            }
        }
    }
    private static final DateFormat DEFAULT_DATE_FORMAT = new SimpleDateFormat("MM/dd/yyyy HH:mm:ss");

    private static OutputTable createQueueInstanceSummaryTable() throws IntrospectionException {

        OutputTable ret = new OutputTable(QueueInstanceSummary.class);

        ret.addCol("name", "queuename", 30);
        ret.addCol("queueType", "qtype", 5);

        Calc slotCalc = new Calc() {

            public Object getValue(Object obj) {

                QueueInstanceSummary qi = (QueueInstanceSummary) obj;

                StringBuilder ret = new StringBuilder();
                ret.append(qi.getReservedSlots());
                ret.append('/');
                ret.append(qi.getUsedSlots());
                ret.append('/');
                ret.append(qi.getTotalSlots());
                return ret.toString();
            }
        };

        ret.addCol("totalSlots", "resv/used/tot.", 14, slotCalc);

        Calc loadAvgCalc = new Calc() {

            public Object getValue(Object obj) {

                QueueInstanceSummary qi = (QueueInstanceSummary) obj;
                if (qi.isHasLoadValueFromObject()) {
                    return new Double(qi.getLoadAvg());
                } else {
                    return "-NA-";
                }
            }
        };

        ret.addCol("load_avg", "load_avg", 8, new DecimalFormat("####0.00"), loadAvgCalc);
        ret.addCol("arch", "arch", 13);
        ret.addCol("state", "states", 6);

        return ret;
    }

    private static OutputTable createHostTable() throws IntrospectionException {

        OutputTable ret = new OutputTable(HostInfo.class);

        ret.addCol("hostname", "HOSTNAME", 23);
        ret.addCol("arch", "ARCH", 13);
        ret.addCol("NumberOfProcessors", "NCPU", 4, OutputTable.Column.RIGHT);
        ret.addCol("loadAvg", "LOAD", 5, OutputTable.Column.RIGHT);
        ret.addCol("memTotal", "MEMTOT", 7, OutputTable.Column.RIGHT);
        ret.addCol("memUsed", "MEMUSE", 7, OutputTable.Column.RIGHT);
        ret.addCol("swapTotal", "SWAPTO", 7, OutputTable.Column.RIGHT);
        ret.addCol("swapUsed", "SWAPUS", 7, OutputTable.Column.RIGHT);

        return ret;
    }

    private static OutputTable createHostQueueTable() throws IntrospectionException {
        OutputTable ret = new OutputTable(QueueInfo.class);
        ret.addCol("qname", "qname", 20);
        ret.addCol("qType", "qtype", 5);

        Calc slotCalc = new Calc() {

            public Object getValue(Object obj) {
                StringBuilder ret = new StringBuilder();
                QueueInfo qi = (QueueInfo) obj;
                // ret.append(qi.getReservedSlots());
                // ret.append('/');
                ret.append(qi.getUsedSlots());
                ret.append('/');
                ret.append(qi.getTotalSlots());
                return ret.toString();
            }
        };
        ret.addCol("usedSlots", "used/tot.", 9, slotCalc);
        ret.addCol("state", "state", 5);

        return ret;
    }

    private static OutputTable createHostJobTable(QHostOptions options) throws IntrospectionException {

        OutputTable ret = new OutputTable(JobInfo.class);

        ret.addCol("id", "job-ID ", 7, OutputTable.Column.RIGHT);
        ret.addCol("priority", "prior", 7, OutputTable.POINT_FIVE_FORMAT);
        ret.addCol("name", "name", 10);
        ret.addCol("user", "user", 12);
        ret.addCol("state", "state", 5);
        Calc timeCalc = new Calc() {

            public Object getValue(Object obj) {
                JobInfo ji = (JobInfo) obj;
                return ji.getStartTime();
            }
        };
        ret.addCol("startTime", "submit/start at     ", 19, DEFAULT_DATE_FORMAT, timeCalc);
        if (!options.includeQueue()) {
            ret.addCol("qinstanceName", "queue", 10);
        }
        ret.addCol("masterQueue", "master", 6);
        ret.addCol("taskID", "ja-task-ID", 10, new JaTaskIdCalc() {

            public Object getValue(Object obj) {
                String ret = ((JobInfo) obj).getTaskId();
                if (ret == null) {
                    return "";
                }
                return ret;
            }
        });

        return ret;
    }

    public static void print(PrintWriter pw, QueueInstanceSummaryResult result, QueueInstanceSummaryOptions options) {
        OutputTable jobSummaryTable = null;
        OutputTable qiTable = null;
        try {
            jobSummaryTable = createJobSummaryTable(options);
            qiTable = createQueueInstanceSummaryTable();
        } catch (IntrospectionException ex) {
            IllegalStateException ex1 = new IllegalStateException("introspection error");
            ex1.initCause(ex);
            throw ex1;
        }


        boolean hadJobs = false;
        if (!result.getQueueInstanceSummary().isEmpty()) {
            if (options.showFullOutput()) {
                qiTable.printHeader(pw);
            }
        }
        for (QueueInstanceSummary qi : result.getQueueInstanceSummary()) {
            if (options.showFullOutput()) {
                qiTable.printDelimiter(pw, '-');
            }
            if (options.showFullOutput()) {
                qiTable.printRow(pw, qi);
                for (String dom : qi.getResourceDominanceSet()) {
                    for (String name : qi.getResourceNames(dom)) {
                        pw.print("    " + dom + ":" + name + "=");
                        pw.println(qi.getResourceValue(dom, name));
                    }
                }
            }
            for (JobSummary js : qi.getJobList()) {
                if (!hadJobs && !options.showFullOutput()) {
                    jobSummaryTable.printHeader(pw);
                    jobSummaryTable.printDelimiter(pw, '-');
                }
                jobSummaryTable.printRow(pw, js);
                if (options.showRequestedResourcesForJobs()) {
                    printRequestedResources(pw, js);
                }
                hadJobs = true;
            }
        }

        List<JobSummary> jobList = result.getPendingJobs();
        if (!jobList.isEmpty()) {
            if (options.showFullOutput()) {
                pw.println();
                qiTable.printDelimiter(pw, '#');
                pw.println(" - PENDING JOBS - PENDING JOBS - PENDING JOBS - PENDING JOBS - PENDING JOBS");
                qiTable.printDelimiter(pw, '#');
            }

            if (!hadJobs && !options.showFullOutput()) {
                jobSummaryTable.printHeader(pw);
                jobSummaryTable.printDelimiter(pw, '-');
            }
            for (JobSummary js : jobList) {
                jobSummaryTable.printRow(pw, js);
                if (options.showRequestedResourcesForJobs()) {
                    printRequestedResources(pw, js);
                }
                hadJobs = true;
            }
        }

        jobList = result.getErrorJobs();
        if (!jobList.isEmpty()) {
            if (options.showFullOutput()) {
                pw.println();
                qiTable.printDelimiter(pw, '-');
                pw.println("     - ERROR JOBS - ERROR JOBS - ERROR JOBS - ERROR JOBS - ERROR JOBS");
                qiTable.printDelimiter(pw, '#');
            }
            if (!hadJobs && !options.showFullOutput()) {
                jobSummaryTable.printHeader(pw);
                jobSummaryTable.printDelimiter(pw, '-');
            }
            for (JobSummary js : jobList) {
                jobSummaryTable.printRow(pw, js);
                if (options.showRequestedResourcesForJobs()) {
                    printRequestedResources(pw, js);
                }
                hadJobs = true;
            }
        }

        jobList = result.getFinishedJobs();
        if (!jobList.isEmpty()) {
            if (options.showFullOutput()) {
                pw.println();
                qiTable.printDelimiter(pw, '#');
                pw.println("- FINISHED JOBS - FINISHED JOBS - FINISHED JOBS - FINISHED JOBS - FINISHED JOBS");
                qiTable.printDelimiter(pw, '#');
            }
            if (!hadJobs && !options.showFullOutput()) {
                jobSummaryTable.printHeader(pw);
                jobSummaryTable.printDelimiter(pw, '-');
            }
            for (JobSummary js : jobList) {
                jobSummaryTable.printRow(pw, js);
                if (options.showRequestedResourcesForJobs()) {
                    printRequestedResources(pw, js);
                }
                hadJobs = true;
            }
        }

        jobList = result.getZombieJobs();
        if (!jobList.isEmpty()) {
            if (options.showFullOutput()) {
                pw.println();
                qiTable.printDelimiter(pw, '#');
                pw.println("   - ZOMBIE JOBS - ZOMBIE JOBS - ZOMBIE JOBS - ZOMBIE JOBS - ZOMBIE JOBS");
                qiTable.printDelimiter(pw, '#');
            }
            if (!hadJobs && !options.showFullOutput()) {
                jobSummaryTable.printHeader(pw);
                jobSummaryTable.printDelimiter(pw, '-');
            }
            for (JobSummary js : jobList) {
                jobSummaryTable.printRow(pw, js);
                if (options.showRequestedResourcesForJobs()) {
                    printRequestedResources(pw, js);
                }
                hadJobs = true;
            }
        }
    }

    public static void print(PrintWriter pw, QHostResult result, QHostOptions options) {
        OutputTable hjTable = null;
        OutputTable hqTable = null;
        OutputTable hiTable = null;

        try {
            hiTable = createHostTable();
            hqTable = createHostQueueTable();
            hjTable = createHostJobTable(options);
        } catch (IntrospectionException ex) {
            IllegalStateException ex1 = new IllegalStateException("   - ZOMBIE JOBS - ZOMBIE JOBS - ZOMBIE JOBS - ZOMBIE JOBS - ZOMBIE JOBS");
            ex1.initCause(ex);
            throw ex1;
        }

        if (!result.getHostInfo().isEmpty()) {
            hiTable.printHeader(pw);
            hiTable.printDelimiter(pw, '-');
        }
        for (HostInfo hi : result.getHostInfo()) {
            hiTable.printRow(pw, hi);
            if (options.getResourceAttributeFilter() != null) {
                for (String dom : hi.getDominanceSet()) {
                    for (String name : hi.getResourceValueNames(dom)) {
                        pw.print("   " + dom + ":" + name + "=");
                        pw.println(hi.getResourceValue(dom, name));
                    }
                }
            }
            if (options.includeQueue() || options.includeJobs()) {
                for (QueueInfo qi : hi.getQueueList()) {
                    if (options.includeQueue()) {
                        //Print queue header
                        /*if (!options.includeJobs()) {
                        pw.print("   ");
                        hqTable.printHeader(pw);
                        pw.print("   ");
                        hqTable.printDelimiter(pw, '-');
                        }*/
                        pw.print("   ");
                        hqTable.printRow(pw, qi);
                    }
                    //Print jobs header
                    /*if (options.includeJobs() && !hi.getJobList().isEmpty() && !options.includeQueue()) {
                    pw.print("   ");
                    hjTable.printHeader(pw);
                    pw.print("   ");
                    hjTable.printDelimiter(pw, '-');
                    }*/
                    if (options.includeJobs()) {
                        if (!hi.getJobList().isEmpty()) {
                            for (JobInfo job : hi.getJobList()) {
                                if (job.getQueue().equals(qi.getQname())) {
                                    pw.print("   ");
                                    hjTable.printRow(pw, job);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    private static void printRequestedResources(PrintWriter pw, JobSummary js) {
        pw.print("      Full jobname: ");
        pw.println(js.getName());
        if (js.getMasterQueue() != null) {
            pw.print("      Master Queue: ");
            pw.println(js.getMasterQueue());
        }

        if (js.getGrantedPEName() != null) {
            pw.print("      Requested PE: ");
            pw.println(js.getParallelEnvironmentName() + " " + js.getParallelEnvironmentRange());
            pw.print("      Granted PE: ");
            pw.print(js.getGrantedPEName());
            pw.print(" ");
            pw.println(js.getGrantedPESlots());
        }

        pw.print("      Hard Resources: ");
        boolean firstRes = true;
        for (String resName : js.getHardRequestNames()) {
            if (firstRes) {
                firstRes = false;
            } else {
                pw.print(", ");
            }
            pw.print(resName);
            pw.print("=");
            HardRequestValue value = js.getHardRequestValue(resName);
            pw.print(value.getValue());
            pw.print(" (");
            pw.print(OutputTable.POINT_FIVE_FORMAT.format(value.getContribution()));
            pw.print(")");
        }
        pw.println();
        pw.print("      Soft Resources: ");
        firstRes = true;
        for (String resName : js.getSoftRequestNames()) {
            if (firstRes) {
                firstRes = false;
            } else {
                pw.print(", ");
            }
            pw.print(resName);
            pw.print("=");
            pw.print(js.getSoftRequestValue(resName));
        }
        pw.println();
    }
}