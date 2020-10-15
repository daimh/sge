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
package com.sun.grid.jgdi.util.shell;

import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.configuration.Job;
import com.sun.grid.jgdi.configuration.JobImpl;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummary;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryPrinter;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryResult;
import com.sun.grid.jgdi.monitoring.filter.JobStateFilter;
import java.util.List;

/**
 *
 */
@CommandAnnotation("qstat")
public class QStatCommand extends AnnotatedCommand {
    ClusterQueueSummaryOptions cqOptions = null;
    QueueInstanceSummaryOptions qiOptions = null;
    
    boolean showXml = false;
    
    public void run(String[] args) throws Exception {
        cqOptions = new ClusterQueueSummaryOptions();
        qiOptions = new QueueInstanceSummaryOptions();
        
        parseOptions(args);
        
        //Lookahead if we have -g c
        OptionInfo oi = getOptionInfo("-g");
        //If so print the cluster queues only
        if (oi != null && oi.getArgsAsString().indexOf("c") >= 0) {
            invokeOptions();
            if (cqOptions == null) {
                return;
            }
            @SuppressWarnings("unchecked")
            List<ClusterQueueSummary> res = jgdi.getClusterQueueSummary(cqOptions);
            if (res.size() == 0) {
                return;
            }
            out.printf("%s%s%n","CLUSTER QUEUE                   CQLOAD   USED    RES  AVAIL  TOTAL aoACDS  cdsuE",cqOptions.showAdditionalAttributes() ? "     s     A     S     C     u     a     d     D     c     o     E" : "");
            out.printf("%s%s%n","--------------------------------------------------------------------------------",cqOptions.showAdditionalAttributes() ? "------------------------------------------------------------------" : "");
            for (ClusterQueueSummary elem : res) {
                out.printf("%-30.30s ",elem.getName());
                if (elem.isLoadSet()) {
                    out.printf("%7.2f ", elem.getLoad());
                } else {
                    out.printf("%7s ", "-NA-");
                }
                out.printf("%6d %6d %6d %6d %6d %6d ", elem.getUsedSlots(), elem.getReservedSlots(), elem.getAvailableSlots(), elem.getTotalSlots(), elem.getTempDisabled(), elem.getManualIntervention());
                if (cqOptions.showAdditionalAttributes()) {
                    out.printf("%5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d ", elem.getSuspendManual(), elem.getSuspendThreshold(),
                            elem.getSuspendOnSubordinate(), elem.getSuspendByCalendar(), elem.getUnknown(), elem.getLoadAlarm(), elem.getDisabledManual(),
                            elem.getDisabledByCalendar(), elem.getAmbiguous(), elem.getOrphaned(), elem.getError());
                }
                out.printf("%n");
            }
            //No, we just print normal queue instances
        } else {
            //fill the option object
            invokeOptions();
            if (qiOptions == null) {
                return;
            }
            QueueInstanceSummaryResult res = jgdi.getQueueInstanceSummary(qiOptions);
            QueueInstanceSummaryPrinter.print(out, res, qiOptions);
        }
    }
    
    //[-ext]                            view additional attributes
    @OptionAnnotation(value = "-ext", min = 0)
    public void showExtendedAttributes(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowAdditionalAttributes(true);
        cqOptions.setShowAdditionalAttributes(true);
    }
    
    //[-explain a|c|A|E]                show reason for a(larm),c(onfiguration ambiguous),suspend A(larm), E(rror)   state
    @OptionAnnotation(value = "-explain")
    public void explainState(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowFullOutput(true);
        String str = oi.getFirstArg();
        if (str.length() != 1) {
            throw new IllegalArgumentException("Invalid explain specifier in -explain "+str);
        }
        qiOptions.setExplain(str.charAt(0));
    }
    
    //[-f]                              full output
    @OptionAnnotation(value = "-f", min = 0)
    public void showFullOutput(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowFullOutput(true);
        qiOptions.setShowEmptyQueues(true);
    }
    
    //[-F [resource_attributes]]        full output and show(selected) resources of queue(s)
    @OptionAnnotation(value = "-F", min =0, extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setResourceAttributes(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowFullOutput(true);
        qiOptions.updateResourceAttributeFilter(oi.getArgsAsString());
        oi.optionDone();
    }
    
    //[-g {c|d|t}]                      display cluster queue summary|all job-array tasks (do not group)|all parallel job tasks (do not group)
    @OptionAnnotation(value = "-g", extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setGroupOptions(final OptionInfo oi) throws JGDIException {
        StringBuilder sb = new StringBuilder(oi.getFirstArg());
        for (int i = 0; i < sb.length(); i++) {
            switch (sb.charAt(i)) {
                case 'd':
                    qiOptions.setShowArrayJobs(true);
                    break;
                case 't':
                    qiOptions.setShowPEJobs(true);
                    break;
                case 'c':
                    break;
                default:
                    throw new IllegalArgumentException("Invalid character '" + sb.charAt(i) + "' in -g " + sb.toString());
            }
        }
    }
    
    //[-help]                                  print this help
    @OptionAnnotation(value = "-help", min = 0)
    public void printUsage(final OptionInfo oi) throws JGDIException {
        out.println(getUsage());
        // To avoid the continue of the command
        throw new AbortException();
    }
    
    //[-j job_identifier_list ]         show scheduler job information
    @OptionAnnotation(value = "-j", extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setShowSchedulerJobInfo(final OptionInfo oi) throws JGDIException {
        //TODO LP: Improve input values and printing
        String job = oi.getFirstArg();
        Job jb = jgdi.getJob(Integer.parseInt(job));
      /*pw.printf("%29.29s %-s%n", "job_number:", jb.getJobNumber());
      pw.printf("%29.29s %-s%n", "exec_file:", jb.getExecFile());*/
        out.print(((JobImpl)jb).dump());
        throw new AbortException();
    }
    
    //[-l resource_list]                request the given resources
    @OptionAnnotation(value = "-l", extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setResourceList(final OptionInfo oi) throws JGDIException {
        qiOptions.updateResourceFilter(oi.getArgsAsString());
        oi.optionDone();
    }
    
    //[-ne]                             hide empty queues
    @OptionAnnotation(value = "-ne", min=0)
    public void hideEmptyQueues(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowEmptyQueues(false);
    }
    
    //[-pe pe_list]                     select only queues with one of these parallel environments
    @OptionAnnotation(value = "-pe", extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setParallelEnvironmentList(final OptionInfo oi) throws JGDIException {
        qiOptions.updatePeFilter(oi.getArgsAsString());
        oi.optionDone();
    }
    
    //[-q wc_queue_list]                print information on given queue
    @OptionAnnotation(value = "-q", extra=OptionAnnotation.MAX_ARG_VALUE)
    public void setQueueList(final OptionInfo oi) throws JGDIException {
        qiOptions.updateQueueFilter(oi.getArgsAsString());
        oi.optionDone();
    }
    
    //[-qs {a|c|d|o|s|u|A|C|D|E|S}]     selects queues, which are in the given state(s)
    @OptionAnnotation(value = "-qs" , extra = OptionAnnotation.MAX_ARG_VALUE)
    public void selectQueuesInState(final OptionInfo oi) throws JGDIException {
        qiOptions.updateQueueStateFilter(oi.getArgsAsString());
        oi.optionDone();
    }
    
    //[-r]                              show requested resources of job(s)
    @OptionAnnotation(value = "-r", min=0)
    public void showRequestedResources(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowRequestedResourcesForJobs(true);
    }
    
   /*[-s {p|r|s|z|hu|ho|hs|hj|ha|h|a}] show pending, running, suspended, zombie jobs,jobs with a user/operator/system hold,
                                          jobs with a start time in future or any combination only.
                                          h is an abbreviation for huhohshjha
                                          a is an abbreviation for prsh*/
    @OptionAnnotation(value = "-s", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void showJobsInState(final OptionInfo oi) throws JGDIException {
        JobStateFilter jobStateFilter = JobStateFilter.parse(oi.getArgsAsString());
        System.out.println("JobStateFilter: " + jobStateFilter.getStateString());
        qiOptions.setJobStateFilter(jobStateFilter);
        oi.optionDone();
    }
    
    //[-t]                              show task information (implicitly -g t)
    @OptionAnnotation(value = "-t", min=0)
    public void showTaskInformation(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowExtendedSubTaskInfo(true);
        qiOptions.setShowPEJobs(true);
    }
    
    //[-u user_list]                    view only jobs of this user
    @OptionAnnotation(value = "-u", extra=OptionAnnotation.MAX_ARG_VALUE)
    public void showJobsForUsersInUserList(final OptionInfo oi) throws JGDIException {
        qiOptions.updateJobUserFilter(oi.getArgsAsString());
        oi.optionDone();
    }
    
    //[-U user_list]                    select only queues where these users have access
    @OptionAnnotation(value = "-U", extra=OptionAnnotation.MAX_ARG_VALUE)
    public void showQueuesAccessibleToUserList(final OptionInfo oi) throws JGDIException {
        qiOptions.updateQueueUserFilter(oi.getArgsAsString());
        oi.optionDone();
    }
    
    //[-urg]                            display job urgency information
    @OptionAnnotation(value = "-urg", min=0)
    public void showJobUrgency(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowJobUrgency(true);
    }
    
    //[-pri]                            display job priority information
    @OptionAnnotation(value = "-pri", min=0)
    public void showJobPriority(final OptionInfo oi) throws JGDIException {
        qiOptions.setShowJobPriorities(true);
    }
    
    //[-xml]                            display the information in XML-Format
    @OptionAnnotation(value="-xml",min=0)
    public void setXml(final OptionInfo oi) throws JGDIException {
        showXml=true;
    }
}
