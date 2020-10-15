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
package com.sun.grid.jgdi;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 */
public class JobSubmitter {
    
    private static final Logger logger = Logger.getLogger(JobSubmitter.class.getName());
    
    private static String replaceParams(ClusterConfig cluster, String str) {
        StringBuilder buf = new StringBuilder(str);
        
        String[] params = new String[]{"$SGE_ROOT"};
        String[] values = new String[]{cluster.getSgeRoot()};
        
        for (int i = 0; i < params.length; i++) {
            int index = buf.indexOf(params[i]);
            if (index >= 0) {
                buf.replace(index, index + params[i].length(), values[i]);
            }
        }
        return buf.toString();
    }
    
    public static int submitJob(ClusterConfig cluster, String[] args) throws IOException, InterruptedException {
        
        String arch = getArch(cluster);
        String qsub = cluster.getSgeRoot() + File.separatorChar + "bin" + File.separatorChar + arch + File.separatorChar + "qsub";
        
        String[] cmd = new String[args.length + 1];
        
        cmd[0] = qsub;
        for (int i = 0; i < args.length; i++) {
            cmd[i + 1] = replaceParams(cluster, args[i]);
        }
        
        String[] env = new String[]{"SGE_ROOT=" + cluster.getSgeRoot(), "SGE_CELL=" + cluster.getSgeCell(), "SGE_QMASTER_PORT=" + cluster.getQmasterPort(), "SGE_EXECD_PORT=" + cluster.getExecdPort(), "LD_LIBRARY_PATH=" + cluster.getSgeRoot() + File.separatorChar + "lib" + File.separatorChar + arch};
        
        if (logger.isLoggable(Level.INFO)) {
            
            StringBuilder buf = new StringBuilder();
            for (int i = 0; i < cmd.length; i++) {
                if (i > 0) {
                    buf.append(" ");
                }
                buf.append(cmd[i]);
            }
            logger.log(Level.INFO, "exec: {0}", buf.toString());
        }
        
        Process p = Runtime.getRuntime().exec(cmd, env);
        
        Pump stdout = new Pump(p.getInputStream());
        Pump stderr = new Pump(p.getErrorStream());
        stdout.start();
        stderr.start();
        try {
            p.waitFor();
        } finally {
            stdout.join(1000);
            if (stdout.isAlive()) {
                stdout.interrupt();
                stdout.join();
            }
            stderr.join(1000);
            if (stderr.isAlive()) {
                stderr.interrupt();
                stderr.join();
            }
        }
        
        if (p.exitValue() == 0) {
            
            List messages = stdout.getOutput();
            if (messages.isEmpty()) {
                throw new IOException("Got not output from qsub");
            }
            String line = (String) messages.get(0);
            
            if (line.startsWith(YOUR_JOB)) {
                int end = line.indexOf('(', YOUR_JOB.length());
                
                if (end < 0) {
                    throw new IOException("Invalid output of qsub (" + line + ")");
                }
                
                String jobStr = line.substring(YOUR_JOB.length(), end).trim();
                
                try {
                    int ret = Integer.parseInt(jobStr);
                    logger.log(Level.INFO, "job {0} submitted", jobStr);
                    return ret;
                } catch (NumberFormatException nfe) {
                    throw new IOException("Invalid output of qsub (" + line + ")");
                }
            } else if (line.startsWith(YOUR_JOB_ARRAY)) {
                int end = line.indexOf('.', YOUR_JOB_ARRAY.length());
                
                if (end < 0) {
                    throw new IOException("Invalid output of qsub (" + line + ")");
                }
                
                String jobStr = line.substring(YOUR_JOB_ARRAY.length(), end).trim();
                
                try {
                    int ret = Integer.parseInt(jobStr);
                    logger.log(Level.INFO, "job {0} submitted", jobStr);
                    return ret;
                } catch (NumberFormatException nfe) {
                    throw new IOException("Invalid output of qsub (" + line + ")");
                }
            } else {
                throw new IOException("Invalid output of qsub (" + line + ")");
            }
        } else {
            List<String> messages = stderr.getOutput();
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            pw.println("qsub exited with status " + p.exitValue());
            for (String message : messages) {
                pw.println(message);
            }
            pw.close();
            throw new IOException(sw.getBuffer().toString());
        }
    }
    private static final String YOUR_JOB = "Your job ";
    private static final String YOUR_JOB_ARRAY = "Your job-array ";
    private static Map archMap = new HashMap();
    
    public static synchronized String getArch(ClusterConfig cluster) throws IOException {
        
        String arch = (String) archMap.get(cluster.getSgeRoot());
        if (arch == null) {
            
            try {
                String cmd = cluster.getSgeRoot() + File.separatorChar + "util" + File.separatorChar + "arch";
                
                String[] env = new String[]{"SGE_ROOT=" + cluster.getSgeRoot()};
                
                Process p = Runtime.getRuntime().exec(cmd, env);
                
                Pump stdout = new Pump(p.getInputStream());
                
                stdout.start();
                
                p.waitFor();
                stdout.interrupt();
                stdout.join();
                
                if (p.exitValue() != 0) {
                    throw new IOException("arch script exited with status " + p.exitValue());
                }
                
                List lines = stdout.getOutput();
                if (lines.size() != 1) {
                    throw new IOException("Invalid output of arch script");
                }
                arch = (String) lines.get(0);
                archMap.put(cluster.getSgeRoot(), arch);
            } catch (InterruptedException ire) {
                throw new IOException("arch script has been interrupted");
            }
        }
        return arch;
    }
    
    private static class Pump extends Thread {
        
        private List<String> output = new LinkedList<String>();
        private InputStream in;
        
        public Pump(InputStream in) {
            this.in = in;
        }
        
        public void run() {
            try {
                BufferedReader rd = new BufferedReader(new InputStreamReader(in));
                String line = null;
                while ((line = rd.readLine()) != null) {
                    getOutput().add(line);
                }
            } catch (IOException ioe) {
                // Ignore
            }
        }
        
        public List getOutput() {
            return output;
        }
    }
}