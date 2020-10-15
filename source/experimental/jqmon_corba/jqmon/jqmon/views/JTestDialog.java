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

import javax.swing.JOptionPane;
import javax.swing.JDialog;
import javax.swing.JTextField;
import java.beans.*;    // property change stuff
import java.awt.*;
import java.awt.event.*;

import jqmon.debug.*;


public class JTestDialog extends JDialog {
   private String typedText = null;
	
	JDebug debug;
	private final JTextField textField = null;	

   private JOptionPane optionPane;

   public String getValidatedText() {
      return typedText;
   }

   public JTestDialog(JDebug d) {
      super();

      setTitle("Dialog Test");

      debug = d;
      debug.DENTER("Class Constructor JTestDialog()");
	
      final String msgString1 = "Imput a long";
      final JTextField textField = new JTextField(10);
      Object[] array = {msgString1, textField};

      final String btnString1 = "Enter";
      final String btnString2 = "Cancel";
      Object[] options = {btnString1, btnString2};

      optionPane = new JOptionPane(array, 
                                   JOptionPane.QUESTION_MESSAGE,
                                   JOptionPane.YES_NO_OPTION,
                                   null,
                                   options,
                                   options[0]);
      setContentPane(optionPane);
      setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
      addWindowListener(new WindowAdapter() {
         public void windowClosing(WindowEvent we) {
            // instead of directly closing the window,
            // we're going to change the JOptionPane's
            // value property.
                 
            optionPane.setValue(new Integer(JOptionPane.CLOSED_OPTION));
         }
      });

      textField.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            optionPane.setValue(btnString1);
         }
      });

      optionPane.addPropertyChangeListener(new PropertyChangeListener() {
         public void propertyChange(PropertyChangeEvent e) {
            String prop = e.getPropertyName();

            if (isVisible() && (e.getSource() == optionPane) && (prop.equals(JOptionPane.VALUE_PROPERTY) || prop.equals(JOptionPane.INPUT_VALUE_PROPERTY))) {
               Object value = optionPane.getValue();

               if (value == JOptionPane.UNINITIALIZED_VALUE) {
                  //ignore reset
                  return;
               }

               if (value.equals(btnString1)) {
                  typedText = textField.getText();
                  // we're done; dismiss the dialog
                  setVisible(false);
               } 
               else { 
                  // user closed dialog or clicked cancel
                  setVisible(false);
               }
            }
         }
      });

      debug.DEXIT();
   }

   public void setLong(long l) {
      textField.setText((new Long(l)).toString());
   }
}
