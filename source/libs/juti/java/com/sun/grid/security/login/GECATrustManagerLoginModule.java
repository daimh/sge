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

import com.sun.org.apache.xerces.internal.impl.dv.util.Base64;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.security.auth.Subject;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.callback.UnsupportedCallbackException;
import javax.security.auth.login.LoginException;
import javax.security.auth.spi.LoginModule;

/** 
 */
public class GECATrustManagerLoginModule implements LoginModule {

    private static final Logger log = Logger.getLogger(GECATrustManagerLoginModule.class.getName());
    private static final String AUTHZ_IDENTITY = "authzIdentity";    
    
    private CallbackHandler callbackHandler;
    private Subject subject;

    private UserPrincipal principal;
    private UserPrincipal authzPrincipal;
    private String authzIdentity;
    private String username;
    
    private boolean loginSucceeded;
    private boolean commitSucceeded;

    private final static Map trustManagerMap = new HashMap(2);
    private File caTop;
    
    /**
     * Initialize this </code>LoginModule</code>.
     *
     * @param subject         the current subject
     * @param callbackHandler callbackHandler for retrieving system name and X509 certificate chain
     * @param sharedState     shared state (not used)
     * @param options         options (not used)
     */
    public void initialize(Subject subject, CallbackHandler callbackHandler, Map sharedState, Map options) {
        log.entering("GECATrustManagerLoginModule", "initialize");
        
        String caTopStr = (String)options.get("caTop");
        
        if(caTopStr != null) {
            caTop = new File(caTopStr);
        } else {
            caTop = null;
        }
        log.log(Level.FINE, "caTop = {0}", caTop);
        
        authzIdentity = (String) options.get(AUTHZ_IDENTITY);
        if(authzIdentity != null && authzIdentity.length() == 0) {
            authzIdentity = null;
        }
        log.log(Level.FINE, "authzIdentity = {0}", authzIdentity);
        
        this.subject = subject;
        this.callbackHandler = callbackHandler;
        
        log.exiting("GECATrustManagerLoginModule", "initialize");
    }
    

    /* get the trust manager for a system */
    private static GECATrustManager getTrustManager(File caTop) throws LoginException {
        log.entering("GECATrustManagerLoginModule", "getTrustManager");
        GECATrustManager ret = null;
        synchronized(trustManagerMap) {
            ret = (GECATrustManager)trustManagerMap.get(caTop);
            if(ret == null) {
                ret = new GECATrustManager(caTop);
                trustManagerMap.put(caTop, ret);
            }
        }
        log.exiting("GECATrustManagerLoginModule", "getTrustManager", ret);
        return ret;
    }


    /**
     * Try to login 
     * 
     * @throws javax.security.auth.login.LoginException if the <code>CallbackHandler</code> does not
     *                support the required <code>Callback</code>s or if an <code>Callback</code> throws
     *                an <code>IOException</code>.
     * @return <code>true</code> if the login was successful
     */
    public boolean login() throws LoginException {
        log.entering("GECATrustManagerLoginModule", "login");

        loginSucceeded = false;
        
        if (caTop == null) {
            log.log(Level.FINE, "login failed, have no caTop");
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }
        
        if (!caTop.exists()) {
            log.log(Level.FINE, "login failed, have no caTop {0} does not exist", caTop);
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }
        
        GECATrustManager trustManager = getTrustManager(caTop);

        if(trustManager == null) {
            log.log(Level.FINE, "login failed, no trust manager found");
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }
        
        NameCallback usernameCallback = new NameCallback("username");
        PasswordCallback passwordCallback = new PasswordCallback("password", true);

        try {
            callbackHandler.handle(new Callback[]{usernameCallback, passwordCallback});
        } catch (UnsupportedCallbackException ex) {
            LoginException le = new LoginException("callback is not supported");
            le.initCause(ex);
            throw le;
        } catch (IOException ex) {
            LoginException le = new LoginException("io error in callback handler");
            le.initCause(ex);
            throw le;
        }

        String password = new String(passwordCallback.getPassword());
        Properties props = new Properties();
        try {
            props.load(new ByteArrayInputStream(password.getBytes()));
        } catch (IOException ex) {
            // Can not happen we are reading from a byte array
            log.log(Level.FINE, "login failed, loading password properties failed", ex);
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }
        
        String messageStr = props.getProperty("message");
        if(messageStr == null || messageStr.length() == 0) {
            log.log(Level.FINE, "login failed, message is empty");
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }
        
        String signatureStr = props.getProperty("signature");
        if(signatureStr == null || signatureStr.length() == 0) {
            log.log(Level.FINE, "login failed, signature is empty");
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }
        
        String algorithm = props.getProperty("algorithm");
        if(algorithm == null || algorithm.length() == 0) {
            log.log(Level.FINE, "login failed, algorithm is empty");
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }

        byte[] message = Base64.decode(messageStr);
        if(message == null) {
            log.log(Level.FINE, "login failed, message is not base 64 encoded");
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }
        
        byte[] signature = Base64.decode(signatureStr);
        if(signature == null) {
            log.log(Level.FINE, "login failed, signature is not base 64 encoded");
            log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
            return loginSucceeded;
        }

        if (trustManager.isValidMessage(usernameCallback.getName(), message, signature, algorithm)) {
            log.log(Level.FINE, "login succeeded, message has a valid signature");
            loginSucceeded = true;
            username = usernameCallback.getName();
        } else {
            log.log(Level.FINE, "login failed, message has an invalid signature");
        }
        log.exiting("GECATrustManagerLoginModule", "login", Boolean.valueOf(loginSucceeded));
        return loginSucceeded;
    }

    /**
     * If the login method had success the commit method adds the <code>X500Principal</code>
     * of the subject of the x509 certicate chain to the current subject.
     *
     * @return <code>true</code> if <code>X500Principal</code> has been added to the subject
     */
    /* @todo use <code>com.sun.grid.ca.GridCAX500Name to parse the common name of the certificate  */
    public boolean commit() {
        log.entering("GECATrustManagerLoginModule", "commit");

        if (loginSucceeded) {
            principal = new UserPrincipal(username);
            subject.getPrincipals().add(principal);
            username = null;
            if (authzIdentity != null) {
                authzPrincipal = new UserPrincipal(authzIdentity);
                subject.getPrincipals().add(authzPrincipal);
            }
            commitSucceeded = true;
        }

        log.exiting("GECATrustManagerLoginModule", "commit", Boolean.valueOf(commitSucceeded));
        return commitSucceeded;
    }

    /**
     * Abort the login.
     *
     * @return always <code>true</code>
     */
    public boolean abort() {
        log.entering("GECATrustManagerLoginModule", "abort");
        logout();
        log.exiting("GECATrustManagerLoginModule", "abort");
        return true;
    }

    /**
     * logout the current subject
     *
     * @return always <code>true</code>
     */
    public boolean logout() {
        log.entering("GECATrustManagerLoginModule", "logout");

        if (commitSucceeded) {
            if(principal != null) {
                subject.getPrincipals().remove(principal);
            }
            if(authzPrincipal != null) {
                subject.getPrincipals().remove(authzPrincipal);
            }
        }

        subject = null;
        principal = null;
        authzPrincipal = null;
        username = null;
        callbackHandler = null;
        commitSucceeded = false;
        loginSucceeded = false;
        log.exiting("GECATrustManagerLoginModule", "logout");
        return true;
    }

}
