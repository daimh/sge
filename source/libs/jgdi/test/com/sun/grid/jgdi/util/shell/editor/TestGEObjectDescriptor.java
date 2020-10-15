//*___INFO__MARK_BEGIN__*/
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


package com.sun.grid.jgdi.util.shell.editor;

import com.sun.grid.jgdi.configuration.*;
import com.sun.grid.jgdi.CullConstants;
import com.sun.grid.jgdi.configuration.reflect.GEObjectDescriptor;
import com.sun.grid.jgdi.configuration.reflect.InvalidObjectException;
import com.sun.grid.jgdi.configuration.reflect.PropertyDescriptor;
import com.sun.grid.jgdi.util.shell.editor.TestGEObject;

public class TestGEObjectDescriptor extends GEObjectDescriptor {
    
    
    public TestGEObjectDescriptor() {
        this(TestGEObject.class, "CQ_Type" );
        setImplClass(TestGEObject.class);
    }
    
    protected TestGEObjectDescriptor(Class type, String name) {
        super(type, name);
        
        
        PropertyDescriptor propDescr = null;
        
        propDescr = addSimple("name", String.class, "SGE_STRING", CullConstants.CQ_name, true, false, true);
        propDescr = addSimple("long", Long.TYPE, "AULNG_Type", CullConstants.CQ_seq_no, true, false, true);
        propDescr = addSimple("double", Double.TYPE, "SGE_DOUBLE", CullConstants.JAT_oticket, false, false, true);
        //LIST
        propDescr = addList("stringList", String.class, "HR_Type", CullConstants.CQ_hostlist, true, false, true );
        ///MAP
        propDescr = addMap("stringMap", String.class, "ATIME_Type", String.class,
                CullConstants.CQ_s_cpu, CullConstants.ATIME_href, CullConstants.ATIME_value,
                "0", false, true );
        //MAPLIST
        propDescr = addMapList("stringMapList", String.class, "ASTRLIST_Type", String.class, "ST_Type",
                CullConstants.CQ_pe_list, CullConstants.ASTRLIST_href, CullConstants.ASTRLIST_value,
                "NONE",false, true );
      /*propDescr = addMapList("loadThresholds", ComplexEntry.class, "ACELIST_Type", String.class, "CE_Type",
             CullConstants.CQ_load_thresholds, CullConstants.ACELIST_href, CullConstants.ACELIST_value,
             "NONE",false, true );*/
    }
    
    public void validate(Object obj) throws InvalidObjectException {
        if( !(obj instanceof ClusterQueue) ) {
            throw new InvalidObjectException(obj, "obj is not an instanceof ClusterQueue");
        }
    }
}

