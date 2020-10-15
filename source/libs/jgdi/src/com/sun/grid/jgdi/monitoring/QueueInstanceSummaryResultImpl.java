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
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
/**
 *
 */
public class QueueInstanceSummaryResultImpl implements QueueInstanceSummaryResult, Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private List<QueueInstanceSummary> queueInstanceSummaryList = new ArrayList<QueueInstanceSummary>();
    private List<JobSummary> unassignedJobSummaryList = new ArrayList<JobSummary>();
    private List<JobSummary> pendingJobList = new ArrayList<JobSummary>();
    private List<JobSummary> errorJobList = new ArrayList<JobSummary>();
    private List<JobSummary> zombieJobList = new ArrayList<JobSummary>();
    private List<JobSummary> finishedJobList = new ArrayList<JobSummary>();
    
    /** Creates a new instance of QueueInstanceSummaryResultImpl */
    public QueueInstanceSummaryResultImpl() {
    }
    
    public List<QueueInstanceSummary> getQueueInstanceSummary() {
        return Collections.unmodifiableList(queueInstanceSummaryList);
    }
    
    public void addQueueInstanceSummary(QueueInstanceSummary qs) {
        queueInstanceSummaryList.add(qs);
    }
    
    public List<JobSummary> getPendingJobs() {
        return Collections.unmodifiableList(pendingJobList);
    }
    
    public void addPendingJobs(List<JobSummary> jobs) {
        pendingJobList.addAll(jobs);
    }
    
    public List<JobSummary> getErrorJobs() {
        return Collections.unmodifiableList(errorJobList);
    }
    
    public void addErrorJobs(List<JobSummary> jobs) {
        errorJobList.addAll(jobs);
    }
    
    public List<JobSummary> getFinishedJobs() {
        return Collections.unmodifiableList(finishedJobList);
    }
    
    public void addFinishedJobs(List<JobSummary> jobs) {
        finishedJobList.addAll(jobs);
    }
    
    public List<JobSummary> getZombieJobs() {
        return Collections.unmodifiableList(zombieJobList);
    }
    
    public void addZombieJobs(List<JobSummary> jobs) {
        zombieJobList.addAll(jobs);
    }
    
}
