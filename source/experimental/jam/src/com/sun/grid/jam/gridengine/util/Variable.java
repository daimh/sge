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
 * This class is a reflection of SGE variable structure.
 *
 * @version 1.1, 06/21/00
 *
 * @author Rafal Piotrowski
 */
public class Variable
{
  //=========== arguments ============
  
  /** name of the variable */
  private String name;
  
  /** value of the variable */
  private String value;

  //=========== constructors ============

  /**
   * Variable (e.g environment variable SHELL) 
   *
   * @param name - name of the variable
   * @param value - value of the variable
   */
  public Variable(String name, String value)
  {
    this.name = name;
    this.value = value;
  }
  
  //=========== getters ============

  /**
   * Gets name of the variable.
   *
   * @return Name of the variable.
   */
  public String getName()
  {
    return name;
  }

  /**
   * Gets value of the variable.
   *
   * @return Value of the variable.
   */
  public String getValue()
  {
    return value;
  }

  //=========== setters ============

  /**
   * Sets name of the variable.
   *
   * @param name - name of the variable
   */
  public void setName(String name)
  {
    this.name = name;
  }

  /**
   * Sets value of the variable.
   *
   * @param value - value of the variable
   */
  public void setValue(String value)
  {
    this.value = value;
  }
}
