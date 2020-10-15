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
 *   and/or Swiss Center for Scientific Computing
 * 
 *   Copyright: 2002 by Sun Microsystems, Inc.
 *   Copyright: 2002 by Swiss Center for Scientific Computing
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jam.app;

import java.awt.event.*;
import java.awt.*;
import javax.swing.*;
import net.jini.core.entry.Entry;
import net.jini.admin.*;
import net.jini.discovery.*;
import net.jini.lookup.*;
import java.io.*;
import java.rmi.*;
import java.rmi.server.*;

import com.sun.grid.jam.util.JAMAdmin;
import com.sun.grid.jam.util.JAMAdminProxy;
import com.sun.grid.jam.queue.QueueInterface;

/**
 * Proxy for Native Application.
 *
 * @version 1.4, 09/22/00
 *
 * @see NativeAppAgent
 *
 * @author Eric Sharakan
 */
public class NativeApp
  extends AppProxy
{
  /**
   * Generate new NativeAppAgent object
   */
  public NativeApp()
  {
    appAgent = new NativeAppAgent();
  }

  /**
   * Implement superclass's abstract submit method.  Directly execute the
   * application on the machine hosting the queue service backend.
   */
  public void submit(QueueInterface queue)
    throws RemoteException, IOException, ClassNotFoundException
  {
    queue.run(appParams.getName());
  }
}


