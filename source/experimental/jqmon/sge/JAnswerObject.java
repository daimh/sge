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
package codine;

import java.lang.*;

import codine.*;

/**
  * Diese Klasse stellt die AnswerList als Java-Objekt dar.
  */
public class JAnswerObject {

	/* Status */
	protected int status = 0;

	/* Text */
	protected String text = null;

	/* Quality */
	protected int quality = 0;


	/** Konstruktoren */
	public JAnswerObject(int status, String text, int quality) {
		this.status  = status;
		this.text    = text;
		this.quality = quality;
	}


	public JAnswerObject(int status, String text) {
		this(status, text, 0);
	}


	public JAnswerObject(int status) {
		this(status, "No Description available");
	}


	public JAnswerObject() {
		this(0);
	}


	/****************************************/
	/* get und set                          */
	/****************************************/
	public void setStatus(int status) {
		this.status = status;
	}

	public int getStatus() {
		return status;
	}

	/* Text */
	public void setText(String text) {
		this.text = text;
	}

	public String getText() {
		return text;
	}

	/* Quality */
	public void setQuality(int quality) {
		this.quality = quality;
	}

	public int getQuality() {
		return quality;
	}

	public String toString() {
		return getText();
	}
	
}
