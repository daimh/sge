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
import javax.swing.text.*; 

import java.awt.Toolkit;
import java.text.NumberFormat;
import java.text.ParseException;
import java.util.Locale;


public class WholeNumberField extends JTextField {

   private Toolkit toolkit;
   private NumberFormat integerFormatter;

   public WholeNumberField(int value, int columns) {
      super(columns);
      setHorizontalAlignment(JTextField.RIGHT);
      toolkit = Toolkit.getDefaultToolkit();
      integerFormatter = NumberFormat.getNumberInstance(Locale.US);
      integerFormatter.setParseIntegerOnly(true);
      setValue(value);
   }

   public int getValue() {
      int retVal = 0;
      try {
         retVal = integerFormatter.parse(getText()).intValue();
      } 
      catch (ParseException e) {
         // this should never happen because insertString allows
         // only properly formatted data to get in the field.
         toolkit.beep();
      }
      return retVal;
   }

   public void setValue(int value) {
      setText(integerFormatter.format(value));
   }

   protected Document createDefaultModel() {
      return new WholeNumberDocument();
   }

   protected class WholeNumberDocument extends PlainDocument {

      public void insertString(int offs, String str, AttributeSet a) throws BadLocationException {

         char[] source = str.toCharArray();
         char[] result = new char[source.length];
         int j = 0;

         for (int i = 0; i < result.length; i++) {
            if (Character.isDigit(source[i]))
               result[j++] = source[i];
            else {
               toolkit.beep();
               System.err.println("insertString: " + source[i]);
            }
         }
         super.insertString(offs, new String(result, 0, j), a);
      }
   }
}
