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
package com.sun.grid;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.Properties;

/**
 *
 */
public class TestConfiguration {
    
    private File catop;
    private File calocaltop;
    private File cascript;
    private String adminUser;
    private Integer daysValid;
    
    private Properties props = new Properties();
    
    private static TestConfiguration theInstance;
    public synchronized static TestConfiguration getInstance() throws IOException {
        if(theInstance == null) {
            theInstance = new TestConfiguration();
        }
        return theInstance;
    }
    
    protected TestConfiguration() throws IOException {
        
        File file = new File("test/TestConfiguration.properties".replace('/', File.separatorChar));
        props.load(new FileInputStream(file));
        
        File privFile = new File("test/TestConfiguration_private.properties".replace('/', File.separatorChar));
        
        if(privFile.exists()) {
            
            Properties privProps = new Properties();
            privProps.load(new FileInputStream(privFile));
            
            Iterator iter = privProps.keySet().iterator();
            while(iter.hasNext()) {
                Object prop = iter.next();
                props.put(prop, privProps.get(prop));
            }
        }  
        
        String fileProp = System.getProperty(getClass().getName() + ".file");
        if(fileProp != null) {
            Properties fileProps = new Properties();
            fileProps.load(new FileInputStream(fileProp));
            
            Iterator iter = fileProps.keySet().iterator();
            while(iter.hasNext()) {
                Object prop = iter.next();
                props.put(prop, fileProps.get(prop));
            }
        }
        
    }
    
    private File getFileFromConfig(String key) {
        String str = props.getProperty(key);
        if(str == null) {
            throw new IllegalArgumentException(key + " is not defined in test configuration");
        }
        return new File(str);
    }
    
    public File getCatop() {
        if(catop == null) {
            String str = getCell() + "common/sgeCA";
            catop = new File(getSgeRoot(), str.replace('/', File.separatorChar) );
        }
        return catop;
    }
    
    public File getCaLocalTop() {
        if(calocaltop == null) {
            String str = "/var/lib/sgeCA/port" + getQMasterPort() + "/" + getCell();
            calocaltop = new File(str.replace('/', File.pathSeparatorChar));
        }
        return calocaltop;
    }
    
    public File getCaScript() {
        if(cascript == null) {
            cascript = new File(getSgeRoot(), "util/sgeCA/sge_ca".replace('/', File.separatorChar));
        }
        return cascript;
    }
    
    
    private int qmasterPort = -1;
    public int getQMasterPort() {
        if(qmasterPort < 0) {
            qmasterPort = Integer.parseInt(props.getProperty("sge_qmaster_port"));
        }
        return qmasterPort;
    }
    
    public String getAdminUser() {
        if(adminUser == null) {
            adminUser = props.getProperty("adminuser");
        }
        return adminUser;
    }

    public int getDaysValid() {
        if (daysValid == null) {
            if ( props.getProperty("daysValid") == null) {
               daysValid = new Integer(365);
            } else {
               daysValid = new Integer(props.getProperty("daysValid"));
            }
        }
        return daysValid.intValue();
    }

    public String getTestUser() {
        return props.getProperty("testuser");
    }
    
    public char [] getTestUserPassword() {
        String pw = props.getProperty("testuser_pw");
        if(pw != null) {
            return pw.toCharArray();
        }
        return null;
    }
    
    public String getUserVerifier() {
        return props.getProperty("userverifier");
    }
    
    public String getPamService() {
        return props.getProperty("pam_service");
    }
    
    private File sgeRoot;
    
    public File getSgeRoot() {
        if(sgeRoot == null) {
            sgeRoot = new File(props.getProperty("sge_root"));
        }
        return sgeRoot;
    }
    
    private String cell;
    
    public String getCell() {
        if(cell == null) {
            cell = props.getProperty("sge_cell");
        }
        return cell;
    }
}
