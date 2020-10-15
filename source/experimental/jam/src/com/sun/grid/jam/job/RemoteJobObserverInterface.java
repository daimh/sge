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
package com.sun.grid.jam.job;

import java.io.IOException;
import java.io.Reader;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.net.URL;
import net.jini.core.entry.Entry;
import net.jini.space.JavaSpace;
import net.jini.core.transaction.TransactionException;

import com.sun.grid.jam.util.RemoteInputStreamInterface;
  
/**
 * The observer interface for monitoring and controlling a job
 *
 * @version 1.4, 09/22/00
 *
 * @author Nello Nellari
 *
 */
interface RemoteJobObserverInterface
  extends Remote
{
  RemoteInputStreamInterface getReader(URL resource)
    throws RemoteException;
  //RemoteOutputStreamInterface getReader(URL resource)
  //  throws RemoteException;
}

