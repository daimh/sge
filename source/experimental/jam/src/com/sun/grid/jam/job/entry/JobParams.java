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

import java.net.URL;
import net.jini.core.entry.Entry;

/**
 * Contains job submission params
 * and RMS specific info
 *
 * @version 1.2, 09/22/00
 *
 * @author Nello Nellari
 *
 */
public class JobParams
  implements Entry
{
  public JobInfo  jobInfo;
  public Integer  jobID;
  public String   script;
  public String[] args;
  public URL      input;
  public URL      output;
  public URL      error;
  public String   submissionName;
  public Integer  submissionID;

  /**
   * Public no argument constructor 
   *
   */
  public JobParams()
  {
    super();
  }

  /**
   * Creates a JobParams for searching
   * in the JobRepository
   *
   */
  public JobParams(JobInfo jobInfo)
  {
    this.jobInfo = jobInfo;
  }

  /**
   * Creates a JobParams for a job
   * submitted in JAM
   *
   */
  public JobParams(JobInfo jobInfo, Integer jobID,
                   String script, String[] args)
  {
    this(jobInfo);
    this.jobID = jobID;
    this.script = script;    
    this.args = args;
  }

  /**
   * Creates a JobParams for a job
   * submitted in JAM
   *
   */
  public JobParams(JobInfo jobInfo, int jobID,
                   String script, String[] args)
  {
    this(jobInfo);
    this.jobID = new Integer(jobID);
    this.script = script;    
    this.args = args;
  }
  
  /**
   * Creates a JobParams for a job
   * submitted in JAM
   *
   */
  public JobParams(JobInfo jobInfo, String script,
                   Integer jobID, String[] args,
                   URL input, URL output, URL error)
  {
    this(jobInfo, jobID, script, args);
    this.input = input;
    this.output = output;
    this.error = error;
  }

  /**
   * Creates a JobParams for a job
   * submitted in JAM
   *
   */
  public JobParams(JobInfo jobInfo, String script,
                   int jobID, String[] args,
                   URL input, URL output, URL error)
  {
    this(jobInfo, jobID, script, args);
    this.input = input;
    this.output = output;
    this.error = error;
  }
  
  public void setSubmissionParams(String submissionName,
                                  int submissionID)
  {
    this.submissionName = submissionName;
    this.submissionID = new Integer(submissionID);
  }

  public String toString()
  {
    return "== JobParams ==\n-] Job Info\n"  +
      jobInfo.toString() + "\n-] JobParams" + "\nID: " +
      jobID + "\nInput: " + input + "\nOutput: " +
      output + "\nError: " + error + "\n-] SGE Params" +
      "\nSubmission Name: " + submissionName + "\nSubmission ID: " +
      submissionID +"\n===============";
  }
}
