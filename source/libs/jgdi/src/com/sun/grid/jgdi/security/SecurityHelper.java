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
package com.sun.grid.jgdi.security;

import com.sun.grid.jgdi.util.Base64;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.security.AccessControlContext;
import java.security.AccessController;
import java.security.cert.CertificateEncodingException;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.security.auth.Subject;
import javax.security.auth.x500.X500PrivateCredential;

/**
 *
 */
public class SecurityHelper {
    
    private static Logger logger = Logger.getLogger(SecurityHelper.class.getName());
    
    private static X500PrivateCredential getPrivateCredentials() {
        AccessControlContext ctx = AccessController.getContext();
        
        if (ctx == null) {
            return null;
        }
        Subject subject = Subject.getSubject(ctx);
        if (subject == null) {
            return null;
        }
        
        Set credSet = subject.getPrivateCredentials(X500PrivateCredential.class);
        if (credSet == null || credSet.isEmpty()) {
            return null;
        }
        return (X500PrivateCredential) credSet.iterator().next();
    }
    
    public static String getUsername() {
        
        String ret = null;
        X500PrivateCredential cred = getPrivateCredentials();
        if (cred == null) {
            Subject sub = Subject.getSubject(AccessController.getContext());
            if(sub != null) {
                Set p = sub.getPrincipals(JGDIPrincipal.class);
                if (p != null && !p.isEmpty()) {
                    ret = ((JGDIPrincipal)p.iterator().next()).getUsername();
                }
            }
            if (ret == null) {
                ret = System.getProperty("user.name");
            }
            
            
        } else {
            return cred.getAlias();
        }
        logger.log(Level.FINE, "user.name: {0}", ret);        
        return ret;
    }
    
    
    public static String getPrivateKey() {
        X500PrivateCredential cred = getPrivateCredentials();
        if (cred == null) {
            return null;
        }
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        
        
        pw.println("-----BEGIN PRIVATE KEY-----");
        String str = Base64.encode(cred.getPrivateKey().getEncoded());
        int lines = str.length() / 64;
        boolean lastline = (str.length() % 64) != 0;
        int i;
        for (i = 0; i < lines; i++) {
            pw.println(str.substring(i * 64, (i * 64) + 64));
        }
        if (lastline) {
            pw.println(str.substring(i * 64));
        }
        pw.println("-----END PRIVATE KEY-----");
        pw.close();
        return sw.getBuffer().toString();
    }
    
    public static String getCertificate() throws CertificateEncodingException {
        X500PrivateCredential cred = getPrivateCredentials();
        if (cred == null) {
            return null;
        }
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        pw.println("-----BEGIN CERTIFICATE-----");
        String str = Base64.encode(cred.getCertificate().getEncoded());
        int lines = str.length() / 64;
        boolean lastline = (str.length() % 64) != 0;
        int i;
        for (i = 0; i < lines; i++) {
            pw.println(str.substring(i * 64, (i * 64) + 64));
        }
        if (lastline) {
            pw.println(str.substring(i * 64));
        }
        pw.println("-----END CERTIFICATE-----");
        pw.close();
        return sw.getBuffer().toString();
    }
}
