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
package com.sun.grid.jgdi;

import com.sun.grid.jgdi.configuration.ExecHost;
import com.sun.grid.jgdi.configuration.JGDIAnswer;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummary;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummaryOptions;
import com.sun.grid.jgdi.monitoring.QQuotaResult;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QHostOptions;
import com.sun.grid.jgdi.monitoring.QHostResult;
import com.sun.grid.jgdi.monitoring.QQuotaOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryResult;
import java.io.File;
import java.util.List;

/**
 * Base class for the <code>JGDI</code> object.
 */
public interface JGDIBase {
    
    /**
     *  Get the admin user
     *  @return the admin user
     *  @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public String getAdminUser() throws JGDIException;
    
    /**
     * Get the root directory of the grid engine installation
     * @throws com.sun.grid.jgdi.JGDIException
     * @return root directory of the gridengine installation
     */
    public File getSGERoot() throws JGDIException;
    
    /**
     * Get the cell of the grid engine
     * @return the cell of the grid engine
     * @throws com.sun.grid.jgdi.JGDIException
     */
    public String getSGECell() throws JGDIException;
    
    /**
     *  Get the hostname of the current qmaster
     *  @return the hostname of the current qmaster
     *  @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public String getActQMaster() throws JGDIException;

    /**
     *  Get the SGE_QMASTER_PORT of the current qmaster
     *  @return the SGE_QMASTER_PORT of the current qmaster
     *  @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public int getSgeQmasterPort() throws JGDIException;

    /**
     *  Get the SGE_EXECD_PORT of the current qmaster
     *  @return the SGE_EXECD_PORT of the current qmaster
     *  @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public int getSgeExecdPort() throws JGDIException;

    /**
     *   Close the <code>JGDI</code> object. After calling this method
     *   the <code>JGDI</code> is not longer useable.
     *   @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void close() throws com.sun.grid.jgdi.JGDIException;
    
    /**
     * Get the list of real exec hosts (excludes "template" and "global").
     *
     * @throws com.sun.grid.jgdi.JGDIException
     * @return the list of real exec hosts
     */
    public List<ExecHost> getRealExecHostList() throws com.sun.grid.jgdi.JGDIException;
    
    // -------- Monitoring interface --------------------------------------------
    /**
     *  Get summary information about cluster hosts.
     *
     *  The CLI equivialent of this method is <code>qhost</code> (see man qhost(1)).
     *
     *  @param options  host summary options
     *  @return the <code>QHostResult</code> object with the host summary information
     *  @throws com.sun.grid.jgdi.JGDIException on any error in the GDI layer
     */
    public QHostResult execQHost(QHostOptions options) throws JGDIException;
    
    /**
     *  Get summary information about cluster queues.
     *
     *  The CLI equivialent of this method is <code>qstat -g c</code> (see man qstat(1)).
     *
     *  @param options  options the cluster queue summary algorithm
     *  @throws com.sun.grid.jgdi.JGDIException on any error in the GDI layer
     */
    public List<ClusterQueueSummary> getClusterQueueSummary(ClusterQueueSummaryOptions options) throws JGDIException;
    
    /**
     *  Get summary information abouter queue instances.
     *
     *  The CLI equivialent of this method is <code>qstat</code> (see man qstat(1)).
     *  @param options Options for the queue instance summary algorithm
     *  @return the queue instance summary object
     *  @throws com.sun.grid.jgdi.JGDIException on any error in the GDI layer
     */
    public QueueInstanceSummaryResult getQueueInstanceSummary(QueueInstanceSummaryOptions options) throws JGDIException;
    
    /**
     *  Get the limit information.
     *
     *  The CLI equivialent of this method is <code>qquota</code> (see man qquota(1)).
     *
     *  @param options  qquota summary options
     *  @return the <code>QQuotaResult</code> object with the qquota summary information
     *  @throws com.sun.grid.jgdi.JGDIException on any error in the GDI layer
     */
    public QQuotaResult getQQuota(QQuotaOptions options) throws JGDIException;
    
    // -------- Managing interface methods --------------------------------------
    /**
     * <p>Clears all user and project usage from  the  sharetree.
     * All usage will be initialized back to zero.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -clearusage</code> (see man qconf(1)).</p>
     *
     * @throws com.sun.grid.jgdi.JGDIException
     */
    public void clearShareTreeUsage() throws JGDIException;
    
    /**
     * <p>Clears all user and project usage from  the  sharetree.
     * All usage will be initialized back to zero.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -clearusage</code> (see man qconf(1)).</p>
     *
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException
     */
    public void clearShareTreeUsageWithAnswer(List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Cleans queue from jobs which haven't been reaped. Primarily for
     *    development purpose. Requires root/manager/operator privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -cq</code> (see man qconf(1)).</p>
     *
     * @param queues a wild card queue list
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void cleanQueues(String[] queues) throws JGDIException;
    
    /**
     * <p>Cleans queue from jobs which haven't been reaped. Primarily for
     *    development purpose. Requires root/manager/operator privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -cq</code> (see man qconf(1)).</p>
     *
     * @param queues a wild card queue list
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void cleanQueuesWithAnswer(String[] queues, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * Send the qmaster a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -km</code> (see man qconf(1)).</p>
     *
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void killMaster() throws JGDIException;
    
    /**
     * Send the qmaster a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -km</code> (see man qconf(1)).</p>
     *
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void killMasterWithAnswer(List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * Send the schedduler a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ks</code> (see man qconf(1)).</p>
     *
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void killScheduler() throws JGDIException;
    
    /**
     * Send the schedduler a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ks</code> (see man qconf(1)).</p>
     *
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void killSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    /**
     * Send the schedduler a start signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -at scheduler</code> (see man qconf(1)).</p>
     *
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void startScheduler() throws JGDIException;
    
    /**
     * Send the schedduler a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -at scheduler</code> (see man qconf(1)).</p>
     *
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void startSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException;
    
    
    /**
     * Send some execution hosts a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ke</code> (see man qconf(1)).</p>
     *
     * @param hosts         a host name list
     * @param terminateJobs if true, all jobs running on the execution hosts are
     *                      aborted prior to termination of the corresponding
     *                      sge_execd(8).
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void killExecd(String[] hosts, boolean terminateJobs) throws JGDIException;
    
    /**
     * Send some execution hosts a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ke</code> (see man qconf(1)).</p>
     *
     * @param hosts         a host name list
     * @param terminateJobs if true, all jobs running on the execution hosts are
     *                      aborted prior to termination of the corresponding
     *                      sge_execd(8).
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void killExecdWithAnswer(String[] hosts, boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * Send all execution hosts a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ke all</code> (see man qconf(1)).</p>
     *
     * @param terminateJobs if true, all jobs running on the execution hosts are
     *                      aborted prior to termination of the corresponding
     *                      sge_execd(8).
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void killAllExecds(boolean terminateJobs) throws JGDIException;
    
    /**
     * Send all execution hosts a kill signal.
     *
     * <p>The CLI equivialent for this method is <code>qconf -ke all</code> (see man qconf(1)).</p>
     *
     * @param terminateJobs if true, all jobs running on the execution hosts are
     *                      aborted prior to termination of the corresponding
     *                      sge_execd(8).
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     */
    public void killAllExecdsWithAnswer(boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Used to shutdown event clients registered at qmaster(8), except
     *    special clients like the sge_schedd(8)</p>
     * <p>Requires root or manager privilege.</p>
     * <p>The CLI equivialent for this method is <code>qconf -secl</code> (see man qconf(1)).</p>
     *
     * @param ids  array with the ids of the event clients which should be killed
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void killEventClients(int[] ids) throws JGDIException;
    
    /**
     * <p>Used to shutdown event clients registered at qmaster(8), except
     *    special clients like the sge_schedd(8)</p>
     * <p>Requires root or manager privilege.</p>
     * <p>The CLI equivialent for this method is <code>qconf -secl</code> (see man qconf(1)).</p>
     *
     * @param ids  array with the ids of the event clients which should be killed
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void killEventClientsWithAnswer(int[] ids, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Used to shutdown event clients all registered at qmaster(8).</p>
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void killAllEventClients() throws JGDIException;
    
    /**
     * <p>Used to shutdown event clients all registered at qmaster(8).</p>
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void killAllEventClientsWithAnswer(List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>The SGE scheduler sge_schedd(8) is forced to print
     *   trace messages of its  next scheduling run to the file
     *  &lt;sge_root&gt;/&lt;cell&gt;/common/schedd_runlog.
     *   The messages indicate the reasons for jobs and queues not being
     *   selected in that run.</p>
     * <p>Requires root or manager privileges.</p>
     * <p>The CLI equivialent for this method is <code>qconf -tsm</code> (see man qconf(1)).</p>
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void triggerSchedulerMonitoring() throws JGDIException;
    
    /**
     * <p>The N1 Grid Engine scheduler sge_schedd(8) is forced to print
     *   trace messages of its  next scheduling run to the file
     *  &lt;sge_root&gt;/&lt;cell&gt;/common/schedd_runlog.
     *   The messages indicate the reasons for jobs and queues not being
     *   selected in that run.</p>
     * <p>Requires root or manager privileges.</p>
     * <p>The CLI equivialent for this method is <code>qconf -tsm</code> (see man qconf(1)).</p>
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void triggerSchedulerMonitoringWithAnswer(List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Return the host where the SGE scheduler sge_schedd(8) is
     *   active or null otherwise</p>
     * <p>The CLI equivialent for this method is <code>qconf -sss</code> (see man qconf(1)).</p>
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public String getSchedulerHost() throws JGDIException;
    
    /**
     * <p>Enable queue(s).</p>
     * <p>The CLI equivialent for this method is <code>qmod -e<code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void enableQueues(String[] queues, boolean force) throws JGDIException;
    
    /**
     * <p>Enable queue(s).</p>
     * <p>The CLI equivialent for this method is <code>qmod -e<code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void enableQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Disable queue(s),  i.e. no further jobs are dispatched to disabled
     *  queues while jobs already executing in these queues are allowed to finish.</p>
     * <p>The CLI equivialent for this method is <code>qmod -d<code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void disableQueues(String[] queues, boolean force) throws JGDIException;
    
    /**
     * <p>Disable queue(s),  i.e. no further jobs are dispatched to disabled
     *  queues while jobs already executing in these queues are allowed to finish.</p>
     * <p>The CLI equivialent for this method is <code>qmod -d<code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void disableQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Suspend the  queues  and  any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is qmod -sq (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void suspend(String[] queues, boolean force) throws JGDIException;
    
    /**
     * <p>Suspend the  queues  and  any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is qmod -sq (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void suspendWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Suspend the  queues  and  any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is qmod -sq (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void suspendQueues(String[] queues, boolean force) throws JGDIException;
    
    /**
     * <p>Suspend the  queues  and  any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is qmod -sq (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void suspendQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Suspends job(s). If a job is  both suspended explicitly and via
     * suspension of its queue, a following unsuspend  of  the  queue  will  not
     * release the suspension state on the job.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -sj</code> (see man qmod(1)).</p>
     *
     * @param jobs   a wildcard job list
     * @param force  force the action
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void suspendJobs(String[] jobs, boolean force) throws JGDIException;
    
    /**
     * <p>Suspends job(s). If a job is  both suspended explicitly and via
     * suspension of its queue, a following unsuspend  of  the  queue  will  not
     * release the suspension state on the job.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -sj</code> (see man qmod(1)).</p>
     *
     * @param jobs   a wildcard job list
     * @param force  force the action
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void suspendJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Unsuspends the queues and any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -usq</code>
     *    (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void unsuspend(String[] queues, boolean force) throws JGDIException;
    
    /**
     * <p>Unsuspends the queues and any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -usq</code>
     *    (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void unsuspendWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Unsuspends the queues and any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -usq</code>
     *    (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void unsuspendQueues(String[] queues, boolean force) throws JGDIException;
    
    /**
     * <p>Unsuspends the queues and any jobs which might be active.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -usq</code>
     *    (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   force the action
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void unsuspendQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
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
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void unsuspendJobs(String[] jobs, boolean force) throws JGDIException;
    
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
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void unsuspendJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Clears the error state of the specified queues(s).</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -cq<code>
     *    (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   <p>Force the modification action for the queue
     *                   despite the apparent  current  state of the queue.</p>
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void clearQueues(String[] queues, boolean force) throws JGDIException;
    
    /**
     * <p>Clears the error state of the specified queues(s).</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -cq<code>
     *    (see man qmod(1)).</p>
     *
     * @param queues  a wildcard queue list
     * @param force   <p>Force the modification action for the queue
     *                   despite the apparent  current  state of the queue.</p>
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void clearQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Clears the error state of the specified jobs(s).</p>
     *
     * <p>The CLI equivialent for this method is qmod -cj (see man qmod(1)).</p>
     *
     * @param jobs   a wildcard job list
     * @param force  Force the modification action for the job(s)
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void clearJobs(String[] jobs, boolean force) throws JGDIException;
    
    /**
     * <p>Clears the error state of the specified jobs(s).</p>
     *
     * <p>The CLI equivialent for this method is qmod -cj (see man qmod(1)).</p>
     *
     * @param jobs   a wildcard job list
     * @param force  Force the modification action for the job(s)
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void clearJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     *  <p>Reschedules  all  jobs  currently running  in  the  queue(s). Requires
     *  root  or  manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -r</code> (see man qmod(1)).</p>
     * @param queue_or_job  a wildcard queue or job list
     * @param force   Force the modification action for the queue
     *                despite the apparent  current  state of the queue.
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void reschedule(String[] queue_or_job, boolean force) throws JGDIException;
    
    /**
     *  <p>Reschedules  all  jobs  currently running  in  the  queue(s). Requires
     *  root  or  manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -r</code> (see man qmod(1)).</p>
     * @param queue_or_job  a wildcard queue or job list
     * @param force   Force the modification action for the queue
     *                despite the apparent  current  state of the queue.
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void rescheduleWithAnswer(String[] queue_or_job, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     *  <p>Reschedules  all  jobs  currently running  in  the  queue(s). Requires
     *  root  or  manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -r</code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   Force the modification action for the queue
     *                despite the apparent  current  state of the queue.
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void rescheduleQueues(String[] queues, boolean force) throws JGDIException;
    
    /**
     *  <p>Reschedules  all  jobs  currently running  in  the  queue(s). Requires
     *  root  or  manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -r</code> (see man qmod(1)).</p>
     * @param queues  a wildcard queue list
     * @param force   Force the modification action for the queue
     *                despite the apparent  current  state of the queue.
     * @param answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void rescheduleQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>If applied  to  running  jobs,  reschedules  the  jobs.
     * Requires root or manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -r</code> (see man qmod(1)).</p>
     *
     * @param jobs   a wildcard job range list
     * @param force  Force the modification action for the job(s)
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void rescheduleJobs(String[] jobs, boolean force) throws JGDIException;
    
    /**
     * <p>If applied  to  running  jobs,  reschedules  the  jobs.
     * Requires root or manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qmod -r</code> (see man qmod(1)).</p>
     *
     * @param  jobs      a wildcard job range list
     * @param  force     force the modification action for the job(s)
     * @param  answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void rescheduleJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException;
    
    /**
     * <p>Show the detached settings of a cluster queue
     * Requires root or manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -sds</code> (see man qconf(1)).</p>
     *
     * @param  queues   a wildcard cluster queue list
     * @return the detached settings summary information as a <code>java.lang.String</code>
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public String showDetachedSettings(String[] queues) throws JGDIException;
    
    /**
     * <p>Show the detached settings of all cluster queues
     * Requires root or manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -sds</code> (see man qconf(1)).</p>
     *
     * @return the detached settings summary information as a <code>java.lang.String</code>
     * @throws com.sun.grid.jgdi.JGDIException
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public String showDetachedSettingsAll() throws JGDIException;

    /**
     * <p>Delete the sharetree
     * Requires root or manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -dstree</code> (see man qconf(1)).</p>
     *
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void deleteShareTree() throws JGDIException;
    
    /**
     * <p>Delete the sharetree
     * Requires root or manager privileges.</p>
     *
     * <p>The CLI equivialent for this method is <code>qconf -dstree</code> (see man qconf(1)).</p>
     *
     * @param  answers   the <code>answer list</code> object
     * @throws com.sun.grid.jgdi.JGDIException JGDIException on any error on the GDI level
     */
    public void deleteShareTreeWithAnswer(List<JGDIAnswer> answers) throws JGDIException;
            
}