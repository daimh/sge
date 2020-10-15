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

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Arrays;
import java.util.List;
import org.ggf.drmaa.DrmaaException;
import org.ggf.drmaa.InvalidJobTemplateException;
import org.ggf.drmaa.JobInfo;
import org.ggf.drmaa.JobTemplate;
import org.ggf.drmaa.Session;
import org.ggf.drmaa.Version;

/**
 * The SessionImpl class provides a DRMAA interface to Grid Engine.  This
 * interface is built on top of the DRMAA C binding using JNI.  In order to keep
 * the native code as localized as possible, this class also provides native
 * DRMAA services to other classes, such as the JobTemplateImpl.
 *
 * <p>This class relies on the version 1.0 <i>drmaa</i> shared library.</p>
 *
 * @see org.ggf.drmaa.Session
 * @see com.sun.grid.drmaa.JobTemplateImpl
 * @see <a href="http://gridengine.sunsource.net/unbranded-source/browse/~checkout~/gridengine/doc/htmlman/manuals.html?content-type=text/html">Grid Engine Man Pages</a>
 * @author dan.templeton@sun.com
 * @since 0.5
 * @version 1.0
 */
public class SessionImpl implements Session {
    static {
        AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                System.loadLibrary("drmaa");
                return null;
            }
        });
    }
    
    /**
     * Creates a new instance of SessionImpl
     */
    SessionImpl() {
    }
    
    /**
     * <p>Controls a jobs.</p>
     *
     * {@inheritDoc}
     *
     * <p>The DRMAA suspend/resume operations are equivalent to the use of the
     * `-s <i>jobid</i>' and `-us <i>jobid</i>' options with qmod.  (See the qmod(1) man
     * page.)</p>
     *
     * <p>The DRMAA hold/release operations are equivalent to the use of
     * qhold and qrls.  (See the qhold(1) and qrls(1) man pages.)</p>
     *
     * <p>The DRMAA terminate operation is equivalent to the use of qdel.  (See
     * the qdel(1) man page.)</p>
     *
     * <p>Only user hold and user suspend can be controled via control().  For
     * affecting system hold and system suspend states the appropriate DRM
     * interfaces must be used.</p>
     *
     * @param jobId {@inheritDoc}
     * @param action {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qmod.html">qmod(1)</a>
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qhold.html">qhold(1)</a>
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qrls.html">qrls(1)</a>
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qdel.html">qdel(1)</a>
     */
    public void control(String jobId, int action) throws DrmaaException {
        this.nativeControl(jobId, action);
    }

    /**
     * Calls drmaa_control().
     */
    private native void nativeControl(String jobId, int action) throws DrmaaException;
    
    /**
     * The exit() method closes the DRMAA session for all threads and must be
     * called before process termination.  The exit() method may be called only
     * once by a single thread in the process and may only be called after the
     * init() function has completed.  Any call to exit() before init() returns
     * or after exit() has already been called will result in a
     * NoActiveSessionException.
     *
     * <p>The exit() method does neccessary clean up of the DRMAA session state,
     * including unregistering from the qmaster.  If the exit() method is not
     * called, the qmaster will store events for the DRMAA client until the
     * connection times out, causing extra work for the qmaster and comsuming
     * system resources.</p>
     *
     * <p>Submitted jobs are not affected by the exit() method.</p>
     *
     * @throws DrmaaException {@inheritDoc}
     */
    public void exit() throws DrmaaException {
        this.nativeExit();
    }

    /**
     * Calls drmaa_exit().
     */
    private native void nativeExit() throws DrmaaException;
    
    /**
     * getContact() returns an opaque string containing contact information
     * related to the current DRMAA session to be used with the init() method.
     * The contact string takes the form <code>[name=value[;name=value]*]</code>,
     * where <i>name</i> and <i>value</i> are both strings, and the supported
     * values of <i>name</i> are:
     *
     * <ul>
     *    <li><code>session</code> - if used, indicates to which session id
     *        to reconnect.</li>
     * </ul>
     *
     * <p>Before the init() method has been called, this method will always
     * return an empty string.  After the init() method has been called, this
     * method will return the set of name=value pairs which represent the
     * currently active session.  The value returned for <code>session</code>
     * can be used with the init() method to reconnect to the current session
     * after exit() has been called.</p>
     *
     * @return {@inheritDoc}
     * @see #init(String)
     */
    public String getContact() {
        return this.nativeGetContact();
    }

    /**
     * Calls drmaa_get_contact().
     */
    private native String nativeGetContact();
    
    /**
     * The getDRMSystem() method returns a string containing the DRM product and
     * version information.  The getDRMSystem() function returns the same value
     * before and after init() is called.
     * @return {@inheritDoc}
     * @see #init(String)
     */
    public String getDrmSystem() {
        return this.nativeGetDRMSInfo();
    }

    /**
     * Calls drmaa_get_DRM_system().
     */
    private native String nativeGetDRMSInfo();
    
    /**
     * The getDrmaaImplementation() method returns a string containing the DRMAA
     * Java language binding implementation version information.  The
     * getDrmaaImplementation() method returns the same value before and after
     * init() is called.
     * @return {@inheritDoc}
     * @see #init(String)
     */
    public String getDrmaaImplementation() {
        /* Because the DRMAA implementation is tightly bound to the DRM, there's
         * no need to distinguish between them.  Version information can be
         * gotten from getVersion() and language information is self-evident. */
        return this.getDrmSystem();
    }
    
    /**
     * <p>Get the job programm status.</p>
     *
     * {@inheritDoc}
     *
     * <p>The control method can be used to control job submitted outside of the scope
     * of the DRMAA session as long as the job identifier for the job is known.</p>
     * @return {@inheritDoc}
     * @param jobId {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public int getJobProgramStatus(String jobId) throws DrmaaException {
        return this.nativeGetJobProgramStatus(jobId);
    }

    /**
     * Calls drmaa_job_ps().
     */
    private native int nativeGetJobProgramStatus(String jobId) throws DrmaaException;
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public JobTemplate createJobTemplate() throws DrmaaException {
        int id = nativeAllocateJobTemplate();
        
        return new JobTemplateImpl(this, id);
    }
    
    /**
     * {@inheritDoc}
     * @param jt {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public void deleteJobTemplate(JobTemplate jt) throws DrmaaException {
        if (jt == null) {
            throw new NullPointerException("JobTemplate is null");
        } else if (jt instanceof JobTemplateImpl) {
            nativeDeleteJobTemplate(((JobTemplateImpl)jt).getId());
        } else {
            throw new InvalidJobTemplateException();
        }
    }
    
    /**
     * {@inheritDoc}
     * @return {@inheritDoc}
     */
    public Version getVersion() {
        return new Version(1, 0);
    }
    
    /**
     * The init() method initializes the Grid Engine DRMAA API library for
     * all threads of the process and creates a new DRMAA Session. This routine
     * must be called once before any other DRMAA call, except for
     * getDrmSystem(), getContact(), and getDrmaaImplementation().
     *
     * <p><i>contact</i> is an implementation dependent string which may be used
     * to specify which Grid Engine cell to use.  The contact string is composed
     * of a series of name=value pairs separated by semicolons.  The supported
     * name=value pairs are:</p>
     *
     * <ul>
     *    <li>
     *      <code>session</code>: the id of the session to which to reconnect
     *    </li>
     * </ul>
     *
     * <p>If <i>contact</i> is null or empty, the default Grid Engine cell will
     * be used.</p>
     *
     * <p>Except for the above listed methods, no DRMAA methods may be called
     * before the init() function <b>completes</b>.  Any DRMAA method which is
     * called before the init() method completes will throw a
     * NoActiveSessionException.  Any additional call to init() by any thread
     * will throw a SessionAlreadyActiveException.</p>
     *
     * <p>Once init() has been called, it is the responsibility of the developer
     * to ensure that the exit() will be called before the program
     * terminates.</p>
     *
     * @param contact {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see #getContact()
     * @see #exit()
     */
    public void init(String contact) throws DrmaaException {
        this.nativeInit(contact);
    }

    /**
     * Calls drmaa_init().
     */
    private native void nativeInit(String contact) throws DrmaaException;
    
    /**
     * The runBulkJobs() method submits a Grid Engine array job very much as
     * if the qsub option `-t <i>start</i>-<i>end</i>:<i>incr</i>' had been used
     * with the corresponding attributes defined in the DRMAA JobTemplate,
     * <i>jt</i>.  The same constraints regarding qsub -t value ranges also
     * apply to the parameters <i>start</i>, <i>end</i>, and <i>incr</i>.  See
     * the qsub(1) man page for more information.
     *
     * <p>On success a String array containing job identifiers for each array
     * job task is returned.</p>
     *
     * @return {@inheritDoc}
     * @param start {@inheritDoc}
     * @param end {@inheritDoc}
     * @param incr {@inheritDoc}
     * @param jt {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @see <a href="http://gridengine.sunsource.net/nonav/source/browse/~checkout~/gridengine/doc/htmlman/htmlman1/qsub.html">qsub(1)</a>
     */
    public List runBulkJobs(JobTemplate jt, int start, int end, int incr) throws DrmaaException {
        if (jt == null) {
            throw new NullPointerException("JobTemplate is null");
        } else if (jt instanceof JobTemplateImpl) {
            String[] jobIds =
                    this.nativeRunBulkJobs(((JobTemplateImpl)jt).getId(),
                                           start, end, incr);
            
            return Arrays.asList(jobIds);
        } else {
            throw new InvalidJobTemplateException();
        }
    }

    /**
     * Calls drmaa_run_bulk_jobs().
     */
    private native String[] nativeRunBulkJobs(int jtId, int start, int end, int incr) throws DrmaaException;
    
    /**
     * The runJob() method submits a Grid Engine job with attributes defined in
     * the DRMAA JobTemplate <i>jt</i>. On success, the job identifier is
     * returned.
     * @param jt {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     * @return {@inheritDoc}
     */
    public String runJob(JobTemplate jt) throws DrmaaException {
        if (jt == null) {
            throw new NullPointerException("JobTemplate is null");
        } else if (jt instanceof JobTemplateImpl) {
            return this.nativeRunJob(((JobTemplateImpl)jt).getId());
        } else {
            throw new InvalidJobTemplateException();
        }
    }

    /**
     * Calls drmaa_run_job().
     */
    private native String nativeRunJob(int jtId) throws DrmaaException;
    
    /**
     * {@inheritDoc}
     * @param jobIds {@inheritDoc}
     * @param timeout {@inheritDoc}
     * @param dispose {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public void synchronize(List jobIds, long timeout, boolean dispose) throws DrmaaException {
        this.nativeSynchronize((String[])jobIds.toArray(new String[jobIds.size()]), timeout, dispose);
    }

    /**
     * Calls drmaa_synchronize().
     */
    private native void nativeSynchronize(String[] jobIds, long timeout, boolean dispose) throws DrmaaException;
    
    /**
     * {@inheritDoc}
     * @param jobId {@inheritDoc}
     * @param timeout {@inheritDoc}
     * @throws DrmaaException {@inheritDoc}
     */
    public JobInfo wait(String jobId, long timeout) throws DrmaaException {
        JobInfoImpl jobInfo = this.nativeWait(jobId, timeout);
        
        return jobInfo;
    }

    /**
     * Calls drmaa_wait().
     */
    private native JobInfoImpl nativeWait(String jobId, long timeout) throws DrmaaException;

    /**
     * Calls drmaa_allocate_job_template() to create a native job template and
     * stores that template reference in a table.  The table index of the
     * reference is then returned.
     * @return the table index for the native job template
     */
    private native int nativeAllocateJobTemplate() throws DrmaaException;

    /**
     * Calls drmaa_set_attribute() on the native job template found using the
     * provided index.
     * @param jtId the table index for the native job template
     */
    native void nativeSetAttributeValue(int jtId, String name, String value) throws DrmaaException;

    /**
     * Calls drmaa_set_vector_attribute() on the native job template found using
     * the provided index.
     * @param jtId the table index for the native job template
     */
    native void nativeSetAttributeValues(int jtId, String name, String[] values) throws DrmaaException;
    
    /**
     * Calls drmaa_get_attribute_names() and drmaa_get_vector_attribute_names()
     * on the native job template found using the provided index.
     * @param jtId the table index for the native job template
     */
    native String[] nativeGetAttributeNames(int jtId) throws DrmaaException;
    
    /**
     * Calls drmaa_get_attribute() or drmaa_get_vector_attribute() (as
     * appropriate) on the native job template found using the provided index.
     * @param jtId the table index for the native job template
     */
    native String[] nativeGetAttribute(int jtId, String name) throws DrmaaException;

    /**
     * Calls drmaa_delete_job_template() with the native job template found
     * using the provided index.
     * @param jtId the table index for the native job template
     */
    native void nativeDeleteJobTemplate(int jtId) throws DrmaaException;
}
