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


public class JCheckpointGeneralPanel extends JPanel {

	protected JDebug debug = null;

	protected static ResourceBundle messages = null;

	JGridBagLayout layout = null;

	protected JTextField name;
	protected JTextField	ckpt_interface;
   protected JTextField ckpt_command;
   protected JTextField migr_command;
   protected JTextField rest_command;
   protected JTextField ckpt_dir;
 //protected JList      queue_list;
   protected JTextField when;
   protected JTextField signal;
   protected JTextField clean_command;
	
	
	public JCheckpointGeneralPanel(JDebug d) {
		super();
		layout = new JGridBagLayout();
		debug  = d;
		Locale locale = Locale.getDefault();
		messages      = ResourceBundle.getBundle("jqmon/views/MessageBundle", locale);
		setLayout(layout);
		initGUI();
	}

	protected void initGUI() {
		debug.DENTER("JCheckpointGeneralPanel.initGUI");
		
		JLabel label1 = new JLabel(messages.getString("checkpointlabel1"), JLabel.RIGHT);
		JLabel label2 = new JLabel(messages.getString("checkpointlabel2"), JLabel.RIGHT);
		JLabel label3 = new JLabel(messages.getString("checkpointlabel3"), JLabel.RIGHT);
		JLabel label4 = new JLabel(messages.getString("checkpointlabel4"), JLabel.RIGHT);
		JLabel label5 = new JLabel(messages.getString("checkpointlabel5"), JLabel.RIGHT);
		JLabel label6 = new JLabel(messages.getString("checkpointlabel6"), JLabel.RIGHT);
		JLabel label7 = new JLabel(messages.getString("checkpointlabel7"), JLabel.RIGHT);
		JLabel label8 = new JLabel(messages.getString("checkpointlabel8"), JLabel.RIGHT);
		JLabel label9 = new JLabel(messages.getString("checkpointlabel9"), JLabel.RIGHT);
      
		name			   = new JTextField(30);
		ckpt_interface = new JTextField(20);
      ckpt_command   = new JTextField(20);
      migr_command   = new JTextField(20);
      rest_command   = new JTextField(20);
      ckpt_dir       = new JTextField(20);
      when           = new JTextField(20);
      signal         = new JTextField(20);
      clean_command  = new JTextField(20);
      
		name.setEditable           (true);
		ckpt_interface.setEditable (true);
      ckpt_command.setEditable   (true);
      migr_command.setEditable   (true);
      rest_command.setEditable   (true);
      ckpt_dir.setEditable       (true);
      when.setEditable           (true); 
      signal.setEditable         (true);
      clean_command.setEditable  (true);
      
		name.requestFocus();

		layout.constrain(this, label1, 1,1,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
		layout.constrain(this, label2, 1,2,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
      layout.constrain(this, label3, 1,3,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
      layout.constrain(this, label4, 1,4,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
      layout.constrain(this, label5, 1,5,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
      layout.constrain(this, label6, 1,6,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
      layout.constrain(this, label7, 1,7,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
      layout.constrain(this, label8, 1,8,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
      layout.constrain(this, label9, 1,9,1,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHEAST, 0.0, 0.0, 4,4,4,4);
      
		layout.constrain(this, name,           2,1,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, ckpt_interface, 2,2,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, ckpt_command,   2,3,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, migr_command,   2,4,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, rest_command,   2,5,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, ckpt_dir,       2,6,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, when,           2,7,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, signal,         2,8,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
		layout.constrain(this, clean_command,  2,9,2,1,    GridBagConstraints.HORIZONTAL, GridBagConstraints.NORTHWEST, 1.0, 0.0, 4,4,4,4);
      
		layout.constrain(this, new JPanel(), 1,300,200,1,  GridBagConstraints.BOTH,       GridBagConstraints.NORTH,     1.0, 1.0, 0,0,0,0);
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
   //--------------------------------------

   // ---interface ------------------------   
	public void setInterface(String s) {
		ckpt_interface.setText(s);
	}
	public String getInterface() {
		return ckpt_interface.getText();
	}
   //--------------------------------------

   // ---ckpt_command----------------------
	public void setCkptCommand(String s) {
		ckpt_command.setText(s);
	}
	public String getCkptCommand() {
		return ckpt_command.getText();
	}
   //--------------------------------------
   
   // ---migr_command----------------------
	public void setMigrCommand(String s) {
		migr_command.setText(s);
	}
	public String getMigrCommand() {
		return migr_command.getText();
	}
   //--------------------------------------
   
   // ---rest_command----------------------
	public void setRestCommand(String s) {
		rest_command.setText(s);
	}
	public String getRestCommand() {
		return rest_command.getText();
	}
   //--------------------------------------
   
   // ---ckpt_dir--------------------------
   public void setCkptDir(String s) {
		ckpt_dir.setText(s);
	}
	public String getCkptDir() {
		return ckpt_dir.getText();
	}
   // -------------------------------------
   
   // ---when------------------------------
	public void setWhen(String s) {
		when.setText(s);
	}
	public String getWhen() {
		return when.getText();
	}
   // -------------------------------------
   
   // ---signal----------------------------
	public void setSignal(String s) {
		signal.setText(s);
	}
	public String getSignal() {
		return signal.getText();
	}
   // -------------------------------------
   
   // ---clean_command---------------------
	public void setCleanCommand(String s) {
		clean_command.setText(s);
	}
	public String getCleanCommand() {
		return clean_command.getText();
	}
   // -------------------------------------
}
