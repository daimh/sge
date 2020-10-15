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
package com.sun.grid.jam.queue.entry;

import net.jini.entry.AbstractEntry;
import net.jini.lookup.entry.ServiceControlled;

/**
 * Reports the general info of a Queue service.
 * Contains the parameters required
 * by job submission
 *
 * @version 1.2, 09/22/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 * @author Eric Sharakan
 *
 */
public class QueueInfo
  extends AbstractEntry
  implements ServiceControlled
{
  public String name;
  public QueueStatus condition;
  public Boolean interactive;
  public Boolean batch;
  public Boolean dedicated;
  public String rms;

  public QueueInfo()
  {
    super();
  }

  public QueueInfo(String name, QueueStatus condition, Boolean
                   interactive, Boolean batch, Boolean dedicated, String rms)
  {
    this.name = name;
    this.condition = condition;
    this.interactive = interactive;
    this.batch = batch;
    this.dedicated = dedicated;
    this.rms = rms;
  }
}
