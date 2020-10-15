/*___INFO__MARK_BEGIN__*//*************************************************************************
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
package com.sun.grid.javaconv;

import java.beans.PropertyDescriptor;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


/**
 *
 */
public class CWrapperHelper {

    private Class clazz;
    private Map<Class, JavaToC> typeMap;

    public CWrapperHelper(Class clazz) {
        this.clazz = clazz;
        typeMap = new HashMap<Class, JavaToC>();

        // Primitive data types
        regp(Integer.TYPE, "jint", "I", "Int", "0");
        regp(Short.TYPE, "jshort", "S", "Short", "0");
        regp(Long.TYPE, "jlong", "J", "Long", "0");
        regp(Boolean.TYPE, "jboolean", "Z", "Boolean", "FALSE");
        regp(Byte.TYPE, "jbyte", "B", "Byte", "0");
        regp(Character.TYPE, "jchar", "C", "Char", "0");
        regp(Float.TYPE, "jfloat", "F", "Float", "0.0f");
        regp(Double.TYPE, "jdouble", "D", "Double", "0.0");
        regp(Void.TYPE, "void", "V", "Void", "void");

        // objects
        reg(Class.class);
        reg(String.class);
    }

    private Map<String, Field> staticFieldMap;
    private List<String> staticFieldNameList;

    public List<String> getStaticFieldNames() {
        
        if (staticFieldMap == null) {
            staticFieldMap = new HashMap<String, Field>();
            Field[] fields = clazz.getFields();

            for (int i = 0; i < fields.length; i++) {
                if (!fields[i].getDeclaringClass().equals(clazz)) {
                    continue;
                }
                if (!Modifier.isPublic(fields[i].getModifiers())) {
                    continue;
                }
                if (!Modifier.isStatic(fields[i].getModifiers())) {
                    continue;
                }
                if (!Modifier.isFinal(fields[i].getModifiers())) {
                    continue;
                }
                String fieldName = fields[i].getName();
                staticFieldMap.put(fieldName, fields[i]);
            }
            staticFieldNameList = new ArrayList(staticFieldMap.keySet());
            Collections.sort(staticFieldNameList);
        }

        return staticFieldNameList;
    }

    public Field getStaticField(String name) {
        return staticFieldMap.get(name);
    }

    private Map<String, Method> staticMethodMap = null;
    private List<String> staticMethodNameList = null;

    public List<String> getStaticMethodNames() {

        if (staticMethodMap == null) {
            staticMethodMap = new HashMap<String, Method>();
            Method[] ms = clazz.getMethods();

            Arrays.sort(ms, new MethodComparator());
            for (int i = 0; i < ms.length; i++) {
                if (!ms[i].getDeclaringClass().equals(clazz)) {
                    continue;
                }
                if (!Modifier.isPublic(ms[i].getModifiers())) {
                    continue;
                }
                if (!Modifier.isStatic(ms[i].getModifiers())) {
                    continue;
                }

                String methodName = ms[i].getName();
                int methodIndex = 0;
                while (staticMethodMap.keySet().contains(methodName)) {
                    methodName = ms[i].getName() + "_" + methodIndex;
                    methodIndex++;
                }
                staticMethodMap.put(methodName, ms[i]);
            }
            staticMethodNameList = new ArrayList(staticMethodMap.keySet());
            Collections.sort(staticMethodNameList);
        }

        return staticMethodNameList;
    }

    public Method getStaticMethod(String name) {
        return staticMethodMap.get(name);
    }

    private Map<String, Method> methodMap = null;
    private List<String> methodNameList = null;

    public List<String> getMethodNames() {

        if (methodMap == null) {
            methodMap = new HashMap<String, Method>();
            Method[] ms = clazz.getMethods();

            Arrays.sort(ms, new MethodComparator());
            for (int i = 0; i < ms.length; i++) {
                if (!ms[i].getDeclaringClass().equals(clazz)) {
                    continue;
                }
                if (!Modifier.isPublic(ms[i].getModifiers())) {
                    continue;
                }
                if (ms[i].getName().equals("wait")) {
                    continue;
                }
                if (ms[i].getName().equals("notify")) {
                    continue;
                }
                if (ms[i].getName().equals("notifyAll")) {
                    continue;
                }

                String methodName = ms[i].getName();
                int methodIndex = 0;
                while (methodMap.keySet().contains(methodName)) {
                    methodName = ms[i].getName() + "_" + methodIndex;
                    methodIndex++;
                }
                methodMap.put(methodName, ms[i]);
            }
            methodNameList = new ArrayList(methodMap.keySet());
            Collections.sort(methodNameList);
        }

        return methodNameList;
    }

    static class MethodComparator implements Comparator {

        private int compare(Class[] p, Class[] p1) {
            int ret = p.length - p1.length;
            if (ret == 0) {
                for (int i = 0; i < p.length && ret == 0; i++) {
                    ret = p[i].getName().compareTo(p1[i].getName());
                }
            }
            return ret;
        }

        public int compare(Object obj, Object obj1) {
            int ret = 0;
            if (obj instanceof Constructor) {
                Constructor c = (Constructor) obj;
                Constructor c1 = (Constructor) obj1;
                ret = compare(c.getParameterTypes(), c1.getParameterTypes());
            } else {
                Method m = (Method) obj;
                Method m1 = (Method) obj1;
                ret = m.getName().compareTo(m1.getName());
                if (ret == 0) {
                    ret = compare(m.getParameterTypes(), m1.getParameterTypes());
                }
            }
            return ret;
        }
    }

    public Method getMethod(String name) {
        return methodMap.get(name);
    }


    private Map<String, Constructor> constructorMap;
    private List<String> constructorNameList;

    public List<String> getConstructorNames() {
        if (constructorMap == null) {
            constructorMap = new HashMap<String, Constructor>();
            Constructor[] cons = clazz.getConstructors();

            Arrays.sort(cons, new MethodComparator());
            for (int i = 0; i < cons.length; i++) {
                String consName = "init";
                int consIndex = 0;
                while (constructorMap.keySet().contains(consName)) {
                    consName = "init_" + consIndex;
                    consIndex++;
                }
                constructorMap.put(consName, cons[i]);
            }
            constructorNameList = new ArrayList(constructorMap.keySet());
            Collections.sort(constructorNameList);
        }
        return constructorNameList;
    }

    public Constructor getConstructor(String name) {
        return constructorMap.get(name);
    }

    public String getCType(PropertyDescriptor pd) {
        return getCType(pd.getPropertyType());
    }

    public String getCType(Class type) {
        if (type.isArray()) {
            return getJTC(type.getComponentType()).getCType() + "*";
        } else {
            return getJTC(type).getCType();
        }
    }

    public String getGetArrayElementsMethod(Class type) {
        return getJTC(type).getGetArrayElementsMethod();
    }

    public String getReleaseArrayElementsMethod(Class type) {
        return getJTC(type).getReleaseArrayElementsMethod();
    }


    public String getCSignator(Class type) {
        if (type.isArray()) {
            Class realType = type.getComponentType();
            JavaToC jtc = typeMap.get(realType);
            return "[" + jtc.getCSignature();
        } else {
            return getJTC(type).getCSignature();
        }
    }

    public String getSignature(Method method) {
        StringBuilder ret = new StringBuilder();
        ret.append("(");
        Class[] params = method.getParameterTypes();
        for (int i = 0; i < params.length; i++) {
            ret.append(getCSignator(params[i]));
        }
        ret.append(")");
        ret.append(getCSignator(method.getReturnType()));
        return ret.toString();
    }

    public String getSignature(Field field) {
        return getCSignator(field.getType());
    }

    public String getSignature(Constructor constructor) {
        StringBuilder ret = new StringBuilder();
        ret.append("(");
        Class[] params = constructor.getParameterTypes();
        for (int i = 0; i < params.length; i++) {
            if (i > 0) {
                ret.append(", ");
            }
            ret.append(getCSignator(params[i]));
        }
        ret.append(")V");
        return ret.toString();
    }

    public String getCallMethod(Class returnType) {
        return getJTC(returnType).getCallMethod();
    }

    public String getStaticCallMethod(Class returnType) {
        return getJTC(returnType).getStaticCallMethod();
    }

    public String getStaticGetFieldMethod(Field field) {
        return getJTC(field.getType()).getStaticGetFieldMethod();
    }

    public String getInitializer(Class type) {
        String ret = getJTC(type).getInitializer();
        return ret;
    }

    protected JavaToC getJTC(Class type) {
        JavaToC ret = typeMap.get(type);
        if (ret == null) {
            ret = new ObjectToC(type);
            typeMap.put(type, ret);
        }
        return ret;
    }

    static abstract class JavaToC {
        private Class javaType;
        
        protected JavaToC(Class javaType) {
            this.javaType = javaType;
        }
        
        public Class getJavaType() {
            return javaType;
        }

        public abstract String getCType();

        public abstract String getCallMethod();

        public abstract String getStaticCallMethod();

        public abstract String getCSignature();

        public abstract String getInitializer();

        public abstract String getStaticGetFieldMethod();

        public abstract String getGetArrayElementsMethod();

        public abstract String getReleaseArrayElementsMethod();
    }

    public class PrimitiveToC extends JavaToC {

        private String ctype;
        private String methodPart;
        private String csignature;
        private String initializer;

        public PrimitiveToC(Class javaType, String ctype, String csignature, String methodPart, String initializer) {
            super(javaType);
            this.csignature = csignature;
            this.ctype = ctype;
            this.methodPart = methodPart;
            this.initializer = initializer;
        }

        public String getCType() {
            return ctype;
        }

        public String getCSignature() {
            return csignature;
        }

        public String getInitializer() {
            return initializer;
        }

        public String getCallMethod() {
            return "Call" + methodPart + "Method";
        }

        public String getStaticCallMethod() {
            return "CallStatic" + methodPart + "Method";
        }

        public String getStaticGetFieldMethod() {
            return "GetStatic" + methodPart + "Field";
        }

        public String getGetArrayElementsMethod() {
            return "Get" + methodPart + "ArrayElements";
        }

        public String getReleaseArrayElementsMethod() {
            return "Release" + methodPart + "ArrayElements";
        }
    }

    public class ObjectToC extends JavaToC {

        public ObjectToC(Class javaType) {
            super(javaType);
        }

        public String getCType() {
            return "jobject";
        }

        public String getCallMethod() {
            return "CallObjectMethod";
        }

        public String getStaticCallMethod() {
            return "CallStaticObjectMethod";
        }

        public String getStaticGetFieldMethod() {
            return "GetStaticObjectField";
        }

        public String getGetArrayElementsMethod() {
            return "GetObjectArrayElements";
        }

        public String getReleaseArrayElementsMethod() {
            return "ReleaseObjectArrayElements";
        }

        private String csignature;

        public String getCSignature() {
            if (csignature == null) {
                csignature = "L" + getJavaType().getName().replace('.', '/') + ";";
            }
            return csignature;
        }

        public String getInitializer() {
            return "NULL";
        }
    }

    void regp(Class javaType, String ctype, String csignature, String methodPart, String initializer) {
        PrimitiveToC jtc = new PrimitiveToC(javaType, ctype, csignature, methodPart, initializer);
        typeMap.put(javaType, jtc);
    }

    void reg(Class javaType) {
        typeMap.put(javaType, new ObjectToC(javaType));
    }
}
