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

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.configuration.ExecHost;
import com.sun.grid.jgdi.configuration.JGDIAnswer;
import com.sun.grid.jgdi.event.EventTypeEnum;
import com.sun.grid.jgdi.management.JGDISession;
import com.sun.grid.jgdi.management.NotificationBridge;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummary;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummaryOptions;
import com.sun.grid.jgdi.monitoring.QHostOptions;
import com.sun.grid.jgdi.monitoring.QHostResult;
import com.sun.grid.jgdi.monitoring.QQuotaOptions;
import com.sun.grid.jgdi.monitoring.QQuotaResult;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryResult;
import java.io.File;
import java.lang.reflect.UndeclaredThrowableException;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.management.*;
//import org.ggf.drmaa.DrmaaException;
//import org.ggf.drmaa.JobTemplate;
//import org.ggf.drmaa.Version;
/**
 * Class JGDIJMXBase
 * JGDIJMXBase MBean
 */
public class JGDIJMXBase implements java.io.Serializable, JGDIJMXBaseMBean, NotificationEmitter {

    private final static long serialVersionUID = 2007121101L;
    protected final Logger log = Logger.getLogger("com.sun.grid.jgdi.management.mbeans");
    private final JGDISession owner;

    /**
     * Create a new instance of JGDIJMXBase
     * @param owner the session of the mbean
     */
    public JGDIJMXBase(JGDISession owner) {
        this.owner = owner;
    }

    protected JGDI getJGDI() throws JGDIException {
        log.entering("JGDIJMXBase", "getJGDI");
        JGDISession session = JGDISession.getCurrentSession();
        if (session.getId() != owner.getId()) {
            throw new SecurityException("Session " + session.getId() + " has no permission to mbean of session " + owner.getId());
        }
        JGDI ret = owner.getJGDI();
        log.exiting("JGDIJMXBase", "getJGDI", ret);
        return ret;
    }

    /**
     * Close the session of this jgdi mbean
     */
    public void close() {
        owner.close();
    }

    protected NotificationBridge getNB() throws JGDIException {
        log.entering("JGDIJMXBase", "getNB");
        NotificationBridge ret = owner.getNotificationBridge();
        log.exiting("JGDIJMXBase", "getNB", ret);
        return ret;
    }

    public String getCurrentJGDIVersion() throws JGDIException {
        log.entering("JGDIJMXBase", "getCurrentJGDIVersion");
        String ret = JGDIFactory.getJGDIVersion();
        log.exiting("JGDIJMXBase", "getCurrentJGDIVersion", ret);
        return ret;
    }

    public void subscribe(Set<EventTypeEnum> subscription) throws JGDIException {
        log.entering("JGDIJMXBase", "subscribe", subscription);
        getNB().subscribe(subscription);
        log.exiting("JGDIJMXBase", "subscribe");
    }

    public void unsubscribe(Set<EventTypeEnum> subscription) throws JGDIException {
        log.entering("JGDIJMXBase", "unsubscribe", subscription);
        getNB().unsubscribe(subscription);
        log.exiting("JGDIJMXBase", "unsubscribe");
    }

    public Set<EventTypeEnum> getSubscription() throws JGDIException {
        log.entering("JGDIJMXBase", "getSubscription");
        Set<EventTypeEnum> ret = null;
        ret = getNB().getSubscription();
        log.exiting("JGDIJMXBase", "getSubscription", ret);
        return ret;
    }

    public void setSubscription(Set<EventTypeEnum> subscription) throws JGDIException {
        log.entering("JGDIJMXBase", "setSubscription", subscription);
        getNB().setSubscription(subscription);
        log.exiting("JGDIJMXBase", "setSubscription");
    }

    public void removeNotificationListener(NotificationListener listener, NotificationFilter filter, Object handback) throws ListenerNotFoundException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "removeNotificationListener", new Object[]{listener, filter, handback});
        }
        try {
            getNB().removeNotificationListener(listener);
        } catch (JGDIException ex) {
            throw new UndeclaredThrowableException(ex);
        }
        log.exiting("JGDIJMXBase", "removeNotificationListener");
    }

    public void addNotificationListener(NotificationListener listener, NotificationFilter filter, Object handback) throws IllegalArgumentException {
        try {
            if (log.isLoggable(Level.FINER)) {
                log.entering("JGDIJMXBase", "addNotificationListener", new Object[]{listener, filter, handback});
            }
            getNB().addNotificationListener(listener, filter, handback);
            log.exiting("JGDIJMXBase", "addNotificationListener");
        } catch (JGDIException ex) {
            IllegalArgumentException ex1 = new IllegalArgumentException("Can not add notification listener", ex);
            log.throwing("JGDIJMXBase", "addNotificationListener", ex1);
            throw ex1;
        }
    }

    public void removeNotificationListener(NotificationListener listener) throws ListenerNotFoundException {
        try {
            log.entering("JGDIJMXBase", "removeNotificationListener", listener);
            getNB().removeNotificationListener(listener);
            log.exiting("JGDIJMXBase", "removeNotificationListener");
        } catch (JGDIException ex) {
            ListenerNotFoundException ex1 = new ListenerNotFoundException("notification bride reports error");
            ex1.initCause(ex);
            log.throwing("JGDIJMXBase", "removeNotificationListener", ex1);
            throw ex1;
        }
    }

    public MBeanNotificationInfo[] getNotificationInfo() {
        log.entering("JGDIJMXBase", "getNotificationInfo");
        MBeanNotificationInfo[] ret = null;
        try {
            ret = getNB().getMBeanNotificationInfo();
        } catch (JGDIException ex) {
            log.throwing("JGDIJMXBase", "getNotificationInfo", ex);
            throw new UndeclaredThrowableException(ex);
        }
        log.exiting("JGDIJMXBase", "removeNotificationListener", ret);
        return ret;
    }
    // JGDI Base methods
    public String getAdminUser() throws JGDIException {
        log.entering("JGDIJMXBase", "getAdminUser");
        String ret = getJGDI().getAdminUser();
        log.exiting("JGDIJMXBase", "getAdminUser", ret);
        return ret;
    }

    public File getSGERoot() throws JGDIException {
        log.entering("JGDIJMXBase", "getSGERoot");
        File ret = getJGDI().getSGERoot();
        log.exiting("JGDIJMXBase", "getSGERoot", ret);
        return ret;
    }

    public String getSGECell() throws JGDIException {
        log.entering("JGDIJMXBase", "getSGECell");
        String ret = getJGDI().getSGECell();
        log.exiting("JGDIJMXBase", "getSGECell", ret);
        return ret;
    }

    public String getActQMaster() throws JGDIException {
        log.entering("JGDIJMXBase", "getActQMaster");
        String ret = getJGDI().getActQMaster();
        log.exiting("JGDIJMXBase", "getActQMaster", ret);
        return ret;
    }

    public int getSgeQmasterPort() throws JGDIException {
        log.entering("JGDIJMXBase", "getSgeQmasterPort");
        int ret = getJGDI().getSgeQmasterPort();
        log.exiting("JGDIJMXBase", "getSgeQmasterPort", ret);
        return ret;
    }

    public int getSgeExecdPort() throws JGDIException {
        log.entering("JGDIJMXBase", "getSgeExecdPort");
        int ret = getJGDI().getSgeExecdPort();
        log.exiting("JGDIJMXBase", "getSgeExecdPort", ret);
        return ret;
    }

    public List<ExecHost> getRealExecHostList() throws JGDIException {
        log.entering("JGDIJMXBase", "getRealExecHostList");
        List<ExecHost> ret = getJGDI().getRealExecHostList();
        log.exiting("JGDIJMXBase", "getRealExecHostList", ret);
        return ret;
    }

    public QHostResult execQHost(QHostOptions options) throws JGDIException {
        log.entering("JGDIJMXBase", "execQHost", options);
        QHostResult ret = getJGDI().execQHost(options);
        log.exiting("JGDIJMXBase", "execQHost", ret);
        return ret;
    }

    public List<ClusterQueueSummary> getClusterQueueSummary(ClusterQueueSummaryOptions options) throws JGDIException {
        log.entering("JGDIJMXBase", "getClusterQueueSummary", options);
        List<ClusterQueueSummary> ret = getJGDI().getClusterQueueSummary(options);
        log.exiting("JGDIJMXBase", "getClusterQueueSummary", ret);
        return ret;
    }

    public QueueInstanceSummaryResult getQueueInstanceSummary(QueueInstanceSummaryOptions options) throws JGDIException {
        log.entering("JGDIJMXBase", "getQueueInstanceSummary", options);
        QueueInstanceSummaryResult ret = getJGDI().getQueueInstanceSummary(options);
        log.exiting("JGDIJMXBase", "getQueueInstanceSummary", ret);
        return ret;
    }

    public QQuotaResult getQQuota(QQuotaOptions options) throws JGDIException {
        log.entering("JGDIJMXBase", "getQQuota", options);
        QQuotaResult ret = getJGDI().getQQuota(options);
        log.exiting("JGDIJMXBase", "getQueueInstanceSummary", ret);
        return ret;
    }

    public void clearShareTreeUsage() throws JGDIException {
        log.entering("JGDIJMXBase", "clearShareTreeUsage");
        getJGDI().clearShareTreeUsage();
        log.exiting("JGDIJMXBase", "clearShareTreeUsage");
    }

    public void clearShareTreeUsageWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        log.entering("JGDIJMXBase", "clearShareTreeUsageWithAnswer");
        getJGDI().clearShareTreeUsageWithAnswer(answers);
        log.exiting("JGDIJMXBase", "clearShareTreeUsageWithAnswer");
    }

    public void cleanQueues(String[] queues) throws JGDIException {
        log.entering("JGDIJMXBase", "cleanQueues", queues);
        getJGDI().cleanQueues(queues);
        log.exiting("JGDIJMXBase", "cleanQueues");
    }

    public void cleanQueuesWithAnswer(String[] queues, List<JGDIAnswer> answers) throws JGDIException {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public void killMaster() throws JGDIException {
        log.entering("JGDIJMXBase", "killMaster");
        getJGDI().killMaster();
        log.exiting("JGDIJMXBase", "killMaster");
    }

    public void killMasterWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public void killScheduler() throws JGDIException {
        log.entering("JGDIJMXBase", "killScheduler");
        getJGDI().killScheduler();
        log.exiting("JGDIJMXBase", "killScheduler");
    }

    public void killSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        log.entering("JGDIJMXBase", "killSchedulerWithAnswer");
        getJGDI().killSchedulerWithAnswer(answers);
        log.exiting("JGDIJMXBase", "killSchedulerWithAnswer");
    }

    public void startScheduler() throws JGDIException {
        log.entering("JGDIJMXBase", "startScheduler");
        getJGDI().startScheduler();
        log.exiting("JGDIJMXBase", "startScheduler");
    }

    public void startSchedulerWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        log.entering("JGDIJMXBase", "startSchedulerWithAnswer");
        getJGDI().startSchedulerWithAnswer(answers);
        log.exiting("JGDIJMXBase", "startSchedulerWithAnswer");
    }

    public void killExecd(String[] hosts, boolean terminateJobs) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "killExecd", new Object[]{hosts, terminateJobs});
        }
        getJGDI().killExecd(hosts, terminateJobs);
        log.exiting("JGDIJMXBase", "killExecd");
    }

    public void killExecdWithAnswer(String[] hosts, boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "killExecdWithAnswer", new Object[]{hosts, terminateJobs});
        }
        getJGDI().killExecdWithAnswer(hosts, terminateJobs, answers);
        log.exiting("JGDIJMXBase", "killExecdWithAnswer");
    }

    public void killAllExecds(boolean terminateJobs) throws JGDIException {
        log.entering("JGDIJMXBase", "killAllExecds", terminateJobs);
        getJGDI().killAllExecds(terminateJobs);
        log.exiting("JGDIJMXBase", "killAllExecds");
    }

    public void killAllExecdsWithAnswer(boolean terminateJobs, List<JGDIAnswer> answers) throws JGDIException {
        log.entering("JGDIJMXBase", "killAllExecdsWithAnswer", terminateJobs);
        getJGDI().killAllExecdsWithAnswer(terminateJobs, answers);
        log.exiting("JGDIJMXBase", "killAllExecdsWithAnswer");
    }

    public void killEventClients(int[] ids) throws JGDIException {
        log.entering("JGDIJMXBase", "killEventClients", ids);
        getJGDI().killEventClients(ids);
        log.exiting("JGDIJMXBase", "killEventClients");
    }

    public void killEventClientsWithAnswer(int[] ids, List<JGDIAnswer> answers) throws JGDIException {
        log.entering("JGDIJMXBase", "killEventClientsWithAnswer", ids);
        getJGDI().killEventClientsWithAnswer(ids, answers);
        log.exiting("JGDIJMXBase", "killEventClientsWithAnswer");
    }

    public void killAllEventClients() throws JGDIException {
        log.entering("JGDIJMXBase", "killAllEventClients");
        getJGDI().killAllEventClients();
        log.exiting("JGDIJMXBase", "killAllEventClients");
    }

    public void killAllEventClientsWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        log.entering("JGDIJMXBase", "killAllEventClientsWithAnswer");
        getJGDI().killAllEventClientsWithAnswer(answers);
        log.exiting("JGDIJMXBase", "killAllEventClientsWithAnswer");
    }

    public void triggerSchedulerMonitoring() throws JGDIException {
        log.entering("JGDIJMXBase", "triggerSchedulerMonitoring");
        getJGDI().triggerSchedulerMonitoring();
        log.exiting("JGDIJMXBase", "triggerSchedulerMonitoring");
    }

    public void triggerSchedulerMonitoringWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        log.entering("JGDIJMXBase", "triggerSchedulerMonitoringWithAnswer");
        getJGDI().triggerSchedulerMonitoringWithAnswer(answers);
        log.exiting("JGDIJMXBase", "triggerSchedulerMonitoringWithAnswer");
    }

    public String getSchedulerHost() throws JGDIException {
        log.entering("JGDIJMXBase", "getSchedulerHost");
        String ret = getJGDI().getSchedulerHost();
        log.exiting("JGDIJMXBase", "getSchedulerHost", ret);
        return ret;
    }

    public void enableQueues(String[] queues, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "enableQueues", new Object[]{queues, force});
        }
        getJGDI().enableQueues(queues, force);
        log.exiting("JGDIJMXBase", "enableQueues");
    }

    public void enableQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "enableQueuesWithAnswer", new Object[]{queues, force});
        }
        getJGDI().enableQueuesWithAnswer(queues, force, answers);
        log.exiting("JGDIJMXBase", "enableQueuesWithAnswer");
    }

    public void disableQueues(String[] queues, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "disableQueues", new Object[]{queues, force});
        }
        getJGDI().disableQueues(queues, force);
        log.exiting("JGDIJMXBase", "disableQueues");
    }

    public void disableQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "disableQueuesWithAnswer", new Object[]{queues, force});
        }
        getJGDI().disableQueuesWithAnswer(queues, force, answers);
        log.exiting("JGDIJMXBase", "disableQueuesWithAnswer");
    }

    public void suspend(String[] queues, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "suspend", new Object[]{queues, force});
        }
        getJGDI().suspend(queues, force);
        log.exiting("JGDIJMXBase", "suspend");
    }

    public void suspendWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "suspendWithAnswer", new Object[]{queues, force});
        }
        getJGDI().suspendWithAnswer(queues, force, answers);
        log.exiting("JGDIJMXBase", "suspendWithAnswer");
    }

    public void suspendQueues(String[] queues, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "suspendQueues", new Object[]{queues, force});
        }
        getJGDI().suspendQueues(queues, force);
        log.exiting("JGDIJMXBase", "suspendQueues");
    }

    public void suspendQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "suspendQueuesWithAnswer", new Object[]{queues, force});
        }
        getJGDI().suspendQueuesWithAnswer(queues, force, answers);
        log.exiting("JGDIJMXBase", "suspendQueuesWithAnswer");
    }

    public void suspendJobs(String[] jobs, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "suspendJobs", new Object[]{jobs, force});
        }
        getJGDI().suspendJobs(jobs, force);
        log.exiting("JGDIJMXBase", "suspendJobs");
    }

    public void suspendJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "suspendJobsWithAnswer", new Object[]{jobs, force});
        }
        getJGDI().suspendJobsWithAnswer(jobs, force, answers);
        log.exiting("JGDIJMXBase", "suspendJobsWithAnswer");
    }

    public void unsuspend(String[] queues, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "unsuspend", new Object[]{queues, force});
        }
        getJGDI().unsuspend(queues, force);
        log.exiting("JGDIJMXBase", "unsuspend");
    }

    public void unsuspendWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "unsuspendWithAnswer", new Object[]{queues, force});
        }
        getJGDI().unsuspendWithAnswer(queues, force, answers);
        log.exiting("JGDIJMXBase", "unsuspendWithAnswer");
    }

    public void unsuspendQueues(String[] queues, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "unsuspendQueues", new Object[]{queues, force});
        }
        getJGDI().unsuspendQueues(queues, force);
        log.exiting("JGDIJMXBase", "unsuspendQueues");
    }

    public void unsuspendQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "unsuspendQueuesWithAnswer", new Object[]{queues, force});
        }
        getJGDI().unsuspendQueuesWithAnswer(queues, force, answers);
        log.exiting("JGDIJMXBase", "unsuspendQueuesWithAnswer");
    }

    public void unsuspendJobs(String[] jobs, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "unsuspendJobs", new Object[]{jobs, force});
        }
        getJGDI().unsuspendJobs(jobs, force);
        log.exiting("JGDIJMXBase", "unsuspendJobs");
    }

    public void unsuspendJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "unsuspendJobsWithAnswer", new Object[]{jobs, force});
        }
        getJGDI().unsuspendJobsWithAnswer(jobs, force, answers);
        log.exiting("JGDIJMXBase", "unsuspendJobsWithAnswer");
    }

    public void clearQueues(String[] queues, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "clearQueues", new Object[]{queues, force});
        }
        getJGDI().clearQueues(queues, force);
        log.exiting("JGDIJMXBase", "clearQueues");
    }

    public void clearQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "clearQueuesWithAnswer", new Object[]{queues, force});
        }
        getJGDI().clearQueuesWithAnswer(queues, force, answers);
        log.exiting("JGDIJMXBase", "clearQueuesWithAnswer");
    }

    public void clearJobs(String[] jobs, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "clearJobs", new Object[]{jobs, force});
        }
        getJGDI().clearJobs(jobs, force);
        log.exiting("JGDIJMXBase", "clearJobs");
    }

    public void clearJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "clearJobsWithAnswer", new Object[]{jobs, force});
        }
        getJGDI().clearJobsWithAnswer(jobs, force, answers);
        log.exiting("JGDIJMXBase", "clearJobsWithAnswer");
    }

    public void reschedule(String[] queue_or_job, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "reschedule", new Object[]{queue_or_job, force});
        }
        getJGDI().reschedule(queue_or_job, force);
        log.exiting("JGDIJMXBase", "reschedule");
    }

    public void rescheduleWithAnswer(String[] queue_or_job, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "rescheduleWithAnswer", new Object[]{queue_or_job, force});
        }
        getJGDI().rescheduleWithAnswer(queue_or_job, force, answers);
        log.exiting("JGDIJMXBase", "rescheduleWithAnswer");
    }

    public void rescheduleQueues(String[] queues, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "rescheduleQueues", new Object[]{queues, force});
        }
        getJGDI().rescheduleQueues(queues, force);
        log.exiting("JGDIJMXBase", "rescheduleQueues");
    }

    public void rescheduleQueuesWithAnswer(String[] queues, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "rescheduleQueuesWithAnswer", new Object[]{queues, force});
        }
        getJGDI().rescheduleQueuesWithAnswer(queues, force, answers);
        log.exiting("JGDIJMXBase", "rescheduleQueuesWithAnswer");
    }

    public void rescheduleJobs(String[] jobs, boolean force) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "rescheduleJobs", new Object[]{jobs, force});
        }
        getJGDI().rescheduleJobs(jobs, force);
        log.exiting("JGDIJMXBase", "rescheduleJobs");
    }

    public void rescheduleJobsWithAnswer(String[] jobs, boolean force, List<JGDIAnswer> answers) throws JGDIException {
        if (log.isLoggable(Level.FINER)) {
            log.entering("JGDIJMXBase", "rescheduleJobsWithAnswer", new Object[]{jobs, force});
        }
        getJGDI().rescheduleJobsWithAnswer(jobs, force, answers);
        log.exiting("JGDIJMXBase", "rescheduleJobsWithAnswer");
    }

    public String showDetachedSettings(String[] queues) throws JGDIException {
        log.entering("JGDIJMXBase", "showDetachedSettings");
        String ret = getJGDI().showDetachedSettings(queues);
        log.exiting("JGDIJMXBase", "showDetachedSettings", ret);
        return ret;
    }

    public void deleteShareTree() throws JGDIException {
        log.entering("JGDIJMXBase", "deleteShareTree");
        getJGDI().deleteShareTree();
        log.exiting("JGDIJMXBase", "deleteShareTree");
    }

    public void deleteShareTreeWithAnswer(List<JGDIAnswer> answers) throws JGDIException {
        log.entering("JGDIJMXBase", "deleteShareTreeWithAnswer");
        getJGDI().deleteShareTreeWithAnswer(answers);
        log.exiting("JGDIJMXBase", "deleteShareTreeWithAnswer");
    }

    public String showDetachedSettingsAll() throws JGDIException {
        log.entering("JGDIJMXBase", "showDetachedSettingsAll");
        String ret = getJGDI().showDetachedSettingsAll();
        log.exiting("JGDIJMXBase", "showDetachedSettingsAll", ret);
        return ret;
    }

    public QHostOptions newQHostOptions() throws JGDIException {
        log.entering("JGDIJMXBase", "newQHostOptions");
        QHostOptions ret = new QHostOptions();
        log.exiting("JGDIJMXBase", "newQHostOptions", ret);
        return ret;
    }

    public ClusterQueueSummaryOptions newClusterQueueSummaryOptions() throws JGDIException {
        log.entering("JGDIJMXBase", "newClusterQueueSummaryOptions");
        ClusterQueueSummaryOptions ret = new ClusterQueueSummaryOptions();
        log.exiting("JGDIJMXBase", "newClusterQueueSummaryOptions", ret);
        return ret;
    }

    public QueueInstanceSummaryOptions newQueueInstanceSummaryOptions() throws JGDIException {
        log.entering("JGDIJMXBase", "newQueueInstanceSummaryOptions");
        QueueInstanceSummaryOptions ret = new QueueInstanceSummaryOptions();
        log.exiting("JGDIJMXBase", "newQueueInstanceSummaryOptions", ret);
        return ret;
    }

    public QQuotaOptions newQQuotaOptions() throws JGDIException {
        log.entering("JGDIJMXBase", "newQQuotaOptions");
        QQuotaOptions ret = new QQuotaOptions();
        log.exiting("JGDIJMXBase", "newQQuotaOptions", ret);
        return ret;
    }
//
//
//    //-------------------------------------------------------------------------
//    // DRMAA specific
//    //-------------------------------------------------------------------------
//    private String contact;
//    private Version drmaaJMXVersion;
//    private String drmSystem = "Unknown";
//    private String drmaaImplementation = "Bi, Ba, Bo";
//
//    public void drmaaInit(String contact) throws DrmaaException {
//        this.contact = contact;
//        this.drmaaImplementation = "DRMAA-JMX-SGE";
//        drmSystem = JGDIFactory.getJGDIVersion();
//        drmaaJMXVersion = new Version(1, 1);
//    }
//    
//    public void drmaaExit() throws DrmaaException {
//        logger.fine("calling exit()");
//    }
//    
//    public JobTemplate drmaaCreateJobTemplate() throws DrmaaException {
//        JobTemplate jt = null;
//        logger.fine("calling createJobTemplate()");
//        return jt;
//    }
//    
//    public void drmaaDeleteJobTemplate(JobTemplate jt)
//            throws DrmaaException {
//        logger.fine("calling deleteJobTemplate()");
//    };
//    
//    public String drmaaRunJob(JobTemplate jt) throws DrmaaException {
//        logger.fine("calling RunJob(JobTemplate jt)");
//        String jobId = "dummyId";
//        return jobId;
//    };
//    
//    public List drmaaRunBulkJobs(JobTemplate jt, int start, int end, int incr)
//             throws DrmaaException {
//        logger.fine("calling runBulkJobs");
//        List<String> jobs = new LinkedList<String>();
//        for (int i=start; i<end; i += incr) {
//            jobs.add("job" + i);
//        }
//        return jobs;
//    }
//    
//    public void drmaaControl(String jobId, int action) throws DrmaaException {
//        logger.fine("calling control for jobId=" + jobId + " and action="+ action);
//    }
//    
//    public void drmaaSynchronize(List jobIds, long timeout, boolean dispose)
//            throws DrmaaException {
//        logger.fine("calling synchronize");
//    }
//    
//    public JobInfo drmaaWait(String jobId, long timeout)
//            throws DrmaaException {
//        JobInfo ji = null;
//        logger.fine("calling wait");
//        return ji;
//    }
//    
//    public int drmaaGetJobProgramStatus(String jobId) throws DrmaaException {
//        logger.fine("calling getJobProgramStatus");
//        int progStatus = 0;
//        return progStatus;
//    }
//    
//    public String drmaaGetContact() {
//        logger.fine("calling getContact");
//        return contact;
//    }
//    
//    public Version drmaaGetVersion() {
//        return drmaaJMXVersion;
//    }
//    
//    public String drmaaGetDrmSystem() {
//        return drmSystem;
//    }
//    
//    public String drmaaGetDrmaaImplementation() {
//        return drmaaImplementation;
//    }
//
}
