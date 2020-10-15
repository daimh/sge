/*___INFO__MARK_BEGIN__*/
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
/*___INFO__MARK_END__*/
package jcodine;

import java.lang.*;

import jqmon.*;
import jqmon.debug.*;


// this is the base class of the jcodine-objects.  
// it should not be instatiated
 
// the class provides an unique ID to identify the objects.

public class JCodObj extends Object implements Cloneable {

	// the ID (64 bit should suffice) 
	private long ID;
	
	// the ID last used 
	private static long lastUsedID;


	// did the object change ? 
	private boolean dirty;

	// the debug-class 
	protected JDebug debug = null;
		
	// set the last used ID to zero, before we start  
	static {
		lastUsedID = 0;
	}
	
	// initialize the object 
	public JCodObj(JDebug d) {
		super();
		debug = d;
		ID    = getNewID();
		dirty = false;
	}

	// returns the ID 
	public long getID() {
		debug.DENTER("JCodineObject.getID");
		debug.DEXIT();
		return ID;
	}

	// returns the new unused ID 
	protected static synchronized long getNewID() {
		lastUsedID++;
		return lastUsedID;
	}

	// did the object change ? 
	public boolean hasChanged() {
		return dirty;
	}

	public void hasChanged(boolean changed) {
		dirty = changed;
	}

	public Object clone() throws CloneNotSupportedException {
		JCodObj c = (JCodObj)super.clone();
		return (Object)c;
	}
}
