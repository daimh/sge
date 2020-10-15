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
import jcodine.*;


public class JComplex extends JCodObj implements Cloneable {
	
	public String            name 				= null;
	public JComplexEntryList complexEntryList = null;
   
	public  Codine.Complex complex = null;

	public JComplex(JDebug debug) {
		super(debug);
	}
	
	public JComplex(JDebug debug, Codine.Complex complex, org.omg.CORBA.Context ctx) {
		this(debug);
		this.complex = complex;
		try {
			name = complex.get_name(ctx);
			Codine.ComplexEntry[] complexEntryArray = complex.get_entries(ctx);
			complexEntryList = new JComplexEntryList();
			complexEntryList.init(debug, complexEntryArray);
		}
		catch(Exception e) {
			// shouldn't exist
		}
	}

	

	public void setName(String name) {
		this.name = name;
	}

	public String getName() {
		return name;
	}
	
	public void setComplexEntryList(JComplexEntryList complexEntryList) {
		this.complexEntryList = complexEntryList;
	}

	public JComplexEntryList getComplexEntryList() {
		return complexEntryList;
	}
	
	public void setComplex(Codine.Complex complex) {
		this.complex = complex;
	}

	public Codine.Complex getComplex() {
		return complex;
	}

	public String toString() {
		return name;
	} 	
		
	//public JComplexEntryList getEntries() {

	//public void setEntries(JComplexEntryList cel) {

	public Object clone() {
                JComplex c = null;
                try {
                        c = (JComplex)super.clone();
                } catch (CloneNotSupportedException e) {
                        debug.DPRINTF("Error in JComplex.java: " + e);
                }
 
                return (Object)c;
        }

}

