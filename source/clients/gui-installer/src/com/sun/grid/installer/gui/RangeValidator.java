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

import java.util.Map;

import com.izforge.izpack.panels.ProcessingClient;
import com.izforge.izpack.panels.Validator;
import com.izforge.izpack.util.Debug;

/**
 * Validator. Checks the following gramar:
 *
 * If 'values' property is given:
 *  <input>= +[<values>]
 * else:
 *  <input>=<number><separator><number>
 *  If 'min' and/or 'max' properties are given:
 *  <min>=<number>
 *  <max>=<number>
 *  <min> < <input> < <max>
 *
 *
 *
 */
public class RangeValidator implements Validator {
    private static String DEF_SEPARATOR = "-";
    private static String PARAM_SEPARATOR = "separator";
    private static String PARAM_MIN = "min";
    private static String PARAM_MAX = "max";
    private static String PARAM_VALUES = "values";

    public boolean validate(ProcessingClient client) {
        String text = client.getText().trim().toLowerCase();
        String separator = DEF_SEPARATOR;
        Integer min = null;
        Integer max = null;
        String values = null;

        if (text.equals("")) {
            return true;
        }

        Debug.trace("RangeValidator - Input= '" + min + "'");

        // read the validator parameters
        if (client.hasParams()) {
        	Map<String, String> params = client.getValidatorParams();
        	
        	if (params.containsKey(PARAM_SEPARATOR)) {
        		separator = params.get(PARAM_SEPARATOR);
                Debug.trace("RangeValidator - " + PARAM_SEPARATOR + "= '" + separator + "'");
        	}
            if (params.containsKey(PARAM_MIN)) {
        		try {
                    min = new Integer(params.get(PARAM_MIN));
                    Debug.trace("RangeValidator - " + PARAM_MIN + "= '" + min + "'");
                } catch (NumberFormatException ex) {
                    Debug.error(ex);
                }
        	}
            if (params.containsKey(PARAM_MAX)) {
                try {
                    max = new Integer(params.get(PARAM_MAX));
                    Debug.trace("RangeValidator - " + PARAM_MAX + "= '" + max + "'");
                } catch (NumberFormatException ex) {
                    Debug.error(ex);
                }
        	}
            if (params.containsKey(PARAM_VALUES)) {
                values = params.get(PARAM_VALUES);
                Debug.trace("RangeValidator - " + PARAM_VALUES + "= '" + values + "'");
        	}
        }

        /*
         * Depending on the given parameters validate the field
         */
        if (values != null) {   // Check #1: text == values(i)
            String[] valuesArray = values.split(",");
            boolean result = false;
            for (int i = 0; i < valuesArray.length; i++) {
                if (valuesArray[i].trim().toLowerCase().equals(text)) {
                    result = true;
                }
            }

            return result;
        } else {                // Check #2: min < lower < upper < max
            String[] range = null;
            if (text.indexOf(separator) == -1) {
                range = new String[]{text, text};
            } else {
                range = text.split(separator);
            }

            try {
                int lower = Integer.valueOf(range[0].trim()).intValue();
                int upper = Integer.valueOf(range[1].trim()).intValue();

                if (lower > upper) {
                    return false;
                }

                if (min != null && min.intValue() > lower) {
                    return false;
                }

                if (max != null && max.intValue() < upper) {
                    return false;
                }
            } catch (NumberFormatException e) {
                return false;
            }
        }

        return true;
    }
}
