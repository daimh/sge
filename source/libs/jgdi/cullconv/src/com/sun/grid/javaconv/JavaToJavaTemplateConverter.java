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

import com.sun.grid.cull.Printer;
import com.sun.grid.cull.template.Template;
import com.sun.grid.cull.template.TemplateFactory;
import java.beans.BeanInfo;
import java.beans.IntrospectionException;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 *
 */
public class JavaToJavaTemplateConverter extends AbstractJavaToJavaConverter {
   
   private TemplateFactory fac;
   private File templateFile;
   private String packageName;
   private String classpath;
   private File   outputDir;
   private String classSuffix;
   
   /** Creates a new instance of CullTemplateConverter */
   public JavaToJavaTemplateConverter(File buildDir, String classpath, File outputDir, String packageName,
                                    File template, String classSuffix, String fileSuffix,
                                    String javaSourceVersion, String javaTargetVersion) {     
      super(outputDir, packageName, fileSuffix);
      fac = new TemplateFactory(buildDir, classpath, javaSourceVersion, javaTargetVersion );
      this.templateFile = template;
      this.classSuffix = classSuffix;
   }
   

   public void convert(Class clazz) throws java.io.IOException {
      
      Template template = fac.createTemplate(templateFile);
      
      Map params = new HashMap();
      
      params.put("class", clazz);
         
      String javaClassName = clazz.getName();
      
      int index = javaClassName.lastIndexOf(".");
      if(index >= 0 ) {
         javaClassName = javaClassName.substring(index+1);
      }
      if(classSuffix != null) {
         javaClassName += classSuffix;
      }
      
      params.put("shortname", javaClassName);
      
      try {
         BeanInfo beanInfo = java.beans.Introspector.getBeanInfo(clazz);

         params.put("beanInfo", beanInfo);

         File f = super.getFileForClass(javaClassName);

         if( !f.exists() || f.lastModified() < templateFile.lastModified() ) { 


            Printer p  = new Printer(f);

            template.print(p, params );
            p.flush();
            p.close();
         }
      } catch(IntrospectionException itse) {
         IOException e = new IOException("Introspection error: " + itse.getMessage());
         e.initCause(itse);
         throw e;
      }
   }
}
