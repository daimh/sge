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
package com.sun.grid.ca;

import com.sun.grid.security.login.ConsoleCallbackHandler;
import com.sun.grid.security.login.TextCallbackHandler;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.security.KeyStore;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.ResourceBundle;
import java.util.logging.Logger;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.callback.UnsupportedCallbackException;

/**
 * CLI util for the <code>GridCA</code>.
 *
 * <H3>Usage of the CLI util</H3>
 *
 *<pre>
 * com.sun.grid.ca.Main [options] &lt;command&gt;
 *
 *  Commands:
 *
 *     init                initialize CA infrastructure
 *     user &lt;u:g:e&gt;        generate certificates and keys for &lt;u:g:e&gt;
 *                         with u=Unix User, g=Common Name, e=email
 *     initks &lt;type&gt; &lt;u&gt; &lt;file&gt; [&lt;pwfile&gt;]  create keystore for user &lt;u&gt;
 *     printcert &lt;u&gt;       print certificate of a user
 *     renew &lt;u&gt; [&lt;days&gt;]  renew certificate for a user
 *
 *  Mandatory Options:
 *    -catop      &lt;dir&gt;       path to the ca root directory
 *    -calocaltop &lt;dir&gt;       path to the local ca root directory
 *    -cascript &lt;script&gt;      path to the sge_ca script
 *
 *  Optional Options:
 *    -tmp &lt;dir&gt;              path tmp files (default system property java.io.tmp)
 *    -config &lt;dir&gt;           path to CA configuration files (default $cadist/util/sgeCA
 *    -adminuser &lt;user&gt;       name of the admin user (default system proeprty user.name)
 *    -days &lt;days&gt;            number of days the certificates are valid (default 365 days)
 *
 * </pre>
 *
 */
public class Main {
    
    private static final Logger LOGGER = Logger.getLogger(Main.class.getName());
    
    private static GridCA ca;
    
    private static void usage(String msg, int exitCode) {
        
        if(msg != null) {
            System.err.println(msg);
        }
        String usage = ResourceBundle.getBundle(GridCAConstants.BUNDLE).getString("main.usage");
        usage = MessageFormat.format(usage, new Object [] { Main.class.getName() });
        
        System.err.println(usage);
        
        System.exit(exitCode);
    }
    
    private static int consumeArgument(GridCAConfiguration config, String [] args, int index) throws GridCAException {
        
        if(args[index].equals("-catop")) {
            index++;
            if(index >= args.length) {
                throw new GridCAException("-catop option requires <catop>");
            }
            config.setCaTop(new File(args[index]));
            return ++index;
        } else if(args[index].equals("-calocaltop")) {
            index++;
            if(index >= args.length) {
                throw new GridCAException("-calocaltop option requires <calocaltop>");
            }
            config.setCaLocalTop(new File(args[index]));
            return ++index;
        } else if(args[index].equals("-tmpdir")) {
            index++;
            if(index >= args.length) {
                throw new GridCAException("-tmp option requires <tmp>");
            }
            config.setTmpDir(new File(args[index]));
            return ++index;
        } else if(args[index].equals("-adminuser")) {
            index++;
            if(index >= args.length) {
                throw new GridCAException("-adminuser option requires <adminuser>");
            }
            config.setAdminUser(args[index]);
            return ++index;
        } else if(args[index].equals("-cascript")) {
            index++;
            if(index >= args.length) {
                throw new GridCAException("-cascript option requires <script>");
            }
            config.setSgeCaScript(new File(args[index]));
            return ++index;
        } else if(args[index].equals("-cahost")) {
            index++;
            if(index >= args.length) {
                throw new GridCAException("-cahost option requires <host>");
            }
            config.setCaHost(args[index]);
            return ++index;
        } else if(args[index].equals("-days")) {
            index++;
            if(index >= args.length) {
                throw new GridCAException("-days option requires <days>");
            }
            try {
                config.setDaysValid(Integer.parseInt(args[index]));
            } catch (NumberFormatException ne) {
                throw new GridCAException("argument for -days must be an integer");
            }   
            return ++index;
        } else {
            // Unknown option
            return 0;
        }
    }
    
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        
        int argIndex = 0;
        GridCAConfiguration config = new GridCAConfiguration();
        config.setCaHost("localhost");
        try {
            // Parse the cmdArgs
            while(argIndex < args.length) {
                int newIndex = consumeArgument(config, args, argIndex);
                if(newIndex > 0) {
                    argIndex = newIndex;
                } else {
                    break;
                }
            }
            
            ca = GridCAFactory.newInstance(config);
        } catch (GridCAException ex) {
            usage(ex.getMessage(), 1);
            System.exit(1);
        }
        
        if(argIndex >= args.length) {
            usage("Missing command", 1);
        }
        
        String cmd = args[argIndex++];
        
        ArrayList cmdArgs = new ArrayList(args.length-argIndex);
        while(argIndex < args.length) {
            cmdArgs.add(args[argIndex++]);
        }
        
        if ("init".equals(cmd)) {
            init();
        } else if ("create".equals(cmd)) {
            create(cmdArgs);
        } else if ("initks".equals(cmd)) {
            initks(cmdArgs);
        } else if ("printcert".equals(cmd)) {
            printCert(cmdArgs);
        } else if ("renew".equals(cmd)) {
            renew(cmdArgs);
        } else {
            usage("Unknown command " + cmd, 1);
        }
    }
    
    
    private static void init() {
        
        try {
            ConsoleCallbackHandler cbh = new ConsoleCallbackHandler();
            InitCAParameters params = InitCAParameters.queryNewInstance(cbh);
            ca.init(params);
        } catch (UnsupportedCallbackException ex) {
            IllegalStateException ex1 = new IllegalStateException("unexpected error");
            ex1.initCause(ex);
            throw ex1;
        } catch (IOException ex) {
            System.err.println(ex.getMessage());
            System.exit(1);
        } catch (GridCAException ex) {
            ex.printStackTrace();
            System.exit(1);
        }
    }
    
    
    public static final int TYPE_USER = 1;
    public static final int TYPE_DAEMON = 2;
    public static final int TYPE_SGE_DAEMON = 3;
    
    public static int parseType(String type) {
        if(type.equals("user")) {
            return TYPE_USER;
        } else if (type.equals("daemon")) {
            return TYPE_DAEMON;
        } else if (type.equals("sge_daemon")) {
            return TYPE_SGE_DAEMON;
        } else {
            usage("invalid <type> '" + type + "'", 1);
            return 0;
        }
    }
    
    private static void create(ArrayList args) {
        
        
        if(args.size() != 2) {
            usage("Invalid number of arguments for the create command", 1);
        }
        
        int type = parseType((String)args.get(0));
        
        String [] userSpec = ((String)args.get(1)).split(":");
        if(userSpec.length != 3) {
            usage("Invalid user definition '" + args.get(1) + "'", 1);
        }
        try {
            switch(type) {
                case TYPE_USER:
                    ca.createUser(userSpec[0], userSpec[2]);
                    break;
                case TYPE_DAEMON:
                    ca.createDaemon(userSpec[0], userSpec[1], userSpec[2]);
                    break;
                default:
                    throw new IllegalStateException("unknown type " + type);
                    
            }
        } catch (GridCAException ex) {
            System.err.println(ex.getMessage());
            System.exit(1);
        }
    }
    
    private static void renew(ArrayList args) {
        
        if(args.size() != 3) {
            usage("Invalid number of arguments for renew command", 1);
        }
        int type = parseType((String)args.get(0));
        
        String name = (String)args.get(1);
        
        int days = -1;
        try {
            days = Integer.parseInt((String)args.get(2));
        } catch(NumberFormatException nfe) {
            usage("invalid <days>", 1);
        }
        try {
            X509Certificate cert = null;
            switch(type) {
                case TYPE_USER:
                    cert = ca.renewCertificate(name, days);
                    break;
                case TYPE_DAEMON:
                    cert = ca.renewDaemonCertificate(name, days);
                    break;
                default:
                    throw new IllegalStateException("unknown type " + type);
                    
            }
            printCert(cert);
        } catch(Exception ex) {
            System.err.println(ex.getMessage());
            System.exit(1);
        }
        
    }
    
    private static void initks(ArrayList args) {
        
        int type = 0;
        String name = null;
        File keyStoreFile = null;
        File pwFile = null;
        
        switch(args.size()) {
            case 4:
                pwFile = new File((String)args.get(3));
                // fall through
            case 3:
                type = parseType((String)args.get(0));
                name = (String)args.get(1);
                keyStoreFile = new File((String)args.get(2));
                break;
            default:
                usage("Invalid number of arguments for initks command", 1);
        }
        try {
            
            
            KeyStore ks = null;
            char [] pw = null;
            switch(type) {
                case TYPE_USER:
                case TYPE_SGE_DAEMON:    
                    if(pwFile == null) {
                        CallbackHandler cbh = new TextCallbackHandler();
                        
                        PasswordCallback keystorePWCallback = new PasswordCallback("keystore password: ", false);
                        
                        Callback [] cb = new Callback [] {
                            keystorePWCallback,
                        };
                        
                        cbh.handle(cb);
                        pw = keystorePWCallback.getPassword();
                        if(pw == null) {
                            pw = new char[0];
                        }
                    } else {
                        FileReader fr = new FileReader(pwFile);
                        BufferedReader br = new BufferedReader(fr);
                        String line = br.readLine();
                        if(line == null || line.length() == 0) {
                            pw = new char[0];
                        } else {
                            pw = line.toCharArray();
                        }
                    }
                    if (type == TYPE_USER) {
                        ks = ca.createKeyStore(name, pw, pw);
                    } else {
                        ks = ca.createSGEDaemonKeyStore(name, pw, pw);
                    }    
                    break;
                case TYPE_DAEMON:
                    pw = new char[0];
                    ks = ca.createDaemonKeyStore(name);
                    break;
                default:
                    throw new IllegalStateException("unknown type " + type);
                    
            }
            ks.store(new FileOutputStream(keyStoreFile), pw);
        } catch (Exception ex) {
            System.err.println(ex.getMessage());
            System.exit(1);
        }
    }
    
    private static void printCert(X509Certificate cert) throws CertificateException {
        System.out.println(cert);
    }
    
    
    private static void printCert(ArrayList args) {
        
        if(args.size() != 2) {
            usage("Invalid number of arguments for printcert command", 1);
        }
        int type = parseType((String)args.get(0));
        String name = (String)args.get(1);
        
        try {
            X509Certificate cert = null;
            switch(type) {
                case TYPE_USER:
                    cert = ca.getCertificate(name);
                    break;
                case TYPE_DAEMON:
                    cert = ca.getDaemonCertificate(name);
                    break;
                default:
                    throw new IllegalStateException("unknown type " + type);
                    
            }
            printCert(cert);
        } catch(Exception ex) {
            System.err.println(ex.getMessage());
            System.exit(1);
        }
        
    }
}
