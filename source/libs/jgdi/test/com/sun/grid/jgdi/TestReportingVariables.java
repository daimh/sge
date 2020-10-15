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
package com.sun.grid.jgdi;

import junit.framework.Test;
import junit.framework.TestSuite;
import com.sun.grid.jgdi.configuration.ExecHost;
import java.util.List;


/**
 * This junit test tests that the reporting_variables of a exec host object
 * can be set, modified and get.
 * This test tests the bug reported with issue 3128.
 */
public class TestReportingVariables extends com.sun.grid.jgdi.BaseTestCase {

    private JGDI jgdi;

    /** Creates a new instance of TestReportingVariables
     * @param testName the name of the test
     */
    public TestReportingVariables(String testName) {
        super(testName);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(TestReportingVariables.class);
        return suite;
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        jgdi = createJGDI();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        if (jgdi != null) {
            jgdi.close();
        }
    }
    
    public void testUpdateGetReportingVariables() throws Exception {
        
        ExecHost eh = jgdi.getExecHost("global");
        List<String> orgRV = eh.getReportVariablesList();
        eh.addReportVariables("cpu");
        eh.addReportVariables("np_load_avg");
        jgdi.updateExecHost(eh);
        try {
            ExecHost eh1 = jgdi.getExecHost("global");
            List<String> rv = eh1.getReportVariablesList();
            assertTrue("reporting variables must contain cpu", rv.contains("cpu"));
            assertTrue("reporting variables must contain np_load_avg", rv.contains("np_load_avg"));
        } finally {
            eh.removeAllReportVariables();
            for(String rv: orgRV) {
                eh.addReportVariables(rv);
            }
            jgdi.updateExecHost(eh);
        }
    }
}
