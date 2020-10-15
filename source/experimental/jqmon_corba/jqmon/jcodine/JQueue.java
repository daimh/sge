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



public class JQueue extends JCodObj implements Cloneable {

	private String qname			= null;
	private String qhostname	= null;
	private String tmpdir		= null;
	private long   jobslots		= 0;
	private long	seq_no		= 0;

	// try to put ComplexList field in Queue
	private JComplexList complexList	= null;   
	
	// create a new queue object
	public JQueue(JDebug d) {
		super(d);
	}


	// returns the name of the queue
	public String toString() {
		return getQName();
	}


	public Object clone() {
		JQueue q = null;
		try {
			q = (JQueue)super.clone();
		} catch (CloneNotSupportedException e) {
			debug.DPRINTF("Error in JQueue.java: " + e);
		}
		
		return (Object)q;
	}


	//*********************************
	//* get and set functions        
	//*********************************

	// QueueName 
	public String getQName() {
		return qname;
	}

	public void setQName(String s) {
		qname = s;
	}

	// Hostname 
	public String getQHostName() {
		return qhostname;
	}

	public void setQHostName(String s) {
		qhostname = s;
	}

	// TmpDir 
	public String getTmpDir() {
		return tmpdir;
	}

	public void setTmpDir(String s) {
		tmpdir = s;
	}

	// Seq_No 
	public long getSeqNo() {
		return seq_no;
	}
	
	public void setSeqNo(long l) {
		seq_no = l;
	}

	// Job_Slots 
	public long getJobSlots() {
		return jobslots;
	}

	public void setJobSlots(long l) {
		jobslots = l;
	}

	// Complex_List
	public JComplexList getComplexList(){
		return complexList;
	}
	
	public void setComplexList(JComplexList complexList){
		this.complexList = complexList;
	}

}
	
