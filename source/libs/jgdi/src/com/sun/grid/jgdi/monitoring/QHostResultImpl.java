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
package com.sun.grid.jgdi.monitoring;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Default Implemenation of the {@link QHostResult} interface.
 *
 */
public class QHostResultImpl implements QHostResult, Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private Map<String, HostInfo> hostInfoMap = new HashMap<String, HostInfo>();
    private List<HostInfo> hostInfoList = new ArrayList<HostInfo>();
    
    public Set<String> getHostNames() {
        return hostInfoMap.keySet();
    }
    
    public HostInfo createHostInfo(String hostname) {
        HostInfoImpl ret = new HostInfoImpl(hostname);
        hostInfoMap.put(ret.getHostname(), ret);
        return ret;
    }
    
    public void addHostInfo(HostInfo hostInfo) {
        hostInfoMap.put(hostInfo.getHostname(), hostInfo);
    }
    
    public HostInfo getHostInfo(String hostname) {
        return hostInfoMap.get(hostname);
    }
    
    public List<HostInfo> getHostInfo() {
        return new ArrayList(hostInfoMap.values());
    }
    
}
