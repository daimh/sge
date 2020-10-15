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
package com.sun.grid.jgdi.configuration.xml;

import com.sun.grid.jgdi.configuration.GEObject;
import com.sun.grid.jgdi.configuration.Util;
import com.sun.grid.jgdi.configuration.reflect.ClassDescriptor;
import com.sun.grid.jgdi.configuration.reflect.ListPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.MapListPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.MapPropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.PropertyDescriptor;
import com.sun.grid.jgdi.configuration.reflect.SimplePropertyDescriptor;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.Writer;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.Stack;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import org.xml.sax.InputSource;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.helpers.DefaultHandler;


/**
 * This class implements the serialisation/deserialion of cull object
 * int xml.
 * @jgdi.todo   alpha ??
 *         <p>Implement missing primitive handlers</p>
 */
public class XMLUtil {
    
    private static Logger logger = Logger.getLogger(XMLUtil.class.getName());
    
    public static final String HEADER = "<?xml version='1.0' encoding='UTF-8'?>";
    
    private static final String NONE = "NONE";
    
    
    public static class Context {
        
        private final IndentedPrintWriter p;
        private boolean hideReadOnly;
        private boolean hideConfigurable;
        private boolean hideBrowseable;
        
        public Context(IndentedPrintWriter p) {
            this.p = p;
            setHideReadOnly(true);
            setHideConfigurable(false);
            setHideBrowseable(false);
        }
        
        public Context(Writer wr)  {
            this(new IndentedPrintWriter(wr));
        }
        
        public Context(OutputStream out) {
            this(new IndentedPrintWriter(out));
        }

        public Context(File file) throws IOException {
            this(new IndentedPrintWriter(file));
        }
        
        public boolean include(PropertyDescriptor pd) {
            if(pd.isReadOnly() && hideReadOnly) {
                return false;
            } else if (pd.isConfigurable() && hideConfigurable) {
                return false;
            } else if (pd.isBrowsable() && hideBrowseable) {
                return false;
            }
            return true;
        }

        public boolean isHideReadOnly() {
            return hideReadOnly;
        }

        public void setHideReadOnly(boolean hideReadOnly) {
            this.hideReadOnly = hideReadOnly;
        }

        public boolean isHideConfigurable() {
            return hideConfigurable;
        }

        public void setHideConfigurable(boolean hideConfigurable) {
            this.hideConfigurable = hideConfigurable;
        }

        public boolean isHideBrowseable() {
            return hideBrowseable;
        }

        public void setHideBrowseable(boolean hideBrowseable) {
            this.hideBrowseable = hideBrowseable;
        }
        
    }
    
    public static boolean write(GEObject obj, Context ctx) {
        ctx.p.println(HEADER);
        write(obj, obj.getClass(), ctx);
        ctx.p.close();
        return ctx.p.checkError();
    }
    
    public static boolean write(GEObject obj, Writer wr) throws IOException {
        Context ctx = new Context(wr);
        return write(obj, ctx);
    }

    public static boolean write(GEObject obj, OutputStream out) throws IOException {
        Context ctx = new Context(out);
        return write(obj, ctx);
    }
    
    public static boolean write(GEObject obj, File file) throws IOException {
        Context ctx = new Context(file);
        return write(obj, ctx);
    }
    
    private static void write(GEObject obj, Class clazz, Context ctx) {
        
        ClassDescriptor cd = Util.getDescriptor(clazz);
        ctx.p.print('<');
        ctx.p.print(cd.getCullName());
        ctx.p.print('>');
        if (obj == null) {
            ctx.p.print(NONE);
        } else {
            ctx.p.println();
            ctx.p.indent();
            for(int i = 0; i < cd.getPropertyCount(); i++) {
                PropertyDescriptor pd = cd.getProperty(i);
                
                if (!ctx.include(pd)) {
                    continue;
                }
                if (pd instanceof SimplePropertyDescriptor) {
                    write(obj, (SimplePropertyDescriptor)pd, ctx);
                } else if (pd instanceof ListPropertyDescriptor) {
                    write(obj, (ListPropertyDescriptor)pd, ctx);
                } else if (pd instanceof MapPropertyDescriptor) {
                    write(obj, (MapPropertyDescriptor)pd, ctx);
                } else if (pd instanceof MapListPropertyDescriptor) {
                    write(obj, (MapListPropertyDescriptor)pd, ctx);
                } else {
                    throw new IllegalStateException("Unknown property type " + pd.getClass());
                }
                
            }
            ctx.p.deindent();
        }
        ctx.p.print("</");
        ctx.p.print(cd.getCullName());
        ctx.p.println('>');
        
    }
    
    private static void write(GEObject obj, SimplePropertyDescriptor pd, Context ctx) {
        
        Object value = pd.getValue(obj);
        if (value != null) {
            ctx.p.print('<');
            ctx.p.print(pd.getPropertyName());
            ctx.p.print('>');
            if (GEObject.class.isAssignableFrom(pd.getPropertyType())) {
                if (value == null) {
                    write((GEObject)value, pd.getPropertyType(), ctx);
                } else {
                    write((GEObject)value, value.getClass(), ctx);
                }
            } else {
                ctx.p.print(quoteCharacters(value.toString()));
            }
            ctx.p.print("</");
            ctx.p.print(pd.getPropertyName());
            ctx.p.println('>');
        }
    }
    
    private static void write(GEObject obj, ListPropertyDescriptor pd, Context ctx) {
        
        int count = pd.getCount(obj);
        
        ctx.p.print('<');
        ctx.p.print(pd.getPropertyName());
        ctx.p.print('>');
        if (count == 0) {
            ctx.p.print(NONE);
        } else {
            ctx.p.println();
            ctx.p.indent();
            for(int i = 0; i < count; i++) {
                Object value = pd.get(obj,i);
                if (GEObject.class.isAssignableFrom(pd.getPropertyType())) {
                    if (value == null) {
                        write((GEObject)value, pd.getPropertyType(), ctx);
                    } else {
                        write((GEObject)value, value.getClass(), ctx);
                    }
                } else {
                    writePrimitive(value, pd.getPropertyType(), ctx);
                }
            }
            ctx.p.deindent();
        }
        ctx.p.print("</");
        ctx.p.print(pd.getPropertyName());
        ctx.p.println('>');
    }
    
    private static void write(GEObject obj, MapPropertyDescriptor pd, Context ctx) {
        
        Set keys = pd.getKeys(obj);
        
        Iterator keyIter = keys.iterator();
        while(keyIter.hasNext()) {
            Object key = keyIter.next();
            Object value = pd.get(obj,key);
            ctx.p.print('<');
            ctx.p.print(pd.getPropertyName());
            ctx.p.print(" key='");
            ctx.p.print(quoteCharacters(key.toString()));
            ctx.p.print("'>");
            if (GEObject.class.isAssignableFrom(pd.getPropertyType())) {
                ctx.p.println();
                ctx.p.indent();
                if (value == null) {
                    write((GEObject)value, pd.getPropertyType(), ctx);
                } else {
                    write((GEObject)value, value.getClass(), ctx);
                }
                ctx.p.deindent();
            } else {
                if (value == null) {
                    ctx.p.print(NONE);
                } else {
                    ctx.p.print(quoteCharacters(value.toString()));
                }
            }
            ctx.p.print("</");
            ctx.p.print(pd.getPropertyName());
            ctx.p.println('>');
        }
    }
    
    public static final String STRING_TAG = "string";
    public static final String BOOLEAN_TAG = "boolean";
    public static final String INT_TAG = "int";
    public static final String SHORT_TAG = "short";
    public static final String LONG_TAG = "long";
    public static final String DOUBLE_TAG = "double";
    public static final String FLOAT_TAG = "float";
    
    private static void writePrimitive(Object value, Class clazz, Context ctx) {
        
        String tag = null;
        if (String.class.isAssignableFrom(clazz)) {
            tag = STRING_TAG;
        } else if (Boolean.TYPE.isAssignableFrom(clazz) ||
                Boolean.class.isAssignableFrom(clazz)) {
            tag = BOOLEAN_TAG;
        } else if (Integer.TYPE.isAssignableFrom(clazz) ||
                Integer.class.isAssignableFrom(clazz)) {
            tag = INT_TAG;
        } else if (Short.TYPE.isAssignableFrom(clazz) ||
                Short.class.isAssignableFrom(clazz)) {
            tag = SHORT_TAG;
        } else if (Long.TYPE.isAssignableFrom(clazz) ||
                Long.class.isAssignableFrom(clazz)) {
            tag = LONG_TAG;
        } else if (Double.TYPE.isAssignableFrom(clazz) ||
                Double.class.isAssignableFrom(clazz)) {
            tag = DOUBLE_TAG;
        } else if (Float.TYPE.isAssignableFrom(clazz) ||
                Float.class.isAssignableFrom(clazz)) {
            tag = FLOAT_TAG;
        } else {
            throw new IllegalArgumentException("Unknown primitive type " + clazz.getName());
        }
        ctx.p.print("<");
        ctx.p.print(tag);
        ctx.p.print('>');
        if (value == null) {
            if (String.class.isAssignableFrom(clazz)) {
                ctx.p.print(NONE);
            } else {
                throw new IllegalArgumentException("Don't know howto handle null value for type " + clazz.getName());
            }
        } else {
            ctx.p.print(quoteCharacters(value.toString()));
        }
        ctx.p.print("</");
        ctx.p.print(tag);
        ctx.p.println('>');
        
    }
    
    private static void write(GEObject obj, MapListPropertyDescriptor pd, Context ctx) {
        
        Set keys = pd.getKeys(obj);
        
        Iterator keyIter = keys.iterator();
        while(keyIter.hasNext()) {
            Object key = keyIter.next();
            ctx.p.print('<');
            ctx.p.print(pd.getPropertyName());
            ctx.p.print(" key='");
            ctx.p.print(quoteCharacters(key.toString()));
            ctx.p.print("'>");
            int count = pd.getCount(obj, key);
            if (count == 0) {
                ctx.p.println();
                ctx.p.indent();
                if (GEObject.class.isAssignableFrom(pd.getPropertyType())) {
                    write((GEObject)null, pd.getPropertyType(), ctx);
                } else {
                    writePrimitive(null, pd.getPropertyType(), ctx);
                }
                ctx.p.deindent();
            } else {
                ctx.p.println();
                ctx.p.indent();
                for(int i = 0; i < count; i++) {
                    Object value = pd.get(obj, key, i);
                    if (GEObject.class.isAssignableFrom(pd.getPropertyType())) {
                        if (value == null) {
                            write((GEObject)value, pd.getPropertyType(), ctx);
                        } else {
                            write((GEObject)value, value.getClass(), ctx);
                        }
                    } else {
                        writePrimitive(value, pd.getPropertyType(), ctx);
                    }
                }
                ctx.p.deindent();
            }
            ctx.p.print("</");
            ctx.p.print(pd.getPropertyName());
            ctx.p.println('>');
        }
    }
    
    private static String quoteCharacters(String s) {
        StringBuilder result = null;
        for(int i = 0, max = s.length(), delta = 0; i < max; i++) {
            char c = s.charAt(i);
            String replacement = null;
            
            if (c == '&') {
                replacement = "&amp;";
            } else if (c == '<') {
                replacement = "&lt;";
            } else if (c == '\r') {
                replacement = "&#13;";
            } else if (c == '>') {
                replacement = "&gt;";
            } else if (c == '"') {
                replacement = "&quot;";
            } else if (c == '\'') {
                replacement = "&apos;";
            }
            
            if (replacement != null) {
                if (result == null) {
                    result = new StringBuilder(s);
                }
                result.replace(i + delta, i + delta + 1, replacement);
                delta += (replacement.length() - 1);
            }
        }
        if (result == null) {
            return s;
        }
        return result.toString();
    }
    
    
    /**
     * Read a XML definition of a gridengine object from an <code>InputStream</code>.
     *
     * @param in   the InputStream
     * @param properties All ${key} expressions in the xml file will be replaced be
     *                   the corresponding value from the properties
     * @throws java.io.IOException   on any I/O Error
     * @throws javax.xml.parsers.ParserConfigurationException if a SAX parser has an invalid configuration
     * @throws org.xml.sax.SAXException on any parse error
     * @return the gridengine object
     */
    public static Object read(InputStream in, Map properties)  throws IOException, ParserConfigurationException, SAXException {
        // Use an instance of ourselves as the SAX event handler
        RootHandler handler = new RootHandler(properties);
        
        // Use the default (non-validating) parser
        SAXParserFactory factory = SAXParserFactory.newInstance();
        
        // Parse the input
        SAXParser saxParser = factory.newSAXParser();
        
        saxParser.parse(in, handler);
        
        return handler.getObject();
    }
    
    /**
     * Read a XML definition of a gridengine object from an <code>InputStream</code>.
     *
     * @param in   the InputStream
     * @throws java.io.IOException   on any I/O Error
     * @throws javax.xml.parsers.ParserConfigurationException if a SAX parser has an invalid configuration
     * @throws org.xml.sax.SAXException on any parse error
     * @return the gridengine object
     */
    public static Object read(InputStream in)  throws IOException, ParserConfigurationException, SAXException {
        return read(in, null);
    }
    
    /**
     * Read a XML definition of a gridengine object from a <code>File</code>.
     *
     * @param file   the file
     * @param properties All ${key} expressions in the xml file will be replaced be
     *                   the corresponding value from the properties
     * @throws java.io.IOException   on any I/O Error
     * @throws javax.xml.parsers.ParserConfigurationException if a SAX parser has an invalid configuration
     * @throws org.xml.sax.SAXException on any parse error
     * @return the gridengine object
     */
    public static Object read(File file, Map properties) throws IOException, ParserConfigurationException, SAXException {
        // Use an instance of ourselves as the SAX event handler
        RootHandler handler = new RootHandler(properties);
        
        // Use the default (non-validating) parser
        SAXParserFactory factory = SAXParserFactory.newInstance();
        
        // Parse the input
        SAXParser saxParser = factory.newSAXParser();
        
        saxParser.parse(file, handler);
        
        return handler.getObject();
    }
    
    /**
     * Read a XML definition of a gridengine object from a <code>File</code>.
     *
     * @param file   the file
     * @throws java.io.IOException   on any I/O Error
     * @throws javax.xml.parsers.ParserConfigurationException if a SAX parser has an invalid configuration
     * @throws org.xml.sax.SAXException on any parse error
     * @return the gridengine object
     */
    public static Object read(File file) throws IOException, ParserConfigurationException, SAXException {
        return read(file, null);
    }
    
    /**
     * Read a XML definition of a gridengine object from a <code>Reader</code>.
     *
     * @param  rd  the reader
     * @throws java.io.IOException   on any I/O Error
     * @throws javax.xml.parsers.ParserConfigurationException if a SAX parser has an invalid configuration
     * @throws org.xml.sax.SAXException on any parse error
     * @return the gridengine object
     */
    public static Object read(Reader rd, Map properties) throws IOException, ParserConfigurationException, SAXException {
        
        // Use an instance of ourselves as the SAX event handler
        RootHandler handler = new RootHandler(properties);
        
        // Use the default (non-validating) parser
        SAXParserFactory factory = SAXParserFactory.newInstance();
        
        // Parse the input
        SAXParser saxParser = factory.newSAXParser();
        InputSource source = new InputSource(rd);
        saxParser.parse(source, handler);
        
        return handler.getObject();
    }
    
    /**
     * Read a XML definition of a gridengine object from a <code>Reader</code>.
     *
     * @param  rd  the reader
     * @throws java.io.IOException   on any I/O Error
     * @throws javax.xml.parsers.ParserConfigurationException if a SAX parser has an invalid configuration
     * @throws org.xml.sax.SAXException on any parse error
     * @return the gridengine object
     */
    public static Object read(Reader rd) throws IOException, ParserConfigurationException, SAXException {
        return read(rd, null);
    }
    
    
    static class RootHandler extends DefaultHandler {
        
        private Map properties;
        
        private Stack stack = new Stack();
        
        private Object rootObject;
        
        private Locator locator;
        
        public RootHandler() {
            this(null);
        }
        
        public RootHandler(Map properties) {
            this.properties = properties;
        }
        
        public Object getObject() {
            return rootObject;
        }
        
        public void setDocumentLocator(Locator locator) {
            this.locator = locator;
        }
        
        public void startElement(String uri, String localName, String qName, org.xml.sax.Attributes attributes) throws org.xml.sax.SAXException {
            
            if (logger.isLoggable(Level.FINEST)) {
                logger.finest("startElement: uri = " + uri + ", localName = " + localName +
                        " qName = " + qName);
            }
            if (stack.isEmpty()) {
                GEObjectHandler handler = new GEObjectHandler(qName);
                rootObject = handler.getObject();
                stack.push(handler);
            } else {
                CullHandler handler = (CullHandler)stack.peek();
                
                handler = handler.getHandler(qName, attributes);
                stack.push(handler);
            }
        }
        
        public void endElement(String uri, String localName, String qName) throws SAXException {
            
            if (logger.isLoggable(Level.FINEST)) {
                logger.finest("endElement: uri = " + uri + ", localName = " + localName +
                        " qName = " + qName);
            }
            CullHandler handler = (CullHandler)stack.pop();
            handler.endElement(uri, localName, qName);
        }
        
        public void characters(char[] ch, int start, int length) throws org.xml.sax.SAXException {
            
            if (logger.isLoggable(Level.FINEST)) {
                logger.finest("characters: '" + new String(ch, start, length) + "'");
            }
            
            if (properties != null) {
                ch = resolveResources(ch, start, length);
                start = 0;
                length = ch.length;
            }
            CullHandler handler = (CullHandler)stack.peek();
            handler.characters(ch, start, length);
        }
        
        char[] resolveResources(char[] ch, int start, int length) throws SAXException {
            int end = start + length;
            int i = start;
            StringBuilder ret = new StringBuilder();
            
            while(i < end) {
                if (ch[i] == '$') {
                    i++;
                    if (i>=end) {
                        ret.append('$');
                        break;
                    }
                    if (ch[i] == '{') {
                        // we found the beginning of a property
                        i++;
                        if (i>=end) {
                            throw new SAXException("Unclosed property in " + new String(ch, start, length));
                        }
                        int startIndex = i;
                        int endIndex = -1;
                        while(i< end) {
                            if (ch[i] == '}') {
                                endIndex = i;
                                break;
                            }
                            i++;
                        }
                        if (endIndex < 0) {
                            throw new SAXException("Unclosed property in " + new String(ch, start, length));
                        }
                        String property = new String(ch, startIndex, endIndex - startIndex);
                        
                        Object value = properties.get(property);
                        
                        logger.fine("Replace property " + property + " with value " + value);
                        if (value != null) {
                            ret.append(value);
                        }
                    } else {
                        // A single $ sign
                        ret.append('$');
                        ret.append(ch[i]);
                    }
                } else {
                    ret.append(ch[i]);
                }
                i++;
            }
            return ret.toString().toCharArray();
        }
        
        abstract class CullHandler extends DefaultHandler {
            
            protected CullHandler parent;
            
            public CullHandler(CullHandler parent) {
                this.parent = parent;
            }
            
            public CullHandler getParent() {
                return parent;
            }
            
            public abstract CullHandler getHandler(String name, org.xml.sax.Attributes attributes) throws SAXException;
        }
        
        abstract class AbstractObjectHandler extends CullHandler {
            
            private String name;
            
            public AbstractObjectHandler(CullPropertyHandler parent, String name) {
                super(parent);
                this.name = name;
            }
            
            public String getName() {
                return name;
            }
            
            public abstract Object getObject();
            
            public void endElement(String uri, String localName, String qName) throws SAXException {
                if (parent != null) {
                    ((CullPropertyHandler)parent).addObject(getObject());
                }
            }
            
        }
        
        class GEObjectHandler extends AbstractObjectHandler {
            
            private ClassDescriptor cd;
            private Object obj;
            
            public GEObjectHandler(String name)  throws SAXException {
                this(null, name);
            }
            
            public Object getObject() {
                return obj;
            }
            
            public GEObjectHandler(CullPropertyHandler parent, String name)  throws SAXException {
                super(parent, name);
                try {
                    cd = Util.getDescriptorForCullType(name);
                } catch(IllegalArgumentException ilae) {
                    throw new SAXParseException("No descriptor for cull type " + name + " found", locator, ilae);
                }
                obj = cd.newInstance();
            }
            
            public CullHandler getHandler(String name, org.xml.sax.Attributes attributes) throws SAXException {
                PropertyDescriptor pd = cd.getProperty(name);
                if (pd == null) {
                    throw new SAXParseException("cull type " + cd.getCullName() + " has no property " + name, locator);
                } if (pd instanceof SimplePropertyDescriptor) {
                    if (pd.getPropertyType().isPrimitive() || pd.getPropertyType().equals(String.class)) {
                        return new SimplePropertyHandler(this, (SimplePropertyDescriptor)pd);
                    } else {
                        return new ObjectPropertyHandler(this, (SimplePropertyDescriptor)pd);
                    }
                } else if (pd instanceof ListPropertyDescriptor) {
                    return new ListPropertyHandler(this, (ListPropertyDescriptor)pd);
                } else if (pd instanceof MapPropertyDescriptor) {
                    return new MapPropertyHandler(this, (MapPropertyDescriptor)pd, attributes);
                } else if (pd instanceof MapListPropertyDescriptor) {
                    return new MapListPropertyHandler(this, (MapListPropertyDescriptor)pd, attributes);
                } else {
                    throw new SAXParseException("Unknown property type " + pd.getClass(), locator);
                }
            }
            
            private StringBuilder value;
            
            public void characters(char[] ch, int start, int length) throws SAXException {
                if (length > 0) {
                    if (value == null) {
                        value = new StringBuilder();
                    }
                    value.append(ch,start,length);
                }
            }
            
            public void endElement(String uri, String localName, String qName) throws SAXException {
                if (value != null) {
                    String str = value.toString().trim();
                    if (str.length()>0) {
                        if (NONE.equalsIgnoreCase(str)) {
                            if (parent instanceof MapListPropertyHandler) {
                                ((MapListPropertyHandler)parent).addEmptyObject();
                            } else if (parent instanceof ListPropertyHandler) {
                                // empty list and null has the same meaning
                            } else {
                                throw new SAXException("object can only have content if parent is a map list or a list (2)");
                            }
                        } else {
                            super.endElement(uri, localName, qName);
                        }
                    } else {
                        super.endElement(uri, localName, qName);
                    }
                }
            }
        }
        
        class StringHandler extends AbstractObjectHandler {
            
            private StringBuilder buffer;
            
            public StringHandler(CullPropertyHandler parent, String name) {
                super(parent, name);
            }
            
            public CullHandler getHandler(String name, org.xml.sax.Attributes attributes) throws SAXException {
                throw new SAXException("String handler " + getName() + " does not support sub element " + name);
            }
            
            public void characters(char[] ch, int start, int length) throws SAXException {
                if (buffer == null) {
                    buffer = new StringBuilder();
                }
                buffer.append(ch,start, length);
            }
            
            public void endElement(String uri, String localName, String qName) throws SAXException {
                if (buffer == null) {
                    throw new SAXException("A string tag must not be empty");
                } else {
                    super.endElement(uri, localName, qName);
                }
            }
            
            public Object getObject() {
                if (buffer != null) {
                    String ret = buffer.toString();
                    if (NONE.equalsIgnoreCase(ret)) {
                        return null;
                    } else {
                        return ret;
                    }
                } else {
                    throw new IllegalStateException("A string tag must not be empty");
                }
            }
        }
        
        class IntHandler extends StringHandler {
            public IntHandler(CullPropertyHandler parent, String name) {
                super(parent, name);
            }
            
            public Object getObject() {
                return new Integer((String)super.getObject());
            }
        }
        
        class LongHandler extends StringHandler {
            public LongHandler(CullPropertyHandler parent, String name) {
                super(parent, name);
            }
            
            public Object getObject() {
                return new Long((String)super.getObject());
            }
        }
        
        class ShortHandler extends StringHandler {
            public ShortHandler(CullPropertyHandler parent, String name) {
                super(parent, name);
            }
            
            public Object getObject() {
                return new Short((String)super.getObject());
            }
        }
        
        class DoubleHandler extends StringHandler {
            public DoubleHandler(CullPropertyHandler parent, String name) {
                super(parent, name);
            }
            
            public Object getObject() {
                return new Double((String)super.getObject());
            }
        }
        
        class FloatHandler extends StringHandler {
            public FloatHandler(CullPropertyHandler parent, String name) {
                super(parent, name);
            }
            
            public Object getObject() {
                return new Float((String)super.getObject());
            }
        }
        
        class BooleanHandler extends StringHandler {
            public BooleanHandler(CullPropertyHandler parent, String name) {
                super(parent, name);
            }
            
            public Object getObject() {
                return new Boolean((String)super.getObject());
            }
        }
        
        
        abstract class CullPropertyHandler extends CullHandler {
            
            
            public CullPropertyHandler(GEObjectHandler parent) {
                super(parent);
                if (parent == null) {
                    throw new NullPointerException("parent is null");
                }
            }
            
            public abstract void addObject(Object obj) throws SAXException;
            
            public CullHandler getHandler(String name, org.xml.sax.Attributes attributes) throws SAXException {
                if (STRING_TAG.equals(name)) {
                    return new StringHandler(this, name);
                } else if (DOUBLE_TAG.equals(name)) {
                    return new DoubleHandler(this, name);
                } else if (BOOLEAN_TAG.equals(name)) {
                    return new BooleanHandler(this, name);
                } else if (INT_TAG.equals(name)) {
                    return new IntHandler(this, name);
                } else if (LONG_TAG.equals(name)) {
                    return new LongHandler(this, name);
                } else if (SHORT_TAG.equals(name)) {
                    return new ShortHandler(this,name);
                } else if (FLOAT_TAG.equals(name)) {
                    return new FloatHandler(this, name);
                }
                return new GEObjectHandler(this, name);
            }
            
        }
        
        class SimplePropertyHandler extends CullPropertyHandler {
            private SimplePropertyDescriptor pd;
            private StringBuilder value;
            
            public SimplePropertyHandler(GEObjectHandler parent, SimplePropertyDescriptor pd) {
                super(parent);
                if (pd == null) {
                    throw new NullPointerException("pd is null");
                }
                this.pd = pd;
            }
            
            public void addObject(Object obj) throws SAXException {
                if (!pd.isReadOnly()) {
                    if (((GEObjectHandler)parent).getObject() == null) {
                        throw new SAXException("parent " + ((GEObjectHandler)parent).getName() + " has no value");
                    }
                    pd.setValue(((GEObjectHandler)parent).getObject(), obj);
                }
            }
            
            public void characters(char[] ch, int start, int length) throws SAXException {
                if (value == null) {
                    value = new StringBuilder();
                }
                value.append(ch,start,length);
            }
            
            public void endElement(String uri, String localName, String qName) throws SAXException {
                
                if (value != null) {
                    // We have a simple element
                    String str = value.toString();
                    addObject(parse(str, pd));
                }
            }
        }
        
        class ObjectPropertyHandler extends CullPropertyHandler {
            private SimplePropertyDescriptor pd;
            
            public ObjectPropertyHandler(GEObjectHandler parent, SimplePropertyDescriptor pd) {
                super(parent);
                this.pd = pd;
            }
            
            public void addObject(Object obj) throws SAXException {
                if (((GEObjectHandler)parent).getObject() == null) {
                    throw new SAXException("parent " + ((GEObjectHandler)parent).getName() + " has not object");
                }
                pd.setValue(((GEObjectHandler)parent).getObject(), obj);
            }
        }
        
        class ListPropertyHandler extends CullPropertyHandler {
            private ListPropertyDescriptor pd;
            
            public ListPropertyHandler(GEObjectHandler parent, ListPropertyDescriptor pd) {
                super(parent);
                this.pd = pd;
            }
            
            public void addObject(Object obj) throws SAXException {
                if (((GEObjectHandler)parent).getObject() == null) {
                    throw new SAXException("parent " + ((GEObjectHandler)parent).getName() + " has not object");
                }
                pd.add(((GEObjectHandler)parent).getObject(), obj);
            }
        }
        
        class MapPropertyHandler extends CullPropertyHandler {
            private MapPropertyDescriptor pd;
            private StringBuilder value;
            private String key;
            
            public MapPropertyHandler(GEObjectHandler parent, MapPropertyDescriptor pd, org.xml.sax.Attributes attributes) {
                super(parent);
                this.pd = pd;
                this.key = attributes.getValue("key");
            }
            
            public CullHandler getHandler(String name, org.xml.sax.Attributes attributes)  throws SAXException {
                if (!pd.getCullType().equals(name)) {
                    throw new SAXParseException("This handler can only handle object of type " + pd.getCullType(), locator);
                }
                return super.getHandler(name, attributes);
            }
            
            public void addObject(Object obj) throws SAXException {
                if (((GEObjectHandler)parent).getObject() == null) {
                    throw new SAXException("parent " + ((GEObjectHandler)parent).getName() + " has no object");
                }
                /* AA */
                if (obj == null) {
                    pd.put(((GEObjectHandler)parent).getObject(), key, "NONE");
                } else {
                    pd.put(((GEObjectHandler)parent).getObject(), key, obj);
                }
            }
            
            public void characters(char[] ch, int start, int length) throws SAXException {
                if (value == null) {
                    value = new StringBuilder();
                }
                value.append(ch,start,length);
            }
            
            public void endElement(String uri, String localName, String qName) throws SAXException {
                if (value != null) {
                    // We have a simple element
                    String str = value.toString();
                    if (NONE.equalsIgnoreCase(str)) {
                        addObject(null);
                    } else {
                        addObject(parse(value.toString(), pd));
                    }
                }
            }
        }
        
        class MapListPropertyHandler extends CullPropertyHandler {
            private MapListPropertyDescriptor pd;
            private String key;
            private StringBuilder value;
            
            public MapListPropertyHandler(GEObjectHandler parent, MapListPropertyDescriptor pd, org.xml.sax.Attributes attributes) {
                super(parent);
                this.pd = pd;
                this.key = attributes.getValue("key");
            }
            
            public CullHandler getHandler(String name, org.xml.sax.Attributes attributes)  throws SAXException {
                return super.getHandler(name, attributes);
            }
            
            public void addObject(Object obj) throws SAXException {
                if (pd == null) {
                    throw new NullPointerException("pd is null");
                }
                if (parent == null) {
                    throw new NullPointerException("parent is null");
                }
                if (((GEObjectHandler)parent).getObject() == null) {
                    throw new SAXException("parent " + ((GEObjectHandler)parent).getName() + " has not object");
                }
                if (obj != null) {
                    pd.add(((GEObjectHandler)parent).getObject(), key, obj);
                } else {
                    addEmptyObject();
                }
                
            }
            
            public void addEmptyObject() throws SAXException {
                if (pd == null) {
                    throw new NullPointerException("pd is null");
                }
                if (parent == null) {
                    throw new NullPointerException("parent is null");
                }
                if (((GEObjectHandler)parent).getObject() == null) {
                    throw new SAXException("parent " + ((GEObjectHandler)parent).getName() + " has no object");
                }
                pd.addEmpty(((GEObjectHandler)parent).getObject(), key);
            }
            
        }
        
        
        /**
         * parse methode for primitive and string elements
         * @param value  the value which should be parsed
         * @param PropertyDescriptor  descriptor of the property
         * @throws org.xml.sax.SAXException the value could not be parsed
         * @return the parsed value
         */
        private Object parse(String value, PropertyDescriptor pd) throws SAXException {
            Class clazz = pd.getPropertyType();
            if (pd.hasCullWrapper()) {
                
                ClassDescriptor realClassDescriptor = Util.getDescriptor(pd.getPropertyType());
                
                Object obj = realClassDescriptor.newInstance();
                
                PropertyDescriptor rpd = realClassDescriptor.getPropertyByCullFieldName(pd.getCullContentField());
                
                if (rpd instanceof SimplePropertyDescriptor) {
                    ((SimplePropertyDescriptor)rpd).setValue(obj, parse(value, rpd));
                    return obj;
                } else {
                    throw new SAXParseException("Can only handle simple wrapped properties", locator);
                }
            } else if (Boolean.TYPE.isAssignableFrom(clazz)) {
                return new Boolean(value);
            } else if (Integer.TYPE.isAssignableFrom(clazz)) {
                try {
                    return new Integer(value);
                } catch(NumberFormatException nfe) {
                    throw new SAXParseException("'" + value + "' is not a valid int value", locator, nfe);
                }
            } else if (Long.TYPE.isAssignableFrom(clazz)) {
                try {
                    return new Long(value);
                } catch(NumberFormatException nfe) {
                    throw new SAXParseException("'" + value + "' is not a valid long value", locator, nfe);
                }
            } else if (Float.TYPE.isAssignableFrom(clazz)) {
                try {
                    return new Float(value);
                } catch(NumberFormatException nfe) {
                    throw new SAXParseException("'" + value + "' is not a valid float value", locator, nfe);
                }
            } else if (Double.TYPE.isAssignableFrom(clazz)) {
                try {
                    return new Double(value);
                } catch(NumberFormatException nfe) {
                    throw new SAXParseException("'" + value + "' is not a valid double value", locator, nfe);
                }
            } else if (String.class.isAssignableFrom(clazz)) {
                return value;
            } else {
                throw new SAXParseException("Can not parse object of type " + clazz, locator);
            }
        }
    }
}
