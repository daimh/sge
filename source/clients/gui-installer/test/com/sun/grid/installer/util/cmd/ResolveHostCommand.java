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

public class ResolveHostCommand extends CmdExec {
    private int exitVal = EXIT_VAL_SUCCESS;

    private String host = "";
    private String hostName = "";
    private String hostAddress = "";

    public ResolveHostCommand() {
        super(Util.DEF_RESOLVE_TIMEOUT);
    }
    
    private ResolveHostCommand(long timeout) {
        super(timeout);
    }

    @Override
    public void execute(String host) {
        this.host = host;
        
        try {
            Thread.sleep(TestBedManager.getInstance().getResolveSleepLength());
        } catch (InterruptedException ex) {
        }
    }

    @Override
    public int getExitValue() {
        return TestBedManager.getInstance().getResolveExitValue(host);
    }

    public String getHostName() {
        if (Util.isIpPattern(host)) {
            return TestBedManager.getInstance().getName(host);
        } else {
            return host;
        }
    }

    public String getHostAddress() {
        if (Util.isIpPattern(host)) {
            return host;
        } else {
            return TestBedManager.getInstance().getIPAddress(host);
        }
    }
}
