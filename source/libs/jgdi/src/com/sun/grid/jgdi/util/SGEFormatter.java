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

import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.MessageFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.MissingResourceException;
import java.util.StringTokenizer;
import java.util.logging.Formatter;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.LogManager;
import java.util.List;
import java.util.ArrayList;

/**
 * This Formater formats LogRecords into one line. What columns and
 * delimiters are formated can be configured.
 * The default format is<br>
 *   <code>
 *   &lt;date&gt;|&lt;host&gt;|&lt;source method&gt;|&lt;level&gt;|
 *   &lt;message&gt;
 *   </code>
 *   <br>
 *
 *  <p>If the withStacktrace flag is set, the stacktrace of a logged excpetion
 *  is also included.</p>
 *
 *  <H3>Example</H3>
 *
 *  <pre>
 *     Logger logger = Logger.getLogger("test logger");
 *
 *     ConsoleHandler consoleHandler = new ConsoleHandler();
 *     SGEFormatter  formatter = new SGEFormatter("test formatter");
 *
 *     int columns [] = {
 *       SGEFormatter.COL_LEVEL_LONG, SGEFormatter.COL_MESSAGE
 *     };
 *     formatter.setColumns(columns);
 *     formatter.setDelimiter(":");
 *
 *     consoleHandler.setFormatter(formatter);
 *     logger.addHandler(consoleHandler);
 *     logger.setUseParentHandlers(false);
 *
 *  </pre>
 *
 *   This example will create log entries with the following format:
 *
 *  <pre>
 *   &lt;log level&gt;:&lt;message&gt;
 *  </pre>
 *
 */
public class SGEFormatter extends Formatter {
    
    /** the NL String. */
    private static final String NL = System.getProperty("line.separator");
    
    /** default length if the indent for the stacktrace. */
    private static final int DEFAULT_STACKTRACE_INDENT = 30;
    /** max. length of the stacktrace. */
    private static final int MAX_STACKTRACE_LEN = 10;
    /** default width of the source code column. */
    private static final int DEFAULT_SOURCE_COL_WIDTH = 30;
    /** the message format which formats the message it self. */
    private MessageFormat mf;
    /** prefix string for stack traces. */
    private String thrownPrefix;
    /** this flag signalizes wether the stacktraces should be included. */
    private boolean withStackTraceFlag;
    
    /** visible columns. */
    private int[] columns;
    
    /** the date object. */
    private Date  date = new Date();
    
    /** the argument array for MessageFormat. */
    private Object [] args = new Object [] {date, null, null, null };
    
    /** the StringWriter buffers the formated strings. */
    private StringWriter sw = new StringWriter();
    
    /** this Printer writes into switch. */
    private PrintWriter  pw = new PrintWriter(sw);
    
    
    /** This column contains the timestamp of the log record. */
    public static final int COL_TIME = 1;
    
    /** This column contains the hostname. */
    public static final int COL_HOST = 2;
    
    /** This columns contains the name of the SGEFormatter. */
    public static final int COL_NAME = 3;
    
    /** This column contains the thread ID of the log record. */
    public static final int COL_THREAD = 4;
    
    /** This column contains the log level of the log record in a
     *  short form (S, I, D). */
    public static final int COL_LEVEL = 5;
    
    /** This column the log message. */
    public static final int COL_MESSAGE = 6;
    
    /** This column contains the source class and source method
     *  name which produces the log message. */
    public static final int COL_SOURCE = 7;
    
    /** This columns contains the log level of the log record in it's
     *  log form.
     *  @see java.util.logging.Level#toString
     */
    public static final int COL_LEVEL_LONG = 8;
    
    private static final Column [] AVAILABLE_COLUMNS = {
        new Column(COL_TIME, "time"),
        new Column(COL_HOST, "host"),
        new Column(COL_NAME, "name"),
        new Column(COL_THREAD, "thread"),
        new Column(COL_LEVEL, "level"),
        new Column(COL_MESSAGE, "message"),
        new Column(COL_SOURCE, "source"),
        new Column(COL_LEVEL_LONG, "level_long")
    };
    
    public static class Column {
        int id;
        String name;
        public Column(int id, String name) {
            this.id = id;
            this.name = name;
        }
        public int getId() { return id; }
        public String getName() {
            return name;
        }
    }
    
    public static Column getColumn(String name) {
        for(int i = 0; i < AVAILABLE_COLUMNS.length; i++ ) {
            if(AVAILABLE_COLUMNS[i].getName().equalsIgnoreCase(name)) {
                return AVAILABLE_COLUMNS[i];
            }
        }
        return null;
    }
    
    /** The default columns are
     *  <code>COL_TIME</code>, <code>COL_HOST</code>, <code>COL_SOURCE</code>,
     *  <code>COL_LEVEL</code>, <code>COL_MESSAGE</code>.
     */
    public static final int [] DEFAULT_COLUMNS = {
        COL_TIME, COL_HOST, COL_SOURCE, COL_LEVEL, COL_MESSAGE
    };
    
    /** name of the formatter. */
    private String name;
    
    /** the delimiter between two columns (default is "|"). */
    private String delimiter = "|";
    
    /**
     *  Create a new <code>SGEFormatter</code>.
     *
     *  @param aName   name of the formatter
     */
    public SGEFormatter(final String aName) {
        this(aName, true);
    }
    
    /**
     *  Create a new <code>SGEFormatter</code>.
     *
     *  @param aName            name of the formatter
     *  @param withStackTrace  include the stacktrace
     */
    public SGEFormatter(final String aName, final boolean withStackTrace) {
        this(aName, withStackTrace, DEFAULT_COLUMNS);
    }
    
    
    
    /**
     *  Create a new <code>SGEFormatter</code>.
     *
     *  @param aName           name of the formatter
     *  @param withStackTrace  include the stacktrace
     *  @param aColumns         visible columns
     */
    public SGEFormatter(final String aName, final boolean withStackTrace,
            final int [] aColumns) {
        this.name = aName;
        this.setWithStackTrace(withStackTrace);
        this.columns = aColumns;
    }
    
    public SGEFormatter() {
        LogManager manager = LogManager.getLogManager();
        
        String cname = getClass().getName();
        
        String str = manager.getProperty(cname + ".withStacktrace");
        if(str != null) {
            setWithStackTrace(Boolean.valueOf(str).booleanValue());
        }
        
        str = manager.getProperty(cname + ".columns");
        
        if(str != null ) {
            StringTokenizer st = new StringTokenizer(str, " ");
            
            List columns = new ArrayList();
            
            while(st.hasMoreTokens() ) {
                Column col = getColumn(st.nextToken());
                if(col != null) {
                    columns.add(col);
                }
            }
            this.columns = new int[columns.size()];
            for(int i = 0; i < columns.size(); i++) {
                this.columns[i] = ((Column)columns.get(i)).getId();
            }
            
        } else {
            this.columns = DEFAULT_COLUMNS;
        }
        
        str = manager.getProperty(cname + ".name");
        if(str == null) {
            str = "Unknown";
        }
        this.name = name;
    }
    
    /**
     *  clear the formatter, with the next call of format the
     *  formatter will be reinitialized.
     */
    private void clear() {
        mf = null;
        args = null;
    }
    
    /**
     *   initialize the formatter.
     */
    private void init() {
        String host;
        
        StringBuilder message = new StringBuilder();
        
        int argsCount = 0;
        int dateCol = -1;
        for (int i = 0; i < columns.length; i++) {
            if (i > 0) {
                message.append(delimiter);
            }
            switch(columns[i]) {
                case COL_TIME:
                    message.append("{");
                    message.append(argsCount);
                    message.append(",date,dd/MM/yyyy HH:mm:ss}");
                    dateCol = argsCount;
                    argsCount++;
                    break;
                case COL_HOST:
                    try {
                        host = InetAddress.getLocalHost().getHostName();
                    } catch (UnknownHostException unhe) {
                        host = "unknown";
                    }
                    message.append(host);
                    break;
                case COL_NAME:
                    message.append(name);
                    break;
                default:
                    message.append("{" + argsCount  + "}");
                    argsCount++;
            }
            
            args = new Object [argsCount];
            if (dateCol >= 0) {
                args[dateCol] = date;
            }
        }
        
        StringBuilder prefix = new StringBuilder();
        int prefixLen = DEFAULT_STACKTRACE_INDENT;
        
        for (int i = 0; i < prefixLen; i++) {
            prefix.append(' ');
        }
        thrownPrefix = prefix.toString();
        
        mf = new MessageFormat(message.toString());
        
    }
    
    /**
     *  set the delimiter.
     *
     *  @param aDelimiter   the new delimiter
     */
    public final void setDelimiter(final String aDelimiter) {
        clear();
        this.delimiter = aDelimiter;
    }
    
    /**
     *  set the columns for the formatter.
     *
     * @param aColumns the columns
     */
    public final void setColumns(final int[] aColumns) {
        clear();
        this.columns = aColumns;
    }
    
    
    /**
     * set the <code>withStackTrace</code> flag.
     *
     * @param  withStackTrace  the new value of the flag
     */
    public final void setWithStackTrace(final boolean withStackTrace) {
        withStackTraceFlag = withStackTrace;
    }
    
    /**
     * get the <code>withStackTrace</code> flag.
     *
     * @return  the <code>withStackTrace</code> flag
     */
    public final boolean getWithStackTrace() {
        return withStackTraceFlag;
    }
    
    /**
     *  get a string with a fixed length.
     *
     *  @param  str         the content of the string
     *  @param  length      the fixed length of the string
     *  @param  rightAlign  should the fixed str be right aligned
     *  @return the string with the fixed length
     */
    private String getFixStr(final String str, final int length,
            final boolean rightAlign) {
        
        if (str.length() > length) {
            return str.substring(str.length() - length);
        } else {
            StringBuilder ret = new StringBuilder(length);
            
            int len = length - str.length();
            if (rightAlign) {
                ret.append(str);
                while (len >= 0) {
                    ret.append(' ');
                    len--;
                }
            } else {
                while (len >= 0) {
                    ret.append(' ');
                    len--;
                }
                ret.append(str);
            }
            return ret.toString();
        }
        
    }
    
    /**
     *   get the formatted message (may be localized) from a log record.
     *   @param record  the log record
     *   @return the formatted message
     */
    private String getMessage(final LogRecord record) {
        String message = null;
        
        if (record.getResourceBundle() != null) {
            try {
                message = record.getMessage();
                message = record.getResourceBundle().getString(message);
            } catch (MissingResourceException mre) {
                message = record.getMessage();
            }
        }
        
        if (message == null) {
            message = record.getMessage();
        }
        
        if (message != null && record.getParameters() != null) {
            message = MessageFormat.format(message, record.getParameters());
        }
        return message;
    }
    
    /**
     *   format a log Record.
     *   <p><b>Attention:  This method is not thread safe</b></p>
     *   @see java.util.logging.Formatter#format
     */
    public final String format(final LogRecord record) {
        
        if (this.mf == null) {
            init();
        }
        
        sw.getBuffer().setLength(0);
        
        int argIndex = 0;
        
        for (int i = 0; i < columns.length; i++) {
            
            switch(columns[i]) {
                case COL_TIME:
                    date.setTime(record.getMillis());
                    argIndex++;
                    break;
                case COL_NAME:
                case COL_HOST:
                    continue;
                case COL_SOURCE:
                    args[argIndex] = getFixStr(record.getSourceClassName()
                    + '.' + record.getSourceMethodName() ,
                            DEFAULT_SOURCE_COL_WIDTH, true);
                    argIndex++;
                    break;
                case COL_MESSAGE:
                    args[argIndex] = getMessage(record);
                    argIndex++;
                    break;
                case COL_LEVEL:
                    args[argIndex] = levelToStr(record.getLevel());
                    argIndex++;
                    break;
                case COL_THREAD:
                    args[argIndex] = Integer.toString(record.getThreadID());
                    argIndex++;
                    break;
                case COL_LEVEL_LONG:
                    args[argIndex] = record.getLevel();
                    argIndex++;
                    break;
                default:
                    throw new IllegalStateException("unknown column "
                            + columns[i]);
            }
            
        }
        
        String message = mf.format(args);
        
        pw.println(message);
        
        
        if (withStackTraceFlag && record.getThrown() != null) {
            Throwable th = record.getThrown();
            
            StackTraceElement [] stack = null;
            
            boolean first = true;
            int len = 0;
            while (th != null) {
                pw.print(thrownPrefix);
                if (first) {
                    first = false;
                } else {
                    pw.print("Caused by ");
                }
                
                String thMessage = th.getLocalizedMessage();
                
                if( thMessage == null ) {
                    thMessage = th.getMessage();
                }
                
                pw.print(th.getClass().getName());
                pw.print(": ");
                pw.println(thMessage);
                stack = th.getStackTrace();
                len = Math.min(MAX_STACKTRACE_LEN, stack.length);
                for (int i = 0; i < len; i++) {
                    pw.print(thrownPrefix);
                    pw.print("  ");
                    pw.println(stack[i].toString());
                }
                th = th.getCause();
            }
        }
        pw.flush();
        return sw.getBuffer().toString();
    }
    
    /** maps Level object to its string representation. */
    private static HashMap levelMap = new HashMap();
    
    /** initializes the level map. */
    static {
        levelMap.put(Level.CONFIG, "C");
        levelMap.put(Level.INFO  , "I");
        levelMap.put(Level.SEVERE, "E");
        levelMap.put(Level.WARNING, "W");
        levelMap.put(Level.FINE   , "D");
        levelMap.put(Level.FINER  , "D");
        levelMap.put(Level.FINEST , "D");
    }
    
    /**
     *  get the string representation of a Level.
     *
     *  @param level  the level
     *  @return the string representation
     */
    private static String levelToStr(final Level level) {
        String ret = (String) levelMap.get(level);
        if (ret == null) {
            return "U";
        } else {
            return ret;
        }
    }
}
