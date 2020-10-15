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
package com.sun.grid.jgdi.util;

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.configuration.GEObject;
import com.sun.grid.jgdi.configuration.xml.XMLUtil;
import com.sun.grid.jgdi.management.JGDIProxy;
import com.sun.grid.jgdi.management.mbeans.JGDIJMXMBean;
import com.sun.grid.jgdi.util.shell.AbortException;
import com.sun.grid.jgdi.util.shell.AbstractCommand;
import com.sun.grid.jgdi.util.shell.AnnotatedCommand;
import com.sun.grid.jgdi.util.shell.CommandAnnotation;
import com.sun.grid.jgdi.util.shell.QStatCommand;
import com.sun.grid.jgdi.util.shell.Shell;
import java.io.BufferedReader;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.ResourceBundle;
import java.util.TreeSet;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;
import javax.security.auth.Subject;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.ConfirmationCallback;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.callback.TextOutputCallback;
import javax.security.auth.callback.UnsupportedCallbackException;
import javax.security.auth.login.LoginContext;
import javax.security.auth.login.LoginException;
import java.io.File;
import java.lang.reflect.Constructor;
import java.net.URI;
import java.net.URL;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 *
 */
public class JGDIShell implements Runnable, Shell {

    private static Logger logger = Logger.getLogger(JGDIShell.class.getName());
    private static String PROMPT = "jgdi> ";
    private static String caTopPath;
    private static String keystorePath;
    private List<HistoryElement> historyList = new LinkedList<HistoryElement>();
    private int historyIndex = 0;
    private int maxHistory = 30;
    private JGDI jgdi;
    private Map<String, Class<? extends AbstractCommand>> cmdMap = new HashMap<String, Class<? extends AbstractCommand>>();
    private TreeSet<String> cmdSet = null;
    private ReadlineHandler readlineHandler;
    private static final ResourceBundle usageResources = ResourceBundle.getBundle("com.sun.grid.jgdi.util.shell.UsageResources");
    private final PrintWriter out;
    private final PrintWriter err;
    private int lastExitCode = 0;
    private static boolean usePW = false;
    private static boolean useKS = false;

    //TODO LP: We should consider having single PrintWriter for all commands
    public JGDIShell() {
        out = new PrintWriter(System.out);
        err = new PrintWriter(System.err);
        // Register all command classes
        try {
            List<Class> classes = getAllAnnotatedClasses(this.getClass().getPackage(), CommandAnnotation.class);
            for (Class cls : classes) {
                addAnnotatedCommand(cls);
            }
        } catch (Exception ex) {
            err.println("Not connected " + ex.getMessage());
        }
        //TODO LP: we need real qselect command, not just alias to qstat
        cmdMap.put("qselect", QStatCommand.class);

        cmdSet = new TreeSet<String>(cmdMap.keySet());
        readlineHandler = createReadlineHandler();

    //Read resource bundles
    }

    /**
     * Register the annotated  class
     * @param cls Annotated class child of AbstractCommand
     * @throws java.lang.Exception
     */
    @SuppressWarnings(value = "unchecked")
    private void addAnnotatedCommand(Class cls) throws Exception {
        AbstractCommand cmd = null;
        CommandAnnotation ca = (CommandAnnotation) cls.getAnnotation(CommandAnnotation.class);
        if (AbstractCommand.class.isAssignableFrom(cls)) {

            cmdMap.put(ca.value(), cls);
            if (AnnotatedCommand.class.isAssignableFrom(cls)) {
                AnnotatedCommand.initOptionDescriptorMap(cls, out, err);
            }
        } else {
            throw new IllegalStateException("Can not assign " + cls.getName() + " from Command.class");
        }
    }

    private AbstractCommand getCommand(String name) throws Exception {
        Class<? extends AbstractCommand> cls = cmdMap.get(name);
        if (cls == null) {
            return null;
        }
        AbstractCommand cmd = null;
        //If JGDIShell inner class
        if (cls.getName().contains(".JGDIShell$")) {
            Constructor c = cls.getConstructor(new Class[]{JGDIShell.class});
            cmd = (AbstractCommand) c.newInstance(new Object[]{this});
        } else {
            cmd = (AbstractCommand) cls.newInstance();
        }
        return cmd;
    }

    /**
     * Getter method
     * @return a <b>JGDI</b> object
     */
    public JGDI getConnection() {
        if (jgdi == null) {
            throw new IllegalStateException("Not connected");
        }
        return jgdi;
    }

    /**
     * Getter method
     * @return a logger
     */
    public Logger getLogger() {
        return logger;
    }

    /**
     * Getter method
     * @return a standard output print writer
     */
    public PrintWriter getOut() {
        if (out == null) {
            throw new IllegalStateException("out pw is not initialized");
        }
        return out;
    }

    /**
     * Getter method
     * @return a standard error print writer
     */
    public PrintWriter getErr() {
        if (err == null) {
            throw new IllegalStateException("err pw is not initialized");
        }
        return err;
    }

    public void run() {
        try {
            while (true) {
                String line = readlineHandler.readline(PROMPT);
                //Print empty line
                if (line == null) {
                    continue;
                }
                lastExitCode = runCommand(line);
            }
        } catch (EOFException eofe) {
        // Ignore
        } catch (IOException ioe) {
            logger.severe(ioe.getMessage());
        }
        readlineHandler.cleanup();
        System.exit(0);
    }

    private int runCommand(String line) {
        int exitCode = 0;
        line = line.trim();
        if (line.length() == 0) {
            return 0;
        }
        try {
            if (line.charAt(0) == '!') {
                int id = -1;
                // get command from history
                String name = line.substring(1);
                try {
                    id = Integer.parseInt(name);
                } catch (NumberFormatException nfe) {
                    throw new IllegalArgumentException("Expected !<num>, but got: " + line);
                }
                line = null;
                try {
                    HistoryElement elem = historyList.get(++id);
                    line = elem.getLine();
                } catch (IndexOutOfBoundsException ioob) {
                    throw new IllegalArgumentException("command with id " + id + " not found in history");
                }
            }
            ParsedLine parsedLine = new ParsedLine(line);
            AbstractCommand cmd = getCommand(parsedLine.cmd);
            if (cmd == null) {
                exitCode = runShellCommand(line);
                return exitCode;
            }

            if (historyList.size() > maxHistory) {
                historyList.remove(0);
            }
            historyList.add(new HistoryElement(++historyIndex, line));

            cmd.init(this);
            cmd.run(parsedLine.args);
            return cmd.getExitCode();
        } catch (AbortException expected) {
            exitCode = 0;
        } catch (JGDIException ex) {
            err.println(ex.getMessage());
            logger.info("Command failed: " + ex.getMessage());
            exitCode = ex.getExitCode();
        } catch (IllegalArgumentException ex) {
            err.println(ex.getMessage());
            logger.info("Command failed: " + ex.getMessage());
            exitCode = 1; //There was an error during the execution
        } catch (Exception ex) {
            ex.printStackTrace(err);
            logger.log(Level.SEVERE, "Command failed: " + ex.getMessage(), ex);
            exitCode = 1000; //There was an error during the execution
        } finally {
            out.flush();
            err.flush();
        }
        return exitCode;
    }

    public int runShellCommand(String line) throws InterruptedException, IOException, InterruptedException {
        out.println("Executing /bin/sh -c " + line);
        out.flush();
        String[] cmds = {"/bin/sh", "-c", line};
        Process p = Runtime.getRuntime().exec(cmds);
        GetResponse to = new GetResponse(p.getInputStream(), out);
        GetResponse te = new GetResponse(p.getErrorStream(), err);
        to.start();
        te.start();
        p.waitFor();
        to.join();
        te.join();
        return p.exitValue();
    }

    @SuppressWarnings(value = "unchecked")
    public static void main(String[] args) {
        String url = null;
        boolean useCsp = false;

        String cmdParams = "";

        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-c")) {
                i++;
                if (i >= args.length) {
                    logger.severe("Missing connection url");
                    System.exit(1);
                }
                url = args[i];
            } else if (args[i].equals("-csp")) {
                useCsp = true;
            } else if (args[i].equals("-pw")) {
                usePW = true;
            } else if (args[i].equals("-ks")) {
                useKS = true;
            } else if (args[i].equals("-catop")) {
                i++;
                if (i >= args.length) {
                    logger.severe("Missing catop path");
                    System.exit(1);
                }
                caTopPath = args[i];
            } else if (args[i].equals("-keystore")) {
                i++;
                if (i >= args.length) {
                    logger.severe("Missing keystore path");
                    System.exit(1);
                }
                keystorePath = args[i];
            } else {
                cmdParams += args[i] + " ";
            }
        }

        final JGDIShell shell = new JGDIShell();

        if (useCsp) {
            LoginContext lc = null;
            String jaasContextName = "jgdi";
            logger.config("setup jaas login context for jgdi");
            final String finalUrl = url;

            try {
                lc = new LoginContext(jaasContextName, new MyCallbackHandler());

                lc.login();
                try {
                    Subject.doAs(lc.getSubject(), new PrivilegedAction() {

                        public Object run() {
                            shell.exec(finalUrl);
                            return null;
                        }
                    });
                } finally {
                    lc.logout();
                }
            } catch (LoginException ex) {
                ex.printStackTrace();
            }
        } else {
            if (cmdParams.length() > 0) {
                shell.exec(url, cmdParams);
            } else {
                shell.exec(url);
            }
        }
    }

    public static String getResourceString(String key) {
        return usageResources.getString(key);
    }

    private void exec(final String url) {
        if (url != null) {
            runCommand("connect " + url);
        }
        run(); //We didn't get any other params, so we actually run the JGDIShell
    }

    private void exec(final String url, final String cmdParams) {
        if (url != null) {
            runCommand("connect " + url);
            //We just want to execute a single command passed as param and exit.
            int exitCode = runCommand(cmdParams);
            System.exit(exitCode);
        }
    }

    class ParsedLine {

        String cmd;
        String[] args;

        public ParsedLine(String line) {
            if (logger.isLoggable(Level.FINE)) {
                logger.fine("parse line '" + line + "'");
            }

            int i = 0;
            for (i = 0; i < line.length(); i++) {
                if (Character.isWhitespace(line.charAt(i))) {
                    cmd = line.substring(0, i).toLowerCase();
                    if (logger.isLoggable(Level.FINE)) {
                        logger.fine("cmd is \'" + cmd + "\'");
                    }
                    break;
                }
            }

            if (cmd == null) {
                cmd = line.toLowerCase();
                args = new String[0];
                if (logger.isLoggable(Level.FINE)) {
                    logger.fine("arg[" + cmd + "] = ");
                }
                return;
            }

            ArrayList<String> argList = new ArrayList<String>();

            while (i < line.length()) {
                char c = line.charAt(i);
                if (Character.isWhitespace(c)) {
                    i++;
                    continue;
                }
                if (c == '\"') {
                    // Quoted String
                    i++;
                    int startIndex = i;
                    while (i < line.length() && line.charAt(i) != '\"') {
                        i++;
                    }
                    if (i >= line.length()) {
                        throw new IllegalArgumentException("exit" + startIndex);
                    }
                    String arg = line.substring(startIndex, i);
                    if (logger.isLoggable(Level.FINE)) {
                        logger.fine("arg[" + argList.size() + "] = " + arg);
                    }
                    argList.add(arg);
                    i++;
                } else {
                    int startIndex = i;
                    while (i < line.length() && !Character.isWhitespace(line.charAt(i))) {
                        i++;
                    }
                    if (startIndex < i) {
                        String arg = line.substring(startIndex, i);
                        if (logger.isLoggable(Level.FINE)) {
                            logger.fine("arg[" + argList.size() + "] = " + arg);
                        }
                        argList.add(arg);
                    } else {
                        logger.fine("empty arg");
                    }
                }
            }
            args = new String[argList.size()];
            argList.toArray(args);
        }
    }

    @CommandAnnotation(value = "exit")
    class ExitCommand extends AbstractCommand {

        public ExitCommand() {
            super();
        }

        public void run(String[] args) throws Exception {
            if (jgdi != null) {
                try {
                    jgdi.close();
                    logger.info("disconnect from " + jgdi);
                } catch (Exception e) {
                // ignore
                }
            }
            readlineHandler.cleanup();
            System.exit(lastExitCode);
        }

        @Override
        public void init(Shell shell) throws Exception {
        }
    }

    @CommandAnnotation(value = "help")
    class HelpCommand extends AbstractCommand {

        public HelpCommand() {
            super();
        }

        public void run(String[] args) throws Exception {
            switch (args.length) {
                case 0:
                    out.println("Available commands: ");
                    for (String cmd : cmdMap.keySet()) {
                        out.println(cmd);
                    }
                    break;
                case 1:
                    AbstractCommand cmd = getCommand(args[0]);
                    if (cmd == null) {
                        out.println("command " + args[0] + " not found");
                        return;
                    }
                    out.println(cmd.getUsage());
                    break;
                default:
                    out.println(getUsage());
            }
        }

        @Override
        public void init(Shell shell) throws Exception {
        }
    }

    @CommandAnnotation(value = "connect")
    class ConnectCommand extends AbstractCommand {

        public ConnectCommand() {
            super();
        }

        public void run(String[] args) throws Exception {

            if (args.length != 1) {
                throw new IllegalArgumentException("Invalid argument count");
            }

            if (jgdi != null) {
                try {
                    jgdi.close();
                } catch (JGDIException ex1) {
                    final String msg = "close failed: " + ex1.getMessage();
                    err.println(msg);
                    logger.warning(msg);
                }
            }
            logger.info("connect to " + args[0]);
            jgdi = JGDIFactory.newInstance(args[0]);
        }

        @Override
        public void init(Shell shell) throws Exception {
        }
    }

    @CommandAnnotation(value = "connectjmx")
    class ConnectJMXCommand extends AbstractCommand {

        public ConnectJMXCommand() {
            super();
        }

        public void run(String[] args) throws Exception {
            if (args.length != 2) {
                throw new IllegalArgumentException("Invalid argument count");
            }

            if (jgdi != null) {
                try {
                    jgdi.close();
                } catch (JGDIException ex1) {
                    final String msg = "close failed: " + ex1.getMessage();
                    err.println(msg);
                    logger.warning(msg);
                }
            }
            String host = args[0];
            int port = Integer.parseInt(args[1]);
            logger.info("connect to host(" + host + ":" + port + ")");
            NameCallback ncb = new NameCallback("user:");
            PasswordCallback pwcb = new PasswordCallback("password:", false);
            Callback[] callbacks = {ncb, pwcb};
            MyCallbackHandler cb = new MyCallbackHandler();
            usePW = true;
            cb.handle(callbacks);
            String username = ncb.getName();
            String userpw = pwcb.getPassword().toString();
            if (useKS) {
                PasswordCallback kspw = new PasswordCallback("keystore password:", true);
                Callback[] kcallbacks = {kspw};
                cb.handle(kcallbacks);
                File caTop = new File(caTopPath);
                File keyStore = new File(keystorePath);
                JGDIProxy.setupSSL(host, port, caTop, keyStore, kspw.getPassword());
                kspw.clearPassword();
                kspw = null;
            }
            Object credentials = new String[]{username, userpw};
            JGDIProxy jgdiProxy = JGDIFactory.newJMXInstance(host, port, credentials);

            jgdi = JMXJGDIBridge.newInstance(jgdiProxy);
        }

        @Override
        public void init(Shell shell) throws Exception {
        }
    }

    static class JMXJGDIBridge implements InvocationHandler {

        private final JGDIProxy jgdiJMX;
        private final JGDI proxy;
        private static boolean libNotLoaded = true;

        private static synchronized void initLib() throws JGDIException {
            if (libNotLoaded) {
                try {
                    System.loadLibrary("jgdi");
                    libNotLoaded = false;
                } catch (Throwable e) {
                    JGDIException ex = new JGDIException("Can not load native jgdi lib: " + e.getMessage());
                    // ex.initCause(e);  does not work for whatever reason
                    throw ex;
                }
            }
        }

        private JMXJGDIBridge(JGDIProxy jgdiJMX) {
            this.jgdiJMX = jgdiJMX;
            this.proxy = (JGDI) Proxy.newProxyInstance(getClass().getClassLoader(), new Class<?>[]{JGDI.class}, this);
        }

        public static JGDI newInstance(JGDIProxy jgdiJMX) throws JGDIException {
            initLib();
            JMXJGDIBridge bridge = new JMXJGDIBridge(jgdiJMX);
            return bridge.proxy;
        }

        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
            if (method.getName().equals("equals")) {
                return this.equals(args[0]);
            } else if (method.getName().equals("hashcode")) {
                return this.hashCode();
            } else if (method.getName().equals("toString")) {
                return String.format("JMX to JGDI Bridge (%s)", jgdiJMX.toString());
            } else {
                Method jmxMethod = JGDIJMXMBean.class.getMethod(method.getName(), method.getParameterTypes());
                return jmxMethod.invoke(jgdiJMX.getProxy(), args);
            }
        }
    }

    @CommandAnnotation(value = "debug")
    class DebugCommand extends AbstractCommand {

        public DebugCommand() {
            super();
        }

        // TODO Logger.global is deprecated, clean up this
        @SuppressWarnings(value = "deprecation")
        public void run(String[] args) throws Exception {

            String loggerName = logger.getName();
            Level level = null;

            switch (args.length) {
                case 0:
                    loggerName = null;
                    level = null;
                    break;
                case 1:
                    loggerName = args[0];
                    break;
                case 2:
                    loggerName = args[0];
                    level = Level.parse(args[1]);
                    break;
                default:
                    throw new IllegalArgumentException("Invalid number of arguments");
            }

            Enumeration en = LogManager.getLogManager().getLoggerNames();
            while (en.hasMoreElements()) {
                Logger aLogger = LogManager.getLogManager().getLogger((String) en.nextElement());
                if (aLogger.equals(Logger.global) || aLogger.getName() == null || aLogger.getName().length() == 0) {
                    continue;
                }
                if (loggerName == null || aLogger.getName().startsWith(loggerName)) {
                    if (level == null) {
                        logger.info(aLogger.getName() + ": " + aLogger.getLevel());
                    } else {
                        logger.info("set level of logger " + aLogger.getName() + " to " + level);
                        aLogger.setLevel(level);
                    }
                }
            }
        }

        @Override
        public void init(Shell shell) throws Exception {
        }
    }

    @CommandAnnotation(value = "history")
    class PrintHistoryCommand extends AbstractCommand {

        public PrintHistoryCommand() {
            super();
        }

        public void run(String[] args) throws Exception {
            for (HistoryElement elem : historyList) {
                out.printf("%5d %s%n", elem.getId(), elem.getLine());
            }
        }

        @Override
        public void init(Shell shell) throws Exception {
        }
    }

    @CommandAnnotation(value = "xmldump")
    class XMLDumpCommand extends AbstractCommand {

        public XMLDumpCommand() {
            super();
        }

        public void run(String[] args) throws Exception {

            if (args.length != 2) {
                throw new IllegalAccessException(" failed");
            }
            if (jgdi == null) {
                throw new IllegalStateException("Can't get class loader.");
            }
            if (args[1].equals("all")) {
                Method method = JGDI.class.getMethod("get" + args[0] + "List", (java.lang.Class[]) null);
                List list = (List) method.invoke(jgdi, (java.lang.Object[]) null);
                Iterator iter = list.iterator();
                while (iter.hasNext()) {
                    Object obj = iter.next();
                    XMLUtil.write((GEObject) obj, System.out);
                    System.out.flush();
                }
            } else {
                Method method = JGDI.class.getMethod("get" + args[0], new Class[]{String.class});
                Object obj = method.invoke(jgdi, new Object[]{args[1]});
                XMLUtil.write((GEObject) obj, System.out);
            }
        }

        @Override
        public void init(Shell shell) throws Exception {
        }
    }

    @CommandAnnotation(value = "$?")
    class GetExitCodeCommand extends AbstractCommand {

        public GetExitCodeCommand() {
            super();
        }

        public void run(String[] args) throws Exception {
            out.println(lastExitCode);
        }

        @Override
        public void init(Shell shell) throws Exception {
        }
    }

    static class HistoryElement {

        private int id;
        private String line;

        public HistoryElement(int id, String line) {
            this.id = id;
            this.line = line;
        }

        public int getId() {
            return id;
        }

        public String getLine() {
            return line;
        }
    }

    private ReadlineHandler createReadlineHandler() {
        try {
            Class readlineClass = Class.forName("org.gnu.readline.Readline");
            Class readlineLibClass = Class.forName("org.gnu.readline.ReadlineLibrary");
            @SuppressWarnings(value = "unchecked")
            Method byNameMethod = readlineLibClass.getMethod("byName", new Class[]{String.class});
            @SuppressWarnings(value = "unchecked")
            Method loadMethod = readlineClass.getMethod("load", new Class[]{readlineLibClass});

            String[] libs = {"GnuReadline", "Editline", "Getline", "PureJava"};

            for (int i = 0; i < libs.length; i++) {
                try {
                    Object readlineLib = byNameMethod.invoke(readlineLibClass, new Object[]{libs[i]});

                    if (readlineLib == null) {
                        logger.fine("lib ReadLine." + libs[i] + " is unknown");
                        continue;
                    }


                    loadMethod.invoke(readlineClass, new Object[]{readlineLib});

                    @SuppressWarnings(value = "unchecked")
                    Method initReadlineMethod = readlineClass.getMethod("initReadline", new Class[]{String.class});
                    initReadlineMethod.invoke(readlineClass, new Object[]{"JGDIShell"});

                    NativeReadlineHandler ret = new NativeReadlineHandler(readlineClass);
                    logger.info("use Readline." + libs[i]);
                    return ret;
                } catch (Exception e) {
                    logger.log(Level.FINE, "Readline." + libs[i] + " failed", e);
                }

            }
        } catch (NoSuchMethodException ex) {
            // Ignore
        } catch (ClassNotFoundException cnfe) {
            // Ignore
        }
        return new DefaultReadlineHandler();
    }

    /**
     * Attempts to list annotated classes in the given package and its ancestors
     * as determined by the context class loader
     *
     * @param pkg a <b>Package</b> as a starting point
     * @param annotated <b>Class</b> the annotation class to search
     * @return a list of all annotated classes that exist within that package
     * @throws ClassNotFoundException if something went wrong
     */
    @SuppressWarnings(value = "unchecked")
    public static List<Class> getAllAnnotatedClasses(Package pkg, Class annotated) throws ClassNotFoundException {
        ArrayList<Class> classes = new ArrayList<Class>();
        try {
            ClassLoader cld = Thread.currentThread().getContextClassLoader();
            if (cld == null) {
                throw new ClassNotFoundException("Can\'t get class loader.");
            }
            String path = pkg.getName().replace('.', '/') + "/";
            // This will hold a list of directories matching the pckgname.
            //There may be more than one if a package is split over multiple jars/paths
            Enumeration<URL> resources = cld.getResources(path);
            while (resources.hasMoreElements()) {
                final URL nextElement = resources.nextElement();
                // Slipt out just the jar file name
                final JarFile jarFile = new JarFile(new File(new URI(nextElement.getPath().split("!")[0])));
                // Go throught all elements in jar and ask for annotation for those in pkg
                for (Enumeration e = jarFile.entries(); e.hasMoreElements();) {
                    JarEntry current = (JarEntry) e.nextElement();
                    final String name = current.getName();
                    // Is at least from the same package as pkg and it is a class
                    if (name.length() > path.length() && name.substring(0, path.length()).equals(path) && name.endsWith(".class")) {
                        try {
                            final Class forName = Class.forName(current.getName().replaceAll("/", ".").replace(".class", ""));
                            if (forName.isAnnotationPresent(annotated)) {
                                classes.add(forName);
                            }
                        } catch (NoClassDefFoundError expected) {
                        }
                    }
                }
            }
        } catch (Exception ex) {
            throw new ClassNotFoundException(ex.getMessage());
        }

        return classes;
    }

    static interface ReadlineHandler {

        public String readline(String prompt) throws IOException;

        public void cleanup();
    }

    class NativeReadlineHandler implements ReadlineHandler {

        private Class readlineClass;
        private Method readlineMethod;
        private Method cleanupMethod;

        @SuppressWarnings(value = "unchecked")
        public NativeReadlineHandler(Class readlineClass) throws Exception {
            this.readlineClass = readlineClass;
            this.readlineMethod = readlineClass.getMethod("readline", new Class[]{String.class});
            this.cleanupMethod = readlineClass.getMethod("cleanup", (java.lang.Class[]) null);

            Class completerClass = Class.forName("org.gnu.readline.ReadlineCompleter");

            Method setCompleterMethod = readlineClass.getMethod("setCompleter", new Class[]{completerClass});

            InvocationHandler handler = new NativeCompletionHandler();
            Object completer = Proxy.newProxyInstance(Thread.currentThread().getContextClassLoader(), new Class[]{completerClass}, handler);

            // Setup a completer proxy
            setCompleterMethod.invoke(readlineClass, new Object[]{completer});
        }

        public String readline(String prompt) throws IOException {
            try {
                return (String) readlineMethod.invoke(readlineClass, new Object[]{prompt});
            } catch (IllegalAccessException ex) {
                IllegalStateException ilse = new IllegalStateException("Unknown error");
                ilse.initCause(ex);
                throw ilse;
            } catch (InvocationTargetException ex) {
                if (ex.getTargetException() instanceof IOException) {
                    throw (IOException) ex.getTargetException();
                } else if (ex.getTargetException() instanceof RuntimeException) {
                    throw (RuntimeException) ex.getTargetException();
                } else {
                    IllegalStateException ilse = new IllegalStateException("Unknown error");
                    ilse.initCause(ex.getTargetException());
                    throw ilse;
                }
            }
        }

        public void cleanup() {
            try {
                this.cleanupMethod.invoke(readlineClass, (java.lang.Object[]) null);
            } catch (Exception e) {
                logger.log(Level.WARNING, "cleanup failed", e);
            }
        }
    }

    class NativeCompletionHandler implements InvocationHandler {

        Iterator possibleValues;

        public java.lang.String complete(java.lang.String text, int state) {
            //System.out.println("complete " + text + ", " + state);
            if (state == 0) {
                possibleValues = cmdSet.tailSet(text).iterator();
            }
            if (possibleValues.hasNext()) {
                String nextKey = (String) possibleValues.next();
                if (nextKey.startsWith(text)) {
                    return nextKey;
                }
            }
            return null;
        }

        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
            return complete((String) args[0], ((Integer) args[1]).intValue());
        }
    }

    class DefaultReadlineHandler implements ReadlineHandler {

        private BufferedReader in;

        public DefaultReadlineHandler() {
            in = new BufferedReader(new InputStreamReader(System.in));
        }

        public String readline(String prompt) throws IOException {
            System.out.print(prompt);
            return in.readLine();
        }

        public void cleanup() {
            try {
                in.close();
            } catch (IOException ioe) {
                logger.log(Level.WARNING, "cleanup failed", ioe);
            }
        }
    }

    private static class MyCallbackHandler implements CallbackHandler {

        public MyCallbackHandler() {
        }

        public void handle(Callback[] callbacks) throws IOException, UnsupportedCallbackException {

            for (int i = 0; i < callbacks.length; i++) {
                logger.fine("handle callback i " + i + ": " + callbacks[i]);
                if (callbacks[i] instanceof TextOutputCallback) {
                    logger.fine("skip text output callback " + callbacks[i]);
                    continue;
                } else if (callbacks[i] instanceof NameCallback) {
                    NameCallback cb = (NameCallback) callbacks[i];
                    // if (cb.getPrompt().indexOf("alias") >= 0) {
                    logger.fine("user.name: " + System.getProperty("user.name"));
                    cb.setName(System.getProperty("user.name"));
//                    } else {
//                        logger.fine("cb.getPrompt(): " + cb.getPrompt());
//                        throw new UnsupportedCallbackException(callbacks[i]);
//                    }
                } else if (callbacks[i] instanceof PasswordCallback) {
                    PasswordCallback cb = (PasswordCallback) callbacks[i];
                    if (usePW) {
                        // TODO: replace by native func to suppress password echo
                        System.out.print(cb.getPrompt() + " ");
                        System.out.flush();
                        String pw = new BufferedReader(new InputStreamReader(System.in)).readLine();
                        cb.setPassword(pw.toCharArray());
                        pw = null;
                    } else {
                        cb.setPassword(new char[0]);
                    }
                } else if (callbacks[i] instanceof ConfirmationCallback) {
                    ConfirmationCallback cb = (ConfirmationCallback) callbacks[i];
                    cb.setSelectedIndex(cb.getDefaultOption());
                } else {
                    throw new UnsupportedCallbackException(callbacks[i]);
                }
            }
        }
    }

    /**
     * Helper for handlinng shell streams in runShellCommand
     */
    class GetResponse extends Thread {

        private InputStream stream;
        private PrintWriter pw;

        public GetResponse(InputStream is, PrintWriter pw) {
            stream = is;
            this.pw = pw;
        }

        @Override
        public void run() {
            while (true) {
                try {
                    int c = stream.read();
                    if (c == -1) {
                        break;
                    }
                    pw.print((char) c);
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
            }
        }
    }
}
