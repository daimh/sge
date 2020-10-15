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
package com.sun.grid.jgdi.util.shell;

import com.sun.grid.jgdi.JGDIException;
import java.io.PrintWriter;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;


/**
 * OptionInfo class holds information about the command option
 */
public class OptionInfo {
    private List<List<String>> args=new ArrayList<List<String>>();
    private OptionDescriptor od=null;
    private Map<String, OptionDescriptor> map=null;
    private boolean optionDone=false;
    
    /**
     * Constructs OptionInfo class
     * @param od
     * @param args List<List<String>> of arguments for the current option
     * @param map
     */
    public OptionInfo(OptionDescriptor od, List<List<String>> args, Map<String, OptionDescriptor> map) {
        this.od = od;
        this.args.addAll(args);
        this.map = map;
    }
    
    /**
     * Constructs OptionInfo class
     * @param od tho option descriptor class
     * @param map a map of
     */
    public OptionInfo(OptionDescriptor od, Map<String, OptionDescriptor> map) {
        this(od, new ArrayList<List<String>>(), map);
    }
    
    /**
     * Invokes the method for current option in a loop until all argumetns
     * are processed.
     * NOTE: Use getFirstArg() and optionDone() to avoid infinite loop.
     * @see OptionInfo#getFirstArg() OptionInfo#optionDone() {@link OptionInfo}
     * @throws java.lang.Exception
     */
    public void invokeOption(AnnotatedCommand command) throws Exception {
        //Invoke it
        do {
            try {
                final Method method = getMethod();
                List<OptionInfo> objList = new ArrayList<OptionInfo>();
                objList.add(this);
                method.invoke(command, objList.toArray());
            } catch (Exception ex) {
                Throwable cause = ex.getCause();
                if (cause != null && cause instanceof JGDIException) {
                    PrintWriter pw = od.getErr();
                    String[] elems = ((JGDIException)cause).getMessage().split("\n");
                    String msg = "";
                    for (String elem : elems) {
                        if (elem.trim().length() > 0) {
                            msg += elem + "\n";
                        }
                    }
                    pw.print(msg);
                    pw.flush();
                    command.setExitCode(((JGDIException) cause).getExitCode());
                    continue;
                }
                // Rethrow the generated Exception, end the option execution if not JGDIException
                optionDone();
                if (cause != null && cause instanceof Exception) {
                    throw (Exception) cause;
                } else {
                    throw ex;
                }
            }
        } while (!isOptionDone() && getOd().isMultiple());
    }
    
    /**
     * Gets the arguments. developer is responsible for calling optionDone() method to avoid infinity loop.
     * @return {@link List} of option arguments
     */
    public List<String> getArgs() {
        List<String> ret = new ArrayList<String>();
        for (List<String> temp : args) {
            for (String arg : temp) {
                ret.add(arg);
            }
        }
        return ret;
    }
    
    /**
     * Gets the arguments
     * @return List<List<String>> of original option arguments (comma separated lists are sub lists)
     */
    public List<List<String>> getOriginalArgs() {
        List<List<String>> ret = new ArrayList<List<String>>(args);
        optionDone();
        return ret;
    }
    
    /**
     * Gets the arguments
     * @return {@link List} of option arguments
     */
    public String getArgsAsString() {
        if (args.size() == 0) {
            return "";
        }
        StringBuilder sb = new StringBuilder();
        for (List<String> temp : args) {
            for(String arg : temp){
                sb.append(arg);
                sb.append(",");
            }
        }
        // Return string without the last comma
        optionDone();
        return sb.deleteCharAt(sb.length()-1).toString();
    }
    /**
     * Getter method
     * @return value
     */
    public Method getMethod() {
        return getOd().getMethod();
    }
    
    /**
     * Getter method
     * @return value
     */
    public OptionDescriptor getOd() {
        return od;
    }
    
    /**
     * Contract method.
     * Terminates option loop. Should be called at the at of every option.
     * @see #getFirstArg()
     */
    public void optionDone(){
        optionDone=true;
    }
    
    /**
     * Contract method.
     * Developer should use it to retrieve option arguments one-by-one. So the
     * option loop is terminated.
     * @return String - first argument of the option or null if no arg
     */
    public String getFirstArg() {
        if (args == null || args.size() == 0) {
            return null;
        }
        List<String> subList = args.remove(0);
        final String ret = subList.remove(0);
        if (subList.size() > 0) {
            args.add(0, subList);
        }
        if (args.isEmpty()) {
            optionDone();
        }
        return ret;
    }
    
    /**
     * Contract method.
     * Developer should use it to retreave last comma separated argument.
     * @return List<String> - last comma separated argument
     */
    public List<String> getLastOriginalArgList() {
        if (args == null || args.size() == 0) {
            return null;
        }
        final List<String> subList = args.remove(args.size() - 1);
        if (args.isEmpty()) {
            optionDone();
        }
        return subList;
    }
    
    /**
     * Contract method.
     * Developer should use it to retreave look at the last argument. So the
     * option loop is terminated.
     * @return String - last argument of the option or null if no arg
     */
    public String showLastArg() {
        if (args == null || args.size() == 0) {
            return null;
        }
        List<String> subList = args.get(args.size() - 1);
        final String ret = subList.get(subList.size() -1);
        //TODO LP: Check if we need to end loop if we looked at the last arg
        if (args.size() == 1 && subList.size() == 1) {
            optionDone();
        }
        return ret;
    }
    
    /**
     * Getter method
     * @return value
     */
    public Map<String, OptionDescriptor> getMap() {
        return map;
    }
    
    public boolean isOptionDone() {
        return optionDone;
    }
}
