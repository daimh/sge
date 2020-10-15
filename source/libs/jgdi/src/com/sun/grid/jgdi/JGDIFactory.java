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

import com.sun.grid.jgdi.jni.EventClientImpl;
import com.sun.grid.jgdi.management.JGDIProxy;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.net.MalformedURLException;
import java.util.logging.Logger;
import javax.management.remote.JMXServiceURL;

/**
 * Factory class for {@link JGDI} objects.
 *
 * @see com.sun.grid.jgdi.JGDI
 */
public class JGDIFactory {

    private final static Logger log = Logger.getLogger(JGDIFactory.class.getName());
    private static String versionString;
    private static boolean libNotLoaded = true;

    private static synchronized void initLib() throws JGDIException {
        if (libNotLoaded) {
            try {
                System.loadLibrary("jgdi");
                libNotLoaded = false;
            } catch (Throwable e) {
                JGDIException ex = new JGDIException("Can not load native jgdi lib: " + e.getMessage());
                // ex.initCause(e);  does not work for whatever reason
                throw ex;
            }
        }
    }

    /**
     * Get a new instance of a <code>JGDI</code> object.
     *
     * @param url  JGDI connection url in the form
     *             <code>bootstrap://&lt;SGE_ROOT&gt;@&lt;SGE_CELL&gt;:&lt;SGE_QMASTER_PORT&gt;</code>
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     * @return the <code>JGDI<code> instance
     */
    public static JGDI newInstance(String url) throws JGDIException {
        log.entering(JGDIFactory.class.getName(), "newInstance", url);
        initLib();
        com.sun.grid.jgdi.jni.JGDIImpl ret = new com.sun.grid.jgdi.jni.JGDIImpl();
        ret.init(url);
        log.exiting(JGDIFactory.class.getName(), "newInstance", ret);
        return ret;
    }

    /**
     * Get a synchronized instance of a <code>JGDI</code> object.
     *
     * @param url  JGDI connection url in the form
     *             <code>bootstrap://&lt;SGE_ROOT&gt;@&lt;SGE_CELL&gt;:&lt;SGE_QMASTER_PORT&gt;</code>
     * @throws com.sun.grid.jgdi.JGDIException on any error on the GDI layer
     * @return the <code>JGDI<code> instance
     * @since  0.91
     */
    public static JGDI newSynchronizedInstance(String url) throws JGDIException {
        return (JGDI) Proxy.newProxyInstance(JGDIFactory.class.getClassLoader(), new Class[]{JGDI.class},
                new SynchronJGDIInvocationHandler(newInstance(url)));
    }

    private static class SynchronJGDIInvocationHandler implements InvocationHandler {

        private final JGDI jgdi;

        public SynchronJGDIInvocationHandler(JGDI jgdi) {
            this.jgdi = jgdi;
        }

        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
            try {
                synchronized (jgdi) {
                    return method.invoke(jgdi, args);
                }
            } catch (InvocationTargetException ex) {
                throw ex.getTargetException();
            }
        }
    }

    /**
     * Create a new event client which receives events from a jgdi
     * connection.
     *
     * @param url  JGDI connection url in the form
     *             <code>bootstrap://&lt;SGE_ROOT&gt;@&lt;SGE_CELL&gt;:&lt;SGE_QMASTER_PORT&gt;</code>
     * @param evcId  id of the event client (0 mean dynamically assigned)
     * @throws com.sun.grid.jgdi.JGDIException
     * @return the new event client
     */
    public static EventClientImpl createEventClient(String url, int evcId) throws JGDIException {
        return new EventClientImpl(url, evcId);
    }

    /**
     * return the jgdi shared library version string, e.g. 'GE maintrunk' or
     * 'GE 6.1u1'
     * @return the jgdi shared library version string
     */
    public static String getJGDIVersion() {
        try {
            initLib();
        } catch (JGDIException ex) {
            throw new IllegalStateException(ex.getCause());
        }
        if (versionString == null) {
            versionString = nativeSetJGDIVersion();
        }
        return versionString;
    }

    private static native String nativeSetJGDIVersion();

    /**
     *   Create a proxy object to the JGDI MBean.
     *
     *   @param  host  the qmaster host
     *   @param  port  port where qmasters MBeanServer is listening
     *   @param  credentials credentials for authentication
     *   @return the JGDI Proxy object
     */
    public static JGDIProxy newJMXInstance(String host, int port, Object credentials) {

        JMXServiceURL url;
        try {
            url = new JMXServiceURL(String.format("service:jmx:rmi:///jndi/rmi://%s:%d/jmxrmi", host, port));
        } catch (MalformedURLException ex) {
            throw new IllegalStateException("Invalid JMX url", ex);
        }

        return new JGDIProxy(url, credentials);
    }
}
