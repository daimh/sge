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
package com.sun.grid.jgdi.util.shell;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.logging.Formatter;
import java.util.logging.Level;
import java.util.logging.LogRecord;

/**
 *
 */
public class ShellFormatter extends Formatter {
    
    private StringWriter sw = new StringWriter();
    private boolean printStacktrace = true;
    
    public String format(LogRecord record) {
        
        
        sw.getBuffer().setLength(0);
        PrintWriter pw = new PrintWriter(sw);
        
        if(record.getLevel().equals(Level.INFO)) {
            pw.println(record.getMessage());
        } else {
            pw.print(record.getLevel());
            pw.print(": ");
            
            String message = record.getMessage();
            if ("ENTRY".equals(message)) {
                pw.print("Entering ");
                pw.print(record.getSourceClassName());
                pw.print(".");
                pw.println(record.getSourceMethodName());
                
            } else {
                pw.println(message);
            }
        }
        
        if(record.getThrown() != null && printStacktrace) {
            record.getThrown().printStackTrace(pw);
        }
        pw.flush();
        return sw.getBuffer().toString();
    }
    
}
