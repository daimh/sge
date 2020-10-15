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
package com.sun.grid.jam.queue;

import java.io.Serializable;
import java.rmi.RemoteException;
import net.jini.core.entry.Entry;
import net.jini.core.event.RemoteEventListener;
import net.jini.core.event.EventRegistration;
import net.jini.admin.*;
import com.sun.grid.jam.app.AppParamsInterface;
import com.sun.grid.jam.admin.UserProperties;
import com.sun.grid.jam.util.JAMProxy;

/**
 * Queue interface service
 *
 * @version 1.5, 09/22/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 * @author Eric Sharakan
 *
 */
public interface QueueInterface
  extends JAMProxy
{

  void submit(AppParamsInterface appParams, Entry [] jobAttr,
              UserProperties userProperties)
    throws ApplicationSubmissionException;
  
  EventRegistration register(AppParamsInterface api, RemoteEventListener
                             queueListener)
    throws RemoteException;
  
  void run(String command)
    throws RemoteException;
  
  int getJobsCount()
    throws RemoteException;
}


