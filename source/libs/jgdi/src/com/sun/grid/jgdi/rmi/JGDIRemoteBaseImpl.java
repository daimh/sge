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

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.monitoring.ClusterQueueSummaryOptions;
import com.sun.grid.jgdi.monitoring.QHostOptions;
import com.sun.grid.jgdi.monitoring.QHostResult;
import com.sun.grid.jgdi.monitoring.QQuotaOptions;
import com.sun.grid.jgdi.monitoring.QQuotaResult;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryOptions;
import com.sun.grid.jgdi.monitoring.QueueInstanceSummaryResult;
import java.io.File;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.List;
import java.util.logging.Logger;


/**
 *
 */
public class JGDIRemoteBaseImpl extends UnicastRemoteObject implements JGDIRemoteBase {
    
    protected JGDI jgdi;
    protected Logger logger = Logger.getLogger("com.sun.grid.jgdi.rmi");
    
    protected JGDIRemoteBaseImpl(String url) throws RemoteException, JGDIException {
// synchronized version does not work with RMI - why ?
//        jgdi = JGDIFactory.newSynchronizedInstance(url);
        jgdi = JGDIFactory.newInstance(url);
    }
    
    public void close() throws RemoteException {
        logger.entering("JGDIRemoteBaseImpl","close");
        JGDI jgdi = this.jgdi;
        this.jgdi = null;
        if( jgdi != null ) {
            try {
                jgdi.close();
            } catch( JGDIException e ) {
                throw new RemoteException(e.getMessage(), e);
            }
        }
        logger.exiting("JGDIRemoteBaseImpl","close");
    }
    
    
    public String getActQMaster() throws java.rmi.RemoteException {
        if( jgdi == null ) {
            return null;
        } else {
            try {
                return jgdi.getActQMaster();
            } catch( JGDIException e ) {
                throw new RemoteException("Can not get act qmaster", e );
            }
        }
    }
    
    public String getAdminUser() throws RemoteException {
        if( jgdi == null ) {
            return null;
        } else {
            try {
                return jgdi.getAdminUser();
            } catch( JGDIException e ) {
                throw new RemoteException("Can not get admin user", e );
            }
        }
    }
    
    public QHostResult execQHost(QHostOptions options) throws RemoteException {
        if( jgdi == null ) {
            return null;
        } else {
            try {
                return jgdi.execQHost(options);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "execQHost", e);
                throw new RemoteException("Error while executing qhost", e );
            }
        }
    }
    
    public File getSGERoot() throws RemoteException {
        return null;
    }
    
    public String getSGECell() throws RemoteException {
        return null;
    }
    
    public List getRealExecHostList() throws RemoteException {
        return null;
    }
    
    public List getClusterQueueSummary(ClusterQueueSummaryOptions options) throws RemoteException {
        return null;
    }
    
    public QueueInstanceSummaryResult getQueueInstanceSummary(QueueInstanceSummaryOptions options) throws RemoteException {
        return null;
    }
    
    public QQuotaResult getQQuota(QQuotaOptions options) throws RemoteException {
        if( jgdi == null ) {
            return null;
        } else {
            try {
                return jgdi.getQQuota(options);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "execQHost", e);
                throw new RemoteException("Error while executing qlimit", e );
            }
        }
    }
    
    public void clearShareTreeUsage() throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.clearShareTreeUsage();
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "clearShareTreeUsage", e);
                throw new RemoteException("Error while executing clearShareTreeUsage", e );
            }
        }
    }
    
    public void cleanQueues(String[] queues) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.cleanQueues(queues);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "cleanQueues", e);
                throw new RemoteException("Error while executing cleanQueues", e );
            }
        }
    }
    
    public void killMaster() throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.killMaster();
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "killMaster", e);
                throw new RemoteException("Error while executing killMaster", e );
            }
        }
    }
    
    public void killScheduler() throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.killScheduler();
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "killScheduler", e);
                throw new RemoteException("Error while executing killScheduler", e );
            }
        }
    }
    
    public void killExecd(String[] hosts, boolean terminateJobs) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.killExecd(hosts, terminateJobs);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "killExecd", e);
                throw new RemoteException("Error while executing killExecd", e );
            }
        }
    }
    
    public void killAllExecds(boolean terminateJobs) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.killAllExecds(terminateJobs);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "killAllExecds", e);
                throw new RemoteException("Error while executing killAllExecds", e );
            }
        }
    }
    
    public void killEventClients(int[] ids) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.killEventClients(ids);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "killEventClients", e);
                throw new RemoteException("Error while executing killEventClients", e );
            }
        }
    }
    
    public void killAllEventClients() throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.killAllEventClients();
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "killAllEventClients", e);
                throw new RemoteException("Error while executing killAllEventClients", e );
            }
        }
    }
    
    public void triggerSchedulerMonitoring() throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.triggerSchedulerMonitoring();
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "triggerSchedulerMonitoring", e);
                throw new RemoteException("Error while executing triggerSchedulerMonitoring", e );
            }
        }
    }
    
    public String getSchedulerHost() throws RemoteException {
        if(jgdi != null) {
            try {
                return jgdi.getSchedulerHost();
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "getSchedulerHost", e);
                throw new RemoteException("Error while executing getSchedulerHost", e );
            }
        }
        return null;
    }
    
    public void enableQueues(String[] queues, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.enableQueues(queues, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "enableQueues", e);
                throw new RemoteException("Error while executing enableQueues", e );
            }
        }
    }
    
    public void disableQueues(String[] queues, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.disableQueues(queues, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "disableQueues", e);
                throw new RemoteException("Error while executing disableQueues", e );
            }
        }
    }
    
    public void suspendQueues(String[] queues, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.suspendQueues(queues, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "suspendQueues", e);
                throw new RemoteException("Error while executing suspendQueues", e );
            }
        }
    }
    
    public void unsuspendQueues(String[] queues, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.unsuspendQueues(queues, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "unsuspendQueues", e);
                throw new RemoteException("Error while executing unsuspendQueues", e );
            }
        }
    }
    
    public void clearQueues(String[] queues, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.clearQueues(queues, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "clearQueues", e);
                throw new RemoteException("Error while executing clearQueues", e );
            }
        }
    }
    
    public void rescheduleQueues(String[] queues, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.rescheduleQueues(queues, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "rescheduleQueues", e);
                throw new RemoteException("Error while executing rescheduleQueues", e );
            }
        }
    }
    
    public void suspendJobs(String[] jobs, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.suspendJobs(jobs, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "suspendJobs", e);
                throw new RemoteException("Error while executing suspendJobs", e );
            }
        }
    }
    
    
    public void unsuspendJobs(String[] jobs, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.unsuspendJobs(jobs, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "unsuspendJobs", e);
                throw new RemoteException("Error while executing unsuspendJobs", e );
            }
        }
    }
    
    
    public void clearJobs(String[] jobs, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.clearJobs(jobs, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "clearJobs", e);
                throw new RemoteException("Error while executing clearJobs", e );
            }
        }
    }
    
    
    public void rescheduleJobs(String[] jobs, boolean force) throws RemoteException {
        if(jgdi != null) {
            try {
                jgdi.rescheduleJobs(jobs, force);
            } catch( JGDIException e ) {
                logger.throwing(getClass().getName(), "rescheduleJobs", e);
                throw new RemoteException("Error while executing rescheduleJobs", e );
            }
        }
    }
}
