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

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 */
public class CullDefinitionFilter {
    
   private static Logger logger = Logger.getLogger("cullconv");
    
   private String [] includes;
   private String [] excludes;
   private String objectFilter;
   private boolean printDependencies;
    

    /** Creates a new instance of CullDefinitionFilter */
    public CullDefinitionFilter(String [] includes, String [] excludes) {
        this.includes = includes;
        this.excludes = excludes;
    }
    
    public java.util.Set getObjectNames(CullDefinition cullDef) {
        
         Set orgSet = cullDef.getObjectNames();
         HashSet filteredCullObjectNameSet = new HashSet();
         
         if( this.includes == null || this.includes.length == 0 ) {
            filteredCullObjectNameSet.addAll(orgSet);
         } else {

            for(int i = 0; i < includes.length; i++) {
               if( orgSet.contains(includes[i]) ) {
                  filteredCullObjectNameSet.add(includes[i]);
               }
            }

         }
         if( this.excludes != null && this.excludes.length > 0 ) {
            for(int i = 0; i < excludes.length; i++ ) {
               filteredCullObjectNameSet.remove(excludes[i]);
            }
         }

         if(isPrintDependencies()) {
            logger.info("dependencies --------------------------------------");
            logger.info("object filter = '" + getObjectFilter() + "'" );
         }
         CullDependencies depend = new CullDependencies(cullDef, filteredCullObjectNameSet, getObjectFilter());
         
         String [] names = new String[filteredCullObjectNameSet.size()];
         filteredCullObjectNameSet.toArray(names);
         for(int i = 0; i < names.length; i++ ) {

            CullDependencies.Node node = depend.getNode(names[i]);
            if( node == null ) {
               logger.warning("node for obj " + names[i] + " not found");
            } else {

               boolean isNeeded = node.isNeeded();
               
               if(isPrintDependencies()) {
                  logger.info(node.toString());
               } else if( logger.isLoggable(Level.FINE ) ) {
                 logger.fine(node.toString());
               }

               if( !isNeeded ) {
                  filteredCullObjectNameSet.remove(names[i]);
               }
            }
         }
         if(filteredCullObjectNameSet.isEmpty()) {
            logger.warning("No cull object matches the objectFilter '" + getObjectFilter() + "'");
         }
         return filteredCullObjectNameSet;
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
