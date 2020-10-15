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
package com.sun.grid.jam.gridengine.util;

import java.util.ArrayList;

/**
 * This class allows us to abstract away from the underlying
 * representation of a list that stores data.
 * We can choose what ever list representation we want, and the only
 * thing that will have to change is this class, all the subclasses
 * will stay intact.
 *
 * @version 1.2, 09/22/00
 *
 * @author Rafal Piotrowski
 */
public abstract class SGEList
{
  //============ attributes =============
  
  private ArrayList list;
  private int index;

  //============ constructors ===========

  public SGEList()
  {
    list = new ArrayList();
    index = 0;
  }
  
  //=========== getters ===============

  /**
   * Checks if the list is empty.
   *
   * @return true if is empty or false if isn't empty
   *
   * @see java.util.ArrayList#isEmpty
   */
  public boolean isEmpty()
  {
    return list.isEmpty();
  }

  /**
   * Gets size of the list.
   *
   * @return size of the list
   *
   * @see java.util.ArrayList#size
   */
  public int size()
  {
    return list.size();
  }

  /**
   * Gets an object at the position specified by index value.
   * Boundary check is performed pior to retreival of an object. If
   * value of the index variable is bigger than the size, null value
   * is returned.
   *
   * @param index - position of an element to retreive
   *
   * @return object element at the position specified by index value
   *
   * @see java.util.ArrayList#get
   */
  public Object getElem(int index)
  {
    if(index > list.size())
      return null;
    return list.get(index);
  }

  /**
   * Retreive first element object from the list. If the list is empty 
   * a null value is returned.
   *
   * @return First element from the list.
   *
   * @see java.util.ArrayList#get
   */
  public Object firstElem()
  {
    if(!isEmpty()) 
      return list.get(0);

    index = 0;
    return null;
  }

  /**
   * Gets next element from the list.
   *
   * @return Next element from the list.
   *
   * @see java.util.ArrayList#get
   */
  public Object nextElem()
  {
    if(index > list.size()) {
      index = 0;
      return null;
    }

    return list.get(++index);
  }

  /**
   * Gets next element after the position specified by index variable
   * value.
   *
   * @return Next element from the list after the position index.
   *
   * @see java.util.ArrayList#get
   */
  public Object nextElem(int index)
  {
    if(++index > list.size()) 
      return null;
    
    this.index = index;
    return list.get(this.index);
  }

  //=========== setters ================

  /**
   * Adds en object element to the internal list.
   *
   * @param obj - object element to be added
   *
   * @see java.util.ArrayList#add
   */
  public void addElem(Object obj)
  {
    list.add(obj);
  }
}
