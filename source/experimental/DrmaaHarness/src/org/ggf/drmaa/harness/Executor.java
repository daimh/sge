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

/*
 * Executor.java
 *
 * Created on March 29, 2005, 9:38 PM
 */

package org.ggf.drmaa.harness;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Properties;
import org.ggf.drmaa.JobTemplate;

/**
 *
 * @author dan.templeton@sun.com
 */
class Executor extends DrmaaThread {
   private Process p = null;
   
   /** Creates a new instance of Executor */
   Executor (String id, JobTemplate jt, Object waitLock) {
      super (id, jt, waitLock);
   }
   
   public void run () {
      Runtime r = Runtime.getRuntime ();
      int numArgs = 0;
      String[] cmdArray = null;
      String[] envArray = null;
      File workingDirectory = null;
      String name = jt.getJobName ();
      String[] args = jt.getArgs ();
      Properties env = jt.getJobEnvironment ();
      String command = jt.getRemoteCommand ();
      String workingDirectoryPath = jt.getWorkingDirectory ();

      if (name == null) {
         name = command;
      }
      
      if (args != null) {
         numArgs = args.length;
      }
      
      cmdArray = new String[numArgs + 1];
      
      cmdArray[0] = command;
      
      if (args != null) {
         System.arraycopy (args, 0, cmdArray, 1, args.length);
      }
      
      if (env != null) {
         int count = 0;
         Iterator i = env.keySet ().iterator ();
         
         envArray = new String[env.size ()];         
         
         while (i.hasNext ()) {
            String var = (String)i.next ();
            String value = (String)env.get (var);
            
            envArray[count++] = var + "=" + value;
         }
      }
      
      if (workingDirectoryPath != null) {
         workingDirectory = new File (workingDirectoryPath);
      }
      else {
         workingDirectory = new File (".");
      }
      
      this.setState (RUNNING);
      
      try {
System.out.println ("Execing: " + Arrays.asList (cmdArray) + ":" + envArray + ":" + workingDirectory);
         this.startTimer ();
         p = r.exec (cmdArray, envArray, workingDirectory);
      }
      catch (IOException e) {
e.printStackTrace ();
         this.stopTimer ();
         this.setState (FAILED);
         this.notifyWaits ();

         return;
      }

      Thread outThread = new Thread (new Writer (workingDirectory, name + ".o" + id, p.getInputStream ()));
      Thread errThread = new Thread (new Writer (workingDirectory, name + ".e" + id, p.getErrorStream ()));
      
      outThread.start ();
      errThread.start ();
      
      boolean done = false;
      
      while (!done) {
         try {
            p.waitFor ();
System.out.println ("Complete");
            exitCode = p.exitValue ();
            done = true;
         }
         catch (InterruptedException e) {
e.printStackTrace ();
            this.stopTimer ();
            this.setState (FAILED);
            this.notifyWaits ();
            
            return;
         }
      }
      
      this.stopTimer ();
      this.setState (DONE); 
      this.notifyWaits ();
   }

   public void kill () {
      p.destroy ();
      
      super.kill ();
   }
}

class Writer implements Runnable {
   private File workingDirectory = null;
   private String name = null;
   private InputStream in = null;
   
   Writer (File workingDirectory, String name, InputStream in) {
      this.workingDirectory = workingDirectory;
      this.name = name;
      this.in = in;
   }
   
   public void run () {
      try {
         File out = new File (workingDirectory, name);
         FileWriter fout = new FileWriter (out);
         BufferedWriter bout = new BufferedWriter (fout);
         InputStreamReader sin = new InputStreamReader (in);
         BufferedReader bin = new BufferedReader (sin);
         String line = null;

         while ((line = bin.readLine ()) != null) {
            bout.write (line);
            bout.newLine ();
         }
         
         bin.close ();
         bout.close ();
      }
      catch (IOException e) {
e.printStackTrace ();
      }
   }
}