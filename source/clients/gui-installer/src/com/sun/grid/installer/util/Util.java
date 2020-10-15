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
package com.sun.grid.installer.util;

import com.izforge.izpack.installer.InstallerFrame;
import com.izforge.izpack.rules.RulesEngine;
import com.izforge.izpack.util.Debug;
import com.izforge.izpack.util.OsVersion;
import com.izforge.izpack.util.VariableSubstitutor;
import com.sun.grid.installer.util.cmd.FsTypeCommand;
import com.sun.grid.installer.gui.*;
import com.sun.grid.installer.util.cmd.SimpleLocalCommand;
import java.awt.Component;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Properties;
import java.util.Scanner;
import javax.swing.AbstractButton;
import javax.swing.JFileChooser;
import javax.swing.SwingUtilities;

/*RFC 952 DoD Internet host table specification defines a hostname as
 <hostname> ::= <name>*["."<name>]
 <name>  ::= <letter>[*[<letter-or-digit-or-hyphen>]<letter-or-digit>]
*/
public class Util implements Config{
    enum Operation {
        ADD, REMOVE, DELETE
    }

    public enum SgeComponents { bdb, qmaster, shadow, execd, admin, submit }

    public static int RESOLVE_THREAD_POOL_SIZE = 12;
    public static int INSTALL_THREAD_POOL_SIZE = 8;
    public static long DEF_RESOLVE_TIMEOUT = 20000;
    public static long DEF_INSTALL_TIMEOUT = 120000;
    public static String DEF_CONNECT_USER = "";
    public static boolean IS_MODE_WINDOWS = false;

    // Currently we accept pattern in list of hosts in a file
    public static List<String> parseFileList(File f) throws FileNotFoundException {
        List<String> hostList = new LinkedList<String>(), tempList;
        Host.Type type;
        Scanner s = new Scanner(f);
        s.useDelimiter("\\s+|;|,");
        String pattern = "";
        while (s.hasNext()) {
            pattern = s.next().toLowerCase().trim();
            type = isIpPattern(pattern) ? Host.Type.IP : Host.Type.HOSTNAME;
            tempList = parsePattern(pattern, type);
            hostList = joinList(hostList, tempList);
        }
        s.close();
        return hostList;
    }

    /**
     * Tests if valid characters for IP
     * @param pattern The patter to test
     * @return true if the pattern contains only characters for IP range. False otherwise.
     */
    public static boolean isIpPattern(String pattern) {
        for (int i=0 ; i < pattern.length() ;i++) {
            char c = pattern.charAt(i);
            switch (c) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '-':
                case '[':
                case ']':
                case '.':
                case ' ':
                    break;
                default:
                    return false;
            }
        }
        return true;
    }

    /**
     * Validates the host ids depending on it's type.
     * @param idList list of ids
     * @param type The {@link Host.Type} of the ids
     *
     * @throws java.lang.IllegalArgumentException if a validation fails
     */
    public static void validateHostIDList(List<String> idList, Host.Type type) throws IllegalArgumentException {
        for (String id : idList) {
            switch (type) {
                case HOSTNAME : {
                    if (id.equals("localhost")) {
                        throw new IllegalArgumentException("the 'localhost' loopback network interface name is not allowed!");
                    }
                    break;
                }
                case IP: {
                    if (id.startsWith("127")) {
                        throw new IllegalArgumentException("range of 127.x.x.x loopback network interface addresses are not allowed!");
                    }
                    break;
                }
                default: {
                    throw new IllegalArgumentException("unknown type '" + type + "'!");
                }
            }
        }
    }

    public static List<String> parseHostPattern(String input) throws IllegalArgumentException {
        //TODO: Might need some additional checks for all the hostnames
        return parsePattern(input.toLowerCase().trim(), Host.Type.HOSTNAME);
    }


    public static List<String> parseIpPattern(String input) throws IllegalArgumentException {
        return parsePattern(input.toLowerCase().trim(), Host.Type.IP);
    }


    public static List<String> parsePattern(String input, Host.Type type) throws IllegalArgumentException {
        long start=System.currentTimeMillis(),endL;
        LinkedList<List<String>> list = new LinkedList<List<String>>();
        int i=0;
        String elem;

        Scanner s = new Scanner(input);
        s.useDelimiter("\\.");
        while (s.hasNext()) {
            elem = s.next();
            i++;
            switch (type) {
                case HOSTNAME:
                    list.add(parseSinglePattern(elem, type));
                    break;
                case IP:
                    if (elem.charAt(0) >= '0' && elem.charAt(0) <= '9' || elem.charAt(0) == '[') {
                        List<String> singlePattern = parseSinglePattern(elem, type);
                        //Valid IP octed must be in 0 to 255
                        LinkedList<String> finalList = new LinkedList<String>();
                        int j;
                        String temp;
                        for (String val : singlePattern) {
                            j = Integer.valueOf(val).intValue();
                            if (j < 0 || j > 255) {
                                j = Math.max(0, j);
                                j = Math.min(255, j);
                                temp = String.valueOf(j);
                                if (!finalList.contains(temp)) {
                                    finalList.add(temp);
                                }
                            } else if (!finalList.contains(val)) {
                                finalList.add(val);
                            }
                        }
                        list.add(finalList);
                        break;
                    }                  
                default:
                    s.close();
                    throw new IllegalArgumentException("invalid value " + elem + " for type " + type.toString() + ".");
            }
        }
        s.close();

        //Ip must have 4 octects
        if (type == Host.Type.IP && i != 4) {
            throw new IllegalArgumentException("IP must have 4 octects! Got " + i + ".");
        }

        //Final reduction to single host list
        List<String> hostList = (LinkedList<String>) list.poll();
        while (!list.isEmpty()) {
            if (list.size() >= 1) {
                LinkedList end = (LinkedList<String>) list.poll();
                hostList = (LinkedList<String>) concatenateList(hostList, end, ".");
            } else {
                hostList = (LinkedList<String>) list.poll();
            }
        }
        endL = System.currentTimeMillis();
        Debug.trace("Generating "+hostList.size()+" took "+ (endL-start)+"ms ("+(endL-start)*100/hostList.size()+" per 100 items)");
        return hostList.size() != 0 ? hostList : null;
    }
    

    private  static List<String> parseSinglePattern(String input, Host.Type type) throws IllegalArgumentException {
        LinkedList list = new LinkedList(), start;
        LinkedList<String> item = new LinkedList();
        char c;
        int pos=0;
        int level=0;
        StringBuilder sb = new StringBuilder();
        while (pos != input.length()) {
            c = input.charAt(pos++);
            switch (type) {
                case HOSTNAME:
                    //We have a string
                    if (c >= 'a' && c <= 'z' || (c >= '0' && c <='9') || c == '_') {
                        sb.append(c);
                        continue;
                    }
                    break;
                case IP:
                    //We have a number
                    if (c >= '0' && c <= '9') {
                        sb.append(c);
                        continue;
                    }
                    break;
                default:
                    throw new IllegalArgumentException("unknown type '" + type + "'!");
            }
            //Something else
            switch (c) {
                case '[':
                    //Store previous element
                    if (sb.toString().trim().length() > 0) {
                       item.add(sb.toString().trim());
                       list.addFirst(item);
                    }
                    sb = new StringBuilder();
                    level++; //increase bracket level
                    //We support only single level of brackets
                    if (level > 1) {
                        throw new IllegalArgumentException("invalid character '" + c + "' in '" + input + "'!");
                    }
                    
                    //TODO We should reduce here what we can
                    /*if (item.size() == 0) {
                        if (list.peek().getClass() == String.class && ((String) list.peek()).equals("[")) {
                            item.add("");
                            list.addFirst(item);
                            list.addFirst("[");
                            item = new LinkedList<String>();
                            continue;
                        } else {
                            item = (LinkedList<String>) list.poll();
                        }
                    }
                    while (list.size() > 0 && !(list.peek().getClass() == String.class && ((String) list.peek()).equals("["))) {
                        if (list.peek().getClass() == String.class && ((String) list.peek()).equals("-")) {
                            list.poll();
                            //Special case test[,-A] => test,test-A ; test[-A, ] => test-A,test
                            if (list.peek().getClass() == String.class && (((String) list.peek()).equals(",") || ((String) list.peek()).equals("["))) {
                                item = (LinkedList<String>) concatenateList(Arrays.asList("-"), item);
                                list.poll();
                                start = (LinkedList<String>) list.poll();
                                item = (LinkedList<String>) joinList(start, item);
                                continue;
                            }
                            //Need to reduce the range to a list
                            start = (LinkedList<String>) list.poll();
                            item = (LinkedList<String>) generateRange(start, item, type);
                        } else if (list.peek().getClass() == String.class && ((String) list.peek()).equals(",")) {
                            //Need to reduce the range to a list
                            list.poll();
                            start = (LinkedList<String>) list.poll();
                            item = (LinkedList<String>) joinList(start, item);
                        } else {
                            start =(LinkedList<String>) list.poll();
                            item = (LinkedList<String>) concatenateList(start, item);
                        }
                    }*/
                    //
                    list.addFirst("[");                    
                    item = new LinkedList<String>();
                    continue;
                case ' ':
                case ',':
                    if (sb.toString().trim().length() > 0) {
                       item.add(sb.toString().trim());
                    }
                    sb = new StringBuilder();
                    if (item.size() == 0) {
                        if (list.size() == 0 || (list.peek().getClass() == String.class && ((String) list.peek()).equals(","))) {
                            continue; //Skip folowing whitespaces
                        } else if (list.peek().getClass() == String.class && ((String) list.peek()).equals("[")) {
                            item.add("");
                            list.addFirst(item);
                            list.addFirst(",");
                            item = new LinkedList<String>();
                            continue;
                        } else {
                            item = (LinkedList<String>) list.poll();
                        }
                    }
                    //Reduction to single list [3-5 6-9]
                    while (list.size() > 0 && !(list.peek().getClass() == String.class && ((String) list.peek()).equals("["))) {
                        if (list.peek().getClass() == String.class && ((String) list.peek()).equals("-")) {
                            //Need to reduce the range to a list
                            list.poll();
                            //Special case test[,-A] => test,test-A ; test[-A, ] => test-A,test
                            if (list.peek().getClass() == String.class && (((String) list.peek()).equals(",") || ((String) list.peek()).equals("["))) {
                                item = (LinkedList<String>) concatenateList(Arrays.asList("-"), item);
                                list.poll();
                                start = (LinkedList<String>) list.poll();
                                item = (LinkedList<String>) joinList(start, item);
                                continue;
                            }
                            start = (LinkedList<String>) list.poll();
                            item = (LinkedList<String>) generateRange(start, item, type);
                        } else if (list.peek().getClass() == String.class && ((String) list.peek()).equals(",")) {
                            //Need to reduce the range to a list
                            list.poll();
                            start = (LinkedList<String>) list.poll();
                            item = (LinkedList<String>) joinList(start, item);
                        } else {
                            start =(LinkedList<String>) list.poll();
                            item = (LinkedList<String>) concatenateList(start, item);
                        }
                    }
                    list.addFirst(item);
                    list.addFirst(",");
                    item = new LinkedList<String>();
                    continue;
                case '-':
                    //Add possible last element
                    if (sb.toString().trim().length() > 0) {
                       Integer num = null;
                       try {
                           num = Integer.parseInt(sb.toString().trim());
                       } catch (NumberFormatException ex) {
                           //Element is not a number!
                           sb.append(c);
                           continue;
                       }
                       if (type == Host.Type.HOSTNAME && level == 0 || num < 0) {
                           //Missing an opening bracket or do not have a valid number - not a range
                           sb.append(c);
                           continue;
                       }
                       item.add(sb.toString().trim());
                       list.addFirst(item);
                    } else if (type == Host.Type.HOSTNAME) {
                       if (level == 0) {
                          //Missing an opening bracket - not a range
                          sb.append(c);
                          continue;
                       } else {
                          //We have "[-" in hostname
                          throw new IllegalArgumentException("invalid character '" + c + "' in '" + input + "'!");
                       }
                    }
                    sb = new StringBuilder();
                    item = new LinkedList<String>();
                    list.addFirst("-");
                    continue;
                case ']':
                    level--;
                    //We support only single level of brackets
                    if (level < 0) {
                        throw new IllegalArgumentException("invalid character '" + c + "' in '" + input + "'!");
                    }
                    if (sb.toString().trim().length() > 0) {
                       item.add(sb.toString().trim());
                    }
                    sb = new StringBuilder();
                    if (item.size() == 0) {
                        if (list.peek().getClass() == String.class && ((String) list.peek()).equals(",")) {
                            item.add("");
                        //We got "[]"
                        } else if (list.peek().getClass() == String.class && ((String) list.peek()).equals("[")) {
                            throw new IllegalArgumentException("empty '[]' in '" + input + "'!");
                        //We got "-]"
                        } else if (list.peek().getClass() == String.class && ((String) list.peek()).equals("-")) {
                            throw new IllegalArgumentException("incomplete range in '" + input + "'!");
                        } else {
                            item = (LinkedList<String>) list.poll();
                        }
                    }
                    while (!(list.peek().getClass() == String.class && ((String) list.peek()).equals("["))) {
                        if (list.peek().getClass() == String.class && ((String) list.peek()).equals("-")) {                            
                            list.poll();
                            //Special case test[,-A] => test,test-A ; test[-A, ] => test-A,test
                            if (list.peek().getClass() == String.class && (((String) list.peek()).equals(",") || ((String) list.peek()).equals("["))) {
                                item = (LinkedList<String>) concatenateList(Arrays.asList("-"), item);
                                list.poll();
                                start = (LinkedList<String>) list.poll();
                                item = (LinkedList<String>) joinList(start, item);
                                continue;
                            }
                            //Need to reduce the range to a list
                            start = (LinkedList<String>) list.poll();
                            item = (LinkedList<String>) generateRange(start, item, type);
                        } else if (list.peek().getClass() == String.class && ((String) list.peek()).equals(",")) {
                            //Need to reduce the range to a list
                            list.poll();
                            start = (LinkedList<String>) list.poll();
                            item = (LinkedList<String>) joinList(start, item);
                        } else {
                            start =(LinkedList<String>) list.poll();
                            item = (LinkedList<String>) concatenateList(start, item);
                        }
                    }
                    list.poll();
                    list.addFirst(item);
                    item = new LinkedList<String>();
                    continue;
                default:
                    throw new IllegalArgumentException("invalid character '" + c + "' in '" + input + "'!");
            }
        }
        if (level > 0) {
            throw new IllegalArgumentException("uneven brackets in '" + input + "'!");
        }
        if (sb.toString().trim().length()>0) {
            if (item.size()>0) {
                list.addFirst(item);
            }
            item = new LinkedList<String>();
            item.addFirst(sb.toString().trim());
        }
        //Get first element if item is empty
        if (item.size() == 0) {
            if (list.peek().getClass() == String.class && ((String) list.peek()).equals(",")) {
                list.poll();
            }
            item = (LinkedList<String>) list.poll();
        }
        //Final reduction to single list
        while (!list.isEmpty()) {
            if (list.peek().getClass() == String.class && ((String) list.peek()).equals("-")) {
                //Need to reduce the range to a list
                list.poll();
                //Special case test[,-A] => test,test-A ; test[-A, ] => test-A,test
                if (list.peek().getClass() == String.class && (((String) list.peek()).equals(",") || ((String) list.peek()).equals("["))) {
                    item = (LinkedList<String>) concatenateList(Arrays.asList("-"), item);
                    list.poll();
                    start = (LinkedList<String>) list.poll();
                    item = (LinkedList<String>) joinList(start, item);
                    continue;
                }
                start = (LinkedList<String>) list.poll();
                item = (LinkedList<String>) generateRange(start, item, type);
            } else if (list.peek().getClass() == String.class && ((String) list.peek()).equals(",")) {
                //Need to reduce the range to a list
                list.poll();
                start = (LinkedList<String>) list.poll();
                item = (LinkedList<String>) joinList(start, item);
            } else if (list.size() >= 1) {
                start = (LinkedList<String>) list.poll();
                item = (LinkedList<String>) concatenateList(start, item);
            } else {
                item = (LinkedList<String>) list.poll();
            }
        }        
        return item;
    }

    private static List<String> generateRange(List<String> start, List<String> end, final Host.Type type) throws IllegalArgumentException {
        List<String> list = new LinkedList();
        String val;
        for (String s : start) {
            for (String e : end) {
                int start_int=0;
                int end_int=0;
                try {
                   start_int = Integer.parseInt(s);
                   end_int = Integer.parseInt(e);
                } catch (NumberFormatException ex) {
                    //Range is not valid
                    switch (type) {
                        case HOSTNAME:
                            //We should just concatenete
                            return concatenateList(start, end, "-");
                        default:
                            throw new IllegalArgumentException("invalid numeric IP range!");
                    }
                }
                //Small fix for invalid in IP ranges
                if (type == Host.Type.IP) {
                    start_int = Math.min(255, start_int);
                    start_int = Math.max(0, start_int);
                    end_int = Math.min(255, end_int);
                    end_int = Math.max(0, end_int);
                }
                boolean isIncreasing = start_int <= end_int;
                do {
                    val = String.valueOf(start_int);
                    //Add trailing zeros
                    while (val.length() < s.length()) {
                        val = "0"+val;
                    }
                    if (!list.contains(val)) {
                        list.add(val);
                    }
                    if (isIncreasing)
                        start_int++;
                    else
                        start_int--;
                } while ((isIncreasing) ? !(start_int > end_int) : !(start_int < end_int));
            }
        }
        return list.size()==0 ? null : list;
    }

    private static List<String> concatenateList(List<String> start, List<String> end) {
        return concatenateList(start, end, "");
    }

    private static List<String> concatenateList(List<String> start, List<String> end, String glue) {
        List<String> list = new LinkedList();
        for (String s : start) {
            for (String e : end) {
                list.add(new String(s+glue+e));
            }
        }
        return list.size()==0 ? null : list;
    }

    private static List<String> joinList(List<String> start, List<String> end) {
        List<String> list = new LinkedList();
        list.addAll(start);
        for (String val : end) {
            if (!list.contains(val)) {
                list.add(val);
            }
        }
        return list.size() == 0 ? null : list;
    }

    public static void saveListToFile(Component component, List<String> list) {
        final JFileChooser fc = new JFileChooser();
        int ret = fc.showSaveDialog(SwingUtilities.getRootPane(component).getContentPane());
        if (ret == JFileChooser.APPROVE_OPTION) {
            FileWriter fw = null;
            File f = null;
            try {
                f = fc.getSelectedFile();
                fw = new FileWriter(f);
                String separator = System.getProperty("line.separator");
                for (String s : list) {
                    fw.write(s+separator);
                }
            } catch (IOException ex) {
                Debug.error("Error: Save failed - " + ex.getMessage());
            } finally {
                try {
                    fw.close();
                    Debug.trace("List saved to " + f.getName());
                } catch (IOException ex) {
                    Debug.error("Error: Save failed - " + ex.getMessage());
                }
            }
        }
    }

    /**
     * Filles up a template file by substituting the variables names with their values
     * @param templateFilePath The template input file to fill up
     * @param resultFilePath The result output file
     * @param variables The variables and their values
     *
     * @throws java.lang.Exception
     *
     * @see Util#fillUpTemplate(java.lang.String, java.lang.String, java.util.Properties, java.lang.String[], boolean) 
     * @see VariableSubstitutor
     */
    public static String fillUpTemplate(String templateFilePath, String resultFilePath, Properties variables) throws Exception {
        return fillUpTemplate(templateFilePath, resultFilePath, variables, null, false);
    }


    /**
     * Filles up a template file by substituting the variables names with their values
     * @param templateFilePath The template input file to fill up
     * @param resultFilePath The result output file
     * @param variables The variables and their values
     * @param commentChars Array of special characters which specify a line as comment (e.g.: new String[]{\/**,*,**\/})
     * @param removeComments indicates whether the comments should be removed from the result.
     *
     * @throws java.lang.Exception
     *
     * @see VariableSubstitutor
     */
    public static String fillUpTemplate(String templateFilePath, String resultFilePath, Properties variables, String[] commentChars, boolean removeComments) throws Exception {
    	BufferedReader bufferedReader = null;
    	BufferedWriter bufferedWriter     = null;
        VariableSubstitutor vs = new VariableSubstitutor(variables);

        Debug.trace("Fill up template from '" + templateFilePath + "' to '" + resultFilePath + "'.");
        File f = new File(resultFilePath);

        try {
    	    bufferedReader = new BufferedReader(new FileReader(templateFilePath));
            bufferedWriter = new BufferedWriter(new FileWriter(f, false));

            String line = "";
            String formattedLine = "";

            boolean commentedLine = false;
            boolean skipLine = false;
            while ((line = bufferedReader.readLine()) != null) {
                line = vs.substituteMultiple(line, null);
                formattedLine = line.trim().toLowerCase();

                // if comment characters are specified....
                if (commentChars != null) {
                    commentedLine = false;

                    // check whether the current line is a comment
                    // TODO handle multiline comments
                    for (String comment : commentChars) {
                        if (formattedLine.startsWith(comment) || formattedLine.endsWith(comment)) {
                            commentedLine = true;
                            break;
                        }
                    }


                    if (commentedLine) {
                        // check whether the actual comment holds special commands
                        if (formattedLine.indexOf(".if") > -1) {
                            // Skip next line(s) if the condition is false
                            skipLine = !evaluateConditionalComment(formattedLine);
                        } else if (formattedLine.indexOf(".endif") > -1) {
                            // Write upcoming lines if the special block is over
                            skipLine = false;
                        }

                        // remove commented lines
                        if (removeComments) {
                            continue;
                        }
                    } else if (skipLine) {
                        continue;
                    }
                }

                bufferedWriter.write(line);
                bufferedWriter.newLine();
            }
    	} finally {
    		if (bufferedReader != null) {
    			bufferedReader.close();
    		}
    		if (bufferedWriter != null) {
    			bufferedWriter.close();
    		}
            return f.getAbsolutePath();
    	}
    }

    /**
     * Evaulates a conditional comment which follows the following rules:<br>
     *  - the condition starts with ".if"<br>
     *  - the condition ends with one "." character<br>
     *  - in the middle contains the condition should be evaulated. Can be string "true" or
     *    "false" or a condition id (with/without '!') which is managed by the {@link RulesEngine}.<br>
     * <br>
     * Example:<br>
     * -html: &lt!--.if cond.true.--&gt<br>
     * -script: #.if cond.true.
     *
     * @param comment A comment line to evaulate
     * @return the result of the evaulation or false in case of failure.
     */
    public static boolean evaluateConditionalComment(String comment) {
        Debug.trace("Evaulate conditional comment: " + comment);
        
        if (comment.indexOf(".if") == -1 || comment.equals("")) {
            return false;
        }

        try {
            String condition = comment.substring(comment.indexOf(".") + 3, comment.lastIndexOf(".")).trim();

            if (condition.equalsIgnoreCase("true")) {
                return true;
            } else if (condition.equalsIgnoreCase("false")) {
                return false;
            } else {
                return InstallerFrame.getRules().isConditionTrue(condition);
            }
        } catch (IndexOutOfBoundsException e) {
            return false;
        }
    }

    /**
     * Recursively searches for the parent InstallerFrame coponent
     * @param comp The Component to search in
     * @return The found InstallerFrame component if exists, null otherwise.
     *
     * @see InstallerFrame
     */
    public static Component findParentInstallerFrameComponent(Component comp) {
        Component parent = (Component)comp.getParent();

        if (parent instanceof InstallerFrame) {
            return parent;
        } else if (parent != null) {
            return findParentInstallerFrameComponent(parent);
        } else {
            return null;
        }
    }

    /**
     * Removes the domain from a FQDN host
     * @param fQDomainName The fully qualified domain name
     * @return The domain name if exists, otherwise an empty String
     */
    public static String getDomainName(String fQDomainName) {
        String domain = "";
        int firstDotIndex = fQDomainName.indexOf(".", 0);

        if (firstDotIndex > -1) {
            domain = fQDomainName.substring(firstDotIndex + 1);
        }
        return domain;
    }

    /**
     * Pings the host
     * @param sgeRoot The SGE_ROOT
     * @param host The host to be reached
     * @param port The port to check
     * @param component The type of the component to find on the given port: "qmaster" or "execd"
     * @return True only and only if the host is reachable on the given port
     */
    /*public static boolean pingHost(Properties variables, Host host, String component, int maxTries) {
        String qping = variables.getProperty(VAR_SGE_ROOT) + "/bin/" + Host.localHostArch + "/qping";
        String port = (component.equalsIgnoreCase("qmaster")) ? variables.getProperty(VAR_SGE_QMASTER_PORT) : (component.equalsIgnoreCase("execd")) ? variables.getProperty(VAR_SGE_EXECD_PORT) : "-1";
        try {
            CommandExecutor cmdExec = null;
            int tries = 0;
            while (tries < maxTries) {
                // TODO does -tcp option is what we need?
                cmdExec = new CommandExecutor(qping, "-tcp", "-info", host.getHostname(), port, component, "1");
                Map env = cmdExec.getEnvironment();
                env.put("SGE_ROOT", variables.getProperty(VAR_SGE_ROOT));
                env.put("SGE_CELL", variables.getProperty(VAR_SGE_CELL_NAME));
                env.put("SGE_QMASTER_PORT", variables.getProperty(VAR_SGE_QMASTER_PORT));
                env.put("SGE_EXECD_PORT", variables.getProperty(VAR_SGE_EXECD_PORT));
                cmdExec.execute();

                if (cmdExec.getExitValue() == 0) {
                    return true;
                } else {
                    Thread.sleep(2000);
                }

                tries++;
            }

            Debug.trace("Tried to ping host " + tries + " time(s).");
        } catch (Exception e) {
            Debug.error(e);
        }

        return false;
    }*/

    /**
     * Returns with the file system type of the given directory. If the directory does not exist first creates it
     * then after the check deletes it immediately.
     * @param dir The directory path to be checked
     * @return The FS type of the given directory if the check was successful, otherwise empty string.
     */
    public static String getDirFSType(String shell, String sge_root, String dir) {
       return getDirFSType(Host.localHostName, Host.localHostArch, shell, sge_root, dir);
    }

    public static String getDirFSType(String host, String arch, String shell, String sge_root, String dir) {    
        String result = "";

        ExtendedFile file = new ExtendedFile(dir).getFirstExistingParent();
        Debug.trace("First existing parent of '" + dir + "' is '" + file.getAbsolutePath() +"'.");
        dir = file.getAbsolutePath();

        try {
            // Call the 'fstype' script of the proper architecture
            String fstypeScript = sge_root + "/utilbin/" + arch + "/fstype";
            FsTypeCommand fstypeCmd = new FsTypeCommand(host, DEF_CONNECT_USER, shell, IS_MODE_WINDOWS, fstypeScript, dir);
            fstypeCmd.execute();

            if (fstypeCmd.getExitValue() == EXIT_VAL_SUCCESS) {
                result = fstypeCmd.getOutput().firstElement().trim();
                Debug.trace("FSType of '" + dir + "' is '" + result +"'.");
            } else {
                Debug.error("Failed to get the FSType of the directory '" + dir + "'! Error: " + fstypeCmd.getError());
            }
        } catch (Exception e) {
            Debug.error(e);
        }

        return result;
    }

    public static String[] getUserGroups(String shell, String userToCheck) {
        return getUserGroups(Host.localHostName, shell, userToCheck);
    }

    /**
     * Returns the group id of the user.
     * @param userToCheck The user name
     * @return The group id of the user if the process was successful, otherwise empty string.
     */
    public static String[] getUserGroups(String host, String shell, String userToCheck) {
        String[] groups = null;
        ExtendedFile tmpFile = null;

        try {
            String command = "groups";
            FsTypeCommand groupCmd = new FsTypeCommand(host, DEF_CONNECT_USER, shell, IS_MODE_WINDOWS, command,  userToCheck);
            groupCmd.execute();

            if (groupCmd.getExitValue() == EXIT_VAL_SUCCESS) {
                groups = groupCmd.getOutput().firstElement().trim().split(" ");

                Debug.trace("Group of user '" + userToCheck + "' are '" + Arrays.toString(groups) + "'.");
            } else {
                Debug.error("Failed to get the group id's of user '" + userToCheck + "'! Error: " + groupCmd.getError());
            }
        } catch (Exception ex) {
            Debug.error("Failed to get the group id's of user '" + userToCheck + "'! " + ex);
        } 

        return groups;
    }

    /**
     * Check whether the executor user is superuser.
     * @param sgeRoot The SGE_ROOT
     * @param architecture The architecture to be used
     * @return true if the user has the id of '0', false otherwise.
     */
    public static boolean isRootUser(String sgeRoot, String architecture) {
        String userId = getUserId(sgeRoot, architecture);

        return userId.equals("0");
    }

    /**
     * Returns with the user id of the executor user
     * @param sgeRoot The SGE_ROOT
     * @param architecture The architecure of the local host
     * @return The id of the executor user
     */
    public static String getUserId(String sgeRoot, String architecture) {
        String userId = "";

        try {
            String command = sgeRoot + "/" + architecture + "/uidgid";
            SimpleLocalCommand cmd = new SimpleLocalCommand(command, "-euid");
            cmd.execute();

            if (cmd.getExitValue() == EXIT_VAL_SUCCESS) {
                userId = cmd.getOutput().firstElement().trim();

                Debug.trace("Id of the executor user is '" + userId + "'.");
            } else {
                Debug.error("Failed to get the user id of executor user! Error: " + cmd.getError());
            }
        } catch (Exception e) {
            Debug.error("Failed to get the user id of executor user! " + e);
        }

        return userId;
    }

    /**
     * Sources the sge <SGE_ROOT>/<CELL_NAME>/common/settings.sh file.
     * @param sgeRoot The SGE_ROOT directory
     * @param cellName The CELL_NAME value
     * @return List of key values pairs sourced from the file.
     * 
     * @throws java.io.FileNotFoundException
     * @throws java.io.IOException
     */
    public static Properties sourceSGESettings(String sgeRoot, String cellName) throws FileNotFoundException, IOException {
        Properties settings = new Properties();
        String settingsshFile = sgeRoot + "/" + cellName + "/common/settings.sh";
        ArrayList<String> settingsshLines = FileHandler.readFileContent(settingsshFile, false);

        for (String line : settingsshLines) {
            // Process lines like 'SGE_CELL=default; export SGE_CELL'
            if (line.startsWith("SGE_")) {
                String[] keyValuePair = line.split(";")[0].split("=");
                settings.setProperty(keyValuePair[0].trim(), keyValuePair[1].trim());
            }
        }

        Debug.trace("Sourced settings from '" + settingsshFile + "' are " + settings + ".");

        return settings;
    }


    /**
     * Sources the sge <SGE_ROOT>/<CELL_NAME>/common/jmx/management.properties file.
     * @param sgeRoot The SGE_ROOT directory
     * @param cellName The CELL_NAME value
     * @return List of key values pairs sourced from the file.
     *
     * @throws java.io.FileNotFoundException
     * @throws java.io.IOException
     */
    public static Properties sourceJMXSettings(String sgeRoot, String cellName) throws FileNotFoundException, IOException {
        Properties settings = new Properties();
        String settingsJMXFile = sgeRoot + "/" + cellName + "/common/jmx/management.properties";
        ArrayList<String> settingsJMXLines = FileHandler.readFileContent(settingsJMXFile, false);

        String key, value;
        for (String line : settingsJMXLines) {
            // Skip comments
            if (line.startsWith("#") || !line.contains("=")) {
                continue;
            }
            // Process lines like 'com.sun.grid.jgdi.management.jmxremote.ssl=true'
            int spaceIndex = line.indexOf('=');
            key = line.substring(0, spaceIndex).trim();
            value = line.substring(spaceIndex + 1).trim();

            //finally set the key, value pair
            settings.setProperty(key, value);
        }

        Debug.trace("Sourced settings from '" + settingsJMXFile + "' are " + settings + ".");
        return settings;
    }

    /**
     * Sources the sge <SGE_ROOT>/<CELL_NAME>/common/sboostrap file.
     * @param sgeRoot The SGE_ROOT directory
     * @param cellName The CELL_NAME value
     * @return List of key values pairs sourced from the file.
     *
     * @throws java.io.FileNotFoundException
     * @throws java.io.IOException
     */
    public static Properties sourceSGEBootstrap(String sgeRoot, String cellName) throws FileNotFoundException, IOException {
        Properties settings = new Properties();
        String boostrapFile = sgeRoot + "/" + cellName + "/common/bootstrap";
        ArrayList<String> bootstrapLines = FileHandler.readFileContent(boostrapFile, false);
        String key, value;
        boolean isClassicSpooling = false;
        for (String line : bootstrapLines) {
            // Process lines like 'spooling_method         berkeleydb'
            if (!line.startsWith("#")) {
                int spaceIndex = line.indexOf(' ');
                key = line.substring(0, spaceIndex).trim();
                value = line.substring(spaceIndex).trim();
                //PRODUCT_MODE
                if (key.equalsIgnoreCase("security_mode")) {
                    if (value.equalsIgnoreCase("none")) {
                        key="add.product.mode";
                        value = "none";
                    } else if (value.equalsIgnoreCase("csp")) {
                        key="add.product.mode";
                        value = "csp";
                    } //Don't support afs => 3
                //HOSTNAME_RESOLVING
                } else if (key.equalsIgnoreCase("ignore_fqdn")) {
                    key = "hostname_resolving";
                } else if (key.equalsIgnoreCase("spooling_method")) {
                    if (value.trim().equalsIgnoreCase("classic")) {
                        isClassicSpooling = true;
                    }
                }
                //finally set the key, value pair
                settings.setProperty(key, value);
                //TODO: Check we have ignore_fqdn, spool, etc/ properly mapped to GUI cfg.* variables!                
            }
        }

        Debug.trace("Sourced settings from '" + boostrapFile + "' are " + settings + ".");

        return settings;
    }

    /**
     * Sources the <SGE_ROOT>/<CELL_NAME>/common/act_qmaster file and return with the name of qmaster host.
     * @param sgeRoot The SGE_ROOT directory.
     * @param cellName The CELL_NAME value
     * @return The name of the qmaster host.
     * @throws java.io.FileNotFoundException
     * @throws java.io.IOException
     */
    public static String getQmasterHost(String sgeRoot, String cellName) throws FileNotFoundException, IOException {
        String qmasterHost = "";
        String actQmasterFile = sgeRoot + "/" + cellName + "/common/act_qmaster";
        ArrayList<String> settingsshLines = FileHandler.readFileContent(actQmasterFile, false);

        if (settingsshLines.size() > 0) {
            qmasterHost = settingsshLines.get(0).trim();
        }

        Debug.trace("Found qmaster host name in '" + actQmasterFile + "' is '" + qmasterHost + "'.");

        return qmasterHost;
    }

    /**
     * Checks whether the given port is free on the specified host
     *
     * NOTE: works properly only on local host
     *
     * @param hostName The host where the port should be checked
     * @param port The port to be checked
     * @return True only if the host is reachable and the port is free to bind.
     */
    public static boolean isPortFree(String hostName, String port) {
        try {
            return isPortFree(hostName, Integer.parseInt(port));
        } catch (Exception e) {
            return false;
        }
    }

    /**
     * Checks whether the given port is free on the specified host
     *
     * NOTE: works properly only on local host
     *
     * @param hostName The host where the port should be checked
     * @param port The port to be checked
     * @return True only if the host is reachable and the port is free to bind.
     */
    public static boolean isPortFree(String hostName, int port) {
        try {
            ServerSocket serverSocket = new ServerSocket(port, 0 , InetAddress.getByName(hostName));
            serverSocket.close();
            return true;
        } catch (IOException e) {
            //e.printStackTrace();
            return false;
        }
    }

    public static List<String> getAllHosts(HostInstallTableModel model, String local) {
        String item ="";
        List<String> tmp = new ArrayList<String>();
        for (int i = 0; i< model.getRowCount(); i++) {
            item = (String) model.getValueAt(i, 1);
            //We don't add duplicate hostnames, local host or special tasks "All hosts"
            if (!tmp.contains(item) && !item.equals(local) && !item.equals(Host.HOST_TYPE_ALL)) {
                tmp.add(item);
            }
        }
        if (local.length() > 0 ) {
            tmp.add(local);
        }
        return tmp;
    }

    public static String listToString(List<String> list) {
        String val = "";
        for (String s: list) {
            val+= s + " ";
        }
        return val.trim();
    }

    public static String generateTimeStamp() {
        return new SimpleDateFormat("yyyy.MM.dd_HH:mm:ss").format(new Date());
    }

        /**
     * Removes the listeners from the given {@link AbstractButton}
     * @param button The button which listeners have to be removed
     * @return The removed listeners
     */
    public static ActionListener[] removeListeners(AbstractButton button) {
        ActionListener[] listeners = button.getActionListeners();

        for (int i = 0; i < listeners.length; i++) {
            button.removeActionListener(listeners[i]);
        }

        return listeners;
    }

    /**
     * Returns all of the host from the list in the given type.
     * @param type The type of the hosts should return.
     * @return All of the hosts with the given type (list is unique)
     *
     * @see Host
     */
    public static ArrayList<Host> getHosts(HostList hostList, SgeComponents type) {
        ArrayList<Host> result = new ArrayList<Host>();
        ArrayList<String> hostnames = new ArrayList<String>();

        for (Host host : hostList) {
            //We need a unique list of Hosts of single type
            if (hostnames.contains(host.getHostname())) {
                continue;
            }

            switch (type) {
                case qmaster: {
                    if (host.isQmasterHost()) break;
                    else continue;
                }
                case execd: {
                    if (host.isExecutionHost()) break;
                    else continue;
                }
                case shadow: {
                    if (host.isShadowHost()) break;
                    else continue;
                }
                case bdb: {
                    if (host.isBdbHost()) break;
                    else continue;
                }
                case admin: {
                    if (host.isAdminHost()) break;
                    else continue;
                }
                case submit: {
                    if (host.isSubmitHost()) break;
                    else continue;
                }
                default: throw new IllegalArgumentException("Unknown sge component: " + type);
            }

            result.add(host);
            hostnames.add(host.getHostname());
        }

        return result;
    }

    /**
     * Creates a string from the host's name
     * @param hosts The host which names should be appended
     * @param separator The separator string between the host names
     * @return The string from the host names
     */
    public static String getHostNames(ArrayList<Host> hosts, String separator) {
        return getHostNames(hosts, null, separator);
    }

    /**
     * Creates a string from the host's name
     * @param hosts The host which names should be appended
     * @param additionalHostnames The hostnames to be appendedt
     * @param separator The separator string between the host names
     * @return The string from the host names
     */
    public static String getHostNames(ArrayList<Host> hosts, List<String> additionalHostnames, String separator) {
        StringBuffer result = new StringBuffer();
        List<String> finalList = new ArrayList<String>();

        for (Host h : hosts) {
            finalList.add(h.getHostname());
        }

        if (additionalHostnames != null) {
            for (String hostname : additionalHostnames) {
                if (!finalList.contains(hostname)) {
                    finalList.add(hostname);
                }
            }
        }
        //Sort the list
        Collections.sort(finalList);

        //Create a string out of the list
        for (int i = 0; i < finalList.size(); i++) {
            result.append(finalList.get(i));
            // do not append separator to the end of the elements
            if (i + 1 != hosts.size()) {
                result.append(separator);
            }
        }

        return result.toString();
    }

    /**
     * Opens a browser.
     * @param url The url to open in the browser
     * @return true only if the a browser was found and the startup was successful. False otherwise.
     */
    public static boolean openBrowser(String url) {
        SimpleLocalCommand command = null;
        if (OsVersion.IS_OSX && OsVersion.IS_MAC) {
            command = new SimpleLocalCommand("open " + url);
            command.execute();
            if (command.getExitValue() == 0) {
                return true;
            }
        } else if (OsVersion.IS_WINDOWS) {
            command = new SimpleLocalCommand("cmd /C start " + url);
            command.execute();
            if (command.getExitValue() == 0) {
                return true;
            }
        } else {
            String[] browsers = new String[]{"firefox", "opera", "mozilla", "netscape", 
            "htmlview", "xdg-open", "gnome-open", "kfmclient openURL", "call-browser",
            "konqueror", "epiphany"};

            for (String browser : browsers) {
                command = new SimpleLocalCommand(3000, browser, url);
                command.execute();
                if (command.getExitValue() == 0) {
                    return true;
                }
            }

        }

        return false;
    }

    /*public static boolean runAsAdminUser(String host, String arch, String sgeRoot, String adminUser, String command, Properties variables) throws Exception {
        CommandExecutor cmdExec = null;

        Debug.trace("Run '" + command + "' as '" + adminUser + "' user.");

        String scriptAdminRun = sgeRoot + "/utilbin/" + arch + "/adminrun";
        cmdExec = new CommandExecutor(variables, variables.getProperty(VAR_SHELL_NAME), host, scriptAdminRun, adminUser, command);
        cmdExec.execute();

        if (cmdExec.getExitValue() == 0) {
            return true;
        } else {
            return false;
        }
    }*/
}
