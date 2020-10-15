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
 * Instances of this class hold the cluster queue summary information
 * of one queue.
 *
 */
public class ClusterQueueSummary implements java.io.Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private String name;
    private double load;
    private boolean   isLoadSet = false;
    
    private int reservedSlots;
    private int usedSlots;
    private int totalSlots;
    private int availableSlots;
    
    private int tempDisabled;
    private int manualIntervention;
    private int suspendManual;
    private int suspendThreshold;
    private int suspendOnSubordinate;
    private int suspendByCalendar;
    private int unknown;
    private int loadAlarm;
    private int disabledManual;
    private int disabledByCalendar;
    private int ambiguous;
    
    private int orphaned;
    
    private int error;
    
    /** Creates a new instance of ClusterQueueSummary */
    public ClusterQueueSummary() {
    }
    
    /**
     *  Get the name of the cluster queue
     */
    public String getName() {
        return name;
    }
    
    /**
     *  Set the name of the cluster queue
     *  @param name  the name
     */
    public void setName(String name) {
        this.name = name;
    }
    
    /**
     *  Get the load of the cluster queue. If the <code>isLoadSet</code> method
     *  returns <code>false</code>, the result of this method is undefined.
     *
     *  @return an average of the normalized load average  of  all queue hosts.  In
     *          order to reflect each hosts different significance the number of
     *          configured slots is used as a weighting  factor  when  determining
     *          cluster queue load. Please note that only hosts with a np_load_value are
     *          considered for this value.
     */
    public double getLoad() {
        return load;
    }
    
    /**
     *  Set the load of the cluster queue
     *  @param load  the new load value
     *  @see #getLoad
     */
    public void setLoad(double load) {
        this.load = load;
        isLoadSet = true;
    }
    
    /**
     *  Has the load value been set
     *
     *  @return  <code>true of the load value is set</code>
     */
    public boolean isLoadSet() {
        return isLoadSet;
    }
    /**
     *  Get the number of currently reserved slots
     *
     *  @return number of currently reserved slots
     */
    public int getReservedSlots() {
        return reservedSlots;
    }
    
    /**
     *  Set the number of currently reserved slots
     *
     *  @param reservedSlots  number of currently reserved slots
     */
    public void setReservedSlots(int reservedSlots) {
        this.reservedSlots = reservedSlots;
    }
    
    
    /**
     *  Get the number of currently used slots
     *
     *  @return number of currently used slots
     */
    public int getUsedSlots() {
        return usedSlots;
    }
    
    /**
     *  Set the number of currently used slots
     *
     *  @param usedSlots  number of currently used slots
     */
    public void setUsedSlots(int usedSlots) {
        this.usedSlots = usedSlots;
    }
    
    /**
     *  Get the total number of slots of this cluster queue
     *
     *  @return the total number of slots
     */
    public int getTotalSlots() {
        return totalSlots;
    }
    
    /**
     *   Set the total number of slots.
     *
     *   @param  totalSlots the total number of slots
     */
    public void setTotalSlots(int totalSlots) {
        this.totalSlots = totalSlots;
    }
    
    /**
     *   Get the number of currently available slots.
     *
     *   @return the number of currently available slots
     */
    public int getAvailableSlots() {
        return availableSlots;
    }
    
    /**
     *   Set the number of currently available slots
     *   @param availableSlots number of currently available slots
     */
    public void setAvailableSlots(int availableSlots) {
        this.availableSlots = availableSlots;
    }
    
    /**
     *  <p>Get the number of temporary disabled queue instances.</p>
     *
     *  <p><b>Note:</b> This value is only set if the <code>showAdditionalAttributes</code>
     *  is set in the options for the {@link com.sun.grid.jgdi.JGDIBase#getClusterQueueSummary
     *  cluster queue summary algorithm}.</p>
     *
     *  @return number of temporary disabled queue instances
     *  @see BasicQueueOptions#showAdditionalAttributes
     */
    public int getTempDisabled() {
        return tempDisabled;
    }
    
    /**
     *  Set the number of temporary disabled queue instances.
     *  @param tempDisabled  the number of temporary disabled queue instances.
     */
    public void setTempDisabled(int tempDisabled) {
        this.tempDisabled = tempDisabled;
    }
    
    
    /**
     *  <p>Get the number of manually disabled queue instances.</p>
     *
     *  <p><b>Note:</b> This value is only set if the <code>showAdditionalAttributes</code>
     *  is set in the options for the {@link com.sun.grid.jgdi.JGDIBase#getClusterQueueSummary
     *  cluster queue summary algorithm}.</p>
     *
     *  @return number of manually disabled queue instances
     *  @see BasicQueueOptions#showAdditionalAttributes
     */
    public int getManualIntervention() {
        return manualIntervention;
    }
    
    /**
     *  Set the number manually disabled queue instances.
     *  @param manualIntervention number manually disabled queue instances.
     */
    public void setManualIntervention(int manualIntervention) {
        this.manualIntervention = manualIntervention;
    }
    
    /**
     *  Get the number of manually suspended queue instances.
     *
     *  <p><b>Note:</b> This value is only set if the <code>showAdditionalAttributes</code>
     *  is set in the options for the {@link com.sun.grid.jgdi.JGDIBase#getClusterQueueSummary
     *  cluster queue summary algorithm}.</p>
     *
     *  @return number of manually suspended queue instances
     */
    public int getSuspendManual() {
        return suspendManual;
    }
    
    /**
     *  Set the number of manually suspended queue instances
     *
     *  @param suspendManual number of manually suspended queue instances
     */
    public void setSuspendManual(int suspendManual) {
        this.suspendManual = suspendManual;
    }
    
    /**
     *  Get the suspend threshold of the cluster queue
     *
     *  <p><b>Note:</b> This value is only set if the <code>showAdditionalAttributes</code>
     *  is set in the options for the {@link com.sun.grid.jgdi.JGDIBase#getClusterQueueSummary
     *  cluster queue summary algorithm}.</p>
     *
     *  @return the suspend threshold of the cluster queue
     */
    public int getSuspendThreshold() {
        return suspendThreshold;
    }
    
    /**
     *  Set the suspend threshold of the cluster queue
     *
     *  @param suspendThreshold the suspend threshold
     */
    public void setSuspendThreshold(int suspendThreshold) {
        this.suspendThreshold = suspendThreshold;
    }
    
    /**
     *  Get the number of queue instances which has been suspended on
     *  a subordinate queue.
     *
     *  <p><b>Note:</b> This value is only set if the <code>showAdditionalAttributes</code>
     *  is set in the options for the {@link com.sun.grid.jgdi.JGDIBase#getClusterQueueSummary
     *  cluster queue summary algorithm}.</p>
     *
     *  @return The number of queue instances which has been suspended on
     *          a subordinate queue.
     */
    public int getSuspendOnSubordinate() {
        return suspendOnSubordinate;
    }
    
    /**
     *  Set the number of queue instances which has been suspended on
     *  a subordinate queue.
     *
     *  @param suspendOnSubordinate The number of queue instances which has been
     *         suspended on a subordinate queue
     */
    public void setSuspendOnSubordinate(int suspendOnSubordinate) {
        this.suspendOnSubordinate = suspendOnSubordinate;
    }
    
    /**
     *  Get the number of queue instances which has been suspended by a
     *  calendar.
     *
     *  <p><b>Note:</b> This value is only set if the <code>showAdditionalAttributes</code>
     *  is set in the options for the {@link com.sun.grid.jgdi.JGDIBase#getClusterQueueSummary
     *  cluster queue summary algorithm}.</p>
     *
     *  @return number of queue instances which has been suspended by a
     *          calendar.
     */
    public int getSuspendByCalendar() {
        return suspendByCalendar;
    }
    
    /**
     *  Set the number of queue instances which has been suspended by a
     *  calendar.
     *
     *  <p><b>Note:</b> This value is only set if the <code>showAdditionalAttributes</code>
     *  is set in the options for the {@link com.sun.grid.jgdi.JGDIBase#getClusterQueueSummary
     *  cluster queue summary algorithm}.</p>
     *
     *  @param suspendByCalendar number of queue instances which has been
     *                           suspended by a calendar.
     */
    public void setSuspendByCalendar(int suspendByCalendar) {
        this.suspendByCalendar = suspendByCalendar;
    }
    
    /**
     *   Get the number of queue instances which are in an unknown state.
     *
     *  <p><b>Note:</b> This value is only set if the <code>showAdditionalAttributes</code>
     *  is set in the options for the {@link com.sun.grid.jgdi.JGDIBase#getClusterQueueSummary
     *  cluster queue summary algorithm}.</p>
     *
     *   @return number of queue instances which are in an unknown state.
     */
    public int getUnknown() {
        return unknown;
    }
    
    /**
     *   Set the number of queue instances which are in an unknown state.
     *   @param unknown number of queue instances which are in an unknown state
     */
    public void setUnknown(int unknown) {
        this.unknown = unknown;
    }
    
    /**
     *  Get the number of queues instances which has an load alarm.
     *
     *  @return number of queues instances which has an load alarm.
     */
    public int getLoadAlarm() {
        return loadAlarm;
    }
    
    /**
     *  Set the number of queue instances which has an load alarm.
     *
     *  @param loadAlarm number of queue instances which has an load alarm.
     *
     */
    public void setLoadAlarm(int loadAlarm) {
        this.loadAlarm = loadAlarm;
    }
    
    /**
     *  Get the number of queue instances which has been manually disabled
     *
     *  @return number of queue instances which has been manually disabled
     */
    public int getDisabledManual() {
        return disabledManual;
    }
    
    /**
     *  Set the number of queue instances which has been manually disabled
     *  @param disabledManual number of queue instances which has been manually disabled
     */
    public void setDisabledManual(int disabledManual) {
        this.disabledManual = disabledManual;
    }
    
    /**
     *  Get the number of queue instances which has been by a calendar
     *
     *  @return number of queue instances which has been by a calendar
     */
    public int getDisabledByCalendar() {
        return disabledByCalendar;
    }
    
    /**
     *  Set the number of queue instances which has been by a calendar
     *
     *  @param disabledByCalendar number of queue instances which has been by a calendar
     */
    public void setDisabledByCalendar(int disabledByCalendar) {
        this.disabledByCalendar = disabledByCalendar;
    }
    
    /**
     *  Get the number of queue instances which are in the ambiguous state
     *
     *  @return number of queue instances which are in the ambiguous state
     */
    public int getAmbiguous() {
        return ambiguous;
    }
    
    /**
     *  Set the number of queue instances which are in the ambiguous state
     *
     *  @param ambiguous number of queue instances which are in the ambiguous state
     */
    public void setAmbiguous(int ambiguous) {
        this.ambiguous = ambiguous;
    }
    
    /**
     *  Get the number of queue instances which are orqhaned.
     *
     *  @return number of queue instances which are orqhaned
     */
    public int getOrphaned() {
        return orphaned;
    }
    
    /**
     *  Set the number of queue instances which are orqhaned.
     *
     *  @param orphaned number of queue instances which are orqhaned
     */
    public void setOrphaned(int orphaned) {
        this.orphaned = orphaned;
    }
    
    /**
     *  Get the number of queue instances which are in error state
     *
     *  @return number of queue instances which are in error state
     */
    public int getError() {
        return error;
    }
    
    /**
     *  Set the number of queue instances which are in error state
     *  @param error number of queue instances which are in error state
     */
    public void setError(int error) {
        this.error = error;
    }
    
}
