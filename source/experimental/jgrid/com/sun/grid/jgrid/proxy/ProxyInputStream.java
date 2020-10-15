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
 * ProxyInputStream.java
 *
 * Created on June 19, 2002, 4:52 PM
 */

package com.sun.grid.jgrid.proxy;

import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectStreamClass;
import java.io.StreamCorruptedException;
import java.util.HashMap;
import java.util.Map;
import sun.rmi.server.MarshalInputStream;


/** This class provides the ability to retrieve the annotation
 * for any object that it reads.
 * @author Daniel Templeton
 * @version 1.4
 * @deprecated No longer used as of 0.2.1
 * @since 0.1
 */
public class ProxyInputStream extends MarshalInputStream {
	/** A map of the annotations read indexed by class name.
	 */	
	private HashMap annotations = new HashMap ();
	/** Since the annotation is read in a method that doesn't know
	 * the name of the class being read, we have to store the class
	 * name in a method that does know it (resolveClass) so it can
	 * be used in readLocation to enter the annotation into the annotations
	 * Map.
	 */	
	private String classToAnnotate = null;
	
	/** Creates a new instance of ProxyInputStream
	 * @param in the InputStream to wrap
	 * @throws IOException if an error occurs while reading from the stream
	 * @throws StreamCorruptedException if the object stream is invalid
	 */
	public ProxyInputStream (InputStream in) throws IOException, StreamCorruptedException {
		super (in);
	}
	
	/** This method overrides its parent's method to store the class
	 * name in classToAnnotation
	 * @param classDesc the ClassDesc object
	 * @throws IOException if an error occurs while reading from the stream
	 * @throws ClassNotFoundException if no class file can be found for the class being read
	 * @return the resolved class object
	 */	
	protected Class resolveClass (ObjectStreamClass classDesc) throws IOException, ClassNotFoundException {
		classToAnnotate = classDesc.getName ();
		return super.resolveClass (classDesc);
	}
	
	/** This method reads the object annotation from the stream.  It
	 * overrides its parent's method to store the annotation in the
	 * annotations map index by the classToAnnotate.
	 * @throws IOException if an error occurs while reading from the stream
	 * @throws ClassNotFoundException If the class file for the annotation can not be found
	 * @return an object representing the annotation
	 */	
	protected Object readLocation () throws IOException, ClassNotFoundException {
		Object annotation = super.readLocation ();
		annotations.put (classToAnnotate, annotation);

		return annotation;
	}
	
	/** This method returns the Map containing the annotations read
	 * so far, indexed by class name.
	 * @return map of annotations
	 */	
	public Map getAnnotations () {
		return new HashMap (annotations);
	}
}
