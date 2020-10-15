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

import java.util.*;
import com.sun.grid.cull.*;

/**
 *
 */
public class CullAttr {
   
   public static final String CULL_PRIMARY_KEY = "CULL_PRIMARY_KEY";
   public static final String CULL_READ_ONLY   = "CULL_JGDI_RO";
   public static final String CULL_HIDDEN      = "CULL_JGDI_HIDDEN";
   public static final String CULL_CONFIGURE   = "CULL_JGDI_CONF";
   private CullObject parent;
   private String name;
   private String type;
   private String def;
   private ArrayList params = new ArrayList();
   private boolean primaryKey;
   private boolean readOnly = false;
   private boolean hidden = false;
   private boolean configurable = false;
   
   /** Creates a new instance of CullAttr */
   public CullAttr() {
   }
   
   /**
    * Getter for property name.
    * @return Value of property name.
    */
   public java.lang.String getName() {
      return name;
   }
   
   /**
    * Setter for property name.
    * @param name New value of property name.
    */
   public void setName(java.lang.String name) {
      this.name = name;
   }
   
   /**
    * Getter for property type.
    * @return Value of property type.
    */
   public java.lang.String getType() {
      return type;
   }
   
   /**
    * Setter for property type.
    * @param type New value of property type.
    */
   public void setType(java.lang.String type) {
      this.type = type;
   }
   /**
    * Getter for property default value.
    * @return Value of property type.
    */
   public java.lang.String getDefault() {
      return def;
   }
   
   /**
    * Setter for property default value.
    * @param def New value of property default value
    */
   public void setDefault(java.lang.String def) {
      this.def = def;
   }
   
   
   public void addParam( String param ) {
      getParams().add( param );
      if( CULL_PRIMARY_KEY.equals(param )) {
         setPrimaryKey(true);
      } else if ( CULL_READ_ONLY.equals(param)) {
         setReadOnly(true);
      } else if ( CULL_HIDDEN.equals(param)) {
         setHidden(true);
      } else if ( CULL_CONFIGURE.equals(param)) {
         setConfigurable(true);
      }   
   }
   
   public int getParamCount() {
      return getParams().size();
   }
   
   public String getParam( int i ) {
      return (String)getParams().get(i);
   }
   
   /**
    * Getter for property parent.
    * @return Value of property parent.
    */
   public CullObject getParent() {
      return parent;
   }   
   
   /**
    * Setter for property parent.
    * @param parent New value of property parent.
    */
   public void setParent(CullObject parent) {
      this.parent = parent;
   }

   public ArrayList getParams() {
      return params;
   }

   public void setParams(ArrayList params) {
      this.params = params;
   }

   public boolean isPrimaryKey() {
      return primaryKey;
   }

   public void setPrimaryKey(boolean primaryKey) {
      this.primaryKey = primaryKey;
   }

   public boolean isReadOnly() {
      return readOnly;
   }

   public void setReadOnly(boolean readOnly) {
      this.readOnly = readOnly;
   }

   public boolean isHidden() {
      return hidden;
   }
   
   public void setHidden(boolean hidden) {
      this.hidden = hidden;
   }

   public boolean isConfigurable() {
      return configurable;
   }

   public void setConfigurable(boolean configurable) {
      this.configurable = configurable;
   }
   
}
