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
package com.sun.grid.jam.job;

import java.io.Serializable;

/**
 * Defines the status of a job
 * item to be stored in the queue space.
 *
 * @version 1.4, 09/22/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 *
 */
public class JobStatus
  implements Serializable
{
  public static final int _UNKNOWN   = 0;
  public static final int _JAMQUEUED = 1;
  public static final int _IDLE      = 2;
  public static final int _HELD      = 3;
  public static final int _RUNNING   = 4;
  public static final int _SUSPENDED = 5;
  public static final int _STOPPED   = 6;
  public static final int _DELETED   = 7;
  public static final int _KILLED    = 8;
  public static final int _FINISHED  = 9;
  public static final int _COMPLETED = 10;
  public static final int _ERROR     = 11;

  public static final JobStatus UNKNOWN   = new JobStatus(_UNKNOWN);
  public static final JobStatus JAMQUEUED = new JobStatus(_JAMQUEUED);
  public static final JobStatus IDLE      = new JobStatus(_IDLE);
  public static final JobStatus HELD      = new JobStatus(_HELD);
  public static final JobStatus RUNNING   = new JobStatus(_RUNNING);
  public static final JobStatus SUSPENDED = new JobStatus(_SUSPENDED);
  public static final JobStatus STOPPED   = new JobStatus(_STOPPED);
  public static final JobStatus DELETED   = new JobStatus(_DELETED);
  public static final JobStatus KILLED    = new JobStatus(_KILLED);
  public static final JobStatus FINISHED  = new JobStatus(_COMPLETED);
  public static final JobStatus COMPLETED = new JobStatus(_COMPLETED);
  public static final JobStatus ERROR     = new JobStatus(_ERROR);
  

  /**
   * Contains the status value. 
   * 
   */
  public Integer status;

  /**
   * Public no argument constructor 
   *
   */
  public JobStatus()
  {
    super();
  }

  /**
   * Creates a JobStatus
   *
   * @param Integer status - the status of the Job
   *
   */
  public JobStatus(Integer status)
  {
    this.status = status;
  }

  /**
   * Creates a JobStatus
   *
   * @param int status - the status of the Job
   *
   */
  public JobStatus(int status)
  {
    this.status = new Integer(status);
  }

  /**
   * Overrides the equals method of java.lang.Object.
   *
   * @param Object the object to be compared
   */
  public boolean equals(Object object)
  {
    if(object instanceof JobStatus)
      return status.equals(((JobStatus)object).status);
    return false;
  }

  /**
   * Return the status of the job as an Integer.
   *
   * @return the status of the job as an Integer object.
   */
  public Integer getStatus()
  {
    return status;
  }
  
  /**
   * Return the status of the job as an int.
   *
   * @return the status of the job as an int value.
   */
  public int intValue()
  {
    return status.intValue();
  }

  /**
   * Overrides the toString method of java.lang.Object.
   *
   * @return the string representation of the object
   */
  public String toString()
  {
    StringBuffer st = new StringBuffer();
    
    switch(status.intValue()){

    case _JAMQUEUED:
      st.append("JAM QUEUED");
      break;

    case _IDLE:
      st.append("IDLE");
      break;

    case _HELD:
      st.append("RMS QUEUED");
      break;

    case _RUNNING:
      st.append("RUNNING");
      break;

    case _SUSPENDED:
      st.append("SUSPENDED");
      break;

    case _STOPPED:
      st.append("STOPPED");
      break;

    case _DELETED:
      st.append("DELETED");
      break;
      
    case _KILLED:
      st.append("KILLED");
      break;

    case _FINISHED:
      st.append("FINISHED");
      break;
      
    case _COMPLETED:
      st.append("COMPLETED");
      break;

    case _ERROR:
      st.append("ERROR");
      break;

    default:
      st.append("UNKNOWN");
      break;
      
    }
    return st.toString();
  }
}
