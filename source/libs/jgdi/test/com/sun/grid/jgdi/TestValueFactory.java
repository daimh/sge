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

import com.sun.grid.jgdi.configuration.ExecHost;
import com.sun.grid.jgdi.configuration.SubmitHost;
import com.sun.grid.jgdi.configuration.xml.XMLUtil;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import javax.xml.parsers.ParserConfigurationException;
import org.xml.sax.SAXException;

/**
 *
 */
public class TestValueFactory {
    
    
    public static final File dataDir = new File("test/testvalues");
    
    private static final TestFilenameFilter filter = new TestFilenameFilter();
    
    private static Properties properties;
    private static int hostIndex = 0;
    private static String[] hostnames = null;
    
    private static void initProperties() {
        if(properties == null) {
            
            properties = new Properties();
            
            File file = new File("test/TestValues.properties");
            
            try {
                FileInputStream fin = new FileInputStream(file);
                
                properties.load(fin);
                
                file = new File("test/TestValues_private.properties");
                
                if(file.exists() ) {
                    
                    Properties props = new Properties();
                    fin = new FileInputStream(file);
                    props.load(fin);
                    
                    Enumeration names = props.propertyNames();
                    while(names.hasMoreElements()) {
                        String name =(String)names.nextElement();
                        properties.put(name, props.getProperty(name));
                    }
                }
            } catch(IOException ioe) {
                IllegalStateException ilse = new IllegalStateException("Can load properties from " + file);
                ilse.initCause(ioe);
                throw ilse;
            }
            
        }
        
    }
    
    public static File getDataDir(Class type) {
        
        String name = type.getName();
        int index = name.lastIndexOf('.');
        if(index > 0) {
            name = name.substring(index+1);
        }
        return new File(dataDir, name);
    }
    
    public static String getNextHostname() {
        if(hostnames == null) {
            initProperties();
            String hosts = properties.getProperty("hosts");
            hostnames = hosts.split(" ");
        }
        String ret = hostnames[hostIndex];
        if(hostIndex >= hostnames.length) {
            hostIndex = 0;
        }
        return ret;
    }
    
    private static void setPrimaryKeyProperties(Class type, Map properties) {
        initProperties();
        String host = getNextHostname();
        
        properties.put("host", host);
        String name = type.getName();
        int index = name.lastIndexOf('.');
        if(index > 0) {
            name = name.substring(index+1);
        }
        properties.put("name", name + System.currentTimeMillis());
    }
    
    public static final Object [] NULL_TEST_VALUES = new Object[0];
    
    public static Object[] getTestValues(Class type) throws IOException, ParserConfigurationException, SAXException {
        
        initProperties();
        File dir = getDataDir(type);
        
        File [] files = dir.listFiles(filter);
        
        if(files != null) {
            Object [] ret = new Object[files.length];
            
            Map props = new HashMap();
            for(int i = 0; i < files.length; i++) {
                props.clear();
                props.putAll(properties);
                setPrimaryKeyProperties(type, props);
                ret[i] = XMLUtil.read(files[i], props);
            }
            return ret;
        } else {
            return NULL_TEST_VALUES;
        }
    }
    
    
    static class TestFilenameFilter implements FileFilter {
        
        public boolean accept(File pathname) {
            return pathname.isFile() &&
                    pathname.getName().toLowerCase().endsWith(".xml");
        }
        
    }
}
