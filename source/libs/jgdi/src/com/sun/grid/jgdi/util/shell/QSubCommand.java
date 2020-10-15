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
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.configuration.ComplexEntry;
import com.sun.grid.jgdi.configuration.ComplexEntryImpl;
import com.sun.grid.jgdi.configuration.JobImpl;
import com.sun.grid.jgdi.configuration.JobReferenceImpl;
import com.sun.grid.jgdi.configuration.JobTaskImpl;
import com.sun.grid.jgdi.configuration.MailReceiver;
import com.sun.grid.jgdi.configuration.MailReceiverImpl;
import com.sun.grid.jgdi.configuration.PathNameImpl;
import com.sun.grid.jgdi.configuration.Range;
import com.sun.grid.jgdi.configuration.RangeImpl;
import com.sun.grid.jgdi.monitoring.filter.ResourceFilter;
import com.sun.grid.jgdi.util.JGDIShell;
import java.io.File;
import java.util.List;
import java.util.Set;
import static com.sun.grid.jgdi.util.JGDIShell.getResourceString;
import static com.sun.grid.jgdi.util.shell.Util.*;

/**
 *
 */
@CommandAnnotation(value = "qsub", hasExtraArgs=true)
public class QSubCommand extends AnnotatedCommand {
    
    JobImpl job = null;
    JobTaskImpl jt = null;
    boolean hard = true;
    boolean verify = false;
    boolean binary = false;
    
    public void run(String[] args) throws Exception {
        // empty job and job_task object
        clear();
        
        // parse arguments and fill the ar object
        parseAndInvokeOptions(args);
        if (hasExtraArguments()) {
            for (String arg : this.getExtraArguments()) {
                job.addJobArgs(arg);
            }
        }
        // Send the ar object to qmaster
        // List<JGDIAnswer> answers = new ArrayList<JGDIAnswer>();
        // job.addJaTasks(jt);
        StringBuilder sb = new StringBuilder();
        for (String str : args) {
            sb.append(str);
            sb.append(" ");
        }
        // TODO PJ This is temporary solution, should be replaced by JDGI
        // This will not work, if the qsub command is not on the path or
        // the current host is not submit host
        JGDIShell jsh = (JGDIShell) shell;
        jsh.runShellCommand("qsub " + sb.toString());
//        List <JGDIAnswer> answers = new LinkedList<JGDIAnswer>();
//        jgdi.addJobWithAnswer(job, answers);
//        printAnswers(answers);
    }
    
// [-a date_time]                           request a start time
    @OptionAnnotation(value = "-a")
    public void setStartTime(final OptionInfo oi) throws JGDIException {
        jt.setStartTime(getDateTimeAsInt(oi.getFirstArg()));
    }
    
// [-ac context_list]                       add context variable(s)
    @OptionAnnotation(value = "-ac", min = 2)
    public void setContext(final OptionInfo oi) throws JGDIException {
        job.putContext(oi.getFirstArg(), oi.getFirstArg());
    }
    
// [-ar ar_id]                              bind job to advance reservation
    @OptionAnnotation(value = "-ar")
    public void setAr(final OptionInfo oi) throws JGDIException {
        job.setAr(Integer.parseInt(oi.getFirstArg()));
    }
    
// [-A account_string]                      account string in accounting record
    @OptionAnnotation(value = "-A")
    public void setAaccountString(final OptionInfo oi) throws JGDIException {
        job.setAccount(oi.getFirstArg());
    }
    
// [-b y[es]|n[o]]                          handle command as binary
    @OptionAnnotation(value = "-b")
    public void setBinary(final OptionInfo oi) throws JGDIException {
        binary = isYes(oi.getFirstArg()); // TODO PJ The binary must be handled
    }
    
// [-c ckpt_selector]                       define type of checkpointing for job
    @OptionAnnotation(value = "-c")
    public void setCheckPoint(final OptionInfo oi) throws JGDIException {
        job.setCheckpointName(oi.getFirstArg());
    }
    
// [-ckpt ckpt-name]                        request checkpoint method
    @OptionAnnotation(value = "-ckpt-name")
    public void setCkptName(final OptionInfo oi) throws JGDIException {
        job.setCheckpointName(oi.getFirstArg());
    }
    
// [-clear]                                 skip previous definitions for job
    @OptionAnnotation(value = "-clear", min = 0)
    public void setClear(final OptionInfo oi) throws JGDIException {
        clear();
    }
    
// [-cwd]                                   use current working directory
    @OptionAnnotation(value = "-cwd", min = 0)
    public void setCwd(final OptionInfo oi) throws JGDIException {
        String cwd = (new File(".")).getPath();
        job.setCwd(cwd);
        oi.optionDone();
    }
    
// [-C directive_prefix]                    define command prefix for job script
    @OptionAnnotation(value = "-C")
    public void setDirectivePrefix(final OptionInfo oi) throws JGDIException {
        job.setDirectivePrefix(oi.getFirstArg());
    }
    
// [-dc simple_context_list]                delete context variable(s)
    @OptionAnnotation(value = "-dc")
    public void deleteContextVariable(final OptionInfo oi) throws JGDIException {
        job.removeContext(oi.getFirstArg());
    }
    
// [-dl date_time]                          request a deadline initiation time
    @OptionAnnotation(value = "-dl")
    public void seDeadLineVariable(final OptionInfo oi) throws JGDIException {
        job.setDeadline(Util.getDateTimeAsInt(oi.getFirstArg()));
    }
    
// [-e path_list]                           specify standard error stream path(s)
    @OptionAnnotation(value = "-e")
    public void setErrorPath(final OptionInfo oi) throws JGDIException {
        job.addStderrPath(new PathNameImpl(oi.getFirstArg()));
    }
    
// [-h]                                     place user hold on job
    @OptionAnnotation(value = "-h")
    public void setHold(final OptionInfo oi) throws JGDIException {
        int aHold = 0; // TODO PJ create hold state
        jt.setHold(aHold);
    }
    
// [-hard]                                  consider following requests "hard"
    @OptionAnnotation(value = "-hard", min = 0)
    public void setHard(final OptionInfo oi) throws JGDIException {
        hard = true;
    }
    
// [-help]                                  print this help
    @OptionAnnotation(value = "-help", min = 0)
    public void printUsage(final OptionInfo oi) throws JGDIException {
        out.println(getUsage());
        // To avoid the continue of the command
        throw new AbortException();
    }
    
// [-hold_jid job_identifier_list]          define jobnet interdependencies
    @OptionAnnotation(value = "-hold_jid", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setJidPredecessor(final OptionInfo oi) throws JGDIException {
        job.addJidPredecessor(new JobReferenceImpl(Integer.parseInt(oi.getFirstArg())));
    }
    
// [-i file_list]                           specify standard input stream file(s)
    @OptionAnnotation(value = "-i", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setStrInPath(final OptionInfo oi) throws JGDIException {
        job.addStdinPath(new PathNameImpl(oi.getFirstArg()));
    }
    
// [-j y[es]|n[o]]                          merge stdout and stderr stream of job
    @OptionAnnotation(value = "-j")
    public void setMerge(final OptionInfo oi) throws JGDIException {
        job.setMergeStderr(Util.isYes(oi.getFirstArg()));
    }
    
// [-js job_share]                          share tree or functional job share
    @OptionAnnotation(value = "-js")
    public void setJobShareMerge(final OptionInfo oi) throws JGDIException {
        job.setMergeStderr(Util.isYes(oi.getFirstArg()));
    }
    
// [-l resource_list]                       request the given resources
    @OptionAnnotation(value = "-l", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setResourceList(final OptionInfo oi) throws JGDIException {
        ResourceFilter resourceList = ResourceFilter.parse(oi.getArgsAsString());
        @SuppressWarnings(value = "unchecked")
        Set<String> names = resourceList.getResourceNames();
        for (String name : names) {
            ComplexEntry ce = new ComplexEntryImpl(name);
            ce.setStringval(resourceList.getResource(name));
            if (hard) {
                job.addHardResource(ce);
            } else {
                job.addSoftResource(ce);
            }
        }
        oi.optionDone();
    }
    
// [-m mail_options]                        define mail notification events
    @OptionAnnotation(value = "-m", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setMailOption(final OptionInfo oi) throws JGDIException {
        job.setMailOptions(getMailOptionsAsInt(oi.getFirstArg()));
    }
    
// [-masterq wc_queue_list]                 bind master task to queue(s)
    @OptionAnnotation(value = "-masterq", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setMasterQueue(final OptionInfo oi) throws JGDIException {
        oi.optionDone();
        for (String amasterQueue : oi.getArgs()) {
            if (hard) {
                job.addMasterHardQueue(amasterQueue);
            } else {
                throw new IllegalStateException("Cold not set soft master queue");
            }
        }
    }
    
// [-notify]                                notify job before killing/suspending it
    @OptionAnnotation(value = "-notify", min = 0, extra = 1)
    public void setNotify(final OptionInfo oi) throws JGDIException {
        boolean notify = true;
        oi.optionDone();
        List<String> args = oi.getArgs();
        if (!args.isEmpty()) {
            notify = args.get(0).toLowerCase().startsWith("y");
        }
        job.setNotify(notify);
    }
    
// [-now y[es]|n[o]]                        start job immediately or not at all
    @OptionAnnotation(value = "-now")
    public void setNow(final OptionInfo oi) throws JGDIException {
        job.setType(getYesNoAsInt(oi.getFirstArg()));
    }
    
// [-M mail_list]                           notify these e-mail addresses
    @OptionAnnotation(value = "-M", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setMailList(final OptionInfo oi) throws JGDIException {
        MailReceiver mr = new MailReceiverImpl(oi.getFirstArg());
        job.addMail(mr);
    }
    
// [-N name]                                specify job name
    @OptionAnnotation(value = "-N")
    public void setJobName(final OptionInfo oi) throws JGDIException {
        job.setName(oi.getFirstArg());
    }
    
// [-o path_list]                           specify standard output stream path(s)
    @OptionAnnotation(value = "-o")
    public void setStrOutPath(final OptionInfo oi) throws JGDIException {
        job.addStdoutPath(new PathNameImpl(oi.getFirstArg()));
    }
    
// [-P project_name]                        set job's project
    @OptionAnnotation(value = "-P")
    public void setProject(final OptionInfo oi) throws JGDIException {
        job.setProject(oi.getFirstArg());
    }
    
// [-p priority]                            define job's relative priority
    @OptionAnnotation(value = "-p")
    public void setPrioroty(final OptionInfo oi) throws JGDIException {
        job.setPriority(Integer.parseInt(oi.getFirstArg()));
    }
    
// [-pe pe-name slot_range]                 request slot range for parallel jobs
    @OptionAnnotation(value = "-pe", min = 2)
    public void setPE(final OptionInfo oi) throws JGDIException {
        job.setPe(oi.getFirstArg());
        String[] range = oi.getFirstArg().split("[-]");
        Range apeRange = new RangeImpl();
        if (range[0] != null && range[0].length() > 0) {
            apeRange.setMin(Integer.parseInt(range[0]));
        }
        if (range.length > 1 && range[1] != null && range[1].length() > 0) {
            apeRange.setMax(Integer.parseInt(range[1]));
        }
        job.addPeRange(apeRange);
    }
    
    // [-q wc_queue_list]                       bind job to queue(s)
    @OptionAnnotation(value = "-q", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setQueueList(final OptionInfo oi) throws JGDIException {
        for (String queue : oi.getArgs()) {
            if (hard) {
                job.addHardQueue(queue);
            } else {
                job.addSoftQueue(queue);
            }
        }
        oi.optionDone();
    }
    
    private void clear() {
        job = new JobImpl(false);
        jt = new JobTaskImpl(false);
    }
    
// [-R y[es]|n[o]]                          reservation desired
    @OptionAnnotation(value = "-R")
    public void setReservations(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("reservation desired is not implemented");
    }
    
// [-r y[es]|n[o]]                          define job as (not) restartable
    @OptionAnnotation(value = "-r")
    public void setRestartable(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("restartable is not implemented");
    }
    
// [-sc context_list]                       set job context (replaces old context)
    @OptionAnnotation(value = "-sc", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setJobContext(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("wait for job is not implemented");
    }
    
// [-shell y[es]|n[o]]                      start command with or without wrapping <loginshell> -c
    @OptionAnnotation(value = "-shell")
    public void setShellWrap(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("command wrapping is not implemented");
    }
    
// [-soft]                                  consider following requests as soft
    @OptionAnnotation(value = "-soft", min = 0)
    public void setSoft(final OptionInfo oi) throws JGDIException {
        hard = false;
    }
    
// [-sync y[es]|n[o]]                       wait for job to end and return exit code
    @OptionAnnotation(value = "-sync")
    public void setWait(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("wait for job is not implemented");
    }
    
// [-S path_list]                           command interpreter to be used
    @OptionAnnotation(value = "-S", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setShell(final OptionInfo oi) throws JGDIException {
        job.addShell(new PathNameImpl(oi.getFirstArg()));
    }
    
// [-t task_id_range]                       create a job-array with these tasks
    @OptionAnnotation(value = "-t", extra = OptionAnnotation.MAX_ARG_VALUE)
    public void setJobArray(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("create a job-array is not implemented");
    }
    
// [-terse]                                 tersed output, print only the job-id
    @OptionAnnotation(value = "-terse", min = 0)
    public void setTerse(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("tersed output is not implemented");
    }
    
// [-v variable_list]                       export these environment variables
    @OptionAnnotation(value = "-v", min = 0)
    public void setVariableList(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("export environment variables is not implemented");
    }
    
// [-verify]                                do not submit just verify
    @OptionAnnotation(value = "-verify", min = 0)
    public void setVerify(final OptionInfo oi) throws JGDIException {
        job.setVerify(1);
    }
    
    // [-V]                                     export all environment variables
    @OptionAnnotation(value = "-V")
    public void exportEnvironment(final OptionInfo oi) throws JGDIException {
        throw new UnsupportedOperationException("export env.variables is not implemented");
    }
    
// [-w e|w|n|v]                             verify mode (error|warning|none|just verify) for jobs
    @OptionAnnotation(value = "-w")
    public void setVerifyMode(final OptionInfo oi) throws JGDIException {
        int verMod = Util.getJobVerifyModeAsInt(oi.getFirstArg());
        job.setVerify(verMod);
    }
    
    // [-wd working_directory]                  use working_directory
    @OptionAnnotation(value = "-wd")
    public void setWd(final OptionInfo oi) throws JGDIException {
        job.setCwd(oi.getFirstArg());
    }
    
// [-@ file]                                read commandline input from file
    @OptionAnnotation(value = "-@")
    public void readOptionsFormFile(final OptionInfo oi) throws JGDIException {
        File file = new File(oi.getFirstArg());
        if (!file.isFile()) {
            throw new IllegalStateException("Invalid file");
        }
        throw new IllegalStateException("Read options from a file is not implemented");
        // TODO PJ Implement read options from the file.
    }
// [{command|-} [command_args]]
}