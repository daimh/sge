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
import Codine.*;
 
 
 
public class JComplexEntry extends JCodObj implements Cloneable {

	private String    name 		   = null;
	private String    shortcut 	= null;
	private String    stringval 	= null;
	private int       valtype		= 0;
	private int       relop		   = 0;
	private boolean   request		= false;
	private boolean   consumable	= false;
	private boolean   forced		= false;	

	public JComplexEntry(JDebug debug) {
                super(debug);
        }

	public JComplexEntry(JDebug debug, ComplexEntry complexEntry) {
		this(debug);
		name 			= complexEntry.name;
		shortcut 	= complexEntry.shortcut;
		stringval	= complexEntry.stringval;
		valtype		= complexEntry.valtype;
		relop			= complexEntry.relop;
		request		= complexEntry.request;
		consumable	= complexEntry.consumable;
		forced		= complexEntry.forced;
	}

	public String toString() {
                return name;
        }
	
	public Object clone() {
                JComplexEntry ce = null;
                try {
                        ce = (JComplexEntry)super.clone();
                } catch (CloneNotSupportedException e) {
                        debug.DPRINTF("Error in JComplexEntry.java: " + e);
                }
 
                return (Object)ce;
        }
	
	public void setName(String name){
		this.name = name;
	}

	public String getName() {
		return name;
	}
	
	public void setShortcut(String shortcut) {
		this.shortcut = shortcut;
	}

	public String getShortcut() {
		return shortcut;
	}

	public void setStringval(String stringval) {
		this.stringval = stringval;
	}

	public String getStringval(){
		return stringval;
	}

	public void setValtype(int valtype){
		this.valtype = valtype;
	}
	
	public int getValtype() {
		return valtype;
	}

	public void setRelop(int relop) {
		this.relop = relop;
	}

	public int getRelop() {
		return relop;
	}

	public void setRequest(boolean request) {
		this.request = request;
	}
	
	public boolean getRequest() {
		return request;
	}
	
	public void setConsumable(boolean consumable) {
		this.consumable = consumable;
	}

	public boolean getConsumable() {
		return consumable;
	}
	
	public void setForced(boolean forced) {
		this.forced = forced;
	}

	public boolean getForced() {
		return forced;
	}

}
