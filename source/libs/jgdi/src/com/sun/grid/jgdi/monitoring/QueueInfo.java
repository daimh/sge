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


/**
 * Holds the information about a queue for qhost
 */
public interface QueueInfo {
    
    /**
     *   Get the qname
     *   @return qname
     */
    public String getQname();
    
    /**
     *  Get the qtype string
     *  @return qtype
     */
    public String getQtype();
    
    /**
     *  Get the state of the queue
     *  @return state of the queue
     */
    public String getState();
    
    /**
     *  Get the number of total queue slots
     *  @return number of total queue slots
     */
    public int getTotalSlots();
    
    /**
     *  Get the number of used queue slots
     *  @return number of used queue slots
     */
    public int getUsedSlots();

    /**
     *  Get the number of reserved queue slots
     *  @return number of reserved queue slots
     */
    public int getReservedSlots();

}
