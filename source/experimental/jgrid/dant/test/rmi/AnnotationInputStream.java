/*
 * AnnotationInputStream.java
 *
 * Created on October 28, 2003, 3:02 PM
 */

package dant.test.rmi;

import java.io.*;
import java.util.*;

/**
 *
 * @author  dant
 */
public class AnnotationInputStream extends ObjectInputStream {
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
	private ObjectInputStream in = null;
	
	/** Creates a new instance of AnnotationInputStream */
	public AnnotationInputStream (ObjectInputStream in) {
		this.in = in;
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
		return in.resolveClass (classDesc);
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
