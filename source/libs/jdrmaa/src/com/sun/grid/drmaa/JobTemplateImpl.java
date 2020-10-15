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
package com.sun.grid.drmaa;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.ggf.drmaa.DrmaaException;
import org.ggf.drmaa.FileTransferMode;
import org.ggf.drmaa.InternalException;
import org.ggf.drmaa.InvalidAttributeValueException;
import org.ggf.drmaa.JobTemplate;
import org.ggf.drmaa.PartialTimestamp;
import org.ggf.drmaa.PartialTimestampFormat;
import org.ggf.drmaa.UnsupportedAttributeException;

/**
 * This class represents a remote job and its attributes.  It is used to
 * set up the environment for a job to be submitted.
 *
 * <h3>DRMMA Attributes</h3>
 *
 * <p>DRMAA job template attributes can be set from six different sources.  In
 * order of precedence, from lowest to highest, there are: options
 * set by DRMAA automatically by default, options set in the sge_request
 * file(s), options set in the script file, options set by the jobCategory
 * property, options set by the nativeSpecification property, and
 * options set through other DRMAA properties.</p>
 *
 * <p>By default DRMAA sets four options for all jobs.  They are
 * &quot;-p  0&quot;, &quot;-b yes&quot;, &quot;-shell no&quot;, and
 * &quot;-w e&quot;.  This means that by default, all jobs will have priority 0,
 * all jobs will be treated as binary, i.e. no scripts args will be parsed, all
 * jobs will be executed without a wrapper shell, and jobs which are
 * unschedulable will cause a submit error.</p>
 *
 * <p>The sge_request file, found in the $SGE_ROOT/$SGE_CELL/common directory, may
 * contain options to be applied to all jobs.  The .sge_request file found in
 * the user's home directory or the current working directory may also contain
 * options to be applied to certain jobs.  See the sge_request(5) man page
 * for more information.</p>
 *
 * <p>If the sge_request file contains &quot;-b no&quot; or if the
 * nativeSpecification property is set and contains &quot;-b no&quot;, the
 * script file will be parsed for in-line arguments. Otherwise, no script's args
 * will be interpreted.  See the qsub(1) man page for more information.</p>
 *
 * <p>If the jobCategory property is set, and the category it points to
 * exists in one of the qtask files, the options associated with that category
 * will be applied to the job template.  See the qtask(5) man page and
 * {@link #setJobCategory(String)} below for more information.</p>
 *
 * <p>If the nativeSpecification property is set, all options contained therein
 * will be applied to the job template.  See
 * {@link #setNativeSpecification(String)} below for more information.</p>
 *
 * <p>Other DRMAA attributes will override any previous settings.  For example,
 * if the sge_request file contains &quot;-j y&quot;, but the joinFiles
 * property is set to <code>false</code>, the ultimate result is that the input
 * and output files will remain separate.</p>
 *
 * <p>For various reasons, some options are silently ignored by DRMAA.  Setting
 * any of these options will have no effect.  The ignored options are:
 * &quot;-cwd&quot;, &quot;-help&quot;, &quot;-sync&quot;, &quot;-t&quot;,
 * &quot;-verify&quot;, &quot;-w w&quot;, and &quot;-w v&quot;.
 * The &quot;-cwd&quot; option can be reenabled by setting the environment
 * variable, SGE_DRMAA_ALLOW_CWD.  However, the &quot;-cwd&quot; option is not
 * thread safe and should not be used in a multi-threaded context.</p>
 *
 * <h3>Attribute Correlations</h3>
 *
 * <p>The following DRMAA attributes correspond to the following qsub
 * options:</p>
 *
 * <table><caption>DRMAA Attributes</caption>
 *  <tr><th>DRMAA Attribute</th><th>qsub Option</th></tr>
 *  <tr><td>remoteCommand</td><td>script file</td>
 *  <tr><td>args</td><td>script file arguments</td>
 *  <tr><td>jobSubmissionState = HOLD_STATE</td><td>-h</td>
 *  <tr><td>jobEnvironment</td><td>-v</td>
 *  <tr><td>workingDirectory = $PWD</td><td>-cwd</td>
 *  <tr><td>jobCategory</td><td>(qtsch qtask)<sup>*</sup></td>
 *  <tr><td>nativeSpecification</td><td>ALL<sup>*</sup></td>
 *  <tr><td>emailAddresses</td><td>-M</td>
 *  <tr><td>blockEmail = true</td><td>-m n</td>
 *  <tr><td>startTime</td><td>-a</td>
 *  <tr><td>jobName</td><td>-N</td>
 *  <tr><td>inputPath</td><td>-i</td>
 *  <tr><td>outputPath</td><td>-o</td>
 *  <tr><td>errorPath</td><td>-e</td>
 *  <tr><td>joinFiles</td><td>-j</td>
 *  <tr><td>transferFiles</td><td>(prolog and epilog)<sup>*</sup></td>
 * </table>
 *
 * <p><sup>*</sup> See the individual attribute setter description below</p>
 *
 * <p>The following attributes are unsupported by Grid Engine:</p>
 *
 * <ul>
 * <li>deadlineTime</li>
 * <li>hardWallclockTimeLimit</li>
 * <li>softWallclockTimeLimit</li>
 * <li>hardRunDurationTimeLimit</li>
 * <li>softRunDurationTimeLimit</li>
 * </ul>
 *
 * <p>Using the accessors for any of these attributes will result in an
 * UnsupportedAttributeException being thrown.</p>
 *
 * @author dan.templeton@sun.com
 * @see org.ggf.drmaa.JobTemplate
 * @see org.ggf.drmaa.Session
 * @see com.sun.grid.drmaa.SessionImpl
 * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman5/sge_request.html">sge_request(5)</a>
 * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qsub.html">qsub(1)</a>
 * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman5/qtask.html">qtask(5)</a>
 * @since 0.5
 * @version 1.0
 */
public class JobTemplateImpl implements JobTemplate {
    private static final String REMOTE_COMMAND = "drmaa_remote_command";
    private static final String INPUT_PARAMETERS = "drmaa_v_argv";
    private static final String JOB_SUBMISSION_STATE = "drmaa_js_state";
    private static final String JOB_ENVIRONMENT = "drmaa_v_env";
    private static final String WORKING_DIRECTORY = "drmaa_wd";
    private static final String JOB_CATEGORY = "drmaa_job_category";
    private static final String NATIVE_SPECIFICATION = "drmaa_native_specification";
    private static final String EMAIL_ADDRESS = "drmaa_v_email";
    private static final String BLOCK_EMAIL = "drmaa_block_email";
    private static final String START_TIME = "drmaa_start_time";
    private static final String JOB_NAME = "drmaa_job_name";
    private static final String INPUT_PATH = "drmaa_input_path";
    private static final String OUTPUT_PATH = "drmaa_output_path";
    private static final String ERROR_PATH = "drmaa_error_path";
    private static final String JOIN_FILES = "drmaa_join_files";
    private static final String TRANSFER_FILES = "drmaa_transfer_files";
    /* Not supported
    private static final String DEADLINE_TIME = "drmaa_deadline_time"
    private static final String HARD_WALLCLOCK_TIME_LIMIT = "drmaa_wct_hlimit"
    private static final String SOFT_WALLCLOCK_TIME_LIMIT = "drmaa_wct_slimit"
    private static final String HARD_RUN_DURATION_LIMIT = "drmaa_run_duration_hlimit"
    private static final String SOFT_RUN_DURATION_LIMIT = "drmaa_run_duration_slimit"
    */
    private static final String HOLD_STRING = "drmaa_hold";
    private static final String ACTIVE_STRING = "drmaa_active";
    private static final String BLOCK_EMAIL_TRUE_STRING = "1";
    private static final String BLOCK_EMAIL_FALSE_STRING = "0";
    private static final String JOIN_FILES_TRUE_STRING = "y";
    private static final String JOIN_FILES_FALSE_STRING = "n";
    private static PartialTimestampFormat ptf = new PartialTimestampFormat();
    private SessionImpl session = null;
    private int id = -1;
    
    /**
     * Creates a new instance of JobTemplateImpl
     * @param session the associated SessionImpl object
     * @param id the table index of the native job template
     */
    JobTemplateImpl(SessionImpl session, int id) {
        this.session = session;
        this.id = id;
    }
    
    /**
     * Returns this template's native job template table index.
     * @return the template's native job template table index
     */
    int getId() {
        return id;
    }
    
    /**
     * Specifies the remote command to execute.  The remoteCommand must be
     * the path of an executable that is available at the job's execution host.
     * If the path is relative, it is assumed to be relative to the working
     * directory, usually set through the workingDirectory property.  If
     * workingDirectory is not set, the path is assumed to be relative to the
     * user's home directory.
     *
     * <p>The file pointed to by remoteCommand may either be an executable
     * binary or an executable script.  If a script, it must include the path to
     * the shell in a #! line at the beginning of the script.  By default, the
     * remote command will be executed directly, as by exec. (See the exec(2)
     * man page.)  To have the remote command executed in a shell, such as to
     * preserve environment settings, use the nativeSpecification property to
     * include the &quot;-shell yes&quot; option.  Jobs which are executed by a
     * wrapper shell fail differently from jobs which are executed directly.
     * When a job which contains a user error, such as an invalid path to the
     * executable, is executed by a wrapper shell, the job will execute
     * successfully, but exit with a return code of 1.  When a job which
     * contains such an error is executed directly, it will enter the
     * DRMAA_PS_FAILED state upon execution.</p>
     *
     * <p>No binary file management is done.</p>
     *
     * @param remoteCommand {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public void setRemoteCommand(String remoteCommand) throws DrmaaException {
        this.setAttribute(REMOTE_COMMAND, remoteCommand);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setRemoteCommand(String)
     */
    public String getRemoteCommand() throws DrmaaException {
        String[] command = this.getAttribute(REMOTE_COMMAND);
        
        if (command != null) {
            return command[0];
        } else {
            return null;
        }
    }
    
    /**
     * Specifies the arguments to the job.
     * @param args {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public void setArgs(List args) throws DrmaaException {
        this.setAttribute(INPUT_PARAMETERS, args);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setArgs(List)
     */
    public List getArgs() throws DrmaaException {
        String[] result = this.getAttribute(INPUT_PARAMETERS);
        List returnValue = null;

        if (result != null) {
            returnValue = Arrays.asList(result);
        }

        return returnValue;
    }
    
    /**
     * Specifies the job state at submission.  The possible values are
     * <code>HOLD_STATE</code> and <code>ACTIVE_STATE</code>:
     *
     * <ul>
     *  <li><code>ACTIVE</code> means the job is submitted in a runnable
     *  state.</li>
     *  <li><code>HOLD</code> means the job is submitted in user hold state
     *  (either <code>Session.USER_ON_HOLD</code> or
     *  <code>Session.USER_SYSTEM_ON_HOLD</code>).</li>
     * </ul>
     * 
     * <p>This parameter is largely equivalent to the qsub submit option
     * &quot;-h&quot;.
     * @param state {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qsub.html">qsub(1)</a>
     */
    public void setJobSubmissionState(int state) throws DrmaaException {
        String stateString = null;
        
        if (state == HOLD_STATE) {
            stateString = HOLD_STRING;
        } else if (state == ACTIVE_STATE) {
            stateString = ACTIVE_STRING;
        } else {
            throw new InvalidAttributeValueException("jobSubmissionState attribute is invalid");
        }
        
        this.setAttribute(JOB_SUBMISSION_STATE, stateString);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setJobSubmissionState(int)
     */
    public int getJobSubmissionState() throws DrmaaException {
        String[] stateString =  this.getAttribute(JOB_SUBMISSION_STATE);
        
        if ((stateString == null) || stateString[0].equals(ACTIVE_STRING)) {
            return ACTIVE_STATE;
        } else if (stateString[0].equals(HOLD_STRING)) {
            return HOLD_STATE;
        } else {
            /* This should never happen */
            throw new InternalException("jobSubmissionState property is unparsable");
        }
    }
    
    /**
     * Sets the environment values that define the remote environment.  The
     * values override the remote environment values if there is a collision.
     * @param env {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public void setJobEnvironment(Map env) throws DrmaaException {
        String[] envStrings = new String[env.size()];
        Iterator i = env.keySet().iterator();
        int count = 0;
        String key = null;
        StringBuffer nameValue = null;
        
        while (i.hasNext()) {
            key = (String)i.next();
            nameValue = new StringBuffer(key);
            nameValue.append('=');
            nameValue.append(env.get(key));
            envStrings[count] = nameValue.toString();
            count++;
        }
        
        this.setAttribute(JOB_ENVIRONMENT, Arrays.asList(envStrings));
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setJobEnvironment(Map)
     */
    public Map getJobEnvironment() throws DrmaaException {
        String[] props = this.getAttribute(JOB_ENVIRONMENT);
        Map env = null;
        
        if (props != null) {
            env = new HashMap();
            
            for (int count = 0; count < props.length; count++) {
                int index = props[count].indexOf('=');
                String name = props[count].substring(0, index);
                String value = props[count].substring(index + 1);
                
                env.put(name, value);
            }
        }
        
        return env;
    }
    
    /**
     * Specifies the directory name where the job will be executed.
     * A <CODE>HOME_DIRECTORY</CODE> placeholder at the beginning of the
     * directory name denotes that the remaining portion of the directory name
     * is to be resolved relative to the job submiter's home directory on the
     * execution host.
     * 
     * <p>When the DRMAA job template is used for bulk job submission (see also
     * {@link org.ggf.drmaa.Session#runBulkJobs(org.ggf.drmaa.JobTemplate,int,int,int)}
     * the <CODE>PARAMETRIC_INDEX</CODE> placeholder can be used at any position
     * within the directory name to cause a substitution with the parametric
     * job's index.  The directory name must be specified in a syntax that is
     * common at the host where the job will be executed.  If no placeholder is
     * used, an absolute directory specification is recommended. If set to a
     * relative path and no placeholder is used, a path relative to the user's
     * home directory is assumed.  If not set, the working directory will
     * default to the user's home directory.  If set, and the directory does
     * not exist, the job will enter the state <code>FAILED</code> when the job
     * is run.</p>
     * @param wd {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see org.ggf.drmaa.JobTemplate#HOME_DIRECTORY
     * @see org.ggf.drmaa.JobTemplate#PARAMETRIC_INDEX
     * @see org.ggf.drmaa.Session#FAILED
     */
    public void setWorkingDirectory(String wd) throws DrmaaException {
        this.setAttribute(WORKING_DIRECTORY, wd);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setWorkingDirectory(String)
     */
    public String getWorkingDirectory() throws DrmaaException {
        String[] wd =  this.getAttribute(WORKING_DIRECTORY);
        
        if (wd != null) {
            return wd[0];
        } else {
            return null;
        }
    }
    
    /**
     * Specifies the DRMAA job category. The category string is used by
     * Grid Engine as reference into the qtask file. Certain qsub options used
     * in the referenced qtask file line are applied to the job template
     * before submission to allow site-specific resolving of resources and/or
     * policies.  The cluster qtask file, the local qtask file, and the user
     * qtask file are searched. Job settings resulting from job template
     * category are overridden by settings resulting from the job template
     * nativeSpecification property as well as by explict DRMAA job template
     * property settings.
     *
     * <p>The options -help, -sync, -t, -verify, and -w w|v are ignored.  The 
     * -cwd option is ignored unless the $SGE_DRMAA_ALLOW_CWD environment 
     * variable is set.</p>
     * @param category {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman5/qtask.html">qtask(5)</a>
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qsub.html">qsub(1)</a>
     */
    public void setJobCategory(String category) throws DrmaaException {
        this.setAttribute(JOB_CATEGORY, category);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setJobCategory(String)
     */
    public String getJobCategory() throws DrmaaException {
        String[] category =  this.getAttribute(JOB_CATEGORY);
        
        if (category != null) {
            return category[0];
        } else {
            return null;
        }
    }
    
    /**
     * Specifies native qsub options which will be interpreted as part of the
     * DRMAA job template.  All options available to the qsub command may be
     * used in the nativeSpecification, except for -help, -sync, -t, -verify, 
     * and -w w|v.  -cwd may only be used if the $SGE_DRMAA_ALLOW_CWD enviroment
     * variable is set.  Options set in the nativeSpecification will be
     * overridden by the corresponding DRMAA properties.  See the qsub(1) man
     * page for more information on qsub command line options.
     * @param spec {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qsub.html">qsub(1)</a>
     */
    public void setNativeSpecification(String spec) throws DrmaaException {
        this.setAttribute(NATIVE_SPECIFICATION, spec);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setNativeSpecification(String)
     */
    public String getNativeSpecification() throws DrmaaException {
        String[] spec =  this.getAttribute(NATIVE_SPECIFICATION);
        
        if (spec != null) {
            return spec[0];
        } else {
            return null;
        }
    }
    
    /**
     * Set the list of email addresses used to report the job completion and
     * status.
     * @param email {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public void setEmail(Set email) throws DrmaaException {
        this.setAttribute(EMAIL_ADDRESS, email);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setEmail(java.util.Set)
     */
    public Set getEmail() throws DrmaaException {
        String[] result = this.getAttribute(EMAIL_ADDRESS);
        Set returnValue = null;

        if (result != null) {
            returnValue = new HashSet(Arrays.asList(result));
        }

        return returnValue;
    }
    
    /**
     * Specifies whether e-mail sending shall blocked or not.  By default email
     * is not sent.  If, however, a setting in a cluster or user settings file
     * or the nativeSpecification or jobCategory property enables sending email
     * in association with job events, the blockEmail property will override
     * that setting, causing no email to be sent.
     * @param blockEmail {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public void setBlockEmail(boolean blockEmail) throws DrmaaException {
        if (blockEmail) {
            this.setAttribute(BLOCK_EMAIL, BLOCK_EMAIL_TRUE_STRING);
        } else {
            this.setAttribute(BLOCK_EMAIL, BLOCK_EMAIL_FALSE_STRING);
        }
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setBlockEmail(boolean)
     */
    public boolean getBlockEmail() throws DrmaaException {
        String[] block =  this.getAttribute(BLOCK_EMAIL);
        
        if (block != null) {
            return block[0].equals(BLOCK_EMAIL_TRUE_STRING);
        } else {
            return false;
        }
    }
    
    /**
     * Set the earliest time when the job may be eligible to be run.
     * @param startTime {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see org.ggf.drmaa.PartialTimestamp
     */
    public void setStartTime(PartialTimestamp startTime) throws DrmaaException {
        this.setAttribute(START_TIME, ptf.format(startTime));
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setStartTime(org.ggf.drmaa.PartialTimestamp)
     */
    public PartialTimestamp getStartTime() throws DrmaaException {
        String[] time =  this.getAttribute(START_TIME);
        
        if (time != null) {
            try {
                return ptf.parse(time[0]);
            } catch (java.text.ParseException e) {
                throw new InternalException("startTime property is unparsable");
            }
        } else {
            return null;
        }
    }
    
    /**
     * Set the name of the job.  A job name will be comprised of alpha-numeric
     * and _ characters.  Setting the job name is equivalent to use of the qsub
     * submit option &quot;-N&quot; with <i>name</i> as option argument and has
     * the same restrictions.  See the qsub(1) man page.
     * @param name {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qsub.html">qsub(1)</a>
     */
    public void setJobName(String name) throws DrmaaException {
        this.setAttribute(JOB_NAME, name);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setJobName(String)
     */
    public String getJobName() throws DrmaaException {
        String[] name =  this.getAttribute(JOB_NAME);
        
        if (name != null) {
            return name[0];
        } else {
            return null;
        }
    }
    
    /**
     * Set the job's standard input path.  Unless set elsewhere, if not
     * explicitly set in the job template, the job is started with an empty
     * input stream.  If the standard input is set, it specifies the network
     * path of the job's input stream file in the form of
     * <code>[hostname]:file_path</code><br>
     * <p>When the transferFiles property is supported and the set
     * TranferFileMode instance's inputStream property is set to
     * <code>true</code>, the input file will be fetched by Grid Engine
     * from the specified host or from the submit host if no hostname
     * is specified.  When the transferFiles property is unsupported or the set
     * TranferFileMode instance's inputStream property is not set or is set to
     * <code>false</code>, the input file is always expected to be at the
     * host where the job is executed, regardless of any hostname specified.</p>
     *
     * <p>When the DRMAA job template is used for bulk job submission (see also
     * {@link org.ggf.drmaa.Session#runBulkJobs(org.ggf.drmaa.JobTemplate,int,int,int)}
     * the <CODE>PARAMETRIC_INDEX</CODE> placeholder can be used at any position
     * within the file name to cause a substitution with the parametric
     * job's index.  A <CODE>HOME_DIRECTORY</CODE> placeholder at the beginning
     * of the file path denotes the remaining portion of the file path as a
     * relative file specification to be resolved relative to the job user's
     * home directory at the host where the file is located.  A
     * <CODE>WORKING_DIRECTORY</CODE> placeholder at the beginning of file path
     * denotes the remaining portion of the file path as a relative file
     * specification to be resolved relative to the job's working directory at
     * the host where the file is located.  The file name must be specified in a
     * syntax that is common at the host where the job will be executed.  If no
     * home or working directory placeholder is used, an absolute file
     * specification is recommended. If set to a relative file path and no home
     * or working directory placeholder is used, a path relative to the user's
     * home directory is assumed.</p>
     *
     * <p>When the job is run, if this attribute is set, and the file can't be
     * read, the job will enter the state <code>FAILED</code>.</p>
     *
     * @param inputPath {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see org.ggf.drmaa.JobTemplate#HOME_DIRECTORY
     * @see org.ggf.drmaa.JobTemplate#WORKING_DIRECTORY
     * @see org.ggf.drmaa.JobTemplate#PARAMETRIC_INDEX
     * @see org.ggf.drmaa.Session#FAILED
     */
    public void setInputPath(String inputPath) throws DrmaaException {
        this.setAttribute(INPUT_PATH, inputPath);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setInputPath(String)
     */
    public String getInputPath() throws DrmaaException {
        String[] path =  this.getAttribute(INPUT_PATH);
        
        if (path != null) {
            return path[0];
        } else {
            return null;
        }
    }
    
    /**
     * Sets how to direct the job's standard output.  Unless set elsewhere, if
     * not explicitly set in the job template, the whereabouts of the job's
     * output stream is not defined.  If the standard output is set, it
     * specifies the network path of the job's output stream file in the form of
     * <code>[hostname]:file_path</code>
     *
     * <p>When the transferFiles property is supported and the set
     * TranferFileMode instance's outputStream property is set to
     * <code>true</code>, the output file will be transferred by Grid Engine
     * to the specified host or to the submit host if no hostname is specified.
     * When the transferFiles property is unsupported or the set
     * TranferFileMode instance's outputStream property is not set or is set to
     * <code>false</code>, the output file is always kept at the host where the
     * job is executed, regardless of any hostname specified.</p>
     *
     * <p>When the DRMAA job template is used for bulk job submission (see also
     * {@link org.ggf.drmaa.Session#runBulkJobs(org.ggf.drmaa.JobTemplate,int,int,int)}
     * the <CODE>PARAMETRIC_INDEX</CODE> placeholder can be used at any position
     * within the file name to cause a substitution with the parametric
     * job's index.  A <CODE>HOME_DIRECTORY</CODE> placeholder at the beginning
     * of the file path denotes the remaining portion of the file path as a
     * relative file specification to be resolved relative to the job user's
     * home directory at the host where the file is located.  A
     * <CODE>WORKING_DIRECTORY</CODE> placeholder at the beginning of file path
     * denotes the remaining portion of the file path as a relative file
     * specification to be resolved relative to the job's working directory at
     * the host where the file is located.  The file name must be specified in a
     * syntax that is common at the host where the job will be executed.  If no
     * home or working directory placeholder is used, an absolute file
     * specification is recommended. If set to a relative file path and no home
     * or working directory placeholder is used, a path relative to the user's
     * home directory is assumed.</p>
     *
     * <p>When the job is run, if this attribute is set, and the file can't be
     * read, the job will enter the state <code>FAILED</code>.</p>
     *
     * @param outputPath {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see org.ggf.drmaa.JobTemplate#HOME_DIRECTORY
     * @see org.ggf.drmaa.JobTemplate#WORKING_DIRECTORY
     * @see org.ggf.drmaa.JobTemplate#PARAMETRIC_INDEX
     * @see org.ggf.drmaa.Session#FAILED
     */
    public void setOutputPath(String outputPath) throws DrmaaException {
        this.setAttribute(OUTPUT_PATH, outputPath);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setOutputPath(String)
     */
    public String getOutputPath() throws DrmaaException {
        String[] path =  this.getAttribute(OUTPUT_PATH);
        
        if (path != null) {
            return path[0];
        } else {
            return null;
        }
    }
    
    /**
     * Sets how to direct the job's standard error.  Unless set elsewhere, if
     * not explicitly set in the job template, the whereabouts of the job's
     * error stream is not defined.  If the standard error is set, it
     * specifies the network path of the job's error stream file in the form of
     * <code>[hostname]:file_path</code><br>
     *
     * <p>When the transferFiles property is supported and the set
     * TranferFileMode instance's errorStream property is set to
     * <code>true</code>, the error file will be transferred by Grid Engine
     * to the specified host or to the submit host if no hostname is specified.
     * When the transferFiles property is unsupported or the set
     * TranferFileMode instance's errorStream property is not set or is set to
     * <code>false</code>, the error file is always kept at the host where the
     * job is executed, regardless of any hostname specified.</p>
     *
     * <p>When the DRMAA job template is used for bulk job submission (see also
     * {@link org.ggf.drmaa.Session#runBulkJobs(org.ggf.drmaa.JobTemplate,int,int,int)}
     * the <CODE>PARAMETRIC_INDEX</CODE> placeholder can be used at any position
     * within the file name to cause a substitution with the parametric
     * job's index.  A <CODE>HOME_DIRECTORY</CODE> placeholder at the beginning
     * of the file path denotes the remaining portion of the file path as a
     * relative file specification to be resolved relative to the job user's
     * home directory at the host where the file is located.  A
     * <CODE>WORKING_DIRECTORY</CODE> placeholder at the beginning of file path
     * denotes the remaining portion of the file path as a relative file
     * specification to be resolved relative to the job's working directory at
     * the host where the file is located.  The file name must be specified in a
     * syntax that is common at the host where the job will be executed.  If no
     * home or working directory placeholder is used, an absolute file
     * specification is recommended. If set to a relative file path and no home
     * or working directory placeholder is used, a path relative to the user's
     * home directory is assumed.</p>
     *
     * <p>When the job is run, if this attribute is set, and the file can't be
     * read, the job will enter the state <code>FAILED</code>.</p>
     *
     * @param errorPath {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see org.ggf.drmaa.JobTemplate#HOME_DIRECTORY
     * @see org.ggf.drmaa.JobTemplate#WORKING_DIRECTORY
     * @see org.ggf.drmaa.JobTemplate#PARAMETRIC_INDEX
     * @see org.ggf.drmaa.Session#FAILED
     */
    public void setErrorPath(String errorPath) throws DrmaaException {
        this.setAttribute(ERROR_PATH, errorPath);
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setErrorPath(String)
     */
    public String getErrorPath() throws DrmaaException {
        String[] path =  this.getAttribute(ERROR_PATH);
        
        if (path != null) {
            return path[0];
        } else {
            return null;
        }
    }
    
    /**
     * Sets whether the error stream should be intermixed with the output
     * stream. If not explicitly set in the job template the attribute defaults
     * to <code>false</code>.  If <code>true</code>, the underlying DRM system
     * will ignore the value of the errorPath property and intermix the standard
     * error stream with the standard output stream as specified with
     * outputPath.
     * @param join {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public void setJoinFiles(boolean join) throws DrmaaException {
        if (join) {
            this.setAttribute(JOIN_FILES, JOIN_FILES_TRUE_STRING);
        } else {
            this.setAttribute(JOIN_FILES, JOIN_FILES_FALSE_STRING);
        }
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setJoinFiles(boolean)
     */
    public boolean getJoinFiles() throws DrmaaException {
        String[] join =  this.getAttribute(JOIN_FILES);
        
        if (join != null) {
            return join[0].equalsIgnoreCase(JOIN_FILES_TRUE_STRING);
        } else {
            return false;
        }
    }
    
    /**
     * <p>Set Transfer Files</p>
     *
     * {@inheritDoc}
     *
     * <p>The file transfer mechanism itself must be configured by the
     * administrator. (See the sge_conf(5) man page.)  When it is configured,
     * the administrator has to enable the transferFiles property by setting the
     * execd param, delegated_file_staging, to true. If it is not configured,
     * transferFiles is not supported and accessing this property will result in
     * an UnsupportedAttributeException being thrown.</p>
     *
     * @param mode {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setInputPath(String)
     * @see #setOutputPath(String)
     * @see #setErrorPath(String)
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman5/sge_conf.html">sge_conf(5)</a>
     */
    public void setTransferFiles(FileTransferMode mode) throws DrmaaException {
        StringBuffer buf = new StringBuffer();
        
        if (mode.getInputStream()) {
            buf.append('i');
        }
        
        if (mode.getOutputStream()) {
            buf.append('o');
        }
        
        if (mode.getErrorStream()) {
            buf.append('e');
        }
        
        this.setAttribute(TRANSFER_FILES, buf.toString());
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #setTransferFiles(org.ggf.drmaa.FileTransferMode)
     */
    public FileTransferMode getTransferFiles() throws DrmaaException {
        String[] mode =  this.getAttribute(TRANSFER_FILES);
        
        if (mode != null) {
            return new FileTransferMode((mode[0].indexOf('i') != -1),
                    (mode[0].indexOf('o') != -1),
                    (mode[0].indexOf('e') != -1));
        } else {
            return null;
        }
    }

    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public void setDeadlineTime(PartialTimestamp deadline)
            throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The deadlineTime attribute " +
                                                "is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public PartialTimestamp getDeadlineTime()
            throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The deadlineTime attribute " +
                                                "is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public void setHardWallclockTimeLimit(long hardWallclockLimit)
             throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The hardWallclockTimeLimit " +
                                                "attribute is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public long getHardWallclockTimeLimit()
             throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The hardWallclockTimeLimit " +
                                                "attribute is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public void setSoftWallclockTimeLimit(long softWallclockLimit)
             throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The softWallclockTimeLimit " +
                                                "attribute is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public long getSoftWallclockTimeLimit()
             throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The softWallclockTimeLimit " +
                                                "attribute is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public void setHardRunDurationLimit(long hardRunLimit)
             throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The hardRunDurationLimit " +
                                                "attribute is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public long getHardRunDurationLimit()
             throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The hardRunDurationLimit " +
                                                "attribute is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public void setSoftRunDurationLimit(long softRunLimit)
             throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The softRunDurationLimit " +
                                                "attribute is not supported.");
    }
    
    /**
     * Unsupported property.  Will throw an UnsupportedAttributeException if
     * called.
     * @throws UnsupportedAttributeException unsupported property
     */
    public long getSoftRunDurationLimit()
             throws UnsupportedAttributeException {
        throw new UnsupportedAttributeException("The softRunDurationLimit " +
                                                "attribute is not supported.");
    }

    /**
     * Uses the SessionImpl instance to get the attribute value from the native
     * job template asssociated with this JobTemplateImpl instance.
     */
    private String[] getAttribute(String name) throws DrmaaException {
        String[] values = session.nativeGetAttribute(id, name);
        
        return values;
    }
    
    /**
     * Uses the SessionImpl instance to set the attribute in the native job
     * template asssociated with this JobTemplateImpl instance.
     */
    private void setAttribute(String name, Collection value) throws DrmaaException {
        session.nativeSetAttributeValues(id, name, (String[])value.toArray(new String[value.size()]));
    }
    
    /**
     * Uses the SessionImpl instance to set the vector attribute in the native
     * job template asssociated with this JobTemplateImpl instance.
     */
    private void setAttribute(String name, String value) throws DrmaaException {
        session.nativeSetAttributeValue(id, name, value);
    }
    
    /**
     * Returns the list of supported properties names.  With the execd param,
     * delegated_file_staging, set to false, this list includes only the list of
     * DRMAA required properties.  With delegated_file_staging set to true, the
     * list also includes the transferFiles property.
     * @return {@inheritDoc}
     */
    public Set getAttributeNames() throws DrmaaException {
        String[] result = session.nativeGetAttributeNames(id);
        Set returnValue = new HashSet();

        if (result != null) {
            returnValue.addAll(Arrays.asList(result));
        }

        return returnValue;
    }
    
    /**
     * Tests whether this JobTemplateImpl represents the same native job
     * template as the given object.  This implementation means that even if two
     * JobTemplateImpl instance's have all the same settings, they are not
     * equal, because they are associated with different native job templates.
     * 
     * @param obj the object against which to compare
     * @return whether the the given object is the same as this object
     */
    public boolean equals(Object obj) {
        if (obj instanceof JobTemplateImpl) {
            return (this.getId() == ((JobTemplateImpl)obj).getId());
        } else {
            return false;
        }
    }
    
    /**
     * Returns a hash code based on the associated native job template's table
     * index.
     * @return the hash code
     */
    public int hashCode() {
        return this.getId();
    }
}
