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

import com.izforge.izpack.installer.InstallData;
import com.izforge.izpack.installer.InstallerFrame;
import com.izforge.izpack.util.Debug;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.ExtendedFile;

import com.sun.grid.installer.util.Util;

public class PreActionPanel extends ActionPanel {

    public PreActionPanel(InstallerFrame parent, InstallData idata) {
        super(parent, idata);
    }

    @Override
    public void doAction() {
        initializeVariables();
 
    }

    protected void initializeVariables() {
        VariableSubstitutor vs = new VariableSubstitutor(idata.getVariables());

        /**
         * Read arguments
         */
        // set thread spool sizes
        int size = 0;
        if (idata.getVariable(ARG_RESOLVE_THREAD_POOL_SIZE) != null) {
            try {
                size = Integer.valueOf(idata.getVariable(ARG_RESOLVE_THREAD_POOL_SIZE));
                if (size < 1) {
                    throw new NumberFormatException();
                }
                Util.RESOLVE_THREAD_POOL_SIZE = size;
            } catch (NumberFormatException e) {
                Debug.error("Invalid '" + ARG_RESOLVE_THREAD_POOL_SIZE + "' value: " + idata.getVariable(ARG_RESOLVE_THREAD_POOL_SIZE));
            } finally {
                Debug.trace(ARG_RESOLVE_THREAD_POOL_SIZE + " is now set to " + Util.RESOLVE_THREAD_POOL_SIZE);
            }
        }
        if (idata.getVariable(ARG_INSTALL_THREAD_POOL_SIZE) != null) {
            try {
                size = Integer.valueOf(idata.getVariable(ARG_INSTALL_THREAD_POOL_SIZE));
                if (size < 1) {
                    throw new NumberFormatException();
                }
                Util.INSTALL_THREAD_POOL_SIZE = size;
            } catch (NumberFormatException e) {
                Debug.error("Invalid '" + ARG_INSTALL_THREAD_POOL_SIZE + "' value: " + idata.getVariable(ARG_INSTALL_THREAD_POOL_SIZE));
            } finally {
                Debug.trace(ARG_INSTALL_THREAD_POOL_SIZE + " is now set to " + Util.INSTALL_THREAD_POOL_SIZE);
            }
        }
        //Set the timeout values
        if (idata.getVariable(ARG_RESOLVE_TIMEOUT) != null) {
            try {
                size = Integer.valueOf(idata.getVariable(ARG_RESOLVE_TIMEOUT));
                if (size < 1) {
                    throw new NumberFormatException();
                }
                Util.DEF_RESOLVE_TIMEOUT = size * 1000; //we need ms not seconds
            } catch (NumberFormatException e) {
                Debug.error("Invalid '" + ARG_RESOLVE_TIMEOUT + "' value: " + idata.getVariable(ARG_RESOLVE_THREAD_POOL_SIZE));
            } finally {
                Debug.trace(ARG_RESOLVE_TIMEOUT + " is now set to " + Util.DEF_RESOLVE_TIMEOUT);
            }
        }
        if (idata.getVariable(ARG_INSTALL_TIMEOUT) != null) {
            try {
                size = Integer.valueOf(idata.getVariable(ARG_INSTALL_TIMEOUT));
                if (size < 1) {
                    throw new NumberFormatException();
                }
                Util.DEF_INSTALL_TIMEOUT = size * 1000; //we need ms not seconds
            } catch (NumberFormatException e) {
                Debug.error("Invalid '" + ARG_INSTALL_TIMEOUT + "' value: " + idata.getVariable(ARG_INSTALL_THREAD_POOL_SIZE));
            } finally {
                Debug.trace(ARG_INSTALL_TIMEOUT + " is now set to " + Util.DEF_INSTALL_TIMEOUT);
            }
        }
        // Set connect_mode
        String mode = idata.getVariable(ARG_CONNECT_MODE);
        Util.IS_MODE_WINDOWS = (mode != null && mode.equalsIgnoreCase(CONST_MODE_WINDOWS));
        if (Util.IS_MODE_WINDOWS) {
           Debug.trace("Using mode '" + CONST_MODE_WINDOWS + "'.");
        }
        // Set connect_user
        Util.DEF_CONNECT_USER = vs.substituteMultiple(idata.getVariable(ARG_CONNECT_USER), null);
        Debug.trace("Using connect user '" + Util.DEF_CONNECT_USER + "'.");

        /**
         * Other initializations
         */
        String sgeRootPath = vs.substituteMultiple(idata.getVariable(VAR_SGE_ROOT), null);
        String userName = vs.substituteMultiple(idata.getVariable(VAR_USER_NAME), null);

        ExtendedFile sgeRootDir = new ExtendedFile(sgeRootPath);

        Debug.trace(sgeRootDir.getPermissions() + " " + sgeRootDir.getOwner() + " " + sgeRootDir.getGroup() + " " + sgeRootPath);

        // cfg.admin.user
        idata.setVariable(VAR_ADMIN_USER, sgeRootDir.getOwner());
        Debug.trace("cfg.admin.user='" + idata.getVariable(VAR_ADMIN_USER) + "'");

        // set cfg.add.to.rc
        if (vs.substituteMultiple(idata.getVariable(VAR_USER_NAME),null).equals(idata.getVariable(VAR_ROOT_USER)) ||
                !vs.substituteMultiple(idata.getVariable(VAR_CONNECT_USER), null).equals(vs.substituteMultiple(idata.getVariable(VAR_USER_NAME), null))) {
            idata.setVariable(VAR_ADD_TO_RC, "true");
        } else {
            idata.setVariable(VAR_ADD_TO_RC, "false");
        }

        // add.qmaster.host
        idata.setVariable(VAR_QMASTER_HOST, Host.localHostName);
        Debug.trace("add.qmaster.host='" + idata.getVariable(VAR_QMASTER_HOST) + "'");

        // cfg.db.spooling.server
        idata.setVariable(VAR_DB_SPOOLING_SERVER, Host.localHostName);
        Debug.trace("cfg.db.spooling.server='" + idata.getVariable(VAR_DB_SPOOLING_SERVER) + "'");

        // cfg.sge.jvm.lib.path  must be detected only when JMX was enabled later via remote call to DetectJvmLibrary.jar
    }
}
