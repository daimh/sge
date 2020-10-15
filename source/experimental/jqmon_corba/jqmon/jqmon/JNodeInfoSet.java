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
import java.util.*;



public class JNodeInfoSet  extends  Vector {

	public JNodeInfoSet() {
   }
   
   
   public int GetType() {
      return ((JNodeInfo)firstElement()).type;
   }
   
   
   public void Add(JNodeInfo nodeinfo) {
      if ( FindByObject(nodeinfo) == null ) {
         addElement(nodeinfo);      
      }
   }
   
   
	public void Delete (JNodeInfo nodeinfo) {
     JNodeInfo ni = FindByObject(nodeinfo);
     if (ni != null) {
        removeElement(nodeinfo);
     }
   }
  
   
  public JNodeInfo FindByObject (JNodeInfo nodeinfo) {
      Enumeration e = elements();
      
      while (e.hasMoreElements() ) {
         JNodeInfo ni = (JNodeInfo)e.nextElement();   
         if (nodeinfo.path.equals(ni.path) && (nodeinfo.type == ni.type) && (nodeinfo.ID == ni.ID) ) {
            return ni;
         }
      }
      return null;
   }
   
   
	public JNodeInfo FindByID (long ID) {
      Enumeration e = elements();
      
      while (e.hasMoreElements() ) {
         JNodeInfo ni = (JNodeInfo)e.nextElement();   
         if (ni.ID == ID) {
            return ni;
         }
      }
      return null;
   }
   
   
	public boolean Contains (long ID) {
      Enumeration e = elements();
      
      while (e.hasMoreElements() ) {
         JNodeInfo ni = (JNodeInfo)e.nextElement();   
         if (ni.ID == ID) {
            return true;
         }
      }
      return false;
   }
   
   
	public boolean ContainsOtherType (int nType) {
      Enumeration e = elements();
      
      while (e.hasMoreElements() ) {
         JNodeInfo ni = (JNodeInfo)e.nextElement();   
         if (ni.type != nType) {
            return true;
         }
      }
      return false;
   }
   
	public void ClearTag() {
      Enumeration e = elements();
      
      while (e.hasMoreElements() ) {
         JNodeInfo ni = (JNodeInfo)e.nextElement();   
         ni.tag = false;
      }
   }
   
   
	public void ClearTag(long ID) {
      JNodeInfo ni = FindByID(ID);
      if (ni != null) {
         ni.tag = false;
      }
   }
   
   
	public void SetTag() {
      Enumeration e = elements();
      
      while (e.hasMoreElements() ) {
         JNodeInfo ni = (JNodeInfo)e.nextElement();   
         ni.tag = true;
      }
   }
   
   
	public void SetTag(long ID) {
      JNodeInfo ni = FindByID(ID);
      if (ni != null) {
         ni.tag = true;
      }
   }
   
   
	public boolean IsTagSet(long ID) {
      JNodeInfo ni = FindByID(ID);
      if (ni != null) {
         return ni.tag;
      }
      return false;
   }

}
