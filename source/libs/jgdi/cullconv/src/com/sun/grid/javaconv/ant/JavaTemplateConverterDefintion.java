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
package com.sun.grid.javaconv.ant;
import com.sun.grid.javaconv.JavaToJavaConverter;
import com.sun.grid.javaconv.JavaToJavaTemplateConverter;
import java.io.File;
import org.apache.tools.ant.Project;

/**
 *
 */
public class JavaTemplateConverterDefintion extends AbstractConverterDefinition  {
   
   private File outputDir;
   private File templateFile;
   private String packageName;
   private String classSuffix;
   private String fileSuffix = "java";
   
   private JavaConvAntTask javaConvAntTask;
   private Project project;
   
   /** Creates a new instance of TemplateConverterDefinition */
   public JavaTemplateConverterDefintion(JavaConvAntTask javaConvAntTask) {
      this.project = project;
      this.javaConvAntTask = javaConvAntTask;
   }


   public File getTemplateFile() {
      return templateFile;
   }

   public void setTemplateFile(File templateFile) {
      this.templateFile = templateFile;
   }
   
   public JavaToJavaConverter createConverter() throws org.apache.tools.ant.BuildException {
      return new JavaToJavaTemplateConverter(javaConvAntTask.getBuildDir(), javaConvAntTask.getClassPathAsString(), outputDir, 
                                             packageName, templateFile, classSuffix, fileSuffix,
                                             javaConvAntTask.getSource(), javaConvAntTask.getTarget());
   }

   public File getOutputDir() {
      return outputDir;
   }

   public void setOutputDir(File outputDir) {
      this.outputDir = outputDir;
   }

   public String getPackageName() {
      return packageName;
   }

   public void setPackageName(String packageName) {
      this.packageName = packageName;
   }

   public String getClassSuffix() {
      return classSuffix;
   }

   public void setClassSuffix(String classSuffix) {
      this.classSuffix = classSuffix;
   }

   public String getFileSuffix() {
      return fileSuffix;
   }

   public void setFileSuffix(String fileSuffix) {
      this.fileSuffix = fileSuffix;
   }
   
   
}
