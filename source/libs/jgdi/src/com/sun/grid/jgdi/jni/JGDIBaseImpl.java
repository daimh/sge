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
package com.sun.grid.jgdi.jni;

import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.configuration.JGDIAnswer;
import com.sun.grid.jgdi.monitoring.*;
import java.io.File;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;

/**
 *
 */
public abstract class JGDIBaseImpl implements com.sun.grid.jgdi.JGDIBase {

    private final static Logger log = Logger.getLogger(JGDIBaseImpl.class.getName());
    private int ctxIndex = -1;
    private File sgeRoot;
    private final static List<JGDIBaseImpl> instances = new LinkedList<JGDIBaseImpl>();
    private static boolean shutdown = false;

    public void init(String url) throws JGDIException {
        synchronized (instances) {
            if (shutdown) {
                throw new JGDIException("shutdown in progress");
            }
            instances.add(this);
        }
        ctxIndex = nativeInit(url);
        if (log.isLoggable(Level.FINE)) {
            log.log(Level.FINE, "native connection with id {1} established (url={0})", new Object[]{url, ctxIndex});
        }
    }

    int getCtxIndex() {
        return ctxIndex;
    }

    public static void resetShutdown() {
        synchronized (instances) {
            shutdown = false;
        }
    }

    public static void closeAllConnections() {
        List<JGDIBaseImpl> currentInstances = null;
        synchronized (instances) {
            shutdown = true;
            currentInstances = new ArrayList<JGDIBaseImpl>(instances);
        }
        for (JGDIBaseImpl jgdi : currentInstances) {
            try {
                jgdi.close();
            } catch (JGDIException ex) {
                LogRecord lr = new LogRecord(Level.FINE, "close of connection {0} failed");
                lr.setParameters(new Object[]{jgdi.ctxIndex});
                lr.setThrown(ex);
                log.log(lr);
            }
        }
    }

    public void close() throws com.sun.grid.jgdi.JGDIException {
        int tmpCtxIndex = -1;
        synchronized (this) {
            tmpCtxIndex = ctxIndex;
            ctxIndex = -1;
        }
        if (tmpCtxIndex >= 0) {
            log.log(Level.FINE, "Closing connection with id {0}", tmpCtxIndex);
            synchronized (instances) {
                instances.remove(this);
            }
            nativeClose(tmpCtxIndex);
            if (log.isLoggable(Level.FINE)) {
                log.log(Level.FINE, "Connection with id {0} closed", tmpCtxIndex);
            }
        }
    }

    public File getSGERoot() throws JGDIException {
        if (sgeRoot == null) {
            sgeRoot = new File(nativeGetSGERoot());
        }
        return sgeRoot;
    }

    public String getSGECell() throws JGDIException {
        return nativeGetSGECell();
    }

    public String getAdminUser() throws JGDIException {
        return nativeGetAdminUser();
    }

    public String getActQMaster() throws JGDIException {
        return nativeGetActQMaster();
    }

    public int getSgeQmasterPort() throws JGDIException {
        return nativeGetSgeQmasterPort();
    }

    public int getSgeExecdPort() throws JGDIException {
        return nativeGetSgeExecdPort();
    }

    public String getSchedulerHost() throws JGDIException {
        return nativeGetSchedulerHost();
    }

    public QHostResult execQHost(QHostOptions options) throws JGDIException {
        QHostResultImpl ret = new QHostResultImpl();
        nativeExecQHost(options, ret);
        return ret;
    }

    public List<ClusterQueueSummary> getClusterQueueSummary(ClusterQueueSummaryOptions options) throws JGDIException {
        List<ClusterQueueSummary> ret = new LinkedList<ClusterQueueSummary>();
        nativeFillClusterQueueSummary(options, ret);
        return ret;
    }

    public QueueInstanceSummaryResult getQueueInstanceSummary(QueueInstanceSummaryOptions options) throws JGDIException {
        QueueInstanceSummaryResultImpl ret = new QueueInstanceSummaryResultImpl();
        nativeFillQueueInstanceSummary(options, ret);
        return ret;
    }

    public QQuotaResult getQQuota(QQuotaOptions options) throws JGDIException {
        QQuotaResultImpl ret = new QQuotaResultImpl();
        nativeGetQQuota(options, ret);
        return ret;
    }

    public void cleanQueuesWithAnswer(String[] queues, List<JGDIAnswer> answers) throws JGDIException {
        nativeCleanQueuesWithAnswer(queues, answers);
    }

    public void cleanQueues(String[] queues) throws JGDIException {
        cleanQueuesWithAnswer(queues, null);
    }

    public void unsuspendWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeUnsuspendWithAnswer(queues, force, answers);
    }

    public void unsuspend(String[] queues, boolean force) throws JGDIException {
        unsuspendWithAnswer(queues, force, null);
    }

    public void unsuspendQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeUnsuspendQueuesWithAnswer(queues, force, answers);
    }

    public void unsuspendQueues(String[] queues, boolean force) throws JGDIException {
        unsuspendQueuesWithAnswer(queues, force, null);
    }

    public void unsuspendJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeUnsuspendJobsWithAnswer(jobs, force, answers);
    }

    public void unsuspendJobs(String[] jobs, boolean force) throws JGDIException {
        unsuspendJobsWithAnswer(jobs, force, null);
    }

    public void suspendWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeSuspendWithAnswer(queues, force, answers);
    }

    public void suspend(String[] queues, boolean force) throws JGDIException {
        suspendWithAnswer(queues, force, null);
    }

    public void suspendQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeSuspendQueuesWithAnswer(queues, force, answers);
    }

    public void suspendQueues(String[] queues, boolean force) throws JGDIException {
        suspendQueuesWithAnswer(queues, force, null);
    }

    public void suspendJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeSuspendJobsWithAnswer(jobs, force, answers);
    }

    public void suspendJobs(String[] jobs, boolean force) throws JGDIException {
        suspendJobsWithAnswer(jobs, force, null);
    }

    public void rescheduleJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeRescheduleJobsWithAnswer(jobs, force, answers);
    }

    public void rescheduleJobs(String[] jobs, boolean force) throws JGDIException {
        rescheduleJobsWithAnswer(jobs, force, null);
    }

    public void rescheduleQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeRescheduleQueuesWithAnswer(queues, force, answers);
    }

    public void rescheduleQueues(String[] queues, boolean force) throws JGDIException {
        rescheduleQueuesWithAnswer(queues, force, null);
    }

    public void rescheduleWithAnswer(String[] queue_or_job, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeRescheduleWithAnswer(queue_or_job, force, answers);
    }

    public void reschedule(String[] queue_or_job, boolean force) throws JGDIException {
        rescheduleWithAnswer(queue_or_job, force, null);
    }

    public void clearJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeClearJobsWithAnswer(jobs, force, answers);
    }

    public void clearJobs(String[] jobs, boolean force) throws JGDIException {
        clearJobsWithAnswer(jobs, force, null);
    }

    public void clearQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeClearQueuesWithAnswer(queues, force, answers);
    }

    public void clearQueues(String[] queues, boolean force) throws JGDIException {
        clearQueuesWithAnswer(queues, force, null);
    }

    public void disableQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeDisableQueuesWithAnswer(queues, force, answers);
    }

    public void disableQueues(String[] queues, boolean force) throws JGDIException {
        disableQueuesWithAnswer(queues, force, null);
    }

    public void enableQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        nativeEnableQueuesWithAnswer(queues, force, answers);
    }

    public void enableQueues(String[] queues, boolean force) throws JGDIException {
        enableQueuesWithAnswer(queues, force, null);
    }

    public void killAllExecdsWithAnswer(boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException {
        nativeKillAllExecdsWithAnswer(terminateJobs, answers);
    }

    public void killAllExecds(boolean terminateJobs) throws JGDIException {
        killAllExecdsWithAnswer(terminateJobs, null);
    }

    public void killExecdWithAnswer(String[] hosts, boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException {
        nativeKillExecdWithAnswer(hosts, terminateJobs, answers);
    }

    public void killExecd(String[] hosts, boolean terminateJobs) throws JGDIException {
        killExecdWithAnswer(hosts, terminateJobs, null);
    }

    public void killEventClientsWithAnswer(int[] ids, List<JGDIAnswer> answers) throws JGDIException {
        nativeKillEventClientsWithAnswer(ids, answers);
    }

    public void killEventClients(int[] ids) throws JGDIException {
        killEventClientsWithAnswer(ids, null);
    }

    public void triggerSchedulerMonitoringWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        nativeTriggerSchedulerMonitoringWithAnswer(answers);
    }

    public void triggerSchedulerMonitoring() throws JGDIException {
        triggerSchedulerMonitoringWithAnswer(null);
    }

    public void clearShareTreeUsageWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        nativeClearShareTreeUsageWithAnswer(answers);
    }

    public void clearShareTreeUsage() throws JGDIException {
        clearShareTreeUsageWithAnswer(null);
    }

    public void killAllEventClientsWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        nativeKillAllEventClientsWithAnswer(answers);
    }

    public void killAllEventClients() throws JGDIException {
        killAllEventClientsWithAnswer(null);
    }

    public void killMasterWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        nativeKillMasterWithAnswer(answers);
    }

    public void killMaster() throws JGDIException {
        killMasterWithAnswer(null);
    }

    public void killSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        nativeKillSchedulerWithAnswer(answers);
    }

    public void killScheduler() throws JGDIException {
        killSchedulerWithAnswer(null);
    }

    public void startSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        nativeStartSchedulerWithAnswer(answers);
    }

    public void startScheduler() throws JGDIException {
        startSchedulerWithAnswer(null);
    }

    public String showDetachedSettingsWithAnswer(String[] queues, List<JGDIAnswer> answers) throws JGDIException {
        return nativeShowDetachedSettingsWithAnswer(queues, answers);
    }

    public String showDetachedSettings(String[] queues) throws JGDIException {
        return showDetachedSettingsWithAnswer(queues, null);
    }

    public String showDetachedSettingsAllWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        return nativeShowDetachedSettingsAllWithAnswer(answers);
    }

    public String showDetachedSettingsAll() throws JGDIException {
        return showDetachedSettingsAllWithAnswer(null);
    }

    public void deleteShareTree() throws JGDIException {
        deleteShareTreeWithAnswer(null);
    }

    public void deleteShareTreeWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        nativeDeleteShareTreeWithAnswer(answers);
    }

    private native int nativeInit(String url) throws JGDIException;

    private native void nativeClose(int ctxIndex) throws JGDIException;

    private native String nativeGetSGERoot() throws JGDIException;

    private native String nativeGetSGECell() throws JGDIException;

    private native String nativeGetAdminUser() throws JGDIException;

    private native String nativeGetActQMaster() throws JGDIException;

    private native int nativeGetSgeQmasterPort() throws JGDIException;

    private native int nativeGetSgeExecdPort() throws JGDIException;

    private native void nativeExecQHost(QHostOptions options, QHostResultImpl result);

    private native void nativeFillClusterQueueSummary(ClusterQueueSummaryOptions options, List<ClusterQueueSummary> list) throws JGDIException;

    private native void nativeFillQueueInstanceSummary(QueueInstanceSummaryOptions options, QueueInstanceSummaryResultImpl result) throws JGDIException;

    private native String nativeGetSchedulerHost() throws JGDIException;

    private native void nativeGetQQuota(QQuotaOptions options, QQuotaResultImpl ret) throws JGDIException;

    private native void nativeCleanQueuesWithAnswer(String[] queues, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeUnsuspendWithAnswer(String[] queues, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeKillSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeStartSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeUnsuspendQueuesWithAnswer(String[] queues, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeUnsuspendJobsWithAnswer(String[] jobs, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeSuspendWithAnswer(String[] queues, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeSuspendQueuesWithAnswer(String[] queues, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeSuspendJobsWithAnswer(String[] jobs, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeRescheduleJobsWithAnswer(String[] jobs, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeRescheduleQueuesWithAnswer(String[] queues, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeRescheduleWithAnswer(String[] queue_or_job, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeClearJobsWithAnswer(String[] jobs, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeClearQueuesWithAnswer(String[] queues, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeDisableQueuesWithAnswer(String[] queues, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeEnableQueuesWithAnswer(String[] queues, boolean force, java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeKillAllExecdsWithAnswer(boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeKillExecdWithAnswer(String[] hosts, boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeKillEventClientsWithAnswer(int[] ids, List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeTriggerSchedulerMonitoringWithAnswer(java.util.List<JGDIAnswer> answer) throws JGDIException;

    private native void nativeClearShareTreeUsageWithAnswer(java.util.List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeKillAllEventClientsWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeKillMasterWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    private native String nativeShowDetachedSettingsWithAnswer(String[] queues, List<JGDIAnswer> answers) throws JGDIException;

    private native String nativeShowDetachedSettingsAllWithAnswer(List<JGDIAnswer> answers) throws JGDIException;

    private native void nativeDeleteShareTreeWithAnswer(List<JGDIAnswer> answers) throws JGDIException;
}
