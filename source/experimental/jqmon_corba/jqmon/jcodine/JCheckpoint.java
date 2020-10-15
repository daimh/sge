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


// thats the java-side checkpoint object
	
public class JCheckpoint extends JCodObj implements Cloneable {

	String	   name		    		= null;
   String      ckpt_interface    = null;
   String      ckpt_command      = null;
   String      migr_command      = null;
   String      rest_command      = null;
   String      ckpt_dir          = null;
   JQueueList  queue_list        = null;
   String      when              = null;
   String      signal            = null;
   String      clean_command     = null;
   
   public Codine.Checkpoint checkpoint = null;
   
	// create a new checkpoint object 
	public JCheckpoint(JDebug d) {
		super(d);
	}

   public JCheckpoint(JDebug debug, Codine.Checkpoint checkpoint, org.omg.CORBA.Context ctx) {
		this(debug);
		this.checkpoint = checkpoint;
		try {
			name = checkpoint.get_name(ctx);
		}
		catch(Exception e) {
			// shouldn't exist
		}
	}

	// returns the name of the checkpoint 
	public String toString() {
		return getName();
	}


	// get and set functions 
   
	public void setName(String s) {
		name = s;
	}
	public String getName() {
		return name;
	}

   public void setInterface(String s) {
      ckpt_interface = s;   
   }
   public String getInterface() {
      return ckpt_interface;   
   }
   
   public void setCkptCommand(String s) {
      ckpt_command = s;   
   }
   public String getCkptCommand() {
      return ckpt_command;   
   }
	
   public void setMigrCommand(String s) {
      migr_command = s;   
   }
   public String getMigrCommand() {
      return migr_command;   
   }
   
   public void setRestCommand(String s) {
      rest_command = s;   
   }
   public String getRestCommand() {
      return rest_command;   
   }
   
   public void setCkptDir(String s) {
      ckpt_dir = s;   
   }
   public String getCkptDir() {
      return ckpt_dir;   
   }
   
   public void setQueueList(JQueueList ql) {
      queue_list = ql;   
   }
   public JQueueList getQueueList() {
      return queue_list;   
   }
   
   public void setWhen(String s) {
      when = s;   
   }
   public String getWhen() {
      return when;   
   }
   
   public void setSignal(String s) {
      signal = s;   
   }
   public String getSignal() {
      return signal;   
   }
   
   public void setCleanCommand(String s) {
      clean_command = s;   
   }
   public String getCleanCommand() {
      return clean_command;   
   }
}
