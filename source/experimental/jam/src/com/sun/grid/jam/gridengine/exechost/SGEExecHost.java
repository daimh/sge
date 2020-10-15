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
package com.sun.grid.jam.gridengine.exechost;

/**
 * Summary.
 *
 * @version 1.3, 09/22/00
 *
 * @author Rafal Piotrowski
 */
public class SGEExecHost
{
  private String name;
  private String manufacturer;
  private String model;
  private String arch;
  private String osName;
  private String osRev;
  private String osArch;
  private String load;
  private int processors;
  
  //========== constructors ==========

  public SGEExecHost() {}
  
  //========== getters ==============

  public String getName()
  {
    return name;
  }

  public String getArch()
  {
    return arch;
  }

  public int getProcessors()
  {
    return processors;
  }
  
  public String getLoad()
  {
    return load;
  }
  
  //========== setters ==============

  public void setName(String name)
  {
    this.name = name;
  }

  public void setArch(String arch)
  {
    this.arch = arch;
  }

  public void setProcessors(int processors)
  {
    this.processors = processors;
  }
  
  public void setLoad(String load)
  {
    this.load = load;
  }  
}
