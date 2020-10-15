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
package com.sun.grid.jgdi.monitoring;

import java.io.Serializable;

/**
 * Default implemenation of {@link QueueInfo}
 */
public class QueueInfoImpl implements QueueInfo, Serializable {

    private final static long serialVersionUID = -2009040301L;
    
    private int slotsTotal;
    private int slotsUsed;
    private int slotsReserved;
    private String state;
    private String qname;
    private String qtype;
    
    /**
     * Creates a new instance of QueueInfo
     */
    public QueueInfoImpl() {
    }
    
    /**
     *  Get the name of the queue
     *  @return the name of the queue
     */
    public String getQname() {
        return qname;
    }
    
    /**
     *  Set the name of the queue
     *  @param qname the name of the queue
     */
    public void setQname(String qname) {
        this.qname = qname;
    }
    
    /**
     *  Get the type of the queue
     *  @return the type of the queue
     */
    public String getQtype() {
        return qtype;
    }
    
    /**
     *  Set the name of the queue
     *  @param qtype the type of the queue
     */
    public void setQtype(String qtype) {
        this.qtype = qtype;
    }
    
    /**
     *  Get the state of the queue
     *  @return state of the queue
     */
    public String getState() {
        return state;
    }
    
    /**
     *  Set the state of the queue
     */
    public void setState(String state) {
        this.state = state;
    }
    
    /**
     *  Get the number of total queue slots
     *  @return number of total queue slots
     */
    public int getTotalSlots() {
        return slotsTotal;
    }
    
    /**
     *  Set the number of total queue slots
     *  @param slotsTotal of total queue slots
     */
    public void setTotalSlots(int slotsTotal) {
        this.slotsTotal = slotsTotal;
    }
    
    /**
     *  Get the number of used queue slots
     *  @return number of used queue slots
     */
    public int getUsedSlots() {
        return slotsUsed;
    }
    
    /**
     *  Set the number of used queue slots
     *  @param slotsUsed of used queue slots
     */
    public void setUsedSlots(int slotsUsed) {
        this.slotsUsed = slotsUsed;
    }

    /**
     *  Get the number of used queue slots
     *  @return number of used queue slots
     */
    public int getReservedSlots() {
        return slotsReserved;
    }

    /**
     *  Set the number of reserved queue slots
     *  @param slotsReserved of used queue slots
     */
    public void setReservedSlots(int slotsReserved) {
        this.slotsReserved = slotsReserved;
    }
}
