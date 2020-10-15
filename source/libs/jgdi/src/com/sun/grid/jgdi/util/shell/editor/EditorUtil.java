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

package com.sun.grid.jgdi.util.shell.editor;

import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.configuration.ClusterQueue;
import com.sun.grid.jgdi.configuration.ComplexEntry;
import com.sun.grid.jgdi.configuration.ComplexEntryImpl;
import com.sun.grid.jgdi.configuration.Configuration;
import com.sun.grid.jgdi.configuration.GEObject;
import com.sun.grid.jgdi.configuration.Project;
import com.sun.grid.jgdi.configuration.SchedConf;
import com.sun.grid.jgdi.configuration.User;
import com.sun.grid.jgdi.configuration.UserSet;
import com.sun.grid.jgdi.configuration.UserSetImpl;
import com.sun.grid.jgdi.configuration.reflect.DefaultListPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.DefaultMapListPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.DefaultMapPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.PropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.SimplePropertyDescriptor;
import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import java.util.Set;

/**
 *
 */
public class EditorUtil {
    public static final String VALUE_NONE = "NONE";
    public static final String VALUE_UNDEFINED = "UNDEFINED";
    public static final String VALUE_INFINITY = "INFINITY";
    public static final String VALUE_DEFAULT = "@/";
    
    public static final String QTYPES[] = { "BATCH", "INTERACTIVE" };
    public static final String QSMTYPES[] = { "load", "seqno" };
    private static final String userset_types[] = {
        "ACL",   /* US_ACL   */
        "DEPT",  /* US_DEPT  */
    };
    
    public static final int PROPERTIES_ALL = 0;
    public static final int PROPERTIES_READ_ONLY = 1;
    public static final int PROPERTIES_CONFIGURABLE = 2;
    
    /* Used only for attribute mapping to the C and Java atrribute names that are not the same */
    private static final ResourceBundle resource = ResourceBundle.getBundle("com.sun.grid.jgdi.util.shell.editor.EditorResources");
    /* Used only for attribute mapping to the C and Java atrribute names that are not the same */
    static Map<String, Map<String, String>> conv = null;
    
    /** Convert JGDI name to equivalent in the clients */
    public static String unifyAttrWithClientNames(String objectName, String name) {
        String val=null;
        if (conv == null) {
            conv = new HashMap<String, Map<String, String>>();
            Enumeration<String> t = resource.getKeys();
            while (t.hasMoreElements()) {
                String[] res = t.nextElement().split("[.]");
                unifyAttrWithClientNames(res[0], res[1]);
            }
        }
        Map<String, String> tr = conv.get(objectName);
        if (tr != null) {
            val = tr.get(name);
        }
        if (val == null) {
            try {
                val = resource.getString(objectName+"."+name);
                if (tr == null) {
                    tr=new HashMap<String, String>();
                    conv.put(objectName, tr);
                }
                tr.put(name, val);
            } catch (MissingResourceException ex) {
                return name;//java2cName(obj, name) name;
            }
        }
        return val;
    }
    
    static String unifyAttrWithClientNames(GEObject obj, String name) {
        String objectName = obj.getClass().getName();
        String prefix = "com.sun.grid.jgdi.configuration.";
        String suffix = "Impl";
        if (objectName.startsWith(prefix)) {
            objectName = objectName.substring(prefix.length(), objectName.length());
        }
        if (objectName.endsWith(suffix)) {
            objectName = objectName.substring(0, objectName.length()-suffix.length());
        }
        return unifyAttrWithClientNames(objectName, name);
    }
    
    /** Convert client name to equivalent name in JGDI */
    static String unifyClientNamesWithAttr(String objectName, String name) {
        if (conv == null) {
            conv = new HashMap<String, Map<String, String>>();
            Enumeration<String> t = resource.getKeys();
            while (t.hasMoreElements()) {
                String[] res = t.nextElement().split("[.]");
                unifyAttrWithClientNames(res[0], res[1]);
            }
        }
        Map<String, String> tr = conv.get(objectName);
        if (tr == null) {
            return name;
        }
        for (Map.Entry<String, String> e : tr.entrySet()) {
            if (e.getValue().equals(name)) {
                return e.getKey();
            }
        }
        return name;
    }
    
    static String unifyClientNamesWithAttr(GEObject obj, String name) {
        String objectName = obj.getClass().getName();
        String prefix = "com.sun.grid.jgdi.configuration.";
        String suffix = "Impl";
        if (objectName.startsWith(prefix)) {
            objectName = objectName.substring(prefix.length(), objectName.length());
        }
        if (objectName.endsWith(suffix)) {
            objectName = objectName.substring(0, objectName.length()-suffix.length());
        }
        return unifyClientNamesWithAttr(objectName, name);
    }
    
    static String c2javaName(GEObject obj, String cName) {
        //Unify with inconsistencies in clients
        String name = unifyClientNamesWithAttr(obj, cName);
        if (!name.equals(cName)) {
            return name;
        }
        StringBuilder sb = new StringBuilder();
        String[] parts = cName.toLowerCase().split("_");
        for (int i=0; i<parts.length; i++) {
            sb.append(Character.toUpperCase(parts[i].charAt(0)));
            sb.append(parts[i].substring(1));
        }
        char c = Character.toLowerCase(sb.charAt(0));
        sb.deleteCharAt(0);
        sb.insert(0,c);
        return sb.toString();
    }
    
    static String java2cName(GEObject obj, String javaName) {
        //Unify with inconsistencies in clients
        String name = unifyAttrWithClientNames(obj, javaName);
        if (!name.equals(javaName)) {
            return name;
        }
        StringBuilder sb = new StringBuilder(javaName);
        char c;
        for (int i = 0; i<sb.length(); i++) {
            c = sb.charAt(i);
            if (Character.isUpperCase(c)) {
                sb.deleteCharAt(i);
                sb.insert(i,"_"+Character.toLowerCase(c));
                i++;
            }
        }
        return sb.toString();
    }
    
    static boolean doNotDisplayAttr(GEObject obj, PropertyDescriptor pd, int propScope) {
        switch (propScope) {
            case PROPERTIES_ALL:
                return doNotDisplayAttrAll(obj, pd);
            case PROPERTIES_CONFIGURABLE:
                return doNotDisplayConfigurableAttr(obj, pd);
            case PROPERTIES_READ_ONLY:
                return false;
            default:
                throw new IllegalArgumentException("Invalid property scope specifier!");
        }
    }
    
    /**
     * Filters GEObject attributes so only those that are displayed by client
     * are also displayed by JGDIShell (Used in qconf -s*)
     */
    static boolean doNotDisplayAttrAll(GEObject obj, PropertyDescriptor pd) {
        String name = java2cName(obj, pd.getPropertyName());
        //USER
        if (obj instanceof User) {
            if (name.equals("acl") || name.equals("xacl")) {
                return true;
            }
        }
        //USERSET
        if (obj instanceof UserSet) {
            if (name.equals("job_cnt")) {
                return true;
            }
        }
        //PROJECT
        if (obj instanceof Project) {
            if (name.equals("delete_time") || name.equals("default_project")) {
                return true;
            }
        }
        //QUEUE
        if (obj instanceof ClusterQueue) {
            if (name.equals("qinstances") || name.equals("tag")) {
                return true;
            }
        }
        //CONFIGURATION
        if (obj instanceof Configuration) {
            if (name.equals("version")) {
                return true;
            }
        }
        //SCHEDCONF
        if (obj instanceof SchedConf) {
            if (name.equals("weight_tickets_override")) {
                return true;
            }
        }
        return false;
    }
    
    /** Filters GEObject attributes so only those that are displayed by client
     * are also displayed by JGDIShell (Used in qconf -a*,-m*) */
    static boolean doNotDisplayConfigurableAttr(GEObject obj, PropertyDescriptor pd) {
        String name = java2cName(obj, pd.getPropertyName());
        //USER
        if (obj instanceof User) {
            if (name.equals("acl") || name.equals("xacl")) {
                return true;
            }
        }
        //PROJECT
        if (obj instanceof Project) {
            if (name.equals("delete_time") || name.equals("default_project")) {
                return true;
            }
        }
        //QUEUE
        if (obj instanceof ClusterQueue) {
            if (name.equals("tag")) {
                return true;
            }
        }
        //SCHECONF
        if (obj instanceof SchedConf) {
            if (name.equals("weight_tickets_override")) {
                return true;
            }
        }
        return false;
    }
    
    private static Double parseDouble(String str) {
        Double d;
        if (str.compareToIgnoreCase(VALUE_NONE)==0) {
            d = null;
        } else if (str.compareToIgnoreCase(VALUE_INFINITY)==0) {
            d = new Double(Double.MAX_VALUE);
        } else {
            d = new Double(str);
        }
        return d;
    }
    
    private static Integer parseInteger(String key, String str) {
        if (str.compareToIgnoreCase(VALUE_NONE)==0) {
            return null;
        } else if (str.compareToIgnoreCase(VALUE_UNDEFINED)==0) {
            return null;
        } else if (str.compareToIgnoreCase(VALUE_INFINITY)==0) {
            return new Integer(Integer.MAX_VALUE);
            //CLUSTERQUEUE - QTYPE
        } else if (key.equalsIgnoreCase("qtype")) {
            return getQtypeValue(str);
            //USERSET - TYPE
      /*} else if (key.equalsIgnoreCase("type")) {
         return getUserSetTypeValue(str);*/
            //SCHEDCONF - queue_sort_method
        } else if (key.equalsIgnoreCase("queueSortMethod")) {
            return getQSMtypeValue(str);
        } else {
            return new Integer(str);
        }
    }
    
    private static Long parseLong(String str) {
        //TIME
        if (str.matches("[0-9][0-9]*:[0-9][0-9]*:[0-9][0-9]*")) {
            return parseTime(str);
        }
        return new Long(Long.parseLong(str));
    }
    
    static Integer getQtypeValue(String str) {
        return getTypeMappedValue(QTYPES, str);
    }
    
    private static String getQtypeString(int val) {
        return getTypeMappedString(QTYPES, val);
    }
    
    static Integer getUserSetTypeValue(String str) {
        return getTypeMappedValue(userset_types, str);
    }
    
    private static String getUserSetTypeString(int val) {
        return getTypeMappedString(userset_types, val);
    }
    
    private static Integer getTypeMappedValue(String[] types, String str) {
        str = str.toUpperCase();
        int val = 0;
        int bitmask = 1;
        int pos=-1;
        for (int i = 0; i < types.length ; i++) {
            if ((pos = str.indexOf(types[i])) !=-1) {
                val |= bitmask;
                str = str.substring(0, pos) + str.substring(pos+types[i].length());
            }
            bitmask <<=1;
        }
        //Check there is nothing else in the line
        if (str.trim().length()>0) {
            throw new IllegalArgumentException("Unknown type value \""+str.trim()+"\"");
        }
        return new Integer(val);
    }
    
    private static String getTypeMappedString(String[] types, int val) {
        String str = "";
        int bitmask = 1;
        int pos=-1;
        for (int i = 0; i < types.length ; i++) {
            if ((val & bitmask) == bitmask) {
                str += types[i] + " ";
            }
            bitmask <<=1;
        }
        return str.trim();
    }
    
    static Integer getQSMtypeValue(String str) {
        str = str.toUpperCase();
        int val = 0;
        int bitmask = 1;
        int pos=-1;
        for (int i = 0; i < QSMTYPES.length ; i++) {
            if (str.equalsIgnoreCase(QSMTYPES[i])) {
                return new Integer(i);
            }
        }
        throw new IllegalArgumentException("Unknown qsm type value \""+str.trim()+"\"");
    }
    
    private static String getQSMtypeString(int val) {
        String str = "";
        int bitmask = 1;
        int pos=-1;
        if (val < 0 || val >= QSMTYPES.length) {
            throw new IllegalArgumentException("Unknown qsm type value \""+str.trim()+"\"");
        }
        return QSMTYPES[val];
    }
    
    private static Long parseTime(String time) {
        String[] elems = time.split(":");
        return new Long(Long.parseLong(elems[0])*3600000+Long.parseLong(elems[1])*60000+Long.parseLong(elems[2])*1000);
    }
    
    private static Boolean parseBoolean(String str) {
        if (str.compareToIgnoreCase("TRUE")==0 || str.compareTo("1")==0) {
            return Boolean.TRUE;
        } else if (str.compareToIgnoreCase("FALSE")==0 || str.compareTo("0")==0) {
            return Boolean.FALSE;
        }
        return null;
    }
    
    static Object translateObjectToStringValue(String key, Object o) {
        if (o == null) {
            return VALUE_NONE;
        }
        if (o instanceof List && ((List)o).size()==0) {
            return VALUE_NONE;
        }
        if (o instanceof Boolean) {
            return String.valueOf(o).toUpperCase();
        }
        if (o instanceof ComplexEntryImpl) {
            ComplexEntry ce = ((ComplexEntryImpl)o);
            return ce.getName()+"="+ce.getStringval();
        }
        if (o instanceof UserSetImpl) {
            UserSet us = ((UserSetImpl)o);
            return us.getName();
        }
        if (o instanceof String) {
            String str = (String) o;
            if (str.trim().length()==0) {
                return null;
            }
        }
        //TODO LP: Will need object type as well
        //CLUSTERQUEUE - QTYPE
        if (key.equalsIgnoreCase("qtype")) {
            if (o instanceof Integer) {
                return getQtypeString(((Integer)o).intValue());
            }
            //USERSET - TYPE
        } else if (key.equalsIgnoreCase("type")) {
            if (o instanceof Integer) {
                return getUserSetTypeString(((Integer)o).intValue());
            }
            //SCHEDCONF - queue_sort_method
        } else if (key.equals("queueSortMethod")) {
            if (o instanceof Integer) {
                return getQSMtypeString(((Integer)o).intValue());
            }
        }
        return o;
    }
    
    static Object translateStringValueToObject(Object o) {
        if (o instanceof String) {
            String str = (String) o;
            if (str.equalsIgnoreCase(VALUE_NONE) || str.trim().length()==0) {
                return null;
            }
        }
        return o;
    }
    
    static Object getParsedValueAsObject(JGDI jgdi, String key, String type, String value) {
        if (type.equals("int")) {
            return parseInteger(key, value);
        } else if (type.equals("long")) {
            return parseLong(value);
        } else if (type.equals("double")) {
            return parseDouble(value);
        } else if (type.equals("boolean")) {
            return parseBoolean(value);
        } else if (type.equals("java.lang.String")) {
            return translateStringValueToObject(value);
        } else if (type.equals("com.sun.grid.jgdi.configuration.ConfigurationElement")) {
            //TODO LP: What here?
        } else if (type.startsWith("com.sun.grid.jgdi.configuration.")) {
            if (translateStringValueToObject(value) == null) {
                return null;
            }
            Object newObj = null;
            try {
                //Used in tests
                if (jgdi == null) {
                    Class cls = Class.forName(type+"Impl");
                    Constructor c = cls.getConstructor(new Class[] {String.class});
                    newObj = c.newInstance(new Object[] {value});
                } else {
                    String [] elems = type.split("\\.");
                    String name = elems[elems.length-1];
                    String tmp = null;
                    //LP temp fix for User and Project
                    if (name.equals("AbstractUser")) {
                        if (key.equals("projects") || key.equals("xprojects")) {
                            name = "Project";
                        } else if (key.equals("acl") || key.equals("xacl")) {
                            name = "User";
                        } else {
                            throw new UnsupportedOperationException("Cannot decide target class for AbstractUser with key=\""+key+"\" value=\""+value+"\"");
                        }
                    } else if (name.equals("ComplexEntry")) {
                        elems = value.split("=");
                        value = elems[0];
                        tmp = elems[1]; //TODO LP: is set in GEObjectEditor.updateOneMapListEntry
                    }
                    Class cls = jgdi.getClass();
                    //TODO LP: Check if anywhere used JGDI.getJob(int jid) and handle it
                    Method m = cls.getDeclaredMethod("get"+name, new Class[] {String.class});
                    newObj = m.invoke(jgdi, new Object[] {value});
                    if (newObj == null) {
                        throw new IllegalArgumentException(name + " \"" + value + "\"" + " does not exist");
                    }
                    if (name.equals("ComplexEntry")) {
                        ((ComplexEntry)newObj).setStringval(tmp);
                    }
                }
            } catch (ClassNotFoundException ex) {
                ex.printStackTrace();
            } catch (InstantiationException ex) {
                ex.printStackTrace();
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
            } catch (SecurityException ex) {
                ex.printStackTrace();
            } catch (NoSuchMethodException ex) {
                ex.printStackTrace();
            } catch (InvocationTargetException ex) {
                ex.printStackTrace();
            }
            return newObj;
        } else {
            throw new IllegalArgumentException("Unknown data type=\""+type+"\"");
        }
        return null;
    }
    
    static Object getPropertyValue(GEObject obj, PropertyDescriptor pd) {
        if (pd instanceof SimplePropertyDescriptor) {
            Object value = ((SimplePropertyDescriptor)pd).getValue(obj);
            //return EditorUtil.translateObjectToStringValue(pd.getPropertyName(), ((SimplePropertyDescriptor)pd).getValue(obj););
            //SchedConf special formatting for double values
            if (obj instanceof SchedConf && value instanceof Double) {
                DecimalFormat df = new DecimalFormat("#0.000000", new DecimalFormatSymbols(Locale.US));
                return df.format(value);
            }
            return value;
        } else if (pd instanceof DefaultListPropertyDescriptor) {
            return convertList2String(obj, (DefaultListPropertyDescriptor)pd);
        } else if (pd instanceof DefaultMapPropertyDescriptor) {
            return convertMap2String(obj, (DefaultMapPropertyDescriptor)pd);
        } else if (pd instanceof DefaultMapListPropertyDescriptor) {
            return convertMapList2String(obj, (DefaultMapListPropertyDescriptor)pd);
        }
        System.err.println("WARNING: "+pd.getPropertyName()+ " type="+pd.getPropertyType()+" unknown="+pd.getClass().getName()+" returning NULL.");
        return null;
    }
    
    private static String convertList2String(GEObject obj, DefaultListPropertyDescriptor pd) {
        StringBuilder sb = new StringBuilder();
        for (int i=0; i < pd.getCount(obj); i++) {
            sb.append(" " + EditorUtil.translateObjectToStringValue(pd.getPropertyName(), pd.get(obj, i)));
        }
        if (sb.length() == 0) {
            return EditorUtil.VALUE_NONE;
        }
        sb.deleteCharAt(0);
        return sb.toString();//+" List";
    }
    
    private static String convertMap2String(GEObject obj, DefaultMapPropertyDescriptor pd) {
        StringBuilder sb = new StringBuilder();
        Object val;
        
        //SchedConf special case with hardcoded sorting cpu,mem,io
        if (obj instanceof SchedConf) {
            Set keys = pd.getKeys(obj);
            if (keys.size() != 3) {
                throw new IllegalArgumentException("Author only expected 3 keys [cpu, mem, io], but you got "+keys.size());
            }
            DecimalFormat df = new DecimalFormat("#0.000000", new DecimalFormatSymbols(Locale.US));
            return "cpu="+df.format(pd.get(obj,"cpu")) +
                    ",mem="+df.format(pd.get(obj,"mem")) +
                    ",io="+df.format(pd.get(obj,"io"));
        }
        
        for (String key : pd.getKeys(obj)) {
            val = EditorUtil.translateObjectToStringValue(pd.getPropertyName(), pd.get(obj, key));
            if (key.equals(EditorUtil.VALUE_DEFAULT)) {
                sb.insert(0,"," + val);
            } else {
                sb.append(",[" + key + "=" + val+"]");
            }
        }
        if (sb.length() == 0) {
            return EditorUtil.VALUE_NONE;
        }
        sb.deleteCharAt(0);
        return sb.toString();//+" Map";
    }
    
    private static String convertMapList2String(GEObject obj, DefaultMapListPropertyDescriptor pd) {
        StringBuilder sb = new StringBuilder();
        Object val;
        for (String key : pd.getKeys(obj)) {
            StringBuilder temp = new StringBuilder();
            val = null;
            for (int i=0; i < pd.getCount(obj, key); i++) {
                val = pd.get(obj, key, i);
                //ClusterQueue - ComplexEntryImpl
                if (val instanceof ComplexEntryImpl) {
                    val = ((ComplexEntryImpl)val).getName()+"="+((ComplexEntryImpl)val).getStringval();
                } else {
                    val = EditorUtil.translateObjectToStringValue(pd.getPropertyName(), val);
                }
                //TODO LP: Decide the default behaviour
                //temp.append(val + ",");
                temp.append(val + " ");
            }
            //Default key is always first
            if (key.equals(EditorUtil.VALUE_DEFAULT)) {
                if (temp.length() == 0 && val == null) {
                    temp = new StringBuilder(EditorUtil.VALUE_NONE);
                } else {
                    temp.deleteCharAt(temp.length()-1);
                }
                sb.insert(0,temp);
            } else {
                if (temp.length() == 0) {
                    continue;
                }
                temp.deleteCharAt(temp.length()-1);
                sb.append(",[" + key + "=" + temp + "]");
            }
        }
        if (sb.length() == 0) {
            return EditorUtil.VALUE_NONE;
        }
        return sb.toString();//+" MapList";
    }
    
    static public int sgeEdit(File file) {
        return nativeSgeEdit(file.getAbsolutePath());
    }
    
    static private native int nativeSgeEdit(String path);
}
