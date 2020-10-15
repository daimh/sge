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

/**
 * Holds the information about a job
 */
public interface JobInfo {
    
    /**
     *   Get the id of the job
     *   @return id of the job
     */
    public int getId();
    
    /**
     *  Get the task id of the job
     *  @return task id of the job
     */
    public String getTaskId();
    
    
    /**
     *  Get the priority of the job
     *  @return the priority of the job
     */
    public double getPriority();
    
    /**
     *  Get the name of the job
     *  @return the name of the job
     */
    public String getName();
    
    /**
     *  Get the owner of the job
     *  @return user of the of the job
     */
    public String getUser();
    
    
    /**
     *  Get the state of the job
     *  @return state of the job
     */
    public String getState();
    
    /**
     *  Get the queue of the job
     *  @return the queue
     */
    public String getQueue();
    
    /**
     *  Get the queue instance name of the job
     *  @return the queue instance name
     */
    public String getQinstanceName();
    
    /**
     *   Get the name of the master queue of the job.
     *
     *   @return name of the master queue
     */
    public String getMasterQueue();
    
    /**
     *  Get the submit time of the job
     *  @return the submit time
     */
    public Date getSubmitTime();
    
    /**
     *  Get the start time of the job
     *  @return the start time
     */
    public Date getStartTime();
    
}
