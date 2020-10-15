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

import com.sun.grid.util.SGEUtil;
import java.io.File;
import java.io.IOException;
import java.security.Principal;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
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
 * This <code>LoginModule</code> authenticates a unix user with username
 * and password against the PAM or system authentication system.
 * 
 * The username is queried with a <code>NameCallback</code>, the password with 
 * a <code>PasswordCallback</code>
 * 
 * <p>After a successfull login this <code>LoginModule</code> adds
 * 
 * <ul>
 *    <li> a {@link com.sun.security.auth.UnixPrincipal} of the authenticated user</li>
 *    <li> a {@link com.sun.security.auth.UnixNumericUserPrincipal} with the user id
 *         of the authenticated user</li>
 *    <li> a {@link  com.sun.security.auth.UnixNumericGroupPrincipal} for each group the authenticated
 *         user belongs too</li>
 * </ul>
 * 
 * to the current subject.</p>
 * 
 * <p>This class uses a {@link java.util.logging.Logger} for log messages. The name of the <code>Logger</code>
 *    is equal to the fullqualified classname of this class.</p>
 * 
 * <H3>Options for UnixLoginModule</H3>
 * 
 * <table>
 *   <tr><th>Option</th><th>description</th></tr>
 *   <tr>
 *      <td>sge_root</td>
 *      <td>path to the gridengine distribution</td>
 *   </tr>
 *   <tr>
 *      <td>auth_method</td>
 *      <td>Autehtication method. Valid values are "pam" and "system"</td>
 *   </tr>
 *   <tr>
 *      <td>pam_service</td>
 *      <td>Name of the pam service (see man pam(5). Required for
 *          PAM authentifcation</td>
 *   </tr>
 * </table>
 * 
 * <H3>Simple jaas config file for PAM authentication</H3>
 * 
 * <pre>
 *  sample {
 *   com.sun.grid.security.login.UnixLoginModule requisite
 *         sge_root="/opt/sge",
 *         auth_method="pam";
 *         pam_service="su";
 *  };
 * </pre>
 * 
 * <H3>Simple jaas config file for system authentication</H3>
 * 
 * <pre>
 *  sample {
 *   com.sun.grid.security.login.UnixLoginModule requisite
 *         command="/opt/sge",
 *         auth_method="system";
 *  };
 * </pre>
 * 
 */
public class UnixLoginModule implements LoginModule {

    private static final String AUTHZ_IDENTITY = "authzIdentity";    
    
    private final static Logger log = Logger.getLogger(UnixLoginModule.class.getName(), RB.BUNDLE);
    private String confError;
    private String authMethod;
    private String pamService;
    private String command;
    private Subject subject;
    private boolean loginSucceded;
    private boolean commitSucceded;
    private CallbackHandler callbackHandler;
    private Set principals = new HashSet();
    private String authzIdentity;
    

    /**
     * Initialize the <code>UnixLoginModule</code>
     * @param subject   the current subject
     * @param callbackHandler the callbackhandler (must at least handle a 
     *                        <code>NameCallback</code> and a 
     *                        </code>PasswordCallback</code>).
     * @param sharedState not used
     * @param options contains the options for the <code>UnixLoginModule</code>.
     */
    public void initialize(Subject subject, CallbackHandler callbackHandler, Map sharedState, Map options) {

        log.entering("UnixLoginModule", "initialize");

        String sgeRoot = (String) options.get("sge_root");
        if (sgeRoot != null) {
            log.log(Level.FINE, "sge_root={0}", sgeRoot);
            try {
                String arch = SGEUtil.getArch(new File(sgeRoot));
                command = sgeRoot + File.separatorChar + "utilbin" + File.separatorChar + arch + File.separatorChar + "authuser";
                if (arch.equals("win32-x86")) {
                    command += ".exe";
                }
                log.log(Level.FINE, "command={0}", command);
            } catch (Exception ex) {
                log.log(Level.WARNING, "unixlogin.error.arch", ex);
                confError = RB.getString("unixlogin.error.arch", ex.getLocalizedMessage());
                return;
            }
        } else {
            log.log(Level.WARNING, "unixlogin.error.arch", "sge_root");
            confError = RB.getString("unixlogin.error.missing", "sge_root");
            return;
        }
        authMethod = (String) options.get("auth_method");
        if (authMethod == null) {
            log.log(Level.WARNING, "unixlogin.error.missing", "auth_method");
            confError = RB.getString("unixlogin.error.missing", "auth_method");
            return;
        }
        log.log(Level.FINE, "auth_method={0}", authMethod);
        if ("pam".equals(authMethod)) {
            pamService = (String) options.get("pam_service");
            if (pamService == null) {
                log.log(Level.WARNING, "unixlogin.error.missing", "pam_service");
                confError = RB.getString("unixlogin.error.missing", "pam_service");
                return;
            } else {
                log.log(Level.FINE, "pam_service={0}", pamService);
            }
        }
        authzIdentity = (String) options.get(AUTHZ_IDENTITY);
        if(authzIdentity != null && authzIdentity.length() == 0) {
            authzIdentity = null;
        }
        
        this.subject = subject;
        this.callbackHandler = callbackHandler;

        log.entering("UnixLoginModule", "exiting");

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

        log.entering("UnixLoginModule", "login");

        if (confError != null) {
            throw RB.newLoginException("unixlogin.error.conf", new Object[]{confError});
        }
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

        String username = nameCallback.getName();
        if (username == null || username.length() == 0) {
            loginSucceded = false;
            log.exiting("UnixLoginModule", "login", Boolean.FALSE);
            return loginSucceded;
        }

        char[] pw = pwCallback.getPassword();
        if (pw == null) {
            loginSucceded = false;
            log.exiting("UnixLoginModule", "login", Boolean.FALSE);
            return loginSucceded;
        }

        AuthUserWrapper authuser = null;
        if (authMethod == null) {
            throw RB.newLoginException("unixlogin.error.missing",
                    new Object[]{"auth_method"});
        } else if ("pam".equals(authMethod)) {
            authuser = AuthUserWrapper.newInstanceForPam(command, pamService);
        } else if ("shadow".equals(authMethod)) {
            log.log(Level.WARNING, "unixlogin.deprecatedAuthMethod",
                    new Object[]{authMethod, "system"});
            authuser = AuthUserWrapper.newInstance(command);
        } else if ("system".equals(authMethod)) {
            authuser = AuthUserWrapper.newInstance(command);
        } else {
            throw RB.newLoginException("unixlogin.error.unknownAuthMethod",
                    new Object[]{authMethod});
        }

        try {
            Set p = authuser.authenticate(username, pw);
            if (p != null) {
                log.log(Level.FINE, "unixlogin.authuser.principal.count", new Integer(p.size()));
                if(log.isLoggable(Level.FINER)) {
                    Iterator iter = p.iterator();
                    while(iter.hasNext()) {
                        Principal pr = (Principal)iter.next();
                        log.log(Level.FINER,"unixlogin.authuser.principal", new Object [] { pr.getClass().getName(), pr.getName() });
                    }
                }
                principals.addAll(p);
                if (authzIdentity != null) {
                    principals.add(new UserPrincipal(authzIdentity));
                }
                
                loginSucceded = true;
            } else {
                log.log(Level.FINE, "unixlogin.authuser.principal.no");
                loginSucceded = false;
            }
        } catch (LoginException ex) {
            log.throwing("UnixLoginModule", "login", ex);
            throw ex;
        }
        log.exiting("UnixLoginModule", "login", Boolean.valueOf(loginSucceded));
        return loginSucceded;
    }

    /**
     * Commit the login (adds the principals to the subject)
     *
     * @return <code>true</code> of the principals has been added to the subject.
     */
    public boolean commit() {
        log.entering("UnixLoginModule", "commit");
        if (loginSucceded) {
            subject.getPrincipals().addAll(principals);
            if (log.isLoggable(Level.FINER)) {
                log.log(Level.FINE, "unixlogin.subject.principal",
                        new Integer(subject.getPrincipals().size()));
            }
            commitSucceded = true;
        }
        log.exiting("UnixLoginModule", "commit", Boolean.valueOf(commitSucceded));
        return commitSucceded;
    }

    /**
     * Abort the login.
     * @return Always <code>true</code>
     */
    public boolean abort() {
        logout();
        return true;
    }

    /**
     * Removes all previously added prinicipals from the subject.
     *
     * @return Always <code>true</code>
     */
    public boolean logout() {
        if (commitSucceded) {
            subject.getPrincipals().removeAll(principals);
        }
        subject = null;
        principals.clear();
        callbackHandler = null;
        loginSucceded = false;
        commitSucceded = false;
        return true;
    }
}
