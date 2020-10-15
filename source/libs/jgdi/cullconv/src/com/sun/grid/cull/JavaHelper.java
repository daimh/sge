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
package com.sun.grid.cull;

import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.logging.Logger;

/**
 * The <code>JavaHelper</code> contains all mapping functionality for mapping
 * cull objects to java objects.
 *
 */
public class JavaHelper {

   /** The suffix string of the cull object ("_Type"). */
   public static final String TYPE_SUFFIX = "_Type";
   
   private Logger logger = Logger.getLogger("cullconv");
   
   /** array with all primitives and it's wrapper types.
    *  @@see #isPrimitiv
    */
   private String [][] PRIMITIVES = {
      { "int", "Integer", "0" },
      { "boolean", "Boolean", "false" },
      { "double", "Double", "0.0" },
      { "float", "Float", "0.0f" },
      { "long", "Long", "0" },
      { "char", "Character", "'n'" },
      { "byte", "Byte", "0" }
   };
   
   
   private static Map builtinTypeMap;
   
   static {
      builtinTypeMap = new HashMap();
      builtinTypeMap.put( "SGE_STRING", "String" );
      builtinTypeMap.put( "SGE_XSTRING", "String" );
      builtinTypeMap.put( "SGE_ULONG",  "int");
      builtinTypeMap.put( "SGE_INT", "int" );
      builtinTypeMap.put( "SGE_LONG", "int" );
      builtinTypeMap.put( "SGE_HOST", "String");
      builtinTypeMap.put( "SGE_BOOL", "boolean");
      builtinTypeMap.put( "SGE_DOUBLE", "double");
      builtinTypeMap.put( "SGE_FLOAT", "double");
      builtinTypeMap.put( "SGE_CHAR", "char");
      builtinTypeMap.put( "SGE_STRING_D", "String" );
      builtinTypeMap.put( "SGE_XSTRING_D", "String" );
      builtinTypeMap.put( "SGE_ULONG_D",  "int");
      builtinTypeMap.put( "SGE_INT_D", "int" );
      builtinTypeMap.put( "SGE_LONG_D", "int" );
      builtinTypeMap.put( "SGE_HOST_D", "String");
      builtinTypeMap.put( "SGE_BOOL_D", "boolean");
      builtinTypeMap.put( "SGE_DOUBLE_D", "double");
      builtinTypeMap.put( "SGE_FLOAT_D", "double");
      builtinTypeMap.put( "SGE_CHAR_D", "char");
      builtinTypeMap.put( "CULL_ANY_SUBTYPE", "CULL_ANY_SUBTYPE");
   }
   
   /** the cull definition object */
   private CullDefinition cullDef;
   
   /**
    * <p>Creates a new instance of JavaHelper.</p>
    *
    * @param cullDef  the cull definition object
    */
   public JavaHelper(CullDefinition cullDef) {
      this.cullDef = cullDef;
   }

   /**
    *   <p>Get the package name of all cull model classes.
    *   This name is defined in the cullconv ant task.</p>
    *
    *   E.g:
    *
    *   If the package name <code>"com.sun.grid.mode"</code> then
    *   the ant task definition is
    *   <pre>
    *
    *   &lt;cullconv packageName="com.sun.grid.jgdi.configuration"
    *            buildDir="${build.dir}/tmp"&gt;
    *   ..
    *   &lt;cullconv&gt;
    *
    *   </pre>
    *
    *   @return the package name of the cull model classes
    */
   public String getCullDefPackageName() {
      return cullDef.getPackageName();
   }
   
   /** package name of the generated classes */
   private String packageName;
   
   
   /**
    * <p>Get the package name of the generated class/classes
    * The package name is defined in the nested element <code>javatemplateconv</code>
    * of a cullconv ant task.</p>
    *
    * <p>Example ant task:</p>
    *
    * <pre>
    * &lt;cullconv packageName="com.sun.grid.jgdi.configuration"&gt;
    *    &lt;javatemplateconv outputDir="${gensrc.dir}"
    *                         classSuffix="ModEvent"
    *                         packagename="<bold>com.sun.grid.jgdi.event</bold>"
    *                         scope="idl_object"
    *                         template="templates/java_modevent.jsp"&gt;
    * &lt;/cullconv&gt;
    * </pre>
    * @return the package name for the generated classes
    */
   public String getPackageName() {
      if(packageName == null ) {
         return getCullDefPackageName();
      } else {
         return packageName;
      }
   }
   
   /**
    * Set the package name for the generated classes
    * @param packageName
    */
   public void setPackageName(String packageName) {
      this.packageName = packageName;
   }
   
   
   /**
    * converts a cull name into a java name
    * @param cullName  the cull name
    * @return  the java name
    */
   private static String cullNameToJava(String cullName) {
      int index = cullName.indexOf(TYPE_SUFFIX);
      if( index > 0 ) {
         return cullName.substring(0,index);
      }
      return cullName;
   }
   
   
   
   /**
    * <p>Get the java classname of a cull object.</p>
    *
    * <p>If a idl name is defined in the cull object, then the classname is
    * equal to the idl name. Otherwise the classname is name of the cull
    * object without the type suffix.</p>
    *
    * <p>Examples:</p>
    *
    * <table border="1">
    *  <tr> <th>Cull definition</th> <th>classname</th> </tr>
    *  <tr>
    *     <td><code>ILISTDEF(CQ_Type, ClusterQueue, SGE_CQUEUE_LIST )</code></td>
    *     <td><code>ClusterQueue</code></td>
    *  </tr>
    *  <tr>
    *     <td><code>LISTDEF(CA_Type)</code></td>
    *     <td><code></code>CA</td>
    *  </tr>
    * </table>
    * @param obj the cull object
    * @return the java classname of the cull object with the packagename
    * @see    #getFullClassName
    */
   public String getClassName(CullObject obj) {
      if(obj.getType() == CullObject.TYPE_PRIMITIVE ) {
         try {
            CullAttr attr = obj.getContentAttr();
            return getClassName(attr.getType());
         } catch (IllegalArgumentException iae) {
            logger.severe("Content attribute + '" + obj.getContentAttr().getName() + "' not found for primitive cull object " + obj.getName());
            for(int i = 0; i < obj.getAttrCount(); i++) {
               logger.severe("attr["+i+"] = '" + obj.getAttr(i).getName() + "'");
            }
            throw iae;
         }
      } else if ( obj.getType() == CullObject.TYPE_MAPPED) {
         return obj.getImplClass();
      } else if( obj.getIdlName() != null ) {
         return obj.getIdlName();
      } else {
         return cullNameToJava(obj.getName());
      }
   }
   
   public String getNonPrimitiveClassname(CullObject obj) {
      
      if( obj.getIdlName() != null ) {
         return obj.getIdlName();
      } else {
         return cullNameToJava(obj.getName());
      }
      
   }
   
   
   /**
    * Get the file were a cull object is defined
    * @param obj the cull object
    * @return the file
    */
   public File getSource(CullObject obj) {
      return cullDef.getSource(obj.getName());
   }
   
   /**
    * <p>Get the full class name (including the cull def package)
    * of a cull object</p>
    *
    * @see #getCullDefPackageName
    * @see #getClassName
    */
   public String getFullClassName(CullObject obj) {
      if(obj.getType() == CullObject.TYPE_PRIMITIVE) {
         CullAttr attr = obj.getContentAttr();
         return getFullClassName(attr.getType());
      } else if( obj.getType() == CullObject.TYPE_MAPPED ) {
         return obj.getImplClass();
      } else {
         return getCullDefPackageName() + '.' + getClassName(obj);
      }
   }
   
   
   /**
    * <p>Get the full java class name of a cull type name.</p>
    *
    * <p>Valid culltype all name of the cull objects and the
    * builtin types. If the builtin type is mapped the to a
    * primitive java type, then the name of the primitiv java
    * type is returned:
    *
    * Examples:
    *
    *
    *
    * <table border="1">
    *   <tr>
    *     <th>Cull type</th>
    *     <th>Full class name</th>
    *   </tr>
    *   <tr>
    *     <td><code>CA_Type</code></td>
    *     <td><code>com.sun.grid.jgdi.configuration.CA</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_BOOL</code></td>
    *     <td><code>boolean</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_INT</code></td>
    *     <td><code>int</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_STRING</code></td>
    *     <td><code>java.lang.String</code></td>
    *   </tr>
    * </table>
    *
    * @param cullType  name of the cull type
    * @return the full classname of the the name of the primitiv data type
    */
   public String getFullClassName(String cullType) {
      String className = (String)builtinTypeMap.get(cullType);
      if( className == null ) {
         CullObject cullObj = this.cullDef.getCullObject(cullType);
         if (cullObj == null ) {
            throw new IllegalArgumentException("cullType " + cullType + " not found");
         } else  {
            
            switch(cullObj.getType()) {
               case CullObject.TYPE_PRIMITIVE:
               {
                  CullAttr attr = cullObj.getContentAttr();
                  className = getFullClassName(attr.getType());
                  break;
               }
               case CullObject.TYPE_MAPPED:
               {
                  return cullObj.getImplClass();
               }
               default:
               {
                  className = getCullDefPackageName() + "." + getClassName(cullType);
               }
            }
            
         }
      }
      
      if(className.equals("String")) {
         return "java.lang.String";
      } else {
         for(int i = 0; i < PRIMITIVES.length; i++) {
            if( PRIMITIVES[i][1].equals(className)) {
               return "java.lang." + PRIMITIVES[i][1];
            }
         }
      }
      
      return className;
   }

   /**
    * <p>Get the full java class name of a cull type name or the
    * wrapper class name of the cull type is mapped to a primitv
    * datatype</p>
    *
    * <p>Examples:</p>
    *
    * <table border="1">
    *   <tr>
    *     <th>Cull type</th>
    *     <th>Full class name</th>
    *   </tr>
    *   <tr>
    *     <td><code>CA_Type</code></td>
    *     <td><code>com.sun.grid.jgdi.configuration.CA</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_BOOL</code></td>
    *     <td><code>java.lang.Boolean</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_INT</code></td>
    *     <td><code>java.lang.Integer</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_STRING</code></td>
    *     <td><code>java.lang.String</code></td>
    *   </tr>
    * </table>
    *
    * @param cullType  name of the cull type
    * @return the full classname of the the name of the primitiv data type
    */
   public String getFullClassNameOrWrapper(String cullType) {
      String className = getFullClassName(cullType);
      for(int i = 0; i < PRIMITIVES.length; i++) {
         if( PRIMITIVES[i][0].equals(className)) {
            return "java.lang." + PRIMITIVES[i][1];
         }
      }
      return className;
   }
   
   /**
    * <p>Get the java of the java class or java type
    * which represents a cull type.</p>
    *
    * <table border="1">
    *   <tr>
    *     <th>Cull type</th>
    *     <th>class name</th>
    *   </tr>
    *   <tr>
    *     <td><code>CA_Type</code></td>
    *     <td><code>CA</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_BOOL</code></td>
    *     <td><code>boolean</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_INT</code></td>
    *     <td><code>int</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_STRING</code></td>
    *     <td><code>String</code></td>
    *   </tr>
    * </table>
    *
    * @param cullType name of the cull type
    * @return the name of the java class or the java type
    */
   public String getClassName(String cullType) {
      String ret = (String)builtinTypeMap.get(cullType);
      if( ret == null ) {
         CullObject cullObj = cullDef.getCullObject(cullType);
         if( cullObj == null ) {
            return null;
         }
         return getClassName(cullObj);
      }
      if( ret == null ) {
         throw new IllegalArgumentException("cull type " + cullType + " not found");
      }
      return ret;
   }
   /**
    * <p>Get the java of the java impl class or java type
    * which represents a cull type.</p>
    *
    * <table border="1">
    *   <tr>
    *     <th>Cull type</th>
    *     <th>class name</th>
    *   </tr>
    *   <tr>
    *     <td><code>CA_Type</code></td>
    *     <td><code>CAImpl</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_BOOL</code></td>
    *     <td><code>boolean</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_INT</code></td>
    *     <td><code>int</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_STRING</code></td>
    *     <td><code>String</code></td>
    *   </tr>
    * </table>
    *
    * @param cullType name of the cull type
    * @return the name of the java class or the java type
    */
   public String getImplClassName(String cullType) {
      String ret = (String)builtinTypeMap.get(cullType);
      if( ret == null ) {
         CullObject cullObj = cullDef.getCullObject(cullType);
         if( cullObj == null ) {
            return null;
         }
         ret = getClassName(cullObj);
         if (ret.equals("String")) {
            return ret;
         } else {
            return  ret + "Impl";
         }
      }
      if( ret == null ) {
         throw new IllegalArgumentException("cull type " + cullType + " not found");
      }
      return ret;
   }
   
   
   /**
    * <p>Get the java class or java type of a cull type
    * with the suffix ".class" or ".TYPE" if the cull type
    * is mapped to a primitiv java type</p>
    *
    * <table border="1">
    *   <tr>
    *     <th>Cull type</th>
    *     <th>class name</th>
    *   </tr>
    *   <tr>
    *     <td><code>CA_Type</code></td>
    *     <td><code>CA.class</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_BOOL</code></td>
    *     <td><code>Boolean.TYPE</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_INT</code></td>
    *     <td><code>Integer.TYPE</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_STRING</code></td>
    *     <td><code>String.class</code></td>
    *   </tr>
    * </table>
    *
    * @param cullType name of the cull type
    * @return the name of the java class or the java type with ".class" or
    *         ".TYPE" suffix
    */
   public String getClassNameWithSuffix(String cullType) {
      String ret = getClassName(cullType);
      for(int i = 0; i < PRIMITIVES.length; i++) {
         if( PRIMITIVES[i][0].equals(ret)) {
            return PRIMITIVES[i][1] + ".TYPE";
         }
      }
      return ret + ".class";
   }
   
   /**
    * <p>Get the java class or java type of a cull type
    * with the suffix ".class" or ".TYPE" if the cull type
    * is mapped to a primitiv java type</p>
    *
    * <table border="1">
    *   <tr>
    *     <th>Cull type</th>
    *     <th>class name</th>
    *   </tr>
    *   <tr>
    *     <td><code>CA_Type</code></td>
    *     <td><code>CAImpl.class</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_BOOL</code></td>
    *     <td><code>Boolean.TYPE</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_INT</code></td>
    *     <td><code>Integer.TYPE</code></td>
    *   </tr>
    *   <tr>
    *     <td><code>SGE_STRING</code></td>
    *     <td><code>String.class</code></td>
    *   </tr>
    * </table>
    *
    * @param cullType name of the cull type
    * @return the name of the java class or the java type with ".class" or
    *         ".TYPE" suffix
    */
   public String getImplClassNameWithSuffix(String cullType) {
      String ret = getImplClassName(cullType);
      for(int i = 0; i < PRIMITIVES.length; i++) {
         if( PRIMITIVES[i][0].equals(ret)) {
            return PRIMITIVES[i][1] + ".TYPE";
         }
      }
      return ret + ".class";
   }
   
   
   /**
    * <p>Get the java name for a cull attribute.</p>
    *
    * <p>Example:</p>
    *
    * <pre>
    * CQ_suspend_interval -&gt; suspendInterval;
    * </pre>
    * @param attr the cull attribute
    * @return  the java name of the attribute
    */
   public String getAttrName(CullAttr attr) {

      String name = attr.getName();
      
      StringTokenizer st = new StringTokenizer( name, "_" );
      
      if( st.countTokens() > 1 ) {
         StringBuffer buf = new StringBuffer();
         st.nextToken();
         buf.append( st.nextToken() );
         String str = null;
         while( st.hasMoreTokens() ) {
            str = st.nextToken();
            buf.append( Character.toUpperCase(str.charAt(0)) );
            buf.append( str.substring(1) );
         }
         name = buf.toString();
      }
      return name;
      
   }
   
   
   
   /**
    * Determine of a cull attribute is mapped to a primitiv
    * java data type
    * @param attr   the cull attribute
    * @return  <code>true</code> if the cull attribute is mapped
    *          to a primitiv java data type
    */
   public boolean isPrimitiv(CullAttr attr) {
      
      return getPrimitivIndex(attr) >= 0;
   }
   
   public boolean isString(CullAttr attr) {
       return getFullClassName(attr.getType()).equals("java.lang.String");
   }

   private int getPrimitivIndex(CullAttr attr) {
      
      String type = attr.getType();
      String javaType = (String)builtinTypeMap.get(type);
      if( javaType == null ) {
         return -1;
      }
      for(int i = 0; i < PRIMITIVES.length; i++) {
         if( PRIMITIVES[i][0].equals(javaType)) {
            return i;
         }
      }
      return -1;
   }
   
   /**
    * get the default null value for cull attribute
    * @param attr   the cull attribute
    * @return  default null value
    */
   public String getNullValue(CullAttr attr) {
      
      int index = getPrimitivIndex(attr);
      
      if (index >= 0) {
         return PRIMITIVES[index][2];
      }
      return "null";
   }
   
   
   public String getInitializer(CullAttr attr, String value) {

      String ret = value;
      String fullValueClassName = getFullClassNameOrWrapper(attr.getType());
      CullObject obj = cullDef.getCullObject(attr.getType());
      if( fullValueClassName.equals(String.class.getName()) ) {
         ret = "\"" + value + "\"";
      } else if (fullValueClassName.equals(Boolean.class.getName())) {

         if (value.equals("TRUE") || value.equals("true") || value.equals("1") ) {
            ret = "true";
         } else {
            ret = "false";
         }
      } else if(obj != null) {
         if(obj.getPrimaryKeyCount() > 0) {
            CullAttr pkAttr = obj.getPrimaryKeyAttr(0);
            ret = "new " + fullValueClassName + "Impl(" + getInitializer(pkAttr, value) + ")";
         } else {
            ret = "new " + fullValueClassName + "Impl()";
         }
      }
      return ret;
   }
   
}
