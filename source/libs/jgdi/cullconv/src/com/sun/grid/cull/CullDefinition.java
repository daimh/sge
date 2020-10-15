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

import java.io.*;
import java.util.*;
import java.util.logging.Logger;

/**
 *
 */
public class CullDefinition {

    private Map<String, Elem> cullObjectMap = new HashMap<String, Elem>();
    private Map<String, NameElem> nameObjectMap = new HashMap<String, NameElem>();
    private Map<String, EnumElem> enumMap = new HashMap<String, EnumElem>();
    private String packageName;

    private Logger logger = Logger.getLogger("cullconv");

    /** Creates a new instance of AbstractCullContext */
    public CullDefinition() {
    }

    protected Elem createElem() {
        return new Elem();
    }

    public void addFile(String file) throws IOException, ParseException {
        addFile(new File(file));
    }

    public void addFile(File file) throws IOException, ParseException {

        FileInputStream in = new FileInputStream(file);
        CullFile cullFile = Cull.parse(in);
        cullFile.setSource(file);

        for (int i = 0; i < cullFile.getCullObjectCount(); i++) {
            addCullObject(cullFile.getCullObject(i), file);
        }
        for (int i = 0; i < cullFile.getNameSpaceCount(); i++) {
            addNameSpace(cullFile.getNameSpace(i), file);
        }
        for (int i = 0; i < cullFile.getEnumCount(); i++) {
            addEnum(cullFile.getEnum(i), file);
        }
    }

    /**
     *  Verify the cull definition
     *  @return number of errors
     */
    public int verify() {

        int errors = 0;
        for (String name : getObjectNames()) {
            CullObject obj = getCullObject(name);
            if (obj.isRootObject()) {
                if (obj.getIdlName() == null) {
                    errors++;
                    logger.severe("Root object " + obj.getName() + " has not idl name");
                }
            }

            if (obj.getParentName() != null) {
                CullObject baseObj = getCullObject(obj.getParentName());
                if (baseObj == null) {
                    errors++;
                    logger.severe("Base type " + obj.getParentName() + " of object " + obj.getName() + "not found");
                } else {
                    obj.setParentObject(baseObj);
                }
            }
            for (int i = 0; i < obj.getAttrCount(); i++) {
                CullAttr attr = obj.getAttr(i);
                if (attr instanceof CullMapAttr) {
                    CullObject mapObj = getCullObject(attr.getType());
                    if (mapObj.getType() == CullObject.TYPE_MAP) {
                        ((CullMapAttr) attr).setMapType(mapObj);
                    } else {
                        errors++;
                        logger.severe(obj.getName() + "." + attr.getName() + ": type " + attr.getType() + " is not a JGDI_MAP_OBJ");
                    }
                }
            }
        }

        return errors;
    }


    public void addNameSpace(CullNameSpace obj, File source) {
        File f = getSource(obj.getNameSpace());

        if (f != null) {
            System.err.println("Warning: duplicate definition of name space object " + obj.getNameSpace());
            System.err.println("         --> " + f);
            System.err.println("         --> " + source);
        }
        NameElem elem = new NameElem();
        elem.obj = obj;
        elem.source = source;
        nameObjectMap.put(obj.getNameSpace(), elem);
    }

    public Set<String> getNameSpaceNameSet() {
        return nameObjectMap.keySet();
    }

    public CullNameSpace getNameSpace(String name) {
        NameElem ret = (NameElem) nameObjectMap.get(name);
        if (ret != null) {
            return ret.obj;
        }
        return null;
    }

    public File getNameSource(String name) {
        NameElem elem = (NameElem) nameObjectMap.get(name);
        if (elem != null) {
            return elem.source;
        }
        return null;
    }

    public void addCullObject(CullObject obj, File source) {
        File f = getSource(obj.getName());

        if (f != null) {
            System.err.println("Warning: duplicate definition of cull object " + obj.getName());
            System.err.println("         --> " + f);
            System.err.println("         --> " + source);
        }
        Elem elem = createElem();
        elem.obj = obj;
        elem.source = source;

        cullObjectMap.put(obj.getName(), elem);
    }

    public Set<String> getObjectNames() {
        return Collections.unmodifiableSet(cullObjectMap.keySet());
    }


    public CullObject getCullObject(String name) {
        Elem ret = (Elem) cullObjectMap.get(name);
        if (ret != null) {
            return ret.obj;
        } else {
            return null;
        }
    }

    public File getSource(String name) {
        Elem ret = (Elem) cullObjectMap.get(name);
        if (ret != null) {
            return ret.source;
        } else {
            return null;
        }
    }

    public static class Elem {
        CullObject obj;
        File source;
    }

    public static class NameElem {
        CullNameSpace obj;
        File source;
    }

    public static class EnumElem {
        CullEnum obj;
        File source;
    }

    public String getPackageName() {
        return packageName;
    }

    public void setPackageName(String packageName) {
        this.packageName = packageName;
    }

    public void addEnum(CullEnum aEnum, File file) {
        EnumElem elem = new EnumElem();
        elem.obj = aEnum;
        elem.source = file;
        enumMap.put(aEnum.getName(), elem);
    }

    public Set<String> getEnumNames() {
        return Collections.unmodifiableSet(enumMap.keySet());
    }

    public CullEnum getEnum(String name) {
        return enumMap.get(name).obj;
    }
}
