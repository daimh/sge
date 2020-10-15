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

/**
 *
 */
public class Printer {
   
      private final static String INDENT = "  ";
      private PrintWriter pw;
      private StringBuffer indent = new StringBuffer();
      private boolean needIndent;
      
      public Printer( File file ) throws IOException {
         this(file, false);
      }
      
      public Printer( File file, boolean append ) throws IOException {
         
         File parent = file.getParentFile();
         if( !parent.exists() ) {
            parent.mkdirs();
         }
          FileWriter fw = new FileWriter( file, append );
          pw = new PrintWriter(fw);
      }
      
      public Printer() {
         pw = new PrintWriter( System.out );
      }
      
      public void printFile(File file) throws IOException {
         
         FileReader fr = new FileReader(file);
         BufferedReader br = new BufferedReader(fr);
         
         String line = null;
         while( (line=br.readLine()) != null) {
            println(line);
         }
         flush();
      }
      
      public void print( Object obj ) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.print( obj );
      }
      
      public void print( char c ) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.print( c );
      }
      
      public void print(int i) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.print( i );
      }
      
      public void print(boolean b) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.print( b );
      }
      
      public void print(long l) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.print( l );
      }
      
      public void println( Object obj ) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.println(obj);
         needIndent = true;
      }

      public void println( char c ) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.println(c);
         needIndent = true;
      }
      
      public void println(int i) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.println( i );
      }
      
      public void println() {
         pw.println();
         needIndent = true;
      }
      
      public void println(boolean b) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.println( b );
      }

      public void println(long l) {
         if( needIndent ) {
            pw.print( indent );
            needIndent = false;
         }
         pw.println( l );
      }
      
      public void indent() {
         indent.append( INDENT );
      }
      public void deindent() {
         indent.setLength( indent.length() - INDENT.length() );
      }
      
      public void flush() {
         pw.flush();
      }
      
      public void close() {
         pw.close();
      }
   
}
