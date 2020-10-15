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

import com.sun.grid.jgdi.monitoring.filter.HostFilter;
import com.sun.grid.jgdi.monitoring.filter.ParallelEnvironmentFilter;
import com.sun.grid.jgdi.monitoring.filter.ProjectFilter;
import com.sun.grid.jgdi.monitoring.filter.QueueFilter;
import com.sun.grid.jgdi.monitoring.filter.ResourceFilter;
import com.sun.grid.jgdi.monitoring.filter.UserFilter;
import java.io.Serializable;

/**
 *
 * @jgdi.todo    beta ??
 *          <p>Add javadoc comments</p>
 */
public class QQuotaOptions implements Serializable {
    
    private final static long serialVersionUID = -2009040301L;
    
    /* -q option */
    private QueueFilter queueFilter;
    
    /* -h option */
    private HostFilter hostFilter;
    
    /* -P option */
    private ProjectFilter projectFilter;
    
    /* -pe option */
    private ParallelEnvironmentFilter peFilter;
    
    /* -l option */
    private ResourceFilter resourceFilter;
    
    /* -u option */
    private UserFilter userFilter;
    
    /** Creates a new instance of QQuotaOptions */
    public QQuotaOptions() {
    }
    
    public QueueFilter getQueueFilter() {
        return queueFilter;
    }
    
    public void setQueueFilter(QueueFilter queueFilter) {
        this.queueFilter = queueFilter;
    }
    
    public HostFilter getHostFilter() {
        return hostFilter;
    }
    
    public void setHostFilter(HostFilter hostFilter) {
        this.hostFilter = hostFilter;
    }
    
    public ProjectFilter getProjectFilter() {
        return projectFilter;
    }
    
    public void setProjectFilter(ProjectFilter projectFilter) {
        this.projectFilter = projectFilter;
    }
    
    public ParallelEnvironmentFilter getPeFilter() {
        return peFilter;
    }
    
    public void setPeFilter(ParallelEnvironmentFilter peFilter) {
        this.peFilter = peFilter;
    }
    
    public ResourceFilter getResourceFilter() {
        return resourceFilter;
    }
    
    public void setResourceFilter(ResourceFilter resourceFilter) {
        this.resourceFilter = resourceFilter;
    }
    
    public UserFilter getUserFilter() {
        return userFilter;
    }
    
    public void setUserFilter(UserFilter userFilter) {
        this.userFilter = userFilter;
    }
    
}
