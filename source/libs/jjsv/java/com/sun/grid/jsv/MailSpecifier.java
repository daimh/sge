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
 *   Copyright: 2009 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jsv;

import java.io.Serializable;

/**
 * The MailSpecifier class represents the occasions when an email notification
 * should be sent to the mail recipients list about the state of this job.
 * @see JobDescription#getMailSpecifier()
 * @see JobDescription#setMailSpecifier(com.sun.grid.jsv.MailSpecifier)
 * @since 6.2u5
 */
public final class MailSpecifier implements Cloneable, Serializable {
    /**
     * String representing that email should never be set.
     */
    public static final String NEVER_STR = "n";
    /**
     * Code representing that email should be sent if the job is abourted.
     */
    public static final int ON_ABORT = 0x04;
    /**
     * String representing that email should be sent if the job is abourted.
     */
    public static final String ON_ABORT_STR = "a";
    /**
     * Code representing that email should be sent when the job is started.
     */
    public static final int ON_BEGIN = 0x01;
    /**
     * String representing that email should be sent when the job is started.
     */
    public static final String ON_BEGIN_STR = "b";
    /**
     * Code representing that email should be sent when the job ends.
     */
    public static final int ON_END = 0x02;
    /**
     * String representing that email should be sent when the job ends.
     */
    public static final String ON_END_STR = "e";
    /**
     * Code representing that email should be sent if the job is suspended.
     */
    public static final int ON_SUSPEND = 0x08;
    /**
     * String representing that email should be sent if the job is suspended.
     */
    public static final String ON_SUSPEND_STR = "s";
    /**
     * Internal representation of the occasion.
     */
    private byte when = 0;

    /**
     * Set whether email should be sent when the job is started.
     * @param set whether email should be sent
     * @return the previous value
     */
    public boolean onBegin(boolean set) {
        boolean ret = (when & ON_BEGIN) != 0;

        if (set) {
            when |= ON_BEGIN;
        } else {
            when &= ~ON_BEGIN;
        }

        return ret;
    }

    /**
     * Set whether email should be sent when the job end.
     * @param set whether email should be sent
     * @return the previous value
     */
    public boolean onEnd(boolean set) {
        boolean ret = (when & ON_END) != 0;

        if (set) {
            when |= ON_END;
        } else {
            when &= ~ON_END;
        }

        return ret;
    }

    /**
     * Set whether email should be sent if the job is aborted.
     * @param set whether email should be sent
     * @return the previous value
     */
    public boolean onAbort(boolean set) {
        boolean ret = (when & ON_ABORT) != 0;

        if (set) {
            when |= ON_ABORT;
        } else {
            when &= ~ON_ABORT;
        }

        return ret;
    }

    /**
     * Set whether email should be sent if the job is suspended.
     * @param set whether email should be sent
     * @return the previous value
     */
    public boolean onSuspend(boolean set) {
        boolean ret = (when & ON_SUSPEND) != 0;

        if (set) {
            when |= ON_SUSPEND;
        } else {
            when &= ~ON_SUSPEND;
        }

        return ret;
    }

    /**
     * Indicate that email should never be sent about a job.  This method
     * effectively clears the flags for all the other occasions.  Susequent
     * calls to other methods may add or set the occasions when email is sent.
     * @return the previous occasion value, as would be returned from
     * getOccasion().
     * @see #getOccasion()
     */
    public byte never() {
        byte ret = when;

        when = 0;

        return ret;
    }

    /**
     * Return a byte value that represents the occasions when email should be
     * sent about a job.  The byte value is composed by ORing together the
     * code values for all of the occasions when email should be sent.
     * @return the occasion value
     * @see #ON_ABORT
     * @see #ON_BEGIN
     * @see #ON_END
     * @see #ON_SUSPEND
     */
    public byte getOccasion() {
        return when;
    }

    /**
     * Set the occasions when email should be sent about a job according to a
     * byte value that is composed by ORing together the code values for the
     * occasions when email should be sent.
     * @param occasion the occasion value
     * @see #ON_ABORT
     * @see #ON_BEGIN
     * @see #ON_END
     * @see #ON_SUSPEND
     */
    void setOccasion(int occasion) {
        if ((occasion & 0xF0) != 0) {
            throw new IllegalArgumentException("attemped to set occasion specifier to an illegal value");
        }

        when = (byte)occasion;
    }

    /**
     * Set the occasions when email should be sent for a job according to a
     * String composed of the string identifiers for the occasions when email
     * should be sent.
     * @param value the occasion string
     * @see #ON_ABORT_STR
     * @see #ON_BEGIN_STR
     * @see #ON_END_STR
     * @see #ON_SUSPEND_STR
     */
    public void setOccasion(String value) {
        if ((value != null) && !value.equals("") && !value.matches("n|[abes]*")) {
            throw new IllegalArgumentException("attempted to set the occasion specifier to an illegal occasion string: " + value);
        }

        never();

        if ((value != null) && !value.equals("")) {
            if (value.contains(ON_ABORT_STR)) {
                onAbort(true);
            }

            if (value.contains(ON_BEGIN_STR)) {
                onBegin(true);
            }

            if (value.contains(ON_END_STR)) {
                onEnd(true);
            }

            if (value.contains(ON_SUSPEND_STR)) {
                onSuspend(true);
            }
        }
    }

    /**
     * Return a String value that represents the occasions when email should be
     * sent about a job.  The String value is composed by combining the
     * string identifiers for all of the occasions when email should be sent.
     * @return the occasion string
     * @see #ON_ABORT_STR
     * @see #ON_BEGIN_STR
     * @see #ON_END_STR
     * @see #ON_SUSPEND_STR
     */
    public String getOccasionString() {
        StringBuilder ret = new StringBuilder();

        if ((when & ON_ABORT) != 0) {
            ret.append(ON_ABORT_STR);
        }

        if ((when & ON_BEGIN) != 0) {
            ret.append(ON_BEGIN_STR);
        }

        if ((when & ON_END) != 0) {
            ret.append(ON_END_STR);
        }

        if ((when & ON_SUSPEND) != 0) {
            ret.append(ON_SUSPEND_STR);
        }

        if (ret.length() == 0) {
            ret.append(NEVER_STR);
        }

        return ret.toString();
    }

    @Override
    public boolean equals(Object obj) {
        boolean ret = false;

        if ((obj instanceof MailSpecifier) &&
                (when == ((MailSpecifier) obj).when)) {
            ret = true;
        }

        return ret;
    }

    @Override
    public int hashCode() {
        return when;
    }

    @Override
    public MailSpecifier clone() {
        MailSpecifier retValue = null;

        try {
            retValue = (MailSpecifier)super.clone();
        } catch (CloneNotSupportedException e) {
            assert false : "MailSpecifier is not cloneable";
        }

        return retValue;
    }

    @Override
    public String toString() {
        return getOccasionString();
    }
}
