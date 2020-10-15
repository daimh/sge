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
 * ComputeException.java
 *
 * Created on May 21, 2002, 4:59 PM
 */

package com.sun.grid.jgrid;

/** This class represents an exception that occurs during the
 * processing of a job.
 * @author Daniel Templeton
 * @version 1.3
 * @since 0.1
 */
public class ComputeException extends JGridException {
	/**
	 * Creates new <code>ComputeException</code> without detail message.
	 */
	public ComputeException() {
	}
	
	/**
	 * Constructs an <code>ComputeException</code> with the specified detail message.
	 * @param msg the detail message.
	 */
	public ComputeException(String msg) {
		super(msg);
	}
}


