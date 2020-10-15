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
package com.sun.grid.jgdi.rmi;

import com.sun.grid.jgdi.monitoring.ClusterQueueSummaryOptions;
import com.sun.grid.jgdi.monitoring.QHostOptions;
import com.sun.grid.jgdi.monitoring.QHostResult;
import com.sun.grid.jgdi.monitoring.QQuotaOptions;
import com.sun.grid.jgdi.monitoring.QQuotaResult;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryResult;
import java.io.File;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.List;


/**
 * Base interface for the JGDIRemote.
 *
 * @jgdi.todo   optional ??
 *         <p>add qquota functions</p>
 *
 * @jgdi.todo   optional ??
 *         <p>add qstat functions</p>
 *
 * @jgdi.todo   optional ??
 *         <p>add qhost functions</p>
 * @jgdi.todo   optional ??
 *         <p>add qmod functions</p>
 */
public interface JGDIRemoteBase extends Remote {
    
    
    /**
     *  Get the admin user
     *  @return the admin user
     *  @throws RemoteException on any error on the GDI layer
     */
    public String getAdminUser() throws RemoteException;
    
    
    
    /**
     * Get the root directory of the grid engine installation
     * @throws RemoteException
     * @return root directory of the gridengine installation
     */
    public File getSGERoot() throws RemoteException;
    
    /**
     * Get the cell of the grid engine
     * @return the cell of the grid engine
     * @throws RemoteException
     */
    public String getSGECell() throws RemoteException;
    
    
    
    /**
     *  Get the hostname of the current qmaster
     *  @return the hostname of the current qmaster
     *  @throws RemoteException on any error on the GDI layer
     */
    public String getActQMaster() throws RemoteException;
    
    
    /**
     *   Close the <code>JGDI</code> object. After calling this method
     *   the <code>JGDI</code> is not longer useable.
     *   @throws RemoteException on any error on the GDI layer
     */
    public void close() throws RemoteException;
    
    
    
    /**
     * Get the list of real exec hosts (excludes "template" and "global").
     *
     * @throws RemoteException
     * @return the list of real exec hosts
     */
    public List getRealExecHostList() throws RemoteException;
    
    // -------- Monitoring interface --------------------------------------------
    
    /**
     *  Get summary information about cluster hosts.
     *
     *  The CLI equivialent of this method is <code>qhost</code> (see man qhost(1)).
     *
     *  @param options  host summary options
     *  @return the <code>QHostResult</code> object which the host summary information
     *  @throws RemoteException on any error in the GDI layer
     */
    public QHostResult execQHost(QHostOptions options) throws RemoteException;
    
    /**
     *  Get summary information about cluster queues.
     *
     *  The CLI equivialent of this method is <code>qstat -g c</code> (see man qstat(1)).
     *
     *  @param options  options the cluster queue summary algorithm
     *  @throws RemoteException on any error in the GDI layer
     */
    public List getClusterQueueSummary(ClusterQueueSummaryOptions options) throws RemoteException;
    
    /**
     *  Get summary information abouter queue instances.
     *
     *  The CLI equivialent of this method is <code>qstat</code> (see man qstat(1)).
     *  @param options Options for the queue instance summary algorithm
     *  @return the queue instance summary object
     *  @throws RemoteException on any error in the GDI layer
     */
    public QueueInstanceSummaryResult getQueueInstanceSummary(QueueInstanceSummaryOptions options) throws RemoteException;
    
    /**
     *  Get the limit information.
     *
     *  The CLI equivialent of this method is <code>qquota</code> (see man qquota(1)).
     *
     *  @param options the options for the qquota algorithm
     *  @return  list of {@link com.sun.grid.jgdi.monitoring.QQuotaResult} instances
     */
    public QQuotaResult getQQuota(QQuotaOptions options) throws RemoteException;
    
    
    // -------- Managing interface methods --------------------------------------
    /**
     * <p>Clears all user and project usage from  the  sharetree.
     * All usage will be initialized back to zero.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -clearusage</code> (see man qconf(1)).</p>
     *
     * @throws RemoteException
     */
    public void clearShareTreeUsage() throws RemoteException;
    
    /**
     * <p>Cleans queue from jobs which haven't been reaped. Primarily for
     *    development purpose. Requires root/manager/operator privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -kcq</code> (see man qconf(1)).</p>
     *
     * @param queues a wild card queue list
     * @throws RemoteException on any error on the GDI layer
     */
    public void cleanQueues(String[] queues) throws RemoteException;
    
    
    /**
     * Send the qmaster a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -km</code> (see man qconf(1)).</p>
     *
     * @throws RemoteException on any error on the GDI layer
     */
    public void killMaster() throws RemoteException;
    
    /**
     * Send the schedduler a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ks</code> (see man qconf(1)).</p>
     *
     * @throws RemoteException on any error on the GDI layer
     */
    public void killScheduler() throws RemoteException;
    
    /**
     * Send some execution hosts a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ke</code> (see man qconf(1)).</p>
     *
     * @param hosts         a host name list
     * @param terminateJobs if true, all jobs running on the execution hosts are
     *                      aborted prior to termination of the corresponding
     *                      sge_execd(8).
     * @throws RemoteException on any error on the GDI layer
     */
    public void killExecd(String[] hosts, boolean terminateJobs) throws RemoteException;
    
    
    /**
     * Send all execution hosts a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ke all</code> (see man qconf(1)).</p>
     *
     * @param terminateJobs if true, all jobs running on the execution hosts are
     *                      aborted prior to termination of the corresponding
     *                      sge_execd(8).
     * @throws RemoteException on any error on the GDI layer
     */
    public void killAllExecds(boolean terminateJobs) throws RemoteException;
    
    /**
     * <p>Used to shutdown event clients registered at qmaster(8), except
     *    special clients like the sge_schedd(8)</p>
     * <p>Requires root or manager privilege.</p>
     * <p>The CLI equivialent for this method is <code>qconf -secl</code> (see man qconf(1)).</p>
     *
     * @param ids  array with the ids of the event clients which should be killed
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void killEventClients(int [] ids) throws RemoteException;
    
    /**
     * <p>Used to shutdown event clients all registered at qmaster(8).</p>
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void killAllEventClients() throws RemoteException;
    
    
    /**
     * <p>The Sun&trade; Grid Engine scheduler sge_schedd(8) is forced to print
     *   trace messages of its  next scheduling run to the file
     *  &lt;sge_root&gt;/&lt;cell&gt;/common/schedd_runlog.
     *   The messages indicate the reasons for jobs and queues not being
     *   selected in that run.</p>
     * <p>Requires root or manager privileges.</p>
     * <p>The CLI equivialent for this method is <code>qconf -tsm</code> (see man qconf(1)).</p>
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void triggerSchedulerMonitoring() throws RemoteException;
    
    /**
     * <p>Return the host where the Sun&trade; Grid Engine scheduler sge_schedd(8) is
     *   active or null otherwise</p>
     * <p>The CLI equivialent for this method is <code>qconf -sss</code> (see man qconf(1)).</p>
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public String getSchedulerHost() throws RemoteException;
    
    
    /**
     * <p>Enable queue(s).</p>
     * <p>The CLI equivialent for this method is <code>qmod -e<code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void enableQueues(String[] queues, boolean force) throws RemoteException;
    
    /**
     * <p>Disable queue(s),  i.e. no further jobs are dispatched to disabled
     *  queues while jobs already executing in these queues are allowed to finish.</p>
     * <p>The CLI equivialent for this method is <code>qmod -d<code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void disableQueues(String[] queues, boolean force) throws RemoteException;
    
    /**
     * <p>Suspend the  queues  and  any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is qmod -sq (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void suspendQueues(String[] queues, boolean force) throws RemoteException;
    
    
    /**
     * <p>Suspends job(s). If a job is  both suspended explicitly and via
     * suspension of its queue, a following unsuspend  of  the  queue  will  not
     * release the suspension state on the job.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -sj</code> (see man qmod(1)).</p>
     *
     * @param jobs   a wildcard job list
     * @param force  force the action
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void suspendJobs(String[] jobs, boolean force) throws RemoteException;
    
    
    /**
     * <p>Unsuspends the queues and any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -usq</code>
     *    (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void unsuspendQueues(String[] queues, boolean force) throws RemoteException;
    
    
    /**
     * <p>Unsuspends the job(s). If a job is both  suspended  explicitly
     * and via suspension of its queue, a following unsuspend of the
     * queue will not release the suspension state on the job.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -usj</code>
     *    (see man qmod(1)).</p>
     *
     * @param  jobs   a wildcard queue list
     * @param  force  force the action
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void unsuspendJobs(String[] jobs, boolean force) throws RemoteException;
    
    /**
     * <p>Clears the error state of the specified queues(s).</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -cq<code>
     *    (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   <p>Force the modification action for the queue
     *                   despite the apparent  current  state of the queue.</p>
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void clearQueues(String[] queues, boolean force) throws RemoteException;
    
    /**
     * <p>Clears the error state of the specified jobs(s).</p>
     *
     * <p>The CLI equivialent for this method is qmod -cj (see man qmod(1)).</p>
     *
     * @param jobs   a wildcard job list
     * @param force  Force the modification action for the job(s)
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void clearJobs(String[] jobs, boolean force) throws RemoteException;
    
    
    /**
     *  <p>Reschedules  all  jobs  currently running  in  the  queue(s). Requires
     *  root  or  manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -r</code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   Force the modification action for the queue
     *                despite the apparent  current  state of the queue.
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void rescheduleQueues(String[] queues, boolean force) throws RemoteException;
    
    
    /**
     * <p>If applied  to  running  jobs,  reschedules  the  jobs.
     * Requires root or manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -r</code> (see man qmod(1)).</p>
     *
     * @param jobs   a wildcard job range list
     * @param force  Force the modification action for the job(s)
     * @throws RemoteException
     * @throws RemoteException RemoteException on any error on the GDI level
     */
    public void rescheduleJobs(String[] jobs, boolean force) throws RemoteException;
    
    
}
