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

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 *
 */
public class CullFile {

   private File  source;
   private List  cullObjs = new ArrayList();
   private List  nameSpaces = new ArrayList();
   
   /** Creates a new instance of CullDefinition */
   public CullFile() {
   }
   
   public File getSource() {
      return source;
   }
   
   public void setSource(File file) {
      this.source = source;
   }
   
   public void addCullObject(CullObject obj) {
      cullObjs.add(obj);
   }
   
   public int getCullObjectCount() {
      return cullObjs.size();
   }
   
   public CullObject getCullObject(int i) {
      return (CullObject)cullObjs.get(i);
   }
   
   public void addNameSpace(CullNameSpace ns) {
      nameSpaces.add(ns);
   }

   public int getNameSpaceCount() {
      return nameSpaces.size();
   }
   
   public CullNameSpace getNameSpace(int i) {
      return (CullNameSpace)nameSpaces.get(i);
   }
   
   private ArrayList enums = new ArrayList();
   
   public void addEnum(CullEnum aEnum) {
      enums.add(aEnum);
   }

   public int getEnumCount() {
      return enums.size();
   }
   public CullEnum getEnum(int index) {
      return (CullEnum)enums.get(index);
   }
   

}
