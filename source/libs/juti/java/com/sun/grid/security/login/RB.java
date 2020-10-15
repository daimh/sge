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

import java.text.MessageFormat;
import java.util.ResourceBundle;
import javax.security.auth.login.LoginException;

/**
 * Helper class i18n.
 *
 */
class RB {
    
    public static final String BUNDLE = "com.sun.grid.security.login.Resources";

    /**
     * Get the resource bundle for this package
     * @return the resource bundle
     */
    public static ResourceBundle rb() {
        return ResourceBundle.getBundle(BUNDLE);
    }
    
    public static String getString(String msg) {
        return rb().getString(msg);
    }

    public static String getString(String msg, Object param) {
        return getString(msg, new Object[] { param });
    }
    
    public static String getString(String msg, Object [] params) {
        String ret = rb().getString(msg);
        if(params != null) {
            ret = MessageFormat.format(ret, params);
        }
        return ret;
    }
    
    public static LoginException newLoginException(String msg) {
        return newLoginException(msg, null);
    }
    
    public static LoginException newLoginException(String msg, Object [] params) {
        String message = rb().getString(msg);
        if(params != null) {
            message = MessageFormat.format(message, params);
        }
        return new LoginException(message);
    }
    
    public static LoginException newLoginException(String msg, Throwable cause, Object [] params) {
        LoginException ret = newLoginException(msg, params);
        ret.initCause(cause);
        return ret;
    }
}
