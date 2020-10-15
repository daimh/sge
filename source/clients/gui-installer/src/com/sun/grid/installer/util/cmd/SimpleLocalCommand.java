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
import com.sun.grid.installer.util.Util;

public class SimpleLocalCommand extends CmdExec {
      private String command;

      public SimpleLocalCommand(String command) {
          this(Util.DEF_RESOLVE_TIMEOUT, command);
      }

      public SimpleLocalCommand(String... commands) {
          this(Util.DEF_RESOLVE_TIMEOUT, getSingleCommand(commands));
      }

      public SimpleLocalCommand(long timeout, String... commands) {
          this(timeout, getSingleCommand(commands));
      }

      public SimpleLocalCommand(long timeout, String command) {
          super(timeout);
          this.command  = command;
      }

      public void execute() {
          Debug.trace("Initializing SimpleLocalCommand: " + command + " timeout="+(MAX_WAIT_TIME/1000)+"sec");
          super.execute(command);
      }

      private static String getSingleCommand(String... cmds) {
        String singleCmd="";
        for (String cmd : cmds) {
           singleCmd += cmd + " ";
        }
        if (singleCmd.length() > 1) {
            return singleCmd.substring(0, singleCmd.length() - 1);
        }
        return null;
    }
}
