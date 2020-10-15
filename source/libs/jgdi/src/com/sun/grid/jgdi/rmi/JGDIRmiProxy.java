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
package com.sun.grid.jgdi.rmi;

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.logging.Logger;
import java.rmi.Remote;
import java.rmi.server.Unreferenced;

/**
 *  Reference implementation of a JGDI Rmi proxy.
 *
 *  <p>The rmi proxy can be started from the commandline. Use the <code>-help</code>
 *     to get more information.</p>
 *
 *  <p>The JGDI Rmi proxy needs shared libs of the Sun&trade; Grid Engine in the
 *     <code>LD_LIBRARY_PATH</code>.</p>
 *
 *  <p><b>Example:</b></p>
 *  <pre>
 *  % setenv LD_LIBRARY_PATH $SGE_ROOT/lib/sol-sparc64
 *  % java -d64 -cp $SGE_ROOT/lib/jgdi.jar com.sun.grid.jgdi.rmi.JGDIRmiProxy -help
 *  ...
 *  </pre>
 *
 *  <p>The JGDIRMIProxy do not install a <code>SecurityManager</code>. Please use the
 *     <code>java.security.manager</code> system property to install <code>SecurityManager</code>.
 *     The JGDIRMIProxy needs the following permissions:</p>
 *
 *  <pre>
 *  permission java.lang.RuntimePermission "loadLibrary.jgdi"
 *  permission java.lang.RuntimePermission "shutdownHooks"
 *  </pre>
 *
 *
 *
 *  <p>This class uses the logger <code>com.sun.grid.jgdi.rmi</code>.
 *
 */
public class JGDIRmiProxy implements Unreferenced {
    
    private static Logger logger = Logger.getLogger("com.sun.grid.jgdi.rmi");
    
    /**
     * Main method of the <code>JGDIRMIProxy</code>. Please use the <code>-help</code> option
     * for a detailed usage message.
     *
     * @param args  arguments for the <code>JGDIRMIProxy</code>
     */
    public final static void main(String [] args) {
        
        if( args.length < 2 ) {
            usage("Invalid number of arguments", 1);
        }
        
        String registryHost = null;
        int    registryPort = 0;
        
        int i = 0;
        while( i < args.length - 2) {
            if(args[i].equals("-help")) {
                usage(null, 0);
            } else if(args[i].equals("-reg")) {
                i++;
                if (i>=args.length-2) {
                    usage("-reg option needs <host>[:<port>]", 1);
                }
                
                int del = args[i].indexOf(':');
                if(del < 0 ) {
                    registryHost = args[i];
                    if(registryHost.equals("local")) {
                        usage("For a local registry a port in mandatory", 1);
                    }
                } else {
                    registryHost = args[i].substring(0,del);
                    String portString = args[i].substring(del+1);
                    try {
                        registryPort = Integer.parseInt(portString);
                    } catch(NumberFormatException nfe) {
                        usage("Invalid port " + portString, 1);
                    }
                }
            } else {
                usage("Invalid option " + args[i], 1);
            }
            i++;
        }
        
        String serviceName = args[i++];
        String sge_url = args[i++];
        
        try {
            
            Registry reg = null;
            if(registryHost == null) {
                reg = LocateRegistry.getRegistry();
            } else if (registryHost.equals("local")) {
                reg = LocateRegistry.createRegistry(registryPort);
                logger.info("local registry at port " + registryPort + " created" );
            } else if ( registryPort == 0 ) {
                logger.info("Get registry from " + registryHost);
                reg = LocateRegistry.getRegistry(registryHost);
            } else {
                logger.info("Get registry from " + registryHost + ":" + registryPort);
                reg = LocateRegistry.getRegistry(registryHost, registryPort);
            }
            JGDIRemoteFactoryImpl service = new JGDIRemoteFactoryImpl(sge_url);
            Thread.sleep(1000);
            Remote stub = (Remote)UnicastRemoteObject.exportObject(service, 0);
            
            reg.bind(serviceName, stub);
            
            logger.info("JGDIRemoteFactory bound to name " + serviceName );
            
            ShutdownHook shutdownHook = new ShutdownHook(reg, serviceName);
            Runtime.getRuntime().addShutdownHook(shutdownHook);
            
            shutdownHook.waitForShutdown();
            
        } catch( Exception e ) {
            e.printStackTrace();
        }
    }
    
    private static class ShutdownHook extends Thread {
        
        private Registry registry;
        private String serviceName;
        
        public ShutdownHook(Registry registry, String serviceName ) {
            this.registry = registry;
            this.serviceName = serviceName;
        }
        
        public void run() {
            try {
                registry.unbind(serviceName);
            } catch(Exception re) {
                logger.throwing(getClass().getName(), "run", re);
            } finally {
                synchronized(this) {
                    notifyAll();
                }
            }
        }
        
        public synchronized void waitForShutdown() throws InterruptedException {
            wait();
        }
    }
    
    
    private static void usage(String msg, int exitCode ) {
        if( msg != null ) {
            System.err.println(msg);
        }
        System.err.println("JGDIRmiProxy [options] <service name> <sge_url>");
        System.err.println();
        System.err.println("  <service name>   Bind name for the rmi service");
        System.err.println("  <sge_url>        Grid Engine connection URL");
        System.err.println("  Options:");
        System.err.println("    -help                 Print this help message");
        System.err.println("    -reg  <host>[:port]   Host and port of the rmi registry.");
        System.err.println("                          If <host> is equal to \"local\" then");
        System.err.println("                          the rmi proxy creates a new rmi registry");
        System.err.println("                          which binds port <port>.");
        System.err.println("                          Port defaults to 1099 and host to localhost");
        System.exit(exitCode);
    }
    
    public void unreferenced() {
        System.out.println("unreferenced");
    }
}
