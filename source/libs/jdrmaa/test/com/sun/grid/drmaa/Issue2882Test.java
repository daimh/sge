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

package com.sun.grid.drmaa;

import junit.framework.*;
import org.ggf.drmaa.*;
/**
 *
 */
public class Issue2882Test extends TestCase {
    private Session session;
    
    public static Test suite () {
        TestSuite suite = new TestSuite (Issue2882Test.class);
        return suite;
    }

    /**
     * Setup of test.
     * @throws DrmaaException
     */
    public void setUp() throws DrmaaException {
        session = SessionFactory.getFactory().getSession();
        /* Do not init() - this is the test for the bug */
        /* session.init(""); */
    }

    /**
     * Tear down of test.
     * @throws DrmaaException
     */
    public void tearDown() throws DrmaaException {
        /* the system should be stable enough to call exit() twice */
         try {
            session.exit();
        } catch (NoActiveSessionException ex) {
            // this exception is expected
        } catch (DrmaaException ex) {
            ex.printStackTrace();
       }
    }

    public void test2882Test() throws DrmaaException {
        System.out.println("testIssue2882");
        /* call the underlaying japi_exit() before japi_init() */
         try {
            session.exit();
        } catch (NoActiveSessionException ex) {
            // this exception is expected
        } catch (DrmaaException ex) {
            ex.printStackTrace();
        }
    }

}
