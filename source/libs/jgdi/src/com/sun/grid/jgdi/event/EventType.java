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
package com.sun.grid.jgdi.event;

/**
 *
 */
public class EventType implements java.io.Serializable {
    
    private int type;
    
    private static final int SGE_EMA_LIST_TYPE = 1;
    private static final int SGE_EMA_ADD_TYPE  = 2;
    private static final int SGE_EMA_MOD_TYPE  = 3;
    private static final int SGE_EMA_DEL_TYPE  = 4;
    private static final int SGE_EMA_TRIGGER_TYPE = 5;
    
    public static final EventType SGE_EMA_LIST = new EventType(SGE_EMA_LIST_TYPE);
    public static final EventType SGE_EMA_ADD  = new EventType(SGE_EMA_ADD_TYPE);
    public static final EventType SGE_EMA_MOD  = new EventType(SGE_EMA_MOD_TYPE);
    public static final EventType SGE_EMA_DEL  = new EventType(SGE_EMA_DEL_TYPE);
    public static final EventType SGE_EMA_TRIGGER = new EventType(SGE_EMA_TRIGGER_TYPE);
    
    public EventType(int type) {
        this.type = type;
    }
    
    
    public final int getType() {
        return type;
    }
    
    public int hashCode() {
        return type;
    }
    
    public boolean equals(Object obj) {
        return (obj instanceof EventType) &&
                type == ((EventType)obj).type;
    }
    
    public String toString() {
        switch(type) {
            case SGE_EMA_LIST_TYPE: return "LIST";
            case  SGE_EMA_ADD_TYPE: return "ADD";
            case  SGE_EMA_MOD_TYPE: return "MOD";
            case  SGE_EMA_DEL_TYPE: return "DEL";
            case SGE_EMA_TRIGGER_TYPE: return "TRIGGER";
            default:
                return "UNKNOWN(" + type + ")";
        }
    }
    
}
