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
 *   Copyright: 2006 by Sun Microsystems, Inc
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jgdi.management;

import javax.net.ssl.KeyManager;


import com.sun.grid.security.login.GECATrustManager;
import com.sun.grid.security.login.GECAKeyManager;
import java.io.File;
import java.security.KeyStore;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;

/**
 * Helper class for SSL setup.
 */
public final class SSLHelper {

    private final static String SSL_PROTOCOL = "SSL";
    private SSLContext ctx;
    private final GECAKeyManager keyManager = new GECAKeyManager();
    private final GECATrustManager trustManager = new GECATrustManager();
    private final Lock lock = new ReentrantLock();
    private final File caTop;

    private SSLHelper(File caTop) {
        this.caTop = caTop;
        trustManager.setCaTop(caTop);
    }
    private static final Lock instanceLock = new ReentrantLock();
    private static final Map<File, SSLHelper> instanceMap = new HashMap<File, SSLHelper>();
    private static final Map<String, SSLHelper> clusterInstanceMap = new HashMap<String, SSLHelper>();

    /**
     * Get the instance of the SSLHelper by the caTop directory
     * @param caTop the caTop directory
     * @return the SSLHelper
     * @deprecated Replaced by getInstanceByHostAndPort
     */
    public static SSLHelper getInstanceByCaTop(File caTop) {
        SSLHelper ret = null;
        instanceLock.lock();
        try {
            ret = instanceMap.get(caTop);

            if (ret == null) {
                for (SSLHelper helper : clusterInstanceMap.values()) {
                    if (helper.caTop.equals(caTop)) {
                        ret = helper;
                        break;
                    }
                }

                if (ret == null) {
                    ret = new SSLHelper(caTop);
                    instanceMap.put(caTop, ret);
                }
            }
        } finally {
            instanceLock.unlock();
        }
        return ret;
    }

    /**
     * Get the instance of the SSLHelper by the caTop directory
     * @param serverHostname  the server hostname
     * @param serverPort  the server port
     * @param caTop the caTop directory
     * @return the SSLHelper
     */
    public static SSLHelper getInstanceByKey(String serverHostname, int serverPort, File caTop) {
        String key = serverHostname + ":" + serverPort;
        SSLHelper ret = null;
        instanceLock.lock();
        try {
            ret = clusterInstanceMap.get(key);

            if (ret == null) {
                ret = instanceMap.get(caTop);
                if (ret == null) {
                    ret = new SSLHelper(caTop);
                    instanceMap.put(caTop, ret);
                }
                clusterInstanceMap.put(key, ret);
            }
        } finally {
            instanceLock.unlock();
        }
        return ret;
    }

    /**
     * Get the instance of the SSLHelper
     * @param sgeRoot  the sge root directory of the addressed cluster
     * @param cell     the cell name of of the addressed cluster
     * @return the SSLHelper
     * @deprecated
     */
    public static SSLHelper getInstance(File sgeRoot, String cell) {
        File caTop = new File(sgeRoot, cell + File.separator + "common" + File.separator + "sgeCA");
        return getInstanceByCaTop(caTop);
    }

    private void initSSLContext() {
        lock.lock();
        try {
            if (ctx == null) {
                try {
                    ctx = SSLContext.getInstance(SSL_PROTOCOL);
                    ctx.init(new KeyManager[]{keyManager},
                            new TrustManager[]{trustManager},
                            null);
                } catch (Exception ex) {
                    throw new SecurityException("Cannot create SSLContext", ex);
                }
            }
        } finally {
            lock.unlock();
        }
    }

    /**
     * Set the keystore for the JGDI ssl context
     * @param ks   the keystore
     * @param pw   the password for the keystore
     */
    void setKeystore(KeyStore ks, char[] pw) {
        lock.lock();
        try {
            initSSLContext();
            keyManager.setKeystore(ks, pw);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Set the keystore for the JGDI ssl context
     * @param keystore   the keystore file
     * @param pw   the password for the keystore
     */
    void setKeystore(File keystore, char[] pw) {
        lock.lock();
        try {
            initSSLContext();
            keyManager.setKeystore(keystore, pw);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Reset the JGDI ssl context
     */
    void reset() {
        lock.lock();
        try {
            ctx = null;
            keyManager.reset();
        } finally {
            lock.unlock();
        }
    }

    /**
     *  Get the ssl socket factory for the application
     *  @return the socket factor for the application
     */
    SSLSocketFactory getSocketFactory() {
        lock.lock();
        try {
            if (ctx == null) {
                return (SSLSocketFactory) SSLSocketFactory.getDefault();
            } else {
                return ctx.getSocketFactory();
            }
        } finally {
            lock.unlock();
        }
    }
}
