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
package org.ggf.drmaa;

import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * This interface represents a template to be used for the creation of a job.
 * The properties set on a JobTemplate instance are used to create a new job and
 * set up the new job's environment.
 *
 * <p>There is a 1:n relationship between JobTemplate instances and jobs.  A
 * single JobTemplate instance can be used to submit any number of jobs.  Once a
 * job has been submitted, e.g. via Session.runJob(), the JobTemplate instance
 * no longer has any affect on that job.  Changes made to the JobTemplate
 * instance will have no affect on already running jobs.  Deleting the
 * JobTemplate instance (via Session.deleteJobTemplate()) also has no effect on
 * running jobs.</p>
 *
 * <p>Once a JobTemplate instance has been created
 * (via Session.createJobTemplate()), it is the responsibility of the developer
 * to delete it when no longer needed (via Session.deleteJobTemplate ()).
 * Failure to do so may result in a memory leak.</p>
 *
 * <p>Example:</p>
 *
 * <pre>public static void main(String[] args) {
 *   SessionFactory factory = SessionFactory.getFactory();
 *   Session session = factory.getSession();
 *
 *   try {
 *      session.init(&quot;&quot;);
 *      JobTemplate jt = session.createJobTemplate();
 *      jt.setRemoteCommand(&quot;sleeper.sh&quot;);
 *      jt.setWorkingDirectory(&quot;:&quot; + HOME_DIRECTORY + &quot;/jobs&quot;);
 *      jt.setArgs(Collections.singletonList(&quot;5&quot;));
 *
 *      String id = session.runJob(jt);
 *
 *      session.deleteJobTemplate(jt);
 *      session.exit();
 *   }
 *   catch (DrmaaException e) {
 *      System.out.println(&quot;Error: &quot; + e.getMessage ());
 *   }
 * }
 * </pre>
 * @author dan.templeton@sun.com
 * @see Session
 * @since 0.4.2
 * @version 1.0
 */
public abstract interface JobTemplate {
    /**
     * Means the job has been queued but it is not
     * eligible to run.  Used with the jobSubmissionState property.
     */
    public static final int HOLD_STATE = 0;
    /**
     * Means the job has been queued and is eligible
     * to run.  Used with the jobSubmissionState property.
     */
    public static final int ACTIVE_STATE = 1;
    /**
     * Placeholder which represents the home directory in the workingDirectory,
     * inputPath, outputPath, and errorPath properties
     */
    public static final String HOME_DIRECTORY = "$drmaa_hd_ph$";
    /**
     * Placeholder which represents the working directory in the
     * workingDirectory, inputPath, outputPath, and errorPath properties
     */
    public static final String WORKING_DIRECTORY = "$drmaa_wd_ph$";
    /**
     * Placeholder which represents the job id for a job in a parametric job set
     * in the inputPath, outputPath, and errorPath properties
     */
    public static final String PARAMETRIC_INDEX = "$drmaa_incr_ph$";

    /**
     * Set the command string to execute as the job.  The command
     * is relative to the execution host and is evaluated on the
     * execution host.
     * @param remoteCommand the command to execute as the job
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setRemoteCommand(String remoteCommand) throws DrmaaException;

    /**
     * Get the command string to execute as the job.
     * @return the command to execute as the job or null if it has not been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setRemoteCommand(String)
     */
    public String getRemoteCommand() throws DrmaaException;

    /**
     * Sets the arguments to the job.
     * @param args the parameters passed as arguments to the job
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     *
     */
    public void setArgs(List args) throws DrmaaException;

    /**
     * Get the arguments to the job.
     * @return the parameters passed as arguments to the job or null if they have
     * not been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setArgs(List)
     */
    public List getArgs() throws DrmaaException;

    /**
     * Set the job state at submission.  The states are
     * HOLD_STATE and ACTIVE_STATE:
     *
     * <UL>
     * <LI>ACTIVE_STATE means job is eligible to run</LI>
     * <LI>HOLD_STATE means job may be queued but is ineligible to run</LI>
     * </UL>
     *
     * <p>A job submitted in the HOLD_STATE state can be made eligible to run
     * through the Session.control() method using the Session.RELEASE
     * constant.</p>
     * @param state the job state at submission
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setJobSubmissionState(int state) throws DrmaaException;

    /**
     * Get the job state at submission.
     * @return the job state at submission
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setJobSubmissionState(int)
     */
    public int getJobSubmissionState() throws DrmaaException;

    /**
     * Set the environment values that define the remote environment.
     * The values override any remote environment values if there is a
     * collision.
     * @param env the environment values that define the remote environment
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setJobEnvironment(Map env) throws DrmaaException;

    /**
     * Get the environment values that define the remote environment.
     * @return the environment values that define the remote environment or null
     * if it has not been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setJobEnvironment(Map)
     */
    public Map getJobEnvironment() throws DrmaaException;

    /**
     * Set the directory where the job is executed.  If the working directory is
     * not set, behavior is implementation dependent.  The working directory is
     * evaluated relative to the execution host.
     *
     * <p>A <CODE>HOME_DIRECTORY</CODE> placeholder at the beginning denotes that
     * the remaining portion of the directory name is resolved
     * relative to the job submiter's home directory on the execution host.</p>
     *
     * <p>The <CODE>PARAMETRIC_INDEX</CODE> placeholder can be used at any position
     * within the directory name of a parametric job and will be replaced
     * by the underlying DRM system with the parametric job's index.</p>
     *
     * <p>The directory name must be specified in a syntax that is common at the
     * host where the job will be executed.</p>
     *
     * <p>If no placeholder is used, an absolute directory specification
     * is expected.</p>
     *
     * <p>If the directory does not exist when the job is run, the job enters the
     * state Session.FAILED.</p>
     *
     * @param wd the directory where the job is executed
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setWorkingDirectory(String wd) throws DrmaaException;

    /**
     * Get the directory where the job is executed.
     * @return the directory where the job is executed or null if it has not been
     * set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setWorkingDirectory(String)
     */
    public String getWorkingDirectory() throws DrmaaException;

    /**
     * Set an opaque string specifying how to resolve site-specific resources
     * and/or policies.  The job category can be used, for example, to submit
     * jobs that are &quot;low priority&quot; jobs.  It is then up to the local
     * DRM administrator to map &quot;low priority&quot; to a set of appropriate
     * job submission characteristics, such as reduced memory allowance or
     * lower priority value.
     * @param category an opaque string specifying how to resolve site-specific
     * resources and/or policies.
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setJobCategory(String category) throws DrmaaException;

    /**
     * Get the opaque string specifying how to resolve site-specific resources
     * and/or policies.
     * @return the opaque string specifying how to resolve site-specific
     * resources and/or policies or null if it has not been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setJobCategory(String)
     */
    public String getJobCategory() throws DrmaaException;

    /**
     * Set an opaque string that is passed by the end user to DRMAA to specify
     * site-specific resources and/or policies.
     * @param spec an opaque string that is passed by the end user to DRMAA to
     * specify site-specific resources and/or policies
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setNativeSpecification(String spec) throws DrmaaException;

    /**
     * Get the opaque string that is passed by the end user to DRMAA to specify
     * site-specific resources and/or policies.
     * @return the opaque string that is passed by the end user to DRMAA to
     * specify site-specific resources and/or policies or null if it has not
     * been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setNativeSpecification(String)
     */
    public String getNativeSpecification() throws DrmaaException;

    /**
     * Set the list of email addresses used to report the job completion and
     * status.
     * @param email the list of email addresses used to report the job
     * completion and status.
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setEmail(Set email) throws DrmaaException;

    /**
     * Get the list of email addresses used to report the job completion and
     * status.
     * @return the list of email addresses used to report the job completion
     * and status or null if they have not been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setEmail(Set)
     */
    public Set getEmail() throws DrmaaException;

    /**
     * Set whether to block sending e-mail by default, regardless of the DRMS
     * settings.  This property can only be used to prevent email from being
     * sent.  It cannot force the DRM to send email.
     * @param blockEmail whether to block sending e-mail by default
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setBlockEmail(boolean blockEmail) throws DrmaaException;

    /**
     * Get whether to block sending e-mail by default, regardless of the DRMS
     * settings.
     * @return whether to block sending e-mail by default
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setBlockEmail(boolean)
     */
    public boolean getBlockEmail() throws DrmaaException;

    /**
     * Set the earliest time when the job may be eligible to be run.
     * @param startTime the earliest time when the job may be eligible to be run
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setStartTime(PartialTimestamp startTime) throws DrmaaException;

    /**
     * Get the earliest time when the job may be eligible to be run.
     * @return the earliest time when the job may be eligible to be run or null
     * if it has not been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setStartTime(PartialTimestamp)
     */
    public PartialTimestamp getStartTime() throws DrmaaException;

    /**
     * Set the name of the job.  A job name will be comprised of alpha-numeric
     * and _ characters.  The DRMAA implementation may truncate client
     * provided job names to an implementation defined length that is at least
     * 31 characters.
     * @param name the name of the job
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setJobName(String name) throws DrmaaException;

    /**
     * Get the name of the job.
     * @return the name of the job or null if it has not been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setJobName(String)
     */
    public String getJobName() throws DrmaaException;

    /**
     * Set the job's standard input path.
     * Unless set elsewhere, if not explicitly set in the job template, the job
     * is started with an empty input stream.
     *
     * <p>If set, specifies the network path of the job's input stream in
     * the form of [hostname]:file_path</p>
     *
     * <p>When the transferFiles property is supported and has it's inputStream
     * property set, the input file will be fetched by the underlying DRM
     * system from the specified host or from the submit host if no hostname
     * is specified.</p>
     *
     * <p>When the transferFiles property is not supported or does not have its
     * inputStream property set, the input file is always expected to be
     * at the host where the job is executed irrespective of a whether a
     * hostname is specified in the path.</p>
     *
     * <p>The <CODE>PARAMETRIC_INDEX</CODE> placeholder can be used at any position
     * within the file path of parametric job templates and will be replaced
     * by the underlying DRM system with the parametric job's index.</p>
     *
     * <p>A <CODE>HOME_DIRECTORY</CODE> placeholder at the beginning of the file
     * path denotes that the remaining portion of the file path is relative to
     * the job submiter's home directory on the host where the file is
     * located.</p>
     *
     * <p>A <CODE>WORKING_DIRECTORY</CODE> placeholder at the beginning of the file
     * path denotes that the remaining portion of the file path is relative to
     * the job's working directory on the host where the file is located.</p>
     *
     * <p>The file path must be specified in a syntax that is common at the host
     * where the file is located.</p>
     *
     * <p>When the job is run, if this property is set, and the file can't be read,
     * the job will enter the state Session.FAILED.</p>
     * @param inputPath the job's standard input path
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setInputPath(String inputPath) throws DrmaaException;

    /**
     * Get the job's standard input path.
     * @return the job's standard input path or null if it has not been set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setInputPath(String)
     */
    public String getInputPath() throws DrmaaException;

    /**
     * Sets how to direct the job's standard output.
     * If not explicitly set in the job template, the whereabouts of the jobs
     * output stream is not defined.
     * If set, specifies the network path of the job's output stream file in the
     * form of [hostname]:file_path
     *
     * <p>When the transferFiles property is supported and has its outputStream
     * property set, the output file will be transferred by the underlying
     * DRM system to the specified host or to the submit host if no hostname is
     * specified.</p>
     *
     * <p>When the transferFiles property is not supported or does
     * not have it's outputStream property set, the output file is always kept
     * at the host where the job is executed irrespective of a whether a
     * hostname is specified in the path.</p>
     *
     * <p>The <CODE>PARAMETRIC_INDEX</CODE> placeholder can be used at any position
     * within the file path of parametric job templates and will be replaced
     * by the underlying DRM system with the parametric job's index.</p>
     *
     * <p>A <CODE>HOME_DIRECTORY</CODE> placeholder at the beginning of the file
     * path denotes that the remaining portion of the file path is relative to
     * the job submiter's home directory on the host where the file is
     * located.</p>
     *
     * <p>A <CODE>WORKING_DIRECTORY</CODE> placeholder at the beginning of the file
     * path denotes that the remaining portion of the file path is relative to
     * the job's working directory on the host where the file is located.</p>
     *
     * <p>The file path must be specified in a syntax that is common at the host
     * where the file is located.</p>
     *
     * <p>When the job is run, if this property is set, and the file can't be
     * written before execution the job will enter the state Session.FAILED.</p>
     * @param outputPath how to direct the job's standard output
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setOutputPath(String outputPath) throws DrmaaException;

    /**
     * Gets how to direct the job's standard output.
     * @return how to direct the job's standard output or null if it has not been
     * set
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setOutputPath(String)
     */
    public String getOutputPath() throws DrmaaException;

    /**
     * Sets how to direct the job's standard error.
     * If not explicitly set in the job template, the whereabouts of the job's
     * error stream is not defined. If set, specifies the network path of the
     * job's error stream file in the form [hostname]:file_path
     *
     * <p>When the transferFiles property is supported and has its errorStream
     * property set, the error file will be transferred by the underlying
     * DRM system to the specified host or to the submit host if no hostname is
     * specified.</p>
     *
     * <p>When the transferFiles property is not supported or does
     * not have it's errorStream property set, the error file is always kept
     * at the host where the job is executed irrespective of a whether a
     * hostname is specified in the path.</p>
     *
     * <p>The <CODE>PARAMETRIC_INDEX</CODE> placeholder can be used at any position
     * within the file path of parametric job templates and will be replaced
     * by the underlying DRM system with the parametric job's index.</p>
     *
     * <p>A <CODE>HOME_DIRECTORY</CODE> placeholder at the beginning of the file
     * path denotes that the remaining portion of the file path is relative to
     * the job submiter's home directory on the host where the file is
     * located.</p>
     *
     * <p>A <CODE>WORKING_DIRECTORY</CODE> placeholder at the beginning of the file
     * path denotes that the remaining portion of the file path is relative to
     * the job's working directory on the host where the file is located.</p>
     *
     * <p>The file path must be specified in a syntax that is common at the host
     * where the file is located.</p>
     *
     * <p>When the job is run, if this property is set, and the file can't be
     * written before execution the job will enter the state Session.FAILED.</p>
     * @param errorPath how to direct the job's standard error
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setErrorPath(String errorPath) throws DrmaaException;

    /**
     * Gets how to direct the job's standard error.
     * @return how to direct the job's standard error
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setErrorPath(String)
     */
    public String getErrorPath() throws DrmaaException;

    /**
     * Sets whether the error stream should be intermixed with the output
     * stream. If not explicitly set in the job template this property defaults
     * to <CODE>false</CODE>.
     *
     * <p>If <CODE>true</CODE> is specified the underlying DRM system will ignore
     * the value of the errorPath property and intermix the standard error
     * stream with the standard output stream as specified by the outputPath
     * property.</p>
     * @param join whether the error stream should be intermixed with the output
     * stream
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setJoinFiles(boolean join) throws DrmaaException;

    /**
     * Gets whether the error stream should be intermixed with the output
     * stream.
     * @return Whether the error stream should be intermixed with the output
     * stream
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setJoinFiles(boolean)
     */
    public boolean getJoinFiles() throws DrmaaException;

    /**
     * Specifies, which of the standard I/O files (stdin, stdout and stderr) are
     * to be transferred to/from the execution host.  If not set, defaults to
     * the equivalent of a FileTransferMode instance with all properties set to
     * false.  See inputPath, outputPath and errorPath setters for information
     * about how to specify the standard input file, standard output file, and
     * standard error file.
     *
     * <p>If the FileTransferMode instance's errorStream property is set to
     * <CODE>true</CODE>, the errorPath property is taken to specify the
     * location to which error files should be transfered after the job
     * finishes.</p>
     *
     * <p>If the FileTransferMode instance's inputStream property is set to
     * <CODE>true</CODE>, the inputPath property is taken to specify the
     * location from which input files should be transfered before the job
     * starts.</p>
     *
     * <p>If the FileTransferMode instance's outputStream property is set to
     * <CODE>true</CODE>, the outputPath property is taken to specify the
     * location to which output files should be transfered after the job
     * finishes.</p>
     *
     * @param mode how to transfer files between hosts.
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setInputPath(String)
     * @see #setOutputPath(String)
     * @see #setErrorPath(String)
     */
    public void setTransferFiles(FileTransferMode mode) throws DrmaaException;

    /**
     * Gets how to transfer files between hosts.
     * @return how to transfer files between hosts.
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setTransferFiles(FileTransferMode)
     */
    public FileTransferMode getTransferFiles() throws DrmaaException;

    /**
     * Sets a deadline after which the DRMS will terminate the job.
     * @param deadline the deadline after which the DRMS will terminate the job
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setDeadlineTime(PartialTimestamp deadline)
        throws DrmaaException;

    /**
     * Sets a deadline after which the DRMS will terminate the job.
     * @return the deadline after which the DRMS will terminate the job
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setDeadlineTime(PartialTimestamp)
     */
    public PartialTimestamp getDeadlineTime() throws DrmaaException;

    /**
     * Sets when the job's wall clock time limit has
     * been exceeded.  The DRMS will terminate a job that has exceeded its wall
     * clock time limit.  Note that time spent suspended is also accounted for
     * here.
     * @param hardWallclockLimit when the job's wall clock time limit has been
     * exceeded.  Specified in seconds
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setHardWallclockTimeLimit(long hardWallclockLimit)
        throws DrmaaException;

    /**
     * Gets the duration of the job's wall clock time limit.
     * @return when the job's wall clock time limit has been exceeded.
     * Specified in seconds
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setHardWallclockTimeLimit(long)
     */
    public long getHardWallclockTimeLimit() throws DrmaaException;

    /**
     * Sets an estimate as to how much wall clock time job will need to
     * complete. Note that time spent suspended is also accounted for here.<P>
     *
     * This attribute is intended to assist the scheduler.
     * If the time specified in insufficient, the
     * drmaa-implementation may impose a scheduling penalty.
     * @param softWallclockLimit an estimate as to how much wall clock time job
     * will need to complete.  Specified in seconds
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setSoftWallclockTimeLimit(long softWallclockLimit)
        throws DrmaaException;

    /**
     * Gets an estimate as to how much wall clock time job will need to
     * complete.
     * @return an estimate as to how much wall clock time job will need
     * to complete.  Specified in seconds
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setSoftWallclockTimeLimit(long)
     */
    public long getSoftWallclockTimeLimit() throws DrmaaException;

    /**
     * Sets how long the job may be in a running state before its limit has been
     * exceeded, and therefore is terminated by the DRMS.
     * @param hardRunLimit how long the job may be in a running state before its
     * limit has been exceeded.  Specified in seconds
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setHardRunDurationLimit(long hardRunLimit)
        throws DrmaaException;

    /**
     * Gets how long the job may be in a running state before its limit has been
     * exceeded.
     * @return how long the job may be in a running state before its limit has
     * been exceeded.  Specified in seconds
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setHardRunDurationLimit(long)
     */
    public long getHardRunDurationLimit() throws DrmaaException;

    /**
     * Sets an estimate as to how long the job will need to remain in a running
     * state to complete.  This attribute is intended to assist the scheduler.
     * If the time specified in insufficient, the DRMAA implementation may
     * impose a scheduling penalty.
     * @param softRunLimit an estimate as to how long the job will need to
     * remain in a running state to complete.  Specified in seconds
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidAttributeFormatException -- the format of the argument is
     * invalid</LI>
     * <LI>InvalidAttributeValueException -- the value of the argument is
     * invalid</LI>
     * <LI>ConflictingAttributeValuesException -- the value of the argument
     * conflicts with the value of another job template property</LI>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public void setSoftRunDurationLimit(long softRunLimit)
        throws DrmaaException;

    /**
     * Gets an estimate as to how long the job will need to remain in a running
     * state to complete.
     * @return an estimate as to how long the job will need to remain
     * in a running state to complete.  Specified in seconds
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #setSoftRunDurationLimit(long)
     */
    public long getSoftRunDurationLimit() throws DrmaaException;

    /**
     * Returns the list of supported property names.  This list
     * includes supported DRMAA reserved property names (both required and
     * optional) and DRM-specific property names.
     * @return the list of supported property names
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>NoActiveSessionException -- the session has not yet been initialized
     * or has already been exited</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public Set getAttributeNames() throws DrmaaException;
}
