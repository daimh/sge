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

/**
 * Instances of this class hold the information for a task of a job.
 */
public interface TaskSummary {
    
    /**
     *  Get the task id.
     *  @return the task id
     */
    public String getTaskId();
    
    /**
     *  Get the state of the task
     *  @return state of the task
     */
    public String getState();
    
    /**
     *  Determine if the task has cpu usage
     *  @return <code>true</code> if the task has cpu usage
     */
    public boolean hasCpuUsage();
    
    /**
     *  Get the cpu usage of the task. Returns only a meanful value
     *  if {@link #hasCpuUsage} returns true.
     *  @return the cpu usage of the task
     */
    public double getCpuUsage();
    
    /**
     *  Determine if the task has mem usage
     *  @return <code>true</code> if the task has mem usage
     */
    public boolean hasMemUsage();
    
    /**
     *  Get the mem usage of the task. Returns only a meanful value
     *  if {@link #hasMemUsage} returns true.
     *  @return the mem usage of the task
     */
    public double getMemUsage();
    
    /**
     *  Determine if the task has io usage
     *  @return <code>true</code> if the task has io usage
     */
    public boolean hasIoUsage();
    
    /**
     *  Get the io usage of the task. Returns only a meanful value
     *  if {@link #hasIoUsage} returns true.
     *  @return the io usage of the task
     */
    public double getIoUsage();
    
    /**
     * Determine if the task is running
     * @return <code>true</code> if task is running
     */
    public boolean isRunning();
    
    /**
     *  Determine if the task has an exit status.
     *  @return <code>true</code> if the task has an exit status
     */
    public boolean hasExitStatus();
    
    /**
     *  Get the exit status of the task. Returns only a meanful value if
     *  {@link #hasExitStatus} returns <code>true</code>.
     *  @return the exit status of the task
     */
    public int getExitStatus();
    
}
