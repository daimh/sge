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
package com.sun.grid.jgdi.monitoring;

import java.io.Serializable;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 */
public class QueueInstanceSummaryImpl implements QueueInstanceSummary, Serializable {

    private final static long serialVersionUID = -2009040301L;

    private static Logger logger = Logger.getLogger(QueueInstanceSummaryImpl.class.getName());
    
    private String name;
    private String queueType;
    private int reservedSlots;
    private int usedSlots;
    private int totalSlots;
    private String arch;
    private String state;
    private String loadAvgStr;
    private boolean hasLoadValue;
    private boolean hasLoadValueFromObject;
    private double loadAvg;
    private List<JobSummary> jobList;
    private String loadAlarmReason;
    private String suspendAlarmReason;
    private List<String> explainMessageList;
    private Map<String, Map<String, String>> resourceMap;
    
    /** Creates a new instance of QueueInstanceSummary */
    public QueueInstanceSummaryImpl() {
    }
    
    /**
     *  Get the name of the queue instance
     *  @return name of the queue instance
     */
    public String getName() {
        return name;
    }
    
    /**
     * Set the name of the queue instance
     * @param name name of the queue instance
     */
    public void setName(String name) {
        this.name = name;
    }
    
    /**
     *  Get the type of the queue
     *  @return type of the queue
     */
    public String getQueueType() {
        return queueType;
    }
    
    /**
     *  Set the type of the queue
     *  @param queueType type of the queue
     */
    public void setQueueType(String queueType) {
        this.queueType = queueType;
    }
    
    /**
     *  Get the number of reserved slots.
     *  @return number of reserved slots
     */
    public int getReservedSlots() {
        return reservedSlots;
    }
    
    /**
     *  Set the number of reserved slots
     *  @param reservedSlots number of reserved slots
     */
    public void setReservedSlots(int reservedSlots) {
        if (logger.isLoggable(Level.FINEST)) {
            logger.finest("setReservedSlots: " + reservedSlots);
        }
        this.reservedSlots = reservedSlots;
    }
    
    
    /**
     *  Get the number of used slots.
     *  @return number of used slots
     */
    public int getUsedSlots() {
        return usedSlots;
    }
    
    /**
     *  Set the number of used slots
     *  @param usedSlots number of used slots
     */
    public void setUsedSlots(int usedSlots) {
        if (logger.isLoggable(Level.FINEST)) {
            logger.finest("setUsedSlots: " + usedSlots);
        }
        this.usedSlots = usedSlots;
    }

    /**
     *  Get the number of free slots
     *  (returns total instead of free slots)
     *  @return number of free slots
     */
    @Deprecated
    public int getFreeSlots() {
        return totalSlots;
    }

    /**
     *  Get the number of total slots
     *  @return number of total slots
     */
    public int getTotalSlots() {
        return totalSlots;
    }
    
    /**
     *  Set the number of total slots
     *  @param totalSlots  number of total slots
     */
    public void setTotalSlots(int totalSlots) {
        this.totalSlots = totalSlots;
    }
    
    /**
     *  Get the architecture of the queue instance
     *  @return architecture of the queue instance
     */
    public String getArch() {
        return arch;
    }
    
    /**
     *  Set the architecture of the queue instance
     *  @param arch architecture of the queue instance
     */
    public void setArch(String arch) {
        this.arch = arch;
    }
    
    /**
     *  Get the state of the queue instance
     *  @return state of the queue instance
     */
    public String getState() {
        return state;
    }
    
    /**
     *  Set the state of a queue instance
     *  @param state state of a queue instance
     */
    public void setState(String state) {
        if (logger.isLoggable(Level.FINEST)) {
            logger.finest("setState: " + state);
        }
        this.state = state;
    }
    
    /**
     * Get the string representation of the load average
     *
     * @return the string representation of the load average
     */
    public String getLoadAvgStr() {
        return loadAvgStr;
    }
    
    /**
     * Set the string representation of the load average
     * @param loadAvgStr the string representation of the load average
     */
    public void setLoadAvgStr(String loadAvgStr) {
        this.loadAvgStr = loadAvgStr;
    }
    
    /**
     * Determine if the queue instance has a load value
     * @return <code>true</code> if the queue instance has a load value
     */
    public boolean hasLoadValue() {
        return hasLoadValue;
    }
    
    /**
     *  Set the has load value flag
     *
     *  @param hasLoadValue the has load value flag
     */
    public void setHasLoadValue(boolean hasLoadValue) {
        this.hasLoadValue = hasLoadValue;
    }
    
    /**
     *  Determine if the load value is derived from an object
     *  @return <code>true</code> if the load value is derived from an object
     */
    public boolean isHasLoadValueFromObject() {
        return hasLoadValueFromObject;
    }
    
    /**
     *  Set the has load value from object flag
     *  @param hasLoadValueFromObject the load value from object flag
     */
    public void setHasLoadValueFromObject(boolean hasLoadValueFromObject) {
        this.hasLoadValueFromObject = hasLoadValueFromObject;
    }
    
    /**
     *  Get the load average of the queue instance. Return only a meanful
     *  value if <code>hasLoadValue</code> returns <code>true</code>.
     *
     *  @return The load average of the queue instance
     *  @see #hasLoadValue
     */
    public double getLoadAvg() {
        return loadAvg;
    }
    
    /**
     *  Set the load average of the queue instance
     *  @param loadAvg the load average
     */
    public void setLoadAvg(double loadAvg) {
        this.loadAvg = loadAvg;
    }
    
    /**
     *  Add all jobs from a list
     *  @param jobList  list of {@link JobSummary} instances
     */
    public void addJobs(List<JobSummary> jobList) {
        if (!jobList.isEmpty()) {
            if (this.jobList == null) {
                this.jobList = new LinkedList<JobSummary>();
            }
            this.jobList.addAll(jobList);
            if (logger.isLoggable(Level.FINER)) {
                for (JobSummary job : jobList) {
                    logger.finer("job " + job.getId() + "(" + job.getTaskId() + ") added");
                }
            }
        }
    }
    
    /**
     *  Add a job.
     *  @param jobSummary the job
     */
    public void addJob(JobSummary jobSummary) {
        if (jobList == null) {
            jobList = new LinkedList<JobSummary>();
        }
        jobList.add(jobSummary);
        if (logger.isLoggable(Level.FINER)) {
            logger.finer("job " + jobSummary + " added");
        }
    }
    
    /**
     *  Get the list of jobs which are assigned to the queue instance
     *  @return list of jobs (instances of {@link JobSummary})
     */
    public List<JobSummary> getJobList() {
        if (jobList != null) {
            return Collections.unmodifiableList(jobList);
        } else {
            return Collections.EMPTY_LIST;
        }
    }
    
    /**
     *  Get the load alarm reason for the queue instance
     *  @return the load alarm reason
     */
    public String getLoadAlarmReason() {
        return loadAlarmReason;
    }
    
    /**
     *  Set the load alarm reason for the queue instance
     *  @param loadAlarmReason the load alarm reason
     */
    public void setLoadAlarmReason(String loadAlarmReason) {
        this.loadAlarmReason = loadAlarmReason;
    }
    
    /**
     *  Get the suspend alarm reason for the queue instance
     *  @return the suspend alarm reason
     */
    public String getSuspendAlarmReason() {
        return suspendAlarmReason;
    }
    
    /**
     *  Set the suspend alarm reason for the queue instance
     */
    public void setSuspendAlarmReason(String suspendAlarmReason) {
        this.suspendAlarmReason = suspendAlarmReason;
    }
    
    /**
     *  Add an explain message
     *  @param message the explain message
     */
    public void addExplainMessage(String message) {
        if (explainMessageList == null) {
            explainMessageList = new LinkedList<String>();
        }
        explainMessageList.add(message);
    }
    
    /**
     *  Get the list of explain messages
     *  @return list of explain messages
     */
    public List getExplainMessageList() {
        return Collections.unmodifiableList(explainMessageList);
    }
    
    /**
     *  Add a resource value to the queue instance
     *  @param dom  dominance of the resource value
     *  @param name name of the resource
     *  @param value value of the resource
     */
    public void addResource(String dom, String name, String value) {
        
        if (resourceMap == null) {
            resourceMap = new HashMap<String, Map<String, String>>();
        }
        
        Map<String, String> map = resourceMap.get(dom);
        if (map == null) {
            map = new HashMap<String, String>();
            resourceMap.put(dom, map);
        }
        map.put(name, value);
    }
    
    /**
     *  Get avaialable dominances for the resource values
     *  @return set of avaialable dominances
     */
    public Set<String> getResourceDominanceSet() {
        if (resourceMap == null) {
            return Collections.EMPTY_SET;
        } else {
            return resourceMap.keySet();
        }
    }
    
    /**
     *  Get all resources names which have the dominane <code>dom</code>
     *  @return set of resource names
     */
    public Set<String> getResourceNames(String dom) {
        if (resourceMap == null) {
            return Collections.EMPTY_SET;
        } else {
            Map<String, String> map = resourceMap.get(dom);
            if (map == null) {
                return Collections.EMPTY_SET;
            } else {
                return map.keySet();
            }
        }
    }
    
    /**
     *  Get a resource value
     *  @param  dom   dominance of the resource value
     *  @param  name  name of the resource
     *  @return the resource value
     */
    public String getResourceValue(String dom, String name) {
        if (resourceMap == null) {
            return null;
        } else {
            Map<String, String> map = resourceMap.get(dom);
            if (map == null) {
                return null;
            } else {
                return map.get(name);
            }
        }
    }
}
