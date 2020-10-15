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

public class FsTypeCommand extends CmdExec {
      //E.g.: rsh/ssh [-l <connect_user>] [-o ....] HOSTNAME FSTYPE DIR
      private static String fsTypeCommand = "{0} {1} {2} {3} \"\\\"{4}\\\" {5}\"";
      //E.g.: FSTYPE DIR
      private static String localFsTypeCommand = "\"{0}\" {1}";
      private String command;

      public FsTypeCommand(String host, String user, String shell, boolean isWindowsMode, String fstypePath, String dirPath) {
          this(Util.DEF_RESOLVE_TIMEOUT, host, user, shell, isWindowsMode, fstypePath, dirPath);
      }

      public FsTypeCommand(long timeout, String host, String user, String shell, boolean isWindowsMode, String fstypePath, String dirPath) {
          super(timeout);
          boolean onLocalHost = host.equalsIgnoreCase(Host.localHostName);

          if (onLocalHost) {
             this.command = MessageFormat.format(localFsTypeCommand, fstypePath, dirPath).trim();
             return;
          }

          String shellArg = "";             //0
          String userArg = "";              //1
          String sshOptions = "";           //2
          String hostToReplace = "";        //3
          shellArg = shell;
          if (user.length() > 0) {
              if (isWindowsMode) {
                  user = host.toUpperCase().split("\\.")[0] + "+" + user;
              }
              userArg = "-l "+user;
          }
          sshOptions = getShellOptions(shell);
          hostToReplace = host;          
          this.command  = MessageFormat.format(fsTypeCommand, shellArg, userArg, sshOptions, hostToReplace, fstypePath, dirPath).trim();
      }

      public void execute() {
          Debug.trace("Initializing GetFsTypeCommand: " + command + " timeout="+(MAX_WAIT_TIME/1000)+"sec");
          super.execute(command);
      }
}
