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
 * Defines the possible actions that
 * a user can request on a JAM job.
 *
 * @version 1.3, 09/22/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 *
 */
public class JobAction
  implements Serializable
{
  public static final int _NONE    = 0;
  public static final int _BLOCK   = 1;
  public static final int _QUEUE   = 2;
  public static final int _SUBMIT  = 3;
  public static final int _SUSPEND = 4;
  public static final int _RESUME  = 5;
  public static final int _STOP    = 6;
  public static final int _START   = 7;
  public static final int _RESTART = 8;
  public static final int _KILL    = 9;

  public static final JobAction NONE    = new JobAction(_NONE);
  public static final JobAction BLOCK   = new JobAction(_BLOCK);
  public static final JobAction QUEUE   = new JobAction(_QUEUE);
  public static final JobAction SUBMIT  = new JobAction(_SUBMIT);
  public static final JobAction SUSPEND = new JobAction(_SUSPEND);
  public static final JobAction RESUME  = new JobAction(_RESUME);
  public static final JobAction STOP    = new JobAction(_STOP);
  public static final JobAction START   = new JobAction(_START);
  public static final JobAction RESTART = new JobAction(_RESTART);
  public static final JobAction KILL    = new JobAction(_KILL);

  /**
   * Contains the action value. 
   * 
   */
  public Integer action;

  /**
   * Public no argument constructor 
   *
   */
  public JobAction()
  {
    super();
  }

  /**
   * Creates a JobAction
   *
   * @param Integer action - the action requested
   *
   */
  public JobAction(Integer action)
  {
    this.action = action;
  }

  /**
   * Creates a JobAction
   *
   * @param int status - the action requested
   *
   */
  public JobAction(int action)
  {
    this.action = new Integer(action);
  }

  /**
   * Overrides the equals method of java.lang.Object.
   *
   * @param Object the object to be compared
   */
  public boolean equals(Object object)
  {
    if(object instanceof JobAction)
      return action.equals(((JobAction)object).action);
    return false;
  }

  /**
   * Return the action of the job as an Integer.
   *
   * @return the action of the job as an Integer object.
   */
  public Integer getAction()
  {
    return action;
  }
  
  /**
   * Return the action of the job as an int.
   *
   * @return the action of the job as an int value.
   */
  public int intValue()
  {
    return action.intValue();
  }

  /**
   * Overrides the toString method of java.lang.Object.
   *
   * @return the string representation of the object
   */
  public String toString()
  {
    StringBuffer st = new StringBuffer();

    switch(action.intValue()){

    case _NONE:
      st.append("No Action");
      break; 

    case _BLOCK:
      st.append("Block");
      break;
      
    case _QUEUE:
      st.append("Job to be Queued");
      break;
      
    case _SUBMIT:
      st.append("Submit");
      break;

    case _SUSPEND:
      st.append("Suspend");
      break;

    case _RESUME:
      st.append("Resume");
      break;

    case _STOP:
      st.append("Stop");
      break;

    case _START:
      st.append("Start");
      break;
      
    case _RESTART: 
      st.append("Re-Start");
      break;

    case _KILL:
      st.append("Kill");
      break;

    default:
      st.append("Unknown");
      break;
      
    }
    return st.toString();
  }
}

