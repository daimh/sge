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

import com.sun.grid.cull.AbstractCullConverter;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import com.sun.grid.cull.ant.*;
import com.sun.grid.javaconv.JavaToJavaConverter;
import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;
import org.apache.tools.ant.types.Path;

/**
 *
 */
public class JavaConvAntTask extends Task {

    private File buildDir;

    private List<AbstractConverterDefinition> converterList = new ArrayList<AbstractConverterDefinition>();

    private Path classpath;

    private String classname;

    private ClassFilter classdef;

    private String source = " converters and ";
    private String target = " classes";

    public ClassFilter createClassDef() {
        if (classdef == null) {
            classdef = new ClassFilter(this);
        }
        return classdef;
    }

    String getClassPathAsString() {
        return classdef.getClassPathAsString();
    }

    public JavaTemplateConverterDefintion createTemplateconv() {
        JavaTemplateConverterDefintion conv = new JavaTemplateConverterDefintion(this);
        converterList.add(conv);
        return conv;
    }

    public TemplateConverterDefinition createSimpleTemplateConv() {
        TemplateConverterDefinition conv = new TemplateConverterDefinition(this);
        converterList.add(conv);
        return conv;
    }


    public void execute() throws org.apache.tools.ant.BuildException {

        Logger logger = AbstractCullConverter.logger;

        Handler[] handler = logger.getHandlers();
        for (int i = 0; i < handler.length; i++) {
            logger.removeHandler(handler[i]);
        }

        logger.setUseParentHandlers(false);
        AntLoggingHandler myHandler = new AntLoggingHandler(getProject());
        logger.addHandler(myHandler);
        logger.setLevel(Level.ALL);
        myHandler.setLevel(Level.ALL);


        try {
            Class[] classes = classdef.getClasses();
            JavaToJavaConverter[] conv = new JavaToJavaConverter[converterList.size()];
            int i = 0;
            for (AbstractConverterDefinition convDef : converterList) {
                conv[i++] = convDef.createConverter();
            }

            logger.fine("has " + conv.length + " converters and " + classes.length + " classes");
            for (int convIndex = 0; convIndex < conv.length; convIndex++) {
                for (int classIndex = 0; classIndex < classes.length; classIndex++) {
                    conv[convIndex].convert(classes[classIndex]);
                }
                conv[convIndex].finish();
            }
        } catch (ClassNotFoundException cnfe) {
            throw new BuildException("a class was not found: " + cnfe.getMessage(), cnfe);
        } catch (IOException ioe) {
            throw new BuildException("IO/Error: " + ioe.getMessage(), ioe);
        }
    }


    public File getBuildDir() {
        return buildDir;
    }

    public void setBuildDir(File buildDir) {
        this.buildDir = buildDir;
    }

    public String getClassname() {
        return classname;
    }

    public String getSource() {
        return source;
    }

    public void setSource(String source) {
        this.source = source;
    }

    public String getTarget() {
        return target;
    }

    public void setTarget(String target) {
        this.target = target;
    }
}
