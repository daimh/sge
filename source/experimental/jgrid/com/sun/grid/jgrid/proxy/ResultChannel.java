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
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*
 * ResultChannel.java
 *
 * Created on May 22, 2002, 2:48 PM
 */

package com.sun.grid.jgrid.proxy;

import java.io.Serializable;
import java.rmi.Remote;
import java.rmi.RemoteException;

/** This class is the interface used by ComputeEngine's to send
 * the results of a job back to the ComputeProxy.
 * @author Daniel Templeton
 * @version 1.4
 * @see ComputeProxy
 * @see com.sun.grid.jgrid.server.JCEPHandler
 * @since 0.1
 */
public interface ResultChannel extends Remote {
	/** The port on which the registry housing the ResultChannel will
	 * be started.
	 */	
	public static final int PORT = 1100;
	/** The name under which the ResultChannel will be registered.
	 */	
	public static final String LOOKUP_NAME = "com.sun.grid.jgrid.ResultChannel";
	/** This method sends the results of a job to the ComputeProxy.
	 * @param result the results object
	 * @param processId the id of the job
	 * @throws RemoteException if an error occurs on the server
	 */	
	public abstract void sendResult (Serializable result, String processId) throws RemoteException;
	/** This method sends the execption resulting from a job to the
	 * ComputeProxy.
	 * @param e the exception
	 * @param processId the id of the job
	 * @throws RemoteException if an error occurs on the server side
	 */	
	public abstract void sendException (Exception e, String processId) throws RemoteException;
}