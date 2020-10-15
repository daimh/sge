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

import com.sun.grid.jam.job.JobType;
import net.jini.core.entry.Entry;

/**
 * Defines the key info for a JAM job.
 *
 *
 * @version 1.2, 09/22/00
 *
 * @author Nello Nellari
 *
 */
public class JobInfo
  implements Entry
{
  public String  name;
  public JobType type;
  public String  queueName;
  public Integer key;
  public String  userID;
  public String  groupID;

  /**
   * Public no argument constructor 
   *
   */
  public JobInfo()
  {
    super();
  }

  /**
   * Creates a JobDescription
   *
   * @param String name      - the name of the Job
   * @param JobType type     - the type of the job
   * @param String queueName - the name of the queue
   * @param Integer jobID    - the ID of the job 
   * @param String userID    - the ID of the user
   * @param String groupID   - the ID of the user's group
   *
   */
  public JobInfo(String name, JobType type,
                 String queueName, String userID,
                 String groupID)
  {
    this.name = name;
    this.type = type;    
    this.queueName = queueName;
    this.userID = userID;
    this.groupID = groupID;
  }

  /**
   * Creates a JobDescription
   *
   * @param String name      - the name of the Job
   * @param JobType type     - the type of the job
   * @param String queueName - the name of the queue
   * @param String userID    - the ID of the user
   * @param String groupID   - the ID of the user's group
   * @param int key          - the ID of the job 
   *
   */
  public JobInfo(String name, JobType type,
                 String queueName, String userID,
                 String groupID, int key)
  {
    this(name, type, queueName, userID, groupID);
    this.key = new Integer(key);
  }

  public void setJobID(int id)
  {
    key = new Integer(id);
  }

  public String toString()
  {
    return "Name : " + name + "\nType : " + type + "\nQueue: " +
      queueName + "\nUser : " + userID + "\nGroup: " +
      groupID + "\nKey  : " + key;
  }
}
