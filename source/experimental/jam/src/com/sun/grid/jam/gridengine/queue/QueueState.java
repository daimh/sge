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
package com.sun.grid.jam.gridengine.queue;

/**
 * This class represent SGE Queue's state.
 *
 * @version 1.3, 09/22/00
 *
 * @author Rafal Piotrowski
 */
public class QueueState
{
  //=========== attributes ================

  /**
   * Queue state.
   */
  int state;
  /**
   * This is the initial state when a new queue has been created.
   * Valid initial states:
   * - enabled
   * - disabled
   * - default
   */
  String initial_state;

  // Queue states
  public final static int QZERO                     = 0x00000000;
  public final static int QALARM                    = 0x00000001;
  public final static int QSUSPEND_ALARM            = 0x00000002;
  public final static int QDISABLED                 = 0x00000004;
  public final static int QENABLED                  = 0x00000008;
  public final static int QRUNNING                  = 0x00000080;
  public final static int QSUSPENDED                = 0x00000100;
  public final static int QUNKNOWN                  = 0x00000400;
  public final static int QERROR                    = 0x00004000;
  public final static int QSUSPENDED_ON_SUBORDINATE = 0x00008000;
  public final static int QCLEAN                    = 0x00010000;
  public final static int QCAL_DISABLED             = 0x00020000;
  public final static int QCAL_SUSPENDED            = 0x00040000;

  //=========== constructors ================
  
  public QueueState(String initial_state, int state)
    throws QueueStateException
  {
    setInitialState(initial_state);
    setState(state);
  }
  
  public QueueState(int state)
    throws QueueStateException
  {
    initial_state = "default";
    setState(state);
  }
  
  public QueueState(String initial_state)
    throws QueueStateException
  {
    setInitialState(initial_state);
  }

  //============== others ==================

  public String toString()
  {
    StringBuffer sb =
      new StringBuffer("Initial State: ").append(initial_state).append("\nState: ").append(getStateAsString());
    return sb.toString();
  }
  
  //============== getters =================
  
  public String getInitialState() 
  {
    return initial_state;
  }

  public int getState()
  {
    return state;
  }

  public String getStateAsString()
  {
    StringBuffer sb = new StringBuffer();
    
    switch(state) {
    case QZERO:
      sb.append( "Zero" );
      break;
    case QALARM:
      sb.append( "Alarmed" );
      break;
    case QSUSPEND_ALARM:
      sb.append( "Suspended and Alarmed" );
      break;
    case QDISABLED:
      sb.append( "Disabled" );
      break;
    case QENABLED:
      sb.append( "Enabled" );
      break;
    case QRUNNING:
      sb.append( "Running" );
      break;
    case QSUSPENDED:
      sb.append( "Suspended" );
      break;
    case QUNKNOWN:
      sb.append( "Unknown" );
      break;
    case QERROR:
      sb.append( "Error" );
      break;
    case QSUSPENDED_ON_SUBORDINATE:
      sb.append( "Suspended on Subordinate" );
      break;
    case QCLEAN:
      sb.append( "Clean" );
      break;
    case QCAL_DISABLED:
      sb.append( "Cal Disabled" );
      break;
    case QCAL_SUSPENDED:
      sb.append( "Cal Suspended" );
      break;
    default:
      sb.append("N/A");
      break;
    }
    return sb.toString();
  }

  //============== setters =================
  
  public void setInitialState(String initial_state)
    throws QueueStateException
  {
    if( initial_state.equals("enabled") ||
        initial_state.equals("disabled") ||
        initial_state.equals("default") )
      this.initial_state = initial_state;
    else
      throw new QueueStateException("Unsuported initial state. See documentation for valid initial state.");
  }
  
  public void setState(int state)
    throws QueueStateException
  {
    if( state == QZERO ||
        state == QALARM ||
        state == QSUSPEND_ALARM ||
        state == QDISABLED ||
        state == QENABLED ||
        state == QRUNNING ||
        state == QSUSPENDED ||
        state == QUNKNOWN ||
        state == QERROR ||
        state == QSUSPENDED_ON_SUBORDINATE ||
        state == QCLEAN ||
        state == QCAL_DISABLED ||
        state == QCAL_SUSPENDED ) {
      this.state = state;
    } else {
      throw new QueueStateException("Unsuported Queue State. See documentation for available queue states.");
    }
  }
}
