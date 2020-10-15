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
package jqmon.views;

import java.awt.*;
import java.awt.event.*;

/* Swing 1.0.x 
import com.sun.java.swing.*;
import com.sun.java.swing.event.*;
import com.sun.java.swing.border.*;
*/

/* Swing 1.1 */
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import java.util.*;

import jqmon.*;
import jqmon.debug.*;
import jqmon.util.*;

public class JHostGeneralPanel extends JPanel {

	protected JDebug debug = null;

	protected static ResourceBundle messages = null;

	JGridBagLayout layout = null;

	protected JTextField name;
	protected JTextField	ltHeardFrom;
	
	
	public JHostGeneralPanel(JDebug d) {
		super();
		layout = new JGridBagLayout();
		setLayout(layout);
		debug = d;
		Locale locale = Locale.getDefault();
		messages = ResourceBundle.getBundle("jqmon/views/MessageBundle", locale);
		initGUI();
	}

	protected void initGUI() {
		debug.DENTER("JHostGeneralPanel.initGUI");
		
		JLabel label1 = new JLabel(messages.getString("hostlabel1"), JLabel.RIGHT);
		JLabel label2 = new JLabel(messages.getString("hostlabel2"), JLabel.RIGHT);
		
		name			  = new JTextField(30);
		ltHeardFrom	  = new JTextField(10);
		
		
		name.setEditable(true);
		ltHeardFrom.setEditable(true);
		
		name.requestFocus();

		layout.constrain(this, label1, 1,1,1,1, GridBagConstraints.HORIZONTAL,
								GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
		layout.constrain(this, name,  2,1,2,1, GridBagConstraints.HORIZONTAL,
								GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, label2, 1,2,1,1, GridBagConstraints.HORIZONTAL,
								GridBagConstraints.NORTHEAST, 0.0, 0.0 ,4,4,4,4);
		layout.constrain(this, ltHeardFrom, 2,2,2,1, GridBagConstraints.HORIZONTAL,
								GridBagConstraints.NORTHWEST, 1.0, 0.0 , 4,4,4,4);
		 
		layout.constrain(this, new JPanel(), 1,100, 100,1, GridBagConstraints.BOTH,
								GridBagConstraints.NORTH, 1.0, 1.0, 0,0,0,0);
		debug.DEXIT();
	}


	/*****************************************/
	/* Get und Set-Funktionen (ohne Debug)   */
	/*****************************************/

	/* Name */
	public void setName(String s) {
		name.setText(s);
	}

	public String getName() {
		return name.getText();
	}


	/* ltHeardFrom */
	public void setLtHeardFrom(long l) {
		Long s = new Long(l);
		ltHeardFrom.setText(s.toString());
	}

	public long getLtHeardFrom() {
		Long s = null;
		try {
			s = new Long(ltHeardFrom.getText());
		} catch (NumberFormatException e) {
			s = new Long(0);
		}
		return s.longValue();
	}
}
