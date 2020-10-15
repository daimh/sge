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

import java.util.Map;

/**
 * This interface represents the status and usage information for a finished
 * job.  It provides access to the job's id, the job's exit status, and a table
 * indicating the amount of resources used during the execution of the job.  The
 * resource table contents are dependent on the DRM.
 *
 * <p>Example</p>
 *
 * <pre>public static void main (String[] args) {
 *   SessionFactory factory = SessionFactory.getFactory();
 *   Session session = factory.getSession();
 *
 *   try {
 *      session.init (&quot;&quot;);
 *      JobTemplate jt = session.createJobTemplate();
 *      jt.setRemoteCommand(&quot;sleeper.sh&quot;);
 *      jt.setArgs(Collections.singletonList(&quot;5&quot;)};
 *
 *      String id = session.runJob(jt);
 *
 *      session.deleteJobTemplate(jt);
 *
 *      JobInfo info = session.wait(id, Session.TIMEOUT_WAIT_FOREVER);
 *
 *      // Interrogate job exit status
 *      if (info.wasAborted()) {
 *         System.out.println(&quot;Job &quot; + info.getJobId() + &quot; never ran&quot;);
 *      } else if (info.hasExited()) {
 *         System.out.println(&quot;Job &quot; + info.getJobId() +
 *                            &quot; finished regularly with exit status &quot; +
 *                            info.getExitStatus());
 *      } else if (info.hasSignaled()) {
 *         System.out.println(&quot;Job &quot; + info.getJobId() +
 *                            &quot; finished due to signal &quot; +
 *                            info.getTerminatingSignal());
 *
 *         if (info.hasCoreDump()) {
 *            System.out.println(&quot;A core dump is available.&quot;);
 *         }
 *      } else {
 *         System.out.println(&quot;Job &quot; + info.getJobId() +
 *                            &quot; finished with unclear conditions&quot;);
 *      }
 *
 *      System.out.println (&quot;\nJob Usage:&quot;);
 *
 *      Map rmap = info.getResourceUsage();
 *      Iterator i = rmap.keySet().iterator();
 *
 *      while (i.hasNext()) {
 *         String name = (String)i.next();
 *         String value = (String)rmap.get(name);
 *
 *         System.out.println(&quot;  &quot; + name + &quot;=&quot; + value);
 *      }
 *
 *      session.exit();
 *   } catch (DrmaaException e) {
 *      System.out.println (&quot;Error: &quot; + e.getMessage());
 *   }
 * }
 * </pre>
 * @author dan.templeton@sun.com
 * @since 0.4.2
 * @version 1.0
 */
public abstract interface JobInfo {
    /**
     * Get the id of the finished job.
     * @return the job id
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
    public String getJobId() throws DrmaaException;
    
    /**
     * Get the resource usage data for the finished job.  If the job finished,
     * but no resource usage data is available, this method will return
     * <code>null</code>.
     * @return the resource usage data
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
    public Map getResourceUsage() throws DrmaaException;
    
    /**
     * Returns <CODE>true</CODE> if the job terminated normally.
     * <CODE>False</CODE> can also indicate that
     * although the job has terminated normally, an exit status is not
     * available, or that it is not known whether the job terminated normally.
     * In both cases getExitStatus() will throw an IllegalStateException.
     * <CODE>True</CODE> indicates that more detailed diagnosis can be
     * discovered by means of getExitStatus().
     * @return if the job has exited
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #getExitStatus()
     */
    public boolean hasExited() throws DrmaaException;
    
    /**
     * If hasExited() returns true,  this function returns the exit code
     * that the job passed to _exit() (see exit(2)) or exit(3C)), or the value
     * that the child process returned from its main method.
     * @return the exit code for the job
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * <LI>IllegalStateException -- if no exit state information is
     * available</LI>
     * </UL>
     * @see #hasExited()
     */
    public int getExitStatus() throws DrmaaException;
    
    /**
     * Returns <CODE>true</CODE> if the job terminated due to the receipt
     * of a signal. <CODE>False</CODE> can also indicate that although the
     * job has terminated due to the receipt of a signal, the signal is not
     * available, or that it is not known whether the job terminated due to
     * the receipt of a signal. In both cases getTerminatingSignal() will throw
     * an IllegalStateException.  <CODE>True</CODE> indicates that the name of
     * the terminating signal can be discovered by means of
     * getTerminatingSignal().
     * @return if the job exited on a signal
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * </UL>
     * @see #getTerminatingSignal()
     */
    public boolean hasSignaled() throws DrmaaException;
    
    /**
     * If hasSignaled() returns <CODE>true</CODE>, this method returns a
     * representation of the signal that caused the termination of the job. For
     * signals declared by POSIX, the symbolic names are be returned (e.g.,
     * SIGABRT, SIGALRM).<P>
     *
     * For signals not declared by POSIX, a DRM dependent string is returned.
     * @return the name of the terminating signal
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * <LI>IllegalStateException -- if the job did not terminate due to the
     * receipt of a signal</LI>
     * </UL>
     * @see #hasSignaled()
     */
    public String getTerminatingSignal() throws DrmaaException;
    
    /**
     * If hasSignaled() returns <CODE>true</CODE>, this function returns
     * <CODE>true</CODE> if a core image of the terminated job was created.
     * @return whether a core dump image was created
     * @throws DrmaaException May be one of the following:
     * <UL>
     * <LI>DrmCommunicationException -- the DRMAA implementation was unable to
     * contact the DRM</LI>
     * <LI>AuthorizationException -- the executing user does not have
     * sufficient permissions to execute the desired action</LI>
     * <LI>InternalException -- an error has occured in the DRMAA
     * implementation</LI>
     * <LI>IllegalStateException -- if the job did not terminate due to the
     * receipt of a signal</LI>
     * </UL>
     * @see #hasSignaled()
     */
    public boolean hasCoreDump() throws DrmaaException;
    
    /**
     * Returns <CODE>true</CODE> if the job ended before entering the running
     * state.
     * @return whether the job ended before entering the running state
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
    public boolean wasAborted() throws DrmaaException;
}
