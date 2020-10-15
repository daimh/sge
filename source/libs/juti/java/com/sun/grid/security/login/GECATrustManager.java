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

package com.sun.grid.security.login;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.InvalidAlgorithmParameterException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.Signature;
import java.security.cert.CRLException;
import java.security.cert.CertStore;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.CollectionCertStoreParameters;
import java.security.cert.PKIXBuilderParameters;
import java.security.cert.X509CertSelector;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;
import javax.net.ssl.CertPathTrustManagerParameters;
import javax.net.ssl.ManagerFactoryParameters;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

/**
 * The GECATrustManager validates the certificates against the CA certificate
 * of a Grid Engine CSP system
 */
public class GECATrustManager implements X509TrustManager {
    
    private final static Logger log = Logger.getLogger(GECATrustManager.class.getName());
    
    /** alias for the ca certificate */
    public static final String CA_ALIAS = "ca";
    
    private File caTop;
    private final Map userCertMap = new HashMap();
    private final Object syncObject = new Object();

    private long lastUpdate;
    private X509TrustManager trustManager;
    private X509Certificate  caCertificate;
    
    
    public GECATrustManager() {
        this(null);
    }
    
    /**
     *  Creates a new instance of GECATrustManager.
     *
     * @param caTop ca top directory of the grid engine ca
     */
    public GECATrustManager(File caTop) {
        this.caTop = caTop;
    }
    
    
    private X509Certificate getUserCert(String username) {
       synchronized(syncObject) {
           if (caTop != null) {
               CertCacheElem elem = (CertCacheElem) userCertMap.get(username);
               if (elem == null) {
                   File certFile = new File(caTop, "usercerts" + File.separator + username + File.separator + "cert.pem");
                   elem = new CertCacheElem(certFile);
                   userCertMap.put(username, elem);
               }
               return elem.getCertificate();
           } else {
               return null;
           }
       }
    }
    
    
    /**
     * set a new caTop directory
     * @param caTop
     */
    public void setCaTop(File caTop) {
        synchronized(syncObject) {
            this.caTop = caTop;
            userCertMap.clear();
            trustManager = null;
        }
    }
            
    
    private class CertCacheElem {
        
        private long lastUpdate;
        private final File certFile;
        private X509Certificate cert;
        
        public CertCacheElem(File certFile) {
            this.certFile = certFile;
        }
        
        public synchronized X509Certificate getCertificate() {
            if(!certFile.exists()) {
                log.log(Level.FINE,"cert file {0} does not exist", certFile);
                return null;
            }
            if(lastUpdate < certFile.lastModified()) {
                try {
                    lastUpdate = certFile.lastModified();
                    InputStream in = new FileInputStream(certFile);
                    try {
                        CertificateFactory certFac = CertificateFactory.getInstance("X.509");
                        cert = (X509Certificate) certFac.generateCertificate(in);
                    } finally {
                        try {
                            in.close();
                        } catch (IOException ex) {
                        // Ignore
                        }
                    }                
                } catch(Exception ex) {
                    LogRecord lr = new LogRecord(Level.WARNING, "Error while reading certificate from file {0}");
                    lr.setParameters(new Object[]{certFile});
                    lr.setThrown(ex);
                    log.log(lr);
                    cert = null;
                }
            }
            return cert;
        }
        
    }
    
    private final static String [] TRUSTED_SIGNATURE_ALGORITHM = new String [] {
        "MD5withRSA"
    };
    
    private boolean isTrustedSignatureAlgorithm(String algorithm) {
        for(int i = 0; i < TRUSTED_SIGNATURE_ALGORITHM.length; i++) {
            if(TRUSTED_SIGNATURE_ALGORITHM[i].equals(algorithm)) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Validate a message of a user.
     * @param username   name of the user
     * @param message    the message
     * @param signature  the signature
     * @param algorithm  the digest algorithm
     * @return <code>true</code> if the message is valid
     */
    public boolean isValidMessage(String username, byte [] message, byte [] signature, String algorithm) {

        synchronized (syncObject) {
            if(caTop == null ) {
                return false;
            }
            if(!isTrustedSignatureAlgorithm(algorithm)) {
                return false;
            }

            X509Certificate[] chain = new X509Certificate[2];

            chain[0] = getUserCert(username);

            if (chain[0] == null) {
                return false;
            }

            try {
                chain[1] = getCACertificate();
            } catch (CertificateException ex) {
                log.log(Level.FINE, "Can not get CA certificate", ex);
                return false;
            }

            try {
                checkClientTrusted(chain, "RSA");
            } catch (CertificateException ex) {
                log.log(Level.FINE, "user certificate is not trusted", ex);
                return false;
            }

            try {
                Signature s = Signature.getInstance(algorithm);
                s.initVerify(chain[0]);
                s.update(message);
                return s.verify(signature);
            } catch (Exception ex) {
                LogRecord lr = new LogRecord(Level.WARNING, "Error while verifing message of user {0}");
                lr.setParameters(new Object[]{username});
                lr.setThrown(ex);
                log.log(lr);
                return false;
            }
        }
    }    
    
        
    private X509TrustManager getTrustManager() throws CertificateException {
        log.entering("GECATrustManager", "getTrustManager");
        X509TrustManager ret = null;
        synchronized(syncObject) {
            reinit();
            ret = trustManager;
        }
        log.exiting("GECATrustManager", "getTrustManager", ret);
        return ret;
    }
    
    private X509Certificate getCACertificate() throws CertificateException {
        log.entering("GECATrustManager", "getCACertificate");
        X509Certificate ret = null;
        synchronized(syncObject) {
            reinit();
            ret = caCertificate;
        }
        log.exiting("GECATrustManager", "getCACertificate", ret);
        return ret;
    }
        
        
    private void reinit() throws CertificateException {
        log.entering("GECATrustManager", "reinit");
        if(caTop == null) {
            throw new CertificateException("caTop not set");
        }
        File caCertFile = new File(caTop, "cacert.pem");
        File caCrlFile = new File(caTop, "ca-crl.pem");

        if (trustManager == null || lastUpdate < caCertFile.lastModified() || lastUpdate < caCrlFile.lastModified()) {
            long alastUpdate = System.currentTimeMillis();
            init(caCertFile, caCrlFile);
            this.lastUpdate = alastUpdate;
        }
        log.exiting("GECATrustManager", "reinit");
    }

    private void init(File caCertFile, File caCrlFile) throws CertificateException {
        if(log.isLoggable(Level.FINER)) {
            log.entering("GECATrustManager", "init", new Object [] { caCertFile, caCrlFile });
        }

        try {
            // Initialize an new empty keystore
            KeyStore ks  = KeyStore.getInstance(KeyStore.getDefaultType());
            ks.load(null, new char[0]);

            log.log(Level.FINE, "read ca certificate from {0}", caCertFile);
            CertificateFactory certFac = CertificateFactory.getInstance("X.509");

            InputStream in = new FileInputStream(caCertFile);

            try {
                X509Certificate cert = (X509Certificate)certFac.generateCertificate(in);
                ks.setCertificateEntry(CA_ALIAS, cert);
                caCertificate = cert;
            } finally {
                try {
                    in.close();
                } catch(IOException ioe) {
                    // Ignore
                }
            }

            PKIXBuilderParameters pkixParams = new PKIXBuilderParameters(ks, new X509CertSelector());

            if (caCrlFile.exists()) {
                log.log(Level.FINE, "read certificate revocation list from {0}", caCrlFile);
                in = new FileInputStream(caCrlFile);

                try {
                    Collection crls = certFac.generateCRLs(in);

                    CollectionCertStoreParameters certStoreParams = new CollectionCertStoreParameters(crls);
                    CertStore certStore = CertStore.getInstance("Collection", certStoreParams);

                    pkixParams.setRevocationEnabled(true);
                    pkixParams.addCertStore(certStore);
                } finally {
                    try {
                        in.close();
                    } catch(IOException ioe) {
                        // Ignore
                    }
                }
            } else {
                // crl file does not exists, disable revocation
                pkixParams.setRevocationEnabled(false);
            }

            // Wrap them as trust manager parameters
            ManagerFactoryParameters trustParams = new CertPathTrustManagerParameters(pkixParams);
            TrustManagerFactory fac = TrustManagerFactory.getInstance("PKIX");

            fac.init(trustParams);

            trustManager = null;
            TrustManager [] trustManagers = fac.getTrustManagers();
            for (int i = 0; i < trustManagers.length; i++) {
                if (trustManagers[i] instanceof X509TrustManager) {
                    trustManager = (X509TrustManager)trustManagers[i];
                    break;
                }
            }

            if (trustManager == null) {
                // This should never happen since we handle only X509 certificates
                throw new CertificateException("Found no X509TrustManager");
            }



        } catch (NoSuchAlgorithmException ex) {
            CertificateException cae = new CertificateException("Certificate algorithm is unknown", ex);
            log.throwing("GECATrustManager", "init", cae);
            throw cae;
        } catch (InvalidAlgorithmParameterException ex) {
            CertificateException cae = new CertificateException("Invalid parameters for crypte algorithms", ex);
            log.throwing("GECATrustManager", "init", cae);
            throw cae;
        } catch(KeyStoreException ex) {
            CertificateException cae = new CertificateException("Cannot create keystore for ca certificate", ex);
            log.throwing("GECATrustManager", "init", cae);
            throw cae;
        } catch(IOException ex) {
            CertificateException cae = new CertificateException("I/O error while initializing the ca certificate", ex);
            log.throwing("GECATrustManager", "init", cae);
            throw cae;
        } catch(CRLException ex) {
            CertificateException cae = new CertificateException("Error in crl file", ex);
            log.throwing("SgeCATrustManager", "init", ex);
            throw cae;
        }
        log.exiting("GECATrustManager", "init");
    }
    
    /**
     * Given the partial or complete certificate chain provided by the
     * peer, build a certificate path to a trusted root and return if
     * it can be validated and is trusted for client SSL
     * authentication based on the authentication type.
     *
     * The authentication type is determined by the actual certificate
     * used. For instance, if RSAPublicKey is used, the authType
     * should be "RSA". Checking is case-sensitive.
     *
     * @param chain the peer certificate chain
     * @param authType the authentication type based on the client certificate
     * @throws IllegalArgumentException if null or zero-length chain
     *         is passed in for the chain parameter or if null or zero-length
     *         string is passed in for the  authType parameter
     * @throws CertificateException if the certificate chain is not trusted
     *         by this TrustManager.
     */
    public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
        if(log.isLoggable(Level.FINER)) {
            log.entering("GECATrustManager", "checkClientTrusted", new Object [] { chain, authType });
        }
        getTrustManager().checkClientTrusted(chain, authType);
        log.exiting("GECATrustManager", "checkClientTrusted");
    }
    
    /**
     * Given the partial or complete certificate chain provided by the
     * peer, build a certificate path to a trusted root and return if
     * it can be validated and is trusted for server SSL
     * authentication based on the authentication type.
     *
     * The authentication type is the key exchange algorithm portion
     * of the cipher suites represented as a String, such as "RSA",
     * "DHE_DSS". Note: for some exportable cipher suites, the key
     * exchange algorithm is determined at run time during the
     * handshake. For instance, for TLS_RSA_EXPORT_WITH_RC4_40_MD5,
     * the authType should be RSA_EXPORT when an ephemeral RSA key is
     * used for the key exchange, and RSA when the key from the server
     * certificate is used. Checking is case-sensitive.
     *
     * @param chain the peer certificate chain
     * @param authType the key exchange algorithm used
     * @throws IllegalArgumentException if null or zero-length chain
     *         is passed in for the chain parameter or if null or zero-length
     *         string is passed in for the  authType parameter
     * @throws CertificateException if the certificate chain is not trusted
     *         by this TrustManager.
     */
    public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
        if(log.isLoggable(Level.FINER)) {
            log.entering("GECATrustManager", "checkServerTrusted", new Object [] { chain, authType });
        }
        getTrustManager().checkServerTrusted(chain, authType);
        log.exiting("GECATrustManager", "checkServerTrusted");
    }
    
    /**
     * Return an array of certificate authority certificates
     * which are trusted for authenticating peers.
     *
     * @return a non-null (possibly empty) array of acceptable
     *		CA issuer certificates.
     */
    public X509Certificate[] getAcceptedIssuers() {
        log.entering("GECATrustManager", "getAcceptedIssuers");
        X509Certificate[] ret = null;
        try {
            ret = getTrustManager().getAcceptedIssuers();
        } catch(CertificateException ex) {
            ret = new X509Certificate[0];
        }
        log.exiting("GECATrustManager", "getAcceptedIssuers", ret);
        return ret;
    }
    
    
}
