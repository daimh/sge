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
package com.sun.grid.security.login;

import com.sun.grid.TestConfiguration;
import com.sun.grid.util.SGEUtil;
import java.io.File;
import java.util.Iterator;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import junit.framework.TestCase;

/**
 *
 */
public class AuthUserWrapperTest extends TestCase{
    
    private static final Logger LOGGER = Logger.getLogger(AuthUserWrapperTest.class.getName());
    private File authuser;
    public AuthUserWrapperTest(String testName) {
        super(testName);
    }

    protected void setUp() throws Exception {
        TestConfiguration config = TestConfiguration.getInstance();
        File utilbin = new File(config.getSgeRoot(), "utilbin");
        File utilbinArch = new File(utilbin, SGEUtil.getArch(config.getSgeRoot()));
        if(SGEUtil.isWindows()) {
            authuser = new File(utilbinArch, "authuser.exe");
        } else {
            authuser = new File(utilbinArch, "authuser");
        }
    }

    protected void tearDown() throws Exception {
    }
    
    public void testSystem() throws Exception {
        
        TestConfiguration config = TestConfiguration.getInstance();
        AuthUserWrapper wrapper = AuthUserWrapper.newInstance(authuser.getAbsolutePath());
        
        Set principals = wrapper.authenticate(config.getTestUser(), config.getTestUserPassword());

        assertNotNull("no principals found", principals);
        
        Iterator iter = principals.iterator();
        while(iter.hasNext()) {
            LOGGER.log(Level.FINE,"user {0} has principal {1}", new Object [] { config.getTestUser(), iter.next() });
        }
        
        try {
            principals =  wrapper.authenticate(config.getTestUser(), new char[0]); 
            assertNull("login which empty password successeded", principals);
        } catch(Exception e) {
            // ignore
        }
    }
    
    public void testPam() throws Exception {
        
        if(!SGEUtil.isWindows()) {
            TestConfiguration config = TestConfiguration.getInstance();

            AuthUserWrapper wrapper = AuthUserWrapper.newInstanceForPam(authuser.getAbsolutePath(), config.getPamService());

            Set principals = wrapper.authenticate(config.getTestUser(), config.getTestUserPassword());

            assertNotNull("no principals found", principals);

            Iterator iter = principals.iterator();
            while(iter.hasNext()) {
                LOGGER.log(Level.FINE,"user {0} has principal {1}", new Object [] { config.getTestUser(), iter.next() });
            }

            principals =  wrapper.authenticate(config.getTestUser(), new char[0]); 
            assertNull("login which empty password successeded", principals);
        }
    }
    
}
