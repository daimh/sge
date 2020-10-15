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
import java.util.List;
import java.util.Set;

/**
 * This interface represent the result of the qquota algorithm.
 *
 */
public interface QQuotaResult {
    
    /**
     *  Get a set of all resource quota rule names
     *  @return set of resource quota
     */
    public Set<String> getResourceQuotaRuleNames();
    
    /**
     *  Get a resource quotal
     *  @param  quotaname  name of the resource quota
     *  @return the resource quota information
     */
    public ResourceQuotaRuleInfo getResourceQuotaRuleInfo(String quotaname);
    
    /**
     *  Get a list of all available resource quota info objects
     *  @return list of all available resource quota info objects
     */
    public List<ResourceQuotaRuleInfo> getResourceQuotaRules();
    
}
