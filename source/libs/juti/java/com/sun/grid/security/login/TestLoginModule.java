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
package com.sun.grid.security.login;

import java.io.IOException;
import java.util.Map;
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
public class TestLoginModule implements LoginModule {

    private static final String AUTHZ_IDENTITY = "authzIdentity";    
    
    private CallbackHandler callbackHandler;
    private final static Logger log = Logger.getLogger(TestLoginModule.class.getName());
    private Subject subject;
    private boolean commitSucceded;
    private String username;
    private UserPrincipal principal;
    private UserPrincipal authzPrincipal;
    private String authzIdentity;
    /**
     * Initialize the <code>TestLoginModule</code>
     * @param subject   the current subject
     * @param callbackHandler the callbackhandler (must at least handle a 
     *                        <code>NameCallback</code> and a 
     *                        </code>PasswordCallback</code>).
     * @param sharedState not used
     * @param options contains the options for the <code>TestLoginModule</code>.
     */
    public void initialize(Subject subject, CallbackHandler callbackHandler, Map sharedState, Map options) {
        log.entering("TestLoginModule", "initialize", options);
        this.subject = subject;
        this.callbackHandler = callbackHandler;
        
        authzIdentity = (String) options.get(AUTHZ_IDENTITY);
        if(authzIdentity != null && authzIdentity.length() == 0) {
            authzIdentity = null;
        }
        
        log.exiting("TestLoginModule", "initialize");
    }

    /**
     * Perform the login.
     *
     * @throws javax.security.auth.login.LoginException <ul>
     *      <li>if the callbackhandler reports an error</li>
     *      <li>if some options are missing (please check the jass.config file)</li>
     *      <li>if the underlying authentication system report an error</li>
     *    </ul>
     * @return <code>true</code> on successfull authentication. <code>false</code>
     *         if username of password is invalid.
     */
    public boolean login() throws LoginException {
        log.entering("TestLoginModule", "login");
        PasswordCallback pwCallback = new PasswordCallback(RB.getString("unixlogin.userprompt"), false);
        NameCallback nameCallback = new NameCallback(RB.getString("unixlogin.pwprompt"));

        try {
            callbackHandler.handle(new Callback[]{nameCallback, pwCallback});
        } catch (IOException ex) {
            throw RB.newLoginException("unixlogin.error.iocb", ex,
                    new Object[]{ex.getLocalizedMessage()});
        } catch (UnsupportedCallbackException ex) {
            throw RB.newLoginException("unixlogin.error.invalidcb", ex,
                    new Object[]{ex.getLocalizedMessage()});
        }

        username = nameCallback.getName();
        log.exiting("TestLoginModule", "login", Boolean.TRUE);
        return true;
    }

    /**
     * Commit the login (adds the principals to the subject)
     *
     * @return <code>true</code> of the principals has been added to the subject.
     */
    public boolean commit() {
        log.entering("TestLoginModule", "commit");
        if (username != null) {
            principal = new UserPrincipal(username);
            subject.getPrincipals().add(principal);
            log.log(Level.FINE, "TestLoginModule: username={0}", username);
            username = null;
            if (authzIdentity != null) {
                authzPrincipal = new UserPrincipal(authzIdentity);
                subject.getPrincipals().add(authzPrincipal);
            }
            commitSucceded = true;
        } else {
            commitSucceded = false;
        }
        log.exiting("TestLoginModule", "commit", Boolean.valueOf(commitSucceded));
        return commitSucceded;
    }

    /**
     * Abort the login.
     * @return Always <code>true</code>
     */
    public boolean abort() {
        log.entering("TestLoginModule", "abort");
        logout();
        log.exiting("TestLoginModule", "abort", Boolean.TRUE);
        return true;
    }

    /**
     * Removes all previously added prinicipals from the subject.
     *
     * @return Always <code>true</code>
     */
    public boolean logout() {
        log.entering("TestLoginModule", "logout");
        if (commitSucceded) {
            subject.getPrincipals().remove(principal);
            if(authzPrincipal != null) {
                subject.getPrincipals().remove(authzPrincipal);
            }
        }
        subject = null;
        principal = null;
        authzPrincipal = null;
        commitSucceded = false;
        log.exiting("TestLoginModule", "logout", Boolean.TRUE);
        return true;
    }
}
