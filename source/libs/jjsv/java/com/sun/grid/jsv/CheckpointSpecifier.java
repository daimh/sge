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
 * The CheckpointSpecifier class represents the checkpointing environment
 * settings for a job, including the name of the checkpoint environment to use
 * and the frequency with which checkpoints should be taken.
 * @see JobDescription#getCheckpointSpecifier()
 * @see JobDescription#setCheckpointSpecifier(com.sun.grid.jsv.CheckpointSpecifier)
 * @since 6.2u5
 */
public final class CheckpointSpecifier implements Cloneable, Serializable {
    /**
     * String indicating that checkpoints should never be taken.
     */
    public static final String NEVER_STR = "n";
    /**
     * Code indicating that checkpoints should be taken when the execd
     * shuts down
     */
    public static final byte ON_SHUTDOWN = 0x01;
    /**
     * Value indicating that checkpoints should be taken when the execd
     * shuts down
     */
    public static final String ON_SHUTDOWN_STR = "s";
    /**
     * Code indicating that checkpoints should be taken periodically at the
     * minimum CPU interval as specified by the queue.
     */
    public static final byte ON_MIN_CPU_INTERVAL = 0x02;
    /**
     * String indicating that checkpoints should be taken periodically at the
     * minimum CPU interval as specified by the queue.
     */
    public static final String ON_MIN_CPU_INTERVAL_STR = "m";
    /**
     * Code indicating that checkpoints should be taken the job is suspended.
     */
    public static final byte ON_SUSPEND = 0x04;
    /**
     * String indicating that checkpoints should be taken the job is suspended.
     */
    public static final String ON_SUSPEND_STR = "x";
    /**
     * Interval representation of the occasion.
     */
    private byte when = 0;
    /**
     * The checkpoint frequency.
     */
    private long interval = 0L;
    /**
     * The name of the checkpointing environment
     */
    private String name = null;

    /**
     * Get the name of the checkpointing environment.
     * @return the name
     */
    public String getName() {
        return name;
    }

    /**
     * Set the name of the checkpointing environment.
     * @param name the name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Set whether a checkpoint should be taken when the execution daemon
     * shuts down.  Setting this value to true will also set the checkpointing
     * interval to 0.
     * @param set whether to take a checkpoint
     * @return the previous value
     * @see #getInterval()
     */
    public boolean onShutdown(boolean set) {
        boolean ret = (when & ON_SHUTDOWN) != 0;

        if (set) {
            when |= ON_SHUTDOWN;
            interval = 0L;
        } else {
            when &= ~ON_SHUTDOWN;
        }

        return ret;
    }

    /**
     * Set whether a checkpoint should be taken periodically at the minimum
     * CPU interval as specified by the queue.  Setting this value to true will
     * also set the checkpointing interval to 0.
     * @param set whether to take a checkpoint
     * @return the previous value
     * @see #getInterval()
     */
    public boolean onMinCpuInterval(boolean set) {
        boolean ret = (when & ON_MIN_CPU_INTERVAL) != 0;

        if (set) {
            when |= ON_MIN_CPU_INTERVAL;
            interval = 0L;
        } else {
            when &= ~ON_MIN_CPU_INTERVAL;
        }

        return ret;
    }

    /**
     * Set whether a checkpoint should be taken when the job is suspended.
     * Setting this value to true will also set the checkpointing
     * interval to 0.
     * @param set whether to take a checkpoint
     * @return the previous value
     * @see #getInterval()
     */
    public boolean onSuspend(boolean set) {
        boolean ret = (when & ON_SUSPEND) != 0;

        if (set) {
            when |= ON_SUSPEND;
            interval = 0L;
        } else {
            when &= ~ON_SUSPEND;
        }

        return ret;
    }

    /**
     * Indicate that a checkpoint should never be taken.
     * @return the previous occasion value, as would be returned by
     * getOccasion()
     * @see #getOccasion()
     */
    public byte never() {
        byte ret = when;

        when = 0;

        return ret;
    }

    /**
     * Set the interval at which checkpoints should be taken.  If the value is
     * 0, periodic checkpoints will not be taken.  If the value is non-zero,
     * the checkpoint occasion will be set to "never".
     * @param sec the number of seconds between checkpoints
     * @return the previous value
     * @see #getOccasion()
     */
    public long setInterval(long sec) {
        if (sec < 0L) {
            throw new IllegalArgumentException("attempted to set the interval to a negative time value");
        }

        long ret = interval;

        interval = sec;

        if (interval > 0) {
            never();
        }

        return ret;
    }

    /**
     * Set the interval at which checkpoints should be taken.  If the value is
     * 0, periodic checkpoints will not be taken.  If the value is non-zero,
     * the checkpoint occasion will be set to "never".
     * @param hours the number of hours between checkpoints -- this value is
     * combined to the number of minutes and seconds
     * @param minutes the number of minutes between checkpoints -- this value is
     * combined to the number of hours and seconds
     * @param seconds the number of seconds between checkpoints -- this value is
     * combined to the number of minutes and hours
     * @return the previous value
     * @see #getOccasion()
     */
    public long setInterval(int hours, int minutes, int seconds) {
        if (hours < 0L) {
            throw new IllegalArgumentException("attempted to set the interval to a negative hours value");
        } else if (minutes < 0L) {
            throw new IllegalArgumentException("attempted to set the interval to a negative minutes value");
        } else if (seconds < 0L) {
            throw new IllegalArgumentException("attempted to set the interval to a negative seconds value");
        }

        return setInterval(seconds + 60 * (minutes + 60 * hours));
    }

    /**
     * Get the number of seconds between checkpoints.
     * @return the number of seconds
     */
    public long getInterval() {
        return interval;
    }

    /**
     * Get a byte value that represents the occasions when the job should be
     * checkpointed.  This value is composed by ORing together the code for the
     * occasions when the job should be checkpointed.  If the occasion value is
     * non-zero, the checkpointing interval will be set to 0.
     * @return the occasion value
     * @see #ON_MIN_CPU_INTERVAL
     * @see #ON_SHUTDOWN
     * @see #ON_SUSPEND
     * @see #getInterval()
     */
    public byte getOccasion() {
        return when;
    }

    /**
     * Set the occasions when the job should be checkpointed according to a
     * a byte value composed by ORing together the code for the
     * occasions when the job should be checkpointed.  If the occasion value is
     * non-zero, the checkpointing interval will be set to 0.
     * @param value the occasion value
     * @see #ON_MIN_CPU_INTERVAL
     * @see #ON_SHUTDOWN
     * @see #ON_SUSPEND
     * @see #getInterval()
     */
    void setOccasion(int value) {
        if (value > 0x07) {
            throw new IllegalArgumentException("attempted to set the occasion specifier to an illegal value: " + value);
        }

        when = (byte)value;

        if (when != 0) {
            interval = 0L;
        }
    }

    /**
     * Set the occasions when the job should be checkpointed according to a
     * a String composed by combining the string values for the
     * occasions when the job should be checkpointed.  If the occasion value is
     * not NEVER, the checkpointing interval will be set to 0.
     * @param value the occasion string
     * @see #ON_MIN_CPU_INTERVAL_STR
     * @see #ON_SHUTDOWN_STR
     * @see #ON_SUSPEND_STR
     * @see #getInterval()
     */
    public void setOccasion(String value) {
        if ((value != null) && !value.equals("") && !value.matches("n|[msx]*")) {
            throw new IllegalArgumentException("attempted to set the occasion specifier to an illegal string: " + value);
        }

        never();

        if ((value != null) && !value.equals("")) {
            if (value.contains(ON_MIN_CPU_INTERVAL_STR)) {
                onMinCpuInterval(true);
            }

            if (value.contains(ON_SHUTDOWN_STR)) {
                onShutdown(true);
            }

            if (value.contains(ON_SUSPEND_STR)) {
                onSuspend(true);
            }
        }
    }

    /**
     * Get an occasion string that represents the occasions when the job should
     * be checkpointed.  This value is composed by combining the string values
     * for the occasions when the job should be checkpointed.
     * @return the occasion string
     * @see #ON_MIN_CPU_INTERVAL_STR
     * @see #ON_SHUTDOWN_STR
     * @see #ON_SUSPEND_STR
     */
    public String getOccasionString() {
        StringBuilder ret = new StringBuilder();

        if ((when & ON_MIN_CPU_INTERVAL) != 0) {
            ret.append(ON_MIN_CPU_INTERVAL_STR);
        }

        if ((when & ON_SHUTDOWN) != 0) {
            ret.append(ON_SHUTDOWN_STR);
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

        if ((obj instanceof CheckpointSpecifier) &&
                (when == ((CheckpointSpecifier) obj).when) &&
                (interval == ((CheckpointSpecifier) obj).interval) &&
                ((name == ((CheckpointSpecifier) obj).name) ||
                name.equals(((CheckpointSpecifier) obj).name))) {
            ret = true;
        }

        return ret;
    }

    @Override
    public int hashCode() {
        if (name != null) {
            return name.hashCode();
        } else if (interval != 0) {
            return (int)interval;
        } else {
            return when;
        }
    }

    @Override
    public CheckpointSpecifier clone() {
        CheckpointSpecifier clone = null;

        try {
            clone = (CheckpointSpecifier)super.clone();
        } catch (CloneNotSupportedException e) {
            assert false : "CheckpointSpecifier class not cloneable";
        }

        return clone;
    }

    @Override
    public String toString() {
        if ((name != null) && (when != 0x00)) {
            return name + ": " + getOccasionString();
        } else if (name != null) {
            return name + ": " + getInterval();
        } else if (when != 0x00) {
            return "[null]: " + getOccasionString();
        } else {
            return "[null]: " + getInterval();
        }
    }
}
