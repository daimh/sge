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
package com.sun.grid.jam.gridengine.queue;

import java.util.ArrayList;

/**
 * This class represent queue type.
 *
 * @version 1.3, 09/22/00
 *
 * @author Rafal Piotrowski
 */
public class QueueType 
{
  // Atributes
  /** Queue Type  */
  private int type;

  /** Queue type can be combination of many states. */
  private ArrayList types;

  
  // SGE's Queue types
  /** Batch Queue */
  public final static int BQ = 0x01;
  public final static Integer _BQ = new Integer(BQ);
  
  /** Interactive Queue */  
  public final static int IQ = 0x02;
  public final static Integer _IQ = new Integer(IQ);
  
  /** Checkpointing Queue */
  public final static int CQ = 0x04;
  public final static Integer _CQ = new Integer(CQ);
  
  /** Parallel Queue */
  public final static int PQ = 0x08;
  public final static Integer _PQ = new Integer(PQ);
  
  /**
   * Transfer Queue
   * why not 0x16 ? - because queue of this type can not be of any
   * other type as well
   */
  public final static int TQ = 0x10;
  public final static Integer _TQ = new Integer(TQ);


  //======== constructors =========

  public QueueType()
  {
    types = new ArrayList();
    setType(BQ);
  }
  
  public QueueType(int type)
  {
    types = new ArrayList();
    setType(type);
  }

  
  //======== getters =============

  public int getType() 
  {
    return type;
  }


  //======== setters =============
  
  /**
   * To set the type to BQ and IQ, user has to add them (i.e. BQ + IQ)
   * before calling this method. 
   *
   * @param type - queue type
   */
  public void setType(int type)
  {
    this.type = type;
    if(!types.isEmpty())
      types.clear();
    parseType(type);
  }    


  //======== public methods ======

  public String toString()
  {
    StringBuffer bf = new StringBuffer();
    try {
      String[] ts = getTypesAsString();
      for(int i = 0; i < ts.length; i++)
        bf = bf.append(" ").append(ts[i]);
    } catch(QueueTypeException e){
      return "Incorrect queue type";
    }
    return bf.toString();
  }

  /**
   * this method is used to ask question such as
   * is the queue of this type?
   */
  public boolean containsType(int type)
  {
    return types.contains(new Integer(type));
  }

  /**
   * this method is used to ask question such as
   * is the queue of this type?
   */
  public boolean containsType(Integer type)
  {
    return types.contains(type);
  }
  
  //======== private methods ======
  
  private void parseType(int type)
  {
    int nextType;
    if(type == 0)
      return;
    else if( (type - PQ) >= 0 ) {
      types.add(_PQ);
      nextType = type - PQ;
      parseType(nextType);
    }
    else if( (type - CQ) >= 0 ) {
      types.add(_CQ);
      nextType = type - CQ;
      parseType(nextType);
    }
    else if( (type - IQ) >= 0) {
      types.add(_IQ);
      nextType = type - IQ;
      parseType(nextType);
    }
    else if( (type - BQ) == 0) {
      // this is the ultimate type that a queue has to of
      // otherwise there is something wrong
      types.add(_BQ);
      nextType = type - BQ;
      parseType(nextType);
    }
    else {
      // in this case set the type to BQ only
      setType(BQ);
    }
  }
  
  private String getTypeAsString(int type)
    throws QueueTypeException
  {
    StringBuffer sb = new StringBuffer();

    switch(type){
    case BQ:
      sb.append( "Batch" );
      break;
    case IQ:
      sb.append( "Interactive");
      break;
    case CQ: 
      sb.append( "Checkpointing" );
      break;
    case PQ:
      sb.append( "Parallel" );
      break;
    case TQ:
      sb.append( "Transfer" );
      break;
    default:
      throw new QueueTypeException("Unknown Queue Type");
    }
    return sb.toString();
  }

  private int[] getTypes()
  {
    if(types.isEmpty())
      return null;
    int size = types.size();
    int[] ti = new int[size];
    for(int i = 0; i < size; i++)
      ti[i] = ((Integer)types.get(i)).intValue();
    return ti;
  }
  
  private String[] getTypesAsString()
    throws QueueTypeException
  {
    if(types.isEmpty())
      return null;
    int size = types.size();
    String[] ts = new String[size];
    for(int i = 0; i < size; i++){
      int ti = ((Integer)types.get(i)).intValue();
      ts[i] = getTypeAsString(ti);
    }
    return ts;
  }
  
}
