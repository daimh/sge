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

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import java.util.*;

import jqmon.*;
import jqmon.debug.*;
import jqmon.util.*;


public class JCalendarGeneralPanel extends JPanel {

	protected JDebug debug = null;

	protected static ResourceBundle messages = null;

	JGridBagLayout layout = null;

	protected JTextField name;
	protected JTextField	yearCalendar;
   protected JTextField weekCalendar;
	
	
	public JCalendarGeneralPanel(JDebug d) {
		super();
		layout = new JGridBagLayout();
		setLayout(layout);
		debug = d;
		Locale locale = Locale.getDefault();
		messages = ResourceBundle.getBundle("jqmon/views/MessageBundle", locale);
		initGUI();
	}

	protected void initGUI() {
		debug.DENTER("JCalendarGeneralPanel.initGUI");
		
		JLabel label1 = new JLabel(messages.getString("calendarlabel1"), JLabel.RIGHT);
		JLabel label2 = new JLabel(messages.getString("calendarlabel2"), JLabel.RIGHT);
		JLabel label3 = new JLabel(messages.getString("calendarlabel3"), JLabel.RIGHT);
		
		name			  = new JTextField(30);
		yearCalendar  = new JTextField(20);
		weekCalendar  = new JTextField(20);
		
		name.setEditable        (true);
		yearCalendar.setEditable(true);
      weekCalendar.setEditable(true);
		
		name.requestFocus();

		layout.constrain(this, label1, 1,1,1,1, GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
		layout.constrain(this, label2, 1,2,1,1, GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0 ,4,4,4,4);
      layout.constrain(this, label3, 1,3,1,1, GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0 ,4,4,4,4);
		layout.constrain(this, name,         2,1,2,1, GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, yearCalendar, 2,2,2,1, GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, weekCalendar, 2,3,2,1, GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		 
		layout.constrain(this, new JPanel(), 1, 300, 200,1, GridBagConstraints.BOTH, GridBagConstraints.NORTH, 1.0, 1.0, 0,0,0,0);
		debug.DEXIT();
	}


	//*****************************************
	//* get and set funktions (without Debug)  
	//*****************************************

   // ---name------------------------------
	public void setName(String s) {
		name.setText(s);
	}

	public String getName() {
		return name.getText();
	}
   // -------------------------------------

   // ---year_calendar---------------------
	public void setYearCalendar(String s) {
		yearCalendar.setText(s);
	}

	public String getYearCalendar() {
		return yearCalendar.getText();
	}
   // -------------------------------------

   // ---week_calendar---------------------
	public void setWeekCalendar(String s) {
		weekCalendar.setText(s);
	}

	public String getWeekCalendar() {
		return weekCalendar.getText();
	}
   // -------------------------------------
}
