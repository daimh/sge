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
 * JCEPOutputStream.java
 *
 * Created on May 19, 2003, 3:25 PM
 */

package com.sun.grid.jgrid.server;

import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.OutputStream;

/** The JCEPOutputStream is used by the Job object to write itself to disk when
 * asked to checkpoint.  It uses the Job's codebase property to annotate the
 * serialized objects.
 * @author dan.templeton@sun.com
 * @version 1.3
 * @since 0.2
 */
public class JCEPOutputStream extends ObjectOutputStream {
	/** The codebase to use for annotating the serialized objects */	
	private String annotation = null;
	
	/** Creates a new instance of JCEPOutputStream that wrap the given OutputStream.
	 * @param out The OutputStream to wrap
	 * @param annotation The codebase to use for annotating the serialized objects
	 * @throws IOException Thrown if there's an error creating the stream
	 */
	public JCEPOutputStream (OutputStream out, String annotation) throws IOException {
		super (out);
		this.annotation = annotation;
	}
	
	/** Writes the annotation to the serialized object
	 * @param cl The Class object for the object being serialized
	 * @throws IOException Thrown when there's an error writing the annotation to the stream
	 */	
	protected void annotateClass (Class cl) throws IOException {
		this.writeObject (annotation);
	}
}
