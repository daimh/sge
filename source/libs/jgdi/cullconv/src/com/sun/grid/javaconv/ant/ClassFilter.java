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

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.types.Path;
import org.apache.tools.ant.types.PatternSet;
import org.apache.tools.ant.util.ClasspathUtils;

/**
 *
 */
public class ClassFilter {

    private ClasspathUtils.Delegate cpDelegate;

    private Project project;
    private Logger logger = Logger.getLogger("cullconv");

    /** Creates a new instance of ClassFilter */
    public ClassFilter(JavaConvAntTask task) {
        this.project = task.getProject();
        this.cpDelegate = ClasspathUtils.getDelegate(task);
    }

    public Path createClasspath() {
        return this.cpDelegate.createClasspath();
    }




    public void addClasses(PackageWrapper packageDef, List classesList) throws ClassNotFoundException {
        File directory = null;
        ClassLoader cld = cpDelegate.getClassLoader();
        if (cld == null) {
            throw new ClassNotFoundException("Can't get class loader.");
        }

        String packagename = packageDef.getName();

        try {
            String path = '/' + packagename.replace('.', '/');

            logger.fine("searchin for resource " + path + " in classpath " + getClassPathAsString());
            URL resource = cld.getResource(path);
            if (resource == null) {
                resource = cld.getSystemResource(path);
                if (resource == null) {
                    throw new ClassNotFoundException("No resource for " + path);
                }
            }
            directory = new File(resource.getFile());
        } catch (NullPointerException x) {
            throw new ClassNotFoundException(packagename + " (" + directory + ") does not appear to be a valid package");
        }
        if (directory.exists()) {
            // Get the list of the files contained in the package
            String[] files = directory.list();
            for (int i = 0; i < files.length; i++) {
                // we are only interested in .class files
                if (files[i].endsWith(".class")) {
                    // removes the .class extension
                    String className = packagename + '.' + files[i].substring(0, files[i].length() - 6);

                    try {
                        if (packageDef.matches(className)) {
                            classesList.add(cld.loadClass(className));
                        }
                    } catch (ClassNotFoundException cnfe) {
                        throw new ClassNotFoundException("Class " + className + " not find in " + cpDelegate.getClasspath());
                    }
                }
            }
        } else {
            throw new ClassNotFoundException(packagename + " does not appear to be a valid package");
        }
    }

    public Class[] getClasses() throws ClassNotFoundException {


        ArrayList classes = new ArrayList();

        if (packages != null) {
            for (PackageWrapper packageWrapper : packages) {
                addClasses(packageWrapper, classes);
            }
        }
        if (classNames != null) {
            ClassLoader cl = cpDelegate.getClassLoader();
            for (ClassWrapper classWrapper : classNames) {
                Class clazz = cl.loadClass(classWrapper.getName());
                classes.add(clazz);
            }
        }
        Class[] classesA = new Class[classes.size()];
        classes.toArray(classesA);
        return classesA;
    }

    private List<PackageWrapper> packages;

    public PackageWrapper createPackage() {
        if (packages == null) {
            packages = new ArrayList<PackageWrapper>();
        }
        PackageWrapper ret = new PackageWrapper();
        packages.add(ret);
        return ret;
    }

    private List<ClassWrapper> classNames;

    public ClassWrapper createClass() {

        if (classNames == null) {
            classNames = new ArrayList<ClassWrapper>();
        }
        ClassWrapper ret = new ClassWrapper();

        classNames.add(ret);
        return ret;
    }

    String getClassPathAsString() {
        return cpDelegate.getClasspath().toString();
    }

    public class PackageWrapper extends PatternSet {

        private String name;
        private PatternSet patternSet;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public PatternSet.NameEntry createInclude() {
            return getPattern().createInclude();
        }

        public PatternSet.NameEntry createExclude() {
            return getPattern().createExclude();
        }

        private PatternSet getPattern() {
            if (patternSet == null) {
                patternSet = new PatternSet();
            }
            return patternSet;
        }

        public boolean matches(String className) {

            String[] incl = null;
            String[] excl = null;
            if (patternSet != null) {
                incl = patternSet.getIncludePatterns(getProject());
                excl = patternSet.getExcludePatterns(getProject());
            }
            int index = className.lastIndexOf('.');
            String shortName = null;
            if (index > 0) {
                shortName = className.substring(index + 1);
            } else {
                shortName = className;
            }

            boolean ret = true;
            if (incl != null) {
                ret = false;
                for (int i = 0; i < incl.length; i++) {
                    if (shortName.startsWith(incl[i])) {
                        logger.fine("include pattern " + incl[i] + " includes class " + className);
                        ret = true;
                        break;
                    }
                }
            }

            if (ret && excl != null) {
                for (int i = 0; i < excl.length; i++) {
                    if (shortName.startsWith(excl[i])) {
                        logger.fine("exclude pattern " + excl[i] + " excludes class " + className);
                        ret = false;
                        break;
                    }
                }
            }
            return ret;
        }
    }

    public class ClassWrapper {

        private String name;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }
    }
}
