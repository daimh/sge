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

package com.sun.grid.loadsensor;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.logging.Logger;

/**
 * The LoadSensorManager implements the load sensor protocol.  A load
 * sensor can use this class either by instantiating an instance and passing
 * in the LoadSensor implementation instance or by running the
 * LoadSensorManager class directly and passing as an argument the
 * name of the class to be run as the LoadSensor implementation.
 *
 * Once started, the LoadSensorManager instance reads the input from the
 * load sensor framework from stdin and writes commands to stdout.  The
 * load sensor protocol is completely encapsulated, however, allowing the
 * LoadSensor implementation to work at the level of name=value pairs for
 * measured load values.
 *
 * The LoadSensorManager will monitor how often it is asked for load values
 * and how long it takes to take a measurement.  It will use that information
 * to try to optimally schedule the next measurement.  The idea is to run
 * the measurement as late as possible but still have it complete before
 * the next time load values are requested.
 *
 * Alternatively, the load sensor may specify its own measurement interval
 * independent of when the load value requests are made.  In that case,
 * the measurements are made in a separate thread, and load value requests
 * are given the last set of load values reported by the measurement
 * thread.
 *
 * The LoadSensorManager class uses the Logger named
 * "com.sun.grid.LoadSensor".  By default, the Logger has no handlers
 * installed.  In order to enable logging, add a handler, i.e. a FileHandler
 * instance.
 *
 * It is always a good idea to enabling logging, as errors in load sensor
 * protocol handling will be logged as warnings, but will not be visible any
 * other way.
 * @since 6.2u5
 */
public class LoadSensorManager {
    // There are no default handlers that log to stdout, so the logger is safe
    private static final Logger log = Logger.getLogger("com.sun.grid.LoadSensor");
    // The load sensor to manage
    private final LoadSensor loadSensor;
    // A timer for delayed measurements
    private Timer timer = null;
    // The last time we were asked to load values
    private long lastRun = 0L;
    // How long the interval between being asked for load values was last time
    private int lastInterval = 0;
    // The average of how long it takes to take a measurement
    private int timeToMeasure = 0;
    // Whether measurements are triggered by requesting load values
    private transient boolean doMeasurement = true;
    // Where output should be sent
    private PrintStream out = System.out;
    // From where input should be read
    private InputStream in = System.in;

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        if (args.length > 0) {
            LoadSensor instance = null;

            try {
                Class loadSensor = Class.forName(args[0]);

                instance = (LoadSensor)loadSensor.newInstance();
            } catch (ClassCastException e) {
                log.severe(args[0] + " does not implement com.sun.grid.loadsensor.LoadSensor");
                System.exit(2);
            } catch (ClassNotFoundException e) {
                log.severe("Unable to load load sensor class: " + args[0]);
                System.exit(3);
            } catch (IllegalAccessException e) {
                log.severe("Unable to access load sensor class: " + args[0]);
                log.severe(e.getMessage());
                System.exit(4);
            } catch (InstantiationException e) {
                log.severe("Unable to instantiate load sensor class: " + args[0]);
                log.severe(e.getMessage());
                System.exit(5);
            } catch (SecurityException e) {
                log.severe("Unable to instantiate load sensor class: " + args[0]);
                log.severe(e.getMessage());
                System.exit(6);
            }

            LoadSensorManager manager = new LoadSensorManager(instance);

            try {
                manager.parse();
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
     * Create an instance for this load sensor.
     * @param loadSensor the load sensor to manage
     */
    public LoadSensorManager(LoadSensor loadSensor) {
        this.loadSensor = loadSensor;

        long before = 0L;
        long after = 0L;

        if (loadSensor != null) {
            log.finest("Collecting load values from load sensor");

            try {
                before = System.currentTimeMillis();
                // We call the LoadSensor.doMeasurement() instead of our own
                // because we don't want to confuse the time to the first load
                // report with an actual load report interval
                loadSensor.doMeasurement();
                after = System.currentTimeMillis();

                adjustTimeToMeasure((int)(after - before));
            } catch (RuntimeException e) {
                log.warning("load sensor threw exception while measuring load values: " + e.getClass().getSimpleName() + " -- " + e.getMessage());
            }
        }
    }
    
    /**
     * Print the usage information.
     */
    private static void printUsage() {
        log.info("java -jar loadsensor.jar load_sensor_class [arg,...]");
    }

    /**
     * Start the load sensor.  This method reads commands from the
     * execution daemon and triggers appropriate actions on the managed
     * load sensor.  This method will not return until the input stream is
     * closed or it reads "QUIT\n" from the stream.
     * @throws IOException Thrown is there is an error communicating
     * with the execution daemon
     */
    public void parse() throws IOException {
        BufferedReader bin = new BufferedReader(new InputStreamReader(in));
        String line = null;

        while ((line = bin.readLine()) != null) {
            log.fine(">>> " + line);

            if (line.equalsIgnoreCase("QUIT")) {
                log.fine("load sensor exiting");
                return;
            } else if (line.equals("")) {
                Map<String,Map<String,String>> values = null;

                calculateTiming();

                try {
                    values = loadSensor.getLoadValues();

                    if ((values != null) && (values.size() > 0)) {
                        send("begin");

                        for (String host : values.keySet()) {
                            if (values.get(host) != null) {
                                for (String key : values.get(host).keySet()) {
                                    send(host + ":" + key + ":" + values.get(host).get(key));
                                }
                            }
                        }

                        send("end");
                    }
                } catch (RuntimeException e) {
                    log.warning("load sensor threw exception while retrieving load values: " + e.getClass().getSimpleName() + " -- " + e.getMessage());
                }

                boolean localMeasurement = false;

                synchronized (this) {
                    localMeasurement = doMeasurement;
                }

                if (localMeasurement) {
                    doMeasurement();
                }
            } else {
                log.warning("ignoring invalid input line: " + line);
            }
        }

        if (timer != null) {
            timer.cancel();
        }
    }

    /**
     * Send a response to the execution daemon
     * @param message the message to send
     * @throws IOException Thrown is there is an error communicating
     * with the execution daemon
     */
    private void send(String message) throws IOException {
        String print = message != null ? message : "";

        log.fine("<<< " + message);
        out.println(print);
    }

    /**
     * This method keeps a running average of the amount of time required
     * to execute the LoadSensor.doMeasurement() method.  The "average" is
     * calculated as 7/8 the previous value plus 1/8 the new value.
     * @param timeToMeasure the time it took to run the
     * LoadSensor.doMeasurement() method in milliseconds
     */
    private void adjustTimeToMeasure(int timeToMeasure) {
        if (timeToMeasure < 0) {
            log.warning("attempted to adjust the measurement time by a negative value");
        } else {
            if (this.timeToMeasure > 0) {
                this.timeToMeasure = (int)Math.ceil((this.timeToMeasure * 7.0f / 8.0f) + (timeToMeasure / 8000.0f));
            } else {
                this.timeToMeasure = (int)Math.ceil(timeToMeasure / 1000.0F);
            }

            // Always assume it takes some time to measure
            if (this.timeToMeasure == 0) {
                this.timeToMeasure = 1;
            }

            log.finest("The adjusted time to measure is now " + this.timeToMeasure);
        }
    }

    /**
     * Calculate how many seconds passed between the last load value
     * requests from the execution daemon.  The result is stored in the
     * lastInterval member variable.
     */
    private void calculateTiming() {
        long thisRun = System.currentTimeMillis();

        if (lastRun != 0) {
            lastInterval = (int)Math.ceil((thisRun - lastRun) / 1000.0F);
            log.finest("The last load report interval was " + lastInterval);
        }

        lastRun = thisRun;
    }

    /**
     * This method triggers a call to the LoadSensor.doMeasurement()
     * method.  Depending on the value returned from the
     * LoadSensor.getMeasurementInterval() method, the call to
     * LoadSensor.doMeasurement() may happen immediately, after a delay,
     * or not at all.
     */
    private void doMeasurement() {
        int timeout = 0;

        try {
            timeout = loadSensor.getMeasurementInterval();
        } catch (RuntimeException e) {
            log.warning("load sensor threw exception while getting the measurement interval: " + e.getClass().getSimpleName() + " -- " + e.getMessage());
        }

        // If the interval is 0, then we measure on demand
        if (timeout <= 0) {
            synchronized (this) {
                // If we were previously measuring periodically, then we have to
                // handle things a little differently
                if (!doMeasurement) {
                    lastInterval = -1;
                    doMeasurement = true;
                }
            }
            
            if (lastInterval > 0) {
                // If we know the last interval, use it
                doDelayedMeasurement(adjustedInterval());
            } else if (lastInterval == 0) {
                takeMeasurement();
            } // Otherwise keep the last measured values and wait for the next
              // load report
        } else {
            // Otherwise, we're measuring periodically
            synchronized (this) {
                doMeasurement = false;
            }

            doPeriodicMeasurement(timeout);
        }
    }

    /**
     * Actually call the LoadSensor.doMeasurement() method.
     */
    private void takeMeasurement() {
        long before = 0L;
        long after = 0L;
        // If this is the first load report interval, just take the
        // measurement now
        log.finest("Collecting load values from load sensor");

        try {
            before = System.currentTimeMillis();
            loadSensor.doMeasurement();
            after = System.currentTimeMillis();

            adjustTimeToMeasure((int)(after - before));
        } catch (RuntimeException e) {
            log.warning("load sensor threw exception while measuring load values: " + e.getClass().getSimpleName() + " -- " + e.getMessage());
        }
    }

    /**
     * This method returns a time in seconds that is how long until the next
     * measurement should be taken.  It is based on how long the previous
     * time between load values requests was and the average duration of
     * the LoadSensor.doMeasurement() method call.
     * @return how long in seconds until the next measurement should be
     * taken
     */
    private int adjustedInterval() {
        int ret = 0;

        if ((lastInterval >= 2 * timeToMeasure) && (timeToMeasure >= 0)) {
            ret = lastInterval - (2 * (timeToMeasure > 0 ? timeToMeasure : 1));
        }

        return ret;
    }

    /**
     * Take a measurement after a number of seconds
     * @param timeout delay in seconds until the next measurement
     */
    private void doDelayedMeasurement(long timeout) {
        if (timer == null) {
            timer = new Timer("Measurement Thread", true);
        }

        log.finest("Collecting load values again in " + timeout + " seconds");
        timer.schedule(new DelayedMeasurementTask(), timeout * 1000);
    }

    /**
     * Take a measurement after a number of seconds and then call the
     * doMeasurement() method again.
     * @param timeout delay in seconds until the next measurement
     * @see #doMeasurement()
     */
    private void doPeriodicMeasurement(long timeout) {
        if (timer == null) {
            timer = new Timer("Measurement Thread", true);
        }

        log.finest("Collecting load values again in " + timeout + " seconds");
        timer.schedule(new PeriodicMeasurementTask(), timeout * 1000);
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
     * TimerTask to execute a delayed measurement.
     */
    private class DelayedMeasurementTask extends TimerTask {
        @Override
        public void run() {
            takeMeasurement();
        }
    }

    /**
     * TimerTask to execute a delayed measurement and then call
     * doMeasurement() again.
     * @see #doMeasurement()
     */
    private class PeriodicMeasurementTask extends DelayedMeasurementTask {
        @Override
        public void run() {
            super.run();
            
            doMeasurement();
        }
    }
}
