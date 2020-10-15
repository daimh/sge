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
package com.sun.grid.cull;

import java.util.HashMap;
import java.util.Map;

/**
 *
 */
public class CullMapAttr extends CullAttr {
   
   private String defaultKey;
   
//   private static Map keyFieldMap = new HashMap();
//   private static Map valueFieldMap = new HashMap();
//   
//   private static void reg(String type, String keyField, String valueField) {
//      keyFieldMap.put(type, keyField);
//      valueFieldMap.put(type, valueField);      
//   }
//   
//   static {      
//      reg("ABOOL_Type", "ABOOL_href", "ABOOL_value");
//   }
   public static final String DEFAULT_KEY_NAME = "key";
   public static final String DEFAULT_VALUE_NAME = "value";
   
   private String keyName = DEFAULT_KEY_NAME;
   private String valueName = DEFAULT_VALUE_NAME;
   
   /** Creates a new instance of CullListAttr */
   public CullMapAttr() {
   }
   
   private CullObject mapType;
   
   public void setMapType(CullObject mapType) {
      this.mapType = mapType;
   }

   public CullAttr getKeyAttr() {
      if(mapType == null) {
         throw new IllegalStateException("mapType not set, did you forget to execute the verify method of CullDefintion?");
      }
      return mapType.getKeyAttr();
   }
   
   public CullAttr getValueAttr() {
      if(mapType == null) {
         throw new IllegalStateException("mapType not set, did you forget to execute the verify method of CullDefintion?");
      }
      return mapType.getValueAttr();
   }

   public String getDefaultKey() {
      return defaultKey;
   }

   public void setDefaultKey(String defaultKey) {
      this.defaultKey = defaultKey;
   }

    public String getKeyName() {
        return keyName;
    }

    public void setKeyName(String keyName) {
        this.keyName = keyName;
    }

    public String getValueName() {
        return valueName;
    }

    public void setValueName(String valueName) {
        this.valueName = valueName;
    }
   
   
}
