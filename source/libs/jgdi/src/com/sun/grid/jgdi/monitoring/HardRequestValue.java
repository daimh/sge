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
 *
 */
public class HardRequestValue implements Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private String resource;
    private String value;
    private double uc;
    
    /**
     *  Create an new instanceof a HardRequestValue
     *  @param resource the name of the resource
     *  @param value  requested value of the resource
     *  @param uc     urgency contribution of the resource
     */
    public HardRequestValue(String resource, String value, double uc) {
        this.resource = resource;
        this.value = value;
        this.uc = uc;
    }
    
    /**
     *  Get the resource
     *  @return the resource
     */
    public String getResource() {
        return resource;
    }
    
    /**
     *  Get the requested value of the resource
     *  @return the requested value
     */
    public String getValue() {
        return value;
    }
    
    /**
     *  Get the urgency contribution  of the resource
     *  @return the urgency contribution
     */
    public double getContribution() {
        return uc;
    }
    
    
}
