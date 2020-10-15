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
package com.sun.grid.jgdi.configuration;

import com.sun.grid.jgdi.configuration.reflect.ClassDescriptor;
import com.sun.grid.jgdi.configuration.reflect.ListPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.PropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.SimplePropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.*;
import java.util.*;

/**
 *
 */
public class Util {
    
    private static Map<Class, ClassDescriptor> classDescriptorMap = Collections.synchronizedMap(new HashMap<Class, ClassDescriptor>());
    
    /**
     *  Get a descriptor of a cull type
     *  @param  cullType name of the cull type
     *  @return the descriptor
     *  @throws IllegalArgumentException if the cullType is unknown
     */
    public static ClassDescriptor getDescriptorForCullType(String cullType) {
        Class clazz = CullTypeMapping.getClassForCullType(cullType);
        return getDescriptor(clazz);
    }
    
    /**
     * Register a new class descriptor.
     * @param aClass   the described class
     * @param cd       the descriptor
     */
    public static void addDescriptor(Class aClass, ClassDescriptor cd) {
        classDescriptorMap.put(aClass, cd);
    }
    
    public static ClassDescriptor getDescriptor(Class aClass) {
        ClassDescriptor ret = null;
        ret = classDescriptorMap.get(aClass);
        if (ret == null) {
            ret = createDescriptor(aClass);
            classDescriptorMap.put(aClass, ret);
        }
        return ret;
    }
    
    public static Object clone(Object obj) {
        ClassDescriptor cd = getDescriptor(obj.getClass());
        return cd.clone(obj);
    }
    
    public static int nextObjectId() {
        synchronized (Util.class) {
            int ret = objectId;
            objectId++;
            return ret;
        }
    }
    // private ------------------------------------------------------------------
    private static int objectId;
    
    private static ClassDescriptor createDescriptor(Class aClass) {
        ClassDescriptor ret = null;
        StringBuilder name = new StringBuilder();
        String packagename = aClass.getPackage().getName();
        name.append(packagename);
        name.append(".reflect.");
        
        String simpleName = aClass.getSimpleName();
        if (simpleName.endsWith("Impl")) {
            name.append(simpleName.substring(0, simpleName.length() - 4));
        } else {
            name.append(aClass.getSimpleName());
        }
        name.append("Descriptor");
        
        try {
            Class descr = Class.forName(name.toString());
            return (ClassDescriptor) descr.newInstance();
        } catch (ClassNotFoundException cnfe) {
            IllegalArgumentException ille = new IllegalArgumentException("Invalid Path: property " + aClass.getName() + " not found");
            ille.initCause(cnfe);
            throw ille;
        } catch (InstantiationException ise) {
            IllegalStateException ilse = new IllegalStateException("Invalid Path: property " + name);
            ise.printStackTrace();
            ilse.initCause(ise);
            throw ilse;
        } catch (IllegalAccessException iae) {
            IllegalStateException ilse = new IllegalStateException("Constructor of " + name + " is not accessible");
            iae.printStackTrace();
            ilse.initCause(iae);
            throw ilse;
        }
    }
    
    public static GEObject findObject(String path, GEObject root) {
        int dotIndex = path.indexOf('.');
        
        String name = path;
        String rest = null;
        
        if (dotIndex > 0) {
            name = path.substring(0, dotIndex);
            rest = path.substring(dotIndex + 1);
        }
        
        int bracketIndex = name.indexOf('[');
        String propertyName = null;
        
        if (bracketIndex > 0) {
            int bracketCloseIndex = name.indexOf(']', bracketIndex);
            
            if (bracketCloseIndex < 0) {
                throw new IllegalArgumentException("Unkonwn property descriptor " + path);
            }
            propertyName = name.substring(0, bracketIndex);
            name = name.substring(bracketIndex + 1, bracketCloseIndex);
        }
        
        if (propertyName != null) {
            ClassDescriptor descr = getDescriptor(root.getClass());
            PropertyDescriptor propDescr = descr.getProperty(propertyName);
            if (propDescr == null) {
                throw new IllegalArgumentException("Invalid Path: property " + propertyName + " in object " + root.getName() + " not found");
            } else if (!GEObject.class.isAssignableFrom(propDescr.getBeanClass())) {
                throw new IllegalArgumentException("Invalid Path: property " + propertyName + " in object " + root.getName() + " is not a GEObject");
            } else {
                GEObject child = null;
                if (propDescr instanceof SimplePropertyDescriptor) {
                    SimplePropertyDescriptor spd = (SimplePropertyDescriptor) propDescr;
                    child = (GEObject) spd.getValue(root);
                } else if (propDescr instanceof ListPropertyDescriptor) {
                    ListPropertyDescriptor lpd = (ListPropertyDescriptor) propDescr;
                    int propCount = lpd.getCount(root);
                    GEObject tmpChild = null;
                    for (int i = 0; i < propCount; i++) {
                        tmpChild = (GEObject) lpd.get(root, i);
                        if (tmpChild.getName().equals(name)) {
                            child = tmpChild;
                            break;
                        }
                    }
                } else {
                    throw new IllegalStateException("Unkonwn property descriptor " + propDescr.getClass().getName());
                }
                if (child == null) {
                    return null;
                } else if (rest != null) {
                    return findObject(rest, child);
                } else {
                    return child;
                }
            }
        } else {
            if (root.getName().equals(name)) {
                if (rest == null) {
                    return root;
                } else {
                    return findObject(rest, root);
                }
            } else {
                return null;
            }
        }
    }
    
    public static void getDifferences(GEObject obj1, GEObject obj2, List<Difference> differences) {
        getDifferences(obj1, obj2, ".", differences);
    }
    
    public static void getDifferences(GEObject obj1, GEObject obj2, String path, List<Difference> differences) {
        
        if (!obj1.getClass().equals(obj2.getClass())) {
            differences.add(new Difference(path, "obj1 and obj2 have not the same class"));
        } else {
            ClassDescriptor cd = getDescriptor(obj1.getClass());
            
            for (int i = 0; i < cd.getPropertyCount(); i++) {
                PropertyDescriptor pd = cd.getProperty(i);
                String propPath = path + pd.getPropertyName();
                
                if (pd instanceof SimplePropertyDescriptor) {
                    
                    SimplePropertyDescriptor spd = (SimplePropertyDescriptor) pd;
                    
                    Object v1 = spd.getValue(obj1);
                    Object v2 = spd.getValue(obj2);
                    
                    if (v1 == null && v2 == null) {
                        // no difference
                    } else if (v1 == null && v2 != null) {
                        differences.add(new Difference(propPath, "LPD: missing in obj1 (" + v2 + ")"));
                    } else if (v1 != null && v2 == null) {
                        differences.add(new Difference(propPath, "LPD: missing in obj2 (" + v1 + ")"));
                    } else {
                        if (GEObject.class.isAssignableFrom(spd.getPropertyType())) {
                            getDifferences((GEObject) v1, (GEObject) v2, propPath + ".", differences);
                        } else {
                            if (!v1.equals(v2)) {
                                differences.add(new Difference(propPath, v1 + " != " + v2));
                            }
                        }
                    }
                } else if (pd instanceof ListPropertyDescriptor) {
                    ListPropertyDescriptor lpd = (ListPropertyDescriptor) pd;
                    
                    int count1 = lpd.getCount(obj1);
                    int count2 = lpd.getCount(obj2);
                    if (count1 != count2) {
                        differences.add(new Difference(propPath, "MLPD: different length (" + count1 + " != " + count2 + ")"));
                    } else {
                        for (int valueIndex = 0; valueIndex < count1; valueIndex++) {
                            Object v1 = lpd.get(obj1, valueIndex);
                            Object v2 = lpd.get(obj2, valueIndex);
                            
                            String myPath = propPath + "[" + valueIndex + "]";
                            if (v1 == null && v2 == null) {
                                // do nothing
                            } else if (v1 == null && v2 != null) {
                                differences.add(new Difference(myPath, "MLPD: missing in obj1 (" + v2 + ")"));
                            } else if (v1 != null && v2 == null) {
                                differences.add(new Difference(myPath, "MLPD: missing in obj2 (" + v1 + ")"));
                            } else {
                                if (GEObject.class.isAssignableFrom(lpd.getPropertyType())) {
                                    getDifferences((GEObject) v1, (GEObject) v2, myPath + ".", differences);
                                } else {
                                    if (!v1.equals(v2)) {
                                        differences.add(new Difference(myPath, v1 + " != " + v2));
                                    }
                                }
                            }
                        }
                    }
                } else if (pd instanceof MapListPropertyDescriptor) {
                    
                    MapListPropertyDescriptor lpd = (MapListPropertyDescriptor) pd;
                    
                    Set keys1 = lpd.getKeys(obj1);
                    Set keys2 = lpd.getKeys(obj2);
                    if (keys1.size() != keys2.size()) {
                        differences.add(new Difference(propPath, "MLPD: different key count (" + keys1.size() + " != " + keys2.size() + ")"));
                    } else {
                        Iterator iter = keys1.iterator();
                        while (iter.hasNext()) {
                            Object key = iter.next();
                            String keyPath = propPath + "[" + key + "]";
                            int count1 = lpd.getCount(obj1, key);
                            int count2 = lpd.getCount(obj2, key);
                            
                            if (count1 != count2) {
                                differences.add(new Difference(keyPath, "MLPD: different length (" + count1 + " != " + count2 + ")"));
                            } else {
                                for (int valueIndex = 0; valueIndex < count1; valueIndex++) {
                                    Object v1 = lpd.get(obj1, key, valueIndex);
                                    Object v2 = lpd.get(obj2, key, valueIndex);
                                    
                                    String myPath = keyPath + "[" + valueIndex + "]";
                                    if (v1 == null && v2 == null) {
                                        // do nothing
                                    } else if (v1 == null && v2 != null) {
                                        differences.add(new Difference(myPath, "MPD: missing in obj1 (" + v2 + ")"));
                                    } else if (v1 != null && v2 == null) {
                                        differences.add(new Difference(myPath, "MPD: missing in obj2 (" + v1 + ")"));
                                    } else {
                                        if (GEObject.class.isAssignableFrom(lpd.getPropertyType())) {
                                            getDifferences((GEObject) v1, (GEObject) v2, myPath + ".", differences);
                                        } else {
                                            if (!v1.equals(v2)) {
                                                differences.add(new Difference(myPath, v1 + " != " + v2));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (pd instanceof MapPropertyDescriptor) {
                    
                    MapPropertyDescriptor lpd = (MapPropertyDescriptor) pd;
                    
                    Set keys1 = lpd.getKeys(obj1);
                    Set keys2 = lpd.getKeys(obj2);
                    
                    if (keys1.size() != keys2.size()) {
                        differences.add(new Difference(propPath, "MPD: different key count (" + keys1.size() + " != " + keys2.size() + ")"));
                    } else {
                        Iterator iter = keys1.iterator();
                        while (iter.hasNext()) {
                            Object key = iter.next();
                            
                            String keyPath = propPath + "[" + key + "]";
                            Object v1 = lpd.get(obj1, key);
                            Object v2 = lpd.get(obj2, key);
                            
                            if (v1 == null && v2 == null) {
                                // do nothing
                            } else if (v1 == null && v2 != null) {
                                differences.add(new Difference(keyPath, "MPD: missing in obj1 (" + v2 + ")"));
                            } else if (v1 != null && v2 == null) {
                                differences.add(new Difference(keyPath, "MPD: missing in obj2 (" + v1 + ")"));
                            } else {
                                if (GEObject.class.isAssignableFrom(lpd.getPropertyType())) {
                                    getDifferences((GEObject) v1, (GEObject) v2, keyPath + ".", differences);
                                } else {
                                    if (!v1.equals(v2)) {
                                        differences.add(new Difference(keyPath, v1 + " != " + v2));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    static class Difference {
        
        private String text;
        private String path;
        
        public Difference(String path, String text) {
            this.path = path;
            this.text = text;
        }
        
        public String toString() {
            return path + ": " + text;
        }
    }
}