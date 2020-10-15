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

import com.izforge.izpack.installer.GUIInstaller;
import com.izforge.izpack.panels.ProcessingClient;
import com.izforge.izpack.panels.Validator;
import com.izforge.izpack.util.Debug;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.Config;
import com.sun.grid.installer.util.Util;
import java.util.Map;
import java.util.Properties;

public class FileSystemTypeValidator implements Validator {
    private static String PARAM_DEFAULT = "default";
    private static String PARAM_VALIDTYPES = "validfstypes";
    private static String PARAM_INVALIDTYPES = "invalidfstypes";

    public boolean validate(ProcessingClient client) {
        String dir = client.getText();
        String[] validTypes = null;
        String[] invalidTypes = null;
        boolean defaultRetValue = false;

        if (dir.equals("")) {
            return true;
        }

        try {
            if (client.hasParams()) {
                Map<String, String> params = client.getValidatorParams();

                if (params.containsKey(PARAM_VALIDTYPES)) {
                    validTypes = params.get(PARAM_VALIDTYPES).split(",");

                    Debug.trace("FileSystemTypeValidator - valid types: '" + params.get(PARAM_VALIDTYPES) + "'");
                }

                if (params.containsKey(PARAM_INVALIDTYPES)) {
                    invalidTypes = params.get(PARAM_INVALIDTYPES).split(",");

                    Debug.trace("FileSystemTypeValidator - invalid types: '" + params.get(PARAM_INVALIDTYPES) + "'");
                }

                if (params.containsKey(PARAM_DEFAULT)) {
                    if (params.get(PARAM_DEFAULT).equals("true")) {
                        defaultRetValue = true;
                    }

                    Debug.trace("FileSystemTypeValidator - default return value: '" + defaultRetValue + "'");
                }

                if (validTypes == null && invalidTypes == null) {
                    return true;
                }

                // Get fs type
                Properties variables = GUIInstaller.getInstallData().getVariables();
                VariableSubstitutor vs = new VariableSubstitutor(variables);
                String sge_root = vs.substituteMultiple(variables.getProperty(Config.VAR_SGE_ROOT, ""), null);
                dir = vs.substituteMultiple(dir, null);
                
                String fsType = Util.getDirFSType(variables.getProperty(Config.VAR_SHELL_NAME, ""), sge_root, dir);

                /**
                 * Validate
                 */

                //check invalid outputs
                if (invalidTypes != null) {
                    for (int i = 0; i < invalidTypes.length; i++) {
                        if (invalidTypes[i].trim().toLowerCase().equals(fsType.toLowerCase())) {
                            return false;
                        }
                    }
                }

                //check valid outputs
                if (validTypes != null) {
                    for (int i = 0; i < validTypes.length; i++) {
                        if (validTypes[i].trim().toLowerCase().equals(fsType.toLowerCase())) {
                            return true;
                        }
                    }
                }

                return defaultRetValue;
            }
        } catch (Exception e) {
            Debug.error(e);
        }

        return true;
    }
}
