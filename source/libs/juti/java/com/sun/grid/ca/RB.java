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

import java.text.MessageFormat;
import java.util.ResourceBundle;
import javax.security.auth.login.LoginException;

/**
 * Helper class for i18n.
 *
 */
class RB {
    
    public static final String BUNDLE = "com.sun.grid.ca.Resources";

    /**
     * Get the resource bundle for this package
     * @return the resource bundle
     */
    public static ResourceBundle rb() {
        return ResourceBundle.getBundle(BUNDLE);
    }
    
    /**
     * Get a string from the resource bundle
     * @param key the key for the string
     * @return the string
     */
    public static String getString(String key) {
        return rb().getString(key);
    }

    /**
     * Get a formatted message from the resource bundle
     * @param key    the key of the message
     * @param param  single parameter for the format
     * @return the formatted message
     */
    public static String getString(String key, Object param) {
        return getString(key, new Object[] { param });
    }
    
    /**
     * Get a formatted message from the resource bundle
     * @param key    the key of the message
     * @param param  arra with parameters for the format
     * @return the formatted message
     */
    public static String getString(String key, Object [] params) {
        String ret = rb().getString(key);
        if(params != null) {
            ret = MessageFormat.format(ret, params);
        }
        return ret;
    }
    
    /**
     * Create a <code>GridCAException</code> with a localized message
     * @param key  the key of the message 
     * @return the <code>GridCAException</code> with a localized message
     */
    public static GridCAException newGridCAException(String key) {
        return newGridCAException(key, null);
    }
    
    /**
     * Create a <code>GridCAException</code> with a localized message
     * @param key  the key of the message 
     * @param params parameters for the message
     * @return the <code>GridCAException</code> with a localized message
     */
    public static GridCAException newGridCAException(String key, Object params) {
        return newGridCAException(key, params == null ? null : new Object [] { params } );
    }
    
    /**
     * Create a <code>GridCAException</code> with a localized message
     * @param key  the key of the message 
     * @param params parameters for the message
     * @return the <code>GridCAException</code> with a localized message
     */
    public static GridCAException newGridCAException(String key, Object [] params) {
        String message = rb().getString(key);
        if(params != null) {
            message = MessageFormat.format(message, params);
        }
        return new GridCAException(message);
    }
    
    /**
     * Create a <code>GridCAException</code> with a localized message
     * @param key  the key of the message 
     * @param cause cause of the exception
     * @param params parameters for the message
     * @return the <code>GridCAException</code> with a localized message
     */
    public static GridCAException newGridCAException(Throwable cause, String key, Object params) {
        return newGridCAException(cause, key, 
                  params == null ? null : new Object [] { params });
    }
    
    /**
     * Create a <code>GridCAException</code> with a localized message
     * @param key  the key of the message 
     * @param cause cause of the exception
     * @param params parameters for the message
     * @return the <code>GridCAException</code> with a localized message
     */
    public static GridCAException newGridCAException(Throwable cause, String key, Object [] params) {
        GridCAException ret = newGridCAException(key, params);
        ret.initCause(cause);
        return ret;
    }
}
