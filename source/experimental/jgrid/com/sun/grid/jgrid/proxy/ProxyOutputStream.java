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
 * ProxyOutputStream.java
 *
 * Created on June 19, 2002, 4:53 PM
 */

package com.sun.grid.jgrid.proxy;

import java.io.IOException;
import java.io.OutputStream;
import java.io.StreamCorruptedException;
import sun.rmi.server.MarshalOutputStream;


/** This class allows the annotation for classes being written
 * to the stream to be explicitly set.
 * @author Daniel Templeton
 * @version 1.3
 * @since 0.1
 */
public class ProxyOutputStream extends MarshalOutputStream {
	/** The annotation to be used when writing objects to the stream.
	 */	
	private String annotation = null;
	
	/** Creates a new instance of ProxyOutputStream
	 * @param out the OutputStream to wrap
	 * @throws IOException if an error occurs while writing to the stream
	 * @throws StreamCorruptedException if the object stream is invalid
	 */
	public ProxyOutputStream (OutputStream out) throws IOException, StreamCorruptedException {
		super (out);
	}
	
	/** This method sets the annotation to be used.
	 * @param annotation The annotation to use
	 */	
	public void setAnnotation (String annotation) {
		this.annotation = annotation;
	}
	
	/** This method overrides its parent's method to write the
	 * annotation object specified regardless of what class is
	 * being written.
	 * @param cl the class to annotate
	 * @throws IOException if an error occurs while writing to the stream
	 */	
	protected void annotateClass (Class cl) throws IOException {
		//We use the URL annotation that was set
		writeLocation (annotation);
	}
}
