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
package com.sun.grid.util;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


public class DetectJvmLibrary {
    public static void main(String[] args) {
        List libPaths = new ArrayList();
        libPaths.addAll(Arrays.asList(System.getProperty("java.library.path").split(":")));
        libPaths.add(System.getProperty("sun.boot.library.path"));
        //MacOS last resort
        libPaths.add("/System/Library/Frameworks/JavaVM.framework/Libraries");
        libPaths.add("/Library/Java/Home");

        String libjvm = "/" + System.mapLibraryName("jvm");
        if (libjvm.endsWith(".jnilib")) {
            libjvm = libjvm.substring(0, libjvm.lastIndexOf(".jnilib")) + ".dylib";
        }

        String libPath = "";
        for (int i=0; i<libPaths.size(); i++) {
            libPath = (String) libPaths.get(i);
            if (new File(libPath + libjvm).exists()) {
                System.out.println(libPath+libjvm);
                System.exit(0);
            }
        }
        System.exit(1); //Not found
    }
}
