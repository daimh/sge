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
 * Computable.java
 *
 * Created on May 21, 2002, 5:04 PM
 */

package com.sun.grid.jgrid;

import java.io.Serializable;


/** This interface marks a class that can be executed by the
 * compute engine.
 * @author Daniel Templeton
 * @version 1.4
 * @since 0.1
 */
public abstract interface Computable extends Serializable {
	/** This method causes this class to execute
	 * @return the results object
	 * @param job The Job object associated with this job.  The Job object is used
	 * to access additional information about the job, such as job id and
	 * properties and is used to access the compute engine logging mechanism.
	 * @throws ComputeException if an error occurs during execution
	 */	
	public abstract Serializable compute (Job job) throws ComputeException;
	/** This method is used to tell the job that a checkpoint is imminent
	 * and that it should save its state now.  The checkpoint will not
	 * occur until this method returns.  For tasks that have to complete
	 * a cycle before being interrupted, this method should block until
	 * the task can be interrupted and the state saved.
	 * @throws NotInterruptableException Thrown when the implemetation does not allow the job to be interrupted for
	 * checkpointing.
	 */	
	public abstract void checkpoint () throws NotInterruptableException;
	/** This method cancels a running job.  It does not need to wait for the job
	 * to stop before returning.  It only needs to set into certain motion the
	 * process of halting the job.
	 * @throws NotInterruptableException Thrown when the implemetation does not allow the job to be interrupted for
	 * cancelation.
	 */	
	public abstract void cancel () throws NotInterruptableException;
	/** This methods suspend a running job.  It does not need to wait for the job
	 * to stop before returning.  It only needs to set into certain motion the
	 * process of suspending the job.
	 * @throws NotInterruptableException Thrown when the implemetation does not allow the job to be interrupted for
	 * suspension.
	 */
	public abstract void suspend () throws NotInterruptableException;
	/** This method resumes a suspended job.  If called on a running job, the
	 * method call will have no effect. */
	public abstract void resume ();
}