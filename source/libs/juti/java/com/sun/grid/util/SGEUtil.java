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
package com.sun.grid.util;

import com.sun.grid.util.expect.Expect;
import com.sun.grid.util.expect.ExpectBuffer;
import com.sun.grid.util.expect.ExpectHandler;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 *  This class defines some helper methods which are vital
 *  for gridengine. 
 *
 */
public class SGEUtil {
    
    
    private SGEUtil() {
    }
    
    private static SGEUtil theInstance;
    
    /**
     * Get the instance of the <code>SGEUtil</code>.
     *
     * The first call of this method tries to load the native
     * library juti.
     *
     * @see java.lang.System#loadLibrary
     * @return the instance of <code>SGEUtil</code>
     */
    public synchronized static SGEUtil getInstance() {
        
        if(theInstance == null) {
            System.loadLibrary("juti");
            theInstance = new SGEUtil();
        }
        return theInstance;
    }
    
    /**
     * get pid of the java virtual machine
     * @return the pid of the java virtual machine
     */
    public native int getPID();
    
    
    
    /**
     * Read a password from the console.
     *
     * @param prompt the prompt
     * @return the password
     */
    public char[] getPassword(String prompt) {
        
        System.out.print(prompt);
        byte [] bytes  = getNativePassword();
        
        if(bytes == null || bytes.length == 0) {
            return null;
        }
        
        Reader rd = new InputStreamReader(new ByteArrayInputStream(bytes));
        
        StringBuffer buf = new StringBuffer();
        char [] tmp = new char[50];
        int len = 0;
        try {
            while( (len = rd.read(tmp)) > 0 ) {
                buf.append(tmp, 0, len);
            }
            
            char [] ret = new char[buf.length()];
            for(int i = 0; i < ret.length; i++) {
                ret[i] = buf.charAt(i);
            }
            
            return ret;
        } catch (IOException ex) {
            throw new IllegalStateException("IO error while reading from byte array");
        }
    }
    
    /**
     * Read a password for the console.
     *
     * @return the bytes of the password
     */
    private native byte[] getNativePassword();
    
    
    private static Map archMap = new HashMap();
    
    private static Boolean isWindows;
    
    public static synchronized boolean isWindows() {
        if (isWindows == null) {
            String osname = System.getProperty("os.name").toLowerCase(Locale.US);
            if(osname.indexOf("windows") >= 0) {
                isWindows = Boolean.TRUE;
            } else {
                isWindows = Boolean.FALSE;
            }
        }
        return isWindows.booleanValue();
    }
    
    /* Get the arch string for a gridengine installation
     * @param sgeRoot the sge root directory
     * @throws java.io.IOException if the execution of the arch script fails
     * @return the arch string
     */
    public static synchronized String getArch(File sgeRoot) throws IOException {
        String ret = (String)archMap.get(sgeRoot);
        if(ret == null) {
            if(isWindows()) {
                ret = "win32-x86";
            } else {
                Expect expect = new Expect();

                expect.env().add("SGE_ROOT=" + sgeRoot.getAbsolutePath());
                expect.command().add( sgeRoot.getAbsolutePath() + File.separatorChar + "util" + File.separatorChar + "arch");

                ArchExpectHandler archHandler = new ArchExpectHandler();
                expect.add(archHandler);
                try {
                    int res = expect.exec(10000);
                    if(res != 0) {
                       throw new IOException("arch script exited with status (" + ret +")"); 
                    }

                    ret = archHandler.getArch();
                    if(ret == null) {
                        throw new IllegalStateException("arch script did not produce any output");
                    }
                } catch (InterruptedException ex) {
                    throw new IllegalStateException("arch script has been interrupted");
                }
            }
            archMap.put(sgeRoot, ret);
        }
        return ret;
    }
    
    
    private static class ArchExpectHandler implements ExpectHandler {

        public void handle(Expect expect, ExpectBuffer buffer) throws IOException {
            String msg = buffer.consumeLine();
            if(msg != null) {
                msg = msg.trim();
                if( msg.length() > 0 && arch == null)
                arch = msg;
            }
        }
        
        private String arch;

        public String getArch() {
            return arch;
        }
        
        
    }
    
    
    
}