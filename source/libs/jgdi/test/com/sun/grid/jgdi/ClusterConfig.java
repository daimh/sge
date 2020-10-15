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
package com.sun.grid.jgdi;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Properties;

/**
 *
 */
public class ClusterConfig {
    private String sgeRoot;
    private String sgeCell;
    private int qmasterPort;
    private int execdPort;
    private String user;
    private boolean csp;
    private String jaasLoginContext;
    private char[] keystorePassword;
    private char[] privateKeyPassword;
    
    /** Creates a new instance of ClusterConfig */
    public ClusterConfig() {
    }
    
    private static ClusterConfig newInstance(Properties props, String prefix) {
        ClusterConfig ret  = null;
        String str = props.getProperty(prefix + ".sge_root");
        if(str != null) {
            ret = new ClusterConfig();
            ret.sgeRoot = str;
            ret.sgeCell = props.getProperty(prefix + ".sge_cell", "default");
            ret.qmasterPort = Integer.parseInt(props.getProperty(prefix + ".qmaster_port"));
            ret.execdPort = Integer.parseInt(props.getProperty(prefix + ".execd_port"));
            ret.user = props.getProperty(prefix + ".user");
            ret.csp = Boolean.valueOf(props.getProperty(prefix + ".csp")).booleanValue();
            ret.jaasLoginContext = props.getProperty(prefix + ".jaas_login_context");
            
            str = props.getProperty(prefix + ".keystore_password");
            if(str != null) {
                ret.keystorePassword = str.toCharArray();
            }
            str = props.getProperty(prefix + ".privatekey_password");
            if(str != null) {
                ret.privateKeyPassword = str.toCharArray();
            }
            
        }
        return ret;
    }
    
    public static ClusterConfig[] getClusters() throws IOException {
        
        Properties props = new Properties();
        
        ClassLoader cl = ClusterConfig.class.getClassLoader();
        String file = System.getProperty("cluster.config.file.location");
        InputStream in = null;
        if (file != null && !file.equals("${cluster.config.file.location}")) {
            in = new FileInputStream(file);
        } else {
            in = cl.getResourceAsStream("ClusterConfig_private.properties");
        }
        if (in == null) {
            in = cl.getResourceAsStream("ClusterConfig.properties");
        }
        if (in == null) {
            throw new IOException("Could not find ClusterConfig.properties file.");
        }
        props.load(in);
        
        int i = 0;
        ArrayList list = new ArrayList();
        
        while(true) {
            
            ClusterConfig conf = newInstance(props, "cluster[" + list.size() + "]");
            if(conf == null) {
                break;
            }
            list.add(conf);
        }
        
        ClusterConfig[] ret = new ClusterConfig[list.size()];
        list.toArray(ret);
        return ret;
    }
    
    public String getSgeRoot() {
        return sgeRoot;
    }
    
    public String getSgeCell() {
        return sgeCell;
    }
    
    public int getQmasterPort() {
        return qmasterPort;
    }
    
    public int getExecdPort() {
        return execdPort;
    }
    
    public String getUser() {
        return user;
    }
    
    public char[] getKeystorePassword() {
        return keystorePassword;
    }
    
    public char[] getPrivateKeyPassword() {
        return privateKeyPassword;
    }
    
    public boolean isCsp() {
        return csp;
    }
    
    public String getJaasLoginContext() {
        return jaasLoginContext;
    }
    
    
}
