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
 * Default Implemenation of the {@link QQuotaResult} interface.
 *
 */
public class QQuotaResultImpl implements QQuotaResult, Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private Map<String, ResourceQuotaRuleInfo> resourceQuotaRuleInfoMap = new HashMap<String, ResourceQuotaRuleInfo>();
    private List<ResourceQuotaRuleInfo> resourceQuotaRuleInfoList = new ArrayList<ResourceQuotaRuleInfo>();
    
    public Set<String> getResourceQuotaRuleNames() {
        return resourceQuotaRuleInfoMap.keySet();
    }
    
    public ResourceQuotaRuleInfo createResourceQuotaRuleInfo(String resouceQuotaRuleName) {
        ResourceQuotaRuleInfoImpl ret = new ResourceQuotaRuleInfoImpl(resouceQuotaRuleName);
        resourceQuotaRuleInfoMap.put(ret.getResouceQuotaRuleName(), ret);
        return ret;
    }
    
    public void addResourceQuotaRuleInfo(ResourceQuotaRuleInfo resouceQuotaRuleInfo) {
        resourceQuotaRuleInfoMap.put(resouceQuotaRuleInfo.getResouceQuotaRuleName(), resouceQuotaRuleInfo);
    }
    
    public ResourceQuotaRuleInfo getResourceQuotaRuleInfo(String resourceQuotaRuleName) {
        return resourceQuotaRuleInfoMap.get(resourceQuotaRuleName);
    }
    
    public List<ResourceQuotaRuleInfo> getResourceQuotaRules() {
        return new ArrayList(resourceQuotaRuleInfoMap.values());
    }
    
}
