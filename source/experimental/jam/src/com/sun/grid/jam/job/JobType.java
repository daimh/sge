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
 * Defines the type of a job
 * 
 * @version 1.2, 09/22/00
 *
 * @author Nello Nellari
 *
 */
public class JobType
  implements Serializable
{
  public static final JobType BATCH = new JobType("BATCH");
  public static final JobType INTERACTIVE = new JobType("INTERACTIVE");
  public static final JobType PARALLEL = new JobType("PARALLEL");
  
  public String type;

  /**
   * Public no argument constructor 
   *
   */
  public JobType()
  {
    super();
  }

  /**
   * Creates a JobType
   *
   * @param String name - the type of the Job
   *
   */
  public JobType(String type)
  {
    this.type = type;
  }

  /**
   * Overrides the equals method of java.lang.Object.
   *
   * @param Object the object to be compared
   */
  public boolean equals(Object object)
  {
    if(object instanceof JobType)
      return type.equals(((JobType)object).type);
    return false;
  }
  
  /**
   * Return the type of the job as a String.
   *
   * @return the type of the job
   */
  public String getType()
  {
    return type;
  }

  /**
   * Overrides the toString method of java.lang.Object.
   *
   * @return the string representation of the object
   */
  public String toString()
  {
    return type;
  }
}
