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

/*
 * JGDISslRMIClientSocketFactory.java
 *
 * Created on May 26, 2006, 1:24 PM
 */

package com.sun.grid.jgdi.management;


import java.io.File;
import java.io.IOException;
import java.net.Socket;
import java.util.StringTokenizer;
import javax.net.SocketFactory;
import javax.net.ssl.SSLSocket;
import javax.rmi.ssl.SslRMIClientSocketFactory;



/**
 * This client socket factory creates <code>SSLSocket</code>s for RMI.
 *
 * @see javax.rmi.ssl.SslRMIClientSocketFactory
 */
public class JGDISslRMIClientSocketFactory extends SslRMIClientSocketFactory {

    private final static long serialVersionUID = -2009062301L;

    private final File caTop;
    private final String serverHostname;
    private final int serverPort;
    
    public JGDISslRMIClientSocketFactory(String serverHostname, int serverPort, File caTop) {
        this.serverHostname = serverHostname;
        this.serverPort = serverPort;
        this.caTop = caTop;
    }
    
    /**
     * <p>Creates an SSL socket.</p>
     *
     * <p>If the system property
     * <code>javax.rmi.ssl.client.enabledCipherSuites</code> is
     * specified, this method will call {@link
     * SSLSocket#setEnabledCipherSuites(String[])} before returning
     * the socket. The value of this system property is a string that
     * is a comma-separated list of SSL/TLS cipher suites to
     * enable.</p>
     *
     * <p>If the system property
     * <code>javax.rmi.ssl.client.enabledProtocols</code> is
     * specified, this method will call {@link
     * SSLSocket#setEnabledProtocols(String[])} before returning the
     * socket. The value of this system property is a string that is a
     * comma-separated list of SSL/TLS protocol versions to
     * enable.</p>
     *
     * @param host  the host
     * @param port  the port
     * @return the created socket
     * @throws java.io.IOException on any io error
     */
    @Override
    public final Socket createSocket(final String host, final int port)
            throws IOException {

        // Retrieve the SSLSocketFactory
        //
        final SocketFactory sslSocketFactory = SSLHelper.getInstanceByKey(serverHostname, serverPort, caTop).getSocketFactory();
        
        // Create the SSLSocket
        //
        final SSLSocket sslSocket =
                (SSLSocket)sslSocketFactory.createSocket(host, port);
        
        // Set the SSLSocket Enabled Cipher Suites
        //
        final String enabledCipherSuites =
                java.lang.System.getProperty("javax.rmi.ssl.client.enabledCipherSuites");

        if (enabledCipherSuites != null) {
            StringTokenizer st = new StringTokenizer(enabledCipherSuites, ",");
            int tokens = st.countTokens();
            String [] enabledCipherSuitesList = new String[tokens];

            for (int i = 0; i < tokens; i++) {
                enabledCipherSuitesList[i] = st.nextToken();
            }

            try {
                sslSocket.setEnabledCipherSuites(enabledCipherSuitesList);
            } catch (IllegalArgumentException e) {
                throw (IOException)new IOException(e.getMessage()).initCause(e);
            }
        }
        // Set the SSLSocket Enabled Protocols
        //
        final String enabledProtocols =
                java.lang.System.getProperty("javax.rmi.ssl.client.enabledProtocols");

        if (enabledProtocols != null) {
            StringTokenizer st = new StringTokenizer(enabledProtocols, ",");
            int tokens = st.countTokens();
            String [] enabledProtocolsList = new String[tokens];

            for (int i = 0; i < tokens; i++) {
                enabledProtocolsList[i] = st.nextToken();
            }

            try {
                sslSocket.setEnabledProtocols(enabledProtocolsList);
            } catch (IllegalArgumentException e) {
                throw (IOException)
                new IOException(e.getMessage()).initCause(e);
            }
        }
        // Return the preconfigured SSLSocket
        //
        return sslSocket;
    }

}
