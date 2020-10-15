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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

package com.sun.grid.installer.util.cmd;

import com.izforge.izpack.util.Debug;
import com.sun.grid.installer.gui.Host;
import com.sun.grid.installer.util.Util;
import java.text.MessageFormat;

public class RemoteComponentScriptCommand extends CmdExec {
      //E.g.: rsh/ssh [-l <connect_user>] [-o ....] HOSTNAME INSTALL_SCRIPT/CHECK_HOST arg_component
      private static String installCommand = "{0} {1} {2} {3} \"\\\"{4}\\\" {5}\"";
      //E.g.: INSTALL_SCRIPT/CHECK_HOST arg_component
      private static String localInstallCommand = "\"{0}\" {1}";
      //private static String installCommand = "{0} {1} {2} {3} {4} {5} ; rm -f {4}";
      private String command;

      public RemoteComponentScriptCommand(Host host, String user, String shell, boolean isWindowsMode, String installScript) {
          this(Util.DEF_INSTALL_TIMEOUT, host, user, shell, isWindowsMode, installScript);
      }

      public RemoteComponentScriptCommand(long timeout, Host host, String user, String shell, boolean isWindowsMode, String installScript) {
          super(timeout);
          String hostname = host.getHostname();
          boolean onLocalHost = hostname.equalsIgnoreCase(Host.localHostName);

          String scriptArgs = String.valueOf(host.isBdbHost()) + " " + String.valueOf(host.isQmasterHost()) + " " + String.valueOf(host.isShadowHost()) + " " + String.valueOf(host.isExecutionHost()); //4

          if (onLocalHost) {
             this.command = MessageFormat.format(localInstallCommand, installScript, scriptArgs).trim();
             return;
          }

          String shellArg = shell;               //0
          String userArg = "";                   //1
          String sshOptions = "";                //2
          String hostArg = host.getHostname();   //3
          if (user.length() > 0) {
              if (isWindowsMode) {
                 user = hostname.toUpperCase().split("\\.")[0] + "+" + user;
              }
              userArg = "-l "+user;
          }
          sshOptions = getShellOptions(shell);
          
          //String extendedTimeout = String.valueOf(timeout/1000 + 10); //We add additinal 10 secs to the timeout after which the installScript kills itself, if Java failed to do so
          this.command  = MessageFormat.format(installCommand, shellArg, userArg, sshOptions, hostArg, installScript, scriptArgs).trim();
      }

      public void execute() {
          Debug.trace("Initializing RemoteComponentScriptCommand: " + command + " timeout="+(MAX_WAIT_TIME/1000)+"sec");
          super.execute(command);
      }
}
