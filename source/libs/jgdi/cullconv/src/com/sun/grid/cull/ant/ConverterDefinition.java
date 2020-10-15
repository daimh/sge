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
import com.sun.grid.cull.CullConverter;
import com.sun.grid.cull.CullDefinition;
import com.sun.grid.cull.CullDefinitionFilter;
import com.sun.grid.cull.CullDefinitionFilterProxy;
import java.util.LinkedList;
import java.util.List;
import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;

/**
 *
 */
public abstract class ConverterDefinition {

    private String classname;
    private Project project;

    /** Creates a new instance of Converter */
    public ConverterDefinition(Project project) {
        this.project = project;
    }
    private boolean forEachObject;
    public static final String SCOPE_DEF = "definition";
    public static final String SCOPE_OBJECTS = "objects";
    private String scope;

    public String getClassname() {
        return classname;
    }

    public void setClassname(String classname) {
        this.classname = classname;
    }

    protected void setScopeInConverter(AbstractCullConverter converter) throws BuildException {

        if (scope == null) {
            converter.setIterateObjects(true);
        } else if (SCOPE_OBJECTS.equals(scope)) {
            converter.setIterateObjects(true);
        } else if (SCOPE_DEF.equals(scope)) {
            converter.setIterateObjects(false);
        } else {
            throw new BuildException("Invalid scope " + scope);
        }
    }

    public CullConverter createConverter() throws BuildException {

        try {

            if (classname == null) {
                throw new BuildException("classname not set");
            }

            Class cls = Class.forName(classname);

            if (!AbstractCullConverter.class.isAssignableFrom(cls)) {
                throw new BuildException("class must be an instanceof " + CullConverter.class.getName());
            }

            return (CullConverter) cls.newInstance();
        } catch (ClassNotFoundException cnfe) {
            throw new BuildException("class " + classname + " not found", cnfe);
        } catch (InstantiationException ise) {
            throw new BuildException("can not instantiate class " + classname, ise);
        } catch (IllegalAccessException iae) {
            throw new BuildException("class " + classname + " is not accessible", iae);
        }
    }
    private List<CullObjectFilterDefinition> patternList = new LinkedList<CullObjectFilterDefinition>();

    public CullObjectFilterDefinition createPattern() {

        CullObjectFilterDefinition ret = new CullObjectFilterDefinition();
        patternList.add(ret);
        return ret;
    }

    public CullDefinition createFilteredCullDefinition(CullDefinition cullDef) {

        if (patternList.isEmpty()) {
            return cullDef;
        } else {
            CullDefinitionFilter[] filters = new CullDefinitionFilter[patternList.size()];
            for (int i = 0; i < filters.length; i++) {
                filters[i] = ((CullObjectFilterDefinition) patternList.get(i)).createFilter();
            }
            return new CullDefinitionFilterProxy(cullDef, filters);
        }
    }

    public String getScope() {
        return scope;
    }

    public void setScope(String scope) {
        this.scope = scope;
    }
}