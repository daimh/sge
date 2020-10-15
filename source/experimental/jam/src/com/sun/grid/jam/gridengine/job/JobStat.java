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
/**
 * This package contains classes and interfaces for SGE Jobs.
 *
 * @version 1.4, 06/21/00
 *
 * @author Rafal Piotrowski
 */
package com.sun.grid.jam.gridengine.job;

import java.util.ArrayList;

/**
 * This class that represents both job's state and status. 
 *
 * @version 1.4, 06/21/00
 *
 * @author Rafal Piotrowski
 */
public class JobStat
{
  //============= attributes ================

  /** Job's state. */
  private int state;
  
  /** Job's status. */
  private int status;
  
  /**
   * Job's state can be combination of many states, so this variable
   * represents job states.
   *
   * @see java.util.ArrayList
   */
  private ArrayList states;
  
  //============= constant values ================

  // JAM's Job states
  public final static int JUNKNOWN                = 0xfffffff0;

  /** New job. */
  public final static int JNEW                    = 0xfffffff8;
  
  // SGE's Job states

  /** Job idle. */
  public final static int JIDLE                   = 0x00000000;

  /** Job enabled. */
  public final static int JENABLED                = 0x00000008;

  /** Job held. */
  public final static int JHELD                   = 0x00000010;

  /** Job migrating. */
  public final static int JMIGRATING              = 0x00000020;

  /** Job queued. */
  public final static int JQUEUED                 = 0x00000040;

  /** Job running. */
  public final static int JRUNNING                = 0x00000080;

  /** Job suspended. */
  public final static int JSUSPENDED              = 0x00000100;

  /** Job transiting. */
  public final static int JTRANSITING             = 0x00000200;

  /** Job deleted. */
  public final static int JDELETED                = 0x00000400;

  /** Job waiting. */
  public final static int JWAITING                = 0x00000800;

  /** Job exiting. */
  public final static int JEXITING                = 0x00001000;

  /** Job written. */
  public final static int JWRITTEN                = 0x00002000;

  /**
   * Job waiting for OS's job ID.
   * used in execd - job waits for getting its ASH/JOBID
   */
  public final static int JWAITING4OSJID          = 0x00004000;

  /**
   * Job error.
   * used in execd - shepherd reports job exit but there
   * are still processes */
  public final static int JERROR                  = 0x00008000;

  /** Job suspended on threshold. */
  public final static int JSUSPENDED_ON_THRESHOLD = 0x00010000;

  /**
   * Job finished.
   * GRD: qmaster delays job removal till schedd 
   * does no longer need this finished job 
   */  
  public final static int JFINISHED               = 0x00010000;

  /**
   * Slave job.
   * used in execd to prevent slave jobs from getting started
   */
  public final static int JSLAVE                  = 0x00020000;


  // constant objects
  // I tnink that this should speed up runtime performance
  public final static Integer _JUNKNOWN                = new
    Integer(JUNKNOWN);
  public final static Integer _JNEW                    = new
    Integer(JNEW);
  public final static Integer _JIDLE                   = new
    Integer(JIDLE);
  public final static Integer _JENABLED                = new
    Integer(JENABLED);
  public final static Integer _JHELD                   = new
    Integer(JHELD);
  public final static Integer _JMIGRATING              = new
    Integer(JMIGRATING);
  public final static Integer _JQUEUED                 = new
    Integer(JQUEUED);
  public final static Integer _JRUNNING                = new
    Integer(JRUNNING);
  public final static Integer _JSUSPENDED              = new
    Integer(JSUSPENDED);
  public final static Integer _JTRANSITING             = new
    Integer(JTRANSITING);
  public final static Integer _JDELETED                = new
    Integer(JDELETED);
  public final static Integer _JWAITING                = new
    Integer(JWAITING);
  public final static Integer _JEXITING                = new
    Integer(JEXITING);
  public final static Integer _JWRITTEN                = new
    Integer(JWRITTEN);
  public final static Integer _JWAITING4OSJID          = new
    Integer(JWAITING4OSJID);
  public final static Integer _JERROR                  = new
    Integer(JERROR);
  public final static Integer _JSUSPENDED_ON_THRESHOLD = new
    Integer(JSUSPENDED_ON_THRESHOLD);
  public final static Integer _JFINISHED               = new
    Integer(JFINISHED);
  public final static Integer _JSLAVE                  = new
    Integer(JSLAVE);
  
  //============== constructors ===============
  
  /** Sets state & status to JNEW. */
  public JobStat()
  {
    states = new ArrayList();
    state = JNEW;
    status = JNEW;
  }

  /**
   * Set state and status to specific values.
   *
   * @param state  - SGE's job state.
   * @param status - SGE's job status.
   */
  public JobStat(int state, int status)
    throws JobStatException
  {
    states = new ArrayList();
    setStatus(status);
    setState(state);
  }
  
  //============== getters ===============
  
  public int getState()
  {
    return state;
  }

  public int getStatus()
  {
    return status;
  }

  public int[] getStates()
  {
    if(states.isEmpty())
      return null;
    int size = states.size();
    int[] sts = new int[size];
    for(int i = 0; i < size; i++)
      sts[i] = ((Integer)states.get(i)).intValue();
    return sts;
  }
  
  public String getStateAsString(int state)
    throws JobStatException
  {
    StringBuffer s = new StringBuffer(10);
    switch(state) {
      case JERROR:
        s.append("Error");
        break;
      case JWAITING:
        s.append("Waiting");
        break;
      case JDELETED:
        s.append("Deleted");
        break;
      case JSUSPENDED:
        s.append("Suspended");
        break;
      case JRUNNING:
        s.append("Running");
        break;
      case JQUEUED:
        s.append("Queued");
        break;
      case JHELD:
        s.append("Held");
        break;
      default:
        throw new JobStatException("Unknown Job State");
      }
    return s.toString();
  }

  public String[] getStatesAsString()
  {
    if(states.isEmpty())
      return null;
    int size = states.size();
    String[] strs = new String[size];
    for(int i = 0; i < size; i++){
      int st = ((Integer)states.get(i)).intValue();
      try {
        strs[i] = getStateAsString(st);
      } catch(JobStatException e) {
        e.printStackTrace();
        return null;
      }
    }
    return strs;
  }

  public String getStatusAsString(int status)
    throws JobStatException
  {
    StringBuffer s = new StringBuffer(11);
    switch(status) {
    case JIDLE:
      s.append("Idle");
      break;
    case JRUNNING:
      s.append("Running");
      break;
    case JTRANSITING:
      s.append( "Transiting" );
      break;
    case JFINISHED:
      s.append( "Finished" );
      break;
    default:
      throw new JobStatException("Unknown Job Status.");
    }
    return s.toString();
  }

  //============== setters ===============
  
  public void setState(int state)
    throws JobStatException
  {
    this.state = state;
    if(!states.isEmpty())
      states.clear();
    parseState(state, 0);
  }

  public void setStatus(int status)
    throws JobStatException
  {
    if(status == JIDLE ||
       status == JRUNNING ||
       status == JTRANSITING ||
       status == JFINISHED) {
      this.status = status;
    } else
      throw new JobStatException("Unknown Job Status. See documentation for available job statuses.");
  }

  //============== others ===============

  public void parseState(int state, int pos)
    throws JobStatException
  {
    int nextState;
    if(state == 0)
      return;
    else if( (state - JERROR) >= 0 ) {
      states.add(_JERROR);
      nextState = state - JERROR;
      parseState(nextState, pos++);
    }
    else if( (state - JWAITING) >= 0 ) {
      states.add(_JWAITING);
      nextState = state - JWAITING;
      parseState(nextState, pos++);
    }
    else if( (state - JDELETED) >= 0 ) {
      states.add(_JDELETED);
      nextState = state - JDELETED;
      parseState(nextState, pos++);
    }
    else if( (state - JSUSPENDED) >= 0 ) {
      states.add(_JSUSPENDED);
      nextState = state - JSUSPENDED;
      parseState(nextState, pos++);
    }
    else if( (state - JRUNNING) >= 0 ) {
      states.add(_JRUNNING);
      nextState = state - JRUNNING;
      parseState(nextState, pos++);
    }
    else if( (state - JQUEUED) >= 0 ) {
      states.add(_JQUEUED);
      nextState = state - JQUEUED;
      parseState(nextState, pos++);
    }
    else if( (state - JHELD) >= 0 ) {
      states.add(_JHELD);
      nextState = state - JHELD;
      parseState(nextState, pos++);
    }
    else
      throw new JobStatException("parseState: unknown job state.");
  }

  public boolean containsState(Integer s)
  {
    return states.contains(s);
  }
    
  public String toString()
  {
    try {
      StringBuffer m = new StringBuffer("Status (").append(status).append("): ").append(getStatusAsString(status)).append("\nStates (").append(state).append("): ");
      String[] sts = getStatesAsString();
      int[] s = getStates();
      if(sts.length == s.length)
        for(int i = sts.length; --i >= 0;)
          m.append(sts[i]).append("(").append(s[i]).append(") ");
      return m.toString();
    } catch(JobStatException jse) {
      return "Incorrect job status/state";
    }
  }
}
