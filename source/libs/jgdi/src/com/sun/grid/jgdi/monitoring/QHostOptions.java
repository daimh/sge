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
import com.sun.grid.jgdi.monitoring.filter.ResourceFilter;
import com.sun.grid.jgdi.monitoring.filter.UserFilter;
import com.sun.grid.jgdi.monitoring.filter.ResourceAttributeFilter;
import com.sun.grid.jgdi.monitoring.filter.HostFilter;

/**
 * Options for the qhost algorithm
 *
 * @jgdi.todo    beta ??
 *          <p>Add javadoc comments</p>
 */
public class QHostOptions implements Serializable {
    
    private final static long serialVersionUID = -2009040301L;

    private boolean includeJobs;
    private boolean includeQueue;
    private boolean showAsXML;
    
    /**
     *  If the hostValueFilter is not set the standard values will be included
     */
    private ResourceAttributeFilter resourceAttributeFilter;
    private HostFilter hostFilter;
    private UserFilter userFilter;
    private ResourceFilter resourceFilter;
    
    /**
     *  Determine if job info should be included into the <code>QHostResult</code>
     *  @see QHostResult#getHostInfo
     *  @see HostInfo#getJobList
     */
    public boolean includeJobs() {
        return includeJobs;
    }
    
    /**
     *  Set the include jobs flag
     *  @param includeJobs the include jobs flag
     */
    public void setIncludeJobs(boolean includeJobs) {
        this.includeJobs = includeJobs;
    }
    
    /**
     *  <p>Determine of the queue values should be included.</p>
     *  <p>The CLI equivialent for this options is <code>qhost -q</code></p>
     *  @return <code>true</code> of queue values should be included
     */
    public boolean includeQueue() {
        return includeQueue;
    }
    
    /**
     *  Set the include queue flag
     *  @param includeQueue the include queue flag
     */
    public void setIncludeQueue(boolean includeQueue) {
        this.includeQueue = includeQueue;
    }
    
    /**
     *  <p>Determine if to show the result as an xml</p>
     *  <p>The CLI equivialent for this options is <code>qhost -xml</code></p>
     *  @return boolean
     */
    public boolean showAsXML() {
        return showAsXML;
    }
    
    /**
     *  Set the showAsXML flag
     *  @param showAsXML the flag
     */
    public void setShowAsXML(boolean showAsXML) {
        this.showAsXML = showAsXML;
    }
    
    /**
     *  <p>Get the host filter.</p>
     *  <p>The CLI equivalent of the host filter is <code>qhost -h</code>
     *     (see man qhost(1)).</p>
     *  @return the host filter
     */
    public HostFilter getHostFilter() {
        return hostFilter;
    }
    
    /**
     *  Set the host filter.
     *  @param hostFilter the host filter
     */
    public void setHostFilter(HostFilter hostFilter) {
        this.hostFilter = hostFilter;
    }
    
    /**
     *  <p>Get the user filter.</p>
     *  <p>The CLI equivialent of the user filter is <code>qhost -u</code>
     *     (see man qhost(1))</p>
     *  @return the user filter
     */
    public UserFilter getUserFilter() {
        return userFilter;
    }
    
    /**
     *  Set the user filter
     *  @param userFilter the user filter
     */
    public void setUserFilter(UserFilter userFilter) {
        this.userFilter = userFilter;
    }
    
    /**
     *  <p>Get the resource filter</p>
     *  <p>The CLI equivialent for the resource filter is <code>qhost -l</code>
     *     (see man qhost(1))</p>
     *  @return the resource filter
     */
    public ResourceFilter getResourceFilter() {
        return resourceFilter;
    }
    
    /**
     *  Set the resource filter.
     *
     *  @param  resourceFilter the resource filter
     */
    public void setResourceFilter(ResourceFilter resourceFilter) {
        this.resourceFilter = resourceFilter;
    }
    
    /**
     *  <p>Get the resource attribute filter.</p>
     *  <p>The CLI equivialent of the resource attribute filter is
     *     <code>qhost -F</code> (see man qhost(1)).</p>
     *  @return the resource attribute filter
     */
    public ResourceAttributeFilter getResourceAttributeFilter() {
        return resourceAttributeFilter;
    }
    
    /**
     *  Set the resource attribute filter
     *  @param resourceAttributeFilter the resource attribute filter
     */
    public void setResourceAttributeFilter(ResourceAttributeFilter resourceAttributeFilter) {
        this.resourceAttributeFilter = resourceAttributeFilter;
    }
    
}
