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
package com.sun.grid.installer.task;

import com.sun.grid.installer.gui.Host;
import java.util.List;

/**
 *
 */
public interface TaskHandler {

    /**
     * Sets the sate of the given host
     * @param host The host
     * @param state The state to set
     */
    public void setHostState(Host host, Host.State state);

    /**
     * Sets the log of the given host
     * @param host The host
     * @param log The log to set
     */
    public void setHostLog(Host host, String log);

    /**
     * Removes the given host
     * @param host The host to remove
     */
    public void removeHost(Host host);
    
    /**
     * Adds the given hosts
     * @param hosts Hosts to add to the data
     */
    public void addHosts(List<Host> hosts);

    /**
     * Returns with localized text for the given key
     * @param key The key which identifies the localized text
     * @return The localized text if there is any. Empty string otherwise.
     */
    public String getLabel(String key);

    /**
     * Returns with localized tooltip for the given key
     * @param key The key which identifies the localized tooltip
     * @return The localized tooltip if there is any. Empty string otherwise.
     */
    public String getTooltip(String key);
}
