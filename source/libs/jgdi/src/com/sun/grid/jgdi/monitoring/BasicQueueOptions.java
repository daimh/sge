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

import com.sun.grid.jgdi.monitoring.filter.*;
import java.io.Serializable;

/**
 * Base class for all classes which implements options for the qstat
 * algorithm.
 */
public abstract class BasicQueueOptions implements Serializable {
    
    /* qstat -ext */
    private boolean showAdditionalAttributes;
    /* qstat -q */
    private QueueFilter queueFilter;
    /* qstat -l */
    private ResourceFilter resourceFilter;
    /* qstat -qs */
    private QueueStateFilter queueStateFilter;
    /* qstat -U */
    private UserFilter queueUserFilter;
    
    
    /**
     * Creates a new instance of BaseQStatFilter
     */
    public BasicQueueOptions() {
    }
    
    /**
     *   Get the queue filter
     *
     *   The CLI qstat equivialent for queue filter is <code>qstat -q</code>
     *
     *   @return the queue filters
     */
    public QueueFilter getQueueFilter() {
        return queueFilter;
    }
    
    /**
     *  Set the queue filter
     *  @param queueFilter the new queue filter
     */
    public void setQueueFilter(QueueFilter queueFilter) {
        this.queueFilter = queueFilter;
    }
    
    /**
     *  Update the content of the queue user filter by adding values in vals
     *  @param vals values to be added to the filter
     */
    public void updateQueueFilter(String vals) {
        if (queueFilter == null) {
            queueFilter = QueueFilter.parse(vals);
        } else {
            queueFilter.fill(vals);
        }
    }
    
    /**
     * Get the the resource filter.
     *
     * The CLI equivialent for resource fitler is <code>qstat -l</code>
     */
    public ResourceFilter getResourceFilter() {
        return resourceFilter;
    }
    
    /**
     *  Set the resource filter
     *
     *  @param resourceFilter the new resource filter
     */
    public void setResourceFilter(ResourceFilter resourceFilter) {
        this.resourceFilter = resourceFilter;
    }
    
    /**
     *  Update the content of the resource filter by adding values in vals
     *  @param vals values to be added to the filter
     */
    public void updateResourceFilter(String vals) {
        if (resourceFilter == null) {
            resourceFilter = ResourceFilter.parse(vals);
        } else {
            resourceFilter.fill(vals);
        }
    }
    
    /**
     *  Get the queue state filter.
     *
     *  The CLI equivialent for queue state filter is <code>qstat -qs</code>
     *  @return the queue state filter
     */
    public QueueStateFilter getQueueStateFilter() {
        return queueStateFilter;
    }
    
    /**
     *  Set the queue state filter.
     *
     *  @param queueStateFilter the queue state filter
     */
    public void setQueueStateFilter(QueueStateFilter queueStateFilter) {
        this.queueStateFilter = queueStateFilter;
    }
    
    /**
     *  Update the content of the queue state filter by adding values in vals
     *  @param vals values to be added to the filter
     */
    public void updateQueueStateFilter(String vals) {
        if (queueStateFilter == null) {
            queueStateFilter = QueueStateFilter.parse(vals);
        } else {
            queueStateFilter.fill(vals);
        }
    }
    
    /**
     *  Get the queue user filter. The result of the qstat algorithm will only
     *  include those queue which are accessible by user of the queue user filter.
     *
     *  The CLI equivialent for the queue user filter is <code>qstat -U</code>
     *  @return the queue user filter
     */
    public UserFilter getQueueUserFilter() {
        return queueUserFilter;
    }
    
    /**
     *  Set a new queue user filter
     *
     *  @param queueUserFilter the new queue user filter
     */
    public void setQueueUserFilter(UserFilter queueUserFilter) {
        this.queueUserFilter = queueUserFilter;
    }
    
    /**
     *  Update the content of the queue user filter by adding values in vals
     *  @param vals values to be added to the filter
     */
    public void updateQueueUserFilter(String vals) {
        if (queueUserFilter == null) {
            queueUserFilter = UserFilter.parse(vals);
        } else {
            queueUserFilter.fill(vals);
        }
    }
    
    /**
     *  Should additional attributes be includes into the result
     *
     *  The CLI equivialent is <code>qstat -ext</code>
     *
     *  @return <code>true</code> of additional attributes should be included
     *          into the result
     */
    public boolean showAdditionalAttributes() {
        return showAdditionalAttributes;
    }
    
    /**
     *  Set the show additional attributes flag
     *  @param showAdditionalAttributes the show additional attributes flag
     */
    public void setShowAdditionalAttributes(boolean showAdditionalAttributes) {
        this.showAdditionalAttributes = showAdditionalAttributes;
    }
    
}
