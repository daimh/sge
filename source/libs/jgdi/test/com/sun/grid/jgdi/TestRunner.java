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
package com.sun.grid.jgdi;

import java.lang.reflect.Method;
import java.security.PrivilegedExceptionAction;
import java.util.Enumeration;
import java.util.logging.Logger;
import junit.framework.TestCase;
import junit.framework.TestFailure;
import junit.framework.TestResult;
import junit.framework.TestSuite;

/**
 *
 */
public class TestRunner implements PrivilegedExceptionAction {
    
    private static Logger logger = Logger.getLogger(TestRunner.class.getName());
    
    private Class testClass;
    
    public TestRunner(Class testClass) {
        this.testClass = testClass;
    }
    
    
    private static void usage(String message, int exitCode) {
        if (message != null) {
            logger.severe(message);
        }
        logger.info("TestRunner <test class>");
        System.exit(exitCode);
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        
        try {
            
            if (args.length < 1) {
                usage("Invalid number of arguments", 1);
            }
            
            String classname = args[0];
            
            logger.info("Search for class " + classname);
            Class testClass = null;
            
            try {
                testClass = Class.forName(classname);
            } catch (ClassNotFoundException cnfe) {
                usage("Class " + classname + " not found", 1);
            }
            
            if (!TestCase.class.isAssignableFrom(testClass)) {
                usage("class " + classname + " is not a TestCase", 1);
            }
            
            TestRunner runner = new TestRunner(testClass);
            
            runner.run();
        } catch (Throwable e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    public Object run() throws Exception {
        Method testSuiteMethod = testClass.getMethod("suite", (java.lang.Class[]) null);
        
        TestSuite suite = (TestSuite) testSuiteMethod.invoke(testClass, (java.lang.Object[])null);
        
        TestResult result = new TestResult();
        
        
        suite.run(result);
        
        if (result.wasSuccessful()) {
            
            logger.info("Success");
        } else {
            logger.severe("Failed");
            
            Enumeration errorEnum = result.errors();
            TestFailure failure = null;
            while (errorEnum.hasMoreElements()) {
                failure = (TestFailure) errorEnum.nextElement();
                logger.severe("Test " + failure.failedTest().toString() + " failed --------");
                Throwable th = failure.thrownException();
                th.printStackTrace(System.err);
            }
            errorEnum = result.failures();
            failure = null;
            while (errorEnum.hasMoreElements()) {
                failure = (TestFailure) errorEnum.nextElement();
                logger.severe("Test " + failure.failedTest().toString() + " failed --------");
                Throwable th = failure.thrownException();
                th.printStackTrace(System.err);
            }
            System.exit(1);
        }
        return null;
    }
}
