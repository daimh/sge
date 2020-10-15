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

/**
 * The Jsv interface is implemented by any class that will be used as a JSV.
 * The interface defines the callback methods used by the JsvManager class to
 * trigger the steps of job verification.
 * @since 6.2u5
 * @see JsvManager
 */
public interface Jsv {
    /**
     * This method is called to start the job verification process.  In general,
     * the most common use for this method is to call the
     * JsvManager.requestEnvironment() method.
     * @param jsv
     * @see JsvManager#requestEnvironment(boolean)
     */
    public abstract void onStart(JsvManager jsv);
    /**
     * This method is called to trigger the job verification step.  This method
     * is used by the Jsv implementation to accept, modify, or reject the job.
     * Information about the job being verified is available from the
     * JobDescription instance retrieved from the JsvManager.getJobDescription()
     * method.  Changes made to the JobDescription instance will be propagated
     * to the JSV framework.  Before this method returns, it should call one of
     * the result methods on the JsvManager instance, accept(), modify(),
     * reject(), rejectWait(), or setResult().  If none is called, the job will
     * be implicitly accepted or modified as appropriate.
     * @param jsv
     * @see JsvManager#getJobDescription()
     * @see JsvManager#accept(java.lang.String)
     * @see JsvManager#modify(java.lang.String)
     * @see JsvManager#reject(java.lang.String)
     * @see JsvManager#rejectWait(java.lang.String)
     * @see JsvManager#setResult(com.sun.grid.jsv.JsvManager.Result, java.lang.String) 
     */
    public abstract void onVerify(JsvManager jsv);
}
