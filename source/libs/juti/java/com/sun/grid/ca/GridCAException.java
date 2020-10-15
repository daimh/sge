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

/**
 * This exception wraps all errors of the GridCA
 *
 */
public class GridCAException extends Exception {

    
    /**
     * Creates a new instance of CAException
     * @param message the message
     */
    public GridCAException(String message) {
        super(message);
    }
    

    /**
     * Creates a new instance of GridCAException.
     *
     * @param message the message
     * @param cause   the cause of the exception
     */
    public GridCAException(String message, Throwable cause) {
        super(message, cause);
    }
    
    /**
     * Creates a new instance of GridCAException with
     * a localized message
     *
     * @param message the message
     * @param bundle  the resource bundle
     */
    public GridCAException(String message, String bundle) {
        this(message, bundle, null);
    }
    
    /**
     * Creates a new instance of GridCAException with
     * a localized message
     *
     * @param message the message
     * @param bundle  the resource bundle
     * @param params  parameters for the message
     */
    public GridCAException(String message, String bundle, Object [] params) {
        super(format(message, bundle, params));
    }
    
    /**
     * Creates a new instance of GridCAException with
     * a localized message
     *
     * @param message the message
     * @param cause   the cause of the error
     * @param bundle  the resource bundle
     * @param params  parameters for the message
     */
    public GridCAException(String message, Throwable cause, String bundle, Object [] params) {
        super(format(message, bundle, params), cause);
    }
    
    private static String format(String message, String bundle, Object [] params) {
        String msg = ResourceBundle.getBundle(bundle).getString(message);
        if(params != null) {
            msg = MessageFormat.format(msg, params);
        }
        return msg;
    }
    
}
