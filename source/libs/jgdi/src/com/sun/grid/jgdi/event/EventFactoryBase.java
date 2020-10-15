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
package com.sun.grid.jgdi.event;

/**
 *
 */
public class EventFactoryBase {
    
    public static QmasterGoesDownEvent createQmasterGoesDownEvent(long timestamp, int id) {
        return new QmasterGoesDownEvent(timestamp, id);
    }
    
    public static ShutdownEvent createShutdownEvent(long timestamp, int id) {
        return new ShutdownEvent(timestamp, id);
    }
    
    public static SchedulerRunEvent createSchedulerRunEvent(long timestamp, int id) {
        return new SchedulerRunEvent(timestamp, id);
    }
    
    public static JobFinishEvent createJobFinishEvent(long timestamp, int id) {
        return new JobFinishEvent(timestamp, id);
    }
    
    public static JobUsageEvent createJobUsageEvent(long timestamp, int id) {
        return new JobUsageEvent(timestamp, id);
    }
    
    public static JobFinalUsageEvent createJobFinalUsageEvent(long timestamp, int id) {
        return new JobFinalUsageEvent(timestamp, id);
    }
    
    public static QueueInstanceSuspendEvent createQueueInstanceSuspendEvent(long timestamp, int id) {
        return new QueueInstanceSuspendEvent(timestamp, id);
    }
    
    public static QueueInstanceUnsuspendEvent createQueueInstanceUnsuspendEvent(long timestamp, int id) {
        return new QueueInstanceUnsuspendEvent(timestamp, id);
    }
    
    public static JobPriorityModEvent createJobPriorityModEvent(long timestamp, int id) {
        return new JobPriorityModEvent(timestamp, id);
    }
}
