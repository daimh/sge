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
 *   and/or Swiss Center for Scientific Computing
 * 
 *   Copyright: 2002 by Sun Microsystems, Inc.
 *   Copyright: 2002 by Swiss Center for Scientific Computing
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jam.job.entry;

import net.jini.core.entry.Entry;
import com.sun.grid.jam.job.JobStatus;
import com.sun.grid.jam.job.JobAction;

/**
 * JavaSpaces entry to define the state of a JAM job
 *
 * @version 1.2, 09/22/00
 *
 * @author Nello Nellari
 *
 */
public class JobControlEntry
  implements Entry
{
  public JobInfo   jobInfo;
  public JobStatus jobStatus;
  public JobAction jobAction;
  public String	   queueName;
  public Entry[]   attributes;
  
  /**
   * Entry requires public no argument constructor 
   *
   */
  public JobControlEntry()
  {
    super();
  }

  /**
   * For creating a JobControlEntry template for matching
   * in the JobRepository
   *
   */
  public JobControlEntry(JobInfo jobInfo)
  {
    this.jobInfo = jobInfo;
    // Need queueName as a top level object so we can match against it.
    // Also should be its own copy, not a reference to the other one
    this.queueName = new String(jobInfo.queueName);
  }

  /**
   * For creating a JobControlEntry template for matching
   * in the JobRepository
   *
   */
  public JobControlEntry(JobStatus jobStatus)
  {
    this.jobStatus = jobStatus;
  }

  /**
   * For creating a JobControlEntry template for matching
   * in the JobRepository
   *
   */
  public JobControlEntry(JobAction jobAction)
  {
    this.jobAction = jobAction;
  }

  /**
   * For creating a JobControlEntry template for matching
   * in the JobRepository by status & action
   *
   */
  public JobControlEntry(JobStatus jobStatus,
                         JobAction jobAction)
  {
    this.jobStatus = jobStatus;
    this.jobAction = jobAction;
  }
  
  /**
   * For creating a JobControlEntry template for matching
   * in the JobRepository by action & queue name
   *
   */
  public JobControlEntry(JobAction jobAction, String qname)
  {
    this.jobAction = jobAction;
    this.queueName = qname;
  }
  
  /**
   * For creating a JobControlEntry template for matching
   * in the JobRepository by status & queue name
   *
   */
  public JobControlEntry(JobStatus jobStatus, String qname)
  {
    this.jobStatus = jobStatus;
    this.queueName = qname;
  }
  
  /**
   * Creates a JobControlEntry for a job
   * submitted in JAM
   *
   */
  public JobControlEntry(JobInfo jobInfo, JobStatus jobStatus,
                         JobAction jobAction, Entry[] attributes)
  {
    this(jobInfo);		// Also sets queueName
    this.jobStatus = jobStatus;
    this.jobAction = jobAction;
    this.attributes = attributes;
  }

  public String toString()
  {
    int n = ((attributes == null) ? 0: attributes.length);
    String s = "== JobControlEntry ==\n== Job Info\n" + jobInfo;
//     if(jobInfo != null)
//       s = s + jobInfo.toString();
    return s + "\n== Control Attributes" + "\nStatus    : " +
      jobStatus + "\nAction    : " + jobAction + "\nAttributes: "
      + n + "\n=====================";
  }
}
