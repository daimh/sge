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

import com.sun.grid.util.expect.Expect;
import com.sun.grid.util.expect.ExpectBuffer;
import com.sun.grid.util.expect.ExpectHandler;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.RandomAccessFile;
import java.io.StringWriter;
import java.io.Writer;
import java.nio.channels.FileLock;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Enumeration;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Default implementation of the <code>GridCA</code>
 *
 * Uses the sge_ca script which is delivered with gridengine to perform
 * actions on the gridengine ca.
 *
 */
public class GridCAImpl implements GridCA {
    
    private static final Logger LOGGER = Logger.getLogger(GridCAImpl.class.getName(), GridCAConstants.BUNDLE);
    
    private GridCAConfiguration config;
    
    /**
     * Create a new instance of <code>GridCAImp</code>
     * @param config the configuration
     * @throws com.sun.grid.ca.GridCAException if the configuration is not valid
     */
    public GridCAImpl(GridCAConfiguration config) throws GridCAException {
        config.validate();
        this.config = config;
    }
  
    protected Expect createProcess() {
        Expect ret = new Expect();
        ret.command().add(config.getSgeCaScript().getAbsolutePath());
        ret.command().add("-catop");
        ret.command().add(config.getCaTop().getAbsolutePath());
        ret.command().add("-calocaltop");
        ret.command().add(config.getCaLocalTop().getAbsolutePath());
        ret.command().add("-cahost");
        ret.command().add(config.getCaHost());
        ret.command().add("-adminuser");
        ret.command().add(config.getAdminUser());
        ret.command().add("-nosge");
        ret.command().add("-days");
        ret.command().add(Integer.toString(config.getDaysValid()));
        return ret;
    }
    
    /**
     * Initialize the gridengine ca.
     *
     * @param  params  parmeters for the CA
     * @throws com.sun.grid.ca.GridCAException 
     */
    public void init(InitCAParameters params) throws GridCAException {
        LOGGER.entering("GridCAImpl", "init");

        params.validate();
        
        File autoFile = createTempFile("auto", ".conf");
        try {
            try {
                PrintWriter pw = new PrintWriter(new FileWriter(autoFile));

                pw.print("CSP_COUNTRY_CODE=\"");
                pw.print(params.getCountry());
                pw.println("\"");
                pw.print("CSP_STATE=\"");
                pw.print(params.getState());
                pw.println("\"");
                pw.print("CSP_LOCATION=\"");
                pw.print(params.getLocation());
                pw.println("\"");
                pw.print("CSP_ORGA=\"");
                pw.print(params.getOrganization());
                pw.println("\"");
                pw.print("CSP_ORGA_UNIT=\"");
                pw.print(params.getOrganizationUnit());
                pw.println("\"");
                pw.print("CSP_MAIL_ADDRESS=\"");
                pw.print(params.getAdminEmailAddress());
                pw.println("\"");
                pw.close();
                
                LOGGER.log(Level.FINE, "gridCAImpl.autoFileWritten");
            } catch(IOException ioe) {
                throw RB.newGridCAException(ioe, "gridCAImpl.error.autoFile",
                                            ioe.getLocalizedMessage());
            }
            
            Expect pb = createProcess();
            pb.command().add("-auto");
            pb.command().add(autoFile.getAbsolutePath());
            pb.command().add("-init");
            
            execute(pb, false);
            
            LOGGER.log(Level.FINE, "gridCAImpl.initSuccess");

        } finally {
            if(autoFile != null) {
                autoFile.delete();
            }
        }
    }
    
    protected File getLocalUserDir(String username) {
        return new File(config.getCaLocalTop(), "userkeys" + File.separatorChar + username);
    }
    
    protected File getCertFileForUser(String username) {
        return new File(getLocalUserDir(username), "cert.pem");
    }

    protected File getLocalDaemonDir(String daemon) {
        return new File(config.getCaLocalTop(), "daemons" + File.separatorChar + daemon);
    }
    
    protected File getCertFileForDaemon(String daemon) {
        return new File(getLocalDaemonDir(daemon), "cert.pem");
    }


    
    protected void execute(Expect pb)  throws GridCAException {
        execute(pb, true);
    } 
    
    protected void execute(Expect pb, boolean setLock) throws GridCAException {
        LOGGER.entering("GridCAImpl", "execute");

        // Since the sge_ca infrastructure can not be shared
        // between processes we have to create a file lock
        File lockFile = null;
        RandomAccessFile raf = null;
        FileLock lock = null;
        if(setLock) {
            lockFile = new File(config.getCaLocalTop(), "lock");
            
            try {
                raf = new RandomAccessFile(lockFile, "rw");
            } catch (IOException ex) {
                throw RB.newGridCAException(ex, "gridCAImpl.error.lockFileNotFound",
                        new Object [] { lockFile, ex.getLocalizedMessage() });
            }


            
            try {
                lock = raf.getChannel().lock();
            } catch (IOException ex) {
                throw RB.newGridCAException(ex, "gridCAImpl.error.lock", 
                          new Object [] { lockFile, ex.getLocalizedMessage() });
            }
        }        
        try {
            ErrorHandler eh = new ErrorHandler();
            pb.add(eh);
            int res;
            try {
                res = pb.exec(60 * 1000);
            } catch (IOException ex) {
                throw RB.newGridCAException(ex, "gridCAImpl.sge_ca.ioError",
                                            ex.getLocalizedMessage());
            } catch (InterruptedException ex) {
                throw RB.newGridCAException("gridCAImpl.sge_ca.interrupt");
            }
            if(res != 0) {
                String error = eh.getError();
                if(error == null || error.length() == 0) {
                    throw RB.newGridCAException("gridCAImpl.sge_ca.unknownError",
                                                new Integer(res));
                } else {
                    throw RB.newGridCAException("gridCAImpl.sge_ca.error",
                                                new Object [] { new Integer(res), error });
                }
            }
        } finally {
            if (setLock) {
                try {
                    lock.release();
                } catch (IOException ex) {
                    // Ingore
                }
                try {
                    raf.close();
                } catch (IOException ex) {
                    // Ingore
                }
            }
        }
        LOGGER.exiting("GridCAImpl", "execute");
    }
    
    class ErrorHandler implements ExpectHandler {
        
        private String error;
        
        public void handle(Expect expect, ExpectBuffer buffer) throws IOException {
            if(getError() == null) {
                error = buffer.consumeLine("Error:");
            }
        }

        public String getError() {
            return error;
        }
    }
    
    /**
     *  Create private key and certificate for a user.
     *
     *  @param username  name of the user
     *  @param gecos     gecos field of the user
     *  @param email     email address of the user
     *  @deprecated the gecos field is no longer used, use @{link #createUser(String,String)} instead
     *  @throws GridCAException if the creation of the private key or the certificate fails
     */
    public void createUser(String username, String gecos, String email) throws GridCAException {
        LOGGER.entering("GridCAImpl", "createUser");
        createUser(username, email);
        LOGGER.exiting("GridCAImpl", "createUser");
    }
    /**
     *  Create private key and certificate for a user.
     *
     *  @param username  name of the user
     *  @param email     email address of the user
     *  @throws GridCAException if the creation of the private key or the certificate fails
     */
    public void createUser(String username, String email) throws GridCAException {
        LOGGER.entering("GridCAImpl", "createUser");
        Expect pb = createProcess();
        pb.command().add("-user");
        pb.command().add(username + ":" + username + ":" + email);

        execute(pb);
        LOGGER.exiting("GridCAImpl", "createUser");
    }
    
    
    /**
     * Create private key and certificate for a sdm daemon.
     *
     * @param daemon name of the daemon
     * @param user   username of the daemon (owner of the process)
     * @param email  email address of the process owner
     * @throws com.sun.grid.ca.GridCAException if the create of the daemon failed
     */
    public void createDaemon(String daemon, String user, String email) throws GridCAException {
        LOGGER.entering("GridCAImpl", "createDaemon");
        Expect pb = createProcess();
        pb.command().add("-sdm_daemon");
        pb.command().add(user + ":" + daemon + ":" + email);

        execute(pb);
        
        LOGGER.exiting("GridCAImpl", "createDaemon");
    }
    
    
    
    private X509Certificate readCertificate(File certFile) throws GridCAException {
        LOGGER.entering("GridCAImpl", "readCertificate");
        try {
            if(!certFile.exists()) {
                throw RB.newGridCAException("gridCAImpl.error.certFileNotFound",
                                            certFile);
            }
            if(!certFile.canRead()) {
                throw RB.newGridCAException("gridCAImpl.error.certFileReadable",
                                            certFile);
            }
            
            FileInputStream in = new FileInputStream(certFile);
            try {
                LOGGER.exiting("GridCAImpl", "readCertificate");
                return (X509Certificate)CertificateFactory.getInstance("X.509").generateCertificate(in);
            } finally {
                in.close();
            }
        } catch(IOException ioe) {
            throw RB.newGridCAException(ioe, "gridCAImpl.error.certFileIOError",
                                        ioe.getLocalizedMessage());
        } catch(CertificateException ex) {
            throw RB.newGridCAException(ex, "gridCAImpl.error.certFileInvalid",
                                        new Object [] { certFile, ex.getLocalizedMessage() });
        }
    }
    
    /**
     *  Get the X.509 certificate of a user.
     *
     *  @param username  name of the user
     *  @return X.509 certificate
     *  @throws GridCAException if the certificate does not exist
     */
    public X509Certificate getCertificate(String username) throws GridCAException {
        LOGGER.entering("GridCAImpl", "getCertificate");
        File certFile = getCertFileForUser(username);

        X509Certificate ret = readCertificate(certFile);
        LOGGER.exiting("GridCAImpl", "getCertificate");
        return ret;
    }
    
    /**
     *  Get the X.509 certificate of a daemon.
     *
     *  @param daemon  name of the daemon
     *  @return X.509 certificate
     *  @throws GridCAException if the certificate does not exist
     */
    public X509Certificate getDaemonCertificate(String daemon) throws GridCAException {
        LOGGER.entering("GridCAImpl", "getDaemonCertificate");
        File certFile = getCertFileForDaemon(daemon);

        X509Certificate ret = readCertificate(certFile);
        LOGGER.exiting("GridCAImpl", "getDaemonCertificate");
        return ret;
    }
    
    
    /**
     *  Renew the certificate of a user.
     *
     *  @param username  name of the user
     *  @param days      validity of the new certificate in days
     *  @return the renewed certificate
     *  @throws GridCAException if the certificate can not be renewed
     */
    public X509Certificate renewCertificate(String username, int days) throws GridCAException {
        LOGGER.entering("GridCAImpl", "renewCertificate");
        
        Expect pb = createProcess();
        pb.command().add("-renew");
        pb.command().add(username);
        pb.command().add("-days");
        pb.command().add(Integer.toString(days));

        execute(pb);
        
        X509Certificate ret = getCertificate(username);
        LOGGER.exiting("GridCAImpl", "renewCertificate");
        return ret;
    }
    
    /**
     *  Renew the certificate of a daemon.
     *
     *  @param daemon  name of the daemon
     *  @param days      validity of the new certificate in days
     *  @return the renewed certificate
     *  @throws GridCAException if the certificate can not be renewed
     */
    public X509Certificate renewDaemonCertificate(String daemon, int days) throws GridCAException {
        LOGGER.entering("GridCAImpl", "renewDaemonCertificate");
        Expect pb = createProcess();
        pb.command().add("-renew_sdm");
        pb.command().add(daemon);
        pb.command().add("-days");
        pb.command().add(Integer.toString(days));

        execute(pb);

        X509Certificate ret = getDaemonCertificate(daemon);
        LOGGER.exiting("GridCAImpl", "renewDaemonCertificate");
        return ret;
    }
    
    public void renewCaCertificate(int days) throws GridCAException {
        LOGGER.entering("GridCAImpl", "renewCaCertificate");
        Expect pb = createProcess();
        pb.command().add("-renew_ca");
        pb.command().add("-days");
        pb.command().add(Integer.toString(days));

        execute(pb);

        LOGGER.exiting("GridCAImpl", "renewCaCertificate");
    }
    
    private File createTempFile(String prefix, String suffix) throws GridCAException {
        try {
            return File.createTempFile(prefix, suffix, config.getTmpDir());
        } catch(IOException ioe) {
            throw RB.newGridCAException(ioe, "gridCAImpl.error.tmpFile",
                                        new Object [] { config.getTmpDir(), ioe.getLocalizedMessage() });
        }
    }
    
    /**
     *  Create a keystore which contains the private key and
     *  certificate of an user.
     *
     *  @param  username         name of the user
     *  @param  keystorePassword password used for encrypt the keystore
     *  @param  privateKeyPassword password for the private key
     *  @return the keystore
     * @throws GridCAException if the keystore could not be created
     */
    public KeyStore createKeyStore(String username, char[] keystorePassword, char[] privateKeyPassword) throws GridCAException {
         return createKeyStore(GridCAX500Name.TYPE_USER, username, keystorePassword, privateKeyPassword);
    }
    
    /**
     * Get the keystore for a daemon.
     *
     * This method can be used be the installation to create keystore for
     * the daemon of a sdm system.
     *
     * @param daemon name of the daemon
     * @throws com.sun.grid.ca.GridCAException 
     * @return the keystore of the daemon
     */
    public KeyStore createDaemonKeyStore(String daemon) throws GridCAException {
         return createKeyStore(GridCAX500Name.TYPE_SDM_DAEMON, daemon, new char[0], new char[0]);
    }
    
    /**
     *  Get the keystore for a SGE daemon.
     *
     *  This method can be used be the installation to create keystore for
     *  the daemon of a sdm system.
     *
     *  @param daemon name of the daemon
     *  @param  keystorePassword password used to encrypt the keystore
     *  @param  privateKeyPassword password used to encrypt the key
     *  @throws com.sun.grid.ca.GridCAException 
     *  @return the keystore of the daemon
     */
    public KeyStore createSGEDaemonKeyStore(String daemon, char[] keystorePassword, char[] privateKeyPassword) throws GridCAException {
         return createKeyStore(GridCAX500Name.TYPE_SGE_DAEMON, daemon, keystorePassword, privateKeyPassword);
    }
    
    
    private KeyStore createKeyStore(String type, String entity, char[] keystorePassword, char[] privateKeyPassword) throws GridCAException {
        LOGGER.entering("GridCAImpl", "createKeyStore");
        
        if(keystorePassword == null) {
            throw RB.newGridCAException("gridCAImpl.error.emptyKeystorePassword", GridCAConstants.BUNDLE);
        }
        if(privateKeyPassword == null) {
            throw RB.newGridCAException("gridCAImpl.error.emptyPKPassword", GridCAConstants.BUNDLE);
        }
        
        KeyStore ks;
        try {
            ks = KeyStore.getInstance("JKS");
        } catch (KeyStoreException ex) {
            throw RB.newGridCAException(ex, "gridCAImpl.error.createKS",
                                        ex.getLocalizedMessage());
        }
        
        char [] pkcs12PW = "changeit".toCharArray();
        try {
            ks.load(null, keystorePassword);
        } catch (NoSuchAlgorithmException ex) {
            throw RB.newGridCAException(ex, "gridCAImpl.error.ksInit",
                                        ex.getLocalizedMessage());
        } catch (CertificateException ex) {
            throw RB.newGridCAException(ex, "gridCAImpl.error.ksInit",
                                        ex.getLocalizedMessage());
        } catch (IOException ex) {
            throw RB.newGridCAException(ex, "gridCAImpl.error.ksInit",
                                        ex.getLocalizedMessage());
        }

        KeyStore pkcs12Ks = createPKCS12KeyStore(type, entity, pkcs12PW);
        Enumeration aliases;
        try {
            aliases = pkcs12Ks.aliases();
        } catch (KeyStoreException ex) {
            throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.aliases",
                                        ex.getLocalizedMessage());
        }

        while(aliases.hasMoreElements()) {
            String alias = (String)aliases.nextElement();

            boolean isKeyEntry = false;
            try {
                isKeyEntry = pkcs12Ks.isKeyEntry(alias);
            } catch (KeyStoreException ex) {
                throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.isKeyEntry",
                                            new Object [] { alias, ex.getLocalizedMessage() });
            }
            if (isKeyEntry) {
                LOGGER.log(Level.FINE, "gridCAImpl.addKey", alias);

                Key key = null;
                try {
                    key = pkcs12Ks.getKey(alias, pkcs12PW);
                } catch(NoSuchAlgorithmException ex) {
                    throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.getKey",
                                                new Object [] { alias, ex.getLocalizedMessage() });
                } catch(UnrecoverableKeyException ex) {
                    throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.getKey",
                                                new Object [] { alias, ex.getLocalizedMessage() });
                } catch (KeyStoreException ex) {
                    throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.getKey",
                                                new Object [] { alias, ex.getLocalizedMessage() });
                }
                Certificate[] chain;
                try {
                    chain = pkcs12Ks.getCertificateChain(alias);
                } catch (KeyStoreException ex) {
                    throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.getCert",
                                                new Object [] { alias, ex.getLocalizedMessage() });
                }

                if(!(chain[0] instanceof X509Certificate)) {
                    throw RB.newGridCAException("gridCAImpl.error.pkcs12.invalidCert",
                                                alias);
                }
                X509Certificate cert = (X509Certificate)chain[0];

                GridCAX500Name name = GridCAX500Name.parse(cert.getSubjectX500Principal().getName());

                
                if(GridCAX500Name.TYPE_SDM_DAEMON.equals(type)) {
                    if(!name.isDaemon()) {
                        throw RB.newGridCAException("gridCAImpl.error.notADaemonCert", 
                                                    cert.getSubjectX500Principal().getName());
                    }
                    alias = name.getDaemonName();
                } else if(GridCAX500Name.TYPE_SGE_DAEMON.equals(type)) {
                    alias = name.getUsername();
                } else if (GridCAX500Name.TYPE_USER.endsWith(type)) {
                    if(name.isDaemon()) {
                        throw RB.newGridCAException("gridCAImpl.error.notAUserCert", 
                                                    cert.getSubjectX500Principal().getName());
                    }
                    alias = name.getUsername();
                }
                try {
                    ks.setKeyEntry(alias,key,privateKeyPassword,chain);
                } catch (KeyStoreException ex) {
                    throw RB.newGridCAException(ex, "gridCAImpl.error.ks.setKeyEntry",
                                                new Object [] { alias, ex.getLocalizedMessage() });
                }
                LOGGER.exiting("GridCAImpl", "createKeyStore");
                return ks;
            }
        }
        throw RB.newGridCAException("gridCaImpl.error.pkcs12.entityNotFound", entity);
    }
    
    
    
    private final KeyStore createPKCS12KeyStore(String type, String username, char [] password) throws GridCAException {
        LOGGER.entering("GridCAImpl", "createPKCS12KeyStore");
        File userFile = null;
        File passwordFile = null;
        File pkcs12File = new File(config.getTmpDir(), username + ".p12");        

        try {
            try {
                // write an empty password file, since the private keys are not crypted
                passwordFile = createTempFile("passin",".txt");

                try {
                    PrintWriter pw = new PrintWriter(new FileWriter(passwordFile));
                    pw.println(password);
                    pw.flush();
                    pw.close();
                } catch(IOException ex) {
                    throw RB.newGridCAException(ex, "gridCAImpl.error.io",ex.getLocalizedMessage());
                }

                Expect pb = createProcess();

                if(type.equals("user")) {
                    pb.command().add("-pkcs12");
                } else if (type.equals("sdm_daemon")) {
                    pb.command().add("-sdm_pkcs12");
                } else if (type.equals("sge_daemon")) {
                    pb.command().add("-sys_pkcs12");
                }
                pb.command().add(username);
                pb.command().add("-pkcs12pwf");
                pb.command().add(passwordFile.getAbsolutePath());
                pb.command().add("-pkcs12dir");
                pb.command().add(config.getTmpDir().getAbsolutePath());
                
                execute(pb);

            } finally {
                if(userFile != null) {
                    userFile.delete();
                }
                if(passwordFile != null) {
                    passwordFile.delete();
                }
            }
            try {
                KeyStore kspkcs12 = KeyStore.getInstance("PKCS12");
                kspkcs12.load(new FileInputStream(pkcs12File), password);
                
                LOGGER.exiting("GridCAImpl", "createPKCS12KeyStore");
                return kspkcs12;
            } catch (CertificateException ex) {
                throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.load",
                                            new Object [] { pkcs12File, ex.getLocalizedMessage() } );
            } catch (IOException ex) {
                throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.load",
                                            new Object [] { pkcs12File, ex.getLocalizedMessage() } );
            } catch (NoSuchAlgorithmException ex) {
                throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.load",
                                            new Object [] { pkcs12File, ex.getLocalizedMessage() } );
            } catch (KeyStoreException ex) {
                throw RB.newGridCAException(ex, "gridCAImpl.error.pkcs12.load",
                                            new Object [] { pkcs12File, ex.getLocalizedMessage() } );
            }        
        } finally {
            if(pkcs12File.exists()) {
                if(!pkcs12File.delete()) {
                    LOGGER.log(Level.WARNING,"gridCAImpl.warn.pkcs12.delete", pkcs12File);
                }
            }
        }
    }

    private static class OutputHandler implements Runnable {
        private InputStream in;
        private final StringWriter errors = new StringWriter();
        private PrintWriter pw;
        
        public OutputHandler(InputStream in, Writer out) {
            this.in = in;
            pw = new PrintWriter(out);
            
        }

        private static String [] ERROR_PATTERNS = {
            "Error:",
            "Error"
        };
        
        public void run() {
            
            PrintWriter epw = new PrintWriter(errors);
            try {
                String line = null;
                BufferedReader br = new BufferedReader(new InputStreamReader(in));
                boolean firstError = false;
                while((line = br.readLine()) != null) {
                    pw.println(line);
                    for(int i = 0; i < ERROR_PATTERNS.length; i++) {
                        if(line.startsWith(ERROR_PATTERNS[i])) {
                            if(firstError) {
                                firstError = false;
                            } else {
                                epw.println();
                            }
                            epw.print(line.substring(ERROR_PATTERNS[i].length()).trim());
                        }
                    }
                }
            } catch(IOException ioe) {
                LOGGER.log(Level.SEVERE, "gridCAImpl.error.outputIO", ioe);
            } finally {
                epw.close();
                pw.close();
            }
        }

        public String getError() {
            return errors.getBuffer().toString();
        }
    }
}


