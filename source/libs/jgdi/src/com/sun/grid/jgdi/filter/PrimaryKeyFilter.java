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
package com.sun.grid.jgdi.filter;

import java.util.LinkedList;
import java.util.Collections;
import java.util.List;

/**
 *
 */
public class PrimaryKeyFilter implements JGDIFilter {
    
    private List<WhereClause> fields = new LinkedList<WhereClause>();
    private String  type;
    private String equalRelop = "==";
    
    /** Creates a new instance of PrimaryKeyFilter */
    public PrimaryKeyFilter(String type) {
        this.type = type;
        if (type.equals("CONF_Type") || type.equals("EH_Type")) {
            equalRelop = "h=";
        }
    }
    
    public void include(int field, String value) {
        fields.add(new StringWhereClause(type, field, equalRelop, value));
    }
    
    public void include(int field, int value) {
        fields.add(new IntWhereClause(type, field, "==", value));
    }
    
    public void exclude(int field, String value) {
        fields.add(new StringWhereClause(type, field, "!=", value));
    }
    
    public void exclude(int field, int value) {
        fields.add(new IntWhereClause(type, field, "!=", value));
    }
    
    public String getType() {
        return type;
    }
    
    public List getFields() {
        return Collections.unmodifiableList(fields);
    }
}
