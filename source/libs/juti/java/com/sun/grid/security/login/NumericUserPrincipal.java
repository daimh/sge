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
 *
 */
public class NumericUserPrincipal implements
        Principal,
        java.io.Serializable {
   /**
    * @serial
    */
   private String name;
   
   /**
    * Create a <code>NumericUserPrincipal</code> using a
    * <code>String</code> representation of the
    * user's identification number (UID).
    *
    * <p>
    *
    * @param name the user identification number (UID) for this user.
    *
    * @exception NullPointerException if the <code>name</code>
    *			is <code>null</code>.
    */
   public NumericUserPrincipal(String name) {
      if (name == null) {
         throw new NullPointerException("name must not be null");
      }
      this.name = name;
   }
   
   /**
    * Create a <code>NumericUserPrincipal</code> using a
    * long representation of the user's identification number (UID).
    *
    * <p>
    *
    * @param name the user identification number (UID) for this user
    *			represented as a long.
    */
   public NumericUserPrincipal(long name) {
      this.name = (new Long(name)).toString();
   }
   
   /**
    * Return the user identification number (UID) for this
    * <code>NumericUserPrincipal</code>.
    *
    * <p>
    *
    * @return the user identification number (UID) for this
    *		<code>NumericUserPrincipal</code>
    */
   public String getName() {
      return name;
   }
   
   /**
    * Return the user identification number (UID) for this
    * <code>NumericUserPrincipal</code> as a long.
    *
    * <p>
    *
    * @return the user identification number (UID) for this
    *		<code>NumericUserPrincipal</code> as a long.
    */
   public long longValue() {
      return ((new Long(name)).longValue());
   }
   
   /**
    * Return a string representation of this
    * <code>NumericUserPrincipal</code>.
    *
    * <p>
    *
    * @return a string representation of this
    *		<code>NumericUserPrincipal</code>.
    */
   public String toString() {
      return "NumericUserPrincipal: name=" + name;
   }
   
   /**
    * Compares the specified Object with this
    * <code>NumericUserPrincipal</code>
    * for equality.  Returns true if the given object is also a
    * <code>NumericUserPrincipal</code> and the two
    * NumericUserPrincipals
    * have the same user identification number (UID).
    *
    * <p>
    *
    * @param o Object to be compared for equality with this
    *		<code>NumericUserPrincipal</code>.
    *
    * @return true if the specified Object is equal equal to this
    *		<code>NumericUserPrincipal</code>.
    */
   public boolean equals(Object o) {
      if (o == null) {
         return false;
      }
      
      if (this == o) {
         return true;
      }
      
      if (!(o instanceof NumericUserPrincipal)) {
         return false;
      }
      NumericUserPrincipal that = (NumericUserPrincipal)o;
      
      if (this.getName().equals(that.getName()))
         return true;
      return false;
   }
   
   /**
    * Return a hash code for this <code>NumericUserPrincipal</code>.
    *
    * <p>
    *
    * @return a hash code for this <code>NumericUserPrincipal</code>.
    */
   public int hashCode() {
      return name.hashCode();
   }
}