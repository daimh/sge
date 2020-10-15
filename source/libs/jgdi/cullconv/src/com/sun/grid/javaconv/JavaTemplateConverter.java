/*___INFO__MARK_BEGIN__*//*************************************************************************
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
import java.util.logging.Logger;

/**
 *
 */
public class JavaTemplateConverter implements JavaToJavaConverter {

    private File outputFile;
    private TemplateFactory fac;
    private File templateFile;
    private String packageName;
    private String classpath;
    private Printer printer;
    private File prologFile;
    private File epilogFile;
    private Logger logger = Logger.getLogger("class");

    /** Creates a new instance of JavaTemplateConverter */
    public JavaTemplateConverter(File buildDir, String classpath, File templateFile, File outputFile, File prologFile, File epilogFile, String javaSourceVersion, String javaTargetVersion) {
        this.templateFile = templateFile;
        this.outputFile = outputFile;
        fac = new TemplateFactory(buildDir, classpath, javaSourceVersion, javaTargetVersion);
        this.prologFile = prologFile;
        this.epilogFile = epilogFile;
        this.printer = null;
    }

    public void convert(Class clazz) throws java.io.IOException {
        Template template = fac.createTemplate(templateFile);

        try {
            if (printer == null) {
                if (!outputFile.exists() || outputFile.lastModified() < templateFile.lastModified() || (prologFile != null && outputFile.lastModified() < prologFile.lastModified()) || (epilogFile != null && outputFile.lastModified() < epilogFile.lastModified())) {
                    printer = new Printer(outputFile);
                    if (prologFile != null) {
                        logger.fine("write prolog " + prologFile);
                        printer.printFile(prologFile);
                    }
                }
            }
            if (printer != null) {
                Map<String, Object> params = new HashMap<String, Object>();
                params.put("shortname", clazz);
                String javaClassName = clazz.getName();
                int index = javaClassName.lastIndexOf(".");
                if (index >= 0) {
                    javaClassName = javaClassName.substring(index + 1);
                }
                params.put("shortname", javaClassName);
                BeanInfo beanInfo = java.beans.Introspector.getBeanInfo(clazz);
                params.put("beanInfo", beanInfo);
                logger.fine("writer class " + clazz.getName());
                template.print(printer, params);
                printer.flush();
            }
        } catch (IntrospectionException itse) {
            IOException e = new IOException("Introspection error: " + itse.getMessage());
            e.initCause(itse);
            throw e;
        }
    }

    public void finish() throws IOException {
        if (printer != null) {
            if (epilogFile != null) {
                logger.fine("write epilog " + epilogFile);
                printer.printFile(epilogFile);
                printer.flush();
            }
            printer.close();
        }
    }
}
