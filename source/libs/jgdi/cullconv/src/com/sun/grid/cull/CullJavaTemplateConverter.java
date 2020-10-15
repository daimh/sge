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

import com.sun.grid.cull.template.Template;
import com.sun.grid.cull.template.TemplateFactory;
import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 *
 */
public class CullJavaTemplateConverter extends AbstractCullToJavaConverter {

    private TemplateFactory fac;
    private File templateFile;

    /** Creates a new instance of CullTemplateConverter */
    public CullJavaTemplateConverter(File buildDir, String classpath, File outputDir, File template, String javaSourceVersion, String javaTargetVersion) {
        fac = new TemplateFactory(buildDir, classpath, javaSourceVersion, javaTargetVersion);
        super.setOutputDir(outputDir);
        this.templateFile = template;
    }


    public void convert(CullDefinition cullDef) throws java.io.IOException {

        Template template = fac.createTemplate(templateFile);
        JavaHelper javaHelper = new JavaHelper(cullDef);

        if (getPackagename() == null) {
            setPackagename(cullDef.getPackageName());
        }

        javaHelper.setPackageName(getPackagename());
        Map params = new HashMap();

        params.put("javaHelper", javaHelper);
        params.put("cullDef", cullDef);

        for (String name : cullDef.getObjectNames()) {
            CullObject obj = cullDef.getCullObject(name);
            params.put("cullObj", obj);
            String javaClassName = javaHelper.getNonPrimitiveClassname(obj);
            File f = super.getFileForClass(javaClassName);
            if (!f.exists() || f.lastModified() < templateFile.lastModified()) {
                Printer p = new Printer(f);
                template.print(p, params);
                p.flush();
                p.close();
            }
        }
    }
}
