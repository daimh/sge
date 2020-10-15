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
package com.sun.grid.jgdi.configuration.xml;

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.configuration.GEObject;
import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.List;

/**
 *
 */
public class XMLDump {
    
    private static void usage(String message, int exitCode) {
        if(message != null) {
            System.err.println(message);
        }
        System.err.println("XMLDump <url> <object> [all|<name>]");
        System.exit(exitCode);
    }
    
    public static void main(String [] args) {
        
        try {
            if(args.length != 3) {
                usage("Invalid number of arguments",1);
            }
            String url = args[0];
            String objectName = args[1];
            String name = args[2];
            
            JGDI jgdi = JGDIFactory.newInstance(url);
            
            if(name.equals("all")) {
                
                Method method = JGDI.class.getMethod("get" + objectName + "List", (java.lang.Class[])null);
                
                List list = (List)method.invoke(jgdi, (java.lang.Object[])null);
                
                Iterator iter = list.iterator();
                while(iter.hasNext()) {
                    GEObject obj = (GEObject)iter.next();
                    XMLUtil.write(obj, System.out);
                    System.out.flush();
                }
            } else {
                
                Method method = JGDI.class.getMethod("get" + objectName , new Class[] { String.class } );
                
                Object obj = method.invoke(jgdi, new Object[] { name } );
                XMLUtil.write((GEObject)obj,System.out);
                System.out.flush();
            }
            
        } catch (Exception e) {
            e.printStackTrace();
        }
        
    }
}
