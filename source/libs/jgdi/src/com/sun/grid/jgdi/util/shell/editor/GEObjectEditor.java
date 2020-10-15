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

import com.sun.grid.jgdi.JGDIException;
//import com.sun.grid.jgdi.configuration.AbstractUser;
import com.sun.grid.jgdi.configuration.ClusterQueue;
import com.sun.grid.jgdi.configuration.ClusterQueueImpl;
import com.sun.grid.jgdi.configuration.Configuration;
import com.sun.grid.jgdi.configuration.ConfigurationElement;
import com.sun.grid.jgdi.configuration.ConfigurationElementImpl;
import com.sun.grid.jgdi.configuration.Util;
import com.sun.grid.jgdi.configuration.GEObject;
import com.sun.grid.jgdi.configuration.reflect.ClassDescriptor;
import com.sun.grid.jgdi.configuration.reflect.DefaultListPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.DefaultMapListPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.DefaultMapPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.PropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.SimplePropertyDescriptor;
import com.sun.grid.jgdi.configuration.xml.XMLUtil;
import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.configuration.ShareTreeImpl;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * GEObject editor is a class for editing any GEObject based on text. This text
 * will be parsed. If it's format is valid atrributes that were provided will be
 * updated based on the values next to the attribute.
 *
 * Less coding when creating a new GeObject:
 *
 * <CODE>ClusterQueue geObj = new ClusterQueue(true);
 * GEObjectEditor.updateObjectWithText(jgdi, geObj, "name test\n acl user1 user2\n xacl user3");
 *   or
 * GEObjectEditor.updateObjectWithText(geObj, "name test\n acl user1 user2\n xacl user3");
 * jgdi.addClusterQueue(geObj);</CODE>
 *
 * Only difference between these methods is that whenJGDI reference is passed attributes
 * are checked if they are known to Grid Engine and their structure is more advaced,
 * they are retrived.
 * Such as can have ...
 *
 * While when not passing JGDI reference, new Objects are created with default values.
 * This can lead to unexpected behaviour when you try to update already existing object
 * or even create a new one.
 *
 * <B>RECOMMENDED IS TO USE <CODE>updateObjectWithText(JGDI jgdi, GEObject obj, String text)</CODE> method.</B>
 *
 * Modifying existing GEObjects:
 *
 * <CODE>ClusterQueue geObj = jgdi.getClusterQueue("test");
 * GEObjectEditor.updateObjectWithText(jgdi, geObj, "xacl user4");
 * jgdi.updateClusterQueue(geObj);</CODE>
 */
public class GEObjectEditor {

    private static final int ELEMENT_NAME = 10;
    private static final int ELEMENT_VALUE = 11;

    /**
     * Updates GEObject based on the text. Text is parsed and if correct, object is updated.
     * Creates new objects where necessary, based on values provided in text, therefore
     * it is recommended to use method <CODE>updateObjectWithText(JGDI jgdi, GEObject obj, String text)</CODE>
     * that retrieves these objects from Grid Engine instead.
     */
    public static <T extends GEObject> T updateObjectWithText(T obj, String text) {
        return doUpdate(null, obj, text);
    }

    /**
     * Updates GEObject based on the text. Text is parsed and if correct, object is updated.
     * Retrives objects from Grid Engine where necessary.
     * Recommended method.
     */
    public static <T extends GEObject> T updateObjectWithText(JGDI jgdi, T obj, String text) {
        if (jgdi == null) {
            throw new IllegalArgumentException("JGDI is NULL");
        }
        return doUpdate(jgdi, obj, text);
    }

    private static <T extends GEObject> T doUpdate(JGDI jgdi, T obj, String text) {
        PropertyDescriptor pd;
        Object key;
        String line;
        if (obj == null) {
            throw new IllegalArgumentException("GEObject is NULL");
        }
        if (text.startsWith(XMLUtil.HEADER)) {
            throw new UnsupportedOperationException("XML based editing not yet implemented!!!");
        } else {
            try {
                Map propertyMap = null;
                if (obj instanceof ShareTreeImpl) {
                    obj = (T) EditorParser.parseShareTreeText(text);
                } else {
                    propertyMap = EditorParser.parsePlainText(obj, text, " ");
                    for (Iterator iter = propertyMap.keySet().iterator(); iter.hasNext();) {
                        key = iter.next();
                        line = (String) propertyMap.get(key);
                        updatePropertyValue(jgdi, obj, key, line);
                    }
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
        return obj;
    }

    private static <T extends GEObject> void updatePropertyValue(JGDI jgdi, T obj, Object key, String values) throws JGDIException {
        if (key instanceof SimplePropertyDescriptor) {
            updateSimpleProperty(jgdi, obj, (SimplePropertyDescriptor) key, values);
        } else if (key instanceof DefaultListPropertyDescriptor) {
            updateListProperty(jgdi, obj, (DefaultListPropertyDescriptor) key, values);
        } else if (key instanceof DefaultMapPropertyDescriptor) {
            updateMapProperty(jgdi, obj, (DefaultMapPropertyDescriptor) key, values);
        } else if (key instanceof DefaultMapListPropertyDescriptor) {
            updateMapListProperty(jgdi, obj, (DefaultMapListPropertyDescriptor) key, values);
        //For CONFIGURATION objects
        } else if (obj instanceof Configuration && key instanceof String) {
            Configuration c = (Configuration) obj;
            ConfigurationElement ce = new ConfigurationElementImpl();
            ce.setName((String) key);
            //TODO LP: Find out expected behaviour! Can values contain list of ConfigElems?
            ce.setValue(values.trim());
            c.addEntries(ce);
            //TODO LP: Need to exit if we get can't resolve hostname. Otherwise we get stacktrace for each element.
            //Also if should be reworked to return correct error code (1).
            jgdi.updateConfiguration(c);
        } else {
            new IllegalArgumentException("Unknown descriptor type=\"" + key.getClass().getName() +
                    "\" for object type " + obj.getClass().getName());
        }
    }

    private static void updateSimpleProperty(JGDI jgdi, GEObject obj, SimplePropertyDescriptor pd, String value) {
        String type = pd.getPropertyType().getName();
        Object val = EditorUtil.getParsedValueAsObject(jgdi, pd.getPropertyName(), type, value);
        if (obj.getClass().getSimpleName().equals("CheckpointImpl") && val == null) {
            val = "NONE";
        }
        pd.setValue(obj, val);
    }

    private static void updateListProperty(JGDI jgdi, GEObject obj, DefaultListPropertyDescriptor pd, String values) {
        String type = pd.getPropertyType().getName();
        String[] elems = values.split(" ");
        Object value;
        pd.removeAll(obj);
        for (int i = 0; i < elems.length; i++) {
            value = EditorUtil.getParsedValueAsObject(jgdi, pd.getPropertyName(), type, elems[i]);
            if (value != null) {
                pd.add(obj, value);
            }
        }
    }

    private static void updateMapProperty(JGDI jgdi, GEObject obj, DefaultMapPropertyDescriptor pd, String values) {
        String attr = pd.getPropertyName();
        String key, elem = null;
        Object val;
        String[] elems = values.substring(1, values.length() - 1).split("] \\[");

        pd.removeAll(obj);
        for (int i = 0; i < elems.length; i++) {
            elem = elems[i];
            //Get a key for the map
            int keyEndPos = elem.indexOf('=');
            key = elem.substring(0, keyEndPos);
            elem = elem.substring(keyEndPos + 1, elem.length());
            //ClusterQueue - QTYPE we already have the int value as String, so we don't convert
            if (attr.equalsIgnoreCase("qtype")) {
                pd.put(obj, key, Integer.valueOf(elem));
                return;
            }
            //TODO Should there be a 'val = getParsedValue(elem)' or map is just String
            val = EditorUtil.getParsedValueAsObject(jgdi, pd.getPropertyName(), pd.getPropertyType().getName(), elem);
            val = EditorUtil.translateStringValueToObject(val);
            //TODO LP: Since we do pd.removeAll() this works as setting null value, but some GEObjects expect to have default value set to null!
            //LP: Temp fix for some objects
            String cls = obj.getClass().getSimpleName();
            boolean putNullValue = ( //cls.equals("ExecHost") ||
                    cls.equals("ClusterQueueImpl") //calendar NONE => needs @/=null
                    ) ? true : false;
            if (val != null || putNullValue) {
                pd.put(obj, key, val);
            }
        }
    }

    private static void updateOneMapListEntry(JGDI jgdi, GEObject obj, DefaultMapListPropertyDescriptor pd, String key, String elem) {
        String type = pd.getPropertyType().getName();
        String[] elems, subElems;
        String name, strVal;
        Object val;
        boolean isCurrentElemMap;
        //Get elements from the line
        elems = elem.split(" ");
        //Set new value for each of them
        for (int i = 0; i < elems.length; i++) {
            isCurrentElemMap = EditorParser.isMap(elems[i]);
            if (isCurrentElemMap) {
                subElems = elems[i].split("=");
                //name = subElems[0];
                strVal = (String) EditorUtil.translateStringValueToObject(subElems[1]);
//                if (strVal == null) {
//                    continue;
//                }
                val = EditorUtil.getParsedValueAsObject(jgdi, pd.getPropertyName(), type, elems[i]);
                if (val == null) {
                    continue;
                }
            } else {
                name = elems[i];
                val = EditorUtil.getParsedValueAsObject(jgdi, pd.getPropertyName(), type, name);
                //Add only 1 NULL value for default key
                if (val == null) {
                    if (!key.equals(EditorUtil.VALUE_DEFAULT)) {
                        continue;
                    }
                    List l = pd.getList(obj, key);
                    if (l != null && l.size() > 0) {
                        continue;
                    }
                }
            }
            if (val == null) {
                pd.addEmpty(obj, key);
            } else {
                pd.add(obj, key, val);
            }
        }
    }

    private static void updateMapListProperty(JGDI jgdi, GEObject obj, DefaultMapListPropertyDescriptor pd, String values) {
        String key, elem = null;
        String[] elems = values.substring(1, values.length() - 1).split("] \\[");

        pd.removeAll(obj);
        for (int i = 0; i < elems.length; i++) {
            elem = elems[i];
            //Get a key for the map
            int keyEndPos = elem.indexOf('=');
            key = elem.substring(0, keyEndPos);
            elem = elem.substring(keyEndPos + 1, elem.length());
            updateOneMapListEntry(jgdi, obj, pd, key, elem);
        }
    }

    /**
     * Retrives all properties known to JGDI for specified GEObject as text. Each property on one line.
     */
    public static String getAllPropertiesAsText(GEObject obj) {
        return getPropertiesAsText(obj, EditorUtil.PROPERTIES_ALL);
    }

    /**
     * Retrives configurable properties for specified GEObject as text. Each property on one line.
     */
    public static String getConfigurablePropertiesAsText(GEObject obj) {
        return getPropertiesAsText(obj, EditorUtil.PROPERTIES_CONFIGURABLE);
    }

    /**
     * Retrives read-only properties for specified GEObject as text. Each property on one line.
     */
    public static String getReadOnlyPropertiesAsText(GEObject obj) {
        return getPropertiesAsText(obj, EditorUtil.PROPERTIES_READ_ONLY);
    }

    public static String getPropertiesAsText(GEObject obj, int propScope) {
        Object o;
        int maxLen = 0;
        String name, spaces;
        Object value;
        StringBuilder sb = new StringBuilder();
        PropertyDescriptor pd;
        List subNames = null, subValues = null;
        for (Iterator iter = getProperties(obj, propScope).iterator(); iter.hasNext();) {
            pd = (PropertyDescriptor) iter.next();
            subNames = getStretchedElementNames(obj, pd);
            if (subNames.size() > 0) {
                for (Iterator it = subNames.iterator(); it.hasNext();) {
                    maxLen = Math.max(maxLen, ((String) it.next()).length());
                }
            } else {
                name = EditorUtil.java2cName(obj, pd.getPropertyName());
                maxLen = Math.max(maxLen, name.length());
            }
        }

        for (Iterator iter = getProperties(obj, propScope).iterator(); iter.hasNext();) {
            pd = (PropertyDescriptor) iter.next();
            subNames = getStretchedElementNames(obj, pd);
            if (subNames.size() > 0) {
                subValues = getStretchedElementValues(obj, pd);
                if (subNames.size() != subValues.size()) {
                    throw new IllegalArgumentException("Unknown error: Expecting name x value lists of a same size! Got sizes " + subNames.size() + " and " + subValues.size());
                }
                for (int j = 0; j < subNames.size(); j++) {
                    name = (String) subNames.get(j);
                    value = (String) subValues.get(j);
                    sb.append(name);
                    for (int i = name.length(); i < maxLen; i++) {
                        sb.append(' ');
                    }
                    sb.append("    " + value + "\n");
                }
            } else {
                name = EditorUtil.java2cName(obj, pd.getPropertyName());
                value = EditorUtil.translateObjectToStringValue(pd.getPropertyName(), EditorUtil.getPropertyValue(obj, pd));
                if (obj instanceof Configuration && name.equals("hostname")) {
                    sb.append("#");
                    sb.append(value);
                    sb.append(":\n");
                } else {
                    sb.append(name);
                    for (int i = name.length(); i < maxLen; i++) {
                        sb.append(' ');
                    }
                    spaces = "    ";
                    sb.append(spaces + value + "\n");
                }
            }
        }
        return sb.toString();
    }

    public static String getPropertyAsText(GEObject obj, int propScope, String propName) throws JGDIException {
        Object o;
        int maxLen = 0;
        String name, spaces;
        Object value;
        StringBuilder sb = new StringBuilder();
        List subNames = null, subValues = null;
        PropertyDescriptor pd = getProperty(obj, propScope, propName);
        if (pd == null) {
            throw new JGDIException("JGDI Error: Attribute \"" + propName + "\" does not exits in " + obj.getName());
        }
        subNames = getStretchedElementNames(obj, pd);
        if (subNames.size() > 0) {
            subValues = getStretchedElementValues(obj, pd);
            if (subNames.size() != subValues.size()) {
                throw new IllegalArgumentException("Unknown error: Expecting name x value lists of a same size! Got sizes " + subNames.size() + " and " + subValues.size());
            }
            for (int j = 0; j < subNames.size(); j++) {
                name = (String) subNames.get(j);
                value = (String) subValues.get(j);
                sb.append(name);
                for (int i = name.length(); i < maxLen; i++) {
                    sb.append(' ');
                }
                sb.append("    " + value + "\n");
            }
        } else {
            name = EditorUtil.java2cName(obj, pd.getPropertyName());
            value = EditorUtil.translateObjectToStringValue(pd.getPropertyName(), EditorUtil.getPropertyValue(obj, pd));
            sb.append(name);
            for (int i = name.length(); i < maxLen; i++) {
                sb.append(' ');
            }
            spaces = "    ";
            sb.append(spaces + value + "\n");
        }
        return sb.toString();
    }

    private static List getStretchedElementNames(GEObject obj, PropertyDescriptor pd) {
        return getStretchedElementList(obj, pd, ELEMENT_NAME);
    }

    private static List getStretchedElementValues(GEObject obj, PropertyDescriptor pd) {
        return getStretchedElementList(obj, pd, ELEMENT_VALUE);
    }

    private static List getStretchedElementList(GEObject obj, PropertyDescriptor pd, int type) {
        List list = new ArrayList();
        //CONFIGURATION
        if (obj instanceof Configuration) {
            String name = EditorUtil.java2cName(obj, pd.getPropertyName());
            if (name.equals("entries")) {
                Iterator iter = ((Configuration) obj).getEntriesList().iterator();
                ConfigurationElement elem = null;
                while (iter.hasNext()) {
                    elem = (ConfigurationElement) iter.next();
                    switch (type) {
                        case ELEMENT_NAME:
                            list.add(elem.getName());
                            break;
                        case ELEMENT_VALUE:
                            list.add(elem.getValue());
                            break;
                        default:
                            throw new IllegalArgumentException("Invalid element type: " + type + "!");
                    }
                }
            }
        }
        return list;
    }

    static List<PropertyDescriptor> getAllProperties(GEObject obj) {
        return getProperties(obj, EditorUtil.PROPERTIES_ALL);
    }

    static List getConfigurableProperties(GEObject obj) {
        return getProperties(obj, EditorUtil.PROPERTIES_CONFIGURABLE);
    }

    static List getReadOnlyProperties(GEObject obj) {
        return getProperties(obj, EditorUtil.PROPERTIES_READ_ONLY);
    }

    static List getProperties(GEObject obj, int propScope) {
        List<PropertyDescriptor> propList = new ArrayList<PropertyDescriptor>();
        ClassDescriptor cd = Util.getDescriptor(obj.getClass());
        for (PropertyDescriptor pd : cd.getProperties()) {
            if (EditorUtil.doNotDisplayAttr(obj, pd, propScope)) {
                continue;
            }
            if (isValidPropertyType(pd, propScope)) {
                propList.add(pd);
            }
        }
        return propList;
    }

    static PropertyDescriptor getProperty(GEObject obj, int propScope, String name) {
        List<PropertyDescriptor> propList = new ArrayList<PropertyDescriptor>();
        ClassDescriptor cd = Util.getDescriptor(obj.getClass());
        PropertyDescriptor pd = cd.getProperty(EditorUtil.unifyClientNamesWithAttr(obj, name));
        if (pd == null) {
            return null;
        }
        if (EditorUtil.doNotDisplayAttr(obj, pd, propScope)) {
            return null;
        }
        if (isValidPropertyType(pd, propScope)) {
            return pd;
        }
        return null;
    }

    static boolean isValidPropertyType(PropertyDescriptor pd, int propScope) {
        switch (propScope) {
            case EditorUtil.PROPERTIES_ALL:
                return true;
            case EditorUtil.PROPERTIES_CONFIGURABLE:
                return pd.isConfigurable();
            case EditorUtil.PROPERTIES_READ_ONLY:
                return pd.isReadOnly();
            default:
                throw new IllegalArgumentException("Invalid property scope specifier!");
        }
    }

    public static void main(String[] args) {
        ClusterQueue geObject = new ClusterQueueImpl(true);
        System.out.println(GEObjectEditor.getConfigurablePropertiesAsText(geObject));
        System.out.println(GEObjectEditor.getReadOnlyPropertiesAsText(geObject));
        System.out.println(GEObjectEditor.getAllPropertiesAsText(geObject));
    }
}
