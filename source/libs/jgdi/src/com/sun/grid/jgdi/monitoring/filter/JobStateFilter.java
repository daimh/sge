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
package com.sun.grid.jgdi.monitoring.filter;

import java.io.Serializable;

/**
 * This class build a job state filter.
 *
 */
public class JobStateFilter implements Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    public static class State {

        private int mask;
        private String value;

        public State(int mask, String value) {
            this.mask = mask;
            this.value = value;
        }

        public int getMask() {
            return mask;
        }

        public String getValue() {
            return value;
        }

        public boolean isSet(int state) {
            return (mask & state) == mask;
        }
    }
/*
    [-s {p|r|s|z|hu|ho|hs|hj|ha|h|a}] show pending, running, suspended, zombie jobs,
    jobs with a user/operator/system hold,
    jobs with a start time in future or any combination only.
    h is an abbreviation for huhohshjha
    a is an abbreviation for prsh
     */
    /**
     *  Jobs in the state hold user (<code>qstat -s hu</code>)
     */
    public static final State HOLD_USER = new State(0x0001, "hu");
    /**
     *  Jobs in the state hold operator (<code>qstat -s ho</code>)
     */
    public static final State HOLD_OPERATOR = new State(0x0002, "ho");
    /**
     *  Jobs in the state hold system (<code>qstat -s hs</code>)
     */
    public static final State HOLD_SYSTEM = new State(0x0004, "hs");
    /**
     *  Pending jobs (<code>qstat -s p</code>)
     */
    public static final State PENDING = new State(0x0008, "p");
    /**
     *  Running jobs (<code>qstat -s r</code>)
     */
    public static final State RUNNING = new State(0x0010, "r");
    /**
     *  Suspended jobs (<code>qstat -s r</code>)
     */
    public static final State SUSPENDED = new State(0x0020, "s");
    /**
     *  Zombie jobs. (<code>qstat -s z</code>)
     */
    public static final State ZOMBIE = new State(0x0040, "z");

    /**
     *  Jobs in the state hold job array job (<code>qstat -s hd</code>)
     */
    public static final State HOLD_JOBARRAYHOLD = new State(0x0080, "hd");

    /**
     *  Jobs in the state hold job hold (<code>qstat -s hj</code>)
     */
    public static final State HOLD_JOBHOLD = new State(0x0100, "hj");

    /**
     *  Jobs in the state hold job hold (<code>qstat -s hj</code>)
     */
    public static final State HOLD_STARTTIMEHOLD = new State(0x0200, "ha");

    /**
     * This state includes <code>HOLD_USER</code>, <code>HOLD_OPERATOR</code>,
     * <code>HOLD_SYSTEM</code>, <code>HOLD_JOBARRAYHOLD</code>, <code>HOLD_JOBHOLD</code>
     * and <code>HOLD_STARTTIMEHOLD</code>
     */
    public static final State HOLD = new State(HOLD_USER.getMask() | HOLD_OPERATOR.getMask() | HOLD_SYSTEM.getMask() | HOLD_JOBARRAYHOLD.getMask() | HOLD_JOBHOLD.getMask() | HOLD_STARTTIMEHOLD.getMask(), "h");

    /**
     *  This state includes a other states besides <code>ZOMBIE</code>.
     *
     *  Same as <code>qstat -s a</code> or <code>qstat -s prsh</code>
     */
    public static final State ALL = new State(PENDING.getMask() | RUNNING.getMask() | SUSPENDED.getMask() /* | HOLD.getMask() */, "a");
    private static final State[] ALL_SIMPLE_STATES = new State[]{HOLD_USER, HOLD_OPERATOR, HOLD_SYSTEM, HOLD_JOBARRAYHOLD, HOLD_JOBHOLD, HOLD_STARTTIMEHOLD, PENDING, RUNNING, SUSPENDED, ZOMBIE};
    private int mask;
    private String stateStr;

    public JobStateFilter(State[] states) {
        for (int i = 0; i < states.length; i++) {
            include(states[i]);
        }
    }

    public JobStateFilter() {
        setStates(0);
    }

    /**
     * Parse a job state string.
     * @param stateString  the job state string
     * @return the job state
     */
    public static JobStateFilter parse(String stateString) {
        return fill(stateString);
    }

    /**
     * Parse a job state string.
     * @param stateString  the job state string
     * @return the job state
     */
    public static JobStateFilter fill(String stateString) {

        JobStateFilter ret = new JobStateFilter();
        String str = null;
        int index = 0;
        outer:
        while (index < stateString.length()) {
            for (int i = 0; i < ALL_SIMPLE_STATES.length; i++) {
                str = ALL_SIMPLE_STATES[i].getValue();
                if (stateString.indexOf(str, index) == index) {
                    ret.include(ALL_SIMPLE_STATES[i]);
                    index += str.length();
                    continue outer;
                }
            }
            str = HOLD.getValue();
            if (stateString.indexOf(str, index) == index) {
                ret.include(HOLD);
                index += str.length();
                continue;
            }
            str = ALL.getValue();
            if (stateString.indexOf(str, index) == index) {
                ret.include(ALL);
                index += str.length();
                continue;
            }

            throw new IllegalStateException("Unknown jobs state " + stateString);
        }
        return ret;
    }

    public void setStates(int mask) {
        if (this.mask != mask) {
            this.mask = mask;
            stateStr = null;
        }
    }

    public int getStates() {
        return this.mask;
    }

    private void include(State state) {
        setStates(mask | state.getMask());
    }

    public void exclude(State state) {
        setStates(mask & (~state.getMask()));
    }

    /**
     *  Include/Exclude jobs in specific state
     *
     *  @param state  the state
     *  @param flag   if <code>true</code> jobs in the state are included
     *                else they are excluded
     */
    public void set(State state, boolean flag) {
        if (flag) {
            include(state);
        } else {
            exclude(state);
        }
    }

    /**
     * Determine if jobs in a specific state are included.
     *
     * @return <code>true</code> if jobs in the specific state are includes
     */
    public boolean isSet(State state) {
        return state.isSet(mask);
    }

    /**
     * Get the state string as it can be used for qstat
     *
     * @return the state string
     */
    public String getStateString() {
        if (stateStr == null) {
            StringBuilder buf = new StringBuilder();

            for (int i = 0; i < ALL_SIMPLE_STATES.length; i++) {
                if (ALL_SIMPLE_STATES[i].isSet(mask)) {
                    buf.append(ALL_SIMPLE_STATES[i].getValue());
                }
            }
            stateStr = buf.toString();
        }
        return stateStr;
    }

    /**
     * Determine of this is equal to <code>obj</code>.
     * Two <code>JobStateFilter</code> objects are equals if they
     * include exactly the same jobs states
     *
     * @param obj the obj
     * @return <code>true</code> if </code>obj</code> is equal to this
     */
    @Override
    public boolean equals(Object obj) {
        return obj instanceof JobStateFilter && mask == ((JobStateFilter) obj).mask;
    }

    /**
     * Get the hashcode of this object
     * @return the hashcode
     */
    @Override
    public int hashCode() {
        return mask;
    }

    /**
     * Get a string representation of this object
     * @return the string representation
     */
    @Override
    public String toString() {
        return "JobStateFilter: " + getStateString();
    }
}
