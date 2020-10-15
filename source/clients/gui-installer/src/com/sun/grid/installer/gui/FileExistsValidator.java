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
import com.sun.grid.installer.util.FileHandler;
import java.io.File;
import java.util.Map;

/**
 * Validator. Check the entered file or directory if it exists. If the file parameter
 * has been given it will be appended to the entered direcory and then will be checked.
 */
public class FileExistsValidator implements Validator {
    private static String PARAM_FILE = "file";
    private static String PARAM_DIR = "dir";
    private static String PARAM_EXISTS = "exists";

    public boolean validate(ProcessingClient client) {
        String file = client.getText();
        boolean exists = true;

        if (file.equals("")) {
            return true;
        }

        File f = new File(file);
        
        if (client.hasParams()) {
        	Map<String, String> params = client.getValidatorParams();
            VariableSubstitutor vs = new VariableSubstitutor(GUIInstaller.getInstallData().getVariables());
            String dirPrefix = "";
            String fileSuffix = "";

            if (params.containsKey(PARAM_DIR)) {
                dirPrefix = params.get(PARAM_DIR);
                dirPrefix = vs.substituteMultiple(dirPrefix, null);

                if (!dirPrefix.endsWith(FileHandler.SEPARATOR)) {
                    dirPrefix = dirPrefix + FileHandler.SEPARATOR;
                }
            }

        	if (params.containsKey(PARAM_FILE)) {
                fileSuffix = params.get(PARAM_FILE);
                fileSuffix = vs.substituteMultiple(fileSuffix, null);

               if (!fileSuffix.startsWith(FileHandler.SEPARATOR)) {
                    fileSuffix = FileHandler.SEPARATOR + fileSuffix;
               }
            }

            if (params.containsKey(PARAM_EXISTS)) {
                exists = Boolean.parseBoolean(params.get(PARAM_EXISTS));

                Debug.trace("FileExistsValidator - exists: '" + exists + "'");
            }

            file = dirPrefix + file + fileSuffix;

            Debug.trace("FileExistsValidator - validate file: '" + file + "'");

            f = new File(file);
        }

        if (exists) {
            return f.exists();
        } else {
            return !f.exists();
        }
    }
}
