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
 * JCEPInputStream.java
 *
 * Created on May 17, 2003, 5:11 PM
 */

package com.sun.grid.jgrid.server;

import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectStreamClass;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;

/** This class is used to read a serialized Job object that was written to disk with
 * annotation for where to find its class data.
 * @author dan.templeton@sun.com
 * @version 1.5
 * @since 0.2
 */
public class JCEPInputStream extends ObjectInputStream {
	/** The last class data URL read from an object annotation by resolveClass(). */	
	private String lastAnnotation = null;
	
	/** Creates a new instance of JCEPInputStream
	 * @throws IOException Thrown when an error occurs while creating the stream
	 */
	public JCEPInputStream () throws IOException {
		super ();
	}
	
	/** Creates an instance of JCEPInputStream that wraps the given InputStream.
	 * @param in The InputStream to wrap
	 * @throws IOException Thrown when an error occurs while creating the stream
	 */	
	public JCEPInputStream (InputStream in) throws IOException {
		super (in);
	}
	
	/** Used to retreive the class file appropriate for a serialized object.  To find
	 * the class file, JCEPInputStream reads an annotation from the serialized object
	 * which contains the URL where the class data can be found.  If the annotation is
	 * null or if the class file can be found in the classpath, the classfile is loaded
	 * from the default ClassLoader.
	 * @param desc The object describing the class to be found and loaded
	 * @throws IOException Thrown if there's an error reading the annotation
	 * @throws ClassNotFoundException Thrown if the class described by desc cannot be found in the classpath or, if
	 * present, in the URL given by the annotation
	 * @throws MalformedURLException Thrown if the URL given in the annotation is not a valid URL
	 * @return A Class object for the class described by desc
	 */	
	protected Class resolveClass (ObjectStreamClass desc) throws IOException, ClassNotFoundException, MalformedURLException {
		try {
			return super.resolveClass (desc);
		}
		catch (ClassNotFoundException e) {
			String codebase = (String)this.readObject ();

			if ((codebase != null) && !codebase.equals ("")) {
				String className = desc.getName ();

				lastAnnotation = codebase;

				/* I create the ClassLoader locally and don't cache it because I want
				 * to be able to load a different version of the class each time.  If I
				 * reused the same ClassLoader, it would complain about version mismatches.
				 * Optimally, I should cache the ClassLoader and the last modified time
				 * for the class data and then trash the ClassLoader if the class data
				 * has been modified.  But that's a lot of work, and right now it's pretty
				 * low on the priority list. */
				/* I should also note this this ability to dynamically reload jobs whose
				 * class files have changed only applies to classes not in the classpath. */
				URLClassLoader loader = new URLClassLoader (new URL[] {new URL (codebase)});

				return loader.loadClass (className);
			}
			else {
				throw e;
			}
		}		
	}	
	
	/** Used to retreive the last annotation read in resolveClass().  This is needed to
	 * set the codebase property of the Job object that is being deserialized.
	 * @return The last read annotation
	 * @see com.sun.grid.jgrid.Job#setAnnotation(String)
	 */	
	public String getLastAnnotation () {
		return lastAnnotation;
	}
}