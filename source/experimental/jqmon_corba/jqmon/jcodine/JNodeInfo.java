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
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
package jcodine;

import java.lang.*;

import jqmon.*;
import jqmon.debug.*;



public class JNodeInfo  extends     Object {
   
   final static int NI_UNKNOWN         = 99;
	final static int NI_QUEUE           = 100;
	final static int NI_JOBROOT         = 101;
	final static int NI_JOB             = 102;
	final static int NI_HOSTROOT        = 103;
	final static int NI_HOST            = 104;
	final static int NI_COMPLEXROOT     = 105;
	final static int NI_COMPLEX         = 106;
	final static int NI_COMPLEXATTRIBUTE= 107;
   
	public String    title;
	public String    path;
	public int	     type   = NI_UNKNOWN;
	public boolean   tag    = false;
	public long      ID     = 0;

	public JNodeInfo() {
      title = "";
      path  = "";
   }
	
   public JNodeInfo(int Type, long iD, String Title) {
      type  = Type;
      ID    = iD;
      title = Title;
      path  = "";
   }
}
