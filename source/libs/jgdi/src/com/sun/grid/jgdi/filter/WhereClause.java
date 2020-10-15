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

/**
 *
 */
public class WhereClause {
    
    private String pattern;
    private String type;
    private int field;
    
    /** Creates a new instance of WhereClause */
    public WhereClause(String type, int field, String relop, Object value) {
        if (type == null) {
            throw new IllegalArgumentException("type is NULL");
        }
        if (relop == null) {
            throw new IllegalArgumentException("relop is NULL");
        }
        this.type = type;
        this.field = field;
        StringBuilder buf = new StringBuilder("%T(%I");
        buf.append(relop);
        if(value instanceof Long) {
            buf.append("%u");
        } else if (value instanceof Integer) {
            // buf.append("%d");
            // usually only %u is used for where patterns currently
            buf.append("%u");
        } else if (value instanceof String) {
            buf.append("%s");
        } else {
            throw new IllegalArgumentException("value must be an instance of Long, Integer or String");
        }
        buf.append(")");
        pattern = buf.toString();
    }
    
    public String getPattern() {
        return pattern;
    }
    
    public String getType() {
        return type;
    }
    
    public int getField() {
        return field;
    }
    
}
