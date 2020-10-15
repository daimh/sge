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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PushbackInputStream;
import java.util.Arrays;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.ConfirmationCallback;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.callback.TextOutputCallback;
import javax.security.auth.callback.UnsupportedCallbackException;

/**
 *
 */
public class TextCallbackHandler implements CallbackHandler {
   
   /**
    * <p>Creates a callback handler that prompts and reads from the
    * command line for answers to authentication questions.
    * This can be used by JAAS applications to instantiate a
    * CallbackHandler.
    */
   public TextCallbackHandler() { }
   
   /**
    * Handles the specified set of callbacks.
    *
    * @param callbacks the callbacks to handle
    * @throws IOException if an input or output error occurs.
    * @throws UnsupportedCallbackException if the callback is not an
    * instance of NameCallback or PasswordCallback
    */
   public void handle(Callback[] callbacks)
   throws IOException, UnsupportedCallbackException {
      ConfirmationCallback confirmation = null;
      
      for (int i = 0; i < callbacks.length; i++) {
         if (callbacks[i] instanceof TextOutputCallback) {
            TextOutputCallback tc = (TextOutputCallback) callbacks[i];
            
            String text;
            switch (tc.getMessageType()) {
               case TextOutputCallback.INFORMATION:
                  text = "";
                  break;
               case TextOutputCallback.WARNING:
                  text = "Warning: ";
                  break;
               case TextOutputCallback.ERROR:
                  text = "Error: ";
                  break;
               default:
                  throw new UnsupportedCallbackException(
                          callbacks[i], "Unrecognized message type");
            }
            
            String message = tc.getMessage();
            if (message != null) {
               text += message;
            }
            if (text != null) {
               System.err.println(text);
            }
            
         } else if (callbacks[i] instanceof NameCallback) {
            NameCallback nc = (NameCallback) callbacks[i];
            
            if (nc.getDefaultName() == null) {
               System.err.print(nc.getPrompt());
            } else {
               System.err.print(nc.getPrompt() +
                       " [" + nc.getDefaultName() + "] ");
            }
            System.err.flush();
            
            String result = readLine();
            if (result.equals("")) {
               result = nc.getDefaultName();
            }
            
            nc.setName(result);
            
         } else if (callbacks[i] instanceof PasswordCallback) {
            PasswordCallback pc = (PasswordCallback) callbacks[i];
            
            System.err.print(pc.getPrompt());
            System.err.flush();
            
            pc.setPassword(readPassword(System.in));
            
         } else if (callbacks[i] instanceof ConfirmationCallback) {
            confirmation = (ConfirmationCallback) callbacks[i];
            
         } else {
            throw new UnsupportedCallbackException(
                    callbacks[i], "Unrecognized Callback");
         }
      }
      
      /* Do the confirmation callback last. */
      if (confirmation != null) {
         doConfirmation(confirmation);
      }
   }
   
   /* Reads a line of input */
   private String readLine() throws IOException {
      return new BufferedReader
              (new InputStreamReader(System.in)).readLine();
   }
   
   /* Reads a user password from an input stream */
   private char[] readPassword(InputStream in) throws IOException {
      
      char[] lineBuffer;
      char[] buf;
      int i;
      
      buf = lineBuffer = new char[128];
      
      int room = buf.length;
      int offset = 0;
      int c;
      
      boolean done = false;
      while (!done) {
         switch (c = in.read()) {
            case -1:
            case '\n':
               done = true;
               break;
               
            case '\r':
               int c2 = in.read();
               if ((c2 != '\n') && (c2 != -1)) {
                  if (!(in instanceof PushbackInputStream)) {
                     in = new PushbackInputStream(in);
                  }
                  ((PushbackInputStream)in).unread(c2);
               } else {
                  done = true;
                  break;
               }
               
            default:
               if (--room < 0) {
                  buf = new char[offset + 128];
                  room = buf.length - offset - 1;
                  System.arraycopy(lineBuffer, 0, buf, 0, offset);
                  Arrays.fill(lineBuffer, ' ');
                  lineBuffer = buf;
               }
               buf[offset++] = (char) c;
               break;
         }
      }
      
      if (offset == 0) {
         return null;
      }
      
      char[] ret = new char[offset];
      System.arraycopy(buf, 0, ret, 0, offset);
      Arrays.fill(buf, ' ');
      
      return ret;
   }
   
   private void doConfirmation(ConfirmationCallback confirmation)
   throws IOException, UnsupportedCallbackException {
      String prefix;
      int messageType = confirmation.getMessageType();
      switch (messageType) {
         case ConfirmationCallback.WARNING:
            prefix =  "Warning: ";
            break;
         case ConfirmationCallback.ERROR:
            prefix = "Error: ";
            break;
         case ConfirmationCallback.INFORMATION:
            prefix = "";
            break;
         default:
            throw new UnsupportedCallbackException(
                    confirmation, "Unrecognized message type: " + messageType);
      }
      
      class OptionInfo {
         String name;
         int value;
         OptionInfo(String name, int value) {
            this.name = name;
            this.value = value;
         }
      }
      
      OptionInfo[] options;
      int optionType = confirmation.getOptionType();
      switch (optionType) {
         case ConfirmationCallback.YES_NO_OPTION:
            options = new OptionInfo[] {
               new OptionInfo("Yes", ConfirmationCallback.YES),
               new OptionInfo("No", ConfirmationCallback.NO)
            };
            break;
         case ConfirmationCallback.YES_NO_CANCEL_OPTION:
            options = new OptionInfo[] {
               new OptionInfo("Yes", ConfirmationCallback.YES),
               new OptionInfo("No", ConfirmationCallback.NO),
               new OptionInfo("Cancel", ConfirmationCallback.CANCEL)
            };
            break;
         case ConfirmationCallback.OK_CANCEL_OPTION:
            options = new OptionInfo[] {
               new OptionInfo("OK", ConfirmationCallback.OK),
               new OptionInfo("Cancel", ConfirmationCallback.CANCEL)
            };
            break;
         case ConfirmationCallback.UNSPECIFIED_OPTION:
            String[] optionStrings = confirmation.getOptions();
            options = new OptionInfo[optionStrings.length];
            for (int i = 0; i < options.length; i++) {
               options[i].value = i;
            }
            break;
         default:
            throw new UnsupportedCallbackException(
                    confirmation, "Unrecognized option type: " + optionType);
      }
      
      int defaultOption = confirmation.getDefaultOption();
      
      String prompt = confirmation.getPrompt();
      if (prompt == null) {
         prompt = "";
      }
      prompt = prefix + prompt;
      if (!prompt.equals("")) {
         System.err.println(prompt);
      }
      
      for (int i = 0; i < options.length; i++) {
         if (optionType == ConfirmationCallback.UNSPECIFIED_OPTION) {
            // defaultOption is an index into the options array
            System.err.println(
                    i + ". " + options[i].name +
                    (i == defaultOption ? " [default]" : ""));
         } else {
            // defaultOption is an option value
            System.err.println(
                    i + ". " + options[i].name +
                    (options[i].value == defaultOption ? " [default]" : ""));
         }
      }
      System.err.print("Enter a number: ");
      System.err.flush();
      int result;
      try {
         result = Integer.parseInt(readLine());
         if (result < 0 || result > (options.length - 1)) {
            result = defaultOption;
         }
         result = options[result].value;
      } catch (NumberFormatException e) {
         result = defaultOption;
      }
      
      confirmation.setSelectedIndex(result);
   }
}
