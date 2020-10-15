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

/**
 * This interface represents the operations available for interacting with
 * the DRM.  In DRMAA, (almost) all DRM interaction occur within the context of
 * a session.  The DRMAA specification also strongly recommends that a DRMAA
 * implementation not allow concurrent sessions.  Since DRMAA has no facility
 * for user authentication or authorization, most DRMAA implementations will
 * likely only support one session per implementation instance.
 *
 * <p>In order to use a Session, it must first be initialized.  Once initialized
 * it is the responsibility of the programmer to ensure that the session also be
 * explicitly terminated.  Otherwise, session artifacts may be left behind on
 * the client and/or server.  A handy way to make sure the Session is terminated
 * is to set up a shutdown hook to call the exit() method on the active
 * session.</p>
 *
 * <p>To get a Session implementation appropriate for the DRM in use, one uses
 * the SessionFactory.getSession() method.</p>
 *
 * <p>Example:</p>
 *
 * <pre>public static void main(String[] args) throws Exception {
 *   SessionFactory factory = SessionFactory.getFactory();
 *   Session session = factory.getSession();
 *
 *   try {
 *      session.init(&quot;&quot;);
 *      JobTemplate jt = session.createJobTemplate();
 *      jt.setRemoteCommand(&quot;sleeper.sh&quot;);
 *      jt.setArgs(Collections.singletonList(&quot;5&quot;));
 *
 *      String id = session.runJob(jt);
 *
 *      session.deleteJobTemplate(jt);
 *
 *      while (session.getJobProgramStatus(id) != Session.RUNNING) {
 *         Thread.sleep(1000);
 *      }
 *
 *      System.out.println(&quot;Job &quot; + id + &quot; is now running.&quot;);
 *
 *      session.control(id, Session.SUSPEND);
 *      Thread.sleep(1000);
 *      session.control(id, Session.RELEASE);
 *
 *      JobInfo info = session.wait(id, Session.TIMEOUT_WAIT_FOREVER);
 *
 *      System.out.println(&quot;Job &quot; + info.getJobId () + &quot; exited with status: &quot; +
 *                         info.getExitStatus ());
 *
 *      session.exit();
 *   }
 *   catch (DrmaaException e) {
 *      System.out.println(&quot;Error: &quot; + e.getMessage ());
 *   }
 * }
 * </pre>
 * @author dan.templeton@sun.com
 * @see java.lang.Runtime#addShutdownHook
 * @see org.ggf.drmaa.SessionFactory
 * @see <a
 * href="http://gridengine.sunsource.net/project/gridengine/howto/drmaa_java.html">Grid
 * Engine DRMAA Java&trade; Language Binding HowTo</a>
 * @since 0.5
 * @version 1.0
 */
public abstract interface Session {
    /**
     * Suspend the job.  Used with the Session.control() method.
     */
    public static final int SUSPEND = 0;
    /**
     * Resume the job.  Used with the Session.control() method.
     */
    public static final int RESUME = 1;
    /**
     * Put the job on hold.  Used with the Session.control() method.
     */
    public static final int HOLD = 2;
    /**
     * Release the hold on the job.  Used with the Session.control() method.
     */
    public static final int RELEASE = 3;
    /**
     * Kill the job.  Used with the Session.control() method.
     */
    public static final int TERMINATE = 4;
    /**
     * All jobs submitted during this DRMAA session.  Used with the
     * Session.control() and Session.synchronize() methods.
     */
    public static final String JOB_IDS_SESSION_ALL =
            "DRMAA_JOB_IDS_SESSION_ALL";
    /**
     * Any job from the session.  Used with the Session.wait() method.
     */
    public static final String JOB_IDS_SESSION_ANY =
            "DRMAA_JOB_IDS_SESSION_ANY";
    /**
     * Wait indefinitely for a result.  Used with the Session.wait() and
     * Session.synchronize() methods.
     */
    public static final long TIMEOUT_WAIT_FOREVER = -1L;
    /**
     * Return immediately if no result is available.  Used with the
     * Session.wait() and Session.synchronize() methods.
     */
    public static final long TIMEOUT_NO_WAIT = 0L;
    /**
     * Job status cannot be determined.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int UNDETERMINED = 0x00;
    /**
     * Job is queued and active.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int QUEUED_ACTIVE = 0x10;
    /**
     * Job is queued and in system hold.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int SYSTEM_ON_HOLD = 0x11;
    /**
     * Job is queued and in user hold.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int USER_ON_HOLD = 0x12;
    /**
     * Job is queued and in user and system hold.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int USER_SYSTEM_ON_HOLD = 0x13;
    /**
     * Job is running.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int RUNNING = 0x20;
    /**
     * Job is system suspended.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int SYSTEM_SUSPENDED = 0x21;
    /**
     * Job is user suspended.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int USER_SUSPENDED = 0x22;
    /**
     * Job is user suspended.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int USER_SYSTEM_SUSPENDED = 0x23;
    /**
     * Job has finished normally.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int DONE = 0x30;
    /**
     * Job finished, but terminated abnormally.  Used with the
     * Session.getJobProgramStatus() method.
     */
    public static final int FAILED = 0x40;
    
    /**
     * Initialize the DRMAA implementation.
     * <i>contact</i> is an implementation-dependent string that may be used to
     * specify which DRM system to use.  This routine must be called before any
     * other DRMAA calls, except for getDrmSystem(),
     * getDrmaaImplementation(), and getContact().
     *
     * <p>If contact is <CODE>null</CODE> or &quot;&quot;, the default DRM system
     * is used, provided there is only one DRMAA implementation available.
     * If there is more than one DRMAA implementation available, init() throws a
     * NoDefaultContactStringSelectedException.</p>
     *
     * <p>init() should be called only once, by only one of the threads. The main
     * thread is recommended.  A call to init() by another thread or additional
     * calls to init() by the same thread with throw a
     * SessionAlreadyActiveException.</p>
     * @param contact implementation-dependent string that may be used to
     * specify which DRM system to use.
     * @throws DrmaaException May be be one of the following:
     * <UL>
     * <LI>DrmsInitException -- the session could not be initialized</LI>
     * <LI>InvalidContactStringException -- the contact string is invalid</LI>
     * <LI>AlreadyActiveSessionException -- the session has already been
     * initialized</LI>
     * <LI>DefaultContactStringException -- no contact string was specified, and
     * the DRMAA implementation was unable to use the default contact
     * information to connect to a DRM</LI>
     * <LI>NoDefaultContactStringSelectedException -- no contact string was
     * specified, and the DRMAA implementation has no default contact
     * information</LI>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>IllegalArgumentException -- an argument is invalid</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public abstract void init(String contact) throws DrmaaException;
    
    /**
     * Disengage from the DRM and allow the DRMAA implementation to perform
     * any necessary internal cleanup.
     * This routine ends the current DRMAA session but doesn't affect any
     * jobs which have already been submitted (e.g., queued and running jobs
     * remain queued and running).
     * exit() should be called only once, by only one of the threads.
     * Additional calls to exit() beyond the first will throw a
     * NoActiveSessionException.
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmsExitException -- an error occured while tearing down the
     * session</LI>
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
    public abstract void exit() throws DrmaaException;
    
    /**
     * Get a new job template.  A job template is used to set the
     * environment for jobs to be submitted.  Once a job template has been
     * created, it should also be deleted (via deleteJobTemplate()) when no
     * longer needed.  Failure to do so may result in a memory leak.
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
     * @return a blank JobTemplate instance
     * @see org.ggf.drmaa.JobTemplate
     */
    public abstract JobTemplate createJobTemplate() throws DrmaaException;
    
    /**
     * Deallocate a job template. This routine has no effect on jobs which have
     * already been submitted.
     * @param jt the JobTemplate to delete
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidJobTemplateException -- the JobTemplate instance does not
     * belong to the current session, was not created with createJobTemplate(),
     * or has already been deleted</LI>
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
    public abstract void deleteJobTemplate(JobTemplate jt)
            throws DrmaaException;
    
    /**
     * Submit a job with attributes defined in the job template, <i>jt</i>.
     * The returned job identifier is a String identical to that returned
     * from the underlying DRM system.
     * @param jt the job template to be used to create the job
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>TryLaterException -- the DRM was temporarily unable to fulfill the
     * request, but a retry might succeed</LI>
     * <LI>DeniedByDrmException -- the DRM has rejected the job due to
     * configuration problems either in the DRM or the job template</LI>
     * <LI>InvalidJobTemplateException -- the JobTemplate instance does not
     * belong to the current session, was not created with createJobTemplate(),
     * or has already been deleted</LI>
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
     * @return job identifier String identical to that returned from the
     * underlying DRM system
     */
    public abstract String runJob(JobTemplate jt) throws DrmaaException;
    
    /**
     * Submit a range of parametric jobs,
     * each with attributes defined in the job template, <i>jt</i>.
     * The returned job identifiers are Strings identical to those returned
     * from the underlying DRM system.  The number of jobs submitted will be
     * Math.floor((<i>end</i> - <i>start</i> + 1) / <i>incr</i>).
     *
     * <p>The JobTemplate class defines a <code>PARAMETRIC_INDEX</code>
     * placeholder for use in specifying paths in the job template.  This
     * placeholder is used to represent the individual identifiers of the tasks
     * submitted through this method.</p>
     *
     * @return job identifier Strings identical to that returned by the
     * underlying DRM system
     * @param start the starting value for the loop index
     * @param end the terminating value for the loop index
     * @param incr the value by which to increment the loop index each iteration
     * @param jt the job template to be used to create the job
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>TryLaterException -- the DRM was temporarily unable to fulfill the
     * request, but a retry might succeed</LI>
     * <LI>DeniedByDrmException -- the DRM has rejected the job due to
     * configuration problems either in the DRM or the job template</LI>
     * <LI>InvalidJobTemplateException -- the JobTemplate instance does not
     * belong to the current session, was not created with createJobTemplate(),
     * or has already been deleted</LI>
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
    public abstract List runBulkJobs(JobTemplate jt, int start, int end,
                                     int incr) throws DrmaaException;
    
    /**
     * <p>Hold, release, suspend, resume, or kill the job identified by
     * <i>jobId</i>.  If <i>jobId</i> is <code>JOB_IDS_SESSION_ALL</code>, then
     * this routine acts on all jobs <B>submitted</B> during this DRMAA session
     * up to the moment control() is called.  To avoid thread races conditions
     * in multithreaded applications, the DRMAA implementation user should
     * explicitly synchronize this call with any other job submission or control
     * calls that may change the number of remote jobs.</p>
     *
     * <p>The legal values for <i>action</i> and their meanings are:</p>
     *
     * <UL>
     * <LI><code>SUSPEND</code>: stop the job,</LI>
     * <LI><code>RESUME</code>: (re)start the job,</LI>
     * <LI><code>HOLD</code>: put the job on-hold,</LI>
     * <LI><code>RELEASE</code>: release the hold on the job, and</LI>
     * <LI><code>TERMINATE</code>: kill the job.</LI>
     * </UL>
     *
     * <p>This method returns once the action has been acknowledged by
     * the DRM system, but it does not necessarily wait until the action
     * has been completed.</p>
     *
     * <p>Some DRMAA implementations may allow this method to be used to control
     * jobs submitted external to the DRMAA session, such as jobs submitted by
     * other DRMAA sessions or jobs submitted via native utilities.</p>
     *
     * <p>If <i>jobId</i> is <code>JOB_IDS_SESSION_ALL</code> and the control
     * action fails for one or more jobs, and InternalException will be thrown,
     * and the state of the jobs in the session will be undefined.</p>
     *
     * @param jobId The id of the job to control
     * @param action the control action to be taken
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>ResumeInconsistentStateException -- an attempt was made to resume a
     * job that was not is a suspended state</LI>
     * <LI>SuspendInconsistentStateException -- an attempt was made to suspend a
     * job that was not is a running state</LI>
     * <LI>HoldInconsistentStateException -- an attempt was made to hold a
     * job that was not is a pending state</LI>
     * <LI>ReleaseInconsistentStateException -- an attempt was made to release a
     * job that was not is a held state</LI>
     * <LI>InvalidJobException -- the job identifier does not refer to an active
     * job</LI>
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
    public abstract void control(String jobId, int action)
            throws DrmaaException;
    
    /**
     * Wait until all jobs specified by <i>jobIds</i> have finished
     * execution.  If <i>jobIds</i> contains <code>JOB_IDS_SESSION_ALL</code>,
     * then this method waits for all jobs <B>submitted</B> during this DRMAA
     * session up to the moment synchronize() is called.  To avoid thread race
     * conditions in multithreaded applications, the DRMAA implementation user
     * should explicitly synchronize this call with any other job submission
     * or control calls that may change the number of remote jobs.
     *
     * <p>To prevent blocking indefinitely in this call, the caller may use a
     * timeout specifying how many seconds to block in this call.  The value
     * <code>TIMEOUT_WAIT_FOREVER</code> may be specified to wait indefinitely
     * for a result.  The value <code>TIMEOUT_NO_WAIT</code> may be specified to
     * return immediately if no result is available. If the call exits before
     * the timeout has elapsed, all the jobs have been waited on or there was an
     * interrupt.  If the invocation exits on timeout, an ExitTimeException is
     * thrown.  The caller should check system time before and after this call
     * in order to be sure of how much time has passed.</p>
     *
     * <p>The <i>dispose</i> parameter specifies how to treat the reaping of the
     * remote jobs' internal data records, which includes a record of the jobs'
     * consumption of system resources during their execution and other
     * statistical information.  If this parameter is set to <code>true</code>,
     * the DRM will dispose of the jobs' data records at the end of
     * the synchroniize() call.  If set to <code>false</code>, the data records
     * will be left for future access via the wait() method.</p>
     *
     * @param jobIds the ids of the jobs to synchronize
     * @param timeout the maximum number of seconds to wait
     * @param dispose specifies how to treat reaping information
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>ExitTimeoutException -- the operation timed out before
     * completing</LI>
     * <LI>InvalidJobException -- the job identifier does not refer to an active
     * job</LI>
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
     * @see #wait
     */
    public abstract void synchronize(List jobIds, long timeout, boolean dispose)
            throws DrmaaException;
    
    /**
     * This method will wait for a job with <i>jobId</i> to finish execution
     * or fail.  If the special string, <code>JOB_IDS_SESSION_ANY</code>, is
     * provided as the <i>jobId</i>, this routine will wait for any job from the
     * session. This routine is modeled on the wait3 POSIX routine.
     *
     * <p>The <i>timeout</i> value is used to specify the desired behavior when
     * a result is not immediately available.  The value,
     * <code>TIMEOUT_WAIT_FOREVER</code>, may be specified to wait indefinitely
     * for a result.  The value, <code>TIMEOUT_NO_WAIT</code>, may be specified
     * to return immediately if no result is available.  Alternatively, a number
     * of seconds may be specified to indicate how long to wait for a result to
     * become available.</p>
     *
     * <p>If the call exits before timeout, either the job has been waited on
     * successfully or there was an interrupt. If the invocation exits on
     * timeout, an ExitTimeoutException is thrown.  The caller should check
     * system time before and after this call in order to be sure how much time
     * has passed.</p>
     *
     * <p>The routine reaps job data records on a successful call, so any
     * subsequent calls to wait(), synchronize(), control(), or
     * getJobProgramStatus() will fail, throwing an InvalidJobException, meaning
     * that the job's data record has been already reaped. This exception is the
     * same as if the job were unknown.  (The only case where wait() can be
     * successfully called on a single job more than once is when the previous
     * call to wait() timed out before the job finished.)</p>
     *
     * <p>When successful, the resource usage information for the job is
     * provided as a Map of usage parameter names and their values in the
     * JobInfo object.  The values contain the amount of resources consumed by
     * the job and are implementation defined.  If no resource usage
     * information is available for the finished job, the resourceUsage
     * property of the returned JobInfo instance will be <code>null</code>.</p>
     *
     * <p>In the 0.5 version of this method, a NoResourceUsageException would be
     * thrown if the target job finished with no resource usage information.  In
     * the current implementation, no exception is thrown in that case.
     * Instead, the JobInfo.getResourceUsage() method will return null.</p>
     *
     * @param jobId the id of the job for which to wait
     * @param timeout the maximum number of seconds to wait
     * @return the resource usage and status information
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>ExitTimeoutException -- the operation timed out before
     * completing</LI>
     * <LI>InvalidJobException -- the job identifier does not refer to an active
     * job</LI>
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
     * @see JobInfo
     */
    public abstract JobInfo wait(String jobId, long timeout)
            throws DrmaaException;
    
    /**
     * Get the program status of the job identified by jobId. The possible
     * values returned from this method are:
     *
     * <UL>
     * <LI><code>UNDETERMINED</code>: process status cannot be determined</LI>
     * <LI><code>QUEUED_ACTIVE</code>: job is queued and active</LI>
     * <LI><code>SYSTEM_ON_HOLD</code>: job is queued and in system hold</LI>
     * <LI><code>USER_ON_HOLD</code>: job is queued and in user hold</LI>
     * <LI><code>USER_SYSTEM_ON_HOLD</code>: job is queued and in user and
     * system hold</LI>
     * <LI><code>RUNNING</code>: job is running</LI>
     * <LI><code>SYSTEM_SUSPENDED</code>: job is system suspended</LI>
     * <LI><code>USER_SUSPENDED</code>: job is user suspended</LI>
     * <LI><code>USER_SYSTEM_SUSPENDED</code>: job is user and system
     * suspended</LI>
     * <LI><code>DONE</code>: job finished normally</LI>
     * <LI><code>FAILED</code>: job finished, but failed.</LI>
     * </UL>
     *
     * <p>The DRMAA implementation must always get the status of jobId from DRM
     * system unless the status has already been determined to be
     * <code>FAILED</code> or <code>DONE</code> and the status has been
     * successfully cached. Terminated jobs return a <code>FAILED</code>
     * status.</p>
     * @return the program status
     * @param jobId the id of the job whose status is to be retrieved
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>InvalidJobException -- the job identifier does not refer to an active
     * job</LI>
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
    public abstract int getJobProgramStatus(String jobId) throws DrmaaException;
    
    /**
     * If called before init(), this method returns a comma delimited String
     * containing the contact Strings available from the default DRMAA
     * implementation, one element per DRM system available. If called after
     * init(), this method returns the contact String for the DRM system to
     * which the session is attached. The returned String is implementation
     * dependent.
     * @return current contact information for DRM system or a comma delimited
     * list of possible contact Strings
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public abstract String getContact();
    
    /**
     * Returns a Version instance containing the major and minor version numbers
     * of the DRMAA library.  For DRMAA 0.5, major is 0 and minor is 5.
     * @return the version number as a Version instance
     * @see Version
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
    public abstract Version getVersion();
    
    /**
     * If called before init(), this method returns a comma delimited list of
     * available DRM systems, one element per DRM system implementation
     * provided. If called after init(), this method returns the selected DRM
     * system. The returned String is implementation dependent.
     * @return DRM system implementation information
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public abstract String getDrmSystem();
    
    /**
     * If called before init(), this method returns a comma delimited list of
     * available DRMAA implementations, one element for each DRMAA
     * implementation provided.  If called after init(), this method returns the
     * selected DRMAA implementation.  The returned String is implementation
     * dependent and may contain the DRM system as a component.
     * @return DRMAA implementation information
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     */
    public abstract String getDrmaaImplementation();
}
