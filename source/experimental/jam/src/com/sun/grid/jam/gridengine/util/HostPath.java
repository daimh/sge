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
/**
 * This package contains utility classes that are used by other
 * classes in com.sun.grid.jam.gridengine, .sge.job, .sge.queue
 * packages.
 *
 * @version 1.1, 06/21/00
 *
 * @author Rafal Piotrowski
 */

package com.sun.grid.jam.gridengine.util;

/**
 * This class is a reflection of SGE host path list.
 *
 * @version 1.1, 06/21/00
 *
 * @author Rafal Piotrowski
 */
public class HostPath
{
  //=========== arguments ============

  /**
   * This is an optional variable. It doesn't need to be set.
   */
  private String host;

  /**
   * This is a mandatory variable. It does need to be set.
   */
  private String path;

  //=========== constructors ============

  public HostPath(String path)
  {
    this.path = path;
    host = null;
  }
  
  public HostPath(String host, String path)
  {
    this.path = path;
    this.host = host;
  }
  
  //=========== getters ============

  /**
   * Gets variable path value.
   *
   * @return Value of the path variable.
   */
  public String getPath()
  {
    return path;
  }

  /**
   * Gets variable host value.
   *
   * @return Value of the host variable.
   */
  public String getHost()
  {
    return host;
  }

  //=========== setters ============

  /**
   * Sets path variable.
   *
   * @param path - new value for path variable
   */
  public void setPath(String path)
  {
    this.path = path;
  }
  
  /**
   * Sets host variable.
   *
   * @param host - new value for host variable
   */
  public void setHost(String host)
  {
    this.host = host;
  }
}
