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
 *   Copyright: 2001 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.cull;

import java.io.*;
import java.util.*;
import java.util.logging.Logger;


/**
 *
 */
public abstract class AbstractCullConverter implements CullConverter {

    public static final Logger logger = Logger.getLogger("cullconv");

    private boolean iterateObjects;

    /** Creates a new instance of AbstractCullConverter */
    protected AbstractCullConverter() {
    }

    protected String[] readArgFile(String filename) throws IOException {

        FileReader fr = new FileReader(filename);
        BufferedReader br = new BufferedReader(fr);

        ArrayList list = new ArrayList();
        String line = null;

        while ((line = br.readLine()) != null) {
            list.add(line);
        }

        String[] ret = new String[list.size()];

        list.toArray(ret);
        return ret;
    }

    public boolean iterateObjects() {
        return iterateObjects;
    }

    public void setIterateObjects(boolean iterateObjects) {
        this.iterateObjects = iterateObjects;
    }
}
