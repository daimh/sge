/*
 * DRMException.java
 *
 * Created on May 23, 2003, 4:40 PM
 */

package com.sun.grid.jgrid.proxy;

import com.sun.grid.jgrid.JGridException;

/** This class signals a error while talking to the Sun ONE Grid Engine directly.
 * @author dan.templeton@sun.com
 * @version 1.3
 * @since 0.1
 */
public class DRMException extends JGridException {
	
	/**
	 * Creates a new instance of <code>SGEException</code> without detail message.
	 */
	public DRMException () {
	}
	
	
	/**
	 * Constructs an instance of <code>SGEException</code> with the specified detail message.
	 * @param msg the detail message.
	 */
	public DRMException (String msg) {
		super (msg);
	}
}
