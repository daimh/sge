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

import java.util.List;
import java.util.Set;

/**
 * Implementating classes of this interface represent the results of a
 * qstat command.
 *
 */
public interface QueueInstanceSummary {
    /**
     *  Get the name of the queue instance
     *  @return name of the queue instance
     */
    public String getName();
    
    /**
     *  Get the type of the queue
     *  @return type of the queue
     */
    public String getQueueType();
    
    /**
     *  Get the number of reserved slots.
     *  @return number of reserved slots
     */
    public int getReservedSlots();
    
    
    /**
     *  Get the number of used slots.
     *  @return number of used slots
     */
    public int getUsedSlots();

    /**
     *
     *  Get the number of free slots
     *  (returns not free but total slots -> use getTotalSlots)
     *  @return number of free slots
     */
    @Deprecated
    public int getFreeSlots();

    /**
     *  Get the number of total slots
     *  @return number of total slots
     */
    public int getTotalSlots();

    /**
     *  Get the architecture of the queue instance
     *  @return architecture of the queue instance
     */
    public String getArch();
    
    /**
     *  Get the state of the queue instance
     *  @return state of the queue instance
     */
    public String getState();
    
    /**
     * Get the string representation of the load average
     *
     * @return the string representation of the load average
     */
    public String getLoadAvgStr();
    
    /**
     * Determine if the queue instance has a load value
     * @return <code>true</code> if the queue instance has a load value
     */
    public boolean hasLoadValue();
    
    /**
     *  Determine if the load value is derived from an object
     *  @return <code>true</code> if the load value is derived from an object
     */
    public boolean isHasLoadValueFromObject();
    
    /**
     *  Get the load average of the queue instance. Return only a meanful
     *  value if <code>hasHasLoadValue</code> returns <code>true</code>.
     *
     *  @return The load average of the queue instance
     *  @see #hasLoadValue
     */
    public double getLoadAvg();
    
    /**
     *  Get the list of jobs which are assigned to the queue instance
     *  @return list of jobs (instances of {@link JobSummary})
     */
    public List<JobSummary> getJobList();
    
    /**
     *  Get the load alarm reason for the queue instance
     *  @return the load alarm reason
     */
    public String getLoadAlarmReason();
    
    /**
     *  Get the suspend alarm reason for the queue instance
     *  @return the suspend alarm reason
     */
    public String getSuspendAlarmReason();
    
    /**
     *  Get the list of explain messages
     *  @return list of explain messages
     */
    public List<String> getExplainMessageList();
    
    /**
     *  Get avaialable dominances for the resource values
     *  @return set of avaialable dominances
     */
    public Set<String> getResourceDominanceSet();
    
    /**
     *  Get all resources names which have the dominane <code>dom</code>
     *  @return set of resource names
     */
    public Set<String> getResourceNames(String dom);
    
    /**
     *  Get a resource value
     *  @param  dom   dominance of the resource value
     *  @param  name  name of the resource
     *  @return the resource value
     */
    public String getResourceValue(String dom, String name);
}
