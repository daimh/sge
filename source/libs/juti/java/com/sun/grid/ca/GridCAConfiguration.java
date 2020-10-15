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
package com.sun.grid.ca;

import java.io.File;
import java.io.Serializable;

/**
 * Configuration of a <code>GridCA</code>.
 *
 * An object of this class holds all necessary information for a GridCA configuration
 * The <code>validate</code> method determines wheter the configuration is valid
 */
public class GridCAConfiguration implements Serializable {
    
    private File sgeCaScript;
    private File caTop;
    private File caLocalTop;
    private File configDir;
    private File tmpDir;
    private String adminUser;
    private String caHost;
    private int daysValid = 365;
    
    /** Creates a new instance of GridCAConfiguration */
    public GridCAConfiguration() {
    }
    
    /**
     *  Validate the configuration of the <code>GridCA</code>.
     *
     *  @throws GridCAException if the <code>GridCA</code> is not proper configured
     */
    public void validate() throws GridCAException {
        
        if(sgeCaScript == null ) {
            throw RB.newGridCAException("gridCAConf.missing", "sgeCaScript");
        }
        if(caTop == null ) {
            throw RB.newGridCAException("gridCAConf.missing", "caTop");
        }
        if(caLocalTop == null ) {
            throw RB.newGridCAException("gridCAConf.missing", "caLocalTop");
        }
        
        if(configDir == null ) {
            configDir = new File(caTop, "util/sgeCA".replace('/', File.separatorChar));
        }
        if(tmpDir == null ) {
            tmpDir = new File(System.getProperty("java.io.tmpdir"));
        }
        if(adminUser == null) {
            adminUser = System.getProperty("user.name");
        }
        if(caHost == null) {
            throw RB.newGridCAException("gridCAConf.missing", "caHost");
        }
    }
    
    
    /**
     *  Get the sge_ca script. This script will be executed to perform the
     *  ca actions
     *
     * @return the sge_ca script
     */
    public File getSgeCaScript() {
        return sgeCaScript;
    }
    
    /**
     * Set the sge_ca script.
     *
     * @param sgeCaScript the sge_ca script
     */
    public void setSgeCaScript(File sgeCaScript) {
        this.sgeCaScript = sgeCaScript;
    }
    
    /**
     * Get the catop directory (shared directory which holds public parts of the ca).
     *
     * @return the catop directory
     */
    public File getCaTop() {
        return caTop;
    }
    
    /**
     * Set the catop directory
     * @param caTop the catop directory
     */
    public void setCaTop(File caTop) {
        this.caTop = caTop;
    }
    
    
    /**
     * Get the calocaltop directory (local directory which holds the private parts of the ca).
     *
     * @return the callocaltop directory
     */
    public File getCaLocalTop() {
        return caLocalTop;
    }
    
    /**
     * Set the calocaltop directory
     * @param caLocalTop the calocaltop directory
     */
    public void setCaLocalTop(File caLocalTop) {
        this.caLocalTop = caLocalTop;
    }
    
    /**
     *  Get the configuration directory of the ca.
     * 
     *  The configuration directory holds the configuration files of the ca. In
     *  a standard Gridengine installation the configuration directory is
     *  $SGE_ROOT/util/sgeCA
     *  
     *  @return the configuration directory
     */
    public File getConfigDir() {
        return configDir;
    }
    
    /**
     * Set the configuration directory of the ca.
     *
     * @param configDir the configuration directory
     */
    public void setConfigDir(File configDir) {
        this.configDir = configDir;
    }
    
    
    /**
     * Get the tmp directory of the ca. 
     *
     * For security reasons the tmp directory should be on a local file system
     * only root and the adminuser of the CA should have access on this directory.
     *
     * @return the tmp directory of the ca
     */
    public File getTmpDir() {
        return tmpDir;
    }
    
    /**
     * Set the tmp directory of the ca.
     *
     * @param tmpDir  the tmp directory
     */
    public void setTmpDir(File tmpDir) {
        this.tmpDir = tmpDir;
    }
    
    /**
     * Get the days the certificates are valid
     * 
     * @return  the days of validity
     */
    public int getDaysValid() {
        return daysValid;
    }
    
    /**
     * Set the days the certificates are valid
     * @param daysValid the days the certificates are valid
     */
    public void setDaysValid(int daysValid) {
        this.daysValid = daysValid;
    }

    /**
     * Get the admin user of the ca.
     *
     * Most of the files are owned by the admin user. This user needs write
     * write access on the catop directory. 
     *
     * @return  the admin user
     */
    public String getAdminUser() {
        return adminUser;
    }
    
    /**
     * Set the admin user of the ca
     * @param adminUser the admin user
     */
    public void setAdminUser(String adminUser) {
        this.adminUser = adminUser;
    }

    
    /**
     * Get the host with the filesystem where the private parts of the 
     * ca are exists.
     * Most action can only be executed in the ca host
     * @return the the ca host
     * @see #getCaLocalTop
     */
    public String getCaHost() {
        return caHost;
    }

    
    /**
     * Set the ca host
     * @param caHost  the ca hsot
     * @see #getCaHost
     */
    public void setCaHost(String caHost) {
        this.caHost = caHost;
    }
    
}