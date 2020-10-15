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

import java.io.File;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.rmi.ssl.SslRMIServerSocketFactory;

/**
 *  RMI server socket factory that uses a customized SSLContext
 *  in jdk 1.6 SSLContext.setDefault(customCtx);
 *  would make this class obsolete
 *
 */
public class JGDISslRMIServerSocketFactory extends SslRMIServerSocketFactory {

    private final File caTop;
    private final String serverHostname;
    private final int serverPort;
    
    /**
     * <p>Creates a new <code>SslRMIServerSocketFactory</code> with
     * the default SSL socket configuration.</p>
     *
     * <p>SSL connections accepted by server sockets created by this
     * factory have the default cipher suites and protocol versions
     * enabled and do not require client authentication.</p>
     * @param caTop the catop directory if the cluster
     */
    public JGDISslRMIServerSocketFactory(String serverHostname, int serverPort, File caTop) {
        this(serverHostname, serverPort, caTop, null, null, false);
    }

    /**
     * <p>Creates a new <code>SslRMIServerSocketFactory</code> with
     * the specified SSL socket configuration.</p>
     *
     * @param caTop the catop directory if the cluster
     * @param enabledCipherSuites names of all the cipher suites to
     * enable on SSL connections accepted by server sockets created by
     * this factory, or <code>null</code> to use the cipher suites
     * that are enabled by default
     *
     * @param enabledProtocols names of all the protocol versions to
     * enable on SSL connections accepted by server sockets created by
     * this factory, or <code>null</code> to use the protocol versions
     * that are enabled by default
     *
     * @param needClientAuth <code>true</code> to require client
     * authentication on SSL connections accepted by server sockets
     * created by this factory; <code>false</code> to not require
     * client authentication
     *
     * @exception IllegalArgumentException when one or more of the cipher
     * suites named by the <code>enabledCipherSuites</code> parameter is
     * not supported, when one or more of the protocols named by the
     * <code>enabledProtocols</code> parameter is not supported or when
     * a problem is encountered while trying to check if the supplied
     * cipher suites and protocols to be enabled are supported.
     *
     * @see SSLSocket#setEnabledCipherSuites
     * @see SSLSocket#setEnabledProtocols
     * @see SSLSocket#setNeedClientAuth
     */
    public JGDISslRMIServerSocketFactory(String serverHostname, int serverPort, File caTop, String[] enabledCipherSuites,
            String[] enabledProtocols,
            boolean needClientAuth)
            throws IllegalArgumentException {

        this.serverHostname = serverHostname;
        this.serverPort = serverPort;
        this.caTop = caTop;
        // Initialize the configuration parameters.
        //
        this.enabledCipherSuites = enabledCipherSuites == null ? null : (String[]) enabledCipherSuites.clone();
        this.enabledProtocols = enabledProtocols == null ? null : (String[]) enabledProtocols.clone();
        this.needClientAuth = needClientAuth;

        // Force the initialization of the default at construction time,
        // rather than delaying it to the first time createServerSocket()
        // is called.
        //
        final SSLSocketFactory sslSocketFactory = SSLHelper.getInstanceByKey(serverHostname, serverPort, caTop).getSocketFactory();
        SSLSocket sslSocket = null;
        if (this.enabledCipherSuites != null || this.enabledProtocols != null) {
            try {
                sslSocket = (SSLSocket) sslSocketFactory.createSocket();
            } catch (Exception e) {
                final String msg = "Unable to check if the cipher suites " +
                        "and protocols to enable are supported";
                throw (IllegalArgumentException) new IllegalArgumentException(msg).initCause(e);
            }
        }

        // Check if all the cipher suites and protocol versions to enable
        // are supported by the underlying SSL/TLS implementation and if
        // true create lists from arrays.
        //
        if (this.enabledCipherSuites != null) {
            sslSocket.setEnabledCipherSuites(this.enabledCipherSuites);
        }
        if (this.enabledProtocols != null) {
            sslSocket.setEnabledProtocols(this.enabledProtocols);
        }
    }

    /**
     * <p>Creates a server socket that accepts SSL connections
     * configured according to this factory's SSL socket configuration
     * parameters.</p>
     * @throws java.io.IOException if the socket can not be created
     */
    @Override
    public ServerSocket createServerSocket(int port) throws IOException {
        final SSLSocketFactory sslSocketFactory = SSLHelper.getInstanceByKey(serverHostname, serverPort, caTop).getSocketFactory();
        return new ServerSocket(port) {

            @Override
            public Socket accept() throws IOException {
                Socket socket = super.accept();
                SSLSocket sslSocket = (SSLSocket) sslSocketFactory.createSocket(
                        socket, socket.getInetAddress().getHostName(),
                        socket.getPort(), true);
                sslSocket.setUseClientMode(false);

                if (enabledCipherSuites != null) {
                    sslSocket.setEnabledCipherSuites(enabledCipherSuites);
                }
                if (enabledProtocols != null) {
                    sslSocket.setEnabledProtocols(enabledProtocols);
                }
                sslSocket.setNeedClientAuth(needClientAuth);
                return sslSocket;
            }
        };

    }
    private final String[] enabledCipherSuites;
    private final String[] enabledProtocols;
    private final boolean needClientAuth;
}
