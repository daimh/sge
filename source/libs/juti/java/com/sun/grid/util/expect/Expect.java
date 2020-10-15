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
package com.sun.grid.util.expect;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.Reader;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Simple helper class for handling output of a process. It works similar as
 * the tcl expect command.
 *
 * 
 *
 * <H3>Example</H3>
 *
 * <pre>
 *   
 *    char [] password = null;
 *    String username = null;
 *    long timeout = 60 * 1000;
 *    String [] cmd = new String[] { "su", "username", "-c" , "ls" };
 *
 *    Expect expect = new Expect(cmd);
 *
 *    expect.addHandler(new ExpectPasswordHandler("password: ", password));
 *
 *    expect.addHandler(new ExpectHandler() {
 *
 *        // print every line after the password question to System.out
 *        public void handle(Expect expect, ExpectBuffer buffer) {
 * 
 *            String line = buffer.consumeLine();
 *            if(line != null) {
 *               System.out.println(line);
 *            } 
 *
 *        }
 *    });
 *
 *    if(expect.exec(timeout) == 0 ) {
 *       System.out.println("ls as user " + username + " successfully executed");
 *    }
 *
 * </pre>
 *
 */
public class Expect {
    
    private final static Logger LOGGER = Logger.getLogger(Expect.class.getName());
    
    private PrintWriter stdin;
    private Process process;
    private Pump stdout;
    private Pump stderr;
    private ExecThread execThread;
    
    private int  exitCode;
    
    private StringBuffer buffer = new StringBuffer();
    private List handlers = new LinkedList();
    
    private List command = new LinkedList();
    private List env = new LinkedList();
            
    public Expect() {        
    }
    
    /**
     * Creates a new instance of Expect
     * @param cmd the command which will be executed
     * @see java.lang.Runtime#exec(String[])
     */
    public Expect(String [] cmd) {
        for(int i = 0; i < cmd.length; i++) {
            command.add(cmd[i]);
        }
    }
    
    public List command() {
        return command;
    }
    
    public List env() {
        return env;
    }
    
    /**
     * Add a new expect handler.
     * @param handler the expect handler
     */
    public void add(ExpectHandler handler) {
        handlers.add(handler);
    }
    
    /**
     * Execute the command and call the registered handlers.
     *
     * @param timeout timout for the command in milliseconds
     * @throws java.io.IOException if the execution of the command failed
     * @throws java.lang.InterruptedException if the current thread has been interrupted
     *             before the command has been finished.
     * @return the exit code of the command
     */
    public int exec(long timeout) throws IOException, InterruptedException {
        
        try {
            
            String [] cmd = new String[command.size()];
            command.toArray(cmd);
            String [] env = new String[this.env.size()];
            this.env.toArray(env);
                    
            if(LOGGER.isLoggable(Level.FINE)) {
                StringBuffer msg = new StringBuffer();
                for(int i = 0; i < cmd.length; i++) {
                    if(i>0) {
                        msg.append(' ');
                    }
                    msg.append(cmd[i]);
                }
                LOGGER.log(Level.FINE, "exec: {0}", msg);
            }
            process = Runtime.getRuntime().exec(cmd, env);
            
            stdout = new Pump(process.getInputStream(), "stdout");
            stderr = new Pump(process.getErrorStream(), "stderr");
            stdin = new PrintWriter(new OutputStreamWriter(process.getOutputStream()));
            execThread = new ExecThread();
            
            execThread.start();
            stdout.start();
            stderr.start();
            
            ExpectBuffer workingBuffer = new ExpectBuffer();
            
            while(true) {
                
                String received = null;
                synchronized(buffer) {
                    if(Thread.currentThread().isInterrupted()) {
                        break;
                    }
                    if(!execThread.isRunning()) {
                        break;
                    }
                    if(buffer.length() == 0) {
                        LOGGER.log(Level.FINER, "waiting for input");
                        long start = System.currentTimeMillis();
                        buffer.wait(timeout);
                        if(start + timeout <= System.currentTimeMillis()) {
                            throw new IOException("timeout");
                        }
                    }
                    received = buffer.toString();
                    buffer.setLength(0);
                }
                
                if(received == null || received.length() == 0) {
                    continue;
                }
                
                
                if(LOGGER.isLoggable(Level.FINE)) {
                    StringBuffer buf = new StringBuffer();
                    for(int i = 0; i < received.length(); i++) {
                        char c = received.charAt(i);
                        if(c == '\n') {
                            LOGGER.log(Level.FINE, "got ''{0}<NL>''", buf.toString());
                            buf.setLength(0);
                        } else if ( c == '\r' ) {
                            continue;
                        } else {
                            buf.append(received.charAt(i));
                        }
                    }
                    if(buf.length() > 0) {
                        LOGGER.log(Level.FINE,"got ''{0}''", buf.toString());
                    }
                }
                
                workingBuffer.append(received);
                
                
                int len = 0;
                do {
                    len = workingBuffer.length();
                    Iterator iter = handlers.iterator();
                    while(iter.hasNext()) {
                        ExpectHandler handler = (ExpectHandler)iter.next();
                        
                        handler.handle(Expect.this, workingBuffer);
                        
                    }
                } while( len > workingBuffer.length() );
            }
            return process.exitValue();
        } catch(IOException ex) {
            LOGGER.throwing("Expect", "exec", ex);
            throw ex;
        } finally {
            if (process != null) {
                process.getOutputStream().close();
                process.getErrorStream().close();
                process.getInputStream().close();
            }
        }
    }
    
    /**
     *  This thread waits for the end of the process
     */
    private class ExecThread extends Thread {
        
        private boolean running = true;
        
        public void run() {
            try {
                process.waitFor();
                LOGGER.log(Level.FINE,"command exited with status " + process.exitValue());
                sleep(100);
            } catch (InterruptedException ex) {
            } finally {
                running = false;
                stdout.interrupt();
                stderr.interrupt();
                synchronized(buffer) {
                    buffer.notifyAll();
                }
            }
        }
        
        public boolean isRunning() {
            return running;
        }
    }
    
    /**
     *  Instances of the class reads stdout or stdin of the process and
     *  add the content of to the buffer
     */
    private class Pump extends Thread {
        
        private InputStream in;
        private IOException error;
        private String tag;
        
        public Pump(InputStream in, String tag) {
            this.in = in;
            this.tag = tag;
        }
        
        public void run() {
            try {
                Reader rd = new InputStreamReader(in);
                char [] buf = new char[1024];
                int len = 0;
                try {
                    while((len = rd.read(buf)) > 0) {
                        if(LOGGER.isLoggable(Level.FINEST)) {
                            LOGGER.log(Level.FINEST, "{0}: {1}", new Object [] { tag, new String(buf, 0, len) } );
                        }
                        synchronized(buffer) {
                            buffer.append(buf, 0, len);
                            buffer.notifyAll();
                        }
                    }
                } catch(IOException ioe) {
                    error = ioe;
                }
            } finally {
                try {                   
                    LOGGER.log(Level.FINEST, "{0} finished", tag);                    
                    in.close();
                } catch (IOException ex) {
                    LOGGER.log(Level.WARNING, "Failed to close stream. Details: {0}", ex.getLocalizedMessage());
                }

            }
        }
    }
    
    /**
     *  Flush stdin
     */
    public void flush() {
        LOGGER.finer("flush");
        stdin.flush();
    }
    
    /**
     *  print a string to stdin (with line separator)
     *  @param x the string
     */ 
    public void println(String x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'");
        }
        stdin.println(x);
    }
    
    /**
     *  print a string to stdin
     *  @param s the string
     */ 
    public void print(String s) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + s + "'");
        }
        stdin.print(s);
    }
    
    /**
     *  print an object to stdin (with linefeed)
     *  @param x the object
     */ 
    public void println(Object x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'");
        }
        stdin.println(x);
    }
    
    /**
     *  print an object to stdin
     *  @param obj the object
     */ 
    public void print(Object obj) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + obj + "'");
        }
        stdin.print(obj);
    }
    
    /**
     *  print an integer to stdin (with linefeed)
     *  @param x the integer
     */ 
    public void println(int x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'");
        }
        stdin.println(x);
    }
    
    /**
     *  print an integer to stdin
     *  @param i the integer
     */ 
    public void print(int i) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + i + "'");
        }
        stdin.print(i);
    }
    
    /**
     *  print a boolean to stdin
     *  @param b the boolean
     */ 
    public void print(boolean b) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + b + "'");
        }
        stdin.print(b);
    }
    
    /**
     *  print a boolean to stdin (with linefeed)
     *  @param x the boolean
     */ 
    public void println(boolean x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'<NL>");
        }
        stdin.println(x);
    }
    
    /**
     *  print a long to stdin (with linefeed)
     *  @param x the long
     */ 
    public void println(long x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'<NL>");
        }
        stdin.println(x);
    }
    
    /**
     *  print a long to stdin
     *  @param l the long
     */ 
    public void print(long l) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + l + "'");
        }
        stdin.print(l);
    }
    
    /**
     *  print a double to stdin (with linefeed)
     *  @param x the double
     */ 
    public void println(double x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'<NL>");
        }
        stdin.println(x);
    }
    
    /**
     *  print a char arrary to stdin (with line feed)
     *  
     *  <b>!!!Attention!!!:</b> For security reasons this method should not 
     *  be used to send passwords to the process, sice the content of <code>x</code>
     *  is written to the logger.
     *
     *  @param x the char array
     */ 
    public void println(char[] x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            StringBuffer msg = new StringBuffer();
            for(int i = 0; i < x.length; i++) {
                if(i>0) {
                    msg.append(' ');
                }
                msg.append('\'');
                msg.append(x[i]);
                msg.append('\'');
            }
            
            LOGGER.log(Level.FINE, "send " + msg + "<NL>");
        }
        stdin.println(x);
    }
    
    /**
     *  print a char arrary to stdin
     *  
     *  <b>!!!Attention!!!:</b> For security reasons this method should not 
     *  be used to send passwords to the process, sice the content of <code>s</code>
     *  is written to the logger.
     *
     *  @param s the char array
     */ 
    public void print(char[] s) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + new String(s) + "'");
        }
        stdin.print(s);
    }

    /**
     *  print a password to stdin
     *  
     *  @param s the password
     */ 
    public void printPassword(char[] s) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send a password");
        }
        stdin.print(s);
    }
    
    /**
     *  print a password to stdin (with linefeed)
     *  
     *  @param s the password
     */ 
    public void printlnPassword(char[] s) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send a password<NL>");
        }
        stdin.println(s);
    }
    
    /**
     *  print a double to stdin
     *  
     *  @param d the double
     */ 
    public void print(double d) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + d + "'");
        }
        stdin.print(d);
    }
    
    /**
     *  print a char to stdin.
     *  
     *  @param x the char
     */ 
    public void println(char x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'<NL>");
        }
        stdin.println(x);
    }
    
    /**
     *  print a char to stdin.
     *  
     *  @param c the char
     */ 
    public void print(char c) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + c + "'<NL>");
        }
        stdin.print(c);
    }
    
    /**
     *  print a float to stdin (with linefeed).
     *  
     *  @param x the float
     */ 
    public void println(float x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'<NL>");
        }
        stdin.println(x);
    }
    
    /**
     *  print a float to stdin.
     *  
     *  @param x the float
     */ 
    public void print(float x) {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send '" + x + "'");
        }
        stdin.print(x);
    }
    
    /**
     *  print linefeed to stdin.
     */ 
    public void println() {
        if(LOGGER.isLoggable(Level.FINE)) {
            LOGGER.log(Level.FINE, "send <NL>");
        }
        stdin.println();
    }
    
    
}
