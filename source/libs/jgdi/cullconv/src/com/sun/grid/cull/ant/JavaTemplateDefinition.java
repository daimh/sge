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
package com.sun.grid.cull.ant;

import com.sun.grid.cull.CullJavaTemplateConverter;
import java.io.File;
import java.util.StringTokenizer;
import org.apache.tools.ant.BuildException;

/**
 *
 */
public class JavaTemplateDefinition extends JavaConverterDefinition {
   
   private File template;
   private String classSuffix;
   
   private CullConvAntTask cullConvAntTask;

   private String packagename;
   
   /** Creates a new instance of JavaTemplateDefinition */
   public JavaTemplateDefinition(CullConvAntTask cullConvAntTask) {
      super(cullConvAntTask.getProject());
      this.cullConvAntTask = cullConvAntTask;
   }

   
   public File getTemplate() {
      return template;
   }

   public void setTemplate(File template) {
      this.template = template;
   }

   public com.sun.grid.cull.CullConverter createConverter() throws BuildException {

      CullJavaTemplateConverter ret = new CullJavaTemplateConverter(cullConvAntTask.getBuildDir(),
                                                                    cullConvAntTask.getClassPathAsString(),
                                                                    getOutputDir(), 
                                                                    template,
                                                                    cullConvAntTask.getSource(),
                                                                    cullConvAntTask.getTarget());
      
      ret.setPackagename(getPackagename());
      
      if(classSuffix != null ) {
         ret.setClassSuffix(classSuffix);
      }
      
      return ret;
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
   
}
