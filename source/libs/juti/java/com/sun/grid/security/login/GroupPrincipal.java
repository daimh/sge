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
package com.sun.grid.security.login;

import java.security.Principal;

/**
 *  Generic <code>Principal</code> represent a group of users
 */
public class GroupPrincipal  implements
        Principal,
        java.io.Serializable {
   /**
    * @serial
    */
   private String name;
   
   /**
    * @serial
    */
   private boolean primaryGroup;
   
   /**
    * Create a <code>GroupPrincipal</code> using a
    * <code>String</code> representation of the user's
    * group name.
    *
    * <p>
    *
    * @param name the user's group<p>
    *
    * @param primaryGroup true if the specified group represents the
    *			primary group to which this user belongs.
    *
    * @exception NullPointerException if the <code>name</code>
    *			is <code>null</code>.
    */
   public GroupPrincipal(String name, boolean primaryGroup) {
      if (name == null) {
         throw new NullPointerException("name must not be null");
      }
      
      this.name = name;
      this.primaryGroup = primaryGroup;
   }
   
   /**
    * Create a <code>GroupPrincipal</code> using a
    * user's group name.
    *
    * <p>
    *
    * @param name the user's group<p>
    *
    * @param primaryGroup true if the specified group represents the
    *			primary group to which this user belongs.
    *
    */
   public GroupPrincipal(long name, boolean primaryGroup) {
      this.name = (new Long(name)).toString();
      this.primaryGroup = primaryGroup;
   }
   
   /**
    * Return the user's group name for this
    * <code>GroupPrincipal</code>.
    *
    * <p>
    *
    * @return the user's group name this
    *		<code>GroupPrincipal</code>
    */
   public String getName() {
      return name;
   }
   
   /**
    * Return whether this group represents
    * the primary group to which this user belongs.
    *
    * <p>
    *
    * @return true if this name of the group represents
    *		the primary group to which this user belongs,
    *		or false otherwise.
    */
   public boolean isPrimaryGroup() {
      return primaryGroup;
   }
   
   /**
    * Return a string representation of this
    * <code>NumericGroupPrincipal</code>.
    *
    * <p>
    *
    * @return a string representation of this
    *		<code>NumericGroupPrincipal</code>.
    */
   public String toString() {
      if (primaryGroup) {
         return "GroupPrincipal [Primary Group]: name=" + name;
      } else {
         return "GroupPrincipal [Supplementary Group]: name=" + name;
      }
   }
   
   /**
    * Compares the specified Object with this
    * <code>NumericGroupPrincipal</code>
    * for equality.  Returns true if the given object is also a
    * <code>NumericGroupPrincipal</code> and the two
    * NumericGroupPrincipals
    * have the same group identification number (GID).
    *
    * <p>
    *
    * @param o Object to be compared for equality with this
    *		<code>NumericGroupPrincipal</code>.
    *
    * @return true if the specified Object is equal equal to this
    *		<code>NumericGroupPrincipal</code>.
    */
   public boolean equals(Object o) {
      if (o == null) {
         return false;
      }
      
      if (this == o) {
         return true;
      }
      
      if (!(o instanceof GroupPrincipal)) {
         return false;
      }
      GroupPrincipal that = (GroupPrincipal)o;
      
      if (this.getName().equals(that.getName()) &&
              this.isPrimaryGroup() == that.isPrimaryGroup()) {
         return true;
      }
      return false;
   }
   
   /**
    * Return a hash code for this <code>NumericGroupPrincipal</code>.
    *
    * <p>
    *
    * @return a hash code for this <code>NumericGroupPrincipal</code>.
    */
   public int hashCode() {
      return toString().hashCode();
   }
   
}
