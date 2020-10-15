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
package com.sun.grid.jgdi.security;

import java.security.Principal;

/**
 *
 */
public class JGDIPrincipal implements Principal, java.io.Serializable {

    private final static long serialVersionUID = -2007121401L;
    
    private final long sessionId;
    private final String name;
    private final String username;

    public final static String SESSION_ID_PREFIX = "(jgdiSessionId=[";
    public final static String SESSION_ID_SUFFIX = "])";
    
    /**
     * Create a JGDIPrincipal with a username.
     *
     * <p>
     *
     * @param username the username for this user.
     *
     * @param sessionId the id of the session
     * @exception NullPointerException if the <code>name</code>
     *			is <code>null</code>.
     */
    public JGDIPrincipal(String username, long sessionId) {
        if (username == null) {
            throw new NullPointerException("username must not be null");
        }
        this.username = username;
        this.sessionId = sessionId;
        
        StringBuilder sb = new StringBuilder();
        sb.append(username);
        sb.append(SESSION_ID_PREFIX);
        sb.append(sessionId);
        sb.append(SESSION_ID_SUFFIX);
        this.name = sb.toString();
    }

    /**
     * Return the username for this <code>JGDIPrincipal</code>.
     *
     * <p>
     *
     * @return the username for this <code>JGDIPrincipal</code>
     */
    public String getName() {
        return name;
    }

    /**
     * Return a string representation of this <code>JGDIPrincipal</code>.
     *
     * <p>
     *
     * @return a string representation of this <code>JGDIPrincipal</code>.
     */
    @Override
    public String toString() {
        return String.format("JGDIPrincipal: %s", name);
    }

    /**
     * Compares the specified Object with this <code>JGDIPrincipal</code>
     * for equality.  Returns true if the given object is also a
     * <code>JGDIPrincipal</code> and the two JGDIPrincipals
     * have the same username.
     *
     * <p>
     *
     * @param o Object to be compared for equality with this
     *		<code>JGDIPrincipal</code>.
     *
     * @return true if the specified Object is equal equal to this
     *		<code>JGDIPrincipal</code>.
     */
    @Override
    public boolean equals(Object o) {
        if (o == null) {
            return false;
        }

        if (this == o) {
            return true;
        }

        if (!(o instanceof JGDIPrincipal)) {
            return false;
        }
        JGDIPrincipal that = (JGDIPrincipal) o;

        if (this.getName().equals(that.getName())) {
            return true;
        }
        return false;
    }

    /**
     * Return a hash code for this <code>JGDIPrincipal</code>.
     *
     * <p>
     *
     * @return a hash code for this <code>JGDIPrincipal</code>.
     */
    @Override
    public int hashCode() {
        return name.hashCode();
    }

    /**
     * Get the session id
     * @return the session id
     */
    public long getSessionId() {
        return sessionId;
    }

    /**
     * Get the username
     * @return the username
     */
    public String getUsername() {
        return username;
    }
    
}
