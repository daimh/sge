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

/* Copies executable script to a remote host. If host is localhost command is skipped */
public class CopyExecutableCommand extends CmdExec {
      //E.g.: rsh/ssh [-l <connect_user>] [-o ....] HOSTNAME < "FILE_TO_COPY" "cat >"DEST_FILE" ; chmod 755 "DEST_FILE""
      private static String copyCommand = "{0} {1} {2} {3} < \"{4}\" \"cat >\\\"{5}\\\" ; chmod 755 \\\"{5}\\\"\"";
      private static String localCopyCommand = "cp \"{0}\" \"{1}\" ; chmod 755 \"{1}\"";
      private String command;

      public CopyExecutableCommand(String host, String user, String shell, boolean isWindowsMode, String inputPath, String outputPath) {
          this(Util.DEF_RESOLVE_TIMEOUT, host, user, shell, isWindowsMode, inputPath, outputPath);
      }

      public CopyExecutableCommand(long timeout, String host, String user, String shell, boolean isWindowsMode, String inputPath, String outputPath) {
          super(timeout);
          //TODO: Need better understading of local
          //If host has aliases they are treated as remote hosts which is not necessary
          boolean onLocalHost = host.equalsIgnoreCase(Host.localHostName);

          if (onLocalHost) {
              this.command = MessageFormat.format(localCopyCommand, inputPath, outputPath).trim();
              return;
          }

          String shellArg = "";
          String userArg = "";
          String sshOptions = "";
          shellArg = shell;
          if (user.length() > 0) {
              if (isWindowsMode) {
                  user = host.toUpperCase().split("\\.")[0] + "+" + user;
              }
              userArg = "-l "+user;
          }
          sshOptions = getShellOptions(shell);
          this.command  = MessageFormat.format(copyCommand, shellArg, userArg, sshOptions, host, inputPath, outputPath).trim();
      }

      public void execute() {
          Debug.trace("Initializing CopyExecutableCommand: " + command + " timeout="+(MAX_WAIT_TIME/1000)+"sec");
          super.execute(command);
      }
}
