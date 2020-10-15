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
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.GECAKeyManager.java
 *
 *   Copyright: 2006 by Sun Microsystems, Inc
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.security.login;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.Socket;
import java.security.KeyStore;
import java.security.Principal;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.X509KeyManager;

/**
 * The GECAKeyManager handles the specific KeyManager properties
 * of a Grid Engine CSP system
 */
public class GECAKeyManager implements X509KeyManager {

    private final static Logger log = Logger.getLogger(GECAKeyManager.class.getName());
    private X509KeyManager keyManager;

    public GECAKeyManager() {
    }
            
    /**
     *  Creates a new instance of GECAKeyManager.
     *
     * @param serverKeystore keystore file of the daemon 
     * @param pw keystore password
     */
    public GECAKeyManager(File serverKeystore, char[] pw) throws SecurityException {
        setKeystore(serverKeystore, pw);
    }
    
    public synchronized void setKeystore(KeyStore serverKeystore, char[] pw) throws SecurityException {
        try {
            KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509");
            kmf.init(serverKeystore, pw);
            keyManager = (X509KeyManager)kmf.getKeyManagers()[0];
        } catch (Exception ex) {
            throw new SecurityException("Cannnot create keymanager", ex);
        }        
    }
    
    public synchronized void setKeystore(File serverKeystore, char[] pw) throws SecurityException {
        try {
            log.log(Level.FINER, "loading keystore file {0}", serverKeystore);
            KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
            FileInputStream fi = new FileInputStream(serverKeystore);
            try {
                ks.load(fi, pw);
                setKeystore(ks, pw);
            } finally {
                try {
                    fi.close();
                } catch (IOException ex) {
                    // Ignore
                }
            }
        } catch (Exception ex) {
            throw new SecurityException("Cannnot create keymanager", ex);
        }
    }
    
    public synchronized void reset() {
        keyManager = null;
    }
            

    public synchronized String[] getClientAliases(String arg0, Principal[] arg1) {
        if(keyManager == null) {
            return new String[0];
        }
        return keyManager.getClientAliases(arg0, arg1);
    }

    public synchronized String chooseClientAlias(String[] keyType, Principal[] arg1, Socket arg2) {
        if(keyManager == null) {
            return null;
        }
        return keyManager.chooseClientAlias(keyType, arg1, arg2);
    }

    public synchronized String[] getServerAliases(String arg0, Principal[] arg1) {
        if(keyManager == null) {
            return new String[0];
        }
        return keyManager.getServerAliases(arg0, arg1);
    }

    public synchronized String chooseServerAlias(String arg0, Principal[] arg1, Socket arg2) {
        if(keyManager == null) {
            return null;
        }
        return keyManager.chooseServerAlias(arg0, arg1, arg2);
    }

    public synchronized X509Certificate[] getCertificateChain(String arg0) {
        if(keyManager == null) {
            return new X509Certificate[0];
        }
        return keyManager.getCertificateChain(arg0);
    }

    public synchronized PrivateKey getPrivateKey(String arg0) {
        if(keyManager == null) {
            return null;
        }
        return keyManager.getPrivateKey(arg0);
    }
}
