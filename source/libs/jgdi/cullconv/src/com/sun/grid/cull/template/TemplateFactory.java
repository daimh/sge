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
package com.sun.grid.cull.template;

import com.sun.grid.cull.Printer;
import java.io.EOFException;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.StringTokenizer;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.tools.ant.BuildException;


/**
 *
 */
public class TemplateFactory {
   
   private File tmpDir;
   private File tmpSrcDir;
   private File tmpClassesDir;
   private String classpath;
   private Logger logger = Logger.getLogger("cullconv");
   private String javaSourceVersion = "1.4";
   private String javaTargetVersion = "1.4";
   
   
   public TemplateFactory(File tmpDir, String classpath, String javaSourceVersion, String javaTargetVersion) {
      
      this.tmpDir = tmpDir;
      this.javaSourceVersion = javaSourceVersion;
      this.javaTargetVersion = javaTargetVersion;
      tmpSrcDir = new File(tmpDir, "src");
      tmpClassesDir = new File(tmpDir, "classes");
      this.classpath = classpath;
      if( !tmpSrcDir.exists() ) {
         if( !tmpSrcDir.mkdir() ) {
            throw new IllegalArgumentException("Can not create src dir " + tmpSrcDir );
         }
      }
      if( !tmpClassesDir.exists() ) {
         if( !tmpClassesDir.mkdir() ) {
            throw new IllegalArgumentException("Can not create classes dir " + tmpSrcDir );
         }
      }
      
   }
   
   private String getClassName(File f) {

      StringBuffer buf = new StringBuffer(f.getName());
      for(int i = 0; i < buf.length(); i++ ) {
         switch(buf.charAt(i)) {
            case '.': 
            case '/':
                 buf.setCharAt(i, '_');
         }         
      }
      return buf.toString();
   }
   
   public Template createTemplate(File f ) throws IOException, BuildException {
      
      String  className = getClassName(f);
      

      File srcFile = new File( tmpSrcDir, className + ".java" );
      
      if( !srcFile.exists() || f.lastModified() > srcFile.lastModified() ) {

         logger.info("create template class " + className + " from file " + f);
         
         // Template class doest not exists or is older the the template file
         // Recompile it
         
         Printer p = new Printer(srcFile);

         writeHeader(p, null, className );
         writeTemplate(p, f);
         writeFooter(p, className );

         p.flush();
         p.close();

         String args [] = {         
            "-d", tmpClassesDir.getAbsolutePath(), 
            "-classpath", classpath,
            "-source", javaSourceVersion,
            "-target", javaTargetVersion,
            srcFile.getAbsolutePath()
         };

           try {
               Class c = Class.forName ("com.sun.tools.javac.Main");
               Object compiler = c.newInstance ();
               Method compile = c.getMethod ("compile",
                   new Class [] {(new String [] {}).getClass ()});

               int result = ((Integer) compile.invoke
                             (compiler, new Object[] {args}))
                   .intValue ();
               if( result != 0 ) {
                  throw new BuildException("Compile returned " + result );
               }
           } catch (Exception ex) {
               if (ex instanceof BuildException) {
                   throw (BuildException) ex;
               } else {
                   throw new BuildException("Error starting compiler",
                                            ex);
               }
           }
      }      
      
      try {
         logger.fine("load template class " + className  );
         
         ClassLoader cl = new URLClassLoader( new URL [] { tmpClassesDir.toURI().toURL() }, getClass().getClassLoader() );

         Class cls = cl.loadClass( className );

         Template ret = (Template)cls.newInstance();

         return ret;
      } catch( Exception e ) {
         throw new BuildException("Can not instantiate class " + className + ", classpath is " + classpath, e );
      }
   }
   
   
   private void writeLine(Printer p, StringBuffer line, boolean newLine ) {

      if(newLine) {
         p.print("p.println(\"");
      } else {
         p.print("p.print(\"");
      }
      for(int i = 0; i < line.length(); i++ ) {
         char c = line.charAt(i);
         switch(c) {
            case '"':  p.print("\\\""); break;
            case '\\': p.print("\\\\"); break;
            default: p.print(c);
         }
      }
      p.println("\");");
      line.setLength(0);
   }
   
   private char read(Reader rd) throws IOException {
      int c = rd.read();
      if( c < 0 ) {
         throw new EOFException("eof");
      }
      if( logger.isLoggable(Level.FINE)) {
         logger.fine("got char " + (char)c);
      }
      return (char)c;
   }
   
   private void writeTemplate(Printer p, File f) throws IOException {
      
      logger.fine("writeTemplate: " +  f );
      Reader br = new FileReader(f);
      File dir = f.getParentFile();
      char c = 0;
      StringBuffer line = new StringBuffer();
      
      try {
         while( true ) {
            c = read(br);
            if( c == '<' ) {
               c = read(br);
               if( c == '%') {
                  writeLine(p,line, false);                  
                  writeCode(p, br, dir);
               } else {
                  line.append("<");
                  line.append(c);
               }
            } else {
               if( c == '\n' || c == '\r' ) {
                  writeLine(p,line, true);
               } else {
                  line.append((char)c);
               }
            }         
         }      
      } catch( EOFException eof ) {
         logger.finer("EOF");
      }
      if( line.length() > 0 ) {
         writeLine(p,line, true);
      }
   }
   
   private void writeCode(Printer p, Reader rd, File dir) throws IOException {
      
      char c = read(rd);
            
      boolean first = true;
      boolean inline = false;
      if( c == '@' ) {
         writeStaticCode(p,rd, dir);
         return;
      } else if( c == '=' ) {
         inline = true;
         p.print("p.print(");
      } else {
         p.print(c);
      }
      
      while( true ) {
         c = read(rd);
         if( c == '%' ) {
            c = read(rd);
            if( c == '>' ) {
               if( inline ) {
                  p.print(");");
               }
               break;
            } else {
               p.print("%");
            }
         } else {
            if( first ) {
               if( c == '=' ) {
                  inline = true;
                  p.print("p.print(");
               } else {
                  p.print(c);
               }
               first = false;               
            } else {
               p.print(c);
            }
         }
      }
   }
   
   
   private void writeStaticCode(Printer p, Reader rd, File dir) throws IOException {
   
      StringBuffer buf = new StringBuffer();
      char c = 0;
      
      while(true) {
         c = read(rd);
         if( c == '%' ) {
            c = read(rd);
            if( c == '>' ) {
               break;
            } else {
               buf.append('%');
            }
         } else {
            buf.append(c);
         }
      }
      
      String str = buf.toString().trim();
      System.out.println("static code:" + str);
      StringTokenizer st = new StringTokenizer(str," ");
      if( st.hasMoreTokens() ) {
         String token = st.nextToken();
         
         if( "include".equals(token) ) {
            if( st.hasMoreTokens() ) {
               token = st.nextToken();
               
               if( token.startsWith("file=") ) {
                  
                  String filename = token.substring(6, token.length() - 1);
                  
                  File newFile = new File(dir, filename );

                  p.println();
                  p.print("// BEGIN ");
                  p.print( newFile.getCanonicalPath() );
                  p.println(" --------------------------");
                  writeTemplate(p, newFile);
                  p.println();
                  p.print("// END ");
                  p.print( newFile.getCanonicalPath() );
                  p.println(" --------------------------");
                  
                  
               } else {
                  throw new IOException("invalid include '" + str + "'" );
               }
               
               
            } else {               
               throw new IOException("invalid include '" + str + "'" );
            }
            
         } else {
            throw new IOException("invalid static code '" + str + "'" );
         }
      } else {
         throw new IOException("invalid static code '" + str + "'" );
      }
   }
   
   private void writeHeader(Printer p, String packageName, String className ) {
      
      if( packageName != null ) {
         p.print("package ");
         p.print( packageName );
         p.println(";");
         
      }

      p.print("public class ");
      p.print( className );
      p.println( " implements com.sun.grid.cull.template.Template {");
      p.indent();

      p.println("public void print(final com.sun.grid.cull.Printer p, final java.util.Map params) {");
      p.indent();
      
    }
      
    private void writeFooter(Printer p, String className ) {
      
      p.deindent();
      p.println("} // end of print");
      p.deindent();
      p.print("} // end of class ");
      p.println( className );
   }
   
   
   
}
