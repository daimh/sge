/*
 * CullObjectFilterDefinition.java
 *
 * Created on September 21, 2006, 4:34 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package com.sun.grid.cull.ant;

import com.sun.grid.cull.CullDefinitionFilter;
import org.apache.tools.ant.types.PatternSet;

/**
 *
 */
public class CullObjectFilterDefinition extends PatternSet {
    
    private String objectFilter;
    private boolean printDependencies;
    
    /** Creates a new instance of CullObjectFilterDefinition */
    public CullObjectFilterDefinition() {
    }
    
    public CullDefinitionFilter createFilter() {
        
        String [] incl = getIncludePatterns(getProject());
        String [] excl = getExcludePatterns(getProject());
        
        CullDefinitionFilter ret = new CullDefinitionFilter(incl, excl);
        ret.setObjectFilter(objectFilter);
        ret.setPrintDependencies(printDependencies);
        
        return ret;
    }
    
    public String getObjectFilter() {
        return objectFilter;
    }
    
    public void setObjectFilter(String objectFilter) {
        this.objectFilter = objectFilter;
    }
    
    public boolean isPrintDependencies() {
        return printDependencies;
    }
    
    public void setPrintDependencies(boolean printDependencies) {
        this.printDependencies = printDependencies;
    }
    
}
