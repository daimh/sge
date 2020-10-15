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
import java.util.List;

/**
 * Objects of this class holds the monitoring information about
 * a cluster limit rule.
 *
 */
public class ResourceQuotaRuleInfoImpl implements ResourceQuotaRuleInfo, Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private String resourceQuotaRuleName;
    private List<String> users = new ArrayList<String>();
    private List<String> pes = new ArrayList<String>();
    private List<String> hosts = new ArrayList<String>();
    private List<String> queues = new ArrayList<String>();
    private List<String> projects = new ArrayList<String>();
    private List<String> xusers = new ArrayList<String>();
    private List<String> xpes = new ArrayList<String>();
    private List<String> xhosts = new ArrayList<String>();
    private List<String> xqueues = new ArrayList<String>();
    private List<String> xprojects = new ArrayList<String>();
    
    private List<ResourceQuota> resourceLimits = new ArrayList<ResourceQuota>();
    
    /**
     * Create a new limit rule info object
     */
    public ResourceQuotaRuleInfoImpl() {}
    
    /**
     * Create a new limit rule info object
     *
     * @param resourceQuotaRuleName  name of the limit rule
     */
    public ResourceQuotaRuleInfoImpl(String resourceQuotaRuleName) {
        setresourceQuotaRuleName(resourceQuotaRuleName);
    }
    
    /**
     *  Get the limit rule name
     *
     *  @return the limit rule name
     */
    public String getResouceQuotaRuleName() {
        return this.resourceQuotaRuleName;
    }
    
    /**
     *  Set the limit rule name
     *
     * @param resourceQuotaRuleName the limit rule name
     */
    public void setresourceQuotaRuleName(String resourceQuotaRuleName) {
        this.resourceQuotaRuleName = resourceQuotaRuleName;
    }
    
    
    /**
     *  Get users of limit rule
     *
     *  @return list of user names
     */
    public List<String> getUsers() {
        return Collections.unmodifiableList(users);
    }
    
    /**
     *  Add user to limit rule users
     *
     *  @param user the user
     */
    public void addUser(String user) {
        users.add(user);
    }
    
    /**
     *  Get projects of limit rule
     *
     *  @return list of project names
     */
    public List<String> getProjects() {
        return Collections.unmodifiableList(projects);
    }
    
    /**
     *  Add project to limit rule projects
     *
     *  @param project the project
     */
    public void addProject(String project) {
        projects.add(project);
    }
    
    
    /**
     *  Get PEs of limit rule
     *
     *  @return list of PE names
     */
    public List<String> getPes() {
        return Collections.unmodifiableList(pes);
    }
    
    /**
     *  Add pe to limit rule pes
     *
     *  @param pe the pe
     */
    public void addPe(String pe) {
        pes.add(pe);
    }
    
    
    /**
     *  Get Queues of limit rule
     *
     *  @return list of queue names
     */
    public List<String> getQueues() {
        return Collections.unmodifiableList(queues);
    }
    
    /**
     *  Add queue to limit rule queues
     *
     *  @param queue the queue
     */
    public void addQueue(String queue) {
        queues.add(queue);
    }
    
    
    /**
     *  Get hosts of limit rule
     *
     *  @return list of host names
     */
    public List<String> getHosts() {
        return Collections.unmodifiableList(hosts);
    }
    
    /**
     *  Add host to limit rule hosts
     *
     *  @param host the host
     */
    public void addHost(String host) {
        hosts.add(host);
    }
    
    /**
     *  Get excluded users of limit rule
     *
     *  @return list of excluded user names
     */
    
    public List<String> getXUsers() {
        return Collections.unmodifiableList(xusers);
    }
    
    /**
     *  Add user to limit rule excluded users
     *
     *  @param user the user
     */
    public void addXUser(String user) {
        xusers.add(user);
    }
    
    /**
     *  Get excluded projects of limit rule
     *
     *  @return list of excluded project names
     */
    public List<String> getXProjects() {
        return Collections.unmodifiableList(xprojects);
    }
    
    /**
     *  Add project to limit rule excluded projects
     *
     *  @param project the project
     */
    public void addXProject(String project) {
        xprojects.add(project);
    }
    
    
    /**
     *  Get excluded PEs of limit rule
     *
     *  @return list of excluded PE names
     */
    public List<String> getXPes() {
        return Collections.unmodifiableList(xpes);
    }
    
    /**
     *  Add pe to limit rule excluded pes
     *
     *  @param pe the pe
     */
    public void addXPe(String pe) {
        xpes.add(pe);
    }
    
    
    /**
     *  Get excluded Queues of limit rule
     *
     *  @return list of excluded queue names
     */
    public List<String> getXQueues() {
        return Collections.unmodifiableList(xqueues);
    }
    
    /**
     *  Add queue to limit rule excluded queues
     *
     *  @param queue the queue
     */
    public void addXQueue(String queue) {
        xqueues.add(queue);
    }
    
    
    /**
     *  Get excluded hosts of limit rule
     *
     *  @return list of excluded host names
     */
    public List<String> getXHosts() {
        return Collections.unmodifiableList(xhosts);
    }
    
    /**
     *  Add host to limit rule excluded hosts
     *
     *  @param host the host
     */
    public void addXHost(String host) {
        xhosts.add(host);
    }
    
    
    /**
     *  Get resource limits of limit rule
     *
     * @return list of ResourceLimit objects
     */
    public List<ResourceQuota> getLimits() {
        return Collections.unmodifiableList(resourceLimits);
    }
    
    /**
     *  Add resourceLimit to limit rule resourceLimits
     *
     *  @param resourceLimit the resourceLimit
     */
    public void addLimit(ResourceQuota resourceLimit) {
        resourceLimits.add(resourceLimit);
    }
    
    
}
