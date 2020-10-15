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
 * The ParallelEnvironment object represents the parallel environment settings
 * for a job.  It contains the name of the parallel environment to be used and
 * the minimum and maximum number of parallel processes to be started.  The
 * minimum and maximum number of processes are set to 1 by default.
 * @see JobDescription#getParallelEnvironment()
 * @see JobDescription#setParallelEnvironment(com.sun.grid.jsv.ParallelEnvironment)
 * @since 6.2u5
 */
public final class ParallelEnvironment implements Cloneable, Serializable {
    private String name = null;
    private int min = 1;
    private int max = 1;

    /**
     * Set the name of the parallel environment to be used
     * @param name the PE name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Get the name of the parallel environment to be used
     * @return the PE name
     */
    public String getName() {
        return name;
    }

    /**
     * Get the minimum number of parallel processes to be started for this job.
     * @return the minimum number of processes
     */
    public int getRangeMin() {
        return min;
    }

    /**
     * Get the maximum number of parallel processes to be started for this job.
     * @return the maximum number of processes
     */
    public int getRangeMax() {
        return max;
    }

    /**
     * Set the minimum and maximum number of parallel processes to be started
     * for this job.
     * @param min the minimum number of processes
     * @param max the maximum number of processes
     */
    public void setRange(int min, int max) {
        if (min < 1) {
            throw new IllegalArgumentException("attempted to set a zero or negative min value");
        } else if (max < 1) {
            throw new IllegalArgumentException("attempted to set a zero or negative max value");
        } else if (min > max) {
            throw new IllegalArgumentException("attempted to set a min value that is higher than the max value");
        }

        this.min = min;
        this.max = max;
    }

    /**
     * Set the minimum and maximum number of parallel processes to be started
     * for this job to the same value, i.e. a range of 1.
     * @param val the minimum and maximum number of processes
     */
    public void setRange(int val) {
        if (val < 1) {
            throw new IllegalArgumentException("attempted to set a zero or negative min/max value");
        }

        this.min = this.max = val;
    }

    /**
     * Set the minimum number of parallel processes to be started for this job.
     * @param min the minimum number of processes
     */
    public void setRangeMin(int min) {
        if (min < 1) {
            throw new IllegalArgumentException("attempted to set a zero or negative min value");
        }

        this.min = min;
    }

    /**
     * Set the maximum number of parallel processes to be started for this job.
     * @param max the maximum number of processes
     */
    public void setRangeMax(int max) {
        if (max < 1) {
            throw new IllegalArgumentException("attempted to set a zero or negative max value");
        }

        this.max = max;
    }

    @Override
    public boolean equals(Object obj) {
        boolean ret = false;

        if ((obj instanceof ParallelEnvironment) &&
                (min == ((ParallelEnvironment) obj).min) &&
                (max == ((ParallelEnvironment) obj).max) &&
                ((name == ((ParallelEnvironment) obj).name) ||
                (name.equals(((ParallelEnvironment) obj).name)))) {
            ret = true;
        }

        return ret;
    }

    @Override
    public int hashCode() {
        return name.hashCode();
    }

    @Override
    public ParallelEnvironment clone() {
        ParallelEnvironment clone = null;

        try {
            clone =  (ParallelEnvironment) super.clone();
        } catch (CloneNotSupportedException e) {
            assert false : "ParallelEnvironment is not cloneable";
        }

        return clone;
    }

    @Override
    public String toString() {
        if (name != null) {
            return name + ": " + min + "-" + max;
        } else {
            return "[null]: " + min + "-" + max;
        }
    }
}
