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

import com.izforge.izpack.panels.ProcessingClient;
import com.izforge.izpack.panels.Validator;
import com.izforge.izpack.util.Debug;
import java.util.Map;

/**
 * Validator. Fills up the field if it's empty with the specified default value.
 */
public class FillIfEmptyValidator implements Validator {
    private static String PARAM_DEFAULT = "default";

    public boolean validate(ProcessingClient client) {
        String text = client.getText().trim();
        String defaultValue = "";

        // if the field is empty fill it up wit hthe default value
        if (text.equals("")) {
            if (client.hasParams()) {
                Map<String, String> params = client.getValidatorParams();

                if (params.containsKey(PARAM_DEFAULT)) {
                    defaultValue = params.get(PARAM_DEFAULT);
                }
            }

            Debug.trace("Set field value with default: '" + defaultValue + "'");
            client.setText(defaultValue);
        }

        return true;
    }
}
