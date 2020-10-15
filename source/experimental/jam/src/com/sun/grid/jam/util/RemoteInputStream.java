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
package com.sun.grid.jam.util;

import java.rmi.Remote;
import java.rmi.RemoteException;
import java.io.FileInputStream;
import java.io.Serializable;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.net.URL;

public class RemoteInputStream
  extends Thread
  implements RemoteInputStreamInterface, Serializable
{
  private String fileName;
  private FileInputStream input;

  public RemoteInputStream(String fileName)
  {
    this.fileName = fileName;
  }

  public RemoteInputStream(URL url)
  {
    this.fileName = url.getFile();
  }

  /**
   * This method takes care for initalizing the input stream. So it
   * should be the first function that is executed after creating an
   * instance of this class. 
   */
  public synchronized int available()
    throws RemoteException
  {
    if(input == null) {
      try {
        input = new FileInputStream(fileName);
      } catch(IOException ioe) {
        // Still could not open input stream
        // NOTE: It is not neccessary to throw an exeption at this
        // point, we can return -1 to show that there was an error. It 
        // works because available method returns 0 or number of
        // bytes that can be read with out blocking. 
        //throw new RemoteException(ioe.toString());
        return -1;
      }
    } // input != null
    // chack for available bytes
    try {
      return input.available();
    } catch(IOException ioe) {
      throw new RemoteException(ioe.toString());
    }
  }

  /**
   * IMPORTANT: This method should not be executed without checking if
   * there are any bytes available.
   */
  public synchronized byte[] read(int max)
    throws RemoteException
  {
    try {
      byte[] b = new byte[max];
      int n = input.read(b, 0, max);
      if(n != max) {
        byte[] bm = new byte[n];
        for(int i = 0; i < n; i++) {
          bm[i] = b[i];
        }
        return bm;
      }
      return b;
    } catch(IOException ioe) {
      throw new RemoteException(ioe.toString());
    }
  }
  
  public void close()
    throws RemoteException
  {
    try {
      if(input != null) {
        input.close();
      }
    } catch(IOException ioe) {
      throw new RemoteException(ioe.toString());
    }
  }
}


