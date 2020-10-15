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
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.logging.Logger;


/**
 *
 */
public class JGDIRemoteCacheBase implements JGDIRemoteBase {
    
    protected JGDIRemote jgdi;
    protected Logger logger = Logger.getLogger("com.sun.grid.jgdi.rmi");
    private List listeners = Collections.synchronizedList(new ArrayList());
    
    /** Creates a new instance of JGDIRemoteCacheBase */
    public JGDIRemoteCacheBase() {
    }
    
    public JGDIRemoteCacheBase(JGDIRemote jgdi) {
        this.jgdi = jgdi;
    }
    
    public void addJGDIRemoteCacheListener(JGDIRemoteCacheListener lis) {
        listeners.add(lis);
    }
    
    public void removeJGDIRemoteCacheListener(JGDIRemoteCacheListener lis) {
        listeners.remove(lis);
    }
    
    public void setJGDI(JGDIRemote jgdi) {
        this.jgdi = jgdi;
    }
    
    public JGDIRemote getJGDI() {
        return jgdi;
    }
    
    public void close() throws RemoteException {
        if( jgdi != null ) {
            jgdi.close();
            jgdi = null;
        }
    }
    
    private boolean actQMasterNotSet = true;
    private String  actQMaster;
    
    private RemoteException error;
    
    private RemoteException getError() {
        return error;
    }
    
    protected void handleError(RemoteException re) {
        try {
            close();
        } catch( RemoteException e) {
            
        } finally {
            Object [] lis = listeners.toArray();
            
            for(int i = 0; i < lis.length; i++ ) {
                ((JGDIRemoteCacheListener)lis[i]).errorOccured(this,re);
            }
            this.error = re;
        }
    }
    
    public void clearError() {
        this.error = null;
    }
    
    public String getActQMaster() {
        if(actQMasterNotSet) {
            if( jgdi != null ) {
                try {
                    actQMaster = jgdi.getActQMaster();
                } catch(RemoteException re) {
                    handleError(re);
                    return null;
                }
            }
            actQMasterNotSet = false;
        }
        return actQMaster;
    }
    
    private boolean actAdminUserNotSet = true;
    private String  adminUser;
    
    public String getAdminUser() {
        if(actAdminUserNotSet) {
            if( jgdi != null ) {
                try {
                    adminUser = jgdi.getAdminUser();
                } catch( RemoteException re ) {
                    handleError(re);
                    return null;
                }
            }
            actAdminUserNotSet = false;
        }
        return adminUser;
    }
    
    private boolean qhostResultNotSet = true;
    private QHostResult qhostResult;
    
    public QHostResult execQHost(QHostOptions options) throws RemoteException {
        if(qhostResultNotSet) {
            if( jgdi != null ) {
                try {
                    qhostResult = jgdi.execQHost(options);
                } catch( RemoteException re ) {
                    handleError(re);
                    return null;
                }
            }
            qhostResultNotSet = false;
        }
        return qhostResult;
    }
    
    private boolean SGERootNotSet = true;
    private File SGERoot;
    
    public File getSGERoot() throws RemoteException {
        if(SGERootNotSet) {
            if( jgdi != null ) {
                try {
                    SGERoot = jgdi.getSGERoot();
                } catch( RemoteException re ) {
                    handleError(re);
                    return null;
                }
            }
            SGERootNotSet = false;
        }
        return SGERoot;
    }
    
    private boolean SGECellNotSet = true;
    private String SGECell;
    
    public String getSGECell() throws RemoteException {
        if(SGECellNotSet) {
            if( jgdi != null ) {
                try {
                    SGECell = jgdi.getSGECell();
                } catch( RemoteException re ) {
                    handleError(re);
                    return null;
                }
            }
            SGECellNotSet = false;
        }
        return SGECell;
    }
    
    private boolean RealExecHostListNotSet = true;
    private List RealExecHostList;
    
    public List getRealExecHostList() throws RemoteException {
        if(RealExecHostListNotSet) {
            if( jgdi != null ) {
                try {
                    RealExecHostList = jgdi.getRealExecHostList();
                } catch( RemoteException re ) {
                    handleError(re);
                    return null;
                }
            }
            RealExecHostListNotSet = false;
        }
        return RealExecHostList;
    }
    
    public List getClusterQueueSummary(ClusterQueueSummaryOptions options) throws RemoteException {
        return null;
    }
    
    public QueueInstanceSummaryResult getQueueInstanceSummary(QueueInstanceSummaryOptions options) throws RemoteException {
        return null;
    }
    
    public QQuotaResult getQQuota(QQuotaOptions options) throws RemoteException {
        return null;
    }
    
    public void clearShareTreeUsage() throws RemoteException {
    }
    
    public void cleanQueues(String[] queues) throws RemoteException {
    }
    
    public void killMaster() throws RemoteException {
    }
    
    public void killScheduler() throws RemoteException {
    }
    
    public void killExecd(String[] hosts, boolean terminateJobs) throws RemoteException {
    }
    
    public void killAllExecds(boolean terminateJobs) throws RemoteException {
    }
    
    public void killEventClients(int[] ids) throws RemoteException {
    }
    
    public void killAllEventClients() throws RemoteException {
    }
    
    public void triggerSchedulerMonitoring() throws RemoteException {
    }
    
    public String getSchedulerHost() throws RemoteException {
        return null;
    }
    
    public void enableQueues(String[] queues, boolean force) throws RemoteException {
    }
    
    public void disableQueues(String[] queues, boolean force) throws RemoteException {
    }
    
    public void suspendQueues(String[] queues, boolean force) throws RemoteException {
    }
    
    public void suspendJobs(String[] jobs, boolean force) throws RemoteException {
    }
    
    public void unsuspendQueues(String[] queues, boolean force) throws RemoteException {
    }
    
    public void unsuspendJobs(String[] jobs, boolean force) throws RemoteException {
    }
    
    public void clearQueues(String[] queues, boolean force) throws RemoteException {
    }
    
    public void clearJobs(String[] jobs, boolean force) throws RemoteException {
    }
    
    public void rescheduleQueues(String[] queues, boolean force) throws RemoteException {
    }
    
    public void rescheduleJobs(String[] jobs, boolean force) throws RemoteException {
    }
    
}
