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

/**
 * Default implementation of the <code>TaskSummary</code> interface
 */
public class TaskSummaryImpl implements TaskSummary, Serializable {
    
    private final static long serialVersionUID = -2009040301L;
    
    private String taskId;
    private String state;
    private boolean hasCpuUsage;
    private double cpuUsage;
    private boolean hasMemUsage;
    private double memUsage;
    private boolean hasIoUsage;
    private double ioUsage;
    private boolean isRunning;
    private boolean hasExitStatus;
    private int exitStatus;
    
    /** Creates a new instance of TaskSummary */
    public TaskSummaryImpl() {
    }
    
    /**
     *  Get the task id.
     *  @return the task id
     */
    public String getTaskId() {
        return taskId;
    }
    
    /**
     *  Set the task id
     *  @param taskId the task id
     */
    public void setTaskId(String taskId) {
        this.taskId = taskId;
    }
    
    /**
     *  Get the state of the task
     *  @return state of the task
     */
    public String getState() {
        return state;
    }
    
    /**
     *  Set the state of the task
     *  @param state state of the task
     */
    public void setState(String state) {
        this.state = state;
    }
    
    /**
     *  Determine if the task has cpu usage
     *  @return <code>true</code> if the task has cpu usage
     */
    public boolean hasCpuUsage() {
        return hasCpuUsage;
    }
    
    /**
     *  Get the cpu usage of the task. Returns only a meanful value
     *  if {@link #hasCpuUsage} returns true.
     *  @return the cpu usage of the task
     */
    public double getCpuUsage() {
        return cpuUsage;
    }
    
    /**
     *  Set the cpu usage of the task.
     *  @param cpuUsage the cpu usage
     */
    public void setCpuUsage(double cpuUsage) {
        this.cpuUsage = cpuUsage;
        hasCpuUsage = true;
    }
    
    /**
     *  Determine if the task has mem usage
     *  @return <code>true</code> if the task has mem usage
     */
    public boolean hasMemUsage() {
        return hasMemUsage;
    }
    
    /**
     *  Get the mem usage of the task. Returns only a meanful value
     *  if {@link #hasMemUsage} returns true.
     *  @return the mem usage of the task
     */
    public double getMemUsage() {
        return memUsage;
    }
    
    /**
     *  Set the mem usage of the task.
     *  @param memUsage the mem usage
     */
    public void setMemUsage(double memUsage) {
        this.memUsage = memUsage;
        hasMemUsage = true;
    }
    
    /**
     *  Determine if the task has io usage
     *  @return <code>true</code> if the task has io usage
     */
    public boolean hasIoUsage() {
        return hasIoUsage;
    }
    
    /**
     *  Get the io usage of the task. Returns only a meanful value
     *  if {@link #hasIoUsage} returns true.
     *  @return the io usage of the task
     */
    public double getIoUsage() {
        return ioUsage;
    }
    
    /**
     *  Set the io usage of the task.
     *  @param ioUsage the io usage
     */
    public void setIoUsage(double ioUsage) {
        this.ioUsage = ioUsage;
        hasIoUsage = true;
    }
    
    /**
     * Determine if the task is running
     * @return <code>true</code> if task is running
     */
    public boolean isRunning() {
        return isRunning;
    }
    
    /**
     * Set the running flag of the task
     * @param isRunning the running flag
     */
    public void setRunning(boolean isRunning) {
        this.isRunning = isRunning;
    }
    
    /**
     *  Determine if the task has an exit status.
     *  @return <code>true</code> if the task has an exit status
     */
    public boolean hasExitStatus() {
        return hasExitStatus;
    }
    
    /**
     *  Get the exit status of the task. Returns only a meanful value if
     *  {@link #hasExitStatus} returns <code>true</code>.
     *  @return the exit status of the task
     */
    public int getExitStatus() {
        return exitStatus;
    }
    
    /**
     *  Set the exit status of the task
     *  @param exitStatus the exit status
     */
    public void setExitStatus(int exitStatus) {
        this.exitStatus = exitStatus;
        hasExitStatus = true;
    }
    
}
