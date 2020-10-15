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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * The JobDescription class represents a job to be verified by the JSV.  It
 * contains the values of all submissions attributes applied to the job as well
 * as the job's environment variables' values if requested.
 *
 * All properties of this class may be null.  A property with a null value
 * indicates that the value was not set by the JSV framework, and hence was not
 * a submission attribute of the job being verified.  Care must be taken with
 * the Boolean and Integer properties to prevent null pointer exceptions caused
 * by autoboxing.  Setting a property value to null deletes the corresponding
 * submission attribute from the job being verified.
 *
 * All properties of Collection type return copies of the Collection from their
 * getter methods.  In order to make changes to the contents of the Collection,
 * the corresponding setter method must be called.  It is acceptable to make
 * the changes to the Collection instance returned from the getter method and
 * then commit those changes by passing that Collection instance to the setter
 * method.
 *
 * The JobDescription class uses the Logger named "com.sun.grid.Jsv".  By
 * default the Logger is set not to pass log records to its parent logger
 * because the default Logger writes to stdout and would disrupt the JSV
 * protocol.  By default, the Logger has no handlers installed.  In order to
 * enable logging, add a handler, i.e. a FileHandler instance.
 *
 * It is always a good idea to enabling logging, as errors in JSV protocol
 * handling will be logged as warnings, but will not be visible any other way.
 * @see JsvManager#requestEnvironment(boolean)
 * @since 6.2u5
 */
public final class JobDescription implements Cloneable, Serializable {
    /**
     * The possible values for the verification property.
     */
    public enum Verification {
        NONE,
        ERROR,
        WARNING,
        VERIFY,
        POKE
    };

    /**
     * String value of the client property that indicates the JSV is running
     * on the master node.
     * @see #getClient()
     */
    public static final String MASTER_CLIENT = "qmaster";
    /**
     * String value of the context property that indicates the JSV is running
     * on the master node.
     * @see #getContext()
     */
    public static final String MASTER_CONTEXT = "master";
    /**
     * String value of the context property that indicates the JSV is running
     * on the master node.
     * @see #getContext()
     */
    public static final String CLIENT_CONTEXT = "client";
    // Job name used when we can't figure out a better one
    private static final String DELETED_BY_JSV = "NO_NAME";
    // -h n
    private static final String NO_HOLD = "n";
    // -h u
    private static final String USER_HOLD = "u";
    // The name of the process running the JSV
    private static final String CLIENT = "CLIENT";
    // The job command
    private static final String COMMAND_NAME = "CMDNAME";
    // Whether the JSV is running on the master or the client
    private static final String CONTEXT = "CONTEXT";
    // Primary group of the job's submitter
    private static final String GROUP = "GROUP";
    // The job id
    private static final String JOB_ID = "JOBID";
    // The username of the job's submitter
    private static final String USER = "USER";
    // The version of the JSV framework
    private static final String VERSION = "VERSION";
    // The number of command args
    private static final String COMMAND_ARGS = "CMDARGS";
    // A specific command arg
    private static final String COMMAND_ARG = "CMDARG";
    // "yes" for boolean attributes, e.g. -b, -r, -R
    private static final String YES = "y";
    // "no" for boolean attributes, e.g. -b, -r, -R
    private static final String NO = "n";
    // -b y|n
    private static final String BINARY = "b";
    // -j y|n
    private static final String MERGE_STREAMS = "j";
    // -notify y|n
    private static final String NOTIFY = "notify";
    // -shell y|n
    private static final String SHELL = "shell";
    // -r y|n
    private static final String RERUNNABLE = "r";
    // -R y|n
    private static final String RESERVATION = "R";
    // -A account
    private static final String ACCOUNT = "A";
    // -ckpt ckptname
    private static final String CHECKPOINT_NAME = "ckpt";
    // -cwd | -wd dir
    private static final String CWD = "cwd";
    // -N name
    private static final String NAME = "N";
    // -P project
    private static final String PROJECT = "P";
    // -i path[,host:path]
    private static final String INPUT_PATH = "i";
    // -o path[,host:path]
    private static final String OUTPUT_PATH = "o";
    // -e path[,host:path]
    private static final String ERROR_PATH = "e";
    // -S path[,host:path]
    private static final String SHELL_PATH = "S";
    // -a date
    private static final String START_TIME = "a";
    // -ac key=value
    private static final String JOB_CONTEXT_ADD = "ac";
    // -dc key=value -- currently unused
    private static final String JOB_CONTEXT_DEL = "dc";
    // -sc key=value -- currently unused
    private static final String JOB_CONTEXT_SET = "sc";
    // -ar id
    private static final String ADVANCE_RESERVATION_ID = "ar";
    // -c interval
    private static final String CHECKPOINT_INTERVAL = "c_interval";
    // -c occasion
    private static final String CHECKPOINT_OCCASION = "c_occasion";
    // -display display
    private static final String DISPLAY = "display";
    // -dl date
    private static final String DEADLINE = "dl";
    // -h u|n
    private static final String HOLD = "h";
    // -hold_jid id[,id]
    private static final String HOLD_JOB_IDS = "hold_jid";
    // -hold_jid_ad id[,id]
    private static final String HOLD_ARRAY_JOB_IDS = "hold_jid_ad";
    // -js share
    private static final String JOB_SHARE = "js";
    // -hard -l key=value
    private static final String HARD_RESOURCE_REQUIREMENTS = "l_hard";
    // -soft -l key=value
    private static final String SOFT_RESOURCE_REQUIREMENTS = "l_soft";
    // -m occasion
    private static final String MAIL = "m";
    // -masterq queue[,queue]
    private static final String MASTER_QUEUE = "masterq";
    // -hard -q queue[,queue]
    private static final String HARD_QUEUE = "q_hard";
    // -soft -q queue[,queue]
    private static final String SOFT_QUEUE = "q_soft";
    // -M email[,email]
    private static final String MAIL_RECIPIENTS = "M";
    // -p priority
    private static final String PRIORITY = "p";
    // -pe name min-max
    private static final String PE_NAME = "pe_name";
    // -pe name min-max
    private static final String PE_MIN = "pe_min";
    // -pe name min-max
    private static final String PE_MAX = "pe_max";
    // -binding type strategy:amount...
    private static final String BINDING_STRATEGY = "binding_strategy";
    // -binding type strategy:amount...
    private static final String BINDING_TYPE = "binding_type";
    // -binding type strategy:amount...
    private static final String BINDING_AMOUNT = "binding_amount";
    // -binding type strategy:amount:socket:core
    private static final String BINDING_SOCKET = "binding_socket";
    // -binding type strategy:amount:socket:core
    private static final String BINDING_CORE = "binding_core";
    // -binding type strategy:amount:step:socket:core
    private static final String BINDING_STEP = "binding_step";
    // -binding type strategy:socket_core_list
    private static final String BINDING_EXP_N = "binding_exp_n";
    // -binding type strategy:socket_core_list
    private static final String BINDING_EXP_SOCKET = "binding_exp_socket";
    // -binding type strategy:socket_core_list
    private static final String BINDING_EXP_CORE = "binding_exp_core";
    // -t min-max:step
    private static final String TASK_MIN = "t_min";
    // -t min-max:step
    private static final String TASK_MAX = "t_max";
    // -t min-max:step
    private static final String TASK_STEP = "t_step";
    // -w verification
    private static final String VERIFICATION = "w";
    private String client = null;
    private String commandName = null;
    private String context = null;
    private String group = null;
    private String jobId = null;
    private String user = null;
    private String version = null;
    private String[] commandArgs = null;
    private Boolean binary = null;
    private Boolean mergeStreams = null;
    private Boolean notify = null;
    private Boolean shell = null;
    private Boolean rerunnable = null;
    private Boolean reservation = null;
    private String account = null;
    private String cwd = null;
    private String name = null;
    private String project = null;
    private Map<String, String> inputPath = null;
    private Map<String, String> errorPath = null;
    private Map<String, String> outputPath = null;
    private Map<String, String> shellPath = null;
    private Calendar startTime = null;
    private Map<String, String> newContext = new HashMap<String, String>();
    private Integer advanceReservationId = null;
    private CheckpointSpecifier checkpointSpecifier = null;
    private String display = null;
    private Calendar deadline = null;
    private Boolean hold = null;
    private List<String> holdJobIds = new ArrayList<String>();
    private List<String> holdArrayJobIds = new ArrayList<String>();
    private Integer jobShare = null;
    private Map<String, String> hardResourceRequirements = new HashMap<String, String>();
    private Map<String, String> softResourceRequirements = new HashMap<String, String>();
    private MailSpecifier mailSpecifier = null;
    private List<String> masterQueue = new ArrayList<String>();
    private List<String> softQueue = new ArrayList<String>();
    private List<String> hardQueue = new ArrayList<String>();
    private List<String> mailRecipients = new ArrayList<String>();
    private Integer priority = null;
    private ParallelEnvironment pe = null;
    private BindingSpecifier binding = null;
    private TaskSpecifier taskSpecifier = null;
    private Verification verification = null;
    private Map<String,String> environment = null;
    private transient JobDescription baseline = null;
    private static final Logger log = Logger.getLogger("com.sun.grid.Jsv");
    private static final Pattern pathPattern = Pattern.compile("^(([^:]+)?:)?([^:]+)$");
    private static final Pattern mapPattern = Pattern.compile("^([^=]+)=([^=]+)$");
    private static final Pattern timePattern = Pattern.compile("((\\d\\d)?\\d\\d)?(\\d\\d)(\\d\\d)(\\d\\d)(\\d\\d)(\\.(\\d\\d))?");

    /**
     * Get the account string.
     *
     * See qsub -A.
     * @return the account string
     */
    public String getAccount() {
        return account;
    }

    /**
     * Get the advance reservation id.
     *
     * Note that the return value will be null
     * if the advance reservation id has not been set.
     *
     * See qsub -ar.
     * @return the advance reservation id
     */
    public Integer getAdvanceReservationId() {
        return advanceReservationId;
    }

    /**
     * Get an object representing the checkpointing options.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     * @return the checkpointing options
     *
     * See qsub -ckpt -c.
     * @see CheckpointSpecifier
     */
    public CheckpointSpecifier getCheckpointSpecifier() {
        CheckpointSpecifier retValue = null;

        if (checkpointSpecifier != null) {
            retValue = checkpointSpecifier.clone();
        }

        return retValue;
    }

    /**
     * Get the client string.  If the JSV is running on the master node, the
     * value of this string will be "qmaster".  If the JSV is running on the
     * submission client node, the value of this string will be the name of
     * the submission utility, e.g. qsub, qrsh, etc.
     * @return the client string
     * @see #MASTER_CLIENT
     */
    public String getClient() {
        return client;
    }

    /**
     * Get the arguments to the job.
     * 
     * The array that is returned will be a copy
     * of the internal command arguments array, so modifications to the
     * returned value will have no effect on the JobDescription instance.
     * @return the job arguments
     */
    public String[] getCommandArgs() {
        String[] retValue = null;

        if (commandArgs != null) {
            retValue = new String[commandArgs.length];
            System.arraycopy(commandArgs, 0, retValue, 0, commandArgs.length);
        }

        return retValue;
    }

    /**
     * Get the job command.
     * @return the job command
     */
    public String getCommandName() {
        return commandName;
    }

    /**
     * Get the context string.  If the JSV is running on the master node, the
     * value of this string will be "master".  If the JSV is running on the
     * submission client node, the value of this string will be "client".
     * @return the context string
     * @see #MASTER_CONTEXT
     * @see #CLIENT_CONTEXT
     */
    public String getContext() {
        return context;
    }

    /**
     * Get the working directory for the job
     *
     * See qsub -cwd -wd.
     * @return the working directory
     */
    public String getWorkingDirectory() {
        return cwd;
    }

    /**
     * Get the job's deadline time.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -dl.
     * @return the deadline time
     */
    public Calendar getDeadline() {
        Calendar retValue = null;

        if (deadline != null) {
            retValue = (Calendar) deadline.clone();
        }

        return retValue;
    }

    /**
     * Get the job's display value.
     *
     * See qsub -display.
     * @return the display value
     */
    public String getDisplay() {
        return display;
    }

    /**
     * Get the job's error path.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -e.
     * @return the error path
     */
    public Map<String, String> getErrorPath() {
        Map<String,String> retValue = null;

        if (errorPath != null) {
            retValue = new HashMap<String,String>(errorPath);
        }

        return retValue;
    }

    /**
     * Get the primary group id for the job's submitter.
     * @return the group id
     */
    public String getGroup() {
        return group;
    }

    /**
     * Get the hard queue list.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -hard -q.
     * @return the hard queue list
     */
    public List<String> getHardQueue() {
        List<String> retValue = null;

        if (hardQueue != null) {
            retValue = new LinkedList<String>(hardQueue);
        }

        return retValue;
    }

    /**
     * Get the hard resource requirements.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -hard -l.
     * @return the hard resource requirements
     */
    public Map<String, String> getHardResourceRequirements() {
        Map<String,String> retValue = null;

        if (hardResourceRequirements != null) {
            retValue = new HashMap<String,String>(hardResourceRequirements);
        }

        return retValue;
    }

    /**
     * Get the list of array job identifiers on which this job's array tasks are
     * dependent.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -hold_jid_ad.
     * @return the array job dependencies
     */
    public List<String> getHoldArrayJobIds() {
        List<String> retValue = null;

        if (holdArrayJobIds != null) {
            retValue = new LinkedList<String>(holdArrayJobIds);
        }

        return retValue;
    }

    /**
     * Get the list of job identifiers on which this job is dependent.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -hold_jid.
     * @return the job dependencies
     */
    public List<String> getHoldJobIds() {
        List<String> retValue = null;

        if (holdJobIds != null) {
            retValue = new LinkedList<String>(holdJobIds);
        }

        return retValue;
    }

    /**
     * Get the job's input path.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -i.
     * @return the input path
     */
    public Map<String, String> getInputPath() {
        Map<String,String> retValue = null;

        if (inputPath != null) {
            retValue = new HashMap<String,String>(inputPath);
        }

        return retValue;
    }

    /**
     * Get the job's job context map.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -ac -dc -sc.
     * @return the job context
     */
    public Map<String, String> getJobContext() {
        Map<String,String> retValue = null;

        if (newContext != null) {
            retValue = new HashMap<String,String>(newContext);
        }

        return retValue;
    }

    /**
     * Get the job's id
     * @return the job id
     */
    public String getJobId() {
        return jobId;
    }

    /**
     * Get the job's assigned functional ticket share.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -js.
     * @return the job share
     */
    public Integer getJobShare() {
        return jobShare;
    }

    /**
     * Get the list of email recipients for this job.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -M.
     * @return the email recipient list
     */
    public List<String> getMailRecipients() {
        List<String> retValue = null;

        if (mailRecipients != null) {
            retValue = new LinkedList<String>(mailRecipients);
        }

        return retValue;
    }

    /**
     * Get a MailSpecifier object that represents the occasions when email
     * notifications should be sent for this job.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -m.
     * @return the mail specifier
     * @see MailSpecifier
     */
    public MailSpecifier getMailSpecifier() {
        MailSpecifier retValue = null;

        if (mailSpecifier != null) {
            retValue = mailSpecifier.clone();
        }

        return retValue;
    }

    /**
     * Get the master queue list.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -masterq.
     * @return the master queue list
     */
    public List<String> getMasterQueue() {
        List<String> retValue = null;

        if (masterQueue != null) {
            retValue = new LinkedList<String>(masterQueue);
        }

        return retValue;
    }

    /**
     * Get the job's name.
     * @return the job name
     */
    public String getName() {
        return name;
    }

    /**
     * Get the job's output path.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -o.
     * @return the output path
     */
    public Map<String, String> getOutputPath() {
        Map<String,String> retValue = null;

        if (outputPath != null) {
            retValue = new HashMap<String,String>(outputPath);
        }

        return retValue;
    }

    /**
     * Get the ParallelEnvironment object that represents the parallel
     * environment settings for the job.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -pe.
     * @return the pe specification
     * @see ParallelEnvironment
     */
    public ParallelEnvironment getParallelEnvironment() {
        ParallelEnvironment retValue = null;

        if (pe != null) {
            retValue = pe.clone();
        }

        return retValue;
    }

    /**
     * Get the BindingSpecifier object that represents the core binding
     * settings for the job.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -binding.
     * @return the binding specification
     * @see BindingSpecifier
     */
    public BindingSpecifier getBindingSpecifier() {
        BindingSpecifier retValue = null;

        if (binding != null) {
            retValue = binding.clone();
        }

        return retValue;
    }

    /**
     * Get the job's priority.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -p.
     * @return the job priority
     */
    public Integer getPriority() {
        return priority;
    }

    /**
     * Get the job's project
     *
     * See qsub -P.
     * @return the job project
     */
    public String getProject() {
        return project;
    }

    /**
     * Get the shell path list.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -S.
     * @return the shell path list
     */
    public Map<String, String> getShellPath() {
        Map<String,String> retValue = null;

        if (shellPath != null) {
            retValue = new HashMap<String,String>(shellPath);
        }

        return retValue;
    }

    /**
     * Get the soft queue list.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -soft -q.
     * @return the soft queue list
     */
    public List<String> getSoftQueue() {
        List<String> retValue = null;

        if (softQueue != null) {
            retValue = new LinkedList<String>(softQueue);
        }

        return retValue;
    }

    /**
     * Get the soft resource requirements map.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -soft -l.
     * @return the soft requirements map
     */
    public Map<String, String> getSoftResourceRequirements() {
        Map<String,String> retValue = null;

        if (softResourceRequirements != null) {
            retValue = new HashMap<String,String>(softResourceRequirements);
        }

        return retValue;
    }

    /**
     * Get the job's start time.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -a.
     * @return the start time
     */
    public Calendar getStartTime() {
        Calendar retValue = null;

        if (startTime != null) {
            retValue = (Calendar) startTime.clone();
        }

        return retValue;
    }

    /**
     * Get the object that represents the array task specifier.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -t.
     * @return the array task specifier
     * @see TaskSpecifier
     */
    public TaskSpecifier getTaskSpecifier() {
        TaskSpecifier retValue = null;

        if (taskSpecifier != null) {
            retValue = taskSpecifier.clone();
        }

        return retValue;
    }

    /**
     * Get the submitting user.
     * @return the submitting user
     */
    public String getUser() {
        return user;
    }

    /**
     * Get the verification level.
     *
     * See qsub -w.
     * @return the verification level
     */
    public Verification getVerification() {
        return verification;
    }

    /**
     * Get the JSV framework version.
     * @return the JSV version
     */
    public String getVersion() {
        return version;
    }

    /**
     * Get whether the job should be launched via a shell.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -shell.
     * @return whether to use a shell
     */
    public Boolean useShell() {
        return shell;
    }

    /**
     * Get whether the job uses resource reservation.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -R.
     * @return whether to use resource reservation
     */
    public Boolean hasResourceReservation() {
        return reservation;
    }

    /**
     * Get whether the job can be requeued during execd failure.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -r.
     * @return whether the job can be requeued
     */
    public Boolean isRerunnable() {
        return rerunnable;
    }

    /**
     * Get whether the job should be sent warning signals before being suspended
     * or terminated.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -notify.
     * @return whether the job should get warning signals
     */
    public Boolean doNotify() {
        return notify;
    }

    /**
     * Get whether the output and error streams should be written into the same
     * file.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -j.
     * @return whether to merge the streams
     */
    public Boolean mergeStreams() {
        return mergeStreams;
    }

    /**
     * Get whether the job is submitted in the hold state.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -h.
     * @return whether the job is submitted in the hold state
     */
    public Boolean onHold() {
        return hold;
    }

    /**
     * Get whether the job is binary.  A true value means the job is treated as
     * a binary.  A false value means the job is treated as a script.
     *
     * Note that the return value will be null if the value has not been set.
     *
     * See qsub -b.
     * @return whether the job is binary
     */
    public Boolean isBinary() {
        return binary;
    }

    /**
     * Get the job's environment variables.
     *
     * The object that is returned will be a copy
     * of the internal object, so modifications to the
     * returned object will have no effect on the JobDescription instance.
     *
     * See qsub -v.
     * @return the environment variables
     */
    public Map<String,String> getEnvironment() {
        Map<String,String> retValue = null;

        if (environment != null) {
            retValue = new HashMap<String,String>(environment);
        }

        return retValue;
    }

    /**
     * Set the value of the account string.
     *
     * See qsub -A.
     * @param account the account string
     */
    public void setAccount(String account) {
        this.account = account;
    }

    /**
     * Set the value of the advance reservation id.
     *
     * See qsub -ar.
     * @param advanceReservationId the advance reservation id
     */
    public void setAdvanceReservationId(Integer advanceReservationId) {
        if ((advanceReservationId != null) && (advanceReservationId < 0)) {
            throw new IllegalArgumentException("attempted to set a negative advance reservation id value");
        }

        this.advanceReservationId = advanceReservationId;
    }

    /**
     * Set whether the job is binary.
     *
     * See qsub -b.
     * @param binary whether the job is binary
     */
    public void setBinary(Boolean binary) {
        this.binary = binary;
    }

    /**
     * Set the CheckpointSpecifier object that defines the checkpointing
     * settings for the job.
     *
     * See qsub -ckpt -c.
     * @param checkpointSpecifier the schpoint specifier
     * @see CheckpointSpecifier
     */
    public void setCheckpointSpecifier(CheckpointSpecifier checkpointSpecifier) {
        if ((checkpointSpecifier != null) && (checkpointSpecifier.getName() == null)) {
            throw new IllegalArgumentException("attempted to set invalid checkpoint specifier");
        } else if (checkpointSpecifier != null) {
            this.checkpointSpecifier = checkpointSpecifier.clone();
        } else {
            this.checkpointSpecifier = null;
        }
    }

    /**
     * Set the client string.  The client string cannot be set after the
     * baseline has been set and cannot be set to null.
     * @param client the client string
     */
    void setClient(String client) {
        if (baseline != null) {
            throw new IllegalStateException("attempted to modify fixed value");
        }

        if (client == null) {
            log.warning("Ignoring null client value");
        } else {
            this.client = client;
        }
    }

    /**
     * Set the job's command arguments.
     * @param commandArgs the new command arguments
     */
    public void setCommandArgs(String[] commandArgs) {
        if (commandArgs != null) {
            if (Arrays.asList(commandArgs).contains(null)) {
                throw new IllegalArgumentException("attempted to set an arg array with a null value");
            }

            if ((this.commandArgs == null) ||
                    (this.commandArgs.length != commandArgs.length)) {
                this.commandArgs = new String[commandArgs.length];
            }

            System.arraycopy(commandArgs, 0, this.commandArgs, 0, commandArgs.length);
        } else {
            this.commandArgs = new String[0];
        }
    }

    /**
     * Set the job's command.  The command name cannot be set after the
     * baseline has been set and cannot be set to null.
     * @param commandName the job command
     */
    void setCommandName(String commandName) {
        if (baseline != null) {
            throw new IllegalStateException("attempted to modify fixed value");
        }

        if (commandName == null) {
            log.warning("Ignoring null client name value");
        } else {
            this.commandName = commandName;
        }
    }

    /**
     * Set the job's client context.  The client context cannot be set after the
     * baseline has been set and cannot be set to null.
     * @param context the client context
     */
    void setContext(String context) {
        if (baseline != null) {
            throw new IllegalStateException("attempted to modify fixed value");
        }

        if (context == null) {
            log.warning("Ignoring null context value");
        } else {
            this.context = context;
        }
    }

    /**
     * Set the working directory.
     *
     * See qsub -wd -cwd.
     * @param cwd the working directory
     */
    public void setWorkingDirectory(String cwd) {
        this.cwd = cwd;
    }

    /**
     * Set the deadline time.
     *
     * See qsub -dl.
     * @param deadline the deadline time
     */
    public void setDeadline(Calendar deadline) {
        if (deadline != null) {
            this.deadline = (Calendar) deadline.clone();
        } else {
            this.deadline = null;
        }
    }

    /**
     * Set the display string.
     *
     * See qsub -display.
     * @param display the display string
     */
    public void setDisplay(String display) {
        this.display = display;
    }

    /**
     * Set the error path.
     *
     * See qsub -e.
     * @param path the error path
     */
    public void setErrorPath(Map<String, String> path) {
        if ((path != null) && (path.size() > 0)) {
            if (path.containsValue(null)) {
                throw new IllegalArgumentException("attempted to set path list with null entry values");
            }

            if (errorPath != null) {
                errorPath.clear();
            } else {
                errorPath = new HashMap<String,String>();
            }

            errorPath.putAll(path);
        } else {
            errorPath = null;
        }
    }

    /**
     * Set the job submitter's primary group.  The group cannot be set after
     * the baseline has been set and cannot be set to null.
     * @param group
     */
    void setGroup(String group) {
        if (baseline != null) {
            throw new IllegalStateException("attempted to modify fixed value");
        }

        if (group == null) {
            log.warning("Ignoring null group value");
        } else {
            this.group = group;
        }
    }

    /**
     * Set the hard queue list.
     *
     * See qsub -hard -q.
     * @param queues the hard queue list
     */
    public void setHardQueue(List<String> queues) {
        if ((queues != null) && (queues.size() > 0)) {
            if (queues.contains(null)) {
                throw new IllegalArgumentException("attempted to set queue list with null entries");
            }

            if (hardQueue != null) {
                hardQueue.clear();
            } else {
                hardQueue = new ArrayList<String>(queues.size());
            }

            hardQueue.addAll(queues);
        } else {
            hardQueue = null;
        }
    }

    /**
     * Set the hard resource requirements map.
     *
     * See qsub -hard -l.
     * @param resources the hard resource requirements
     */
    public void setHardResourceRequirements(Map<String, String> resources) {
        if ((resources != null) && (resources.size() > 0)) {
            if (resources.containsKey(null) || resources.containsValue(null)) {
                throw new IllegalArgumentException("attempted to set resource requirements with null entries");
            }

            if (hardResourceRequirements != null) {
                hardResourceRequirements.clear();
            } else {
                hardResourceRequirements = new HashMap<String, String>();
            }

            hardResourceRequirements.putAll(resources);
        } else {
            hardResourceRequirements = null;
        }
    }

    /**
     * Set whether the job should be submitted in hold state.
     *
     * See qsub -h.
     * @param hold whether the job should be submitted in hold state
     */
    public void setHold(Boolean hold) {
        this.hold = hold;
    }

    /**
     * Set the list of job identifiers on which the job depeneds.
     *
     * See qsub -hold_jid.
     * @param jobIds the job dependency list
     */
    public void setHoldJobIds(List<String> jobIds) {
        if ((jobIds != null) && (jobIds.size() > 0)) {
            if (jobIds.contains(null)) {
                throw new IllegalArgumentException("attempted to set job id list with null entries");
            }

            if (holdJobIds != null) {
                holdJobIds.clear();
            } else {
                holdJobIds = new ArrayList<String>(jobIds.size());
            }

            holdJobIds.addAll(jobIds);
        } else {
            holdJobIds = null;
        }
    }

    /**
     * Set the list of array job identifiers on which the job tasks depened.
     *
     * See qsub -hold_jid_ad.
     * @param jobIds the array job dependency list
     */
    public void setHoldArrayJobIds(List<String> jobIds) {
        if ((jobIds != null) && (jobIds.size() > 0)) {
            if (jobIds.contains(null)) {
                throw new IllegalArgumentException("attempted to set job id list with null entries");
            }

            if (holdArrayJobIds != null) {
                holdArrayJobIds.clear();
            } else {
                holdArrayJobIds = new ArrayList<String>(jobIds.size());
            }

            holdArrayJobIds.addAll(jobIds);
        } else {
            holdArrayJobIds = null;
        }
    }

    /**
     * Set the input path.
     *
     * See qsub -i.
     * @param path the error path
     */
    public void setInputPath(Map<String, String> path) {
        if ((path != null) && (path.size() > 0)) {
            if (path.containsValue(null)) {
                throw new IllegalArgumentException("attempted to set path list with null entry values");
            }

            if (inputPath == null) {
                inputPath = new HashMap<String,String>();
            } else {
                inputPath.clear();
            }

            inputPath.putAll(path);
        } else {
            inputPath = null;
        }
    }

    /**
     * Set the job's id.  The job id cannot be set after the baseline has been
     * set and cannot be set to null.
     * @param jobId the job id
     */
    void setJobId(String jobId) {
        if (baseline != null) {
            throw new IllegalStateException("attempted to modify fixed value");
        }

        if (jobId == null) {
            log.warning("Ignoring null job id value");
        } else {
            this.jobId = jobId;
        }
    }

    /**
     * Set the job's context.
     *
     * See qsub -ac -dc -sc.
     * @param context the job context
     */
    public void setJobContext(Map<String, String> context) {
        if ((context != null) && (context.size() > 0)) {
            if (context.containsValue(null) || context.containsKey(null)) {
                throw new IllegalArgumentException("attempted to set job context with null entries");
            }

            if (newContext != null) {
                newContext.clear();
            } else {
                newContext = new HashMap<String, String>();
            }

            newContext.putAll(context);
        } else {
            newContext = null;
        }
    }

    /**
     * Set the job's functional ticket share
     *
     * See qsub -js.
     * @param jobShare the job share
     */
    public void setJobShare(Integer jobShare) {
        if ((jobShare != null) && (jobShare < 0)) {
            throw new IllegalArgumentException("attempted to set a negative job share value");
        }

        this.jobShare = jobShare;
    }

    /**
     * Set the list of email recipients to be notified on job events.
     *
     * See qsub -M.
     * @param recipients the email list
     */
    public void setMailRecipients(List<String> recipients) {
        if ((recipients != null) && (recipients.size() > 0)) {
            if (recipients.contains(null)) {
                throw new IllegalArgumentException("attempted to set mail list with null entries");
            }

            if (mailRecipients != null) {
                mailRecipients.clear();
            } else {
                mailRecipients = new ArrayList<String>(recipients.size());
            }

            mailRecipients.addAll(recipients);
        } else {
            mailRecipients = null;
        }
    }

    /**
     * Set the MailSpecifier object that represents when email notifications
     * should be sent.
     *
     * See qsub -m.
     * @param mailSpecifier
     * @see MailSpecifier
     */
    public void setMailSpecifier(MailSpecifier mailSpecifier) {
        if (mailSpecifier != null) {
            this.mailSpecifier = mailSpecifier.clone();
        } else {
            this.mailSpecifier = null;
        }
    }

    /**
     * Set the master queue list.
     *
     * See qsub -masterq.
     * @param queues the master queue list
     */
    public void setMasterQueue(List<String> queues) {
        if ((queues != null) && (queues.size() > 0)) {
            if (queues.contains(null)) {
                throw new IllegalArgumentException("attempted to set queue list with null entries");
            }

            if (masterQueue != null) {
                masterQueue.clear();
            } else {
                masterQueue = new ArrayList<String>(queues.size());
            }

            masterQueue.addAll(queues);
        } else {
            masterQueue = null;
        }
    }

    /**
     * Set whether the output and error streams should be merged.
     *
     * See qsub -j.
     * @param mergeStreams whether to merge the streams
     */
    public void setMergeStreams(Boolean mergeStreams) {
        this.mergeStreams = mergeStreams;
    }

    /**
     * Set the job's name.
     *
     * See qsub -N.
     * @param name the job name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Set whether the job should be sent warning signals before being
     * suspended or terminated.
     *
     * See qsub -notify.
     * @param notify whether the job shoud receive warning signals
     */
    public void setNotify(Boolean notify) {
        this.notify = notify;
    }

    /**
     * Set the output path.
     *
     * See qsub -o.
     * @param path the output path
     */
    public void setOutputPath(Map<String, String> path) {
        if ((path != null) && (path.size() > 0)) {
            if (path.containsValue(null)) {
                throw new IllegalArgumentException("attempted to set path list with null entry values");
            }

            if (outputPath == null) {
                outputPath = new HashMap<String,String>();
            } else {
                outputPath.clear();
            }

            outputPath.putAll(path);
        } else {
            outputPath = null;
        }
    }

    /**
     * Set the ParallelEnvironment object that represent's the job's PE
     * settings.
     *
     * See qsub -pe.
     * @param pe the job PE
     * @see ParallelEnvironment
     */
    public void setParallelEnvironment(ParallelEnvironment pe) {
        if ((pe != null) && (pe.getName() == null)) {
            throw new IllegalArgumentException("attempted to set invalid parallel environment");
        } else if (pe != null) {
            this.pe = pe;
        } else {
            this.pe = null;
        }
    }

    /**
     * Set the BindingSpecifier object that represents the job to core binding
     * settings.
     *
     * See qsub -binding.
     * @param binding the job binding
     * @see BindingSpecifier
     */
    public void setBindingSpecifier(BindingSpecifier binding) {
        // null is allowed to remove the binding
        this.binding = binding;
    }

    /**
     * Set the job's priority.
     *
     * See qsub -p.
     * @param priority the job priority
     */
    public void setPriority(Integer priority) {
        if ((priority != null) && (priority < 0)) {
            throw new IllegalArgumentException("attempted to set a negative priority value");
        }

        this.priority = priority;
    }

    /**
     * Set the job's project.
     *
     * See qsub -P.
     * @param project the job project
     */
    public void setProject(String project) {
        this.project = project;
    }

    /**
     * Set whether the job can be requeued in the event of execd failure.
     *
     * See qsub -R.
     * @param rerunnable whether the job can be requeued
     */
    public void setRerunnable(Boolean rerunnable) {
        this.rerunnable = rerunnable;
    }

    /**
     * Set whether the job should use resource reservation.
     *
     * See qsub -r.
     * @param reservation whether the job should use resource reservation
     */
    public void setResourceReservation(Boolean reservation) {
        this.reservation = reservation;
    }

    /**
     * Set whether the job should be launched by a shell.
     *
     * See qsub -shell.
     * @param shell whether the job should use a shell
     */
    public void setShell(Boolean shell) {
        this.shell = shell;
    }

    /**
     * Set the job's shell path.
     *
     * See qsub -S.
     * @param path the shell path
     */
    public void setShellPath(Map<String, String> path) {
        if ((path != null) && (path.size() > 0)) {
            if (path.containsValue(null)) {
                throw new IllegalArgumentException("attempted to set path list with null entry values");
            }

            if (shellPath != null) {
                shellPath.clear();
            } else {
                shellPath = new HashMap<String, String>();
            }

            shellPath.putAll(path);
        } else {
            shellPath = null;
        }
    }

    /**
     * Set the job's soft queue list
     *
     * See qsub -soft -q.
     * @param queues the soft queue list
     */
    public void setSoftQueue(List<String> queues) {
        if ((queues != null) && (queues.size() > 0)) {
            if (queues.contains(null)) {
                throw new IllegalArgumentException("attempted to set queue list with null entries");
            }

            if (softQueue != null) {
                softQueue.clear();
            } else {
                softQueue = new ArrayList<String>(queues.size());
            }

            softQueue.addAll(queues);
        } else {
            softQueue = null;
        }
    }

    /**
     * Set the job's soft resource requirements map.
     *
     * See qsub -soft -l.
     * @param resources the soft resource requirements
     */
    public void setSoftResourceRequirements(Map<String, String> resources) {
        if ((resources != null) && (resources.size() > 0)) {
            if (resources.containsKey(null) || resources.containsValue(null)) {
                throw new IllegalArgumentException("attempted to set resource requirements with null entries");
            }

            if (softResourceRequirements != null) {
                softResourceRequirements.clear();
            } else {
                softResourceRequirements = new HashMap<String, String>();
            }

            softResourceRequirements.putAll(resources);
        } else {
            softResourceRequirements = null;
        }
    }

    /**
     * Set the job's start time.
     *
     * See qsub -a.
     * @param startTime the start time
     */
    public void setStartTime(Calendar startTime) {
        if (startTime != null) {
            this.startTime = (Calendar) startTime.clone();
        } else {
            this.startTime = null;
        }
    }

    /**
     * Set the TaskSpecifier object that defines the job's array task settings.
     *
     * See qsub -t.
     * @param taskSpecifier the task specifier
     * @see TaskSpecifier
     */
    public void setTaskSpecifier(TaskSpecifier taskSpecifier) {
        if (taskSpecifier != null) {
            this.taskSpecifier = taskSpecifier.clone();
        } else {
            this.taskSpecifier = null;
        }
    }

    /**
     * Set the job's submitting user name.  The user cannot be set after the
     * baseline has been set and cannot be set to null.
     * @param user
     */
    void setUser(String user) {
        if (baseline != null) {
            throw new IllegalStateException("attempted to modify fixed value");
        }

        if (user == null) {
            log.warning("Ingoring null user value");
        } else {
            this.user = user;
        }
    }

    /**
     * Set the job's verification level.
     *
     * See qsub -w.
     * @param verification the verification level
     */
    public void setVerification(Verification verification) {
        this.verification = verification;
    }

    /**
     * Set the JSV framework's version.  The version cannot be set after the
     * baseline has been set and cannot be set to null.
     * @param version the JSV version
     */
    void setVersion(String version) {
        if (baseline != null) {
            throw new IllegalStateException("attempted to modify fixed value");
        }

        if (version == null) {
            log.warning("Ignoring null version value");
        } else {
            this.version = version;
        }
    }

    /**
     * Set the job's environment variable.
     *
     * See qsub -v.
     * @param environment the environment variables
     */
    public void setEnvironment(Map<String,String> environment) {
        if (environment != null) {
            if (environment.containsKey(null) || environment.containsValue(null)) {
                throw new IllegalArgumentException("attempted to set environment with null entries");
            }

            if (this.environment != null) {
                this.environment.clear();
            } else {
                this.environment = new HashMap<String, String>();
            }

            this.environment.putAll(environment);
        } else {
            this.environment = null;
        }
    }

    /**
     * Create a baseline from the current job description settings so that
     * later the then current settings can be compared against the settings in
     * the baseline.  The baseline can be set more than once, with each new
     * baseline replacing the last.
     */
    void setBaseline() {
        baseline = this.clone();
    }

    /**
     * Compare the current settings against the baseline and return a map of
     * the differences.  The map's keys are parameters and
     * pseudo-parameters that are understood by the JSV framework.  This method
     * should not be called before the setBaseline() method has been called.
     * @return the differences between the current job description settings and
     * the baseline in terms of the JSV framework's submit parameters and
     * pseudo-parameters
     * @see #setBaseline()
     */
    Map<String, String> getDifference() {
        HashMap<String, String> ret = new HashMap<String, String>();

        if (baseline != null) {
            putBooleanDifference(ret, baseline.binary, binary, BINARY, YES, NO, NO);
            putBooleanDifference(ret, baseline.mergeStreams, mergeStreams, MERGE_STREAMS, YES, NO, NO);
            putBooleanDifference(ret, baseline.notify, notify, NOTIFY, YES, NO, NO);
            // The default -shell is yes
            putBooleanDifference(ret, baseline.shell, shell, SHELL, YES, NO, YES);
            putBooleanDifference(ret, baseline.rerunnable, rerunnable, RERUNNABLE, YES, NO, NO);
            putBooleanDifference(ret, baseline.reservation, reservation, RESERVATION, YES, NO, NO);
            putDifference(ret, baseline.account, account, ACCOUNT);
            putDifference(ret, baseline.cwd, cwd, CWD);
            putDifference(ret, baseline.project, project, PROJECT);
            putMapDifference(ret, baseline.inputPath, inputPath, INPUT_PATH);
            putMapDifference(ret, baseline.outputPath, outputPath, OUTPUT_PATH);
            putMapDifference(ret, baseline.errorPath, errorPath, ERROR_PATH);
            putMapDifference(ret, baseline.shellPath, shellPath, SHELL_PATH);
            putTimeDifference(ret, baseline.startTime, startTime, START_TIME);
            putDifference(ret, baseline.advanceReservationId, advanceReservationId, ADVANCE_RESERVATION_ID);
            putDifference(ret, baseline.display, display, DISPLAY);
            putTimeDifference(ret, baseline.deadline, deadline, DEADLINE);
            putBooleanDifference(ret, baseline.hold, hold, HOLD, USER_HOLD, NO_HOLD);
            putListDifference(ret, baseline.holdJobIds, holdJobIds, HOLD_JOB_IDS);
            putListDifference(ret, baseline.holdArrayJobIds, holdArrayJobIds, HOLD_ARRAY_JOB_IDS);
            putMapDifference(ret, baseline.newContext, newContext, JOB_CONTEXT_ADD);
            putDifference(ret, baseline.jobShare, jobShare, JOB_SHARE);
            putMapDifference(ret, baseline.hardResourceRequirements, hardResourceRequirements, HARD_RESOURCE_REQUIREMENTS);
            putMapDifference(ret, baseline.softResourceRequirements, softResourceRequirements, SOFT_RESOURCE_REQUIREMENTS);
            putListDifference(ret, baseline.masterQueue, masterQueue, MASTER_QUEUE);
            putListDifference(ret, baseline.softQueue, softQueue, SOFT_QUEUE);
            putListDifference(ret, baseline.hardQueue, hardQueue, HARD_QUEUE);
            putListDifference(ret, baseline.mailRecipients, mailRecipients, MAIL_RECIPIENTS);
            putDifference(ret, baseline.priority, priority, PRIORITY);
            putCmdArgsDifference(ret, baseline.commandArgs, commandArgs);
            putCheckpointDifference(ret, baseline.checkpointSpecifier, checkpointSpecifier);
            putMailDifference(ret, baseline.mailSpecifier, mailSpecifier);
            putPeDifference(ret, baseline.pe, pe);
            putBindingDifference(ret, baseline.binding, binding);
            putTaskDifference(ret, baseline.taskSpecifier, taskSpecifier);
            putVerificationDifference(ret, baseline.verification, verification);
            putNameDifference(ret, baseline.name, name, commandName);
        } else {
            log.warning("Tried to compare job description without a baseline");
        }

        return ret;
    }

    /**
     * Compare the two objects.  If they are different, add the difference to
     * the map under the given key.
     * @param map the difference map
     * @param oldObject the old setting
     * @param newObject the new setting
     * @param name the map key
     */
    private static void putDifference(Map<String,String> map, Object oldObject, Object newObject, String name) {
            if ((newObject == null) && (oldObject != null)) {
                map.put(name, null);
            } else if ((newObject != null) && !newObject.equals(oldObject)) {
                map.put(name, newObject.toString());
            }
    }

    /**
     * Compare the two objects.  If they are different, add the difference to
     * the map under the given key.  This method is currently unused.
     * @param map the difference map
     * @param oldObject the old setting
     * @param newObject the new setting
     * @param name the map key
     * @param defaultValue the default value to use if the new object is null
     */
    private static void putDifference(Map<String,String> map, Object oldObject, Object newObject, String name, String defaultValue) {
            if ((newObject == null) && (oldObject != null)) {
                map.put(name, defaultValue);
            } else if ((newObject != null) && !newObject.equals(oldObject)) {
                map.put(name, newObject.toString());
            }
    }

    /**
     * Compare the two boolean objects.  If they are different, add the
     * difference to the map under the given key.
     * @param map the difference map
     * @param oldBool the old setting
     * @param newBool the new setting
     * @param name the map key
     * @param trueValue the String to represent true
     * @param falseValue the String to represent false
     */
    private static void putBooleanDifference(Map<String,String> map, Boolean oldBool, Boolean newBool, String name, String trueValue, String falseValue) {
        if ((newBool == null) && (oldBool != null)) {
            map.put(name, null);
        } else if ((newBool != null) && !newBool.equals(oldBool)) {
            map.put(name, newBool ? trueValue : falseValue);
        }
    }

    /**
     * Compare the two boolean objects.  If they are different, add the
     * difference to the map under the given key.
     * @param map the difference map
     * @param oldBool the old setting
     * @param newBool the new setting
     * @param name the map key
     * @param trueValue the String to represent true
     * @param falseValue the String to represent false
     * @param defaultValue the default value to use if the new object is null
     */
    private static void putBooleanDifference(Map<String,String> map, Boolean oldBool, Boolean newBool, String name, String trueValue, String falseValue, String defaultValue) {
        if ((newBool == null) && (oldBool != null)) {
            map.put(name, defaultValue);
        } else if ((newBool != null) && !newBool.equals(oldBool)) {
            map.put(name, newBool ? trueValue : falseValue);
        }
    }

    /**
     * Compare the two lists.  If they are different, add the difference to
     * the map under the given key.
     * @param map the difference map
     * @param oldList the old setting
     * @param newList the new setting
     * @param name the map key
     */
    private static void putListDifference(Map<String,String> map, List<? extends Object> oldList, List<? extends Object> newList, String name) {
        if (isListDifferent(newList, oldList)) {
            map.put(name, listToString(newList));
        }
    }

    /**
     * Compare the two maps.  If they are different, add the difference to
     * the map under the given key.
     * @param map the difference map
     * @param oldMap the old setting
     * @param newMap the new setting
     * @param name the map key
     */
    private static void putMapDifference(Map<String,String> map, Map<String,String> oldMap, Map<String,String> newMap, String name) {
        if (isMapDifferent(newMap, oldMap)) {
            map.put(name, mapToString(newMap));
        }
    }

    /**
     * Compare the two Calendar instances.  If they are different, add the
     * difference to the map under the given key.
     * @param map the difference map
     * @param oldTime the old setting
     * @param newTime the new setting
     * @param name the map key
     */
    private static void putTimeDifference(Map<String,String> map, Calendar oldTime, Calendar newTime, String name) {
            if (((newTime == null) && (oldTime != null)) ||
                    ((newTime != null) && !newTime.equals(oldTime))) {
                map.put(name, formatTime(newTime));
            }
    }

    /**
     * Compare the two command argument arrays.  If they are different, add the
     * difference to the map.
     * @param map the difference map
     * @param oldArgs the old setting
     * @param newArgs the new setting
     */
    private static void putCmdArgsDifference(Map<String,String> map, String[] oldArgs, String[] newArgs) {
        if (newArgs != null) {
            if ((oldArgs == null) ||
                    (newArgs.length != oldArgs.length)) {
                map.put(COMMAND_ARGS, Integer.toString(newArgs.length));
            }

            int i = 0;

            if (oldArgs != null) {
                int end = Math.min(oldArgs.length, newArgs.length);

                for (; i < end; i++) {
                    if (!newArgs[i].equals(oldArgs[i])) {
                        map.put(COMMAND_ARG + i, newArgs[i]);
                    }
                }
            }

            for (; i < newArgs.length; i++) {
                map.put(COMMAND_ARG + i, newArgs[i]);
            }
        } else if (oldArgs != null) {
            map.put(COMMAND_ARGS, "0");
        }
    }

    /**
     * Compare the two checkpoint specifiers.  If they are different, add the
     * difference to the map.
     * @param map the difference map
     * @param oldCkpt the old setting
     * @param newCkpt the new setting
     */
    private static void putCheckpointDifference(Map<String,String> map, CheckpointSpecifier oldCkpt, CheckpointSpecifier newCkpt) {
        if ((newCkpt == null) && (oldCkpt != null)) {
            map.put(CHECKPOINT_NAME, null);
        } else if ((newCkpt != null) &&
                !newCkpt.equals(oldCkpt)) {
            map.put(CHECKPOINT_NAME, newCkpt.getName());

            if (newCkpt.getInterval() != 0) {
                map.put(CHECKPOINT_INTERVAL, Long.toString(newCkpt.getInterval()));
            } else {
                map.put(CHECKPOINT_OCCASION, newCkpt.getOccasionString());
            }
        }
    }

    /**
     * Compare the two mail specifiers.  If they are different, add the
     * difference to the map.
     * @param map the difference map
     * @param oldMail the old setting
     * @param newMail the new setting
     */
    private static void putMailDifference(Map<String,String> map, MailSpecifier oldMail, MailSpecifier newMail) {
        if ((newMail == null) && (oldMail != null)) {
            map.put(MAIL, null);
        } else if ((newMail != null) && !newMail.equals(oldMail)) {
            map.put(MAIL, newMail.getOccasionString());
        }
    }

    /**
     * Compare the two parallel environments.  If they are different, add the
     * difference to the map.
     * @param map the difference map
     * @param oldPe the old setting
     * @param newPe the new setting
     */
    private static void putPeDifference(Map<String,String> map, ParallelEnvironment oldPe, ParallelEnvironment newPe) {
        if ((newPe == null) && (oldPe != null)) {
            map.put(PE_NAME, null);
        } else if ((newPe != null) && !newPe.equals(oldPe)) {
            map.put(PE_NAME, newPe.getName());
            map.put(PE_MIN, Integer.toString(newPe.getRangeMin()));
            map.put(PE_MAX, Integer.toString(newPe.getRangeMax()));
        }
    }

        /**
     * Compare the two binding specifiers. If they are different, add the
     * difference to the map.
     * @param map the difference map
     * @param oldBinding the old setting
     * @param newBinding the new setting
     */
    private static void putBindingDifference(Map<String,String> map, BindingSpecifier oldBinding, BindingSpecifier newBinding) {
        if ((newBinding == null) && (oldBinding != null)) {
            map.put(BINDING_STRATEGY, null);
        } else if ((newBinding != null) && !newBinding.equals(oldBinding)) {
            int i = 0;

            /*
             * TODO: Although it might not be necessary we send
             * all binding related parameters for simplicity reason in
             * the moment.
             */
            map.put(BINDING_STRATEGY, newBinding.getStrategy().toString());
            map.put(BINDING_TYPE, newBinding.getType().toString());
            map.put(BINDING_AMOUNT, Integer.toString(newBinding.getAmount()));
            map.put(BINDING_SOCKET, Integer.toString(newBinding.getSocket()));
            map.put(BINDING_CORE, Integer.toString(newBinding.getCore()));
            map.put(BINDING_STEP, Integer.toString(newBinding.getStep()));
            map.put(BINDING_EXP_N, Integer.toString(newBinding.getCoreSpecifiers().size()));

            for (BindingSpecifier.CoreSpecifier cs : newBinding.getCoreSpecifiers()) {
                map.put(BINDING_EXP_SOCKET + i, Integer.toString(cs.socket));
                map.put(BINDING_EXP_CORE + i, Integer.toString(cs.core));
                i++;
            }
        }
    }

    /**
     * Compare the two task specifiers.  If they are different, add the
     * difference to the map.
     * @param map the difference map
     * @param oldTask the old setting
     * @param newTask the new setting
     */
    private static void putTaskDifference(Map<String,String> map, TaskSpecifier oldTask, TaskSpecifier newTask) {
        if ((newTask == null) && (oldTask != null)) {
            // t_min, t_max, and t_step cannot current be deleted
            // TODO: Add CR number
            map.put(TASK_MIN, "1");
            map.put(TASK_MAX, "1");
            map.put(TASK_STEP, "1");
        } else if ((newTask != null) && !newTask.equals(oldTask)) {
            map.put(TASK_MIN, Integer.toString(newTask.getMin()));
            map.put(TASK_MAX, Integer.toString(newTask.getMax()));
            map.put(TASK_STEP, Integer.toString(newTask.getStep()));
        }
    }

    /**
     * Compare the two verification levels.  If they are different, add the
     * difference to the map.
     * @param map the difference map
     * @param oldVer the old setting
     * @param newVer the new setting
     */
    private static void putVerificationDifference(Map<String,String> map, Verification oldVer, Verification newVer) {
        if (((newVer == null) && (oldVer != null)) ||
                ((newVer != null) && !newVer.equals(oldVer))) {
            map.put(VERIFICATION, verificationToString(newVer));
        }
    }

    /**
     * Compare the two job names.  If they are different, add the
     * difference to the map.
     * @param map the difference map
     * @param oldName the old setting
     * @param newName the new setting
     * @param cmd the job command from which a default job name may be extracted
     * @see #extractName(java.lang.String)
     */
    private static void putNameDifference(Map<String,String> map, String oldName, String newName, String cmd) {
            if ((newName == null) && (oldName != null)) {
                map.put(NAME, extractName(cmd));
            } else if ((newName != null) && !newName.equals(oldName)) {
                map.put(NAME, newName.toString());
            }
    }

    /**
     * Utility method to add a comma to a list if needed.
     * @param sb the StringBuilder containing the list
     */
    private static void appendComma(StringBuilder sb) {
        if (sb.length() > 0) {
            sb.append(',');
        }
    }

    /**
     * Extract a job name from the given job command.  The job name is extracted
     * by removing any path information and replacing illegal job name
     * characters with underscores.
     * TODO: Remove -- it should be done in the client/master
     * @param cmd the job command
     * @return a default job name
     */
    private static String extractName(String cmd) {
        String name = null;

        if ((cmd != null) && !cmd.equals("")) {
            int index = cmd.lastIndexOf('/');

            if (index < cmd.length() - 1) {
                if (index < 0) {
                    index = 0;
                } else {
                    index++;
                }

                name = cmd.substring(index).replaceAll("[\\n\\t\\r/:'\\\\@*?]+", "_");
            } else {
                name = DELETED_BY_JSV;
            }
        } else {
            name = DELETED_BY_JSV;
        }

        return name;
    }

    /**
     * Get the difference between the current environment variable settings and
     * the baseline environment variable settings.  The three Map parameters
     * are "out" parameters used to store the environment variable differences.
     * @param add the environment variables that have been added
     * @param mod the environment variables that have been modified
     * @param del the environment variables that have been deleted
     */
    void getEnvironmentDifference(Map<String, String> add, Map<String, String> mod, Map<String, String> del) {
        if (baseline == null) {
            log.warning("Attempted to get environment difference before setting baseline");
        } else if ((baseline.environment == null) && (environment != null)) {
            add.putAll(environment);
        } else if ((baseline.environment != null) && (environment == null)) {
            del.putAll(baseline.environment);
        } else {
            HashMap<String,String> copy = new HashMap<String,String>(environment);

            for (String var : baseline.environment.keySet()) {
                if (!environment.containsKey(var)) {
                    del.put(var, baseline.environment.get(var));
                } else if (!baseline.environment.get(var).equals(environment.get(var))) {
                    mod.put(var, environment.get(var));
                    copy.remove(var);
                } else {
                    copy.remove(var);
                }
            }

            add.putAll(copy);
        }
    }

    /**
     * Set the value of a job description property parsed from a String.
     * @param parameter the name of the property to set
     * @param rawValue the value of the property to parse.
     */
    void set(String parameter, String rawValue) {
        String value = null;

        // To simplify the string parsing later, translate "" into null
        if ((rawValue != null) && !rawValue.equals("")) {
            value = rawValue;
        }

        if (baseline != null) {
            throw new IllegalArgumentException("tried to set a parameter value after the baseline was set");
        } else if (parameter == null) {
            log.warning("Ignoring null parameter: " + parameter);
        } else if (parameter.equals(CLIENT)) {
            client = value;
        } else if (parameter.equals(COMMAND_NAME)) {
            commandName = value;
        } else if (parameter.equals(CONTEXT)) {
            context = value;
        } else if (parameter.equals(GROUP)) {
            group = value;
        } else if (parameter.equals(JOB_ID)) {
            jobId = value;
        } else if (parameter.equals(USER)) {
            user = value;
        } else if (parameter.equals(VERSION)) {
            version = value;
        } else if (parameter.equals(COMMAND_ARGS)) {
            commandArgs = parseCmdArgs(value);
        } else if (parameter.startsWith(COMMAND_ARG)) {
            commandArgs = parseCmdArg(parameter, value);
        } else if (parameter.equals(BINARY)) {
            binary = parseBoolean(value);
        } else if (parameter.equals(MERGE_STREAMS)) {
            mergeStreams = parseBoolean(value);
        } else if (parameter.equals(NOTIFY)) {
            notify = parseBoolean(value);
        } else if (parameter.equals(SHELL)) {
            shell = parseBoolean(value);
        } else if (parameter.equals(RERUNNABLE)) {
            rerunnable = parseBoolean(value);
        } else if (parameter.equals(RESERVATION)) {
            reservation = parseBoolean(value);
        } else if (parameter.equals(ACCOUNT)) {
            account = value;
        } else if (parameter.equals(CWD)) {
            cwd = value;
        } else if (parameter.equals(NAME)) {
            name = value;
        } else if (parameter.equals(PROJECT)) {
            project = value;
        } else if (parameter.equals(INPUT_PATH)) {
            inputPath = parsePath(value);
        } else if (parameter.equals(OUTPUT_PATH)) {
            outputPath = parsePath(value);
        } else if (parameter.equals(ERROR_PATH)) {
            errorPath = parsePath(value);
        } else if (parameter.equals(SHELL_PATH)) {
            shellPath = parsePath(value);
        } else if (parameter.equals(START_TIME)) {
            startTime = parseTime(value);
        } else if (parameter.equals(JOB_CONTEXT_ADD)) {
            newContext = parseMap(value);
        } else if (parameter.equals(ADVANCE_RESERVATION_ID)) {
            advanceReservationId = parseInt(value);
        } else if (parameter.equals(CHECKPOINT_NAME)) {
            checkpointSpecifier = parseCheckpointName(value);
        } else if (parameter.equals(CHECKPOINT_INTERVAL)) {
            checkpointSpecifier = parseCheckpointInterval(value);
        } else if (parameter.equals(CHECKPOINT_OCCASION)) {
            checkpointSpecifier = parseCheckpointOccasion(value);
        } else if (parameter.equals(DISPLAY)) {
            display = value;
        } else if (parameter.equals(DEADLINE)) {
            deadline = parseTime(value);
        } else if (parameter.equals(HOLD)) {
            hold = parseHold(value);
        } else if (parameter.equals(HOLD_JOB_IDS)) {
            holdJobIds = parseList(value);
        } else if (parameter.equals(HOLD_ARRAY_JOB_IDS)) {
            holdArrayJobIds = parseList(value);
        } else if (parameter.equals(JOB_SHARE)) {
            jobShare = parseInt(value);
        } else if (parameter.equals(HARD_RESOURCE_REQUIREMENTS)) {
            hardResourceRequirements = parseMap(value);
        } else if (parameter.equals(SOFT_RESOURCE_REQUIREMENTS)) {
            softResourceRequirements = parseMap(value);
        } else if (parameter.equals(MAIL)) {
            mailSpecifier = parseMail(value);
        } else if (parameter.equals(MASTER_QUEUE)) {
            masterQueue = parseList(value);
        } else if (parameter.equals(HARD_QUEUE)) {
            hardQueue = parseList(value);
        } else if (parameter.equals(SOFT_QUEUE)) {
            softQueue = parseList(value);
        } else if (parameter.equals(MAIL_RECIPIENTS)) {
            mailRecipients = parseList(value);
        } else if (parameter.equals(PRIORITY)) {
            priority = parseInt(value);
        } else if (parameter.equals(PE_NAME)) {
            pe = parsePeName(value);
        } else if (parameter.equals(PE_MIN)) {
            pe = parsePeMin(value);
        } else if (parameter.equals(PE_MAX)) {
            pe = parsePeMax(value);
        } else if (parameter.equals(BINDING_STRATEGY)) {
            binding = parseBindingStrategy(value);
        } else if (parameter.equals(BINDING_TYPE)) {
            binding = parseBindingType(BindingSpecifier.Type.valueOf(value.toUpperCase()));
        } else if (parameter.equals(BINDING_AMOUNT)) {
            binding = parseBindingAmount(value);
        } else if (parameter.equals(BINDING_SOCKET)) {
            binding = parseBindingSocket(value);
        } else if (parameter.equals(BINDING_CORE)) {
            binding = parseBindingCore(value);
        } else if (parameter.equals(BINDING_STEP)) {
            binding = parseBindingStep(value);
        } else if (parameter.equals(BINDING_EXP_N)) {
            binding = parseBindingExpN(value);
        } else if (parameter.startsWith(BINDING_EXP_SOCKET)) {
            int pos = parseBindingPos(parameter);
            binding = parseBindingExpSocket(pos, value);
        } else if (parameter.startsWith(BINDING_EXP_CORE)) {
            int pos = parseBindingPos(parameter);
            binding = parseBindingExpCore(pos, value);
        } else if (parameter.equals(TASK_MIN)) {
            taskSpecifier = parseTaskMin(value);
        } else if (parameter.equals(TASK_MAX)) {
            taskSpecifier = parseTaskMax(value);
        } else if (parameter.equals(TASK_STEP)) {
            taskSpecifier = parseTaskStep(value);
        } else if (parameter.equals(VERIFICATION)) {
            verification = parseVerification(value);
        } else {
            // If the parameter doesn't match anything we know about, just log
            // the error.  Since this is an internal-only call, there's no
            // value in throwing an exception.
            log.warning("Ignoring bad parameter value: " + parameter);
        }
    }

    /**
     * Parse the number of job command arguments.
     * @param value the String to parse
     * @return the number of command arguments
     */
    private String[] parseCmdArgs(String value) {
        String[] ret = commandArgs;
        int length = 0;

        if (value != null) {
            try {
                length = Integer.parseInt(value);

                if ((commandArgs == null) || (length != commandArgs.length)) {
                    String[] newArgs = new String[length];

                    if (commandArgs != null) {
                        System.arraycopy(commandArgs, 0, newArgs, 0, Math.min(commandArgs.length, length));
                    }

                    ret = newArgs;
                }
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid argument array length: " + value);
            }
        }

        return ret;
    }

    /**
     * Parse a job command argument.
     * @param parameter the parameter name
     * @param value the String to parse
     * @return the command argument
     */
    private String[] parseCmdArg(String parameter, String value) {
        String[] ret = commandArgs;
        int index = -1;

        if ((parameter != null) && parameter.startsWith(COMMAND_ARG)) {
            try {
                index = Integer.parseInt(parameter.substring(6));

                if (index < 0) {
                    log.warning("Ignoring invalid command argument: " + parameter + "=" + value);
                } else if (value != null) {
                    if ((commandArgs == null) || (index >= commandArgs.length)) {
                        String[] newArgs = new String[index + 1];

                        if (commandArgs != null) {
                            System.arraycopy(commandArgs, 0, newArgs, 0, commandArgs.length);
                        }

                        ret = newArgs;
                    }

                    ret[index] = value;
                } else if ((commandArgs != null) && (index < commandArgs.length)) {
                    String[] newArgs = new String[commandArgs.length - 1];

                    System.arraycopy(commandArgs, 0, newArgs, 0, index);
                    System.arraycopy(commandArgs, index + 1, newArgs, index, commandArgs.length - index - 1);

                    ret = newArgs;
                }
                // else we're tring to remove something that doesn't exist, so who just ignore it
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid command argument: " + parameter + "=" + value);
            }
        } else {
            log.warning("Ignoring invalid command argument: " + parameter + "=" + value);
        }

        return ret;
    }

    /**
     * Parse a boolean value.
     * @param value the String to parse
     * @return the boolean value
     */
    private static Boolean parseBoolean(String value) {
        Boolean ret = null;

        if ((value != null) && value.equalsIgnoreCase("y")) {
            ret = true;
        } else if ((value != null) && value.equalsIgnoreCase("n")) {
            ret = false;
        } else {
            log.warning("Ignoring invalid boolean value: " + value);
        }
        
        return ret;
    }

    /**
     * Parse an integer value.
     * @param value the String to parse
     * @return the integer value
     */
    private static Integer parseInt(String value) {
        Integer ret = null;
        
        if (value != null) {
            try {
                ret = Integer.parseInt(value);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid integer value: " + value);
            }
        }

        return ret;
    }

    /**
     * Parse a the checkpoint specifier name.
     * @param value the String to parse
     * @return the checkpoint specifier name
     */
    private CheckpointSpecifier parseCheckpointName(String value) {
        CheckpointSpecifier ret = null;

        if (value != null) {
            ret = checkpointSpecifier;

            if (ret == null) {
                ret = new CheckpointSpecifier();
            }

            ret.setName(value);
        }

        return ret;
    }

    /**
     * Parse a the checkpoint specifier occasion string.
     * @param value the String to parse
     * @return the checkpoint specifier occasion string
     */
    private CheckpointSpecifier parseCheckpointOccasion(String value) {
        CheckpointSpecifier ret = checkpointSpecifier;

        if (value != null) {
            if (ret == null) {
                ret = new CheckpointSpecifier();
            }

            try {
                ret.setOccasion(value);
            } catch (IllegalArgumentException e) {
                log.warning("Ignoring invalid checkpoint specifier occasion string: " + value);
                // Don't change anything
                ret = checkpointSpecifier;
            }
        } else if (ret != null) {
            ret.setOccasion(0x00);
        }

        return ret;
    }

    /**
     * Parse a the checkpoint specifier interval.
     * @param value the String to parse
     * @return the checkpoint specifier interval
     */
    private CheckpointSpecifier parseCheckpointInterval(String value) {
        CheckpointSpecifier ret = checkpointSpecifier;
        long interval = 0L;

        if (value != null) {
            try {
                interval = Long.parseLong(value);

                if (ret == null) {
                    ret = new CheckpointSpecifier();
                }
                
                ret.setInterval(interval);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid checkpoint specifier interval value: " + value);
            }
        } else if (ret != null) {
            ret.setInterval(interval);
        }

        return ret;
    }

    /**
     * Parse a the hold state.
     * @param value the String to parse
     * @return the hold state
     */
    private static Boolean parseHold(String value) {
        Boolean ret = null;

        if ((value != null) && value.equals(USER_HOLD)) {
            ret = true;
        } else if ((value != null) && value.equals(NO_HOLD)) {
            ret = false;
        } else {
            log.warning("Ignoring invalid hold value: " + value);
        }

        return ret;
    }

    /**
     * Parse a the mail specifier.
     * @param value the String to parse
     * @return the mail specifier
     */
    private MailSpecifier parseMail(String value) {
        MailSpecifier ret = mailSpecifier;

        if (value != null) {
            if (ret == null) {
                ret = new MailSpecifier();
            }

            try {
                ret.setOccasion(value);
            } catch (IllegalArgumentException e) {
                log.warning("Ignoring invalid mail specifier occasion string: " + value);
            }
        } else if (ret != null) {
            ret.setOccasion(0x00);
        }

        return ret;
    }

    /**
     * Parse a the task specifier min value.
     * @param value the String to parse
     * @return the task specifier min value
     */
    private TaskSpecifier parseTaskMin(String value) {
        TaskSpecifier ret = null;

        if (value != null) {
            ret = taskSpecifier;
            
            try {
                int min = Integer.parseInt(value);

                if (ret == null) {
                    ret = new TaskSpecifier();
                }

                ret.setMin(min);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid task specifier min value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a the task specifier max value.
     * @param value the String to parse
     * @return the task specifier max value
     */
    private TaskSpecifier parseTaskMax(String value) {
        TaskSpecifier ret = null;

        if (value != null) {
            ret = taskSpecifier;

            try {
                int max = Integer.parseInt(value);

                if (ret == null) {
                    ret = new TaskSpecifier();
                }

                ret.setMax(max);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid task specifier max value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a the task specifier step value.
     * @param value the String to parse
     * @return the task specifier step value
     */
    private TaskSpecifier parseTaskStep(String value) {
        TaskSpecifier ret = null;

        if (value != null) {
            ret = taskSpecifier;

            try {
                int step = Integer.parseInt(value);

                if (ret == null) {
                    ret = new TaskSpecifier();
                }

                ret.setStep(step);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid task specifier step value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a the parallel environment name.
     * @param value the String to parse
     * @return the parallel environment name
     */
    private ParallelEnvironment parsePeName(String value) {
        ParallelEnvironment ret = null;

        if (value != null) {
            ret = pe;

            if (ret == null) {
                ret = new ParallelEnvironment();
            }

            ret.setName(value);
        }

        return ret;
    }

    /**
     * Parse a the parallel environment min value.
     * @param value the String to parse
     * @return the parallel environment min value
     */
    private ParallelEnvironment parsePeMin(String value) {
        ParallelEnvironment ret = null;

        if (value != null) {
            ret = pe;

            try {
                int min = Integer.parseInt(value);

                if (ret == null) {
                    ret = new ParallelEnvironment();
                }

                ret.setRangeMin(min);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid parallel environment min range value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a the parallel environment max value.
     * @param value the String to parse
     * @return the parallel environment max value
     */
    private ParallelEnvironment parsePeMax(String value) {
        ParallelEnvironment ret = null;

        if (value != null) {
            ret = pe;

            try {
                int max = Integer.parseInt(value);

                if (ret == null) {
                    ret = new ParallelEnvironment();
                }

                ret.setRangeMax(max);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid parallel environment max range value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a binding strategy string
     * @param value the String to parse
     * @return the binding specifier
     */
    private BindingSpecifier parseBindingStrategy(String value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            if (ret == null) {
                ret = new BindingSpecifier();
            }

            ret.setStrategy(BindingSpecifier.Strategy.valueOf(value.toUpperCase()));
        }

        return ret;
    }

    /**
     * Parse a binding type string
     * @param value the String to parse
     * @return the binding type
     */
    private BindingSpecifier parseBindingType(BindingSpecifier.Type value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            if (ret == null) {
                ret = new BindingSpecifier();
            }

            ret.setType(value);
        }

        return ret;
    }

    /**
     * Parse a socket number
     * @param value the String to parse
     * @return the socket number
     */
    private BindingSpecifier parseBindingSocket(String value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            try {
                int socket = Integer.parseInt(value);

                if (ret == null) {
                    ret = new BindingSpecifier();
                }

                ret.setSocket(socket);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid binding socket value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a core number
     * @param value the String to parse
     * @return the core number
     */
    private BindingSpecifier parseBindingCore(String value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            try {
                int core = Integer.parseInt(value);

                if (ret == null) {
                    ret = new BindingSpecifier();
                }

                ret.setCore(core);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid binding core value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a step number
     * @param value the String to parse
     * @return the step number
     */
    private BindingSpecifier parseBindingStep(String value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            try {
                int step = Integer.parseInt(value);

                if (ret == null) {
                    ret = new BindingSpecifier();
                }

                ret.setStep(step);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid binding step value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a binding amount number
     * @param value the String to parse
     * @return the amount number
     */
    private BindingSpecifier parseBindingAmount(String value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            try {
                int amount = Integer.parseInt(value);

                if (ret == null) {
                    ret = new BindingSpecifier();
                }

                ret.setAmount(amount);
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid binding amount value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a binding explicit socket core count value
     * @param value the String to parse
     * @return the value
     */
    private BindingSpecifier parseBindingExpN(String value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            try {
                int length = Integer.parseInt(value);

                if (ret == null) {
                    ret = new BindingSpecifier();
                }

                List<BindingSpecifier.CoreSpecifier> list = ret.getCoreSpecifiers();
                int difference = length - list.size();
                if (difference > 0) {
                    // add missing elements
                    for (int i = 0; i < difference; i++) {
                        BindingSpecifier.CoreSpecifier newCs = null;

                        newCs = ret.new CoreSpecifier();
                        list.add(newCs);
                    }
                } else if (difference < 0) {
                    // remove elements from end of the list
                    for (int i = 0; i < difference; i++) {
                        list.remove(list.size() - 1);
                    }
                }
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid bindings explicit socket core list length value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a binding socket core list position value
     * @param value the String to parse
     * @return the position value
     */
    private int parseBindingPos(String value) {
        int ret = -1;

        if (value != null) {
            try {
                String posString = null;

                if (value.startsWith("binding_exp_socket")) {
                    posString = value.substring("binding_exp_socket".length());
                } else if (value.startsWith("binding_exp_core")) {
                    posString = value.substring("binding_exp_core".length());
                }
                ret = Integer.parseInt(posString);
                            } catch (NumberFormatException e) {
            } catch (IllegalArgumentException e) {
                // Ignore: ret is already -1
            }
        }

        return ret;
    }

    /**
     * Parse a binding explicit socket list value
     * @param value the String to parse
     * @return the value
     */
    private BindingSpecifier parseBindingExpSocket(Integer pos, String value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            try {
                int socket = Integer.parseInt(value);

                if (ret == null) {
                    ret = new BindingSpecifier();
                }

                List<BindingSpecifier.CoreSpecifier> list = ret.getCoreSpecifiers();
                int difference = pos - list.size();
                if (difference > 0) {
                    // add missing elements
                    for (int i = 0; i < difference; i++) {
                        BindingSpecifier.CoreSpecifier newCs = null;

                        newCs = ret.new CoreSpecifier();
                        list.add(newCs);
                    }
                }
                list.get(pos).socket = socket;
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid bindings explicit socket list length value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a binding explicit core list value
     * @param value the String to parse
     * @return the value
     */
    private BindingSpecifier parseBindingExpCore(Integer pos, String value) {
        BindingSpecifier ret = null;

        if (value != null) {
            ret = binding;

            try {
                int core = Integer.parseInt(value);

                if (ret == null) {
                    ret = new BindingSpecifier();
                }

                List<BindingSpecifier.CoreSpecifier> list = ret.getCoreSpecifiers();
                int difference = pos - list.size();
                if (difference > 0) {
                    // add missing elements
                    for (int i = 0; i < difference; i++) {
                        BindingSpecifier.CoreSpecifier newCs = null;

                        newCs = ret.new CoreSpecifier();
                        list.add(newCs);
                    }
                }
                list.get(pos).core = core;
            } catch (NumberFormatException e) {
                log.warning("Ignoring invalid bindings explicit core list value: " + value);
            } catch (IllegalArgumentException e) {
                // Basically means < 1
                ret = null;
            }
        }

        return ret;
    }

    /**
     * Parse a the verification level.
     * @param value the String to parse
     * @return the verification level
     */
    private static Verification parseVerification(String value) {
        Verification ret = null;

        if ((value != null) && (value.equals("e"))) {
            ret = Verification.ERROR;
        } else if ((value != null) && (value.equals("w"))) {
            ret = Verification.WARNING;
        } else if ((value != null) && (value.equals("n"))) {
            ret = Verification.NONE;
        } else if ((value != null) && (value.equals("v"))) {
            ret = Verification.VERIFY;
        } else if ((value != null) && (value.equals("p"))) {
            ret = Verification.POKE;
        } else if (value != null) {
            log.warning("Ignoring invalid verification value: " + value);
        }

        return ret;
    }

    /**
     * Get whether the two maps are different.  This method allows either or
     * both maps to be null.
     * @param mod the new value
     * @param base the old value
     * @return whether the two values are different
     */
    private static boolean isMapDifferent(Map<String, String> mod, Map<String, String> base) {
        boolean retValue = ((mod != null) && !mod.equals(base)) ||
                ((mod == null) && (base != null));

        return retValue;
    }

    /**
     * Format the map as a string that can be understood by the JSV framework.
     * @param map the map to translate
     * @return a JSV-friendly string
     */
    private static String mapToString(Map<String, String> map) {
        String ret = null;

        if (map != null) {
            StringBuilder value = new StringBuilder();

            if (map != null) {
                for (String key : map.keySet()) {
                    String val = map.get(key);

                    if (val != null) {
                        appendComma(value);

                        if (key != null) {
                            value.append(key);
                        }
                        if (val.length() != 0) {
                            value.append('=');
                            value.append(val);
                        }
                    }
                }
            }

            ret = value.toString();
        }

        return ret;
    }

    /**
     * Get whether the two lists are different.  This method allows either or
     * both lists to be null.
     * @param mod the new value
     * @param base the old value
     * @return whether the two values are different
     */
    private static boolean isListDifferent(List<? extends Object> mod, List<? extends Object> base) {
        boolean retValue = ((mod != null) && !mod.equals(base)) ||
                ((mod == null) && (base != null));

        return retValue;
    }

    /**
     * Format the list as a string that can be understood by the JSV framework.
     * @param list the list to translate
     * @return a JSV-friendly string
     */
    private static String listToString(List<? extends Object> list) {
        String ret = null;

        if (list != null) {
            StringBuilder value = new StringBuilder();

            if (list != null) {
                for (Object item : list) {
                    appendComma(value);

                    value.append(item);
                }
            }

            ret = value.toString();
        }

        return ret;
    }

    /**
     * Format the verification as a string that can be understood by the
     * JSV framework.
     * @param verification the verification to translate
     * @return a JSV-friendly string
     */
    private static String verificationToString(Verification verification) {
        String value = null;

        if (verification != null) {
            switch (verification) {
                case ERROR:
                    value = "e";
                    break;
                case NONE:
                    value = "n";
                    break;
                case POKE:
                    value = "p";
                    break;
                case VERIFY:
                    value = "v";
                    break;
                case WARNING:
                    value = "w";
                    break;
            }
        }

        return value;
    }

    /**
     * Parse the path map.
     * @param value the String to parse
     * @return the path map
     */
    private static Map<String,String> parsePath(String value) {
        HashMap<String,String> map = new HashMap<String, String>();

        for (String element : value.split("[, ]+")) {
            Matcher m = pathPattern.matcher(element);

            if (m.matches() && (m.group(1) != null) && (m.group(2) != null)) {
                map.put(m.group(2), m.group(3));
            } else if (m.matches()) {
                map.put(null, m.group(3));
            } else {
                // Since this method is only used to parse input from the
                // client/server, there's no reason to make a big deal out of
                // a malformed component.  Log it and let the admin worry about
                // it later.
                log.warning("Ignoring malformed path element: " + element);
            }
        }

        return map;
    }

    /**
     * Parse the map.
     * @param value the String to parse
     * @return the map
     */
    private static Map<String,String> parseMap(String value) {
        HashMap<String,String> map = new HashMap<String, String>();

        for (String element : value.split("[, ]+")) {
            Matcher m = mapPattern.matcher(element);

            if (m.matches()) {
                String key = m.group(1);

                if (key.length() > 0) {
                    map.put(key, m.group(2));
                } else {
                    log.warning("Ignoring malformed key with length 0: " + key);
                }
            } else {
                if (element.length() > 0  && !element.contains("=")) {
                    map.put(element, "");
                } else {
                    log.warning("Ignoring malformed key=value pair: " + element);
                }
            }
        }

        return map;
    }

    /**
     * Parse the list.
     * @param value the String to parse
     * @return the list
     */
    private static List<String> parseList(String value) {
        List<String> list = new LinkedList<String>();

        list.addAll(Arrays.asList(value.split("[ ,]+")));

        return list;
    }

    /**
     * Format a Calendar instance in a way the JSV framework can understand
     * @param time the Calendar instance to parse
     * @return a JSV-friendly date-time string
     */
    private static String formatTime(Calendar time) {
        String ret = null;

        if (time != null) {
            StringBuilder sb = new StringBuilder();

            sb.append(time.get(Calendar.YEAR));

            if (time.get(Calendar.MONTH) < 9) {
                sb.append('0');
            }

            sb.append(time.get(Calendar.MONTH) + 1);

            if (time.get(Calendar.DAY_OF_MONTH) < 10) {
                sb.append('0');
            }

            sb.append(time.get(Calendar.DAY_OF_MONTH));

            if (time.get(Calendar.HOUR_OF_DAY) < 10) {
                sb.append('0');
            }

            sb.append(time.get(Calendar.HOUR_OF_DAY));

            if (time.get(Calendar.MINUTE) < 10) {
                sb.append('0');
            }

            sb.append(time.get(Calendar.MINUTE));
            sb.append('.');

            if (time.get(Calendar.SECOND) < 10) {
                sb.append('0');
            }

            sb.append(time.get(Calendar.SECOND));

            ret = sb.toString();
        }

        return ret;
    }

    /**
     * Parse the date-time string.
     * @param value the String to parse
     * @return the Calendar instance
     */
    private static Calendar parseTime(String value) {
        Calendar c = Calendar.getInstance();
        Matcher m = timePattern.matcher(value);

        if (m.matches()) {
            if (m.group(2) != null) {
                c.set(Calendar.YEAR, Integer.parseInt(m.group(1)));
            } else if (m.group(1) != null) {
                c.set(Calendar.YEAR, (c.get(Calendar.YEAR) / 100) * 100 + Integer.parseInt(m.group(1)));
            }

            c.set(Calendar.MONTH, Integer.parseInt(m.group(3)) - 1);
            c.set(Calendar.DAY_OF_MONTH, Integer.parseInt(m.group(4)));
            c.set(Calendar.HOUR_OF_DAY, Integer.parseInt(m.group(5)));
            c.set(Calendar.MINUTE, Integer.parseInt(m.group(6)));

            if (m.group(7) != null) {
                c.set(Calendar.SECOND, Integer.parseInt(m.group(8)));
            } else {
                c.set(Calendar.SECOND, 0);
            }

            c.set(Calendar.MILLISECOND, 0);
        } else {
            // Since there's no way to recover from an error like this, we have
            // no real choice but to throw an exception.
            throw new IllegalArgumentException("attempted to parse an invalid date string");
        }

        return c;
    }

    @Override
    protected JobDescription clone() {
        JobDescription copy = new JobDescription();

        copy.client = client;
        copy.commandName = commandName;
        copy.context = context;
        copy.group = group;
        copy.jobId = jobId;
        copy.user = user;
        copy.version = version;

        if (commandArgs != null) {
            copy.commandArgs = new String[commandArgs.length];
            System.arraycopy(commandArgs, 0, copy.commandArgs, 0, copy.commandArgs.length);
        }

        copy.binary = binary;
        copy.mergeStreams = mergeStreams;
        copy.notify = notify;
        copy.shell = shell;
        copy.rerunnable = rerunnable;
        copy.reservation = reservation;
        copy.account = account;
        copy.cwd = cwd;
        copy.name = name;
        copy.project = project;

        if (inputPath != null) {
            copy.inputPath = new HashMap<String,String>(inputPath);
        }

        if (errorPath != null) {
            copy.errorPath = new HashMap<String,String>(errorPath);
        }

        if (outputPath != null) {
            copy.outputPath = new HashMap<String,String>(outputPath);
        }

        if (shellPath != null) {
            copy.shellPath = new HashMap<String,String>(shellPath);
        }

        if (startTime != null) {
            copy.startTime = (Calendar) startTime.clone();
        }

        if (newContext != null) {
            copy.newContext = new HashMap<String,String>(newContext);
        }

        copy.advanceReservationId = advanceReservationId;

        if (checkpointSpecifier != null) {
            copy.checkpointSpecifier = checkpointSpecifier.clone();
        }

        copy.display = display;

        if (deadline != null) {
            copy.deadline = (Calendar) deadline.clone();
        }

        copy.hold = hold;

        if (holdJobIds != null) {
            copy.holdJobIds = new ArrayList<String>(holdJobIds);
        }

        if (holdArrayJobIds != null) {
            copy.holdArrayJobIds = new ArrayList<String>(holdArrayJobIds);
        }

        copy.jobShare = jobShare;

        if (hardResourceRequirements != null) {
            copy.hardResourceRequirements = new HashMap<String, String>(hardResourceRequirements);
        }

        if (hardResourceRequirements != null) {
            copy.softResourceRequirements = new HashMap<String, String>(softResourceRequirements);
        }

        if (mailSpecifier != null) {
            copy.mailSpecifier = mailSpecifier.clone();
        }

        if (masterQueue != null) {
            copy.masterQueue = new ArrayList<String>(masterQueue);
        }

        if (softQueue != null) {
            copy.softQueue = new ArrayList<String>(softQueue);
        }

        if (hardQueue != null) {
            copy.hardQueue = new ArrayList<String>(hardQueue);
        }

        if (mailRecipients != null) {
            copy.mailRecipients = new ArrayList<String>(mailRecipients);
        }

        copy.priority = priority;

        if (pe != null) {
            copy.pe = pe.clone();
        }

        if (binding != null) {
            copy.binding = binding.clone();
        }

        if (taskSpecifier != null) {
            copy.taskSpecifier = taskSpecifier.clone();
        }

        copy.verification = verification;

        if (environment != null) {
            copy.environment = new HashMap<String,String>(environment);
        }

        return copy;
    }
}
