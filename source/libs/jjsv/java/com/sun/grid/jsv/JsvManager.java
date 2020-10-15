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
 *   Copyright: 2009 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jsv;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Logger;

/**
 * The JsvManager class is implements the JSV protocol.  A JSV can use the
 * JsvManager class either by instantiating an instance and passing in the
 * Jsv implementation instance or by running the JsvManager class directly
 * and passing as an arguement the name of the class to be run as the Jsv
 * implementation.
 *
 * Once started, the JsvManager instance reads the input from the JSV framework
 * from stdin and writes commands to stdout.  The JSV protocol is completely
 * encapsulated, however, allowing the Jsv implementation to work at the
 * level of JobDescription objects.
 *
 * The JsvManager class uses the Logger named "com.sun.grid.Jsv".  By default
 * the Logger is set not to pass log records to its parent logger because
 * the default Logger writes to stdout and would disrupt the JSV protocol.  By
 * default, the Logger has no handlers installed.  In order to enable logging,
 * add a handler, i.e. a FileHandler instance.
 * 
 * It is always a good idea to enabling logging, as errors in JSV protocol
 * handling will be logged as warnings, but will not be visible any other way.
 *
 * TODO: Write docs
 * TODO: Add method to log the entire parameter set
 * TODO: Look at showMaps() to see if it can be done more cleanly
 * TODO: Add main() method
 * TODO: ant script
 * TODO: Wrapper script
 * TODO: correct=&gt;modify in other languages
 * @see JobDescription
 * @since 6.2u5
 */
public class JsvManager {
    /**
     * The LogLevel enumeration is used to control the log level of messages
     * logged through the JSV framework.  Note that this log level is
     * independent of the Logger.
     * @see #log(com.sun.grid.jsv.JsvManager.LogLevel, java.lang.String)
     */
    public enum LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    /**
     * The Result enumeration is used to set the result of a JSV verification
     * operation.
     * @see Jsv#onVerify(com.sun.grid.jsv.JsvManager)
     */
    public enum Result {
        ACCEPT,
        MODIFY,
        REJECT,
        REJECT_WAIT;

        @Override
        public String toString() {
            String retValue = super.toString();
            
            if (this == Result.MODIFY) {
                retValue = "CORRECT";
            }

            return retValue;
        }
    }

    /**
     * The Command enumeration is used internally when sending commands via the
     * JSV framework.
     */
    private enum Command {
        LOG,
        PARAM,
        ENV
    };

    /**
     * The Operation enumeration is used internally when sending environment
     * commands via the JSV framework.
     */
    private enum Operation {
        ADD,
        MOD,
        DEL
    }

    private static final Logger log;
    // Temporary holding place for incoming environment data
    private final Map<String, String> environment =
            new HashMap<String, String>();
    // Log of the parameters sent by the JSV framework for logMaps()
    private final Map<String,String> paramLog = new HashMap<String,String>();
    // Log of the environment variables sent by the JSV framework for logMaps()
    private final Map<String,String> envLog = new HashMap<String,String>();
    // The description of the current job
    private JobDescription parameters = new JobDescription();
    // The result of the current verification operation
    private Result result = Result.ACCEPT;
    // The explanation of the result of the current verification operation
    private String resultMessage = null;
    // Whether the JSV has requested that the environment be sent
    private boolean requestEnv = false;
    // Where output should be sent
    private PrintStream out = System.out;
    // From where input should be read
    private InputStream in = System.in;

    // Initialize the Logger and turn off parent handlers
    static {
        // Set up out logger so that we don't log to stdout by default
        log = Logger.getLogger("com.sun.grid.Jsv");
        log.setUseParentHandlers(false);
    }

    /**
     * The main() method takes the name of a Jsv implementation class and uses
     * that class to instantiate and run a JSV.
     * @param args String array that contains the name of the class to run
     */
    public static void main(String[] args) {
        if (args.length > 0) {
            Jsv instance = null;

            try {
                Class jsv = Class.forName(args[0]);

                instance = (Jsv)jsv.newInstance();
            } catch (ClassCastException e) {
                log.severe(args[0] + " does not implement com.sun.grid.jsv.Jsv");
                System.exit(2);
            } catch (ClassNotFoundException e) {
                log.severe("Unable to load JSV class: " + args[0]);
                System.exit(3);
            } catch (IllegalAccessException e) {
                log.severe("Unable to access JSV class: " + args[0]);
                log.severe(e.getMessage());
                System.exit(4);
            } catch (InstantiationException e) {
                log.severe("Unable to instantiate JSV class: " + args[0]);
                log.severe(e.getMessage());
                System.exit(5);
            } catch (SecurityException e) {
                log.severe("Unable to instantiate JSV class: " + args[0]);
                log.severe(e.getMessage());
                System.exit(6);
            }

            JsvManager manager = new JsvManager();

            try {
                manager.parse(instance);
            } catch (IOException e) {
                log.severe("Error while communicating with framework: " +
                        e.getMessage());
                System.exit(7);
            } catch (IllegalArgumentException e) {
                log.severe("Error while setting job parameters: " +
                        e.getMessage());
                System.exit(8);
            } catch (RuntimeException e) {
                log.severe("Error while executing JSV: " +
                        e.getClass().getSimpleName() + " -- " + e.getMessage());
                System.exit(8);
            }
        } else {
            log.severe("No arguments specified");
            printUsage();
            System.exit(1);
        }
    }

    /**
     * Print the usage information.
     */
    private static void printUsage() {
        log.info("java -jar JSV.jar jsv_class [arg,...]");
    }

    /**
     * Set the destination for output.  Mostly useful for testing.
     * @param out The new destination for output
     */
    void setOut(PrintStream out) {
        if (out != null) {
            this.out = out;
        } else {
            this.out = System.out;
        }
    }

    /**
     * Set the new source for input.  Mostly useful for testing.
     * @param in The new source for input
     */
    void setIn(InputStream in) {
        if (in != null) {
            this.in = in;
        } else {
            this.in = System.in;
        }
    }

    /**
     * Start the JSV.  This method reads input and triggers appropriate actions
     * on the given Jsv implementation.  It will not return until it receives
     * the "QUIT" command from the JSV framework.
     * @param jsv The Jsv implementation to be run
     * @throws IOException Thrown if it is not possible to communicate with the
     * JSV framework
     */
    public void parse(Jsv jsv) throws IOException {
        String line = null;
        BufferedReader bin = new BufferedReader(new InputStreamReader(in));

        while ((line = bin.readLine()) != null) {
            log.fine(">>> " + line);

            String[] tokens = line.split(" ");

            if (tokens.length > 0) {
                if ((tokens.length == 1) && tokens[0].equals("START")) {
                    // Begin verification phase
                    resetState();
                    
                    // Call handler
                    if (jsv != null) {
                        try {
                            jsv.onStart(this);
                        } catch (RuntimeException e) {
                            log.warning("jsv threw an exception while starting: " + e.getClass().getSimpleName() + " -- " + e.getMessage());
                        }
                    }

                    // Request environment
                    if (requestEnv) {
                        sendCommand("SEND ENV");
                    }

                    // End start phase
                    sendCommand("STARTED");
                } else if ((tokens.length == 1) && tokens[0].equals("BEGIN")) {
                    // Prepare for handler
                    prepareMaps();

                    // Run handler
                    if (jsv != null) {
                        try {
                            jsv.onVerify(this);
                        } catch (RuntimeException e) {
                            log.warning("jsv threw an exception while verifying: " + e.getClass().getSimpleName() + " -- " + e.getMessage());
                        }
                    }

                    // Process results of handler
                    processMaps();

                    // End verify phase
                    sendResult();
                } else if (((tokens.length == 3) || (tokens.length == 4))
                           && tokens[0].equals("ENV") && tokens[1].equals("ADD")) {
                    tokens[3] = tokens[3].replaceAll("\\\\b", "\b");
                    tokens[3] = tokens[3].replaceAll("\\\\f", "\f");
                    tokens[3] = tokens[3].replaceAll("\\\\n", "\n");
                    tokens[3] = tokens[3].replaceAll("\\\\r", "\r");
                    tokens[3] = tokens[3].replaceAll("\\\\t", "\t");
                    tokens[3] = tokens[3].replaceAll("\\\\", "\\");

                    // Log the env for the SHOW command
                    envLog.put(tokens[2], tokens[3]);
                    // Add env to the master environment
                    environment.put(tokens[2], tokens[3]);
                } else if ((tokens.length == 3) && tokens[0].equals("PARAM")) {
                    // Log the param for the SHOW command
                    paramLog.put(tokens[1], tokens[2]);

                    // Set param in the JSV parameters
                    parameters.set(tokens[1], tokens[2]);
                } else if ((tokens.length == 1) && tokens[0].equals("SHOW")) {
                    logMaps();
                } else if ((tokens.length == 1) && tokens[0].equals("QUIT")) {
                    return;
                } else {
                    log.warning("ignoring invalid input line: " + line);
                }
            }
        }
    }

    /**
     * Prepare the data sent by the JSV framework for use by the JSV.  Called
     * immediately before onVerify()
     * @see Jsv#onVerify(com.sun.grid.jsv.JsvManager)
     */
    private void prepareMaps() {
        // Add the environment to the parameters
        parameters.setEnvironment(environment);
        // Set the parameters' baseline
        parameters.setBaseline();
    }

    /**
     * Process the changes and results from the JSV verification operation.
     * Called immediately after onVerify().  Changes from the JSV will be sent
     * to the JSV framework along with the result.
     * @see Jsv#onVerify(com.sun.grid.jsv.JsvManager)
     */
    private void processMaps() {
        // If the job is being rejected, there's no point in processing the maps
        if ((result == Result.ACCEPT) || (result == Result.MODIFY)) {
            Map<String, String> params = parameters.getDifference();
            boolean modified = false;

            for (String key : params.keySet()) {
                sendParameter(key, params.get(key));
                modified = true;
            }

            Map<String, String> add = new HashMap<String, String>();
            Map<String, String> mod = new HashMap<String, String>();
            Map<String, String> del = new HashMap<String, String>();

            parameters.getEnvironmentDifference(add, mod, del);

            for (String key : add.keySet()) {
                sendEnvironment(Operation.ADD, key, add.get(key));
                modified = true;
            }

            for (String key : mod.keySet()) {
                sendEnvironment(Operation.MOD, key, mod.get(key));
                modified = true;
            }

            for (String key : del.keySet()) {
                sendEnvironment(Operation.DEL, key, del.get(key));
                modified = true;
            }

            // EB: Following code is NOT correct. If JSV says ACCEPT
            // although there are modifications then these modifications
            // should be trashed!
            
            // If the job was modified, the result must be MODIFY
            // if (modified && (result == Result.ACCEPT)) {
            //    result = Result.MODIFY;
            //}
        }
    }

    /**
     * Write the parameters and environment variables sent by the JSV framework
     * to the JSV's log.  Called in response to a "SHOW" command.
     */
    private void logMaps() {
        for (String param : paramLog.keySet()) {
            log(LogLevel.INFO, "got param: " + param + "='" + paramLog.get(param) + "'");
        }

        for (String env : envLog.keySet()) {
            log(LogLevel.INFO, "got env: " + env + "='" + envLog.get(env) + "'");
        }
    }

    /**
     * Write a log message to the JSV's log.
     * @param level The LogLevel to use for the message
     * @param message The message to log
     */
    public void log(LogLevel level, String message) {
        if (level == null) {
            throw new IllegalArgumentException("attempted to set a null level value");
        }

        sendCommand("LOG " + level + " " + message);
    }

    /**
     * Send a parameter command to the JSV framework.
     * @param parameter The name of the parameter
     * @param value The parameter's value
     */
    private void sendParameter(String parameter, String value) {
        if (parameter == null) {
            log.warning("ignoring parameter command with null parameter");
        } else if (value != null) {
            sendCommand("PARAM " + parameter + " " + value);
        } else {
            sendCommand("PARAM " + parameter);
        }
    }

    /**
     * Send an environment command to the JSV framework
     * @param op Whether the ADD, MOD, or DEL the variable
     * @param var The variable's name
     * @param value The variable's value
     */
    private void sendEnvironment(Operation op, String var, String value) {
        if (op == null) {
            log.warning("ignoring environment command with null environment variable operation");
        } else if (var == null) {
            log.warning("ignoring environment command with null environment variable");
        } 
        if (op == Operation.ADD || op == Operation.MOD) {
            sendCommand("ENV " + op + " " + var + " " + value);
        } else if (op == Operation.DEL) {
            sendCommand("ENV " + op + " " + var );
        }
    }

    /**
     * Reset the state of the manager.  Called before starting a new JSV
     * verification operation.
     */
    private void resetState() {
        // Make sure that we're starting with a clean slate
        parameters = new JobDescription();
        environment.clear();
        paramLog.clear();
        envLog.clear();
        requestEnv = false;
        result = Result.ACCEPT;
        resultMessage = null;
    }

    /**
     * Send the result of the JSV verification operation to the JSV framework.
     * Called as the last step of processing a "BEGIN" command.
     */
    private void sendResult() {
        if (resultMessage != null) {
            sendCommand("RESULT STATE " + result.toString() + " " + resultMessage);
        } else {
            sendCommand("RESULT STATE " + result.toString() + " ");
        }
    }

    /**
     * Send a command to the JSV framework.
     * @param command The command to send
     */
    private void sendCommand(String command) {
        if (command == null) {
            log.warning("ignoring command with null command");
        } else {
            String toSend = command.replaceAll("[\\n\\r]", " ").trim();

            log.fine("<<< " + toSend);
            out.println(toSend);
        }
    }

    /**
     * Accept the job.
     * @param message Detail about the acceptance
     */
    public void accept(String message) {
        setResult(Result.ACCEPT, message);
    }

    /**
     * Modify the job.
     * @param message Detail about the modification
     */
    public void modify(String message) {
        setResult(Result.MODIFY, message);
    }

    /**
     * Reject the job.
     * @param message Detail about the rejection
     */
    public void reject(String message) {
        setResult(Result.REJECT, message);
    }

    /**
     * Reject the job but indicate that it may succeed later.
     * @param message Detail about the rejection
     */
    public void rejectWait(String message) {
        setResult(Result.REJECT_WAIT, message);
    }

    /**
     * Set the result of the JSV verification operation.
     * @param result The result of the verification operation
     * @param message Detail about the result
     */
    public void setResult(Result result, String message) {
        if (result == null) {
            throw new IllegalArgumentException("attempted to set the result to a null value");
        }

        this.result = result;
        this.resultMessage = message;
    }

    /**
     * Return the current result.  The result is not final until the onVerify()
     * method has returned.
     * @return the current result
     * @see Jsv#onVerify(com.sun.grid.jsv.JsvManager)
     */
    public Result getResult() {
        return result;
    }

    /**
     * Indicate whether the JSV is interested in information about jobs'
     * environment variables.  This method is normally called from the onStart()
     * method, but it can be called any time prior to the receipt of a "START"
     * command
     * @param request Whether to request that the JSV framework send the
     * complete list of environment variables for every job.
     * @see Jsv#onStart(com.sun.grid.jsv.JsvManager)
     */
    public void requestEnvironment(boolean request) {
        requestEnv = request;
    }

    /**
     * Get the description of the job for the current verification operation.
     * @return the current job description
     */
    public JobDescription getJobDescription() {
        return parameters;
    }
}
