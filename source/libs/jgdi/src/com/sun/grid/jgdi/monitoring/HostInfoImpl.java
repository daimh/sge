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
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Objects of this class holds the monitoring information about
 * a cluster host.
 *
 */
public class HostInfoImpl implements HostInfo, Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private String hostname;
    private Map<String, Object> hostValueMap = new HashMap<String, Object>();
    private Map<String, Map<String, Object>> resourceMap = new HashMap<String, Map<String, Object>>();
    private List<JobInfo> jobList = new ArrayList<JobInfo>();
    private List<QueueInfo>  queueList = new ArrayList<QueueInfo>();
    
    /**
     * Create a new host info object
     */
    public HostInfoImpl() {}
    
    /**
     * Create a new host info object
     * @param hostname  name of the host
     */
    public HostInfoImpl(String hostname) {
        setHostname(hostname);
    }
    
    
    /**
     * Get a host value
     * @param name name of the host value
     * @return  the host value or <code>null</code>
     */
    public Object getHostValue(String name) {
        return hostValueMap.get(name);
    }
    
    /**
     *  Get the architecture of the host
     *  @return the architecture of the host
     */
    public String getArch() {
        return (String)getHostValue("arch_string");
    }
    
    /**
     *  Get the number of processors of the host
     *  @return number of processors of the host
     */
    public String getNumberOfProcessors() {
        return (String)getHostValue("num_proc");
    }
    
    
    /**
     *  Get the load average of the host
     *  @return the load average of the host
     */
    public String getLoadAvg() {
        return (String)getHostValue("load_avg");
    }
    
    /**
     *  Get the total memory of the host
     *  @return total memory of the host
     */
    public String getMemTotal() {
        return (String)getHostValue("mem_total");
    }
    
    /**
     *  Get the used memory of the host
     *  @return the used memory
     */
    public String getMemUsed() {
        return (String)getHostValue("mem_used");
    }
    
    /**
     *  Get the size of the swap of the host
     *  @return size of the swap of the host
     */
    public String getSwapTotal() {
        return (String)getHostValue("swap_total");
    }
    
    /**
     *  Get the size of used swap space
     *  @return size of the used swap space
     */
    public String getSwapUsed() {
        return (String)getHostValue("swap_used");
    }
    
    /**
     *  Get the number of host values
     *  @return number of host values
     */
    public int getHostValueCount() {
        return hostValueMap.values().size();
    }
    
    /**
     * Add a host value
     * @param name   name of the host value
     * @param value  value
     */
    public void putHostValue(String name, Object value) {
        hostValueMap.put(name, value);
    }
    
    /**
     *  Get the set of host value names
     *
     *  @return set of host value names
     */
    public Set<String> getHostValueKeys() {
        return hostValueMap.keySet();
    }
    
    /**
     * remove a host value
     * @param name   name of the host value
     */
    public void removeHostValue(String name) {
        hostValueMap.remove(name);
    }
    
    /**
     * remove all host value
     */
    public void removeAllHostValue() {
        hostValueMap.clear();
    }
    
    
    
    /**
     * Get a resource value of the host
     * @param dominance  dominace of the resource value
     * @param name       name of the resource
     * @return  the resource value
     */
    public Object getResourceValue(String dominance, String name) {
        Map dominanceMap = (Map)resourceMap.get(dominance);
        if(dominanceMap != null) {
            return dominanceMap.get(name);
        }
        return null;
    }
    
    
    /**
     * add a new resoruce value
     * @param dominance  dominance of the resource value
     * @param name   name of the resource
     * @param value  value
     */
    public void putResourceValue(String dominance, String name, Object value) {
        Map<String, Object> dominanceMap = resourceMap.get(dominance);
        if(dominanceMap == null) {
            dominanceMap = new HashMap<String, Object>();
            resourceMap.put(dominance, dominanceMap);
        }
        dominanceMap.put(name, value);
    }
    
    
    
    /**
     * Get the set of available domincances
     * @return  the set of available domincances
     */
    public Set<String> getDominanceSet() {
        return resourceMap.keySet();
    }
    
    /**
     * Get the set of resource names for a dominance
     * @param dominance  the dominance
     * @return set of resource names
     */
    public Set<String> getResourceValueNames(String dominance) {
        Map<String, Object> dominanceMap = resourceMap.get(dominance);
        if (dominanceMap == null) {
            return Collections.EMPTY_SET;
        } else {
            return dominanceMap.keySet();
        }
    }
    
    /**
     *  Get the hostname
     *  @return the hostname
     */
    public String getHostname() {
        return hostname;
    }
    
    /**
     *  Set the hostname
     *
     *  @param hostname the hostname
     */
    public void setHostname(String hostname) {
        this.hostname = hostname;
    }
    
    /**
     *  Get the list of jobs which are running on the host
     *
     *  <p><b>Note:</b> The job list is only set if the
     *  {@link QHostOptions#includeJobs} flag is set.</p>
     *
     *  @return list of jobs (instances of {@link JobInfo})
     *  @see com.sun.grid.jgdi.JGDI#execQHost
     */
    public List<JobInfo> getJobList() {
        return Collections.unmodifiableList(jobList);
    }
    
    /**
     *  Get the number of entries in the job list
     *
     *  @return the number of entries in the job list
     */
    public int getJobCount() {
        return jobList.size();
    }
    
    /**
     *  Add a job to the job list:
     *
     *  @param  job  the job info object
     */
    public void addJob(JobInfo job) {
        jobList.add(job);
    }
    
    /**
     *  Get the list of queues which are available on the host
     *
     *  <p><b>Note:</b> The queue list is only set if the
     *  {@link QHostOptions#includeQueue} flag is set.</p>
     *
     *  @return list of queues (instances of {@link QueueInfo})
     *  @see com.sun.grid.jgdi.JGDI#execQHost
     */
    public List<QueueInfo> getQueueList() {
        return Collections.unmodifiableList(queueList);
    }
    
    /**
     *  Get the number of entries in the queue list
     *  @return the number of entries in the queue list
     */
    public int getQueueCount() {
        return queueList.size();
    }
    
    /**
     *  Add a QueueInfo object to the queue list:
     *  @param  queue  the queue info object
     */
    public void addQueue(QueueInfo queue) {
        queueList.add(queue);
    }
    
}
