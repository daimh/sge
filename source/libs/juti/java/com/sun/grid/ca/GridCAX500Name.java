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
package com.sun.grid.ca;

/**
 * Helper class for parsing X500 names.
 *
 * Daemon certificates have the form
 *
 * <pre>
 *   ....,UID=sdm_daemon_&lt;username&gt;,CN=<daemon name>,...
 * </pre>
 *
 * User certificates have the form
 *
 * <pre>
 *   ....,UID=&lt;username&gt;,...
 * </pre>
 *
 */
public class GridCAX500Name {
    
    public final static String TYPE_SGE_DAEMON = "sge_daemon";
    public final static String TYPE_SDM_DAEMON = "sdm_daemon";
    public final static String TYPE_USER       = "user";
    
    public static final String SDM_DAEMON_PREFIX = TYPE_SDM_DAEMON  + '_';
    private boolean daemon;
    private String  username;
    private String  daemonName;
    
    private GridCAX500Name(String name) throws GridCAException {
        String uid = getField(name, "UID");
        if(uid.startsWith(SDM_DAEMON_PREFIX)) {
            daemon = true;
            username = uid.substring(SDM_DAEMON_PREFIX.length());
            daemonName = getField(name, "CN");
        } else {
            daemon = false;
            username = uid;
            daemonName = null;
        }
    }
    
    private static String getField(String name, String field) throws GridCAException {
        
        String prefix = field + "=";
        int start = name.indexOf(prefix);
        
        if(start < 0) {
            RB.newGridCAException("x500.fieldNotFound", new Object[] { field, name });
        }
        start +=prefix.length();
        int end = name.indexOf(',', start);
        if(end < 0) {
            end = name.length();
        }
        return name.substring(start, end);
    }
    
    /**
     * Parse a x500 name
     * @param name   the x500 name
     * @throws com.sun.grid.ca.GridCAException if <code>name</code> is not a valid grid ca x500 name
     * @return the grid ca x500 name
     */
    public static GridCAX500Name parse(String name) throws GridCAException {
        return new GridCAX500Name(name);
    }

    /**
     * Determine of the x500 name describes a daemon
     * @return  <code>true</code> if the x500 name describes a daemon
     */
    public boolean isDaemon() {
        return daemon;
    }

    /**
     * Get the username from the x500 name
     * @return the name of the user
     */
    public String getUsername() {
        return username;
    }

    /**
     * Get the name of the daemon
     * @return the name of the damon or <code>null</code> of the name
     *         does not describe a daemon
     */
    public String getDaemonName() {
        return daemonName;
    }
}
