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
package com.sun.grid.util.expect;

import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * An <code>ExpectBuffer</code> holds the content of stdout and sterr of
 * a process. <code>ExpectHandler</code>s can use the <code>consume...</code>
 * method to consome content.
 *
 * The <code>Expect</code>object uses the <code>append</code> methods to add
 * new content to the buffer.
 * 
 */
public class ExpectBuffer {

    private static final Logger LOGGER = Logger.getLogger(ExpectBuffer.class.getName());
    
    public final static String NL = System.getProperty("line.separator");
    
    private final StringBuffer buffer = new StringBuffer();
    
    
    /**
     * Add new content to the buffer
     * @param s       char array with the content
     * @param offset  offset in the char array
     * @param len     len of the the content
     */
    public void append(char [] s, int offset, int len) {
        buffer.append(s, offset, len);
    }
    
    
    /**
     * Add new content to the buffer
     *
     * @param s a string with the content
     */
    public void append(String s) {
        buffer.append(s);
    }
    
    
    /**
     * get the length of the buffer
     * @return length of the buffer
     */
    public int length() {
        return buffer.length();
    }
    
    
    /**
     * Search the first occurancy of <code>s</code> is the buffer and consume
     * it (<code>s</code> is also consumed).
     *
     * If the content of the buffer is "1234" a consume("2") will remove "12".
     *
     * @param s the search string
     * @return the consumed string ("12")
     */
    public String consume(String s) {
        String ret = null;
        synchronized(buffer) {
            int index = buffer.indexOf(s);
            if(index >= 0) {
                int end = index + s.length();
                ret = buffer.substring(0, end);
                LOGGER.log(Level.FINER, "consumed: ''{0}''", ret);
                buffer.delete(0, end);
            }
        }
        return ret;
    }
    
    /**
     * Search a line which contains with <code>prefix</code> and consume it.
     * 
     * If the content of a buffer is "1\\n123\\m" a consumeLine("2") will consume
     * "1\\n122\\n".
     *
     * @param prefix the prefix
     * @return the consumed string
     */
    public String consumeLine(String prefix) {
        String ret = null;
        synchronized(buffer) {
            int index = buffer.indexOf(prefix);
            if(index >= 0) {
                int nl = buffer.indexOf(NL, index + 1);
                if(nl > index) {
                    ret = buffer.substring(index + prefix.length(), nl);
                    LOGGER.log(Level.FINER, "consumed: ''{0}''", buffer.substring(0, nl));
                    buffer.delete(0, nl);
                }
            }
        }
        return ret;
    }

    /**
     *  The consume the content of the buffer until the next linefeed.
     *
     *  @return the consumed string
     */
    public String consumeLine() {
        String ret = null;
        synchronized(buffer) {
            int nl = buffer.indexOf(NL);
            if(nl >= 0) {
                ret = buffer.substring(0,nl);
                LOGGER.log(Level.FINER, "consumed: ''{0}''", ret);
                buffer.delete(0, nl+1);
            }
        }
        return ret;
    }
    
}
