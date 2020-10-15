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
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *   Copyright: 2009 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

package com.sun.grid.loadsensor.examples;

import com.sun.grid.loadsensor.*;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 *
 * @author dant
 */
public class SimpleLoadSensor implements LoadSensor {
    private int count = 0;
    
    public int getMeasurementInterval() {
        return MEASURE_ON_DEMAND;
    }

    public void doMeasurement() {
        count++;
    }

    public Map<String, Map<String, String>> getLoadValues() {
        Map<String,Map<String,String>> values = new HashMap<String, Map<String, String>>();

        values.put("ultra20", Collections.singletonMap("angles", Integer.toString(count)));

        return values;
    }
}
