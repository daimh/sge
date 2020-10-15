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

import java.util.*;
import java.util.logging.Logger;

/**
 *
 */
public class CullObject {
    
    private Logger logger = Logger.getLogger("cullconv");
    
    private String name;
    private ArrayList attrs = new ArrayList();
    private ArrayList params = new ArrayList();
    private String    idlName;
    
    private boolean hasAddOperation;
    private boolean hasModifyOperation;
    private boolean hasDeleteOperation;
    private boolean hasGetOperation;
    private boolean hasGetListOperation;
    
    private boolean hasAddEvent;
    private boolean hasModifyEvent;
    private boolean hasDeleteEvent;
    private boolean hasGetListEvent;
    
    private String addEventName;
    private String modifyEventName;
    private String deleteEventName;
    private String getListEventName;
    
    private boolean rootObject;
    private String  listName;
    
    public static final int TYPE_SIMPLE    = 1;
    public static final int TYPE_PRIMITIVE = 2;
    public static final int TYPE_MAP       = 3;
    public static final int TYPE_MAPPED    = 4;
    
	 private int type = TYPE_SIMPLE;
    
    private CullObject parentObject;
    private String     parentName;
    
    private String keyAttrName;
    private String valueAttrName;
    private String implClass;
    
    private String contentAttrName;
    
    /** Creates a new instance of CullObject */
    public CullObject() {
    }
    
    /**
     * Getter for property name.
     * @return Value of property name.
     */
    public java.lang.String getName() {
        return name;
    }
    
    /**
     * Setter for property name.
     * @param name New value of property name.
     */
    public void setName(java.lang.String name) {
        this.name = name;
    }
    
    
    public void addParam(String param ) {
        params.add(param);
    }
    
    public int getParamCount() {
        return params.size();
    }
    
    public String getParam(int index) {
        return (String)params.get(index);
    }
    
    public void addAttr( CullAttr attr ) {
        if(!attr.isHidden()) {
            logger.fine(getName() + ": add attr " + attr.getName());
            attrs.add( attr );
            attr.setParent( this );
            attrList = null;
            primaryKeyAttrList = null;
        }
    }
    
    private List attrList;
    
    private void initAttrList() {
        if(attrList == null) {
            attrList = new ArrayList();
            if(getParentObject() != null) {
                attrList.addAll(getParentObject().getAttrList());
            }
            attrList.addAll(attrs);
        }
    }
    
    /**
     *  Get the number of attributes of this cull object
     *  incl. the attributes of the parent object
     *  return number of attributes
     */
    public int getAttrCount() {
        initAttrList();
        return attrList.size();
    }
    
    /**
     *  Get a list of all attributes of this cull object including
     *  the attributes of the parent object
     *  @return list of all attributes
     */
    public List getAttrList() {
        initAttrList();
        return Collections.unmodifiableList(attrList);
    }
    
    /**
     *  Get the attributes at index i
     *  @param i   index of the attributes
     *  @return  the attributes
     */
    public CullAttr getAttr( int i ) {
        initAttrList();
        return (CullAttr)attrList.get(i);
    }
    
    /**
     *  Get a attribute by its name
     *  @param name the name of the attribute
     *  @return the attributes
     */
    public CullAttr getAttr(String name) {
        initAttrList();
        CullAttr ret = null;
        for(int i = 0; i < attrList.size(); i++ ) {
            CullAttr tmp = (CullAttr)attrList.get(i);
            if( tmp.getName().equals(name)) {
                ret = tmp;
                break;
            }
        }
        if(ret == null) {
            throw new IllegalArgumentException("attribute " + name + " not found in cull object " + getName() );
        }
        return ret;
    }
    
    public int getOwnAttrCount() {
        return attrs.size();
    }
    
    public List getOwnAttrList() {
        return Collections.unmodifiableList(attrs);
    }
    
    public CullAttr getOwnAttr(int i) {
        return (CullAttr)attrs.get(i);
    }
    
    public String getIdlName() {
        return idlName;
    }
    
    public void setIdlName(String idlName) {
        this.idlName = idlName;
    }
    
    
    private List primaryKeyAttrList;
    
    private void initPrimaryKeyAttrList() {
        if(primaryKeyAttrList == null) {
            
            int type = this.type;
            if(getParentObject() != null) {
                type = getParentObject().getType();
            }
            
            if(type == TYPE_PRIMITIVE || type == TYPE_MAPPED) {
                primaryKeyAttrList = Collections.EMPTY_LIST;
            } else {
                primaryKeyAttrList = new ArrayList();
                
                for(int i = 0; i < getAttrCount(); i++) {
                    CullAttr attr = getAttr(i);
                    if( attr.isPrimaryKey() ) {
                        primaryKeyAttrList.add(attr);
                    }
                }
            }
        }
    }
    
    public CullAttr getFirstPrimaryKeyAttr() {
        initPrimaryKeyAttrList();
        if(!primaryKeyAttrList.isEmpty()) {
            return (CullAttr)primaryKeyAttrList.get(0);
        }
        return null;
    }
    
    
    public List getPrimaryKeyAttrs() {
        initPrimaryKeyAttrList();
        return Collections.unmodifiableList(primaryKeyAttrList);
    }
    
    public int getPrimaryKeyCount() {
        initPrimaryKeyAttrList();
        return primaryKeyAttrList.size();
    }
    
    public CullAttr getPrimaryKeyAttr(int i) {
        initPrimaryKeyAttrList();
        return (CullAttr)primaryKeyAttrList.get(i);
    }
    
    
    public boolean hasAddOperation() {
        return hasAddOperation;
    }
    
    public void setHasAddOperation(boolean hasAddOperation) {
        this.hasAddOperation = hasAddOperation;
    }
    
    public boolean hasModifyOperation() {
        return hasModifyOperation;
    }
    
    public void setHasModifyOperation(boolean hasModifyOperation) {
        this.hasModifyOperation = hasModifyOperation;
    }
    
    public boolean hasDeleteOperation() {
        return hasDeleteOperation;
    }
    
    public void setHasDeleteOperation(boolean hasDeleteOperation) {
        this.hasDeleteOperation = hasDeleteOperation;
    }
    
    public boolean hasGetOperation() {
        return hasGetOperation;
    }
    
    public void setHasGetOperation(boolean hasGetOperation) {
        this.hasGetOperation = hasGetOperation;
    }
    
    public boolean hasGetListOperation() {
        return hasGetListOperation;
    }
    
    public void setHasGetListOperation(boolean hasGetListOperation) {
        this.hasGetListOperation = hasGetListOperation;
    }
    
    public String getOperationString() {
        StringBuffer ret = new StringBuffer();
        if(hasGetOperation()) {
            ret.append("GET ");
        }
        if(hasGetListOperation()) {
            ret.append("GET_LIST ");
        }
        if(hasAddOperation()) {
            ret.append("ADD ");
        }
        if(hasModifyOperation()) {
            ret.append("MODIFY ");
        }
        if(hasDeleteOperation()) {
            ret.append("DELETE ");
        }
        return ret.toString();
    }
    
    public boolean isRootObject() {
        return rootObject;
    }
    
    public void setRootObject(boolean rootObject) {
        this.rootObject = rootObject;
    }
    
    public int getType() {
        if(getParentObject() != null) {
            return getParentObject().getType();
        }
        return type;
    }
    
    public void setType(int type) {
        this.type = type;
    }
    
    public CullObject getParentObject() {
        return parentObject;
    }
    
    public void setParentObject(CullObject parentObject) {
        this.parentObject = parentObject;
        attrList = null;
        primaryKeyAttrList = null;
    }
    
    public String getParentName() {
        return parentName;
    }
    
    public void setParentName(String parentName) {
        this.parentName = parentName;
    }
    
    public String getKeyAttrName() {
        String ret = keyAttrName;
        if(ret == null && getParentObject() != null) {
            ret = getParentObject().getKeyAttrName();
        }
        return ret;
    }
    
    public void setKeyAttrName(String keyAttrName) {
        this.keyAttrName = keyAttrName;
    }
    
    public CullAttr getKeyAttr() {
        return getAttr(getKeyAttrName());
    }
    
    public String getValueAttrName() {
        String ret = valueAttrName;
        if(ret == null && getParentObject() != null) {
            ret = getParentObject().getValueAttrName();
        }
        return ret;
    }
    
    public CullAttr getValueAttr() {
        return getAttr(getValueAttrName());
    }
    
    public void setValueAttrName(String valueAttrName) {
        this.valueAttrName = valueAttrName;
    }
    
    public String getImplClass() {
        String ret = implClass;
        if(ret == null && getParentObject() != null) {
            ret = getParentObject().getImplClass();
        }
        return ret;
    }
    
    public void setImplClass(String implClass) {
        this.implClass = implClass;
    }
    
    public String getContentAttrName() {
        String ret = contentAttrName;
        if(ret == null && getParentObject() != null) {
            ret = getParentObject().getContentAttrName();
        }
        return ret;
    }
    
    public void setContentAttrName(String contentAttrName) {
        this.contentAttrName = contentAttrName;
    }
    
    public CullAttr getContentAttr() {
        return getAttr(getContentAttrName());
    }
    
    public String getListName() {
        return listName;
    }
    
    public void setListName(String listName) {
        this.listName = listName;
    }
    
    public boolean hasEvents() {
        return hasAddEvent || hasModifyEvent || hasDeleteEvent || hasGetListEvent;
    }
    
    public boolean hasAddEvent() {
        return hasAddEvent;
    }
    
    public void setHasAddEvent(boolean hasAddEvent) {
        this.hasAddEvent = hasAddEvent;
    }
    
    public boolean hasModifyEvent() {
        return hasModifyEvent;
    }
    
    public void setHasModifyEvent(boolean hasModifyEvent) {
        this.hasModifyEvent = hasModifyEvent;
    }
    
    public boolean hasDeleteEvent() {
        return hasDeleteEvent;
    }
    
    public void setHasDeleteEvent(boolean hasDeleteEvent) {
        this.hasDeleteEvent = hasDeleteEvent;
    }
    
    public boolean hasGetListEvent() {
        return hasGetListEvent;
    }
    
    public void setHasGetListEvent(boolean hasGetListEvent) {
        this.hasGetListEvent = hasGetListEvent;
    }
    
    public String getAddEventName() {
        return addEventName;
    }
    
    public void setAddEventName(String addEventName) {
        this.addEventName = addEventName;
    }
    
    public String getModifyEventName() {
        return modifyEventName;
    }
    
    public void setModifyEventName(String modifyEventName) {
        this.modifyEventName = modifyEventName;
    }
    
    public String getDeleteEventName() {
        return deleteEventName;
    }
    
    public void setDeleteEventName(String deleteEventName) {
        this.deleteEventName = deleteEventName;
    }
    
    public String getGetListEventName() {
        return getListEventName;
    }
    
    public void setGetListEventName(String getListEventName) {
        this.getListEventName = getListEventName;
    }
    
}
