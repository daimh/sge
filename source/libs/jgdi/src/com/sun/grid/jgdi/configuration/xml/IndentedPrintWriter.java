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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.Writer;

/**
 * This class implements a PrintWriter with
 * indentication
 */
public class IndentedPrintWriter extends PrintWriter {
    
    private StringBuilder indent = new StringBuilder();
    
    /** this flag indicates that a indent should
     *  be added.
     *  @see #print(String)
     */
    private boolean addIndent = false;
    
    /** size of an indent step: */
    private int indentSize = 3;
    
    /**
     *  Create a new IndentedPrintWriter on an OutputStream
     *  @param  out the output stream
     */
    public IndentedPrintWriter(OutputStream out) {
        super(out);
    }
    
    /** Creates a new IndentedPrintWriter on a Writer
     *  @param writer the writer
     */
    public IndentedPrintWriter(Writer writer) {
        super(writer);
    }
    
    /** Creates a new IndentedPrintWriter on a File
     *  @param file the file
     */
    public IndentedPrintWriter(File file) throws IOException {
        super(new FileWriter(file));
    }
    
    /**
     * add an indent.
     */
    public void indent() {
        for(int i = 0; i < indentSize; i++) {
            indent.append(' ');
        }
    }
    
    /**
     * remove an indent.
     */
    public void deindent() {
        int newLen = Math.max(0,indent.length() - indentSize);
        indent.setLength(newLen);
    }
    
    
    public void println(String x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void print(String s) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(s);
    }
    
    public void println(Object x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void print(Object obj) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(obj);
    }
    
    public void println(int x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void print(int i) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(i);
    }
    
    public void print(boolean b) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(b);
    }
    
    public void println(boolean x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void println(long x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void print(long l) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(l);
    }
    
    public void println(double x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void println(char[] x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void print(char[] s) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(s);
    }
    
    public void print(double d) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(d);
    }
    
    public void println(char x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void print(char c) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(c);
    }
    
    
    public void println(float x) {
        print(x);
        super.println();
        addIndent = true;
    }
    
    public void print(float f) {
        if(addIndent) {
            super.print(indent);
            addIndent = false;
        }
        super.print(f);
    }
    
    
    public void println() {
        super.println();
        addIndent = true;
    }
    
    public int getIndentSize() {
        return indentSize;
    }
    
    public void setIndentSize(int indentSize) {
        this.indentSize = indentSize;
    }
    
}
