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

import com.sun.grid.javaconv.JavaTemplateConverter;
import com.sun.grid.javaconv.JavaToJavaConverter;
import java.io.File;
import org.apache.tools.ant.Project;


/**
 *
 */
public class TemplateConverterDefinition extends AbstractConverterDefinition {
   
   private File outputFile;
   private File templateFile;
   private File prologFile;
   private File epilogFile;
   private String packageName;
   private String classSuffix;
   private String fileSuffix = "java";
   
   private JavaConvAntTask javaConvAntTask;
   private Project project;
   
   /** Creates a new instance of TemplateConverterDefinition */
   public TemplateConverterDefinition(JavaConvAntTask javaConvAntTask) {
      this.project = project;
      this.javaConvAntTask = javaConvAntTask;
   }

   public File getOutputFile() {
      return outputFile;
   }

   public void setOutputFile(File outputFile) {
      this.outputFile = outputFile;
   }

   public File getTemplateFile() {
      return templateFile;
   }

   public void setTemplateFile(File templateFile) {
      this.templateFile = templateFile;
   }
   
   public JavaToJavaConverter createConverter() throws org.apache.tools.ant.BuildException {
      return new JavaTemplateConverter(javaConvAntTask.getBuildDir(), javaConvAntTask.getClassPathAsString(), 
                                       templateFile, outputFile, prologFile, epilogFile,
                                       javaConvAntTask.getSource(), javaConvAntTask.getTarget());
                                       
   }

   public File getPrologFile() {
      return prologFile;
   }

   public void setPrologFile(File prologFile) {
      this.prologFile = prologFile;
   }

   public File getEpilogFile() {
      return epilogFile;
   }

   public void setEpilogFile(File epilogFile) {
      this.epilogFile = epilogFile;
   }
   
   
}
