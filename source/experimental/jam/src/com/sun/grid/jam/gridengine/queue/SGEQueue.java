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
 * Summary.
 *
 * @version 1.3, 06/27/00
 *
 * @author Rafal Piotrowski
 */
package com.sun.grid.jam.gridengine.queue;

import  com.sun.grid.jam.gridengine.exechost.SGEExecHost;

/**
 * Summary.
 *
 * @version 1.3, 06/27/00
 *
 * @author Rafal Piotrowski
 */
public class SGEQueue
{

  public final static String QMASTER_NAME = "master.q";
  
  // Attributes
  /**
   * Queue name.
   */
  String name;

  /**
   * Host name where the queue is located.
   */
  String hostname;

  /**
   * Execution host that the queue will use to run its jobs.
   */
  SGEExecHost execHost;
  
  /**
   * Number of job slots.
   */
  long jobSlots;

  /**
   * BATCH, INTERACTIVE, ...
   */
  QueueType type;

  /**
   * This class stores the queue's initial state and a state.
   * QACTIVE, QSUSPENDED, ...
   */
  QueueState state;
  
  /**
   * Temporary working directory.
   * e.g. /tmp
   */
  String tmpDir;

  /**
   * Type of shell to used on this queue.
   * e.g. /bin/sh
   */
  String shell;

  /**
   * Sequence number useed for queue monitoring.
   */
  long seqNo;

  
  //========== constructors =============
  
  public SGEQueue(){}

  public SGEQueue(String name) { this.name = name; }

  public SGEQueue(String name,
                     String hostname,
                     int type,
                     int state,
                     long seqNo,
                     long jobSlots,
                     String tmpDir,
                     String shell)
  {
    this.name = name;
    this.hostname = hostname;
    setType(type);
    setState(state);
    this.seqNo = seqNo;
    this.jobSlots = jobSlots;
    this.tmpDir = tmpDir;
    this.shell = shell;
  }
  
  public SGEQueue(String name,
                     String hostname,
                     QueueType type,
                     QueueState state,
                     long seqNo,
                     long jobSlots,
                     String tmpDir,
                     String shell)
  {
    this.name = name;
    this.hostname = hostname;
    this.type = type;
    this.state = state;
    this.seqNo = seqNo;
    this.jobSlots = jobSlots;
    this.tmpDir = tmpDir;
    this.shell = shell;
  }

  //========= others ============

  public boolean isInteractive()
  {
    return type.containsType(QueueType.IQ);
  }

  public boolean isBatch()
  {
    return type.containsType(QueueType.BQ);
  }
  
  //========= getters ==========
  
  public String getName()
  {
    return name;
  }

  public String getHostName() 
  {
    return hostname;
  }

  public SGEExecHost getExecHost()
  {
    return execHost;
  }

  public QueueType getType()
  {
    return type;
  }

  public QueueState getState()
  {
    return state;
  }

  public long getJobSlots()
  {
    return jobSlots;
  }

  public String getTmpDir() 
  {
    return tmpDir;
  }

  public String getShell()
  {
    return shell;
  }

  public long getSeqNo()
  {
    return seqNo;
  }
  
  //========= setters =============
  
  public void setName(String name) 
  {
    this.name = name;
  }  

  public void setHostName(String hostname)
  {
    this.hostname = hostname;
  }

  public void setExecHost(SGEExecHost execHost)
  {
    this.execHost = execHost;
  }
  
  public void setType(QueueType type)
  {
    this.type = type;
  }

  public void setType(int type)
  {
    this.type = new QueueType(type);
  }  

  public void setState(QueueState state)
  {
    this.state = state;
  }

  public void setState(int state)
  {
    try {
      this.state = new QueueState(state);
    } catch(QueueStateException e) {
      e.printStackTrace();
    }
  }
  
  public void setJobSlots(long jobSlots) 
  {
    this.jobSlots = jobSlots;
  }

  public void setTmpDir(String tmpDir) 
  {
    this.tmpDir = tmpDir;
  }

  public void setShell(String shell)
  {
    this.shell = shell;
  }
  
  public void setSeqNo(long seqNo)
  {
    this.seqNo = seqNo;
  }

}
