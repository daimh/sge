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

import java.util.Date;
import java.util.List;
import java.util.Set;

/**
 * The class JobSummary extends the class JobInfo and adds addtional
 * information about the job.
 *
 */
public interface JobSummary extends JobInfo {
    
    /**
     *  Get the list of tasks of this job
     *  @return the list of tasks
     */
    public List getTaskList();
    
    /**
     *  Get the number of tasks of this job
     *  @return the number of tasks
     */
    public int getTaskCount();
    
    /**
     *   <p>Get all resources which have been requested by this job.</p>
     *
     *   <p>recource requsts are specified by qsub with the -l option (see man qsub).</p>
     *
     *   @return set of resources names
     */
    public Set<String> getRequestNames();
    
    /**
     *  Get value of a requested resource
     *
     *  @param  name  name of the requested resource
     *  @return value of the requested resource
     */
    public String getRequestValue(String name);
    
    /**
     *  Get a set of all hard requested resources.
     *  @return set of resource names
     */
    public Set<String> getHardRequestNames();
    
    /**
     *  Get a value of a hard requested resource
     *  @param name  name of the hard requested resource
     *  @return  the hard requested resource value (includes the
     *           requested value and the urgency contribution)
     */
    public HardRequestValue getHardRequestValue(String name);
    
    /**
     *  Get a set of all soft requested resources
     *  @return set of all soft requested resources
     */
    public Set<String> getSoftRequestNames();
    
    /**
     *  Get the value of a soft requested resource
     *  @param name name of the resource
     *  @return requested value
     */
    public String getSoftRequestValue(String name);
    
    /**
     *  Get a list of all hard requested queues
     *  @return list of hard requested queues
     */
    public List getHardRequestedQueues();
    
    /**
     *  Add a soft requested queue
     *
     *  @param  qname name of the soft requested queue
     */
    public void addSoftRequestedQueue(String qname);
    
    /**
     *  Get a list of all soft requested queues
     *  @return list of all soft requested queues
     */
    public List getSoftRequestedQueues();
    
    /**
     *  Get a list of all hard requested master queues
     *  @return list of all hard requested master queues
     */
    public List getHardRequestedMasterQueues();
    
    /**
     *  Add a soft requested master queue
     *  (see qsub -masterq)
     *  @param qname of the soft requested master queue
     */
    public void addSoftRequestedMasterQueue(String qname);
    
    /**
     *  Get a list of all soft requested master queues
     *  @return list of all soft requested master queues
     */
    public List getSoftRequestedMasterQueues();
    
    
    /**
     * Get a list of all requested predecessors
     * @return list of requested predecessors job names
     */
    public List getRequestedPredecessors();
    
   /**
    * Get a list of all requested array predecessors
    * @return list of requested array predecessors job names
    */
   public List getRequestedArrayPredecessors();

    /**
     *  Get a list of all predecessor job id
     *  @return list of job ids (java.lang.Integer)
     */
    public List getPredecessors();
    
    /**
     *  Get a list of all array predecessor job id
     *  @return list of job ids (java.lang.Integer)
     */
    public List getArrayPredecessors();

    /**
     *  Get the jobs total urgency value in normalized fashion.
     *  @return the jobs total urgency value in normalized fashion
     */
    public double getNormalizedUrgency();
    
    /**
     * Set the jobs total urgency value in normalized fashion.
     * @param nurg the normalized total urgency
     */
    public void setNormalizedUrgency(double nurg);
    
    /**
     *  Get the total urgency of the job
     *  @return total urgency of the job
     */
    public double getUrgency();
    
    /**
     *  Get the priority of the job which has been
     *  requested by the user in normalized form
     *
     *  @return the normalized job priority
     */
    public double getNormalizedRequestedPriority();
    
    /**
     *  Get the normalized priority of the job
     *  @return the normalized priority of the job
     */
    public double getNormalizedPriority();
    
    /**
     *  Get the normalized total number of tickets
     *  @return the normalized total number of tickets
     */
    public double getNormalizedTickets();
    
    /**
     * Get the urgency value contribution that  reflects the urgency
     * that is related to the jobs overall resource requirement.
     *
     * @return the urgency value contribution
     */
    public double getRrcontr();
    
    /**
     * Get the urgency value contribution that reflects the
     * urgency related to the jobs waiting time.
     * @return the urgency value contribution
     */
    public double getWtcontr();
    
    /**
     *  Get the urgency value contribution that reflects the
     *  urgency related to the jobs deadline initiation time.
     *  @return the urgency value contribution
     */
    public double getDlcontr();
    
    /**
     *  Get the project of the job
     *  @return the project of the job
     */
    public String getProject();
    
    /**
     *  Get the department of the job
     *  @return the department of the job
     */
    public String getDepartment();
    
    /**
     *  Get the deadline of the job
     *  @return the deadline
     */
    public Date getDeadline();
    
    /**
     *  Determine if the job has a cpu usage
     *  @return true of the job has a cpu usage
     */
    public boolean hasCpuUsage();
    
    /**
     *  Get the cpu usage of the job. If the jobs has no
     *  cpu usage, <code>0</code> is returned
     *  @return  the cpu usage of the job
     *  @see #hasCpuUsage
     */
    public int getCpuUsage();
    
    /**
     *  Determine if the job has a memory usage
     *  @return <code>true</code> if the job has a memory usage
     */
    public boolean hasMemUsage();
    
    /**
     *  Get the memory usage of the job. If the job has no memory
     *  usage <code>0</code> is returned.
     *
     *  @return the memory usage of the job
     *  @see #hasMemUsage
     */
    public double getMemUsage();
    
    /**
     *  Determine if the job has a io usage
     *  @return true of the job has a io usage
     */
    public boolean hasIoUsage();
    
    /**
     *  Get the io usage of the job. If the job has no io
     *  usage <code>0</code> is returned.
     *
     *  @return the io usage of the job
     *  @see #hasIoUsage
     */
    public double getIoUsage();
    
    /**
     *  Determine if the job is a zombie
     *  @return <code>false</code> if the job a zombie
     */
    public boolean isZombie();
    
    /**
     *  Get the override tickets of the job
     *  @return the override tickets of the job
     */
    public long getOverrideTickets();
    
    /**
     *  Determine if the job is assigned to a queue
     *  @return <code>true</code> if the job is assigned to a queue
     */
    public boolean isQueueAssigned();
    
    /**
     *  Get the currently number of tickets of the job
     *  @return currently number of tickets
     */
    public long getTickets();
    
    /**
     * Get the override portion of the total number of tickets
     * assigned to the job currently
     * @return override portion of the total number of tickets
     */
    public long getOtickets();
    
    /**
     *  Get the functional portion of the total number of tickets
     *  assigned to the job currently.
     *
     *  @return the functional portion of the total number of tickets
     */
    public long getFtickets();
    
    /**
     * Get the share portion of the total number of tickets
     * assigned to the job currently.
     *
     * @return the share portion of the total number of tickets
     */
    public long getStickets();
    
    /**
     * Get the share of the total system to which the job is entitled currently.
     * @return the share of the job
     */
    public double getShare();
    
    /**
     *  Get the number of used slots
     *  @return the number of used slots
     */
    public int getSlots();
    
    /**
     *  Set the number of used slots
     *  @param slots the number of used slots
     */
    public void setSlots(int slots);
    
    /**
     *  Determine if the job is an array job
     *  @return <code>true</code> if the job is an array job
     */
    public boolean isArray();
    
    
    /**
     *  Determine if the job is running
     *  @return <code>true</code> of the job is running
     */
    public boolean isRunning();
    
    /**
     *  Get the name of the parallel environment of the job
     *  @return name of the parallel environment
     */
    public String getParallelEnvironmentName();
    
    /**
     * Get the requested PE slot range.
     * @return the requested PE slot range
     */
    public String getParallelEnvironmentRange();
    
    /**
     *   Get the name of the granted PE
     *   @return name of the granted PE
     */
    public String getGrantedPEName();
    
    
    /**
     *  Get the number of granted PE slots
     *  @return number of granted PE slots
     */
    public int getGrantedPESlots();
    
    /**
     *  Get the checkpoint environment of the job
     *  @return the checkpoint environment of the job
     */
    public String getCheckpointEnv();
    
}
