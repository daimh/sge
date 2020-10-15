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

public class GetArchCommand extends CmdExec {
      //         0             1              2          3          4    5                      6
      //E.g.: rsh/ssh [-l <connect_user>] [-o ....] HOSTNAME [\"sh -c ']if [ ! -s SGE_ROOT....['\"]
      private static String getArchCommand = "{0} {1} {2} {3} {4}if [ ! -s {5}/util/arch ]; then echo File\\ {5}/util/arch\\ does\\ not\\ exist\\ or\\ is\\ empty. ; echo ___EXIT_CODE_"+EXIT_VAL_CMDEXEC_MISSING_FILE+"___ ; exit " + EXIT_VAL_CMDEXEC_MISSING_FILE + "; else {5}/util/arch ; fi{6}";
      private static String localGetArchCommand = "if [ ! -s \"{0}\"/util/arch ]; then echo File\\ \"{0}\"/util/arch\\ does\\ not\\ exist\\ or\\ is\\ empty. ; echo ___EXIT_CODE_"+EXIT_VAL_CMDEXEC_MISSING_FILE+"___ ; exit " + EXIT_VAL_CMDEXEC_MISSING_FILE + "; else \"{0}\"/util/arch ; fi";
      private String command;

      public GetArchCommand(String host, String user, String shell, boolean isWindowsMode, String sge_root) {
          this(Util.DEF_RESOLVE_TIMEOUT, host, user, shell, isWindowsMode, sge_root);
      }

      public GetArchCommand(long timeout, String host, String user, String shell, boolean isWindowsMode, String sge_root) {
          super(timeout);
          boolean onLocalHost = host.equalsIgnoreCase(Host.localHostName);

          if (onLocalHost) {
             this.command = MessageFormat.format(localGetArchCommand, sge_root).trim();
             return;
          }

          String shellArg = shell;               //0
          String userArg = "";                   //1
          String sshOptions = "";                //2
          String hostToReplace = host;           //3
          String arg4 = "\"sh -c '";             //4
          String arg6 = "'\"";                   //6
          if (user.length() > 0) {
              if (isWindowsMode) {
                  user = host.toUpperCase().split("\\.")[0] + "+" + user;
              }
              userArg = "-l "+user;
          }
          sshOptions = getShellOptions(shell);

          //String extendedTimeout = String.valueOf(timeout/1000 + 10); //We add additinal 10 secs to the timeout after which the installScript kills itself, if Java failed to do so in within timeout
          this.command  = MessageFormat.format(getArchCommand, shellArg, userArg, sshOptions, hostToReplace, arg4, "\\\""+sge_root+"\\\"", arg6).trim();
      }

      public void execute() {
          Debug.trace("Initializing GetArchCommand: " + command + " timeout="+(MAX_WAIT_TIME/1000)+"sec");
          super.execute(command);
      }
}
