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
package com.sun.grid.jgdi.event;

import com.sun.grid.jgdi.BaseTestCase;
import com.sun.grid.jgdi.JGDI;
import com.sun.grid.jgdi.EventClient;
import com.sun.grid.jgdi.JGDIFactory;
import com.sun.grid.jgdi.JobSubmitter;
import com.sun.grid.jgdi.configuration.JobTask;
import junit.framework.Test;
import junit.framework.TestSuite;

/**
 *
 */
public class SpecialEventTestCase extends BaseTestCase {
    
    private JGDI jgdi;
    private EventClient evc;
    
    /** Creates a new instance of SpecialEventTestCase */
    public SpecialEventTestCase(String testName) {
        super(testName);
    }
    
    protected void setUp() throws Exception {
        
        jgdi = createJGDI();
        evc = createEventClient(0);
        super.setUp();
        logger.fine("SetUp done");
    }
    
    protected void tearDown() throws Exception {
        try {
            evc.close();
        } finally {
            jgdi.close();
        }
    }
    
    
    public static Test suite() {
        TestSuite suite = new TestSuite( SpecialEventTestCase.class);
        return suite;
    }
}
