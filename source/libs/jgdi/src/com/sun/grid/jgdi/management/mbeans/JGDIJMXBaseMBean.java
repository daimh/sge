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
package com.sun.grid.jgdi.management.mbeans;

import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.configuration.ExecHost;
import com.sun.grid.jgdi.configuration.JGDIAnswer;
import com.sun.grid.jgdi.event.EventTypeEnum;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummary;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummaryOptions;
import com.sun.grid.jgdi.monitoring.QHostOptions;
import com.sun.grid.jgdi.monitoring.QHostResult;
import com.sun.grid.jgdi.monitoring.QQuotaOptions;
import com.sun.grid.jgdi.monitoring.QQuotaResult;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryResult;
import java.io.File;
import java.util.List;
import java.util.Set;

// drmaa specific imports
// import org.ggf.drmaa.DrmaaException;
// import org.ggf.drmaa.JobInfo;
// import org.ggf.drmaa.JobTemplate;
// import org.ggf.drmaa.Version;
/**
 * Interface JGDIJMXBaseMBean
 *
 *
 */
public interface JGDIJMXBaseMBean {

    public String getCurrentJGDIVersion() throws JGDIException;

    /**
     * Close the jgdi session
     */
    public void close();

    /**
     *   Subscribe a set of event types if they are not already subscribed.
     *
     *   @param eventTypeSet  set of event types
     */
    public void subscribe(Set<EventTypeEnum> eventTypeSet) throws JGDIException;

    /**
     *   Unsubcribe a set of event types if the are not already unsubscribed.
     *
     *   @param eventTypeSet  set of event type which should be unsubcribed
     */
    public void unsubscribe(Set<EventTypeEnum> eventTypeSet) throws JGDIException;

    /**
     *  Get the current event subscription.
     *
     *  @return set of event types which are currently subscribed
     */
    public Set<EventTypeEnum> getSubscription() throws JGDIException;

    /**
     *   Set the current event subscription.
     *
     *   @param eventTypeSet  the set of event types to subscribe
     */
    public void setSubscription(Set<EventTypeEnum> eventTypeSet) throws JGDIException;

    // ========= JGDIBase methods ================================
    public String getAdminUser() throws JGDIException;

    public File getSGERoot() throws JGDIException;

    public String getSGECell() throws JGDIException;

    public String getActQMaster() throws JGDIException;

    public int getSgeQmasterPort() throws JGDIException;

    public int getSgeExecdPort() throws JGDIException;

    public List<ExecHost> getRealExecHostList() throws JGDIException;

    // -------- Monitoring interface --------------------------------------------
    public QHostOptions newQHostOptions() throws JGDIException;

    public ClusterQueueSummaryOptions newClusterQueueSummaryOptions() throws JGDIException;

    public QueueInstanceSummaryOptions newQueueInstanceSummaryOptions() throws JGDIException;

    public QQuotaOptions newQQuotaOptions() throws JGDIException;

    public QHostResult execQHost(QHostOptions options) throws JGDIException;

    public List<ClusterQueueSummary> getClusterQueueSummary(ClusterQueueSummaryOptions options) throws JGDIException;

    public QueueInstanceSummaryResult getQueueInstanceSummary(QueueInstanceSummaryOptions options) throws JGDIException;

    public QQuotaResult getQQuota(QQuotaOptions options) throws JGDIException;

    // -------- Managing interface methods --------------------------------------
    public void clearShareTreeUsage() throws JGDIException;

    public void clearShareTreeUsageWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    public void cleanQueues(String[] queues) throws JGDIException;

    public void cleanQueuesWithAnswer(String[] queues, List<JGDIAnswer> answers) throws JGDIException;

    public void killMaster() throws JGDIException;

    public void killMasterWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    public void killScheduler() throws JGDIException;

    public void killSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    public void startScheduler() throws JGDIException;

    public void startSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    public void killExecd(String[] hosts, boolean terminateJobs) throws JGDIException;

    public void killExecdWithAnswer(String[] hosts, boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException;

    public void killAllExecds(boolean terminateJobs) throws JGDIException;

    public void killAllExecdsWithAnswer(boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException;

    public void killEventClients(int[] ids) throws JGDIException;

    public void killEventClientsWithAnswer(int[] ids, List<JGDIAnswer> answers) throws JGDIException;

    public void killAllEventClients() throws JGDIException;

    public void killAllEventClientsWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    public void triggerSchedulerMonitoring() throws JGDIException;

    public void triggerSchedulerMonitoringWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    public String getSchedulerHost() throws JGDIException;

    public void enableQueues(String[] queues, boolean force) throws JGDIException;

    public void enableQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void disableQueues(String[] queues, boolean force) throws JGDIException;

    public void disableQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void suspend(String[] queues, boolean force) throws JGDIException;

    public void suspendWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void suspendQueues(String[] queues, boolean force) throws JGDIException;

    public void suspendQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void suspendJobs(String[] jobs, boolean force) throws JGDIException;

    public void suspendJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void unsuspend(String[] queues, boolean force) throws JGDIException;

    public void unsuspendWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void unsuspendQueues(String[] queues, boolean force) throws JGDIException;

    public void unsuspendQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void unsuspendJobs(String[] jobs, boolean force) throws JGDIException;

    public void unsuspendJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void clearQueues(String[] queues, boolean force) throws JGDIException;

    public void clearQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void clearJobs(String[] jobs, boolean force) throws JGDIException;

    public void clearJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void reschedule(String[] queue_or_job, boolean force) throws JGDIException;

    public void rescheduleWithAnswer(String[] queue_or_job, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void rescheduleQueues(String[] queues, boolean force) throws JGDIException;

    public void rescheduleQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public void rescheduleJobs(String[] jobs, boolean force) throws JGDIException;

    public void rescheduleJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException;

    public String showDetachedSettings(String[] queues) throws JGDIException;

    public String showDetachedSettingsAll() throws JGDIException;

    public void deleteShareTree() throws JGDIException;

    public void deleteShareTreeWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

//     // DRMAA specific methods
//     public void drmaaInit(String contact) throws DrmaaException;
//     public void drmaaExit() throws DrmaaException;
//     // public JobTemplate drmaaCreateJobTemplate() throws DrmaaException;
//     // public void deleteJobTemplate(JobTemplate jt) throws DrmaaException;
//     public String drmaaRunJob(JobTemplate jt) throws DrmaaException;
//     public List drmaaRunBulkJobs(JobTemplate jt, int start, int end, int incr) throws DrmaaException;
//     public void drmaaControl(String jobId, int action) throws DrmaaException;
//     public void drmaaSynchronize(List jobIds, long timeout, boolean dispose) throws DrmaaException;
//     public JobInfo drmaaWait(String jobId, long timeout) throws DrmaaException;
//     public int drmaaGetJobProgramStatus(String jobId) throws DrmaaException;
//     public String drmaaGetContact();
//     public Version drmaaGetVersion();
//     public String drmaaGetDrmSystem();
//     public String drmaaGetDrmaaImplementation();
}

