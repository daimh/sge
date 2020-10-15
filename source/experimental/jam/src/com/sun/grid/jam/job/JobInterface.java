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
import java.io.Reader;
import java.rmi.RemoteException;
import net.jini.admin.*;
import com.sun.grid.jam.util.JAMProxy;

/**
 * Define the interface for a Job service
 *
 * @version 1.7, 09/22/00
 *
 * @author Nello Nellari
 *
 */
public interface JobInterface
  extends JAMProxy
{

//   /**
//    * Returns the job name assigned by the user.
//    * @return    the job name
//    * @exception RemoteException
//    */
//   String getName()
//     throws RemoteException;

//   /**
//    * Returns the job ID assigned in the submission
//    * procedure.
//    * @return    the job ID
//    * @exception RemoteException
//    */
//   String getID()
//     throws RemoteException;

//   /**
//    * Returns the job status.
//    *
//    * @return    the job status
//    * @exception RemoteException
//    */
//   String getStatus()
//     throws RemoteException;

//   /**
//    * Returns the ID of the user.
//    * 
//    * @return    the user ID
//    * @exception RemoteException
//    */
//   String getUserID()
//     throws RemoteException;

//   /**
//    * Returns the name of the queue
//    * where the job has been submitted.
//    * 
//    * @return    the queue name
//    * @exception RemoteException
//    */
//   String getQueueName()
//     throws RemoteException;

//   /**
//    * Returns the output message.
//    * Usually when the job has been completed.
//    * 
//    * @return    the output message
//    * @exception RemoteException
//    */
//   String getOutputData()
//     throws RemoteException;

//   /**
//    * Returns the error message.
//    * 
//    * @return    the error message
//    * @exception RemoteException
//    */
//   String getErrorMesg()
//     throws RemoteException;

//   /**
//    * Returns the reader for job output data.
//    * 
//    * @return    the output reader
//    * @exception RemoteException
//    */
//   Reader getOutputReader()
//     throws RemoteException;

//   /**
//    * Returns the reader for job error data.
//    * 
//    * @return    the error reader
//    * @exception RemoteException
//    */
//   Reader getErrorReader()
//     throws RemoteException;

  /**
   * Sends a suspend request for the job.
   * 
   * @exception RemoteException
   */
  void suspend()
    throws RemoteException;

  /**
   * Sends a resume request for the job.
   * 
   * @exception RemoteException
   */
  void resume()
    throws RemoteException; 

  /**
   * Sends a kill request for the job.
   * 
   * @exception RemoteException
   */
  void kill()
    throws RemoteException;

  /**
   * Sends a stop request for the job.
   * 
   * @exception RemoteException
   */
  void stop()
    throws RemoteException;
  
  /**
   * Sends a start request for the job.
   * 
   * @exception RemoteException
   */
  void start()
    throws RemoteException;

  /**
   * Sends a restart request for the job.
   * 
   * @exception RemoteException
   */
  void restart()
    throws RemoteException;
}



