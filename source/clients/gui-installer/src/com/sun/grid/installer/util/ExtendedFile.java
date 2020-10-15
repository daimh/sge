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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.installer.util;

import java.io.File;
import com.izforge.izpack.util.Debug;

/**
 * Extends the File class to provide attributes as owner, permission and group.
 *
 * @see FileHandler#getFileProps()
 */
public class ExtendedFile extends File {
	public static final String PERM_USER_READ     = "^.r........*";
	public static final String PERM_USER_WRITE    = "^..w.......*";
	public static final String PERM_USER_EXECUTE  = "^...x......*";
	public static final String PERM_GROUP_READ    = "^....r.....*";
	public static final String PERM_GROUP_WRITE   = "^.....w....*";
	public static final String PERM_GROUP_EXECUTE = "^......x...*";
	public static final String PERM_WORLD_READ    = "^.......r..*";
	public static final String PERM_WORLD_WRITE   = "^........w.*";
	public static final String PERM_WORLD_EXECUTE = "^.........x*";
    
	private String owner       = null;
    private String permissions = null;
    private String group       = null;

    /**
     * Constructor
     * @param pathName The file path.
     */
    public ExtendedFile(String pathName) {
        super(pathName);
        
        setExtendedProps();
    }

    /**
     * Sets the owner, permission, group properties if this file/directory.
     */
    public void setExtendedProps() {
        String[] props = FileHandler.getFileProps(getAbsolutePath());

        if (props != null) {
            owner       = props[FileHandler.POS_OWNER_FIELD];
            permissions = props[FileHandler.POS_PERMISSION_FIELD];
            group       = props[FileHandler.POS_GROUP_FIELD];
        }
    }

    /**
     * Returns the group id of this file.
     * @return The group id.
     */
    public String getGroup() {
        if (group == null) {
            setExtendedProps();
        }
        return group;
    }

    /**
     * Returns the owner of this file.
     * @return The owner.
     */
    public String getOwner() {
        if (owner == null) {
            setExtendedProps();
        }
        return owner;
    }

    /**
     * Returns the permission string of this file.
     * @return The permissions.
     */
    public String getPermissions() {
        if (permissions == null) {
            setExtendedProps();
        }
        return permissions;
    }

    @Override
    public String toString() {
        return permissions + " " + owner + " " + group + " " + super.toString();
    }

    /**
     * Returns with the first existing parent of this file/directory
     * @return this if it exists, otherwise the first existing parent directory.
     */
    public ExtendedFile getFirstExistingParent() {
        if (exists()) {
            return this;
        }

        File file = this.getParentFile();
        while (file != null && !file.exists()) {
            file = file.getParentFile();
        }

        return new ExtendedFile(file.getAbsolutePath());
    }

    /**
     * Checks whether the given user has permission to read the given file.
     * @param user The user to check
     * @param group The group the user belongs to
     * @return true if the file exists and user has permission to read the file.
     */
    public boolean hasReadPermission(String user, String group) {
        if (permissions == null) {
            setExtendedProps();
        }

        if (permissions == null) {
            return false;
        }

        if (owner.equals(user)) {
            return permissions.matches(PERM_USER_READ);
        } else if (this.group.equals(group)) {
            return permissions.matches(PERM_GROUP_READ);
        } else {
            return permissions.matches(PERM_WORLD_READ);
        }
    }

    /**
     * Checks whether the given user has permission to write the given file.
     * @param user The user to check
     * @param group The group the user belongs to
     * @return true if the file exists and user has permission to write the file.
     */
    public boolean hasWritePermission(String user, String group) {
        if (permissions == null) {
            setExtendedProps();
        }

        if (permissions == null) {
Debug.trace("no perms -null");
            return false;
        }

        if (owner.equals(user)) {
Debug.trace("Owner="+owner+" user="+user+" perms="+permissions+" res="+permissions.matches(PERM_USER_WRITE));
            return permissions.matches(PERM_USER_WRITE);
        } else if (group.equals(group)) {
Debug.trace("Group="+group+" user="+user+" perms="+permissions+" res="+permissions.matches(PERM_GROUP_WRITE));
            return permissions.matches(PERM_GROUP_WRITE);
        } else {
            return permissions.matches(PERM_WORLD_WRITE);
        }
    }

    /**
     * Checks whether the given user has permission to execute the given file.
     * @param user The user to check
     * @param group The group the user belongs to
     * @return true if the file exists and user has permission to execute the file.
     */
    public boolean hasExecutePermission(String user, String group) {
        if (permissions == null) {
            setExtendedProps();
        }

        if (permissions == null) {
            return false;
        }

        if (owner.equals(user)) {
            return permissions.matches(PERM_USER_EXECUTE);
        } else if (group.equals(group)) {
            return permissions.matches(PERM_GROUP_EXECUTE);
        } else {
            return permissions.matches(PERM_WORLD_EXECUTE);
        }
    }
}
