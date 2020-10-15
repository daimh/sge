package com.sun.grid.jgdi.util;

import java.beans.BeanInfo;
import java.beans.IntrospectionException;
import java.beans.Introspector;
import java.beans.PropertyDescriptor;
import java.io.PrintWriter;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.LinkedList;
import java.util.List;
import java.text.DecimalFormat;
import java.text.Format;

public class OutputTable {
    
    private List<Column> columns = new LinkedList<Column>();
    private Class clazz;
    int rowWidth = -1;
    
    public static final DecimalFormat DEFAULT_NUMBER_FORMAT = new DecimalFormat("#####0");
    public static final DecimalFormat POINT_FIVE_FORMAT = new DecimalFormat("####0.00000");
    
    
    public OutputTable(Class clazz) {
        this.clazz = clazz;
    }
    
    public Column addCol(String name, String header) throws IntrospectionException {
        return addCol(name, header, 0);
    }
    
    public Column addCol(String name, String header, int width) throws IntrospectionException {
        Column col = new PropertyColumn(name, header, width);
        addCol(col);
        return col;
    }
    
    public Column addCol(String name, String header, int width, int alignment) throws IntrospectionException {
        Column col = new PropertyColumn(name, header, width, alignment);
        addCol(col);
        return col;
    }
    
    public Column addCol(String name, String header, int width, int alignment, Calc calc) throws IntrospectionException {
        Column col = new CalcColumn(name, header, width, alignment, calc);
        addCol(col);
        return col;
    }
    
    public Column addCol(String name, String header, int width, Format format) throws IntrospectionException {
        Column col = new PropertyColumn(name, header, width, format);
        addCol(col);
        return col;
    }
    
    public Column addCol(String name, String header, int width, Format format, Calc calc) throws IntrospectionException {
        Column col = new CalcColumn(name, header, width, format, calc);
        addCol(col);
        return col;
    }
    
    public Column addCol(String name, String header, int width, Calc calc) throws IntrospectionException {
        Column col = new CalcColumn(name, header, width, calc);
        addCol(col);
        return col;
    }
    
    public void addCol(Column col) {
        columns.add(col);
        rowWidth = -1;
    }
    
    public void printHeader(PrintWriter pw) {
        for (Column col : columns) {
            col.printStr(pw, col.getHeader());
            pw.print(' ');
        }
        pw.println();
    }
    
    public int getRowWidth() {
        if (rowWidth < 0) {
            int ret = 0;
            for (Column col : columns) {
                ret += col.getWidth();
                // +1 for every space, for last column we subtract it
                ret++;
            }
            rowWidth = --ret;
        }
        return rowWidth;
    }
    
    public void printRow(PrintWriter pw, Object obj) {
        for (Column col : columns) {
            col.print(pw, obj);
            pw.print(' ');
        }
        pw.println();
    }
    
    public void printDelimiter(PrintWriter pw, char del) {
        for (int i = 0; i < getRowWidth(); i++) {
            pw.print(del);
        }
        pw.println();
    }
    
    public abstract class Column {
        
        public static final int DEFAULT_COL_WIDTH = 10;
        
        String name;
        private String header;
        private Format format;
        private int width;
        
        public static final int RIGHT = 0;
        public static final int CENTER = 1;
        public static final int LEFT = 2;
        
        
        private int alignment = LEFT;
        private Method getter = null;
        
        public Column(String name, String header) {
            this(name, header, DEFAULT_COL_WIDTH);
        }
        
        public Column(String name, String header, int width) {
            this(name, header, width, LEFT);
        }
        
        public Column(String name, String header, int width, int alignment) {
            this(name, header, width, alignment, null);
        }
        
        public Column(String name, String header, Format format) {
            this(name, header);
            this.setFormat(format);
        }
        
        public Column(String name, String header, int width, int alignment, Format format) {
            this.name = name;
            this.setHeader(header);
            this.setWidth(width);
            this.alignment = alignment;
            this.setFormat(format);
        }
        
        public String getName() {
            return name;
        }
        
        public abstract Object getValue(Object obj);
        
        
        public void printStr(PrintWriter pw, String str) {
            
            int spaceCount = getWidth() - str.length();
            
            if (spaceCount == 0) {
                pw.print(str);
            } else {
                switch(getAlignment()) {
                    case RIGHT:
                        
                        if (spaceCount > 0) {
                            for (int i = 0; i < spaceCount; i++) {
                                pw.print(' ');
                            }
                            pw.print(str);
                        }  else { /* spaceCount < 0 */
                            str = str.substring(Math.abs(spaceCount));
                            pw.print(str);
                        }
                        
                        break;
                    case CENTER:
                        
                        if (spaceCount > 0) {
                            int prefix = spaceCount / 2;
                            for (int i = 0; i < prefix; i++) {
                                pw.print(' ');
                            }
                            pw.print(str);
                            spaceCount -= prefix;
                            for (int i = 0; i < spaceCount; i++) {
                                pw.print(' ');
                            }
                        }  else {
                            str = str.substring(Math.abs(spaceCount));
                            pw.print(str);
                        }
                    case LEFT:
                        
                        if (spaceCount > 0) {
                            pw.print(str);
                            for (int i = 0; i < spaceCount; i++) {
                                pw.print(' ');
                            }
                        }  else {
                            str = str.substring(0, str.length()  - Math.abs(spaceCount));
                            pw.print(str);
                        }
                }
            }
        }
        
        
        public void print(PrintWriter pw, Object obj) {
            Object value = getValue(obj);
            String str = null;
            if (value == null) {
                str = "";
            } else {
                if (getFormat() == null) {
                    str = value.toString();
                } else if (value.equals("-NA-")) {
                    str = value.toString();                    
                } else {
                    try {
                        str = getFormat().format(value);
                    } catch (RuntimeException e) {
                        System.err.println("format error in colum " + getName() + " formatter can not format value " + value);
                        throw e;
                    }
                }
            }
            printStr(pw, str);
        }
        
        public int getAlignment() {
            return alignment;
        }
        
        public void setAlignment(int alignment) {
            this.alignment = alignment;
        }
        
        public Format getFormat() {
            return format;
        }
        
        public void setFormat(Format format) {
            this.format = format;
        }
        
        public String getHeader() {
            return header;
        }
        
        public void setHeader(String header) {
            this.header = header;
        }
        
        public int getWidth() {
            return width;
        }
        
        public void setWidth(int width) {
            this.width = width;
        }
    }
    
    class PropertyColumn extends Column {
        
        private Method getter = null;
        
        public PropertyColumn(String name, String header) {
            this(name, header, DEFAULT_COL_WIDTH);
        }
        
        public PropertyColumn(String name, String header, int width) {
            this(name, header, width, LEFT);
        }
        
        public PropertyColumn(String name, String header, int width, int alignment) {
            this(name, header, width, alignment, null);
        }
        
        public PropertyColumn(String name, String header, Format format) {
            this(name, header);
            this.setFormat(format);
        }
        
        public PropertyColumn(String name, String header, int width, Format format) {
            this(name, header, width);
            this.setFormat(format);
        }
        
        public PropertyColumn(String name, String header, int width, int alignment, Format format) {
            super(name, header, width, alignment, format);
            BeanInfo beanInfo = null;
            
            Class aClass = clazz;
            
            outer:
                while (getter == null && aClass != null) {
                    getter = getGetter(name, aClass);
                    if (getter == null) {
                        Class[] interfaces = aClass.getInterfaces();
                        for (int i = 0; i < interfaces.length; i++) {
                            getter = getGetter(name, interfaces[i]);
                            if(getter != null) {
                                break outer;
                            }
                        }
                        aClass = aClass.getSuperclass();
                    }
                }
                if (getter == null) {
                    throw new IllegalStateException("getter for " + name + " not found in class " + clazz.getName());
                }
        }
        
        private Method getGetter(String name, Class aClass) {
            BeanInfo beanInfo = null;
            try {
                beanInfo = Introspector.getBeanInfo(aClass);
            }  catch (IntrospectionException ex) {
                IllegalStateException ex1 = new IllegalStateException("Can not introspec class " + clazz.getName());
                ex1.initCause(ex);
                throw ex1;
            }
            PropertyDescriptor[] props = beanInfo.getPropertyDescriptors();
            for (int i = 0; i < props.length; i++) {
                if (props[i].getName().equalsIgnoreCase(name)) {
                    getter = props[i].getReadMethod();
                    if (getter == null) {
                        throw new IllegalStateException("property " + name + " has not read method");
                    }
                    return getter;
                }
            }
            return null;
        }
        
        
        public Object getValue(Object obj) {
            try {
                return getter.invoke(obj, (java.lang.Object[])null);
            } catch (IllegalAccessException ex) {
                IllegalStateException ex1 = new IllegalStateException("No access in property " + clazz.getName() + "." + getName());
                ex1.initCause(ex);
                throw ex1;
            }  catch (InvocationTargetException ex) {
                IllegalStateException ex1 = new IllegalStateException("Error in getter " + clazz.getName() + "." + getName());
                ex1.initCause(ex.getTargetException());
                throw ex1;
            }
        }
        
    }
    
    
    class CalcColumn extends Column {
        
        private Calc calc;
        
        public CalcColumn(String name, String header, Calc calc) {
            this(name, header, DEFAULT_COL_WIDTH, calc);
        }
        
        public CalcColumn(String name, String header, int width, Calc calc) {
            this(name, header, width, LEFT, calc);
        }
        
        public CalcColumn(String name, String header, int width, int alignment, Calc calc) {
            this(name, header, width, alignment, null, calc);
        }
        
        public CalcColumn(String name, String header, Format format, Calc calc) {
            this(name, header, calc);
            this.setFormat(format);
        }
        
        public CalcColumn(String name, String header, int width, Format format, Calc calc) {
            super(name, header, width);
            setFormat(format);
            this.calc = calc;
        }
        
        public CalcColumn(String name, String header, int width, int alignment, Format format, Calc calc) {
            super(name, header, width, alignment, format);
            this.calc = calc;
        }
        
        public Object getValue(Object obj) {
            return calc.getValue(obj);
        }
        
    }
    
    public interface Calc {
        public Object getValue(Object obj);
    }
}