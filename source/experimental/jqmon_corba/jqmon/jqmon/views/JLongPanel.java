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

import javax.swing.*;
import java.awt.event.*;
import javax.swing.event.*;

import jqmon.views.*;

import jqmon.debug.*;

public class JLongPanel extends JPanel implements AdjustmentListener, DocumentListener {
	//private Long value;
	private WholeNumberField noField;
	private JScrollBar scrollBar;
	
	JDebug debug = null;

	public JLongPanel(int value, int columns, JDebug debug) {
		super();
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		noField = new WholeNumberField(value,columns);
		noField.getDocument().addDocumentListener(this);
		scrollBar = new JScrollBar(JScrollBar.VERTICAL, value, 0, -2147483648, 2147483647);
		scrollBar.setUnitIncrement(-1);
		scrollBar.addAdjustmentListener(this);
		add(noField);
		add(scrollBar);

		this.debug = debug;
	}
	
	// implementing the interface AdjustmentListener
	public void adjustmentValueChanged(AdjustmentEvent e) { 
		if(e.getAdjustmentType() == AdjustmentEvent.TRACK) {	
			int i = e.getValue();
                	Integer integer = new Integer(i);
                	debug.DPRINTF("Scroll value: " + integer.toString());

			if(Math.abs(e.getValue()) != noField.getValue()) {
				// update the noField too
				noField.setValue(e.getValue());
			}
		}

/*		if(scrollBar.getValue() != noField.getValue()){
			noField.setValue(scrollBar.getValue());
		}
*/
	}

	//implementing the inteface DocumentListener
	public void changedUpdate(DocumentEvent e) {
		int i = noField.getValue();
		Integer integer = new Integer(i);
		scrollBar.setValue(noField.getValue());

		debug.DPRINTF("Field value: " + integer.toString());
		i = scrollBar.getValue();
		integer = new Integer(i);
		debug.DPRINTF("Scroll value: " + integer.toString());

	}

   // implementing the inteface DocumentListener
	public void removeUpdate(DocumentEvent e) {
		// do nothing
		// scrollBar.setValue(noField.getValue());
	}

	//implementing the inteface DocumentListener
   public void insertUpdate(DocumentEvent e){
      // do nothing
		// scrollBar.setValue(noField.getValue());	
		// int i = noField.getValue();
                //Integer integer = new Integer(i);
		// debug.DPRINTF("Field value: " + integer.toString());
		if(noField.getValue() != Math.abs(scrollBar.getValue())) {
			// update the scrollBar too
			scrollBar.setValue(noField.getValue());
		}
	}
	
	//
	public void setValue(int value) {
		noField.setValue(value);
		// the value of the scrollBar should be modified too
		// scrollBar.setValue(value);
	}
	
	public int getValue() {
		return noField.getValue();
	}

	//
	public void addActionListener(ActionListener l) {
		noField.addActionListener(l);
	}
}
