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
import java.util.logging.*;

/**
 *
 */
public class CullDefinitionFilterProxy extends CullDefinition {
   
   private static Logger logger = Logger.getLogger("cullconv");
   
   
   
   private CullDefinition cullDef;
   private Set filteredCullObjectNameSet;
   private  CullDefinitionFilter[] filters;
   
   /** Creates a new instance of CullDefinitionFilter */
   public CullDefinitionFilterProxy(CullDefinition cullDef, CullDefinitionFilter[] filters) {
      this.cullDef = cullDef;
      this.filters  = filters;
   }

   public void setPackageName(String packageName) {
      super.setPackageName(packageName);
   }

   public java.io.File getSource(String name) {
      return cullDef.getSource(name);
   }

   public void addFile(String file) throws java.io.IOException, ParseException {
      cullDef.addFile(file);
   }

   public com.sun.grid.cull.CullNameSpace getNameSpace(String name) {
      return cullDef.getNameSpace(name);
   }

   public com.sun.grid.cull.CullObject getCullObject(String name) {
      return cullDef.getCullObject(name);
   }

   public java.io.File getNameSource(String name) {
      return cullDef.getNameSource(name);
   }

   public void addCullObject(com.sun.grid.cull.CullObject obj, java.io.File source) {
      cullDef.addCullObject(obj, source);
   }

   public void addNameSpace(com.sun.grid.cull.CullNameSpace obj, java.io.File source) {

      cullDef.addNameSpace(obj, source);
   }

   public void addFile(java.io.File file) throws java.io.IOException, ParseException {

      cullDef.addFile(file);
   }

   public String getPackageName() {
      return cullDef.getPackageName();
   }

   public java.util.Set getObjectNames() {
      if( this.filteredCullObjectNameSet == null ) {
          
         filteredCullObjectNameSet = new HashSet();
         for(int i = 0; i < filters.length; i++) {
             Set filteredObjs = filters[i].getObjectNames(cullDef);
             filteredCullObjectNameSet.addAll(filteredObjs);
         }
      }
      return Collections.unmodifiableSet(filteredCullObjectNameSet);
   }

   public java.util.Set getNameSpaceNameSet() {
      return cullDef.getNameSpaceNameSet();
   }

   protected com.sun.grid.cull.CullDefinition.Elem createElem() {
      return cullDef.createElem();
   }
}
