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

import java.io.IOException;
import java.security.PrivilegedAction;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.security.auth.Subject;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.ConfirmationCallback;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.callback.TextOutputCallback;
import javax.security.auth.callback.UnsupportedCallbackException;
import javax.security.auth.login.LoginContext;
import junit.framework.TestCase;
import junit.framework.TestResult;

/**
 *
 */
public class BaseTestCase extends TestCase {

    protected Logger logger = Logger.getLogger(getClass().getName());
    private ClusterConfig currentCluster;
    private ClusterConfig[] cluster;

    /** Creates a new instance of BaseTestCase */
    public BaseTestCase(String testName) {
        super(testName);
    }

    private void initConfig() {
        try {
            cluster = ClusterConfig.getClusters();
        } catch (IOException ioe) {
            IllegalStateException ilse = new IllegalStateException("can not read cluster config");
            ilse.initCause(ioe);
        }
    }

    protected ClusterConfig getCurrentCluster() {
        return currentCluster;
    }

    protected JGDI createJGDI() throws Exception {
        return createJGDI(currentCluster);
    }

    private String getConnectURL(ClusterConfig cluster) {
        initConfig();
        String url = "bootstrap://" + cluster.getSgeRoot() + "@" + cluster.getSgeCell() + ":" + cluster.getQmasterPort();
        return url;
    }

    private JGDI createJGDI(ClusterConfig cluster) throws Exception {
        String url = getConnectURL(cluster);
        if (logger.isLoggable(Level.FINE)) {
            logger.fine("create jgdi ctx for cluster " + url);
        }
        return JGDIFactory.newInstance(url);
    }

    protected EventClient createEventClient(int evcId) throws Exception {
        return createEventClient(currentCluster, evcId);
    }

    private EventClient createEventClient(ClusterConfig cluster, int evcId) throws Exception {
        String url = getConnectURL(cluster);
        if (logger.isLoggable(Level.FINE)) {
            logger.fine("create event client ctx for cluster " + url);
        }
        return JGDIFactory.createEventClient(url, evcId);
    }

    protected String[] getClusterNames() throws Exception {
        return null;
    }

    public void run(TestResult result) {
        initConfig();
        try {
            for (int i = 0; i < cluster.length; i++) {
                currentCluster = cluster[i];
                if (logger.isLoggable(Level.CONFIG)) {
                    logger.config("===================================================================");
                    logger.config("start test  on cluster " + i);
                }
                if (currentCluster.isCsp()) {
                    LoginContext lc = null;
                    logger.config("setup jaas login context for " + currentCluster.getJaasLoginContext());
                    lc = new LoginContext(currentCluster.getJaasLoginContext(), new ClusterConfigCallbackHandler(currentCluster));
                    lc.login();
                    try {
                        RunAction action = new RunAction(result);
                        logger.config("run test in jaas subject");
                        Subject.doAs(lc.getSubject(), action);
                    } finally {
                        lc.logout();
                    }
                } else {
                    super.run(result);
                }
            }
        } catch (Exception e) {
            result.addError(this, e);
        }
    }

/**
     *  CallbackHandler which sets callback information from a cluster configuration
     *  object
     *
     */
    class ClusterConfigCallbackHandler implements CallbackHandler {

        private ClusterConfig cluster;

        public ClusterConfigCallbackHandler(ClusterConfig cluster) {
            this.cluster = cluster;
        }

        public void handle(Callback[] callbacks) throws IOException, UnsupportedCallbackException {

            for (int i = 0; i < callbacks.length; i++) {

                if (callbacks[i] instanceof TextOutputCallback) {
                    logger.fine("skip text output callback " + callbacks[i]);
                    continue;
                } else if (callbacks[i] instanceof NameCallback) {

                    NameCallback cb = (NameCallback) callbacks[i];

                    if (cb.getPrompt().indexOf("alias") >= 0) {
                        logger.fine("handle alias callback: " + cluster.getUser());
                        cb.setName(cluster.getUser());
                    } else {
                        throw new UnsupportedCallbackException(callbacks[i]);
                    }
                } else if (callbacks[i] instanceof PasswordCallback) {

                    PasswordCallback cb = (PasswordCallback) callbacks[i];

                    String prompt = cb.getPrompt().toLowerCase();
                    logger.fine("handle password callback " + prompt);
                    if (prompt.indexOf("keystore password") >= 0) {
                        logger.fine("found keystore password callback");
                        cb.setPassword(cluster.getKeystorePassword());
                    } else if (prompt.indexOf("key password") >= 0) {
                        logger.fine("found key password callback");
                        cb.setPassword(cluster.getPrivateKeyPassword());
                    } else {
                        throw new UnsupportedCallbackException(callbacks[i]);
                    }
                } else if (callbacks[i] instanceof ConfirmationCallback) {
                    logger.fine("handle confirm callback");
                    ConfirmationCallback cb = (ConfirmationCallback) callbacks[i];
                    cb.setSelectedIndex(cb.getDefaultOption());
                } else {
                    throw new UnsupportedCallbackException(callbacks[i]);
                }
            }
        }
    }

    class RunAction implements PrivilegedAction {

        private TestResult result;

        public RunAction(TestResult result) {
            this.result = result;
        }

        public Object run() {
            try {
                BaseTestCase.super.run(result);
            } catch (Exception e) {
                result.addError(BaseTestCase.this, e);
            }
            return null;
        }
    }
}