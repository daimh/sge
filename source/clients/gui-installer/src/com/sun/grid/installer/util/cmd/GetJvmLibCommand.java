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
import java.text.MessageFormat;
import com.sun.grid.installer.util.Util;

public class GetJvmLibCommand extends CmdExec {
      //E.g.: rsh/ssh [-l <connect_user>] [-o ....] HOSTNAME CMD
      private static String getJvmLibCommand = "{0} {1} {2} {3} {4}export SGE_ROOT={5} ; cd {5} ; . {5}/util/install_modules/inst_qmaster.sh ; HaveSuitableJavaBin 1.5.0 jvm print'\"";
      //E.g.: CMD
      private static String localGetJvmLibCommand = "export SGE_ROOT=\"{0}\" ; cd \"{0}\" ; . \"{0}\"/util/install_modules/inst_qmaster.sh ; HaveSuitableJavaBin 1.5.0 jvm print";
      private String command;

      public GetJvmLibCommand(String host, String user, String shell, boolean isWindowsMode, String sge_root) {
          this(Util.DEF_RESOLVE_TIMEOUT, host, user, shell, isWindowsMode, sge_root);
      }

      public GetJvmLibCommand(long timeout, String host, String user, String shell, boolean isWindowsMode, String sge_root) {
          super(timeout);
          boolean onLocalHost = host.equalsIgnoreCase(Host.localHostName);

          if (onLocalHost) {
             this.command = MessageFormat.format(localGetJvmLibCommand, sge_root).trim();
             return;
          }

          String shellArg = shell;               //0
          String userArg = "";                   //1
          String sshOptions = "";                //2
          String hostToReplace = host;           //3
          String arg4 = "\"sh -c '";             //4
          if (user.length() > 0) {
              if (isWindowsMode) {
                  user = host.toUpperCase().split("\\.")[0] + "+" + user;
              }
              userArg = "-l "+user;
          }
          sshOptions = getShellOptions(shell);
          
          this.command  = MessageFormat.format(getJvmLibCommand, shellArg, userArg, sshOptions, hostToReplace, arg4, "\\\""+sge_root+"\\\"").trim();
      }

      public void execute() {
          Debug.trace("Initializing GetJvmLibCommand: " + command + " timeout="+(MAX_WAIT_TIME/1000)+"sec");
          super.execute(command);
      }
}
