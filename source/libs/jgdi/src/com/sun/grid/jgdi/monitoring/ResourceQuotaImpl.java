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

/**
 * Resource Limit
 *
 */
public class ResourceQuotaImpl implements ResourceQuota, Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private String resourceName;
    private String limitValue;
    private String usageValue;
    
    /**
     * Create a new resource quota info object
     */
    public ResourceQuotaImpl() {}
    
    /**
     * Create a new limit rule info object
     * @param name  name of the resource quota
     */
    public ResourceQuotaImpl(String name) {
        setName(name);
    }
    
    /**
     * Create a new resource quota
     * @param name  name of the resource quota
     * @param limitValue the limit value
     * @param usageValue the usage value
     */
    public ResourceQuotaImpl(String name, String limitValue, String usageValue) {
        setName(name);
        setLimitValue(limitValue);
        setLimitValue(usageValue);
    }
    
    /**
     *  Get the resource name
     *  @return the resource name
     */
    public String getName() {
        return this.resourceName;
    }
    
    /**
     *  Set the resource name
     *  @param name the resource name
     */
    public void setName(String name) {
        this.resourceName = name;
    }
    
    /**
     *  Get the limit value
     *  @return the limit value
     */
    public String getLimitValue() {
        return this.limitValue;
    }
    
    /**
     *  Set the limit value
     *  @param limitValue the limit value
     */
    public void setLimitValue(String limitValue) {
        this.limitValue = limitValue;
    }
    
    /**
     *  Get the usage value
     *  @return the usage value
     */
    public String getUsageValue() {
        return this.usageValue;
    }
    
    /**
     *  Set the usage value
     *  @param usageValue the usage value
     */
    public void setUsageValue(String usageValue) {
        this.usageValue = usageValue;
    }
    
}
