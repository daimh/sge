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
package com.sun.grid.javaconv;

import java.io.File;
import java.io.IOException;

/**
 *
 */
public abstract class AbstractJavaToJavaConverter implements JavaToJavaConverter {

   private File   outputDir = new File(".");
   private String packagename;
   
   private String classSuffix;
   private String fileSuffix;
   
   public AbstractJavaToJavaConverter(String packageName) {
      this(new File("."), packageName);
   }
   
   public AbstractJavaToJavaConverter(File outputDir, String packageName) {
      this(outputDir, packageName, "java");
   }
   
   /** Creates a new instance of AbstractCallToJavaConverter */
   public AbstractJavaToJavaConverter(File outputDir, String packageName, String fileSuffix) {
      this.outputDir = outputDir;
      this.packagename = packageName;
      this.fileSuffix = fileSuffix;
   }

   public File getOutputDir() {
      return outputDir;
   }

   public void setOutputDir(File outputDir) {
      this.outputDir = outputDir;
   }
   
   public File getFileForClass(String className) {  

      String name = getPackagename();

      if( name != null ) {
        name = name + "." + className; 
        name = name.replace('.', File.separatorChar );
      } else {
        name = className;
      }
      
      if( classSuffix != null ) {
         name += classSuffix;
      }
      if(fileSuffix == null) {
         fileSuffix = "java";
      }
      name += "." + fileSuffix;
      
      return new File( getOutputDir(), name );
   }
   
   public String getClassSuffix() {
      return classSuffix;
   }

   public void setClassSuffix(String classSuffix) {
      this.classSuffix = classSuffix;
   }

   public String getPackagename() {
      return packagename;
   }

   public void setPackagename(String packagename) {
      this.packagename = packagename;
   }

   public String getFileSuffix() {
      return fileSuffix;
   }

   public void setFileSuffix(String fileSuffix) {
      this.fileSuffix = fileSuffix;
   }
   
   public void finish() throws IOException {
   }
   
}
