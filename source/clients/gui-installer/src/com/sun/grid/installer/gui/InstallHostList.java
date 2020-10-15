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
package com.sun.grid.installer.gui;

public class InstallHostList extends HostList {

    public InstallHostList() {
        super();
    }

    /* Add all hosts from the hostList as 1 component per host (each host appears as many times as many components should be installed on it) */
    public boolean appendHostList(HostList hostList, boolean installQmaster, boolean installBdb, boolean installShadow, boolean installExecd) {
        boolean addOk = true;
        Host h;
        for (Host host : hostList) {
            
            if (host.isShadowHost() && installShadow) {
                h = new Host(host);
                h.setState(Host.State.READY_TO_INSTALL);
                clearComponentSelection(h);
                h.setShadowHost(true);
                addOk |= this.addUnchecked(h);
            }

            if (host.isExecutionHost() && installExecd) {
                h = new Host(host);
                h.setState(Host.State.READY_TO_INSTALL);
                clearComponentSelection(h);
                h.setExecutionHost(true);
                addOk |= this.addUnchecked(h);
            }

            if (host.isQmasterHost() && installQmaster) {
                h = new Host(host);
                h.setState(Host.State.READY_TO_INSTALL);
                clearComponentSelection(h);
                h.setQmasterHost(true);
                addOk |= this.addUnchecked(h);
            }

            if (host.isBdbHost() && installBdb) {
                h = new Host(host);
                h.setState(Host.State.READY_TO_INSTALL);
                clearComponentSelection(h);
                h.setBdbHost(true);
                addOk |= this.addUnchecked(h);
            }
            //Let's do the install at once!!

        }
        return addOk;
    }

    private void clearComponentSelection(Host host) {
        host.setQmasterHost(false);
        host.setExecutionHost(false);
        host.setBdbHost(false);
        host.setShadowHost(false);
    }
}
