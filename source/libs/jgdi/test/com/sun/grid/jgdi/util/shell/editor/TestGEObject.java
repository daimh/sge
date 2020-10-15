//*___INFO__MARK_BEGIN__*/
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

import com.sun.grid.jgdi.configuration.GEObject;
import com.sun.grid.jgdi.configuration.GEObjectImpl;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 *
 */
public class TestGEObject extends GEObjectImpl {
    private java.lang.String m_name;
    //private int intNumber;
    private long m_long;
    private boolean isSetLong = false;
    //private float floatNumber;
    private double m_double;
    private boolean isSetDouble = false;
    
    private List m_stringList;//, m_geobjectList;
    private Map m_stringMap;
    private Map m_stringMapList;
    
    public boolean equalsCompletely(Object obj) {
        return false;
    }
    
    //STRING
    public java.lang.String getName() {
        return m_name;
    }
    
    public void setName(java.lang.String string) {
        this.m_name = string;
    }
    
    public boolean isSetName() {
        return  m_name != null;
    }
    
    //LONG
    public long getLong() {
        return m_long;
    }
    
    public void setLong(long longNumber) {
        this.m_long = longNumber;
        isSetLong = true;
    }
    
    public boolean isSetLong() {
        return  isSetLong;
    }
    
    public double getDouble() {
        return m_double;
    }
    
    public void setDouble(double doubleNumber) {
        this.m_double = doubleNumber;
        isSetDouble = true;
    }
    
    public boolean isSetDouble() {
        return isSetDouble;
    }
    
    //STRING_LIST
    private void initStringList() {
        if (m_stringList == null) {
            m_stringList = new ArrayList();
        }
    }
    
    public void addStringList(String obj) {
        initStringList();
        m_stringList.add(obj);
    }
    
    public void setStringList(int index, String obj) {
        initStringList();
        m_stringList.set(index, obj);
    }
    
    public void removeAllStringList() {
        if(m_stringList != null) {
            m_stringList.clear();
        }
    }
    
    public com.sun.grid.jgdi.configuration.GEObject removeStringList(int index) {
        initStringList();
        return (com.sun.grid.jgdi.configuration.GEObject)m_stringList.remove(index);
    }
    
    public boolean removeStringList(String obj) {
        initStringList();
        return m_stringList.remove(obj);
    }
    
    public List getStringList() {
        initStringList();
        return Collections.unmodifiableList(m_stringList);
    }
    
    public int getStringListCount() {
        if( m_stringList != null ) {
            return m_stringList.size();
        } else {
            return 0;
        }
    }
    
    public String getStringList(int index) {
        initStringList();
        return (String)m_stringList.get(index);
    }
    
    public boolean isSetStringList() {
        return m_stringList != null && !m_stringList.isEmpty();
    }
    
    
    //STRING_MAP
    private void initStringMap() {
        if( m_stringMap == null ) {
            m_stringMap = new HashMap();
        }
    }
    
    public void putStringMap(String hostname, String sCpu) {
        initStringMap();
        m_stringMap.put(hostname,  sCpu  );
    }
    
    public String getDefaultStringMap() {
        if(m_stringMap == null ) {
            throw new IllegalStateException("default value for attribute sCpu not found");
        }
        java.lang.String ret = (java.lang.String)m_stringMap.get("@/");
        if( ret == null ) {
            throw new IllegalStateException("default value for attribute sCpu not found");
        }
        
        return ret;
        
    }
    
    public String getStringMap(String hostname) {
        initStringMap();
        boolean keyExists = m_stringMap.containsKey(hostname);
        if (!keyExists) {
            return getDefaultStringMap();
        }
        java.lang.String ret = (java.lang.String)m_stringMap.get(hostname);
        
        return ret;
        
    }
    
    public java.lang.String removeStringMap(String hostname) {
        initStringMap();
        return (java.lang.String)m_stringMap.remove(hostname);
    }
    
    public void removeAllStringMap() {
        if( m_stringMap != null ) {
            m_stringMap.clear();
        }
    }
    
    public Set getStringMapKeys() {
        if( m_stringMap == null ) {
            return Collections.EMPTY_SET;
        } else {
            return Collections.unmodifiableSet(m_stringMap.keySet());
        }
    }
    
    public int getStringMapCount() {
        if( m_stringMap == null ) {
            return 0;
        } else {
            return m_stringMap.size();
        }
    }
    
    public boolean isSetStringMap() {
        return m_stringMap != null && !m_stringMap.isEmpty();
    }
    
    public boolean isSetStringMap(String hostname) {
        return m_stringMap != null &&
                m_stringMap.get(hostname) != null;
    }
    
    //GEObject_LIST
   /*private void initGEObjectList() {
      if (m_geobjectList == null) {
         m_geobjectList = new ArrayList();
      }
   }
    
   public void addGEObjectList(com.sun.grid.jgdi.configuration.GEObject obj) {
      initGEObjectList();
      m_geobjectList.add(obj);
   }
    
   public void setGEObjectList(int index, com.sun.grid.jgdi.configuration.GEObject obj) {
      initGEObjectList();
      m_geobjectList.set(index, obj);
   }
    
   public void removeAllGEObjectList() {
      if(m_geobjectList != null) {
          m_geobjectList.clear();
      }
   }
    
   public com.sun.grid.jgdi.configuration.GEObject removeGEObjectList(int index) {
      initGEObjectList();
      return (com.sun.grid.jgdi.configuration.GEObject)m_geobjectList.remove(index);
   }
    
   public boolean removeGEObjectList(com.sun.grid.jgdi.configuration.GEObject obj) {
      initGEObjectList();
      return m_geobjectList.remove(obj);
   }
    
   public List getGEObjectList() {
      initGEObjectList();
      return Collections.unmodifiableList(m_geobjectList);
   }
    
   public int getGEObjectListCount() {
      if( m_geobjectList != null ) {
          return m_geobjectList.size();
      } else {
          return 0;
      }
   }
    
   public com.sun.grid.jgdi.configuration.GEObject getGEObjectList(int index) {
      initGEObjectList();
      return (com.sun.grid.jgdi.configuration.GEObject)m_geobjectList.get(index);
   }
    
   public boolean isSetGEObjectListList () {
      return m_geobjectList != null;
   }*/
    
    //MapList
    private void initStringMapList() {
        if (m_stringMapList == null) {
            m_stringMapList = new HashMap();
        }
    }
    
    public String getStringMapList(String name, int index) {
        initStringMapList();
        List list = (List)m_stringMapList.get(name);
        String ret = null;
        if (list != null) {
            ret = (String)list.get(index);
        }
        return ret;
    }
    
    
    public int getStringMapListCount(String name) {
        if (m_stringMapList != null) {
            List list = (List)m_stringMapList.get(name);
            if(list != null) {
                return list.size();
            }
        }
        return 0;
    }
    
    
    public void addStringMapList(String name, String obj) {
        initStringMapList();
        List list = (List)m_stringMapList.get(name);
        if (list == null) {
            list = new ArrayList();
            m_stringMapList.put(name, list);
        }
        list.add(obj);
    }
    
    public void addEmptyStringMapList(String name) {
        initStringMapList();
        List list = (List)m_stringMapList.get(name);
        if (list == null) {
            list = new ArrayList();
            m_stringMapList.put(name, list);
        }
    }
    
    public void setStringMapList(String name, int index, String value) {
        initStringMapList();
        List list = (List)m_stringMapList.get(name);
        if (list == null) {
            list = new ArrayList();
            m_stringMapList.put(name, list);
        }
        list.set(index, value);
        
    }
    
    public Object removeStringMapListAt(String name, int index) {
        if (m_stringMapList != null) {
            List list = (List)m_stringMapList.get(name);
            if (list != null) {
                return list.remove(index);
            }
        }
        return null;
    }
    
    public boolean removeStringMapList(String name, String obj) {
        if (m_stringMapList != null) {
            List list = (List)m_stringMapList.get(name);
            if (list != null) {
                return list.remove(obj);
            }
        }
        return false;
    }
    
    public void removeAllStringMapList() {
        if (m_stringMapList != null) {
            m_stringMapList.clear();
        }
    }
    
    public void removeAllStringMapList(String name) {
        if (m_stringMapList != null) {
            List list = (List)m_stringMapList.get(name);
            if (list != null) {
                list.clear();
            }
        }
    }
    
    public Set getStringMapListKeys() {
        if (m_stringMapList != null) {
            return Collections.unmodifiableSet(m_stringMapList.keySet());
        } else {
            return Collections.EMPTY_SET;
        }
    }
    
    
    public List getStringMapListList(String name) {
        if (m_stringMapList != null) {
            List list = (List)m_stringMapList.get(name);
            if( list != null ) {
                return Collections.unmodifiableList(list);
            }
        }
        return Collections.EMPTY_LIST;
    }
    
    public String getDefaultmapList(int index) {
        return (String)getStringMapList("@/", index);
    }
    
    public int getDefaultStringMapListCount() {
        return getStringMapListCount("@/");
    }
    
    public void addDefaultStringMapList(String obj) {
        addStringMapList("@/", obj);
    }
    
    public void setDefaultStringMapList(int index, String obj) {
        setStringMapList("@/", index, obj);
    }
    
    public Object removeDefaultStringMapListAt(String name, int index) {
        return removeStringMapListAt("@/", index);
    }
    
    public boolean removeDefaultStringMapList(String name, String obj) {
        return removeStringMapList("@/", obj);
    }
    
    public void removeAllDefaultStringMapList() {
        removeAllStringMapList("@/");
    }
    
    public boolean isSetStringMapList() {
        return m_stringMapList != null &&
                !m_stringMapList.isEmpty();
    }
}
