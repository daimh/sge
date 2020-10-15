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
package com.sun.grid.jgdi.monitoring.filter;

import java.io.Serializable;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.StringTokenizer;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 */
public class HostFilter implements Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private static Logger logger = Logger.getLogger(HostFilter.class.getName());
    private List<String> hostList = new LinkedList<String>();

    /** Creates a new instance of HostFilter */
    public HostFilter() {
    }

    public static HostFilter parse(String hosts) {
        HostFilter ret = new HostFilter();
        StringTokenizer st = new StringTokenizer(hosts, ",");
        while (st.hasMoreTokens()) {
            ret.addHost(st.nextToken());
        }
        return ret;
    }

    public void addHost(String hostname) {
        if (logger.isLoggable(Level.FINE)) {
            logger.fine("add host " + hostname + " to filter");
        }
        hostList.add(hostname);
    }

    public List getHosts() {
        logger.fine("get hosts");
        return Collections.unmodifiableList(hostList);
    }

    @Override
    public String toString() {
        StringBuilder ret = new StringBuilder();
        boolean first = true;
        ret.append("Hosts[");
        for (String host : hostList) {
            if (first) {
                first = false;
            } else {
                ret.append(", ");
            }
            ret.append(host);
        }
        ret.append("]");
        return ret.toString();
    }
}