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
package com.sun.grid.jam.queue.entry;

/**
 * Defines the status of a Queue service.
 *
 * @version 1.4, 09/22/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 * @author Eric Sharakan
 *
 * @see QueueInfo
 */
public class QueueStatus
  implements java.io.Serializable
{
  public static final QueueStatus QUEUE_OPEN = new QueueStatus("OPEN");
  public static final QueueStatus QUEUE_CLOSE = new QueueStatus("CLOSE");
  public static final QueueStatus QUEUE_ENABLED = new QueueStatus("ENABLED");
  public static final QueueStatus QUEUE_DISABLED = new QueueStatus("DISABLED");
  
  public String status;
   
  public QueueStatus()
  {
    super();
  }

  public QueueStatus(String st)
  {
    status = st;
  }

  public QueueStatus(com.sun.grid.jam.gridengine.queue.QueueState st)
  {
    switch(st.getState()) {
    case com.sun.grid.jam.gridengine.queue.QueueState.QZERO:
      status = new String( "ENABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QALARM:
      status = new String( "DISABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QSUSPEND_ALARM:
      status = new String( "DISABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QDISABLED:
      status = new String( "DISABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QENABLED:
      status = new String( "ENABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QRUNNING:
      status = new String( "ENABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QSUSPENDED:
      status = new String( "DISABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QUNKNOWN:
      status = new String( "DISABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QERROR:
      status = new String( "DISABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QSUSPENDED_ON_SUBORDINATE:
      status = new String( "DISABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QCLEAN:
      status = new String( "ENABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QCAL_DISABLED:
      status = new String( "DISABLED" );
      break;
    case com.sun.grid.jam.gridengine.queue.QueueState.QCAL_SUSPENDED:
      status = new String( "DISABLED" );
      break;
    default:
      status = new String("DISABLED");
      break;
    }
  }
}
