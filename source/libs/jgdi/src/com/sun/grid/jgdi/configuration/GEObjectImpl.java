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

/**
 *
 */
public abstract class GEObjectImpl implements java.io.Serializable, GEObject, Cloneable {

    /* parent object. */
private GEObjectImpl parent;
    /* name of the object. */
    private String name;

    public GEObjectImpl() {
    }

    public GEObjectImpl(String name) {
        setName(name);
    }

    /**
     * Creates a new instance of GEObjectImpl
     */
    public GEObjectImpl(GEObjectImpl parent) {
        this(parent, null);
    }

    public GEObjectImpl(GEObjectImpl parent, String name) {
        setParent(parent);
        setName(name);
    }

    /**
     * Getter for property name.
     * @return Value of property name.
     */
    public String getName() {
        return name;
    }

    /**
     * Setter for property name.
     * @param name New value of property name.
     */
    public void setName(java.lang.String name) {
        this.name = name;
    }

    /**
     * Getter for property parent.
     * @return Value of property parent.
     */
    public GEObject getParent() {
        return parent;
    }

    /**
     * Setter for property parent.
     * @param parent New value of property parent.
     */
    public void setParent(GEObjectImpl parent) {
        this.parent = parent;
    }

    public String getPathName() {
        if (parent == null) {
            return name;
        } else {
            return parent.getName() + "/" + name;
        }
    }

    public GEObjectImpl getRoot() {
        if (parent != null) {
            return parent.getRoot();
        } else {
            return this;
        }
    }
}