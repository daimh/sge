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

import com.sun.grid.installer.gui.Host;
import com.sun.grid.installer.util.Util;
import java.util.Properties;

public class RemoteComponentScriptCommand extends CmdExec {
      private Host host;
      private String user;
      private String shell;
      private boolean isWindowsMode;
      private String installScript;

      public RemoteComponentScriptCommand(Host host, String user, String shell, boolean isWindowsMode, String installScript) {
          this(Util.DEF_INSTALL_TIMEOUT, host, user, shell, isWindowsMode, installScript);
      }

      public RemoteComponentScriptCommand(long timeout, Host host, String user, String shell, boolean isWindowsMode, String installScript) {
          super(timeout);

          this.host = host;
          this.user = user;
          this.shell = shell;
          this.isWindowsMode = isWindowsMode;
          this.installScript = installScript;
      }

      public void execute() {
          try {
              if (installScript.indexOf("install_component") > -1) {
                  Thread.sleep(TestBedManager.getInstance().getInstallationSleepLength());
              } else if (installScript.indexOf("check_host") > -1) {
                  Thread.sleep(TestBedManager.getInstance().getValidationSleepLength());
              } else {
                  throw new IllegalArgumentException("Unknown script: " + installScript);
              }

          } catch (InterruptedException ex) {
          }
      }

    @Override
    public int getExitValue() {
        if (installScript.indexOf("install_component") > -1) {
            return TestBedManager.getInstance().getInstallationExitValue(host.getHostname());
        } else if (installScript.indexOf("check_host") > -1) {
            return TestBedManager.getInstance().getValidationExitValue(host.getHostname());
        } else {
            throw new IllegalArgumentException("Unknown script: " + installScript);
        }
    }

    @Override
    public String generateLog(Properties msgs) {
        return generateLog(0, msgs);
    }

    @Override
    public String generateLog(int exitValue, Properties msgs) {
        return TestBedManager.generateLog(host.getHostname());
    }


}
