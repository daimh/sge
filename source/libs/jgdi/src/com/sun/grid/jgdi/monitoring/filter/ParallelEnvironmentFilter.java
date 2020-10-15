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
import java.util.LinkedList;
import java.util.Collections;
import java.util.List;

/**
 *
 */
public class ParallelEnvironmentFilter implements Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private List<String> peList = new LinkedList<String>();
    
    /** Creates a new instance of ParallelEnvironmentFilter */
    public ParallelEnvironmentFilter() {
    }
    
    public static ParallelEnvironmentFilter parse(String str) {
        ParallelEnvironmentFilter ret = new ParallelEnvironmentFilter();
        return ret.fill(str);
    }
    
    public ParallelEnvironmentFilter fill(String list) {
        String[] elems = list.split(",");
        for (String elem : elems) {
            addPE(elem);
        }
        return this;
    }
    
    public void addPE(String peName) {
        peList.add(peName);
    }
    
    public List getPEList() {
        return Collections.unmodifiableList(peList);
    }
    
    public int getPECount() {
        return peList.size();
    }
    
    @Override
    public String toString() {
        StringBuilder ret = new StringBuilder();
        boolean first = true;
        ret.append("ParallelEnvironmentFilter[");
        for (String pe : peList) {
            if (first) {
                first = false;
            } else {
                ret.append(", ");
            }
            ret.append(pe);
        }
        ret.append("]");
        return ret.toString();
    }
}
