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

import com.sun.grid.installer.util.Util;
import java.util.Vector;

public class GetArchCommand extends CmdExec {
    private String host;
    private String user;
    private String shell;
    private boolean isWindowsMode;
    private String sge_root;

    public GetArchCommand(String host, String user, String shell, boolean isWindowsMode, String sge_root) {
        this(Util.DEF_RESOLVE_TIMEOUT, host, user, shell, isWindowsMode, sge_root);
    }

    public GetArchCommand(long timeout, String host, String user, String shell, boolean isWindowsMode, String sge_root) {
        super(timeout);

        this.host = host;
        this.user = user;
        this.shell = shell;
        this.isWindowsMode = isWindowsMode;
        this.sge_root = sge_root;
    }

    public void execute() {
        try {
            Thread.sleep(TestBedManager.getInstance().getGetArchitectureSleepLength());
        } catch (InterruptedException ex) {
        }
    }

    @Override
    public int getExitValue() {
        return TestBedManager.getInstance().getGetArchitectureExitValue(host);
    }

    @Override
    public Vector<String> getOutput() {
        Vector<String> result = new Vector<String>();
        result.add(TestBedManager.getInstance().getArchitecture(host));

        return result;
    }
}
