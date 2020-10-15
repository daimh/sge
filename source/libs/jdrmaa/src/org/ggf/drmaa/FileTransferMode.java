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
/*
 * FileTransferMode.java
 *
 * Created on October 6, 2004, 6:05 PM
 */

package org.ggf.drmaa;

import java.io.Serializable;

/**
 * This class represents the streams which should be used for file transfers.
 * For each of the three properties which is set to true, the corresponding
 * stream's path property in the job template will be treated as a source or
 * destination (depending on the stream) for file tranfers.  For example, if the
 * inputStream property is set to true, the inputPath property of the
 * JobTemplate will be interpreted as a source from which to transfer files.
 * @author dan.templeton@sun.com
 * @since 0.5
 * @version 1.0
 */
public class FileTransferMode implements Serializable, Cloneable {
    /** Whether to transfer error stream files. */
    private boolean errorStream = false;
    /** Whether to transfer input stream files. */
    private boolean inputStream = false;
    /** Whether to transfer output stream files. */
    private boolean outputStream = false;

    /**
     * Creates a new instance of FileTransferMode
     */
    public FileTransferMode() {
    }

    /**
     * Create a new instance with the property values preset.
     * @param inputStream whether to transfer input stream files
     * @param outputStream whether to transfer output stream files
     * @param errorStream whether to transfer error stream files
     */
    public FileTransferMode(boolean inputStream, boolean outputStream, boolean errorStream) {
        this.errorStream = errorStream;
        this.inputStream = inputStream;
        this.outputStream = outputStream;
    }

    /**
     * Set whether to transfer error stream files.
     * @param errorStream whether to transfer error stream files
     */
    public void setErrorStream(boolean errorStream) {
        this.errorStream = errorStream;
    }

    /**
     * Whether to transfer error stream files.
     * @return whether to transfer error stream files
     */
    public boolean getErrorStream() {
        return errorStream;
    }

    /**
     * Set whether to transfer error stream files.
     * @param inputStream whether to transfer error stream files
     */
    public void setInputStream(boolean inputStream) {
        this.inputStream = inputStream;
    }

    /**
     * Whether to transfer error stream files.
     * @return whether to transfer error stream files
     */
    public boolean getInputStream() {
        return inputStream;
    }

    /**
     * Set whether to transfer error stream files.
     * @param outputStream whether to transfer error stream files
     */
    public void setOutputStream(boolean outputStream) {
        this.outputStream = outputStream;
    }

    /**
     * Whether to transfer error stream files.
     * @return whether to transfer error stream files
     */
    public boolean getOutputStream() {
        return outputStream;
    }

    /**
     * Test whether two FileTransferMode objects have the same property
     * settings.
     * @param obj the Object to test for equality
     * @return whether the FileTransferMode object has the same property
     * settings as this one
     */
    public boolean equals(Object obj) {
        return ((obj instanceof FileTransferMode) &&
                (((FileTransferMode)obj).errorStream == errorStream) &&
                (((FileTransferMode)obj).inputStream == inputStream) &&
                (((FileTransferMode)obj).outputStream == outputStream));
    }

    /**
     * Returns a hash code based on the file transfer properties.
     * @return a hash code based on the file transfer properties
     */
    public int hashCode() {
        int ret = 0;

        ret += inputStream ? 1 : 0;
        ret += outputStream ? 2 : 0;
        ret += errorStream ? 4 : 0;

        return ret;
    }

    /**
     * Creates a copy of this FileTransferMode object.
     * @return a copy of this FileTransferMode object
     */
    public Object clone() {
        try {
            return super.clone();
        } catch (CloneNotSupportedException e) {
            // this shouldn't happen, since we are Cloneable
            throw new InternalError();
        }
    }

    /**
     * Returns a string containing the stream settings.
     * @return a string containing the stream settings
     */
    public String toString() {
        StringBuffer out = new StringBuffer();
        boolean firstProperty = true;

        if (inputStream) {
            out.append("input");
            firstProperty = false;
        }

        if (outputStream) {
            if (firstProperty) {
                firstProperty = false;
            } else {
                out.append(", ");
            }

            out.append("output");
        }

        if (errorStream) {
            if (!firstProperty) {
                out.append(", ");
            }

            out.append("error");
        }

        return out.toString();
    }
}
