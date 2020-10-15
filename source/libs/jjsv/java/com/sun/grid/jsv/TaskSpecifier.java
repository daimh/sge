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
 * The TaskSpecifier class represents an array task specification for a job.
 * It defines the minimum and maximum values in the task identifier range as
 * well as the incremental step value.  By default, the minimum, maximum, and
 * step values are set to 1.
 * @see JobDescription#getTaskSpecifier()
 * @see JobDescription#setTaskSpecifier(com.sun.grid.jsv.TaskSpecifier)
 * @since 6.2u5
 */
public final class TaskSpecifier implements Cloneable, Serializable {
    private int min = 1;
    private int max = 1;
    private int step = 1;

    /**
     * Get the minimum value of the array task id range.
     * @return the minimum range value
     */
    public int getMin() {
        return min;
    }

    /**
     * Get the maximum value of the array task id range.
     * @return the maximum range value
     */
    public int getMax() {
        return max;
    }

    /**
     * Get the incremental step value for the array task id range.
     * @return the step value
     */
    public int getStep() {
        return step;
    }

    /**
     * Set the minimum value of the array task id range.  Values less than 1
     * will cause an IllegalArgumentException to be thrown.
     * @param min the minimum range value
     */
    public void setMin(int min) {
        if (min < 1) {
            throw new IllegalArgumentException("attempted to set min to a zero or negative value");
        }

        this.min = min;
    }

    /**
     * Set the maximum value of the array task id range.  Values less than 1
     * will cause an IllegalArgumentException to be thrown.
     * @param max the maximum range value
     */
    public void setMax(int max) {
        if (max < 1) {
            throw new IllegalArgumentException("attempted to set max to a zero or negative value");
        }

        this.max = max;
    }

    /**
     * Set the incremental step value of the array task id range.  Values less
     * than 1 will cause an IllegalArgumentException to be thrown.
     * @param step the incremental step value
     */
    public void setStep(int step) {
        if (step < 1) {
            throw new IllegalArgumentException("attempted to set step to a zero or negative value");
        }

        this.step = step;
    }

    /**
     * Set the minimum and maximum values of the array task id range.  Values
     * less than 1 will cause an IllegalArgumentException to be thrown, as will
     * a maximum range value that is less than the minimum range value.
     * @param min the minimum range value
     * @param max the maximum range value
     */
    public void setRange(int min, int max) {
        if (min < 1) {
            throw new IllegalArgumentException("attempted to set min to a zero or negative value");
        } else if (max < 1) {
            throw new IllegalArgumentException("attempted to set max to a zero or negative value");
        } else if (max < min) {
            throw new IllegalArgumentException("attempted to set max to a value less than min");
        }

        setMin(min);
        setMax(max);
    }

    /**
     * Set the minimum and maximum values of the array task id range as well as
     * the incremental step value.  Values less than 1 will cause an
     * IllegalArgumentException to be thrown, as will a maximum range value that
     * is less than the minimum range value.
     * @param min the minimum range value
     * @param max the maximum range value
     * @param step the step value
     */
    public void setRange(int min, int max, int step) {
        if (min < 1) {
            throw new IllegalArgumentException("attempted to set min to a zero or negative value");
        } else if (max < 1) {
            throw new IllegalArgumentException("attempted to set max to a zero or negative value");
        } else if (max < min) {
            throw new IllegalArgumentException("attempted to set max to a value less than min");
        } else if (step < 1) {
            throw new IllegalArgumentException("attempted to set step to a zero or negative value");
        }

        setRange(min, max);
        setStep(step);
    }

    @Override
    public boolean equals(Object obj) {
        boolean ret = false;

        if ((obj instanceof TaskSpecifier) &&
                (min == ((TaskSpecifier) obj).min) &&
                (max == ((TaskSpecifier) obj).max) &&
                (step == ((TaskSpecifier) obj).step)) {
            ret = true;
        }

        return ret;
    }

    @Override
    public int hashCode() {
        return step + min * 1000 + max * 10000;
    }

    @Override
    public TaskSpecifier clone() {
        TaskSpecifier clone = null;

        try {
            clone = (TaskSpecifier) super.clone();
        } catch (CloneNotSupportedException e) {
            assert false : "TaskSpecifier class not cloneable";
        }

        return clone;
    }

    @Override
    public String toString() {
        return min + "-" + max + ":" + step;
    }
}
