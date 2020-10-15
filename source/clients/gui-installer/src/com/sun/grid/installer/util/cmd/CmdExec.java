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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.installer.util.cmd;

import java.io.IOException;
import java.util.List;
import java.io.File;
import java.util.Map;
import java.util.Vector;

import com.izforge.izpack.util.Debug;
import com.sun.grid.installer.util.Config;
import java.io.FileReader;
import java.io.LineNumberReader;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Properties;

/**
 * Executes the given commands and stores their exit value and output streams.
 */
public abstract class CmdExec implements Config {
    private ProcessBuilder processBuilder = null;

    private static long WAIT_TIME = 300;
    long MAX_WAIT_TIME;

    private int exitValue = EXIT_VAL_CMDEXEC_INITIAL;

    private Vector<String> outVector, errVector, additionalErrors;
    private String firstMessage;

    private File outFile, errFile;

    public CmdExec(long timeout) {
        MAX_WAIT_TIME = timeout;
        additionalErrors = new Vector<String>();
    }

    static boolean isSameCommand(String first, String second) {
        first = first.toLowerCase();
        second = second.toLowerCase();
        
        if (first.equals(second)) {
            return true;
        }
        //We also accept "/usr/local/ssh -X" as equal to "ssh"
        String[] elems = first.split("\\s");
        for (String elem : elems) {
            if (elem.endsWith("/"+second)) {
                return true;
            }
        }
        return false;
    }

    public static String  getShellOptions(String shell) {
        return (isSameCommand(shell, "ssh")) ? "-o StrictHostKeyChecking=yes -o PreferredAuthentications=gssapi-keyex,publickey" : "";
    }

    private String setupOutputFiles(String commands) {
        try {
            outFile = File.createTempFile("gui-cmdexec", ".out", new File("/tmp"));
            errFile = File.createTempFile("gui-cmdexec", ".err", new File("/tmp"));
            commands += " > "+outFile.getAbsolutePath()+" 2> "+ errFile.getAbsolutePath();
        } catch (IOException ex) {
            additionalErrors.add(this.getClass().getName() + ".init: " + ex.getMessage());
        } finally {
            return commands;
        }
    }


    public void execute(String commands) {
        //Exit if we skipped the commands (on local host)
        if (exitValue != -1) {
            return;
        }
        List<String> cmdList = new ArrayList<String>();
        cmdList.add("sh");
        cmdList.add("-c");
        commands = setupOutputFiles(commands);
        cmdList.add(commands);
        processBuilder = new ProcessBuilder(cmdList);

        Process process = null;
        long cmdId = 0;

        try {
            process = processBuilder.start();

            long startTime = System.currentTimeMillis();
            long waitTime = 0;
            while (true) {
                try {
                    exitValue = process.exitValue();
                    break;
                } catch (IllegalThreadStateException ex) {
                    Thread.sleep(WAIT_TIME);
                    waitTime = System.currentTimeMillis() - startTime;
                    if (waitTime > MAX_WAIT_TIME) {
                        Debug.error("Terminated ("+MAX_WAIT_TIME / 1000+"sec): '" + processBuilder.command() + "'!");
                        process.destroy();
                        exitValue = EXIT_VAL_CMDEXEC_TERMINATED;
                        break;
                    }
                }
            }
            cmdId = (long)(Math.random()*1000000);
        } catch (IOException ex) {
            exitValue = EXIT_VAL_CMDEXEC_OTHER;
            additionalErrors.add(ex.getLocalizedMessage());
        } catch (InterruptedException ex) {
            exitValue = EXIT_VAL_CMDEXEC_INTERRUPTED;
            additionalErrors.add(ex.getLocalizedMessage());
        } finally {
            outVector = getInput(outFile);
            errVector = getInput(errFile);
            errVector.addAll(additionalErrors);
            Debug.trace(cmdId + " output: "+ outVector);
            Debug.trace(cmdId + " error: "+ errVector);
            //Parse the output and change the exit code if needed! We can't trust RSH, so we look for __EXIT_CODE_num___
            int tmpExitValue = 0;
            String tmpNum = "", d;
            int exitStringPos=-1;
            //Our special exit code should be the last line!
            for (int i = outVector.size() - 1 ; i >= 0 ; i--) {
                d = outVector.get(i);
                if (d.matches("___EXIT_CODE_[1-9]?[0-9]___")) {
                    exitStringPos = i;
                    tmpNum = d.substring("___EXIT_CODE_".length());
                    tmpNum = tmpNum.substring(0, tmpNum.indexOf("_"));
                    tmpExitValue = Integer.valueOf(tmpNum);
                    break;
                }
            }
            //Remove this exit line!
            if (exitStringPos >= 0) {
                outVector.remove(exitStringPos);
            }
            //Set the correct exitValue
            exitValue = (tmpExitValue != 0 && exitValue == 0) ? tmpExitValue : exitValue;
            //Delete on exit to have better debug, until we close the APP
            outFile.deleteOnExit();
            errFile.deleteOnExit();
            Debug.trace(cmdId + " Command: " + processBuilder.command() + " timeout: "+ MAX_WAIT_TIME/1000 + "sec exitValue: "+exitValue);
        }
    }

    public void setWorkingDirectory(File directory) {
        processBuilder.directory(directory);
    }

    public List getCommands() {
        return processBuilder.command();
    }

    public Map getEnvironment() {
        return processBuilder.environment();
    }

    public Vector<String> getOutput() {
        return outVector;
    }

    public void setFirstLogMessage(String message) {
        firstMessage = message;
    }

    /* Command must have finished! */
    public String generateLog(final Properties msgs) {
        return generateLog(exitValue, msgs);
    }

    /* Command must have finished! */
    public String generateLog(int exitValue, final Properties msgs) {
        String log = "", stdLog = "", errLog = "", message = "";

        if (exitValue == EXIT_VAL_CMDEXEC_INITIAL) {
            return "Tried to retrieve exit code before command has finished! We have no log so far.";
        }

        String SEP = System.getProperty("line.separator");
        message = (String) msgs.get("msg.exit." + exitValue);
        if (exitValue == EXIT_VAL_CMDEXEC_TERMINATED) {
            //timeout
            message = MessageFormat.format(message, MAX_WAIT_TIME / 1000);
            log += "FAILED: " + message + SEP;
        } else if (exitValue > 0 && message != null && message.length() > 0) {
            log += "FAILED: " + message + SEP;
        } else if (exitValue > 0) {
            message = MessageFormat.format((String) msgs.get("msg.exit.fail.general"), exitValue);
            log += "FAILED: " + message + SEP;
        }
        //Add firstMessage
        if (firstMessage != null && firstMessage.trim().length() > 0) {
            log += firstMessage;
        }
        //Append both output/error streams, delete all empty lines
        if (getOutput() != null) {
            for (String d : getOutput()) {
                if (d.trim().length() > 0) {
                    stdLog += d + SEP;
                }
            }
        }
        if (getError() != null) {
            for (String d : getError()) {
                if (d.trim().length() > 0) {
                    errLog += d + SEP;
                }
            }
        }
        //Finalize the output
        if (stdLog.trim().length() > 0) {
            if (log.length() == 0) {
               log = "OUTPUT:" + SEP + stdLog;
            } else {
               log += SEP + "OUTPUT:" + SEP + stdLog;
            }
        }
        if (errLog.trim().length() > 0) {
            log += SEP + "ERROR:" + SEP + errLog;
        }
        if (log.trim().length() == 0) {
            log = "No output.";
        }
        return log;
    }

    public Vector<String> getError() {
        return  errVector;
    }

    /**
     * @return the exitValue
     */
    public int getExitValue() {
        return exitValue;
    }


    private Vector<String> getInput(File f) {
        LineNumberReader lnr = null;
        Vector<String> v = new Vector<String>();
        try {
            lnr = new LineNumberReader(new FileReader(f));
            String line;
            while ((line = lnr.readLine()) != null) {
                v.add(line);
            }
            return v;
        } catch (IOException ex) {
            additionalErrors.add(ex.getLocalizedMessage());
            return v;
        } finally {
            try {
                lnr.close();
            } catch (IOException ex) {
            }
        }
    }
}
