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

import com.sun.grid.util.SGEUtil;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.callback.TextInputCallback;
import javax.security.auth.callback.TextOutputCallback;
import javax.security.auth.callback.UnsupportedCallbackException;

/**
 * This <code>CallbackHandler</code> reads input from the console.
 *
 * The following callback can be handled:
 *
 * <ul>
 *   <li>{@link javax.security.auth.callback.NameCallback}</li>
 *   <li>{@link javax.security.auth.callback.PasswordCallback}</li>
 *   <li>{@link javax.security.auth.callback.TextInputCallback}</li>
 *   <li>{@link javax.security.auth.callback.TextOutputCallback}</li>
 * </uL>
 *
 * It uses the native library juti to turn of echoing.
 *
 * @see com.sun.grid.util.SGEUtil#getPassword
 */
public class ConsoleCallbackHandler implements CallbackHandler {
    
    private BufferedReader reader;
    
    private String readLine() throws IOException {
       if(reader == null) {
           reader = new BufferedReader(new InputStreamReader(System.in));
       } 
       return reader.readLine();
    }
    
    /**
     * Handle callbacks
     * @param callbacks  array of callbacks 
     * @throws java.io.IOException  on an IO error
     * @throws javax.security.auth.callback.UnsupportedCallbackException if a callback
     *              is not supported
     */
    public void handle(Callback[] callbacks) throws IOException, UnsupportedCallbackException {
        
        for(int i = 0; i < callbacks.length; i++) {
            if(callbacks[i] instanceof NameCallback) {
                NameCallback cb = (NameCallback)callbacks[i];
                System.out.print(cb.getPrompt());
                if(cb.getDefaultName() != null) {
                    System.out.print(" [" + cb.getDefaultName() + "] ");
                }
                String name = readLine();
                if(name.length() == 0) {
                    name = cb.getDefaultName();
                }
                cb.setName(name);
            } else if(callbacks[i] instanceof TextOutputCallback) {
                TextOutputCallback cb = (TextOutputCallback)callbacks[i];
                switch(cb.getMessageType()) {
                    case TextOutputCallback.ERROR:
                        System.out.print(RB.rb().getString("ccb.error"));
                        break;
                    case TextOutputCallback.WARNING:
                        System.out.print(RB.rb().getString("ccb.warning"));
                        break;
                    default:
                        // do nothing
                }
                System.out.println(cb.getMessage());
            } else if(callbacks[i] instanceof TextInputCallback) {
                TextInputCallback cb = (TextInputCallback)callbacks[i];
                System.out.print(cb.getPrompt());
                if(cb.getDefaultText() != null) {
                    System.out.print(" [" + cb.getDefaultText() + "] ");
                }
                String text = readLine();
                if(text.length() == 0) {
                    text = cb.getDefaultText();
                }
                cb.setText(text);
            } else if(callbacks[i] instanceof PasswordCallback) {
                PasswordCallback cb = (PasswordCallback)callbacks[i];
                cb.setPassword(SGEUtil.getInstance().getPassword(cb.getPrompt()));
            } else {
                throw new UnsupportedCallbackException(callbacks[i]);
            }
        }
    }
    
}
