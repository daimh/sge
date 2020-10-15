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
 *   Copyright: 2006 by Sun Microsystems, Inc
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jgdi.management;

import com.sun.grid.jgdi.JGDIException;
import com.sun.grid.jgdi.event.ConnectionClosedEvent;
import com.sun.grid.jgdi.event.ConnectionFailedEvent;
import com.sun.grid.jgdi.event.Event;
import com.sun.grid.jgdi.event.EventListener;
import com.sun.grid.jgdi.management.mbeans.JGDIJMXMBean;
import com.sun.grid.jgdi.util.Base64;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.Signature;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import javax.management.Attribute;
import javax.management.InstanceNotFoundException;
import javax.management.ListenerNotFoundException;
import javax.management.MBeanException;
import javax.management.MBeanServerConnection;
import javax.management.Notification;
import javax.management.NotificationListener;
import javax.management.ObjectName;
import javax.management.remote.JMXConnectionNotification;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;
import javax.net.ssl.SSLHandshakeException;

/**
 *  <p>
 *  This class can be used to communicate with qmaster over JMX.
 *  </p>
 */
public class JGDIProxy implements InvocationHandler, NotificationListener {

    private static final Map<Method, MethodInvocationHandler> handlerMap = new HashMap<Method, MethodInvocationHandler>();
    private ObjectName name;
    private final JMXServiceURL url;
    private JGDIJMXMBean proxy;
    private final Object credentials;
    private JMXConnector connector;
    private MBeanServerConnection connection;
    private Set<EventListener> listeners = Collections.<EventListener>emptySet();
    private boolean closeEventSent = false;

    /**
     *  Create a new proxy to the jgdi MBean
     *
     *  @param url  jmx connection url to qmaster
     *         allows username/password authentication this parameter must be a
     *         string array. The first element is the username, the second element
     *         is the password.
     *  @param credentials the credentials for jmx authentication
     *
     */
    public JGDIProxy(JMXServiceURL url, Object credentials) {
        this.url = url;
        this.credentials = credentials;
        Class<?>[] types = new Class<?>[]{JGDIJMXMBean.class};
        proxy = (JGDIJMXMBean) Proxy.newProxyInstance(JGDIJMXMBean.class.getClassLoader(), types, this);
    }

    /**
     *   Get the dynamic proxy object
     *
     *   @return the dynamic proxy object
     */
    public JGDIJMXMBean getProxy() {
        return proxy;
    }

    /**
     *   Get the MBeanServerConnection connection
     *
     *   @return the MBeanServerConnection connection
     */
    public MBeanServerConnection getMBeanServerConnection() {
        return connection;
    }

    
    /**
     * Set up the ssl context
     * @param caTop  ca top directory if the Grid Engine CA ($SGE_ROOT/$SGE_CELL/common/sgeCA
     * @param ks     keystore of the user
     * @param pw     password for the keystore
     * @deprecated
     */
    public static void setupSSL(File caTop, KeyStore ks, char[] pw) {
        SSLHelper.getInstanceByCaTop(caTop).setKeystore(ks, pw);
    }

    /**
     * Set up the ssl context
     * @param caTop  ca top directory if the Grid Engine CA ($SGE_ROOT/$SGE_CELL/common/sgeCA
     * @param ks     keystore file of the user
     * @param pw     password for the keystore
     * @deprecated
     */
    public static void setupSSL(File caTop, File ks, char[] pw) {
        SSLHelper.getInstanceByCaTop(caTop).setKeystore(ks, pw);
    }

    /**
     * Reset the SSL setup.
     * 
     * @param caTop the ca top directory of the cluster
     * @deprecated
     */
    public static void resetSSL(File caTop) {
        SSLHelper.getInstanceByCaTop(caTop).reset();
    }

    /**
     * Set up the ssl context
     * @param serverHostname the server hostname
     * @param serverPort the server port
     * @param caTop  ca top directory if the Grid Engine CA ($SGE_ROOT/$SGE_CELL/common/sgeCA
     * @param ks     keystore of the user
     * @param pw     password for the keystore
     */
    public static void setupSSL(String serverHostname, int serverPort, File caTop, KeyStore ks, char[] pw) {
        SSLHelper.getInstanceByKey(serverHostname, serverPort, caTop).setKeystore(ks, pw);
    }

    /**
     * Set up the ssl context
     * @param serverHostname the server hostname
     * @param serverPort the server port
     * @param caTop  ca top directory if the Grid Engine CA ($SGE_ROOT/$SGE_CELL/common/sgeCA
     * @param ks     keystore file of the user
     * @param pw     password for the keystore
     */
    public static void setupSSL(String serverHostname, int serverPort, File caTop, File ks, char[] pw) {
        SSLHelper.getInstanceByKey(serverHostname, serverPort, caTop).setKeystore(ks, pw);
    }

    /**
     * Reset the SSL setup.
     * @param serverHostname the server hostname
     * @param serverPort the server port
     * @param caTop the ca top directory of the cluster
     */
    public static void resetSSL(String serverHostname, int serverPort, File caTop) {
        SSLHelper.getInstanceByKey(serverHostname, serverPort, caTop).reset();
    }


    /**
     *   Register an jgdi event listener.
     *
     *   @param lis the jgdi event listener
     */
    public synchronized void addEventListener(EventListener lis) {
        Set<EventListener> newListeners = new HashSet<EventListener>(listeners.size() + 1);
        newListeners.addAll(listeners);
        newListeners.add(lis);
        listeners = newListeners;
    }

    /**
     *   Remove a jgdi event listener.
     *
     *   @param lis the jgdi event listener
     */
    public synchronized void removeEventListener(EventListener lis) {
        Set<EventListener> newListeners = new HashSet<EventListener>(listeners);
        newListeners.remove(lis);
        listeners = newListeners;
    }

    /**
     *  JMX will call this method of a notification for the proxy is available.
     *
     *  If the notification contains a JGDI event it will be propagated to all
     *  registered JGDI event listeners.
     *
     *  @param notification  the notification
     *  @param handback      the handback object
     */
    public void handleNotification(Notification notification, Object handback) {

        Event evt = null;
        if (notification.getUserData() instanceof Event) {
            evt = (Event) notification.getUserData();
        } else if (JMXConnectionNotification.CLOSED.equals(notification.getType())) {
            synchronized (this) {
                if (connector != null) {
                    try {
                        connector.removeConnectionNotificationListener(this);
                        close();
                        evt = new ConnectionClosedEvent(System.currentTimeMillis() / 1000, 0);
                    } catch (ListenerNotFoundException ex) {
                    // Ignore
                    }
                }
            }
        } else if (JMXConnectionNotification.FAILED.equals(notification.getType())) {
            synchronized (this) {
                if (connector != null) {
                    try {
                        connector.removeConnectionNotificationListener(this);
                        close();
                        evt = new ConnectionFailedEvent(System.currentTimeMillis() / 1000, 0);
                    } catch (ListenerNotFoundException ex) {
                    // Ignore
                    }
                }
            }
        }
        if (evt != null) {
            boolean isCloseEvent = ConnectionClosedEvent.class.isAssignableFrom(evt.getClass());
            if (!isCloseEvent || !closeEventSent) {
                Set<EventListener> tmpLis = null;
                synchronized (this) {
                    tmpLis = listeners;
                }
                for (EventListener lis : tmpLis) {
                    lis.eventOccured(evt);
                }
            }
            if (isCloseEvent) {
                closeEventSent = true;
                close();
            }
        }
    }

    /**
     *   Determine of the connection to the JMX MBean servers is established
     *
     *   @return <code>true</code> if the connection is established otherwise
     *           <code>false</code>
     */
    public synchronized boolean isConnected() {
        return connection != null;
    }

    /**
     *   Close the connection to the MBean server
     */
    public synchronized void close() {

        if (connection != null) {
            try {
                connector.close();
            } catch (IOException ex) {
            // Ignore it
            } finally {
                connection = null;
                connector = null;
            }
        }
    }

    private void connect() throws JGDIException {
        if (connection == null) {
            Map<String, Object> env = new HashMap<String, Object>();
            env.put("jmx.remote.default.class.loader", JGDIJMXMBean.class.getClassLoader());
            if (credentials != null) {
                env.put("jmx.remote.credentials", credentials);
            }
            try {
                closeEventSent = false;
                connector = JMXConnectorFactory.connect(url, env);
                connection = connector.getMBeanServerConnection();

                name = JGDIAgent.getObjectNameFromConnectionId(connector.getConnectionId());
                if (name == null) {
                    throw new JGDIException("jmx connection id contains no jgdi session id. Please check qmaster's JAAS configuration (JGDILoginModule)");
                }
                connection.addNotificationListener(name, this, null, null);
                connector.addConnectionNotificationListener(this, null, connector.getConnectionId());
            } catch (NullPointerException ex) {
                close();
                throw new JGDIException(ex, "jgdi mbean is null");
            } catch (InstanceNotFoundException ex) {
                close();
                throw new JGDIException(ex, "jgdi mbean not active in qmaster");
            } catch (IOException ex) {
                close();
                Throwable realError = null;
                if ((realError = findCause(ex, SSLHandshakeException.class)) != null) {
                    throw new JGDIException(realError, "SSL error: " + realError.getLocalizedMessage());
                } else if ((realError = findCause(ex, java.net.ConnectException.class)) != null) {
                    throw new JGDIException(realError, "Connection refused");
                }
                throw new JGDIException(ex, "connection to " + url + "failed");
            }
        }
    }

    private Throwable findCause(Throwable ex, Class<? extends Exception> type) {
        for (Throwable t = ex; t != null; t = t.getCause()) {
            if (type.isAssignableFrom(t.getClass())) {
                return t;
            }
        }
        return null;
    }

    /**
     *   Invoke a method on the remote MBean.
     *
     *   @param proxy the JGDIProxy object
     *   @param method the method which should be invoked
     *   @param args   arguments for the method
     */
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        connect();
        return getHandler(method).invoke(connection, name, args);
    }

    private static MethodInvocationHandler getHandler(Method m) {
        MethodInvocationHandler ret = null;
        synchronized (handlerMap) {
            ret = handlerMap.get(m);
            if (ret == null) {
                ret = createHandler(m);
                handlerMap.put(m, ret);
            }
        }
        return ret;
    }

    private static MethodInvocationHandler createHandler(Method method) {

        // Is the method an attribute getter?
        String attrname = isAttributeGetter(method);
        if (attrname != null) {
            return new AttributeGetter(attrname);
        }

        // Is the method an attribute setter
        attrname = isAttributeSetter(method);
        if (attrname != null) {
            return new AttributeSetter(attrname);
        }

        if (method.getName().equals("toString")) {
            return new ToStringMethodInvocationHandler();
        }

        return new MethodInvoker(method);
    }

    private static interface MethodInvocationHandler {

        public Object invoke(MBeanServerConnection connection, ObjectName name, Object[] args) throws Throwable;
    }

    private static class AttributeGetter implements MethodInvocationHandler {

        private String attribute;

        public AttributeGetter(String attribute) {
            this.attribute = attribute;
        }

        public Object invoke(MBeanServerConnection connection, ObjectName name, Object[] args) throws Throwable {
            return connection.getAttribute(name, attribute);
        }
    }

    private static class AttributeSetter implements MethodInvocationHandler {

        private String attribute;

        public AttributeSetter(String attribute) {
            this.attribute = attribute;
        }

        public Object invoke(MBeanServerConnection connection, ObjectName name, Object[] args) throws Throwable {
            connection.setAttribute(name, new Attribute(attribute, args[0]));
            return null;
        }
    }

    private static class MethodInvoker implements MethodInvocationHandler {

        private final String[] signature;
        private final String methodName;
        private final Class returnType;

        public MethodInvoker(Method method) {
            this.methodName = method.getName();
            Class<?>[] paramTypes = method.getParameterTypes();
            this.signature = new String[paramTypes.length];
            for (int i = 0; i < paramTypes.length; i++) {
                this.signature[i] = paramTypes[i].getName();
            }
            returnType = method.getReturnType();
        }

        public Object invoke(MBeanServerConnection connection, ObjectName name, Object[] args) throws Throwable {
            try {
                Object ret = connection.invoke(name, methodName, args, signature);

                if (ret != null) {
                    try {
                        return returnType.cast(ret);
                    } catch (Exception ex) {
                        throw new IllegalStateException("return type does not match (" +
                                typeToString(returnType) + " <-> " + typeToString(ret.getClass()));
                    }
                } else {
                    return ret;
                }
            } catch (MBeanException ex) {
                // Is it a real exception (thrown be the component implementation?
                if (ex.getTargetException() instanceof InvocationTargetException) {
                    throw ((InvocationTargetException) ex.getTargetException()).getTargetException();
                } else {
                    throw ex;
                }
            }
        }

        private static String typeToString(Class<?> clazz) {
            if (clazz != null) {
                return String.format("%s@%s(%s)", clazz.getName(), clazz.getProtectionDomain().getCodeSource(),
                        clazz.getClassLoader().getClass().getName());
            } else {
                return "null";
            }
        }
    }

    private static class ToStringMethodInvocationHandler implements MethodInvocationHandler {

        public Object invoke(MBeanServerConnection connection, ObjectName name, Object[] args) throws Throwable {
            return String.format("[JGDIClientProxy to %s]", name.toString());
        }
    }

    private static String isAttributeGetter(Method method) {

        Class returnType = method.getReturnType();

        String attrName = null;

        if (!returnType.equals(Void.TYPE)) {
            Class[] paramTypes = method.getParameterTypes();
            if (paramTypes.length == 0) {
                String methodName = method.getName();
                if ((returnType.equals(Boolean.TYPE) || returnType.equals(Boolean.class)) && methodName.length() > 2 && methodName.startsWith("is")) {
                    attrName = methodName.substring(2);
                } else if (method.getName().length() > 3 && method.getName().startsWith("get")) {
                    attrName = methodName.substring(3);
                }
            }
        }

        return attrName;
    }

    /**
     * Get a new instanceof of <code>ComponentAttributeDescriptor</code> from an
     * attribute getter.
     *
     * @param method the attribute getter
     * @return the <code>CoComponentAttributeDescriptorcode> or <code>null</code> if <code>method</code>
     *         is not an attribute getter
     */
    public static String isAttributeSetter(Method method) {

        Class[] paramTypes = method.getParameterTypes();
        Class returnType = method.getReturnType();

        if (returnType.equals(Void.TYPE) && paramTypes.length == 1 && method.getName().length() > 3 && method.getName().startsWith("set")) {
            String attrName = method.getName();
            return attrName.substring(3);
        } else {
            return null;
        }
    }

    /**
     * Create JMX credentials for password less authentication with a keystore
     * @param ks        the keystore
     * @param username  the username
     * @param pw        password of the private key in the keystore
     * @return the jmx credentials
     * @throws com.sun.grid.jgdi.JGDIException
     */
    public static String[] createCredentialsFromKeyStore(KeyStore ks, String username, char[] pw) throws JGDIException {
        try {

            PrivateKey pk = (PrivateKey) ks.getKey(username, pw);

            String algorithm = "MD5withRSA";
            Signature s = Signature.getInstance(algorithm);
            s.initSign(pk);

            byte[] message = "Super secret message".getBytes();

            s.update(message);

            byte[] signature = s.sign();

            Properties props = new Properties();
            props.put("algorithm", algorithm);
            props.put("message", Base64.encode(message));
            props.put("signature", Base64.encode(signature));

            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            props.store(bos, null);

            return new String[]{
                username,
                bos.toString()
            };
        } catch (Exception ex) {
            throw new JGDIException("Can not create credentials from keystore", ex);
        }
    }
}
