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

import com.sun.grid.jgdi.monitoring.filter.ResourceAttributeFilter;
import com.sun.grid.jgdi.monitoring.filter.JobStateFilter;
import com.sun.grid.jgdi.monitoring.filter.ParallelEnvironmentFilter;
import com.sun.grid.jgdi.monitoring.filter.UserFilter;
import java.io.Serializable;

/**
 * Options for the QueueInstanceSummary algorithm
 * @see com.sun.grid.jgdi.JGDI#getQueueInstanceSummary
 */
public class QueueInstanceSummaryOptions extends BasicQueueOptions implements Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    /* qstat -F */
    private ResourceAttributeFilter resourceAttributeFilter;
    
    /* qstat -pe pe_name,... */
    private ParallelEnvironmentFilter peFilter;
    
    /* qstat -s */
    private JobStateFilter jobStateFilter;
    
    /* qstat -u */
    private UserFilter jobUserFilter;
    
    /* qstat -f */
    private boolean showFullOutput = false;
    
    /* qstat -ne */
    private boolean showEmptyQueues = true;
    
    /* qstat -r */
    private boolean showRequestedResourcesForJobs = false;
    
    /* qstat -g d */
    private boolean showArrayJobs = false;
    
    /* qstat -g t */
    private boolean showPEJobs = false;
    
    /* qstat -pri */
    private boolean showJobPriorities = false;
    
    /* qstat -urg */
    private boolean showJobUrgency = false;
    
    
    /* qstat -t */
    private boolean showExtendedSubTaskInfo = false;
    
    /* -explain a|A|c|E */
    private char explain;
    private boolean isExplainSet = false;
    
    
    /** Creates a new instance of QueueInstanceSummaryOptions */
    public QueueInstanceSummaryOptions() {
    }
    
    /**
     *  <p>Get the resource attribute filter.</p>
     *  <p>The CLI equivialent for the resource attribute filter is qstat -F
     *     (see man qstat(1)).</p>
     */
    public ResourceAttributeFilter getResourceAttributeFilter() {
        return resourceAttributeFilter;
    }
    
    /**
     *  Set the resource attribute filter.
     *
     *  @param resourceAttributeFilter the resource attribute filter
     */
    public void setResourceAttributeFilter(ResourceAttributeFilter resourceAttributeFilter) {
        this.resourceAttributeFilter = resourceAttributeFilter;
    }
    
    /**
     *  Update the content of the resource attribute filter by adding values in vals
     *  @param vals values to be added to the filter
     */
    public void updateResourceAttributeFilter(String vals) {
        if (resourceAttributeFilter == null) {
            resourceAttributeFilter = ResourceAttributeFilter.parse(vals);
        } else {
            resourceAttributeFilter.fill(vals);
        }
    }
    
    /**
     * <p>Determine if the explain flag is set.</p>
     *
     * <p>If the explain flag is set, the {@link QueueInstanceSummary} will include
     * additional information for each job related to the job ticket policy scheme.</p>
     *
     * <p>The CLI equivialent for the explain flag is the <code>qstat -explain</code> option
     *    (see man qstat(1)).</p>
     *
     * @return <code>true</code> if the explain set is set
     */
    public boolean isExplainSet() {
        return isExplainSet;
    }
    
    /**
     *  <p>Get the explain option.</p>
     *
     * <p><b>Possible values:</b></p>
     * <ul>
     *   <li> <b>'c'</b> displays the reason for the configuration ambiguous state of a queue instance.</li>
     *   <li> <b>'a'</b> shows the reason for the alarm  state.
     *   <li> Suspend  alarm  state  reasons  will  be displayed  by <b>'A'</b>.</li>
     *   <li> <b>'E'</b> displays the reason for a queue instance error state.</li>
     * </ul>
     *
     * <p>The CLI equivialent for the explain option is <code>qstat -explain</code>
     *    (see man qstat(1))</p>
     */
    public char getExplain() {
        return explain;
    }
    
    /**
     * Set the explain character.
     * @param explain the explain character.
     * @throws IllegalArgumentException if explain is not 'a', 'A', 'c' or 'E'
     * @see #getExplain
     */
    public void setExplain(char explain) {
        switch(explain) {
            case 'a':
            case 'A':
            case 'c':
            case 'E':
                this.explain = explain;
                this.isExplainSet = true;
                break;
            default:
                throw new IllegalArgumentException("explain must be a|A|c|E");
        }
    }
    
    /**
     * <p>The <code>showFullOutput</code> option causes summary information on all queues to
     * be returned along with the queued job list.</p>
     * <p>The <code>showFullOutput</code> option is the equivalent to the
     * <code>qstat -f</code> options</p>
     *
     * @return the showFullOutput flag
     */
    public boolean showFullOutput() {
        return showFullOutput;
    }
    
    /**
     * Set the <code>showFullOutput</code> option
     * @param showFullOutput new value for the <code>showFullOutput</code> option
     * @see #showFullOutput
     */
    public void setShowFullOutput(boolean showFullOutput) {
        this.showFullOutput = showFullOutput;
    }
    
    /**
     * If the <code>showArrayJobs</code> option is set, array jobs are displayed verbosely.
     * By default, array jobs are grouped and all tasks with the same status (for pending
     * tasks  only)  are included in a single object. The array job task id range field
     * in the output specifies the corresponding set of tasks.
     * The <code>showArrayJobs</code> option is the equivalent to the <code>qstat -g d</code>
     * option.
     * @return <code>showArrayJobs</code> option
     */
    public boolean showArrayJobs() {
        return showArrayJobs;
    }
    
    /**
     * Set the <code>showArrayJobs</code> option
     * @param showArrayJobs the new value for the <code>showArrayJobs</code> option
     * @see #showArrayJobs()
     */
    public void setShowArrayJobs(boolean showArrayJobs) {
        this.showArrayJobs = showArrayJobs;
        //Only -g d or -g t is valid
        this.showPEJobs = false;
    }
    
    /**
     * <p>With <code>showPEJobs</code> option detailed information about parallel jobs
     * is included into the result.</p>
     * <p>The <code>showPEJobs</code> option is the equivalent to the <code>qstat -g t</code>
     * option.</p>
     * @return the <code>showPEJobs</code> option
     */
    public boolean showPEJobs() {
        return showPEJobs;
    }
    
    /**
     * set the  <code>showPEJobs</code> option
     * @param showPEJobs new value for the <code>showPEJobs</code> option
     * @see #showPEJobs
     */
    public void setShowPEJobs(boolean showPEJobs) {
        this.showPEJobs = showPEJobs;
        //Only -g d or -g t is valid
        this.showArrayJobs = false;
    }
    
    /**
     * <p>Equivalent to the <code>qstat -pe</code> option.</p>
     * @return the pe filter
     */
    public ParallelEnvironmentFilter getPeFilter() {
        return peFilter;
    }
    
    /**
     * set the pe filter
     * @param peFilter the peFilter
     */
    public void setPeFilter(ParallelEnvironmentFilter peFilter) {
        this.peFilter = peFilter;
    }
    
    /**
     *  Update the content of the PE filter by adding values in vals
     *  @param vals values to be added to the filter
     */
    public void updatePeFilter(String vals) {
        if (peFilter == null) {
            peFilter = ParallelEnvironmentFilter.parse(vals);
        } else {
            peFilter.fill(vals);
        }
    }
    
    /**
     * <p>Equivalent to the <code>qstat -pri</code> option.</p>
     * @return the show priorities flag
     */
    public boolean showJobPriorities() {
        return showJobPriorities;
    }
    
    /**
     * set the <code>showJobPriorities</code> option
     * @param showJobPriorities  the <code>showJobPriorities</code> option
     */
    public void setShowJobPriorities(boolean showJobPriorities) {
        this.showJobPriorities = showJobPriorities;
    }
    
    /**
     *  <p>Get the job state filter</p>
     *  <p>The CLI equivialent for the job state filter is <code>qstat -s</code>
     *  return the job state filter
     */
    public JobStateFilter getJobStateFilter() {
        return jobStateFilter;
    }
    
    /**
     *  Set the job state filter
     *  @param jobStateFilter the job state filter
     *  @see #getJobStateFilter
     */
    public void setJobStateFilter(JobStateFilter jobStateFilter) {
        this.jobStateFilter = jobStateFilter;
    }
    
    /**
     *  <p>Should the extended sub task info be included into the QueueInstanceSummary</p>
     *  <p>The CLI equivialent for the extended sub task info is <code>qstat -t</code></p>
     *  @return <code>true</code> if the extended sub task info should be included
     */
    public boolean showExtendedSubTaskInfo() {
        return showExtendedSubTaskInfo;
    }
    
    /**
     *  Set the extended sub task info flag
     *  @param showExtendedSubTaskInfo the  extended sub task info flag
     *  @see #showExtendedSubTaskInfo
     */
    public void setShowExtendedSubTaskInfo(boolean showExtendedSubTaskInfo) {
        this.showExtendedSubTaskInfo = showExtendedSubTaskInfo;
    }
    
    /**
     *  <p>Get the job user filter. If set only those queues and jobs will be
     *     included into the {@link QueueInstanceSummary} which are assosiciated
     *     with the users in the <code>UserFilter</code>.</p>
     *
     *  <p>The CLI equivialent for the job user filter is <code>qstat -u</code></p>
     *
     *  @return the job user filter
     */
    public UserFilter getJobUserFilter() {
        return jobUserFilter;
    }
    
    /**
     *   Set the job user filter.
     *   @param jobUserFilter the job user filter
     *   @see #getJobUserFilter
     */
    public void setJobUserFilter(UserFilter jobUserFilter) {
        this.jobUserFilter = jobUserFilter;
    }
    
    /**
     *  Update the content of the job user filter by adding values in vals
     *  @param vals values to be added to the filter
     */
    public void updateJobUserFilter(String vals) {
        if (jobUserFilter == null) {
            jobUserFilter = UserFilter.parse(vals);
        } else {
            jobUserFilter.fill(vals);
        }
    }
    
    /**
     *  <p>Determine if the job urgency should be included into the {@link QueueInstanceSummary}.</p>
     *  <p>The CLI equivialent for the show job urgency flag is <code>qstat -urg</code></p>
     *  @return the show job urgency flag
     */
    public boolean showJobUrgency() {
        return showJobUrgency;
    }
    
    /**
     *  Set the job urgency flag
     *  @param showJobUrgency the job urgency flag
     *  @see #showJobUrgency
     */
    public void setShowJobUrgency(boolean showJobUrgency) {
        this.showJobUrgency = showJobUrgency;
    }
    
    /**
     *  <p>Should empty queues be include into the {@link QueueInstanceSummary}.</p>
     *  <p>The CLI equivialent for this options is <code>qstat -ne</code></p>
     *  @return the show empty queues flag
     */
    public boolean showEmptyQueues() {
        return showEmptyQueues;
    }
    
    /**
     * Set the show empty queues flag
     * @param showEmptyQueues the show empty queues flag
     * @see #showEmptyQueues
     */
    public void setShowEmptyQueues(boolean showEmptyQueues) {
        this.showEmptyQueues = showEmptyQueues;
    }
    
    /**
     *  <p>Should the requested resource for jobs be included into
     *     the {@link QueueInstanceSummary}.</p>
     *  <p>The CLI equivialent is <code>qstat -r</code>.</p>
     *  @return the requested resource for jobs flag
     */
    public boolean showRequestedResourcesForJobs() {
        return showRequestedResourcesForJobs;
    }
    
    /**
     *  Set the requested resource for jobs flag.
     *  @param showRequestedResourcesForJobs the requested resource for jobs flag
     *  @see #showRequestedResourcesForJobs
     */
    public void setShowRequestedResourcesForJobs(boolean showRequestedResourcesForJobs) {
        this.showRequestedResourcesForJobs = showRequestedResourcesForJobs;
    }
    
}
