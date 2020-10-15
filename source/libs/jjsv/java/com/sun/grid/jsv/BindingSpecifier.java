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
import java.util.List;

/**
 * The BindingSpecifier object represents the core binding settings
 * for a job.  It contains the binding type, strategy, and strategy
 * parameters.
 * @see JobDescription#getBindingSpecifier()
<!-- * @see JobDescription#setBindingSpecifier() --> <!-- javadoc complains -->
 * @since 6.2u5
 */
public final class BindingSpecifier implements Cloneable, Serializable {
    public final class CoreSpecifier {
        public int socket;
        public int core;

        public CoreSpecifier(int socket, int core) {
            this.socket = socket;
            this.core = core;
        }

        public CoreSpecifier() {
            this(0, 0);
        }

        @Override
        public boolean equals(Object obj) {
            boolean ret = false;

            if (obj instanceof CoreSpecifier) {
                CoreSpecifier sc = (CoreSpecifier)obj;

                if (sc.socket == socket &&
                    sc.core == core) {
                    ret = true;
                }
            }
            return ret;
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 79 * hash + this.socket;
            hash = 79 * hash + this.core;
            return hash;
        }
    }

    public enum Strategy {
        LINEAR,
        STRIDING,
        EXPLICIT;

        @Override
        public String toString() {
            String ret = super.toString();

            if (this == Strategy.LINEAR) {
                ret = "linear";
            } else if (this == Strategy.STRIDING) {
                ret = "striding";
            } else if (this == Strategy.EXPLICIT) {
                ret = "explicit";
            } else {
                ret = "unknown";
            }

            return ret;
        }
    }

    public enum Type {
        SET,
        PE,
        ENV;

        @Override
        public String toString() {
            String ret = super.toString();

            if (this == Type.SET) {
                ret = "set";
            } else if (this == Type.PE) {
                ret = "pe";
            } else if (this == Type.ENV) {
                ret = "env";
            } else {
                ret = "unknown";
            }

            return ret;
        }
    }

    private Strategy strategy = null;
    private Type type = Type.SET; /* if not other specified set is used */
    private int amount = -1;
    private int step = -1;
    private int socket = -1;
    private int core = -1;
    private List<CoreSpecifier> socketCore = null;

    private void setStrategy(Strategy strategy, int amount, int socket, int core, List<CoreSpecifier> socketCore) {
        this.strategy = strategy;
        this.amount = amount;
        this.socket = socket;
        this.core = core;
        this.socketCore = socketCore;
    }

    /**
     * Set binding strategy string
     * @param strategy new strategy
     */
    public void setStrategy(Strategy strategy) {
        this.strategy = strategy;
    }

    /**
     * Set linear binding strategy
     * @param amount number of cores
     * @param socket first socket
     * @param core first core on socket
     */
    public void setLinearStrategy(int amount, int socket, int core) {
        if (amount < 0 || socket < 0 || core < 0) {
            throw new IllegalArgumentException("amount, socket and core must be geater that zero");
        } else {
            setStrategy(Strategy.LINEAR, amount, socket, core, null);
        }
    }

    /**
     * Set linear-automatic binding strategy
     * @param amount number of sockets
     */
    public void setLinearStrategy(int amount) {
        if (amount < 0) {
            throw new IllegalArgumentException("amount must be geater that zero");
        } else {
            setStrategy(Strategy.LINEAR, amount, 0, 0, null);
        }
    }

    /**
     * Set striding binding strategy
     * @param amount number of cores
     * @param socket first socket
     * @param core first core on socket
     */
    public void setStridingStrategy(int amount, int socket, int core) {
        if (amount < 0 || socket < 0 || core < 0) {
            throw new IllegalArgumentException("amount, socket and core must be geater that zero");
        } else {
            setStrategy(Strategy.STRIDING, amount, socket, core, null);
        }
    }

    /**
     * Set striding-automatic binding strategy
     * @param amount number of cores
     */
    public void setStridingStrategy(int amount) {
        if (amount < 0) {
            throw new IllegalArgumentException("amount must be geater that zero");
        } else {
            setStrategy(Strategy.STRIDING, amount, 0, 0, null);
        }
    }

    /**
     * Set explicit binding strategy specifying socket/core map
     * @param socketCore list of cores
     */
    public void setExplicitStrategy(List<CoreSpecifier> socketCore) {
        if (socketCore.size() == 0) {
            throw new IllegalArgumentException("map has to contain at least one entry");
        } else {
            setStrategy(Strategy.EXPLICIT, -1, -1, -1, socketCore);
        }
    }


    /**
     * Returns true if binding strategy is linear
     * @return true in case of linear binding
     */
    public Boolean isLinear() {
        return strategy == Strategy.LINEAR;
    }

    /**
     * Returns true if binding strategy is striding
     * @return true in case of striding binding
     */
    public Boolean isStriding() {
        return strategy == Strategy.STRIDING;
    }

    /**
     * Returns true if binding strategy is explicit
     * @return true in case of explicit binding
     */
    public Boolean isExplicit() {
        return strategy == Strategy.EXPLICIT;
    }

    /**
     * Returns the binding strategy string
     * @return binding strategy
     */
    public Strategy getStrategy() {
        return strategy;
    }

    /**
     * Set binding type
     */
    public void setType(Type type) {
        this.type = type;
    }

    /**
     * Set binding type to type 'set'
     */
    public void setSetType() {
        setType(Type.SET);
    }

    /**
     * Set binding type to type pe
     */
    public void setPeType() {
        setType(Type.PE);
    }

    /**
     * Set binding type to type env
     */
    private void setEnvType() {
        setType(Type.ENV);
    }

    /**
     * Returns true if type is set
     * @return true in case of set type
     */
    public Boolean isSetType() {
        return type.equals(Type.SET);
    }

    /**
     * Returns true if type is pe
     * @return true in case of pe type
     */
    public Boolean isPeType() {
        return type.equals(Type.PE);
    }

    /**
     * Returns true if type is env
     * @return true in case of env type
     */
    public Boolean isEnvType() {
        return type.equals(Type.ENV);
    }

    /**
     * Returns the binding type
     * @return binding type
     */
    public Type getType() {
        return type;
    }


    /**
     * Returns the number of cores
     * @return core amount
     */
    public int getAmount() {
        return amount;
    }

    /**
     * Sets the binding amount
     * @param amount binding amount
     */
    public void setAmount(int amount) {
        this.amount = amount;
    }

    /**
     * Returns the start socket
     * @return start socket
     */
    public int getSocket() {
        return socket;
    }

    /**
     * Sets the binding socket
     * @param socket binding socket
     */
    public void setSocket(int socket) {
        this.socket = socket;
    }

    /**
     * Returns the core on the start socket
     * @return start core
     */
    public int getCore() {
        return core;
    }

    /**
     * Sets the binding core
     * @param core binding core
     */
    public void setCore(int core) {
        this.core = core;
    }

    /**
     * Returns the step size
     * @return step size
     */
    public int getStep() {
        return step;
    }

    /**
     * Sets the binding step size
     * @param step binding step size
     */
    public void setStep(int step) {
        this.step = step;
    }

    /**
     * Returns the core specifier list (only explicit binding)
     * @return list of core specifiers
     */
    public List<CoreSpecifier> getCoreSpecifiers() {
        return socketCore;
    }


    @Override
    public boolean equals(Object obj) {
        boolean ret = false;

        if (obj instanceof BindingSpecifier) {
            BindingSpecifier bs = (BindingSpecifier)obj;
            
            if (((strategy == null && bs.strategy == null) 
                         || (strategy != null && bs.strategy != null && strategy.equals(bs.strategy))) &&
                ((type == null && bs.type == null) 
                         || (type != null && bs.type != null && type.equals(bs.type))) &&
                ((socketCore == null && bs.socketCore == null)
                         || (socketCore != null && bs.socketCore != null && socketCore.equals(bs.socketCore))) &&
                amount == bs.amount &&
                socket == bs.socket &&
                core == bs.core &&
                step == bs.step) {
               ret = true;
            }
        }

        return ret;
    }

    @Override
    public int hashCode() {
        return strategy.hashCode();
    }

    @Override
    public BindingSpecifier clone() {
        BindingSpecifier clone = null;

        try {
            clone = (BindingSpecifier) super.clone();
        } catch (CloneNotSupportedException e) {
            assert false : "BindingSpecifier is not cloneable";
        }

        return clone;
    }

    @Override
    public String toString() {
        StringBuffer ret = new StringBuffer();

        if (type != null) {
            ret.append(type + " ");
        }
        if (strategy != null) {
            ret.append(strategy);
        }
        if (strategy == Strategy.LINEAR) {
            ret.append(":" + amount);
            if (socket != -1 && core != -1) {
                ret.append(":" + socket + "," + core);
            }
        } else if (strategy == Strategy.STRIDING) {
            ret.append(":" + amount);
            ret.append(":" + step);
            if (socket != -1 && core != -1) {
                ret.append(":" + socket + "," + core);
            }
        } else {
            Boolean isFirst = true;

            ret.append(strategy);
            ret.append(":");

            for (CoreSpecifier sc : socketCore) {
                if (isFirst) {
                    isFirst = false;
                } else {
                    ret.append(':');
                }
                ret.append(sc.socket + "," + sc.core);
            }

        }

        return ret.toString();
    }
}
