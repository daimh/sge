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
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * The class JobSummary extends the class JobInfo and adds addtional
 * information about the job.
 *
 */
public class JobSummaryImpl extends JobInfoImpl implements JobSummary, Serializable {

    private final static long serialVersionUID = -2009040301L;

    private double nurg;
    private double urg;
    private double nppri;
    private double nprior;
    private double ntckts;
    private double rrcontr;
    private double wtcontr;
    private double dlcontr;
    private String project;
    private String department;
    private Date deadline;
    
    private boolean hasCpuUsage;
    private int cpuUsage;
    private boolean hasMemUsage;
    private double memUsage;
    private boolean hasIoUsage;
    private double ioUsage;
    
    private boolean isZombie;
    private long overrideTickets;
    private boolean isQueueAssigned;
    private long tickets;
    private long otickets;
    private long ftickets;
    private long stickets;
    
    private double share;
    private String queue;
    private String master;
    private int slots;
    private boolean isArray;
    private boolean isRunning;
    
    private String parallelEnvironmentName;
    private String parallelEnvironmentRange;
    
    private String grantedPEName;
    private int grantedPESlots;
    
    private String checkpointEnv;
    private String masterQueue;
    
    private List<TaskSummary> taskList;
    
    /* Name value pairs for requested resources */
    private Map<String, String> requestMap;
    
    private Map<String, HardRequestValue> hardRequestMap;
    
    private Map<String, String> softRequestMap;
    
    private List<String> hardRequestedQueueList;
    
    private List<String> softRequestedQueueList;
    
    private List<String> hardRequestedMasterQueueList;
    
    private List<String> softRequestedMasterQueueList;
    
    private List<String> requestedPredecessorList;

    private List<String> requestedArrayPredecessorList;
    
    private List<Integer> predecessorList;
    
   private List<Integer> arrayPredecessorList;

    /** Creates a new instance of JobSummary */
    public JobSummaryImpl() {
    }
    
    /**
     *  Get the list of tasks of this job
     *  @return the list of tasks
     */
    public List<TaskSummary> getTaskList() {
        if (taskList == null) {
            return Collections.EMPTY_LIST;
        }
        return taskList;
    }
    
    /**
     *  Get the number of tasks of this job
     *  @return the number of tasks
     */
    public int getTaskCount() {
        if (taskList == null) {
            return 0;
        } else {
            return taskList.size();
        }
    }
    
    /**
     *  Add a new task to the job
     *  @param taskSummary the new task
     */
    public void addTask(TaskSummary taskSummary) {
        
        if (taskList == null) {
            taskList = new ArrayList<TaskSummary>();
        }
        taskList.add(taskSummary);
    }
    
    /**
     *  Add a resource request to the job
     *  @param  name of the resource
     *  @param  value requested value of the resource
     */
    public void addRequest(String name, String value) {
        if (requestMap == null) {
            requestMap = new HashMap<String, String>();
        }
        requestMap.put(name, value);
    }
    
    /**
     *   <p>Get all resources which have been requested by this job.</p>
     *
     *   <p>recource requsts are specified by qsub with the -l option (see man qsub).</p>
     *
     *   @return set of resources names
     */
    public Set<String> getRequestNames() {
        if (requestMap == null) {
            return Collections.EMPTY_SET;
        } else {
            return requestMap.keySet();
        }
    }
    
    /**
     *  Get value of a requested resource
     *
     *  @param  name  name of the requested resource
     *  @return value of the requested resource
     */
    public String getRequestValue(String name) {
        if (requestMap == null) {
            return null;
        } else {
            return requestMap.get(name);
        }
    }
    
    /**
     *  Add a hard resource request to the job summary.
     *  @param name  name of the resource
     *  @param value requested value
     *  @param uc    jobs urgency contribution for this resource
     */
    public void addHardRequest(String name, String value, double uc) {
        if (hardRequestMap == null) {
            hardRequestMap = new HashMap<String, HardRequestValue>();
        }
        hardRequestMap.put(name, new HardRequestValue(name, value, uc));
    }
    
    /**
     *  Get a set of all hard requested resources.
     *  @return set of resource names
     */
    public Set<String> getHardRequestNames() {
        if (hardRequestMap == null) {
            return Collections.EMPTY_SET;
        } else {
            return hardRequestMap.keySet();
        }
    }
    
    /**
     *  Get a value of a hard requested resource
     *  @param name  name of the hard requested resource
     *  @return  the hard requested resource value (includes the
     *           requested value and the urgency contribution)
     */
    public HardRequestValue getHardRequestValue(String name) {
        if (hardRequestMap == null) {
            return null;
        } else {
            return hardRequestMap.get(name);
        }
    }
    
    /**
     *  Add a soft request to the job
     *  @param name  name of the requested resource
     *  @param value requested value
     */
    public void addSoftRequest(String name, String value) {
        if (softRequestMap == null) {
            softRequestMap = new HashMap<String, String>();
        }
        softRequestMap.put(name, value);
    }
    
    /**
     *  Get a set of all soft requested resources
     *  @return set of all soft requested resources
     */
    public Set<String> getSoftRequestNames() {
        if (softRequestMap == null) {
            return Collections.EMPTY_SET;
        } else {
            return softRequestMap.keySet();
        }
    }
    
    /**
     *  Get the value of a soft requested resource
     *  @param name name of the resource
     *  @return requested value
     */
    public String getSoftRequestValue(String name) {
        if (softRequestMap == null) {
            return null;
        } else {
            return softRequestMap.get(name);
        }
    }
    
    /**
     *   Add a hard requested queue
     *
     *   @param qname of the hard requested queue
     */
    public void addHardRequestedQueue(String qname) {
        if (hardRequestedQueueList == null) {
            hardRequestedQueueList = new ArrayList<String>();
        }
        hardRequestedQueueList.add(qname);
    }
    
    /**
     *  Get a list of all hard requested queues
     *  @return list of hard requested queues
     */
    public List<String> getHardRequestedQueues() {
        if (hardRequestedQueueList == null) {
            return Collections.EMPTY_LIST;
        } else {
            return Collections.unmodifiableList(hardRequestedQueueList);
        }
    }
    
    /**
     *  Add a soft requested queue
     *
     *  @param  qname name of the soft requested queue
     */
    public void addSoftRequestedQueue(String qname) {
        if (softRequestedQueueList == null) {
            softRequestedQueueList = new ArrayList<String>();
        }
        softRequestedQueueList.add(qname);
    }
    
    /**
     *  Get a list of all soft requested queues
     *  @return list of all soft requested queues
     */
    public List<String> getSoftRequestedQueues() {
        if (softRequestedQueueList == null) {
            return Collections.EMPTY_LIST;
        } else {
            return Collections.unmodifiableList(softRequestedQueueList);
        }
    }
    
    /**
     *  Adds a hard requested master queue to the job
     *  (see qsub -masterq).
     *
     *  @param qname  name of the hard requested master queue
     */
    public void addHardRequestedMasterQueue(String qname) {
        if (hardRequestedMasterQueueList == null) {
            hardRequestedMasterQueueList = new ArrayList<String>();
        }
        hardRequestedMasterQueueList.add(qname);
    }
    
    /**
     *  Get a list of all hard requested master queues
     *  @return list of all hard requested master queues
     */
    public List<String> getHardRequestedMasterQueues() {
        if (hardRequestedMasterQueueList == null) {
            return Collections.EMPTY_LIST;
        } else {
            return Collections.unmodifiableList(hardRequestedMasterQueueList);
        }
    }
    
    /**
     *  Add a soft requested master queue
     *  (see qsub -masterq)
     *  @param qname of the soft requested master queue
     */
    public void addSoftRequestedMasterQueue(String qname) {
        if (softRequestedMasterQueueList == null) {
            softRequestedMasterQueueList = new ArrayList<String>();
        }
        softRequestedMasterQueueList.add(qname);
    }
    
    /**
     *  Get a list of all soft requested master queues
     *  @return list of all soft requested master queues
     */
    public List getSoftRequestedMasterQueues() {
        if (softRequestedMasterQueueList == null) {
            return Collections.EMPTY_LIST;
        } else {
            return Collections.unmodifiableList(softRequestedMasterQueueList);
        }
    }
    
    /**
     *  add a requested predecessor to the job
     *  (predecessor of a job can be requested with
     *   qsub -hold_jid)
     *
     *  @param  name of the predecessor
     */
    public void addRequestedPredecessor(String name) {
        if (requestedPredecessorList == null) {
            requestedPredecessorList = new ArrayList<String>();
        }
        requestedPredecessorList.add(name);
    }

   /**
    *  add a requested array predecessor to the job
    *  (array predecessor of a job can be requested with
    *   qsub -hold_jid_ad)
    *
    *  @param  name of the array predecessor
    */
   public void addRequestedArrayPredecessor(String name) {
      if (requestedArrayPredecessorList == null) {
         requestedArrayPredecessorList = new ArrayList<String>();
      }
      requestedArrayPredecessorList.add(name);
   }

    
    /**
     * Get a list of all requested predecessors
     * @return list of requested predecessors job names
     */
    public List<String> getRequestedPredecessors() {
        if (requestedPredecessorList == null) {
            return Collections.EMPTY_LIST;
        } else {
            return Collections.unmodifiableList(requestedPredecessorList);
        }
    }
    
   /**
    * Get a list of all requested array predecessors
    * @return list of requested array predecessors job names
    */
   public List<String> getRequestedArrayPredecessors() {
      if(requestedArrayPredecessorList == null) {
         return Collections.EMPTY_LIST;
      } else {
         return Collections.unmodifiableList(requestedArrayPredecessorList);
      }
   }

    /**
     *  Add the job id of a predecessor
     *  @param job_id job id of the predecessor
     */
    public void addPredecessor(int job_id) {
        if (predecessorList == null) {
            predecessorList = new ArrayList<Integer>();
        }
        predecessorList.add(new Integer(job_id));
    }
    
   /**
    *  Add the job id of an array predecessor
    *  @param job_id job id of the array predecessor
    */
   public void addArrayPredecessor(int job_id) {
      if (arrayPredecessorList == null) {
         arrayPredecessorList = new ArrayList<Integer>();
      }
      arrayPredecessorList.add(new Integer(job_id));
   }

    /**
     *  Get a list of all predecessor job id
     *  @return list of job ids (java.lang.Integer)
     */
    public List<Integer> getPredecessors() {
        if (predecessorList == null) {
            return Collections.EMPTY_LIST;
        } else {
            return Collections.unmodifiableList(predecessorList);
        }
    }
    
    /**
     *  Get a list of all array predecessor job id
     *  @return list of job ids (java.lang.Integer)
     */
    public List<Integer> getArrayPredecessors() {
       if(arrayPredecessorList == null) {
          return Collections.EMPTY_LIST;
       } else {
          return Collections.unmodifiableList(arrayPredecessorList);
       }
    }
 
   /**
     *  Get the jobs total urgency value in normalized fashion.
     *  @return the jobs total urgency value in normalized fashion
     */
    public double getNormalizedUrgency() {
        return nurg;
    }
    
    /**
     * Set the jobs total urgency value in normalized fashion.
     * @param nurg the normalized total urgency
     */
    public void setNormalizedUrgency(double nurg) {
        this.nurg = nurg;
    }
    
    /**
     *  Get the total urgency of the job
     *  @return total urgency of the job
     */
    public double getUrgency() {
        return urg;
    }
    
    /**
     *  Set the total urgency of the job
     *  @param urg the total urgency
     */
    public void setUrgency(double urg) {
        this.urg = urg;
    }
    
    /**
     *  Get the priority of the job which has been
     *  requested by the user in normalized form
     *
     *  @return the normalized job priority
     */
    public double getNormalizedRequestedPriority() {
        return nppri;
    }
    
    /**
     *  Set the priority of the job which has been requested
     *  by the user in normalized form
     *  @param nppri requested priority in normalized
     */
    public void setNormalizedRequestedPriority(double nppri) {
        this.nppri = nppri;
    }
    
    /**
     *  Get the normalized priority of the job
     *  @return the normalized priority of the job
     */
    public double getNormalizedPriority() {
        return nprior;
    }
    
    /**
     *  Get the normalized priority of the job
     *  param nprior  the normalized priority
     */
    public void setNormalizedPriority(double nprior) {
        this.nprior = nprior;
    }
    
    /**
     *  Get the normalized total number of tickets
     *  @return the normalized total number of tickets
     */
    public double getNormalizedTickets() {
        return ntckts;
    }
    
    /**
     *  Set the normalized total number of tickets
     *  @param ntckts the normalized total number of tickets
     */
    public void setNormalizedTickets(double ntckts) {
        this.ntckts = ntckts;
    }
    
    /**
     * Get the urgency value contribution that  reflects the urgency
     * that is related to the jobs overall resource requirement.
     *
     * @return the urgency value contribution
     */
    public double getRrcontr() {
        return rrcontr;
    }
    
    /**
     * Set the urgency value contribution that  reflects the urgency
     * that is related to the jobs overall resource requirement.
     */
    public void setRrcontr(double rrcontr) {
        this.rrcontr = rrcontr;
    }
    
    /**
     * Get the urgency value contribution that reflects the
     * urgency related to the jobs waiting time.
     * @return the urgency value contribution
     */
    public double getWtcontr() {
        return wtcontr;
    }
    
    /**
     * Set the urgency value contribution that reflects the
     * urgency related to the jobs waiting time.
     * @param wtcontr the urgency value contribution
     */
    public void setWtcontr(double wtcontr) {
        this.wtcontr = wtcontr;
    }
    
    /**
     *  Get the urgency value contribution that reflects the
     *  urgency related to the jobs deadline initiation time.
     *  @return the urgency value contribution
     */
    public double getDlcontr() {
        return dlcontr;
    }
    
    /**
     *  Set the urgency value contribution that reflects the
     *  urgency related to the jobs deadline initiation time.
     *  @param dlcontr the urgency value contribution
     */
    public void setDlcontr(double dlcontr) {
        this.dlcontr = dlcontr;
    }
    
    /**
     *  Get the project of the job
     *  @return the project of the job
     */
    public String getProject() {
        return project;
    }
    
    /**
     *  Set the project of the job
     *  @param  project project of the job
     */
    public void setProject(String project) {
        this.project = project;
    }
    
    /**
     *  Get the department of the job
     *  @return the department of the job
     */
    public String getDepartment() {
        return department;
    }
    
    /**
     *  Set the department of the job
     *  @param department the department of the job
     */
    public void setDepartment(String department) {
        this.department = department;
    }
    
    /**
     *  Get the deadline of the job
     *  @return the deadline
     */
    public Date getDeadline() {
        return deadline;
    }
    
    /**
     *  Set the deadline of the job
     *  @param deadline the deadline of the job
     */
    public void setDeadline(Date deadline) {
        this.deadline = deadline;
    }
    
    /**
     *  Set the deadline of the job
     *  @param deadline the deadline of the job in millis
     */
    public void setDeadline(long deadline) {
        this.deadline = new Date(deadline);
    }
    
    /**
     *  Determine if the job has a cpu usage
     *  @return true of the job has a cpu usage
     */
    public boolean hasCpuUsage() {
        return hasCpuUsage;
    }
    
    /**
     *  Get the cpu usage of the job. If the jobs has no
     *  cpu usage, <code>0</code> is returned
     *  @return  the cpu usage of the job
     *  @see #hasCpuUsage
     */
    public int getCpuUsage() {
        return cpuUsage;
    }
    
    /**
     *  Set the cpu usage of the job
     *  @param cpuUsage the cpu usage
     */
    public void setCpuUsage(int cpuUsage) {
        this.cpuUsage = cpuUsage;
        hasCpuUsage = true;
    }
    
    /**
     *  Determine if the job has a memory usage
     *  @return <code>true</code> if the job has a memory usage
     */
    public boolean hasMemUsage() {
        return hasMemUsage;
    }
    
    /**
     *  Get the memory usage of the job. If the job has no memory
     *  usage <code>0</code> is returned.
     *
     *  @return the memory usage of the job
     *  @see #hasMemUsage
     */
    public double getMemUsage() {
        return memUsage;
    }
    
    /**
     *  Set the memory usage of the job
     *  @param memUsage the memory usage
     */
    public void setMemUsage(double memUsage) {
        this.memUsage = memUsage;
        hasMemUsage = true;
    }
    
    /**
     *  Determine if the job has a io usage
     *  @return true of the job has a io usage
     */
    public boolean hasIoUsage() {
        return hasIoUsage;
    }
    
    /**
     *  Get the io usage of the job. If the job has no io
     *  usage <code>0</code> is returned.
     *
     *  @return the io usage of the job
     *  @see #hasIoUsage
     */
    public double getIoUsage() {
        return ioUsage;
    }
    
    /**
     *  Set the io usage of the job.
     *  @param ioUsage the io usage of the job
     */
    public void setIoUsage(double ioUsage) {
        this.ioUsage = ioUsage;
        hasIoUsage = true;
    }
    
    /**
     *  Determine if the job is a zombie
     *  @return <code>false</code> if the job a zombie
     */
    public boolean isZombie() {
        return isZombie;
    }
    
    /**
     *  Set the zombie flag
     *  @param  isZombie the zombie flag
     */
    public void setZombie(boolean isZombie) {
        this.isZombie = isZombie;
    }
    
    /**
     *  Get the override tickets of the job
     *  @return the override tickets of the job
     */
    public long getOverrideTickets() {
        return overrideTickets;
    }
    
    /**
     *  Set the override tickets of the job
     *  @param overrideTickets the override tickets of the job
     */
    public void setOverrideTickets(long overrideTickets) {
        this.overrideTickets = overrideTickets;
    }
    
    /**
     *  Determine if the job is assigned to a queue
     *  @return <code>true</code> if the job is assigned to a queue
     */
    public boolean isQueueAssigned() {
        return isQueueAssigned;
    }
    
    /**
     *  Set the queue assigned falg
     *  @param isQueueAssigned the queue assigned flag
     */
    public void setQueueAssigned(boolean isQueueAssigned) {
        this.isQueueAssigned = isQueueAssigned;
    }
    
    /**
     *  Get the currently number of tickets of the job
     *  @return currently number of tickets
     */
    public long getTickets() {
        return tickets;
    }
    
    /**
     *  Set the currently number of tickets of the job
     *  @param tickets currently number of tickets
     */
    public void setTickets(long tickets) {
        this.tickets = tickets;
    }
    
    /**
     * Get the override portion of the total number of tickets
     * assigned to the job currently
     * @return override portion of the total number of tickets
     */
    public long getOtickets() {
        return otickets;
    }
    
    /**
     *  Set the override portion of the total number of tickets
     *  assigned to the job currently
     *  @param otickets the override portion of the total number of tickets
     */
    public void setOtickets(long otickets) {
        this.otickets = otickets;
    }
    
    /**
     *  Get the functional portion of the total number of tickets
     *  assigned to the job currently.
     *
     *  @return the functional portion of the total number of tickets
     */
    public long getFtickets() {
        return ftickets;
    }
    
    /**
     *  Set the functional portion of the total number of tickets
     *  assigned to the job currently.
     *
     *  @param ftickets the functional portion of the total number of tickets
     */
    public void setFtickets(long ftickets) {
        this.ftickets = ftickets;
    }
    
    /**
     * Get the share portion of the total number of tickets
     * assigned to the job currently.
     *
     * @return the share portion of the total number of tickets
     */
    public long getStickets() {
        return stickets;
    }
    
    /**
     * Set the share portion of the total number of tickets
     * assigned to the job currently.
     *
     * @param stickets the share portion of the total number of tickets
     */
    public void setStickets(long stickets) {
        this.stickets = stickets;
    }
    
    /**
     * Get the share of the total system to which the job is entitled currently.
     * @return the share of the job
     */
    public double getShare() {
        return share;
    }
    
    /**
     * Set the share of the total system to which the job is entitled currently.
     * @param share the share of the job
     */
    public void setShare(double share) {
        this.share = share;
    }
    
    /**
     *  Get the number of used slots
     *  @return the number of used slots
     */
    public int getSlots() {
        return slots;
    }
    
    /**
     *  Set the number of used slots
     *  @param slots the number of used slots
     */
    public void setSlots(int slots) {
        this.slots = slots;
    }
    
    /**
     *  Determine if the job is an array job
     *  @return <code>true</code> if the job is an array job
     */
    public boolean isArray() {
        return isArray;
    }
    
    /**
     *  Set the array job flag
     *  @param isArray the array job flag
     */
    public void setArray(boolean isArray) {
        this.isArray = isArray;
    }
    
    /**
     *  Determine if the job is running
     *  @return <code>true</code> of the job is running
     */
    public boolean isRunning() {
        return isRunning;
    }
    
    /**
     *  Set the is running flag
     *  @param isRunning the is running flag
     */
    public void setRunning(boolean isRunning) {
        this.isRunning = isRunning;
    }
    
    /**
     *  Get the name of the parallel environment of the job
     *  @return name of the parallel environment
     */
    public String getParallelEnvironmentName() {
        return parallelEnvironmentName;
    }
    
    /**
     *  Set the name of the parallel environment of the job
     *  @param parallelEnvironmentName name of the parallel environment
     */
    public void setParallelEnvironmentName(String parallelEnvironmentName) {
        this.parallelEnvironmentName = parallelEnvironmentName;
    }
    
    /**
     * Get the requested PE slot range.
     * @return the requested PE slot range
     */
    public String getParallelEnvironmentRange() {
        return parallelEnvironmentRange;
    }
    
    /**
     * Set the requested PE slot range.
     * @param parallelEnvironmentRange the requested PE slot range
     */
    public void setParallelEnvironmentRange(String parallelEnvironmentRange) {
        this.parallelEnvironmentRange = parallelEnvironmentRange;
    }
    
    /**
     *   Get the name of the granted PE
     *   @return name of the granted PE
     */
    public String getGrantedPEName() {
        return grantedPEName;
    }
    
    /**
     *   Set the name of the granted PE
     *   @param grantedPEName name of the granted PE
     */
    public void setGrantedPEName(String grantedPEName) {
        this.grantedPEName = grantedPEName;
    }
    
    /**
     *  Get the number of granted PE slots
     *  @return number of granted PE slots
     */
    public int getGrantedPESlots() {
        return grantedPESlots;
    }
    
    /**
     *  Set the number of granted PE slots
     *  @param grantedPESlots number of granted PE slots
     */
    public void setGrantedPESlots(int grantedPESlots) {
        this.grantedPESlots = grantedPESlots;
    }
    
    /**
     *  Get the checkpoint environment of the job
     *  @return the checkpoint environment of the job
     */
    public String getCheckpointEnv() {
        return checkpointEnv;
    }
    
    /**
     *  Set the checkpoint environment of the job
     *  @param checkpointEnv the checkpoint environment of the job
     */
    public void setCheckpointEnv(String checkpointEnv) {
        this.checkpointEnv = checkpointEnv;
    }
}
