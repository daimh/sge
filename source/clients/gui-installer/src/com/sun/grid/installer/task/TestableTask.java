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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.installer.task;

import com.sun.grid.installer.util.Config;
import java.util.Vector;

public abstract class TestableTask implements Runnable, Config {

    private boolean testMode = false;
    private int testExitValue = EXIT_VAL_CMDEXEC_INITIAL;
    private Vector<String> testOutput = new Vector<String>();
    private String taskName = "";

    public String getTaskName() {
        return taskName;
    }

    public void setTaskName(String taskName) {
        this.taskName = taskName;
    }

    public boolean isIsTestMode() {
        return testMode;
    }

    public void setTestMode(boolean isTestMode) {
        this.testMode = isTestMode;
    }

    public int getTestExitValue() {
        return testExitValue;
    }

    public void setTestExitValue(int testExitValue) {
        this.testExitValue = testExitValue;
    }

    public Vector<String> getTestOutput() {
        return testOutput;
    }

    public void setTestOutput(Vector<String> testOutput) {
        this.testOutput = testOutput;
    }
}
