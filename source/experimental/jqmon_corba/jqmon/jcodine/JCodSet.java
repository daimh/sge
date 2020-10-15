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

import java.util.*;


public class JCodSet {

	private Vector ambiguousSet = null;
	private Vector modifiedSet  = null;
	
	public JCodSet() {
		ambiguousSet = new Vector(10,10);
		modifiedSet  = new Vector(10,10);
	}

	// marks the specified field as modified 
	public void SetModified(int Fieldname) {
		Integer wert = new Integer(Fieldname);
		if ( !(modifiedSet.contains(wert)) ) { 
			modifiedSet.addElement(wert);
		}
	}

   // clears the modified flag of all fields
   public void ClearModified() {
      modifiedSet.removeAllElements();
   }

	// clears the modified flag of the specified field 
	public void ClearModified(int Fieldname) {
		modifiedSet.removeElement(new Integer(Fieldname));
	}

	// returns true if the specified field has been modified 
	public boolean isModified(int Fieldname) {
		if ( modifiedSet.contains(new Integer(Fieldname)) ) {
			return true;
		} else {
			return false;
		}
	}

	// returns true if any field has been modified 
	public boolean isModified() {
		return !(modifiedSet.isEmpty());
	}

	// determines the specified field as ambiguous 
	public void SetAmbiguous(int Fieldname) {
		Integer wert = new Integer(Fieldname);
		if ( !(ambiguousSet.contains(wert)) ) { 
			ambiguousSet.addElement(wert);
		}
	}

   // clears the ambiguous flag of all fields 
   public void ClearAmbiguous() {
      ambiguousSet.removeAllElements();
   }
   
	// clears the ambiguous flag of the specified field 
	public void ClearAmbiguous(int Fieldname) {
		ambiguousSet.removeElement(new Integer(Fieldname));
	}

	// returns true if the specified field is ambiguous 
	public boolean isAmbiguous(int Fieldname) {
		if ( ambiguousSet.contains(new Integer(Fieldname)) ) {
			return true;
		} else {
			return false;
		}
	}

	// returns true if any field is ambiguous 
	public boolean isAmbiguous() {
		return !(ambiguousSet.isEmpty());
	}
}


