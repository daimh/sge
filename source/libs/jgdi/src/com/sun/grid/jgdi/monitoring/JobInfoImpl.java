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
import java.util.Date;

/**
 * Default implemenation of {@link JobInfo}
 */
public class JobInfoImpl implements JobInfo, Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private int id;
    private String taskId;
    private double priority;
    private String name;
    private String user;
    private String state;
    private String queue;
    private String qinstance;
    private String masterQueue;
    private Date submitTime;
    private Date startTime;
    
    
    /** Creates a new instance of JobInfo */
    public JobInfoImpl() {
    }
    
    /**
     *   Get the id of the job
     *   @return id of the job
     */
    public int getId() {
        return id;
    }
    
    /**
     *   Set the id of the job
     *   @param id  id of the job
     */
    public void setId(int id) {
        this.id = id;
    }
    
    /**
     *  Get the task id of the job
     *  @return task id of the job
     */
    public String getTaskId() {
        return taskId;
    }
    
    /**
     *  Set the task id of the job
     *  @param taskId the task id of the job
     */
    public void setTaskId(String taskId) {
        this.taskId = taskId;
    }
    
    /**
     *  Get the priority of the job
     *  @return the priority of the job
     */
    public double getPriority() {
        return priority;
    }
    
    /**
     *  Set the priority of the job
     *  @param priority the priority of the job
     */
    public void setPriority(double priority) {
        this.priority = priority;
    }
    
    /**
     *  Get the name of the job
     *  @return the name of the job
     */
    public String getName() {
        return name;
    }
    
    /**
     *  Set the name of the job
     *  @param name the name of the job
     */
    public void setName(String name) {
        this.name = name;
    }
    
    /**
     *  Get the owner of the job
     *  @return user of the of the job
     */
    public String getUser() {
        return user;
    }
    
    /**
     *  Set the owner of the job
     *  @param user owner of the job
     */
    public void setUser(String user) {
        this.user = user;
    }
    
    /**
     *  Get the state of the job
     *  @return state of the job
     */
    public String getState() {
        return state;
    }
    
    /**
     *  Set the state of the job
     */
    public void setState(String state) {
        this.state = state;
    }
    
    /**
     *  Get the queue of the job
     *  @return the queue
     */
    public String getQueue() {
        return queue;
    }
    
    /**
     *  Set the queue of the job
     *  @param queue the queue of the job
     */
    public void setQueue(String queue) {
        this.queue = queue;
    }
    /**
     *  Get the queue instance name of the job
     *  @return the queue instance name
     */
    public String getQinstanceName() {
        return qinstance;
    }
    
    /**
     *  Set the queue instance name of the job
     *  @param qinstance the queue instance name of the job
     */
    public void setQinstanceName(String qinstance) {
        this.qinstance = qinstance;
    }
    
    
    /**
     *   Get the name of the master queue of the job.
     *
     *   @return name of the master queue
     */
    public String getMasterQueue() {
        return masterQueue;
    }
    
    /**
     *   Set the name of the master queue of the job
     *   @param  qname of the master queue
     */
    public void setMasterQueue(String qname) {
        this.masterQueue = qname;
    }
    
    /**
     *  Get the submit time of the job
     *  @return the submit time
     */
    public Date getSubmitTime() {
        return submitTime;
    }
    
    /**
     *  Set the submit time of the job
     *  @param submitTime the submit time
     */
    public void setSubmitTime(Date submitTime) {
        this.submitTime = submitTime;
    }
    
    /**
     *  Set the submit time of the job
     *  @param submitTime  submit time in millis
     */
    public void setSubmitTime(long submitTime) {
        this.submitTime = new Date(submitTime);
    }
    
    /**
     *  Get the start time of the job
     *  @return the start time
     */
    public Date getStartTime() {
        return startTime;
    }
    
    /**
     *  Set the start time of the job
     *  @param startTime the submit time
     */
    public void setStartTime(Date startTime) {
        this.startTime = startTime;
    }
    
    /**
     *  Set the start time of the job
     *  @param startTime  start time in millis
     */
    public void setStartTime(long startTime) {
        this.startTime = new Date(startTime);
    }
}
