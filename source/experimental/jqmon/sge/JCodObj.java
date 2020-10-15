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
package codine;

import java.lang.*;

import jqmon.*;
import jqmon.debug.*;


/**
  * Dies ist die Basisklasse aller Codine-Objekte. Es sollte 
  * nicht noetig sein, direkt eine Instanz von dieser Klasse
  * zu erzeugen.
  * Diese Klasse implementiert eine eindeutige ID, mittels der
  * Codine-Objekte identifiziert werden koennen.
  */

public class JCodObj extends Object implements Cloneable {

	/** Die ID (64 Bit sollten genuegen) */
	private long ID;
	
	/** Die zuletzt benutzte ID */
	private static long lastUsedID;


	/** Wurde das Objekt veraendert? */
	private boolean dirty;

	/** Die Debug-Klasse */
	protected JDebug debug = null;
		
	/* Bevor es ueberhaupt losgeht, die zuletzt benutzte ID 
		auf 0 setzen */
	static {
		lastUsedID = 0;
	}
	
	/** Initialisiert ein CodineObject */
	public JCodObj(JDebug d) {
		super();
		debug = d;
		ID    = getNewID();
		dirty = false;
	}

	/** Liefert die eigene ID zurueck. */
	public long getID() {
		debug.DENTER("JCodineObject.getID");
		debug.DEXIT();
		return ID;
	}

	/** Liefert eine neue, unbenutzte ID zurueck */
	protected static synchronized long getNewID() {
		lastUsedID++;
		return lastUsedID;
	}

	/** Wurde das Objekt geaendert? */
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
