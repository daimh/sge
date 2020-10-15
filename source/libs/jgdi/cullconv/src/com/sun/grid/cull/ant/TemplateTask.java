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

import com.sun.grid.cull.Printer;
import com.sun.grid.cull.template.Template;
import com.sun.grid.cull.template.TemplateFactory;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Handler;
import java.util.logging.Logger;
import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;
import org.apache.tools.ant.types.Path;

/**
 *
 */
public class TemplateTask extends Task {

    private File buildDir;
    private File template;
    private String classname;
    private File outputFile;
    private Path classpath;
    private String source = "1.4";
    private String target = "1.4";

    /** Creates a new instance of TemplateTask */
    public TemplateTask() {
        super();
    }

    public File getBuildDir() {
        return buildDir;
    }

    public void setBuildDir(File buildDir) {
        this.buildDir = buildDir;
    }

    public void execute() throws org.apache.tools.ant.BuildException {

        Logger logger = Logger.getLogger("cullconv");

        Handler[] handler = logger.getHandlers();
        for (int i = 0; i < handler.length; i++) {
            logger.removeHandler(handler[i]);
        }

        logger.setUseParentHandlers(false);
        AntLoggingHandler myHandler = new AntLoggingHandler(getProject());

        logger.addHandler(myHandler);

        if (classpath == null) {
            throw new BuildException("classpath not set");
        }
        try {
            TemplateFactory fac = new TemplateFactory(buildDir, classpath.toString(), source, target);
            Template t = fac.createTemplate(template);

            Printer p = null;

            if (getOutputFile() == null) {
                p = new Printer();
            } else {
                p = new Printer(getOutputFile());
            }




            Map<String, String> paramMap = new HashMap<String, String>();
            for (Param param : params) {
                paramMap.put(param.getName(), param.getValue());
            }
            t.print(p, paramMap);
            p.flush();
        } catch (IOException ioe) {
            throw new BuildException("I/O Error: " + ioe.getMessage(), ioe);
        }
    }

    private ArrayList<Param> params = new ArrayList<Param>();

    private Param createParam() {
        Param ret = new Param();
        params.add(ret);
        return ret;
    }

    static class Param {
        private String name;
        private String value;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getValue() {
            return value;
        }

        public void setValue(String value) {
            this.value = value;
        }
    }

    public File getOutputFile() {
        return outputFile;
    }

    public void setOutputFile(File outputFile) {
        this.outputFile = outputFile;
    }

    public File getTemplate() {
        return template;
    }

    public void setTemplate(File template) {
        this.template = template;
    }

    public String getClassname() {
        return classname;
    }

    public void setClassname(String classname) {
        this.classname = classname;
    }

    public Path createClasspath() {
        if (classpath == null) {
            classpath = new Path(getProject());
        }
        return classpath;
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
