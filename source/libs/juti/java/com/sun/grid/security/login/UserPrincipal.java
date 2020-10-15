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
 *  Generic principal for a logged in user.
 */
public class UserPrincipal implements Principal, java.io.Serializable {
   
    private final static long serialVersionUID = -2007121701L;
    
   /**
    * @serial
    */
   private String name;
   
   /**
    * Create a UserPrincipal with a username.
    *
    * <p>
    *
    * @param name the username for this user.
    *
    * @exception NullPointerException if the <code>name</code>
    *			is <code>null</code>.
    */
   public UserPrincipal(String name) {
      if (name == null) {
         throw new NullPointerException("name must not be null");
      }
      this.name = name;
   }
   
   /**
    * Return the username for this <code>UserPrincipal</code>.
    *
    * <p>
    *
    * @return the username for this <code>UserPrincipal</code>
    */
   public String getName() {
      return name;
   }
   
   /**
    * Return a string representation of this <code>UserPrincipal</code>.
    *
    * <p>
    *
    * @return a string representation of this <code>UserPrincipal</code>.
    */
   public String toString() {
      return "UserPrincipal: name=" + name;
   }
   
   /**
    * Compares the specified Object with this <code>UserPrincipal</code>
    * for equality.  Returns true if the given object is also a
    * <code>UserPrincipal</code> and the two UserPrincipals
    * have the same username.
    *
    * <p>
    *
    * @param o Object to be compared for equality with this
    *		<code>UserPrincipal</code>.
    *
    * @return true if the specified Object is equal equal to this
    *		<code>UserPrincipal</code>.
    */
   public boolean equals(Object o) {
      if (o == null) {
         return false;
      }
      
      if (this == o) {
         return true;
      }
      
      if (!(o instanceof UserPrincipal)) {
         return false;
      }
      
      UserPrincipal that = (UserPrincipal)o;
      
      if (this.getName().equals(that.getName())) {
         return true;
      }
      
      return false;
   }
   
   /**
    * Return a hash code for this <code>UserPrincipal</code>.
    *
    * <p>
    *
    * @return a hash code for this <code>UserPrincipal</code>.
    */
   public int hashCode() {
      return name.hashCode();
   }
}
