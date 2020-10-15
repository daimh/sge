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
 * JobCanceledException.java
 *
 * Created on October 16, 2003, 2:25 PM
 */

package com.sun.grid.jgrid;

/** This exception is used as a placeholder to pass back to through the result
 * channel to notify the client that the job has been canceled before completing.
 * @author dan.templeton@sun.com
 * @version 1.4
 * @since 0.2
 */
public class JobCanceledException extends JGridException {
	
	/**
	 * Creates a new instance of <code>JobCanceledException</code> without detail message.
	 */
	public JobCanceledException () {
	}
	
	
	/**
	 * Constructs an instance of <code>JobCanceledException</code> with the specified detail message.
	 * @param msg the detail message.
	 */
	public JobCanceledException (String msg) {
		super (msg);
	}
}
