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

import com.sun.grid.cull.AbstractCullConverter;
import com.sun.grid.cull.CullDefinition;
import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.DirectoryScanner;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.Task;
import org.apache.tools.ant.types.FileSet;
import org.apache.tools.ant.types.Path;

/**
 *
 */
public class CullConvAntTask extends Task {

    private FileSet cullfiles = null;

    private File buildDir;

    private List<ConverterDefinition> converterList = new ArrayList<ConverterDefinition>();

    private String packageName;

    private Path classpath;

    private String source = "1.5";
    private String target = "1.5";

    /** Creates a new instance of CullConvAntTask */
    public CullConvAntTask() {
        super();
    }


    public Path createClasspath() {
        if (classpath == null) {
            classpath = new Path(getProject());
        }
        return classpath;
    }

    String getClassPathAsString() {
        return classpath.toString();
    }

    /**
     * Create the fileset fo the cull files
     * @param p the project to use to create the path in
     * @return a path to be configured
     */
    public FileSet createCullfiles() {
        if (cullfiles == null) {
            cullfiles = new FileSet();
        }
        return cullfiles;
    }

    public JavaConverterDefinition createJavaconv() {
        JavaConverterDefinition conv = new JavaConverterDefinition(getProject());
        converterList.add(conv);
        return conv;
    }

    public JavaTemplateDefinition createJavatemplateconv() {
        JavaTemplateDefinition conv = new JavaTemplateDefinition(this);
        converterList.add(conv);
        return conv;
    }

    public TemplateConverterDefinition createTemplateconv() {
        TemplateConverterDefinition conv = new TemplateConverterDefinition(this);
        converterList.add(conv);
        return conv;
    }


    private CullDefinition parseFiles() throws BuildException {

        DirectoryScanner ds = cullfiles.getDirectoryScanner(getProject());

        File baseDir = ds.getBasedir();

        CullDefinition ret = new CullDefinition();

        String[] files = ds.getIncludedFiles();

        for (int i = 0; i < files.length; i++) {
            File f = new File(baseDir, files[i]);
            try {
                log("parse file " + f, Project.MSG_VERBOSE);
                ret.addFile(f);
            } catch (IOException ioe) {
                throw new BuildException("I/O Error while parsing file " + f, ioe);
            } catch (com.sun.grid.cull.ParseException e) {
                throw new BuildException("Parse Error: " + f + ": " + e.getMessage(), e);
            }
        }
        int errorCount = ret.verify();
        if (errorCount > 0) {
            throw new BuildException("cull defintion contains " + errorCount + " errors");
        }
        return ret;
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


        CullDefinition def = parseFiles();

        def.setPackageName(packageName);

        for (ConverterDefinition conv : converterList) {
            try {
                CullDefinition filteredDef = conv.createFilteredCullDefinition(def);
                conv.createConverter().convert(filteredDef);
            } catch (BuildException be) {
                throw be;
            } catch (IOException ioe) {
                throw new BuildException("converter " + conv.getClassname() + " failed: " + ioe.getMessage(), ioe);
            } catch (RuntimeException e) {
                StringWriter sw = new StringWriter();
                PrintWriter pw = new PrintWriter(sw);
                e.printStackTrace(pw);
                pw.flush();
                logger.severe("Runtime error in converter " + conv.getClassname());
                logger.severe(sw.getBuffer().toString());
                throw new BuildException("converter " + conv.getClassname() + " failed: " + e.getMessage(), e);
            }
        }
    }

    public String getPackageName() {
        return packageName;
    }

    public void setPackageName(String packageName) {
        this.packageName = packageName;
    }

    public File getBuildDir() {
        return buildDir;
    }

    public void setBuildDir(File buildDir) {
        this.buildDir = buildDir;
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
